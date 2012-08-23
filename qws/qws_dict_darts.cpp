/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

#include "qws_dict_darts.h"

#include <cstdio>
#include <algorithm>

BEGIN_NAMESPACE_QWS

QWSDict::QWSDict()
{
    m_darts = new Darts::DoubleArray();
}

QWSDict::QWSDict(const std::string &path)
{
    m_path = path;
    m_darts = new Darts::DoubleArray();
    if (!load()) {
        fprintf(stderr, "load darts dict error\n");
        exit(-1);
    }
}


QWSDict::~QWSDict()
{
    if (m_darts) {
        delete m_darts;
    }
}


bool QWSDict::load()
{
    if (m_darts->open(m_path.c_str()) == 0) {
        return true;
    } else {
        return false;
    }
}

bool QWSDict::save()
{
    return m_darts->save(m_path.c_str());
}

void QWSDict::insert(const std::string &/*word*/, int /*value*/)
{
    // don't support insert
    assert(false);
}

void QWSDict::build(const QWSDartsKeyList &list)
{
    QWSDartsKeyList key_list(list);
    std::stable_sort(key_list.begin(), key_list.end(), QWSDartsKeyComparator());

    int key_size = key_list.size();
    QDD("build size : %d\n", key_size);
    const char **keys = new const char *[key_size];
    std::size_t *lengths = new std::size_t[key_size];
    int *values = new int[key_size];
    for (int i = 0; i < key_size; i++) {
        keys[i] = key_list[i].word.c_str();
        lengths[i] = key_list[i].word.length();
        values[i] = key_list[i].value;
    }

    m_darts->build(key_size, keys, lengths, values);
}

bool QWSDict::query(const char* word, int size, int *value)
{
    Darts::DoubleArray::result_pair_type result;
    m_darts->exactMatchSearch(word, result, size);
    if (result.value >= 0) {
        *value = result.value;
        return true;
    } else {
        return false;
    }
}

bool QWSDict::query(const std::string &word, int *value)
{
    Darts::DoubleArray::result_pair_type result;
    m_darts->exactMatchSearch(word.c_str(), result, word.size());
    if (result.value >= 0) {
        *value = result.value;
        return true;
    } else {
        return false;
    }
}
/*
int QWSDict::traverse(const char *key, std::size_t &node_pos, std::size_t &key_pos, std::size_t length) const
{
    return m_darts->traverse(key, node_pos, key_pos, length);
}
*/

END_NAMESPACE_QWS
