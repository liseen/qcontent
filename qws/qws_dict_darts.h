/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

#ifndef QCONTENT_QWS_DICT_H
#define QCONTENT_QWS_DICT_H

#include <string>
#include <vector>

#include "qws_common.h"
#include "darts.h"

BEGIN_NAMESPACE_QWS

struct QWSDartsKey {
    std::string word;
    int value;
};

struct QWSDartsKeyComparator {
    bool operator() (const QWSDartsKey &l, const QWSDartsKey &r) {
        return l.word < r.word;
    }
};

typedef std::vector<QWSDartsKey> QWSDartsKeyList;
typedef std::vector<QWSDartsKey>::iterator QWSDartsKeyListIter;

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
    void build(const QWSDartsKeyList &list);
    bool query(const std::string &word, int *value);
    bool query(const char* word, int size, int *value);

    inline int traverse(const char *key, std::size_t &node_pos, std::size_t &key_pos, std::size_t length) const {
        return m_darts->traverse(key, node_pos, key_pos, length);
    }

protected:
    std::string m_path;
    Darts::DoubleArray *m_darts;
};

END_NAMESPACE_QWS

#endif
