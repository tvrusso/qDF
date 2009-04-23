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

  QString theDefaultCSName=settings_.getDefaultCSName();

  comboBox_CoordSys->clear();
  comboBox_CoordSys->addItems(settings_.getSupportedCoordSys());
  int theDefaultIndex=comboBox_CoordSys->findText(theDefaultCSName);
  comboBox_CoordSys->setCurrentIndex(theDefaultIndex);

  comboBox_EquipmentType->clear();
  comboBox_EquipmentType->addItems(settings_.getEquipTypes());
  comboBox_EquipmentType->setCurrentIndex(0);

}

void getReportDialog::equipmentChanged(const QString & equipmentSelected)
{
  comboBox_Quality->clear();
  comboBox_Quality->addItems(settings_.getQualityNames(equipmentSelected));
  comboBox_Quality->setCurrentIndex(0);
}
