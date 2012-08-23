#include  "qcontenthub_rpc.h"

#include <time.h>
#include <sys/time.h>
#include <cassert>

#define ENTER_FUNCTION \
    std::cout << "enter " << __FUNCTION__ << std::endl;

#define QUIT_FUNCTION \
    std::cout << "quit " << __FUNCTION__ << std::endl;

int QContentHubServer::add_queue(const std::string &name, int capacity)
{
	mp::sync<queue_map_t>::ref ref(q_map);
    queue_map_it_t it = ref->find(name);
    if (it == ref->end()) {
        queue_t * q = new queue_t();
        if (q == NULL) {
            return QCONTENTHUB_ERROR;
        }
        pthread_mutex_init(&q->lock,NULL);
        pthread_cond_init(&q->not_empty,NULL);
        pthread_cond_init(&q->not_full,NULL);

        q->stop = 0;
        q->capacity = capacity;
        (*ref)[name] = q;
        return QCONTENTHUB_OK;
    } else {
        return QCONTENTHUB_WARN;
    }
}

void QContentHubServer::add_queue(msgpack::rpc::request &req, const std::string &name, int capacity)
{
    req.result(add_queue(name, capacity));
}

/*
void QContentHubServer::del_queue(msgpack::rpc::request &req, const std::string &name)
{
    int ret;
    {
        mp::sync<queue_map_t>::ref ref(q_map);
        queue_map_it_t it = ref->find(name);
        if (it == ref->end()) {
            ret = QCONTENTHUB_WARN;
        } else if (it->second->str_q.size() == 0) {
            delete it->second;
            ref->erase(it);
            ret = QCONTENTHUB_OK;
        } else {
            ret = QCONTENTHUB_WARN;
        }
    }

    req.result(ret);
}

void QContentHubServer::force_del_queue(msgpack::rpc::request &req, const std::string &name)
{
    int ret;
    {
        mp::sync<queue_map_t>::ref ref(q_map);
        queue_map_it_t it = ref->find(name);
        if (it == ref->end()) {
            ret = QCONTENTHUB_WARN;
        } else {
            delete it->second;
            ref->erase(it);
            ret = QCONTENTHUB_OK;
        }
    }
    req.result(ret);
}
*/

void QContentHubServer::set_queue_capacity(msgpack::rpc::request &req, const std::string &name, int capacity)
{
    queue_map_t &qmap = q_map.unsafe_ref();
    queue_map_it_t it = qmap.find(name);
    if (it == qmap.end()) {
        req.result(QCONTENTHUB_WARN);
    } else {
        queue_t *q = it->second;
        pthread_mutex_lock(&q->lock);
        q->capacity = capacity;
        pthread_mutex_unlock(&q->lock);
    }
    req.result(QCONTENTHUB_OK);
}


void QContentHubServer::start_queue(msgpack::rpc::request &req, const std::string &name)
{
    queue_map_t &qmap = q_map.unsafe_ref();
    queue_map_it_t it = qmap.find(name);
    if (it == qmap.end()) {
        req.result(QCONTENTHUB_WARN);
    } else {
        queue_t *q = it->second;
        pthread_mutex_lock(&q->lock);
        q->stop = 0;
        pthread_mutex_unlock(&q->lock);
    }
}

void QContentHubServer::stop_queue(msgpack::rpc::request &req, const std::string &name)
{
    queue_map_t &qmap = q_map.unsafe_ref();
    queue_map_it_t it = qmap.find(name);
    if (it == qmap.end()) {
        req.result(QCONTENTHUB_WARN);
    } else {
        queue_t *q = it->second;
        pthread_mutex_lock(&q->lock);
        q->stop = 1;
        pthread_mutex_unlock(&q->lock);
    }
}

void QContentHubServer::clear_queue(msgpack::rpc::request &req, const std::string &name)
{
    queue_map_t &qmap = q_map.unsafe_ref();
    queue_map_it_t it = qmap.find(name);
    if (it == qmap.end()) {
        req.result(QCONTENTHUB_WARN);
    } else {
        queue_t *q = it->second;
        pthread_mutex_lock(&q->lock);
        while (!q->str_q.empty()) {
            q->str_q.pop();
        }
        pthread_mutex_unlock(&q->lock);
    }
}

