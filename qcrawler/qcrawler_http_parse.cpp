
#line 1 "qcrawler_http_parse.rl"
#include "qcrawler_http_parse.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <zlib.h>

#define QCRAWLER_DECOMPRESS_BUF_SIZE 2*1024*1024

//#define TEST_PARSE 1

namespace qcrawler
{

static int httpgzdecompress(Byte *zdata, uLong nzdata, Byte *data, uLong *ndata);


#line 21 "qcrawler_http_parse.cpp"
static const char _chunked_actions[] = {
	0, 1, 0, 1, 2, 1, 3, 2, 
	0, 2, 2, 1, 0, 2, 2, 3, 
	3, 2, 1, 0
};

static const char _chunked_cond_offsets[] = {
	0, 0, 0, 0, 0, 1, 1, 1, 
	1, 1, 1, 1, 1, 2, 2, 3, 
	4, 5, 5, 6, 6, 6, 6, 6, 
	6, 6, 6, 7, 7, 7, 8, 9, 
	10, 10, 10, 10, 10, 10, 10, 10, 
	11, 12, 13, 14, 15, 16, 17, 18, 
	19, 20, 21, 22, 23, 24, 25, 25, 
	25, 25, 25, 25, 25, 26, 27, 28, 
	29, 30, 31, 31, 31
};

static const char _chunked_cond_lengths[] = {
	0, 0, 0, 0, 1, 0, 0, 0, 
	0, 0, 0, 0, 1, 0, 1, 1, 
	1, 0, 1, 0, 0, 0, 0, 0, 
	0, 0, 1, 0, 0, 1, 1, 1, 
	0, 0, 0, 0, 0, 0, 0, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 0, 0, 
	0, 0, 0, 0, 1, 1, 1, 1, 
	1, 1, 0, 0, 1
};

static const short _chunked_cond_keys[] = {
	-128, 127, -128, 127, -128, 127, -128, 127, 
	-128, 127, -128, 127, -128, 127, -128, 127, 
	-128, 127, -128, 127, -128, 127, -128, 127, 
	-128, 127, -128, 127, -128, 127, -128, 127, 
	-128, 127, -128, 127, -128, 127, -128, 127, 
	-128, 127, -128, 127, -128, 127, -128, 127, 
	-128, 127, -128, 127, -128, 127, -128, 127, 
	-128, 127, -128, 127, -128, 127, -128, 127, 
	0
};

static const char _chunked_cond_spaces[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0
};

static const short _chunked_key_offsets[] = {
	0, 0, 7, 16, 17, 26, 28, 29, 
	31, 32, 33, 41, 42, 46, 47, 53, 
	77, 103, 104, 137, 146, 152, 161, 168, 
	171, 174, 183, 192, 193, 201, 207, 213, 
	237, 238, 242, 249, 254, 257, 259, 267, 
	273, 289, 313, 331, 342, 348, 372, 407, 
	413, 438, 473, 500, 511, 522, 557, 561, 
	568, 573, 576, 578, 586, 602, 626, 644, 
	655, 661, 685, 685, 692
};

static const short _chunked_trans_keys[] = {
	48, 49, 57, 65, 70, 97, 102, 13, 
	48, 59, 49, 57, 65, 70, 97, 102, 
	10, 269, 522, 525, 128, 265, 267, 383, 
	384, 639, 10, 13, 10, 10, 13, 10, 
	10, 13, 59, 48, 57, 65, 70, 97, 
	102, 10, 269, 525, 384, 639, 10, 266, 
	269, 522, 525, 384, 639, 269, 304, 525, 
	560, 305, 313, 321, 326, 353, 358, 384, 
	559, 561, 569, 570, 576, 577, 582, 583, 
	608, 609, 614, 615, 639, 269, 304, 315, 
	525, 560, 571, 305, 313, 321, 326, 353, 
	358, 384, 559, 561, 569, 570, 576, 577, 
	582, 583, 608, 609, 614, 615, 639, 10, 
	269, 522, 525, 128, 265, 267, 303, 304, 
	313, 314, 320, 321, 326, 327, 352, 353, 
	358, 359, 383, 384, 559, 560, 569, 570, 
	576, 577, 582, 583, 608, 609, 614, 615, 
	639, 10, 13, 59, 48, 57, 65, 70, 
	97, 102, 10, 13, 65, 90, 97, 122, 
	10, 13, 61, 48, 57, 65, 90, 97, 
	122, 10, 13, 34, 65, 90, 97, 122, 
	10, 13, 34, 10, 13, 59, 10, 13, 
	59, 48, 57, 65, 90, 97, 122, 269, 
	522, 525, 128, 265, 267, 383, 384, 639, 
	10, 10, 13, 48, 57, 65, 70, 97, 
	102, 266, 269, 522, 525, 384, 639, 266, 
	269, 522, 525, 384, 639, 269, 315, 525, 
	571, 304, 313, 321, 326, 353, 358, 384, 
	559, 560, 569, 570, 576, 577, 582, 583, 
	608, 609, 614, 615, 639, 10, 65, 90, 
	97, 122, 61, 48, 57, 65, 90, 97, 
	122, 34, 65, 90, 97, 122, 10, 13, 
	34, 13, 59, 13, 59, 48, 57, 65, 
	90, 97, 122, 266, 269, 522, 525, 384, 
	639, 269, 525, 321, 346, 353, 378, 384, 
	576, 577, 602, 603, 608, 609, 634, 635, 
	639, 269, 317, 525, 573, 304, 313, 321, 
	346, 353, 378, 384, 559, 560, 569, 570, 
	576, 577, 602, 603, 608, 609, 634, 635, 
	639, 269, 290, 525, 546, 321, 346, 353, 
	378, 384, 576, 577, 602, 603, 608, 609, 
	634, 635, 639, 269, 290, 522, 525, 546, 
	128, 265, 267, 383, 384, 639, 269, 315, 
	525, 571, 384, 639, 269, 315, 525, 571, 
	304, 313, 321, 346, 353, 378, 384, 559, 
	560, 569, 570, 576, 577, 602, 603, 608, 
	609, 634, 635, 639, 269, 315, 522, 525, 
	571, 128, 265, 267, 303, 304, 313, 314, 
	320, 321, 326, 327, 352, 353, 358, 359, 
	383, 384, 559, 560, 569, 570, 576, 577, 
	582, 583, 608, 609, 614, 615, 639, 266, 
	269, 522, 525, 384, 639, 269, 522, 525, 
	128, 265, 267, 320, 321, 346, 347, 352, 
	353, 378, 379, 383, 384, 576, 577, 602, 
	603, 608, 609, 634, 635, 639, 269, 317, 
	522, 525, 573, 128, 265, 267, 303, 304, 
	313, 314, 320, 321, 346, 347, 352, 353, 
	378, 379, 383, 384, 559, 560, 569, 570, 
	576, 577, 602, 603, 608, 609, 634, 635, 
	639, 269, 290, 522, 525, 546, 128, 265, 
	267, 320, 321, 346, 347, 352, 353, 378, 
	379, 383, 384, 576, 577, 602, 603, 608, 
	609, 634, 635, 639, 269, 290, 522, 525, 
	546, 128, 265, 267, 383, 384, 639, 269, 
	315, 522, 525, 571, 128, 265, 267, 383, 
	384, 639, 269, 315, 522, 525, 571, 128, 
	265, 267, 303, 304, 313, 314, 320, 321, 
	346, 347, 352, 353, 378, 379, 383, 384, 
	559, 560, 569, 570, 576, 577, 602, 603, 
	608, 609, 634, 635, 639, 65, 90, 97, 
	122, 61, 48, 57, 65, 90, 97, 122, 
	34, 65, 90, 97, 122, 10, 13, 34, 
	13, 59, 13, 59, 48, 57, 65, 90, 
	97, 122, 269, 525, 321, 346, 353, 378, 
	384, 576, 577, 602, 603, 608, 609, 634, 
	635, 639, 269, 317, 525, 573, 304, 313, 
	321, 346, 353, 378, 384, 559, 560, 569, 
	570, 576, 577, 602, 603, 608, 609, 634, 
	635, 639, 269, 290, 525, 546, 321, 346, 
	353, 378, 384, 576, 577, 602, 603, 608, 
	609, 634, 635, 639, 269, 290, 522, 525, 
	546, 128, 265, 267, 383, 384, 639, 269, 
	315, 525, 571, 384, 639, 269, 315, 525, 
	571, 304, 313, 321, 346, 353, 378, 384, 
	559, 560, 569, 570, 576, 577, 602, 603, 
	608, 609, 634, 635, 639, 48, 49, 57, 
	65, 70, 97, 102, 269, 304, 525, 560, 
	305, 313, 321, 326, 353, 358, 384, 559, 
	561, 569, 570, 576, 577, 582, 583, 608, 
	609, 614, 615, 639, 0
};

static const char _chunked_single_lengths[] = {
	0, 1, 3, 1, 3, 2, 1, 2, 
	1, 1, 2, 1, 2, 1, 4, 4, 
	6, 1, 3, 3, 2, 3, 3, 3, 
	3, 3, 3, 1, 2, 4, 4, 4, 
	1, 0, 1, 1, 3, 2, 2, 4, 
	2, 4, 4, 5, 4, 4, 5, 4, 
	3, 5, 5, 5, 5, 5, 0, 1, 
	1, 3, 2, 2, 2, 4, 4, 5, 
	4, 4, 0, 1, 4
};

static const char _chunked_range_lengths[] = {
	0, 3, 3, 0, 3, 0, 0, 0, 
	0, 0, 3, 0, 1, 0, 1, 10, 
	10, 0, 15, 3, 2, 3, 2, 0, 
	0, 3, 3, 0, 3, 1, 1, 10, 
	0, 2, 3, 2, 0, 0, 3, 1, 
	7, 10, 7, 3, 1, 10, 15, 1, 
	11, 15, 11, 3, 3, 15, 2, 3, 
	2, 0, 0, 3, 7, 10, 7, 3, 
	1, 10, 0, 3, 10
};

static const short _chunked_index_offsets[] = {
	0, 0, 5, 12, 14, 21, 24, 26, 
	29, 31, 33, 39, 41, 45, 47, 53, 
	68, 85, 87, 106, 113, 118, 125, 131, 
	135, 139, 146, 153, 155, 161, 167, 173, 
	188, 190, 193, 198, 202, 206, 209, 215, 
	221, 231, 246, 258, 267, 273, 288, 309, 
	315, 330, 351, 368, 377, 386, 407, 410, 
	415, 419, 423, 426, 432, 442, 457, 469, 
	478, 484, 499, 500, 505
};

static const char _chunked_indicies[] = {
	0, 2, 2, 2, 1, 3, 4, 6, 
	5, 5, 5, 1, 7, 1, 9, 11, 
	12, 8, 8, 10, 1, 1, 13, 8, 
	14, 1, 1, 15, 8, 16, 1, 17, 
	1, 18, 19, 5, 5, 5, 1, 20, 
	1, 21, 22, 11, 1, 23, 1, 23, 
	21, 24, 22, 11, 1, 21, 0, 22, 
	25, 2, 2, 2, 11, 26, 11, 26, 
	11, 26, 11, 1, 27, 4, 6, 28, 
	29, 31, 5, 5, 5, 11, 30, 11, 
	30, 11, 30, 11, 1, 32, 1, 9, 
	11, 12, 8, 8, 33, 8, 33, 8, 
	33, 8, 10, 34, 10, 34, 10, 34, 
	10, 1, 1, 3, 36, 35, 35, 35, 
	8, 1, 13, 37, 37, 8, 1, 13, 
	38, 37, 37, 37, 8, 1, 13, 39, 
	40, 40, 8, 1, 13, 41, 39, 1, 
	3, 36, 8, 1, 3, 36, 40, 40, 
	40, 8, 42, 11, 43, 8, 8, 10, 
	1, 44, 1, 1, 15, 33, 33, 33, 
	8, 44, 21, 45, 22, 11, 1, 17, 
	21, 46, 22, 11, 1, 47, 19, 48, 
	49, 5, 5, 5, 11, 30, 11, 30, 
	11, 30, 11, 1, 50, 1, 51, 51, 
	1, 52, 51, 51, 51, 1, 53, 54, 
	54, 1, 1, 1, 55, 53, 18, 19, 
	1, 18, 19, 54, 54, 54, 1, 50, 
	21, 24, 22, 11, 1, 21, 22, 51, 
	51, 11, 56, 11, 56, 11, 1, 21, 
	52, 22, 57, 51, 51, 51, 11, 56, 
	11, 56, 11, 56, 11, 1, 21, 53, 
	22, 58, 54, 54, 11, 59, 11, 59, 
	11, 1, 21, 55, 11, 22, 60, 53, 
	53, 58, 1, 47, 19, 48, 49, 11, 
	1, 47, 19, 48, 49, 54, 54, 54, 
	11, 59, 11, 59, 11, 59, 11, 1, 
	27, 36, 11, 28, 62, 8, 8, 35, 
	8, 35, 8, 35, 8, 10, 61, 10, 
	61, 10, 61, 10, 1, 32, 21, 45, 
	22, 11, 1, 42, 11, 43, 8, 8, 
	37, 8, 37, 8, 10, 63, 10, 63, 
	10, 1, 42, 38, 11, 43, 64, 8, 
	8, 37, 8, 37, 8, 37, 8, 10, 
	63, 10, 63, 10, 63, 10, 1, 42, 
	39, 11, 43, 65, 8, 8, 40, 8, 
	40, 8, 10, 66, 10, 66, 10, 1, 
	42, 41, 11, 43, 67, 39, 39, 65, 
	1, 27, 36, 11, 28, 62, 8, 8, 
	10, 1, 27, 36, 11, 28, 62, 8, 
	8, 40, 8, 40, 8, 40, 8, 10, 
	66, 10, 66, 10, 66, 10, 1, 68, 
	68, 1, 69, 68, 68, 68, 1, 70, 
	71, 71, 1, 1, 1, 72, 70, 3, 
	6, 1, 3, 6, 71, 71, 71, 1, 
	21, 22, 68, 68, 11, 73, 11, 73, 
	11, 1, 21, 69, 22, 74, 68, 68, 
	68, 11, 73, 11, 73, 11, 73, 11, 
	1, 21, 70, 22, 75, 71, 71, 11, 
	76, 11, 76, 11, 1, 21, 72, 11, 
	22, 77, 70, 70, 75, 1, 27, 6, 
	28, 31, 11, 1, 27, 6, 28, 31, 
	71, 71, 71, 11, 76, 11, 76, 11, 
	76, 11, 1, 1, 0, 2, 2, 2, 
	1, 21, 0, 22, 25, 2, 2, 2, 
	11, 26, 11, 26, 11, 26, 11, 1, 
	0
};

static const char _chunked_trans_targs[] = {
	2, 0, 10, 3, 2, 10, 54, 4, 
	5, 9, 26, 12, 30, 6, 7, 8, 
	66, 67, 11, 33, 12, 13, 14, 1, 
	15, 16, 31, 17, 47, 16, 31, 60, 
	18, 19, 46, 19, 20, 21, 22, 23, 
	25, 24, 27, 29, 28, 18, 68, 32, 
	39, 40, 15, 34, 35, 36, 38, 37, 
	41, 42, 43, 45, 44, 46, 48, 49, 
	50, 51, 53, 52, 55, 56, 57, 59, 
	58, 61, 62, 63, 65, 64
};

static const char _chunked_trans_actions[] = {
	10, 0, 10, 0, 1, 1, 0, 0, 
	0, 5, 3, 3, 13, 0, 0, 0, 
	0, 0, 0, 0, 0, 5, 13, 0, 
	3, 16, 16, 5, 13, 7, 7, 3, 
	0, 10, 16, 1, 0, 0, 0, 0, 
	0, 0, 5, 13, 0, 3, 3, 5, 
	13, 3, 0, 0, 0, 0, 0, 0, 
	3, 3, 3, 3, 3, 7, 3, 3, 
	3, 3, 3, 3, 0, 0, 0, 0, 
	0, 3, 3, 3, 3, 3
};

static const int chunked_start = 1;
static const int chunked_first_final = 66;
static const int chunked_error = 0;

static const int chunked_en_main = 1;
static const int chunked_en_main_chunked_body_chunk_chunk_data = 12;


#line 20 "qcrawler_http_parse.rl"


bool http_chunked_parse(const char *message_body, size_t message_body_size, std::string &parsed_body)
{
	const char *p = message_body;
    const char *pe = p + message_body_size;
    //const char *eof = pe;

    int cs;
    int chunk_data_count = 0;
    int chunk_data_size = 0;

    
#line 330 "qcrawler_http_parse.cpp"
	{
	cs = chunked_start;
	}

#line 335 "qcrawler_http_parse.cpp"
	{
	int _klen;
	unsigned int _trans;
	short _widec;
	const char *_acts;
	unsigned int _nacts;
	const short *_keys;

	if ( p == pe )
		goto _test_eof;
	if ( cs == 0 )
		goto _out;
_resume:
	_widec = (*p);
	_klen = _chunked_cond_lengths[cs];
	_keys = _chunked_cond_keys + (_chunked_cond_offsets[cs]*2);
	if ( _klen > 0 ) {
		const short *_lower = _keys;
		const short *_mid;
		const short *_upper = _keys + (_klen<<1) - 2;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + (((_upper-_lower) >> 1) & ~1);
			if ( _widec < _mid[0] )
				_upper = _mid - 2;
			else if ( _widec > _mid[1] )
				_lower = _mid + 2;
			else {
				switch ( _chunked_cond_spaces[_chunked_cond_offsets[cs] + ((_mid - _keys)>>1)] ) {
	case 0: {
		_widec = (short)(128 + ((*p) - -128));
		if ( 
#line 33 "qcrawler_http_parse.rl"

            chunk_data_count++ < chunk_data_size
         ) _widec += 256;
		break;
	}
				}
				break;
			}
		}
	}

	_keys = _chunked_trans_keys + _chunked_key_offsets[cs];
	_trans = _chunked_index_offsets[cs];

	_klen = _chunked_single_lengths[cs];
	if ( _klen > 0 ) {
		const short *_lower = _keys;
		const short *_mid;
		const short *_upper = _keys + _klen - 1;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + ((_upper-_lower) >> 1);
			if ( _widec < *_mid )
				_upper = _mid - 1;
			else if ( _widec > *_mid )
				_lower = _mid + 1;
			else {
				_trans += (_mid - _keys);
				goto _match;
			}
		}
		_keys += _klen;
		_trans += _klen;
	}

