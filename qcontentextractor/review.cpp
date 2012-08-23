#include <fstream>
#include <iostream>
#include <vdom.h>
#include <tcrdb.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include <strtk.hpp>
#include <vector>
#include <QTime>
#include <qurlfilter.h>
#include <googleurl/src/gurl.h>

#include <google/protobuf/text_format.h>

#include "vdom_content_extractor.h"

using namespace std;

static int mytcadbmulidx(const void *kbuf, int ksiz){
    uint32_t hash = 20090810;
    const char *rp = (char *)kbuf + ksiz;
    while(ksiz--){
        hash = (hash * 29) ^ *(uint8_t *)--rp;
    }
    return hash % 16;
}

struct MD5Rec {
    MD5Rec(const std::string &m, const std::string &k, const std::string &u) : md5(m), key(k), url(u), idx(0) {}
    std::string md5;
    std::string key;
    std::string url;
    int idx;
};

bool myidxsort(const MD5Rec &s1, const MD5Rec &s2) {
    return s1.idx > s2.idx;
}


int main(int argc, char* argv[])
{
    std::string host;
    if (argc <= 1) {
        host = "crwl4";
    } else {
        host = argv[1];
    }

    TCRDB *rdb;
    int ecode;
    char *value;

    /* create the object */
    rdb = tcrdbnew();

    /* connect to the server */
    if(!tcrdbopen(rdb, host.c_str(), 9860)){
        ecode = tcrdbecode(rdb);
        fprintf(stderr, "open error: %s\n", tcrdberrmsg(ecode));
    }

    vdom::content::Extractor extractor(false, false);

    qcontent::QUrlFilter qurlfilter("/opt/qcontent/etc/qurlfilter.conf");

    std::vector<MD5Rec> key_vec;
    std::string line;
    while (getline(cin, line)) {
        std::string md5;
        std::string url;

        if (!strtk::parse(line, "\t", md5, url)) {
            fprintf(stderr, "strtk parse format error %s\n", md5.c_str());
            continue;
        }

        if (qurlfilter.filter(url).empty()) {
            continue;
        }

        std::string vdom_key = md5 + ".v";
        key_vec.push_back(MD5Rec(md5, vdom_key, url));
    }

    for (std::vector<MD5Rec>::iterator it = key_vec.begin(); it != key_vec.end(); ++it) {
        it->idx = mytcadbmulidx(it->key.c_str(), it->key.size());
    }

    sort(key_vec.begin(), key_vec.end(), myidxsort);

    for (std::vector<MD5Rec>::iterator it = key_vec.begin(); it != key_vec.end(); ++it) {
        const std::string &vdom_key = it->key;
        std::string vdom_content;

        int vdom_content_size;
        value = (char*)tcrdbget(rdb, vdom_key.c_str(), vdom_key.size(), &vdom_content_size);
        if(value){
            vdom_content.append(value, vdom_content_size);
            free(value);
        } else {
            ecode = tcrdbecode(rdb);
            fprintf(stderr, "get error: %s\n", tcrdberrmsg(ecode));
            continue;
        }

        vdom::Window win;
        win.ParseFromString(vdom_content);
        vdom::content::Result ret;
        extractor.extract(&win, 0, ret);
        if (ret.extracted_okay) {
            ret.review_print(it->md5);
        } else {
            fprintf(stderr, "%s\t%s\textracted failed\n", it->md5.c_str(), it->url.c_str());
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
