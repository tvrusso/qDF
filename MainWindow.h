#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "ui_MainWindow.h"
#include "qDFProjReport.hpp"
#include "qDFProjReportCollection.hpp"
#include "Settings.hpp"

class MainWindow : public QMainWindow, public Ui::MainWindow
{
  Q_OBJECT ;
 public:
  MainWindow(QWidget *parent = 0);
  ~MainWindow();

  void setupConnections();

 private:
  qDFProjReportCollection theReportCollection;
  void printCoords(const vector<double> &ll,const string &text);
  void formatCoords(const vector<double> &ll,vector<string> &formattedCoords);
  Settings theSettings_;

  private slots:
   void newReportClicked();
   void toggleValidityClicked();
   void newReportReceived(qDFProjReport *report);
   void reportCollectionChanged();
   void updateCollectionDisplay(int);
   void listItemDoubleClicked(QListWidgetItem *);

 signals:
   void newReportCreated(qDFProjReport *report);
};
#endif
