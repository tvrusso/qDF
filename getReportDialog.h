#ifndef GETREPORTDIALOG_H
#define GETREPORTDIALOG_H

#include <QDialog>
#include "ui_getReportDialog.h"
#include "Settings.hpp"
#include "qDFProjReport.hpp"

class getReportDialog : public QDialog, public Ui::getReportDialog
{
  Q_OBJECT;

 public:
  getReportDialog(const Settings &,const QList<QString> &, QWidget *parent=0);
  getReportDialog(const Settings &,const qDFProjReport *, QWidget *parent=0);

 private:
  Settings settings_;
  QList<QString> existingNames_;
  const qDFProjReport *theReport_;

 private slots:
  void equipmentChanged(const QString &);
  void somethingChanged();
  
};

#endif

