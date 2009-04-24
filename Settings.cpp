#include "Settings.hpp"
#include <iostream>
using namespace std;

Settings::Settings()
  :defaultDeclination_(9.3),
   defaultUTMZone_(13),
   defaultFCAMinAngle_(5.0)
{
  defaultCS_=theCoordSysBuilder_.getCoordSys(QString("WGS84 Lat/Lon"));

  equipQualSDMap_["Interferometer"]["Very Good"]=3.0;
  equipQualSDMap_["Interferometer"]["Good"]=6.0;
  equipQualSDMap_["Interferometer"]["OK"]=9.0;

  equipQualSDMap_["Long Base Avg"]["Very Good"]=5.0;
  equipQualSDMap_["Long Base Avg"]["Good"]=9.0;
  equipQualSDMap_["Long Base Avg"]["OK"]=13.0;

  equipQualSDMap_["ELPER"]["Very Good"]=6.0;
  equipQualSDMap_["ELPER"]["Good"]=12.0;
  equipQualSDMap_["ELPER"]["OK"]=18.0;

  equipQualSDMap_["Beam"]["Very Good"]=9.0;
  equipQualSDMap_["Beam"]["Good"]=18.0;
  equipQualSDMap_["Beam"]["OK"]=27.0;

}
Settings::Settings(const Settings &right)
  :theCoordSysBuilder_(right.theCoordSysBuilder_),
   defaultCS_(right.defaultCS_),
   equipQualSDMap_(right.equipQualSDMap_),
   defaultDeclination_(right.defaultDeclination_),
   defaultUTMZone_(right.defaultUTMZone_),
   defaultFCAMinAngle_(right.defaultFCAMinAngle_)
{
}


CoordSys Settings::getCoordSys(const QString &csName) const
{
  return (theCoordSysBuilder_.getCoordSys(csName));
}

QList<QString> Settings::getSupportedCoordSys() const
{
  return (theCoordSysBuilder_.qGetSupportedCoordSys());
}

double Settings::getStandardDeviation(const QString &equipType,
                                      const QString &quality) const
{
  return (equipQualSDMap_[equipType][quality]);
}

const CoordSys &Settings::getDefaultCS() const { return defaultCS_;};

QString Settings::getDefaultCSName() const
{
  QString theName=QString::fromStdString(defaultCS_.getBaseName());
  return (theName);
}


void Settings::setDefaultCS(const QString & csName)
{
  defaultCS_=theCoordSysBuilder_.getCoordSys(csName);
}

