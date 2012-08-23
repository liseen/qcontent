
SRC_DIR=$$PWD/../qws/
HEADER_DIR=$$PWD/../qws/

SOURCES += $$SRC_DIR/qws_crf_tokenizer.cpp   $$SRC_DIR/qws_util.cpp $$SRC_DIR/qws_config.cpp \
           $$SRC_DIR/qws_mfm_tokenizer.cpp   $$SRC_DIR/qws_viterbi_tokenizer.cpp \
           $$SRC_DIR/qws.cpp  $$SRC_DIR/qws_dict_darts.cpp  $$SRC_DIR/qws_tokenizer.cpp

INSTALL_HEADERS += $$HEADER_DIR/qws_tokenizer.h $$HEADER_DIR/qws_common.h  $$HEADER_DIR/qws.h \
           $$HEADER_DIR/qws_symbol.h $$HEADER_DIR/qws_token.h $$HEADER_DIR/qws_util.h $$HEADER_DIR/qws_config.h

HEADERS += $$HEADER_DIR/darts.h $$HEADER_DIR/qws_crf_tokenizer.h $$HEADER_DIR/qws_normalize.h  \
           $$HEADER_DIR/qws_dict_darts.h  $$HEADER_DIR/utf8.h \
           $$HEADER_DIR/qws_mfm_tokenizer.h $$HEADER_DIR/qws_viterbi_tokenizer.h \
           $$INSTALL_HEADERS



