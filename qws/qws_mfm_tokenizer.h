/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

#ifndef QCONTENT_QWS_MFM_TOKENIZE_H
#define QCONTENT_QWS_MFM_TOKENIZE_H

#include "qws_tokenizer.h"
#include "qws_dict_darts.h"

BEGIN_NAMESPACE_QWS

class QWSMFMTokenizer : public QWSTokenizer
{
public:
    QWSMFMTokenizer();
    virtual ~QWSMFMTokenizer();

    virtual TokenizerType type();

    virtual bool init(QWSConfig *config);
    virtual void tokenize_sentence(
            const std::string &normalized,
            QWSSymbolList &symbols,
            int symbol_start, int symbol_end);

private:
    QWSDict *m_word_dict;
    int max_word_length;
};

END_NAMESPACE_QWS

#endif
