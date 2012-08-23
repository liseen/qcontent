TEMPLATE=app

SOURCES += qws_dict_darts_main.cpp  qws_dict_darts.cpp  qws_util.cpp
HEADERS += qws_common.h  qws_util.h  qws_dict_darts.h  utf8.h darts.h

INCLUDEPATH += /opt/qcontent/include
LIBS += -L/opt/qcontent/lib -ltrie

CONFIG += debug

QT -= gui core
