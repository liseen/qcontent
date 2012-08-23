TEMPLATE=lib

SOURCES +=  qstore.cpp
HEADERS += qstore.h

include($$PWD/../qcontent.pri)

LIBS += -lmemcached
CONFIG += release
QT -= gui core

headers.files = $$HEADERS
headers.path = $$INSTALLDIR/include/qstore/

target.path  = $$INSTALLDIR/lib

INSTALLS += headers target

