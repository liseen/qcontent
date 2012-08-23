/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

#include "qws_dict_trie.h"

#include <cstdio>

BEGIN_NAMESPACE_QWS

QWSDict::QWSDict()
{
    m_trie = dutil::trie::create_trie(dutil::trie::SINGLE_TRIE, 1000000);
    //m_trie = dutil::trie::create_trie(dutil::trie::DOUBLE_TRIE);
}

QWSDict::QWSDict(const std::string &path)
{
    m_path = path;
    m_trie = NULL;
    load();
}


QWSDict::~QWSDict()
{
    if (m_trie) {
        delete m_trie;
    }
}


bool QWSDict::load()
{
    try {
        if (m_trie) {
            delete m_trie;
            m_trie = NULL;
        }

        m_trie = dutil::trie::create_trie(m_path.c_str());
    } catch (std::exception &e) {
        fprintf(stderr, "load dict error: %s\n", e.what());
        return false;
    }

    return true;
}

bool QWSDict::save()
{
    try {
        m_trie->build(m_path.c_str(), true);
    } catch (std::exception &e) {
        fprintf(stderr, "build dict error: %s\n", e.what());
        return false;
    }

    return true;
}

void QWSDict::insert(const std::string &word, int value)
{
    m_trie->insert(word.c_str(), word.size(), value);
}

bool QWSDict::query(const char* word, int size, int *value)
{
    return m_trie->search(word, size, value);
}

bool QWSDict::query(const std::string &word, int *value)
{
    return m_trie->search(word.c_str(), word.size(), value);
}

END_NAMESPACE_QWS
