/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

#include "qcrawler.h"

#include <ev.h>
#include <cassert>
#include <QString>
#include <QRegExp>
#include <glog/logging.h>

using namespace qcontent;

namespace qcrawler
{
const size_t MAX_RAW_HTML_CONTENT_SIZE = 1024 * 1024;

static int continuing_dns_error_times = 0;

//static ev_tstamp global_timer_last_activity;
static void global_timer_callback(EV_P_ ev_timer *w, int revents);
static void qcrawler_http_fetch_callback(HttpRequest *req);

static void qcrawler_cycle_fetch_task(QCrawler *qcrawler);
static void qcrawler_cycle_fetch_task_with_limit(QCrawler *qcrawler, int limit);

static int qcrawler_fetch_task(QCrawler* qcrawler, QCrawlerRecord **prec);
bool qcrawler_need_crawl(QCrawler* qcrawler, QCrawlerRecord *rec);

static void qcrawler_cycle_push_task(QCrawler *qcrawler);
static int qcrawler_push_task(QCrawlerRecord &rec);
static int qcrawler_push_task(const std::string &rec_str);

QCrawler* QCrawler::qcrawler = NULL;

QCrawler* QCrawler::get_instance()
{
    if (qcrawler == NULL) {
        qcrawler = new QCrawler();
    }

    return qcrawler;
}

QCrawler::QCrawler()
{
    m_global_stop = false;
    m_pause_crawl = false;
    m_fetch_task_cycle = 1.0;
    m_fetch_task_min_cycle = 0.1;
    m_fetch_task_max_cycle = 1.0;
    m_multiple = 60;
    m_record_buf_size = 400;
    m_http_crawler = QCrawlerHttp::get_instance();
    m_http_crawler->set_qcrawler_http_fetch_callback(qcrawler_http_fetch_callback);
}

void QCrawler::run()
{
    struct ev_loop *http_loop = m_http_crawler->get_http_loop();

    struct ev_timer global_timer;

    global_timer.data = this;

    ev_init(&global_timer, global_timer_callback);
    global_timer.repeat = m_fetch_task_cycle;

    ev_timer_again(http_loop, &global_timer);

    ev_set_timeout_collect_interval(http_loop, 0.1);
    ev_loop(http_loop, 0);
}

void QCrawler::stop()
{
    set_global_stop(true);
    QCrawlerDNS::get_instance()->stop();
}

int qcrawler_fetch_task(QCrawler* qcrawler, QCrawlerRecord **prec)
{
    std::string record_str;
    int ret = qcrawler->url_queue()->pop_url(record_str);
    if (ret == QCONTENTHUB_OK) {
        msgpack::zone zone;
        msgpack::object obj;
        QCrawlerRecord *rec = new QCrawlerRecord();

        try {
            msgpack::unpack(record_str.c_str(), record_str.size(), NULL, &zone, &obj);
            obj.convert(rec);
            *prec = rec;
        } catch (std::exception& e) {
            // TODO
            LOG(ERROR) << "uppack crawler record error " << e.what();
            delete rec;

            return QCONTENTHUB_ERROR;
        }
    }

    return ret;
}

bool qcrawler_need_crawl(QCrawler *qcrawler, QCrawlerRecord *rec)
{
    std::string &want_type = rec->want_type;
    if (want_type == mimetype::thumb) {
        if (qcrawler->qurlfilter()->filter(rec->site, rec->host, rec->url).empty()) {
            LOG(INFO) << "filtered " << rec->url;
            rec->crawled_okay = false;
            rec->crawl_status = crawlstatus::DISALLOWED;
            rec->status = qrecordstatus::CRAWLED_FAIL_DISALLOWED;
            return false;
        }


        std::string &crawl_tag = rec->crawl_tag;
        if (!crawl_tag.empty()) {
            int status;
            // check need crawl image for thumb
            //if (qcrawler->qstore()->url_status(crawl_tag, status) == 0 && status == crawlstatus::CRAWLED) {
            //    rec->crawl_status = crawlstatus::NOT_NEED;
            //    return false;
            //}

            std::string thumb;
            if (qcrawler->qstore()->get_thumb(rec->url_md5 + ".t", thumb)) {
                LOG(INFO) << "get thumb cache okay" << rec->url;
                rec->crawled_okay = true;
                rec->crawl_status = crawlstatus::CRAWLED;
                rec->raw_content = thumb;
                return false;
            }

        } else {
            LOG(WARNING) << "thumb url not crawl tag found";
        }
    } else {
        if (qcrawler->qurlfilter()->filter(rec->site, rec->host, rec->url).empty()) {
            LOG(INFO) << "filtered " << rec->url;
            rec->crawled_okay = false;
            rec->crawl_status = crawlstatus::DISALLOWED;
            rec->status = qrecordstatus::CRAWLED_FAIL_DISALLOWED;
            return false;
        }


        const std::string &crawl_type = rec->crawl_type;
        /*
        char crawl_cmd = crawl_type.size() > 0 ? \
                        crawl_type.at(0) : crawlcommand::GAIN;
        */
        char self_restricted = crawl_type.size() > 1 ? \
                            crawl_type.at(1) : crawlrestriction::UPDATE;
        if (self_restricted == crawlrestriction::INSERT) {
            int status;
            if (qcrawler->qstore()->url_status(rec->url_md5, status) == 0
                    && status == crawlstatus::CRAWLED) {
                LOG(INFO) << "has crawled " << rec->url;
                rec->crawled_okay = false;
                rec->crawl_status = crawlstatus::NOT_NEED;
                return false;
            }
        }
    }

    return true;
}

static void global_timer_callback(EV_P_ ev_timer *w, int /*revents*/)
{
    DLOG(INFO) << "global_timer ";
    QCrawler *qcrawler = (QCrawler *)w->data;

    qcrawler_cycle_push_task(qcrawler);
    qcrawler_cycle_fetch_task(qcrawler);

    if (!qcrawler->global_stop() || !qcrawler->record_buffer()->empty()) {
        w->repeat = qcrawler->fetch_task_cycle();
        ev_timer_again(qcrawler->http_crawler()->get_http_loop(), w);
    } else {
        ev_timer_stop(EV_A_ w);
        //ev_unloop(EV_A_ EVUNLOOP_ALL);
    }
}

static void qcrawler_cycle_fetch_task(QCrawler *qcrawler)
{
    int limit = 500;
    qcrawler_cycle_fetch_task_with_limit(qcrawler, limit);
}

static void qcrawler_cycle_fetch_task_with_limit(QCrawler *qcrawler, int limit)
{
    if (qcrawler->global_stop()) {
        return;
    }

    if (qcrawler->record_buffer()->empty()) {
        qcrawler->resume_crawl();
    }

    QCrawlerHttp *http_crawler = qcrawler->http_crawler();

    int cnt = 0;
    while (!qcrawler->crawl_is_pausing()
            && cnt < limit
            && qcrawler->record_buffer()->size() < qcrawler->record_buf_size()
            && http_crawler->current_connections() < qcrawler->multiple()) {

        ++cnt;

        QCrawlerRecord *rec;
        QCrawlerRecord **prec = &rec;
        int ret = qcrawler_fetch_task(qcrawler, prec);
        if ( ret == QCONTENTHUB_OK) {
            //QCrawlerRecord *rec = *prec;
            rec->download_time = (int)ev_now(http_crawler->get_http_loop());
            if (qcrawler_need_crawl(qcrawler, rec)) {
                HttpRequest * req = new HttpRequest(rec->url, rec);
                //LOG(INFO) << "crawl " << rec->url;
                http_crawler->submit_http_fetch(req);
            } else {
                if (rec->crawl_status != crawlstatus::NOT_NEED) {
                    qcrawler_push_task(*rec);
                }
            }
        } else if (ret == QCONTENTHUB_AGAIN) {
            break;
        } else {
            break;
        }
    }

    if (cnt < limit && !qcrawler->crawl_is_pausing()
            && qcrawler->record_buffer()->size() < qcrawler->record_buf_size()) {
        int connections = http_crawler->current_connections();
        if (connections < qcrawler->multiple()) {
            double cycle = qcrawler->fetch_task_cycle() / 1.05;
            double min_cycle = qcrawler->fetch_task_min_cycle();
            if (cycle < min_cycle) {
                cycle = min_cycle;
            }
            qcrawler->set_fetch_task_cycle(cycle);
        } else {
            double cycle = qcrawler->fetch_task_cycle() * 1.05;
            double max_cycle = qcrawler->fetch_task_max_cycle();
            if (cycle > max_cycle) {
                cycle = max_cycle;
            }
            qcrawler->set_fetch_task_cycle(cycle);
        }
    }
}

static void qcrawler_http_fetch_callback(HttpRequest *req)
{
    DLOG(INFO) << __func__ << req->url.spec();
    QCrawlerRecord *prec = (QCrawlerRecord *)req->other_data;
    QCrawlerRecord &rec = *prec;

    HttpResponse *res = req->res;

    int error_code = res->error_code;
    int http_code = res->http_status;

    if (error_code == HttpResponse::DNS_FATAL_ERROR) {
        ++continuing_dns_error_times;
        if (continuing_dns_error_times > 2) {
            QCrawler::get_instance()->stop();
        }
    } else {
        continuing_dns_error_times = 0;
    }

    bool need_push = true;
    if (error_code == HttpResponse::RESPONSE_OKAY) {
        rec.http_code = http_code;
        if (http_code == 301  || http_code == 302) {
            rec.is_redirect = true;
            if (rec.redirect_times > 3) {
                rec.crawled_okay = false;
                LOG(INFO) << "crawl_fail " << rec.crawl_failed_times << " " << rec.url;
                rec.status = qrecordstatus::CRAWLED_FAIL_TOO_MANY_REDIRECT;
            } else {
                ++rec.redirect_times;
                GURL gurl(res->header_location);
                if (!gurl.IsStandard()) {
                    gurl = req->url.Resolve(res->header_location);
                }

                if (gurl.is_valid()) {
                    rec.redirect_url = gurl.spec();
                    // crawl redirect url
                    HttpRequest * redirect_req = new HttpRequest(rec.redirect_url, &rec);
                    QCrawler *qcrawler = QCrawler::get_instance();
                    qcrawler->http_crawler()->submit_http_fetch(redirect_req);
                    need_push = false;
                } else {
                    rec.crawled_okay = false;
                    LOG(INFO) << "crawl_fail invalid redirect url " << rec.url;
                    rec.status = qrecordstatus::CRAWLED_FAIL_OTHER;
                }
            }
        } else if (http_code == 200) {
            //rec.crawl_failed_time = 0
            if (rec.want_type == qcontent::mimetype::thumb) {
                rec.crawled_okay = true;
                rec.raw_content.clear();
                rec.raw_content = res->content;
                LOG(INFO) << "crawl_thumb_okay " << rec.url;
            } else if (rec.want_type == qcontent::mimetype::image) {
                rec.crawled_okay = true;
                rec.raw_content.clear();
                rec.raw_content = res->content;
                LOG(INFO) << "crawl_image_okay " << rec.url;
            } else { /* default html */
                std::string &content_type = res->header_content_type;
                if (content_type.find("text/html") == std::string::npos) {
                    LOG(INFO) << "crawl_rejected not html format "
                        << content_type << " " << rec.url;
                    rec.crawled_okay = false;
                    rec.status = qrecordstatus::CRAWLED_FAIL_OTHER;
                } else if (res->headers.find("content-disposition") != res->headers.end()) {
                    LOG(INFO) << "crawl_rejected disposition doc "
                        << content_type << " " << rec.url;
                    rec.crawled_okay = false;
                    rec.status = qrecordstatus::CRAWLED_FAIL_OTHER;
                } else if (res->content.size() > MAX_RAW_HTML_CONTENT_SIZE) {
                    LOG(INFO) << "crawl_rejected large content size "
                        << res->content.size() << " " << rec.url;
                    rec.crawled_okay = false;
                    rec.status = qrecordstatus::CRAWLED_FAIL_OTHER;
                } else {
                    rec.crawled_okay = true;
                    rec.mime_type = qcontent::mimetype::html;
                    rec.raw_content.clear();

                    QString raw_str(res->content.c_str());
                    QString qcontent_type(content_type.c_str());
                    if (qcontent_type.contains("charset", Qt::CaseInsensitive)) {
                        qcontent_type.replace("gb2312", "gbk", Qt::CaseInsensitive);
                        QString new_type  =
                            QString("\\1 <meta http-equiv=\"Content-Type\" content=\"%1\">")
                            .arg(qcontent_type);

                        static QRegExp head_re("(<\\s*head[^<]*>)", Qt::CaseInsensitive);
                        raw_str.replace(head_re, new_type);
                        VLOG(1) << " add meta content type: " << qcontent_type.toStdString()
                            << " for: " << rec.url;
                    } else {
                        static QRegExp content_type_meta("<\\s*meta\\s+http-equiv\\s*=\\s*\"?Content-Type\"?\\s+content\\s*=\\s*\"text/html;\\s*charset\\s*=\\s*gb2312\\s*\"\\s*/?\\s*>", Qt::CaseInsensitive);
                        raw_str.replace(content_type_meta,  "<head><meta http-equiv=\"Content-Type\" content=\"text/html;charset=gbk\">");
                        //LOG(INFO) << "replace meta for content type has chaset: " << rec.url;
                    }

                    rec.raw_content.append(raw_str.toStdString());
                    LOG(INFO) << "crawl_okay " <<  rec.crawl_type << " " << rec.crawl_level << " " << rec.url;
                }
            }
        } else if (http_code >= 500) {
            rec.status = qrecordstatus::CRAWLED_FAIL_SERVER_ERROR;
            rec.crawled_okay = false;
        } else if (http_code >= 400) {
            rec.status = qrecordstatus::CRAWLED_FAIL_NOT_FOUND;
            rec.crawled_okay = false;
        } else {
            rec.crawled_okay = false;
            rec.status = qrecordstatus::CRAWLED_FAIL_OTHER;
        }
    } else {
        rec.crawled_okay = false;
        switch (error_code) {
            case HttpResponse::DNS_ERROR:
            case HttpResponse::DNS_FATAL_ERROR:
                rec.status = qrecordstatus::CRAWLED_FAIL_DNS_ERROR;
                break;
            case HttpResponse::TIMEOUT_ERROR:
                rec.status = qrecordstatus::CRAWLED_FAIL_TIMEOUT;
                break;
            case HttpResponse::CONNECT_ERROR:
                rec.status = qrecordstatus::CRAWLED_FAIL_CONNECT_ERROR;
                break;
            case HttpResponse::HTTP_FORMAT_ERROR:
                rec.status = qrecordstatus::CRAWLED_FAIL_FORMAT_ERROR;
                break;
            default :
                rec.status = qrecordstatus::CRAWLED_FAIL_OTHER;
        }
        ++rec.crawl_failed_times;
        rec.http_code = http_code;
        LOG(INFO) << "crawl_fail "  << " error_code: " << error_code << " " << rec.url;
    }

    if (need_push) {
        qcrawler_push_task(rec);
    }
}

static int qcrawler_push_task(QCrawlerRecord &rec) {
    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, rec);
    std::string rec_str(sbuf.data(), sbuf.size());

