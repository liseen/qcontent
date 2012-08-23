/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

#ifndef QCRAWLER_HTTP_H
#define QCRAWLER_HTTP_H

#include <string>
#include <map>

#include <ev.h>
#include <googleurl/src/gurl.h>

#include "qcrawler_dns.h"


namespace qcrawler
{

struct HttpRequest;
struct HttpResponse;

typedef void (*QCrawlerHttpFetchCallback) (HttpRequest *req);

struct HttpRequest
{
    struct ev_io conn_io;
    struct ev_io read_io;
    struct ev_timer timeout_io;

    enum HttpMethod {GET, HEAD, POST};

    HttpRequest();
    HttpRequest(const std::string &u, void *odata);

    void fetch_callback();

    int sock_fd;

    GURL url;

    HttpMethod method;
    std::string referer;

    int timeout;
    HttpResponse *res;
    void *other_data;
};

struct HttpResponse
{
    enum ResponseCode {RESPONSE_OKAY = 0, DNS_ERROR, DNS_FATAL_ERROR, SOCKET_ERROR, TIMEOUT_ERROR, CONNECT_ERROR, READ_TIMEOUT, READ_ERROR, HTTP_FORMAT_ERROR};

    HttpResponse(const HttpRequest *req);
    ~HttpResponse();

    ResponseCode error_code;
    HttpRequest *req;

    std::string raw_response;
    int http_status;
    int header_content_length;
    std::string header_content_type;
    std::string header_location;
    std::map<std::string, std::string> headers;
    std::string content;
};


class QCrawlerHttp
{
public:
    static QCrawlerHttp * get_instance();

    void set_qcrawler_http_fetch_callback(QCrawlerHttpFetchCallback cb);
    int get_http_timeout();
    void set_http_timeout(int timeout);

    std::string get_http_useragent();
    void set_http_useragent(const std::string &useragent);

    struct ev_loop* get_http_loop();
    int current_connections();
    void submit_http_fetch(HttpRequest *req);

private:
    QCrawlerHttp();
    QCrawlerHttp(const QCrawlerHttp &http);
    QCrawlerHttp & operator=(const QCrawlerHttp &http);

    static QCrawlerHttp *http_crawler;
};

} // end namespace qcrawler

#endif
