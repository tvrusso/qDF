#include "rawTextDisplay.hpp"
#include "qDFProjReport.hpp"
#include <DF_Proj_Point.hpp>
#include <QString>
#include <vector>
rawTextDisplay::rawTextDisplay()
  : fileIsOpen_(false)
{
  CoordSysBuilder myCSB;
  QString myCSName="WGS84 Lat/Lon";
  // APRS always uses WGS84 no matter what our other settings may be
  myCS_ = myCSB.getCoordSys(myCSName);
}

rawTextDisplay::~rawTextDisplay()
{
  closeDisplay();
}

void rawTextDisplay::initializeDisplay()
{
  if (!fileIsOpen_)
  {
    theFile_.setFileName("foobar.txt");
    if (!theFile_.open(QIODevice::WriteOnly|QIODevice::Truncate|QIODevice::Text))
    {
      return;
      fileIsOpen_=false;
    }

    rawTextOut_.setDevice(&theFile_);
    rawTextOut_.setRealNumberNotation(QTextStream::FixedNotation);
    rawTextOut_.setRealNumberPrecision(8);
    fileIsOpen_=true;
    rawTextOut_ << "Hello, world!" << endl;
  }
  // if it was already open, we do nothing.
}

void rawTextDisplay::closeDisplay()
{
  if (fileIsOpen_)
  {
    rawTextOut_ << "File closed cleanly." << endl;
    rawTextOut_.flush();
    theFile_.close();
    fileIsOpen_=false;
  }
}

void rawTextDisplay::clearDisplay()
{
  //  To clear the "display" we want to clobber the file.  So close it
  // and reinitialize.
  closeDisplay();
  initializeDisplay();
}

void rawTextDisplay::displayDFReport(const qDFProjReport *theReport)
{
  if (fileIsOpen_)
  {
    rawTextOut_ << "Display Report: " << endl;
    rawTextOut_ << "   Name: " << theReport->getReportNameQS() << endl;
    if (theReport->isValid())
    {
      rawTextOut_ << "    **valid report." << endl;
    }
    DFLib::Proj::Point tempPoint=theReport->getReceiverPoint();
    tempPoint.setUserProj(myCS_.getProj4Params());
    vector<double> coords(2);
    coords=tempPoint.getUserCoords();
    rawTextOut_ << "     Coordinates: " << coords[0]<<","<<coords[1]<<endl;
    rawTextOut_ << "         Bearing: " << theReport->getBearing() << endl;
    rawTextOut_ << "        SD (deg): " << theReport->getSigma() << endl;

    QVector<double> lats;
    QVector<double> lons;
    computeRhumbline_(tempPoint, theReport->getBearing(),
                      2000, 20, lats,lons);
    rawTextOut_ << "  -----Bearing Line-----" << endl;
    for (int i=0;i<lats.size();i++)
    {
      rawTextOut_ << lons[i]<<","<<lats[i]<<",0"<<endl;
    }
  }
}

void rawTextDisplay::displayLSFix(DFLib::Proj::Point & LSFixPoint)
{
  DFLib::Proj::Point tempPoint = LSFixPoint;
  tempPoint.setUserProj(myCS_.getProj4Params());
  vector<double> coords=tempPoint.getUserCoords();
  if (fileIsOpen_)
  {
    rawTextOut_ << "Least Squares Fix:" << endl;
    rawTextOut_ << "     Coordinates: " << coords[0]<<","<<coords[1]<<endl;
  }
}

void rawTextDisplay::displayMLFix(DFLib::Proj::Point & MLFixPoint,
                                  double am2, double bm2, double phi)
{
  DFLib::Proj::Point tempPoint = MLFixPoint;
  tempPoint.setUserProj(myCS_.getProj4Params());
  vector<double> coords=tempPoint.getUserCoords();
  if (fileIsOpen_)
  {
    rawTextOut_ << "ML Fix:" << endl;
    rawTextOut_ << "     Coordinates: " << coords[0]<<","<<coords[1]<<endl;
    rawTextOut_ << "  ellipse params: " << am2 << " " << bm2 << " " << phi 
                << endl;
  }
}
void rawTextDisplay::displayBPEFix(DFLib::Proj::Point & BPEFixPoint,
                                  double am2, double bm2, double phi)
{
  DFLib::Proj::Point tempPoint = BPEFixPoint;
  tempPoint.setUserProj(myCS_.getProj4Params());
  vector<double> coords=tempPoint.getUserCoords();
  if (fileIsOpen_)
  {
    rawTextOut_ << "BPE Fix:" << endl;
    rawTextOut_ << "     Coordinates: " << coords[0]<<","<<coords[1]<<endl;
    rawTextOut_ << "  ellipse params: " << am2 << " " << bm2 << " " << phi 
                << endl;
    if (am2 != 0 && bm2!= 0)
    {
      QVector<double> lats;
      QVector<double> lons;
      rawTextOut_ << "  -----50% ellipse-----" << endl;
      computeEllipse_(tempPoint,am2,bm2,phi,sqrt(-2*log(1-.5)),20,
                      lats,lons);
      for (int i=0;i<lats.size();i++)
      {
        rawTextOut_ << lons[i]<<","<<lats[i]<<",0"<<endl;
      }
      rawTextOut_ << "  -----75% ellipse-----" << endl;
      computeEllipse_(tempPoint,am2,bm2,phi,sqrt(-2*log(1-.75)),20,
                      lats,lons);
      for (int i=0;i<lats.size();i++)
      {
        rawTextOut_ << lons[i]<<","<<lats[i]<<",0"<<endl;
      }
      rawTextOut_ << "  -----95% ellipse-----" << endl;
      computeEllipse_(tempPoint,am2,bm2,phi,sqrt(-2*log(1-.95)),20,
                      lats,lons);
      for (int i=0;i<lats.size();i++)
      {
        rawTextOut_ << lons[i]<<","<<lats[i]<<",0"<<endl;
      }

    }
  }
}
void rawTextDisplay::displayFCAFix(DFLib::Proj::Point & FCAFixPoint, std::vector<double> stddevs)
{
  DFLib::Proj::Point tempPoint = FCAFixPoint;
  tempPoint.setUserProj(myCS_.getProj4Params());
  vector<double> coords=tempPoint.getUserCoords();
  if (fileIsOpen_)
  {
    rawTextOut_ << "FCA Fix:" << endl;
    rawTextOut_ << "     Coordinates: " << coords[0]<<","<<coords[1]<<endl;
    rawTextOut_ << "             sds: " << stddevs[0]<<","<<stddevs[1]<<endl;

    if (stddevs[0] != 0 && stddevs[1]!= 0)
    {
      QVector<double> lats;
      QVector<double> lons;
      rawTextOut_ << "  -----SD ellipse-----" << endl;

      for (int i=0;i<20;i++)
      {
        lons.push_back(coords[0]+stddevs[0]*cos(2*M_PI/(19)*i));
        lats.push_back(coords[1]+stddevs[1]*sin(2*M_PI/(19)*i));
      }
      for (int i=0;i<lats.size();i++)
      {
        rawTextOut_ << lons[i]<<","<<lats[i]<<",0"<<endl;
      }
    }
  }
}

