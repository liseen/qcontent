/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 *    zhenbao.zhou (周桢堡)  <zhenbao.zhou@qunar.com>
 *
 * Main function for client
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <inttypes.h>
#include <sys/mman.h>
#include <getopt.h>
#include <glog/logging.h>


#include <vector>
#include <cassert>
#include <string>
#include <assert.h>

#include <strtk.hpp>

#include "qcontent_record.h"
#include "qdedup_client.h"
#include "qdedup_constant.h"

#define TO_OUT 0
#define TO_DEDUPED 1
#define TO_BAD_LIST 2

static int verbose = 0;
static int debug_mode = 0;

static int enable_stat = 0;
static int document_num = 0;
static int passed_url = 0;
static int deduped_url = 0;
static int bad_list = 0;
static int error_url = 0;
static int gain_and_good_list = 0;

qcontent::QDedupClient *qdc = NULL;

FILE *output_passed = NULL;
FILE *output_deduped = NULL;
FILE *output_bad_list = NULL;

int open_files(const std::string &file_prefix);
void close_files();
void print_usage(const char *program_name, int exit_code);
void deal_line(const std::string &line);
void output_result(int type, std::vector<std::string> &record);
int split_string(const std::string &line, std::string &site,
                  std::string &all_content, std::vector<std::string> &record);

int open_files(const std::string &file_prefix) {
    std::string passed_file = file_prefix + ".out";
    std::string deduped_file = file_prefix + ".del";
    std::string bad_list_file = file_prefix + ".bad_list";

    LOG(INFO) << "open file with file prefix:" << file_prefix;
    output_passed = fopen(passed_file.c_str(), "w");
    if (NULL == output_passed) {
        LOG(ERROR) << "error in open file:" << passed_file;
        return 1;
    }

    output_deduped = fopen(deduped_file.c_str(), "w");
    if (NULL == output_deduped) {
        LOG(ERROR) << "error in open file:" << deduped_file;
        return 1;
    }

    output_bad_list = fopen(bad_list_file.c_str(), "w");
    if (NULL == output_bad_list) {
        LOG(ERROR) << "error in open file:" << bad_list_file;
        return 1;
    }
        
    return 0;
}

void close_files() {
    fclose(output_passed);
    fclose(output_deduped);
    fclose(output_bad_list);

    output_passed = NULL;
    output_deduped = NULL;
    output_bad_list = NULL;
}

void deal_line(const std::string &line) {
    std::string site;
    std::string content;
    std::vector<std::string> record;
    
    int split_re = split_string(line, site, content, record);

    switch (split_re) {
        case -1: {
            if (enable_stat) {
                document_num++;
                error_url++;
            }
            
            return;
        }
        case -2: {
            if (enable_stat) {
                document_num++;
                bad_list++;
            }
            
            output_result(TO_BAD_LIST, record);
            break;
        }
        case 0: {
            int ret = qdc->dedup_check(site.c_str(), site.size(),
                                       content.c_str(), content.size());
            if (ret == IS_NEW_CONTENT) {
                output_result(TO_OUT, record);
            } else {
                output_result(TO_DEDUPED, record);
            }
            
            if (enable_stat) {
                document_num++;
                if (ret == IS_NEW_CONTENT) {
                    passed_url++;
                } else {
                    deduped_url++;
                }
            }
            break;
        }
        case 1:
        case 2: {
            if (enable_stat) {
                document_num++;
                gain_and_good_list++;
            }

            output_result(TO_OUT, record);
            break;
        }
        default: {
            LOG(ERROR) << "error of split_string";
            break;
        }
    }
}

void output_result(int type, std::vector<std::string> &record) {
    if (verbose == 0 && TO_OUT != type) {
        // donot output title, desc, content
        record[20] = "";
        record[23] = "";
        record[24] = "";
    }

    FILE *file_dest = NULL;
    if (TO_OUT == type) {
        file_dest = output_passed;
    } else if (TO_BAD_LIST == type) {
        file_dest = output_bad_list;
    } else {
        file_dest = output_deduped;
    }

    uint32_t record_size = record.size();
    for (uint32_t i = 0; i < record_size; i++) {
        fprintf(file_dest, "%s", record[i].c_str());
        if(record_size - 1 == i) {
            fprintf(file_dest, "\n");
        } else {
            fprintf(file_dest, "\t");
        }
    }
}


/* returns:
 *  2: list url wit >0 new_links_size.  Do not need go to function 'dedup'.   Output to *.out.
 *  1: gained url. Do not need go to function 'dedup'. Output to *.out
 *  0: normal CONTENT url. Dedup is needed. Output to *.out or *.stderr
 *  -1: error url. discard
 *  -2: list url with 0 new_links_size.     Output this article to .bad_list
 *
 *   notice: site, all_content, vector<string> record are also the output of split_string
 *
 */
