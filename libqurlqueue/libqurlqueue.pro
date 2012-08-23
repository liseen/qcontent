TEMPLATE=lib
TARGET=qurlqueue

SOURCES += libqurlqueue.cpp
HEADERS += libqurlqueue.h

include($$PWD/../qcontent.pri)

LIBS += -lmsgpack-rpc -lmsgpack

CONFIG += release
QT -= gui

headers.files = $$HEADERS
headers.path = $$INSTALLDIR/include/qurlqueue/
target.path  = $$INSTALLDIR/lib

INSTALLS += headers target
