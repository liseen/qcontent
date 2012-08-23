/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

#include "qpredict_linear_learner.h"

#include <cstdio>
#include <cstdlib>
#include <algorithm>

namespace qcontent
{

QPredictLinearLearner::QPredictLinearLearner()
{
    m_model = NULL;

    m_model_file = "./linear.model";
    //m_param.solver_type = L2R_L2LOSS_SVC_DUAL;
    m_param.solver_type = L2R_LR;
    //m_param.solver_type = L2R_LR_DUAL;
    m_param.C = 8;

    if(m_param.solver_type == L2R_LR || m_param.solver_type == L2R_L2LOSS_SVC) {
        m_param.eps = 0.01;
    } else if(m_param.solver_type == L2R_L2LOSS_SVC_DUAL || m_param.solver_type == L2R_L1LOSS_SVC_DUAL
            || m_param.solver_type == MCSVM_CS || m_param.solver_type == L2R_LR_DUAL) {
        m_param.eps = 0.1;
    } else if(m_param.solver_type == L1R_L2LOSS_SVC || m_param.solver_type == L1R_LR) {
        m_param.eps = 0.01;
    }
	
    m_param.nr_weight = 0;
    m_param.weight_label = NULL;
    m_param.weight = NULL;
    m_bias = -1;
}

QPredictLinearLearner::~QPredictLinearLearner()
{
    if (m_model) {
        free_and_destroy_model(&m_model);
    }

}

bool QPredictLinearLearner::init(QPredictConfig *config)
{
    config->get_string("liblinear.model_file", m_model_file);
    config->get_double("liblinear.bias", m_bias);

    return true;
}

bool QPredictLinearLearner::train(QPredictDocumentList &doc_list)
{
    uint32_t cnt;
	struct problem prob;
    struct feature_node *x_space;
    int32_t max_index;
    size_t num_space;
    const char * error_msg;
    int i, j;

    cnt = 1;
    prob.l = doc_list.size();

    prob.y = new int[prob.l];
    memset(prob.y, 0, sizeof(int) * prob.l);
    prob.x = new struct feature_node *[prob.l];
    memset(prob.x, 0, sizeof(struct feature_node *) * prob.l);

    prob.bias = m_bias;

    num_space = 0;

    QPredictDocumentListIter  end_doc_it = doc_list.end();
    for (QPredictDocumentListIter doc_it = doc_list.begin(); doc_it != end_doc_it; ++doc_it) {
        num_space += doc_it->feature_list.size() + 1;
    }

    x_space = new struct feature_node[num_space + prob.l];
    memset(x_space, 0, sizeof(struct feature_node) * (num_space + prob.l));

    max_index = 0;

    for (i = 0, j = 0; i < prob.l; i++) {
        std::vector<uint32_t>::iterator termIt;

        prob.x[i] = &x_space[j];
        prob.y[i] = doc_list[i].class_index;

        QPredictFeatureList &feature_list = doc_list[i].feature_list;
        // sort feature
        sort(feature_list.begin(), feature_list.end(), QPredictFeature::feature_compare);

        const QPredictFeatureListIter &feature_end_it = feature_list.end();
        for (QPredictFeatureListIter feature_it = feature_list.begin(); feature_it != feature_end_it; ++feature_it) {
            x_space[j].index = feature_it->id;
            x_space[j].value = feature_it->value;

            ++j;
        }

        if (j >= 1 && x_space[j - 1].index > max_index)
            max_index = x_space[j - 1].index;

        if(prob.bias >= 0)
            x_space[j++].value = prob.bias;

        x_space[j++].index = -1;

        cnt++;
    }

    if(prob.bias >= 0) {
        prob.n = max_index + 1;
        for(i = 1;i < prob.l; i++)
            (prob.x[i]-2)->index = prob.n;
        x_space[j-2].index = prob.n;
    } else {
        prob.n = max_index;
    }

    if ((error_msg = check_parameter(&prob, &m_param))) {
        //
        std::cerr << "error " << error_msg << std::endl;
    }

    m_model = ::train(&prob, &m_param);

    /*/ cross validation
	int total_correct = 0;
	int *target = (int*) malloc(sizeof(int) * prob.l);

	cross_validation(&prob,&m_param,10,target);

	for(int i=0;i<prob.l;i++)
		if(target[i] == prob.y[i])
			++total_correct;
	fprintf(stderr, "Cross Validation Accuracy = %g%%\n",100.0*total_correct/prob.l);

	free(target);
    */
    //destroy_param(&m_param);

    delete []prob.y;
    delete []prob.x;
    delete []x_space;

    return true;
}

bool QPredictLinearLearner::predict(QPredictDocument &doc)
{
    QPredictFeatureList &feature_list = doc.feature_list;

    int num_space = feature_list.size();
    num_space++;
    num_space++; // for bias

    struct feature_node *x_space = new struct feature_node[num_space];

    int nr_feature = get_nr_feature(m_model);
    int n;
    if (m_model->bias >= 0)
        n = nr_feature + 1;
    else
        n = nr_feature;

    sort(feature_list.begin(), feature_list.end(), QPredictFeature::feature_compare);
    const QPredictFeatureListIter &feature_end_it = feature_list.end();
    int j = 0;
    for (QPredictFeatureListIter feature_it = feature_list.begin(); feature_it != feature_end_it; ++feature_it) {
        x_space[j].index = feature_it->id;
        x_space[j].value = feature_it->value;
        ++j;
    }

    if(m_model->bias >= 0) {
        x_space[j].index = n;
        x_space[j].value = m_model->bias;
        ++j;
    }

    x_space[j].index = -1;
    x_space[j].value = -1;

    if (check_probability_model(m_model)) {
        doc.predict_class_index = static_cast<uint32_t>(
                ::predict_probability(m_model, x_space, doc.predict_class_probs)
                );
    } else {
        doc.predict_class_index = static_cast<uint32_t>(
                ::predict(m_model, x_space)
                );
    }

    delete []x_space;

    return true;
}

bool QPredictLinearLearner::save_model()
{
    assert(m_model != NULL);
    int r = ::save_model(m_model_file.c_str(), m_model);
    if (r == 0) {
        return true;
    }

    return false;
}

bool QPredictLinearLearner::load_model()
{
    assert(m_model == NULL);

    m_model = ::load_model(m_model_file.c_str());
    if (m_model) {
        return true;
    }

    return false;
}

int QPredictLinearLearner::num_of_features()
{
    assert(m_model != NULL);
    return get_nr_feature(m_model);
}

} // end for namesapce qcontent
