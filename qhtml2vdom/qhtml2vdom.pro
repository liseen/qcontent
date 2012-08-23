TEMPLATE=lib

SOURCES += qhtml2vdom.cpp   qhtml2vdom_webpage.cpp
HEADERS += qhtml2vdom_eventloop.h  qhtml2vdom.h  qhtml2vdom_webpage.h

include($$PWD/../qcontent.pri)

INCLUDEPATH += ../qwebvdom
LIBS += -lvdom

CONFIG += release
QT += network webkit testlib

headers.files = $$HEADERS
headers.path = $$INSTALLDIR/include/qhtml2vdom/

target.path  = $$INSTALLDIR/lib

INSTALLS += headers target

