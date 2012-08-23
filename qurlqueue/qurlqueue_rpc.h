#ifndef QURLQUEUE_RPC_H
#define QURLQUEUE_RPC_H

#include <msgpack/rpc/loop.h>
#include <msgpack/rpc/server.h>
#include <mp/sync.h>
#include <map>
#include <queue>
#include <deque>
#include <string>
#include "qcontenthub.h"

#include <tr1/functional>
#include <tr1/unordered_map>
#include <tr1/unordered_set>

#define MAX_PRIORITY 10

namespace qurlqueue {
	using std::tr1::unordered_map;
	using std::tr1::unordered_set;
	using std::tr1::unordered_multimap;
	using std::tr1::unordered_multiset;
	template <typename T> struct hash : public std::tr1::hash<T> { };
}


namespace qurlqueue {

struct Site;

typedef std::tr1::unordered_map<std::string, Site *> SiteMap;
typedef std::tr1::unordered_map<std::string, Site *>::iterator SiteMapIter;
typedef std::tr1::unordered_map<std::string, int> SiteIntervalMap;
typedef std::tr1::unordered_map<std::string, int>::iterator SiteIntervalMapIter;

struct UrlItem {
    UrlItem(int p, const std::string &v) :priority(p), value(v) {
    }

    int priority;
    std::string value;
};

class UrlItemCmp {
public:
    bool operator() (const UrlItem &lurl, const UrlItem &rurl) const
    {
        return lurl.priority < rurl.priority;
    }
};

typedef std::priority_queue<UrlItem, std::vector<UrlItem>, UrlItemCmp>  SiteUrlQueue;

struct Site {
    Site(): stop(false), interval(1000), ref_cnt(0), enqueue_items(0), dequeue_items(0), clear_items(0), next_crawl_time(0) {
    }

    bool stop;
    std::string name;
    int interval;
    int ref_cnt;

    uint64_t enqueue_items;
    uint64_t dequeue_items;
    uint64_t clear_items;
    uint64_t next_crawl_time;

    //bool site_dumping;
    //std::deque<std::string>::iterator site_dump_it;

    //bool dump_all_site_dumping;
    //std::deque<std::string>::iterator dump_all_site_dump_it;
    SiteUrlQueue url_queue;

};

class SiteCmp {
public:
    bool operator() (Site* &lhs, Site* &rhs) const
    {
        return lhs->next_crawl_time > rhs->next_crawl_time;
    }

};

typedef std::priority_queue<Site *, std::vector<Site*>, SiteCmp> SiteQueue;

class QUrlQueueServer : public msgpack::rpc::server::base {

public:
    QUrlQueueServer(msgpack::rpc::loop lo = msgpack::rpc::loop()) : msgpack::rpc::server::base(lo), m_enqueue_items(0), m_dequeue_items(0), m_clear_items(0), m_stop_all(false), m_dump_all_dumping(false) {}
    //void push_url(msgpack::rpc::request &req, const std::string &site, const std::string &record);
    void push_url(msgpack::rpc::request &req, int priority, const std::string &site, const std::string &record);
    //void push_url_front(msgpack::rpc::request &req, const std::string &site, const std::string &record);
    void push_url_front(msgpack::rpc::request &req, int priority, const std::string &site, const std::string &record);
    int push_url(int priority, const std::string &site, const std::string &record, bool push_front = false);

    void pop_url(msgpack::rpc::request &req);
    void pop_url(std::string &ret);
    void start_all(msgpack::rpc::request &req);
    void stop_all(msgpack::rpc::request &req);
    void stats(msgpack::rpc::request &req);
    void clear_all(msgpack::rpc::request &req);
    void start_dump_all(msgpack::rpc::request &req);
    void dump_all(msgpack::rpc::request &req);

    void set_default_interval(msgpack::rpc::request &req, int interval);
    void set_site_interval(msgpack::rpc::request &req, const std::string &site, int interval);
    void stat_site(msgpack::rpc::request &req, const std::string &site);
    void dump_ordered_site(msgpack::rpc::request &req, int limit);
    void start_site(msgpack::rpc::request &req, const std::string &site);
    void stop_site(msgpack::rpc::request &req, const std::string &site);
    void clear_site(msgpack::rpc::request &req, const std::string &site);
    void start_dump_site(msgpack::rpc::request &req, const std::string &site);
    void dump_site(msgpack::rpc::request &req, const std::string &site);

    void clear_empty_site(msgpack::rpc::request &req);
    int clear_empty_site();

    void start(int multiple);
public:
    void dispatch(msgpack::rpc::request req);

public:
    static bool set_current_time();
    // micro secs
    static uint64_t get_current_time();
protected:
    void push_ordered_site(int priority, Site *s);
private:
    static int  m_default_interval;
    uint64_t m_enqueue_items;
    uint64_t m_dequeue_items;
    uint64_t m_clear_items;

    SiteQueue site_queues[MAX_PRIORITY];
    SiteIntervalMap m_site_interval_map;
    mp::sync<SiteMap> m_site_map;

    volatile bool m_stop_all;
    static volatile uint64_t m_current_time;
    uint64_t m_start_time;

    SiteMapIter m_dump_all_it;
    bool m_dump_all_dumping;
};

} // end namespace qurlqueue

#endif

