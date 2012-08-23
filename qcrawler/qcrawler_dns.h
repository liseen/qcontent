/*
 * qcralwer_dns.h
 * @author liseen
 */

#ifndef QCRAWLER_DNS_H
#define QCRAWLER_DNS_H

#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/queue.h>
#include <netinet/in.h>
#include <netdb.h>

#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>

#include <ev.h>

#include <string>
#include <map>
#include <vector>

#include "udns.h"

namespace qcrawler
{


typedef void (*QCrawlerDNSFetchCallback) (int dns_status, struct in_addr *addr,  void *data);

struct QCrawlerDNSEntry
{
    std::vector<struct in_addr> addr_list;
    int last_pos;
    ev_tstamp resolve_time;
};


class QCrawlerDNS
{
public:
    static QCrawlerDNS *get_instance();

    void init(struct ev_loop * loop);
    void set_dns_fetch_callback(QCrawlerDNSFetchCallback cb) {
        dns_callback = cb;
    }

    void stop() {
        ev_io_stop(dns_loop, &dns_io_watcher);
        ev_timer_stop(dns_loop, &dns_timeout_watcher);
    }

    QCrawlerDNSFetchCallback dns_fetch_callback() {
        return dns_callback;
    }

    int resolve_host(const std::string &host, struct in_addr* addr, int now = 0);
    //int async_resolve_host(const std::string &host, struct in_addr* addr, int now = 0);
    void async_resolve_host(const std::string &host, void *data);

    void add_dns_entry_cache(const std::string &host, struct in_addr* addr_list, int addr_size);

private:
    QCrawlerDNS();

    static QCrawlerDNS *qcrawler_dns;

    struct ev_loop *dns_loop;
    struct dns_ctx *dns_ctx;
    int dns_fd;

    ev_io dns_io_watcher;
    ev_timer dns_timeout_watcher;

    // dns callback
    QCrawlerDNSFetchCallback dns_callback;

    // dns cache options
    int ok_dns_cache_time; // second
    int fail_dns_cache_time;
    std::map<std::string, QCrawlerDNSEntry> dns_cache;

};

} // end namespace qcrawler

#endif


