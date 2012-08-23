TEMPLATE=app
SOURCES += main.cpp qcrawler_dns.cpp qcrawler.cpp qcrawler_http_parse.cpp qcrawler_http.cpp
HEADERS += qcrawler_dns.h qcrawler_uri.h qcrawler.h qcrawler_http_parse.h qcrawler_http.h

DEFINES += HAVE_CONFIG_H

SOURCES += \
    udns-0.1/udns_dn.c\
    udns-0.1/udns_dntosp.c\
    udns-0.1/udns_parse.c\
    udns-0.1/udns_resolver.c\
    udns-0.1/udns_init.c\
    udns-0.1/udns_misc.c\
    udns-0.1/udns_XtoX.c\
    udns-0.1/udns_rr_a.c\
    udns-0.1/udns_rr_ptr.c\
    udns-0.1/udns_rr_mx.c\
    udns-0.1/udns_rr_txt.c\
    udns-0.1/udns_bl.c\
    udns-0.1/udns_rr_srv.c\
    udns-0.1/udns_rr_naptr.c\
    udns-0.1/udns_codes.c\
    udns-0.1/udns_jran.c

include($$PWD/../qcontent.pri)

LIBS += -lev -L/opt/qcontent/lib/-lglog -licuuc -lgoogleurl -lqcontenthub -lqurlqueue -lqcontentcommon -lglog -lmsgpack -lmsgpack-rpc -lqstore -ltokyotyrant -ltokyocabinet -lqurlfilter -lmpio -lz

INCLUDEPATH += ./udns-0.1
#LIBS += -Ludns-0.1 -ludns

QT -= gui

DEFINES += NDEBUG
#CONFIG += debug
CONFIG += realease

target.path  = $$INSTALLDIR/bin

INSTALLS += target

