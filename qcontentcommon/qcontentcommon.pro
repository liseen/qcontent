TEMPLATE=lib

SOURCES += qcontent_config.cpp
HEADERS += qcontent_config.h  qcontent_processor.h  qcontent_record.h  qcrawler_processor.h strtk.hpp

include($$PWD/../qcontent.pri)

LIBS += -lmsgpack

CONFIG += release
QT -= gui core

headers.files = $$HEADERS
headers.path = $$INSTALLDIR/include/qcontentcommon/

target.path  = $$INSTALLDIR/lib

INSTALLS += headers target

