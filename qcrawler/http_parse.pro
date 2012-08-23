TEMPLATE=app
SOURCES += http_parse.cpp qcrawler_http_parse.cpp
HEADERS += qcrawler_http_parse.h

include($$PWD/../qcontent.pri)
LIBS += -lglog -lz

#DEFINES += TEST_PARSE

CONFIG += debug
QT -= gui core
target.path  = $$INSTALLDIR/bin