	_klen = _chunked_range_lengths[cs];
	if ( _klen > 0 ) {
		const short *_lower = _keys;
		const short *_mid;
		const short *_upper = _keys + (_klen<<1) - 2;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + (((_upper-_lower) >> 1) & ~1);
			if ( _widec < _mid[0] )
				_upper = _mid - 2;
			else if ( _widec > _mid[1] )
				_lower = _mid + 2;
			else {
				_trans += ((_mid - _keys)>>1);
				goto _match;
			}
		}
		_trans += _klen;
	}

_match:
	_trans = _chunked_indicies[_trans];
	cs = _chunked_trans_targs[_trans];

	if ( _chunked_trans_actions[_trans] == 0 )
		goto _again;

	_acts = _chunked_actions + _chunked_trans_actions[_trans];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 )
	{
		switch ( *_acts++ )
		{
	case 0:
#line 37 "qcrawler_http_parse.rl"
	{
            if ((*p) >= 'A' && (*p) <= 'F') {
                chunk_data_size = chunk_data_size * 16 + ((*p) - 'A' + 10);
            } else if ((*p) >= 'a' && (*p) <= 'f') {
                chunk_data_size = chunk_data_size * 16 + ((*p) - 'a' + 10);
            } else {
                chunk_data_size = chunk_data_size * 16 + ((*p) - '0');
            }
            #ifdef TEST_PARSE
            fprintf(stdout, "read_chunk_size: %c %d\n", (*p), chunk_data_size);
            #endif
        }
	break;
	case 1:
#line 56 "qcrawler_http_parse.rl"
	{
            #ifdef TEST_PARSE
            fprintf(stdout, "entering_chunk_size\n");
            #endif
            chunk_data_count = 0;
            chunk_data_size = 0;
        }
	break;
	case 2:
#line 64 "qcrawler_http_parse.rl"
	{
            parsed_body.append(1, (*p));
            #ifdef TEST_PARSE
            fprintf(stdout, "read_chunk_data: %d, %d, %c\n", chunk_data_count, chunk_data_size, (*p));
            #endif
        }
	break;
	case 3:
#line 71 "qcrawler_http_parse.rl"
	{
            if (chunk_data_count < chunk_data_size) {
                {cs = 12; goto _again;}
            }
        }
	break;
#line 485 "qcrawler_http_parse.cpp"
		}
	}

