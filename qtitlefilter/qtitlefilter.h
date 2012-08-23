/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 *
 * qtitlefilter.h
 * fitler raw title's common prefix or suffix
 */

#ifndef QTITLE_FITER_H
#define QTITLE_FITER_H

#include <stdint.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <iostream>

#include <tr1/functional>
#include <tr1/unordered_map>
#include <tr1/unordered_set>

namespace qcontent
{

#ifndef QCONTENT_TR1_UNORDERED
#define QCONTENT_TR1_UNORDERED

using std::tr1::unordered_map;
using std::tr1::unordered_set;
using std::tr1::unordered_multimap;
using std::tr1::unordered_multiset;

template <typename T> struct hash : public std::tr1::hash<T> {};

#endif

class QTitleFilter {
public:
    enum FilterMode {
        STAT,
        FILTER,
        GEN_ATTR
    };

    typedef unordered_map<std::string, uint32_t>  SliceMap;
    typedef unordered_map<std::string, uint32_t>::iterator SliceMapIter;

    struct FilterHost {
        FilterHost(uint32_t c = 0):url_count(c) {};
        uint32_t url_count;
        SliceMap slice_map;
    };

    struct FilterTitleSlice {
        FilterTitleSlice(): start_pos(0), end_pos(0),
                    sc(0),
                    min_sc_suffix(0), min_sc_prefix(0) {
        }

        int start_pos;
        int end_pos;
        int sc;
        int min_sc_suffix;
        int min_sc_prefix;
    };

    typedef unordered_map<std::string, SliceMap>  HostSliceMap;
    typedef unordered_map<std::string, SliceMap>::iterator  HostSliceMapIter;

    typedef unordered_map<std::string, FilterHost>  FilterHostMap;
    typedef unordered_map<std::string, FilterHost>::iterator  FilterHostMapIter;

    QTitleFilter();
    QTitleFilter(FilterMode m, const std::string &file);
    ~QTitleFilter();

    bool init();
    void stat(const std::string &host, const std::string &url, const std::string &raw_title);

    void print_stat_result();

    bool read_stat_result();
    bool save_stat_result();

    bool filter(const std::string &host, const std::string &url,
            const std::string &raw_title, std::string &filtered);
    bool gen_title_attribute(const std::string &host, const std::string &url,
            const std::string &raw_title, std::string &title_attr);

    bool is_separator(unsigned char c) {
        if (c == '|' || c == '-' || c == '_') {
            return true;
        }

        return false;
    }

    void count_slice(FilterHost &filter_host, const char *p, size_t len)
    {
        std::string slice(p, len);

        SliceMap &slice_map = filter_host.slice_map;
        if (slice_map.find(slice) == slice_map.end()) {
            slice_map.insert(std::pair<std::string, uint32_t> (slice, 1));
        } else {
            slice_map[slice]++;
        }
    }

    uint32_t get_slice_count(const std::string &host, const char *p, size_t len)
    {
        FilterHostMapIter host_it = filter_hosts.find(host);
        if (host_it == filter_hosts.end()) {
            return 0;
        }

        SliceMap &slice_map =  host_it->second.slice_map;

        std::string slice(p, len);
        if (slice_map.find(slice) == slice_map.end()) {
            return 0;
        } else {
            return slice_map[slice];
        }
    }

    uint32_t get_slice_count(SliceMap &slice_map, const std::string &slice)
    {
        if (slice_map.find(slice) == slice_map.end()) {
            return 0;
        } else {
            return slice_map[slice];
        }
    }



    void read_slice(const std::string &host, uint32_t url_count, const std::string &slice, uint32_t cnt)
    {
        FilterHostMapIter host_it = filter_hosts.find(host);
        if (host_it == filter_hosts.end()) {
            filter_hosts.insert(std::pair<std::string, FilterHost> (host, FilterHost(url_count)));
            host_it = filter_hosts.find(host);
        }

        SliceMap &slice_map =  host_it->second.slice_map;
        slice_map.insert(std::pair<std::string, uint32_t> (slice, cnt));
    }

private:
    FilterMode mode;
    size_t slice_size_min_limit;
    size_t slice_count_min_limit;
    size_t host_stat_max_limit;

    uint32_t uniq_map_size;
    uint32_t *url_uniq_map;

    int filtered_slice_count_min_limit;

    std::string stat_file;

    FilterHostMap filter_hosts;
    std::string new_stat_data_file;
};

} //end namespace qcontent

#endif