int split_string(const std::string &line, std::string &site,
                 std::string &all_content, std::vector<std::string> &record) {
    record.clear();
    std::string delim = "\t";
    uint32_t sp_num = 25;
    if (debug_mode) {
        delim = " |\t";
        sp_num = 2;
    }

    strtk::parse(line.c_str(), line.c_str() + line.size(),
                 delim, record, strtk::split_options::default_mode);

    if (record.size() < sp_num) {
        LOG(ERROR) << "error line: " << line;
        return -1;
    }

    if (debug_mode) {
        site = record[0];
        all_content = record[1];
    } else {
        site = record[6];

        const std::string &crawl_type = record[8];
        const std::string &raw_title = record[20];
        const std::string &desc = record[23];
        const std::string &content = record[24];
        const std::string &is_list_str = record[12];
        const std::string &new_links_size_str = record[14];
        
        if (content.size() == 0 || site.size() == 0) {
            return -1;
        }

        char crawl_cmd = crawl_type.size() > 0 ? \
                         crawl_type.at(0) : qcontent::crawlcommand::GAIN;

        if (crawl_cmd == qcontent::crawlcommand::GAIN) {
            return 1;
        }

        int is_list = 0;
        strtk::string_to_type_converter(is_list_str, is_list);

        int new_links_size = 0;
        strtk::string_to_type_converter(new_links_size_str, new_links_size);
        
        if (1 == is_list) {
            if (0 == new_links_size) {
                return -2;
            } else {
                return 2;
            }
        }

        all_content.reserve(desc.size() + 1 +
                            raw_title.size() + 1 + content.size());
        all_content.append(desc);
        all_content.append("\n");
        all_content.append(raw_title);
        all_content.append("\n");
        all_content.append(content);
    }

    return 0;
}

// useless right now
int parse_string_to_addrVec(const std::string &str,
                            std::vector<qcontent::qdedup_addr> *addr_vec) {
    std::vector<std::string> tmp_vec;
    strtk::parse(str, ",|;", tmp_vec);
    for (uint32_t i = 0; i < tmp_vec.size(); i++) {
        const std::string &host_port_str = tmp_vec[i];
        std::string host;
        int port;

        strtk::parse(host_port_str, ":", host, port);
        if (host.empty()) {
            return 1;
        }

        qcontent::qdedup_addr addr;
        addr.host = host;
        addr.port = port;
        addr_vec->push_back(addr);
    }
    return 0;
}

//  a macro by strtk.
strtk_string_to_type_begin(qcontent::qdedup_addr)
static const std::string delimiters(":");
return strtk::parse(begin, end, delimiters,
                    t.host, t.port);
strtk_string_to_type_end()

void print_usage(const char* program_name, int exit_code) {
    FILE* stream = stdout;
    if (exit_code != 0) {
        stream = stderr;
    }

    fprintf(stream, "Usage:  %s options \n", program_name);
    fprintf(stream,
            "  -h               Display this usage information.\n"
            "  -u               Display usage of mmap file.\n"
            "  -c               Config file for client. (Default dedup.conf)\n"
            "  -d               Debug mode. Another format of input line\n"
            "  -s               Also output Deduped url in stderr.Notice: Undeduped content always output in stdout\n"
            "  -t               Stat the documents\n"
            "  -S               Such as 'localhost:8891,localhost:8893,192.168.4.2:2313'\n"
            "  -o               Output file's name\n"
            "  -H               Halt dedup servers\n"
            "  -v               Print verbose for every article. Otherwise, content and title will not output for deduped files\n");
    exit(exit_code);
}


int main(int argc, char *argv[]) {
    /* init */
    bool usage = false;
    bool show_deduped = false;
    char *server_info = NULL;
    char *output = "qdedup_client";
    bool halt_server = false;
    
    const char* program_name = argv[0];

    /* parse options */
    int next_option;

    const char* const short_options = "huvdstS:o:H";
    const struct option long_options[] = {
        { "help",     0, NULL, 'h' },
        { "usage",  0, NULL, 'u' },
        { "verbose",  0, NULL, 'v' },
        { "debug",  0, NULL, 'd' },
        { "show_deduped",  0, NULL, 's' },
        { "stat",  0, NULL, 't' },
        { "output", 0, NULL, 'o' },
        { "Server info", 0, NULL, 'S' },
        { "Halt server", 0, NULL, 'H' },
        { NULL,       0, NULL, 0   }
    };

    do {
        next_option = getopt_long(argc, argv, short_options,
                long_options, NULL);

        switch (next_option) {
            case 'h':
                print_usage(program_name, 0);
            case 'u':
                usage = true;
                break;
            case 'v':
                verbose = 1;
                break;
            case '?':
                print_usage(program_name, 1);
            case 'd':
                debug_mode = 1;
                break;
            case 's':
                show_deduped = true;
                break;
            case 'S':
                server_info = optarg;
                break;
            case 't':
                enable_stat = true;
                break;
            case 'o':
                output = optarg;
                break;
            case 'H':
                halt_server = true;
                break;
            case -1:
                break;
            default:
                abort();
        }
    } while (next_option != -1);

    google::InitGoogleLogging(argv[0]);
    
    std::vector<qcontent::qdedup_addr> addr_vec;
    strtk::parse(server_info, ",;", addr_vec);

    if (addr_vec.empty()) {
        std::cerr << "No input qdedup_addr. Exits." << std::endl;
        print_usage(program_name, 1);
    }

    qdc = new qcontent::QDedupClient(addr_vec);
    if (qdc->init_client() != 0) {
        std::cerr << "error in init client" << std::endl;
        exit(-1);
    }

    if (usage) {
        qdc->check_usage();
    } else if (halt_server) {
        qdc->halt_server();
    } else {
        if (open_files(output) != 0) {
            LOG(FATAL) << "Cannot init file handlers. file_prefix" << output;
        }
        
        std::string line;
        while (getline(std::cin, line)) {
            deal_line(line);
        }  // end of while

        close_files();
    }  // end of if(usage)

    delete qdc;

    if (enable_stat) {
        std::cout << "documents dealed:" << document_num << std::endl;
        std::cout << "error url:\t" << error_url << std::endl;
        std::cout << "bad list:\t" << bad_list << std::endl;
        std::cout << "gain url and good list:\t" << gain_and_good_list << std::endl;
        std::cout << "passed url:\t" << passed_url << std::endl;
        std::cout << "deduped url:\t" << deduped_url << std::endl;
    }

    return 0;
}
