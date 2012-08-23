/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

#ifndef QCONTENT_QWS_NORMALIZE_H
#define QCONTENT_QWS_NORMALIZE_H

#include "qws_common.h"
#include <string>

BEGIN_NAMESPACE_QWS

class QWSNormalize {
public:
    enum NormalizeMethod {
        DBC_TO_SBC = 1,
        SBC_TO_DBC = 2,
        TO_LOWER = 4,
        TO_UPPER = 8
    };

    inline static const char * dbc_to_sbc(int *val);
    inline static const char * upper_to_lower(int *val);

    inline static void normalize(const char *start, size_t len, int val, int normalized_method,\
            std::string *str, int *normal_val);
};

inline void QWSNormalize::normalize(const char *start, size_t len, int val, int normalized_method,
        std::string *str, int *normal_val) {
    const char *normal_char = NULL;
    const char *s = NULL;
    int tmp_val = val;
    if (normalized_method & DBC_TO_SBC) {
        if ((s = dbc_to_sbc(&tmp_val)) != NULL) {
            normal_char = s;
        }
    }

    if (normalized_method & TO_LOWER) {
        if ((s = QWSNormalize::upper_to_lower(&tmp_val)) != NULL) {
            normal_char = s;
        }
    }

    if (normal_char) {
        str->append(normal_char);
        *normal_val = tmp_val;
    } else {
        str->append(start, len);
        *normal_val = val;
    }
}

inline const char * QWSNormalize::dbc_to_sbc(int *val) {
    int v = *val;
    static const char * dbc_to_sbc_array[] = {
        "!", // ！ => !
        "\"", // ＂ => "
        "#", // ＃ => #
        "$", // ＄ => $
        "%", // ％ => %
        "&", // ＆ => &
        "'", // ＇ => '
        "(", // （ => (
        ")", // ） => )
        "*", // ＊ => *
        "+", // ＋ => +
        ",", // ， => ,
        "-", // － => -
        ".", // ． => .
        "/", // ／ => /
        "0", // ０ => 0
        "1", // １ => 1
        "2", // ２ => 2
        "3", // ３ => 3
        "4", // ４ => 4
        "5", // ５ => 5
        "6", // ６ => 6
        "7", // ７ => 7
        "8", // ８ => 8
        "9", // ９ => 9
        ":", // ： => :
        ";", // ； => ;
        "<", // ＜ => <
        "=", // ＝ => =
        ">", // ＞ => >
        "?", // ？ => ?
        "@", // ＠ => @
        "A", // Ａ => A
        "B", // Ｂ => B
        "C", // Ｃ => C
        "D", // Ｄ => D
        "E", // Ｅ => E
        "F", // Ｆ => F
        "G", // Ｇ => G
        "H", // Ｈ => H
        "I", // Ｉ => I
        "J", // Ｊ => J
        "K", // Ｋ => K
        "L", // Ｌ => L
        "M", // Ｍ => M
        "N", // Ｎ => N
        "O", // Ｏ => O
        "P", // Ｐ => P
        "Q", // Ｑ => Q
        "R", // Ｒ => R
        "S", // Ｓ => S
        "T", // Ｔ => T
        "U", // Ｕ => U
        "V", // Ｖ => V
        "W", // Ｗ => W
        "X", // Ｘ => X
        "Y", // Ｙ => Y
        "Z", // Ｚ => Z
        "[", // ［ => [
        "\\", // ＼ => "\"
        "]", // ］ => ]
        "^", // ＾ => ^
        "_", // ＿ => _
        "`", // ｀ => `
        "a", // ａ => a
        "b", // ｂ => b
        "c", // ｃ => c
        "d", // ｄ => d
        "e", // ｅ => e
        "f", // ｆ => f
        "g", // ｇ => g
        "h", // ｈ => h
        "i", // ｉ => i
        "j", // ｊ => j
        "k", // ｋ => k
        "l", // ｌ => l
        "m", // ｍ => m
        "n", // ｎ => n
        "o", // ｏ => o
        "p", // ｐ => p
        "q", // ｑ => q
        "r", // ｒ => r
        "s", // ｓ => s
        "t", // ｔ => t
        "u", // ｕ => u
        "v", // ｖ => v
        "w", // ｗ => w
        "x", // ｘ => x
        "y", // ｙ => y
        "z", // ｚ => z
        "{", // ｛ => {
        "|", // ｜ => |
        "}", // ｝ => }
        "~", // ～ => ~
        NULL,
    };

    if (v == 12288) { // ' '
        *val = ' ';
        return " ";
    } else if (v == 12290) { // 。
        *val = 46;
        return ".";
    } if (v >= 65281 && v <= 65374) {
        // bias 65248
        *val = v - 65248;
        return dbc_to_sbc_array[v - 65281];
    }

    return NULL;
}

inline const char * QWSNormalize::upper_to_lower(int *val) {
    int v = *val;
    static const char *to_lower_array[] = {
        "a", "b", "c", "d", "e", "f", "g", \
        "h", "i", "j", "k", "l", "m", "n", \
        "o", "p", "q", "r", "s", "t", \
        "u", "v", "w", "x", "y", "z", \
        NULL
    };
    if (v >= static_cast<int>('A') && v <= static_cast<int>('Z')) {
       *val = v - static_cast<int>('A') + static_cast<int>('a');
       return to_lower_array[v - 'A'];
    }

    return NULL;
}

END_NAMESPACE_QWS

#endif

