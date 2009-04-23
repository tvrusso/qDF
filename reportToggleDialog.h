#ifndef REPORTTOGGLEDIALOG_H
#define REPORTTOGGLEDIALOG_H

#include <QDialog>
#include "ui_reportToggleDialog.h"

class reportToggleDialog : public QDialog, public Ui::reportToggleDialog
{
  Q_OBJECT;

 public:
  reportToggleDialog(QWidget *parent=0);
  
};

#endif

