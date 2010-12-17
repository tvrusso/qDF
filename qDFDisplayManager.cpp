#include "qDFDisplayInterface.hpp"
#include <DF_Proj_Point.hpp>
#include "qDFDisplayManager.hpp"

qDFDisplayManager::~qDFDisplayManager()
{
  closeDisplays();
  enabledMap_.clear();
  interfacesMap_.clear();
}

void qDFDisplayManager::addDisplay(const QString displayName, 
                                   qDFDisplayInterface *theDisplay,
                                   bool enabled)
{
  // only add non-null displays!
  if (theDisplay)
  {
    enabledMap_[displayName] = enabled;
    interfacesMap_[displayName] = theDisplay;
    
    if (enabled)
      theDisplay->initializeDisplay();
  }
}

void qDFDisplayManager::enableDisplay(const QString displayName)
{
  qDFDisplayInterface *theDisplay = interfacesMap_.value(displayName);
  if (theDisplay)
  {
    // if it wasn't already enabled, initialize it
    if (!enabledMap_.value(displayName))
      theDisplay->initializeDisplay();
    enabledMap_[displayName] = true;
  }
}

void qDFDisplayManager::disableDisplay(const QString displayName)
{
  qDFDisplayInterface *theDisplay = interfacesMap_.value(displayName);
  if (theDisplay)
  {
    // if it wasn't already disabled, close it
    if (enabledMap_.value(displayName))
      theDisplay->closeDisplay();
    enabledMap_[displayName] = false;
  }
}

void qDFDisplayManager::initializeDisplays()
{
  foreach (QString theDisplayName, enabledMap_.keys())
  {
    if (enabledMap_[theDisplayName])
    {
      if (interfacesMap_[theDisplayName])
        interfacesMap_[theDisplayName]->initializeDisplay();
    }
  }
}

void qDFDisplayManager::closeDisplays()
{
  foreach (QString theDisplayName, enabledMap_.keys())
  {
    if (enabledMap_[theDisplayName])
    {
      if (interfacesMap_[theDisplayName])
        interfacesMap_[theDisplayName]->closeDisplay();
    }
  }
}

void qDFDisplayManager::clearDisplays()
{
  foreach (QString theDisplayName, enabledMap_.keys())
  {
    if (enabledMap_[theDisplayName])
    {
      if (interfacesMap_[theDisplayName])
        interfacesMap_[theDisplayName]->clearDisplay();
    }
  }
}

void qDFDisplayManager::displayDFReport(const qDFProjReport *theReport)
{
  foreach (QString theDisplayName, enabledMap_.keys())
  {
    if (enabledMap_[theDisplayName])
    {
      if (interfacesMap_[theDisplayName])
        interfacesMap_[theDisplayName]->displayDFReport(theReport);
    }
  }
}

void qDFDisplayManager::undisplayDFReport(const qDFProjReport *theReport)
{
  foreach (QString theDisplayName, enabledMap_.keys())
  {
    if (enabledMap_[theDisplayName])
    {
      if (interfacesMap_[theDisplayName])
        interfacesMap_[theDisplayName]->undisplayDFReport(theReport);
    }
  }
}

void qDFDisplayManager::displayLSFix(DFLib::Proj::Point & LSFixPoint)
{
  foreach (QString theDisplayName, enabledMap_.keys())
  {
    if (enabledMap_[theDisplayName])
    {
      if (interfacesMap_[theDisplayName])
        interfacesMap_[theDisplayName]->displayLSFix(LSFixPoint);
    }
  }
}
void qDFDisplayManager::undisplayLSFix()
{
  foreach (QString theDisplayName, enabledMap_.keys())
  {
    if (enabledMap_[theDisplayName])
    {
      if (interfacesMap_[theDisplayName])
        interfacesMap_[theDisplayName]->undisplayLSFix();
    }
  }
}

void qDFDisplayManager::displayFCAFix(DFLib::Proj::Point & FCAFixPoint, std::vector<double> stddevs)
{
  foreach (QString theDisplayName, enabledMap_.keys())
  {
    if (enabledMap_[theDisplayName])
    {
      if (interfacesMap_[theDisplayName])
        interfacesMap_[theDisplayName]->displayFCAFix(FCAFixPoint,stddevs);
    }
  }
}
void qDFDisplayManager::undisplayFCAFix()
{
  foreach (QString theDisplayName, enabledMap_.keys())
  {
    if (enabledMap_[theDisplayName])
    {
      if (interfacesMap_[theDisplayName])
        interfacesMap_[theDisplayName]->undisplayFCAFix();
    }
  }
}
void qDFDisplayManager::displayMLFix(DFLib::Proj::Point & MLFixPoint, double am2, double bm2, double phi)
{
  foreach (QString theDisplayName, enabledMap_.keys())
  {
    if (enabledMap_[theDisplayName])
    {
      if (interfacesMap_[theDisplayName])
        interfacesMap_[theDisplayName]->displayMLFix(MLFixPoint,am2,bm2,phi);
    }
  }
}
void qDFDisplayManager::undisplayMLFix()
{
  foreach (QString theDisplayName, enabledMap_.keys())
  {
    if (enabledMap_[theDisplayName])
    {
      if (interfacesMap_[theDisplayName])
        interfacesMap_[theDisplayName]->undisplayMLFix();
    }
  }
}
void qDFDisplayManager::displayBPEFix(DFLib::Proj::Point & BPEFixPoint, double am2, double bm2, double phi)
{
  foreach (QString theDisplayName, enabledMap_.keys())
  {
    if (enabledMap_[theDisplayName])
    {
      if (interfacesMap_[theDisplayName])
        interfacesMap_[theDisplayName]->displayBPEFix(BPEFixPoint,am2,bm2,phi);
    }
  }
}
void qDFDisplayManager::undisplayBPEFix()
{
  foreach (QString theDisplayName, enabledMap_.keys())
  {
    if (enabledMap_[theDisplayName])
    {
      if (interfacesMap_[theDisplayName])
        interfacesMap_[theDisplayName]->undisplayBPEFix();
    }
  }
}
