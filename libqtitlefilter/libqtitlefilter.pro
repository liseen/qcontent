TEMPLATE=lib
TARGET=qtitlefilter

SOURCES += ../qtitlefilter/qtitlefilter.cpp
HEADERS += ../qtitlefilter/qtitlefilter.h

#CONFIG += debug
#QT -= core gui

include($$PWD/../qcontent.pri)
CONFIG += release
QT -= gui core


INCLUDEPATH  += /opt/qcontent/include
LIBS += -L/opt/qcontent/lib -lqcontentcommon -lcityhash

headers.files = $$HEADERS
headers.path = $$INSTALLDIR/include/qtitlefilter/
target.path  = $$INSTALLDIR/lib
INSTALLS += headers target
