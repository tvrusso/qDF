#ifndef qDFProjReportCollection_HPP
#define qDFProjReportCollection_HPP

#include <DF_Report_Collection.hpp>
#include <QObject>
#include <QMap>
#include <QString>
#include "qDFProjReport.hpp"

class qDFProjReportCollection: public QObject, public DFLib::ReportCollection
{
  Q_OBJECT;

public:
  qDFProjReportCollection();
  ~qDFProjReportCollection();

  int addReport(qDFProjReport *theReport);
  QString getReportSummary(const QString &reportName,
                           const std::vector<std::string>&projArgs) const;
  QString getReportName(int reportIndex);
  virtual void deleteReports();
  QList<QString> getReportNames() { return reportMap_.keys();};
  qDFProjReport * getReportPointer(const QString & rN);

private:
  QMap<QString,qDFProjReport*> reportMap_;

signals:
  void collectionChanged();
  void collectionCleared();
  void collectionChanged(int i);
                        
public slots:
  void reportChanged(qDFProjReport *);
  void newReport(qDFProjReport *theReport);

  friend QDataStream &operator<<(QDataStream &out, 
                                 const qDFProjReportCollection &aCollection);
  friend QDataStream &operator>>(QDataStream &in, 
                                 qDFProjReportCollection &aCollection);

};
#endif
