/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

#include <map>
#include <set>
#include <algorithm>

#include <google/protobuf/text_format.h>
#include <googleurl/src/gurl.h>
#include <glog/logging.h>
#include <json/json.h>

#include "vdom_content_extractor.h"
#include "vdom_content_util.h"

#define VD_MIN(a,b) ((a)>(b)?(b):(a))
#define VD_MAX(a,b) ((a)>(b)?(a):(b))

#define SCREEN_HEIGHT 843

namespace vdom {
namespace content {

static void debug_print_block_list(TextBlockList &list)
{
    int count = 0;
    for (TextBlockIter it = list.begin(); it != list.end(); ++it) {
        DLOG(INFO) << count++ <<  ": -------------------";
        it->debug_print();
    }
}

void Extractor::extract_urls(vdom::Window *win, UrlList &urls)
{
    if (win == NULL) {
        return;
    }

    vdom::Document *doc = win->mutable_doc();
    if (! doc->has_body()) {
        return;
    }

    vdom::Node* body = doc->mutable_body();

    std::list<vdom::Node*> links;
    body->get_elements_by_tag_name("A", links);

    std::list<vdom::Node*>::iterator end_it = links.end();
    for (std::list<vdom::Node*>::iterator it = links.begin(); it != end_it; ++it) {
        vdom::Node *node = *it;
        if (! Util::is_binary_link(node->href())) {
            urls.push_back(std::pair<std::string, std::string>(node->href(), node->normalized_content()));
        }
    }
}

bool Extractor::extract(vdom::Window *win, int download_time, Result &result)
{
    // init
    vdom::Document *doc = win->mutable_doc();

    if (! doc->has_body()) {
        result.extracted_okay = false;
        return false;
    }

    vdom::Node* body = doc->mutable_body();
    body->build_vdom_tree(win, doc, NULL, 0);
    body->build_repeat_sig();

    // get filelds don't need extract
    Util::normalize_text_to_line(doc->title(), result.raw_title);
    Util::normalize_text_to_line(doc->keywords(), result.keywords);
    Util::normalize_text_to_line(doc->description(), result.description);

    Page page;
    page.win = win;
    page.doc = doc;
    page.body = body;
    page.url = win->location();
    GURL gurl(page.url);
    if (gurl.is_valid()) {
        page.host = gurl.host();
    }

    page.download_time = download_time;
    page.ret = &result;

    page.doc_height = doc->height();
    page.doc_width = doc->width();

    check_is_bbs(page);
    extract_blocks(page);

    select_good_block(page);

    // extract title
    if (need_title) {
        extract_title(page);
    }

    expand_good_block(page);

#ifdef CONTENT_EXTRACTOR_DEBUG
    debug_print_block_list(page.block_list);
#endif

    // list page confidence
    if (need_list_confidence || need_content || need_content_attr) {
        extract_repeat_groups(page);
        compute_list_confidence(page);
    }

    if (need_content_confidence || need_content || need_content_attr) {
        compute_content_confidence(page);
    }

    //correct_title_for_list(page);
    if (need_content) {
        merge_content_block(page);
    }

    if (need_content_attr) {
        merge_content_block_attribute(page);
    }
    // extract images
    if (need_images) {
        extract_images(page);
    }

    if (need_publish_time) {
        extract_publish_time(page);
    }

#ifdef CONTENT_EXTRACTOR_DEBUG
    DLOG(INFO) << "repeat group size " << page.group_list.size();
    for (RepeatGroupListIter lit = page.group_list.begin(); lit != page.group_list.end(); ++lit) {
        DLOG(INFO) << "group: ==========================================================";
        DLOG(INFO) << "is_link_group: " <<  is_link_group(*lit, false);
        for (RepeatGroupIter it = lit->begin(); it != lit->end(); ++it) {
            vdom::Node *node = *it;
            DLOG(INFO) << "node: =========================";
            if (node->repeat_sig().find("#A") != std::string::npos) {
                std::list<vdom::Node*> links;
                node->get_elements_by_tag_name("A", links);
                DLOG(INFO) << "x: " << (*it)->repeat_sig();
                DLOG(INFO) << "x: " << (*it)->x();
                DLOG(INFO) << "y: " << (*it)->y();
                DLOG(INFO) << "w: " << (*it)->w();
                DLOG(INFO) << "h: " << (*it)->h();
                std::string normal;
                Util::normalize_text_to_line((*it)->content(), normal);
                DLOG(INFO) << "content: " << normal;
                //DLOG(INFO) << "content: " << (*it)->content();
            }
        }
    }
#endif

    result.extracted_okay = true;

    return true;
}

void Extractor::check_is_bbs(Page &page)
{
    page.is_bbs = Util::is_bbs_link(page.win->location());
}

void Extractor::extract_blocks(Page &page)
{
    extract_block_list(page, page.body, page.block_list);
}

/* top down first, get all text block */
void Extractor::extract_block_list(Page &page, Node* node, std::list<TextBlock> &block_list)
{
    if (node->type() == Node::ELEMENT) {
        /* filters */
        if (check_is_noise(page, node)) {
            prev_is_noise = true;
            if (!block_list.empty()) {
                block_list.back().set_next_is_noise(true);
            }
        } else {
            if (node->all_children_inline()) {
                TextBlock block;
                block.set_node(node);
                block.set_prev_is_noise(prev_is_noise);
                block_list.push_back(block);
                prev_is_noise = false;
            } else {
                int child_size = node->child_nodes_size();
                for (int i = 0; i < child_size; ++i) {
                    extract_block_list(page, node->mutable_child_nodes(i), block_list);
                }
            }
        }
    } else {
        TextBlock block;
        block.set_node(node);
        block.set_prev_is_noise(prev_is_noise);
        block_list.push_back(block);
        prev_is_noise = false;
    }
}

static bool node_font_cmp(TextBlock *n,  TextBlock *m) {
    return (n->title_weight() > m->title_weight());
}

void Extractor::extract_title(Page &page)
{
    std::list<TextBlock> &list = page.block_list;
    std::vector<TextBlock *> cadidate_list;

    int max_y = 550;
    bool start_content = false;
    int content_size_count = 0;
    int good_block_count = 0;

    const std::string &raw_title = page.ret->raw_title;
    std::string filtered_title;
    if (qtitlefilter != NULL) {
        qtitlefilter->filter(page.host, page.url, raw_title, filtered_title);
    }

    DLOG(INFO) << "page.host: " << page.host;
    DLOG(INFO) << "page.url: " << page.url;
    DLOG(INFO) << "raw_title: "  << raw_title;
    DLOG(INFO) << "filtered title: " << filtered_title;

    for (TextBlockIter it = list.begin(); it != list.end(); ++it) {
        Node *node = it->node();
        int x = node->x();
        int y = node->y();
        int w = node->w();
        int h = node->h();

        if (content_size_count >= 200) {
            break;;
        }

        if (it->is_good()) {
            start_content = true;
            ++good_block_count;
            if (it->node()->h() > 50) {
                content_size_count += it->node()->content().size();
            }

            if (!page.is_bbs && it->node()->y() <= 200) {
                max_y = 400;
            }
        }

	if (y > max_y) {
            break;
        }

        if (good_block_count >= 3) {
            break;
        }

        //if (h > 100 || x > 0.6 * page.doc_width || w < 0.2 * page.doc_width || x + w < 0.3 * page.doc_width) {
        if (h > 100 || x > 0.6 * page.doc_width || w < 60) {
            continue;
        }

        if (!page.is_bbs &&  x + w < 120) {
            continue;
        }

        if (node->numeric_font_size() * node->numeric_font_weight() < 400 * 12) {
            continue;
        }

        if (node->normalized_content().size() < 6 || node->normalized_content().size() > 200) {
            continue;
        }

        DLOG(INFO) << "extract title before noise: " << node->normalized_content();
        if (Util::contain_noise_title(node->normalized_content())) {
            continue;
        }

        DLOG(INFO) << "extract title after noise: " << node->normalized_content();
        //bool contain_out_link = false;
        int out_link_text_size = 0;
        std::list<vdom::Node*> links;
        node->get_elements_by_tag_name("A", links);
        for (std::list<vdom::Node*>::iterator link_it = links.begin(); link_it != links.end(); ++link_it) {
            vdom::Node *link_node = *link_it;
            if (link_node->href().length() > 0 && link_node->href() != page.win->location()
                    && link_node->href() != page.win->location() + "#") {
                //contain_out_link = true;
                out_link_text_size += link_node->normalized_content().size();
                //break;
            }
        }

        if (out_link_text_size >= 18
                || out_link_text_size >= 0.33 * node->normalized_content().size()) {
            continue;
        }

        DLOG(INFO) << "cad title filter: " << node->normalized_content();
        // computer title weight for title_weight and page_title similar
        bool contain_seo = Util::title_contain_seo(filtered_title);
        if (!it->compute_title_weight(filtered_title, contain_seo)) {
            continue;
        }
        DLOG(INFO) << "cad title filter: " << node->normalized_content();

        bool sub_raw_title = (raw_title.find(node->normalized_content()) != std::string::npos);

        if ((node->normalized_content().size() < 12 && contain_seo)
                && (it->page_title_similar() <= 0.05 || !sub_raw_title)) {
            continue;
        }

        DLOG(INFO) << "cad title filter: " << node->normalized_content();
        const std::string &location = page.win->location();
        bool first_page = location.size() >= 1 && location[location.size() - 1] == '/';

        if ((first_page || contain_seo) \
                && node->normalized_content().size() <= 21 && !sub_raw_title \
                && it->page_title_similar() * raw_title.size() <= 0.8 * node->normalized_content().size()) {
            continue;
        }

        DLOG(INFO) << "cad title filter: " << node->normalized_content();
        if (page.is_bbs && node->normalized_content().size() >= 12 \
                && !sub_raw_title&& it->page_title_similar() <= 0.15) {
            continue;
        }

        DLOG(INFO) << "cadidate title: " << node->normalized_content() << "weight: " << it->title_weight();
        cadidate_list.push_back(&(*it));
    }

    std::sort(cadidate_list.begin(), cadidate_list.end(), node_font_cmp);
    DLOG(INFO) << "page title: " << page.doc->title();
    DLOG(INFO) << "cadidate title size: " << cadidate_list.size();

    if (cadidate_list.size() > 0) {
        TextBlock *first_cad = cadidate_list[0];
        DLOG(INFO) << "first cadidate title size: " << first_cad->title_weight();
        if (first_cad->title_weight() >= 0.3) {
            page.title_block = first_cad;
        } else if (cadidate_list.size() == 1 && first_cad->title_weight() >= 0.15) {
            page.title_block = first_cad;
        } else if (cadidate_list.size() >= 2 && first_cad->title_weight() >= 0.15
                && (cadidate_list[1]->title_weight() <= first_cad->title_weight() / 1.2)) {
            page.title_block = first_cad;
        }
    }

    if (page.title_block != NULL) {
        DLOG(INFO) << "extracted_title: " << page.title_block->node()->content();
        page.title_block->set_is_good(false);
        page.title_block->set_is_bad(true);
        Util::normalize_text_to_line(page.title_block->node()->content(), page.ret->title);
    }
}

void Extractor::correct_title_for_list(Page &page)
{
    DLOG(INFO) << page.ret->list_confidence;
    if (page.title_block != NULL && check_is_list(page)) {
        page.title_block->set_is_good(true);
        page.title_block->set_is_bad(false);
        page.title_block = NULL;
        page.ret->title.clear();
    }
}

void Extractor::extract_images(Page &page)
{
    if (check_is_list(page)) {
        return;
    }

    int max_content_diff = 3;

    std::list<vdom::Node*> imgs;

    bool start = false;
    bool begin_content = false;
    int content_dist = 0;

    bool bbs_detail_page = false;

    // bbs detail page
    if (page.is_bbs && page.ret->list_confidence < 10) {
        bbs_detail_page = true;
        max_content_diff = 10;
    }

    if (page.title_block == NULL) {
        std::list<TextBlock> &list = page.block_list;
        for (TextBlockIter it = list.begin(); it != list.end(); ++it) {
            Node *node = it->node();
            if (!begin_content && it->is_content()) {
                begin_content = true;
            }

            if (begin_content) {
                if (it->is_content()) {
                    content_dist = 0;
                    node->get_elements_by_tag_name("IMG", imgs);
                } else {
                    if (it->space_ratio() < 90) {
                        ++content_dist;
                    }
                    //if (content_dist <= max_content_diff && !it->is_bad()) {
                    if (content_dist <= max_content_diff) {
                        node->get_elements_by_tag_name("IMG", imgs);
                    }
                }
            } else {
                //node->get_elements_by_tag_name("IMG", imgs);
            }
        }
    } else {
        std::list<TextBlock> &list = page.block_list;
        for (TextBlockIter it = list.begin(); it != list.end(); ++it) {
            Node *node = it->node();
            if (start) {
                if (begin_content) {
                    if (it->is_content()) {
                        content_dist = 0;
                        node->get_elements_by_tag_name("IMG", imgs);
                    } else {
                        if (it->space_ratio() < 90) {
                            ++content_dist;
                        }
                        //if (content_dist <= max_content_diff && !it->is_bad()) {
                        if (content_dist <= max_content_diff) {
                            node->get_elements_by_tag_name("IMG", imgs);
                        }
                    }
                } else {
                    node->get_elements_by_tag_name("IMG", imgs);
                }
            } else if (page.title_block == & (*it)) {
                start = true;
            }
        }
    }

    std::map<std::string, int> dedup_map;
    for (std::list<vdom::Node*>::iterator it = imgs.begin(); it != imgs.end(); ++it) {
        const std::string &src = (*it)->src();
        if (dedup_map.find(src) == dedup_map.end()) {
            dedup_map[src] = 1;
        } else {
            ++dedup_map[src];
        }
    }

    int min_y_limit = 100;
    if (page.title_block != NULL) {
        min_y_limit = page.title_block->node()->y();
    }

    int valid_image_cnt = 0;

    for (std::list<vdom::Node*>::iterator it = imgs.begin(); it != imgs.end(); ++it) {
        DLOG(INFO) << "image cadidate: " << (*it)->Utf8DebugString();

        Node *img = *it;
        int x = img->x();
        int y = img->y();
        int w = img->w();
        int h = img->h();

        const std::string &src = img->src();
        if (src.size() == 0 || dedup_map[src] > 1) {
            continue;
        }

        if (bbs_detail_page && x < 100) {
            continue;
        }

        if (w >= 40 && w <= 250) {
            continue;
        }
        if (h >= 40 && h <= 150) {
            continue;
        }

        if (h > 20 && w > 20 && h + w > 80 && (w / h  >= 3 || h / w >= 3 )) {
            continue;
        }

        int doc_width = img->owner_document()->width();

        if (x > 0.9 * doc_width) {
            continue;
        }

        if (y < min_y_limit || y >= page.doc_height - 150) {
            continue;
        }

        if (!Util::is_good_image_link(src)) {
            continue;
        }

        if (Util::is_noise_image_link(src)) {
            continue;
        }


        DLOG(INFO) << "good image";

        GURL gurl(src);
        if (!gurl.is_valid()) {
            continue;
        } else {
            ++valid_image_cnt;
            std::string normal_alt;
            Util::normalize_text_to_line(img->alt(), normal_alt);
            page.ret->images_str.append(normal_alt);
            page.ret->images_str.append("\t");
            page.ret->images_str.append(gurl.spec());
            page.ret->images_str.append("\n");
        }

        if (valid_image_cnt == 100) {
            DLOG(INFO) << "image ignored when count more than 100";
            break;
        }
    }
}

void Extractor::extract_publish_time(Page &page)
{
    if (check_is_list(page)) {
        return;
    }

    int not_content = 0;
    int start_content = 0;
    int content_after_title = 0;

    int max_y_limit = 500;
    if (page.title_block != NULL) {
        max_y_limit = page.title_block->node()->y() + 200;
    }

    bool prev_is_title_block = false;
    std::list<TextBlock> &list = page.block_list;
    for (TextBlockIter it = list.begin(); it != list.end(); ++it) {
        int y = it->node()->y();

        if (y > max_y_limit) {
            //continue;
        }

        if (it->node()->normalized_content().size() <= 1) {
            continue;
        }

        DLOG(INFO) << "publish time cadidate: " << it->node()->normalized_content();
        if (it->is_content()) {
            ++start_content;
            if (content_after_title) {
                ++content_after_title;
            }
            not_content = 0;
        } else {
            if (start_content) {
                ++not_content;
            }
        }

        if (page.title_block == NULL) {
            if (start_content > 4) {
                break;
            }
        } else {
            if (content_after_title > 4) {
                //break;
            }

            if (not_content > 4) {
                break;
            }
        }

        if ((page.is_bbs && it->node()->x() + it->node()->w() < 0.4 * page.doc->width()) \
                || it->node()->x() > 700) {
            continue;
        }

        if (it->node()->normalized_content().size() >= 200) {
            continue;
        }

        if (page.title_block != NULL) {
            const vdom::Node* title_node = page.title_block->node();

            if (!prev_is_title_block
                && it->node()->y() >= title_node->y() + title_node->h()
                && it->node()->y() <= title_node->y() + 70
                && it->node()->h() <= 50) {
                prev_is_title_block = true;
            }
        }

        bool is_title_block = page.title_block != NULL && page.title_block == &(*it);

        DLOG(INFO) << "is_title_block: " << is_title_block << " " << it->node()->normalized_content();
        DLOG(INFO) << "prev_is_title_block: " << prev_is_title_block << " " << it->node()->normalized_content();

        int publish_time = Util::match_date(it->node()->normalized_content(), \
                                page.download_time, is_title_block, prev_is_title_block);

        if (publish_time) {
            page.ret->publish_time = publish_time;
            return;
        }

        if (is_title_block) {
            content_after_title = 1;
            prev_is_title_block = true;
        } else {
            prev_is_title_block = false;
        }
    }
}

bool Extractor::check_is_noise(Page &page, Node *node)
{
    int doc_width = node->owner_document()->width();

    if (Util::is_noise_tag_name(node->tag_name())) {
        return true;
    }

    int x = node->x();
    //int y = node->y();
    int w = node->w();
    int h = node->h();

    /* navigator or menu */
    /*
    if (x < 0.3 * doc_width && y < 100
            && w > 0.9 * doc_width && h < 100
            && (float)h/w < 0.1 && node->content().size() < 250) {
        //return true;
    }
    */

    /* left and right advertise banner */
    if (w > 0  && w < 350 && h > 200 && (float)h/w > 0.8
            && (x > 0.6 * doc_width || x + w  < 0.3 * doc_width) ) {
        return true;
    }

    if (page.is_bbs && w > 0.3 * doc_width && h < 600
            && (Util::is_bbs_signature(node->class_name()) || Util::is_bbs_signature(node->id()))) {
        DLOG(INFO) << "bbs signatures: " << node->class_name();
        return true;
    }
    return false;
}


void Extractor::tag_block(Page &page, TextBlock &block)
{
    Node *node = block.node();

    int x = node->x();
    int y = node->y();
    int w = node->w();
    int h = node->h();

    bool contain_noise_word = Util::contain_noise_content(node->content());
    if (h < 200 && node->normalized_content().size() < 200 && contain_noise_word) {
        block.set_is_bad(true);
        return;
    }

    int good_block_max_height = (int)(0.84 * page.doc_height);
    if (good_block_max_height < page.doc_height - 200) {
        good_block_max_height = page.doc_height - 200;
    }

    int min_y_limit = 100;
    if (page.title_block != NULL && (int) page.title_block->node()->y() > min_y_limit) {
        min_y_limit = page.title_block->node()->y();
    }

    if (x + w > 0.3 * page.doc_width && \
        x < 0.6 * page.doc_width && \
        y > min_y_limit && \
        y < good_block_max_height && \
        block.content_size() > 35 && \
        block.anchor_ratio() < 20 && block.tag_density() < 0.4 && block.space_ratio() < 50 && \
        (w > 300 || (x > 100 && w > 200) || Util::contain_good_punct(block.node()->content())) \
        ) {

        block.set_is_good(true);

        return;
    }
}

void Extractor::select_good_block(Page &page)
{
    std::list<TextBlock> &list = page.block_list;
    for (TextBlockIter it = list.begin(); it != list.end(); ++it) {
        tag_block(page, *it);
    }
}

void Extractor::expand_good_block(Page &page)
{
    std::list<TextBlock> &list = page.block_list;
    int doc_width = page.doc_width;

    TextBlockIter end_it = list.end();
    for (TextBlockIter it = list.begin(); it != end_it; ++it) {
        if (it->is_good()) {
            it->set_is_content(true);
            TextBlockIter ei = it;
            ++ei;
            TextBlockIter last_content = ei;
            int space_blocks = 0;
            int text_blocks = 0;
            // forward expand
            for (; ei != end_it; ++ei) {
                Node* node = ei->node();
                if ((ei->space_ratio() > 90 || ei->content_size() == 0) && node->w() == 0) {
                    ++space_blocks;
                }
                if (node->h() > 0 && ei->space_ratio() < 90 && ei->content_size() > 0) {
                    ++text_blocks;
                }

                if (space_blocks > 8 || text_blocks > 3) {
                    break;
                } else if (ei->is_bad()) {
                    break;
                } else if ((ei->is_good() || ei->is_content()) && abs(node->y() - it->node()->y()) < 400 ) {
                    for (; last_content != ei; ++last_content) {
                        last_content->set_is_content(true);
                    }
                    break;
                } else if (abs(node->y() - it->node()->y() - it->node()->h()) < 200 &&\
                        node->x() + node->w() > 0.11 * doc_width && \
                        ei->content_size() > 20 && \
                        ei->anchor_ratio() < 50 && ei->tag_density() < 0.2 && ei->space_ratio() < 50 ) {

                    ei->set_is_content(true);
                    for (; last_content != ei; ++last_content) {
                        last_content->set_is_content(true);
                    }
                }

                if (ei->next_is_noise()) {
                    break;
                }
            }

            space_blocks = 0;
            text_blocks = 0;
            ei = it;
            --ei;
            last_content = ei;
            // backward
            for (; ei != end_it; --ei) {
                Node* node = ei->node();
                if ((ei->space_ratio() > 90 || ei->content_size() == 0) && node->w() == 0) {
                    ++space_blocks;
                }
                if (node->h() > 0 && ei->space_ratio() < 90 && ei->content_size() > 0) {
                    ++text_blocks;
                }

                if (space_blocks > 8 || text_blocks > 3) {
                    break;
                }
                if (ei->is_bad()) {
                    break;
                }

                if ((ei->is_good() || ei->is_content()) && abs(node->y() - it->node()->y()) < 400 ) {
                    for (; last_content != ei; --last_content) {
                        last_content->set_is_content(true);
                    }
                    break;
                } else if (abs(node->y() - it->node()->y() - node->h()) < 200 &&\
                        node->x() + node->w() > 0.11 * doc_width && \
                        ei->content_size() > 20 && \
                        ei->anchor_ratio() < 50 && ei->tag_density() < 0.2 && ei->space_ratio() < 50 ) {

                    ei->set_is_content(true);
                    for (; last_content != ei; --last_content) {
                        last_content->set_is_content(true);
                    }
                }

                if (ei->prev_is_noise()) {
                    break;
                }
            }
        }
    }
}

/* Bottom-up merge expand good block_list*/
void Extractor::merge_content_block(Page &page)
{
    bool prev_is_content = true;
    bool prev_is_space = true;

    bool has_content = false;

    std::string content;
    content.reserve(50 * 1024);

    std::list<TextBlock> &list = page.block_list;
    TextBlockIter end_it = list.end();
    
    if (!check_is_list(page)) {
        for (TextBlockIter it = list.begin(); it != end_it; ++it) {
            if (it->is_content()) {
                if (!has_content) {
                    has_content = true;
                }

                if (!prev_is_content) {
                    content.append("\x03");
                }
                prev_is_content = true;

                if (!prev_is_space) {
                    content.append(" ");
                }
                if (it->space_ratio() == 100) {
                    prev_is_space = true;
                } else {
                    prev_is_space = false;
                }

                if (it->node()->type() == vdom::Node::ELEMENT && it->node()->render_type() == vdom::Node::BLOCK) {
                    content.append("\x02");
                }
                content.append(it->node()->content());
            } else {
                if (has_content) {
                    prev_is_content = false;
                }
            }
        }
    }

    if (has_content) {
//        page.ret->content_confidence = 100;
    } else {
//        page.ret->content_confidence = 50;
        for (TextBlockIter it = list.begin(); it != end_it; ++it) {
            if (!it->is_bad()) {
                content.append(it->node()->content());
                if (it->node()->type() == vdom::Node::ELEMENT && it->node()->render_type() == vdom::Node::BLOCK) {
                    content.append("\x02");
                }
            }
        }
    }

    Util::normalize_content(content, page.ret->content);
}

static void gen_json_block_attr(Json::Value &block, vdom::Node *node)
{
    block["tag_name"] = node->tag_name();
    block["x"] = node->x();
    block["y"] = node->y();
    block["w"] = node->w();
    block["h"] = node->h();
    block["font_size"] = node->font_size();
    block["font_weight"] = node->font_weight();

    block["content"] = node->content();
}

void Extractor::merge_content_block_attribute(Page &page)
{
    bool has_content = false;

    Json::Value content_json;
    Json::Value content_attr;
    std::list<TextBlock> &list = page.block_list;
    TextBlockIter end_it = list.end();
    if (!check_is_list(page)) {
        for (TextBlockIter it = list.begin(); it != end_it; ++it) {
            if (it->is_content() && it->space_ratio() != 100
                    && it->node()->normalized_content().size() > 0) {
                Json::Value block;
                gen_json_block_attr(block, it->node());
                content_attr.append(block);
            }
        }
    }

    if (!has_content) {
        for (TextBlockIter it = list.begin(); it != end_it; ++it) {
            if (!it->is_bad() && it->space_ratio() != 100
                    && it->node()->normalized_content().size() > 0) {
                Json::Value block;
                gen_json_block_attr(block, it->node());
                content_attr.append(block);
            }
        }
    }

    content_json["version"] = 1;
    content_json["content_attr"] = content_attr;
    Json::FastWriter fast_writer;
    page.ret->content_attr = fast_writer.write(content_json);
}

void Extractor::extract_repeat_groups(Page &page)
{
    extract_repeat_group_list(page, page.body, page.group_list);
}

void Extractor::extract_repeat_group_list(Page &page, Node *node, RepeatGroupList &groups)
{
    std::map<std::string, RepeatGroup> group_map;
    std::map<std::string, RepeatGroup>::iterator end_it = group_map.end();
    std::map<std::string, RepeatGroup>::iterator it;

    int child_size = node->child_nodes_size();
    for (int i = 0; i < child_size; ++i) {
        Node* child = node->mutable_child_nodes(i);
        if (child->type() == Node::ELEMENT && child->w() >= 200 &&
                child->repeat_sig().size() > 0 && child->repeat_sig().find("#A") != std::string::npos) {
            it = group_map.find(child->repeat_sig());
            if (it == end_it) {
                RepeatGroup group;
                group.push_back(child);
                group_map.insert(std::pair<std::string, RepeatGroup>(child->repeat_sig(), group));
            } else {
                it->second.push_back(child);
            }
        }
    }

    for (it = group_map.begin() ; it != end_it; ++it ) {
        if (it->second.size() >= 3) {
            groups.push_back(it->second);
            continue;
        }

        if (it->second.size() == 2) {
            int area_sum = 0;
            for (RepeatGroupIter git = it->second.begin(); git != it->second.end(); ++git) {
                area_sum += (*git)->w() * (*git)->h();
            }

            if (area_sum >= 2 * 50000) {
                groups.push_back(it->second);
                continue;
            }
        }

        if (!check_is_noise(page, it->second.front())) {
            // Node *tmp = it->second.front();
            // LOG(INFO) << " 00000000000000000000000000000 ";
            // LOG(INFO) << "tmp content:" << tmp->normalized_content();
            // LOG(INFO) << "tmp repeat_sig: " << tmp->repeat_sig();
            extract_repeat_group_list(page, it->second.front(), groups);
        }
    }
}

bool Extractor::is_link_group(RepeatGroup &group, bool is_bbs)
{
    if (group.size() <= 0) {
        return false;
    }

    int times = 0;
    int avr_w =  -1;
    int avr_x =  -1;
    int avr_h =  -1;
    int total_h =  0;
    int total_links_text_size = 0;
    int total_text_size = 0;

    std::set<std::string> url_set;
    std::vector<int> rel_x_vec;
    int all_same_links_times = 0;

    DLOG(INFO) << "have repeat group";

    for (RepeatGroupIter it = group.begin(); it != group.end(); ++it) {
        ++times;

        vdom::Node *node = *it;
        if (times == 1) {
            avr_w = node->w();
            avr_x = node->x();
            avr_h = node->h();
        } else if (node->w() > 2.0 * avr_w || node->w() < 0.5 * avr_w ) {
            DLOG(INFO) << "width abnormal";
            return false;
        } else if (node->tag_name() == "A"
                && (node->w() < 250 || node->x() > 1.05 * avr_x || node->x() < 0.95 * avr_x) ) {
            DLOG(INFO) << "x abnormal when node is A";
            return false;
        }

        total_h += node->h();

        std::list<vdom::Node*> links;
        node->get_elements_by_tag_name("A", links);

        bool contain_good_link = false;
        bool contain_same_rel = false;
        bool all_same_links = true;

        int text_size = node->normalized_content().size();
        total_text_size += text_size;
        int links_text_size = 0;

        int link_pos = 0;
        for (std::list<vdom::Node*>::iterator link_it = links.begin(); link_it != links.end(); ++link_it) {
            vdom::Node *link_node = *link_it;

            int size = link_node->normalized_content().size();
            links_text_size += size;
            total_links_text_size += size;

            // DLOG(INFO) << "size:" << size;
            // DLOG(INFO) << "links_text_size: " << links_text_size;
            // DLOG(INFO) << "total_links_text_size:" << total_links_text_size;
            // DLOG(INFO) << "link_node.href()" << link_node->href();
            // DLOG(INFO) << "content:" << link_node->content();

            // if(link_node->href().size() == 0) {
            //     DLOG(INFO) << "link_node.src()" << link_node->src();
            //     DLOG(INFO) << "link_node.value()" << link_node->value();
            //     DLOG(INFO) << "link_node.alt()" << link_node->alt();
                
            // }
            
            if (size > 1 && size < 800 && link_node->href().size() >= 3
                    && link_node->href().find("attachment.") == std::string::npos
                    && link_node->content().find(".JPG") == std::string::npos
                    && link_node->content().find(".jpg") == std::string::npos
                    && link_node->content() != link_node->href()) {
                contain_good_link = true;
            }

            int rel_x = link_node->x() - node->x();
            DLOG(INFO) << "rel_x: " << rel_x << " " << link_node->x() << " " << node->x() \
                << " " << link_node->normalized_content();
            if (times == 1) {
                rel_x_vec.push_back(rel_x);
                std::string link_key = link_node->normalized_content();
                if (is_bbs) {
                } else {
                    link_key.append(link_node->href());
                }
                url_set.insert(link_key);
            } else {
                if (rel_x >= 0.95 * rel_x_vec[link_pos] && rel_x <= 1.05 * rel_x_vec[link_pos]) {
                    contain_same_rel = true;
                }

                std::string link_key = link_node->normalized_content();
                if (is_bbs) {
                    //link_key.append(Util::filter_number(link_node->href()));
                } else {
                    link_key.append(link_node->href());
                }

                if (url_set.find(link_key) == url_set.end()) {
                    url_set.insert(link_key);
                    all_same_links = false;
                }
            }

            ++link_pos;
        }

        if (!contain_good_link) {
            DLOG(INFO) << "don't contain good link";
            return false;
        }

        if (times > 1  && !contain_same_rel) {
            //return false;
        }

        //if (text_size >= 600 && links_text_size < 50) {
        if (links_text_size < 50 && \
                ((text_size > 800 && node->h() < 200) || (text_size > 650 && node->h() >= 200))) {
            DLOG(INFO) << "check text size fail " << text_size << " " << links_text_size;
            return false;
        }

        if (times > 1 && all_same_links) {
            ++all_same_links_times;
        }
    }

    int avg_h = total_h/group.size();
    if ((avg_h > 300 && total_links_text_size * 2 < total_text_size) || (is_bbs && avg_h > 150)) {
        DLOG(INFO) << "avg h check fail";
        return false;
    }

    if (group.size() <= 6 && all_same_links_times > 0) {
        DLOG(INFO) << "all same links check < 6";
        return false;
    } else if (group.size() > 6 && all_same_links_times >= 0.25 * group.size()) {
        DLOG(INFO) << "all same links check > 6";
        return false;
    }

    return true;
}

void Extractor::compute_list_confidence(Page &page)
{
    int region_width = page.doc_width;
    int region_height = page.doc_height;

    int max_height = 1.8 * SCREEN_HEIGHT;

    if (region_height < max_height) {
        region_height = max_height;
    } else if (region_height > max_height) {
        region_height = max_height;
    }

    int central_w = (int) (0.618 * region_width);
    int central_h = (int) (0.618 * region_height);
    int central_x = (int) (0.5 * (1 - 0.618) * region_width);
    int central_y = (int) (0.5 * (1 - 0.618) * region_height);
    int central_max_y = central_y + central_h;

    // int list_area_sum = 0;
    int list_items_count = 0;

    for (RepeatGroupListIter lit = page.group_list.begin(); lit != page.group_list.end(); ++lit) {
       
        bool need_count = false; 
        if (is_link_group(*lit, page.is_bbs)) {
            // computer area sum
            for (RepeatGroupIter it = lit->begin(); it != lit->end(); ++it) {
                vdom::Node *node = *it;
                // DLOG(INFO) << node->normalized_content();
                int x = node->x();
                int y = node->y();
                
                if (y > central_max_y) {
                    continue;
                }

                int w = node->w();
                int h = node->h();

                // fix a bug like http://trip56773.tripc.net/moban/Travellinelist_56773_6218_1.html.
                // sometimes, w,h is not correct.
                
                // TODO bug of qlibvdom
                if (h < 10 && node->normalized_content().size() > 1) {
                    int child_size = node->child_nodes_size();
                    for (int i = 0; i < child_size; ++i) {
                        if ((int)node->child_nodes(i).h() > h) {
                            h = node->child_nodes(i).h();
                        }
                    }
                }

                int x1 = VD_MAX(x, central_x);
                int x2 = VD_MIN(x + w, central_x + central_w);
                int y1 = VD_MAX(y, central_y);
                int y2 = VD_MIN(y + h, central_y + central_h);
                if (x2 > x1 && y2 > y1) {
                    int area = (x2 - x1) * (y2 - y1);
                    if (area * 2 > w * h) {
                        // list_area_sum += area;
                        need_count = true;
                    }
                }
            }

        }

        if (need_count) { 
            list_items_count += lit->size();
        }
    }

    int list_confidence = list_items_count * 10;
    if (list_confidence > 100) {
        list_confidence = 100;
    }

#if 0
    int list_confidence = list_area_sum * 100 / (central_w * central_h + 1);

    if (list_confidence > 100) {
        list_confidence = 100;
    }

    // delete this code by gambol. on 2011-8-16
    if (list_confidence > 0 && list_confidence <= 40) {
        // computer text area_sum
        int text_area_sum = 0;
        std::list<TextBlock> &list = page.block_list;
        for (TextBlockIter it = list.begin(); it != page.block_list.end(); ++it) {
            if (it->is_good()) {
                Node *node = it->node();
                int x = node->x();
                int y = node->y();
                if (y > central_max_y) {
                    continue;
                }
                int w = node->w();
                int h = node->h();
                int x1 = VD_MAX(x, central_x);
                int x2 = VD_MIN(x + w, central_x + central_w);
                int y1 = VD_MAX(y, central_y);
                int y2 = VD_MIN(y + h, central_y + central_h);
                if (x2 > x1 && y2 > y1) {
                    int area = (x2 - x1) * (y2 - y1);
                    if (area * 2 > w * h) {
                        text_area_sum += area;
                    }
                }

            }
        }
        int &text_confidence = page.text_confidence;

        text_confidence = text_area_sum * 100 / (central_w * central_h + 1);

        DLOG(INFO) <<  "text_confidence: " << text_confidence;
        DLOG(INFO) <<  "list_confidence: " << list_confidence;

        float weight = 1;
        if (page.title_block != NULL && page.title_block) {
            weight = 0.75;
        }
        //if (text_confidence >= 2.0 * list_confidence ||
        const std::string &page_title = page.doc->title();
        if (text_confidence >= 3.0 * list_confidence) {
            list_confidence = 0;
        } else if (page.title_block != NULL && page.ret->title.size() >= 20
                && page.title_block->page_title_similar() >= 0.33
                && page.title_block->page_title_similar() * page_title.size() >= 0.6 * page.ret->title.size()
                && list_confidence <= 32 && text_confidence >= 2) {
            list_confidence = 0;
        } else if (page.title_block != NULL && text_confidence >= 6 && list_confidence < 10) {
            list_confidence = 0;
        } else if (text_confidence >= 1.5 * list_confidence) {
            list_confidence = 0.4 * weight * list_confidence;
        } else if (text_confidence >= list_confidence) {
            list_confidence = 0.5 * weight * list_confidence;
        }

        /*
        } else if (text_confidence >= 1.5 * list_confidence) {
            list_confidence = 0.3 * weight * list_confidence;
        } else if (text_confidence >= list_confidence) {
            list_confidence = 0.4 * weight * list_confidence;
        } else if (text_confidence >= 0.5 * list_confidence && text_confidence >= 5) {
            list_confidence = 0.5 * weight * list_confidence;
        }
        */
    }
#endif
    
    page.ret->list_confidence = list_confidence;
}

bool Extractor::check_is_list(Page &page)
{
    return page.ret->list_confidence >= 100 || (page.ret->list_confidence > 50 && page.text_confidence < 6);
}

// added by ganbao in 5-15
void Extractor::compute_content_confidence(Page &page)
{
    const int GOOD_CONTENT_SIZE_THESHOLD = 500;
    const int GOOD_TOTAL_CONTENT_SIZE_THESHOLD = 6000;
    const int AVG_CONTENT_SIZE_NORM_FACTOR = 300;
    //    const int MAX_CONTENT_BLOCK_NUM_NORM_FACTOR = 20;
    const float GOOD_BLOCK_RATIO_NORM_FACTOR = 0.4;

    int max_2_screen_content_size = 0;
    int tmp_sum_content_size = 0;
    int total_content_size = 0;
    int avg_content_size = 0;

    int merged_content_block_num = 0;
    int good_block_num = 0;
    int content_block_num = 0;

    unsigned int two_screen_height = (unsigned int)(SCREEN_HEIGHT * 2);

    bool prev_is_content = false;

    std::list<TextBlock> &list = page.block_list;
    TextBlockIter end_it = list.end();

    int basic_content_confidence = 50;
    int bouns_content_confidence = 0;
    int other_content_confidence = 50;

    int start_content_block_y = 0;

    if (!check_is_list(page)) {
        for (TextBlockIter it = list.begin(); it != end_it; ++it) {
            if (!it->is_content()) {
                // 如果是非content block，则断开good block的合并。

                if (prev_is_content) {
                    // 开始计数
                    merged_content_block_num++;
                    if (max_2_screen_content_size < tmp_sum_content_size) {
                        max_2_screen_content_size = tmp_sum_content_size;
                    }
                } // end of if(prev_is_content)

                tmp_sum_content_size = 0;
                prev_is_content = false;

                continue;
            } else {   // 是content block块
                content_block_num++;

                if (!prev_is_content) {
                    // 一个大的，被merge之后的content block起始的y 地址
                    start_content_block_y = it->node()->y();
                }

                Node *node = it->node();
                int content_size = node->normalized_content().size();
                total_content_size += content_size;

                // 如果是前2屏幕，则加到tmp_sum_content_size里，用来统计前2屏最大的块
                if (start_content_block_y < two_screen_height) {
                    tmp_sum_content_size += content_size;
                }

                if (it->is_good()) {
                    good_block_num++;
                }
                prev_is_content = true;
            }
        } // end of for(TextIterator

        // 计算最后一个块
        if (prev_is_content) {
            merged_content_block_num++;
            if (max_2_screen_content_size < tmp_sum_content_size) {
                max_2_screen_content_size = tmp_sum_content_size;
            }
        }

        if (0 != content_block_num && 0 != merged_content_block_num) {
            float good_block_ratio = (float)good_block_num/content_block_num;
            avg_content_size = total_content_size/merged_content_block_num;

            // debug info
            /*
            std::cout << "good_block_num:" << good_block_num<< std::endl;
            std::cout << "content_block_num:" << content_block_num<< std::endl;
            std::cout << "good_block_ratio:" << good_block_ratio<< std::endl;
            std::cout << "merged_block_num:" << merged_content_block_num<< std::endl;
            std::cout << "max_2_screen_content_size:" << max_2_screen_content_size<< std::endl;
            std::cout << "total_content_size:" << total_content_size << std::endl;
            std::cout << "avg_content_size:" << avg_content_size << std::endl;;
            */

            if (max_2_screen_content_size > GOOD_CONTENT_SIZE_THESHOLD ||
                total_content_size > GOOD_TOTAL_CONTENT_SIZE_THESHOLD) {
                //文字数目非常多
                bouns_content_confidence = other_content_confidence;
            } else {
                //注意以下除法， 分母都需要为正数
                float block_ratio_factor = \
                    (good_block_ratio - 0.1) / (GOOD_BLOCK_RATIO_NORM_FACTOR - 0.1);

                block_ratio_factor = block_ratio_factor > 1 ? 1 : block_ratio_factor;

                float avg_content_size_factor = \
                    (float)(avg_content_size - 50) / (AVG_CONTENT_SIZE_NORM_FACTOR - 50);

                avg_content_size_factor = avg_content_size_factor > 1 ? 1 : avg_content_size_factor;

                float screen_2_content_size_ratio = \
                    (float)(max_2_screen_content_size - 100) / (GOOD_CONTENT_SIZE_THESHOLD - 100);

                bouns_content_confidence = \
                    int(10*block_ratio_factor + 20*avg_content_size_factor + 20*screen_2_content_size_ratio);
            }
        }  // end of if (0 != content_block_num)
    }

    // 列表页，content_confidence统一为50
    page.ret->content_confidence = basic_content_confidence + bouns_content_confidence;
}

} //namespace vdom
} //namespace content


