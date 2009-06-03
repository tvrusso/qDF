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
  getReportDialog theRD(theSettings_);

  if (theRD.exec())
  {
    string reportName= theRD.lineEdit_ReportName->text().toStdString();
    QString coordSysName=theRD.comboBox_CoordSys->currentText();
    double bearing=theRD.lineEdit_bearing->text().toDouble()+9.8;
    double sd=
      theSettings_.getStandardDeviation(theRD.comboBox_EquipmentType->currentText(),
                                        theRD.comboBox_Quality->currentText());


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

       vector<double> LS_point=LSFix.getUserCoords();
       //       printCoords(LS_point,string("Least Squares Fix"));
       formatCoords(LS_point,formattedCoords);
       label_LS_Longitude->setText(QString::fromStdString(formattedCoords[0]));
       label_LS_Latitude->setText(QString::fromStdString(formattedCoords[1]));


       aprsPointObject("LS-Fix",LS_point,"Ln"," Least Squares Solution");

       if (MLFix_computed)
       {
         vector<double> ML_point=MLFix.getUserCoords();
         //       printCoords(ML_point,string("Maximum Likelihood Fix"));
         formatCoords(ML_point,formattedCoords);
         label_ML_Longitude->setText(QString::fromStdString(formattedCoords[0]));
         label_ML_Latitude->setText(QString::fromStdString(formattedCoords[1]));
         
         aprsPointObject("ML-Fix",ML_point,"Mn"," Maximum Likelihood Solution");
       }
       else
       {
         label_ML_Longitude->setText("Invalid");
         label_ML_Latitude->setText("Invalid");
       }

       if (StansfieldFix_computed)
       {
         vector<double> Stansfield_point=StansfieldFix.getUserCoords();
         //       printCoords(ML_point,string("Maximum Likelihood Fix"));
         formatCoords(Stansfield_point,formattedCoords);
         label_Stansfield_Longitude->setText(QString::fromStdString(formattedCoords[0]));
         label_Stansfield_Latitude->setText(QString::fromStdString(formattedCoords[1]));
         
         aprsPointObject("BPE-Fix",Stansfield_point,"Sn"," Stansfield BPE");

         aprsStansfieldEllipse(StansfieldFix,am2,bm2,phi,50);
         aprsStansfieldEllipse(StansfieldFix,am2,bm2,phi,75);
         aprsStansfieldEllipse(StansfieldFix,am2,bm2,phi,95);
       }
       else
       {
         label_Stansfield_Longitude->setText("Invalid");
         label_Stansfield_Latitude->setText("Invalid");
         deleteAPRSObject("BPE-Fix");
         deleteAPRSObject("SErr50");
         deleteAPRSObject("SErr75");
         deleteAPRSObject("SErr95");
       }

     }
     else
     {
       label_ML_Longitude->setText("Not Available");
       label_ML_Latitude->setText("Not Available");
       label_LS_Longitude->setText("Not Available");
       label_LS_Latitude->setText("Not Available");

       deleteAPRSObject("ML-Fix");
       deleteAPRSObject("LS-Fix");
       
     }       
     if (FCA_computed)
     {
       vector<double> FCA_point=FCA.getUserCoords();
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
       
       aprsPointObject("FCA-Fix",FCA_point, "An"," Fix Cut Average Solution");

       if (FCA_stddev[0] != 0 && FCA_stddev[1] != 0)
       {
         aprsDFErrorObject("FCA-err", FCA_point,FCA_stddev);
       }
     }
     else
     {
       label_FCA_Longitude->setText("Not Available");
       label_FCA_Latitude->setText("Not Available");
       label_FCA_Longitude_SD->setText("Not Available");
       label_FCA_Latitude_SD->setText("Not Available");
     }
       
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

     deleteAPRSObject("ML-Fix");
     deleteAPRSObject("LS-Fix");
     deleteAPRSObject("FCA-Fix");
     deleteAPRSObject("FCA-err");
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

  //APRS: must delete all objects here, too
  deleteAllAPRSObjects();

}

void MainWindow::updateCollectionDisplay(int reportIndex)
{
  vector<string> theProj4Params=theSettings_.getDefaultCS().getProj4Params();
  QString theReportSummary=QString::fromStdString(theReportCollection.getReportSummary(reportIndex,theProj4Params) );


  // First job is to update the display in the clickable list

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


  // Next job is to send objects to APRS
  sendReportAPRS(dynamic_cast<const qDFProjReport *>(theReportCollection.getReport(reportIndex)));
    

}

void MainWindow::listItemDoubleClicked(QListWidgetItem *item)
{
  int theRow=reportListWidget->row(item);
  // we've assured that row and report index are the same.
  theReportCollection.toggleValidity(theRow);
}

