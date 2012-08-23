TEMPLATE = app
SOURCES += qurluniqmgr.cpp
include($$PWD/../qcontent.pri)

LIBS += -lmemcached

CONFIG += release

QT += network
QT -= gui

target.path  = $$INSTALLDIR/bin

INSTALLS += target

