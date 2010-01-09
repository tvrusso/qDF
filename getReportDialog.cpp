#include <QtGui>

#include "getReportDialog.h"
#include "Settings.hpp"

getReportDialog::getReportDialog(const Settings &theSettings,QWidget *parent)
  :QDialog(parent),
   settings_(theSettings)
{
  setupUi(this);
  connect(comboBox_EquipmentType,SIGNAL(currentIndexChanged(const QString &)),
          this,SLOT(equipmentChanged(const QString &)));
  connect(lineEdit_ReportName,SIGNAL(textChanged(const QString &)),
          this,SLOT(somethingChanged()));
  connect(latLon,SIGNAL(coordsChanged()),
          this,SLOT(somethingChanged()));
  connect(lineEdit_bearing,SIGNAL(textChanged(const QString &)),
          this,SLOT(somethingChanged()));

  QString theDefaultCSName=settings_.getDefaultCSName();

  comboBox_CoordSys->clear();
  comboBox_CoordSys->addItems(settings_.getSupportedCoordSys());
  int theDefaultIndex=comboBox_CoordSys->findText(theDefaultCSName);
  comboBox_CoordSys->setCurrentIndex(theDefaultIndex);

  comboBox_EquipmentType->clear();
  comboBox_EquipmentType->addItems(settings_.getEquipTypes());
  comboBox_EquipmentType->setCurrentIndex(0);
  emit somethingChanged();
}

void getReportDialog::equipmentChanged(const QString & equipmentSelected)
{
  comboBox_Quality->clear();
  comboBox_Quality->addItems(settings_.getQualityNames(equipmentSelected));
  comboBox_Quality->setCurrentIndex(0);
}

// This sets the OK button enabled or disabled depending on whether
// we have valid data
void getReportDialog::somethingChanged()
{
  bool enableOK=false;

  if (!(lineEdit_ReportName->text().isEmpty()) && 
      latLon->isValid() &&
      !(lineEdit_bearing->text().isEmpty()))
  {
    enableOK=true;
  }
  else
  {
    enableOK=false;
  }

  buttonBox->button(QDialogButtonBox::Ok)->setEnabled(enableOK);
}
