// Base class for the qDF display interface

// Include the header, because we need references even here in the abstract class
#include <DF_Proj_Point.hpp>

// Forward declaration, because we don't really need the guts of the class here
class qDFProjReport;

class qDFDisplayInterface
{

public:
  // This is largely an abstract class, so
  // the constructor does nothing at all.
  qDFDisplayInterface() {};

  // A pure virtual member to force the class to be abstract
  // These methods *must* get implemented.
  virtual void clear()=0;
  virtual void initialize()=0;
  // Everything else will default to null operations if the child class
  // chooses not to implement 'em
  virtual void close() {};
  virtual void displayDFRreport(const qDFProjReport *theReport){};
  virtual void undisplayDFReport(const qDFProjReport *theReport){};
  virtual void displayLSFix(DFLib::Proj::Point & LSFixPoint){};
  virtual void undisplayLSFix(){};
  virtual void displayFCAFix(DFLib::Proj::Point & FCAFixPoint, std::vector<double> stddevs){};
  virtual void undisplayFCAFix(){};
  virtual void displayMLFix(DFLib::Proj::Point & MLFixPoint, double am2=0, double bm2=0, double phi=0){};
  virtual void undisplayMLFix(){};
  virtual void displayBPEFix(DFLib::Proj::Point & BPEFixPoint, double am2=0, double bm2=0, double phi=0){};
  virtual void undisplayBPEFix(){};
};

