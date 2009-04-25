#include <QApplication>
#include <QDialog>

#include "udpClient.h"

int main(int argc, char ** argv)
{
	QApplication app(argc,argv);
        
	udpClient *dialog= new udpClient;
	dialog->show();

	return app.exec();
}

