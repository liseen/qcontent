TEMPLATE=app

SOURCES += main.cpp

include($$PWD/../qcontent.pri)
CONFIG += release
QT -= gui

LIBS += -lgoogleurl -lglog -lqhost2site

target.path  = $$INSTALLDIR/bin
INSTALLS += target

