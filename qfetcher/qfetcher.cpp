#include <stdio.h>
#include <stdlib.h>
#include <glog/logging.h>
#include <googleurl/src/gurl.h>
#include <time.h>
#include <sys/time.h>

#include <QDebug>
#include <QUrl>
#include <QDateTime>
#include <QTime>
#include <QCoreApplication>
#include <cassert>
#include <string>
#include <msgpack.hpp>

#include "qfetcher.h"

namespace qcontent {

void QFetcher::cycleFetch()
{
    VLOG(1) << __FUNCTION__;
    if (m_global_stop) {
        return;
    }

    if (m_stop_crawl || (int)m_record_buffer.size() >= m_concurrent) {
        return;
    }

    if (m_current_downloads.size() > m_concurrent/4) {
        return;
    }

    int fetch_times = 0;
    while ((m_current_downloads.size() < m_concurrent) && (fetch_times++ < 200)) {
        int fetch_ret = fetchCrawlerRecord();
        if (fetch_ret != QCONTENTHUB_OK) {
            break;
        }
    }
}

void QFetcher::cyclePush()
{
    VLOG(1) << __FUNCTION__;
    while (!m_record_buffer.empty()) {
        int push_ret = pushCrawlerRecord(m_record_buffer.front());
        if (push_ret == QCONTENTHUB_OK) {
            m_record_buffer.pop();
        } else if (push_ret == QCONTENTHUB_AGAIN) {
            return;
        } else if (push_ret == QCONTENTHUB_WARN) {
            LOG(WARNING) << "push crawled record warn";
            m_record_buffer.pop();
            return;
        } else if (push_ret == QCONTENTHUB_ERROR) {
            LOG(ERROR) << "push crawled record error";
            m_record_buffer.pop();
            return;
        }
    }
}


void QFetcher::crawlUrl(const QUrl &qurl, QCrawlerRecord &record)
{
    LOG(INFO) << "crawl: " << record.url;
    QNetworkRequest request(qurl);
    request.setRawHeader("User-Agent", "Mozilla/5.0 QunarBot/1.0");
    request.setRawHeader("Referer", record.url.c_str());

    QNetworkReply *reply = m_manager.get(request);

    record.download_time = m_cycle_time;
    m_current_downloads.insert(reply, record);
}

void QFetcher::start()
{
    LOG(INFO) << "start qfetcher";
    m_global_stop = false;

    if (cycle_timer == NULL) {
        cycle_timer = new QTimer(this);
        connect(cycle_timer, SIGNAL(timeout()), this, SLOT(cycle()));
        cycle_timer->start(1000);
    }

}

void QFetcher::cycleTimeout()
{
    int now = m_cycle_time;
    QList<QNetworkReply *> list;
    QHash<QNetworkReply *, QCrawlerRecord>::iterator it = m_current_downloads.begin();
    while (it != m_current_downloads.end()) {
        if (now - it.value().download_time > 1 * 60) {
            LOG(INFO) << "crawl_timeout: " << it.value().url;
            list.push_back(it.key());
        }
        ++it;
    }

    int len = list.size();
    for (int i = 0; i < len; ++i) {
        downloadTimeout(list.at(i));
    }
}

void QFetcher::cycleStop()
{
    LOG(INFO) << "cycle stop qfetcher "
        << "global_stop: " << m_global_stop
        << " downloads.size: " << m_current_downloads.size()
        << " buffer.size: " << m_record_buffer.size();

    if (m_current_downloads.size() == 0 &&
            m_record_buffer.size() == 0) {
        LOG(INFO) << "quit qfetcher";
        QCoreApplication::quit();
    }
}

int QFetcher::get_current_time()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec;
}

