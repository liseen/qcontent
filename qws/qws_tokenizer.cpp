/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

#include "qws_tokenizer.h"
#include "qws_mfm_tokenizer.h"
#include "qws_viterbi_tokenizer.h"

BEGIN_NAMESPACE_QWS

QWSTokenizer* QWSTokenizer::create_tokenizer(const TokenizerType &type) {
    switch (type) {
        case UNIGRAM:
            return new QWSTokenizer();
        case MFM:
            return new QWSMFMTokenizer();
        case VITERBI:
            return new QWSVITERBITokenizer();
        case CRF:
        default:
            return NULL;
    }
}


QWSTokenizer::QWSTokenizer()
{
}

QWSTokenizer::~QWSTokenizer()
{

}

bool QWSTokenizer::init(QWSConfig * /*config*/)
{
    return true;
}

QWSTokenizer::TokenizerType QWSTokenizer::type()
{
    return UNIGRAM;
}

void QWSTokenizer::tokenize(const std::string &normalized_text, QWSSymbolList &symbols)
{
    int symbol_start = 0;

    bool pre_is_english_symbol = false;
    int symbol_size =  symbols.size();
    for (int i = 0; i < symbol_size; ++i) {
        QWSSymbol &symbol = symbols[i];

        if (QWSUtil::is_sentence_separator(symbol.value)) {
            if (pre_is_english_symbol) {
                symbols[i - 1].token_position = 1;
            }
            pre_is_english_symbol = false;

            symbols[i].token_position = 1;
            if (i > 0) {
                symbols[i-1].token_position = 1;
            }
            if (i > symbol_start + 1) {
                if (!is_english_word(symbols, symbol_start, i)) {
                    tokenize_sentence(normalized_text, symbols, symbol_start, i);
                }
            }
            symbol_start = i + 1;
        } else if (QWSUtil::is_english_symbol(symbol.value)) {
            if (!pre_is_english_symbol) {
                if (i > 0) {
                    symbols[i-1].token_position = 1;
                }
                if (i > symbol_start + 1) {
                    if (!is_english_word(symbols, symbol_start, i)) {
                        tokenize_sentence(normalized_text, symbols, symbol_start, i);
                    }
                }
                symbol_start = i + 1;
            }
            pre_is_english_symbol = true;
        } else {
            if (pre_is_english_symbol) {
                symbols[i - 1].token_position = 1;
            }
            pre_is_english_symbol = false;
        }
    }

    // last sentence without separator
    if (symbol_start < symbol_size) {
        symbols[symbol_size-1].token_position = 1;
        if (!is_english_word(symbols, symbol_start, symbol_size)) {
            tokenize_sentence(normalized_text, symbols, symbol_start, symbol_size);
        }
    }
}

void QWSTokenizer::tokenize_sentence(
            const std::string & /*normalized*/ ,
            QWSSymbolList &symbols,
            int symbol_start, int symbol_end)
{
    for (int i = symbol_start; i < symbol_end; ++i) {
        symbols[i].token_position = 1;
    }
}


END_NAMESPACE_QWS
