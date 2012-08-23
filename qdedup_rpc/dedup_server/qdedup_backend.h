/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 *    zhenbao.zhou (周桢堡)  <zhenbao.zhou@qunar.com>
 *
 * dedup logic here
 */

#ifndef QDEDUP_RPC_QDEDUP_BACKEND_H_
#define QDEDUP_RPC_QDEDUP_BACKEND_H_

#include <stdio.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <inttypes.h>
#include <string>
#include <vector>

#include "qdedup_common.h"

namespace qcontent {

struct sentence_flag_st {
    uint64_t byte_pos;
    uint8_t bit_pos;
};

class QDedupBackend {
  public:
    QDedupBackend(const std::string &map_file, size_t map_size, size_t sync_threshold = 500000);

    /*
     * Init backend
     */
    int init_backend();

    /*
     * Inputs:
     * struct str_finger_t is needed before using.
     *
     * returns:
     * IS_NEW_CONTENT
     * IS_DEDUP_CONTENT
     */
    int dedup_check(const std::vector<str_finger_t> &fingers);

    /*
     * return the usage of the dedup mmap.
     * such as 86(%)
     */
    const mmap_stat_t & check_usage();
    
    void mmap_sync ();

    void stop();
    
    ~QDedupBackend();

  private:
    // if deduped_content ratio larger than DEDUP_THESHOLD,
    // mark this article as deduped content
    const static float DEDUP_THESHOLD = 0.85;

    int init_mmap();
    int close_mmap();

    int dedup_check_sentence(const sentence_flag_st &sentence);

    void insert_into_mmap(const std::vector<sentence_flag_st> &sentence_flags);

    std::string mmap_file;
    size_t mmap_size;
    int mmap_fd;
    uint8_t *dedup_mmap;

    //  record how much mmap_flags_size stored in mmap.
    size_t mmap_flags_counter;
    size_t mmap_sync_threshold;
    uint64_t request_counter;
};

}  //  end of namespace qcontent
#endif  // QDEDUP_RPC_QDEDUP_BACKEND_H_
