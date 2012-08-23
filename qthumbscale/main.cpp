
#include <cstdio>
#include <cstdlib>
#include <getopt.h>
#include <tcrdb.h>
#include <strtk.hpp>

#include <string>
#include <iostream>

#include <glog/logging.h>
#include "qthumbscale.h"

#include <QBuffer>
#include <QCryptographicHash>

#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

static std::string md5_hash(const std::string &data)
{
    return std::string(QCryptographicHash::hash(QByteArray(data.c_str(), \
                    data.size()), QCryptographicHash::Md5).toHex().constData());
}


static void print_usage(FILE* stream, int exit_code) {
    fprintf(stream, "Usage: qthumbscale options \n");
    fprintf(stream,
            "  -h --host <host>  Host.\n"
            "  -p --port <port>  Port.\n");

    exit(exit_code);
}


int main(int argc, char *argv[]) {

    std::string media_host = "crwl5";
    uint16_t media_port = 9880;
    std::string cascade_config = "./cascade_conf.xml";
    double cascade_scale = 1.3;
    std::string debug_file;

    const char* const short_options = "h:p:c:s:d:";
    const struct option long_options[] = {
        { "host",  1, NULL, 'h' },
        { "port",  1, NULL, 'p' },
        { "cascade_conf",  1, NULL, 'c' },
        { "cascade_scale",  1, NULL, 's' },
        { "debug_file",  1, NULL, 'd' },
        { NULL,    0, NULL, 0   }
    };

    int next_option;
    do {
        next_option = getopt_long (argc, argv, short_options,
                               long_options, NULL);
        switch (next_option) {
            case 'h':
                media_host = optarg;
                break;
            case 'p':
                media_port = (uint16_t)atoi(optarg);
                break;
            case 'c':
                cascade_config = optarg;
                break;
            case 's':
                cascade_scale = (double)atof(optarg);
                break;
            case 'd':
                debug_file = optarg;
                break;
            case -1:
                break;
            case '?':
                print_usage(stderr, 1);
            default:
                print_usage(stderr, 1);
        }
    } while (next_option != -1);


    if (!debug_file.empty()) {
        qcontent::QThumbScale qthumb;
        if (!qthumb.init(cascade_config, cascade_scale)) {
            LOG(FATAL) << "init qthumbscale error";
        }

        cv::Mat img = cv::imread(debug_file, 1);
        std::string thumb;
        qthumb.enable_debug();
        qthumb.scale_to_thumb(img, thumb);
        std::cout << "thumb size: " << thumb.size() << std::endl;
        std::cout << "data:image/jpeg;base64," << thumb.substr(thumb.find_last_of(" ") + 1) << std::endl;;
        return 0;
    }

    google::InitGoogleLogging(argv[0]);
    qcontent::QThumbScale qthumb;
    if (!qthumb.init(cascade_config, cascade_scale)) {
        LOG(FATAL) << "init qthumbscale error";
    }

    TCRDB *rdb;
    int ecode;
    char *value;

    rdb = tcrdbnew();
    /* connect to the server */
    if(!tcrdbopen(rdb, media_host.c_str(), media_port)){
        ecode = tcrdbecode(rdb);
        fprintf(stderr, "open media db error: %s\n", tcrdberrmsg(ecode));
        print_usage(stderr, 2);
    }

    int lineno = 0;
    std::string line;
    while (getline(std::cin, line)) {
        lineno++;
        if (line.empty()) {
            continue;
        }

        std::vector<std::string> splits;
        strtk::parse(line.c_str(), line.c_str() + line.size(),  "\t", splits, strtk::split_options::default_mode);
        if (splits.size() < 4) {
            LOG(INFO) << "format error";
            continue;
        }

        std::string md5 = splits[0];
        std::string download_time = splits[1];
        std::string url = splits[2];
        std::string raw_url = splits[3];

        std::string key = md5_hash(raw_url) + ".i";

        int img_raw_content_size = 0;
        value = (char*)tcrdbget(rdb, key.c_str(), key.size(), &img_raw_content_size);
        if (value) {
            // for performance
            if (img_raw_content_size >= 20) {
                std::string thumb;
                if (qthumb.scale_to_thumb((const unsigned char*)value, img_raw_content_size, thumb)) {
                    std::cout << line << "\t" << thumb << "\n";
                } else {
                    LOG(WARNING) << "line : " << lineno << " thumb fail";
                }
            }
            free(value);
        } else {
            ecode = tcrdbecode(rdb);
            if (ecode == 7) {
                fprintf(stderr, "NA\t%s\n", line.c_str());
            } else {
                fprintf(stderr, "get error: %s, data: %s\n", tcrdberrmsg(ecode), line.c_str());
            }
        }
    }

    /* close the connection */
    if(!tcrdbclose(rdb)){
        ecode = tcrdbecode(rdb);
        fprintf(stderr, "close error: %s\n", tcrdberrmsg(ecode));
    }

    /* delete the object */
    tcrdbdel(rdb);

    return 0;
}
