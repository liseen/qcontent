/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

#include "qextractor.h"

#include <iconv.h>
#include <unistd.h>

#include <map>
#include <string>
#include <iostream>
#include <sstream>

#include <QHash>
#include <QImage>
#include <QCryptographicHash>

#include <googleurl/src/gurl.h>

#include "qhost2site/qhost2site.h"

#define MAX_URL_LENGTH 2000
#define MAX_CRAWL_LEVEL 10

#include <glog/logging.h>

namespace qcontent
{

QExtractor::QExtractor( \
            HubQueue *input, HubQueue *output,  HubQueue *url_output, \
            UrlQueue *urlqueue, \
            QHtml2Vdom *qhtml2vdom, \
            QStore *store, \
            QUrlFilter *qurlfilter, \
            QThumbScale *qthumb, \
            QPredict *qclassifier, \
            QPredict *qpredict, \
            const std::string &titlefilter_stat_file)
{
    m_input = input;
    m_output = output;
    m_url_output = url_output;

    m_urlqueue = urlqueue;
    m_qhtml2vdom = qhtml2vdom;
    m_store = store;
    m_qurlfilter = qurlfilter;
    m_qthumbscale = qthumb;

    m_qclassifier = qclassifier;
    m_qpredict = qpredict;

    m_stop = false;
    m_extract_url_count = 0;
    m_extract_url_limit = 0;

    m_qtitlefilter_stat_file = titlefilter_stat_file;
    LOG(INFO) << "qtitlefilter_stat_file : " << m_qtitlefilter_stat_file;

    m_content_extractor = new vdom::content::Extractor(m_qtitlefilter_stat_file);
}

QExtractor::~QExtractor()
{
    delete m_content_extractor;
}

int QExtractor::fetch_record(QContentRecord &record)
{
    int ret;
    std::string crawler_str;
    ret = m_input->pop(crawler_str);
    if (ret == QCONTENTHUB_OK) {
        msgpack::zone zone;
        msgpack::object obj;
        try {
            msgpack::unpack(crawler_str.c_str(), crawler_str.size(), NULL, &zone, &obj);
            QCrawlerRecord *prec = &record;
            obj.convert(prec);
            return ret;
        } catch (std::exception& e) {
            LOG(ERROR) << "Get crawler record error: " << e.what();
            return QCONTENTHUB_ERROR;
        }
    } else {
        return ret;
    }
}

void QExtractor::extract(QContentRecord &rec)
{
    vdom::Window *win;
    vdom::Window window;

    if (rec.win != NULL) {
        win = rec.win;
    } else {
        window.ParseFromString(rec.vdom);
        win = &window;
    }

    vdom::content::Result result;
    m_content_extractor->extract(win, rec.download_time, result);
    rec.raw_title = result.raw_title;
    rec.title = result.title;
    rec.keywords = result.keywords;
    rec.description = result.description;
    rec.content = result.content;
    rec.content_confidence = result.content_confidence;
    rec.list_confidence = result.list_confidence;
    rec.publish_time = result.publish_time;
    rec.images = result.images_str;
}

/*
void QExtractor::extract(QContentRecord &rec)
{
    const std::string &crawl_type = rec.crawl_type;
    char crawl_cmd = crawl_type.size() > 0 ? \
                     crawl_type.at(0) : crawlcommand::GAIN;

    if (crawl_cmd == crawlcommand::GAIN) {
        extract(rec, false);
    } else if (crawl_cmd == crawlcommand::FIND) {
        extract(rec, true);
    } else {
        LOG(WARNING) << "No crawl command " << crawl_cmd << " found when crawl " << rec.url;
    }
}
*/

void QExtractor::process_list_page(QContentRecord &rec)
{
    std::string raw_charset;
    if (rec.raw_charset.empty()) {
        // get raw charset
    } else {
        raw_charset = rec.raw_charset;
    }

    iconv_t conv = iconv_open("UTF-8", raw_charset.c_str());
    char *in_p = (char*)rec.raw_content.c_str();
    size_t in_len = rec.raw_content.size();
    size_t out_len = in_len * 3;

    char *dst_p = (char*)calloc(out_len, 1);

    if (dst_p != NULL) {
        char *out_p = dst_p;
        iconv(conv, &in_p, &in_len, &out_p, &out_len);
        iconv_close(conv);

        // get all urls
        free(dst_p);
    }
}

void QExtractor::focus_new_links(QContentRecord &rec)
{
    const std::string &crawl_type = rec.crawl_type;
    char crawl_cmd = crawl_type.size() > 0 ? \
                     crawl_type.at(0) : crawlcommand::GAIN;

    if (crawl_cmd != crawlcommand::FIND) {
        return;
    }

    if (rec.crawl_level >= rec.max_crawl_level) {
        return;
    }

    /* classify the record*/
    int parent_predict = -1;
    if (rec.crawl_level >= 1) {
        //m_classifier
        QPredictDocument doc;
        doc.raw_data.reserve(rec.title.size() + 1 + rec.content.size());
        doc.raw_data.append(rec.title);
        doc.raw_data.append(" ");
        doc.raw_data.append(rec.content);

        m_qclassifier->predict(doc);
        double travel_prob = doc.predict_class_probs[0];
        parent_predict = travel_prob * 100;

        if (parent_predict <= m_parent_predict_limit) {
            LOG(INFO) << "focus new links from noTrip url " << rec.url << " ignored";
            return;
        }
    }

    char find_restriction = get_find_restriction(crawl_type);
    char find_self_restriction = get_find_self_restriction(crawl_type);

    vdom::content::Extractor::UrlList sub_urls;

    m_content_extractor->extract_urls(rec.win, sub_urls);


    msgpack::sbuffer urls_buffer;
    msgpack::packer<msgpack::sbuffer> pk(&urls_buffer);

    vdom::content::Extractor::UrlListIter end_it = sub_urls.end();

    typedef std::pair<int, vdom::content::Extractor::UrlListIter> DedupURLPair;
    typedef unordered_map<std::string, DedupURLPair> DedupURLMap;
    typedef unordered_map<std::string, DedupURLPair>::iterator DedupURLMapIter;

    DedupURLMap dedup_urls;
    for (vdom::content::Extractor::UrlListIter it = sub_urls.begin(); it != end_it; ++it) {
        GURL gurl(it->first);
        if (!gurl.is_valid()) {
            continue;
        }

        if (gurl.has_ref()) {
            continue;
        }

        if (gurl.spec().size() >  MAX_URL_LENGTH) {
            continue;
        }

        const std::string &url = gurl.spec();
        const std::string &anchor = it->second;

        DedupURLMapIter dedup_url_it = dedup_urls.find(url);
        if (dedup_url_it == dedup_urls.end()) {
            dedup_urls.insert(
                    std::pair<std::string, DedupURLPair> ( url,
                        DedupURLPair(anchor.size(), it)));
        } else {
            if (dedup_url_it->second.first > anchor.size()) {
                dedup_urls[url] = DedupURLPair(anchor.size(), it);
            }
        }
    }


    for (DedupURLMapIter it = dedup_urls.begin(); it != dedup_urls.end(); ++it) {
        const std::string &url = it->first;
        const std::string &anchor_text = ((it->second).second)->second;

        GURL gurl(url);

        const std::string &host = gurl.host();
        std::string site = qhost2site(host);

        /*
         * vertical search also need cross site crawler
        if (site != rec.site) {
            continue;
        }
        */

        // TODO support replace url
        if (m_qurlfilter->filter(site, host, url).empty()) {
            continue;
        }

        std::string url_md5 = md5_hash(url);

        if (url_md5 == rec.url_md5) {
            continue;
        }

        if (rec.parent_url_md5.length() > 0 && url_md5 == rec.parent_url_md5) {
            continue;
        }

        bool is_new_link = false;
        if (find_self_restriction == crawlrestriction::INSERT) {
            // check url
            int status;
            if (m_store->url_status(url_md5, status) != 0) {
                LOG(ERROR) << "url seen error";
                continue;
            }

            if (status == crawlstatus::NOT_SEEN) {
                is_new_link = true;
            } else {
                is_new_link = false;
            }
        } else {
            is_new_link = true;
        }


        bool is_need_link = false;
        if (find_restriction == crawlrestriction::HOST) {
            const std::string &condition = rec.crawl_condition;
            if (host.size() >= condition.size() \
                    && host.compare(host.size() - condition.size(), condition.size(), condition) == 0) {
                rec.links_size++;
                if (is_new_link) {
                    is_need_link = true;
                    rec.new_links_size++;
                }
            }
        } else if (find_restriction == crawlrestriction::SITE) {
            if (site == rec.crawl_condition) {
                rec.links_size++;
                if (is_new_link) {
                    is_need_link = true;
                    rec.new_links_size++;
                }
            }
        } else if (find_restriction == crawlrestriction::ALL) {
            rec.links_size++;
            if (is_new_link) {
                is_need_link = true;
                rec.new_links_size++;
            }
        } else if (find_restriction == crawlrestriction::REGEX) {
            if ( url.find(rec.crawl_condition) != std::string::npos) {
                rec.links_size++;
                if (is_new_link) {
                    is_need_link = true;
                    rec.new_links_size++;
                }
            }
        } else {
            LOG(WARNING) << "unkonwn find restriction " << find_restriction;
        }

        if (is_need_link) {
            //QCrawlerRecord crawler_url;
            //crawler_url.url_md5 = url_md5;
            //crawler_url.url = url;
            //crawler_url.anchor_text = it->second;
            //crawler_url.host = host;
            //crawler_url.site = site;
            //crawler_url.parent_url_md5 = rec.url_md5;
            //crawler_url.crawl_type = rec.crawl_type;
            //crawler_url.max_crawl_level = rec.max_crawl_level;
            //crawler_url.crawl_level = rec.crawl_level + 1;
            //crawler_url.crawl_condition = rec.crawl_condition;

            //msgpack::sbuffer sbuf;
		    //msgpack::pack(sbuf, crawler_url);

            //std::string crawler_str(sbuf.data(), sbuf.size());

            // computer the priority by qpredict
            //std::string &anchor_text = it->second;
            //int anchor_predict = -1;
            //if (!anchor_text.empty()) {
            //    QPredictDocument doc;
            //    doc.raw_data = anchor_text;
            //    bool predict_ret = m_qpredict->predict(doc);
            //    if (!predict_ret) {
            //        LOG(FATAL) << "predict fail";
            //    } else {
            //        double travel_prob = doc.predict_class_probs[0];
            //        anchor_predict = travel_prob * 100;
            //    }
            //}

            std::stringstream stream;
            stream << url_md5
                    << "\t" << url
                    << "\t" << anchor_text
                    << "\t" << host
                    << "\t" << site
                    << "\t" << rec.url_md5
                    << "\t" << rec.crawl_type
                    << "\t" << rec.max_crawl_level
                    << "\t" << (rec.crawl_level + 1)
                    << "\t" << rec.crawl_condition
                    << "\t" << parent_predict;

            std::string rec_str = stream.str();

            pk.pack(rec_str);

            // TODO check
            // m_urlqueue->push_url(priority, site, crawler_str);
        }
    }

    if (urls_buffer.size() > 0) {
        std::string urls_rec_str(urls_buffer.data(), urls_buffer.size());
        int rc = QCONTENTHUB_AGAIN;

        while (rc == QCONTENTHUB_AGAIN) {
            rc = m_url_output->push(urls_rec_str);
            if (rc == QCONTENTHUB_OK) {
                break;
            } else if (rc == QCONTENTHUB_AGAIN) {
                sleep(1);
            } else if (rc == QCONTENTHUB_WARN){
                LOG(WARNING) << "push url hub warn";
                sleep(1);
            } else if (rc == QCONTENTHUB_ERROR){
                LOG(ERROR) << "push url hub error " << m_url_output->error_str();
                sleep(1);
            }
        }
    }
}

int QExtractor::push_extracted_queue(QContentRecord &rec)
{
    //msgpack::sbuffer sbuf;
    //msgpack::pack(sbuf, rec);
    //std::string rec_str(sbuf.data(), sbuf.size());

    std::stringstream stream;
    stream << rec.url_md5
        << "\t" << rec.download_time
        << "\t" << rec.status
        << "\t" << rec.parent_url_md5
        << "\t" << rec.url
        << "\t" << rec.site
        << "\t" << rec.host
        << "\t" << rec.crawled_okay
        << "\t" << rec.crawl_type
        << "\t" << rec.crawl_tag
        << "\t" << rec.is_redirect
        << "\t" << rec.redirect_url
        << "\t" << rec.is_list
        << "\t" << rec.links_size
        << "\t" << rec.new_links_size
        << "\t" << rec.loading_time
        << "\t" << rec.list_confidence
        << "\t" << rec.content_confidence
        << "\t" << rec.publish_time
        << "\t" << rec.anchor_text
        << "\t" << rec.raw_title
        << "\t" << rec.title
        << "\t" << rec.keywords
        << "\t" << rec.description
        << "\t" << rec.content;

    std::string rec_str = stream.str();

    int rc = QCONTENTHUB_AGAIN;

    while (rc == QCONTENTHUB_AGAIN) {
        rc = m_output->push(rec_str);
        if (rc == QCONTENTHUB_OK) {
            return 0;
        } else if (rc == QCONTENTHUB_AGAIN) {
            sleep(1);
        } else if (rc == QCONTENTHUB_WARN){
            LOG(WARNING) << "push extracted queue warn";
            sleep(1);
            return 1;
        } else if (rc == QCONTENTHUB_ERROR){
            LOG(ERROR) << "push extracted queue error " << m_output->error_str();
            sleep(1);
            return -1;
        }
    }

    return 0;
}


std::string QExtractor::md5_hash(const std::string &data)
{
    return std::string(QCryptographicHash::hash(
                QByteArray(data.c_str(), data.size()),
                QCryptographicHash::Md5).toHex().constData());
}

void QExtractor::start()
{
    while (!m_stop) {
        if (m_extract_url_limit) {
            if (++m_extract_url_count > m_extract_url_limit) {
                break;
            }
        }

        QContentRecord rec;
        vdom::Window win;
        rec.win = &win;

        int ret = fetch_record(rec);
        LOG(INFO) << "fetch record ret " << ret;
        if (ret == QCONTENTHUB_AGAIN) {
            sleep(1);
            continue;
        } else if (ret == QCONTENTHUB_ERROR) {
            break;
        } else if (ret == QCONTENTHUB_OK) {
            int r;

            // update status
            if (!rec.crawled_okay && rec.status == 0) {
                rec.status = qrecordstatus::CRAWLED_FAIL_OTHER;
            }

            //if (m_qurlfilter->filter(rec.site, rec.host, rec.url).empty()) {
            //    LOG(INFO) << "filter extract " << rec.url;
            //    continue;
            //}

            m_store->url_crawled(rec.url_md5);

            if (rec.want_type == mimetype::thumb) {
                if (rec.crawled_okay) {
                    std::string thumb;
                    if (rec.crawl_status == crawlstatus::CRAWLED) {
                        thumb = rec.raw_content;
                    } else {
                        m_qthumbscale->scale_to_thumb((const uchar*)rec.raw_content.c_str(), \
                                rec.raw_content.size(), thumb);
                        // store thumb for image url
                        r = m_store->store_thumb(rec.url_md5 + ".t", thumb);
                        if (r > 0) {
                            LOG(WARNING) << "store thumb process warn";
                            continue;
                        } else if (r < 0) {
                            LOG(ERROR) << "store  thumb process error";
                            //break;
                        }
                    }

                    //if (thumb.size() > 10 && !rec.crawl_tag.empty()) {
                    //    m_store->url_crawled(rec.crawl_tag);
                    //}
                    if (thumb.size() > 10) {
                        rec.content = thumb;
                    }
                }

                LOG(INFO) << "push_thumb " << rec.url;
                r = push_extracted_queue(rec);
                if (r > 0) {
                    LOG(WARNING) << "push thumb warn";
                    continue;
                } else if (r < 0) {
                    LOG(ERROR) << "push thumb warn";
                    break;
                }

            } else if (rec.want_type == mimetype::image) {
                if (rec.crawled_okay) {
                    QImage img = QImage::fromData((const uchar*)rec.raw_content.c_str(), \
                            rec.raw_content.size());

                    if (img.isNull() || img.height() * img.width() < 90000 \
                            || img.width() > 3 * img.height() \
                            || 2 * img.width() < img.height()) {
                                rec.raw_content = "1";
                            }

                } else {
                    rec.raw_content = "0";
                }

                LOG(INFO) << "store image " << rec.url;
                r = m_store->process(rec);
                if (r > 0) {
                    LOG(WARNING) << "store process warn";
                    continue;
                } else if (r < 0) {
                    LOG(ERROR) << "store process error";
                    break;
                }
            } else {
                if (rec.crawled_okay) {
                    LOG(INFO) << "html2vdom " << rec.url;
                    r = m_qhtml2vdom->process(dynamic_cast<QCrawlerRecord&>(rec));
                    if (r == 0) {
                        LOG(INFO) << "extract " << rec.url;
                        extract(rec);
                        focus_new_links(rec);
                    } else if (r > 0) {
                        LOG(WARNING) << "html2vdom process warn";
                        //continue;
                        rec.crawled_okay = false;
                        rec.status = qcontent::qrecordstatus::QHTML2VDOM_OTHER;
                    } else if (r < 0) {
                        LOG(ERROR) << "html2vdom process error";
                        break;
                    }
                }

                // normalize anchor
                std::string anchor = rec.anchor_text;
                rec.anchor_text.clear();
                vdom::content::Util::normalize_text_to_line(anchor, rec.anchor_text);

                LOG(INFO) << "push_extracted " << rec.url;
                r = push_extracted_queue(rec);
                if (r > 0) {
                    LOG(WARNING) << "push extracted warn";
                    continue;
                } else if (r < 0) {
                    LOG(ERROR) << "push extracted warn";
                    break;
                }

                if (rec.is_list) {
                    LOG(INFO) << "store ignore " << rec.url;
                } else {
                    LOG(INFO) << "store " << rec.url;
                    r = m_store->process(rec);
                }

                if (r > 0) {
                    LOG(WARNING) << "store process warn";
                    continue;
                } else if (r < 0) {
                    LOG(ERROR) << "store process error";
                    //break;
                }
            }
        } else {

        }
    }
}

void QExtractor::stop()
{
    m_stop = true;
}

} // end namesapce qcontent

