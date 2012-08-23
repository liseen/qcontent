TEMPLATE=app

SOURCES += main.cpp

include($$PWD/../qcontent.pri)

CONFIG += release
QT -= gui

LIBS += -lgoogleurl -lglog

target.path  = $$INSTALLDIR/bin
INSTALLS += target

