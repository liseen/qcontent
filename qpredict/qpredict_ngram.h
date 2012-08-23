/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

/*
 *
 */

#ifndef QPREDICT_NGRAM_H
#define QPREDICT_NGRAM_H

#include "qpredict_define.h"
#include "qpredict_term.h"

namespace qcontent
{

class QPredictNgram
{
public:
    //enum type {BYTE CHARACTER};
    //charset
    //stop words
    //stop regex
    QPredictNgram(int n = 2) :m_ngram(n)
    {

    }

    int terms_count(const std::string &text, TermHash &terms);

private:
    int m_ngram;
};

} // end for namespace qcontent

#endif
