/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 *
 * qpredic_dict.h
 *
 */
#ifndef QPREDICT_DICT_H
#define QPREDICT_DICT_H

#include <cstdlib>
#include <string>
#include "qpredict_define.h"
#include <strtk.hpp>

namespace qcontent
{

struct QPredictDictWord {
    QPredictDictWord() {
    }
    QPredictDictWord(int i, double f) : id(i), idf(f) {
    }

    int id;
    double idf;
};

typedef unordered_map<std::string, QPredictDictWord> QPredictDictWordMap;
typedef unordered_map<std::string, QPredictDictWord>::iterator QPredictDictWordMapIter;

class QPredictDict {
public:
    QPredictDict();
    QPredictDict(const std::string &path);
    virtual ~QPredictDict();

    inline void set_path(const std::string &path) {
        m_path = path;
    }

    inline const std::string & path() {
        return m_path;
    }

    inline const std::string & path() const {
        return m_path;
    }

    bool load();
    bool save();

    void insert(const std::string &w, int id, double value);
    bool query(const std::string &w, int *id, double *value);

protected:
    bool parse_line(const std::string &line, int *id, std::string *w, double *val) {
        if (line.size() > 0 && strtk::parse(line, "\t", *id, *w, *val)) {
            return true;
        } else {
            return false;
        }
    }

    std::string m_path;
    QPredictDictWordMap words;
};

} // end for namespace qcontent

#endif
