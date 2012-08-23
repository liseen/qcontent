TEMPLATE=lib
TARGET=qcontenthub

SOURCES += libqcontenthub.cpp libqcontentqueue.cpp
HEADERS += libqcontenthub.h libqcontentqueue.h

include($$PWD/../qcontent.pri)

LIBS += -lmsgpack-rpc -lmsgpack

CONFIG += release
QT -= gui

headers.files = $$HEADERS
headers.path = $$INSTALLDIR/include/
target.path  = $$INSTALLDIR/lib

INSTALLS += headers target
