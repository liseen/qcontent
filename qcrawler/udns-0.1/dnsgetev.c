
#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <unistd.h>

#include <ev.h>

#include "udns.h"

void /* A query callback routine */
dns_a4_cb(struct dns_ctx *ctx, struct dns_rr_a4 *result, void *data)
{
    if (result && result->dnsa4_n44 >= 1) {
        printf("okay host: %s, size: %d\n", result->dnsa4_qname, result->dnsa4_nrr);
    } else {
        printf("fail\n");
    }
}

static void
dns_io_cb (EV_P_ ev_io *w, int revents)
{
    printf("dns_io_cb\n");
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

int main(int argc, char *argv[])
{
    struct ev_loop *loop = ev_default_loop (0);

    int dns_fd;
    if (dns_init(NULL, 0) < 0 ) {
        printf("unable to initialize dns library\n");
    }

    dns_fd = dns_open(NULL);
    if (dns_fd < 0) {
        printf("unable to initialize dns context\n");
    }

    printf("dns_fd: %d\n", dns_fd);
    int i= 0, now = 0;

/*
    fd_set fds;
    struct timeval tv;
    while((i = dns_timeouts(NULL, -1, now)) > 0) {
        FD_SET(dns_fd, &fds);
        tv.tv_sec = i;
        tv.tv_usec = 0;
        i = select(dns_fd+1, &fds, 0, 0, &tv);
        now = time(NULL);
        if (i > 0) dns_ioevent(NULL, now);
   }
   */

    ev_io dns_io_watcher;
    ev_io_init(&dns_io_watcher, dns_io_cb, dns_fd, EV_READ);
    ev_io_start(loop, &dns_io_watcher);

    ev_timer dns_timeout_watcher;
    ev_timer_init (&dns_timeout_watcher, dns_timeout_cb, 0, 0.);
    ev_timer_start (loop, &dns_timeout_watcher);


    void *data = NULL;
    /*
    if (!dns_submit_a4(NULL, "www.baidu.com", 0, dns_a4_cb, data)) {
        printf("submit error %d\n", dns_status(NULL));
    }
    if (!dns_submit_a4(NULL, "www.baidu1.com", 0, dns_a4_cb, data)) {
        printf("submit error %d\n", dns_status(NULL));
    }
    if (!dns_submit_a4(NULL, "www.baidu2.com", 0, dns_a4_cb, data)) {
        printf("submit error %d\n", dns_status(NULL));
    }
    */
    if (!dns_submit_a4(NULL, "www.qafadfadsflf2.com", 0, dns_a4_cb, data)) {
        printf("submit error %d\n", dns_status(NULL));
    }
    //dns_timeout_wathcher.repeat = 0;
    //ev_timer_again (loop, &dns_timeout_watcher);



    printf("submit \n");

    ev_loop (loop, 0);

    return 0;
}
