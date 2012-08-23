/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

#include "qpredict_ngram.h"

#include <iostream>
#include <vector>

#include "utf8.h"

namespace qcontent
{

// only supported utf8
int QPredictNgram::terms_count(const std::string &text, TermHash &terms)
{
    const char *w = text.c_str();

    const char *prev_w = w;

    const char *end = text.c_str() + text.size();

    bool prev_is_noise = false;
    std::vector<std::string> str_vec;
    str_vec.reserve(text.size());
    while (w != end) {
        prev_w = w;
        int cp = utf8::next(w, end);
        if (cp > 0) {
            //if (cp > 0x4E00 || (cp < 128 && cp > 20)) {
            if (cp > 0x4E00 && cp < 0x9FBF) {
                std::string word;
                word.append(prev_w, w-prev_w);
                str_vec.push_back(word);
                prev_is_noise = false;
            } else {
                if (!prev_is_noise) {
                    str_vec.push_back("_");
                }
                prev_is_noise = true;
            }
        }
    }
/*
    for (int i = 0; i < word_size; ++i) {
        std::cout << str_vec[i] << " ";
    }
    std::cout << "\n";
*/
    int word_size = str_vec.size();
    //for (int i = 0; i < m_ngram; ++i) {
    for (int i = 1; i < m_ngram; ++i) {
        for (int j = i; j < word_size; ++j) {
            std::string word;
            for (int k = j - i; k < j + 1; ++k) {
                word.append(str_vec[k]);
            }

            if (word == "_") {
                continue;
            }

            TermHashIter it = terms.find(word);
            if (it == terms.end()) {
                QPredictTerm term;
                term.tf = 1;
                terms[word] = term;
            } else {
                it->second.tf += 1;
            }

            //std::cout << "#" << word << "#";
        }
    }

    //std::cout << "\n";

    return terms.size();
}

} // end for namespace qcontent
