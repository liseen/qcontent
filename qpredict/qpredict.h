/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

/*
 * qpredict.h qpredict class
 *
 */
#ifndef Q_QPREDICT_H
#define Q_QPREDICT_H

#include "qpredict_define.h"
#include "qpredict_config.h"

#include "qpredict_term.h"
#include "qpredict_dict.h"

#include "qpredict_document.h"
#include "qpredict_tokenizer.h"
#include "qpredict_learner.h"

namespace qcontent
{

class QPredict {

public:
    explicit QPredict();
    virtual ~QPredict();

    bool init(QPredictConfig *config);
    bool train(QPredictDocumentList &doc_list);
    bool valid(QPredictDocumentList &doc_list);
    bool predict(QPredictDocument &doc);
    bool load_qpredict_model();
    bool save_qpredict_model();

    unordered_map<std::string, uint32_t> class_label2id;
    std::vector<std::string> class_id2label;

private:
    int term_feature_size_limit;
    int term_feature_size;
    int lda_feature_size;

    QPredictConfig *config;

    QPredictDict term_feature_dict;
    std::string term_feature_dict_file;

    //std::vector<double> term_feature_idf_dict;
    //std::string term_feature_idf_dict_file;

    QPredictTokenizer *tokenizer;
    QPredictLearner *learner;
    //QPredictTokenizer *m_tokenizer;
};

} // end for namespace qcontent

#endif
