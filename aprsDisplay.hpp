#ifndef aprsDisplay_HPP
#define aprsDisplay_HPP
#include "qDFDisplayInterface.hpp"
// forward references for our declaration purposes
class APRS;
class QPlainTextEdit;
class qDFProjReport;
#include <QString>
#include <DF_Proj_Point.hpp>
#include <vector>
#include "CoordSysBuilder.hpp"

class aprsDisplay :  public qDFDisplayInterface
{

public:
  aprsDisplay(APRS *theAPRS,  QPlainTextEdit *theAPRSTextEdit);
  virtual void clearDisplay();
  virtual void initializeDisplay();
  virtual void closeDisplay();
  virtual void displayDFReport(const qDFProjReport *theReport);
  virtual void undisplayDFReport(const qDFProjReport *theReport);
  virtual void displayLSFix(DFLib::Proj::Point & LSFixPoint);
  virtual void undisplayLSFix();
  virtual void displayFCAFix(DFLib::Proj::Point & FCAFixPoint, std::vector<double> stddevs);
  virtual void undisplayFCAFix();
  virtual void displayMLFix(DFLib::Proj::Point & MLFixPoint, double am2=0, double bm2=0, double phi=0);
  virtual void undisplayMLFix();
  virtual void displayBPEFix(DFLib::Proj::Point & BPEFixPoint, double am2=0, double bm2=0, double phi=0);
  virtual void undisplayBPEFix();

private:
  APRS *theAPRS_;
  QPlainTextEdit *aprsPacketsTextEdit_;
  CoordSys myCS_;
  void errorEllipse_(DFLib::Proj::Point & theCenterPoint, double am2, double bm2, double phi, 
                     int percent, std::vector<double> & lats, std::vector<double> & lons);
  void displayAPRSText_(const QString &);
  void aprsPointObject_(const QString &oName, 
                        const std::vector<double>& oPoint,
                        const QString &oSym,
                        const QString & oComment);
  void aprsDFErrorObject_(const QString &oName,const std::vector<double> &oPoint,
                          const std::vector<double> &oSDs);

  void aprsRotatedEllipse_(const QString &oName,DFLib::Proj::Point &thePoint,
                             double am2, double bm2, double phi, int percent);
  void deleteAPRSObject_(const QString &oName);
};
#endif
