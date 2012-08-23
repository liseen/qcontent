#include <cstdio>
#include <cstdlib>
#include <string>
#include <iostream>
#include <getopt.h>

#include "qws_dict.h"
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

            std::string word16;
            if (qcontent::QWSUtil::utf8_to_utf16(word, word16)) {
                qws_dict.insert(word16, value);
            } else {
                std::cerr << "error line(" << lineno << "): " << word << std::endl;
            }
        }

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

            std::string word16;
            if (qcontent::QWSUtil::utf8_to_utf16(word, word16)) {
                int value = -1;
                qws_dict.query(word16, &value);
                std::cout << word << "\t" << value << std::endl;
            } else {
                std::cerr << "error line(" << lineno << "): " << word << std::endl;
            }
        }
    } else {
        std::cerr << "Unsupported cmd " << cmd << std::endl;
    }
}
