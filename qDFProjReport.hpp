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
                const string &theName, const CoordSys &CS);

  virtual ~qDFProjReport();
  virtual void setReceiverLocationUser(vector<double> &theLocation);
  virtual void setReceiverLocationMercator(vector<double> &theLocation);
  virtual void setBearing(double Bearing);
  virtual void setSigma(double Sigma);
  virtual void toggleValidity();
  virtual string getReportSummary(const vector<string> &projArgs) const;

private:
  CoordSys theCS_;
  
 signals:
  void reportChanged();

  friend QDataStream &operator<<(QDataStream &out, 
                                 const qDFProjReport &aReport);

};
#endif
