TEMPLATE = lib

SOURCES += vdom_content_extractor.cpp vdom_content_util.cpp
HEADERS += vdom_content_block.h  vdom_content_extractor.h  vdom_content_result.h vdom_content_util.h vdom_content_page.h

include($$PWD/../qcontent.pri)

LIBS +=  -lglog -licuuc -lgoogleurl -lprotobuf -lvdom -lpcre -lpthread -lqcontentcommon -ljson -lqtitlefilter

DEFINES += NDEBUG
CONFIG += release
QT -= gui


headers.files = $$HEADERS
headers.path = $$INSTALLDIR/include/qcontentextractor/

target.path  = $$INSTALLDIR/lib

INSTALLS += headers target

