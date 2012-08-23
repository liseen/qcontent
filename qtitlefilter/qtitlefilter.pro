TEMPLATE=app
SOURCES += qtitlefilter.cpp main.cpp
HEADERS += qtitlefilter.h

#CONFIG += debug
#QT -= core gui

include($$PWD/../qcontent.pri)
CONFIG += release
QT -= gui core


LIBS += -L/opt/qcontent/lib -lqcontentcommon -lcityhash

target.path  = $$INSTALLDIR/bin
INSTALLS += target

