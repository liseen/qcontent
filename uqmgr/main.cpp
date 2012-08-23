/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

#include <getopt.h>
#include <stdlib.h>
#include <strtk.hpp>

#include <iostream>
#include <vector>
#include <list>
#include <QHash>
#include <QCryptographicHash>


#include <googleurl/src/gurl.h>
#include <qcontent_record.h>
#include "libqurlqueue/libqurlqueue.h"
#include "qhost2site/qhost2site.h"

#define QH_OK QCONTENTHUB_OK
#define QH_AGAIN QCONTENTHUB_AGAIN
#define QH_WARN QCONTENTHUB_WARN
#define QH_ERROR QCONTENTHUB_ERROR
#define QH_END QCONTENTHUB_END

static std::string md5_hash(const std::string &data);
static int process(qcontent::UrlQueue &uq,  const std::string &line);

int push_url_prepare(const std::string &args,
        int &priority, std::string &site, bool new_url, std::string &url_msg) {

    std::vector<std::string> splits;

    strtk::parse(args.c_str(), args.c_str() + args.size(),  "\t", splits, \
            strtk::split_options::default_mode);

    if (splits.size() < 4) {
        std::cerr << "error format line " << args << std::endl;
        return QH_ERROR;
    }

    // priority, want_type, crawl_type, max_crawl_level, url,
    // condition, crawl_tag, crawl_level, anchor_text, parent_url_md5

    strtk::string_to_type_converter(splits[0], priority);
    std::string &want_type = splits[1];
    std::string &crawl_type = splits[2];

    int max_crawl_level = 1;
    strtk::string_to_type_converter(splits[3], max_crawl_level);
    std::string &url = splits[4];

    std::string condition;
    if (splits.size() >= 6) {
        condition = splits[5];
    }

    std::string crawl_tag;
    if (splits.size() >= 7) {
        crawl_tag = splits[6];
    }

    int crawl_level = 0;
    std::string anchor_text;
    std::string parent_url_md5;
    if (splits.size() >= 10) {
        strtk::string_to_type_converter(splits[7], crawl_level);
        anchor_text = splits[8];
        parent_url_md5 = splits[9];
    }

    GURL gurl(url);
    if (!gurl.is_valid()) {
        std::cerr << "invalid url " << url << std::endl;
        return QH_ERROR;
    }
    if (gurl.spec().size() > 4000) {
        std::cerr << "exceeded limit url " << url << std::endl;
        return QH_ERROR;
    }

    if (crawl_type.size() < 4) {
        std::cerr << "crawl type is not right, url " << url << std::endl;
        return QH_ERROR;
    }

    qcontent::QCrawlerRecord crawl_url;

    crawl_url.url = gurl.spec();
    crawl_url.url_md5 = md5_hash(crawl_url.url);
    crawl_url.host = gurl.host();
    crawl_url.site = qhost2site(crawl_url.host);
    crawl_url.want_type = want_type;
    crawl_url.crawl_type = crawl_type;
    crawl_url.max_crawl_level = max_crawl_level;
    crawl_url.crawl_tag = crawl_tag;

    crawl_url.crawl_level = crawl_level;
    crawl_url.anchor_text = anchor_text;
    crawl_url.parent_url_md5 = parent_url_md5;

    if (crawl_type.at(0) == qcontent::crawlcommand::FIND && !new_url) {
        crawl_url.is_list = true;
    }


    if (condition.empty()) {
        char find_restricted = crawl_type.size() > 2 ? \
                            crawl_type.at(2) : qcontent::crawlrestriction::HOST;
        if (find_restricted == qcontent::crawlrestriction::HOST) {
            crawl_url.crawl_condition = crawl_url.host;
        } else if (find_restricted == qcontent::crawlrestriction::SITE) {
            crawl_url.crawl_condition = crawl_url.site;
        } else if (find_restricted == qcontent::crawlrestriction::ALL) {
            // we don't need crawl_condition in all mode
            // crawl_url.crawl_condition = crawl_url.site;
        } else if (find_restricted == qcontent::crawlrestriction::REGEX) {
            crawl_url.crawl_condition = condition;
        } else  {
            std::cerr << "unkonwn find restricted " << url << std::endl;
            return QH_ERROR;
        }
    } else {
        crawl_url.crawl_condition = condition;
    }


    msgpack::sbuffer sbuf;
	msgpack::pack(sbuf, crawl_url);

    url_msg.append(sbuf.data(), sbuf.size());
    site = crawl_url.site;
    return QH_OK;
}


int push_url(qcontent::UrlQueue &uq, const std::string &args)
{
    int priority = 0;
    std::string site;
    std::string url_msg;

    int ret;
    ret = push_url_prepare(args, priority, site, false, url_msg);
    if (ret == QH_OK) {
        return uq.push_url(priority, site, url_msg);
    }

    return ret;
}

