/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

#include <pcre.h>
#include <stdio.h>
#include <cassert>
#include <iostream>
#include <strtk.hpp>
#include <glog/logging.h>

#include "vdom_content_util.h"
namespace vdom {
namespace content {

#define OVECCOUNT 60    /* should be a multiple of 3 */
#define QMIN3(a,b,c) (a < b ? \
    (a < c ? a : c) : \
    (b < c ? b : c))

static pcre *noise_tag_name_re = NULL;
static const char *noise_tag_name_pat = "(?:TEXTAREA|INPUT|BUTTON|IFRAME)";

static pcre *bbs_signature_re = NULL;
static const char *bbs_signature_pat = "^(?:signatures|t_signature|signed|sign|signature)$";


static pcre *binary_link_re = NULL;
static const char *binary_link_pat = "(?:\\.(?:arff|au|avi|class|exe|fig|gif|gz|hqx|ica|jpeg|jpg|mat|mdb|mov|mp3|mpeg|mpg|msi|pcx|pdb|psd|ram|rar|raw|rmd|rmx|sav|sdd|shar|tar|tga|tgz|tif|tiff|vo|wav|wmv|wmz|xbm|xpm|z|zip)$)";

static pcre *good_image_link_re = NULL;
static const char *good_image_link_pat = "^http.*\\.(?:(?:jpg|jpeg|png)$|\\bpic\\b)";

static pcre *noise_image_link_re = NULL;
static const char *noise_image_link_pat = "(?:avatar_middle|banner|\\blogo|\\bsign\\b|\\bweibo\\b)";

static pcre *bbs_link_re = NULL;
static const char *bbs_link_pat = "(?:thread(?:-\\d+)+\\.html|\\bviewthread\\b|\\bshowthread\\b|\\bshowtopic\\b|\\bforum\\b|\\bviewtopic\\b|\\bbbs\\b|\\bclub\\b|\\btid-\\d+|\\btid=\\d+|bbs.asp\\b|\\btopic-\\d+|\\bcomment\\b|\\bcomments\\b)";

static pcre *noise_title_re = NULL;
static const char *noise_title_pat = "(?:copyright|版权|正在载入|友情链接|用户发表意见|免责申明|免责声明|法律申明|尊重网上道德|联系我们|注册|详情请看|本网站不保证|\\b首页|备案|打印本页|只看该作者|\\[\\s*只看此人\\s*\\]|^\\s*-{15,}\\s*$|Post By：\\d{4}|您是本帖第|人关注过本帖|下一页|免费服务|来源\\s*:|来源\\s*：|发布：|收录时间|提问时间|^\\s*关键词\\s*(?::|：)|\\bICP|©|\\[打印\\]|MSN|QQ|回复|\\[详细\\]|上一条:|下一条:|本文一共被浏览|我来说两句|如果这篇文章|http:\\/\\/travel.sina.com.cn|点击下载|All Rights Reserved|保险在线购卖|作者被禁止或删除|\\[编辑帖子\\])";


static pcre *noise_content_re = NULL;
static const char *noise_content_pat = "(?:copyright|版权|正在载入|友情链接|用户发表意见|免责申明|免责声明|法律申明|尊重网上道德|联系我们|注册|详情请看|本网站不保证|\\b首页|>|备案|打印本页|只看该作者|\\[\\s*只看此人\\s*\\]|^\\s*-{15,}\\s*$|Post By：\\d{4}|您是本帖第|人关注过本帖|下一页|免费服务|来源\\s*:|来源\\s*：|发布：|收录时间|提问时间|^\\s*关键词\\s*(?::|：)|\\bICP|©|\\[打印\\]|MSN|QQ|回复|\\[详细\\]|上一条:|下一条:|本文一共被浏览|我来说两句|如果这篇文章|http:\\/\\/travel.sina.com.cn|点击下载|All Rights Reserved|保险在线购卖|作者被禁止或删除|\\[编辑帖子\\]|★★★★★★★★★★★★★★★★★★)";

static pcre *punct_re = NULL;
static const char *punct_pat = "(?:，|。|？|!|；|,|\\.|\\?|!|;|~)";

static pcre *noise_date_re = NULL;
static const char *noise_date_pat = "(?:注册|注册时间|最后登录|上市时间|登录时间|相关新闻|~|～|註冊|註冊時間|最後登錄|上市時間|登錄時間|相關新聞)";

static pcre *date_re = NULL;
static const char *date_pat = "((?:19|20)\\d{2})\\s*(?:年|－|\\.|-|/)\\s*(\\d{1,2})\\s*(?:月|－|\\.|-|/)\\s*(\\d{1,2})(?:\\s*日|\\s*号|(?:\\s+|]\\s*)(?:\\d{1,2}:\\d{1,2}(?:[:\\d\\.]*)?)?|$)\\s*";

static pcre *date_prefix_re = NULL;
static const char *date_prefix_pat = "(?:\\bcom\\b|解决时间|发布日期|发布时间|发布|Published\\s*Date|报道|更新日期|更新时间|发表时间|发表日期|出版日期|出版时间|添加时间|添加日期|刊登时间|创建时间|Post\\s+By|发表于|来源|来自|作者|编辑|回答|修改|解決時間|發佈日期|發佈時間|發表時間|責編\\s*(?::|：)|來源\\s*(?::|：)|稿源|發佈|報導|報道|更新日期|更新時間|發表時間|發表日期|出版時間|添加時間|添加日期|刊登時間|最後更新時間|發表於|來源\\s*(?::|：)|編輯\\s*(?::|：))";

static pcre *date_suffix_re = NULL;
static const char *date_suffix_pat = "(?:来源|稿源|作者|编辑|來源|責編|編輯|發佈|報導|報道|提问者|更新|发布|回复)";

static pcre *date_yestoday_re = NULL;
static const char *date_yestoday_pat = "(?:发表于\\s*昨天)";

static pcre *date_before_yestoday_re = NULL;
static const char *date_before_yestoday_pat = "(?:发表于\\s*前天)";

static pcre *filter_page_title_re = NULL;
static const char *filter_page_title_pat = "";

static pcre *contain_seo_re = NULL;
static const char *contain_seo_pat = "(?:-|－|_|\\|)";

static int get_current_time()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec;
}

