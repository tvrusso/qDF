#ifndef qDFProjReportCollection_HPP
#define qDFProjReportCollection_HPP

#include <DF_Report_Collection.hpp>
#include <QObject>
#include "qDFProjReport.hpp"

class qDFProjReportCollection: public QObject, public DFLib::ReportCollection
{
  Q_OBJECT;

public:
  qDFProjReportCollection();
  ~qDFProjReportCollection();

  int addReport(qDFProjReport *theReport);
  string getReportSummary(int reportIndex,const vector<string>&projArgs) const;
  virtual void deleteReports();

signals:
  void collectionChanged();
  void collectionCleared();
  void collectionChanged(int i);
                        
public slots:
  void reportChanged();
  void newReport(qDFProjReport *theReport);

  friend QDataStream &operator<<(QDataStream &out, 
                                 const qDFProjReportCollection &aCollection);
  friend QDataStream &operator>>(QDataStream &in, 
                                 qDFProjReportCollection &aCollection);

};
#endif