_again:
	if ( cs == 0 )
		goto _out;
	if ( ++p != pe )
		goto _resume;
	_test_eof: {}
	_out: {}
	}

#line 97 "qcrawler_http_parse.rl"


    //fprintf(stderr, "----------------\nchunk_data_size: %d\n", chunk_data_size);
	if (cs < chunked_first_final) {
        return false;
    }

    return true;
}


#line 510 "qcrawler_http_parse.cpp"
static const char _parse_actions[] = {
	0, 1, 0, 1, 1, 1, 2, 1, 
	3, 1, 4, 1, 5
};

static const char _parse_key_offsets[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 
	7, 9, 10, 13, 16, 19, 22, 23, 
	28, 33, 36, 38, 40, 41, 41
};

static const char _parse_trans_keys[] = {
	72, 84, 84, 80, 47, 49, 46, 48, 
	57, 32, 32, 48, 57, 32, 48, 57, 
	10, 13, 94, 10, 13, 94, 10, 13, 
	32, 58, 9, 10, 13, 32, 58, 9, 
	10, 10, 13, 32, 10, 13, 10, 13, 
	10, 0
};

static const char _parse_single_lengths[] = {
	0, 1, 1, 1, 1, 1, 1, 1, 
	0, 1, 1, 1, 3, 3, 1, 3, 
	3, 3, 2, 2, 1, 0, 0
};

