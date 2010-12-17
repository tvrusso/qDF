#include "rawTextDisplay.hpp"
#include "qDFProjReport.hpp"
#include <DF_Proj_Point.hpp>
#include <QString>

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
