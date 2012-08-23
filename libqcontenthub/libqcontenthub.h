/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

#ifndef LIBQCONTENTHUB_H
#define LIBQCONTENTHUB_H

#include <msgpack/rpc/client.h>
#include <string>

#include "qcontenthub/qcontenthub.h"

namespace qcontent {

class HubClient {
public:
    HubClient(const std::string &host, uint16_t port, uint32_t timeout = 200000):\
        m_client(host, port), m_host(host), m_port(port), m_timeout(timeout) {

        m_client.set_timeout(timeout);
    }

    int add_queue(const std::string &name, int capacity = 1000);
    int del_queue(const std::string &name);
    int set_queue_capacity(const std::string &name, int capacity);
    /*
    int start_queue(const std::string &name);
    int stop_queue(const std::string &name);
    */
    int force_del_queue(const std::string &name);
    int push_queue(const std::string &name, const std::string &content);
    int push_queue_nowait(const std::string &name, const std::string &content);
    int pop_queue(const std::string &name, std::string &content);
    int pop_queue_nowait(const std::string &name, std::string &content);
    int stats(std::string &stats_content);
    int stat_queue(const std::string &name, std::string &stats_content);

    const std::string& error_str();

private:
    std::string m_error_str;
    msgpack::rpc::client m_client;
    std::string m_host;
    uint16_t m_port;
    uint32_t m_timeout;
};


}

#endif