bool Util::is_noise_tag_name(const std::string &tag_name)
{
    if (noise_tag_name_re == NULL) {
        const char *error;
        int erroffset;
        noise_tag_name_re = pcre_compile(noise_tag_name_pat, 0, &error, &erroffset, NULL);
        if (noise_tag_name_re == NULL) {
            fprintf(stderr, "PCRE compilation failed at offset %d: %s\n", erroffset, error);
            return false;
        }
    }

    assert(noise_tag_name_re != NULL);

    int ovector[OVECCOUNT];
    int rc = pcre_exec(noise_tag_name_re, NULL, tag_name.c_str(), tag_name.size(), 0, 0, ovector, OVECCOUNT);
    if (rc < 0) {
        switch(rc) {
            case PCRE_ERROR_NOMATCH: return false; break;
            default: fprintf(stderr, "Noise tag name %s matching error %d\n", tag_name.c_str(), rc); break;
        }
    } else {
        return true;
    }

    return false;
}

bool Util::is_bbs_signature(const std::string &class_name)
{
    if (bbs_signature_re == NULL) {
        const char *error;
        int erroffset;
        bbs_signature_re = pcre_compile(bbs_signature_pat, PCRE_CASELESS, &error, &erroffset, NULL);
        if (bbs_signature_pat == NULL) {
            fprintf(stderr, "PCRE compilation failed at offset %d: %s\n", erroffset, error);
            return false;
        }
    }

    assert(bbs_signature_re != NULL);

    int ovector[OVECCOUNT];
    int rc = pcre_exec(bbs_signature_re, NULL, class_name.c_str(), class_name.size(), 0, 0, ovector, OVECCOUNT);
    if (rc < 0) {
        switch(rc) {
            case PCRE_ERROR_NOMATCH: return false; break;
            default: fprintf(stderr, "bbs signature name %s matching error %d\n", class_name.c_str(), rc); break;
        }
    } else {
        return true;
    }

    return false;


}

bool Util::is_binary_link(const std::string &href)
{
    if (binary_link_re == NULL) {
        const char *error;
        int erroffset;
        binary_link_re = pcre_compile(binary_link_pat, PCRE_CASELESS, &error, &erroffset, NULL);
        if (binary_link_re == NULL) {
            fprintf(stderr, "PCRE compilation failed at offset %d: %s\n", erroffset, error);
            return false;
        }
    }

    assert(binary_link_re != NULL);

    int ovector[OVECCOUNT];
    int rc = pcre_exec(binary_link_re, NULL, href.c_str(), href.size(), 0, 0, ovector, OVECCOUNT);
    if (rc < 0) {
        switch(rc) {
            case PCRE_ERROR_NOMATCH: return false; break;
            default: fprintf(stderr, "binary link %s matching error %d\n", href.c_str(), rc); break;
        }
    } else {
        return true;
    }

    return false;
}

