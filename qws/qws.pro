TEMPLATE=app

include($$PWD/../qcontent.pri)

#INSTALLDIR=/opt/qcontent/
#INCLUDEPATH += $$INSTALLDIR/include/ ../
#LIBS += -L$$INSTALLDIR/lib/  -Wl,-rpath,$$INSTALLDIR/lib/

include($$PWD/qws.pri)

SOURCES += main.cpp


#QMAKE_CFLAGS+=-pg
#QMAKE_CXXFLAGS+=-pg
#QMAKE_LFLAGS+=-pg

#CONFIG += debug
#DEFINES += QWS_DEBUG

LIBS += -ltcmalloc
CONFIG += release

QT -= gui core
