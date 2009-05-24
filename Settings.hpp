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
  double defaultFCAMinAngle_;

  QString aprsServer_;
  QString aprsCallsign_;
  QString aprsCallpass_;
  quint16 aprsPort_;
  bool publishAPRS_;

public:
  Settings();
  Settings(const Settings &right);
  QList<QString> getEquipTypes() const {return (equipQualSDMap_.keys());};

  QList<QString> getQualityNames(const QString &equipType) const
  {return (equipQualSDMap_[equipType].keys());};

  double getStandardDeviation(const QString &equipType, 
                              const QString &quality) const;
  double getDefaultDeclination() const {return(defaultDeclination_);};
  double getDefaultFCAMinAngle() const {return(defaultFCAMinAngle_);};
  int getDefaultUTMZone() const { return(defaultUTMZone_);};
  int getAPRSPort() const{return(aprsPort_);};
  QString getAPRSServer() const {return(aprsServer_);};
  QString getAPRSCallsign() const {return(aprsCallsign_);};
  QString getAPRSCallpass() const {return(aprsCallpass_);};

  CoordSys getCoordSys(const QString &csName) const;
  QList<QString> getSupportedCoordSys() const;
  QString getDefaultCSName() const;
  const CoordSys &getDefaultCS() const;

  inline const QMap<QString,QMap<QString,double> > &getEQMap()
  {return(equipQualSDMap_);};

  void setDefaultCS(const QString & csName);
  void setEquipMap(const QMap<QString,QMap<QString,double> > &eqM);

  inline void setDefaultDeclination(double defDec) 
  {defaultDeclination_=defDec;};

  inline void setDefaultFCAMinAngle(double defFCAMA) 
  {defaultFCAMinAngle_=defFCAMA;};

  inline void setDefaultUTMZone(int defUTMZ)
  {defaultUTMZone_=defUTMZ;};

  inline void setAPRSCallsign(const QString &c) {aprsCallsign_=c;};
  inline void setAPRSCallpass(const QString &c) {aprsCallpass_=c;};
  inline void setAPRSServer(const QString &c) {aprsServer_=c;};
  inline void setAPRSPort(int p) {aprsPort_=p;};

  inline bool publishAPRS() const {return publishAPRS_;};
  inline void setPublishAPRS(bool p) {publishAPRS_=p;};

};

#endif
