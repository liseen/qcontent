/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

/*
 *
 *
 */

#ifndef QVDDOM_TO_HTML_H
#define QVDDOM_TO_HTML_H

#include <vdom.h>

namespace qcontent
{

class QVDom2Html
{
public:
    //static bool binary_vdom_to_html(const std::string &binary_vdom, std::string &html);
    static bool text_vdom_to_html(const std::string &vdom, std::string &html);
    static bool vdom_to_html(const vdom::Window &win, std::string &html);
};


} // end namespace qcontent

#endif
