#include "APRS.hpp"
#include <QDateTime>
#include <cmath>
#include <QUdpSocket>
#include <QHostInfo>
#include <QHostAddress>
#include <QTimer>
#include <QMessageBox>

#include <iostream>
using namespace std;

APRS::APRS()
  :server_(""),
   port_(0),
   callsign_(""),
   callpass_(""),
   timerActive(false)
{
}

APRS::APRS(const QString &server,quint16 port, const QString &callsign,const QString &callpass)
 :server_(server),
   port_(port),
   callsign_(callsign),
  callpass_(callpass),
  timerActive(false)
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
    QUdpSocket *theSocket  = new QUdpSocket;

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
      udpSockets.append(QPair<QUdpSocket *,int>(theSocket,0));
#if 0
      // now wait for the ack...
      QHostAddress sender;
      quint16 senderPort;
      int bytesRead;

      datagram.clear();
      datagram.resize(256);
      while ((bytesRead= theSocket->readDatagram(datagram.data(),
                                                 datagram.size(),&sender,
                                                 &senderPort)) == -1)
      {}
      QString nackorack(datagram);
      cout << "udpClient: received a packet of "<< bytesRead << " " 
           << nackorack.toStdString() << endl;;
#endif
      if (!timerActive)
      {
        QTimer::singleShot(1000,this,SLOT(checkPendingDatagrams()));
        timerActive=true;
      }
    }
    else
    {
      QMessageBox::warning(0,tr("APRS"),
                           QString("Host name %1 could not be resolved.").arg(server_),
                           QMessageBox::Ok);
    }
  }
}


// makeMultiline
// Create an APRS multiline string given an array of lat/lon pairs.
//
// Allocates memory that must be freed by the caller.
//
// lon and lat are arrays.  lonObj, latObj are return values of object
// location (point from which offsets are computed).  
//
// lineType is 0 for a closed polygon, 1 for a polyline
//
// colorStyle is a character as defined in the wxsvr.net multiline protocol
// web site at wxsvr.net.  
//
// character | color | style
//   a          red     solid
//   b          red     dashed
//   c          red     double-dashed
//   d          yellow  solid
//   e          yellow  dashed
//   f          yellow  double-dashed
//   g          blue    solid
//   h          blue    dashed
//   i          blue    double-dashed
//   j          green   solid
//   k          green   dashed
//   l          green   double-dashed

// Returns a null pointer if user requested too many vertices, or if scale
// is out of range, or if we fail to malloc the string.
//
// One could pass only a list of lat/lons here and get back a point at which
// to create an object (at the centroid) and a string representing the 
// multiline.
#define minFun(a,b) ( ((a)<(b))?(a):(b))
#define maxFun(a,b) ( ((a)>(b))?(a):(b))

QString APRS::makeMultiline(const vector<double> &lon, const vector<double> &lat, 
                      char colorStyle, int lineType, const QString &sequence,
                      double *lonCentr, double *latCentr  )
{
    
    QString returnString;
    int numPairs=lon.size();
    // the APRS spec requires a max of 43 chars in the comment section of 
    // objects, which leaves room for only so many vertices in a multiline 
    //   number allowed= (43-(6-5))/2=16
    // 43chars - 6 for the sequence number- 5 for the starting pattern leaves
    // 32 characters for lat/lon pairs, or 16 pairs

    if ( numPairs > 16) {
        returnString = "";
    } else {
        double minLat, minLon;
        double maxLat, maxLon;
        int iPair;
        double scale1,scale2,scale;
        
        // find min/max of arrays
        minLat=minLon=180;
        maxLat=maxLon=-180;
        
        for ( iPair=0; iPair < numPairs; iPair++) {
            minLon = minFun(minLon,lon[iPair]);
            minLat = minFun(minLat,lat[iPair]);
            maxLon = maxFun(maxLon,lon[iPair]);
            maxLat = maxFun(maxLat,lat[iPair]);
        }

        *lonCentr = (maxLon+minLon)/2;
        *latCentr = (maxLat+minLat)/2;
        
        // Compute scale:
        // The scale is the value that makes the maximum or minimum offset
        // map to +44 or -45.  Pick the scale factor that keeps the
        // offsets in that range:
        
        if (maxLat > maxLon) {
            scale1= (maxLat-*latCentr)/44.0;
        } else {
            scale1= (maxLon-*lonCentr)/44.0;
        }

        if (minLat < minLon) {
            scale2 = (minLat-*latCentr)/(-45.0);
        } else {
            scale2 = (minLon-*lonCentr)/(-45.0);
        }

        scale = maxFun(scale1,scale2);
        
        if (scale < .0001) {
            scale=0.0001;
        }

        if (scale > 1) {
            // Out of range, no shape returned
            returnString = "";
        } else {
            // Not all systems have a log10(), but they all have log() 
            // So let's stick with natural logs
            double ln10=log(10.0);
            // KLUDGE:  the multiline spec says we use 
            // 20*(int)(log10(scale/.0001)) to generate the scale char,
            // but this means we'll often produce real scales that are smaller
            // than the one we just calculated, which means we'd produce
            // offsets outside the (-45,44) allowed range.  So kludge and 
            // add 1 to the value
            int lnscalefac=20*log(scale/.0001)/ln10+1;

            // Now recompute the scale to be the one we actually transmitted
            // This pretty much means we'll never have the best precision
            // we could possibly have, but it'll be close enough
            scale=pow(10,(double)lnscalefac/20-4);

            // We're ready to produce the multiline string.  So get on with it
            
            // multiline string is "}CTS" (literal "}" followed by
            // line Color-style specifier, followed by open/closed
            // Type specifier, followed by Scale character), followed
            // by even number of character pairs, followed by "{seqnc"
            // (sequence number).


            returnString=" }";
            returnString.append(QChar(colorStyle));
            returnString.append(QChar((lineType == 0)?'0':'1'));
            
            returnString.append(QChar(lnscalefac+33));
            
            for ( iPair=0; iPair<numPairs; ++iPair) {
              double latOffset=lat[iPair]-*latCentr;
              // the wxsvr protocol is Western Hemisphere-Centric,
              // and treats positive offsets in longitude as being
              // west of the reference point.  So have to reverse
              // the sense of direction here.
              // This will yield positive offsets if lonCenter is
              // negative (west) and lon[iPair] is more negative 
              // (more west)
              double lonOffset=*lonCentr-lon[iPair];
              
              returnString.append( QChar(((int)(latOffset/scale)+78)));
              returnString.append(QChar(((int)(lonOffset/scale)+78)));
            }
            returnString.append(QChar('{'));
            returnString.append(sequence.left(6));
        }
    }
    return (returnString);
}

