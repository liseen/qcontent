/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

#include "qurlfilter.h"

#include <glog/logging.h>
#include <iostream>

int main()
{
    qcontent::QUrlFilter urlfilter("./test.conf");

    std::string url;
    while (getline(std::cin, url)) {
        std::string ret = urlfilter.filter(url);
        if (!ret.empty()) {
            std::cout << ret << std::endl;
        }
    }

    return 0;
}
