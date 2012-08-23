/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

#ifndef QCONTENT_QWS_UTIL_H
#define QCONTENT_QWS_UTIL_H

#include <string>

#include "qws_common.h"

BEGIN_NAMESPACE_QWS

class QWSUtil {
public:
    static bool utf8_to_utf16(const std::string &utf8_text, std::string &utf16_text);
    static bool symbol_to_utf8(int val, std::string &word);

    inline static bool is_sentence_separator(int val) {
        if (val == static_cast<int>(' ') \
                ||  val == static_cast<int>(',') \
                ||  val == static_cast<int>('.') \
                ||  val == static_cast<int>('?') \
                ||  val == static_cast<int>('!') \
                ||  val == static_cast<int>('"') \
                ||  val == static_cast<int>(';') \
                ||  val == static_cast<int>(':') \
                ||  val == static_cast<int>('(') \
                ||  val == static_cast<int>(')') \
                ||  val == static_cast<int>('[') \
                ||  val == static_cast<int>(']') \
                ||  val == static_cast<int>('{') \
                ||  val == static_cast<int>('}') \
                ||  val == static_cast<int>('=') \
                ||  val == static_cast<int>('=') \
                ||  val == static_cast<int>('\\') \
                ||  val == static_cast<int>('/') \
                ||  val == static_cast<int>('%') \
           ) {
            return true;
        } else {
            return false;
        }
    }

    inline static bool is_english_symbol(int val) {
        return val <= 127;
    }


};

END_NAMESPACE_QWS

#endif
