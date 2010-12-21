#include "kmlDisplay.hpp"
#include "qDFProjReport.hpp"
#include <DF_Proj_Point.hpp>
#include <QString>
#include <vector>
kmlDisplay::kmlDisplay(QString theFileName)
  :theFileName_(theFileName)
{
  CoordSysBuilder myCSB;
  QString myCSName="WGS84 Lat/Lon";
  // APRS always uses WGS84 no matter what our other settings may be
  myCS_ = myCSB.getCoordSys(myCSName);

  theDataFileName_=theFileName_.left(theFileName.lastIndexOf(".kml"));
  theDataFileName_.append("_data.kml");
}

kmlDisplay::~kmlDisplay()
{
  closeDisplay();
}

void kmlDisplay::initializeDisplay()
{
  commitMaster_();  // output the master file with refresh turned on
  clearDisplay();   // this will also output the data file (which will be 
                    // empty.
  opened_=true;
}

void kmlDisplay::closeDisplay()
{
  commitMaster_(false); // output a version of the master file wit refresh
                        // turned off.
  opened_=false;
}

void kmlDisplay::clearDisplay()
{
  dfReportStrings_.clear();
  dfFixStrings_.clear();
  dfEllipseStrings_.clear();
  commit_();
}

void kmlDisplay::displayDFReport(const qDFProjReport *theReport)
{
  QString myTempString;
  QTextStream kmlStringStream_(&myTempString);
  kmlStringStream_.setRealNumberNotation(QTextStream::FixedNotation);
  kmlStringStream_.setRealNumberPrecision(8);

  if (theReport->isValid())
  {
    QString theEquipType = theReport->getEquipType();
    QString theQuality = theReport->getQuality();
    kmlStringStream_ << "<Placemark>" << endl;
    kmlStringStream_ << "  <name>" << theReport->getReportNameQS()<<"</name>"<< endl;
    kmlStringStream_ << "  <description>"  << " DF Report Equipment:" << theEquipType
            << " Quality:" 
            << theQuality
            << " Bearing: " <<theReport->getBearing() 
            << " SD (deg): " << theReport->getSigma() 
            << "</description>"
            << endl;
    kmlStringStream_ << "  <styleUrl>#DFBearingLines</styleUrl>" << endl;
    kmlStringStream_ << "  <MultiGeometry>" <<endl;
    kmlStringStream_ << "    <Point>" << endl;
    
    DFLib::Proj::Point tempPoint=theReport->getReceiverPoint();
    tempPoint.setUserProj(myCS_.getProj4Params());
    vector<double> coords(2);
    coords=tempPoint.getUserCoords();
    kmlStringStream_ << "     <coordinates>" << coords[0]<<","<<coords[1]
                     << ",0</coordinates>" << endl;
    kmlStringStream_ << "    </Point>" << endl;
    kmlStringStream_ << "    <LineString>" << endl;
    kmlStringStream_ << "      <tessellate>1</tessellate>" << endl;
    kmlStringStream_ << "      <coordinates> " << endl;

    QVector<double> lats;
    QVector<double> lons;
    computeRhumbline_(tempPoint, theReport->getBearing(),
                      2000, 20, lats,lons);
    for (int i=0;i<lats.size();i++)
    {
      kmlStringStream_ << "       "<<lons[i]<<","<<lats[i]<<",0"<<endl;
    }
    kmlStringStream_ << "      </coordinates> " << endl;
    kmlStringStream_ << "    </LineString> " << endl;
    kmlStringStream_ << "  </MultiGeometry> " << endl;
    kmlStringStream_ << "</Placemark> " << endl;

    dfReportStrings_[theReport->getReportNameQS()] = myTempString;

  }
  else
  {
    dfReportStrings_[theReport->getReportNameQS()]="";
  }

  commit_();
}

void kmlDisplay::undisplayDFReport(const qDFProjReport *theReport)
{
  dfReportStrings_[theReport->getReportNameQS()]="";
  commit_();
}

