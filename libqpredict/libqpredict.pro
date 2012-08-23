TEMPLATE=lib
TARGET = qpredict

include($$PWD/../qcontent.pri)
include($$PWD/../qpredict/qpredict.pri)

LIBS += -lqcontentcommon -lglog -licuuc

CONFIG += release
QT -= core gui

headers.files = $$INSTALL_HEADERS
headers.path = $$INSTALLDIR/include/qpredict/
target.path  = $$INSTALLDIR/lib

INSTALLS += headers target



