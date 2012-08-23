/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 *    zhenbao.zhou (周桢堡)  <zhenbao.zhou@qunar.com>
 *
 * Some common struct between client and server
 */

#ifndef QDEDUP_RPC_QDEDUP_COMMON_H_
#define QDEDUP_RPC_QDEDUP_COMMON_H_

#include <sys/types.h>
#include <inttypes.h>
#include <msgpack.hpp>
#include <vector>

#include "qdedup_constant.h"

namespace qcontent {

// a finger of string
/* 将某个字符串转换成指纹*/

struct str_finger_t {
    uint64_t dedup_mark;    // a hash value of the str
    uint32_t content_size;  // the length of the str
    MSGPACK_DEFINE(dedup_mark, content_size);
};

// mmap stat
struct mmap_stat_t {
    uint64_t mmap_file_size;
    uint32_t mmap_usage;
    MSGPACK_DEFINE(mmap_file_size, mmap_usage);
};
}

#endif  // QDEDUP_RPC_QDEDUP_COMMON_H_
