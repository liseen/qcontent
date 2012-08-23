/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

#include "qurlfilter.h"
#include <cassert>

#include <strtk.hpp>
#include <qcontent_config.h>
#include <googleurl/src/gurl.h>

#include "qhost2site/qhost2site.h"

namespace qcontent
{

QContainRule::QContainRule(const std::string &rule_str)
{
    int len = rule_str.size();
    bool start_and = true;
    int start_pos = 0;
    int i = 0;
    for (i = 0; i < len; ++i) {
        if (rule_str[i] == '\x01') {
            if (i - start_pos > 0) {
                if (start_and) {
                    crule_vec.push_back(CRule(true, rule_str.substr(start_pos, i - start_pos)));
                } else {
                    crule_vec.push_back(CRule(false, rule_str.substr(start_pos, i - start_pos)));
                }
            }

            start_and = true;
            start_pos = i + 1;
        } else if (rule_str[i] == '\x02') {
            if (i - start_pos > 0) {
                if (start_and) {
                    crule_vec.push_back(CRule(true, rule_str.substr(start_pos, i - start_pos)));
                } else {
                    crule_vec.push_back(CRule(false, rule_str.substr(start_pos, i - start_pos)));
                }
            }
            start_and = false;
            start_pos = i + 1;
        }
    }
    if (i - start_pos > 0) {
        if (start_and) {
            crule_vec.push_back(CRule(true, rule_str.substr(start_pos, i - start_pos)));
        } else {
            crule_vec.push_back(CRule(false, rule_str.substr(start_pos, i - start_pos)));
        }
    }
}

bool QContainRule::pass_rule(const std::string &str) const
{
    for (std::vector<CRule>::const_iterator it = crule_vec.begin(); it != crule_vec.end(); ++it) {
        bool contain = str.find(it->second) != std::string::npos;
        if (it->first && !contain) {
            return false;
        }
        if (!it->first && contain) {
            return false;
        }
    }

    return true;
}

QUrlFilter::QUrlFilter()
{
    has_global = false;
}

QUrlFilter::QUrlFilter(const std::string &conf_file)
{
    has_global = false;
    load_rule(conf_file);
}

void string_to_contain_rules(const std::string &contain_str, std::vector<QContainRule> &contain_rules)
{
    std::vector<std::string> contain_rule_strs;
    contain_rules.clear();
    strtk::parse(contain_str, " ", contain_rule_strs);
    int size = contain_rule_strs.size();
    for (int i = 0; i < size; i++) {
        contain_rules.push_back(QContainRule(contain_rule_strs[i]));
/*
        std::string inc;
        std::string ex;
        bool parse_ret = strtk::parse(contain_rule_strs[i], "\x01\x02", inc, ex);
        if (!parse_ret) {
            inc = contain_rule_strs[i];
        }
        contain_rules.push_back(QContainRule(inc, ex));
        */
    }
}

bool QUrlFilter::load_rule(const std::string &conf_file)
{
    qcontent::QContentConfig config;
    if (!config.parse_file(conf_file)) {
        LOG(FATAL) << "Parse config file " << conf_file << " error";
        return false;
    }

    const std::vector<std::string> & groups = config.get_groups();

    int size = groups.size();
    for (int i = 0; i < size; i++) {
        std::string site = groups[i];
        if (site == "global") {
            has_global = true;
        }

        bool skip = (bool)config.get_integer(site + ".skip", 0);
        std::string focus_hosts_str = config.get_string(site + ".focus_hosts", "");
        std::string skip_hosts_str = config.get_string(site + ".skip_hosts", "");
        std::string focus_hosts_contains_str =
            config.get_string(site + ".focus_hosts_contains", "");
        std::string focus_urls_contains_str =
            config.get_string(site + ".focus_urls_contains", "");
        std::string skip_hosts_contains_str =
            config.get_string(site + ".skip_hosts_contains", "");
        std::string skip_urls_contains_str =
            config.get_string(site + ".skip_urls_contains", "");

        site_filters[site] = QFilterRule();

        QFilterRule &rule = site_filters[site];

        rule.skip = skip;
        std::vector<std::string> focus_hosts_vec;
        strtk::parse(focus_hosts_str, " ", focus_hosts_vec);
        int len = focus_hosts_vec.size();
        for (int j = 0; j < len; j++) {
            const std::string &host = focus_hosts_vec[j];
            rule.focus_hosts.insert(host);
        }

        std::vector<std::string> skip_hosts_vec;
        strtk::parse(skip_hosts_str, " ", skip_hosts_vec);
        len = skip_hosts_vec.size();
        for (int j = 0; j < len; j++) {
            const std::string &host = skip_hosts_vec[j];
            rule.skip_hosts.insert(host);
        }

        string_to_contain_rules(focus_hosts_contains_str, rule.focus_hosts_contains);
        string_to_contain_rules(focus_urls_contains_str, rule.focus_urls_contains);
        string_to_contain_rules(skip_hosts_contains_str, rule.skip_hosts_contains);
        string_to_contain_rules(skip_urls_contains_str, rule.skip_urls_contains);
    }

    return true;
}

std::string QUrlFilter::filter(const std::string &site, const std::string &host, const std::string &url)
{
    if (has_global && site != "global") {
        std::string tmp_url = filter("global", host, url);
        if (tmp_url.empty()) {
            return "";
        }
    }

    //std::map<std::string, QFilterRule>::iterator it = site_filters.find(site);
    unordered_map<std::string, QFilterRule>::iterator it = site_filters.find(site);
    if (it == site_filters.end()) {
        return url;
    } else {
        const QFilterRule &rule = it->second;
        if (rule.skip) {
            return "";
        }

        if (!rule.focus_hosts.empty() && rule.focus_hosts.find(host) == rule.focus_hosts.end()) {
            return "";
        }

        if (!rule.focus_hosts_contains.empty()) {
            int size = rule.focus_hosts_contains.size();
            bool contain = false;
            for (int i = 0; i < size; ++i) {
                const QContainRule &contain_rule = rule.focus_hosts_contains[i];
                if (contain_rule.pass_rule(host)) {
                    contain = true;
                    break;
                }
            }
            if (!contain) {
                return "";
            }
        }

        if (!rule.focus_urls_contains.empty()) {
            int size = rule.focus_urls_contains.size();
            bool contain = false;
            for (int i = 0; i < size; ++i) {
                const QContainRule &contain_rule = rule.focus_urls_contains[i];
                if (contain_rule.pass_rule(url)) {
                    contain = true;
                    break;
                }
            }
            if (!contain) {
                return "";
            }
        }

        if (!rule.skip_hosts.empty() && rule.skip_hosts.find(host) != rule.skip_hosts.end()) {
            return "";
        }

        if (!rule.skip_hosts_contains.empty()) {
            int size = rule.skip_hosts_contains.size();
            for (int i = 0; i < size; ++i) {
                const QContainRule &contain_rule = rule.skip_hosts_contains[i];
                if (contain_rule.pass_rule(host)) {
                    return "";
                }
            }
        }

        if (!rule.skip_urls_contains.empty()) {
            int size = rule.skip_urls_contains.size();
            for (int i = 0; i < size; ++i) {
                const QContainRule &contain_rule = rule.skip_urls_contains[i];
                if (contain_rule.pass_rule(url)) {
                    return "";
                }
            }
        }

        return url;
    }
}

std::string QUrlFilter::filter(const std::string &host, const std::string &url)
{
    return filter(qhost2site(host), host, url);
}

std::string QUrlFilter::filter(const std::string &url)
{
    GURL gurl(url);
    if (!gurl.is_valid()) {
        return "";
    }

    const std::string &host = gurl.host();
    std::string site = qhost2site(host);

    return filter(site, host, url);

}

} // end namespace qcontent 
