#include <iostream>
using namespace std;

#include <QDataStream>

#include "qDFProjReport.hpp"
#include "qDFProjReportCollection.hpp"

qDFProjReportCollection::qDFProjReportCollection()
  : DFLib::ReportCollection()
{
}

qDFProjReportCollection::~qDFProjReportCollection()
{
}

void qDFProjReportCollection::deleteReports()
{
  DFLib::ReportCollection::deleteReports();
  reportMap_.clear();
  emit collectionCleared();
}

int qDFProjReportCollection::addReport(qDFProjReport * aReport)
{
  int returnValue=DFLib::ReportCollection::addReport(dynamic_cast<DFLib::Abstract::Report *>(aReport));

  // Add the report to our name/pointer map:
  reportMap_.insert(QString::fromStdString(aReport->getReportName()),aReport);

  // This makes sure that whenever any report in our collection emits a signal
  // that it's changed, we emit a signal that the collection has changed.
  connect(aReport,SIGNAL(reportChanged(qDFProjReport *)),this,SLOT(reportChanged(qDFProjReport *)));
  emit collectionChanged();
  emit collectionChanged(returnValue); // for those customers who care which
                                       // is the new report index

  return (returnValue);
}

void qDFProjReportCollection::newReport(qDFProjReport * aReport)
{
  
  addReport(aReport);
  //  cout << "newReport slot called, added report number "<< i << endl;
}

void qDFProjReportCollection::reportChanged(qDFProjReport *theChangedReport)
{
  int index=getReportIndex(theChangedReport);
  emit collectionChanged(index); // for those who care which
  emit collectionChanged(); // for those who don't

}

QString qDFProjReportCollection::getReportName(int reportIndex)
{
  return (QString::fromStdString(dynamic_cast<const qDFProjReport *>(getReport(reportIndex))->getReportName()));
}


qDFProjReport * qDFProjReportCollection::getReportPointer(const QString &rN)
{
  return reportMap_[rN];
  // will return 0 if report doesn't exist.  Let the caller figure that out.
}

string qDFProjReportCollection::getReportSummary(const QString &reportName,
                                                 const vector<string> & projArgs) const
{
  return (reportMap_[reportName]->getReportSummary(projArgs));
}

QDataStream & operator<<(QDataStream &out, const qDFProjReportCollection &tC)
{
  out << quint32(tC.size());
  for (int i=0; i<tC.size(); i++)
  {
    out << *(dynamic_cast<const qDFProjReport *> (tC.getReport(i)));
  }

  return out;

}

QDataStream & operator>>(QDataStream &in, qDFProjReportCollection &tC)
{

  quint32 nrep;

  in >> nrep;

  for (unsigned int i=0; i<nrep; i++)
  {
    QString reportName;
    vector<double> coords(2);
    double bearing;
    double sigma;
    bool validity;
    QString csName;
    quint32 zone;
    QString equipType;
    QString quality;
    CoordSysBuilder csB;
    in >> reportName;
    in >> coords[0] >> coords[1];
    in >> bearing >> sigma;
    in >> validity;
    in >> csName;
    in >> zone;
    in >> equipType;
    in >> quality;

    CoordSys CS=csB.getCoordSys(csName.toStdString());
    if (CS.isZoneRequired())
      CS.setZone(zone);

    qDFProjReport *nR = new qDFProjReport(coords,bearing,sigma,reportName.
                                          toStdString(),CS, equipType,
                                          quality);
    if (validity)
      nR->setValid();
    else
      nR->setInvalid();
    tC.addReport(nR);
  }
  return in;
}
