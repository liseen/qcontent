#include <stdlib.h>
#include <getopt.h>
#include <glog/logging.h>

#include <QNetworkAccessManager>
#include <QNetworkProxy>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QStringList>
#include <QCoreApplication>

#include <qcontent_config.h>

#include "qfetcher.h"

static qcontent::QFetcher *global_qfetcher = NULL;

static void signal_handler(int sig)
{
    switch(sig) {
        case SIGTERM:
        case SIGHUP:
        case SIGINT:
            // TODO
            if (global_qfetcher) {
                global_qfetcher->stop();
            }

            break;
        default:
            break;
    }
}


static void print_usage(FILE* stream, int exit_code) {
    fprintf(stream, "Usage: qfetcher options \n");
    fprintf(stream,
            "  -h --help         Display this usage information.\n"
            "  -d --daemon       Run as a daemon\n"
            "  -c --conf <file>  Config file\n");

    exit(exit_code);
}


int main(int argc, char **argv)
{

    int daemon = 0;
    pid_t   pid, sid;
    std::string config_file = "./qfetcher.conf";

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

    signal(SIGHUP, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);
    signal(SIGQUIT, signal_handler);

    google::InitGoogleLogging(argv[0]);

    std::string url_queue_host = "localhost";
    uint16_t url_queue_port = 19854;

    std::string crawled_queue_name = "crawled";
    std::string crawled_queue_host = "localhost";
    uint16_t crawled_queue_port = 7676;

    int multiple = 100;
    int enable_proxy = 0;
    std::string proxy_host = "localhost";
    uint16_t proxy_port = 3128;



    qcontent::QContentConfig *config = qcontent::QContentConfig::get_instance();
    if (!config->parse_file(config_file)) {
        LOG(FATAL) << "Parse config file error";
    }

    url_queue_host = config->get_string("url_queue.host", url_queue_host);
    url_queue_port = (uint16_t) config->get_integer("url_queue.port", url_queue_port);

    crawled_queue_name = config->get_string("crawled_queue.name", crawled_queue_name);
    crawled_queue_host = config->get_string("crawled_queue.host", crawled_queue_host);
    crawled_queue_port = (uint16_t) config->get_integer("crawled_queue.port", crawled_queue_port);

    multiple = config->get_integer("qfetcher.multiple", multiple);
    enable_proxy = config->get_integer("qfetcher.enable_proxy", enable_proxy);
    proxy_host = config->get_string("qfetcher.proxy_host", proxy_host);
    proxy_port = config->get_integer("qfetcher.proxy_port", proxy_port);

    QCoreApplication app(argc, argv);

    if (enable_proxy) {
        QNetworkProxy proxy(QNetworkProxy::HttpProxy, QString::fromUtf8(proxy_host.c_str()), (quint16)proxy_port);
        QNetworkProxy::setApplicationProxy(proxy);
    }

    qcontent::UrlQueue urlqueue(url_queue_host, url_queue_port);
    qcontent::HubQueue crawled_queue(crawled_queue_host.c_str(), crawled_queue_port, crawled_queue_name);

    qcontent::QStore qstore(config);
    qcontent::QFetcher fetcher(&urlqueue, &crawled_queue, &qstore, multiple);

    global_qfetcher = &fetcher;

    fetcher.start();

    app.exec();
    LOG(INFO) << "stop qfetcher finished";

}
