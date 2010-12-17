#ifndef qDFDisplayManager_HPP
#define qDFDisplayManager_HPP
#include <QMap>
#include <QString>

class qDFDisplayInterface;
class qDFProjReport;
#include <DF_Proj_Point.hpp>
class qDFDisplayManager
{

public:
  qDFDisplayManager() {};  // constructor does nothing, really
  ~qDFDisplayManager();

  void addDisplay(const QString displayName, qDFDisplayInterface *theDisplay, 
             bool enabled=true);
  void enableDisplay(QString displayName);
  void disableDisplay(QString displayName);

  void initializeDisplays();
  void clearDisplays();
  void closeDisplays();
  void displayDFReport(const qDFProjReport *theReport);
  void undisplayDFReport(const qDFProjReport *theReport);
  void displayLSFix(DFLib::Proj::Point & LSFixPoint);
  void undisplayLSFix();
  void displayFCAFix(DFLib::Proj::Point & FCAFixPoint, std::vector<double> stddevs);
  void undisplayFCAFix();
  void displayMLFix(DFLib::Proj::Point & MLFixPoint, double am2=0, double bm2=0, double phi=0);
  void undisplayMLFix();
  void displayBPEFix(DFLib::Proj::Point & BPEFixPoint, double am2=0, double bm2=0, double phi=0);
  void undisplayBPEFix();
  
private:
  QMap<QString,bool> enabledMap_;
  QMap<QString,qDFDisplayInterface *> interfacesMap_;
};

#endif
