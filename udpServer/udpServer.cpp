#include "udpServer.h"
#include<QDialog>
#include <QUdpSocket>
#include <QHostInfo>
#include <iostream>
using namespace std;
udpServer::udpServer(QWidget *parent)
  :QMainWindow(parent)
{

  setupUi(this);

  initSocket();

  connect(udpSocket, SIGNAL(readyRead()),
          this, SLOT(processPendingDatagrams()));

  plainTextEdit->clear();
}


void udpServer::initSocket()
{
  udpSocket= new QUdpSocket(this);
  udpSocket->bind(QHostAddress::LocalHost,2023);
}

void udpServer::processPendingDatagrams()
{

  QByteArray datagram;

  do 
  {
    QHostAddress sender;
    quint16 senderPort;
    datagram.resize(udpSocket->pendingDatagramSize());
    udpSocket->readDatagram(datagram.data(),datagram.size(),&sender,&senderPort);

    QDataStream in(&datagram,QIODevice::ReadOnly);
    in.setVersion(QDataStream::Qt_4_4);
    
    QString aprsPacket(datagram);

    plainTextEdit->insertPlainText(aprsPacket);
    
    QByteArray ackDatagram("ACK");

    udpSocket->writeDatagram(ackDatagram,sender,senderPort);

    plainTextEdit->insertPlainText(QString(ackDatagram));

  } while (udpSocket->hasPendingDatagrams());
}

    
