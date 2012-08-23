/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

#include <fstream>
#include <iostream>
#include <vdom.h>
#include <tcrdb.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include <google/protobuf/text_format.h>

#include "vdom_content_extractor.h"

using namespace std;

void get_vdom_from_file(const char *file, std::string &vdom_content)
{
    ifstream in_file;
    in_file.open(file);
    if (!in_file) {
        std::cerr << "cannot open vdom file " << file << std::endl;
        exit(1);
    }
    std::string line;
    while (getline(in_file, line)) {
        vdom_content.append(line);
        vdom_content.append("\n");
    }

    if (vdom_content.size() >= 1) {
        vdom_content = vdom_content.substr(0, vdom_content.size() - 1);
    }

    in_file.close();
}

void get_vdom_from_remote(const char *host, uint16_t port, const std::string &md5, std::string &vdom_content)
{
    std::string vdom_key = md5 + ".v";

    TCRDB *rdb;
    int ecode;
    char *value;

    rdb = tcrdbnew();
    if(!tcrdbopen(rdb, host, port)){
        ecode = tcrdbecode(rdb);
        fprintf(stderr, "open error: %s\n", tcrdberrmsg(ecode));
        exit(1);
    }

    int vdom_content_size;
    value = (char*)tcrdbget(rdb, vdom_key.c_str(), vdom_key.size(), &vdom_content_size);
    if(value){
        vdom_content.append(value, vdom_content_size);
        free(value);
    } else {
        ecode = tcrdbecode(rdb);
        fprintf(stderr, "get error: %s\n", tcrdberrmsg(ecode));
        exit(1);
    }

    if(!tcrdbclose(rdb)){
        ecode = tcrdbecode(rdb);
        fprintf(stderr, "close error: %s\n", tcrdberrmsg(ecode));
    }

    tcrdbdel(rdb);
}


int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cerr << "argv error" << std::endl;
    }
    std::string vdom_content;
    std::string cmd(argv[1]);
    if (cmd == "get_vdom") {
        assert(argc >= 3);
        std::string md5 = argv[2];
        get_vdom_from_remote("crwl4", 9860, md5, vdom_content);
        vdom::Window win;
        win.ParseFromString(vdom_content);
        std::cout << win.Utf8DebugString();

        return 0;
    } else {
        get_vdom_from_file(argv[1], vdom_content);
    }

    vdom::Window win;
    ::google::protobuf::TextFormat::ParseFromString(vdom_content, &win);
    //vdom::Document *doc = win.mutable_doc();

    //vdom::content::Extractor extractor;
    vdom::content::Extractor extractor("./qtitlefilter_stat.txt");
    vdom::content::Result ret;
    extractor.extract(&win, 0, ret);
    if (ret.extracted_okay) {
        std::cout << "\n\n\n=== Test 1";
        std::cout << "\n--- url\n" << win.location();
        std::cout << "\n--- raw_title\n" << ret.raw_title;
        ret.test_print();
        //ret.review_print(win.location());
    }

    return 0;
}