int push_url_front(qcontent::UrlQueue &uq, const std::string &args)
{
    int priority = 1000000; // very large
    std::string site;
    std::string url_msg;

    int ret;
    ret = push_url_prepare(args, priority, site, false, url_msg);
    if (ret == QH_OK) {
        return uq.push_url_front(site, url_msg);
    }

    return ret;
}

int push_new_url(qcontent::UrlQueue &uq, const std::string &args)
{
    int priority = 0;
    std::string site;
    std::string url_msg;

    int ret;
    ret = push_url_prepare(args, priority, site, true, url_msg);
    if (ret == QH_OK) {
        return uq.push_url(priority, site, url_msg);
    }

    return ret;
}

int dump_all(qcontent::UrlQueue &uq, const std::string &/*args*/)
{

    int ret;
    ret = uq.stop_all();
    if (ret != QCONTENTHUB_OK) {
        std::cerr << "start dump all: " << uq.error_str() << std::endl;
        return ret;
    }

    ret = uq.start_dump_all();
    if (ret != QCONTENTHUB_OK) {
        std::cerr << "start dump all: " << uq.error_str() << std::endl;
        return ret;
    }

    while (true) {
        std::string content;
        ret = uq.dump_all(content);
        if (ret == QCONTENTHUB_OK) {
            qcontent::QCrawlerRecord record;
            msgpack::zone zone;
            msgpack::object obj;
            try {
                msgpack::unpack(content.c_str(), content.size(), NULL, &zone, &obj);
                obj.convert(&record);
                std::cout << record.want_type
                    << "\t" << record.crawl_type
                    << "\t" << record.max_crawl_level
                    << "\t" << record.url
                    << "\t" << record.crawl_condition
                    << "\t" << record.crawl_tag << "\n";
            } catch (std::exception& e) {
                // TODO
                std::cerr << "uppack crawler record error " << e.what() << std::endl;
            }

        } else if (ret == QCONTENTHUB_END) {
            ret = QCONTENTHUB_OK;
            break;
        } else {
            std::cerr << "dump all error " << uq.error_str() << std::endl;
            ret = QCONTENTHUB_ERROR;
            break;
        }
    }

    int start_ret = uq.start_all();
    if (start_ret != QCONTENTHUB_OK) {
        std::cerr << "start all: " << uq.error_str() << std::endl;
    }

    return ret;
}

int start_all(qcontent::UrlQueue &uq, const std::string &/*args*/)
{

    int ret;
    ret = uq.start_all();
    if (ret != QCONTENTHUB_OK) {
        std::cerr << "start all: " << uq.error_str() << std::endl;
        return ret;
    }

    return ret;
}


int set_default_interval(qcontent::UrlQueue &uq, const std::string &args)
{
    int interval = 1000;
    strtk::parse(args, "\t", interval);

    int ret = uq.set_default_interval(interval);

    return ret;
}

int set_site_interval(qcontent::UrlQueue &uq, const std::string &args)
{
    std::string site;
    int interval = 1000;
    strtk::parse(args, "\t", site, interval) ||
        strtk::parse(args, "\t", site);

    if (site.size() == 0) {
        std::cerr <<  "empty site" << std::endl;
        return QH_ERROR;
    }

    int ret = uq.set_site_interval(site, interval);

    return ret;
}

int clear_site(qcontent::UrlQueue &uq, const std::string &args)
{
    std::string site;
    //strtk::parse(args, "\t", site);
    site = args;

    if (site.empty()) {
        std::cerr <<  "empty site" << std::endl;
        return QH_ERROR;
    }

    int ret = uq.clear_site(site);

    return ret;
}

int dump_site(qcontent::UrlQueue &uq, const std::string &args)
{
    int ret;
    std::string site;
    //strtk::parse(args, "\t", site);
    site = args;
    if (site.empty()) {
        std::cerr <<  "empty site" << std::endl;
        return QH_ERROR;
    }

    ret = uq.stop_site(site);
    if (ret != QCONTENTHUB_OK) {
        std::cerr << "stop site on dumping site: " << uq.error_str() << std::endl;
    }

    ret = uq.start_dump_site(site);
    if (ret != QCONTENTHUB_OK) {
        std::cerr << "start dump site: " << uq.error_str() << std::endl;
        return ret;
    }

    while (true) {
        std::string content;
        ret = uq.dump_site(site, content);
        if (ret == QCONTENTHUB_OK) {
            qcontent::QCrawlerRecord record;
            msgpack::zone zone;
            msgpack::object obj;
            try {
                msgpack::unpack(content.c_str(), content.size(), NULL, &zone, &obj);
                obj.convert(&record);
                std::cout << record.want_type
                    << "\t" << record.crawl_type
                    << "\t" << record.max_crawl_level
                    << "\t" << record.url
                    << "\t" << record.crawl_condition << "\n";
            } catch (std::exception& e) {
                // TODO
                std::cerr << "uppack crawler record error " << e.what() << std::endl;
            }

        } else if (ret == QCONTENTHUB_END) {
            ret = QCONTENTHUB_OK;
            break;
        } else {
            std::cerr << "dump site error " << uq.error_str() << std::endl;
            ret = QCONTENTHUB_ERROR;
            break;
        }
    }

    int start_ret = uq.start_site(site);
    if (start_ret != QCONTENTHUB_OK) {
        std::cerr << "start site on dumping site: " << uq.error_str() << std::endl;
    }

    return ret;
}


