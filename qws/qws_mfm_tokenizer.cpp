/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

#include "qws_mfm_tokenizer.h"


BEGIN_NAMESPACE_QWS

QWSMFMTokenizer::QWSMFMTokenizer()
{
    m_word_dict = new QWSDict();
}

QWSMFMTokenizer::~QWSMFMTokenizer()
{
    if (m_word_dict) {
        delete m_word_dict;
    }
}

QWSTokenizer::TokenizerType QWSMFMTokenizer::type()
{
    return MFM;
}

bool QWSMFMTokenizer::init(QWSConfig *config)
{
    std::string word_dict_path = "mfm_word.dict";
    config->get_string("qws.mfm_word_dict", word_dict_path);

    m_word_dict->set_path(word_dict_path);
    if (!m_word_dict->load()) {
        fprintf(stderr, "load mfm key dict error: %s\n", word_dict_path.c_str());
        return false;
    }

    max_word_length = 6;
    return true;
}

void QWSMFMTokenizer::tokenize_sentence(
        const std::string &normalized,
        QWSSymbolList &symbols,
        int symbol_start, int symbol_end)
{
    const char *sentence_base = normalized.c_str();
    const char *base;
    size_t node_pos = 0, key_pos = 0;
    int value = 0;

    for (int i = symbol_start; i < symbol_end; i++) {
        int token_offset = i;

        QWSSymbol &symbol = symbols[i];
        base = sentence_base + symbol.normalized_start;
        node_pos = key_pos = 0;
        value = 0;
        for (int j = i; j < symbol_end; ++j) {
            QDD("i : %d, j: %d node_pos: %d key_pos:%d length: %d\n", i, j, \
                    static_cast<int>(node_pos), static_cast<int>(key_pos), \
                    symbols[j].normalized_end - symbol.normalized_start);

            value = m_word_dict->traverse(base, node_pos, key_pos,
                        symbols[j].normalized_end - symbol.normalized_start);

            QDD("value: %d\n", value);

            if (value >= 0) {
                token_offset = j;
            } else if (value == -1) {
                // continue;
            } else if (value == -2) {
                symbols[token_offset].token_position = 1;
                i = token_offset;
                break;
            } else {
                assert(false);
            }

            if (j == symbol_end - 1 && value >= -1) {
                symbols[token_offset].token_position = 1;
                i = token_offset;
            }
        }
    }
}

END_NAMESPACE_QWS
