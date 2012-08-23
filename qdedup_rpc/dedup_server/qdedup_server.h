/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 *    zhenbao.zhou (周桢堡)  <zhenbao.zhou@qunar.com>
 *
 * dedup server
 */

#ifndef QDEDUP_RPC_QDEDUP_SERVER_H_
#define QDEDUP_RPC_QDEDUP_SERVER_H_

#include <msgpack/rpc/loop.h>
#include <msgpack/rpc/server.h>
#include <string>
#include <vector>

#include "qdedup_backend.h"

namespace qcontent {

class QDedupServer : public msgpack::rpc::server::base {
  public:
    QDedupServer(msgpack::rpc::loop lo = msgpack::rpc::loop())
            :msgpack::rpc::server::base(lo) {
        qdedup_backend = NULL;
    };

    int init_server(const std::string &mmap_file, size_t size, size_t sync_threshold = 500000);

    ~QDedupServer() {}

    int is_dedup(const std::vector<str_finger_t> &finger_vector);

    // return the usage of this mmap
    const mmap_stat_t & check_usage();

    void dispatch(msgpack::rpc::request req);

    void start(int multiple);

    void stop();
  private:
    QDedupBackend *qdedup_backend;
};

}  // end of namespace qcontent{}
#endif  // QDEDUP_RPC_QDEDUP_SERVER_H_
