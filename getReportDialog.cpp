#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif

#include "getReportDialog.h"
#include "Settings.hpp"
#include "qDFProjReport.hpp"

getReportDialog::getReportDialog(const Settings &theSettings,const QList<QString> &existingNames, QWidget *parent)
  :QDialog(parent),
   settings_(theSettings),
   existingNames_(existingNames),
   theReport_(0)
{
  setupUi(this);
  connect(comboBox_EquipmentType,SIGNAL(currentTextChanged(const QString &)),
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

  latLon->setDefaultHemispheres(settings_.getDefaultNSHemisphere(),settings_.getDefaultEWHemisphere());

  emit somethingChanged();
}

getReportDialog::getReportDialog(const Settings &theSettings,const qDFProjReport *theReport, QWidget *parent)
  :QDialog(parent),
   settings_(theSettings),
   theReport_(theReport)
{
  setupUi(this);
  existingNames_.clear();

  connect(comboBox_EquipmentType,SIGNAL(currentTextChanged(const QString &)),
          this,SLOT(equipmentChanged(const QString &)));
  connect(latLon,SIGNAL(coordsChanged()),
          this,SLOT(somethingChanged()));
  connect(lineEdit_bearing,SIGNAL(textChanged(const QString &)),
          this,SLOT(somethingChanged()));

  QString theDefaultCSName=theReport_->getCSName();

  // Get the coordinate system set up
  comboBox_CoordSys->clear();
  comboBox_CoordSys->addItems(settings_.getSupportedCoordSys());
  int theDefaultIndex=comboBox_CoordSys->findText(theDefaultCSName);
  comboBox_CoordSys->setCurrentIndex(theDefaultIndex);

  // Set the equipment type (quality will be handled later)
  comboBox_EquipmentType->clear();
  comboBox_EquipmentType->addItems(settings_.getEquipTypes());
  comboBox_EquipmentType->setCurrentIndex(comboBox_EquipmentType->findText(theReport_->getEquipType()));

  QVector<double> llCoords=theReport_->getReceiverLocationUser();
  
  // Initialize the lat/lon box.
  latLon->setCoords(llCoords);

  // Initialize and disable the name
  lineEdit_ReportName->setText(theReport->getReportNameQS());
  lineEdit_ReportName->setEnabled(false);

  // Initialize the bearing
  lineEdit_bearing->setText(QString::number(theReport_->getBearing()-settings_.getDefaultDeclination(),'f'));
  
  // Initialize validity:
  if (theReport_->isValid())
    checkBox_validity->setChecked(true);
  else
    checkBox_validity->setChecked(false);

  emit somethingChanged();
}

void getReportDialog::equipmentChanged(const QString & equipmentSelected)
{
  comboBox_Quality->clear();
  comboBox_Quality->addItems(settings_.getQualityNames(equipmentSelected));
  if (theReport_)
    comboBox_Quality->setCurrentIndex(comboBox_Quality->findText(theReport_->getQuality()));
  else
    comboBox_Quality->setCurrentIndex(0);
}

// This sets the OK button enabled or disabled depending on whether
// we have valid data
void getReportDialog::somethingChanged()
{
  bool enableOK=false;
  QPushButton * okButton = buttonBox->button(QDialogButtonBox::Ok);

  if (!(lineEdit_ReportName->text().isEmpty()) && 
      existingNames_.indexOf(lineEdit_ReportName->text())==-1 &&
      latLon->isValid() &&
      !(lineEdit_bearing->text().isEmpty()))
  {
    enableOK=true;
    okButton->setToolTip(tr("Click to enter the report."));
  }
  else
  {
    enableOK=false;
    if (lineEdit_ReportName->text().isEmpty())
      okButton->setToolTip(tr("You must enter a report name"));
    else if (existingNames_.indexOf(lineEdit_ReportName->text())!=-1)
      okButton->setToolTip(tr("You have entered the name of an existing report.  Please choose another name."));
    else if (!latLon->isValid())
      okButton->setToolTip(tr("You must enter a valid latitude and longitude."));
    else 
      okButton->setToolTip(tr("You must enter a valid bearing."));

  }

  buttonBox->button(QDialogButtonBox::Ok)->setEnabled(enableOK);
}
