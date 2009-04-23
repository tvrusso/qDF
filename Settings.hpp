#ifndef SETTINGS_HPP
#define SETTINGS_HPP
#include <QObject>
#include <QMap>
#include <QString>
#include <QList>

#include "CoordSysBuilder.hpp"

class Settings
{

private:
  CoordSysBuilder theCoordSysBuilder_;
  CoordSys defaultCS_;
  QMap<QString,QMap<QString,double> > equipQualSDMap_;
  double defaultDeclination_;
  int defaultUTMZone_;
  
public:
  Settings();
  Settings(const Settings &right);
  QList<QString> getEquipTypes() const {return (equipQualSDMap_.keys());};

  QList<QString> getQualityNames(const QString &equipType) const
  {return (equipQualSDMap_[equipType].keys());};

  double getStandardDeviation(const QString &equipType, 
                              const QString &quality) const;
  double getDefaultDeclination() const {return(defaultDeclination_);};
  int getDefaultUTMZone() const { return(defaultUTMZone_);};
  CoordSys getCoordSys(const QString &csName) const;
  QList<QString> getSupportedCoordSys() const;
  QString getDefaultCSName() const;
};

#endif