static const char _parse_range_lengths[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 
	1, 0, 1, 1, 0, 0, 0, 1, 
	1, 0, 0, 0, 0, 0, 0
};

static const char _parse_index_offsets[] = {
	0, 0, 2, 4, 6, 8, 10, 12, 
	14, 16, 18, 21, 24, 28, 32, 34, 
	39, 44, 48, 51, 54, 56, 57
};

static const char _parse_indicies[] = {
	0, 1, 2, 1, 3, 1, 4, 1, 
	5, 1, 6, 1, 7, 1, 8, 1, 
	9, 1, 9, 10, 1, 11, 10, 1, 
	1, 1, 1, 12, 1, 13, 1, 12, 
	14, 1, 16, 1, 1, 1, 15, 1, 
	1, 17, 1, 15, 1, 19, 17, 18, 
	1, 1, 20, 1, 19, 20, 21, 1, 
	22, 23, 0
};

static const char _parse_trans_targs[] = {
	2, 0, 3, 4, 5, 6, 7, 8, 
	9, 10, 11, 12, 13, 14, 15, 16, 
	20, 17, 18, 14, 19, 21, 22, 22
};

static const char _parse_trans_actions[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 1, 0, 0, 0, 0, 3, 
	0, 0, 5, 7, 5, 0, 9, 0
};

