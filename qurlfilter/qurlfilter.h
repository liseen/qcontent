/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

#ifndef QURLFILTER_H
#define QURLFILTER_H

#include <string>
#include <glog/logging.h>
#include <qcontent_config.h>

#include <map>
#include <set>

#ifndef QCONTENT_TR1_UNORDERED
#define QCONTENT_TR1_UNORDERED

#include <tr1/functional>
#include <tr1/unordered_map>
#include <tr1/unordered_set>
namespace qcontent
{
	using std::tr1::unordered_map;
	using std::tr1::unordered_set;
	using std::tr1::unordered_multimap;
	using std::tr1::unordered_multiset;
	template <typename T> struct hash : public std::tr1::hash<T> { };
}

#endif

namespace qcontent
{

class QUrlFilter;

//typedef std::pair<std::string, std::string> QContainRule;
class QContainRule {
public:
    typedef std::pair<bool, std::string> CRule;
    QContainRule(const std::string &rule_str);
    bool pass_rule(const std::string &str) const;
private:
    std::vector<CRule> crule_vec;
};

class QFilterRule
{
public:
    friend class QUrlFilter;

private:
    int skip;
    std::set<std::string> focus_hosts;
    std::vector<QContainRule> focus_hosts_contains;
    std::vector<QContainRule> focus_urls_contains;
    //std::vector<> focus_urls
    std::set<std::string> skip_hosts;
    std::vector<QContainRule> skip_hosts_contains;
    std::vector<QContainRule> skip_urls_contains;
    //std::vector<>skip_urls
    //replace_urls
};

class QUrlFilter
{
public:
    QUrlFilter();
    QUrlFilter(const std::string &conf_file);

    bool load_rule(const std::string &conf_file);
    std::string filter(const std::string &site, const std::string &host, const std::string &url);
    std::string filter(const std::string &host, const std::string &url);
    std::string filter(const std::string &url);

private:
    bool has_global;
    //std::map<std::string, QFilterRule> site_filters;
    unordered_map<std::string, QFilterRule> site_filters;
};

} // end namespace qcontent

#endif
