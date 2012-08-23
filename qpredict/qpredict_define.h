/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

/*
 *
 *
 */

#ifndef QPREDCIT_DEFINE_H
#define QPREDCIT_DEFINE_H

#define QPREDICT_MAX_CLASSES 10

#include <cassert>

#include <string>

#include "qpredict_log.h"

#ifndef QCONTENT_TR1_UNORDERED
#define QCONTENT_TR1_UNORDERED

#include <tr1/functional>
#include <tr1/unordered_map>
#include <tr1/unordered_set>
namespace qcontent
{
    using std::tr1::unordered_map;
    using std::tr1::unordered_set;
    using std::tr1::unordered_multimap;
    using std::tr1::unordered_multiset;
    template <typename T> struct hash : public std::tr1::hash<T> { };
}

#endif

namespace qcontent
{

struct str_hash {
    size_t operator()(const std::string &s) const
    {
        // ref: http://www.cse.yorku.ca/~oz/hash.html

        unsigned long   hash = 5381;
        int 			c;
        const char      *str = s.c_str();

        while ((c = *str++))
            hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

        return hash;
    }
};

struct str_equal {
    bool operator()(const std::string &lhs, const std::string &rhs) const
    {
        return lhs == rhs;
    }
};


} // end for namespace qcontent

namespace qcontent
{

} // end for namespace qcontent

#endif