bool Util::contain_noise_title(const std::string &text)
{
    if (noise_title_re == NULL) {
        const char *error;
        int erroffset;
        noise_title_re = pcre_compile(noise_title_pat, PCRE_CASELESS, &error, &erroffset, NULL);
        if (noise_title_re == NULL) {
            fprintf(stderr, "PCRE compilation failed at offset %d: %s\n", erroffset, error);
            return false;
        }
    }

    assert(noise_title_re != NULL);

    int ovector[OVECCOUNT];
    int rc = pcre_exec(noise_title_re, NULL, text.c_str(), text.size(), 0, 0, ovector, OVECCOUNT);
    if (rc < 0) {
        switch(rc) {
            case PCRE_ERROR_NOMATCH: return false; break;
            default: fprintf(stderr, "Matching error %d\n", rc); break;
        }
    } else {
        return true;
    }

    return false;
}

bool Util::contain_noise_content(const std::string &text)
{
    if (noise_content_re == NULL) {
        const char *error;
        int erroffset;
        // noise_content_re = pcre_compile(noise_content_pat, PCRE_CASELESS | PCRE_UTF8, &error, &erroffset, NULL);
        noise_content_re = pcre_compile(noise_content_pat, PCRE_CASELESS, &error, &erroffset, NULL);
        if (noise_content_re == NULL) {
            fprintf(stderr, "PCRE compilation failed at offset %d: %s\n", erroffset, error);
            return false;
        }
    }

    assert(noise_content_re != NULL);

    int ovector[OVECCOUNT];
    int rc = pcre_exec(noise_content_re, NULL, text.c_str(), text.size(), 0, 0, ovector, OVECCOUNT);
    if (rc < 0) {
        switch(rc) {
            case PCRE_ERROR_NOMATCH: return false; break;
            default: fprintf(stderr, "Matching error %d\n", rc); break;
        }
    } else {
        return true;
    }

    return false;
}

bool Util::contain_good_punct(const std::string &text)
{
    if (punct_re == NULL) {
        const char *error;
        int erroffset;
        punct_re = pcre_compile(punct_pat, 0, &error, &erroffset, NULL);
        if (punct_re == NULL) {
            fprintf(stderr, "PCRE compilation failed at offset %d: %s\n", erroffset, error);
            return false;
        }
    }
    int ovector[OVECCOUNT];
    int rc = pcre_exec(punct_re, NULL, text.c_str(), text.size(), 0, 0, ovector, OVECCOUNT);
    if (rc < 0) {
        switch(rc) {
            case PCRE_ERROR_NOMATCH: return false; break;
            default: fprintf(stderr, "Matching error %d\n", rc); break;
        }
    } else {
        return true;
    }

    return false;
}

bool Util::title_contain_seo(const std::string &page_title)
{
    if (contain_seo_re == NULL) {
        const char *error;
        int erroffset;
        contain_seo_re = pcre_compile(contain_seo_pat, 0, &error, &erroffset, NULL);
        if (contain_seo_re == NULL) {
            fprintf(stderr, "PCRE compilation failed at offset %d: %s\n", erroffset, error);
            return false;
        }
    }
    int ovector[OVECCOUNT];
    int rc = pcre_exec(contain_seo_re, NULL, page_title.c_str(), page_title.size(), 0, 0, ovector, OVECCOUNT);
    if (rc < 0) {
        switch(rc) {
            case PCRE_ERROR_NOMATCH: return false; break;
            default: fprintf(stderr, "Matching error %d\n", rc); break;
        }
    } else {
        return true;
    }

    return false;
}

