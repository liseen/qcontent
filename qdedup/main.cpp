/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <inttypes.h>
#include <sys/mman.h>
#include <getopt.h>
#include <vector>
#include <cassert>
#include <iostream>

#include <strtk.hpp>

using namespace std;

#include "qcontent_record.h"

#define MAX_SENTENCES 1000

//#define DEDUP_DEBUG

static uint8_t* dedup_mmap = NULL;
typedef struct {
    uint64_t byte_pos;
    uint8_t bit_pos;
} sentence_flag_st;

static int verbose = 0;

static uint64_t get_hash_base(uint64_t max_limit);
static uint32_t sentence_flags_size = 0;
static sentence_flag_st sentence_flags[MAX_SENTENCES];

static uint64_t get_hash_base(uint64_t max_limit) {
    uint64_t primes[] = {
        1, 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 43, 47, 53, 59, 61, 71, 79, 83,
        89, 103, 109, 113, 127, 139, 157, 173, 191, 199, 223, 239, 251, 283, 317, 349,
        383, 409, 443, 479, 509, 571, 631, 701, 761, 829, 887, 953, 1021, 1151, 1279,
        1399, 1531, 1663, 1789, 1913, 2039, 2297, 2557, 2803, 3067, 3323, 3583, 3833,
        4093, 4603, 5119, 5623, 6143, 6653, 7159, 7673, 8191, 9209, 10223, 11261,
        12281, 13309, 14327, 15359, 16381, 18427, 20479, 22511, 24571, 26597, 28669,
        30713, 32749, 36857, 40949, 45053, 49139, 53239, 57331, 61417, 65521, 73727,
        81919, 90107, 98299, 106487, 114679, 122869, 131071, 147451, 163819, 180221,
        196597, 212987, 229373, 245759, 262139, 294911, 327673, 360439, 393209, 425977,
        458747, 491503, 524287, 589811, 655357, 720887, 786431, 851957, 917503, 982981,
        1048573, 1179641, 1310719, 1441771, 1572853, 1703903, 1835003, 1966079,
        2097143, 2359267, 2621431, 2883577, 3145721, 3407857, 3670013, 3932153,
        4194301, 4718579, 5242877, 5767129, 6291449, 6815741, 7340009, 7864301,
        8388593, 9437179, 10485751, 11534329, 12582893, 13631477, 14680063, 15728611,
        16777213, 18874367, 20971507, 23068667, 25165813, 27262931, 29360087, 31457269,
        33554393, 37748717, 41943023, 46137319, 50331599, 54525917, 58720253, 62914549,
        67108859, 75497467, 83886053, 92274671, 100663291, 109051903, 117440509,
        125829103, 134217689, 150994939, 167772107, 184549373, 201326557, 218103799,
        234881011, 251658227, 268435399, 301989881, 335544301, 369098707, 402653171,
        436207613, 469762043, 503316469, 536870909, 603979769, 671088637, 738197503,
        805306357, 872415211, 939524087, 1006632947, 1073741789, 1207959503,
        1342177237, 1476394991, 1610612711, 1744830457, 1879048183, 2013265907,
        2576980349, 3092376431, 3710851741, 4718021527, 6133428047, 7973456459,
        10365493393, 13475141413, 17517683831, 22772988923, 29604885677, 38486351381,
        50032256819, 65041933867, 84554514043, 109920868241, 153889215497, 0
    };
    uint64_t i, max_pos = 0;
    for(i = 0; primes[i] > 0; i++){
        if(max_limit > primes[i])
            max_pos = i;
    }

    return primes[max_pos];
}

inline static uint64_t hash_bytes(const char* str, size_t size, uint64_t base, uint64_t host_base = 0) {
    uint64_t state = host_base;
    uint32_t count = 0;
    while (size--) {
        if (isspace(*str)) {
            ++str;
            continue;
        }
        ++count;
        uint64_t highorder = state & 0x8000000000000000;
        state = state << 1;
        state = state ^ (highorder >> 63);
        if (host_base && isdigit(*str)) {
            // host_base is NOT 0, not hostname, in content digit char will be replaced.
            state = state ^ (uint8_t)('_');
            str++;
        } else {
            // host_base is 0, this str is hostname, digit char will be used.
            state = state ^ (uint8_t)(*str++);
        }
    }

    return state % base;
}

