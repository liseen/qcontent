TEMPLATE=app

SOURCES += main.cpp

include($$PWD/../qcontent.pri)

CONFIG += release
QT -= gui

LIBS += -lgoogleurl -lglog -lmsgpack-rpc -lmsgpack -lqhost2site -lqcontenthub -lmpio

target.path  = $$INSTALLDIR/bin

INSTALLS += target
