TEMPLATE = app

SOURCES += main.cpp  vdom_content_extractor.cpp vdom_content_util.cpp
HEADERS += vdom_content_block.h  vdom_content_extractor.h  vdom_content_result.h

include($$PWD/../qcontent.pri)

DESTDIR=$$PWD

DEFINES += CONTENT_EXTRACTOR_DEBUG

LIBS += -lglog -licuuc -lgoogleurl -lprotobuf -lvdom -lpcre -lpthread -lqcontentcommon -ljson -lqtitlefilter
LIBS += -lprotobuf -lpcre -ltokyotyrant

QT -= gui

CONFIG += debug
