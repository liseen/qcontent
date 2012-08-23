TEMPLATE=app

SOURCES += main.cpp

include($$PWD/../qcontent.pri)

CONFIG += release
LIBS += -lcityhash

QT -= core gui

target.path  = $$INSTALLDIR/bin
INSTALLS += target

