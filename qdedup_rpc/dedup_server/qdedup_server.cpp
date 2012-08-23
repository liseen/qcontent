/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 *    zhenbao.zhou (周桢堡)  <zhenbao.zhou@qunar.com>
 *
 * dedup server
 */

#include <vector>
#include <string>

#include <glog/logging.h>

#include "qdedup_server.h"
#include "qdedup_common.h"

namespace qcontent {

int QDedupServer::is_dedup(const std::vector<str_finger_t> &finger_vector) {
    return qdedup_backend->dedup_check(finger_vector);
}

const mmap_stat_t & QDedupServer::check_usage() {
    return qdedup_backend->check_usage();
}

void QDedupServer::dispatch(msgpack::rpc::request req) {
    try {
        std::string method;
        req.method().convert(&method);

        if (method == "dedup") {
            msgpack::type::tuple< std::vector<str_finger_t> > params;
            req.params().convert(&params);
            int re = is_dedup(params.get<0>());
            req.result(re);
        } else if (method == "check_usage") {
            DLOG(INFO) << "get a check_usage";
            const mmap_stat_t &mmap_stat = check_usage();
            req.result(mmap_stat);
        } else if (method == "stop") {
            DLOG(INFO) << "get a stop info";
            this->stop(); // stop backend ,and exist(0);
            req.result(0);
            // exit
            exit(0);
        } else {
            LOG(ERROR) << "get a UNKNOWN METHOD";
            req.error(msgpack::rpc::NO_METHOD_ERROR);
        }
    } catch(const msgpack::type_error& e) {
        req.error(msgpack::rpc::ARGUMENT_ERROR);
        return;
    } catch(const std::exception& e) {
        req.error(std::string(e.what()));
        return;
    }
}

void QDedupServer::start(int multiple) {
    this->instance.run(multiple);
}

int QDedupServer::init_server(const std::string &mmap_file, size_t mmap_size, size_t sync_threshold) {
    qdedup_backend = new QDedupBackend(mmap_file, mmap_size, sync_threshold);
    if (qdedup_backend->init_backend() != 0) {
        LOG(ERROR) << "errror in init server: mmap_file:"
                  << mmap_size << " size:"
                  << mmap_size ;
        return 1;
    }
    return 0;
}

void QDedupServer::stop() {
    LOG(INFO) << "stop qdedup server";
    qdedup_backend->stop();
    delete qdedup_backend;
    qdedup_backend = NULL;
}

}  // end of namespace qcontent{}

