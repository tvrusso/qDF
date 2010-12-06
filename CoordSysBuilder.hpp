#ifndef CoordSysBuilder_HPP
#define CoordSysBuilder_HPP

#include <string>
#include <vector>
#include <map>

#include <QString>
#include <QList>

class CoordSys
  {
  private:
    QString csName_;
    std::vector<std::string> proj4Params_;
    std::vector<std::string> coordinateNames_;
    bool zoneRequired_;
    int zone_;
  public:
    CoordSys();
    CoordSys(const CoordSys & right);
    CoordSys(QString name,std::vector<std::string> &p4P,
             std::vector<std::string> & cN, bool zR, int z=-1);
    CoordSys& operator=(const CoordSys& rhs);

    inline const std::vector<std::string> & getProj4Params() const
      {return (proj4Params_);};

    inline const std::vector<std::string> & getCoordinateNames() const
      {return (coordinateNames_);};

    inline int getZone() const {return (zone_);};

    void setZone(int z);
    QString getName() const;
    QString getBaseName() const;
    inline bool isZoneRequired() const { return(zoneRequired_);};
  };

  class CoordSysBuilder
  {
  private:
    std::map<std::string, CoordSys > CSMap_;

  public:
    CoordSysBuilder();
    CoordSys getCoordSys(const std::string & csName) const;
    CoordSys getCoordSys(const QString & csName) const;
    std::vector<std::string> getSupportedCoordSys() const;
    QList<QString> qGetSupportedCoordSys() const;
  };

#endif
