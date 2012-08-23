/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

#ifndef QCONTENT_QWS_CONFIG_H
#define QCONTENT_QWS_CONFIG_H

#include <string>
#include <map>

#include "qws_common.h"

BEGIN_NAMESPACE_QWS

typedef std::map<std::string, std::string> QWSConfigMap;
typedef std::map<std::string, std::string>::iterator QWSConfigMapIter;

class QWSConfig {
public:
    QWSConfig();
    QWSConfig(const QWSConfigMap &config_map);
    ~QWSConfig();

    bool get_integer(const std::string &option, int &value);
    bool get_double(const std::string &option, double &value);
    bool get_string(const std::string &option, std::string &value);
    bool get_boolean(const std::string &option, bool &value);

private:
    QWSConfigMap m_config_map;
};

END_NAMESPACE_QWS

#endif
