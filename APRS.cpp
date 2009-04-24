#include "APRS.hpp"
#include <QDateTime>
#include <cmath>
#include <QUdpSocket>
#include <QHostInfo>
#include <QHostAddress>
#include <iostream>
using namespace std;

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
  lonS.append(EW);

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

  sendPacketToServer(formattedPosit);
  return formattedPosit;
}

QString APRS::deleteObject(const QString &oName)
{
  QMap<QString, QString>::iterator i = activeObjects.find(oName);
  QString retval="";
  if (i != activeObjects.end())
  {
    retval=i.value();
    if (!retval.isEmpty())
    {
      retval.replace(retval.indexOf('*'),1,'_');
      activeObjects[oName]="";
      sendPacketToServer(retval);
    }
  }
  return retval;
}

QStringList APRS::deleteAllObjects()
{
  QStringList returnList;
  QMap<QString, QString>::iterator i;

  for (i=activeObjects.begin(); i!=activeObjects.end(); i++)
  {
    QString retString=deleteObject(i.key());
    if (!retString.isEmpty()) returnList.append(retString);
  }
  return returnList;
}

void APRS::sendPacketToServer(const QString &payload)
{

  if (!payload.isEmpty())
  {

    // Create a new udpSocket for this one packet (?)
    QUdpSocket *theSocket  = new QUdpSocket(this);

    // Now send our payload in the proper format.  Xastir is expecting:
    // CALL,CALLPASS\nPAYLOAD\n

    QString sendText=callsign_;
    sendText.append(",");
    sendText.append(callpass_);
    sendText.append("\n");
    sendText.append(payload);
    sendText.append("\n");

    QByteArray datagram=sendText.toAscii();

    QHostInfo info=QHostInfo::fromName(server_);
    if (!info.addresses().isEmpty())
    {
      theSocket->writeDatagram(datagram,info.addresses().first(),port_);
      
      // now set this socket up to listen there...
      theSocket->bind(QHostAddress::LocalHost,port_);
      // and set us up to handle anything the server sends back:
      connect(theSocket,SIGNAL(readyRead()),this,SLOT(processPendingDatagrams()));
    }
    else
    {
      cout << " sendPacketToServer error: host name could not be resolved." << endl;
    }
  }
}

void APRS::processPendingDatagrams()
{
  // The server will send an ACK or NACK back.  Get it, then close the socket.
  // get the socket that generated the signal...
  // This means that this method should ONLY ever be used as a slot connected
  // to a readReady() signal of a QUdpSocket!  DANGER!

  QUdpSocket *theSocket = dynamic_cast<QUdpSocket *>(QObject::sender());


  QByteArray datagram;

  do
  {
    QHostAddress sender;
    quint16 senderPort;
    datagram.resize(theSocket->pendingDatagramSize());
    theSocket->readDatagram(datagram.data(),datagram.size(),&sender,&senderPort);
    
    QDataStream in(&datagram,QIODevice::ReadOnly);
    in.setVersion(QDataStream::Qt_4_4);
    
    QString aprsPacket(datagram);
    

  } while (theSocket->hasPendingDatagrams());

  // we're now done with this socket, delete it
  delete theSocket;
}

