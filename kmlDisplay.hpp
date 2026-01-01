#ifndef kmlDisplay_HPP
#define kmlDisplay_HPP
#include <QFile>
#include <QTextStream>
#include <QMap>
#include <QVector>
#include <DF_Proj_Point.hpp>
#include "CoordSysBuilder.hpp"
#include "qDFDisplayInterface.hpp"

// Forward declaration, because we don't really need the guts of the class here
class qDFProjReport;

class kmlDisplay : public qDFDisplayInterface
{

public:
  // the constructor does nothing at all.
  kmlDisplay(QString fileName);
  ~kmlDisplay();
  // A pure virtual member to force the class to be abstract
  // These methods *must* get implemented.
  virtual void clearDisplay();       // just clears the display
  virtual void initializeDisplay();  // Does any important initialization and clears disply
  // Everything else will default to null operations if the child class
  // chooses not to implement 'em
  virtual void closeDisplay();      // Close out a display when done with the problem.  May clear
                                       // but might not always (e.g. file-based displays might just
                                       // be closed with the final state of the solution still in them)
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

  void setFileName(QString theFileName);

private:
  QString theFileName_;   // name of "master" kml file.
  QString theDataFileName_; // name of the one we actually update often.
  QFile theFile_;
  QTextStream kmlFileOut_;
  CoordSys myCS_;
  bool opened_;

  QMap<QString,QString> dfReportStrings_;
  QMap<QString,QString> dfBearingStrings_;
  QMap<QString,QString> dfFixStrings_;
  QMap<QString,QString> dfEllipseStrings_;
  
  void computeRhumbline_(DFLib::Proj::Point & startingPoint, double azimuth, 
                         double meters, int npoints, 
                         QVector<double> &lats, QVector<double> &lons);
  void computeEllipse_(DFLib::Proj::Point & centerPoint, double am2, double bm2,
                       double phi, double rho, int npoints, 
                       QVector<double> &lats, QVector<double> &lons);

  void commitMaster_(bool refresh=true);
  void commit_();
  void outputPreamble_();
  void commitReports_();
  void commitFixes_();
  void commitEllipses_();
  void outputFooter_();
};

#endif
