/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

#ifndef QCONTENT_QWS_H
#define QCONTENT_QWS_H

#include <string>
#include <vector>

#include "qws_common.h"
#include "qws_config.h"

#include "qws_symbol.h"
#include "qws_token.h"
#include "qws_tokenizer.h"

BEGIN_NAMESPACE_QWS

class QWS {
public:
    explicit QWS();
    virtual ~QWS();

    bool init(QWSConfig *config);

    void tokenize(const std::string &text, QWSTokenList &tokens);

protected:
    void parse(const std::string &text, std::string &normalized_text, QWSSymbolList &symbols);

    QWSTokenizer *tokenizer;

    int normalize_method;
    bool output_normalized;
};


END_NAMESPACE_QWS

#endif
