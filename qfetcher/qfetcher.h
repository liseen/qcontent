#ifndef QCONTENT_FETCHER_H
#define QCONTENT_FETCHER_H

#include <QCoreApplication>
#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QStringList>
#include <QDateTime>
#include <QTime>
#include <QTimer>
#include <QUrl>
#include <QHash>
#include <string>
#include <queue>
#include <vector>

#include <libqcontentqueue.h>
#include <libqurlqueue.h>
#include <qstore.h>
#include <qcontent_record.h>

#define MAX_RAW_HTML_CONTENT_SIZE 1048576

namespace qcontent {

class QFetcher: public QObject
{
    Q_OBJECT
public:
    QFetcher(UrlQueue *input_queue, HubQueue *out_queue, QStore *store, int concurrent = 10) : m_input_queue(input_queue), m_out_queue(out_queue), m_store(store), m_concurrent(concurrent), m_stop_crawl(false) {
        cycle_timer = NULL;
        m_cycle_time = get_current_time();
        m_global_stop = false;
        connect(&m_manager, SIGNAL(finished(QNetworkReply*)),
            SLOT(downloadFinished(QNetworkReply*)));
    }

    int get_current_time();
    void start();
    void stop();
    void crawlUrl(const QUrl &qurl, QCrawlerRecord &record);
    int fetchCrawlerRecord();
    int pushCrawlerRecord(const QCrawlerRecord &record);
    int pushCrawlerRecord(const std::string &record);

public slots:
    void cycleFetch();
    void cyclePush();
    void cycleTimeout();
    void cycleStop();
    void cycle();
    void downloadFinished(QNetworkReply *reply);
    void downloadTimeout(QNetworkReply *reply);

private:
    UrlQueue *m_input_queue;
    HubQueue *m_out_queue;

    QStore *m_store;

    QNetworkAccessManager m_manager;
    QHash<QNetworkReply *, QCrawlerRecord> m_current_downloads;
    std::queue<std::string> m_record_buffer;
    QTimer *cycle_timer;
    volatile int m_cycle_time;

    int m_concurrent;
    volatile bool m_stop_crawl;
    volatile bool m_global_stop;
};

} // end namespace qcontent

#endif