    delete &rec;

    return qcrawler_push_task(rec_str);
}

static int qcrawler_push_task(const std::string &rec_str) {
    QCrawler *qcrawler = QCrawler::get_instance();

    int push_ret = qcrawler->hub_queue()->push_nowait(rec_str);
    if (push_ret == QCONTENTHUB_OK) {
        qcrawler->resume_crawl();

        // try fetch task
        qcrawler_cycle_fetch_task(qcrawler);

    } else {
        qcrawler->pause_crawl();
        qcrawler->record_buffer()->push(rec_str);
    }

    return push_ret;


}

static void qcrawler_cycle_push_task(QCrawler *qcrawler)
{
    while (!qcrawler->record_buffer()->empty()) {
        int push_ret = qcrawler->hub_queue()->push_nowait(
                qcrawler->record_buffer()->front());

        if (push_ret == QCONTENTHUB_OK) {
            qcrawler->record_buffer()->pop();
        } else if (push_ret == QCONTENTHUB_AGAIN) {
            qcrawler->pause_crawl();
            return;
        } else if (push_ret == QCONTENTHUB_WARN) {
            LOG(WARNING) << "push crawled record warn";
            qcrawler->record_buffer()->pop();
            return;
        } else if (push_ret == QCONTENTHUB_ERROR) {
            LOG(ERROR) << "push crawled record error";
            qcrawler->record_buffer()->pop();
            return;
        }
    }
}

} // end namespace qcrawler
