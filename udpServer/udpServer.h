#ifndef UDPSERVER_H
#define UDPSERVER_H
#include <QMainWindow>
#include <ui_udpServer.h>

class QUdpSocket;

class udpServer : public QMainWindow, public Ui::udpServer
{
  Q_OBJECT;

 public:
  udpServer(QWidget *parent=0);
  void initSocket();

 private: 
  QUdpSocket *udpSocket;

 private slots:
  void processPendingDatagrams();

};


#endif
