/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

/*
 * qpredict_term.h
 *
 */

#ifndef QPREDICT_TERM_H
#define QPREDICT_TERM_H

#include <stdint.h>
#include <iostream>
#include <vector>
#include "qpredict_define.h"

namespace qcontent
{

struct QPredictTerm
{
public:
    QPredictTerm():id(0), tf(0), df(0), feature_score(0.0) {
        for (uint32_t i = 0; i < QPREDICT_MAX_CLASSES; ++i) {
            df_per_class[i] = 0;
        }
    }

    std::string value;

    uint32_t id;
    uint32_t tf;
    uint32_t df;

    uint32_t df_per_class[QPREDICT_MAX_CLASSES];
    double feature_score;
    double feature_value;

    void debug_print() {
        std::cout << "value: " << value
            << " id: " << id
            << " tf: " << tf
            << " df: " << df
            << " feature_score: " << feature_score
            << " feature_value: " << feature_value
            << " df_per_class0: " << df_per_class[0]
            << " df_per_class1: " << df_per_class[1]
            << "\n";
    }
};

//typedef hash_map<std::string, QPredictTerm, str_hash, str_equal>  TermHash;
//typedef hash_map<std::string, QPredictTerm, str_hash, str_equal>::iterator  TermHashIter;

typedef unordered_map<std::string, QPredictTerm>  TermHash;
typedef unordered_map<std::string, QPredictTerm>::iterator  TermHashIter;

} // end for namespace qcontent

#endif
