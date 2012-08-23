/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

#include "qcontent_config.h"

namespace qcontent {

QContentConfig * QContentConfig::_instance = NULL;

QContentConfig* QContentConfig::get_instance()
{
    if (_instance == NULL) {
        _instance = new QContentConfig();
    }
    return _instance;
}

bool QContentConfig::parse_file(const std::string &filename) {

    std::ifstream fin(filename.c_str());

    if (!fin) {
        return false;
    }

    std::string region = "global";
    std::string key;
    std::string value;
    std::string line;
    while (getline(fin, line)) {
        strtk::remove_leading(" \t", line);
        strtk::remove_trailing(" \t", line);
        if (line.length() == 0) {
            continue;
        } else if (line.at(0) == '#' || line.at(0) == ';') {
            continue;
        }

        size_t comment = line.find_first_of("#;");
        if (comment != std::string::npos) {
            line = line.substr(0, comment);
        }

        if (line.at(0) == '[') {
            strtk::remove_inplace(' ', line);
            strtk::remove_inplace('[', line);
            strtk::remove_inplace(']', line);
            region = line;

            groups.push_back(region);
            continue;
        } else {
            key.clear();
            value.clear();

            size_t pos = line.find_first_of("=");
            if (pos == std::string::npos) {
                key = line;
            } else {
                key = line.substr(0, pos);
                value = line.substr(pos + 1);
            }

            strtk::remove_leading(" \t", key);
            strtk::remove_trailing(" \t", key);
            strtk::remove_leading(" \t", value);
            strtk::remove_trailing(" \t", value);

            key = region + "." + key;
            values[key] = value;
        }
    }

    fin.close();

    return true;
}

const std::vector<std::string> & QContentConfig::get_groups()
{
    return groups;
}

std::string QContentConfig::get_string(const std::string &key, const std::string &_default) {
    StrMapIter it = values.find(key);
    if (it == values.end()) {
        return _default;
    } else {
        return it->second;
    }
}

int QContentConfig::get_integer(const std::string &key, int _default)
{
    StrMapIter it = values.find(key);
    if (it == values.end()) {
        return _default;
    } else {
        int n = 0;
        strtk::string_to_type_converter(it->second, n);
        return n;
    }
}

} // end namesapce qcontent

