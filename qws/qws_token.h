/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

#ifndef QCONTENT_QWS_TOKEN_H
#define QCONTENT_QWS_TOKEN_H

#include <string>
#include <deque>

#include "qws_common.h"

BEGIN_NAMESPACE_QWS

struct QWSToken {
    std::string value;
};

typedef std::deque<QWSToken> QWSTokenList;
typedef std::deque<QWSToken>::iterator QWSTokenListIter;

END_NAMESPACE_QWS

#endif
