TEMPLATE=lib

SOURCES=qhost2site.cpp
HEADERS=qhost2site.h

include($$PWD/../qcontent.pri)

CONFIG += release
QT -= gui core


headers.files = $$HEADERS
headers.path = $$INSTALLDIR/include/qhost2site/

target.path  = $$INSTALLDIR/lib

INSTALLS += headers target

