TEMPLATE=app

SOURCES += qws_dict_main.cpp  qws_dict.cpp  qws_util.cpp
HEADERS += qws_common.h  qws_util.h  qws_dict.h  utf8.h

INCLUDEPATH += /opt/qcontent/include
LIBS += -L/opt/qcontent/lib -ltrie

CONFIG += debug

QT -= gui core
