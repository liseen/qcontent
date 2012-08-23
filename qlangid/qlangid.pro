TEMPLATE=app

SOURCES += main.cpp  qlangid.cpp crc32.c  utils.c

HEADERS += qlangid.h  udm_common.h  udm_crc32.h  udm_guesser.h  udm_utils.h

include($$PWD/../qcontent.pri)

DEFINES+=UDM_GUESSER_STANDALONE LMDIR=\"./maps\" MGUESSER_VERSION=\"0.3\"

#QMAKE_CFLAGS+=-pg
#QMAKE_CXXFLAGS+=-pg
#QMAKE_LFLAGS+=-pg

#CONFIG += debug
CONFIG += release
QT -= gui

LIBS += -lglog -lqcontentcommon

target.path  = $$INSTALLDIR/bin
INSTALLS += target


