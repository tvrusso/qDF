#ifdef _MSC_VER
#define _USE_MATH_DEFINES
#include <cfloat>
inline bool isnan(double v) {return _isnan(v)!=0;}
inline bool isinf(double v) {return !_finite(v);}
#endif

#include <QtGui>
#include <sstream>
#include <iostream>
using namespace std;

#include "MainWindow.h"
#include "Settings.hpp"
#include "getReportDialog.h"
#include "reportToggleDialog.h"
#include "settingsDialog.h"
#include "qDFProjReport.hpp"
#include "aprsDisplay.hpp"
#include "rawTextDisplay.hpp"
#include "kmlDisplay.hpp"
#include "qDFDisplayManager.hpp"
#include <Util_Misc.hpp>

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent),
    dirtyCollection(false),
    currentFileName("")
{
  setupUi(this);

  menuHelp->addAction(QWhatsThis::createAction());

  readSettings();
  theAPRS.setPort(theSettings_.getAPRSPort());
  theAPRS.setServer(theSettings_.getAPRSServer());
  theAPRS.setCallsign(theSettings_.getAPRSCallsign());
  theAPRS.setCallpass(theSettings_.getAPRSCallpass());

  theAPRSDisplay_ = new aprsDisplay(&theAPRS,aprsPacketsTextEdit);
  theRTDisplay_ = new rawTextDisplay();
  theKMLDisplay_ = new kmlDisplay("qDF_GE.kml");

  theDisplayManager.addDisplay("MainWindow",this,true);
  theDisplayManager.addDisplay("APRS",theAPRSDisplay_,theSettings_.publishAPRS());
  theDisplayManager.addDisplay("RawText",theRTDisplay_,false);
  theDisplayManager.addDisplay("KML",theKMLDisplay_,true);
}

MainWindow::~MainWindow()
{
  theReportCollection.deleteReports();
}

