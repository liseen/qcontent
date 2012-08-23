/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

#ifndef QCONTENT_QWS_TOKENIZE_H
#define QCONTENT_QWS_TOKENIZE_H

#include "qws_common.h"
#include "qws_config.h"
#include "qws_symbol.h"

BEGIN_NAMESPACE_QWS

class QWSTokenizer {
public:
    enum TokenizerType {
        UNIGRAM = 1,
        MFM,
        VITERBI,
        CRF
    };

    QWSTokenizer();
    virtual ~QWSTokenizer();

    virtual TokenizerType type();

    virtual bool init(QWSConfig *config);
    virtual void tokenize(const std::string &normalized_text, QWSSymbolList &symbols);

    virtual void tokenize_sentence(
            const std::string &normalized,
            QWSSymbolList &symbols,
            int symbol_start, int symbol_end);

    static QWSTokenizer* create_tokenizer(const TokenizerType &type);
protected:
    inline bool is_english_word(QWSSymbolList &symbols, int symbol_start, int symbol_end) {
        for (int i = symbol_start; i < symbol_end; ++i) {
            if (symbols[i].value > 127) {
                return false;
            }
        }
        return true;
    }
};

END_NAMESPACE_QWS

#endif