void kmlDisplay::displayLSFix(DFLib::Proj::Point & LSFixPoint)
{
  QString myTempString;
  QTextStream kmlStringStream_(&myTempString);
  kmlStringStream_.setRealNumberNotation(QTextStream::FixedNotation);
  kmlStringStream_.setRealNumberPrecision(8);

  kmlStringStream_ << "<Placemark>" << endl;
  kmlStringStream_ << "  <name>Least Squares Fix</name>"<< endl;
    kmlStringStream_ << "  <description>"  << " Least Squares Fix"
            << "</description>"
            << endl;
    kmlStringStream_ << "  <styleUrl>#FixPoints</styleUrl>" << endl;
    kmlStringStream_ << "  <Point>" << endl;
  DFLib::Proj::Point tempPoint = LSFixPoint;
  tempPoint.setUserProj(myCS_.getProj4Params());
  vector<double> coords=tempPoint.getUserCoords();
  kmlStringStream_ << "Least Squares Fix:" << endl;
  kmlStringStream_ << "      <coordinates>" << coords[0]<<","<<coords[1]
          << ",0</coordinates>" << endl;
  kmlStringStream_ << "    </Point>" << endl;
  kmlStringStream_ << "</Placemark>"<< endl;
  dfFixStrings_["LSFix"]=myTempString;
  commit_();
}

