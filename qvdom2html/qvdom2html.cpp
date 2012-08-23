/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

#include <google/protobuf/text_format.h>

#include <string>
#include <sstream>
#include "qvdom2html.h"


namespace qcontent
{
static void node2text(const vdom::Node *parent, const vdom::Node &node, std::string &html);
static void node2html(const vdom::Node *parent, const vdom::Node &node, std::string &html);
static std::string escape_text(const std::string &text);
static void gen_html_attrs(const vdom::Node *parent, const vdom::Node &elem, std::string &html);
static void txtnode2html(const vdom::Node *parent, const vdom::Node &txtnode, std::string &html);

bool QVDom2Html::text_vdom_to_html(const std::string &vdom, std::string &html)
{
    vdom::Window win;
    ::google::protobuf::TextFormat::ParseFromString(vdom, &win);
    return vdom_to_html(win, html);
}


bool QVDom2Html::vdom_to_html(const vdom::Window &win, std::string &html)
{
    html.reserve(1024 * 1024);

    html += "<html>\n";
    html += "<head>\n";
    html += "<style type=\"text/css\">"
    "li.header_li_link {"
    "    text-align: center;"
    "    float: left;"
    "    margin: 3px;"
    "    padding: 3px;"
    "    list-style-type: none;"
    "}"
    "html, body, div, span, applet, object, iframe,"
    "h1, h2, h3, h4, h5, h6, p, blockquote, pre,"
    "a, abbr, acronym, address, big, cite, code,"
    "del, dfn, em, font, img, ins, kbd, q, s, samp,"
    "small, strike, strong, sub, sup, tt, var,"
    "b, u, i, center,"
    "dl, dt, dd, ol, ul, "
    "fieldset, form, label, legend,"
    "table, caption, tbody, tfoot, thead, tr, th, td {"
    "    clear: both;"
    "}";
    html += "</style>\n";
/*
"html, body, div, span, applet, object, iframe,"
"h1, h2, h3, h4, h5, h6, p, blockquote, pre,"
"a, abbr, acronym, address, big, cite, code,"
"del, dfn, em, font, img, ins, kbd, q, s, samp,"
"small, strike, strong, sub, sup, tt, var,"
"b, u, i, center,"
"dl, dt, dd, ol, ul, li,"
"fieldset, form, label, legend,"
"table, caption, tbody, tfoot, thead, tr, th, td {"
"    margin: 0;"
"    padding: 0;"
"    border: 0;"
"    outline: 0;"
"    font-size: 100%;"
"    vertical-align: baseline;"
"    background: transparent;"
"}"
"body {"
"    line-height: 1;"
"}"
"ol, ul {"
"    list-style: none;"
"}"
"blockquote, q {"
"    quotes: none;"
"}"
"blockquote:before, blockquote:after,"
"q:before, q:after {"
"    content: '';"
"    content: none;"
"}"
":focus {"
"    outline: 0;"
"}"
"ins {"
"    text-decoration: none;"
"}"
"del {"
"    text-decoration: line-through;"
"}"
"table {"
"    border-collapse: collapse;"
"    border-spacing: 0;"
"}\n";
*/
    html += "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />\n";
    html += "<title>";
    html += escape_text(win.doc().title());
    html += "</title>\n";
    html += "</head>\n";

    node2html(NULL, win.doc().body(), html);
    html += "</html>\n";

    return true;
}

// TODO
std::string escape_text(const std::string &txt)
{
    std::stringstream str;
    int len = txt.size();
    for (int i = 0; i < len; ++i) {
        if (txt[i] == '&') {
            str << "&amp;";
        } else if(txt[i] == '<') {
            str << "&lt;";
        } else if(txt[i] == '>') {
            str << "&gt;";
        } else if(txt[i] == '"') {
            str << "&quot;";
        } else {
            str << txt[i];
        }
    }

    return txt;
}

void node2text(const vdom::Node *parent, const vdom::Node &node, std::string &html) {
    if (node.type() == vdom::Node::TEXT) {
        html += node.content();
    } else {
        int child_size = node.child_nodes_size();
        for (int i = 0; i < child_size; ++i) {
            node2text(&node, node.child_nodes(i), html);
        }
    }
}

void node2html(const vdom::Node *parent, const vdom::Node &node, std::string &html)
{
    if (node.tag_name() == "BR") {
        html += "<br/>\n";
    } else if (node.tag_name() == "IFRAME") {// && node.x() * node.y() < 400 * 500) {
        //html += "<br/>\n";
    } else if (node.type() == vdom::Node::ELEMENT) {
        html += "<";
        html += node.tag_name();
        gen_html_attrs(parent, node, html);
        html += ">\n";

        int child_size = node.child_nodes_size();
        for (int i = 0; i < child_size; ++i) {
            node2html(&node, node.child_nodes(i), html);
        }
        html += "\n</";
        html += node.tag_name();
        html += ">\n";
    } else {
        txtnode2html(parent, node, html);
    }
}

void gen_html_attrs(const vdom::Node *parent, const vdom::Node &elem, std::string &html)
{
    /*
     id name class_name
    */

    if (!elem.href().empty()) {
        html += " href=\"";
        html += escape_text(elem.href());
        html += "\"";
    }

    if (!elem.src().empty()) {
        html += " src=\"";
        html += escape_text(elem.src());
        html += "\"";
    }

    if (!elem.alt().empty()) {
        html += " alt=\"";
        html += escape_text(elem.alt());
        html += "\"";
    }

    if (elem.tag_name() == "LI" && elem.y() < 250) {
        html += " class=\"header_li_link\"";
    }

    int left = 0;
    int top = 0;
    if (parent != NULL) {
        left = elem.x() - parent->x();
        top = elem.y() - parent->y();
    } else {
        left = elem.x();
        top = elem.y();
    }

    std::stringstream style;
    style << "color: " << elem.color() << ";"
        << "background-color: " << elem.background_color() << ";"
        << "font-size: " << elem.font_size() << ";"
        << "font-weight: " << elem.font_weight() << ";"
        << "font-family: " << elem.font_family() << ";"
        << "font-style: " << elem.font_style() << ";";

    style << "position: absolute;"
            << "left:" << (int)(left * 1.0) << ";"
            << "top:" << (int)(top * 1.0)<< ";"
            << "width:" << (int)(elem.w() * 1.0)  << ";"
            << "height:" << (int)(elem.h() * 1.0) << ";";
 
/*
   if (elem.render_type() != vdom::Node::INLINE) {
        style << "position: absolute;"
            << "left:" << (int)(left * 1.0) << ";"
            << "top:" << (int)(top * 1.0)<< ";"
            << "width:" << (int)(elem.w() * 1.0)  << ";"
            << "height:" << (int)(elem.h() * 1.0) << ";";
   }
*/
   //if (elem.render_type() == vdom::Node::BLOCK || elem.render_type() == vdom::Node::INLINE) {
        /*style << "position: absolute;"
            << "left:" << left << ";"
            << "top:" << top << ";"
            << "width:" << elem.w() << ";"
            << "height:" << elem.h() << ";";
        */
   //}

    std::string style_str = style.str();
    if (!style_str.empty()) {
        html += " style=\"";
        html += escape_text(style_str);
        html += "\" ";
    }
}

void txtnode2html(const vdom::Node *parent, const vdom::Node &node, std::string &html)
{
    if (node.w() == 0) {
        return;
    }

    int left = 0;
    int top = 0;
    if (parent != NULL) {
        left = node.x() - parent->x();
        top = node.y() - parent->y();
    } else {
        left = node.x();
        top = node.y();
    }

    if (top <= 10) {
        top = 0;
    }

    std::stringstream style;

    style << "position: absolute;"
        << "left:" << left << ";"
        << "top:" << top << ";"
        << "width:" << node.w() << ";"
        << "height:" << node.h() << ";";

    std::string style_str = style.str();
    html += "<span";
    if (!style_str.empty()) {
        html += " style=\"" + style_str;
    }
    html += "\">" + escape_text(node.text());
    html += "</span>";
}

}
