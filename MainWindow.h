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
  enum { MagicNumber = 0xA550DEAD};

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
  QString currentFileName;

  APRS theAPRS;

  private slots:
   void newReportClicked();
   void toggleValidityClicked();
   void newReportReceived(qDFProjReport *report);
   void reportCollectionChanged();
   void updateCollectionDisplay(int);
   void clearCollectionDisplay();
   void listItemDoubleClicked(QListWidgetItem *);
   void about();
   bool saveAs();
   bool save();
   void open();

 signals:
   void newReportCreated(qDFProjReport *report);
};
#endif
