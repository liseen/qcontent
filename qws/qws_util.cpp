/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

#include "qws_util.h"

#include "utf8.h"
#include <cstring>

BEGIN_NAMESPACE_QWS

bool QWSUtil::utf8_to_utf16(const std::string &utf8_text, std::string &utf16_text)
{
    const char *w = utf8_text.c_str();
    //const char *prev_w = w;
    const char *end = utf8_text.c_str() + utf8_text.size();

    try {
        while (w != end) {
            int cp = utf8::next(w, end);
            if (cp > 0 && cp <= 0xFFFF) {
                unsigned char buf[2];
                buf[0] = cp & 0xFF;
                buf[1] = (cp >> 8) & 0xFF;
                utf16_text.append((const char*)buf, 2);
            }
        }
    } catch (std::exception &e) {
        return false;
    }

    return true;
}

bool QWSUtil::symbol_to_utf8(int val, std::string &word)
{
    char utf8_buf[4];
    memset(utf8_buf, 0, 4);
    char *utf8_end = utf8::unchecked::append(val, utf8_buf);
    word.append(utf8_buf, utf8_end - utf8_buf);

    return true;
}

END_NAMESPACE_QWS
