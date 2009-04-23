#include <QtGui>
#include <sstream>
#include <iostream>
using namespace std;

#include "MainWindow.h"
#include "Settings.hpp"
#include "getReportDialog.h"
#include "reportToggleDialog.h"
#include "qDFProjReport.hpp"
#include <Util_Misc.hpp>

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
{
  setupUi(this);
}

MainWindow::~MainWindow()
{
  theReportCollection.deleteReports();
}

void MainWindow::setupConnections()
{
  connect(actionNew_Report, SIGNAL(triggered()), this, SLOT(newReportClicked()));
  connect(actionToggle_Validity, SIGNAL(triggered()), this, SLOT(toggleValidityClicked()));

  connect(this, SIGNAL(newReportCreated(qDFProjReport *)), 
          &theReportCollection, SLOT(newReport(qDFProjReport *)));

  connect(&theReportCollection,SIGNAL(collectionChanged()),
          this,SLOT(reportCollectionChanged()));

  connect(&theReportCollection,SIGNAL(collectionChanged(int)),
          this,SLOT(updateCollectionDisplay(int)));

  connect(reportListWidget,SIGNAL(itemDoubleClicked(QListWidgetItem *)),
          this,SLOT(listItemDoubleClicked(QListWidgetItem *)));
}

void MainWindow::toggleValidityClicked()
{
  reportToggleDialog theToggleDialog;

  if (theToggleDialog.exec())
  {
    string reportName = theToggleDialog.lineEdit_ReportName->text().toStdString();
    int reportIndex = theReportCollection.getReportIndex(reportName);

    if (reportIndex != -1)
    {
      theReportCollection.toggleValidity(reportIndex);
      //      cout << "Toggling validity of report number " << reportIndex << " named " << reportName << endl;
    }
  }
}

void MainWindow::newReportClicked()
{
  getReportDialog theRD(theSettings_);

  if (theRD.exec())
  {
    string reportName= theRD.lineEdit_ReportName->text().toStdString();
    QString coordSysName=theRD.comboBox_CoordSys->currentText();
    string coord0=theRD.lineEdit_Coord0->text().toStdString();
    string coord1=theRD.lineEdit_Coord1->text().toStdString();
    double bearing=theRD.lineEdit_bearing->text().toDouble()+9.8;
    double sd=
      theSettings_.getStandardDeviation(theRD.comboBox_EquipmentType->currentText(),
                                        theRD.comboBox_Quality->currentText());

    // this also not right yet, might not be in degrees if UTM supported
    vector<double> coords(2);
    coords[0]=dmstor(coord0.c_str(),NULL)*RAD_TO_DEG;
    coords[1]=dmstor(coord1.c_str(),NULL)*RAD_TO_DEG;


    // This is not right yet..., need to check for zone.  Don't worry now.
    //    CoordSysBuilder myCSB;
    CoordSys myCS=theSettings_.getCoordSys(coordSysName);

    qDFProjReport *theNewReport = new qDFProjReport(coords,bearing,sd,
                                                    reportName,myCS.getProj4Params());
    emit newReportCreated(theNewReport);
  }
  else
  {
    //    cout << "You clicked Cancel" << endl;
  }

}

