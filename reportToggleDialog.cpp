#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif

#include "reportToggleDialog.h"

reportToggleDialog::reportToggleDialog(QWidget *parent)
  :QDialog(parent)
{
  setupUi(this);
}