static int start_with_punct(const char *str) {
    switch (*str) {
//        case ',':
        case '.':
//        case ':':
        case '!':
        case '?':
        case ';':
        case '\n':
        case '\x01':
            return 1;
    };

    typedef struct {
        const char * punct;
        int size;
    } cn_punct;

    static cn_punct cn_puncts[] = {
 //       { "，", sizeof("，") - 1 },
        { "。", sizeof("。") - 1 },
        { "！", sizeof("！") - 1},
        { "？", sizeof("？") - 1},
        { "；", sizeof("？") - 1},
        { "·", sizeof("·") - 1},
        //{ "“", sizeof("“")  - 1},
        //{ "”", sizeof("”")  - 1},
        //{ "、", sizeof("、") - 1 },
        { NULL, 0}
    };

    for (cn_punct *it = cn_puncts; it->punct != NULL; it++) {
        if (strncmp(str, it->punct,  it->size) == 0) {
            return it->size;
        }
    }

    return 0;
}

static void check_usage(uint64_t hash_base)
{
    static uint8_t bit_powers[] = {
        0x01,
        0x02,
        0x04,
        0x08,
        0x10,
        0x20,
        0x40,
        0x80
    };


    uint64_t usage_pos = 0;
    for (uint64_t i = 0; i < hash_base; ++i) {
        uint64_t byte_pos = i >> 3;
        uint8_t bit_pos = bit_powers[i & 0x0000000000000007];
        if (dedup_mmap[byte_pos] & bit_pos) {
           ++usage_pos;
        }
    }

    fprintf(stderr, "usage_pos: %ld, hash_base: %ld ratio: %f\n",
            usage_pos, hash_base,
            static_cast<double>(usage_pos)/static_cast<double>(hash_base));
}

static int  dedup_check(uint64_t hash_base, const char* host, size_t host_size, const char* content, size_t content_size) {
    static uint8_t bit_powers[] = {
        0x01,
        0x02,
        0x04,
        0x08,
        0x10,
        0x20,
        0x40,
        0x80
    };

    uint64_t host_base = 0;

    if (host != NULL) {
        host_base = hash_bytes(host, host_size, hash_base, 0);
    }

    const char *prev_begin = content;
    const char *p = content;
    const char *end_p = content + content_size;

    sentence_flags_size = 0;

    int repeat_count =  0;
    int total = 0;
    int total_record = 0;
    bool enough_sentence = false;

    while (p < end_p) {
        if (total_record > MAX_SENTENCES - 1 ) {
            enough_sentence = true;
            break;
        }

        int punct_size = start_with_punct(p);
        if (punct_size == 0) {
            p++;
            continue;
        } else {
            // process prev
            uint32_t size = p - prev_begin;
            if (size > 20) {
                uint64_t hash = hash_bytes(prev_begin, size, hash_base, host_base);

#ifdef DEDUP_DEBUG
                std::string debug_str;
                debug_str.append(prev_begin, p - prev_begin);
                cerr << "sentence hash: " << hash << " str_size: " << size << " str:" <<  debug_str << endl;
#endif
                total += size;
                total_record++;
                uint64_t byte_pos = hash >> 3;
                uint8_t bit_pos = bit_powers[hash & 0x0000000000000007];
                if (dedup_mmap[byte_pos] & bit_pos) {
                    repeat_count += size;
                } else {
                    sentence_flags[sentence_flags_size].byte_pos = byte_pos;
                    sentence_flags[sentence_flags_size].bit_pos = bit_pos;
                    sentence_flags_size++;
                }
            }

            p = p + punct_size;
            prev_begin = p;
        }
    }

    uint32_t size = p - prev_begin;
    if (!enough_sentence && size > 20) {
        uint64_t hash = hash_bytes(prev_begin, size, hash_base, host_base);
        total += size;
        uint64_t byte_pos = hash >> 3;
        uint8_t bit_pos = bit_powers[hash & 0x0000000000000007];

#ifdef DEDUP_DEBUG
        std::string debug_str;
        debug_str.append(prev_begin, p - prev_begin);
        cerr << "last sentence hash: " << hash << " str_size: " << size << " str:" <<  debug_str << endl;
#endif
        if (dedup_mmap[byte_pos] & bit_pos) {
            repeat_count += size;
        } else {
            sentence_flags[sentence_flags_size].byte_pos = byte_pos;
            sentence_flags[sentence_flags_size].bit_pos = bit_pos;
            sentence_flags_size++;
        }
    }

    if (total == 0) {
        return 1;
    } else {
        float ratio = (float) repeat_count / total;
#ifdef DEDUP_DEBUG
        std::cerr << "total: " << total << " repeat: " << repeat_count << " content: " << content << "\n";
#endif
        if (ratio >= 0.85) {
            return 1;
        } else {
            for (uint32_t i = 0; i < sentence_flags_size; i++) {
                dedup_mmap[sentence_flags[i].byte_pos] |= sentence_flags[i].bit_pos;
            }
            return 0;
        }
    }
}