static const char _parse_eof_actions[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 11
};

static const int parse_start = 1;
static const int parse_first_final = 21;
static const int parse_error = 0;

static const int parse_en_main = 1;


#line 110 "qcrawler_http_parse.rl"


bool http_parse(const std::string &raw_content, int &status, std::map<std::string, std::string> &headers, std::string &body)
{
	const char *p = raw_content.c_str();
    const char *pe = p + raw_content.size();
    const char *eof = pe;

    int cs;

    status = 0;
    int content_length = 0;
    std::string head_name;
    std::string head_content;
    head_name.reserve(64);
    head_content.reserve(64);
    headers.clear();
    body.clear();

    const char *body_start = NULL, *body_end = NULL;

    
#line 608 "qcrawler_http_parse.cpp"
	{
	cs = parse_start;
	}

#line 613 "qcrawler_http_parse.cpp"
	{
	int _klen;
	unsigned int _trans;
	const char *_acts;
	unsigned int _nacts;
	const char *_keys;

	if ( p == pe )
		goto _test_eof;
	if ( cs == 0 )
		goto _out;
_resume:
	_keys = _parse_trans_keys + _parse_key_offsets[cs];
	_trans = _parse_index_offsets[cs];

	_klen = _parse_single_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + _klen - 1;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + ((_upper-_lower) >> 1);
			if ( (*p) < *_mid )
				_upper = _mid - 1;
			else if ( (*p) > *_mid )
				_lower = _mid + 1;
			else {
				_trans += (_mid - _keys);
				goto _match;
			}
		}
		_keys += _klen;
		_trans += _klen;
	}

	_klen = _parse_range_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + (_klen<<1) - 2;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + (((_upper-_lower) >> 1) & ~1);
			if ( (*p) < _mid[0] )
				_upper = _mid - 2;
			else if ( (*p) > _mid[1] )
				_lower = _mid + 2;
			else {
				_trans += ((_mid - _keys)>>1);
				goto _match;
			}
		}
		_trans += _klen;
	}

