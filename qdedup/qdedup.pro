TEMPLATE=app

SOURCES += main.cpp

include($$PWD/../qcontent.pri)
CONFIG += release
QT -= gui core

LIBS += -lgoogleurl -lglog -lqhost2site -lqcontentcommon

target.path  = $$INSTALLDIR/bin
INSTALLS += target

