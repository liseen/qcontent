TEMPLATE=app

SOURCES += qimgfilter.cpp

include($$PWD/../qcontent.pri)
CONFIG += release

LIBS += -lprotobuf -lpcre -ltokyotyrant

target.path  = $$INSTALLDIR/bin
INSTALLS += target