void kmlDisplay::displayMLFix(DFLib::Proj::Point & MLFixPoint,
                                  double am2, double bm2, double phi)
{
  QString myTempString;
  QTextStream kmlStringStream_(&myTempString);
  kmlStringStream_.setRealNumberNotation(QTextStream::FixedNotation);
  kmlStringStream_.setRealNumberPrecision(8);

  kmlStringStream_ << "<Placemark>" << endl;
  kmlStringStream_ << "  <name>Max Likelihood Fix</name>"<< endl;
  kmlStringStream_ << "  <description>"  << "Maximum Likelihood Fix"
          << "</description>"
          << endl;
  kmlStringStream_ << "  <styleUrl>#FixPoints</styleUrl>" << endl;
  kmlStringStream_ << "  <Point>" << endl;
  DFLib::Proj::Point tempPoint = MLFixPoint;
  tempPoint.setUserProj(myCS_.getProj4Params());
  vector<double> coords=tempPoint.getUserCoords();
  kmlStringStream_ << "      <coordinates>" << coords[0]<<","<<coords[1]
          << ",0</coordinates>" << endl;
  kmlStringStream_ << "    </Point>" << endl;
  kmlStringStream_ << "</Placemark>"<< endl;
  dfFixStrings_["MLFix"]=myTempString;
  
  myTempString="";
  if (am2 != 0 && bm2!= 0)
  {
    QVector<double> lats;
    QVector<double> lons;
    kmlStringStream_ << "<Placemark>" << endl;
    kmlStringStream_ << "  <name>ML 50%</name>" << endl;
    kmlStringStream_ << "  <description>Ellipse containing 50% probability zone for Maximum Likelihood Fix</description>" << endl;
    kmlStringStream_ << "  <styleUrl>#50PercentEllipses</styleUrl>"<<endl;
    kmlStringStream_ << "  <LineString>"<< endl;
    kmlStringStream_ << "    <tessellate>1</tessellate>" <<endl;
    kmlStringStream_ << "    <coordinates>" << endl;
    computeEllipse_(tempPoint,am2,bm2,phi,sqrt(-2*log(1-.5)),20,
                    lats,lons);
    for (int i=0;i<lats.size();i++)
    {
      kmlStringStream_ << "      " << lons[i]<<","<<lats[i]<<",0"<<endl;
    }
    kmlStringStream_ << "    </coordinates>" << endl;
    kmlStringStream_ << "  </LineString>"<< endl;
    kmlStringStream_ << "</Placemark>" << endl;
    kmlStringStream_ << "<Placemark>" << endl;
    kmlStringStream_ << "  <name>ML 75%</name>" << endl;
    kmlStringStream_ << "  <description>Ellipse containing 75% probability zone for Maximum Likelihood Fix</description>" << endl;
    kmlStringStream_ << "  <styleUrl>#75PercentEllipses</styleUrl>"<<endl;
    kmlStringStream_ << "  <LineString>"<< endl;
    kmlStringStream_ << "    <tessellate>1</tessellate>" <<endl;
    kmlStringStream_ << "    <coordinates>" << endl;
    computeEllipse_(tempPoint,am2,bm2,phi,sqrt(-2*log(1-.75)),20,
                    lats,lons);
    for (int i=0;i<lats.size();i++)
    {
      kmlStringStream_ << "      " << lons[i]<<","<<lats[i]<<",0"<<endl;
    }
    kmlStringStream_ << "    </coordinates>" << endl;
    kmlStringStream_ << "  </LineString>"<< endl;
    kmlStringStream_ << "</Placemark>" << endl;
    kmlStringStream_ << "<Placemark>" << endl;
    kmlStringStream_ << "  <name>ML 95%</name>" << endl;
    kmlStringStream_ << "  <description>Ellipse containing 75% probability zone for Maximum Likelihood Fix</description>" << endl;
    kmlStringStream_ << "  <styleUrl>#95PercentEllipses</styleUrl>"<<endl;
    kmlStringStream_ << "  <LineString>"<< endl;
    kmlStringStream_ << "    <tessellate>1</tessellate>" <<endl;
    kmlStringStream_ << "    <coordinates>" << endl;
    computeEllipse_(tempPoint,am2,bm2,phi,sqrt(-2*log(1-.95)),20,
                    lats,lons);
    for (int i=0;i<lats.size();i++)
    {
      kmlStringStream_ << "      " << lons[i]<<","<<lats[i]<<",0"<<endl;
    }
    kmlStringStream_ << "    </coordinates>" << endl;
    kmlStringStream_ << "  </LineString>"<< endl;
    kmlStringStream_ << "</Placemark>" << endl;

    dfEllipseStrings_["MLFix"]=myTempString;
  }
  else
    dfEllipseStrings_["MLFix"]="";

  commit_();
}
void kmlDisplay::displayBPEFix(DFLib::Proj::Point & BPEFixPoint,
                                  double am2, double bm2, double phi)
{
  QString myTempString;
  QTextStream kmlStringStream_(&myTempString);
  kmlStringStream_.setRealNumberNotation(QTextStream::FixedNotation);
  kmlStringStream_.setRealNumberPrecision(8);

  kmlStringStream_ << "<Placemark>" << endl;
  kmlStringStream_ << "  <name>BPE Fix</name>"<< endl;
  kmlStringStream_ << "  <description>"  << "Stansfield Best Position Estimate Fix"
          << "</description>"
          << endl;
  kmlStringStream_ << "  <styleUrl>#FixPoints</styleUrl>" << endl;
  kmlStringStream_ << "  <Point>" << endl;
  DFLib::Proj::Point tempPoint = BPEFixPoint;
  tempPoint.setUserProj(myCS_.getProj4Params());
  vector<double> coords=tempPoint.getUserCoords();
  kmlStringStream_ << "      <coordinates>" << coords[0]<<","<<coords[1]
          << ",0</coordinates>" << endl;
  kmlStringStream_ << "    </Point>" << endl;
  kmlStringStream_ << "</Placemark>"<< endl;
  dfFixStrings_["BPEFix"]=myTempString;

  myTempString="";
  if (am2 != 0 && bm2!= 0)
  {
    QVector<double> lats;
    QVector<double> lons;
    kmlStringStream_ << "<Placemark>" << endl;
    kmlStringStream_ << "  <name>BPE 50%</name>" << endl;
    kmlStringStream_ << "  <description>Ellipse containing 50% probability zone for Stansfield Fix</description>" << endl;
    kmlStringStream_ << "  <styleUrl>#50PercentEllipses</styleUrl>"<<endl;
    kmlStringStream_ << "  <LineString>"<< endl;
    kmlStringStream_ << "    <tessellate>1</tessellate>" <<endl;
    kmlStringStream_ << "    <coordinates>" << endl;
    computeEllipse_(tempPoint,am2,bm2,phi,sqrt(-2*log(1-.5)),20,
                    lats,lons);
    for (int i=0;i<lats.size();i++)
    {
      kmlStringStream_ << "      " << lons[i]<<","<<lats[i]<<",0"<<endl;
    }
    kmlStringStream_ << "    </coordinates>" << endl;
    kmlStringStream_ << "  </LineString>"<< endl;
    kmlStringStream_ << "</Placemark>" << endl;
    kmlStringStream_ << "<Placemark>" << endl;
    kmlStringStream_ << "  <name>BPE 75%</name>" << endl;
    kmlStringStream_ << "  <description>Ellipse containing 75% probability zone for Stansfield Fix</description>" << endl;
    kmlStringStream_ << "  <styleUrl>#75PercentEllipses</styleUrl>"<<endl;
    kmlStringStream_ << "  <LineString>"<< endl;
    kmlStringStream_ << "    <tessellate>1</tessellate>" <<endl;
    kmlStringStream_ << "    <coordinates>" << endl;
    computeEllipse_(tempPoint,am2,bm2,phi,sqrt(-2*log(1-.75)),20,
                    lats,lons);
    for (int i=0;i<lats.size();i++)
    {
      kmlStringStream_ << "      " << lons[i]<<","<<lats[i]<<",0"<<endl;
    }
    kmlStringStream_ << "    </coordinates>" << endl;
    kmlStringStream_ << "  </LineString>"<< endl;
    kmlStringStream_ << "</Placemark>" << endl;
    kmlStringStream_ << "<Placemark>" << endl;
    kmlStringStream_ << "  <name>BPE 95%</name>" << endl;
    kmlStringStream_ << "  <description>Ellipse containing 95% probability zone for Stansfield Fix</description>" << endl;
    kmlStringStream_ << "  <styleUrl>#95PercentEllipses</styleUrl>"<<endl;
    kmlStringStream_ << "  <LineString>"<< endl;
    kmlStringStream_ << "    <tessellate>1</tessellate>" <<endl;
    kmlStringStream_ << "    <coordinates>" << endl;
    computeEllipse_(tempPoint,am2,bm2,phi,sqrt(-2*log(1-.95)),20,
                    lats,lons);
    for (int i=0;i<lats.size();i++)
    {
      kmlStringStream_ << "      " << lons[i]<<","<<lats[i]<<",0"<<endl;
    }
    kmlStringStream_ << "    </coordinates>" << endl;
    kmlStringStream_ << "  </LineString>"<< endl;
    kmlStringStream_ << "</Placemark>" << endl;

    dfEllipseStrings_["BPEFix"]=myTempString;
  }
  else
    dfEllipseStrings_["BPEFix"]="";

  commit_();
}

