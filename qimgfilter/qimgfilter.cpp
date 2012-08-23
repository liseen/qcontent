#include <stdlib.h>
#include <getopt.h>
#include <tcrdb.h>

#include <cmath>
#include <iostream>
#include <vector>
#include <map>

#include <QBuffer>
#include <QImage>
#include <QCryptographicHash>

#include <strtk.hpp>

struct ImgRecord {
    std::string md5;
    std::string url;

    // img attributes
    std::string alt;
    std::string img_raw_url;
    std::string img_raw_content;
    std::string thumb_content;
    std::string thumb_base64;
    int width;
    int height;
};

static std::string md5_hash(const std::string &data);
static int filter_best_image(std::vector<ImgRecord> &img_vec);

static const int THUMB_HEIGHT = 80;

static void print_usage(FILE* stream, int exit_code) {
    fprintf(stream, "Usage: qimgfilter options \n");
    fprintf(stream,
            "  -h --host <host>  Host.\n"
            "  -p --port <port>  Port.\n");

    exit(exit_code);
}

int main(int argc , char *argv[])
{
    std::string media_host = "localhost";
    uint16_t media_port = 9860;

    const char* const short_options = "h:p:";
    const struct option long_options[] = {
        { "host",  1, NULL, 'h' },
        { "port",  1, NULL, 'p' },
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
            case -1:
                break;
            case '?':
                print_usage(stderr, 1);
            default:
                print_usage(stderr, 1);
        }
    } while (next_option != -1);

    TCRDB *rdb;
    int ecode;
    char *value;

    rdb = tcrdbnew();

    /* connect to the server */
    if(!tcrdbopen(rdb, media_host.c_str(), media_port)){
        ecode = tcrdbecode(rdb);
        fprintf(stderr, "open error: %s\n", tcrdberrmsg(ecode));
        print_usage(stderr, 2);
    }

    std::string last_md5;
    std::string line;

    std::vector<ImgRecord> img_vec;

    int lineno = 0;
    while (getline(std::cin, line)) {
        ++lineno;

        if (lineno % 1000 == 0) {
            fprintf(stderr, "completed %d lines\n", lineno);
        }

        ImgRecord ir;

        std::vector<std::string> splits;
        strtk::parse(line.c_str(), line.c_str() + line.size(),  "\t", splits, strtk::split_options::default_mode);
        if (splits.size() < 3) {
            continue;
        }

        ir.url = splits[0];
        ir.alt = splits[1];
        ir.img_raw_url = splits[2];

        ir.md5 = md5_hash(ir.url);

        std::string raw_url_md5_key = md5_hash(ir.img_raw_url) + ".i";

        int img_raw_content_size = 0;
        value = (char*)tcrdbget(rdb, raw_url_md5_key.c_str(), raw_url_md5_key.size(), \
                &img_raw_content_size);
        if(value){
            ir.img_raw_content.append(value, img_raw_content_size);
            free(value);
        } else {
            ecode = tcrdbecode(rdb);
            if (ecode == 7) {
                fprintf(stderr, "NA\t%s\n", line.c_str());
            } else {
                fprintf(stderr, "get error: %s, data: %s\n", tcrdberrmsg(ecode), line.c_str());
            }

            continue;
        }

        // OPTIMISE for first best
        if (last_md5 == ir.md5) {
            continue;
        }

        //std::cout << ir.img_raw_url << "\t" << img_raw_content_size << std::endl;
        QImage img = QImage::fromData((const uchar*)ir.img_raw_content.c_str(), \
                ir.img_raw_content.size());

        if (img.isNull() || img.width() <= 0) {
            continue;
        }

        ir.width = img.width();
        ir.height = img.height();

        float h_ratio = (float)ir.height / THUMB_HEIGHT;

        QImage thumb_img = img.scaled(ir.width / h_ratio, THUMB_HEIGHT,  Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        QByteArray ba;
        QBuffer buffer(&ba);
        buffer.open(QIODevice::WriteOnly);
        thumb_img.save(&buffer, "jpeg");
        ir.thumb_content.append(ba.constData(), ba.size());
        ir.thumb_base64.append("data:image/jpeg;base64,");
        QByteArray base = ba.toBase64();

        ir.thumb_base64.append(base.constData(), base.size());

        if (!last_md5.empty() && ir.md5 != last_md5) {
            int best_pos = filter_best_image(img_vec);
            if (best_pos >= 0) {
                const ImgRecord &bi = img_vec[best_pos];
                std::cout << bi.md5 << "\t"
                      << bi.url << "\t"
                      << bi.img_raw_url << "\t"
                      << bi.width << "\t"
                      << bi.height << "\t"
                      << bi.alt << "\t"
                      << bi.thumb_base64 << "\n";
            }
            last_md5 = ir.md5;
            img_vec.clear();
            img_vec.push_back(ir);
        } else {
            img_vec.push_back(ir);
            last_md5 = ir.md5;
        }
    }

    if (!last_md5.empty()) {
        int best_pos = filter_best_image(img_vec);
        if (best_pos >= 0) {
            const ImgRecord &bi = img_vec[best_pos];
            std::cout << bi.md5 << "\t"
                      << bi.url << "\t"
                      << bi.img_raw_url << "\t"
                      << bi.width << "\t"
                      << bi.height << "\t"
                      << bi.alt << "\t"
                      << bi.thumb_base64 << "\n";
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

static std::string md5_hash(const std::string &data)
{
    return std::string(QCryptographicHash::hash(QByteArray(data.c_str(), \
                    data.size()), QCryptographicHash::Md5).toHex().constData());
}


static int filter_best_image(std::vector<ImgRecord> &img_vec)
{
    if (img_vec.empty()) {
        return -1;
    }

    // OPTIMISE for first best
    if (img_vec.size() >= 1) {
        return 0;
    }

    float min_ratio_diff = 100000;
    int best_pos = -1;

    int size = img_vec.size();
    for (int i = 0; i < size; ++i) {
        const ImgRecord &ir = img_vec[i];

        float ratio_diff = fabs(0.66667 - (float)ir.width / (float) ir.height);
        if (ratio_diff < min_ratio_diff) {
            best_pos = i;
            min_ratio_diff = ratio_diff;
        }
    }

    return best_pos;
}
