/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 *    zhenbao.zhou (周桢堡)  <zhenbao.zhou@qunar.com>
 *
 * Dedup logic here
 */

#include <msgpack.hpp>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>
#include <vector>

#include <glog/logging.h>

#include "qdedup_backend.h"

namespace qcontent {

QDedupBackend::QDedupBackend(const std::string &file, size_t size, size_t sync_threshold) {
    mmap_file = file;
    mmap_size = size;
    mmap_sync_threshold = sync_threshold;
    
    dedup_mmap = NULL;
    mmap_fd = -1;
    mmap_flags_counter = 0;
}

const mmap_stat_t& QDedupBackend::check_usage() {
    uint64_t usage_pos = 0;
    for (uint64_t i = 0; i < mmap_size * 8; ++i) {
        uint64_t byte_pos = i >> 3;
        uint8_t bit_pos = 1 << (i & 0x07);
        if (dedup_mmap[byte_pos] & bit_pos) {
           ++usage_pos;
        }
    }

    mmap_stat_t mmap_stat;
    mmap_stat.mmap_file_size = mmap_size;
    mmap_stat.mmap_usage = usage_pos * 100 / (mmap_size * 8);
    return mmap_stat;
}

int QDedupBackend::init_backend() {
    return init_mmap();
}

int QDedupBackend::init_mmap() {
    int result;
    const char *m_file = mmap_file.c_str();
    int fd = open(m_file, O_CREAT | O_RDWR, (mode_t)0777);
    if (fd == -1) {
        perror("Error opening file for writing");
        return 1;
    }

    result = lseek(fd, mmap_size - 1, SEEK_SET);
    if (result == -1) {
        close(fd);
        perror("Error calling lseek() to 'stretch' the file");
        return 1;
    }

    result = write(fd, "", 1);
    if (result != 1) {
        close(fd);
        perror("Error writing last byte of the file");
        return  1;
    }

    uint8_t * map = (unsigned char*) mmap(0, mmap_size,
                                          PROT_READ | PROT_WRITE,
                                          MAP_SHARED | MAP_NORESERVE, fd, 0);
    if  (map == MAP_FAILED) {
        perror("Error mmap file");
        close(fd);
        return 1;
    }

    mmap_fd = fd;
    dedup_mmap = map;

    return 0;
}

void  QDedupBackend::insert_into_mmap(const std::vector<sentence_flag_st> &sentence_flags) {
    uint32_t flags_size = sentence_flags.size();
    for (uint32_t i = 0; i < flags_size; i++) {
        uint32_t byte_pos  = sentence_flags[i].byte_pos;

        assert(byte_pos < mmap_size);
        dedup_mmap[sentence_flags[i].byte_pos] |= sentence_flags[i].bit_pos;
        mmap_flags_counter++;

        //  commits every mmap_sync_threshold flags
        if (mmap_flags_counter > mmap_sync_threshold) {
            mmap_sync();
        }
    }
    
    return;
}

int QDedupBackend::dedup_check(const std::vector<str_finger_t> &fingers) {
    request_counter++;
    LOG_EVERY_N(INFO, 10000) << "Got " << request_counter << "dedup request";
    std::vector<sentence_flag_st> sentence_flags;
    uint32_t marks_num = fingers.size();

    if (marks_num == 0) {
        return IS_DEDUP_CONTENT;
    }

    if (marks_num > (uint32_t)MAX_SENTENCES) {
        marks_num = MAX_SENTENCES;
    }

    int repeat_count = 0;
    int total_count = 0;
    uint32_t i = 0;

    for (; i < marks_num; i++) {
        str_finger_t sft = fingers[i];
        total_count += sft.content_size;

        // cerr << "dedup_mark:" << sft.dedup_mark << endl;
        // cerr << "len : "  << sft.content_size << endl;

        // A byte contains 8 bits. So, this mmap_file can only
        // contains mmap_file*8 dedup_marks
        uint32_t used_dedup = sft.dedup_mark % (mmap_size << 3);

        uint64_t byte_pos = used_dedup >> 3;
        uint8_t bit_pos = 1 << (used_dedup & 0x07);

        sentence_flag_st sentence;
        sentence.byte_pos = byte_pos;
        sentence.bit_pos = bit_pos;

        if (dedup_check_sentence(sentence) == IS_DEDUP_CONTENT) {
            repeat_count += sft.content_size;
        } else {
            sentence_flags.push_back(sentence);
        }
    }

    float dedup_ratio = (float) repeat_count / total_count;
    if (dedup_ratio > DEDUP_THESHOLD) {
        return IS_DEDUP_CONTENT;
    } else {
        insert_into_mmap(sentence_flags);
    }

    return IS_NEW_CONTENT;
}

int QDedupBackend::dedup_check_sentence(const sentence_flag_st &sentence) {
    if (dedup_mmap[sentence.byte_pos] & sentence.bit_pos) {
        return IS_DEDUP_CONTENT;
    } else {
        return IS_NEW_CONTENT;
    }
}

void QDedupBackend::mmap_sync() {
    LOG(INFO) << "Start sync mmap. mmap_flags_counter: " << mmap_flags_counter;
    msync(dedup_mmap, mmap_size, MS_ASYNC);
    LOG(INFO) << "Stop sync mmap";

    mmap_flags_counter = 0;
    return;
}

void QDedupBackend::stop() {
    LOG(INFO) << "Dedup backend stops";

    mmap_sync();
    close_mmap();

    return;
}
        

int QDedupBackend::close_mmap() {
    munmap(dedup_mmap, mmap_size);
    close(mmap_fd);

    dedup_mmap = NULL;
    return 0;
}

QDedupBackend::~QDedupBackend() {
    //    close_mmap();
}
}
