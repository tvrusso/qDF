#ifndef rawTextDisplay_HPP
#define rawTextDisplay_HPP
#include <QFile>
#include <QTextStream>

#include <DF_Proj_Point.hpp>
#include "CoordSysBuilder.hpp"
#include "qDFDisplayInterface.hpp"

// Forward declaration, because we don't really need the guts of the class here
class qDFProjReport;

class rawTextDisplay : public qDFDisplayInterface
{

public:
  // the constructor does nothing at all.
  rawTextDisplay();
  ~rawTextDisplay();
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
  virtual void undisplayDFReport(const qDFProjReport *theReport){};
  virtual void displayLSFix(DFLib::Proj::Point & LSFixPoint);
  virtual void undisplayLSFix();
  virtual void displayFCAFix(DFLib::Proj::Point & FCAFixPoint, std::vector<double> stddevs);
  virtual void undisplayFCAFix();
  virtual void displayMLFix(DFLib::Proj::Point & MLFixPoint, double am2=0, double bm2=0, double phi=0);
  virtual void undisplayMLFix();
  virtual void displayBPEFix(DFLib::Proj::Point & BPEFixPoint, double am2=0, double bm2=0, double phi=0);
  virtual void undisplayBPEFix();

private:
  QFile theFile_;
  QTextStream rawTextOut_;
  bool fileIsOpen_;
  CoordSys myCS_;

};

#endif
