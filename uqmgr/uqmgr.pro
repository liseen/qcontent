TEMPLATE=app
TARGET=uqmgr

SOURCES += main.cpp

include($$PWD/../qcontent.pri)

CONFIG += release
QT -= gui

LIBS += -lgoogleurl -lglog -lmsgpack-rpc -lmsgpack -lqhost2site -lqcontenthub -lqurlqueue -lmpio

target.path  = $$INSTALLDIR/bin

INSTALLS += target
