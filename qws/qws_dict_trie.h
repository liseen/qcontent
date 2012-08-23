/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

#ifndef QCONTENT_QWS_DICT_H
#define QCONTENT_QWS_DICT_H

#include <trie.h>
#include "qws_common.h"

BEGIN_NAMESPACE_QWS

class QWSDict {
public:
    QWSDict();
    QWSDict(const std::string &path);
    virtual ~QWSDict();

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
    void insert(const std::string &word, int value);
    bool query(const std::string &word, int *value);
    bool query(const char* word, int size, int *value);

protected:
    std::string m_path;
    dutil::trie *m_trie;
};

END_NAMESPACE_QWS

#endif
