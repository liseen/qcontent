TEMPLATE=app

include($$PWD/../qcontent.pri)

include($$PWD/qpredict.pri)

SOURCES +=  main.cpp

LIBS +=-lqcontentcommon -lglog -licuuc -lqws

#CONFIG += debug
CONFIG += release

QT -= core gui

