/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

/*
 * qpredict_linear_learner.h
 *
 */

#ifndef  QPREDICT_LINEAR_LEARNER_H
#define  QPREDICT_LINEAR_LEARNER_H

#include <string>
#include <iostream>

#include "linear.h"

#include "qpredict_config.h"

#include "qpredict_learner.h"

namespace qcontent
{

class QPredictLinearLearner : public QPredictLearner
{
public:
    QPredictLinearLearner();
    virtual ~QPredictLinearLearner();

    virtual bool init(QPredictConfig *config);
    virtual bool train(QPredictDocumentList &doc_list);
    virtual bool predict(QPredictDocument &doc);
    virtual bool save_model();
    virtual bool load_model();
    virtual int num_of_features();

protected:
    struct model *m_model;
	struct parameter m_param;

    std::string m_model_file;
    //int    m_fold;

    double m_bias;
};

} // end for namesapce qcontent

#endif
