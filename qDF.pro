######################################################################
# Automatically generated by qmake (2.01a) Tue Apr 21 22:09:14 2009
######################################################################

TEMPLATE = app
TARGET = 
DEPENDPATH += .
INCLUDEPATH += .

QT += network

# Input
HEADERS += CoordSysBuilder.hpp \
           Settings.hpp \
           APRS.hpp \
           getReportDialog.h \
           reportToggleDialog.h \
           MainWindow.h \
           qDFProjReport.hpp \
           qDFProjReportCollection.hpp
FORMS += getReportDialog.ui MainWindow.ui reportToggleDialog.ui
SOURCES += CoordSysBuilder.cpp \
           Settings.cpp \
           APRS.cpp \
           getReportDialog.cpp \
           reportToggleDialog.cpp \
           main.cpp \
           MainWindow.cpp \
           qDFProjReport.cpp \
           qDFProjReportCollection.cpp

LIBS += -L/home/russo/src/DFLib -lDFLib -lproj
INCLUDEPATH += /home/russo/src/DFLib

