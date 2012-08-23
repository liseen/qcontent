/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

#ifndef QEXTRACTOR_H
#define QEXTRACTOR_H

#include <qcontent_processor.h>

#include "qcontentextractor/vdom_content_extractor.h"
#include "qcontentextractor/vdom_content_util.h"
#include "qhtml2vdom/qhtml2vdom.h"
#include "qstore/qstore.h"
#include "libqcontenthub/libqcontenthub.h"
#include "libqcontenthub/libqcontentqueue.h"
#include "libqurlqueue/libqurlqueue.h"
#include "qurlfilter/qurlfilter.h"
#include "qthumbscale/qthumbscale.h"
#include "qpredict/qpredict.h"

namespace qcontent
{

class QExtractor
{
public:
    QExtractor(HubQueue *input, HubQueue *output,  HubQueue *url_output, \
            UrlQueue *urlqueue, \
            QHtml2Vdom *qhtml2vdom, \
            QStore *store, \
            QUrlFilter *qurlfilter, \
            QThumbScale *qthumb, \
            QPredict *qclassifier, \
            QPredict *qpredict, \
            const std::string &titlefilter_stat_file);

    ~QExtractor();

    inline void set_extract_url_limit(int extract_url_limit) {
        m_extract_url_limit = extract_url_limit;
    }

    inline void set_parent_predict_limit(int parent_predict_limit) {
        m_parent_predict_limit = parent_predict_limit;
    }


    int fetch_record(QContentRecord &record);

    //void extract(QContentRecord &record, bool need_urls);
    void extract(QContentRecord &record);
    void process_list_page(QContentRecord &record);
    void focus_new_links(QContentRecord &rec);
    int push_extracted_queue(QContentRecord &rec);
    std::string md5_hash(const std::string &data);
    void start();
    void stop();

private:
    HubQueue *m_input;
    HubQueue *m_output;
    HubQueue *m_url_output;

    UrlQueue *m_urlqueue;
    vdom::content::Extractor *m_content_extractor;
    QHtml2Vdom *m_qhtml2vdom;
    QStore *m_store;
    QUrlFilter *m_qurlfilter;
    QThumbScale *m_qthumbscale;
    QPredict *m_qclassifier;
    QPredict *m_qpredict;
    std::string m_qtitlefilter_stat_file;

    volatile bool m_stop;
    int m_extract_url_count;
    int m_extract_url_limit;
    int m_parent_predict_limit;
};

} // end namesapce qcontent

#endif
