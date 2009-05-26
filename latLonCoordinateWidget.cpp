#include <QtGui>
#include "latLonCoordinateWidget.h"

latLonCoordinateWidget::latLonCoordinateWidget(QWidget *parent)
  :QWidget(parent)
{
  setupUi(this);
  QWidget::setFocusProxy(latDegLineEdit);

  // Initialize the regexps:
  latDegRegExp=QRegExp("([0-8]{0,1}[0-9]||90)(?:\\.[0-9]*){0,1}");
  lonDegRegExp=QRegExp("([0]{0,1}[0-9]{1,2}||1[0-7][0-9]||180)(?:\\.[0-9]*){0,1}");
  minSecDecRegExp=QRegExp("[0-5]{0,1}[0-9](?:\\.[0-9]*){0,1}");
  intRegExp=QRegExp("[0-5]{0,1}[0-9]");
  intLonRegExp=QRegExp("([0]{0,1}[0-9]{1,2}||1[0-7][0-9]||180)");
  intLatRegExp=QRegExp("([0-8]{0,1}[0-9]||90)");

  // Our initial validators (will be modified as text is entered)
  latDegLineEdit->setValidator(new QRegExpValidator(latDegRegExp, this));
  lonDegLineEdit->setValidator(new QRegExpValidator(lonDegRegExp, this));
  latMinLineEdit->setValidator(new QRegExpValidator(minSecDecRegExp, this));
  lonMinLineEdit->setValidator(new QRegExpValidator(minSecDecRegExp, this));
  latSecLineEdit->setValidator(new QRegExpValidator(minSecDecRegExp, this));
  lonSecLineEdit->setValidator(new QRegExpValidator(minSecDecRegExp, this));

  // This is some ugly stuff that really indicates that Lat and Lon need
  // to be refactored so that there's less duplication.
  connect(latDegLineEdit,SIGNAL(textChanged(const QString &)),this,SLOT(latDegChanged(const QString &)));
  connect(latMinLineEdit,SIGNAL(textChanged(const QString &)),this,SLOT(latMinChanged(const QString &)));
  connect(latSecLineEdit,SIGNAL(textChanged(const QString &)),this,SLOT(latSecChanged(const QString &)));
  connect(lonDegLineEdit,SIGNAL(textChanged(const QString &)),this,SLOT(lonDegChanged(const QString &)));
  connect(lonMinLineEdit,SIGNAL(textChanged(const QString &)),this,SLOT(lonMinChanged(const QString &)));
  connect(lonSecLineEdit,SIGNAL(textChanged(const QString &)),this,SLOT(lonSecChanged(const QString &)));

  connect(latNRadioButton,SIGNAL(toggled(bool)),this,SLOT(updateLatitude()));
  connect(latSRadioButton,SIGNAL(toggled(bool)),this,SLOT(updateLatitude()));
  connect(lonERadioButton,SIGNAL(toggled(bool)),this,SLOT(updateLongitude()));
  connect(lonWRadioButton,SIGNAL(toggled(bool)),this,SLOT(updateLongitude()));

}

latLonCoordinateWidget::~latLonCoordinateWidget()
{
}

void latLonCoordinateWidget::latDegChanged(const QString &boxText)
{
  // if we have a decimal point, clear and disable minutes and seconds.
  if (boxText.contains('.'))
  {
    latMinLineEdit->setText("");
    latMinLineEdit->setEnabled(false);
    latSecLineEdit->setText("");
    latSecLineEdit->setEnabled(false);
  }
  else
  {
    latMinLineEdit->setValidator(new QRegExpValidator(minSecDecRegExp, this));
    latSecLineEdit->setValidator(new QRegExpValidator(minSecDecRegExp, this));
    latMinLineEdit->setEnabled(true);
    latSecLineEdit->setEnabled(true);
  }

  updateLatitude();
}
void latLonCoordinateWidget::lonDegChanged(const QString &boxText)
{
  // if we have a decimal point, clear and disable minutes and seconds.
  if (boxText.contains('.'))
  {
    lonMinLineEdit->setText("");
    lonMinLineEdit->setEnabled(false);
    lonSecLineEdit->setText("");
    lonSecLineEdit->setEnabled(false);
  }
  else
  {
    lonMinLineEdit->setValidator(new QRegExpValidator(minSecDecRegExp, this));
    lonSecLineEdit->setValidator(new QRegExpValidator(minSecDecRegExp, this));
    lonMinLineEdit->setEnabled(true);
    lonSecLineEdit->setEnabled(true);
  }
  updateLongitude();
}

void latLonCoordinateWidget::latMinChanged(const QString &boxText)
{
  //  if we're empty, then both degrees and seconds are legit.  Set 
  // validators appropriately
    if (boxText.isEmpty())
    {
      latDegLineEdit->setValidator(new QRegExpValidator(latDegRegExp, this));
      latMinLineEdit->setValidator(new QRegExpValidator(minSecDecRegExp, this));
      latSecLineEdit->setValidator(new QRegExpValidator(minSecDecRegExp, this));
      latSecLineEdit->setEnabled(true);
    }
    else
    {
      // If we have any minutes specified, then degrees must be integer:
      int degrees=latDegLineEdit->text().toInt();
      latDegLineEdit->setText(QString::number(degrees));
      latDegLineEdit->setValidator(new QRegExpValidator(intLatRegExp, this));

      if (boxText.contains('.'))
      {
        // and if we have a decimal point, disable seconds
        latSecLineEdit->setText("");
        latSecLineEdit->setEnabled(false);
      }
      else
      {
        latSecLineEdit->setEnabled(true);
      }
    }
  updateLatitude();
}

