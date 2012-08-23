#include <glog/logging.h>

#include "qcrawler_http_parse.h"

#include <iostream>
#include <zlib.h>
#include <cassert>
#define QCRAWLER_DECOMPRESS_BUF_SIZE 2*1024*1024

/* HTTP gzip decompress */
int httpgzdecompress(Byte *zdata, uLong nzdata, Byte *data, uLong *ndata)
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

int main()
{
    std::string response;
    std::string html;
    int status;
    std::map<std::string, std::string> headers;
    std::string line;

    while (std::getline(std::cin, line)) {
        response.append(line);
        response.append("\n");
    }
    if (!response.empty()) {
        response = response.substr(0, response.size() -1);
    }

    //LOG(INFO) << "start http_parse" << response;
    bool r = qcrawler::http_parse(response, status, headers, html);

    /*
    uLong buf_len = QCRAWLER_DECOMPRESS_BUF_SIZE;
    char buf[QCRAWLER_DECOMPRESS_BUF_SIZE];
    char *dest = buf;
    //int r = uncompress((Bytef*)buf, (uLongf*)&buf_len, (const Bytef*)response.c_str(), response.size());
    //int r = inflate_read(response.c_str(), response.size(), &dest, 1);
    int r = httpgzdecompress((Byte *)response.c_str(), uLong(response.size()), (Byte *)buf, (uLong *)&buf_len);
    */
    if (r) {
        LOG(INFO) << "z_ok" ;
        std::map<std::string, std::string>::iterator it  = headers.begin();
        while (it != headers.end()) {
            std::cout << "header, " << it->first << " : " << it->second << std::endl;
            it++;
        }
    } else {
        LOG(INFO) << "z_fail" << r;
    }

    return 0;
}
