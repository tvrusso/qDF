#ifndef UDPCLIENT_H
#define UDPCLIENT_H

#include <QMainWindow>
#include <ui_udpClient.h>

class QUdpSocket;

class udpClient : public QMainWindow, public Ui::udpClient
{
  Q_OBJECT;

 public:
  udpClient(QWidget *parent=0);
  void initSocket();

 private: 
  QUdpSocket *udpSocket;
  QString serverName_;

 private slots:
  void sendDatagram();
  //  void close();

};


#endif

