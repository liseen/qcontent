TEMPLATE = app

TARGET=qcontenthubd

SOURCES += qcontenthub_rpc.cpp  main.cpp
HEADERS += qcontenthub_rpc.h qcontenthub.h

#QMAKE_CFLAGS+=-p -pg
#QMAKE_CXXFLAGS+=-p -pg
#QMAKE_LFLAGS+=-p -pg

#CONFIG += debug

CONFIG += release
QT -= gui core

LIBS = -lmsgpack-rpc -lrt

INSTALLDIR=/opt/qcontent/

INCLUDEPATH += $$INSTALLDIR/include
LIBS += -L$$INSTALLDIR/lib  -ltcmalloc -lmpio

headers.files = qcontenthub.h
headers.path = $$INSTALLDIR/include
target.path  = $$INSTALLDIR/bin

INSTALLS += headers target


