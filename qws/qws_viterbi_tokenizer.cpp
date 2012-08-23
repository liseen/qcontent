/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

#include "qws_viterbi_tokenizer.h"

#include <cmath>
#include <limits>

BEGIN_NAMESPACE_QWS

QWSVITERBITokenizer::QWSVITERBITokenizer()
{
    m_word_dict = new QWSDict();
}

QWSVITERBITokenizer::~QWSVITERBITokenizer()
{
    if (m_word_dict) {
        delete m_word_dict;
    }
}

QWSTokenizer::TokenizerType QWSVITERBITokenizer::type()
{
    return VITERBI;
}

bool QWSVITERBITokenizer::init(QWSConfig *config)
{
    std::string word_dict_path = "viterbi_word.dict";

    config->get_string("qws.viterbi_word_dict", word_dict_path);

    m_word_dict->set_path(word_dict_path);

    if (!m_word_dict->load()) {
        fprintf(stderr, "load viterbi word dict file error: %s\n", word_dict_path.c_str());
        return false;
    }

    m_total_words_frequency = 1168782086; // the feequency of all words

    config->get_double("qws.viterbi_total_words_frequency",  m_total_words_frequency);

    //matches.reserve(10240);
    init_single_symbols();

    return true;
}

void QWSVITERBITokenizer::init_single_symbols()
{
    QDD("init single_symbols: %d\n", MAX_SINGLE_SYMBOL_SIZE);
    for (int i = 0; i < MAX_SINGLE_SYMBOL_SIZE; ++i) {
        QWSSingleSymbolMatch &single_symbol = single_symbols[i];
        size_t node_pos = 0, key_pos = 0;
        int value = -2;
        std::string symbol_utf8;
        QWSUtil::symbol_to_utf8(i, symbol_utf8);
        value = m_word_dict->traverse(symbol_utf8.c_str(), node_pos, key_pos, symbol_utf8.length());
        single_symbol.traverse_value = value;
        single_symbol.traverse_node_pos = node_pos;
        single_symbol.traverse_key_pos = key_pos;
    }
}

void QWSVITERBITokenizer::find_matches(
            const std::string &normalized,
            QWSSymbolList &symbols,
            int symbol_start, int symbol_end,
            QWSMatchList &matches) {

    int max_word_length = 6;
    for (int i = symbol_start; i < symbol_end; i++) {
        QWSSymbol &symbol = symbols[i];

        int word_end = i + max_word_length;
        if (word_end > symbol_end) {
            word_end = symbol_end;
        }

        int value;

        QWSMatch match;
        match.match_start = i;
        match.match_end = i + 1;

        if (m_word_dict->query(normalized.c_str() + symbol.normalized_start, \
                        symbol.normalized_end - symbol.normalized_start, &value)) {
            match.frequency = value;
        } else {
            match.frequency = 0;
        }

        matches.push_back(match);

        for (int j = word_end - 1; j > i; --j) {
            if (m_word_dict->query(normalized.c_str() + symbol.normalized_start, \
                        symbols[j].normalized_end - symbol.normalized_start, &value)) {
                QWSMatch match;
                match.match_start = i;
                match.match_end = j + 1;
                match.frequency = value;
                matches.push_back(match);
            }
        }
    }
}

