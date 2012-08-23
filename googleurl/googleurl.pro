TEMPLATE=lib

SOURCES +=\
    ../googleurl/src/gurl.cc\
    ../googleurl/src/url_canon_internal.cc\
    ../googleurl/src/url_canon_stdurl.cc\
    ../googleurl/src/url_canon_ip.cc\
    ../googleurl/src/url_canon_mailtourl.cc\
    ../googleurl/src/url_parse.cc\
    ../googleurl/src/url_canon_etc.cc\
    ../googleurl/src/url_canon_path.cc\
    ../googleurl/src/url_parse_file.cc\
    ../googleurl/src/url_canon_fileurl.cc\
    ../googleurl/src/url_canon_pathurl.cc\
    ../googleurl/src/url_canon_host.cc\
    ../googleurl/src/url_canon_query.cc\
    ../googleurl/src/url_util.cc\
    ../googleurl/src/url_canon_relative.cc\
    ../googleurl/src/url_canon_icu.cc\
    ../base/string16.cc

HEADERS +=\
    ../googleurl/src/gurl.h\
    ../googleurl/src/url_canon.h\
    ../googleurl/src/url_canon_icu.h\
    ../googleurl/src/url_canon_internal_file.h\
    ../googleurl/src/url_canon_internal.h\
    ../googleurl/src/url_canon_ip.h\
    ../googleurl/src/url_canon_stdstring.h\
    ../googleurl/src/url_file.h\
    ../googleurl/src/url_parse.h\
    ../googleurl/src/url_parse_internal.h\
    ../googleurl/src/url_util.h

include($$PWD/../qcontent.pri)

CONFIG += release
QT -= gui core

INCLUDEPATH = ../ \
            $$INCLUDEPATH

LIBS += -lglog -licuuc

headers.files = $$HEADERS
headers.path = $$INSTALLDIR/include/googleurl/src/

target.path  = $$INSTALLDIR/lib

INSTALLS += target headers
