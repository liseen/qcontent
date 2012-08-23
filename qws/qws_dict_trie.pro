TEMPLATE=app

SOURCES += qws_dict_trie_main.cpp  qws_dict_trie.cpp  qws_util.cpp
HEADERS += qws_common.h  qws_util.h  qws_dict_trie.h  utf8.h

INCLUDEPATH += /opt/qcontent/include
LIBS += -L/opt/qcontent/lib -ltrie

CONFIG += debug

QT -= gui core
