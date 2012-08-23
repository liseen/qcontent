/*
 *
 *
 */

#ifndef QCONTENT_QWS_DICT_H
#define QCONTENT_QWS_DICT_H

#include "qws_common.h"
#include <string>
#include <tr1/functional>
#include <tr1/unordered_map>
#include <tr1/unordered_set>

BEGIN_NAMESPACE_QWS
using std::tr1::unordered_map;
using std::tr1::unordered_set;
using std::tr1::unordered_multimap;
using std::tr1::unordered_multiset;
template <typename T> struct hash : public std::tr1::hash<T> { };
END_NAMESPACE_QWS

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
    unordered_map<std::string, int> m_map;
};

END_NAMESPACE_QWS

#endif
