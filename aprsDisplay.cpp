#include <QtGui>
#include "aprsDisplay.hpp"
#include "APRS.hpp"
#include "qDFProjReport.hpp"
#include "CoordSysBuilder.hpp"

// Display class for APRS

// Constructor
aprsDisplay::aprsDisplay(APRS * theAPRS, QPlainTextEdit *theTextEdit)
  :theAPRS_(theAPRS),
   aprsPacketsTextEdit_(theTextEdit)
{
  CoordSysBuilder myCSB;
  QString myCSName="WGS84 Lat/Lon";
  // APRS always uses WGS84 no matter what our other settings may be
  myCS_ = myCSB.getCoordSys(myCSName);
}

// public implementations of qDFDisplayInterface methods
void aprsDisplay::initializeDisplay()
{
  clearDisplay();
}

void aprsDisplay::clearDisplay()
{
  QStringList foo=theAPRS_->deleteAllObjects();
  foreach(QString str,foo)
  {
    displayAPRSText_(str);
  }
}

void aprsDisplay::closeDisplay()
{
  clearDisplay();
}

void aprsDisplay::displayDFReport(const qDFProjReport *theReport)
{
  if (theReport->isValid())
  {
    DFLib::Proj::Point tempPoint=theReport->getReceiverPoint();
    tempPoint.setUserProj(myCS_.getProj4Params());
    vector<double> coords(2);
    coords=tempPoint.getUserCoords();
    QString oName=theReport->getReportNameQS();
    QString aprsPosit=theAPRS_->createDFObject(oName,coords,
                                             theReport->getBearing(),
                                             theReport->getSigma(),
                                             " via qDF");
    displayAPRSText_(aprsPosit);
  }
  else
  {
    QString oName=theReport->getReportNameQS();
    QString aprsPosit=theAPRS_->deleteObject(oName);
    if (!aprsPosit.isEmpty()) displayAPRSText_(aprsPosit);
  }
}

void aprsDisplay::undisplayDFReport(const qDFProjReport *theReport)
{
    QString oName=theReport->getReportNameQS();
    QString aprsPosit=theAPRS_->deleteObject(oName);
    if (!aprsPosit.isEmpty()) displayAPRSText_(aprsPosit);
}

void aprsDisplay::displayLSFix(DFLib::Proj::Point & LSFixPoint)
{
  DFLib::Proj::Point tempPoint = LSFixPoint;
  tempPoint.setUserProj(myCS_.getProj4Params());
  vector<double> LS_point=tempPoint.getUserCoords();
  aprsPointObject_("LS-Fix",LS_point,"Ln"," Least Squares Solution");
}

void aprsDisplay::undisplayLSFix()
{
  deleteAPRSObject_("LS-Fix");
}

void aprsDisplay::displayFCAFix(DFLib::Proj::Point & FCAFixPoint, std::vector<double> stddevs)
{
  DFLib::Proj::Point tempPoint = FCAFixPoint;
  tempPoint.setUserProj(myCS_.getProj4Params());
  vector<double> FCA_point=tempPoint.getUserCoords();
  aprsPointObject_("FCA-Fix",FCA_point,"An"," Fix Cut Average Solution");
  if (stddevs[0] != 0 && stddevs[1]!=0)
  {
    aprsDFErrorObject_("FCA-err", FCA_point,stddevs);
  }
}

void aprsDisplay::undisplayFCAFix()
{
  deleteAPRSObject_("FCA-Fix");
  deleteAPRSObject_("FCA-err");
}

void aprsDisplay::displayMLFix(DFLib::Proj::Point & MLFixPoint, 
                               double am2, double bm2, double phi)
{

  DFLib::Proj::Point tempPoint = MLFixPoint;
  tempPoint.setUserProj(myCS_.getProj4Params());
  vector<double> ML_point=tempPoint.getUserCoords();
  aprsPointObject_("ML-Fix",ML_point,"Mn"," Maximum Likelihood Solution");

}

void aprsDisplay::undisplayMLFix()
{
  deleteAPRSObject_("ML-Fix");
}

