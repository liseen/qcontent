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

%%{
    machine chunked;
    write data;
}%%

bool http_chunked_parse(const char *message_body, size_t message_body_size, std::string &parsed_body)
{
	const char *p = message_body;
    const char *pe = p + message_body_size;
    //const char *eof = pe;

    int cs;
    int chunk_data_count = 0;
    int chunk_data_size = 0;

    %%{
        action test_chunk_size {
            chunk_data_count++ < chunk_data_size
        }

        action read_chunk_size {
            if (fc >= 'A' && fc <= 'F') {
                chunk_data_size = chunk_data_size * 16 + (fc - 'A' + 10);
            } else if (fc >= 'a' && fc <= 'f') {
                chunk_data_size = chunk_data_size * 16 + (fc - 'a' + 10);
            } else {
                chunk_data_size = chunk_data_size * 16 + (fc - '0');
            }
            #ifdef TEST_PARSE
            fprintf(stdout, "read_chunk_size: %c %d\n", fc, chunk_data_size);
            #endif
        }

        action leaving_chunk_size {
            if (chunk_data_size == 0) {
                fhold; fgoto last_chunk;
            }
        }

        action entering_chunk_size {
            #ifdef TEST_PARSE
            fprintf(stdout, "entering_chunk_size\n");
            #endif
            chunk_data_count = 0;
            chunk_data_size = 0;
        }

        action read_chunk_data {
            parsed_body.append(1, fc);
            #ifdef TEST_PARSE
            fprintf(stdout, "read_chunk_data: %d, %d, %c\n", chunk_data_count, chunk_data_size, fc);
            #endif
        }

        action leaving_chunk_data {
            if (chunk_data_count < chunk_data_size) {
                fgoto chunk_data;
            }
        }

        CRLF = '\r\n';
        token = alpha alnum*;
        quoted_string = '"' [^\"\r\n]* '"';
        chunk_size     = (xdigit @read_chunk_size)+;
        chunk_ext_name = token;
        chunk_ext_val  = token | quoted_string;
        chunk_extension= (';' chunk_ext_name '=' chunk_ext_val)*;
        last_chunk     = '0'+ chunk_extension CRLF;
        entity_header = [^\r\n]+;
        trailer        = (entity_header CRLF)*;
        chunk_data     = (any when test_chunk_size @read_chunk_data)*;
        chunk          = (chunk_size >entering_chunk_size) chunk_extension CRLF (chunk_data %leaving_chunk_data) CRLF;
        chunked_body   = (chunk)* last_chunk trailer CRLF;
        #chunked_body   = (chunk)* last_chunk CRLF;

        main := chunked_body;

		# Initialize and execute.
		write init;
		write exec;
	}%%

    //fprintf(stderr, "----------------\nchunk_data_size: %d\n", chunk_data_size);
	if (cs < chunked_first_final) {
        return false;
    }

    return true;
}

%%{
	machine parse;
	write data;
}%%

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

    %%{
        action read_test {
            printf("read_test: %c\n", fc);
        }

        action read_status {
			status = status * 10 + (fc - '0');
        }

        action read_header_name {
            #ifdef TEST_PARSE
            printf("read_header_name: %c\n", fc);
            #endif
            head_name.append(1, tolower(fc));
        }

        action read_header_content {
            head_content.append(1, fc);
        }

        action leaving_header {
            #ifdef TEST_PARSE
            printf("leaving header_name: %c\n", fc);
            #endif
            if (!head_name.empty()) {
                headers[head_name] = head_content;
            }
            head_name.clear();
            head_content.clear();
        }

        action entering_body {
            body_start = fpc;
        }

        action leaving_body {
            body_end = fpc;
        }

        crlf = '\r\n';
        http_version = 'HTTP/1.' digit;
        start_line = http_version ' '+ (digit @read_status)+ ' '+ ([^\r^\n])+;
        header_content = ([^ \r\n] @read_header_content) ([^\r\n] @read_header_content)+;
        message_header = ([^: \t\n\r] @read_header_name)+ ':' ' '* header_content?;
        message_body = any+;
        main := start_line crlf ((message_header %leaving_header) crlf)* crlf (message_body >entering_body %leaving_body)?;

		# Initialize and execute.
		write init;
		write exec;
	}%%

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
