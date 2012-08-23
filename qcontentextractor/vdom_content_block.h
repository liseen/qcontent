/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

#ifndef VDOM_CONTENT_BLOCK_H
#define VDOM_CONTENT_BLOCK_H

#include <string>
#include <iostream>

#include <glog/logging.h>

#include <vdom.h>

#include "vdom_content_util.h"

namespace vdom {
namespace content {

struct TextBlock {
    TextBlock() {
        _node = NULL;

        _has_anchor_length = false;
        _has_anchor_ratio = false;
        _has_tag_density = false;
        _has_punct_count = false;
        _has_punct_ratio = false;
        _has_space_ratio = false;
        _has_contain_outside_link = false;
        _has_contain_advertise_link = false;
        _has_title_weight = false;

        _contain_outside_link = false;
        _contain_advertise_link = false;
        _is_good = false;
        _is_bad = false;
        _is_content = false;
        _prev_is_noise = false;
        _next_is_noise = false;
        _title_weight = 0;
        _page_title_similar = 0;
    }

public:
    inline bool has_anchor_length() {
        return _has_anchor_length;
    }

    int compute_anchor_length(Node *cnode) {
        if (cnode->tag_name() == "A") {
            return cnode->content().size();
        }

        int al = 0;
        int child_size = cnode->child_nodes_size();
        for (int i = 0; i < child_size; ++i) {
            Node *child_node = cnode->mutable_child_nodes(i);
            if (child_node->tag_name() == "A") {
                al += child_node->content().size();
            } else if (child_node->type() == Node::ELEMENT) {
                al += compute_anchor_length(child_node);
            }
        }

        return al;
    }

    int anchor_length() {
        if (has_anchor_length()) {
            return _anchor_length;
        } else {
            _has_anchor_length = true;
            _anchor_length = compute_anchor_length(_node);
            return _anchor_length;
        }
    }

    inline bool has_anchor_ratio() {
        return _has_anchor_ratio;
    }

    inline int anchor_ratio() {
        if (has_anchor_ratio()) {
            return _anchor_ratio;
        } else {
            _has_anchor_ratio = true;
            size_t total = _node->content().size();
            if ( total == 0) {
                _anchor_ratio = 0;
            }  else {
                _anchor_ratio = 100 * anchor_length() / total;
            }
            return _anchor_ratio;
        }
    }

    inline bool has_tag_density() {
        return _has_tag_density;
    }

    int compute_tag_count(Node* cnode) {
        int tc = 0;
        int child_size = cnode->child_nodes_size();
        tc += child_size;
        for (int i = 0; i < child_size; ++i) {
            Node *child_node = cnode->mutable_child_nodes(i);
            if (child_node->type() == Node::ELEMENT) {
                tc += compute_tag_count(child_node);
            }
        }
        return tc;
    }

    float tag_density() {
        if (has_tag_density()) {
            return _tag_density;
        } else {
            _has_tag_density = true;
            int tag_cnt = compute_tag_count(_node);
            size_t total = _node->content().size();
            if ( total == 0) {
                _tag_density = 0;
            }  else {
                _tag_density = (float)tag_cnt / total;
            }
            return _tag_density;
        }
    }

    inline bool has_punct_count() {
        return _has_punct_count;
    }

    int punct_count() {
        if (has_punct_count()) {
            return _punct_count;
        } else {
            _has_punct_count = true;
            // TODO
            _punct_count = 1;
        }
    }

    inline bool has_punct_ratio() {
        return _has_punct_ratio;
    }

    int punct_ratio() {
        if (has_punct_ratio()) {
            return _punct_ratio;
        } else {
            _has_punct_ratio = true;

            if (_node->content().size() == 0) {
                _punct_ratio = 0;
            } else {
                _punct_ratio = punct_count() / _node->content().size();
            }

            return _punct_ratio;
        }
    }

    inline int content_size() {
        return _node->content().size();
    }

    inline bool has_space_ratio() {
        return _has_space_ratio;
    }

    int space_count() {
        int cnt = 0;
        const char *p = _node->content().c_str();
        int size = _node->content().size();
        for (int i = 0; i < size; i++) {
            switch (*(p + i)) {
                case ' ':
                case '\t':
                case '\n':
                    ++cnt;
                    break;
            }
        }

        return cnt;
    }

    int space_ratio() {
        if (has_space_ratio()) {
            return _space_ratio;
        } else {
            _has_space_ratio = true;

            if (content_size() == 0) {
                _space_ratio = 0;
            } else {
                _space_ratio = 100 * space_count() / content_size();
            }

            return _space_ratio;
        }
    }

    inline bool has_contain_outside_link() const {
        return _has_contain_outside_link;
    }

    inline bool contain_outside_link() const {
        if (has_contain_outside_link()) {
            return _contain_outside_link;
        } else {
            if (_node->tag_name() ==  "A") {
            } else {

            }
        }
    }

    inline bool is_good() {
        return _is_good;
    }
    inline void set_is_good(bool is) {
        _is_good = is;
    }

    inline bool is_content() {
        return _is_content;
    }
    inline void set_is_content(bool is) {
        _is_content = is;
    }

    inline bool is_bad() {
        return _is_bad;
    }

    inline void set_is_bad(bool is) {
        _is_bad = is;
    }

    inline bool prev_is_noise() {
        return _prev_is_noise;
    }

    inline void set_prev_is_noise(bool is) {
        _prev_is_noise = is;
    }

