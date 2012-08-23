/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

#include "libqcontentqueue.h"

namespace qcontent {

int HubQueue::add(int capacity) {
    return m_hub_client.add_queue(m_name, capacity);
}

/*
int HubQueue::start() {
    return m_hub_client.start_queue(m_name);
}

int HubQueue::stop() {
    return m_hub_client.stop_queue(m_name);
}
*/

int HubQueue::force_del() {
    return m_hub_client.force_del_queue(m_name);
}

int HubQueue::push(const std::string &content) {
    return m_hub_client.push_queue(m_name, content);
}

int HubQueue::push_nowait(const std::string &content) {
    return m_hub_client.push_queue_nowait(m_name, content);
}

int HubQueue::pop(std::string &content) {
    return m_hub_client.pop_queue(m_name, content);
}

int HubQueue::pop_nowait(std::string &content) {
    return m_hub_client.pop_queue_nowait(m_name, content);
}

int HubQueue::stats(std::string &stats_content) {
    return m_hub_client.stats(stats_content);
}

int HubQueue::stat(std::string &stats_content) {
    return m_hub_client.stat_queue(m_name, stats_content);
}

const std::string& HubQueue::error_str() {
    return m_hub_client.error_str();
}

HubClient & HubQueue::hub_client() {
    return m_hub_client;
}

}


