/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

#ifndef VDOM_CONTENT_PAGE_H
#define VDOM_CONTENT_PAGE_H

#include <list>

#include <vdom.h>

#include "vdom_content_block.h"
#include "vdom_content_result.h"

namespace vdom {
namespace content {

typedef std::list<TextBlock> TextBlockList;
typedef std::list<TextBlock>::iterator TextBlockIter;
typedef std::list<TextBlock>::reverse_iterator TextBlockRIter;

typedef std::list<Node*> RepeatGroup;
typedef std::list<Node*>::iterator RepeatGroupIter;
typedef std::list<RepeatGroup> RepeatGroupList;
typedef std::list<RepeatGroup>::iterator RepeatGroupListIter;

struct Page {
    Page() : win(NULL), doc(NULL), body(NULL), ret(NULL), \
             doc_width(0), doc_height(0), text_confidence(100), title_block(NULL), is_bbs(false) {
    }

    vdom::Window *win;
    vdom::Document *doc;
    vdom::Node *body;

    int download_time;

    Result *ret;
    std::string host;
    std::string url;
    int doc_width;
    int doc_height;

    int text_confidence;
    TextBlock *title_block;
    bool is_bbs;
    std::list<TextBlock> block_list;
    RepeatGroupList group_list;
};

} //namespace vdom
} //namespace content

#endif