int Util::match_date(const std::string &text, int download_time, bool is_title_block, bool pre_is_title_block)
{
    const char *error;
    int erroffset;

    if (noise_date_re == NULL) {
        noise_date_re = pcre_compile(noise_date_pat, 0, &error, &erroffset, NULL);
    }

    if (date_re == NULL) {
        date_re = pcre_compile(date_pat, 0, &error, &erroffset, NULL);
    }

    if (date_prefix_re == NULL) {
        date_prefix_re = pcre_compile(date_prefix_pat, 0, &error, &erroffset, NULL);
    }

    if (date_suffix_re == NULL) {
        date_suffix_re = pcre_compile(date_suffix_pat, 0, &error, &erroffset, NULL);
    }

    if (date_yestoday_re == NULL) {
        date_yestoday_re = pcre_compile(date_yestoday_pat, 0, &error, &erroffset, NULL);
    }

    if (date_before_yestoday_re == NULL) {
        date_before_yestoday_re = pcre_compile(date_before_yestoday_pat, 0, &error, &erroffset, NULL);
    }

    assert(noise_date_re != NULL);
    assert(date_re != NULL);
    assert(date_prefix_re != NULL);
    assert(date_suffix_re != NULL);
    assert(date_yestoday_re != NULL);
    assert(date_before_yestoday_re != NULL);

    int ovector[OVECCOUNT];
    int rc;

    rc = pcre_exec(noise_date_re, NULL, text.c_str(), text.size(), 0, 0, ovector, OVECCOUNT);
    if (rc < 0) {
        switch(rc) {
            case PCRE_ERROR_NOMATCH: break;
            default: fprintf(stderr, "matching noise date error %d\n", rc); break;
        }
    } else {
        return 0;
    }

    rc = pcre_exec(date_re, NULL, text.c_str(), text.size(), 0, 0, ovector, OVECCOUNT);
    if (rc < 0) {
        switch(rc) {
            case PCRE_ERROR_NOMATCH: break;
            default: fprintf(stderr, "matching date error %d\n", rc); break;
        }
    } else {
        int prefix_ovector[OVECCOUNT];
        int suffix_ovector[OVECCOUNT];
        bool contain_publish_flag =\
            pcre_exec(date_prefix_re, NULL, text.c_str(), ovector[2*1], 0, 0, prefix_ovector, OVECCOUNT) >= 0
            || pcre_exec(date_suffix_re, NULL, text.c_str() + ovector[2*3+1], text.size() - ovector[2*3+1], 0, 0, suffix_ovector, OVECCOUNT) >= 0;

        if (is_title_block) {
            if (ovector[1] != text.size()
                && (!contain_publish_flag)) {
                return 0;
            }
        } else if (pre_is_title_block) {
            if (ovector[0] != 0
                && (ovector[1] - ovector[0]) * 4 < text.size()
                && (!contain_publish_flag)) {
                return 0;
            }
        } else {
            if (ovector[0] != 0
                && (!contain_publish_flag)) {
                return 0;
            }
        }

        int year, month, day;

        year = month = day = 0;
        strtk::string_to_type_converter(text.c_str() + ovector[2*1], text.c_str() + ovector[2*1+1], year);
        strtk::string_to_type_converter(text.c_str() + ovector[2*2], text.c_str() + ovector[2*2+1], month);
        strtk::string_to_type_converter(text.c_str() + ovector[2*3], text.c_str() + ovector[2*3+1], day);
        if (ovector[3] - ovector[2] == 2 && year < 100) {
            if (year >= 50) {
                year += 1900;
            } else {
                year += 2000;
            }
        }

        if (year > 2000 && year < 3000 && month >= 1 && month <= 12 && day <= 31 && day >= 1) {
            time_t rawtime;
            struct tm * timeinfo;
            time ( &rawtime );
            timeinfo = localtime(&rawtime );
            timeinfo->tm_year = year - 1900;
            timeinfo->tm_mon = month - 1;
            timeinfo->tm_mday = day;
            timeinfo->tm_hour = 0;
            timeinfo->tm_min = 0;
            timeinfo->tm_sec = 0;

            int ptime = mktime(timeinfo);

            if (ptime > 946711611 && ptime <= rawtime + 6400) {
                return ptime;
            }
        }
    }

    rc = pcre_exec(date_yestoday_re, NULL, text.c_str(), text.size(), 0, 0, ovector, OVECCOUNT);
    if (rc < 0) {
        switch(rc) {
            case PCRE_ERROR_NOMATCH: break;
            default: assert(false); break;
        }
    } else {
        if (download_time) {
            return download_time - 86400;
        } else {
            return 0;
        }
    }

    rc = pcre_exec(date_before_yestoday_re, NULL, text.c_str(), text.size(), 0, 0, ovector, OVECCOUNT);
    if (rc < 0) {
        switch(rc) {
            case PCRE_ERROR_NOMATCH: break;
            default: assert(false); break;
        }
    } else {
        if (download_time) {
            return download_time - 2 * 86400;
        } else {
            return 0;
        }
    }

    return 0;
}

