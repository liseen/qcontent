/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

#include <cstdio>
#include <cstdlib>
#include <string>
#include <iostream>
#include <getopt.h>

#include "qws_dict_darts.h"
#include "qws_util.h"

int main(int argc, char *argv[])
{
    std::string cmd = "build";
    std::string path = "key.dict";

    if (argc > 1) {
        cmd = argv[1];
    }

    if (argc > 2) {
        path = argv[2];
    }

    if (cmd == "build") {
        qcontent::QWSDict qws_dict;

        int lineno = 0;
        std::string line;
        qcontent::QWSDartsKeyList key_list;
        while (std::getline(std::cin, line)) {
            ++lineno;
            if (line.empty()) {
                continue;
            }
            std::string word;
            int value = lineno;
            size_t pos = line.find("\t");
            if (pos != std::string::npos) {
                word = line.substr(0, pos);
                value = atoi(line.substr(pos).c_str());
            } else {
                word = line;
            }

            qcontent::QWSDartsKey key;
            key.word = word;
            key.value = value;
            key_list.push_back(key);
/*
            std::string word16;
            if (qcontent::QWSUtil::utf8_to_utf16(word, word16)) {
                qcontent::QWSDartsKey key;
                key.word = word16;
                key.value = value;
                key_list.push_back(key);
             } else {
                std::cerr << "error line(" << lineno << "): " << word << std::endl;
            }
            */
        }

        // build
        qws_dict.build(key_list);
        // save
        qws_dict.set_path(path);
        qws_dict.save();
    } else if (cmd == "list") {
        qcontent::QWSDict qws_dict(path);
        std::string line;
        int lineno = 0;
        while (std::getline(std::cin, line)) {
            lineno++;
            if (line.empty()) {
                continue;
            }

            std::string word;
            size_t pos = line.find("\t");
            if (pos != std::string::npos) {
                word = line.substr(0, pos);
            } else {
                word = line;
            }

            int value = -1;
            qws_dict.query(word, &value);
            std::cout << word << "\t" << value << std::endl;
            /*
            std::string word16;
            if (qcontent::QWSUtil::utf8_to_utf16(word, word16)) {
                int value = -1;
                qws_dict.query(word16, &value);
                std::cout << word << "\t" << value << std::endl;
            } else {
                std::cerr << "error line(" << lineno << "): " << word << std::endl;
            }
            */
        }
    } else {
        std::cerr << "Unsupported cmd " << cmd << std::endl;
    }
}
