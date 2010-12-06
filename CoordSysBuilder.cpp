#include "CoordSysBuilder.hpp"
#include <Util_Misc.hpp>
#include <sstream>

  CoordSysBuilder::CoordSysBuilder()
  {
    std::vector<std::string> temp;
    std::vector<std::string> coordNames;

    temp.clear();
    temp.resize(2);
    coordNames.resize(2);
    coordNames[0] = "Longitude";
    coordNames[1] = "Latitude";
    temp[0] = "proj=latlong";
    temp[1] = "datum=WGS84";
    CSMap_["WGS84 Lat/Lon"] = CoordSys("WGS84 Lat/Lon",temp,coordNames,false);

    temp.clear();
    temp.resize(2);
    temp[0] = "proj=latlong";
    temp[1] = "datum=NAD27";
    coordNames[0] = "Longitude";
    coordNames[1] = "Latitude";

    CSMap_["NAD27 Lat/Lon"] =  CoordSys("NAD27 Lat/Lon",temp,coordNames,false);

    // comment this out for now, we're not ready to support it upstream
#if 0
    temp.clear();
    temp.resize(2);
    coordNames.resize(2);
    coordNames[0] = "Easting";
    coordNames[1] = "Northing";
    temp[0] = "proj=utm";
    temp[1] = "datum=WGS84";
    CSMap_["WGS84 UTM"] = CoordSys("WGS 84 UTM",temp,coordNames,true);
#endif

  }

  CoordSys CoordSysBuilder::getCoordSys(const std::string & csname) const
  {
    std::map<std::string,CoordSys>::const_iterator foo;
    std::map<std::string,CoordSys>::const_iterator theEnd=CSMap_.end();

    foo=CSMap_.find(csname);
    if (foo == theEnd)
      throw(DFLib::Util::Exception("Coordinate system not found in map"));

    return((*foo).second);
  }

  CoordSys CoordSysBuilder::getCoordSys(const QString & csname)  const
  {
    return(getCoordSys(csname.toStdString()));
  }

  std::vector<std::string> CoordSysBuilder::getSupportedCoordSys() const
  {
    
    std::vector<std::string> foo;
    std::map<std::string,CoordSys >::const_iterator start=CSMap_.begin();
    std::map<std::string,CoordSys >::const_iterator finish=CSMap_.end();
    std::map<std::string,CoordSys >::const_iterator current;

    for ( current=start; current != finish; current++)
    {
      foo.push_back(current->first);
    }
    return (foo);
    
  }

  QList<QString> CoordSysBuilder::qGetSupportedCoordSys() const
  {
    QList<QString> theList;
    std::map<std::string,CoordSys >::const_iterator start=CSMap_.begin();
    std::map<std::string,CoordSys >::const_iterator finish=CSMap_.end();
    std::map<std::string,CoordSys >::const_iterator current;

    for ( current=start; current != finish; current++)
    {
      theList.append(QString::fromStdString((current->first)));
    }
    return (theList);
  }

  CoordSys::CoordSys()
    : csName_("unknown"),
      zone_(-1)
  {
    proj4Params_.clear();
    coordinateNames_.clear();
  }

  CoordSys::CoordSys(QString n, std::vector<std::string> & p4P,
                     std::vector<std::string> & cN, bool zR, int z)
    :csName_(n),
     proj4Params_(p4P),
     coordinateNames_(cN),
     zoneRequired_(zR),
     zone_(z)
  {
  }
  // Copy constructor
  CoordSys::CoordSys(const CoordSys & right)
    : csName_(right.csName_),
      proj4Params_(right.proj4Params_),
      coordinateNames_(right.coordinateNames_),
      zoneRequired_(right.zoneRequired_),
      zone_(right.zone_)
  {
  }

  // assignment operator:
  CoordSys& CoordSys::operator=(const CoordSys & rhs)
  {

    // if the same object (same address) is on both sides of the assignment,
    // do nothing.

    if (this == &rhs)
      return *this;

    csName_=rhs.csName_;
    proj4Params_=rhs.proj4Params_;
    coordinateNames_=rhs.coordinateNames_;
    zoneRequired_=rhs.zoneRequired_;
    zone_=rhs.zone_;

    return *this;
  }

  QString CoordSys::getBaseName() const
  {
    return (csName_);
  }

  QString CoordSys::getName() const
  {
    ostringstream returnName;
    returnName << csName_.toStdString();

    if (zoneRequired_)
    {
      if (zone_ == -1)
      {
        returnName << " Zone Not Set";
      }
      else
      {
        returnName << " Zone " << zone_;
      }
    }
    return (QString::fromStdString(returnName.str()));
  }

  void CoordSys::setZone(int z)
  {
    ostringstream zoneString;
    zoneString << "zone = " << z;

    if (zone_ == -1)  // never been set
    {
      proj4Params_.push_back(zoneString.str());
    }
    else // we already have one, find it and replace
    {
      for (int i=0;i<proj4Params_.size(); i++)
      {
        size_t found=proj4Params_[i].find("zone=");
        if (found != std::string::npos)
        {
          proj4Params_[i] = zoneString.str();
        }
      }
    }
    zone_=z;
  }