static uint8_t * init_mmap(const char *map_file, size_t map_size, int* map_fd) {
    int result;
    int fd = open(map_file, O_CREAT | O_RDWR, (mode_t)0777);
    if (fd == -1) {
        perror("Error opening file for writing");
        exit(EXIT_FAILURE);
    }
    result = lseek(fd, map_size - 1, SEEK_SET);
    if (result == -1) {
        close(fd);
        perror("Error calling lseek() to 'stretch' the file");
        exit(EXIT_FAILURE);
    }
    result = write(fd, "", 1);
    if (result != 1) {
        close(fd);
        perror("Error writing last byte of the file");
        exit(EXIT_FAILURE);
    }
    uint8_t * map =(unsigned char*) mmap(0, map_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_NORESERVE, fd, 0);
    //uint8_t * map =(unsigned char*) mmap(0, map_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if  (map == MAP_FAILED) {
        perror("Error mmap file");
        close(fd);
        exit(EXIT_FAILURE);
    }

    *map_fd = fd;
    return map;
}

/*
static void reset_map() {
    memset(map, 0, map_size);
}

static size_t  filter_content(const char* content, size_t content_size) {
    int first_puct_pos = -1;
    for (int i = 0; i < (int)content_size; i++) {
        int punct_size = start_with_punct(content + i);
        if (punct_size > 0) {
            first_puct_pos = i;
            break;
        }
    }

    if (first_puct_pos >= 0) {
        bool has_found_sentence = false;
        for (int i = first_puct_pos - 1; i >= 0; i--) {
            unsigned char c = *(content + i);
            if (c == ' ' || c == '\t' || c == '\n') {
                if (has_found_sentence)
                    return i;
            } else {
                has_found_sentence = true;
            }
        }
        return 0;
    } else {
        return -1;
    }
}
*/

void print_usage (const char* program_name, int exit_code)
{
    FILE* stream = stdout;
    if (exit_code != 0 ) {
        stream = stderr;
    }

    fprintf (stream, "Usage:  %s options \n", program_name);
    fprintf (stream,
            "  -h  --help             Display this usage information.\n"
            "  -m  --mmap-file        Dedup mmap filename.\n"
            "  -s  --mmap-size        Dedup mmap filesize.\n"
            "  -t  --enable-stat      Dedup stat output.\n"
            "  -f  --enable-filter    Dedup enable content filter.\n"
            "  -v  --verbose          Print verbose messages.\n");
    exit (exit_code);
}


