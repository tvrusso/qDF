#include "udpClient.h"
#include<QDialog>
#include <QUdpSocket>
#include <QHostInfo>
#include <iostream>
using namespace std;
udpClient::udpClient(QWidget *parent)
  :QMainWindow(parent),
   serverName_("localhost")
{

  setupUi(this);

  initSocket();

  connect (pushButton,SIGNAL(clicked()),
           this, SLOT(sendDatagram()));

  outputBox->clear();
}


void udpClient::initSocket()
{
  udpSocket= new QUdpSocket(this);
}

void udpClient::sendDatagram()
{
  QByteArray theDatagram(lineEdit->text().toAscii());
  QHostInfo info=QHostInfo::fromName(serverName_);
  QHostAddress sender;
  quint16 senderPort;
  int bytesRead;
  udpSocket->writeDatagram(theDatagram,info.addresses().first(),2023);
  outputBox->insertPlainText("Output a datagram:");
  outputBox->insertPlainText("\n");
  outputBox->insertPlainText("Waiting for ACK or NACK...");
  theDatagram.clear();
  theDatagram.resize(256);
  while ((bytesRead= udpSocket->readDatagram(theDatagram.data(),
                                             theDatagram.size(),&sender,
                                             &senderPort)) == -1)
  {}
  outputBox->insertPlainText("done.\n");
  QString aprsPacket(theDatagram);
  cout << "udpClient: received a packet of "<< bytesRead << " " << aprsPacket.toStdString() << endl;;

}

#if 0
void udpClient::close()
{

}
#endif
