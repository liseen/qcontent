
SOURCES += qdedup_client.cpp client_main.cpp

include($$PWD/../../qcontent.pri)
CONFIG += release
#CONFIG += debug
QT -= gui core
INCLUDEPATH += ../include/

LIBS += -lgoogleurl -lglog -lqhost2site -lqcontentcommon  -lcityhash -lmsgpack-rpc -lrt -lmsgpack -lmpio

target.path  = $$INSTALLDIR/bin
INSTALLS += target

headers.path = $$INSTALLDIR/include
headers.files = ../include/qdedup_common.h  qdedup_client.h ../include/qdedup_constant.h
#INCLUDEPATH += /opt/qcontent/include/
#HEADERS += city.h