// For generating error ellipses.
void APRS::generateEllipse(double lonCentr, double latCentr, 
                     double axisLon, double axisLat,
                     int numPoints,
                     vector<double> &lons, vector<double> &lats)
{
  int iPair;
  double pi=4*atan(1.0);
  lons.resize(numPoints);
  lats.resize(numPoints);
  for (iPair = 0; iPair < numPoints; ++iPair)
  {
    lons[iPair] = lonCentr + axisLon*cos(2.0*pi/((double)numPoints)*iPair);
    lats[iPair] = latCentr + axisLat*sin(2.0*pi/((double)numPoints)*iPair);
  }
}

QString APRS::createDFErrorObject(const QString &oName,
                                  const vector<double> &coords,
                                  double axisLon,double axisLat)
{
  vector<double> lats;
  vector<double> lons;
  double latC=coords[1];
  double lonC=coords[0];

  generateEllipse(coords[0],coords[1],axisLon,axisLat,16,lons,lats);
  QString theMultiline=makeMultiline(lons,lats,'e',1,"error",
                                     &lonC,&latC);
  if (theMultiline.isEmpty())
    return (QString(""));

  return(createObject(oName,coords,"\\l",theMultiline));
}

void APRS::checkPendingDatagrams()
{
  QUdpSocket *theSocket;
  QList<QPair<QUdpSocket *,int> >::iterator iter;
  QList<QPair<QUdpSocket *,int> >::iterator listEnd=udpSockets.end();
  QVector<QList<QPair<QUdpSocket *,int> >::iterator> itersToDelete;
  itersToDelete.clear();
  int socketnum=0;

  for (iter=udpSockets.begin();iter!=listEnd;++iter)
  {
    theSocket=(*iter).first;
    int numTries=(*iter).second;
    int bytesRead;
    QByteArray datagram;
    QHostAddress sender;
    quint16 senderPort;
    datagram.resize(256);
    
    if ((bytesRead=theSocket->readDatagram(datagram.data(),
                                          datagram.size(),&sender,
                                           &senderPort))!= -1)
    {

      // don't care what it was --- we know it was an ack      
      theSocket->deleteLater();  // we don't need this anymore, tell it to go away.
      itersToDelete.append(iter);
    }
    else
    {
      if (numTries<10)
      {
        ((*iter).second)++;
      }
      else
      {
        // it ain't ack'd after 10 seconds, give up on it.
        theSocket->deleteLater();
        itersToDelete.append(iter);
      }
    }
    socketnum++;
  }
  // Delete any iterators we flagged
  if (itersToDelete.size()!= 0)
  {
    for (int i=0; i<itersToDelete.size(); ++i)
      udpSockets.erase(itersToDelete[i]);
  }

  // If we still haven't emptied the udpSockets list, then some socket still
  // hasn't acked.  Reset the timer.
  if (udpSockets.size() != 0)
  {
    QTimer::singleShot(1000,this,SLOT(checkPendingDatagrams()));
    timerActive=true;
  }
  else
  {
    timerActive=false;
  }
}

