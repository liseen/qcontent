TEMPLATE=lib

SOURCES += qurlfilter.cpp
HEADERS += qurlfilter.h

include($$PWD/../qcontent.pri)

#CONFIG += debug
CONFIG += release
QT -= gui

LIBS += -lgoogleurl -lglog -lmsgpack-rpc -lmsgpack -lqhost2site -lqcontentcommon

headers.files = $$HEADERS
headers.path = $$INSTALLDIR/include/qurlfilter/
target.path  = $$INSTALLDIR/lib

INSTALLS += headers target
