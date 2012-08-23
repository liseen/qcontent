#ifndef QCONTENT_RECORD_H
#define QCONTENT_RECORD_H

#include <string>
#include <list>
#include <msgpack.hpp>

//#include <vdom.h>
// declare vdom::Window
namespace vdom {
    class Window;
}


#define get_self_restriction(crawl_type) \
    crawl_type.size() > 1 ? crawl_type.at(1) : crawlrestriction::UPDATE;

#define get_find_restriction \
    crawl_type.size() > 2 ? crawl_type.at(2) : crawlrestriction::HOST;

#define get_find_self_restriction \
    crawl_type.size() > 3 ? crawl_type.at(3) : crawlrestriction::INSERT;



namespace qcontent {

namespace crawlcommand {
    const char GAIN = 'g';
    const char FIND = 'f';
}

namespace crawlrestriction {
    const char HOST = 'h';
    const char SITE = 's';
    const char ALL = 'a';
    const char REGEX = 'r';
    const char UPDATE = 'u';
    const char INSERT = 'i';
}



namespace mimetype {
    const std::string image = "image";
    const std::string html = "html";
    const std::string thumb = "thumb";
    const std::string pdf = "pdf";
}

namespace crawlstatus {
    const int NOT_SEEN = 0;
    const int NOT_CRAWLED = 1;
    const int CRAWLED = 2;
    const int NOT_NEED = 3;
    const int DISALLOWED = 4;
}

namespace qrecordstatus {
    const int CRAWLED_FAIL_NOT_FOUND = 10;
    const int CRAWLED_FAIL_TOO_MANY_REDIRECT = 11;
    const int CRAWLED_FAIL_SERVER_ERROR = 12;
    const int CRAWLED_FAIL_TIMEOUT = 13;
    const int CRAWLED_FAIL_DNS_ERROR = 14;
    const int CRAWLED_FAIL_CONNECT_ERROR = 15;
    const int CRAWLED_FAIL_FORMAT_ERROR = 16;

    const int CRAWLED_FAIL_DISALLOWED = 18;
    const int CRAWLED_FAIL_OTHER = 19;

    const int QHTML2VDOM_OTHER = 21;

    const int QDEDUP_DEDUPED = 31;
}

namespace language {
    const std::string chinese = "zh";
    const std::string japanese = "ja";
    const std::string korean = "ko";
    const std::string english = "en";
    const std::string french = "fr";
    const std::string german = "de";
    const std::string unknown= "UN";
}

struct QCrawlerRecord {
    QCrawlerRecord(): \
        status(0),\
        crawled_okay(false),\
        crawl_status(0),\
        crawl_level(0),\
        max_crawl_level(1),\
        is_redirect(false),\
        redirect_times(0),\
        is_list(false),\
        find_time(0),\
        crawl_failed_times(0),\
        http_code(0),\
        last_modified(0),\
        download_time(0),\
        loading_time(0),\
        win(NULL)
    {}
public:
    int status;
    std::string url;
    std::string crawl_type;
    bool crawled_okay;
    int crawl_status;
    int crawl_level;
    int max_crawl_level;
    bool is_redirect;
    int redirect_times;
    bool is_list;
    std::string redirect_url;
    std::string want_type;
    std::string crawl_condition;
    std::string crawl_tag;

    std::string host;
    std::string site;

    int find_time;
    std::string url_md5;
    std::string parent_url_md5;
    std::string anchor_text;

    int crawl_failed_times;
    int http_code;
    std::string mime_type;
    int last_modified;
    int expires;
    int download_time;
    int loading_time;

    std::string raw_content;
    std::string raw_charset;
    std::string raw_html;
    std::string html;
    std::string vdom;

    std::list<std::string> sub_urls;
    std::list<std::string> new_urls;

	MSGPACK_DEFINE(\
            url,\
            crawl_type,\
            crawled_okay,\
            crawl_status,\
            crawl_level,\
            max_crawl_level,\
            want_type,\
            crawl_condition,\
            is_redirect,\
            redirect_times,\
            redirect_url,\
            is_list,\
            crawl_tag,\
            host,\
            site,\
            url_md5,\
            parent_url_md5,\
            anchor_text,\
            status,\
            mime_type,\
            last_modified,\
            expires,\
            download_time,\
            loading_time,\
            raw_content,\
            raw_charset,\
            raw_html,\
            html,\
            vdom
        );

    vdom::Window* win;
};

struct QContentRecord : public QCrawlerRecord {
    QContentRecord(): \
        version(1), \
        content_confidence(0),\
        list_confidence(0),\
        publish_time(0),\
        links_size(0),\
        new_links_size(0),\
        deduped(false),\
        page_rank(0)
    {}

    //QCrawlerRecord crawl_record;

    int version;

    // the confidence of a page's main content, now only two values: 50, 100
    int content_confidence;

    // the confidence that a page is a list page
    int list_confidence;

    // extracted publish time
    int publish_time;

    // page title metadata
    std::string raw_title;

    // page keywords metadata
    std::string keywords;

    // page description metadata
    std::string description;

    // extracted title, content, images
    std::string title;
    std::string content;
    std::string images;

    // attributes for list page
    // all links size in same site or host
    int links_size;
    // new links size in same site or host
    int new_links_size;

    // is deduped?
    bool deduped;

    // the confidence that a page is about travel, for qdcp
    int travel_confidence;

    int page_rank;

    // qnc result
    float qnc_cat;
    float qnc_chi;
    float qnc_zhu;
    float qnc_xing;
    float qnc_you;
    float qnc_gou;
    float qnc_le;

    // language identify
    std::string lan;

	MSGPACK_DEFINE(\
            version,\
            status,\
            crawled_okay,\
            crawl_status,\
            download_time,\
            host,\
            url_md5,\
            url,\
            parent_url_md5,\
            loading_time,\
            publish_time,\
            content_confidence,\
            is_list,\
            links_size,\
            new_links_size,\
            list_confidence,\
            page_rank,\
            deduped,\
            lan,\
            travel_confidence,\
            anchor_text,\
            title,\
            keywords,\
            description,\
            content,\
            images
        );
};

} // end namespace qcontent
#endif
