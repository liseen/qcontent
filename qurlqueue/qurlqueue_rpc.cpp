#include "qurlqueue_rpc.h"
#include <iostream>
#include <sys/time.h>

namespace qurlqueue {

int QUrlQueueServer::m_default_interval = 1000;
volatile uint64_t QUrlQueueServer::m_current_time = 0;

bool QUrlQueueServer::set_current_time()
{
    m_current_time = get_current_time();
    return true;
}

uint64_t QUrlQueueServer::get_current_time()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + (int)tv.tv_usec / 1000;
}

void QUrlQueueServer::push_ordered_site(int priority, Site *s){
    if (priority <= 0) {
        site_queues[0].push(s);
    } else if (priority >= MAX_PRIORITY) {
        site_queues[MAX_PRIORITY - 1].push(s);
    } else {
        site_queues[priority].push(s);
    }
}

int QUrlQueueServer::push_url(int priority, const std::string &site, const std::string &record, bool /*push_front*/)
{
    //return QCONTENTHUB_OK;
    if (m_stop_all) {
        return QCONTENTHUB_AGAIN;
    }

    UrlItem url_item(priority, record);

    mp::sync<SiteMap>::ref ref(m_site_map);

    SiteMapIter it = ref->find(site);
    //return QCONTENTHUB_OK;
    if (it == ref->end()) {
        Site * s = new Site();
        s->url_queue.push(url_item);
        s->name = site;
        s->enqueue_items = 1;
        ref->insert(std::pair<std::string, Site *>(site, s));
        s->ref_cnt++;
        push_ordered_site(priority, s);
    } else {
        Site * s = it->second;
        if (s->url_queue.empty()) {
            s->ref_cnt++;
            push_ordered_site(priority, s);
        }

        s->url_queue.push(url_item);

        s->enqueue_items++;
    }
    m_enqueue_items++;

    return QCONTENTHUB_OK;
}

void QUrlQueueServer::push_url(msgpack::rpc::request &req, int priority, const std::string &site, const std::string &record)
{	
    int ret = push_url(priority, site, record, false);
    req.result(ret);
}

void QUrlQueueServer::push_url_front(msgpack::rpc::request &req, int /*priority*/, const std::string &site, const std::string &record)
{	
    int ret = push_url(MAX_PRIORITY, site, record, true);
    req.result(ret);
}

void QUrlQueueServer::pop_url(std::string &content)
{
    mp::sync<SiteMap>::ref ref(m_site_map);

    if (m_stop_all) {
        content = QCONTENTHUB_STRAGAIN;
        return;
    }

    uint64_t current_time = get_current_time();

    for (int i = MAX_PRIORITY - 1; i >= 0; --i) {
        SiteQueue &ordered_sites = site_queues[i];

        while (!ordered_sites.empty()) {
            Site * s = ordered_sites.top();
            if (s->stop || s->url_queue.empty()) {
                s->ref_cnt--;
                ordered_sites.pop();
            // do nothing
            } else if (s->next_crawl_time > current_time) {
                content = QCONTENTHUB_STRAGAIN;
                break;
            } else {
                ordered_sites.pop();
                int interval;
                SiteIntervalMapIter it = m_site_interval_map.find(s->name);
                if (it == m_site_interval_map.end()) {
                    interval = m_default_interval;
                } else {
                    interval = it->second;
                }

                s->next_crawl_time = current_time + interval;

                content = s->url_queue.top().value;
                s->dequeue_items++;
                m_dequeue_items++;
                s->url_queue.pop();

                if (!s->url_queue.empty()) {
                    push_ordered_site(s->url_queue.top().priority, s);
                }
                return;
            }
        }
    }

    content = QCONTENTHUB_STRAGAIN;
}

void QUrlQueueServer::pop_url(msgpack::rpc::request &req)
{
    std::string ret;
    pop_url(ret);
    req.result(ret);
}

void QUrlQueueServer::start_all(msgpack::rpc::request &req)
{
    m_stop_all = false;
    req.result(QCONTENTHUB_OK);
}

void QUrlQueueServer::stop_all(msgpack::rpc::request &req)
{
    m_stop_all = true;
    req.result(QCONTENTHUB_OK);
}