void rawTextDisplay::undisplayLSFix()
{
  if (fileIsOpen_)
  {
    rawTextOut_ << "LS Fix now invalid." << endl;
  }
}
void rawTextDisplay::undisplayBPEFix()
{
  if (fileIsOpen_)
  {
    rawTextOut_ << "BPE Fix now invalid." << endl;
  }
}
    
void rawTextDisplay::undisplayMLFix()
{
  if (fileIsOpen_)
  {
    rawTextOut_ << "ML Fix now invalid." << endl;
  }
}
void rawTextDisplay::undisplayFCAFix()
{
  if (fileIsOpen_)
  {
    rawTextOut_ << "FCA Fix now invalid." << endl;
  }
}

// low-level computations of stuff like rhumblines and ellipses
void rawTextDisplay::computeRhumbline_(DFLib::Proj::Point & startingPoint,
                                      double azimuth, double meters,
                                      int npoints,
                                      QVector<double> &lats, 
                                      QVector<double> &lons)
{
  DFLib::Proj::Point tempPoint=startingPoint;
  tempPoint.setUserProj(myCS_.getProj4Params());
  vector<double>  XYcoords=startingPoint.getXY();
  double ds=meters/(npoints-1);
  double azimuthRadians=azimuth*M_PI/180.0;
  double sintheta=sin(azimuthRadians);
  double costheta=cos(azimuthRadians);
  vector<double>tempCoords = XYcoords;

  lats.clear();
  lons.clear();

  for (int i=0;i<npoints;i++)
  {
    tempPoint.setXY(tempCoords);
    tempCoords=tempPoint.getUserCoords();
    lons.push_back(tempCoords[0]);
    lats.push_back(tempCoords[1]);
    tempCoords=tempPoint.getXY();
    tempCoords[0] += ds*sintheta;
    tempCoords[1] += ds*costheta;
  }
}  

  
  
void rawTextDisplay::computeEllipse_(DFLib::Proj::Point & centerPoint,
                                     double am2, double bm2, double phi,
                                     double rho, int npoints,
                                     QVector<double> &lats, 
                                     QVector<double> &lons)
{

  DFLib::Proj::Point tempPoint=centerPoint;
  tempPoint.setUserProj(myCS_.getProj4Params());
  vector<double>  XYcoords=centerPoint.getXY();
  vector<double>centerCoords = XYcoords;
  vector<double>tempCoords = XYcoords;
  double a=sqrt(1.0/am2);
  double b=sqrt(1.0/bm2);
  double cosphi=cos(phi);
  double sinphi=sin(phi);
  const double pi=M_PI;

  lats.clear();
  lons.clear();
  // this is the equation of an ellipse of axis a*rho and b*rho, rotated
  // by an angle phi.
  for (int i=0;i<npoints;i++)
  {
    tempCoords[0]=centerCoords[0]
      +a*rho*cosphi*cos(2.0*pi/((double)(npoints-1))*i)-b*rho*sinphi*sin(2.0*pi/((double)(npoints-1))*i);
    tempCoords[1]=centerCoords[1]
      +a*rho*sinphi*cos(2.0*pi/((double)(npoints-1))*i)+b*rho*cosphi*sin(2.0*pi/((double)(npoints-1))*i);
    
    // those are the coordinates in XY.  Now get 'em in lat/lon
    tempPoint.setXY(tempCoords);
    tempCoords=tempPoint.getUserCoords();
    lons.push_back(tempCoords[0]);
    lats.push_back(tempCoords[1]);
  }
}
