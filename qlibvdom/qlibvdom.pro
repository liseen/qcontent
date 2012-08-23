TEMPLATE=lib
TARGET=vdom

VERSION = 1.0.0

SOURCES += vdom.cpp
HEADERS += vdom.h

include($$PWD/../qcontent.pri)

CONFIG += release
QT -= gui core

INCLUDEPATH = $$INSTALLDIR/include
LIBS += -L$$INSTALLDIR/lib -lprotobuf

headers.files = $$HEADERS
headers.path = $$INSTALLDIR/include

target.path  = $$INSTALLDIR/lib

INSTALLS += headers target

