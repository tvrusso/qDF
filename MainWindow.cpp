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
  : QMainWindow(parent),
    dirtyCollection(false),
    currentFileName(""),
    theAPRS("localhost",2023,"KM5VY-8","21753")
{
  setupUi(this);

  menuHelp->addAction(QWhatsThis::createAction());

  readSettings();
}

MainWindow::~MainWindow()
{
  theReportCollection.deleteReports();
}

void MainWindow::setupConnections()
{
  // Report menu
  connect(actionNew_Report, SIGNAL(triggered()), this, SLOT(newReportClicked()));
  connect(actionSave_As, SIGNAL(triggered()), this, SLOT(saveAs()));
  connect(actionSave, SIGNAL(triggered()), this, SLOT(save()));
  connect(actionOpen, SIGNAL(triggered()), this, SLOT(open()));

  // Report menu
  connect(actionToggle_Validity, SIGNAL(triggered()), this, SLOT(toggleValidityClicked()));

  connect(this, SIGNAL(newReportCreated(qDFProjReport *)), 
          &theReportCollection, SLOT(newReport(qDFProjReport *)));

  connect(&theReportCollection,SIGNAL(collectionChanged()),
          this,SLOT(reportCollectionChanged()));

  connect(&theReportCollection,SIGNAL(collectionCleared()),
          this,SLOT(reportCollectionChanged()));

  connect(&theReportCollection,SIGNAL(collectionCleared()),
          this,SLOT(clearCollectionDisplay()));

  connect(&theReportCollection,SIGNAL(collectionChanged(int)),
          this,SLOT(updateCollectionDisplay(int)));

  connect(reportListWidget,SIGNAL(itemDoubleClicked(QListWidgetItem *)),
          this,SLOT(listItemDoubleClicked(QListWidgetItem *)));

  // Help menu
  connect(actionAbout,SIGNAL(triggered()),this,SLOT(about()));

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


    // This is not right yet..., need to check for zone.  Don't worry now.
    //    CoordSysBuilder myCSB;
    CoordSys myCS=theSettings_.getCoordSys(coordSysName);

    vector<double> coords(2,0.0);

    //KLUDGE!  Only convert DMS strings if the coordinate system is lat/lon!
    DFLib::Proj::Point junk(coords,myCS.getProj4Params());
    if (junk.isUserProjLatLong())
    {
      coords[0]=dmstor(coord0.c_str(),NULL)*RAD_TO_DEG;
      coords[1]=dmstor(coord1.c_str(),NULL)*RAD_TO_DEG;
    }
    else
    {
      coords[0]=strtod(coord0.c_str(),NULL);
      coords[1]=strtod(coord1.c_str(),NULL);
    }
      

    qDFProjReport *theNewReport = new qDFProjReport(coords,bearing,sd,
                                                    reportName,myCS);
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

   if (nreports>0)
     dirtyCollection=true;  // otherwise we might have cleared it!

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
     theReportCollection.computeFixCutAverage(FCA,FCA_stddev,
                                              theSettings_.getDefaultFCAMinAngle());

     
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

void MainWindow::clearCollectionDisplay()
{
  dirtyCollection=false;
  reportListWidget->clear();
}

void MainWindow::updateCollectionDisplay(int reportIndex)
{
  CoordSys myCS=theSettings_.getCoordSys("WGS84 Lat/Lon");
  vector<string> theProj4Params=myCS.getProj4Params();
  QString theReportSummary=QString::fromStdString(theReportCollection.getReportSummary(reportIndex,theProj4Params) );

  // add (or replace) the report summary to the list
  QListWidgetItem *theWidgetItem=reportListWidget->item(reportIndex);
  if(theWidgetItem)
    theWidgetItem->setText(theReportSummary);
  else
  {
    reportListWidget->insertItem(reportIndex,theReportSummary);
    theWidgetItem=reportListWidget->item(reportIndex);
  }

  // set the validity indicator
  QFont theItemFont=theWidgetItem->font();
  if (theReportCollection.isValid(reportIndex))
  {
    theItemFont.setItalic(false);
  }
  else
  {
    theItemFont.setItalic(true);
  }
  theWidgetItem->setFont(theItemFont);


  // hack
  const qDFProjReport * report=dynamic_cast<const qDFProjReport *>(theReportCollection.getReport(reportIndex));
  if (report->isValid())
  {
    DFLib::Proj::Point tempPoint=report->getReceiverPoint();
    vector<double> coords(2);
    coords=tempPoint.getUserCoords();
    QString oName=QString::fromStdString(report->getReportName());
    QString aprsPosit=theAPRS.createDFObject(oName,coords,report->getBearing(),
                                             report->getSigma()," Hey, Foo!");
    listWidgetAPRS->addItem(aprsPosit);
  }
  else
  {
    QString oName=QString::fromStdString(report->getReportName());
    QString aprsPosit=theAPRS.deleteObject(oName);
    listWidgetAPRS->addItem(aprsPosit);
  }
    

}

void MainWindow::listItemDoubleClicked(QListWidgetItem *item)
{
  int theRow=reportListWidget->row(item);
  // we've assured that row and report index are the same.
  theReportCollection.toggleValidity(theRow);
}

void MainWindow::about()
{
  QMessageBox::about(this,tr("About qDF"),
                     tr("<h2>qDF 0.1</h2>"
                        "<p>Copyright &copy; 2009 Tom Russo."
                        "<p>qDF is a simple application for computing "
                        "fixes of transmitter location from bearings-only "
                        "measurements by receivers."
                        "<p>It computes the Fix Cut Average, Least Squares, "
                        "and Maximum Likelihood fixes."
                        "<p>The Fix Cut Average is the average position of "
                        "pairwise intersections of report lines."
                        "<p>The Least Squares fix is the point that minimizes "
                        "the sum of squares of perpendicular distances to "
                        "bearing lines."
                        "<p>The Maximum Likelihood fix is the point that "
                        "maximizes the probability distribution assuming "
                        "Gaussian distributions of error from each receiver."));
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (okToContinue()) {
      writeSettings();
      event->accept();
    } else {
      event->ignore();
    }
    theReportCollection.deleteReports();
}

bool MainWindow::okToContinue()
{
  if (theReportCollection.size()!=0 && dirtyCollection) 
  {
    int r = QMessageBox::warning(this, tr("qDF"),
                                 tr("There are reports in the collection"
                                    " that have not been saved\n"
                                    "Do you want to save them?"),
                                 QMessageBox::Yes | QMessageBox::Default,
                                 QMessageBox::No,
                                 QMessageBox::Cancel | QMessageBox::Escape);
    if (r == QMessageBox::Yes) 
    {
      return save();
    }
    else if (r == QMessageBox::Cancel) 
    {
      return false;
    }
  }
  return true;
}

void MainWindow::readSettings()
{
  QSettings qsettings("Tom Russo","qDF");

  QRect rect=qsettings.value("geometry",QRect(0,0,800,600)).toRect();
  move(rect.topLeft());
  resize(rect.size());

  QString csName=qsettings.value("defaultCoordSysName",
                                theSettings_.getDefaultCSName()).toString();
  theSettings_.setDefaultCS(csName);

  double defDec=qsettings.value("defaultDeclination",
                               theSettings_.getDefaultDeclination()).toDouble();
  theSettings_.setDefaultDeclination(defDec);

  double defFCAMA=qsettings.value("defaultFCAMinAngle",
                               theSettings_.getDefaultFCAMinAngle()).toDouble();
  theSettings_.setDefaultFCAMinAngle(defFCAMA);

  int defZone=qsettings.value("defaultUTMZone",
                               theSettings_.getDefaultUTMZone()).toInt();
  theSettings_.setDefaultUTMZone(defZone);

  // can't do the equipment map exactly.  Work it out...
}

void MainWindow::writeSettings()
{
  QSettings settings("Tom Russo","qDF");

  settings.setValue("geometry",geometry());
  settings.setValue("defaultCoordSysName",theSettings_.getDefaultCSName());
  settings.setValue("defaultDeclination",theSettings_.getDefaultDeclination());
  settings.setValue("defaultFCAMinAngle",theSettings_.getDefaultFCAMinAngle());
  settings.setValue("defaultUTMZone",theSettings_.getDefaultUTMZone());
}

bool MainWindow::writeFile(const QString &fileName)
{
  QFile file(fileName);
  if (!file.open(QIODevice::WriteOnly))
  {
    QMessageBox::warning(this, tr("qDF"),
                         tr("Cannot write file %1:\n%2.")
                         .arg(file.fileName())
                         .arg(file.errorString()));
    return false;
  }
  QDataStream out(&file);
  out.setVersion(QDataStream::Qt_4_4);

  out << quint32(MagicNumber);
  QApplication::setOverrideCursor(Qt::WaitCursor);

  out << theReportCollection;
  QApplication::restoreOverrideCursor();
  return true;
}

bool MainWindow::loadFile(const QString &fileName)
{
  QFile file(fileName);
  if (!file.open(QIODevice::ReadOnly))
  {
    QMessageBox::warning(this, tr("qDF"),
                         tr("Cannot read file %1:\n%2.")
                         .arg(file.fileName())
                         .arg(file.errorString()));
    return false;
  }
  QDataStream in(&file);
  in.setVersion(QDataStream::Qt_4_4);

  quint32 magic;
  in >> magic;
  if (magic != MagicNumber)
  {
    QMessageBox::warning(this,tr("qDF"),
                         tr("The file is not a qDF file."));
    return false;
  }

  theReportCollection.deleteReports();
  
  QApplication::setOverrideCursor(Qt::WaitCursor);

  in >> theReportCollection;
  dirtyCollection=false;
  QApplication::restoreOverrideCursor();
  return true;
}

bool MainWindow::save()
{
    if (currentFileName.isEmpty()) 
    {
      return saveAs();
    } 
    else 
    {
      return saveFile(currentFileName);
    }
}

bool MainWindow::saveAs()
{
  QString fileName = QFileDialog::getSaveFileName(this,
                                                  tr("Save Reports"),".",
                                                  tr("qDF Files (*.qdf)"));
  if (fileName.isEmpty())
    return false;
  return saveFile(fileName);
}

void MainWindow::open()
{
  if (okToContinue()) {
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open DF Reports"), ".",
                                                    tr("qDF files (*.qdf)"));
    if (!fileName.isEmpty())
    {
      loadFile(fileName);
      currentFileName=fileName;
    }

  }
}


bool MainWindow::saveFile(const QString &fileName)
{
  bool retcode=writeFile(fileName);
  if (retcode)
  {
    dirtyCollection=false;
    currentFileName=fileName;
  }

  return(retcode);
}
