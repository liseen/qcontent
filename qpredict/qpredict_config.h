/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

/*
 *
 *
 */

#ifndef QPREDICT_CONFIG_H
#define QPREDICT_CONFIG_H

#include <string>
#include <map>

#include "qpredict_define.h"

namespace qcontent
{

typedef std::map<std::string, std::string> QPredictConfigMap;
typedef std::map<std::string, std::string>::iterator QPredictConfigMapIter;

class QPredictConfig
{
public:
    QPredictConfig() {
    }

    QPredictConfig(const QPredictConfigMap &cm) : m_config_map(cm) {
    }
/*
    QPredictConfig(const std::map<std::string, std::string> &m) {
        for (std::map<std::string, std::string>::const_iterator it = m.begin();
                it != m.end(); ++it) {
            m_config_map[it->first] = it->second;
        }
    }
*/

    ~QPredictConfig() {

    }

    bool get_integer(const std::string &option, int &value);
    bool get_double(const std::string &option, double &value);
    bool get_string(const std::string &option, std::string &value);
    bool get_boolean(const std::string &option, bool &value);

    const QPredictConfigMap & config_map() {
        return m_config_map;
    }

    QPredictConfigMap *mutable_config_map() {
        return &m_config_map;
    }

private:
    QPredictConfigMap m_config_map;
};

} // end for namespace qcontent

#endif
