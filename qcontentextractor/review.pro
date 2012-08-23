TEMPLATE = app
SOURCES += review.cpp  vdom_content_extractor.cpp vdom_content_util.cpp
HEADERS += vdom_content_block.h  vdom_content_extractor.h  vdom_content_result.h vdom_content_page.h

#QMAKE_CFLAGS+=-pg
#QMAKE_CXXFLAGS+=-pg
#QMAKE_LFLAGS+=-pg

DEFINES += NDEBUG
CONFIG += release
QT -= gui

include($$PWD/../qcontent.pri)

LIBS +=  -lglog -licuuc -lgoogleurl -lprotobuf -lvdom -lpcre -lpthread -lqcontentcommon
LIBS += -lprotobuf -lpcre -ltokyotyrant -lqhost2site -lqurlfilter


CONFIG += release
QT -= gui

