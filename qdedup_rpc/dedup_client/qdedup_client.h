/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 *    zhenbao.zhou (周桢堡)  <zhenbao.zhou@qunar.com>
 *
 * Client for qdedup
 */


#ifndef QDEDUP_RPC_QDEDUP_CLIENT_H_
#define QDEDUP_RPC_QDEDUP_CLIENT_H_

#include <msgpack/rpc/client.h>
#include <stdint.h>
#include <string>
#include <vector>

namespace qcontent {

struct qdedup_addr {
    std::string host;
    int port;
};

class QDedupClient {
  public:
    QDedupClient(std::vector<qdedup_addr> addr_vec, int timeout = 1000);

    int init_client();

    void check_usage();

    int  dedup_check(const char *prefix, size_t prefix_size,
                     const char *content, size_t content_size);

    void halt_server();
    
    virtual ~QDedupClient();

    inline const std::vector<qdedup_addr>& get_qdedup_addr() const;

    inline int get_timeout();
    inline void set_timeout(int timeout);

  private:
    QDedupClient() {}

    int start_with_punct(const char *str);

    msgpack::rpc::client* get_client(uint64_t site_seed);

    void close_client();

    std::vector<msgpack::rpc::client *> client_vector;
    std::vector<qdedup_addr> qdedup_addr_vector;
    int timeout;  //  timeout for every rpc client
};

}  //  end of namespace std

#endif  // QDEDUP_RPC_QDEDUP_CLIENT_H_