void QFetcher::cycle()
{
    if ((m_cycle_time & 63) == 0) {
        m_cycle_time = get_current_time();
        cycleTimeout();
    } else {
        ++m_cycle_time;
    }

    cycleFetch();

    if ((m_cycle_time & 3) == 0) {
        cyclePush();
    }

    if (m_global_stop && ((m_cycle_time & 31) == 0)) {
        cycleStop();
    }
}

void QFetcher::stop()
{
    LOG(INFO) << "stop qfetcher started";
    m_global_stop = true;
}

int QFetcher::fetchCrawlerRecord()
{
    QCrawlerRecord record;
    std::string record_str;
    int ret = m_input_queue->pop_url(record_str);
    VLOG(1) << "pop_url ret: " << ret;
    if (ret == QCONTENTHUB_OK) {
        msgpack::zone zone;
        msgpack::object obj;
        try {
            msgpack::unpack(record_str.c_str(), record_str.size(), NULL, &zone, &obj);
            obj.convert(&record);
        } catch (std::exception& e) {
            // TODO
            LOG(ERROR) << "uppack crawler record error " << e.what();
            return QCONTENTHUB_ERROR;
        }

        const std::string &crawl_type = record.crawl_type;
        char crawl_cmd = crawl_type.size() > 0 ? \
                     crawl_type.at(0) : crawlcommand::GAIN;
        char self_restricted = crawl_type.size() > 1 ? \
                           crawl_type.at(1) : crawlrestriction::UPDATE;
        if (self_restricted == crawlrestriction::INSERT) {
            int status;
            if (m_store->url_status(record.url_md5, status) == 0 && status == crawlstatus::CRAWLED) {
                LOG(INFO) << "has crawled " << record.url;
                return QCONTENTHUB_OK;
            }
        }

        if (crawl_cmd == crawlcommand::FIND
                && record.crawl_level > 0
                && (record.url.find("?") != std::string::npos || record.url.find("search") != std::string::npos)) {
            LOG(INFO) << "search url " << record.url;
            return QCONTENTHUB_OK;
        }

        QUrl qurl;
        qurl.setEncodedUrl(QByteArray(record.url.c_str()));
        crawlUrl(qurl, record);
    }

    return ret;
}

int QFetcher::pushCrawlerRecord(const QCrawlerRecord &record)
{
    msgpack::sbuffer sbuf;
    msgpack::pack(sbuf, record);
    std::string rec_str(sbuf.data(), sbuf.size());
    return pushCrawlerRecord(rec_str);
}

int QFetcher::pushCrawlerRecord(const std::string &rec_str)
{
    VLOG(1) << __FUNCTION__;
    int push_ret = m_out_queue->push_nowait(rec_str);
    if (push_ret == QCONTENTHUB_OK) {
        m_stop_crawl = false;
        if (!m_global_stop && (int)m_current_downloads.size() < m_concurrent
                && (int)m_record_buffer.size() < m_concurrent) {
            fetchCrawlerRecord();
        }
    } else {
        m_stop_crawl = true;
        m_record_buffer.push(rec_str);
    }

    return push_ret;
}

void QFetcher::downloadTimeout(QNetworkReply *reply)
{
    reply->abort();
    if (m_current_downloads.find(reply) == m_current_downloads.end()) {
        return;
    }

    QCrawlerRecord &rec = m_current_downloads[reply];
    rec.crawled_okay = false;
    rec.http_code = -4000;
    ++rec.crawl_failed_times;
    LOG(INFO) << "crawl_fail " << " crawl_failed_times: " << rec.crawl_failed_times << " " << rec.url;
    pushCrawlerRecord(rec);
    m_current_downloads.remove(reply);
    reply->deleteLater();
}

