/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

#include <iostream>
#include <cstdio>
#include <googleurl/src/gurl.h>

using namespace std;

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "args error\n");
        fprintf(stderr, "Usage: normal-url <valid|invalid>\n");
        exit(1);
    }

    string arg(argv[1]);
    bool output_valid = false;
    bool output_invalid = false;

    if (arg == "valid") {
        output_valid = true;
    } else if (arg == "invalid") {
        output_invalid = true;
    } else {
        fprintf(stderr, "args error\n");
        fprintf(stderr, "Usage: normal-url <valid|invalid>\n");
        exit(1);
    }

    string line;
    while (getline(cin, line)) {
        GURL url(line);
        if (url.is_valid()) {
            if (output_valid) {
                cout << url.spec() << endl;
            }
        } else {
            if (output_invalid) {
                cout << line << endl;
            }
        }
    }

    return 0;
}
