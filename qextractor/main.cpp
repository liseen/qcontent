#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <getopt.h>



#include <glog/logging.h>

#include "qextractor.h"

#include <QApplication>
#include <QNetworkProxy>


static qcontent::QExtractor *global_qextractor = NULL;

static void signal_handler(int sig)
{
    switch(sig) {
        case SIGTERM:
        case SIGHUP:
        case SIGINT:
            if (global_qextractor) {
                global_qextractor->stop();
            }
            break;
        default:
            break;
    }
}

static void print_usage(FILE* stream, int exit_code) {
    fprintf(stream, "Usage: qcontenthubd options \n");
    fprintf(stream,
            "  -h --help               Display this usage information.\n"
            "  -d --daemon             Run as a daemon\n"
            "  -c --conf <file>        Config file\n");

    exit(exit_code);
}


int main(int argc, char *argv[])
{

    int daemon = 0;
    pid_t   pid, sid;
    std::string config_file = "./qextractor.conf";

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


    google::InitGoogleLogging(argv[0]);
    QApplication app(argc, argv);
    app.setApplicationName("qextractor");

    signal(SIGHUP, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);
    signal(SIGQUIT, signal_handler);

    int extract_url_limit = 0;
    int parent_predict_limit = 45;

    std::string url_queue_host = "localhost";
    uint16_t url_queue_port = 19854;

    std::string crawled_queue_name = "crawled";
    std::string crawled_queue_host = "localhost";
    uint16_t crawled_queue_port = 7676;

    std::string extracted_queue_name = "extracted";
    std::string extracted_queue_host = "localhost";
    uint16_t extracted_queue_port = 7676;

    std::string new_url_queue_name = "new_url";
    std::string new_url_queue_host = "localhost";
    uint16_t new_url_queue_port = 7676;


    // html2vdom options
    int enable_proxy = 0;
    std::string proxy_host = "localhost";
    uint16_t proxy_port = 3128;

    // qstore
    std::string url_seen_server = "localhost:9850";
    std::string html_db_server = "localhost:9860";
    std::string record_db_server = "localhost:9870";
    std::string media_db_server = "localhost:9880";

    // qurlfilter
    std::string qurlfilter_conf_file = "./qurlfilter.conf";

    // qthumbscale
    std::string qthumbscale_cascade = "./qthumbscale.conf";
    double qthumbscale_scale = 1.3;

    qcontent::QContentConfig *config = qcontent::QContentConfig::get_instance();
    if (!config->parse_file(config_file)) {
        LOG(FATAL) << "Parse config file error";
        exit(-1);
    }


    extract_url_limit = config->get_integer("qextractor.extract_url_limit", extract_url_limit);
    LOG(INFO) << "extract url limit: " << extract_url_limit;

    parent_predict_limit = config->get_integer("qextractor.parent_predict_limit", parent_predict_limit);
    LOG(INFO) << "parent predict limit: " << parent_predict_limit;


    url_queue_host = config->get_string("url_queue.host", url_queue_host);
    url_queue_port = (uint16_t) config->get_integer("url_queue.port", url_queue_port);

    crawled_queue_name = config->get_string("crawled_queue.name", crawled_queue_name);
    crawled_queue_host = config->get_string("crawled_queue.host", crawled_queue_host);
    crawled_queue_port = (uint16_t) config->get_integer("crawled_queue.port", crawled_queue_port);

    extracted_queue_name = config->get_string("extracted_queue.name", extracted_queue_name);
    extracted_queue_host = config->get_string("extracted_queue.host", extracted_queue_host);
    extracted_queue_port = (uint16_t) config->get_integer("extracted_queue.port", extracted_queue_port);

    new_url_queue_name = config->get_string("new_url_queue.name", new_url_queue_name);
    new_url_queue_host = config->get_string("new_url_queue.host", new_url_queue_host);
    new_url_queue_port = (uint16_t) config->get_integer("new_url_queue.port", new_url_queue_port);

    enable_proxy = config->get_integer("qhtml2vdom.enable_proxy", enable_proxy);
    proxy_host = config->get_string("qhtml2vdom.proxy_host", proxy_host);
    proxy_port = config->get_integer("qhtml2vdom.enable_port", proxy_port);

    /*
    url_seen_server = config->get_string("qstore.url_seen_db_server", url_seen_server);
    html_db_server = config->get_string("qstore.html_db_server", html_db_server);
    record_db_server = config->get_string("qstore.record_db_server", record_db_server);
    media_db_server = config->get_string("qstore.media_db_server", media_db_server);
*/
    qurlfilter_conf_file = config->get_string("qurlfilter.conf_file", qurlfilter_conf_file);

    qthumbscale_cascade = config->get_string("qthumbscale.cascade_conf", qthumbscale_cascade);

    if (enable_proxy) {
        LOG(INFO) << "enabe proxy " << proxy_host << ":" << proxy_port;
        QNetworkProxy proxy(QNetworkProxy::HttpProxy, QString::fromUtf8(proxy_host.c_str()), (quint16)proxy_port);
        QNetworkProxy::setApplicationProxy(proxy);
    }

    qcontent::UrlQueue urlqueue(url_queue_host, url_queue_port);
    qcontent::HubQueue crawled(crawled_queue_host.c_str(), crawled_queue_port, crawled_queue_name, 10000);
    qcontent::HubQueue extracted(extracted_queue_host.c_str(), extracted_queue_port, extracted_queue_name, 10000);
    qcontent::HubQueue new_url(new_url_queue_host.c_str(), new_url_queue_port, new_url_queue_name, 10000);

    qcontent::QHtml2Vdom qhtml2vdom;
    //qcontent::QStore store(url_seen_server, html_db_server, record_db_server, media_db_server);
    qcontent::QStore store(config);

    qcontent::QUrlFilter qurlfilter(qurlfilter_conf_file);

    qcontent::QThumbScale qthumb;
    if (!qthumb.init(qthumbscale_cascade, qthumbscale_scale)) {
        LOG(FATAL) << "init qthumbscale error";
    }

    std::map<std::string, std::string> qclassifier_map;
    config->prefix_values_map("qclassifier", qclassifier_map);

    qcontent::QPredictConfig qclassifier_config(qclassifier_map);
    qcontent::QPredict qclassifier;
    if (!qclassifier.init(&qclassifier_config)) {
        LOG(FATAL) << "init qclassifier fatal";
    }

    if (!qclassifier.load_qpredict_model()) {
        LOG(FATAL) << "qclassifier load fatal";
    }

    std::map<std::string, std::string> qpredict_map;
    config->prefix_values_map("qpredict", qpredict_map);
    qcontent::QPredictConfig qpredict_config(qpredict_map);
    qcontent::QPredict qpredict;
    if (!qpredict.init(&qpredict_config)) {
        LOG(FATAL) << "init qpredict fatal";
    }

    if (!qpredict.load_qpredict_model()) {
        LOG(FATAL) << "qpredict load fatal";
    }

    std::string titlefilter_stat_file = config->get_string("qcontentextractor.qtitlefilter.stat_file", "");

    qcontent::QExtractor qextractor(&crawled, &extracted, &new_url, &urlqueue, \
            &qhtml2vdom, &store, &qurlfilter, &qthumb, &qclassifier, &qpredict,
            titlefilter_stat_file
            );

    if (extract_url_limit > 0) {
       qextractor.set_extract_url_limit(extract_url_limit);
    }

    if (parent_predict_limit > 0) {
       qextractor.set_parent_predict_limit(parent_predict_limit);
    }


    global_qextractor = &qextractor;

    qextractor.start();

    return 0;
}
