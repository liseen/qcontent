TEMPLATE=app

SOURCES += qfetcher.cpp main.cpp
HEADERS += qfetcher.h

include($$PWD/../qcontent.pri)

#QMAKE_CFLAGS+=-pg
#QMAKE_CXXFLAGS+=-pg
#QMAKE_LFLAGS+=-pg
#CONFIG += debug

LIBS += -lglog -licuuc -lgoogleurl -lqcontenthub -lqurlqueue -lqcontentcommon -lglog -lmsgpack -lmsgpack-rpc -ltcmalloc -lqstore -ltokyotyrant -ltokyocabinet

CONFIG += release

QT += network
QT -= gui

target.path  = $$INSTALLDIR/bin

INSTALLS += target

