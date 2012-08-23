/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */


#include <cassert>

#include "qpredict_ngram.h"
#include "qpredict_dict.h"

namespace qcontent {
    void test_ngrams() {
        qcontent::QPredictNgram ngramT;
        std::string str = "你好们如果是a你人们你";
        TermHash terms;
        ngramT.terms_count(str, terms);
    }

    void test_dict() {
        QPredictDict dict;
        std::string word = "nihao";

        dict.insert(word, 1);
        assert(dict.query(word) == 1);
    }
}



int main(int argc, char *argv[])
{
    qcontent::test_ngrams();
    qcontent::test_dict();

    return 0;
}
