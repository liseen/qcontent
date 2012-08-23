/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

#ifndef QCONTENT_CONFIG_H
#define QCONTENT_CONFIG_H

#include <string>
#include <map>
#include <vector>
#include <strtk.hpp>
#include <fstream>


namespace qcontent {

class QContentConfig;
class QContentConfig
{
public:
    typedef std::pair<std::string, std::string> StrPair;
    typedef std::map<std::string, std::string> StrMap;
    typedef std::map<std::string, std::string>::iterator StrMapIter;

    static QContentConfig* get_instance();
    bool parse_file(const std::string &filename);

    const std::vector<std::string> & get_groups();
    std::string get_string(const std::string &key, const std::string &_default);
    int get_integer(const std::string &key, int _default);

    StrMap &values_map() {
        return values;
    }

    void prefix_values_map(const std::string &prefix, StrMap &prefix_values) {
        for (StrMapIter it = values.begin(); it != values.end(); ++it) {
            size_t pos = it->first.find(prefix + ".");
            if (pos == 0) {
                prefix_values.insert(StrPair(it->first.substr(prefix.size() + 1), it->second));
            }
        }
    }

    QContentConfig() {}
    ~QContentConfig() {}
private:
    QContentConfig(const QContentConfig &);
    QContentConfig & operator=(const QContentConfig &);

    static QContentConfig *_instance;
    StrMap values;
    std::vector<std::string> groups;
};


} // end namespace qcontent

#endif
