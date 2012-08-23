/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

#ifndef VDOM_CONTENT_UTIL_H
#define VDOM_CONTENT_UTIL_H

#include <string>
#include <vector>

namespace vdom {
namespace content {

class Util {
public:
    static bool is_noise_tag_name(const std::string &tag_name);
    static bool is_bbs_signature(const std::string &class_name);
    static bool is_binary_link(const std::string &href);
    static bool contain_noise_title(const std::string &text);
    static bool contain_noise_content(const std::string &text);
    static bool contain_good_punct(const std::string &text);

    static bool title_contain_seo(const std::string &page_title);
    static int match_date(const std::string &text, int download_time, bool is_title_block, bool pre_is_title_block);
    static void split_text(const std::string &text, std::vector<std::string> &str_vec, unsigned char sep = '\t');

    //static bool noise_publish_time(const std::string &content);

    //static int utf8_to_utf16(const char *str, size_t size, int &char_size);
    //
    static int levenshtein(const char *s1, int l1, const char *s2, int l2);
    static unsigned int edit_distance(const std::string& s1, const std::string& s2);
    static bool is_good_image_link(const std::string &src);
    static bool is_noise_image_link(const std::string &src);
    static bool is_bbs_link(const std::string &src);

    static void filter_page_title(const std::string &page_title, std::string &filtered_title);

    static void normalize_text_to_line(const std::string &raw, std::string &nomalized);
    static void normalize_content(const std::string &content, std::string &filtered_content);
    static void normalize_title(const std::string &content, std::string &filtered_content);
    static std::string filter_number(const std::string &src);
};

}
}

#endif
