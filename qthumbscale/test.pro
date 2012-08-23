TEMPLATE=app
TARGET=qthumbscale
SOURCES += qthumbscale.cpp main.cpp
HEADERS += qthumbscale.h

include($$PWD/../qcontent.pri)

#INCLUDEPATH +=
LIBS += -lglog -lpcre -ltokyotyrant -lopencv_core -lopencv_flann -lopencv_imgproc -lopencv_highgui -lopencv_ml -lopencv_video -lopencv_objdetect -lopencv_features2d -lopencv_calib3d -lopencv_legacy -lopencv_contrib

CONFIG += release
QT -= gui

target.path  = $$INSTALLDIR/bin

INSTALLS += target


