#include <iostream>
using namespace std;

#include "qDFProjReport.hpp"
#include "qDFProjReportCollection.hpp"

qDFProjReportCollection::qDFProjReportCollection()
  : DFLib::ReportCollection()
{
}

qDFProjReportCollection::~qDFProjReportCollection()
{
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