int
Util::levenshtein(const char *s1, int l1, const char *s2, int l2)
{
    int i, j;
    int len = (l1 + 1) * (l2 + 1);
    char *p1, *p2;
    int d1, d2, d3, *d, *dp, res;

    if (l1 == 0) {
        return l2;
    } else if (l2 == 0) {
        return l1;
    }

    d = (int*)malloc(len * sizeof(int));

    *d = 0;
    for(i = 1, dp = d + l2 + 1;
            i < l1 + 1;
            ++i, dp += l2 + 1) {
        *dp = i;
    }
    for(j = 1, dp = d + 1;
            j < l2 + 1;
            ++j, ++dp) {
        *dp = j;
    }

    for(i = 1, p1 = (char*) s1, dp = d + l2 + 2;
            i < l1 + 1;
            ++i, ++p1, ++dp) {
        for(j = 1, p2 = (char*) s2;
                j < l2 + 1;
                ++j, ++p2, ++dp) {
            if((*p1) == (*p2)) {
                *dp = *(dp - l2 - 2);
            } else {
                d1 = *(dp - 1) + 1;
                d2 = *(dp - l2 - 1) + 1;
                d3 = *(dp - l2 - 2) + 1;
                *dp = QMIN3(d1, d2, d3);
            }
        }
    }
    res = *(dp - 2);

    dp = NULL;
    free(d);
    return res;
}

unsigned int
Util::edit_distance(const std::string& s1, const std::string& s2)
{
    const size_t len1 = s1.size(), len2 = s2.size();
    std::vector<std::vector<unsigned int> > d(len1 + 1, std::vector<unsigned int>(len2 + 1));

    d[0][0] = 0;
    for(unsigned int i = 1; i <= len1; ++i) d[i][0] = i;
    for(unsigned int i = 1; i <= len2; ++i) d[0][i] = i;

    for(unsigned int i = 1; i <= len1; ++i)
        for(unsigned int j = 1; j <= len2; ++j)

            d[i][j] = std::min( std::min(d[i - 1][j] + 1,d[i][j - 1] + 1),
                    d[i - 1][j - 1] + (s1[i - 1] == s2[j - 1] ? 0 : 1) );
    return d[len1][len2];
}

bool Util::is_good_image_link(const std::string &src)
{
    if (good_image_link_re == NULL) {
        const char *error;
        int erroffset;
        good_image_link_re = pcre_compile(good_image_link_pat, PCRE_CASELESS, &error, &erroffset, NULL);
        if (good_image_link_re == NULL) {
            fprintf(stderr, "PCRE compilation failed at offset %d: %s\n", erroffset, error);
            return false;
        }
    }

    assert(good_image_link_re != NULL);

    int ovector[OVECCOUNT];
    int rc = pcre_exec(good_image_link_re, NULL, src.c_str(), src.size(), 0, 0, ovector, OVECCOUNT);
    if (rc < 0) {
        switch(rc) {
            case PCRE_ERROR_NOMATCH: return false; break;
            default: fprintf(stderr, "binary link %s matching error %d\n", src.c_str(), rc); break;
        }
    } else {
        return true;
    }

    return false;
}

bool Util::is_noise_image_link(const std::string &src)
{
    if (noise_image_link_re == NULL) {
        const char *error;
        int erroffset;
        noise_image_link_re = pcre_compile(noise_image_link_pat, 0, &error, &erroffset, NULL);
        if (noise_image_link_re == NULL) {
            fprintf(stderr, "PCRE compilation failed at offset %d: %s\n", erroffset, error);
            return false;
        }
    }

    assert(noise_image_link_re != NULL);

    int ovector[OVECCOUNT];
    int rc = pcre_exec(noise_image_link_re, NULL, src.c_str(), src.size(), 0, 0, ovector, OVECCOUNT);
    if (rc < 0) {
        switch(rc) {
            case PCRE_ERROR_NOMATCH: return false; break;
            default: fprintf(stderr, "noise link %s matching error %d\n", src.c_str(), rc); break;
        }
    } else {
        return true;
    }

    return false;
}


