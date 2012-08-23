/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

#include "qws.h"

#include <cstring>
#include <cassert>

#include "utf8.h"
#include "qws_util.h"
#include "qws_normalize.h"

BEGIN_NAMESPACE_QWS

QWS::QWS(): tokenizer(NULL), normalize_method(0), output_normalized(false)
{
}

QWS::~QWS()
{
}

bool QWS::init(QWSConfig *config)
{
    QWSTokenizer::TokenizerType type;
    std::string tokenizer_type;
    config->get_string("qws.tokenizer_type", tokenizer_type);
    if (tokenizer_type == "mfm") {
        type = QWSTokenizer::MFM;
    } else if (tokenizer_type == "viterbi") {
        type = QWSTokenizer::VITERBI;
    } else {
        fprintf(stderr, "init, no tokenizer type %s, use default type viterbi\n"
                , tokenizer_type.c_str());
        tokenizer_type = "viterbi";
        type = QWSTokenizer::VITERBI;
    }

    config->get_boolean("qws.output_normalized", output_normalized);

    tokenizer = QWSTokenizer::create_tokenizer(type);
    if (!tokenizer->init(config)) {
        fprintf(stderr, "init tokenizer %s error\n", tokenizer_type.c_str());
        return false;
    }

    normalize_method = QWSNormalize::DBC_TO_SBC | QWSNormalize::TO_LOWER;

    return true;
}

void QWS::parse(const std::string &text, std::string &normalized_text, QWSSymbolList &symbols)
{
    // from utf8 to utf16
    // normalize to symbol
    const char *w = text.c_str();
    const char *prev_w = w;
    const char *end = text.c_str() + text.size();
    normalized_text.reserve(text.size());
    int symbol_offset = 0;
    while (w != end) {
        prev_w = w;
        //int cp = utf8::unchecked::next(w, end);
        int val = utf8::unchecked::next(w);
        if (val > 0) {
            if (val <= 0xFFFF) {
                QWSSymbol symbol;
                memset(&symbol, 0, sizeof(symbol));
                symbol.origin_start = prev_w - text.c_str();
                symbol.origin_end = w - text.c_str();
                symbol.offset = symbol_offset;
                symbol.origin_value = val;

                symbol.normalized_start = normalized_text.size();
                QWSNormalize::normalize(prev_w, w - prev_w, val, normalize_method, &normalized_text, &symbol.value);
                symbol.normalized_end = normalized_text.size();
                symbols.push_back(symbol);
            }
            ++symbol_offset;
        } else {
            assert(false);
        }
    }
}

void QWS::tokenize(const std::string &text, QWSTokenList &tokens)
{
    // normalize to symbol
    std::string normalized_text;
    QWSSymbolList symbols;
    symbols.reserve(text.size());

    // parse
    parse(text, normalized_text, symbols);

    // tokenize
    tokenizer->tokenize(normalized_text, symbols);

    int symbol_size = symbols.size();

#ifdef QWS_DEBUG
    for (int i = 0; i < symbol_size; ++i) {
        symbols[i].display(text, normalized_text);
    }
#endif

    // output
    int token_start = 0;
    for (int i = 0; i < symbol_size; ++i) {
        QWSSymbol &symbol = symbols[i];
        if (symbol.token_position) {
            QWSToken token;
            if (output_normalized) {
                token.value.append(normalized_text.c_str() + token_start, symbol.normalized_end - token_start);
                token_start = symbol.normalized_end;
            } else {
                token.value.append(text.c_str() + token_start, symbol.origin_end - token_start);
                token_start = symbol.origin_end;
            }
            tokens.push_back(token);
        }
    }

}

END_NAMESPACE_QWS