void MainWindow::aboutQt()
{
  QMessageBox::aboutQt(this,tr("About Qt"));
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
    if (theReportCollection.size())
    {
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
  }

}

void MainWindow::displayAPRSText(const QString & str)
{
  aprsPacketsTextEdit->insertPlainText(str);
  aprsPacketsTextEdit->insertPlainText("\n");
}  

void MainWindow::sendReportAPRS(const qDFProjReport *theReport)
{
  if (theSettings_.publishAPRS())
  {
    if (theReport->isValid())
    {
      DFLib::Proj::Point tempPoint=theReport->getReceiverPoint();
      tempPoint.setUserProj(theSettings_.getDefaultCS().getProj4Params());
      vector<double> coords(2);
      coords=tempPoint.getUserCoords();
      QString oName=QString::fromStdString(theReport->getReportName());
      QString aprsPosit=theAPRS.createDFObject(oName,coords,
                                               theReport->getBearing(),
                                               theReport->getSigma(),
                                               " via qDF");
      displayAPRSText(aprsPosit);
    }
    else
    {
      QString oName=QString::fromStdString(theReport->getReportName());
      QString aprsPosit=theAPRS.deleteObject(oName);
      if (!aprsPosit.isEmpty()) displayAPRSText(aprsPosit);
    }
  }
}

void MainWindow::aprsPointObject(const QString &oName, 
                                 const vector<double>& oPoint,
                                 const QString &oSym,
                                 const QString & oComment)
{
  if (theSettings_.publishAPRS())
  {
    QString aprsPosit=theAPRS.createObject(oName,oPoint,oSym,oComment);
    displayAPRSText(aprsPosit);
  }
}

void MainWindow::aprsDFErrorObject(const QString &oName,
                                   const vector<double>&oPoint,
                                   const vector<double>&oSDs)
{
  if (theSettings_.publishAPRS())
  {
    QString aprsPosit=theAPRS.createDFErrorObject(oName,oPoint,oSDs[0],oSDs[1]);
    displayAPRSText(aprsPosit);
  }
}

void MainWindow::deleteAPRSObject(const QString &oName)
{
  if (theSettings_.publishAPRS())
  {
    QString aprsPosit=theAPRS.deleteObject(oName);
    if (!aprsPosit.isEmpty()) 
      displayAPRSText(aprsPosit);
  }
}

void MainWindow::deleteAllAPRSObjects()
{
  if (theSettings_.publishAPRS())
  {
    QStringList foo=theAPRS.deleteAllObjects();
    foreach (QString str,foo)
    {
      displayAPRSText(str);
    }
  }
}

void MainWindow::aprsStansfieldEllipse(DFLib::Proj::Point &thePoint,
                                       double am2, double bm2, double phi,
                                       int percent)
{
  if (theSettings_.publishAPRS())
  {
    if (am2>0 && bm2>0)
    {
      
      // this is tricky.  We need to calculate the Stansfield ellipses in 
      // XY coordinates, then convert their points to lat/lon before generating
      // the APRS object.
      
      double P=((double)percent)/100.0;
      double rho=-2*log(1-P);
      DFLib::Proj::Point tempPoint(thePoint);
      vector<double> lats;
      vector<double> lons;
      vector<double> centerCoords=thePoint.getXY();
      vector<double> tempCoords(2);
      double a=sqrt(1.0/am2);
      double b=sqrt(1.0/bm2);
      double cosphi=cos(phi);
      double sinphi=sin(phi);
      const double pi=4*atan(1.0);

      // this is the equation of an ellipse of axis a*rho and b*rho, rotated
      // by an angle phi.
      for (int i=0;i<16;i++)
      {
        tempCoords[0]=centerCoords[0]
          +a*rho*cosphi*cos(2.0*pi/15.0*i)-b*rho*sinphi*sin(2.0*pi/15.0*i);
        tempCoords[1]=centerCoords[1]
          +a*rho*sinphi*cos(2.0*pi/15.0*i)+b*rho*cosphi*sin(2.0*pi/15.0*i);

        // those are the coordinates in XY.  Now get 'em in lat/lon
        tempPoint.setXY(tempCoords);
        tempCoords=tempPoint.getUserCoords();
        lons.push_back(tempCoords[0]);
        lats.push_back(tempCoords[1]);
      }

      // we now have our rotated ellipse transformed to lat/lon.  Make the
      // APRS object
      char colorStyle;
      if (percent>=50)
        colorStyle='g';
      if (percent>=75)
        colorStyle='e';
      if (percent>90)
        colorStyle='a';
      centerCoords=thePoint.getUserCoords();
      QString aprsPosit=
        theAPRS.createMultilineObject(QString("SErr%1").arg(percent),
                                      lats,lons,centerCoords,
                                      colorStyle,0,
                                      "\\l");
      displayAPRSText(aprsPosit);
    }
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