bool Util::is_bbs_link(const std::string &src)
{
    if (bbs_link_re == NULL) {
        const char *error;
        int erroffset;
        bbs_link_re = pcre_compile(bbs_link_pat, 0, &error, &erroffset, NULL);
        if (bbs_link_re == NULL) {
            fprintf(stderr, "PCRE compilation failed at offset %d: %s\n", erroffset, error);
            return false;
        }
    }

    assert(bbs_link_re != NULL);

    int ovector[OVECCOUNT];
    int rc = pcre_exec(bbs_link_re, NULL, src.c_str(), src.size(), 0, 0, ovector, OVECCOUNT);
    if (rc < 0) {
        switch(rc) {
            case PCRE_ERROR_NOMATCH: return false; break;
            default: fprintf(stderr, "binary link %s matching error %d\n", src.c_str(), rc); break;
        }
    } else {
        return true;
    }

    return false;
}

static void filter_page_title(const std::string &page_title, std::string &filtered_title)
{
    if (filter_page_title_re == NULL) {
        const char *error;
        int erroffset;
        filter_page_title_re = pcre_compile(filter_page_title_pat, 0, &error, &erroffset, NULL);
        if (filter_page_title_re == NULL) {
            LOG(FATAL) << "PCRE compilation failed at offset: " << erroffset << " error :" << error;
        }
    }

    int ovector[OVECCOUNT];
    int rc = pcre_exec(filter_page_title_re, NULL, page_title.c_str(), page_title.size(), 0, 0, ovector, OVECCOUNT);
    if (rc < 0) {
        filtered_title = page_title;
    } else {
        filtered_title.append(page_title.c_str(), ovector[2*1]);
    }
}

std::string Util::filter_number(const std::string &src)
{
    std::string ret;
    ret.reserve(src.size());
    const char *p = src.c_str();
    while (*p++) {
        if (!isdigit(*p)) {
            ret.append(p, 1);
        }
    }
    return ret;
}

void Util::normalize_text_to_line(const std::string &raw, std::string &normalized)
{
    bool start_content = false;
    bool has_space = false;

    const char *p = raw.c_str();
    int len = raw.size();
    normalized.reserve(len);
    for (int i = 0; i < len; i++) {
        unsigned char c = *(p+i);
        if (isspace(c)) {
            has_space = true;
        } else if (c == (unsigned char)'\xc2' && i < len - 1 && (*(p+i+1)) == '\xa0') {
            has_space = true;
            ++i;
        } else if (c == (unsigned char)'\xe3' && i < len - 2 && (*(p+i+1)) == '\x80' && (*(p+i+2)) == '\x80') {
            has_space = true;
            i += 2;
        } else if ((int) c <= 20) {
            // skip
            //has_space = true;
        } else {
            if (start_content && has_space) {
                normalized.append(" ");
            }

            if (!start_content) {
                start_content = true;
            }

            has_space = false;
            normalized.append(1, c);
        }
    }
}

void Util::normalize_content(const std::string &content, std::string &filtered_content)
{

    const char *p = content.c_str();
    bool start_content = false;
    bool has_section = false;
    bool has_paragraph = false;
    bool has_line = false;
    bool has_space = false;

    int len = content.length();
    filtered_content.reserve(len + 50);
    for (int i = 0; i < len; ++i) {
        unsigned char c = *(p + i);
        if (c == '\x03') {
            has_section = true;
        } else if (c == '\x02') {
            has_paragraph = true;
        } else if (c == '\n') {
            has_line = true;
        } else if (isspace(c)) {
            has_space = true;
        } else if (c == (unsigned char)'\xc2' && i < len - 1 && (*(p+i+1)) == '\xa0') {
            has_space = true;
            ++i;
        } else if (c == (unsigned char)'\xe3' && i < len - 2 && (*(p+i+1)) == '\x80' && (*(p+i+2)) == '\x80') {
            has_space = true;
            i += 2;
        } else if ((int)c <= 20) {
            // skip
            //has_space = true;
        } else {
            if (start_content) {
                if (has_section) {
                    filtered_content.append("\x01\x01\x01");
                } else if (has_paragraph) {
                    filtered_content.append("\x01\x01");
                } else if (has_line) {
                    filtered_content.append("\x01");
                } else if (has_space) {
                    filtered_content.append(" ");
                }
            }

            if (!start_content) {
                start_content = true;
            }

            filtered_content.append(1, (unsigned char)c);
            has_section = has_paragraph = has_line = has_space = false;
        }
    }
}

} // namespace content
} // namespace vdom
