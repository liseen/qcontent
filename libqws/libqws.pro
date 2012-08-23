TEMPLATE=lib
TARGET=qws

include($$PWD/../qcontent.pri)
include($$PWD/../qws/qws.pri)


#DEFINES += QWS_DEBUG
#CONFIG += debug

LIBS += -ltcmalloc
CONFIG += release

QT -= gui core


headers.files = $$INSTALL_HEADERS
headers.path = $$INSTALLDIR/include/qws/
target.path  = $$INSTALLDIR/lib

INSTALLS += headers target
