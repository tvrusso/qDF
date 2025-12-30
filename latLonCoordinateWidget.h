#ifndef  LATLONCOORDINATEWIDGET_H
#define  LATLONCOORDINATEWIDGET_H

#include <QWidget>
#include <QVector>
#include <QRegularExpression>

#include <ui_latLonCoordinateWidget.h>

class latLonCoordinateWidget : public QWidget, public Ui::latLonCoordinateWidget
{

  Q_OBJECT;

 public: 
  latLonCoordinateWidget(QWidget *parent=0);
  ~latLonCoordinateWidget();
  bool isValid();
  void getCoords(QVector<double> &coordVect);
  void setCoords(QVector<double> &coordVect);
  void setDefaultHemispheres(int NS, int EW);

 private:
  QRegularExpression latDegRegExp;
  QRegularExpression lonDegRegExp;
  QRegularExpression intRegExp;
  QRegularExpression intLatRegExp;
  QRegularExpression intLonRegExp;
  QRegularExpression minSecDecRegExp;
  double latitude;
  double longitude;
  
  bool latValid();
  bool lonValid();

signals:
  void coordsChanged();

 private slots:
  void latDegChanged(const QString &);
  void lonDegChanged(const QString &);
  void latMinChanged(const QString &);
  void lonMinChanged(const QString &);
  void latSecChanged(const QString &);
  void lonSecChanged(const QString &);
  void updateLatitude();
  void updateLongitude();
};
#endif
