/*
 *  initialize the qurluniq server
 */

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <libmemcached/memcached.h>

#define BATCH_SIZE 20

int main(int argc, char *argv[])
{
    if (argc < 3) {
        std::cerr << "Usage: qurluniqmgr set|get servers" << std::endl;
        exit(1);
    }

    std::string cmd = argv[1];
    std::string server = argv[2];

    memcached_st *queue;
    queue  = memcached_create(NULL);
    memcached_server_st *servers;
    servers= memcached_servers_parse(server.c_str());
    if (servers != NULL) {
       memcached_server_push(queue, servers);
       memcached_server_list_free(servers);
    } else {
        std::cerr << "Error server config error" << std::endl;
        std::cerr << "Usage: qurluniqmgr set|get servers" << std::endl;
        exit(1);
    }

    if (cmd == "set") {
        std::string line;
        int count = 0;
        while (std::getline(std::cin, line)) {
            ++count;
            if (count % 100000 == 0) {
                std::cerr << "get total " << count << std::endl;
            }

            if (line.size() == 0) {
                continue;
            }

            memcached_return_t rc = memcached_set(queue, line.c_str(), line.size(), "1", 1, 0, 0);
            if (rc != MEMCACHED_SUCCESS) {
                std::cerr << "set " << line <<  " error " << std::endl;
            }
        }
    } else if (cmd == "set0") {
        std::string line;
        int count = 0;
        while (std::getline(std::cin, line)) {
            ++count;
            if (count % 100000 == 0) {
                std::cerr << "get total " << count << std::endl;
            }

            if (line.size() == 0) {
                continue;
            }

            memcached_return_t rc = memcached_set(queue, line.c_str(), line.size(), "0", 1, 0, 0);
            if (rc != MEMCACHED_SUCCESS) {
                std::cerr << "set " << line <<  " error " << std::endl;
            }
        }
    } else if (cmd == "get") {
        const char* keys[BATCH_SIZE];
        size_t key_lens[BATCH_SIZE];

        char ret_key[256];
        size_t ret_key_len;
        size_t ret_val_len;
        uint32_t flags;
        char * ret_val = NULL;
        memcached_return rc;

        std::vector<std::string> batch_vec;
        std::string line;
        int count = 0;
        while (std::getline(std::cin, line)) {
            ++count;
            if (count % 100000 == 0) {
                std::cerr << "get total " << count << std::endl;
            }

            if (line.size() == 0) {
                continue;
            }
            batch_vec.push_back(line);
            if (batch_vec.size() == BATCH_SIZE) {
                for (size_t i = 0; i < batch_vec.size(); i++) {
                    keys[i] = batch_vec[i].c_str();
                    key_lens[i] = batch_vec[i].size();
                }

                rc = memcached_mget(queue, keys, key_lens, batch_vec.size());

                if (rc == MEMCACHED_SUCCESS || rc == MEMCACHED_DATA_EXISTS || rc == MEMCACHED_STORED) {
                    while ((ret_val = memcached_fetch(queue, ret_key, &ret_key_len, &ret_val_len, &flags, &rc))) {
                        free(ret_val);
                    }
                } else {
                    std::cerr << "ERROR when store " << std::endl;
                }

                batch_vec.clear();
            }
        }

        if (batch_vec.size() > 0) {
            for (size_t i = 0; i < batch_vec.size(); i++) {
                keys[i] = batch_vec[i].c_str();
                key_lens[i] = batch_vec[i].size();
            }

            rc = memcached_mget(queue, keys, key_lens, batch_vec.size());
            if (rc == MEMCACHED_SUCCESS || rc == MEMCACHED_DATA_EXISTS || rc == MEMCACHED_STORED) {
                while ((ret_val = memcached_fetch(queue, ret_key, &ret_key_len, &ret_val_len, &flags, &rc))) {
                    free(ret_val);
                }
            } else {
                std::cerr << "ERROR when store " << std::endl;
            }

            batch_vec.clear();
        }
    } else {
        std::cerr << "No cmd " << cmd << " supported"<< std::endl;
        std::cerr << "Usage: qurluniqmgr set|get servers" << std::endl;
        exit(1);
    }

    return 0;
}
