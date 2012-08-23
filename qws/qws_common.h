/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

#ifndef QCONTENT_QWS_COMMON_H
#define QCONTENT_QWS_COMMON_H

#include <cstdio>
#include <cstdlib>
#include <cassert>

#define BEGIN_NAMESPACE_QWS namespace qcontent {
#define END_NAMESPACE_QWS }

//#define QWS_DEBUG 1

#ifdef QWS_DEBUG

#define QWS_WHERESTR  "[%s:%d] "
#define QWS_WHEREARG  __FILE__, __LINE__
#define QWS_DEBUGPRINT2(...)       fprintf(stderr, __VA_ARGS__)
#define QDD(_fmt, ...)  QWS_DEBUGPRINT2(QWS_WHERESTR _fmt, QWS_WHEREARG, __VA_ARGS__)

#else

#define QDD(_fmt, ...)  do {} while(0);

#endif

#endif
