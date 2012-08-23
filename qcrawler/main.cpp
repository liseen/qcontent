/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */


#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include <glog/logging.h>

#include <qcontent_config.h>

#include "qcrawler_dns.h"
#include "qcrawler.h"
#include "qcrawler_http.h"

using namespace qcrawler;

static QCrawler *global_qcrawler = NULL;
static void signal_handler(int sig)
{
    switch(sig) {
        case SIGTERM:
        case SIGHUP:
        case SIGINT:
            if (global_qcrawler) {
                global_qcrawler->stop();
            }

            break;
        default:
            break;
    }
}


static void print_usage(FILE* stream, int exit_code) {
    fprintf(stream, "Usage: qcrawler options \n");
    fprintf(stream,
            "  -h --help         Display this usage information.\n"
            "  -d --daemon       Run as a daemon\n"
            "  -c --conf <file>  Config file\n");

    exit(exit_code);
}



int main(int argc, char *argv[])
{
    int daemon = 0;

    pid_t   pid, sid;
    std::string config_file = "./qcrawler.conf";

    const char* const short_options = "hdc:";
    const struct option long_options[] = {
        { "help",     0, NULL, 'h' },
        { "daemon",   0, NULL, 'd' },
        { "conf",    1, NULL, 'c' },
        { NULL,       0, NULL, 0   }
    };

    int next_option;
    do {
        next_option = getopt_long (argc, argv, short_options,
                               long_options, NULL);
        switch (next_option) {
            case 'd':
                daemon = 1;
                break;
            case 'h':
                print_usage(stdout, 0);
                break;
            case 'c':
                config_file = optarg;
                break;
            case -1:
                break;
            case '?':
                print_usage(stderr, 1);
            default:
                print_usage(stderr, 1);
        }
    } while (next_option != -1);

    /*
    if (daemon) {
        pid = fork();
        if (pid < 0) {
            exit(EXIT_FAILURE);
        } else if (pid > 0) {
            exit(EXIT_SUCCESS);
        }

        umask(0);
        sid = setsid();
        if (sid < 0) {
            exit(EXIT_FAILURE);
        }
    }
    */

    signal(SIGHUP, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);
    signal(SIGQUIT, signal_handler);

    google::InitGoogleLogging(argv[0]);
    //
    std::string url_queue_host = "localhost";
    uint16_t url_queue_port = 19854;

    std::string crawled_queue_name = "crawled";
    std::string crawled_queue_host = "localhost";
    uint16_t crawled_queue_port = 7676;

    int multiple = 60;
    int record_buf_size = 60;
    int fetch_task_min_cycle = 100;
    int fetch_task_max_cycle = 1000;

    std::string useragent = "libqcrawler";
    int http_timeout = 120;

    qcontent::QContentConfig *config = qcontent::QContentConfig::get_instance();
    if (!config->parse_file(config_file)) {
        LOG(FATAL) << "Parse config file error";
    }

    url_queue_host = config->get_string("url_queue.host", url_queue_host);
    url_queue_port = (uint16_t) config->get_integer("url_queue.port", url_queue_port);

    crawled_queue_name = config->get_string("crawled_queue.name", crawled_queue_name);
    crawled_queue_host = config->get_string("crawled_queue.host", crawled_queue_host);
    crawled_queue_port = (uint16_t) config->get_integer("crawled_queue.port", crawled_queue_port);

    fetch_task_min_cycle = config->get_integer("qcrawler.fetch_task_min_cycle", fetch_task_min_cycle);
    fetch_task_max_cycle = config->get_integer("qcrawler.fetch_task_max_cycle", fetch_task_max_cycle);

    multiple = config->get_integer("qcrawler.multiple", multiple);
    record_buf_size = config->get_integer("qcrawler.record_buf_size", record_buf_size);
    useragent = config->get_string("qcrawler.useragent", useragent);
    http_timeout = config->get_integer("qcrawler.http_timeout", http_timeout);

    qcontent::UrlQueue url_queue(url_queue_host, url_queue_port);
    qcontent::HubQueue crawled_queue(crawled_queue_host.c_str(), crawled_queue_port, crawled_queue_name);

    qcontent::QStore qstore(config);

    std::string qurlfilter_conf_file = "./qurlfilter.conf";
    qurlfilter_conf_file = config->get_string("qurlfilter.conf_file", qurlfilter_conf_file);
    qcontent::QUrlFilter qurlfilter(qurlfilter_conf_file);

    QCrawler *qcrawler = QCrawler::get_instance();

    global_qcrawler = qcrawler;
    qcrawler->init(fetch_task_min_cycle, fetch_task_max_cycle,
            multiple, record_buf_size,
            useragent, http_timeout,
            &url_queue, &crawled_queue, &qstore, &qurlfilter);

    qcrawler->run();

    LOG(INFO) << "safe stop qcrawler okay";
    return 0;
}
