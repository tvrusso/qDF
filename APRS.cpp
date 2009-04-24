#include "APRS.hpp"
#include <QDateTime>
#include <cmath>

APRS::APRS()
  :server_(""),
   port_(0),
   callsign_(""),
   callpass_("")
{
}

APRS::APRS(const QString &server,quint16 port, const QString &callsign,const QString &callpass)
  :server_(server),
   port_(port),
   callsign_(callsign),
   callpass_(callpass)
{
}

void APRS::setServer(const QString &server)
{
  server_=server;
}

void APRS::setCallsign(const QString &callsign)
{
  callsign_=callsign;
}

void APRS::setCallpass(const QString &callpass)
{
  callpass_=callpass;
}

void APRS::setPort(const quint16 port)
{
  port_=port;
}

const QString & APRS::getServer() const
{
  return server_;
}

const QString & APRS::getCallsign() const
{
  return callsign_;
}

const QString & APRS::getCallpass() const
{
  return callpass_;
}

const quint16 APRS::getPort() const
{
  return port_;
}

QString APRS::createDFObject(const QString &oName,const vector<double> &coords,
                           double bearing,double sigma, const QString &comment)
{
  QString BRG;
  QString NRQ;
  QString rep;

  BRG.sprintf("%03.0lf/",bearing);

  int Q=9-int(log(sigma)/log(2.0));
  NRQ.sprintf("86%1d ",Q);
  
  rep="000/000/";
  rep.append(BRG);
  rep.append(NRQ);
  rep.append(comment.left(26));
  return createObject(oName,coords,"/\\",rep);
}

QString APRS::createObject(const QString &oName,const vector<double> &coords,
                     const QString &symbol, const QString &comment)
{
  QString formattedPosit;

  // APRS objects always have 9 character names, pad with spaces.
  QString objectName=oName.leftJustified(9,' ');

  // we are given coordinates in decimal degrees.  Convert to APRS format.

  QString NS="N";
  QString EW="E";
  QString latS;
  QString lonS;

  double lat=coords[1];
  double lon=coords[0];

  if (lon<0)
  {
    lon*=-1;
    EW="W";
  }

  if (lat<0)
  {
    lat*=-1;
    NS="S";
  }
  
  int deg;
  double min;

  deg=(int) lat;
  min=(lat-deg)*60;
  
  latS.sprintf("%02d%05.2lf",deg,min);
  latS.append(NS);


  deg=(int) lon;
  min=(lon-deg)*60;
  
  lonS.sprintf("%03d%05.2lf",deg,min);
  lonS.append(NS);

  formattedPosit=callsign_;
  formattedPosit.append(">APZTVR:;");
  formattedPosit.append(objectName);
  formattedPosit.append("*");

  QDateTime theCurrentDateTime=QDateTime::currentDateTime().toUTC();
  formattedPosit.append(theCurrentDateTime.toString("ddhhmm"));
  formattedPosit.append("z");
  formattedPosit.append(latS);
  formattedPosit.append(symbol[0]);
  formattedPosit.append(lonS);
  formattedPosit.append(symbol[1]);
  formattedPosit.append(comment.left(43));

  activeObjects[oName]=formattedPosit;
  return formattedPosit;
}

QString APRS::deleteObject(const QString &oName)
{
  QMap<QString, QString>::iterator i = activeObjects.find(oName);
  QString retval="";
  if (i != activeObjects.end())
  {
    retval=i.value();
    retval.replace(retval.indexOf('*'),1,'_');
    activeObjects[oName]=retval;
  }
  return retval;
}
