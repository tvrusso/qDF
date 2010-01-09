#ifndef GETREPORTDIALOG_H
#define GETREPORTDIALOG_H

#include <QDialog>
#include "ui_getReportDialog.h"
#include "Settings.hpp"

class getReportDialog : public QDialog, public Ui::getReportDialog
{
  Q_OBJECT;

 public:
  getReportDialog(const Settings &,QWidget *parent=0);

 private:
  Settings settings_;

 private slots:
  void equipmentChanged(const QString &);
  void somethingChanged();
  
};

#endif

