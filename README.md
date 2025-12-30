# qDF: a GUI wrapper to DFLib

qDF allows a user to enter data from radio direction finding activity,
computes target location estimates using
[DFLib](https://github.com/tvrusso/DFLib) (using 4 different
estimators), and creates a KML file for plotting the activity in
Google Earth.  It also optionally feeds data to an APRS client via UDP
datagrams (only Xastir has been tested with this capability, as that's
all I use), which can then transmit the data to the APRS RF network or
an APRS-IS server.

## Prerequisites
  1. proj libraries and development headers (version 6 or later)
  1. Qt6 libraries, headers, and development programs (qmake, moc, etc.)
  1. [DFLib](https://github.com/tvrusso/DFLib)

## Building

Assuming all dependencies have been installed properly, building qDF
requires only a few steps

```
mkdir build
cd build
qmake /path/to/qDF/sources/qDF.pro
make
```

On some systems, qmake might be called "qmake-qt6" or "qmake6".

## Running

Execute the "qDF" program created in the previous step.  It will pop
up a main window with a menu bar.

### Set the preferences

On your first run, choose "Edit->Settings..." and fill in what you
need here.  If you're going to be using APRS display, set your call
sign, the IP address (or DNS name) of the APRS client's server port,
and the port number in the "APRS" tab.  If you're not using APRS,
unclick the "Publish to APRS" checkbox.

Set the magnetic declination, UTM zone, and preferred coordinate
system in the "Geographic Settings" tab.  You can also change your
default hemisphere if you're not in North America.  The most important
setting here is the default declination, as qDF always treats bearings
you enter for DF reports as magnetic bearings, and converts them to
true bearings using this setting.  All internal calculations are done
using the computed true bearing --- if you don't get this setting
right, your reports will all be misinterpreted.

You can leave "DF Fix settings" alone for now.

These settings are saved across runs.

### Enter DF reports.

Use the "Report->New Report" menu entry to enter DF reports as they come in.

Give your report a unique name.  qDF will keep the "Ok" button greyed
out if you try to enter the name of an existing report here, and keep
it greyed out until the name is made unique.

Enter the latitude and longitude of the receiver position.  This can
be done in one of three formats: decimal degrees, degrees/decimal
minute, degrees/minutes/decimal seconds.  As soon as you enter a
decimal in the deg, min, or sec box, the smaller unit box is greyed
out.

Enter the MAGNETIC bearing reported by the DFer directly off of their
magnetic compass without declination correction.  qDF will use your
default magnetic declination (from the settings dialog) to convert
this internally to true bearing.

Enter equipment type: Beam (i.e Yagi), Elper (i.e. L-tronics "L'il
L-per"), Interferometer, or Long Baseline Average.

Enter report quality: OK, Good, Very Good.  This is a rough,
subjective measure of the precision of the bearing taken.  If the DFer
thinks that his equipment gives him high confidence that the bearing
he reports really is the bearing to the transmitter, that's "Very
Good".  If the DFer feels that the signal is so weak, or that he had
trouble pinning down the exact direction, that's "OK."  Somewhere in
between is "Good."  These report qualities, combined with the
equipment type, are mapped to bearing error standard deviations and
used in the statistical estimators (Stansfield and Maximum
Likelihood).

### Watch the fixes

As soon as two or more reports are entered, target location estimates
start showing up in the main window, and start being written to the
KML and APRS displays.

If you want to look at the KML in Google Earth, open the file
"qDF_GE.kml".  This is a wrapper file that tells Google Earth to
periodically re-read the real data, which is in "qDF_GE_data.kml".
That way your Google Earth display is kept continuously updated as qDF
writes out more data.

qDF writes out the following information to APRS and KML displays:
  1. receiver locations for each report
  1. bearing lines from receivers
  1. estimated target location for each estimator
  1. 50%, 75%, and 95% confidence regions for the Stansfield and Maximum Likelihood estimators

