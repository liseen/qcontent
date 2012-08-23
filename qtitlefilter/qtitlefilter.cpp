/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

#include "qtitlefilter.h"

#include <cstring>
#include <iostream>
#include <fstream>
#include <limits>
#include <vector>

#include <strtk.hpp>

#include <city.h>

namespace qcontent
{

QTitleFilter::QTitleFilter()
{
    mode = STAT;
    host_stat_max_limit = 100000;

    slice_size_min_limit = 9;
    slice_count_min_limit = 50;

    uniq_map_size = 8 * 8 * 1024 * 1024;

    url_uniq_map = NULL;

    filtered_slice_count_min_limit = 50;

    stat_file = "host_slice_freq.txt";
}

QTitleFilter::QTitleFilter(FilterMode m, const std::string &file)
{
    mode = m;

    host_stat_max_limit = 100000;

    slice_size_min_limit = 9;
    slice_count_min_limit = 50;

    uniq_map_size = 8 * 8 * 1024 * 1024;

    url_uniq_map = NULL;

    filtered_slice_count_min_limit = 50;

    stat_file = file;
}

QTitleFilter::~QTitleFilter()
{
    if (url_uniq_map) {
        delete [] url_uniq_map;
    }
}

bool QTitleFilter::init()
{
    if (mode == STAT) {
        url_uniq_map = new uint32_t[uniq_map_size / 4];
        if (url_uniq_map == NULL) {
            fprintf(stderr, "malloc url uniq map error\n");
            return false;
        }

        memset(url_uniq_map, 0, uniq_map_size);
    } else {
        if (!read_stat_result()) {
            return false;
        }
    }

    return true;
}

void QTitleFilter::stat(const std::string &host, const std::string &url, const std::string &raw_title)
{
    static uint8_t bit_powers[] = {
        0x01,
        0x02,
        0x04,
        0x08,
        0x10,
        0x20,
        0x40,
        0x80
    };

    uint64_t hash_val = CityHash64(url.c_str(), url.length());
    hash_val = hash_val % uniq_map_size;
    uint64_t byte_pos = hash_val >> 3;
    uint8_t bit_pos = bit_powers[hash_val & 0x0000000000000007];

    if (url_uniq_map[byte_pos] & bit_pos) {
        return;
    } else {
        url_uniq_map[byte_pos] |= bit_pos;
    }


    FilterHostMapIter host_it = filter_hosts.find(host);
    if (host_it == filter_hosts.end()) {
        filter_hosts.insert(std::pair<std::string, FilterHost> (host, FilterHost()));
        host_it = filter_hosts.find(host);
    }

    FilterHost &filter_host = host_it->second;

    filter_host.url_count++;


    const char *p = raw_title.c_str();

    size_t len = raw_title.size();
    size_t last_pos = 0;
    for (size_t i = 0; i < len; ++i) {
        if (is_separator(raw_title[i])) {
            //if (i - last_pos >= slice_size_min_limit) {
                count_slice(filter_host, p + last_pos, i - last_pos);
            //}
            last_pos = i + 1;
        }

    }

    // all title is a slice
    if (last_pos < len) {
        count_slice(filter_host, p + last_pos, len - last_pos);
    }

}

void QTitleFilter::print_stat_result()
{
    for (FilterHostMapIter host_it = filter_hosts.begin(); host_it != filter_hosts.end(); ++host_it) {
        const std::string &host = host_it->first;

        uint32_t url_count = host_it->second.url_count;
        SliceMap &slice_map = host_it->second.slice_map;

        for (SliceMapIter it = slice_map.begin(); it != slice_map.end(); ++it) {
            if (it->second >= slice_count_min_limit) {
                std::cout << host << "\t" << url_count << "\t"
                    << it->first << "\t" << it->second << "\n";
            }
        }
    }
}

bool QTitleFilter::save_stat_result()
{
    std::ofstream fout;
    fout.open(stat_file.c_str());
    if (fout.good()) {
        for (FilterHostMapIter host_it = filter_hosts.begin(); host_it != filter_hosts.end(); ++host_it) {
            const std::string &host = host_it->first;

            uint32_t url_count = host_it->second.url_count;
            SliceMap &slice_map = host_it->second.slice_map;
            for (SliceMapIter it = slice_map.begin(); it != slice_map.end(); ++it) {
                if (it->second >= slice_count_min_limit) {
                    fout << host << "\t" << url_count << "\t" \
                        << it->first << "\t" << it->second << "\n";
                }
            }
        }

        fout.close();
        return  true;
    } else {
        return false;
    }
}

bool QTitleFilter::read_stat_result()
{
    std::ifstream fin;
    fin.open(stat_file.c_str());
    if (fin.good()) {
        std::string line;
        while (std::getline(fin, line)) {
            std::string host;
            uint32_t url_count;
            std::string slice;
            int cnt = 0;
            if (strtk::parse(line, "\t", host, url_count, slice, cnt)) {
                read_slice(host, url_count, slice, cnt);
            }
        }

        fin.close();
        return true;
    } else {
        return false;
    }
}

/*
bool QTitleFilter::filter(const std::string &host, const std::string &url,
        const std::string &raw_title, std::string &filtered_title)
{
    if (raw_title.size() < slice_size_min_limit) {
        filtered_title = raw_title;
    }

    const char *p = raw_title.c_str();
        // split to slices
    int len = raw_title.size();
    int max_prefix_end = -1;
    int min_suffix_end = len;
    for (int i = 1; i < len - 1; ++i) {
        if (is_seperator(raw_title[i])) {
            // slice 0 .. i
            if (i + 1 >= slice_size_min_limit
                    && get_slice_count(host, p, i+1) > filtered_slice_count_min_limit) {
                max_prefix_end = i;
            }

            // slice i .. len
            if (len - i >= slice_size_min_limit
                    && i < min_suffix_end
                    && get_slice_count(host, p+i, len - i) > filtered_slice_count_min_limit) {
                min_suffix_end = i;
            }
        }
    }

    // slice 0 .. len
    if (len >= slice_size_min_limit) {
        if (get_slice_count(host, p, len) > filtered_slice_count_min_limit) {
            max_prefix_end = len;
            min_suffix_end = 0;
        }
    }

    if (min_suffix_end > max_prefix_end) {
        filtered_title = raw_title.substr(max_prefix_end + 1, min_suffix_end - max_prefix_end - 1);
    } else {
        filtered_title = "";
    }

    return true;
}

*/

bool QTitleFilter::filter(const std::string &host, const std::string &url,
        const std::string &raw_title, std::string &filtered_title)
{
    if (raw_title.size() < slice_size_min_limit) {
        filtered_title = raw_title;
    }

    FilterHostMapIter host_it = filter_hosts.find(host);
    if (host_it == filter_hosts.end()) {
        filtered_title = raw_title;
    } else {
        const char *p = raw_title.c_str();
        uint32_t hc = host_it->second.url_count;
        SliceMap &slice_map = host_it->second.slice_map;
        size_t len = raw_title.size();
        size_t last_pos = 0;

        std::vector<FilterTitleSlice> slice_vec;
        for (size_t i = 0; i < len; ++i) {
            if (is_separator(raw_title[i])) {
                if (i - last_pos >= (int)slice_size_min_limit) {
                    FilterTitleSlice title_slice;

                    title_slice.start_pos = last_pos;
                    title_slice.end_pos = i;
                    std::string slice(p + last_pos, i - last_pos);
                    title_slice.sc = get_slice_count(slice_map, slice);
                    slice_vec.push_back(title_slice);
                } else {
                    FilterTitleSlice title_slice;

                    title_slice.start_pos = last_pos;
                    title_slice.end_pos = i;

                    std::string slice(p + last_pos, i - last_pos);
                    title_slice.sc = get_slice_count(slice_map, slice);
                    // title_slice.sc = 0;
                    slice_vec.push_back(title_slice);
                }
                last_pos = i + 1;
            }
        }

        // std::cout << "last_pos: " << last_pos << std::endl;
        // all title is a slice
        if (last_pos < len) {
            FilterTitleSlice title_slice;
            title_slice.start_pos = last_pos;
            title_slice.end_pos = len - 1;

            std::string slice(p + last_pos, len - last_pos);
            title_slice.sc = get_slice_count(slice_map, slice);
            slice_vec.push_back(title_slice);
        }


        // TODO
        // compute min_sc_prefix
        int min_sc_prefix = std::numeric_limits<int>::max();
        int vec_size = slice_vec.size();
        for (int i = 0; i < vec_size; ++i) {
            if (slice_vec[i].sc < min_sc_prefix) {
                min_sc_prefix = slice_vec[i].sc;
            }
            slice_vec[i].min_sc_prefix = min_sc_prefix;
        }

        // compute min_sc_suffix
        int min_sc_suffix = std::numeric_limits<int>::max();
        for (int i = vec_size - 1; i >= 0; --i) {
            if (slice_vec[i].sc < min_sc_suffix) {
                min_sc_suffix = slice_vec[i].sc;
            }
            slice_vec[i].min_sc_suffix = min_sc_suffix;
        }

        /*
        for (int i = 0; i < vec_size; ++i) {
            std::cout << "===================\n";
            std::cout << slice_vec[i].start_pos << "\n";
            std::cout << slice_vec[i].end_pos << "\n";
            std::cout << slice_vec[i].sc << "\n";
            std::cout << slice_vec[i].min_sc_suffix << "\n";
            std::cout << slice_vec[i].min_sc_prefix << "\n";
        }
        */

        int max_prefix_end = -1;
        int min_suffix_end = len;
        for (int i = 0; i < vec_size; ++i) {
            if (slice_vec[i].min_sc_prefix >= filtered_slice_count_min_limit) {
                max_prefix_end = slice_vec[i].end_pos;
            } else {
                break;
            }
        }

        for (int i = vec_size - 1; i >= 0; --i) {
            if (slice_vec[i].min_sc_suffix >= filtered_slice_count_min_limit) {
                min_suffix_end = slice_vec[i].start_pos - 1;
            } else {
                break;
            }
        }

        if (min_suffix_end > max_prefix_end) {
            filtered_title = raw_title.substr(max_prefix_end + 1, min_suffix_end - max_prefix_end - 1);
        } else {
            filtered_title = "";
        }
    }

    return true;
}

bool QTitleFilter::gen_title_attribute(const std::string &host, const std::string &url,
        const std::string &raw_title, std::string &title_attr)
{
    title_attr.append("[");

    char buf[128];
    FilterHostMapIter host_it = filter_hosts.find(host);
    if (host_it == filter_hosts.end()) {
        sprintf(buf, "{b:%d,e:%d,hc:%d,sc:%d}", 0,  (int)raw_title.size(), 0, 0);
        title_attr.append(buf);
    } else {
        const char *p = raw_title.c_str();
        uint32_t hc = host_it->second.url_count;

        SliceMap &slice_map = host_it->second.slice_map;
        int len = raw_title.size();
        int last_pos = 0;
        for (int i = 0; i < len; ++i) {
            if (is_separator(raw_title[i])) {
                if (i - last_pos >= (int)slice_size_min_limit) {
                    std::string slice(p + last_pos, i - last_pos);
                    uint32_t sc = get_slice_count(slice_map, slice);
                    sprintf(buf, "{b:%d,e:%d,hc:%d,sc:%d},", last_pos,  i, hc, sc);
                    title_attr.append(buf);
                } else {
                    std::string slice(p + last_pos, i - last_pos);
                    uint32_t sc = get_slice_count(slice_map, slice);
                    sprintf(buf, "{b:%d,e:%d,hc:%d,sc:%d},", last_pos,  i, hc, sc);
                    title_attr.append(buf);
                }
                last_pos = i + 1;

            }

        }
        // std::cout << "last_pos: " << last_pos << std::endl;
        // all title is a slice
        if (last_pos < len) {
            std::string slice(p + last_pos, len - last_pos);
            uint32_t sc = get_slice_count(slice_map, slice);
            sprintf(buf, "{b:%d,e:%d,hc:%d,sc:%d}", last_pos,  len, hc, sc);
            title_attr.append(buf);
        }
    }

    title_attr.append("]");

    return true;
}

} //end namespace qcontent
