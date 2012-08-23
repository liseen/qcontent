TEMPLATE=app

SOURCES += qdedup_server.cpp qdedup_backend.cpp server_main.cpp

include($$PWD/../../qcontent.pri)
CONFIG += release
#CONFIG += debug
QT -= gui core

LIBS += -lgoogleurl -lglog  -lqcontentcommon   -lmsgpack-rpc -lrt -lmpio -lmsgpack

target.path  = $$INSTALLDIR/bin
INSTALLS += target

headers.path = $$INSTALLDIR/include
headers.files = qdedup_server.h ../include/qdedup_common.h qdedup_backend.h ../include/qdedup_constant.h

INCLUDEPATH += ../include/
#HEADERS += city.h

