#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "ui_MainWindow.h"
#include "qDFProjReport.hpp"
#include "qDFProjReportCollection.hpp"
#include "Settings.hpp"
#include "APRS.hpp"

class MainWindow : public QMainWindow, public Ui::MainWindow
{
  Q_OBJECT ;
 public:
  MainWindow(QWidget *parent = 0);
  ~MainWindow();

  void setupConnections();

 protected:
  void closeEvent(QCloseEvent *event);

 private:
  enum { MagicNumber = 0xDFDFa550};

  qDFProjReportCollection theReportCollection;
  void printCoords(const vector<double> &ll,const string &text);
  void formatCoords(const vector<double> &ll,vector<string> &formattedCoords);
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

  void sendReportAPRS(const qDFProjReport * theReport);
  void aprsPointObject(const QString &oName,const vector<double> & oPoint,
                       const QString &oSym,
                       const QString &oComment);
  void aprsDFErrorObject(const QString &oName,const vector<double> &oPoint,
                         const vector<double> &oSDs);
  void aprsStansfieldEllipse(DFLib::Proj::Point &thePoint,
                             double am2, double bm2, double phi, int percent);

  void deleteAPRSObject(const QString &oName);
  void deleteAllAPRSObjects();
  
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
