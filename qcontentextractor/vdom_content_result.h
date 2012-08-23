/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

#ifndef VDOM_CONTENT_RESULT_H
#define VDOM_CONTENT_RESULT_H

#include <list>
#include <string>
#include <iostream>

#include "vdom_content_util.h"

namespace vdom {
namespace content {

// TODO image src and around text
struct ImageResult {
    std::string src;
    std::string alt;
    std::string around_text;
};

struct Result
{
    Result() {
        extracted_okay = false;
        list_confidence = 0; // %100
        content_confidence = 0; // %100
        publish_time = 0;
    }

    bool extracted_okay;

    int list_confidence; // %100
    int content_confidence; // %100
    int publish_time;
    std::string raw_title;
    std::string title;
    std::string keywords;
    std::string description;
    std::string content;
    std::string content_attr;
    std::list<std::string> urls;
    std::list<ImageResult> images;
    std::string images_str;

    void test_print() {
        std::cout << "\n--- list_confidence\n" << list_confidence;
        std::cout << "\n--- content_confidence\n" << content_confidence;
        std::cout << "\n--- pushlish_time\n" << publish_time;
        std::cout << "\n--- images\n" << images_str;
        std::string normal_title;
        Util::normalize_content(title, normal_title);
        std::cout << "\n--- title\n" << normal_title;
        //std::string normal_content;
        //Util::filter_content(content, normal_content);
        //Util::normalize_content(content, normal_content);
        std::cout << "\n--- content\n" << content;
        std::cout << "\n--- content_attr\n" << content_attr;
    }

    void review_print(const std::string &key) {
        std::string normal_image;
        Util::normalize_content(images_str, normal_image);

        std::cout << key
            << "\t" << list_confidence
            << "\t" << content_confidence
            << "\t" << publish_time
            << "\t" << raw_title
            << "\t" << title
            << "\t" << content
            << "\t" << normal_image << "\n";
    }

    void debug_print() {
        std::cout << "okay: " << extracted_okay << std::endl;
        std::cout << "list_confidence: " << list_confidence << std::endl;
        std::cout << "content_confidence: " << content_confidence << std::endl;
        std::cout << "title: " << title << std::endl;
        std::cout << "pushlish_time: " << publish_time << std::endl;
//        std::cout << "keywords: " << keywords << std::endl;
//        std::cout << "description: " << description << std::endl;
//        std::cout << "content: " << content << std::endl;
        //std::string normal_content;
        //Util::filter_content(content, normal_content);
        std::cout << "normal_content: " << content << std::endl;
        //std::cout << "normal_content: " << content << std::endl;
    }
};

} //namespace vdom
} //namespace content

#endif