void QUrlQueueServer::clear_all(msgpack::rpc::request &req)
{
    {
        mp::sync<SiteMap>::ref ref(m_site_map);
        for (SiteMapIter it = ref->begin(); it != ref->end(); ++it) {
            int size = it->second->url_queue.size();
            it->second->clear_items += size;
            m_clear_items += size;

            // clear
            SiteUrlQueue &url_queue = it->second->url_queue;
            while (!url_queue.empty()) {
                url_queue.pop();
            }
        }
    }

    req.result(QCONTENTHUB_OK);
}

void QUrlQueueServer::start_dump_all(msgpack::rpc::request &req)
{
    if (m_dump_all_dumping) {
        req.result(QCONTENTHUB_ERROR);
    } else {
        m_dump_all_dumping = true;
        m_dump_all_it = m_site_map.unsafe_ref().begin();
        req.result(QCONTENTHUB_OK);
    }
}

void QUrlQueueServer::dump_all(msgpack::rpc::request &req)
{
    std::string content;
    if (!m_dump_all_dumping) {
        content = QCONTENTHUB_STRERROR;
    } else {
        mp::sync<SiteMap>::ref ref(m_site_map);
        while (m_dump_all_it != ref->end()) {
            Site *s = m_dump_all_it->second;
            while (!s->url_queue.empty()) {
                content = s->url_queue.top().value;
                s->url_queue.pop();
                return;
            }

            m_dump_all_it++;
        }

        if (m_dump_all_it == ref->end()) {
            content = QCONTENTHUB_STREND;
            m_dump_all_dumping = false;
        }
    }
    req.result(content);
}

void QUrlQueueServer::start_dump_site(msgpack::rpc::request &req, const std::string &/*site*/)
{
    req.result(QCONTENTHUB_OK);
}

void QUrlQueueServer::dump_site(msgpack::rpc::request &req, const std::string &site)
{
    std::string content;
    {
        mp::sync<SiteMap>::ref ref(m_site_map);
        SiteMapIter it = ref->find(site);
        if (it == ref->end()) {
            content = QCONTENTHUB_STREND;
        } else {
            Site *s = it->second;
            if (!s->url_queue.empty()) {
                content = s->url_queue.top().value;
                s->url_queue.pop();
            } else {
                content = QCONTENTHUB_STREND;
            }
        }
    }

    req.result(content);
}

void QUrlQueueServer::clear_site(msgpack::rpc::request &req, const std::string &site)
{
    {
        mp::sync<SiteMap>::ref ref(m_site_map);
        SiteMapIter it = ref->find(site);
        if (it != ref->end()) {
            int size = it->second->url_queue.size();
            it->second->clear_items += size;
            m_clear_items += size;

            SiteUrlQueue &url_queue = it->second->url_queue;
            while (!url_queue.empty()) {
                url_queue.pop();
            }
        }
    }
    req.result(QCONTENTHUB_OK);
}

void QUrlQueueServer::set_default_interval(msgpack::rpc::request &req, int interval)
{
    {
        mp::sync<SiteMap>::ref ref(m_site_map);
        m_default_interval = interval;
    }
    req.result(QCONTENTHUB_OK);
}

void QUrlQueueServer::set_site_interval(msgpack::rpc::request &req, const std::string &site, int interval)
{
    {
        mp::sync<SiteMap>::ref ref(m_site_map);
        m_site_interval_map[site] = interval;
    }
    req.result(QCONTENTHUB_OK);
}