void kmlDisplay::displayFCAFix(DFLib::Proj::Point & FCAFixPoint, std::vector<double> stddevs)
{
  QString myTempString;
  QTextStream kmlStringStream_(&myTempString);
  kmlStringStream_.setRealNumberNotation(QTextStream::FixedNotation);
  kmlStringStream_.setRealNumberPrecision(8);

  kmlStringStream_ << "<Placemark>" << endl;
  kmlStringStream_ << "  <name>FCA Fix</name>"<< endl;
  kmlStringStream_ << "  <description>"  << "Fix Cut Average"
          << "</description>"
          << endl;
  kmlStringStream_ << "  <styleUrl>#FixPoints</styleUrl>" << endl;
  kmlStringStream_ << "  <Point>" << endl;
  DFLib::Proj::Point tempPoint = FCAFixPoint;
  tempPoint.setUserProj(myCS_.getProj4Params());
  vector<double> coords=tempPoint.getUserCoords();
  kmlStringStream_ << "      <coordinates>" << coords[0]<<","<<coords[1]
          << ",0</coordinates>" << endl;
  kmlStringStream_ << "    </Point>" << endl;
  kmlStringStream_ << "</Placemark>"<< endl;
  dfFixStrings_["FCAFix"]=myTempString;

  myTempString="";

  if (stddevs[0] != 0 && stddevs[1]!= 0)
  {
    QVector<double> lats;
    QVector<double> lons;
    
    for (int i=0;i<20;i++)
    {
      lons.push_back(coords[0]+stddevs[0]*cos(2*M_PI/(19)*i));
      lats.push_back(coords[1]+stddevs[1]*sin(2*M_PI/(19)*i));
    }
    
    kmlStringStream_ << "<Placemark>" << endl;
    kmlStringStream_ << "  <name>FCA Standard Dev</name>" << endl;
    kmlStringStream_ << "  <description>Ellipse major and minor axes correspond to standard deviation of fix cuts in latitude and longitude directions</description>" << endl;
    kmlStringStream_ << "  <styleUrl>#50PercentEllipses</styleUrl>"<<endl;
    kmlStringStream_ << "  <LineString>"<< endl;
    kmlStringStream_ << "    <tessellate>1</tessellate>" <<endl;
    kmlStringStream_ << "    <coordinates>" << endl;
    for (int i=0;i<lats.size();i++)
    {
      kmlStringStream_ << "      " << lons[i]<<","<<lats[i]<<",0"<<endl;
    }
    kmlStringStream_ << "    </coordinates>" << endl;
    kmlStringStream_ << "  </LineString>"<< endl;
    kmlStringStream_ << "</Placemark>" << endl;
    dfEllipseStrings_["FCAFix"] = myTempString;
  }
  else
    dfEllipseStrings_["FCAFix"]="";

  commit_();
}

