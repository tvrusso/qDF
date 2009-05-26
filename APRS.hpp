#ifndef APRS_HPP
#define APRS_HPP

#include <QObject>
#include <QString>
#include <QQueue>
#include <QMap>
#include <QStringList>
#include <QUdpSocket>

#include <vector>
using namespace std;

class APRS:public QObject
{
  Q_OBJECT;

public:
  APRS();
  APRS(const QString &server,const quint16 port, const QString &callsign, 
       const QString &callpass);
  QString createObject(const QString &oName,const vector<double> &coords,
                       const QString &symbol,const QString &comment);
  QString createDFObject(const QString &oName, const vector<double> &coords,
                         double bearing, double sigma, const QString &comment);
  QString createDFErrorObject(const QString &oName,
                              const vector<double> &coords,
                              double axisLon,double axisLat);
  QString createMultilineObject(const QString &oName,
                                const vector<double> &lats, 
                                const vector<double> &lons,
                                const vector<double> &coords,
                                char colorStyle, int lineType,
                                const QString &oSym);
  QString deleteObject(const QString &oName);
  QStringList deleteAllObjects();

  void sendPacketToServer(const QString &payload);

  void setServer(const QString &server);
  void setCallsign(const QString &callsign);
  void setCallpass(const QString &callpass);
  void setPort(const quint16 port);

  const QString &getServer() const;
  const QString &getCallsign() const;
  const QString &getCallpass() const;
  const quint16 getPort() const;

  QString makeMultiline(const vector<double> &lon,const vector<double>&lat,
                        char colorStyle, int lineType, const QString &sequence,
                        double *lonCentr, double *latCentr);
  void generateEllipse(double lonCentr, double latCentr, 
                       double axisLon, double axisLat,
                       int numPoints,
                       vector<double> &lons, vector<double> &lats);
  


private:
  QMap<QString,QString> activeObjects;
  QString server_;
  quint16 port_;
  QString callsign_;
  QString callpass_;
  //  QList<QPair<QUdpSocket *,int> > udpSockets;
  QQueue<QPair<QString,int> > pendingPackets;
  bool timerActive;
  QUdpSocket *theSocket;

private slots:
  void checkPendingDatagrams();

signals:
  void queueCleared();

};

#endif
