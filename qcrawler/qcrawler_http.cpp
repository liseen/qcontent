/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

#include "qcrawler_http.h"

#include <cassert>
#include <sstream>
#include <glog/logging.h>

#include "qcrawler_http_parse.h"

namespace qcrawler
{

static struct ev_loop *http_loop = ev_default_loop(0);

static int http_connections = 0;
static int http_timeout = 60;
static std::string http_useragent = "qcrawler";

static QCrawlerHttpFetchCallback qcrawler_http_fetch_callback = NULL;


static void http_dns_cb(int dns_status, struct in_addr *addr, void *data);
static void http_connect_cb (EV_P_ ev_io *w, int revents);
static void http_read_cb (EV_P_ ev_io *w, int revents);
static void http_timeout_cb(EV_P_ ev_timer *w, int /*revents*/);

static void http_fetch_cb(HttpRequest *req);

// util function for socket
static int make_socket_nonblocking(int fd);
static size_t  atomicio(ssize_t (*f)(int, const void*, size_t), int fd, const void *_s, size_t n);


HttpRequest::HttpRequest()
{
    method = GET;
    res = NULL;

    other_data = NULL;

    // must init struct ev_TYPE
    ev_init(&conn_io, http_connect_cb);
    ev_init(&read_io, http_read_cb);
    ev_init(&timeout_io, http_timeout_cb);
}

HttpRequest::HttpRequest(const std::string &u, void *odata)
{
    method = GET;
    url = GURL(u);
    referer = u;
    res = NULL;
    other_data = odata;

    // must init struct ev_TYPE
    ev_init(&conn_io, http_connect_cb);
    ev_init(&read_io, http_read_cb);
    ev_init(&timeout_io, http_timeout_cb);

}

void HttpRequest::fetch_callback()
{
    http_fetch_cb(this);
}


HttpResponse::HttpResponse(const HttpRequest *req)
{
    req = req;
    error_code = RESPONSE_OKAY;
    header_content_length = 0;
    http_status = 0;
    raw_response.reserve(1024 * 1024);
}

HttpResponse::~HttpResponse()
{
}

QCrawlerHttp * QCrawlerHttp::http_crawler = NULL;

QCrawlerHttp * QCrawlerHttp::get_instance()
{
    if (http_crawler == NULL) {
        http_crawler = new QCrawlerHttp();

        QCrawlerDNS::get_instance()->init(http_loop);
        QCrawlerDNS::get_instance()->set_dns_fetch_callback(http_dns_cb);
    }
    return http_crawler;
}

QCrawlerHttp::QCrawlerHttp()
{
    http_timeout = 60;
    qcrawler_http_fetch_callback = NULL;
}

void QCrawlerHttp::set_qcrawler_http_fetch_callback(QCrawlerHttpFetchCallback cb)
{
    qcrawler_http_fetch_callback = cb;
}

int QCrawlerHttp::get_http_timeout()
{
    return http_timeout;
}

void QCrawlerHttp::set_http_timeout(int timeout)
{
    http_timeout = timeout;
}

std::string QCrawlerHttp::get_http_useragent()
{
    return http_useragent;
}

void QCrawlerHttp::set_http_useragent(const std::string &useragent)
{
    http_useragent = useragent;
}

struct ev_loop * QCrawlerHttp::get_http_loop()
{
    return http_loop;
}

int QCrawlerHttp::current_connections()
{
   return http_connections;
}

void QCrawlerHttp::submit_http_fetch(HttpRequest *req)
{
    DLOG(INFO) << __func__;

    ++http_connections;

    HttpResponse *res = new HttpResponse(req);
    req->res = res;

    QCrawlerDNS *dns = QCrawlerDNS::get_instance();
    dns->async_resolve_host(req->url.host(), (void*)req);
}

static void http_fetch_cb(HttpRequest *req)
{
    DLOG(INFO) << __func__;

    --http_connections;

    HttpResponse *res = req->res;
    if (res->error_code != HttpResponse::TIMEOUT_ERROR && http_timeout > 0) {
        if (ev_is_active(&(req->timeout_io))) {
            ev_timer_stop(http_loop, &(req->timeout_io));
        }
    }

    assert(res);
    if (res->error_code == HttpResponse::RESPONSE_OKAY) {
        if (http_parse(res->raw_response, res->http_status, res->headers, res->content)) {
            DLOG(INFO) << "status: " << res->http_status;
            //DLOG(INFO) << "body: " << res->content;
            if (res->headers.find("location") != res->headers.end()) {
                res->header_location = res->headers["location"];
            }

            if (res->headers.find("content-type") != res->headers.end()) {
                res->header_content_type = res->headers["content-type"];
            }

            if (res->headers.find("content-length") != res->headers.end()) {
                res->header_content_length = atoi(res->headers["content-length"].c_str());
            }

/*
            if (res->header_content_length > 0 && res->header_content_length != res->content.size()) {
                LOG(INFO) << "content length mismatch " << req->url.spec();

                if (abs(res->header_content_length  - (int)res->content.size() ) > 10) {
                    res->error_code = HttpResponse::HTTP_FORMAT_ERROR;
                }
            }
*/
        } else {
            res->error_code = HttpResponse::HTTP_FORMAT_ERROR;
        }
    }

    if (qcrawler_http_fetch_callback) {
        qcrawler_http_fetch_callback(req);
    }

    if (res) {
        delete res;
    }

    delete req;
}

static void http_read_cb (EV_P_ ev_io *w, int /*revents*/)
{
    DLOG(INFO) << __func__;

    HttpRequest *req = (HttpRequest *) (w->data);
    int fd = req->sock_fd;

#define SI 64 * 1024
    char buf[SI + 1];
    memset(buf, 0, SI + 1);
    //int r = atomicio(read, fd, buf,  SI);
    int r = read(fd, buf,  SI);

    if (r < 0) {
        ev_io_stop(EV_A_ w);
        close(fd);
        req->res->error_code = HttpResponse::READ_ERROR;
        req->fetch_callback();
        return;
    } else if (r == 0) {
        ev_io_stop(EV_A_ w);
        close(fd);
        req->fetch_callback();
        return;
    } else {
       req->res->raw_response.append(buf, r);
    }
}

static void http_timeout_cb(EV_P_ ev_timer *w, int /*revents*/)
{
    DLOG(INFO) << __func__;
    HttpRequest *req = (HttpRequest *) (w->data);

    ev_timer_stop (EV_A_ w);

    struct ev_io *io;
    io = &(req->conn_io);
    if (ev_is_active(io)) {
        ev_io_stop(EV_A_ io);
        close(req->sock_fd);
    }

    io = &(req->read_io);
    if (ev_is_active(io)) {
        ev_io_stop(EV_A_ io);
        close(req->sock_fd);
    }

    req->res->error_code = HttpResponse::TIMEOUT_ERROR;
    req->fetch_callback();
}

static void http_connect_cb (EV_P_ ev_io *w, int /*revents*/)
{
    DLOG(INFO) << __func__;

    HttpRequest *req = (HttpRequest *) (w->data);

       /* Check if the connection completed */
    int error;
    socklen_t errsz = sizeof(error);
    if (getsockopt(req->sock_fd, SOL_SOCKET, SO_ERROR, &error,
                &errsz) == -1) {
        LOG(WARNING) << " gbetsockopt error for " << req->url.spec();
        req->res->error_code = HttpResponse::CONNECT_ERROR;
        req->fetch_callback();

        ev_io_stop(EV_A_ w);
        close(req->sock_fd);

        return;
    }

    if (error) {
        req->res->error_code = HttpResponse::CONNECT_ERROR;
        req->fetch_callback();
        ev_io_stop(EV_A_ w);
        close(req->sock_fd);
        return;
    }

    std::stringstream req_stream;

    GURL &url = req->url;
    req_stream << "GET " << url.PathForRequest() << " HTTP/1.0\r\n";
    if (!http_useragent.empty()) {
        req_stream << "User-Agent: " << http_useragent << "\r\n";
    }

    req_stream << "Host: " << url.host();

    if (url.has_port()) {
        req_stream << ":" << url.port();
    }
    req_stream << "\r\n";

    req_stream << "Referer: " << req->referer << "\r\n";
    req_stream << "Accept-Encoding: " << "gzip" << "\r\n";
    req_stream << "\r\n";

    std::string req_str = req_stream.str();
    DLOG(INFO) << "req_str: " << req_str;

    atomicio(write, req->sock_fd, req_str.c_str(), req_str.size());

    ev_io_stop(EV_A_ w);

    req->read_io.data = req;
    ev_io_set(&(req->read_io), req->sock_fd, EV_READ);
    ev_io_start(http_loop, &(req->read_io));
}

static void http_dns_cb(int dns_status, struct in_addr *addr, void *data)
{
    HttpRequest *req = (HttpRequest *) (data);
    // connect
    if (dns_status < 0) {
        DLOG(INFO) << "no dns found for " << req->url.host();
        req->res->error_code = HttpResponse::DNS_FATAL_ERROR;
        req->fetch_callback();

        return;
    } else if (dns_status > 0) {
        DLOG(INFO) << "dns found error  " << req->url.host();
        req->res->error_code = HttpResponse::DNS_ERROR;
        req->fetch_callback();

        return;
    }

    struct sockaddr_in name;
    name.sin_family = AF_INET;
    name.sin_addr = *addr;
    name.sin_port = htons(80);

    int sock = socket (AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        req->res->error_code = HttpResponse::SOCKET_ERROR;
        req->fetch_callback();
        return;
    }

    req->sock_fd = sock;

    if (http_timeout > 0) {
        req->timeout_io.data = req;
        ev_timer_set (&(req->timeout_io), http_timeout, 0.);
        ev_timer_start (http_loop, &(req->timeout_io));
    }

    make_socket_nonblocking(sock);
    int cs = connect(sock, (struct sockaddr *)&name, sizeof (struct sockaddr_in));
    if (cs == -1 && errno != EINPROGRESS) {
        req->res->error_code = HttpResponse::CONNECT_ERROR;
        close(sock);
        req->fetch_callback();
        return;
    }

    req->conn_io.data = (void*) req;
    ev_io_set(&(req->conn_io), req->sock_fd, EV_WRITE);
    ev_io_start(http_loop, &(req->conn_io));
}


static int make_socket_nonblocking(int fd)
{
#ifdef WIN32
    {
        unsigned long nonblocking = 1;
        ioctlsocket(fd, FIONBIO, (unsigned long*) &nonblocking);
    }
#else
    {
        long flags;
        if ((flags = fcntl(fd, F_GETFL, NULL)) < 0) {
            //event_warn("fcntl(%d, F_GETFL)", fd);
            return -1;
        }
        if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
            //event_warn("fcntl(%d, F_SETFL)", fd);
            return -1;
        }
    }
#endif
    return 0;
}

static size_t  atomicio(ssize_t (*f)(int, const void*, size_t), int fd, const void *_s, size_t n)
{
    const char *s = (const char*)_s;
    ssize_t res, pos = 0;

    while ((ssize_t)n > pos) {
        res = (f) (fd, s + pos, n - pos);
        switch (res) {
            case -1:
                if (errno == EINTR || errno == EAGAIN)
                    continue;
            case 0:
                return (res);
            default:
                pos += res;
        }
    }
    return (pos);
}



}