void QWSVITERBITokenizer::traverse_matches(
            const std::string &normalized,
            QWSSymbolList &symbols,
            int symbol_start, int symbol_end,
            QWSMatchList &matches)
{
    const char *sentence_base = normalized.c_str();
    const char *base;
    size_t node_pos = 0, key_pos = 0;
    int value = 0;

    for (int i = symbol_start; i < symbol_end; ++i) {
        QWSSymbol &symbol = symbols[i];
        base = sentence_base + symbol.normalized_start;

        QWSSingleSymbolMatch &single_symbol = single_symbols[symbol.value];
        value = single_symbol.traverse_value;
        node_pos = single_symbol.traverse_node_pos;
        key_pos = single_symbol.traverse_key_pos;

        if (value >= 0) {
            QWSMatch match(i, i + 1, value);
            matches.push_back(match);
            // continue;
        } else if (value == -1) {
            QWSMatch match(i, i + 1, 0);
            matches.push_back(match);
            // continue;
        } else if (value == -2) {
            QWSMatch match(i, i + 1, 0);
            matches.push_back(match);
            continue; // next symbol
        } else {
            assert(false);
        }

        for (int j = i + 1; j < symbol_end; ++j) {
            value = m_word_dict->traverse(base, node_pos, key_pos,
                symbols[j].normalized_end - symbol.normalized_start);
            if (value >= 0) {
                //add a match
                QWSMatch match(i, j + 1, value);
                matches.push_back(match);
            } else if (value == -1) {
                // continue;
            } else if (value == -2) {
                break;
            } else {
                assert(false);
            }
        }

/*
        for (int j = i; j < symbol_end; ++j) {
            QDD("i : %d, j: %d node_pos: %d key_pos:%d length: %d\n", i, j, \
                    node_pos, key_pos, \
                    symbols[j].normalized_end - symbol.normalized_start);
            // cache for single symbols
            if ( j == i) {
                QWSSingleSymbolMatch &single_symbol = single_symbols[symbol.value];
                if (single_symbol.cached_traverse) {
                    value = single_symbol.traverse_value;
                    node_pos = single_symbol.traverse_node_pos;
                    key_pos = single_symbol.traverse_key_pos;
                } else {
                    value = m_word_dict->traverse(base, node_pos, key_pos,
                        symbols[j].normalized_end - symbol.normalized_start);
                    single_symbol.traverse_value = value;
                    single_symbol.traverse_node_pos = node_pos;
                    single_symbol.traverse_key_pos = key_pos;
                    single_symbol.cached_traverse = true;
                }
                if (value < 0) {
                    QWSMatch match(i, j + 1, 0);
                    matches.push_back(match);
                }
            } else {
                value = m_word_dict->traverse(base, node_pos, key_pos,
                    symbols[j].normalized_end - symbol.normalized_start);
            }

            QDD("value: %d\n", value);
            if (value >= 0) {
                //add a match
                QWSMatch match(i, j + 1, value);
                matches.push_back(match);
            } else if (value == -1) {
                // continue;
            } else if (value == -2) {
                break;
            } else {
                assert(false);
            }
        }
*/
    }
}

void QWSVITERBITokenizer::tokenize_sentence(
            const std::string &normalized,
            QWSSymbolList &symbols,
            int symbol_start, int symbol_end)
{
    // match
    QWSMatchList matches;
    //matches.clear();
    //matches.reserve((symbol_end - symbol_start) * 2);
    //find_matches(normalized, symbols, symbol_start, symbol_end, matches);
    traverse_matches(normalized, symbols, symbol_start, symbol_end, matches);
    // viterbi
    viterbi(symbols, symbol_start, symbol_end, matches);
    //print_matches(matches);
}

void QWSVITERBITokenizer::viterbi(QWSSymbolList &symbols, int symbol_start, int symbol_end, QWSMatchList &matches)
{
    std::vector<double> fee_vec(symbol_end - symbol_start + 1, std::numeric_limits<double>::max());
    std::vector<int> fee_match_vec(symbol_end - symbol_start + 1, -1);

    fee_vec[0] = 0.0;

    int matches_size = matches.size();
    for (int i = 0; i < matches_size; ++i) {
        QWSMatch &match = matches[i];

        double unigram_prob = computer_unigram_prob(match.frequency);
        double fee = -log(unigram_prob);

#ifdef QWS_DEBUG
        std::string word;
        for (int d = match.match_start; d < match.match_end; ++d) {
            std::string s;
            QWSUtil::symbol_to_utf8(symbols[d].value, s);
            word.append(s);
        }
        QDD("match: %d..%d word: %s\tprob: %f fee %f\n", \
                match.match_start, match.match_end, word.c_str(), unigram_prob, fee);
#endif
        int match_offset_end = match.match_end - symbol_start;
        int match_offset_start = match.match_start - symbol_start;
        if (fee_vec[match_offset_start] + fee < fee_vec[match_offset_end]) {
            fee_vec[match_offset_end] = fee_vec[match_offset_start] + fee;
            fee_match_vec[match_offset_end] = match.match_start;
        }
    }

    int end_idx = symbol_end;
    while (end_idx > symbol_start) {
        int match_start = fee_match_vec[end_idx - symbol_start];
        if (match_start < 0) {
            --end_idx;
        } else {
            if (match_start > 0) {
                symbols[match_start - 1].token_position = 1;
            }
            end_idx = match_start;
        }
    }
}

END_NAMESPACE_QWS
