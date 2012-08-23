#include <cstdio>
#include <cstdlib>
#include <getopt.h>

#include <string>
#include <vector>
#include <iostream>

#include <strtk.hpp>
#include "qtitlefilter.h"

void print_usage (const char* program_name, int exit_code)
{
    FILE* stream = stdout;
    if (exit_code != 0 ) {
        stream = stderr;
    }

    fprintf (stream, "Usage:  %s options \n", program_name);
    fprintf (stream,
            "  -h  --help                         Display this usage information.\n"
            "  -m  --mode <stat|gen_attr|filter>  Mode.\n"
            "  -f  --stat_file  <filepath>        Stat file path.\n"
            );
    exit (exit_code);
}



int main(int argc, char *argv[])
{
    const char* program_name = argv[0];

    std::string mode_arg;
    qcontent::QTitleFilter::FilterMode mode = qcontent::QTitleFilter::STAT;
    std::string stat_file = "./qtitlefilter_stat_file.txt";
    // TODO other argument

    /* parse options */
    int next_option;

    const char* const short_options = "m:f:";
    const struct option long_options[] = {
        { "help",     0, NULL, 'h' },
        { "mode",   1, NULL, 'm' },
        { "file",   1, NULL, 'f' },
        { NULL,       0, NULL, 0   }
    };

    do {
        next_option = getopt_long(argc, argv, short_options,
                long_options, NULL);
        switch (next_option)
        {
            case 'h':
                print_usage(program_name, 0);
                break;
            case 'f':
                stat_file = optarg;
                break;
            case 'm':
                mode_arg = optarg;
                if (mode_arg == "stat") {
                    mode = qcontent::QTitleFilter::STAT;
                } else if (mode_arg == "gen_attr") {
                    mode = qcontent::QTitleFilter::GEN_ATTR;
                } else {
                    mode = qcontent::QTitleFilter::FILTER;
                }

                break;
            case '?':
                print_usage(program_name, 1);
            case -1:
                break;
            default:
                abort ();
        }
    } while (next_option != -1);

    std::vector<std::string> record;
    std::string line;

    qcontent::QTitleFilter qtitlefilter(mode, stat_file);
    if (!qtitlefilter.init()) {
        fprintf(stderr, "init qtitlefilter error\n");
        exit(1);
    }

    int count = 0;
    if (mode == qcontent::QTitleFilter::STAT) {
        while (std::getline(std::cin, line)) {
            ++count;
            if (count % 100000 == 0) {
                std::cerr << "Processing " << count << " line\n";
            }
            record.clear();
            strtk::parse(line.c_str(), line.c_str() + line.size(),  "\t", record, strtk::split_options::default_mode);
            if (record.size() <= 21) {
                continue;
            }

            std::string &host = record[1];
            std::string &url = record[2];
            std::string &raw_title = record[20];

            if (raw_title.empty()) {
                continue;
            }

            qtitlefilter.stat(host, url, raw_title);
        }
        qtitlefilter.save_stat_result();
    } else if (mode == qcontent::QTitleFilter::GEN_ATTR) {
        while (std::getline(std::cin, line)) {
            ++count;
            if (count % 100000 == 0) {
                std::cerr << "Processing " << count << " line\n";
            }
            record.clear();
            strtk::parse(line.c_str(), line.c_str() + line.size(),  "\t", record, strtk::split_options::default_mode);
            if (record.size() <= 21) {
                continue;
            }

            std::string &host = record[1];
            std::string &url = record[2];
            std::string &raw_title = record[20];

            std::string title_attr;
            qtitlefilter.gen_title_attribute(host, url, raw_title, title_attr);
            std::cout << host << "\t" << raw_title << "\t" << title_attr << "\n";
        }
    } else {
        while (std::getline(std::cin, line)) {
            ++count;
            if (count % 100000 == 0) {
                std::cerr << "Processing " << count << " line\n";
            }
            record.clear();
            strtk::parse(line.c_str(), line.c_str() + line.size(),  "\t", record, strtk::split_options::default_mode);
            if (record.size() <= 21) {
                continue;
            }


            std::string &host = record[1];
            std::string &url = record[2];
            std::string &raw_title = record[20];

            std::string filtered;
            qtitlefilter.filter(host, url, raw_title, filtered);
            std::cout << host << "\t" << raw_title << "\t" << filtered << "\n";
        }
    }

    std::cerr << "Processed " << count << " line\n";

    return 0;
}