void MainWindow::setupConnections()
{
  // File menu
  connect(actionSave_As, SIGNAL(triggered()), this, SLOT(saveAs()));
  connect(actionSave, SIGNAL(triggered()), this, SLOT(save()));
  connect(actionOpen, SIGNAL(triggered()), this, SLOT(open()));
  connect(actionClose, SIGNAL(triggered()), this, SLOT(closeFile()));

  // Edit menu:
  connect(actionSettings, SIGNAL(triggered()), this, SLOT(editSettings()));

  // Report menu
  connect(actionNew_Report, SIGNAL(triggered()), this, SLOT(newReportClicked()));
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
  connect(actionAbout_Qt,SIGNAL(triggered()),this,SLOT(aboutQt()));

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
  QList<QString> existingNames=theReportCollection.getReportNames();

  getReportDialog theRD(theSettings_,existingNames);

  if (theRD.exec())
  {
    string reportName= theRD.lineEdit_ReportName->text().toStdString();
    QString coordSysName=theRD.comboBox_CoordSys->currentText();
    QString equipType=theRD.comboBox_EquipmentType->currentText();
    QString quality=theRD.comboBox_Quality->currentText();
    double bearing=theRD.lineEdit_bearing->text().toDouble()+
      theSettings_.getDefaultDeclination();
    double sd=
      theSettings_.getStandardDeviation(equipType, quality);
    bool isValid=theRD.checkBox_validity->isChecked();

    // This is not right yet..., need to check for zone.  Don't worry now.
    //    CoordSysBuilder myCSB;
    CoordSys myCS=theSettings_.getCoordSys(coordSysName);

    QVector<double> llCoords(2);
    theRD.latLon->getCoords(llCoords);
    vector<double> coords(2,0.0);
    coords[0]=llCoords[1]; // llCoords[1] is latitude
    coords[1]=llCoords[0]; // llcoords[0] is longitude.  Rearrange for DFLib,
                           // which expects longitude (X) and latitude (Y) in 
                           // that order.

    qDFProjReport *theNewReport = new qDFProjReport(coords,bearing,sd,
                                                    reportName,myCS,
                                                    equipType, quality);
    if (isValid)
      theNewReport->setValid();
    else
      theNewReport->setInvalid();


    // At this point, if the collection is completely empty, intialize the displays.  WE'll add our
    // new report after this, and the displays will get updated.
    if (theReportCollection.size() == 0)
    {
      theDisplayManager.initializeDisplays();
    }

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
     CoordSys myCS=theSettings_.getDefaultCS();
     
     vector<double> foo(2,0.0);
     vector<double> FCA_stddev(2);
     vector<string> formattedCoords;

     DFLib::Proj::Point FCA(foo,myCS.getProj4Params());
     bool FCA_computed=false;
     DFLib::Proj::Point LSFix=FCA;
     bool LSFix_computed=false;
     DFLib::Proj::Point MLFix=LSFix;
     bool MLFix_computed=false;
     DFLib::Proj::Point StansfieldFix=LSFix;
     bool StansfieldFix_computed=false;

     FCA_computed=theReportCollection.computeFixCutAverage(FCA,FCA_stddev,
                                         theSettings_.getDefaultFCAMinAngle());
     
     if (nreports>2)
     {
       theReportCollection.computeLeastSquaresFix(LSFix);
       LSFix_computed=true;
     }
     if (nreports >= 2)
     {
       try
       {
         theReportCollection.computeMLFix(MLFix);
         MLFix_computed=checkValidMLFix(MLFix);
         if (!MLFix_computed)
         {
           MLFix=LSFix; // reset
           theReportCollection.aggressiveComputeMLFix(MLFix);
           MLFix_computed=checkValidMLFix(MLFix);
         }
       }
       catch (...)
       {
         // If computeMLFix throws an exception, such as for
         // convergence failures, ignore it
         MLFix_computed=false;
       }
       double am2ML=0;
       double bm2ML=0;
       double phiML;
       if (MLFix_computed)
       {
         theReportCollection.computeCramerRaoBounds(MLFix,am2ML,bm2ML,phiML);
       }

       // Compute the Stansfield fix and its error ellipse parameters:
       double am2,bm2,phi;
       try
       {
         theReportCollection.computeStansfieldFix(StansfieldFix,am2,bm2,phi);
         StansfieldFix_computed=true;
       }
       catch (...)
       {
         StansfieldFix_computed=false;
       }

       theDisplayManager.displayLSFix(LSFix);

       if (MLFix_computed)
       {
         theDisplayManager.displayMLFix(MLFix,am2ML,bm2ML,phiML);
       }
       else
       {
         theDisplayManager.undisplayMLFix();
       }

       if (StansfieldFix_computed)
       {
         theDisplayManager.displayBPEFix(StansfieldFix,am2,bm2,phi);
       }
       else
       {
         theDisplayManager.undisplayBPEFix();
       }

     }
     else
     {
       theDisplayManager.undisplayMLFix();
       theDisplayManager.undisplayLSFix();
     }       
     if (FCA_computed)
     {
       theDisplayManager.displayFCAFix(FCA,FCA_stddev);
     }
     else
     {
       theDisplayManager.undisplayFCAFix();
     }
       
   }
   else
   {
     // we don't have enough reports do do any analysis, make sure we don't
     // show data for the fixes.
     theDisplayManager.undisplayFCAFix();
     theDisplayManager.undisplayMLFix();
     theDisplayManager.undisplayLSFix();
     theDisplayManager.undisplayBPEFix();
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
  theDisplayManager.closeDisplays();
}

void MainWindow::updateCollectionDisplay(int reportIndex)
{
  theDisplayManager.displayDFReport(dynamic_cast<const qDFProjReport *>(theReportCollection.getReport(reportIndex)));
}

void MainWindow::listItemDoubleClicked(QListWidgetItem *item)
{
  //  int theReportIndex=theReportCollection.getReportIndex(item->data(Qt::UserRole).toString().toStdString());
  //  theReportCollection.toggleValidity(theReportIndex);
  

  QString reportName=item->data(Qt::UserRole).toString();

  editReport(reportName);
}

void MainWindow::editReport(QString & rN)
{
  qDFProjReport * theReportPtr = theReportCollection.getReportPointer(rN);

  if (theReportPtr)
  {
    getReportDialog theRD(theSettings_,theReportPtr);

    if (theRD.exec())
    {
      QString coordSysName=theRD.comboBox_CoordSys->currentText();
      QString equipType=theRD.comboBox_EquipmentType->currentText();
      QString quality=theRD.comboBox_Quality->currentText();
      double bearing=theRD.lineEdit_bearing->text().toDouble()+
        theSettings_.getDefaultDeclination();
      double sd=
        theSettings_.getStandardDeviation(equipType, quality);
      bool isValid=theRD.checkBox_validity->isChecked();
      
      // This is not right yet..., need to check for zone.  Don't worry now.
      //    CoordSysBuilder myCSB;
      CoordSys myCS=theSettings_.getCoordSys(coordSysName);
      
      QVector<double> llCoords(2);
      theRD.latLon->getCoords(llCoords);
      vector<double> coords(2,0.0);
      coords[0]=llCoords[1]; // llCoords[1] is latitude
      coords[1]=llCoords[0]; // llcoords[0] is longitude.  Rearrange for DFLib,
                             // which expects longitude (X) and latitude (Y) in 
                             // that order.

      if (isValid)
        theReportPtr->setValid();
      else
        theReportPtr->setInvalid();
      // The next line takes care of the "emit" of a reportChanged"
      theReportPtr->setAll(coords,bearing,sd,myCS,equipType,quality);
    }
  }
}

void MainWindow::aboutQt()
{
  QMessageBox::aboutQt(this,tr("About Qt"));
}

void MainWindow::about()
{
  QMessageBox::about(this,tr("About qDF"),
                     tr("<h2>qDF 0.2</h2>"
                        "<p>Copyright &copy; 2010 Tom Russo, KM5VY."
                        "<p>qDF is an application for computing "
                        "fixes of transmitter location from bearings-only "
                        "measurements by receivers."
                        "<p>It computes the Fix Cut Average, Least Squares, "
                        "Maximum Likelihood, and Stansfield fixes."
                        "<p>The Fix Cut Average is the average position of "
                        "pairwise intersections of report lines."
                        "<p>The Least Squares fix is the point that minimizes "
                        "the sum of squares of perpendicular distances to "
                        "bearing lines."
                        "<p>The Maximum Likelihood fix is the point that "
                        "maximizes the probability distribution assuming "
                        "Gaussian distributions of error from each receiver."
                        "<p>The Stansfield fix is a version of the Maximum "
                        "Likelihood fix that incorporates the assumption that "
                        "angular error is small and distance between receiver "
                        "and transmitter is large.  It is based on the seminal "
                        "1947 paper by R.G. Stansfield."));
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (okToContinue()) {
      writeSettings();
      event->accept();
    } else {
      event->ignore();
    }
    if (theReportCollection.size())
    {
      theDisplayManager.closeDisplays();
      theReportCollection.deleteReports();
      // Now we have an issue.  If we just exit now, the (possibly lengthy)
      // process of deleting APRS objects has only just begun.  So what we'll do
      // is ignore this event and set it up so that when APRS emits a signal 
      // that the queue is cleared, we get called again.  At that point,
      // theReportCollection will have been emptied, so this will work.
      if (theSettings_.publishAPRS())
      {
        connect(&theAPRS,SIGNAL(queueCleared()),this,SLOT(close()));
        event->ignore();
      }
    }

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

  int defNS=qsettings.value("defaultNS",
                               theSettings_.getDefaultNSHemisphere()).toInt();
  theSettings_.setDefaultNSHemisphere(defNS);
  int defEW=qsettings.value("defaultEW",
                               theSettings_.getDefaultEWHemisphere()).toInt();
  theSettings_.setDefaultEWHemisphere(defEW);

  quint16 defPort=qsettings.value("aprsPort",
                                  theSettings_.getAPRSPort()).toInt();
  theSettings_.setAPRSPort(defPort);

  QString aprsServer=qsettings.value("aprsServer",
                                theSettings_.getAPRSServer()).toString();
  theSettings_.setAPRSServer(aprsServer);

  QString aprsCallsign=qsettings.value("aprsCallsign",
                                theSettings_.getAPRSCallsign()).toString();
  theSettings_.setAPRSCallsign(aprsCallsign);

  QString aprsCallpass=qsettings.value("aprsCallpass",
                                theSettings_.getAPRSCallpass()).toString();
  theSettings_.setAPRSCallpass(aprsCallpass);


  bool publishAPRS=qsettings.value("publishAPRS",
                                   theSettings_.publishAPRS()).toBool();
  theSettings_.setPublishAPRS(publishAPRS);

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
  settings.setValue("defaultNS",theSettings_.getDefaultNSHemisphere());
  settings.setValue("defaultEW",theSettings_.getDefaultEWHemisphere());
  settings.setValue("aprsPort",theSettings_.getAPRSPort());
  settings.setValue("aprsCallsign",theSettings_.getAPRSCallsign());
  settings.setValue("aprsCallpass",theSettings_.getAPRSCallpass());
  settings.setValue("aprsServer",theSettings_.getAPRSServer());
  settings.setValue("publishAPRS",theSettings_.publishAPRS());
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
                         tr("The file is not a qDF file for this version of qDF."));
    return false;
  }

  theReportCollection.deleteReports();
  theDisplayManager.initializeDisplays();

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
  if (okToContinue()) 
  {
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

void MainWindow::closeFile()
{
  if (okToContinue()) 
  {
    theDisplayManager.closeDisplays();
    theReportCollection.deleteReports();
    currentFileName="";
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

void MainWindow::editSettings()
{
  settingsDialog theSD(theSettings_);

  if (theSD.exec())
  {
    theSD.retrieveSettings(theSettings_);
    theAPRS.setPort(theSettings_.getAPRSPort());
    theAPRS.setServer(theSettings_.getAPRSServer());
    theAPRS.setCallsign(theSettings_.getAPRSCallsign());
    theAPRS.setCallpass(theSettings_.getAPRSCallpass());

    if (theSettings_.publishAPRS())
      theDisplayManager.enableDisplay("APRS");
    else
      theDisplayManager.disableDisplay("APRS");

  }

}

// ML sometimes returns complete garbage for the ML fix.  When it does,
// the point is off at infinity.  Sanity check:
bool MainWindow::checkValidMLFix(DFLib::Proj::Point &thePoint)
{
  
  DFLib::Proj::Point tempPoint=thePoint;

  for (int i=0; i<theReportCollection.size();i++)
  {
    if (theReportCollection.isValid(i))
    {
      tempPoint=dynamic_cast<const DFLib::Proj::Report *>(theReportCollection.getReport(i))->getReceiverPoint();
      break;
    }
  }
  tempPoint.setUserProj(theSettings_.getDefaultCS().getProj4Params());
  vector<double>latlon=thePoint.getUserCoords();
  vector<double>merc=thePoint.getXY();
  vector<double>r0_coords=tempPoint.getUserCoords();

  if (!(isinf(latlon[0])||isinf(latlon[1])||isnan(latlon[0])||isnan(latlon[1])
        ||
        isinf(merc[0])||isinf(merc[1])||isnan(merc[0])||isnan(merc[1])))
  {
    // Use the Haversine formula to compute the distance between the fix and
    // the first valid report location we have.
    // compute very rough distance on sphere with haversine formula:
    double dlon=(latlon[0]-r0_coords[0])/RAD_TO_DEG;
    double dlat=(latlon[1]-r0_coords[1])/RAD_TO_DEG;
    double haversin_a=sin(dlat/2.0)*sin(dlat/2.0)+cos(latlon[1]/RAD_TO_DEG)*cos(r0_coords[1]/RAD_TO_DEG)*sin(dlon/2)*sin(dlon/2);
    double haversin_c=2*atan2(sqrt(haversin_a),sqrt(1-haversin_a));
    double haversin_d=3596*haversin_c;   // miles, give or take
    return (haversin_d<100);
  }
  return(false);
}

// qDFDisplayInterface methods
void MainWindow::initializeDisplay()
{
  // Takes no initialization steps
}

void MainWindow::clearDisplay()
{
  label_FCA_Longitude->setText("Not Available");
  label_FCA_Latitude->setText("Not Available");
  label_FCA_Longitude_SD->setText("Not Available");
  label_FCA_Latitude_SD->setText("Not Available");
  label_ML_Longitude->setText("Not Available");
  label_ML_Latitude->setText("Not Available");
  label_LS_Longitude->setText("Not Available");
  label_LS_Latitude->setText("Not Available");
  label_Stansfield_Longitude->setText("Not Available");
  label_Stansfield_Latitude->setText("Not Available");

  reportListWidget->clear();
}

void MainWindow::closeDisplay()
{
  clearDisplay();
}

void MainWindow::displayDFReport(const qDFProjReport *theReport)
{
  vector<string> theProj4Params = theSettings_.getDefaultCS().getProj4Params();
  QString theReportName=theReport->getReportNameQS();
  QString theReportSummary=theReport->getReportSummary(theProj4Params);

  // Don't like this, but we need the index to get at the item in the list widget
  int reportIndex = theReportCollection.getReportIndex(theReport);
  // First job is to update the display in the clickable list

  // add (or replace) the report summary to the list
  QListWidgetItem *theWidgetItem=reportListWidget->item(reportIndex);
  if(theWidgetItem)
  {
    theWidgetItem->setText(theReportSummary);
  }
  else
  {
    reportListWidget->insertItem(reportIndex,theReportSummary);
    theWidgetItem=reportListWidget->item(reportIndex);
  }

  // set the data for the listitem to be our name.
  theWidgetItem->setData(Qt::UserRole,theReportName);

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

}

void MainWindow::displayLSFix(DFLib::Proj::Point & LSFix)
{
  
  vector<string> formattedCoords;
  vector<double> LS_point=LSFix.getUserCoords();
  //       printCoords(LS_point,string("Least Squares Fix"));
  formatCoords(LS_point,formattedCoords);
  label_LS_Longitude->setText(QString::fromStdString(formattedCoords[0]));
  label_LS_Latitude->setText(QString::fromStdString(formattedCoords[1]));
}

void MainWindow::undisplayLSFix()
{
  label_LS_Longitude->setText("Not Available");
  label_LS_Latitude->setText("Not Available");
}

void MainWindow::displayFCAFix(DFLib::Proj::Point & FCAFix, std::vector<double> FCA_stddev)
{
  vector<string> formattedCoords;
  vector<double> FCA_point=FCAFix.getUserCoords();
  //     printCoords(FCA_point,string("Fix Cut Average"));
  formatCoords(FCA_point,formattedCoords);
  label_FCA_Longitude->setText(QString::fromStdString(formattedCoords[0]));
  label_FCA_Latitude->setText(QString::fromStdString(formattedCoords[1]));
  ostringstream os;
  os << FCA_stddev[0];
  label_FCA_Longitude_SD->setText(QString::fromStdString(os.str()));
  os.str("");
  os << FCA_stddev[1];
  label_FCA_Latitude_SD->setText(QString::fromStdString(os.str()));
}

void MainWindow::undisplayFCAFix()
{
  label_FCA_Longitude->setText("Not Available");
  label_FCA_Latitude->setText("Not Available");
  label_FCA_Longitude_SD->setText("Not Available");
  label_FCA_Latitude_SD->setText("Not Available");
}

void MainWindow::displayBPEFix(DFLib::Proj::Point & StansfieldFix, double am2, double bm2, double phi)
{
  vector<string> formattedCoords;
  vector<double> Stansfield_point=StansfieldFix.getUserCoords();
  //       printCoords(ML_point,string("Maximum Likelihood Fix"));
  formatCoords(Stansfield_point,formattedCoords);
  label_Stansfield_Longitude->setText(QString::fromStdString(formattedCoords[0]));
  label_Stansfield_Latitude->setText(QString::fromStdString(formattedCoords[1]));

  // we ignore the error stuff for the main window display
}

void MainWindow::undisplayBPEFix()
{
  label_Stansfield_Longitude->setText("Not Available");
  label_Stansfield_Latitude->setText("Not Available");
}

void MainWindow::displayMLFix(DFLib::Proj::Point & MLFix, double am2, double bm2, double phi)
{
  vector<string> formattedCoords;
  vector<double> ML_point=MLFix.getUserCoords();
  
  //       printCoords(ML_point,string("Maximum Likelihood Fix"));
  formatCoords(ML_point,formattedCoords);
  label_ML_Longitude->setText(QString::fromStdString(formattedCoords[0]));
  label_ML_Latitude->setText(QString::fromStdString(formattedCoords[1]));
}

void MainWindow::undisplayMLFix()
{
  label_ML_Longitude->setText("Not Available");
  label_ML_Latitude->setText("Not Available");
}
