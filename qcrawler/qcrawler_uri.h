/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

#ifndef QCRAWLER_URI_H
#define QCRAWLER_URI_H

#include <string>

namespace qcrawler
{

struct QCrawlerURI
{
    struct ev_io io;
    std::string host;
    unsigned short port;
    //QCrawlerRecord rec;
};

} // end namespace qcrawler

#endif

