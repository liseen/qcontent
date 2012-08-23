/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

#ifndef QSTORE_H
#define QSTORE_H

#include <string.h>
#include <stdlib.h>

#include <tcrdb.h>
#include <libmemcached/memcached.h>

#include <strtk.hpp>
#include <qcontent_config.h>
#include <qcontent_processor.h>

namespace qcontent {

class QStore : public QContentProcessor
{
public:
    QStore(QContentConfig *config);
    virtual ~QStore();
    virtual int process(QContentRecord &record);

    int url_crawled(const std::string &md5);
    int url_status(const std::string &md5, int &seen);

    bool get_thumb(const std::string &thumb_key, std::string &thumb);
    bool store_thumb(const std::string &thumb_key, const std::string &thumb);
protected:

    void init(bool need_url_seen_db, const std::string &url_seen_server,
        bool need_html_db, const std::string &html_db_server,
        bool need_record_db, const std::string &record_db_server,
        bool need_media_db, const std::string &media_db_server);

private:
    bool need_url_seen_db;
    memcached_st *url_seen_memc;
    memcached_server_st *url_seen_servers;

    /*
    bool need_html_db;
    TCRDB *html_db;
    std::string html_db_host;
    uint16_t html_db_port;
*/
    bool need_html_db;
    memcached_st *html_memc;
    memcached_server_st *html_servers;


    bool need_record_db;
    TCRDB *record_db;
    std::string record_db_host;
    uint16_t record_db_port;


    /*
    TCRDB *url_hash_db;
    std::string url_hash_db_host;
    uint16_t url_hash_db_port;
    */

    bool need_media_db;
    TCRDB *media_db;
    std::string media_db_host;
    uint16_t media_db_port;
};

} // end namespace qcontent

#endif
