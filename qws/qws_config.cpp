/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

#include "qws_config.h"

#include <cstdio>
#include <cstdlib>

#include "qws_common.h"


BEGIN_NAMESPACE_QWS

QWSConfig::QWSConfig()
{
}

QWSConfig::QWSConfig(const QWSConfigMap &config_map) : m_config_map(config_map)
{
}

QWSConfig::~QWSConfig()
{
}

bool QWSConfig::get_integer(const std::string &option, int &value)
{
    if (m_config_map.find(option) != m_config_map.end()) {
        const std::string &val = m_config_map[option];
        value = atoi(val.c_str());
        return true;
    }

    return false;
}

bool QWSConfig::get_double(const std::string &option, double &value)
{
    if (m_config_map.find(option) != m_config_map.end()) {
        const std::string &val = m_config_map[option];
        value = atof(val.c_str());
        return true;
    }

    return false;
}

bool QWSConfig::get_string(const std::string &option, std::string &value)
{
    if (m_config_map.find(option) != m_config_map.end()) {
        const std::string &val = m_config_map[option];
        value = val;
        return true;
    }

    return false;

}

bool QWSConfig::get_boolean(const std::string &option, bool &value)
{
    if (m_config_map.find(option) != m_config_map.end()) {
        const std::string &val = m_config_map[option];

        if (val == "true") {
            value = true;
        } else if (val == "false") {
            value = false;
        } else if (atoi(val.c_str())) {
            value = true;
        } else {
            value = false;
        }

        return true;
    }

    return false;
}

END_NAMESPACE_QWS