void QContentHubServer::push_queue(msgpack::rpc::request &req, const std::string &name, const std::string &obj)
{
    queue_map_t &qmap = q_map.unsafe_ref();

    queue_map_it_t it = qmap.find(name);
    if (it == qmap.end()) {
        int ret = add_queue(name, DEFAULT_QUEUE_CAPACITY);
        if (ret == QCONTENTHUB_ERROR) {
            req.result(ret);
        } else {
            push_queue(req, name, obj);
        }
    } else {
        queue_t *q = it->second;
        pthread_mutex_lock(&q->lock);

        while ((int)q->str_q.size() > q->capacity) {
            struct timespec ts;
            clock_gettime(CLOCK_REALTIME, &ts);
            ts.tv_sec += 60;
            int rc = pthread_cond_timedwait(&q->not_full, &q->lock, &ts);
            if (rc == ETIMEDOUT) {
                pthread_mutex_unlock(&q->lock);
                req.result(QCONTENTHUB_AGAIN);
                return;
            } else if (rc != 0) {
                pthread_mutex_unlock(&q->lock);
                req.result(QCONTENTHUB_ERROR);
                return;
            }
        }
        q->str_q.push(obj);
        q->enqueue_items++;
        pthread_cond_signal(&q->not_empty);
        pthread_mutex_unlock(&q->lock);
        req.result(QCONTENTHUB_OK);
    }
}

void QContentHubServer::push_queue_nowait(msgpack::rpc::request &req, const std::string &name, const std::string &obj)
{
    queue_map_t &qmap = q_map.unsafe_ref();

    queue_map_it_t it = qmap.find(name);
    if (it == qmap.end()) {
        int ret = add_queue(name, DEFAULT_QUEUE_CAPACITY);
        if (ret == QCONTENTHUB_ERROR) {
            req.result(ret);
        } else {
            push_queue(req, name, obj);
        }
    } else {
        queue_t *q = it->second;
        pthread_mutex_lock(&q->lock);
        if ((int)q->str_q.size() > q->capacity) {
            pthread_mutex_unlock(&q->lock);
            req.result(QCONTENTHUB_AGAIN);
        } else {
            q->str_q.push(obj);
            q->enqueue_items++;
            pthread_cond_signal(&q->not_empty);
            pthread_mutex_unlock(&q->lock);
            req.result(QCONTENTHUB_OK);
        }
    }
}


void QContentHubServer::pop_queue(msgpack::rpc::request &req, const std::string &name)
{
    queue_map_t &qmap = q_map.unsafe_ref();
    queue_map_it_t it = qmap.find(name);
    if (it == qmap.end()) {
        req.result(QCONTENTHUB_STRAGAIN);
    } else {
        queue_t *q = it->second;
        if (q->stop) {
            req.result(QCONTENTHUB_STRAGAIN);
            return;
        }

        pthread_mutex_lock(&(q->lock));
        while (q->str_q.size() == 0) {
            struct timespec ts;
            clock_gettime(CLOCK_REALTIME, &ts);
            ts.tv_sec += 60;
            int rc = pthread_cond_timedwait(&q->not_empty, &q->lock, &ts);
            if (rc == ETIMEDOUT) {
                pthread_mutex_unlock(&(q->lock));
                req.result(QCONTENTHUB_STRAGAIN);
                return;
            } else if (rc != 0) {
                pthread_mutex_unlock(&(q->lock));
                req.result(QCONTENTHUB_STRERROR);
                return;
            }
        }
        //assert(q->str_q.size() > 0);
        std::string content = q->str_q.front();
        q->str_q.pop();
        pthread_cond_signal(&q->not_full);
        pthread_mutex_unlock(&(q->lock));
        req.result(content);
    }
}

void QContentHubServer::pop_queue_nowait(msgpack::rpc::request &req, const std::string &name)
{
    std::string ret;
    queue_map_t &qmap = q_map.unsafe_ref();
    queue_map_it_t it = qmap.find(name);
    if (it == qmap.end()) {
        req.result(QCONTENTHUB_STRERROR);
    } else {
        queue_t *q = it->second;
        if (q->stop) {
            req.result(QCONTENTHUB_STRAGAIN);
            return;
        }
        pthread_mutex_lock(&(q->lock));
        if (q->str_q.size() == 0) {
            ret = QCONTENTHUB_STRAGAIN;
        } else {
            ret = q->str_q.front();
            q->str_q.pop();
            pthread_cond_signal(&q->not_full);
        }
        pthread_mutex_unlock(&(q->lock));
        req.result(ret);
    }
}

