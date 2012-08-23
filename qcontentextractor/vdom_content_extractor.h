/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

#ifndef VDOM_CONTENT_EXTRACTOR_H
#define VDOM_CONTENT_EXTRACTOR_H

#include <list>
#include <glog/logging.h>

#include <vdom.h>

#include "qtitlefilter/qtitlefilter.h"

#include "vdom_content_block.h"
#include "vdom_content_result.h"
#include "vdom_content_page.h"

namespace vdom {
namespace content {

class Extractor
{
public:
    typedef std::list< std::pair<std::string, std::string> >  UrlList;
    typedef std::list< std::pair<std::string, std::string> >::iterator  UrlListIter;

    Extractor(const std::string &titlefilter_stat_file = std::string()) :
                  need_title(true), need_content(true),
                  need_list_confidence(true), need_content_confidence(true),
                  need_images(true), need_publish_time(true),
                  need_content_attr(false),
                  qtitlefilter_stat_file(titlefilter_stat_file)
                {

        qtitlefilter = NULL;

        if (!qtitlefilter_stat_file.empty()) {
            qtitlefilter = new qcontent::QTitleFilter(qcontent::QTitleFilter::FILTER, \
                    qtitlefilter_stat_file);
            if (!qtitlefilter->init()) {
                LOG(FATAL) << "init qtitlefilter fail";
            }
        }

    }

    ~Extractor() {
    }

    void extract_urls(vdom::Window *win, UrlList &urls);
    bool extract(vdom::Window *win, int download_time, Result &result);

    void set_need_title(bool need_title) {
        this->need_title = need_title;
    }

    void set_need_content(bool need_content) {
        this->need_content = need_content;
    }

    void set_need_list_confidence(bool need_list_confidence) {
        this->need_list_confidence = need_list_confidence;
    }

    void set_need_content_confidence(bool need_content_confidence) {
        this->need_content_confidence = need_content_confidence;
    }

    void set_need_images(bool need_images) {
        this->need_images = need_images;
    }

    void set_need_publish_time(bool need_publish_time) {
        this->need_publish_time = need_publish_time;
    }

    void set_need_content_attr(bool need_content_attr) {
        this->need_content_attr = need_content_attr;
    }

private:
    bool check_is_noise(Page &page, Node *node);

    void extract_blocks(Page &page);
    void check_is_bbs(Page &page);
    void extract_block_list(Page &page, Node* node, TextBlockList  &block_list);

    void tag_block(Page &page, TextBlock &block);
    // int doc_width, int doc_height, TextBlock *title_block);

    void extract_title(Page &page);
    void correct_title_for_list(Page &page);
    void extract_images(Page &page);
    void extract_publish_time(Page &page);

    void select_good_block(Page &page);
    void expand_good_block(Page &page);
    void merge_content_block(Page &page);
    void merge_content_block_attribute(Page &page);

    void extract_repeat_groups(Page &page);
    void extract_repeat_group_list(Page &page, Node *node, RepeatGroupList &groups);

    bool is_link_group(RepeatGroup &group, bool is_bbs);

    void compute_list_confidence(Page &page);
    bool check_is_list(Page &page);

    //added by ganbao in 5-15
    void compute_content_confidence(Page &page);

private:
    bool prev_is_noise;

    bool need_title;
    bool need_content;
    bool need_list_confidence;
    bool need_content_confidence;
    bool need_images;
    bool need_publish_time;
    bool need_content_attr;

    bool enable_titlefilter;
    std::string qtitlefilter_stat_file;
    qcontent::QTitleFilter *qtitlefilter;

};

}  // namespace vdom
}  // namespace content

#endif