void kmlDisplay::undisplayLSFix()
{
  dfFixStrings_["LSFix"]="";
  commit_();

}
void kmlDisplay::undisplayBPEFix()
{
  dfFixStrings_["BPEFix"]="";
  dfEllipseStrings_["BPEFix"]="";
  commit_();
}
    
void kmlDisplay::undisplayMLFix()
{
  dfFixStrings_["MLFix"]="";
  dfEllipseStrings_["MLFix"]="";
  commit_();
}
void kmlDisplay::undisplayFCAFix()
{
  dfFixStrings_["FCAFix"]="";
  dfEllipseStrings_["FCAFix"]="";
  commit_();
}

// class-specific public methods:
void kmlDisplay::setFileName(QString theFileName)
{
  theFileName_=theFileName_;
  theDataFileName_=theFileName_.left(theFileName.lastIndexOf(".kml")+1);
  theDataFileName_.append("_data.kml");

  commitMaster_();
  commit_();
}

// class-specific private methods
void kmlDisplay::commit_()
{

  // Ok.  Here's the problem:  If we always commit when asked, even after a 
  // close, then we'll wipe out our fixes when qDF processes 
  // the "collectionCleared" signal.  Ick.

  if (opened_)
  {
    theFile_.setFileName(theDataFileName_);
    if (theFile_.open(QIODevice::WriteOnly|QIODevice::Truncate|QIODevice::Text))
    {
      kmlFileOut_.setDevice(&theFile_);
      kmlFileOut_.setRealNumberNotation(QTextStream::FixedNotation);
      kmlFileOut_.setRealNumberPrecision(8);
      
      outputPreamble_();
      commitReports_();
      commitFixes_();
      commitEllipses_();
      outputFooter_();
      kmlFileOut_.flush();
      theFile_.close();
    }
  }
}

void kmlDisplay::commitMaster_(bool refresh)
{
  theFile_.setFileName(theFileName_);
  if (theFile_.open(QIODevice::WriteOnly|QIODevice::Truncate|QIODevice::Text))
  {
    kmlFileOut_.setDevice(&theFile_);
    kmlFileOut_.setRealNumberNotation(QTextStream::FixedNotation);
    kmlFileOut_.setRealNumberPrecision(8);

    kmlFileOut_<<"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"<<endl;
    kmlFileOut_<<"<kml xmlns=\"http://www.opengis.net/kml/2.2\">" << endl;
    kmlFileOut_<<"  <Document>" << endl;
    kmlFileOut_<<"    <NetworkLink>"<<endl;
    kmlFileOut_<<"      <name>DF Solution: " << theFileName_ << " </name>"<<endl;
    kmlFileOut_<<"      <description>Produced KM5VY\'s qDF</description>"<<endl;
    kmlFileOut_<<"      <flyToView>0</flyToView>"<<endl;
    kmlFileOut_<<"      <Link>"<<endl;
    kmlFileOut_<<"        <href>"<<theDataFileName_<<"</href>"<<endl;
    if (refresh)
    {
      kmlFileOut_<<"        <refreshMode>onInterval</refreshMode>"<<endl;
      kmlFileOut_<<"        <refreshInterval>10</refreshInterval>"<<endl;
      kmlFileOut_<<"        <viewRefreshMode>onStop</viewRefreshMode>"<<endl;
      kmlFileOut_<<"        <viewRefreshTime>7</viewRefreshTime>" << endl;
      kmlFileOut_<<"        <viewFormat></viewFormat>"<<endl;
    }
    kmlFileOut_<<"      </Link>"<<endl;
    kmlFileOut_<<"    </NetworkLink>"<<endl;
    kmlFileOut_<<"  </Document>" << endl;
    kmlFileOut_<<"</kml>"<<endl;


    kmlFileOut_.flush();
    theFile_.close();
  }
}