void QContentHubServer::stats(msgpack::rpc::request &req)
{
    char buf[64];
    std::string ret;
    int current = get_current_time();
    ret.append("STAT uptime ");
    sprintf(buf, "%d", current - m_start_time);
    ret.append(buf);
    ret.append("\n");
    ret.append("STAT time ");
    sprintf(buf, "%d", current);
    ret.append(buf);
    ret.append("\n");

    queue_map_t &qmap = q_map.unsafe_ref();
    for (queue_map_it_t it = qmap.begin(); it != qmap.end(); it++) {
        ret.append("STAT name ");
        ret.append(it->first);
        ret.append("\nSTAT enqueue_items ");
        sprintf(buf, "%ld", it->second->enqueue_items);
        ret.append(buf);
        ret.append("\nSTAT size ");
        sprintf(buf, "%ld", it->second->str_q.size());
        ret.append(buf);
        ret.append("\n");
    }
    req.result(ret);
}

void QContentHubServer::stat_queue(msgpack::rpc::request &req, const std::string &name)
{
    char buf[64];
    std::string ret;
    int current = get_current_time();
    ret.append("STAT uptime ");
    sprintf(buf, "%d", current - m_start_time);
    ret.append(buf);
    ret.append("\n");
    ret.append("STAT time ");
    sprintf(buf, "%d", current);
    ret.append(buf);
    ret.append("\n");

    queue_map_t &qmap = q_map.unsafe_ref();
    queue_map_it_t it = qmap.find(name);
    if (it == qmap.end()) {
        req.result(ret);
    } else {
        ret.append("STAT name ");
        ret.append(name);
        ret.append("\nSTAT enqueue_items ");
        sprintf(buf, "%ld", it->second->enqueue_items);
        ret.append(buf);
        ret.append("\nSTAT size ");
        sprintf(buf, "%ld", it->second->str_q.size());
        ret.append(buf);
        ret.append("\n");
        req.result(ret);
    }
}

void QContentHubServer::dispatch(msgpack::rpc::request req) {
    try {
        std::string method;
        req.method().convert(&method);

        if(method == "push") {
            msgpack::type::tuple<std::string, std::string> params;
            req.params().convert(&params);
            push_queue(req, params.get<0>(), params.get<1>());
        } else if(method == "pop") {
            msgpack::type::tuple<std::string> params;
            req.params().convert(&params);
            pop_queue(req, params.get<0>());
        } else if(method == "push_nowait") {
            msgpack::type::tuple<std::string, std::string> params;
            req.params().convert(&params);
            push_queue_nowait(req, params.get<0>(), params.get<1>());
        } else if(method == "pop_nowait") {
            msgpack::type::tuple<std::string> params;
            req.params().convert(&params);
            pop_queue_nowait(req, params.get<0>());
        } else if(method == "add") {
            msgpack::type::tuple<std::string, int> params;
            req.params().convert(&params);
            add_queue(req, params.get<0>(), params.get<1>());
        /*
        } else if(method == "del") {
            msgpack::type::tuple<std::string> params;
            req.params().convert(&params);
            del_queue(req, params.get<0>());
        } else if(method == "fdel") {
            msgpack::type::tuple<std::string> params;
            req.params().convert(&params);
            force_del_queue(req, params.get<0>());
        */
        } else if(method == "set_capacity") {
            msgpack::type::tuple<std::string, int> params;
            req.params().convert(&params);
            set_queue_capacity(req, params.get<0>(), params.get<1>());
        } else if(method == "start") {
            msgpack::type::tuple<std::string> params;
            req.params().convert(&params);
            start_queue(req, params.get<0>());
        } else if(method == "stop") {
            msgpack::type::tuple<std::string> params;
            req.params().convert(&params);
            stop_queue(req, params.get<0>());
        } else if(method == "clear") {
            msgpack::type::tuple<std::string> params;
            req.params().convert(&params);
            clear_queue(req, params.get<0>());
        } else if(method == "stats") {
            stats(req);
        } else if(method == "stat_queue") {
            msgpack::type::tuple<std::string> params;
            req.params().convert(&params);
            stat_queue(req, params.get<0>());
        } else {
            req.error(msgpack::rpc::NO_METHOD_ERROR);
        }
    } catch (msgpack::type_error& e) {
        req.error(msgpack::rpc::ARGUMENT_ERROR);
        return;

    } catch (std::exception& e) {
        req.error(std::string(e.what()));
        return;
    }
}

void QContentHubServer::listen(uint16_t port)
{
    this->instance.listen("0.0.0.0", port);
}

void QContentHubServer::start(int multiple)
{
    m_start_time = get_current_time();
    this->instance.run(multiple);
}

int QContentHubServer::get_current_time()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec;
}

