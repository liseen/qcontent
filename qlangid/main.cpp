/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

#include <strtk.hpp>
#include <vector>
#include <string>

#include <iostream>

#include "qlangid.h"

#include <getopt.h>

static void print_usage(FILE* stream, int exit_code) {
    fprintf(stream, "Usage: qcontenthubd options \n");
    fprintf(stream,
            "  -h --help               Display this usage information.\n"
            "  -c --conf <file>        Config file\n");

    exit(exit_code);
}


int main(int argc, char *argv[])
{
    std::string conf_file = "./qlangid.conf";

    const char* const short_options = "hdc:";
    const struct option long_options[] = {
        { "help",     0, NULL, 'h' },
        { "conf",    1, NULL, 'c' },
        { NULL,       0, NULL, 0   }
    };

    int next_option;
    do {
        next_option = getopt_long (argc, argv, short_options,
                               long_options, NULL);
        switch (next_option) {
            case 'h':
                print_usage(stdout, 0);
                break;
            case 'c':
                conf_file = optarg;
                break;
            case -1:
                break;
            case '?':
                print_usage(stderr, 1);
            default:
                print_usage(stderr, 1);
        }
    } while (next_option != -1);

    qcontent::QLangId detector(conf_file);
    std::string lan;
    std::string line;
    int lineno = 0;
    std::vector<std::string> record;
    while (getline(std::cin, line)) {
        ++lineno;
//        lan = detector.detect("", line);
//        std::cout << line << "\t" << lan << "\n";

        record.clear();
        strtk::parse(line.c_str(), line.c_str() + line.size(),  "\t", record, strtk::split_options::default_mode);
        if (record.size() > 0) {
            lan = detector.detect("", record[record.size() - 1]);
            std::cout << line << "\t" << lan << "\n";
        } else {
            std::cerr << "error found lineno: " << lineno << std::endl;
        }
    }

    return 0;
}