void latLonCoordinateWidget::lonMinChanged(const QString &boxText)
{
  //  if we're empty, then both degrees and seconds are legit.  Set 
  // validators appropriately
    if (boxText.isEmpty())
    {
      lonDegLineEdit->setValidator(new QRegExpValidator(lonDegRegExp, this));
      lonMinLineEdit->setValidator(new QRegExpValidator(minSecDecRegExp, this));
      lonSecLineEdit->setValidator(new QRegExpValidator(minSecDecRegExp, this));
      lonSecLineEdit->setEnabled(true);
    }
    else
    {
      // If we have any minutes specified, then degrees must be integer:
      int degrees=lonDegLineEdit->text().toInt();
      lonDegLineEdit->setText(QString::number(degrees));
      lonDegLineEdit->setValidator(new QRegExpValidator(intLonRegExp, this));

      if (boxText.contains('.'))
      {
        // and if we have a decimal point, disable seconds
        lonSecLineEdit->setText("");
        lonSecLineEdit->setEnabled(false);
      }
      else
      {
        lonSecLineEdit->setEnabled(true);
      }
    }
  updateLongitude();
}

void latLonCoordinateWidget::latSecChanged(const QString &boxText)
{
  //  if we're empty, then decimal degrees and decimal minutes, and secs
  // are legit.  Set validators appropriately
    if (boxText.isEmpty())
    {
      latDegLineEdit->setValidator(new QRegExpValidator(latDegRegExp, this));
      latMinLineEdit->setValidator(new QRegExpValidator(minSecDecRegExp, this));
      latSecLineEdit->setValidator(new QRegExpValidator(minSecDecRegExp, this));
    }
    else
    {
      // If we have any seconds specified, then degs and mins must be integer:
      int degrees=latDegLineEdit->text().toInt();
      latDegLineEdit->setText(QString::number(degrees));
      latDegLineEdit->setValidator(new QRegExpValidator(intLatRegExp, this));

      int mins=latMinLineEdit->text().toInt();
      latMinLineEdit->setText(QString::number(mins));
      latMinLineEdit->setValidator(new QRegExpValidator(intRegExp, this));
    }
  updateLatitude();
}

void latLonCoordinateWidget::lonSecChanged(const QString &boxText)
{
  //  if we're empty, then decimal degrees and decimal minutes, and secs
  // are legit.  Set validators appropriately
    if (boxText.isEmpty())
    {
      lonDegLineEdit->setValidator(new QRegExpValidator(lonDegRegExp, this));
      lonMinLineEdit->setValidator(new QRegExpValidator(minSecDecRegExp, this));
      lonSecLineEdit->setValidator(new QRegExpValidator(minSecDecRegExp, this));
    }
    else
    {
      // If we have any seconds specified, then degs and mins must be integer:
      int degrees=lonDegLineEdit->text().toInt();
      lonDegLineEdit->setText(QString::number(degrees));
      lonDegLineEdit->setValidator(new QRegExpValidator(intLonRegExp, this));

      int mins=lonMinLineEdit->text().toInt();
      lonMinLineEdit->setText(QString::number(mins));
      lonMinLineEdit->setValidator(new QRegExpValidator(intRegExp, this));
    }
  updateLongitude();
}

void latLonCoordinateWidget::updateLatitude()
{
  latitude=0;
  if(latDegLineEdit->hasAcceptableInput()
     &&(latMinLineEdit->text().isEmpty()||latMinLineEdit->hasAcceptableInput())
     &&(latSecLineEdit->text().isEmpty()||latSecLineEdit->hasAcceptableInput()))
  {
    latitude=latDegLineEdit->text().toDouble();
    if (!(latMinLineEdit->text().isEmpty()))
    {    
      latitude +=latMinLineEdit->text().toDouble()/60;
      if (!(latSecLineEdit->text().isEmpty()))
      {    
        latitude +=latSecLineEdit->text().toDouble()/3600;
      }
    }
  }
  if (latSRadioButton->isChecked())
    latitude *= -1;

  emit (coordsChanged());
}

void latLonCoordinateWidget::updateLongitude()
{

  longitude=0;
  if(lonDegLineEdit->hasAcceptableInput()
     &&(lonMinLineEdit->text().isEmpty()||lonMinLineEdit->hasAcceptableInput())
     &&(lonSecLineEdit->text().isEmpty()||lonSecLineEdit->hasAcceptableInput()))
  {
    longitude=lonDegLineEdit->text().toDouble();
    if (!(lonMinLineEdit->text().isEmpty()))
    {    
      longitude +=lonMinLineEdit->text().toDouble()/60;
      if (!(lonSecLineEdit->text().isEmpty()))
      {    
        longitude +=lonSecLineEdit->text().toDouble()/3600;
      }
    }
  }
  if (lonWRadioButton->isChecked())
    longitude *= -1;
  emit (coordsChanged());
}

bool latLonCoordinateWidget::latValid()
{
  return (latitude>=-90 && latitude<=90);
}

bool latLonCoordinateWidget::lonValid()
{
  return (longitude >= -180 && longitude<=180) ;
}

void latLonCoordinateWidget::getCoords(QVector<double> &coordVect)
{
  coordVect.resize(2);
  coordVect[0]=latitude;
  coordVect[1]=longitude;

}

bool latLonCoordinateWidget::isValid()
{
  return (latValid() && lonValid());
}
