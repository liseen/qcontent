/*
 * qcralwer.h
 * @author liseen
 */

#ifndef QCRAWLER_H
#define QCRAWLER_H

#include <queue>

#include <ev.h>

#include <qcontent_record.h>

#include "libqcontenthub/libqcontenthub.h"
#include "libqcontenthub/libqcontentqueue.h"
#include "libqurlqueue/libqurlqueue.h"
#include "qstore/qstore.h"
#include "qurlfilter/qurlfilter.h"

#include "qcrawler_dns.h"
#include "qcrawler_http.h"

namespace qcrawler
{

class QCrawler
{
public:
    void run();

    static QCrawler* get_instance();

    void init(
            int fetch_task_min_cycle_ms,
            int fetch_task_max_cycle_ms,
            int multiple,
            int record_buf_size,
            const std::string &useragent,
            int http_timeout,
            qcontent::UrlQueue *uq,
            qcontent::HubQueue *hq,
            qcontent::QStore *qstore,
            qcontent::QUrlFilter *qfilter
            ) {
        // seconds
        m_fetch_task_min_cycle = ((double)fetch_task_min_cycle_ms / 1000);
        m_fetch_task_max_cycle = ((double)fetch_task_max_cycle_ms / 1000);

        m_multiple = multiple;
        m_record_buf_size = record_buf_size;
        if (m_record_buf_size < m_multiple) {
            m_record_buf_size = m_multiple;
        }
        m_useragent = useragent;
        m_uq = uq;
        m_hq = hq;
        m_qstore = qstore;
        m_qurlfilter = qfilter;

        m_http_crawler->set_http_useragent(m_useragent);
        m_http_crawler->set_http_timeout(http_timeout);
    }

    double fetch_task_cycle() {
        return m_fetch_task_cycle;
    }

    void set_fetch_task_cycle(double cycle) {
        m_fetch_task_cycle = cycle;
    }

    double fetch_task_min_cycle() {
        return m_fetch_task_min_cycle;
    }

    double fetch_task_max_cycle() {
        return m_fetch_task_max_cycle;
    }

    int multiple() {
        return m_multiple;
    }

    void set_multiple(int multiple) {
        m_multiple = multiple;
    }

    int record_buf_size() {
        return m_record_buf_size;
    }

    void set_record_buf_size(int record_buf_size) {
        m_record_buf_size = record_buf_size;
    }

    qcontent::UrlQueue * url_queue() {
        return m_uq;
    }

    void set_url_queue(qcontent::UrlQueue *uq)  {
        m_uq = uq;
    }

    qcontent::HubQueue * hub_queue() {
        return m_hq;
    }

    void set_hub_queue(qcontent::HubQueue *hq)  {
        m_hq = hq;
    }

    qcontent::QStore *qstore() {
        return m_qstore;
    }

    void set_qstore(qcontent::QStore *qstore) {
        m_qstore = qstore;
    }

    QCrawlerHttp * http_crawler() {
        return m_http_crawler;
    }

    qcontent::QUrlFilter *qurlfilter() {
        return m_qurlfilter;
    }


    bool crawl_is_pausing() {
        return m_pause_crawl;
    }

    void pause_crawl() {
        m_pause_crawl = true;
    }

    void resume_crawl() {
        m_pause_crawl = false;
    }

    std::queue<std::string>* record_buffer() {
        return &m_record_buffer;
    }

    void set_global_stop(bool stop) {
        m_global_stop = stop;
    }

    bool global_stop() {
        return m_global_stop;
    }

    void stop();

private:
    QCrawler();

    static QCrawler * qcrawler;

    bool m_global_stop;
    bool m_pause_crawl;

    double m_fetch_task_cycle;
    double m_fetch_task_min_cycle;
    double m_fetch_task_max_cycle;

    int m_multiple;
    int m_record_buf_size;

    std::string m_useragent;
    qcontent::UrlQueue *m_uq;
    qcontent::HubQueue *m_hq;
    qcontent::QStore *m_qstore;
    qcontent::QUrlFilter *m_qurlfilter;

    std::queue<std::string> m_record_buffer;
    QCrawlerHttp *m_http_crawler;
};


} // end namespace qcrawler

#endif

