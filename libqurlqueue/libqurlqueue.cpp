/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

#include "libqurlqueue.h"

namespace qcontent {

int UrlQueue::push_url(const std::string &site, const std::string &content)
{
    return push_url(0, site, content);
}

int UrlQueue::push_url_front(const std::string &site, const std::string &content)
{
    return push_url_front(10000, site, content);
}


int UrlQueue::push_url(int priority, const std::string &site, const std::string &content)
{
    m_error_str.clear();
    try {
        return m_client.call("push", priority, site, content).get<int>();
    } catch (std::exception& e) {
       m_error_str = std::string(e.what());
       return QCONTENTHUB_ERROR;
    }
}

int UrlQueue::push_url_front(int priority, const std::string &site, const std::string &content)
{
    m_error_str.clear();
    try {
        return m_client.call("push_url_front", priority, site, content).get<int>();
    } catch (std::exception& e) {
       m_error_str = std::string(e.what());
       return QCONTENTHUB_ERROR;
    }
}

int UrlQueue::pop_url(std::string &record)
{
    m_error_str.clear();
    try {
        record = m_client.call("pop").get<std::string>();
        if (record == QCONTENTHUB_STRERROR) {
            m_error_str = "qcontenthub server pop error";
            return QCONTENTHUB_ERROR;
        } else if (record == QCONTENTHUB_STRAGAIN) {
            return QCONTENTHUB_AGAIN;
        }
        return QCONTENTHUB_OK;
    } catch (std::exception& e) {
       m_error_str = std::string(e.what());
       return QCONTENTHUB_ERROR;
    }
}

int UrlQueue::start_dump_all()
{
    m_error_str.clear();
    try {
        return m_client.call("start_dump_all").get<int>();
    } catch (std::exception& e) {
       m_error_str = std::string(e.what());
       return QCONTENTHUB_ERROR;
    }
}

int UrlQueue::dump_all(std::string &record)
{
    m_error_str.clear();
    try {
        record = m_client.call("dump_all").get<std::string>();
        if (record == QCONTENTHUB_STRERROR) {
            m_error_str = "qcontenthub server dump all error";
            return QCONTENTHUB_ERROR;
        } else if (record == QCONTENTHUB_STRAGAIN) {
            return QCONTENTHUB_AGAIN;
        } else if (record == QCONTENTHUB_STREND) {
            return QCONTENTHUB_END;
        }

        return QCONTENTHUB_OK;
    } catch (std::exception& e) {
       m_error_str = std::string(e.what());
       return QCONTENTHUB_ERROR;
    }
}

int UrlQueue::set_default_interval(int interval)
{
    m_error_str.clear();
    try {
        return m_client.call("set_default_interval", interval).get<int>();
    } catch (std::exception& e) {
       m_error_str = std::string(e.what());
       return QCONTENTHUB_ERROR;
    }
}

int UrlQueue::set_site_interval(const std::string &site, int interval)
{
    m_error_str.clear();
    try {
        return m_client.call("set_site_interval", site, interval).get<int>();
    } catch (std::exception& e) {
       m_error_str = std::string(e.what());
       return QCONTENTHUB_ERROR;
    }
}

int UrlQueue::start_all()
{
    m_error_str.clear();
    try {
        return m_client.call("start_all").get<int>();
    } catch (std::exception& e) {
       m_error_str = std::string(e.what());
       return QCONTENTHUB_ERROR;
    }
}

int UrlQueue::stop_all()
{
    m_error_str.clear();
    try {
        return m_client.call("stop_all").get<int>();
    } catch (std::exception& e) {
       m_error_str = std::string(e.what());
       return QCONTENTHUB_ERROR;
    }
}

int UrlQueue::clear_empty_site()
{
    m_error_str.clear();
    try {
        return m_client.call("clear_empty_site").get<int>();
    } catch (std::exception& e) {
        m_error_str = std::string(e.what());
        return QCONTENTHUB_ERROR;
    }
}


int UrlQueue::stats(std::string &stats_content)
{
    m_error_str.clear();
    try {
        stats_content = m_client.call("stats").get<std::string>();
        return QCONTENTHUB_OK;
    } catch (std::exception& e) {
        m_error_str = std::string(e.what());
        return QCONTENTHUB_ERROR;
    }
}

int UrlQueue::dump_ordered_site(std::string &sites, int limit)
{
    m_error_str.clear();
    try {
        sites = m_client.call("dump_ordered_site", limit).get<std::string>();
        return QCONTENTHUB_OK;
    } catch (std::exception& e) {
        m_error_str = std::string(e.what());
        return QCONTENTHUB_ERROR;
    }
}

int UrlQueue::clear_site(const std::string &site)
{
    m_error_str.clear();
    try {
        return m_client.call("clear_site", site).get<int>();
    } catch (std::exception& e) {
        m_error_str = std::string(e.what());
        return QCONTENTHUB_ERROR;
    }
}

int UrlQueue::stat_site(const std::string &site, std::string &stats_content)
{
    m_error_str.clear();
    try {
        stats_content = m_client.call("stat_site", site).get<std::string>();
        return QCONTENTHUB_OK;
    } catch (std::exception& e) {
        m_error_str = std::string(e.what());
        return QCONTENTHUB_ERROR;
    }
}

int UrlQueue::start_dump_site(const std::string &site)
{
    m_error_str.clear();
    try {
        return m_client.call("start_dump_site", site).get<int>();
    } catch (std::exception& e) {
        m_error_str = std::string(e.what());
        return QCONTENTHUB_ERROR;
    }
}

int UrlQueue::start_site(const std::string &site)
{
    m_error_str.clear();
    try {
        return m_client.call("start_site", site).get<int>();
    } catch (std::exception& e) {
        m_error_str = std::string(e.what());
        return QCONTENTHUB_ERROR;
    }
}

int UrlQueue::stop_site(const std::string &site)
{
    m_error_str.clear();
    try {
        return m_client.call("stop_site", site).get<int>();
    } catch (std::exception& e) {
        m_error_str = std::string(e.what());
        return QCONTENTHUB_ERROR;
    }
}

int UrlQueue::dump_site(const std::string &site, std::string &content)
{
    m_error_str.clear();
    try {
        content = m_client.call("dump_site", site).get<std::string>();
        return QCONTENTHUB_OK;
    } catch (std::exception& e) {
        m_error_str = std::string(e.what());
        return QCONTENTHUB_ERROR;
    }
}


const std::string& UrlQueue::error_str()
{
    return m_error_str;
}

}