_match:
	_trans = _parse_indicies[_trans];
	cs = _parse_trans_targs[_trans];

	if ( _parse_trans_actions[_trans] == 0 )
		goto _again;

	_acts = _parse_actions + _parse_trans_actions[_trans];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 )
	{
		switch ( *_acts++ )
		{
	case 0:
#line 136 "qcrawler_http_parse.rl"
	{
			status = status * 10 + ((*p) - '0');
        }
	break;
	case 1:
#line 140 "qcrawler_http_parse.rl"
	{
            #ifdef TEST_PARSE
            printf("read_header_name: %c\n", (*p));
            #endif
            head_name.append(1, tolower((*p)));
        }
	break;
	case 2:
#line 147 "qcrawler_http_parse.rl"
	{
            head_content.append(1, (*p));
        }
	break;
	case 3:
#line 151 "qcrawler_http_parse.rl"
	{
            #ifdef TEST_PARSE
            printf("leaving header_name: %c\n", (*p));
            #endif
            if (!head_name.empty()) {
                headers[head_name] = head_content;
            }
            head_name.clear();
            head_content.clear();
        }
	break;
	case 4:
#line 162 "qcrawler_http_parse.rl"
	{
            body_start = p;
        }
	break;
#line 727 "qcrawler_http_parse.cpp"
		}
	}