void kmlDisplay::outputPreamble_()
{
    kmlFileOut_<<"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"<<endl;
    kmlFileOut_<<"<kml xmlns=\"http://www.opengis.net/kml/2.2\">" << endl;
    kmlFileOut_<<"  <Document>" << endl;
    kmlFileOut_<<"    <name>DF Solution: " << theFileName_ << " </name>"<<endl;
    kmlFileOut_<<"    <description>Produced KM5VY\'s qDF</description>"<<endl;
    kmlFileOut_<<"    <Style id=\"DFBearingLines\">"<<endl;
    kmlFileOut_<<"      <IconStyle>"<<endl;
    kmlFileOut_<<"        <color>7d0000ff</color>"<<endl;
    kmlFileOut_<<"      </IconStyle>"<<endl;
    kmlFileOut_<<"      <LineStyle>"<<endl;
    kmlFileOut_<<"        <width>1.5</width>"<<endl;
    kmlFileOut_<<"        <color>7d0000ff</color>"<<endl;
    kmlFileOut_<<"      </LineStyle>"<<endl;
    kmlFileOut_<<"    </Style>"<<endl;
    kmlFileOut_<<"    <Style id=\"FixPoints\">"<<endl;
    kmlFileOut_<<"      <IconStyle>"<<endl;
    kmlFileOut_<<"        <color>7f00ff00</color>"<<endl;
    kmlFileOut_<<"      </IconStyle>"<<endl;
    kmlFileOut_<<"    </Style>"<<endl;
    kmlFileOut_<<"    <Style id=\"50PercentEllipses\">"<<endl;
    kmlFileOut_<<"      <LineStyle>"<<endl;
    kmlFileOut_<<"        <width>1.5</width>"<<endl;
    kmlFileOut_<<"        <color>7fff0000</color>"<<endl;
    kmlFileOut_<<"      </LineStyle>"<<endl;
    kmlFileOut_<<"    </Style>"<<endl;
    kmlFileOut_<<"    <Style id=\"75PercentEllipses\">"<<endl;
    kmlFileOut_<<"      <LineStyle>"<<endl;
    kmlFileOut_<<"        <width>1.5</width>"<<endl;
    kmlFileOut_<<"        <color>7f00ffff</color>"<<endl;
    kmlFileOut_<<"      </LineStyle>"<<endl;
    kmlFileOut_<<"    </Style>"<<endl;
    kmlFileOut_<<"    <Style id=\"95PercentEllipses\">"<<endl;
    kmlFileOut_<<"      <LineStyle>"<<endl;
    kmlFileOut_<<"        <width>1.5</width>"<<endl;
    kmlFileOut_<<"        <color>7f0000ff</color>"<<endl;
    kmlFileOut_<<"      </LineStyle>"<<endl;
    kmlFileOut_<<"    </Style>"<<endl;
}
void kmlDisplay::commitReports_()
{
  kmlFileOut_<<"    <Folder>"<<endl;
  kmlFileOut_<<"      <name>Reports</name>"<<endl;

  foreach(QString key, dfReportStrings_.keys())
  {
    kmlFileOut_<< dfReportStrings_[key] <<endl;
  }
  kmlFileOut_<<"    </Folder>"<<endl;
}

void kmlDisplay::commitFixes_()
{
  kmlFileOut_<<"    <Folder>"<<endl;
  kmlFileOut_<<"      <name>Fixes</name>"<<endl;

  foreach(QString key, dfFixStrings_.keys())
  {
    kmlFileOut_<< dfFixStrings_[key] <<endl;
  }
  kmlFileOut_<<"    </Folder>"<<endl;
}

void kmlDisplay::commitEllipses_()
{
  kmlFileOut_<<"    <Folder>"<<endl;
  kmlFileOut_<<"      <name>Error Ellipses</name>"<<endl;
  
  foreach(QString key, dfEllipseStrings_.keys())
  {
    kmlFileOut_<<"        <Folder>"<<endl;
    kmlFileOut_<<"          <name>"<<key<<" error ellipses</name>"<<endl;
    kmlFileOut_<< dfEllipseStrings_[key] << endl;
    kmlFileOut_<<"        </Folder>"<<endl;
  }
  kmlFileOut_<<"    </Folder>"<<endl;
}

void kmlDisplay::outputFooter_()
{
    kmlFileOut_<<"  </Document>" << endl;
    kmlFileOut_<<"</kml>"<<endl;
}

// low-level computations of stuff like rhumblines and ellipses
void kmlDisplay::computeRhumbline_(DFLib::Proj::Point & startingPoint,
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

  
  
void kmlDisplay::computeEllipse_(DFLib::Proj::Point & centerPoint,
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
