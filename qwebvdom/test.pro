TEMPLATE = app
SOURCES +=\
    test.cpp

HEADERS += qwebvdom.h

INCLUDEPATH  = /opt/qcontent/include
LIBS = -L /opt/qcontent/lib -L. -lvdom -lprotobuf
QT += webkit
