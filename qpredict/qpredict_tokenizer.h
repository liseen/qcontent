/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

/*
 *
 */

#ifndef QPREDICT_TOKENIZER_H
#define QPREDICT_TOKENIZER_H

#include "qpredict_define.h"
#include "qpredict_term.h"
#include "qpredict_config.h"

#include <qws/qws.h>

namespace qcontent
{

class QPredictTokenizer
{
public:
    QPredictTokenizer();
    bool init(QPredictConfig *config);
    int terms_count(const std::string &text, TermHash &terms);

private:
    qcontent::QWS *qws;
    qcontent::QWSConfig *qws_config;
};

}

#endif