void aprsDisplay::displayBPEFix(DFLib::Proj::Point & BPEFixPoint, 
                                double am2, double bm2, double phi)
{
  DFLib::Proj::Point tempPoint = BPEFixPoint;
  tempPoint.setUserProj(myCS_.getProj4Params());
  vector<double> BPE_point=tempPoint.getUserCoords();

  aprsPointObject_("BPE-Fix",BPE_point,"Sn"," Stansfield BPE");
  aprsRotatedEllipse_("SErr50",BPEFixPoint,am2,bm2,phi,50);
  aprsRotatedEllipse_("SErr75",BPEFixPoint,am2,bm2,phi,75);
  aprsRotatedEllipse_("SErr95",BPEFixPoint,am2,bm2,phi,95);
}

void aprsDisplay::undisplayBPEFix()
{
  deleteAPRSObject_("BPE-Fix");
  deleteAPRSObject_("SErr50");
  deleteAPRSObject_("SErr75");
  deleteAPRSObject_("SErr95");
}

// Private methods
void aprsDisplay::displayAPRSText_(const QString & str)
{
  aprsPacketsTextEdit_->insertPlainText(str);
  aprsPacketsTextEdit_->insertPlainText("\n");
}  

void aprsDisplay::aprsPointObject_(const QString &oName, 
                                 const vector<double>& oPoint,
                                 const QString &oSym,
                                 const QString & oComment)
{
  QString aprsPosit=theAPRS_->createObject(oName,oPoint,oSym,oComment);
  displayAPRSText_(aprsPosit);
}

void aprsDisplay::aprsDFErrorObject_(const QString &oName,
                                   const vector<double>&oPoint,
                                   const vector<double>&oSDs)
{
    QString aprsPosit=theAPRS_->createDFErrorObject(oName,oPoint,oSDs[0],oSDs[1]);
    displayAPRSText_(aprsPosit);
}

void aprsDisplay::deleteAPRSObject_(const QString &oName)
{
  QString aprsPosit=theAPRS_->deleteObject(oName);
  if (!aprsPosit.isEmpty()) 
    displayAPRSText_(aprsPosit);
}

void aprsDisplay::aprsRotatedEllipse_(const QString &oName, DFLib::Proj::Point &thePoint,
                                       double am2, double bm2, double phi,
                                       int percent)
{
  if (am2>0 && bm2>0)
  {
    
    // this is tricky.  We need to calculate the Stansfield ellipses in 
    // XY coordinates, then convert their points to lat/lon before generating
    // the APRS object.
    
    double P=((double)percent)/100.0;
    double rho=sqrt(-2*log(1-P));
    DFLib::Proj::Point tempPoint(thePoint);
    tempPoint.setUserProj(myCS_.getProj4Params());
    vector<double> lats;
    vector<double> lons;
    vector<double> centerCoords=thePoint.getXY();
    vector<double> tempCoords(2);
    double a=sqrt(1.0/am2);
    double b=sqrt(1.0/bm2);
    double cosphi=cos(phi);
    double sinphi=sin(phi);
    const double pi=4*atan(1.0);
    
    // this is the equation of an ellipse of axis a*rho and b*rho, rotated
    // by an angle phi.
    for (int i=0;i<16;i++)
    {
      tempCoords[0]=centerCoords[0]
        +a*rho*cosphi*cos(2.0*pi/15.0*i)-b*rho*sinphi*sin(2.0*pi/15.0*i);
      tempCoords[1]=centerCoords[1]
        +a*rho*sinphi*cos(2.0*pi/15.0*i)+b*rho*cosphi*sin(2.0*pi/15.0*i);
      
      // those are the coordinates in XY.  Now get 'em in lat/lon
      tempPoint.setXY(tempCoords);
      tempCoords=tempPoint.getUserCoords();
      lons.push_back(tempCoords[0]);
      lats.push_back(tempCoords[1]);
    }
    
    // we now have our rotated ellipse transformed to lat/lon.  Make the
    // APRS object
    char colorStyle;
    if (percent>=50)
      colorStyle='g';
    if (percent>=75)
      colorStyle='e';
    if (percent>90)
      colorStyle='a';
    tempPoint.setXY(centerCoords);
    centerCoords=tempPoint.getUserCoords();
    QString aprsPosit=
      theAPRS_->createMultilineObject(oName,
                                      lats,lons,centerCoords,
                                      colorStyle,0,
                                      "\\l");
    displayAPRSText_(aprsPosit);
  }
}
