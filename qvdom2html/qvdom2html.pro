TEMPLATE=app
SOURCES += main.cpp  qvdom2html.cpp
HEADERS += qvdom2html.h

include($$PWD/../qcontent.pri)
LIBS +=  -lglog -licuuc -lgoogleurl -lprotobuf -lvdom -lpcre -lpthread -lqcontentcommon -lmemcached
CONFIG += debug
