#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QRegularExpression>

#include "ui_settingsDialog.h"
#include "Settings.hpp"

class settingsDialog : public QDialog, public Ui::settingsDialog
{
  Q_OBJECT;

public:
  settingsDialog(const Settings &theSettings, QWidget *parent=0);

private:
  QRegularExpression portValidator;

public:
  void retrieveSettings(Settings &theSettings);

};

#endif
