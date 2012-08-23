/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 *
 */

#include <getopt.h>
#include <stdlib.h>
#include <strtk.hpp>
#include <googleurl/src/gurl.h>

#include <iostream>
#include <vector>
#include <list>
#include <QHash>
#include <QCryptographicHash>

#include <qcontent_record.h>

#include "libqcontenthub/libqcontenthub.h"
#include "libqurlqueue/libqurlqueue.h"
#include "qhost2site/qhost2site.h"

#define QH_OK QCONTENTHUB_OK
#define QH_AGAIN QCONTENTHUB_AGAIN
#define QH_WARN QCONTENTHUB_WARN
#define QH_ERROR QCONTENTHUB_ERROR
#define QH_END QCONTENTHUB_END

static std::string md5_hash(const std::string &data);
static int process(qcontent::HubClient &uc,  const std::string &line);

int stats(qcontent::HubClient &uc, const std::string &args)
{
    std::string content;
    int ret = uc.stats(content);
    if (ret == QH_OK) {
        std::cout << content << "\n";
    }

    return ret;
}

int set_capacity(qcontent::HubClient &uc, const std::string &args)
{
    std::string queue;
    int capacity;

    bool parse_ret = strtk::parse(args, "\t", queue, capacity);
    if (!parse_ret) {
        return QH_ERROR;
    }

    int ret = uc.set_queue_capacity(queue, capacity);
    return ret;
}

int dump_extracted(qcontent::HubClient &uc, const std::string &args)
{
    int max_count = 100;
    strtk::parse(args, "\t", max_count);
    std::string content;
    int cnt = 0;
    int ret = QH_OK;
    while (ret == QH_OK) {
        ret = uc.pop_queue("extracted", content);
        if (ret == QH_OK) {
            cnt++;
            std::cout << content << "\n";
        }

        if (cnt >= max_count) {
            break;
        }
    }
    if (ret == QH_ERROR) {
        return ret;
    } else {
        return QH_OK;
    }
}

int dump_new_url(qcontent::HubClient &uc, const std::string &args)
{
    int max_count = 1000;
    strtk::parse(args, "\t", max_count);
    std::string content;
    int cnt = 0;
    int ret = QH_OK;
    while (ret == QH_OK) {
        ret = uc.pop_queue("new_url", content);
        if (ret == QH_OK) {
            cnt++;
            msgpack::unpacker pac;
            pac.reserve_buffer(content.size());
            memcpy(pac.buffer(), content.data(), content.size());
            pac.buffer_consumed(content.size());
            msgpack::unpacked result;
            while(pac.next(&result)) {
                std::string rstr;
                result.get().convert(&rstr);
                std::cout << rstr << "\n";
            }
        }

        if (cnt >= max_count) {
            break;
        }
    }
    if (ret == QH_ERROR) {
        return ret;
    } else {
        return QH_OK;
    }
}


static std::string md5_hash(const std::string &data)
{
    return std::string(QCryptographicHash::hash(QByteArray(data.c_str(), data.size()),
                QCryptographicHash::Md5).toHex().constData());
}

static void print_usage(FILE* stream, int exit_code) {
    fprintf(stream, "Usage: hubmgr options \n");
    fprintf(stream,
            "  -h --host <host>  Host.\n"
            "  -p --port <port>  Port.\n");

    exit(exit_code);
}

int main(int argc, char *argv[])
{
    std::string host = "localhost";
    uint16_t port = 7676;

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
                host = optarg;
                break;
            case 'p':
                port = (uint16_t)atoi(optarg);
                break;
            case -1:
                break;
            case '?':
                print_usage(stderr, 1);
            default:
                print_usage(stderr, 1);
        }
    } while (next_option != -1);


    qcontent::HubClient uc(host, port);

    int lineno = 0;
    std::string line;
    while (std::getline(std::cin, line)) {
        lineno++;
        if (line.size() == 0) {
            continue;
        }

        int ret = QH_OK;
        do {
            if (ret == QH_AGAIN) {
                sleep(1);
            }
            ret = process(uc, line);
            if (ret == QH_WARN) {
                std::cerr << "warn line " << lineno << " line: " << line << std::endl;
            } else if (ret == QH_ERROR) {
                std::cerr << "error line " << lineno << " line: " << line << std::endl;
            }
        } while ( ret == QH_AGAIN);
    }

    return 0;
}

int process(qcontent::HubClient &uc, const std::string &line)
{
    // split
    std::string cmd;
    std::string args;

    size_t pos = line.find("\t");
    if (pos != std::string::npos) {
        cmd = line.substr(0, pos);
        args = line.substr(pos + 1);
    } else {
        cmd = line;
    }

    if (cmd.size() == 0) {
        fprintf(stderr, "null comamnd\n");
        return QH_WARN;
    } else if (cmd == "stats" || cmd == "stat_all") {
        return stats(uc, args);
    } else if (cmd == "set_capacity") {
        return set_capacity(uc, args);
    } else if (cmd == "dump_extracted") {
        return dump_extracted(uc, args);
    } else if (cmd == "dump_new_url") {
        return dump_new_url(uc, args);
    } else if (cmd == "quit") {
        exit(0);
    } else {
        fprintf(stderr, "Unspported comamnd: %s\n", cmd.c_str());
        return QH_WARN;
    }

}
