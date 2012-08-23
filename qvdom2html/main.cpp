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
#include <libmemcached/memcached.h>
#include "qvdom2html.h"

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
        //vdom_content = vdom_content.substr(0, vdom_content.size() - 1);
    }

    in_file.close();
}

int main(int argc, char* argv[])
{
    string file;
    if (argc < 2) {
        file = "./1.vdom";
    } else {
        file = argv[1];
    }

    std::string vdom_content;
    if (file.find(".vdom") == std::string::npos) {
        memcached_st *vdom_store;
        vdom_store  = memcached_create(NULL);
        memcached_server_st *servers;
        servers= memcached_servers_parse("l-crwl3:9890,l-crwl4:9890,l-crwl5:9890,l-crwl6:9890");
        if (servers != NULL) {
        memcached_server_push(vdom_store, servers);
        memcached_server_list_free(servers);
        } else {
            std::cerr << "Error server config error" << std::endl;
            std::cerr << "Usage: qurluniqmgr set|get servers" << std::endl;
            exit(1);
        }

        uint32_t flags;
        memcached_return rc;
        size_t value_length;

        char * value = memcached_get (vdom_store,
                          file.c_str(), file.size(),
                          &value_length,
                          &flags,
                          &rc);
        if (value) {
            vdom_content.append(value, value_length);
        } else {
            std::cerr << "Error get vdom from vdom store" << std::endl;
        }
    } else {
        get_vdom_from_file(file.c_str(), vdom_content);
    }

    vdom::Window win;
    win.ParseFromString(vdom_content);
    //::google::protobuf::TextFormat::ParseFromString(vdom_content, &win);

    std::string html;
    qcontent::QVDom2Html::vdom_to_html(win, html);
    std::cout << html;

    return 0;
}
