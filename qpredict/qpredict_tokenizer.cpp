/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

#include "qpredict_tokenizer.h"

#include <pcre.h>

namespace qcontent
{

static pcre *stop_word_re = NULL;
static const char *default_stop_word_pat = "^(?:[\\d\\w\\s\\-_:&\\.\\$=\\/]+||的|了|你|我|他|，|。|（|）|：|”|作者|编辑|首页|“|发表|　)$";

QPredictTokenizer::QPredictTokenizer() : qws(NULL)
{
}

bool QPredictTokenizer::init(QPredictConfig *config)
{
    qws_config = new qcontent::QWSConfig(config->config_map());

    qws = new qcontent::QWS();

    if (!qws->init(qws_config)) {
        fprintf(stderr, "init qws for qpredict tokenizer error");
        return false;
    }

    std::string stop_word_regex;
    if (!config->get_string("qpredict.stop_word_regex", stop_word_regex)) {
        stop_word_regex.append(default_stop_word_pat);
    }

    if (stop_word_re == NULL) {
        const char *error;
        int erroffset;
        stop_word_re = pcre_compile(stop_word_regex.c_str(), 0, &error, &erroffset, NULL);
        if (stop_word_re == NULL) {
            fprintf(stderr, "pcre compilation %s failed at \
                    offset %d: %s when init qpredict tokenizer\n", \
                    stop_word_regex.c_str(), erroffset, error);
            return false;
        }
    }

    return true;
}

int QPredictTokenizer::terms_count(const std::string &text, TermHash &terms)
{
    terms.clear();

    qcontent::QWSTokenList tokens;
    qws->tokenize(text, tokens);
    int tokens_size = tokens.size();
    for (int i = 0; i < tokens_size; ++i) {
        const std::string &w = tokens[i].value;

        if (w.size() < 3) {
            continue;
        }

        // stop word...
        int rc = pcre_exec(stop_word_re, NULL, w.c_str(), w.size(), 0, 0, NULL, 0);

        if (rc < 0) {
            switch(rc) {
                case PCRE_ERROR_NOMATCH:
                    break;
                default:
                    fprintf(stderr, "stop word regex match error: %d\n", rc);
                    break;
            }
        } else {
            continue;
        }

        TermHashIter it = terms.find(w);
        if (it == terms.end()) {
            QPredictTerm term;
            term.tf = 1;
            terms[w] = term;
        } else {
            ++(it->second.tf);
        }
    }

    return terms.size();
}

}
