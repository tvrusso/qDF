#include <QDataStream>
#include <QVector>

#include "qDFProjReport.hpp"
#include <sstream>

qDFProjReport::qDFProjReport(const vector <double> &theLocationUser,
                             const double &bearing, const double &std_dev,
                             const string &theName, const CoordSys &aCS,
                             const QString & equipmentType,
                             const QString & quality
                             )
  :DFLib::Proj::Report(theLocationUser,bearing,std_dev,
                       theName, aCS.getProj4Params()),
   theCS_(aCS),
   equipmentType_(equipmentType),
   quality_(quality)
{
}

qDFProjReport::~qDFProjReport()
{
}

void qDFProjReport::setReceiverLocationUser(const vector<double> &theLocation)
{
  DFLib::Proj::Report::setReceiverLocationUser(theLocation);
  emit reportChanged(this);
}

void qDFProjReport::setReceiverLocationMercator(const vector<double> &theLocation)
{
  DFLib::Proj::Report::setReceiverLocationMercator(theLocation);
  emit reportChanged(this);
}

void qDFProjReport::setBearing(double Bearing)
{
  DFLib::Proj::Report::setBearing(Bearing);
  emit reportChanged(this);
}

void qDFProjReport::setSigma(double Sigma)
{
  DFLib::Proj::Report::setSigma(Sigma);
  emit reportChanged(this);
}

void qDFProjReport::setUserProj(const vector<string> &projArgs)
{
  DFLib::Proj::Report::setUserProj(projArgs);
  emit reportChanged(this);
}

void qDFProjReport::setEquipType(const QString & equipType)
{
  equipmentType_=equipType;
  // We don't emit reportChanged because this doesn't actually impact 
  // the fix --- the caller must *also* call setSigma.
}
void qDFProjReport::setQuality(const QString & quality)
{
  quality_=quality;
  // We don't emit reportChanged because this doesn't actually impact 
  // the fix --- the caller must *also* call setSigma.
}

void qDFProjReport::setCS(const CoordSys & cs)
{
  theCS_ = cs;
  setUserProj(cs.getProj4Params());  // this does the emit of reportChanged
}
  
const QString & qDFProjReport::getEquipType() const
{
  return equipmentType_;
}

const QString & qDFProjReport::getQuality() const
{
  return quality_;
}

const QString qDFProjReport::getCSName() const
{

  return theCS_.getName();
}


QVector<double> qDFProjReport::getReceiverLocationUser() const
{
  QVector<double> theLoc(2);
  vector<double> coords(2);
  DFLib::Proj::Point tempPoint=getReceiverPoint();
  coords = tempPoint.getUserCoords();
  theLoc[1]=coords[0];
  theLoc[0]=coords[1];
  return theLoc;
}
void qDFProjReport::toggleValidity()
{
  DFLib::Proj::Report::toggleValidity();
  emit reportChanged(this);
}

QString qDFProjReport::getReportNameQS() const
{
  QString name=QString::fromStdString(getReportName());
  return (name);
}

void qDFProjReport::setAll(const vector<double> &theLocationUser,
                           const double &bearing,
                           const double &std_dev,
                           const CoordSys &CS,
                           const QString &equipmentType, 
                           const QString &quality)
{
  setCS(CS);
  setEquipType(equipmentType);
  setQuality(quality);
  DFLib::Proj::Report::setReceiverLocationUser(theLocationUser);
  DFLib::Proj::Report::setBearing(bearing);
  DFLib::Proj::Report::setSigma(std_dev);
  emit reportChanged(this);  // this is redundant, since setCS did it, but let's
                       // be sure, just in case there's a queueing issue.
}
  
QString qDFProjReport::getReportSummary(const vector<string> &projArgs) const
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

  return (QString::fromStdString(os.str()));
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
      << tR.theCS_.getBaseName()
      << quint32(tR.theCS_.getZone());
  out << tR.getEquipType();
  out << tR.getQuality();
  return out;
}
