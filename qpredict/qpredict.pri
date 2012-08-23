
SOURCES += \
            ../qpredict/qpredict.cpp \
            ../qpredict/qpredict_config.cpp \
            ../qpredict/qpredict_document.cpp \
            ../qpredict/qpredict_tokenizer.cpp \
            ../qpredict/qpredict_dict.cpp \
            ../qpredict/qpredict_linear_learner.cpp \
            ../qpredict/liblinear/linear.cpp \
            ../qpredict/liblinear/tron.cpp  \
            ../qpredict/liblinear/blas/daxpy.c \
            ../qpredict/liblinear/blas/ddot.c \
            ../qpredict/liblinear/blas/dnrm2.c \
            ../qpredict/liblinear/blas/dscal.c

INSTALL_HEADERS += \
            ../qpredict/qpredict_define.h \
            ../qpredict/qpredict_config.h \
            ../qpredict/qpredict_document.h  \
            ../qpredict/qpredict.h  \
            ../qpredict/qpredict_log.h \
            ../qpredict/qpredict_term.h \
            ../qpredict/qpredict_feature.h \
            ../qpredict/qpredict_dict.h \
            ../qpredict/qpredict_tokenizer.h \
            ../qpredict/qpredict_learner.h \
            ../qpredict/qpredict_linear_learner.h


HEADERS +=  ../qpredict/utf8/checked.h \
            ../qpredict/utf8/core.h \
            ../qpredict/utf8.h \
            ../qpredict/liblinear/linear.h \
            ../qpredict/liblinear/tron.h \
            ../qpredict/liblinear/blas/blas.h \
            ../qpredict/liblinear/blas/blasp.h \
            $$INSTALL_HEADERS


INCLUDEPATH += ../qpredict \
               ../qpredict/liblinear \
               ../qpredict/liblinear/blas/

LIBS += -lpcre -lqws
