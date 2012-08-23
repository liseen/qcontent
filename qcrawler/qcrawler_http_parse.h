/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

#ifndef QCRAWLER_HTTP_PARSE
#define QCRAWLER_HTTP_PARSE

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <string>
#include <map>

#include <glog/logging.h>

namespace qcrawler
{
    bool http_parse(const std::string &raw_content, int &status, \
            std::map<std::string, std::string> &headers, std::string &body);

} // end namespace qcrawler

#endif