void QUrlQueueServer::stats(msgpack::rpc::request &req)
{
    char buf[64];
    std::string ret;
    uint64_t current_time = get_current_time();
    uint64_t current = current_time / 1000;

    ret.append("STAT uptime ");
    sprintf(buf, "%ld", current - m_start_time);
    ret.append(buf);

    ret.append("\nSTAT time ");
    sprintf(buf, "%ld", current);
    ret.append(buf);

    ret.append("\nSTAT default_interval ");
    sprintf(buf, "%d", m_default_interval);
    ret.append(buf);

    ret.append("\nSTAT stop_all ");
    sprintf(buf, "%d", m_stop_all);
    ret.append(buf);

    ret.append("\nSTAT site_items ");
    sprintf(buf, "%ld", m_site_map.unsafe_ref().size());
    ret.append(buf);

    int ordered_site_size = 0;
    for (int i = 0; i < MAX_PRIORITY; i++) {
        sprintf(buf, "\nSTAT ordered_site_%d items ", i);
        ret.append(buf);
        sprintf(buf, "%ld", site_queues[i].size());
        ret.append(buf);

        ordered_site_size += site_queues[i].size();
    }

    ret.append("\nSTAT ordered_site_total items ");
    sprintf(buf, "%d", ordered_site_size);
    ret.append(buf);

    ret.append("\nSTAT enqueue_items ");
    sprintf(buf, "%ld", m_enqueue_items);
    ret.append(buf);

    ret.append("\nSTAT dequeue_items ");
    sprintf(buf, "%ld", m_dequeue_items);
    ret.append(buf);

    ret.append("\nSTAT clear_items ");
    sprintf(buf, "%ld", m_clear_items);
    ret.append(buf);

    // TODO:
    // STAT curr_connections 141

    ret.append("\nEND\r\n");
    req.result(ret);
}

void QUrlQueueServer::stat_site(msgpack::rpc::request &req, const std::string &site)
{
    char buf[64];
    std::string ret;
    mp::sync<SiteMap>::ref ref(m_site_map);
    ret.append("STAT site ");
    ret.append(site);

    SiteIntervalMapIter interval_it = m_site_interval_map.find(site);
    ret.append("\nSTAT interval ");
    if (interval_it == m_site_interval_map.end()) {
        ret.append("default ");
        sprintf(buf, "%d", m_default_interval);
        ret.append(buf);
    } else {
        sprintf(buf, "%d", interval_it->second);
        ret.append(buf);
    }

    SiteMapIter it = ref->find(site);
    if (it != ref->end()) {
        ret.append("\nSTAT stop ");
        if (it->second->stop) {
            ret.append("1");
        } else {
            ret.append("0");
        }

        ret.append("\nSTAT enqueue_items ");
        sprintf(buf, "%ld", it->second->enqueue_items);
        ret.append(buf);

        ret.append("\nSTAT dequeue_items ");
        sprintf(buf, "%ld", it->second->dequeue_items);
        ret.append(buf);

        ret.append("\nSTAT clear_items ");
        sprintf(buf, "%ld", it->second->clear_items);
        ret.append(buf);

        ret.append("\nSTAT items_size ");
        sprintf(buf, "%ld", it->second->url_queue.size());
        ret.append(buf);
    }

    ret.append("\nEND\r\n");
    req.result(ret);
}

void QUrlQueueServer::dump_ordered_site(msgpack::rpc::request &req, int limit)
{
    int cnt = 0;
    char buf[64];
    std::string ret;
    {
        mp::sync<SiteMap>::ref ref(m_site_map);
        for (SiteMapIter it = ref->begin(); it != ref->end(); ++it) {
            Site * s = it->second;
            if (!s->url_queue.empty()) {
                ret.append("STAT site ");
                ret.append(s->name);

                ret.append("\nSTAT enqueue_items ");
                sprintf(buf, "%ld", s->enqueue_items);
                ret.append(buf);

                ret.append("\nSTAT dequeue_items ");
                sprintf(buf, "%ld", s->dequeue_items);
                ret.append(buf);

                ret.append("\nSTAT clear_items ");
                sprintf(buf, "%ld", s->clear_items);
                ret.append(buf);

                ret.append("\nSTAT items_size ");
                sprintf(buf, "%ld", s->url_queue.size());
                ret.append(buf);

                ret.append("\n\n");
                ++cnt;
            }

            if (cnt >= limit) {
                break;
            }
        }
    }

    ret.append("\nEND\r\n");
    req.result(ret);
}


void QUrlQueueServer::start_site(msgpack::rpc::request &req, const std::string &site)
{
    {
        mp::sync<SiteMap>::ref ref(m_site_map);
        SiteMapIter it = ref->find(site);
        if (it != ref->end()) {
            Site * s = it->second;
            it->second->stop = false;
            s->ref_cnt++;
            if (!s->url_queue.empty()) {
                push_ordered_site(s->url_queue.top().priority, s);
            }
        }
    }

    req.result(QCONTENTHUB_OK);
}

