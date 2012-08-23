/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

/*
 * qpredict_learner.h
 *
 */

#ifndef QPREDICT_LEARNER_H
#define QPREDICT_LEARNER_H

#include "qpredict_define.h"
#include "qpredict_config.h"

#include "qpredict_document.h"

namespace qcontent
{

class QPredictLearner
{
public:
    virtual bool init(QPredictConfig *config) = 0;
    virtual bool train(QPredictDocumentList &doc_list) = 0;
    virtual bool predict(QPredictDocument &doc) = 0;
    virtual bool save_model() = 0;
    virtual bool load_model() = 0;

    virtual int num_of_features() = 0;
};

} // end for qcontent

#endif
