/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

#ifndef QCONTENT_QWS_VITERBI_TOKENIZER_H
#define QCONTENT_QWS_VITERBI_TOKENIZER_H

#include "qws_tokenizer.h"
#include <cstring>
#include <deque>
#include <vector>

#include "qws_util.h"
#include "qws_dict_darts.h"

#define MAX_SINGLE_SYMBOL_SIZE 65536

BEGIN_NAMESPACE_QWS

struct QWSMatch {
    QWSMatch() {
    }

    QWSMatch(int s, int e, int f):match_start(s), match_end(e), frequency(f) {
    }

    int match_start;
    int match_end;
    int frequency;

    void print() {
        fprintf(stderr, "match_start: %d\n"
                        "match_end: %d\n"
                        "frequency: %d\n", match_start, match_end, frequency);
    }
};

typedef std::deque<QWSMatch> QWSMatchList;
typedef std::deque<QWSMatch>::iterator QWSMatchListIter;

struct QWSSingleSymbolMatch {
    QWSSingleSymbolMatch(): traverse_value(-2), traverse_node_pos(0), traverse_key_pos(0), frequency(false), viterbi_fee(0.0) {
    }
    int traverse_value;
    int traverse_node_pos;
    int traverse_key_pos;

    int frequency;
    double viterbi_fee;
};

class QWSVITERBITokenizer : public QWSTokenizer
{
public:
    QWSVITERBITokenizer();
    ~QWSVITERBITokenizer();

    virtual TokenizerType type();

    virtual bool init(QWSConfig *config);
    void init_single_symbols();

    virtual void tokenize_sentence(
            const std::string &normalized,
            QWSSymbolList &symbols,
            int symbol_start, int symbol_end);

    inline double computer_unigram_prob(int frequency) {
        return (frequency + 1) / (m_total_words_frequency);
    }

    void find_matches(
            const std::string &normalized,
            QWSSymbolList &symbols,
            int symbol_start, int symbol_end,
            QWSMatchList &matches);

    void traverse_matches(
            const std::string &normalized,
            QWSSymbolList &symbols,
            int symbol_start, int symbol_end,
            QWSMatchList &matches);

    void viterbi(QWSSymbolList &symbols, int symbol_start, int symbol_end, QWSMatchList &matches);

    void print_matches(QWSMatchList &matches) {
        for (size_t i = 0; i < matches.size(); i++) {
            matches[i].print();
        }
    }

private:
    QWSDict *m_word_dict;
    double m_total_words_frequency; //the size of doc list

    //QWSMatchList matches;
    QWSSingleSymbolMatch single_symbols[MAX_SINGLE_SYMBOL_SIZE];
};

END_NAMESPACE_QWS

#endif