void MainWindow::newReportReceived(qDFProjReport *report)
{
  //  cout << " received a new report, its pointer is " << report << endl;
  //  delete report;
}

 void MainWindow::reportCollectionChanged()
 {
   int nreports= theReportCollection.numValidReports() ;
   //   cout << " in slot reportCollectionChanged." << endl;
   //   cout << " collection now has " << nreports
   //        << " valid reports out of a total of " << theReportCollection.size()
   //        << " reports. " <<endl;

   lcdNumberNReports->setSegmentStyle(QLCDNumber::Filled);
   lcdNumberNReports->display(nreports);
   
   if (nreports>=2)
   {
     CoordSys myCS=theSettings_.getCoordSys("WGS84 Lat/Lon");
     
     vector<double> foo(2,0.0);
     vector<double> FCA_stddev(2);
     vector<string> formattedCoords;

     DFLib::Proj::Point FCA(foo,myCS.getProj4Params());
     theReportCollection.computeFixCutAverage(FCA,FCA_stddev);

     
     if (nreports>2)
     {
       DFLib::Proj::Point LSFix=FCA;
       theReportCollection.computeLeastSquaresFix(LSFix);

       DFLib::Proj::Point MLFix=LSFix;
       theReportCollection.computeMLFix(MLFix);

       vector<double> LS_point=LSFix.getUserCoords();
       //       printCoords(LS_point,string("Least Squares Fix"));
       formatCoords(LS_point,formattedCoords);
       label_LS_Longitude->setText(QString::fromStdString(formattedCoords[0]));
       label_LS_Latitude->setText(QString::fromStdString(formattedCoords[1]));
       
       vector<double> ML_point=MLFix.getUserCoords();
       //       printCoords(ML_point,string("Maximum Likelihood Fix"));
       formatCoords(ML_point,formattedCoords);
       label_ML_Longitude->setText(QString::fromStdString(formattedCoords[0]));
       label_ML_Latitude->setText(QString::fromStdString(formattedCoords[1]));
     }
     else
     {
       label_ML_Longitude->setText("Not Available");
       label_ML_Latitude->setText("Not Available");
       label_LS_Longitude->setText("Not Available");
       label_LS_Latitude->setText("Not Available");
     }       

     vector<double> FCA_point=FCA.getUserCoords();
     //     printCoords(FCA_point,string("Fix Cut Average"));
     formatCoords(FCA_point,formattedCoords);
     label_FCA_Longitude->setText(QString::fromStdString(formattedCoords[0]));
     label_FCA_Latitude->setText(QString::fromStdString(formattedCoords[1]));
     //     cout << " Standard deviation of FCA is " << FCA_stddev[0] << " longitude"
     //          << " and " << FCA_stddev[1] << " latitude." << endl;

    
     ostringstream os;
     os << FCA_stddev[0];
     label_FCA_Longitude_SD->setText(QString::fromStdString(os.str()));
     os.str("");
     os << FCA_stddev[1];
     label_FCA_Latitude_SD->setText(QString::fromStdString(os.str()));


   }
   else
   {
     label_FCA_Longitude->setText("Not Available");
     label_FCA_Latitude->setText("Not Available");
     label_FCA_Longitude_SD->setText("Not Available");
     label_FCA_Latitude_SD->setText("Not Available");
     label_ML_Longitude->setText("Not Available");
     label_ML_Latitude->setText("Not Available");
     label_LS_Longitude->setText("Not Available");
     label_LS_Latitude->setText("Not Available");
   }       
     
     
 }

void MainWindow::printCoords(const vector<double> &latlon,const string &text)
{

  vector<string>formattedCoords;

  formatCoords(latlon,formattedCoords);

  cout << " Longitude of " << text << ": " << formattedCoords[0]
       << endl;

  cout << " Latitude of " << text << ": " << formattedCoords[1]
       << endl;

}

void MainWindow::formatCoords(const vector<double> &latlon,
                              vector<string> &formattedCoords)
{
  int latfac=1;  int lonfac=1;
  char EW,NS;

  formattedCoords.resize(2);

  EW='E';
  NS='N';

  if (latlon[0] < 0)
  {
    lonfac=-1;
    EW='W';
  }

  if (latlon[1] < 0)
  {
    latfac=-1;
    NS='S';
  }

  ostringstream os;
  os << static_cast<int>(latlon[0]*lonfac)
     << "d" 
     << (latlon[0]*lonfac-static_cast<int>(latlon[0]*lonfac))*60 
     << "'" << EW ;
  formattedCoords[0]=os.str();
  os.str("");

  os << static_cast<int>(latlon[1]*latfac)
     << "d" 
     << (latlon[1]*latfac-static_cast<int>(latlon[1]*latfac))*60 
     << "'" << NS ;
  formattedCoords[1]=os.str();

}

void MainWindow::updateCollectionDisplay(int reportIndex)
{

  CoordSys myCS=theSettings_.getCoordSys("WGS84 Lat/Lon");
  vector<string> theProj4Params=myCS.getProj4Params();
  QString theReportSummary=QString::fromStdString(theReportCollection.getReportSummary(reportIndex,theProj4Params) );
  cout << theReportSummary.toStdString()
         << endl;
  QListWidgetItem *theWidgetItem=reportListWidget->item(reportIndex);
  if(theWidgetItem)
    theWidgetItem->setText(theReportSummary);
  else
    reportListWidget->insertItem(reportIndex,theReportSummary);
}

void MainWindow::listItemDoubleClicked(QListWidgetItem *item)
{
  int theRow=reportListWidget->row(item);
  cout << " You double clicked row " << theRow<<endl;
  QFont theItemFont=item->font();
  if(theItemFont.italic())
    theItemFont.setItalic(false);
  else
    theItemFont.setItalic(true);
  item->setFont(theItemFont);
  // we've assured that row and report index are the same.
  theReportCollection.toggleValidity(theRow);
}