void QUrlQueueServer::stop_site(msgpack::rpc::request &req, const std::string &site)
{
    {
        mp::sync<SiteMap>::ref ref(m_site_map);
        SiteMapIter it = ref->find(site);
        if (it != ref->end()) {
            it->second->stop = true;
        }
    }

    req.result(QCONTENTHUB_OK);
}


void QUrlQueueServer::clear_empty_site(msgpack::rpc::request &req)
{
    int ret = clear_empty_site();
    req.result(ret);
}

int QUrlQueueServer::clear_empty_site()
{
    uint64_t current_time = get_current_time();

    mp::sync<SiteMap>::ref ref(m_site_map);
    std::vector<std::string> site_vec;
    for (SiteMapIter it = ref->begin(); it != ref->end(); ++it) {
        Site *s = it->second;
        if (s->next_crawl_time > 0 && s->next_crawl_time < current_time - 86400000
                && s->url_queue.empty() && s->ref_cnt == 0) {
            site_vec.push_back(it->first);
        }
        if (site_vec.size() > 2000) {
            break;
        }
    }

    int site_vec_size = site_vec.size();
    for (int i = 0; i < site_vec_size; i++) {
        SiteMapIter del_it = ref->find(site_vec[i]);
        if (del_it != ref->end()) {
            delete del_it->second;
            ref->erase(del_it);
        }
    }

    return site_vec_size;
}


void QUrlQueueServer::dispatch(msgpack::rpc::request req)
{
    try {
        std::string method;
        req.method().convert(&method);

        if(method == "push") {
            msgpack::type::tuple<int, std::string, std::string> params;
            req.params().convert(&params);
            push_url(req, params.get<0>(), params.get<1>(), params.get<2>());
        } else if(method == "pop") {
            pop_url(req);
        } else if(method == "push_url_front") {
            msgpack::type::tuple<int, std::string, std::string> params;
            req.params().convert(&params);
            push_url_front(req, params.get<0>(), params.get<1>(), params.get<2>());
        } else if(method == "start_dump_all") {
            start_dump_all(req);
        } else if(method == "dump_all") {
            dump_all(req);
        } else if(method == "stats") {
            stats(req);
        } else if(method == "set_default_interval") {
            msgpack::type::tuple<int> params;
            req.params().convert(&params);
            set_default_interval(req, params.get<0>());
        } else if(method == "set_site_interval") {
            msgpack::type::tuple<std::string, int> params;
            req.params().convert(&params);
            set_site_interval(req, params.get<0>(), params.get<1>());
        } else if(method == "stat_site") {
            msgpack::type::tuple<std::string> params;
            req.params().convert(&params);
            stat_site(req, params.get<0>());
        } else if(method == "dump_ordered_site") {
            msgpack::type::tuple<int> params;
            req.params().convert(&params);
            dump_ordered_site(req, params.get<0>());
        } else if(method == "start_all") {
            start_all(req);
        } else if(method == "stop_all") {
            stop_all(req);
        } else if(method == "start_site") {
            msgpack::type::tuple<std::string> params;
            req.params().convert(&params);
            start_site(req, params.get<0>());
        } else if(method == "stop_site") {
            msgpack::type::tuple<std::string> params;
            req.params().convert(&params);
            stop_site(req, params.get<0>());
        } else if(method == "clear_site") {
            msgpack::type::tuple<std::string> params;
            req.params().convert(&params);
            clear_site(req, params.get<0>());
        } else if(method == "start_dump_site") {
            msgpack::type::tuple<std::string> params;
            req.params().convert(&params);
            start_dump_site(req, params.get<0>());
        } else if(method == "dump_site") {
            msgpack::type::tuple<std::string> params;
            req.params().convert(&params);
            dump_site(req, params.get<0>());
        } else if(method == "clear_empty_site") {
            clear_empty_site(req);
        } else {
            req.error(msgpack::rpc::NO_METHOD_ERROR);
        }
    } catch (msgpack::type_error& e) {
        req.error(msgpack::rpc::ARGUMENT_ERROR);
        return;

    } catch (std::exception& e) {
        req.error(std::string(e.what()));
        return;
    }
}

void QUrlQueueServer::start(int multiple)
{
    m_start_time = get_current_time() / 1000;
    this->instance.run(multiple);
}

} // end namespace qurlqueue
