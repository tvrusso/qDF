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
  emit collectionCleared();
}

int qDFProjReportCollection::addReport(qDFProjReport * aReport)
{
  int returnValue=DFLib::ReportCollection::addReport(dynamic_cast<DFLib::Abstract::Report *>(aReport));

  // This makes sure that whenever any report in our collection emits a signal
  // that it's changed, we emit a signal that the collection has changed.
  connect(aReport,SIGNAL(reportChanged()),this,SLOT(reportChanged()));
  emit collectionChanged();
  emit collectionChanged(returnValue); // for those customers who care which
                                       // is the new report index

  return (returnValue);
}

void qDFProjReportCollection::newReport(qDFProjReport * aReport)
{
  
  int i=addReport(aReport);
  //  cout << "newReport slot called, added report number "<< i << endl;
}

void qDFProjReportCollection::reportChanged()
{
  //  cout << " Collection has been informed that a report has changed. " << endl;
  //  cout << " Pointer to that report is " << QObject::sender() << endl;
  int index=getReportIndex(dynamic_cast<qDFProjReport *>(QObject::sender()));
  //  cout << " that is index " << index << " in the collection." << endl;
  emit collectionChanged(index); // for those who care which
  emit collectionChanged(); // for those who don't

}

string qDFProjReportCollection::getReportSummary(int reportIndex,
                                                 const vector<string> & projArgs) const
{
  return ((dynamic_cast<const qDFProjReport *>(getReport(reportIndex))->getReportSummary(projArgs)));
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

  for (int i=0; i<nrep; i++)
  {
    QString reportName;
    vector<double> coords(2);
    double bearing;
    double sigma;
    bool validity;
    QString csName;
    quint32 zone;

    CoordSysBuilder csB;
    in >> reportName;
    in >> coords[0] >> coords[1];
    in >> bearing >> sigma;
    in >> validity;
    in >> csName;
    in >> zone;

    CoordSys CS=csB.getCoordSys(csName.toStdString());
    if (CS.isZoneRequired())
      CS.setZone(zone);

    qDFProjReport *nR = new qDFProjReport(coords,bearing,sigma,reportName.
                                          toStdString(),CS);
    if (validity)
      nR->setValid();
    else
      nR->setInvalid();
    tC.addReport(nR);
  }
  return in;
}
