/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

#include <string>
#include <googleurl/src/gurl.h>

#include "qhost2site/qhost2site.h"

using namespace std;

int main()
{
    string line;
    while (getline(cin, line)) {
        GURL url(line);
        //std::cout << "standard: " << url.IsStandard() << "\n";
        //std::cout << "has port: " << url.has_port() << "\n";

        if (url.is_valid()) {
            std::cout << qhost2site(url.host()) << "\t" << url.host() << "\n";
        }
    }

    return 0;
}
