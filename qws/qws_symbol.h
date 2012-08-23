/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

#ifndef QCONTENT_QWS_SYMBOL_H
#define QCONTENT_QWS_SYMBOL_H

#include <vector>
#include <iostream>

#include "qws_common.h"

BEGIN_NAMESPACE_QWS

struct QWSSymbol
{
    int offset;

    int origin_start;
    int origin_end;
    int origin_value;

    int normalized_start;
    int normalized_end;

    int is_sentence_separator;

    int value;
    int token_position;

    void display(const std::string &origin_text, const std::string &/*normalized_text*/) {
        std::cerr \
            << "offset: " << offset << "\n"
            << "origin_start: " << origin_start <<  "\n"
            << "origin_end: " << origin_end << "\n"
            << "origin_value: " << origin_value << "\n"
            << "origin_sym: " << origin_text.substr(origin_start, origin_end - origin_start) << "\n"
            << "value: " << value << "\n"
            << "is_sentence_separator: " << is_sentence_separator << "\n"
            << "token_position: " << token_position << "\n";

        std::cerr << std::endl;
    }
};

typedef std::vector<QWSSymbol> QWSSymbolList;
typedef std::vector<QWSSymbol>::iterator QWSSymbolListIter;
typedef std::vector<QWSSymbol>::const_iterator QWSSymbolListConstIter;

END_NAMESPACE_QWS

#endif