_again:
	if ( cs == 0 )
		goto _out;
	if ( ++p != pe )
		goto _resume;
	_test_eof: {}
	if ( p == eof )
	{
	const char *__acts = _parse_actions + _parse_eof_actions[cs];
	unsigned int __nacts = (unsigned int) *__acts++;
	while ( __nacts-- > 0 ) {
		switch ( *__acts++ ) {
	case 5:
#line 166 "qcrawler_http_parse.rl"
	{
            body_end = p;
        }
	break;
#line 749 "qcrawler_http_parse.cpp"
		}
	}
	}

	_out: {}
	}

#line 181 "qcrawler_http_parse.rl"


	if (cs < parse_first_final) {
        return false;
    }

    if (body_end && body_start) {
        if (headers.find("content-length") != headers.end()) {
            content_length = atoi(headers["content-length"].c_str());
            if (abs(content_length - (body_end - body_start)) > 10) {
                LOG(WARNING) << "content length mis match ";
                return false;
            }
        }


#ifdef TEST_PARSE
        for (std::map<std::string, std::string>::iterator it = headers.begin();
                it != headers.end(); it++) {
            LOG(INFO) << it->first << ": " << it->second;
        }
        LOG(INFO) <<"body_start: " << body_start - raw_content.c_str();
        LOG(INFO) <<"body_end: " << body_end - raw_content.c_str();
        LOG(INFO) <<"raw_content_size: " << raw_content.size();
        LOG(INFO) <<"body_len: " << body_end - body_start;
#endif

        std::string body_for_chunked;
        if (headers.find("transfer-encoding") != headers.end() \
                && headers["transfer-encoding"] == "chunked") {
            body_for_chunked.reserve(body_end - body_start);
            // we all think the chunk is right
            bool chunk_ret = http_chunked_parse(body_start, body_end - body_start, body_for_chunked);
            body_start = body_for_chunked.c_str();
            body_end = body_for_chunked.c_str() + body_for_chunked.size();
            if (!chunk_ret) {
                LOG(WARNING) << "decode chunked data fail, but we ignored";
            }
        }

        if (headers.find("content-encoding") != headers.end() \
                && (headers["content-encoding"] == "gzip")) {

            uLong buf_len = QCRAWLER_DECOMPRESS_BUF_SIZE;
            static char buf[QCRAWLER_DECOMPRESS_BUF_SIZE];

            //int r = uncompress((Bytef*)buf, (uLongf*)&buf_len, (const Bytef*)body_start, body_end - body_start);
            int r = httpgzdecompress((Byte *)body_start, body_end - body_start, (Byte *)buf, (uLong *)&buf_len);
            if (r == 0) {
                body.append(buf, buf_len);
            } else {
                LOG(WARNING) << "gzip uncompress fail " << r << " len: " << body_end - body_start;
                return false;
            }
        } else {
            body.append(body_start, body_end - body_start);
        }
    }

    return true;
}

