#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "ui_MainWindow.h"
#include "qDFProjReport.hpp"
#include "qDFProjReportCollection.hpp"
#include "Settings.hpp"
#include "APRS.hpp"
#include "qDFDisplayInterface.hpp"
#include "qDFDisplayManager.hpp"

class aprsDisplay;
class rawTextDisplay;
class kmlDisplay;
class MainWindow : public QMainWindow, public Ui::MainWindow, public qDFDisplayInterface
{
  Q_OBJECT ;
 public:
  MainWindow(QWidget *parent = 0);
  ~MainWindow();

  void setupConnections();

  // display interface methods
  virtual void clearDisplay();
  virtual void initializeDisplay();
  virtual void closeDisplay();
  virtual void displayDFReport(const qDFProjReport *theReport);
  //  virtual void undisplayDFReport(const qDFProjReport *theReport);
  virtual void displayLSFix(DFLib::Proj::Point & LSFixPoint);
  virtual void undisplayLSFix();
  virtual void displayFCAFix(DFLib::Proj::Point & FCAFixPoint, std::vector<double> stddevs);
  virtual void undisplayFCAFix();
  virtual void displayMLFix(DFLib::Proj::Point & MLFixPoint, double am2=0, double bm2=0, double phi=0);
  virtual void undisplayMLFix();
  virtual void displayBPEFix(DFLib::Proj::Point & BPEFixPoint, double am2=0, double bm2=0, double phi=0);
  virtual void undisplayBPEFix();

  
 protected:
  void closeEvent(QCloseEvent *event);

 private:
  enum { MagicNumber = 0xDFDFa550};

  qDFProjReportCollection theReportCollection;
  void printCoords(const std::vector<double> &ll,const std::string &text);
  void formatCoords(const std::vector<double> &ll,std::vector<std::string> &formattedCoords);
  Settings theSettings_;
  bool okToContinue();
  void readSettings();
  void writeSettings();
  bool saveFile(const QString &fileName);
  bool writeFile(const QString &fileName);
  bool loadFile(const QString &fileName);
  bool dirtyCollection;
  void displayAPRSText(const QString &str);
  QString currentFileName;

  APRS theAPRS;
  aprsDisplay *theAPRSDisplay_;
  rawTextDisplay *theRTDisplay_;
  kmlDisplay *theKMLDisplay_;

  qDFDisplayManager theDisplayManager;

  bool checkValidMLFix(DFLib::Proj::Point &thePoint);
  void editReport(QString &rN);

  private slots:
   void newReportClicked();
   void toggleValidityClicked();
   void newReportReceived(qDFProjReport *report);
   void reportCollectionChanged();
   void updateCollectionDisplay(int);
   void clearCollectionDisplay();
   void listItemDoubleClicked(QListWidgetItem *);
   void about();
   void aboutQt();
   bool saveAs();
   bool save();
   void open();
   void closeFile();
   void editSettings();

 signals:
   void newReportCreated(qDFProjReport *report);
};
#endif
