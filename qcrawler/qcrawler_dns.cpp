#include "qcrawler_dns.h"

#include <glog/logging.h>
#include <ev.h>
#include <cassert>

#include "qcrawler_http.h"

namespace qcrawler
{

static void dns_a4_cb(struct dns_ctx *ctx, struct dns_rr_a4 *result, void *data);
static void dns_io_cb (EV_P_ ev_io *w, int revents);
static void dns_timeout_cb (EV_P_ ev_timer *w, int revents);

QCrawlerDNS *QCrawlerDNS::qcrawler_dns = NULL;

QCrawlerDNS *QCrawlerDNS::get_instance()
{
    if (qcrawler_dns == NULL) {
        qcrawler_dns = new QCrawlerDNS();
    }

    return qcrawler_dns;
}

QCrawlerDNS::QCrawlerDNS() :
    dns_ctx(NULL),
    dns_callback(NULL),
    ok_dns_cache_time(3600),
    fail_dns_cache_time(10)
{

}

void QCrawlerDNS::init(struct ev_loop * loop)
{
    dns_loop = loop;

    if (dns_init(NULL, 0) < 0 ) {
        LOG(FATAL) << "init dns fatal";
    }

    dns_fd = dns_open(NULL);
    if (dns_fd < 0) {
        LOG(FATAL) << "open dns fatal";
    }

    if (dns_loop == NULL) {
        dns_loop = ev_default_loop(0);
    }

    ev_io_init(&dns_io_watcher, dns_io_cb, dns_fd, EV_READ);
    ev_io_start(dns_loop, &dns_io_watcher);

    ev_timer_init (&dns_timeout_watcher, dns_timeout_cb, 1, 0.);
    ev_timer_start (dns_loop, &dns_timeout_watcher);
}

void QCrawlerDNS::async_resolve_host(const std::string &host, void *data)
{
    assert(dns_loop);
    ev_tstamp now = ev_now(dns_loop);

    // check dns cache
    if (dns_cache.find(host) != dns_cache.end()) {
        QCrawlerDNSEntry &entry = dns_cache[host];
        QCrawlerDNSFetchCallback cb = QCrawlerDNS::get_instance()->dns_fetch_callback();

        if (!entry.addr_list.empty()) {
            if (entry.resolve_time + ok_dns_cache_time >= now)  {
                entry.last_pos = (entry.last_pos + 1) % entry.addr_list.size();
                if (cb) {
                    cb(0, &(entry.addr_list[entry.last_pos]), data);
                }

                return;
            }

        } else {
            if (entry.resolve_time + fail_dns_cache_time > now) {
                if (cb) {
                    cb(1, NULL, data);
                }
                return;
            }

        }
    }

    // submit dns task
    if (!dns_submit_a4(dns_ctx, host.c_str(), 0, dns_a4_cb, data)) {
        LOG(WARNING) << "dns sumit a4 eror";
    }
}

int QCrawlerDNS::resolve_host(const std::string &host, struct in_addr *addr,  int now)
{
    // check cache
    if (dns_cache.find(host) != dns_cache.end()) {
        QCrawlerDNSEntry &entry = dns_cache[host];
        if (!entry.addr_list.empty()) {
            if (entry.resolve_time + ok_dns_cache_time >= now)  {
                entry.last_pos = (entry.last_pos + 1) % entry.addr_list.size();
                *addr = entry.addr_list[entry.last_pos];
                return 0;
            }

        } else {
            if (entry.resolve_time + fail_dns_cache_time > now) {
                return 1;
            }

        }
    }

    QCrawlerDNSEntry entry;
    entry.last_pos = 0;

    entry.resolve_time = now;

    struct addrinfo hints, *res, *result;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int error;
    error = getaddrinfo(host.c_str(), "http", &hints, &result);
    int cnt = 0;
    while (error == EAI_AGAIN && ++cnt < 3) {
        ev_sleep(1);
        error = getaddrinfo(host.c_str(), NULL, NULL, &result);
    }
    if (error == EAI_AGAIN && cnt >= 3) {
        LOG(WARNING) << "error in get addrinfo EAI AGAIN fatal error " << gai_strerror(error);
        return -1;
    }

    if (error != 0) {
       LOG(INFO) << "error in get addrinfo " << gai_strerror(error);
       if (error != EAI_AGAIN) {
            dns_cache[host] = entry;
       }
       return 1;
    } else {

        for (res = result; res != NULL; res = res->ai_next) {
            entry.addr_list.push_back(((sockaddr_in*)(res->ai_addr))->sin_addr);
        }
        freeaddrinfo(result);

        *addr = entry.addr_list[0];

        dns_cache[host] = entry;

        return 0;
    }
}

void QCrawlerDNS::add_dns_entry_cache(const std::string &host, struct in_addr* addr_list, int addr_size)
{
    QCrawlerDNSEntry entry;
    entry.last_pos = 0;
    entry.resolve_time = ev_now(dns_loop);

    for (int i = 0; i < addr_size; ++i) {
        entry.addr_list.push_back(addr_list[i]);
    }

    dns_cache[host] = entry;
}

static void dns_a4_cb(struct dns_ctx *ctx, struct dns_rr_a4 *rr, void *data)
{
    QCrawlerDNS *qcrawler_dns = QCrawlerDNS::get_instance();
    QCrawlerDNSFetchCallback cb = qcrawler_dns->dns_fetch_callback();

    if (!cb) {
        return;
    }

    if (!rr) {
        HttpRequest *req = (HttpRequest*)data;
        qcrawler_dns->add_dns_entry_cache(req->url.host(), NULL, 0);
        cb(1, NULL, data);
    } else if (rr->dnsa4_nrr >= 1) {
        qcrawler_dns->add_dns_entry_cache(std::string(rr->dnsa4_qname), rr->dnsa4_addr, rr->dnsa4_nrr);
        cb(0, rr->dnsa4_addr, data);
    } else {
        qcrawler_dns->add_dns_entry_cache(std::string(rr->dnsa4_qname), rr->dnsa4_addr, rr->dnsa4_nrr);
        cb(1, rr->dnsa4_addr, data);
    }
}

static void dns_io_cb (EV_P_ ev_io *w, int revents)
{
    ev_tstamp now = ev_now(EV_A );
    dns_ioevent(NULL, now);
}

static void dns_timeout_cb (EV_P_ ev_timer *w, int revents)
{
    ev_now_update(EV_A);
    ev_tstamp now = ev_now(EV_A );
    int next = dns_timeouts(NULL, -1, now);

    if (next > 0) {
        w->repeat = next;
        ev_timer_again (EV_A_ w);
    } else {
        w->repeat = 1;
        ev_timer_again (EV_A_ w);
    }
}

} // end namespace qcrawler