/* HTTP gzip decompress */
static int httpgzdecompress(Byte *zdata, uLong nzdata, Byte *data, uLong *ndata)
{
    int err = 0;
    z_stream d_stream;
    // = {0}; /* decompression stream */
    static char dummy_head[2] = 
    {
        0x8 + 0x7 * 0x10,
        (((0x8 + 0x7 * 0x10) * 0x100 + 30) / 31 * 31) & 0xFF,
    };
    d_stream.zalloc = (alloc_func)0;
    d_stream.zfree = (free_func)0;
    d_stream.opaque = (voidpf)0;
    d_stream.next_in  = zdata;
    d_stream.avail_in = 0;
    d_stream.next_out = data;
    if(inflateInit2(&d_stream, 47) != Z_OK) return -1;
    while (d_stream.total_out < *ndata && d_stream.total_in < nzdata) {
        d_stream.avail_in = d_stream.avail_out = 1; /* force small buffers */
        if((err = inflate(&d_stream, Z_NO_FLUSH)) == Z_STREAM_END) break;
        if(err != Z_OK )
        {
            if(err == Z_DATA_ERROR)
            {
                d_stream.next_in = (Bytef*) dummy_head;
                d_stream.avail_in = sizeof(dummy_head);
                if((err = inflate(&d_stream, Z_NO_FLUSH)) != Z_OK) 
                {
                    return -1;
                }
            }
            else return -1;
        }
    }
    if(inflateEnd(&d_stream) != Z_OK) return -1;
    *ndata = d_stream.total_out;
    return 0;
}


}
