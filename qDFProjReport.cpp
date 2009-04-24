#include <QDataStream>

#include "qDFProjReport.hpp"
#include <sstream>

qDFProjReport::qDFProjReport(const vector <double> &theLocationUser,
                const double &bearing, const double &std_dev,
                const string &theName, const CoordSys &aCS)
  :DFLib::Proj::Report(theLocationUser,bearing,std_dev,
                       theName, aCS.getProj4Params()),
   theCS_(aCS)
{
}

qDFProjReport::~qDFProjReport()
{
}

void qDFProjReport::setReceiverLocationUser(vector<double> &theLocation)
{
  DFLib::Proj::Report::setReceiverLocationUser(theLocation);
  emit reportChanged();
}

void qDFProjReport::setReceiverLocationMercator(vector<double> &theLocation)
{
  DFLib::Proj::Report::setReceiverLocationMercator(theLocation);
  emit reportChanged();
}

void qDFProjReport::setBearing(double Bearing)
{
  DFLib::Proj::Report::setBearing(Bearing);
  emit reportChanged();
}

void qDFProjReport::setSigma(double Sigma)
{
  DFLib::Proj::Report::setSigma(Sigma);
  emit reportChanged();
}

void qDFProjReport::toggleValidity()
{
  DFLib::Proj::Report::toggleValidity();
  emit reportChanged();
}

string qDFProjReport::getReportSummary(const vector<string> &projArgs) const
{

  // Get a copy of the point
  DFLib::Proj::Point tempPoint=getReceiverPoint();
  tempPoint.setUserProj(projArgs); 

  // get the coordinates in our requested projection (which might not be the
  // one that the report uses

  vector <double> coords(2);
  coords=tempPoint.getUserCoords();

  // Our summary will be "name lon lat bearing sd validity"
  ostringstream os;
  os << getReportName() << " " << coords[0] << " " << coords[1] << " "
     << getBearing() << " " << getSigma();
  if (isValid())
    os << " Valid ";
  else 
    os << " Invalid ";

  return (os.str());
}

QDataStream &operator<<(QDataStream &out,const qDFProjReport &tR)
{
  // Get a copy of the point
  DFLib::Proj::Point tempPoint=tR.getReceiverPoint();
  vector <double> coords(2);
  coords=tempPoint.getUserCoords();
  out << QString::fromStdString(tR.getReportName());
  out << coords[0] << coords[1]
      << tR.getBearing() << tR.getSigma();
  out << tR.isValid()
      << QString::fromStdString(tR.theCS_.getBaseName())
      << quint32(tR.theCS_.getZone());
  return out;
}
