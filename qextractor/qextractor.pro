TEMPLATE = app

SOURCES += qextractor.cpp main.cpp
HEADERS += qextractor.h

include($$PWD/../qcontent.pri)

LIBS += -lglog -licuuc -lgoogleurl -lprotobuf -lmsgpack-rpc -lmsgpack -lmpio -ltokyotyrant -ltokyocabinet -lz -lbz2 -lresolv -lnsl -ldl -lrt -lpthread -lm -lc -lvdom -lqcontenthub -lqurlqueue -lqcontentextractor -lqhost2site -lqhtml2vdom -lqstore -lqcontentcommon -lqurlfilter -lqthumbscale -lqpredict -lqws -lqtitlefilter

CONFIG += release

QT += network webkit testlib

target.path  = $$INSTALLDIR/bin

INSTALLS += target

