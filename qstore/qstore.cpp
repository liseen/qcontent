/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

#include <glog/logging.h>

#include "qstore.h"
#include <cassert>

namespace qcontent {

static void get_host_port_from_server(const std::string &server, uint16_t default_port, std::string &host, uint16_t &port)
{
    const char *pos = strchr(server.c_str(), ':');
    if (pos == NULL) {
        host = server;
        port = default_port;
    } else {
        host = server.substr(0, pos - server.c_str());;
        port = (uint16_t)atoi(pos + 1);
    }
}


QStore::QStore(QContentConfig *config)
{
    std::string url_seen_server = "localhost:9850";
    std::string html_db_server = "localhost:9860";
    std::string record_db_server = "localhost:9870";
    std::string media_db_server = "localhost:9880";

    url_seen_server = config->get_string("qstore.url_seen_db_server", url_seen_server);
    html_db_server = config->get_string("qstore.html_db_server", html_db_server);
    record_db_server = config->get_string("qstore.record_db_server", record_db_server);
    media_db_server = config->get_string("qstore.media_db_server", media_db_server);

    need_url_seen_db = (bool)config->get_integer("qstore.need_url_seen_db", 1);
    need_record_db = (bool)config->get_integer("qstore.need_record_db", 1);
    need_html_db = (bool)config->get_integer("qstore.need_html_db", 1);
    need_media_db = (bool)config->get_integer("qstore.need_media_db", 1);

    init(need_url_seen_db, url_seen_server, need_html_db, html_db_server,
            need_record_db, record_db_server, need_media_db, media_db_server);

}

void QStore::init(bool need_url_seen_db, const std::string &url_seen_server,
        bool need_html_db, const std::string &html_db_server,
        bool need_record_db, const std::string &record_db_server,
        bool need_media_db, const std::string &media_db_server)
{
    if (need_url_seen_db) {
        url_seen_memc  = memcached_create(NULL);
        url_seen_servers = memcached_servers_parse(url_seen_server.c_str());
        if (url_seen_servers != NULL) {
            memcached_return_t rc = memcached_server_push(url_seen_memc, url_seen_servers);
            memcached_server_list_free(url_seen_servers);
            if (rc != MEMCACHED_SUCCESS) {
                LOG(FATAL) << "open url seen db " << url_seen_server << " fail " << memcached_strerror(url_seen_memc, rc);
            }
        } else {
            LOG(FATAL) << "url seen servers server config error " << url_seen_server;
        }
    }
/*
    if (need_html_db) {
        get_host_port_from_server(html_db_server, 9860, html_db_host, html_db_port);
        html_db = tcrdbnew();
        if(!tcrdbopen(html_db, html_db_host.c_str(), html_db_port)){
            int ecode = tcrdbecode(html_db);
            LOG(FATAL) << "open html db " << html_db_server << " fail " << tcrdberrmsg(ecode);
        } else {
            LOG(INFO) << "open html db " << html_db_server << " okay";
        }
    }
*/
    if (need_html_db) {
        html_memc  = memcached_create(NULL);
        html_servers = memcached_servers_parse(html_db_server.c_str());
        if (html_servers != NULL) {
            memcached_return_t rc = memcached_server_push(html_memc, html_servers);
            memcached_server_list_free(html_servers);
            if (rc != MEMCACHED_SUCCESS) {
                LOG(FATAL) << "open html db " << html_db_server << " fail " << memcached_strerror(html_memc, rc);
            }
        } else {
            LOG(FATAL) << "html servers server config error " << html_db_server;
        }
    }

    if (need_record_db) {
        get_host_port_from_server(record_db_server, 9870, record_db_host, record_db_port);
        record_db = tcrdbnew();
        if(!tcrdbopen(record_db, record_db_host.c_str(), record_db_port)){
            int ecode = tcrdbecode(record_db);
            LOG(FATAL) << "open record db " << record_db_server << " fail " << tcrdberrmsg(ecode);
        } else {
            LOG(INFO) << "open record db " << record_db_server << " okay";
        }
    }

    if (need_media_db) {
        get_host_port_from_server(media_db_server, 9880, media_db_host, media_db_port);
        media_db = tcrdbnew();
        if(!tcrdbopen(media_db, media_db_host.c_str(), media_db_port)){
            int ecode = tcrdbecode(media_db);
            LOG(FATAL) << "open media db " << media_db_server << " fail " << tcrdberrmsg(ecode);
        } else {
            LOG(INFO) << "open media db " << media_db_server << " okay";
        }
    }
}

QStore::~QStore()
{
    if (need_url_seen_db) {
        memcached_free(url_seen_memc);
    }

    if (need_html_db) {
        memcached_free(html_memc);
    }

    if (need_record_db) {
        if(!tcrdbclose(record_db)){
            int ecode = tcrdbecode(record_db);
            fprintf(stderr, "close record db error: %s\n", tcrdberrmsg(ecode));
        }
        tcrdbdel(record_db);
    }


    if (need_media_db) {
        if(!tcrdbclose(media_db)){
            int ecode = tcrdbecode(media_db);
            fprintf(stderr, "close media db error: %s\n", tcrdberrmsg(ecode));
        }
        tcrdbdel(media_db);
    }
}


int QStore::process(QContentRecord &record)
{

    if (record.want_type == mimetype::image) {
        assert(need_media_db);
        const std::string &url_md5 = record.url_md5;
        std::string media_key = url_md5 + ".i";
        if (!tcrdbput(media_db, media_key.c_str(), media_key.size(), record.raw_content.c_str(), record.raw_content.size())) {
            int ecode = tcrdbecode(media_db);
            // TODO log
            LOG(ERROR) << "put media url " << record.url << " error " <<  tcrdberrmsg(ecode);
            return -1;
        }
    } else { // default text/html
        assert(need_html_db && need_record_db);
        std::string tmps;

        TCMAP *cols = tcmapnew();
        tcmapput2(cols, "url", record.url.c_str());
        tcmapput2(cols, "host", record.host.c_str());
        tcmapput2(cols, "url_md5", record.url_md5.c_str());
        tcmapput2(cols, "parent_url_md5", record.parent_url_md5.c_str());
        strtk::type_to_string(record.crawl_level, tmps);
        tcmapput2(cols, "crawl_level", tmps.c_str());
        strtk::type_to_string(record.find_time, tmps);
        tcmapput2(cols, "find_time", tmps.c_str());
        tcmapput2(cols, "anchor_text", record.anchor_text.c_str());
        tcmapput2(cols, "crawl_tag", record.crawl_tag.c_str());
        // last download_time
        strtk::type_to_string(record.download_time, tmps);
        tcmapput2(cols, "download_time", tmps.c_str());
        strtk::type_to_string(record.http_code, tmps);
        tcmapput2(cols, "http_code", tmps.c_str());
        if (record.is_list) {
            tcmapput2(cols, "is_list", "1");
        } else {
            tcmapput2(cols, "is_list", "0");
        }

        if (record.crawled_okay) {
            tcmapput2(cols, "crawled_okay", "1");
            tcmapput2(cols, "raw_title", record.raw_title.c_str());
            tcmapput2(cols, "title", record.title.c_str());
            tcmapput2(cols, "keywords", record.keywords.c_str());
            tcmapput2(cols, "description", record.description.c_str());
            tcmapput2(cols, "content", record.content.c_str());
            strtk::type_to_string(record.publish_time, tmps);
            tcmapput2(cols, "publish_time", tmps.c_str());
            tcmapput2(cols, "images", record.images.c_str());
            if (record.is_redirect) {
                tcmapput2(cols, "is_redirect", "1");
            } else {
                tcmapput2(cols, "is_redirect", "0");
            }
            tcmapput2(cols, "redirect_url", record.redirect_url.c_str());
            strtk::type_to_string(record.content_confidence, tmps);
            tcmapput2(cols, "content_confidence", tmps.c_str());
            strtk::type_to_string(record.list_confidence, tmps);
            tcmapput2(cols, "list_confidence", tmps.c_str());
            strtk::type_to_string(record.links_size, tmps);
            tcmapput2(cols, "links_size", tmps.c_str());

            strtk::type_to_string(record.last_modified, tmps);
            tcmapput2(cols, "last_modified", tmps.c_str());
            strtk::type_to_string(record.loading_time, tmps);
            tcmapput2(cols, "loading_time", tmps.c_str());

            strtk::type_to_string(record.new_links_size, tmps);
            tcmapput2(cols, "new_links_size", tmps.c_str());
        } else {
            tcmapput2(cols, "crawled_okay", "0");
        }

        if(!tcrdbtblput(record_db, record.url_md5.c_str(), record.url_md5.size(), cols)){
            int ecode = tcrdbecode(record_db);
            LOG(ERROR) << "put record error " <<  tcrdberrmsg(ecode);
            tcmapdel(cols);
            return -1;
        }
        tcmapdel(cols);

        std::string url_md5_vdom = record.url_md5 + ".v";

        memcached_return_t rc = memcached_set(html_memc, url_md5_vdom.c_str(), url_md5_vdom.size(),
                record.vdom.c_str(), record.vdom.size(), 0, 0);
        if (rc != MEMCACHED_SUCCESS) {
            LOG(ERROR) << "put vdom url " << record.url << " size: " << record.vdom.size()
                       << " error: " << memcached_strerror(html_memc, rc);
            return -1;
        }
/*
        if (!tcrdbput(html_db, url_md5_vdom.c_str(), url_md5_vdom.size(), record.vdom.c_str(), record.vdom.size())) {
            int ecode = tcrdbecode(html_db);
            // TODO log
            LOG(ERROR) << "put vdom url " << record.url << " size: " << record.vdom.size() << " error " <<  tcrdberrmsg(ecode);
            return -1;
        }
*/
    }

    return 0;
}

int QStore::url_crawled(const std::string &url_md5)
{
    memcached_return_t rc = memcached_set(url_seen_memc, url_md5.c_str(), url_md5.length(), "1", 1, 0, 0);
    if (rc == MEMCACHED_SUCCESS) {
        return 0;
    } else {
        LOG(ERROR) << "set url crawled status error " << url_md5;
        return -1;
    }

    return 0;
}

int QStore::url_status(const std::string &url_md5, int &status)
{
    char *value;
    size_t value_length;
    uint32_t flags;
    memcached_return_t rc;

    value = memcached_get(url_seen_memc, url_md5.c_str(), url_md5.length(), &value_length, &flags, &rc);

    if (rc == MEMCACHED_SUCCESS) {
        if (strcmp(value, "1") == 0) {
            status = crawlstatus::CRAWLED;
        } else {
            status = crawlstatus::NOT_CRAWLED;
        }
        free(value);
    } else if (rc == MEMCACHED_NOTFOUND) {
        status = crawlstatus::NOT_SEEN;
    } else {
        LOG(ERROR) << "get url seen status error " << url_md5;
        return -1;
    }

    return 0;
/*
    value = tcrdbget2(url_hash_db, url_md5.c_str());
    if(value){
        status = crawlstatus::CRAWLED;
        free(value);
    } else {
        int ecode = tcrdbecode(url_hash_db);
        if (ecode == 7) {
            status = crawlstatus::NOT_SEEN;
            if ( !tcrdbputkeep2(url_hash_db, url_md5.c_str(), "1") ) {
                int ecode = tcrdbecode(url_hash_db);
                if ( ecode != TTEKEEP) {
                    LOG(ERROR) << "put url seen error" <<  tcrdberrmsg(ecode);
                    return -1;
                }
            }
        } else {
            LOG(ERROR) << "get url seen error" <<  tcrdberrmsg(ecode);
            return -1;
        }
    }

    return 0;
    */
}

bool QStore::get_thumb(const std::string &thumb_key, std::string &thumb)
{
    assert(need_media_db);

    char *value = tcrdbget2(media_db, thumb_key.c_str());
    if (value) {
        thumb.append(value);
        free(value);
        return true;
    } else {
        return false;
    }
}

bool QStore::store_thumb(const std::string &thumb_key, const std::string &thumb)
{
    assert(need_media_db);
    if (!tcrdbput(media_db, thumb_key.c_str(), thumb_key.size(), thumb.c_str(), thumb.size())) {
        int ecode = tcrdbecode(media_db);
        // TODO log
        LOG(ERROR) << "store thumb for thumb key " << thumb_key << " error " <<  tcrdberrmsg(ecode);
        return false;
    }

    return true;
}

} // end namespace qcontent

