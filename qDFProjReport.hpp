#ifndef qDFProjReport_HPP
#define qDFProjReport_HPP

#include <DF_Proj_Report.hpp>
#include <QDataStream>
#include <QObject>
#include "CoordSysBuilder.hpp"

class qDFProjReport:  public QObject, public DFLib::Proj::Report
{
  Q_OBJECT;

public:
  qDFProjReport(const vector <double> &theLocationUser,
                const double &bearing, const double &std_dev,
                const string &theName, const CoordSys &CS,
                const QString &equipmentType, const QString &quality);

  virtual ~qDFProjReport();
  virtual void setReceiverLocationUser(const vector<double> &theLocation);
  virtual void setReceiverLocationMercator(const vector<double> &theLocation);
  virtual void setBearing(double Bearing);
  virtual void setSigma(double Sigma);
  virtual void setUserProj(const vector<string> &projArgs);
  virtual void toggleValidity();
  virtual QString getReportNameQS() const;
  void  setEquipType(const QString & equipType);
  void  setQuality(const QString & quality);
  void  setCS(const CoordSys & cs);
  virtual QString getReportSummary(const vector<string> &projArgs) const;
  const QString & getEquipType() const;
  const QString & getQuality() const;
  const QString getCSName() const;
  QVector<double> getReceiverLocationUser() const;

  void setAll(const vector<double> &theLocationUser, const double &bearing,
              const double &std_dev, const CoordSys &CS,
              const QString &equipmentType, const QString &quality);

private:
  CoordSys theCS_;
  QString equipmentType_;
  QString quality_;

 signals:
  void reportChanged(qDFProjReport *);

  friend QDataStream &operator<<(QDataStream &out, 
                                 const qDFProjReport &aReport);

};
#endif