int main(int argc, char *argv[])
{
    /* init */
    int  dedup_mmap_fd;
    const char* program_name = argv[0];
    uint64_t document_total = 0;

    const char* dedup_mmap_file = "default_dedup_mmap.dat";
    size_t dedup_mmap_size =  1024 * 1024 * 1024;
    bool enable_filter = false;
    bool enable_stat = false;
    bool usage = false;

    /* parse options */
    int next_option;

    const char* const short_options = "hm:s:tfuv";
    const struct option long_options[] = {
        { "help",     0, NULL, 'h' },
        { "mmap-file",   1, NULL, 'm' },
        { "mmap-size",   1, NULL, 's' },
        { "enable-stat",  0, NULL, 't' },
        { "enable-filter",  0, NULL, 'f' },
        { "usage",  0, NULL, 'u' },
        { "verbose",  0, NULL, 'v' },
        { NULL,       0, NULL, 0   }
    };

    do {
        next_option = getopt_long (argc, argv, short_options,
                long_options, NULL);
        switch (next_option)
        {
            case 'h':
                print_usage(program_name, 0);
            case 'm':
                dedup_mmap_file = optarg;
                break;
            case 's':
                dedup_mmap_size = atoi(optarg) * 1024 * 1024;
                break;
            case 't':
                enable_stat = true;
                break;
            case 'f':
                enable_filter = true;
                break;
            case 'u':
                usage = true;
                break;
            case 'v':
                verbose = 1;
                break;
            case '?':
                print_usage(program_name, 1);
            case -1:
                break;
            default:
                abort ();
        }
    } while (next_option != -1);

    if (dedup_mmap_size < 64 * 1024 * 1024) {
        dedup_mmap_size = 64 * 1024 * 1024;
    }

    // init hash base
    uint64_t  hash_base = get_hash_base(dedup_mmap_size * 8);
    if (verbose) {
        fprintf(stderr, "hash_base prime: %ld\n", hash_base);
    }

    // init map
    dedup_mmap = init_mmap(dedup_mmap_file, dedup_mmap_size, &dedup_mmap_fd);
    if (dedup_mmap == NULL) {
        fprintf(stderr, "init mmap file: %s error\n", dedup_mmap_file);
        exit(EXIT_FAILURE);
    }

    if (usage) {
        check_usage(hash_base);
    } else {
        std::string line;
        int lineno = 0;
        std::vector<std::string> record;
        while (getline(cin, line)) {
            ++lineno;

            if ((lineno % 100000) == 0 && enable_stat) {
                fprintf(stderr, "......lines: %d , documents: %ld ......\n", lineno, document_total);
            }

            record.clear();
            strtk::parse(line.c_str(), line.c_str() + line.size(),  "\t", record, strtk::split_options::default_mode);
            if (record.size() < 26) {
                std::cerr << "error found lineno: " << lineno << std::endl;
                continue;
            }

            const std::string &site = record[6];
            //const std::string &host = record[7];

            const std::string &list_confidence_str = record[17];
            const std::string &content_confidence_str = record[18];

            const std::string crawl_type = record[9];
            const std::string &raw_title = record[21];
            //const std::string &title = record[22];
            //const std::string &keywords = record[23];
            const std::string &desc = record[24];
            const std::string &content = record[25];

            int list_confidence = 0;
            strtk::string_to_type_converter(list_confidence_str, list_confidence);
            int content_confidence = 0;
            strtk::string_to_type_converter(content_confidence_str, content_confidence);

            char crawl_cmd = crawl_type.size() > 0 ? \
                        crawl_type.at(0) : qcontent::crawlcommand::GAIN;

            if (crawl_cmd == qcontent::crawlcommand::GAIN) {
                std::cout << line << "\n";
                continue;
            }

            if (list_confidence > 0 && content_confidence <= 50) {
                std::cout << line << "\n";
                continue;
            }

            std::string all_content;
            all_content.reserve(desc.size() + 1 + raw_title.size() + 1 + content.size());
            all_content.append(desc);
            all_content.append("\n");
            all_content.append(raw_title);
            all_content.append("\n");
            all_content.append(content);

            if (content.size() == 0 || site.size() == 0) {
                continue;
            }

            int ret = dedup_check(hash_base, site.c_str(), site.size(), all_content.c_str(), all_content.size());
            if (!ret) {
                document_total++;
                std::cout << line << "\n";
            } else {
                if (verbose) {
                    std::cerr << line << "\n";
                }
            }

            // msync per 1000000 record
            if (document_total != 0 && document_total % 1000000 == 0) {
                msync(dedup_mmap, dedup_mmap_size, MS_ASYNC);
            }
        }

        if (enable_stat) {
            fprintf(stderr, "\n");
        }
    }

    if (munmap(dedup_mmap, dedup_mmap_size) == -1) {
        perror("Error un-mmapping the file");
    }
    close(dedup_mmap_fd);

    return 0;
}
