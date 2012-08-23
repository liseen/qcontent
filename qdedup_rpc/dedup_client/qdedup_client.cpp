/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 *    zhenbao.zhou (周桢堡)  <zhenbao.zhou@qunar.com>
 *
 * Client for qdedup
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <inttypes.h>
#include <sys/mman.h>
#include <getopt.h>
#include <strtk.hpp>
#include <vector>
#include <cassert>
#include <string>
#include <assert.h>
#include <glog/logging.h>

#include <city.h>
#include "qdedup_common.h"
#include "qcontent_record.h"
#include "qdedup_client.h"

namespace qcontent {

QDedupClient::QDedupClient(std::vector<qdedup_addr> addr_vector, int t) {
    timeout = t;
    client_vector.clear();
    qdedup_addr_vector = addr_vector;
}

// get the usage of total mmaps
void QDedupClient::check_usage() {
    uint32_t total_usage = 0;
    uint32_t total_size = 0;
    for (uint32_t i = 0; i < client_vector.size(); i++) {
        mmap_stat_t mmap_stat = client_vector[i]->call("check_usage")
                                .get<mmap_stat_t>();
        uint32_t usage = mmap_stat.mmap_usage;

        // changes it to MB
        uint64_t file_size = mmap_stat.mmap_file_size / (1024 * 1024);
        std::cout << " the usage of " << " mmap is :"
                  << usage << "%" << std::endl;
        std::cout << " the size of " << " mmap is :"
                  << file_size << "MB" << std::endl;
        total_usage += usage;
        total_size += file_size;
    }

    int avg = total_usage / client_vector.size();
    std::cout << "Avergae usage is " << avg << "%" << std::endl;
    std::cout << "All mmap size is " << total_size << "MB" << std::endl;
}

/*
 *returns:
 * IS_NEW_CONTENT
 * IS_DEDUP_CONTENT
 */
int QDedupClient::dedup_check(const char *prefix, size_t prefix_size,
                              const char *content, size_t content_size) {
    const char *prev_begin = content;
    const char *p = content;
    const char *end_p = content + content_size;

    int total_record = 0;
    bool enough_sentence = false;

    std::vector<str_finger_t> finger_vec;

    uint64_t site_seed = CityHash64(prefix, prefix_size);
    
    while (p < end_p) {
      if (total_record > MAX_SENTENCES - 1) {
            enough_sentence = true;
            break;
        }

        int punct_size = start_with_punct(p);
        if (punct_size == 0) {
            p++;
            continue;
        } else {
            uint32_t size = p - prev_begin;
            if (size > 20) {
                uint64_t hash  = CityHash64WithSeed(content, content_size, site_seed);
                // uint64_t hash = city_hash_wrapper(prev_begin, size,
                //                                   prefix, prefix_size);
                str_finger_t finger;
                finger.dedup_mark = hash;
                finger.content_size = size;
                finger_vec.push_back(finger);
            }

            p = p + punct_size;
            prev_begin = p;
        }
    }

    uint32_t size = p - prev_begin;
    if (!enough_sentence && size > 20) {
        uint64_t hash  = CityHash64WithSeed(content, content_size, site_seed);

        // uint64_t hash = city_hash_wrapper(prev_begin, size, prefix, prefix_size);
        // uint64_t hash = hash_bytes(prev_begin, size, hash_base, host_base)
        str_finger_t finger;
        finger.dedup_mark = hash;
        finger.content_size = size;
        finger_vec.push_back(finger);
    }

    msgpack::rpc::client *client = get_client(site_seed);
    int rpc_re = client->call("dedup", finger_vec).get<int>();

    return rpc_re;
}

void QDedupClient::halt_server() {
    for (uint32_t i = 0; i < client_vector.size(); i++) {
        LOG(INFO) << "halt server:" << i;
        int re = client_vector[i]->call("stop").get<int>();
    }
}


const std::vector<qdedup_addr>& QDedupClient::get_qdedup_addr() const {
    return qdedup_addr_vector;
}

int QDedupClient::get_timeout() {
    return timeout;
}

void QDedupClient::set_timeout(int t) {
    timeout = t;
    for (int i = 0; i < client_vector.size(); i++) {
        assert(client_vector[i] != NULL);
        client_vector[i]->set_timeout(timeout);
    }
}

QDedupClient::~QDedupClient() {
    close_client();
}

/******************  private functions ***************************/

int QDedupClient::start_with_punct(const char *str) {
    switch (*str) {
//        case ',':
        case '.':
//        case ':':
        case '!':
        case '?':
        case ';':
        case '\n':
        case '\x01':
            return 1;
    };

    typedef struct {
        const char * punct;
        int size;
    } cn_punct;

    static cn_punct cn_puncts[] = {
        { "。", sizeof("。") - 1},
        { "！", sizeof("！") - 1},
        { "？", sizeof("？") - 1},
        { "；", sizeof("？") - 1},
        { "·", sizeof("·") - 1},
         //       { "，", sizeof("，") - 1 },
        // { "“", sizeof("“")  - 1},
        // { "”", sizeof("”")  - 1},
        // { "、", sizeof("、") - 1 },
        { NULL, 0}
    };

    for (cn_punct *it = cn_puncts; it->punct != NULL; it++) {
        if (strncmp(str, it->punct,  it->size) == 0) {
            return it->size;
        }
    }

    return 0;
}

int QDedupClient::init_client() {
    int size = qdedup_addr_vector.size();

    for (int i = 0; i < size; i++) {
        msgpack::rpc::client* c;
        try {
            c = new msgpack::rpc::client(qdedup_addr_vector[i].host,
                                         qdedup_addr_vector[i].port);
            c->set_timeout(timeout);
        } catch(const std::exception &e) {
            std::cerr << "error in connect "
                      << qdedup_addr_vector[i].host << ":"
                      << qdedup_addr_vector[i].port << " .Error msg:"
                      << e.what() << std::endl;
            return 1;
        }

        client_vector.push_back(c);
    }

    return 0;
}

msgpack::rpc::client* QDedupClient::get_client(uint64_t site_seed) {

    int client_num = client_vector.size();
    int index = site_seed % client_num;
    assert(client_vector[index] != NULL);
    return client_vector[index];
}


void QDedupClient::close_client() {
    for (uint32_t i = 0; i < client_vector.size(); i++) {
        delete client_vector[i];
    }
}
}  //  end of namespace qcontent