    inline bool next_is_noise() {
        return _next_is_noise;
    }

    inline void set_next_is_noise(bool is) {
        _next_is_noise = is;
    }

    inline Node* node() {
        return _node;
    }

    inline void set_node(Node *n) {
        _node = n;
    }

    inline bool compute_title_weight(const std::string &filtered_page_title, bool contain_seo) {
        std::list<vdom::Node*> all_texts;
        _node->get_all_texts(all_texts);

        int all_texts_size = 0;
        float all_texts_font_weight = 0;
        int all_normal_texts_count = 0;

        for (std::list<vdom::Node*>::iterator it = all_texts.begin(); it != all_texts.end(); ++it) {
            vdom::Node *txt = *it;

            int txt_normal_size = txt->normalized_content().size();
            if (txt_normal_size <= 1) {
                continue;
            }

            ++all_normal_texts_count;

            const vdom::Node *parent = txt->parent_node();
            float font = 0.7 * ((float)parent->numeric_font_size() * parent->numeric_font_weight() / (400 * 12)) - 0.6;
            if (font > 1.5) {
                font = 1.5;
            }
            all_texts_font_weight += txt_normal_size * font;
            all_texts_size += txt_normal_size;
        }

        if (all_normal_texts_count > 5) {
            return false;
        }

        float font_title_weight = 0.0;
        if (all_texts_size != 0) {
            font_title_weight = all_texts_font_weight / all_texts_size;
        }

        const std::string &content = _node->normalized_content();
        float edit_similar = 0;
        int dist = Util::edit_distance(content, filtered_page_title);
        if (filtered_page_title.size() > 0) {
            int max_size = content.size() > filtered_page_title.size() ? content.size() : filtered_page_title.size();
            float diff_dist = abs(max_size - dist);
            edit_similar = diff_dist/max_size;
        } else {
            edit_similar = 0.75;
        }

        if (contain_seo) {
            _title_weight = font_title_weight/1.5 + 1.7 * edit_similar;
        } else {
            _title_weight = font_title_weight + 1.2 * edit_similar;
        }

        DLOG(INFO) << "cadtitle content: " << _node->normalized_content();
        DLOG(INFO) << "sig: " << _node->repeat_sig();
        DLOG(INFO) << "font_title_weight: " << font_title_weight;
        DLOG(INFO) << "edit similar: " << edit_similar;
        DLOG(INFO) << "title weight " << _title_weight;


        const std::string &tag_name = _node->tag_name();
        if (tag_name == "H1") {
            _title_weight += 0.3;
        } else if (tag_name == "H2") {
            _title_weight += 0.2;
        } else if (tag_name == "H3") {
            _title_weight += 0.1;
        } else if (tag_name == "B") {
            _title_weight += 0.1;
        }

        _has_title_weight = true;
        _page_title_similar = edit_similar;
        _title_weight = _title_weight / 3;
        return true;
    }

    inline float title_weight() {
        assert(_has_title_weight);
        return _title_weight;
    }

    inline float page_title_similar() {
        assert(_has_title_weight);
        return _page_title_similar;
    }

    void debug_print() {
        if (_node == NULL) {
            DLOG(INFO) << "content: " << "node is null";
        } else {
            DLOG(INFO)  << "type: " << _node->type();
            DLOG(INFO) << "tagname: " << _node->tag_name();
            DLOG(INFO) << "allchildreninline: " << _node->all_children_inline();
            DLOG(INFO) << "x: " << _node->x();
            DLOG(INFO) << "y: " << _node->y();
            DLOG(INFO) << "w: " << _node->w();
            DLOG(INFO) << "h: " << _node->h();
            DLOG(INFO) << "is_good: " << is_good();
            DLOG(INFO) << "is_bad: " << is_bad();
            DLOG(INFO) << "prev_is_noise: " << prev_is_noise();
            DLOG(INFO) << "next_is_noise: " << next_is_noise();
            DLOG(INFO) << "is_content: " << is_content();
            DLOG(INFO) << "content_size: " << content_size();
            DLOG(INFO)<< "content: " << _node->content();
            DLOG(INFO) << "normalized_content: " << _node->normalized_content();
            DLOG(INFO) << "tag_desity: " << tag_density();
            DLOG(INFO) << "space_ratio " << space_ratio();
            DLOG(INFO) << "anchor_length: " << anchor_length();
            DLOG(INFO) << "anchor_ratio: " << anchor_ratio();
            DLOG(INFO) << "repeat_sig: " << _node->repeat_sig();
        }
    }

private:
    // ratio ~[0-100]
    bool _has_anchor_ratio;
    int _anchor_ratio;
    bool _has_anchor_length;
    int _anchor_length;
    bool _has_tag_density;
    float _tag_density;
    bool _has_punct_count;
    int _punct_count;
    bool _has_punct_ratio;
    int _punct_ratio;
    bool _has_space_ratio;
    int _space_ratio;
    float _title_weight;
    float _page_title_similar;

    bool _has_contain_outside_link;
    bool _contain_outside_link;
    bool _has_contain_advertise_link;
    bool _contain_advertise_link;
    bool _prev_is_noise;
    bool _next_is_noise;
    bool _has_title_weight;
    bool _is_good;
    bool _is_bad;
    bool _is_content;
    std::string text;
    Node *_node;
};

} //namespace vdom
} //namespace content

#endif
