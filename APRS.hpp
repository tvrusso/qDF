#ifndef APRS_HPP
#define APRS_HPP

#include <QObject>
#include <QString>
#include <QMap>
#include <QStringList>

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


private:
  QString server_;
  quint16 port_;
  QString callsign_;
  QString callpass_;

  QMap<QString,QString> activeObjects;

private slots:
  void processPendingDatagrams();

};

#endif