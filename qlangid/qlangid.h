/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

#ifndef QLANGID_H
#define QLANGID_H

#include <string>
#include <cassert>

#include "udm_common.h"

#define QLANGID_THRESHOLD_DIFF 1.03
#define QLANGID_THRESHOLD_DIFF_ON_TINY 2

namespace qcontent {

class QLangId {

typedef void *  textcat_handler;
public:
    QLangId(const std::string &conf_file);
    ~QLangId();
    std::string detect(const std::string &origin_charset, const std::string &text);

private:
    UDM_ENV *env;
};

}


#endif