int clear_empty_site(qcontent::UrlQueue &uq, const std::string &/*args*/)
{
    int ret = uq.clear_empty_site();

    return ret;
}

int stat_site(qcontent::UrlQueue &uq, const std::string &args)
{
    std::string site;
    //strtk::parse(args, "\t", site);
    site = args;

    if (site.size() == 0) {
        std::cerr <<  "empty site" << std::endl;
        return QH_ERROR;
    }

    std::string content;
    int ret = uq.stat_site(site, content);
    if (ret == QH_OK) {
        std::cout << content << "\n";
    }

    return ret;
}

int stop_site(qcontent::UrlQueue &uq, const std::string &args)
{
    std::string site;
    //strtk::parse(args, "\t", site);
    site = args;

    if (site.size() == 0) {
        std::cerr <<  "empty site" << std::endl;
        return QH_ERROR;
    }

    return uq.stop_site(site);
}

int start_site(qcontent::UrlQueue &uq, const std::string &args)
{
    std::string site;
    //strtk::parse(args, "\t", site);
    site = args;

    if (site.size() == 0) {
        std::cerr <<  "empty site" << std::endl;
        return QH_ERROR;
    }

    return uq.start_site(site);
}


int dump_ordered_site(qcontent::UrlQueue &uq, const std::string &args)
{
    int limit = 1000;
    strtk::parse(args, "\t", limit);
    std::string content;
    int ret = uq.dump_ordered_site(content, limit);
    if (ret == QH_OK) {
        std::cout << content << "\n";
    }

    return ret;
}


int stats(qcontent::UrlQueue &uq, const std::string &/*args*/)
{
    std::string content;
    int ret = uq.stats(content);
    if (ret == QH_OK) {
        std::cout << content << "\n";
    }

    return ret;
}


static void print_usage(FILE* stream, int exit_code) {
    fprintf(stream, "Usage: uqmgr options \n");
    fprintf(stream,
            "  -h --host <host>  Host.\n"
            "  -p --port <port>  Port.\n");

    exit(exit_code);
}


int main(int argc, char *argv[])
{
    std::string host = "localhost";
    uint16_t port = 19854;

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

    qcontent::UrlQueue uq(host, port);

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
            ret = process(uq, line);
            if (ret == QH_WARN) {
                std::cerr << "warn line " << lineno << " line: " << line << std::endl;
            } else if (ret == QH_ERROR) {
                std::cerr << "error line " << lineno << " line: " << line << std::endl;
            }
        } while ( ret == QH_AGAIN);
    }

    return 0;
}

int process(qcontent::UrlQueue &uq, const std::string &line)
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
    } else if (cmd == "push_url") {
        return push_url(uq, args);
    } else if (cmd == "push_new_url") {
        return push_new_url(uq, args);
    } else if (cmd == "push_url_front") {
        return push_url_front(uq, args);
    } else if (cmd == "start_all") {
        return start_all(uq, args);
    } else if (cmd == "dump_all") {
        return dump_all(uq, args);
    } else if (cmd == "set_default_interval") {
        return set_default_interval(uq, args);
    } else if (cmd == "set_site_interval") {
        return set_site_interval(uq, args);
    } else if (cmd == "clear_empty_site") {
        return clear_empty_site(uq, args);
    } else if (cmd == "clear_site") {
        return clear_site(uq, args);
    } else if (cmd == "stat_site") {
        return stat_site(uq, args);
    } else if (cmd == "start_site") {
        return start_site(uq, args);
    } else if (cmd == "stop_site") {
        return stop_site(uq, args);
    } else if (cmd == "dump_site") {
        return dump_site(uq, args);
    } else if (cmd == "dump_ordered_site") {
        return dump_ordered_site(uq, args);
    } else if (cmd == "stats" || cmd == "stat_all") {
        return stats(uq, args);
    } else if (cmd == "quit") {
        exit(0);
    } else {
        fprintf(stderr, "Unspported comamnd: %s\n", cmd.c_str());
        return QH_WARN;
    }

}

static std::string md5_hash(const std::string &data)
{
    return std::string(QCryptographicHash::hash(QByteArray(data.c_str(), data.size()), \
                QCryptographicHash::Md5).toHex().constData());
}

