/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 *    zhenbao.zhou (周桢堡)  <zhenbao.zhou@qunar.com>
 *
 * dedup server main function
 */

#include <vector>
#include <string>
#include <getopt.h>
#include <signal.h>
#include <unistd.h>

#include <glog/logging.h>

#include "qdedup_server.h"
#include "qdedup_common.h"

qcontent::QDedupServer *qdedup_server;

void print_usage(const char* program_name, int exit_code) {
    FILE* stream = stdout;
    if (exit_code != 0) {
        stream = stderr;
    }

    fprintf(stream, "Usage:  %s options \n", program_name);
    fprintf(stream,
            "  -h  --help             Display this usage information.\n"
            "  -f                     Mmap file name. (Default is 'dedup_mmap_file') \n"
            "  -s                     Mmap file size. (Default is 100M)\n"
            "  -p                     Server port.   (Default is 8891)\n"
            "  -t                     Server thread num(Default is 10) \n"
            "  -D                     run as a daemon\n"
            "  -T                     Sync threshold of mmap (Default is 500000) \n");
    exit(exit_code);
}

void signal_handler(int sig) {
    switch(sig) {
        case SIGTERM:
        case SIGHUP:
        case SIGINT:
        case SIGQUIT: {
            if (qdedup_server) {
                qdedup_server->stop();
                exit(0);
            }
            break;
        }
        default:
            break;
    }
}


// main function here
int main(int argc, char *argv[]) {
    const char *program_name = argv[0];
    int port = 8881;
    char *mmap_file = "dedup_mmap_file";
    int mmap_size = 100;
    int thread_num = 10;
    int sync_threshold = 500000;
    bool is_daemon = false;
    
    const char* const short_options = "hp:f:s:t:T:D";
    const struct option long_options[] = {
        { "help",     0, NULL, 'h' },
        { "file",    0, NULL, 'f' },
        { "size",    0, NULL, 's' },
        { "port",    0, NULL, 'p' },
        { "thread_num", 0, NULL, 't' },
        { "sync_threshold", 0, NULL, 'T' },
        { NULL,       0, NULL, 0   }
    };

        /* parse options */
    int next_option;

    do {
        next_option = getopt_long(argc, argv, short_options,
                long_options, NULL);

        switch (next_option) {
            case 'h':
                print_usage(program_name, 0);
            case 'p':
                port = atoi(optarg);
                break;
            case 'f':
                mmap_file = optarg;
                break;
            case 's':
                mmap_size = atoi(optarg);
                break;
            case 't':
                thread_num = atoi(optarg);
                break;
            case 'T':
                sync_threshold = atoi(optarg);
                break;
            case 'D':
                is_daemon = true;
                break;
            case '?':
                print_usage(program_name, 1);
                break;
            case -1:
                break;
            default:
                abort();
        }
    } while (next_option != -1);

    google::InitGoogleLogging(argv[0]);
    mmap_size *= 1024 * 1024;
    if (port == 0 || strlen(mmap_file) == 0) {
        LOG(FATAL) << " port == 0 or empty mmap_file";
    }
    
    LOG(INFO) << "Start server:";
    LOG(INFO) << "Host:\t0.0.0.0" ;
    LOG(INFO) << "Port:\t" << port ;
    LOG(INFO) << "mmap_file:\t" << mmap_file ;
    LOG(INFO) << "mmap_size:\t" << mmap_size ;
    LOG(INFO) << "mmap_sync_threshold:\t" << sync_threshold; 

    msgpack::rpc::loop lo;
    qcontent::QDedupServer srv(lo);
    qdedup_server = &srv;


    signal(SIGHUP, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);
    signal(SIGQUIT, signal_handler);

    if (srv.init_server(mmap_file, mmap_size, sync_threshold) != 0) {
        //        std::cerr << "error in init server. exists" << std::endl;
        LOG(FATAL) << "error in init server. exits....." ;
    }

    srv.instance.listen("0.0.0.0", port);
    
    if (is_daemon) {
        daemon(1, 1);
    }

    srv.start(thread_num);

    
    return 0;
}
