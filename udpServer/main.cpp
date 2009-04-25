#include <QApplication>
#include <QDialog>

#include "udpServer.h"

int main(int argc, char ** argv)
{
	QApplication app(argc,argv);

	udpServer *dialog= new udpServer;
	dialog->show();

	return app.exec();
}

