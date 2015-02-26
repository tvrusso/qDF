#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif
#include "settingsDialog.h"
#include "Settings.hpp"

settingsDialog::settingsDialog(const Settings &theSettings, QWidget *parent)
  :QDialog(parent)
{
  setupUi(this);

  // Set up Geographic Settings
  QString theDefaultCSName = theSettings.getDefaultCSName();
  CSComboBox->clear();
  CSComboBox->addItems(theSettings.getSupportedCoordSys());
  int theDefaultIndex=CSComboBox->findText(theDefaultCSName);
  CSComboBox->setCurrentIndex(theDefaultIndex);
  DeclinationLineEdit->setText(QString::number(theSettings.getDefaultDeclination()));
  UTMZoneSpinBox->setValue(theSettings.getDefaultUTMZone());
  if (theSettings.getDefaultNSHemisphere()<0)
  {
    latSRadioButton->setChecked(true);
    latNRadioButton->setChecked(false);
  }
  else
  {
    latSRadioButton->setChecked(false);
    latNRadioButton->setChecked(true);
  }
  if (theSettings.getDefaultEWHemisphere()<0)
  {
    lonWRadioButton->setChecked(true);
    lonERadioButton->setChecked(false);
  }
  else
  {
    lonWRadioButton->setChecked(false);
    lonERadioButton->setChecked(true);
  }

  // set up DF Settings
  MinFCASpinBox->setValue(theSettings.getDefaultFCAMinAngle());

  // set up APRS settings
  APRSServerLineEdit->setText(theSettings.getAPRSServer());
  APRSServerPortLineEdit->setText(QString::number(theSettings.getAPRSPort()));
  portValidator=QRegExp("[0-9]{1,4}");
  APRSServerPortLineEdit->setValidator(new QRegExpValidator(portValidator,this));
  CallSignLineEdit->setText(theSettings.getAPRSCallsign());
  CallPassLineEdit->setText(theSettings.getAPRSCallpass());

  APRSPublishCheckBox->setChecked(theSettings.publishAPRS());
}

void settingsDialog::retrieveSettings(Settings &theSettings)
{
  // get Geo settings:
  theSettings.setDefaultCS(CSComboBox->currentText());
  theSettings.setDefaultDeclination(DeclinationLineEdit->text().toDouble());
  theSettings.setDefaultUTMZone(UTMZoneSpinBox->value());

  int NS=1;
  if (latSRadioButton->isChecked())
  {
    NS=-1;
  }
  int EW=1;
  if (lonWRadioButton->isChecked())
  {
    EW=-1;
  }
  theSettings.setDefaultNSHemisphere(NS);
  theSettings.setDefaultEWHemisphere(EW);

  // DF settings:
  theSettings.setDefaultFCAMinAngle(MinFCASpinBox->value());

  // APRS
  theSettings.setAPRSServer(APRSServerLineEdit->text());
  theSettings.setAPRSPort(APRSServerPortLineEdit->text().toInt());
  theSettings.setAPRSCallsign(CallSignLineEdit->text());
  theSettings.setAPRSCallpass(CallPassLineEdit->text());
  theSettings.setPublishAPRS(APRSPublishCheckBox->isChecked());
}
