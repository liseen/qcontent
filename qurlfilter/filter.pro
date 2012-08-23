TEMPLATE=app

SOURCES += qurlfilter.cpp main.cpp
HEADERS += qurlfilter.h

include($$PWD/../qcontent.pri)
QMAKE_CFLAGS+=-pg
QMAKE_CXXFLAGS+=-pg
QMAKE_LFLAGS+=-pg

CONFIG += debug
#CONFIG += release
QT -= gui

LIBS += -lgoogleurl -lglog -lmsgpack-rpc -lmsgpack -lqhost2site -lqcontentcommon

headers.files = $$HEADERS
headers.path = $$INSTALLDIR/include/
target.path  = $$INSTALLDIR/lib

INSTALLS += headers target