void QFetcher::downloadFinished(QNetworkReply *reply)
{
    if (m_current_downloads.find(reply) == m_current_downloads.end()) {
        return;
    }

    QCrawlerRecord &rec = m_current_downloads[reply];

    int http_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    int err_no = reply->error();
    if (err_no) {
        rec.crawled_okay = false;
        ++rec.crawl_failed_times;
        rec.http_code = http_code;
        LOG(INFO) << "crawl_fail " << "code: " << http_code << " err_no: " << err_no << " " << rec.url << " " << reply->errorString().toStdString();
        pushCrawlerRecord(rec);
    } else {
        rec.http_code = http_code;
        if (http_code == 301  || http_code == 302) {
            rec.is_redirect = true;
            if (rec.redirect_times > 4) {
                rec.crawled_okay = false;
                LOG(INFO) << "crawl_fail " << rec.crawl_failed_times << " " << rec.url;
                pushCrawlerRecord(rec);
            } else {
                // continue crawl redict url
                ++rec.redirect_times;
                QUrl qredirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
                if (qredirect.isRelative()) {
                    qredirect = reply->url().resolved(qredirect);
                }

                GURL gurl(qredirect.toString().toUtf8().constData());
                if (gurl.is_valid()) {
                    rec.redirect_url = gurl.spec();
                    crawlUrl(qredirect, rec);
                } else {
                    rec.crawled_okay = false;
                    LOG(INFO) << "crawl_fail " << rec.crawl_failed_times << " " << rec.url;
                    pushCrawlerRecord(rec);
                }
            }
        } else if (http_code == 200) {
            //rec.crawl_failed_time = 0

            QByteArray data = reply->readAll();
            if (rec.want_type == qcontent::mimetype::image) {
                rec.crawled_okay = true;
                rec.raw_content.clear();
                rec.raw_content.append(data.data(), data.size());
                LOG(INFO) << "crawl_image_okay " << rec.url;
                pushCrawlerRecord(rec);
            } else { /* default html */
                QString content_type(reply->rawHeader("content-type"));
                if (!content_type.contains("text/html")) {
                    LOG(INFO) << "crawl_rejected not html format " << content_type.data() << " " << rec.url;
                } else if (reply->rawHeader("content-disposition").size() > 0) {
                    LOG(INFO) << "crawl_rejected disposition doc " << content_type.data() << " " << rec.url;
                } else if (data.size() > MAX_RAW_HTML_CONTENT_SIZE) {
                    LOG(INFO) << "crawl_rejected large content size " << data.size() << " " << rec.url;
                } else {
                    rec.crawled_okay = true;
                    rec.mime_type = qcontent::mimetype::html;
                    rec.raw_content.clear();

                    QString raw_str(data);
                    if (content_type.contains("charset", Qt::CaseInsensitive)) {
                        content_type.replace("gb2312", "gbk", Qt::CaseInsensitive);
                        QString new_type  = QString("\\1 <meta http-equiv=\"Content-Type\" content=\"%1\">").arg(content_type);
                        static QRegExp head_re("(<\\s*head[^<]*>)", Qt::CaseInsensitive);
                        raw_str.replace(head_re, new_type);
                        VLOG(1) << " add meta content type: " << content_type.toStdString() << " for: " << rec.url;
                    } else {
                        static QRegExp content_type_meta("<\\s*meta\\s+http-equiv\\s*=\\s*\"?Content-Type\"?\\s+content\\s*=\\s*\"text/html;\\s*charset\\s*=\\s*gb2312\\s*\"\\s*/?\\s*>", Qt::CaseInsensitive);
                        raw_str.replace(content_type_meta,  "<head><meta http-equiv=\"Content-Type\" content=\"text/html;charset=gbk\">");
                        //LOG(INFO) << "replace meta for content type has chaset: " << rec.url;
                    }

                    rec.raw_content.append(raw_str.toStdString());
                    LOG(INFO) << "crawl_okay " <<  rec.crawl_type << " " << rec.crawl_level << " " << rec.url;
                    pushCrawlerRecord(rec);
                }
            }
        } else  {

        }
    }

    //reply->close();
    m_current_downloads.remove(reply);
    reply->deleteLater();
}

} //end namespace qcrawler
