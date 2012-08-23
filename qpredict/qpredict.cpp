/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

/*
 *
 */

#include "qpredict.h"

#include <cmath>
#include <cassert>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "qpredict_tokenizer.h"
#include "qpredict_feature_selector.h"

#include "qpredict_linear_learner.h"

namespace qcontent
{

QPredict::QPredict()
{

    term_feature_size_limit = 50000;
    term_feature_size = 0;

    lda_feature_size = 0;

    term_feature_dict_file = "./term_feature.dict";

    tokenizer = new QPredictTokenizer();
    learner = new QPredictLinearLearner();
}

QPredict::~QPredict()
{
    term_feature_size_limit = 50000;
    delete tokenizer;
    delete learner;
}

static void qpredict_string_split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while(std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
}

bool QPredict::init(QPredictConfig *config)
{
    config->get_integer("lda_feature.size", lda_feature_size);

    config->get_integer("term_feature.size_limit", term_feature_size_limit);
    config->get_string("term_feature.dict_file", term_feature_dict_file);

    std::string class_labels_str = "trip,noTrip";

    config->get_string("qpredict.class_labels", class_labels_str);

    qpredict_string_split(class_labels_str, ',', class_id2label);
    if (class_id2label.empty()) {
        LOG(ERROR) << "no class labels initialized";
        return false;
    }

    for (size_t i = 0; i < class_id2label.size(); ++i) {
        class_label2id[ class_id2label[i] ] = i;
    }

    if (!tokenizer->init(config)) {
        LOG(ERROR) << "init tokenizer fail";
        return false;
    }

    if (!learner->init(config)) {
        LOG(ERROR) << "init learner fail";
        return false;
    }

    return true;
}

bool QPredict::train(QPredictDocumentList &doc_list)
{
    // get field
    TermHash df_term_hash;
    QPredictFeatureSelector selector;

    QPredictDocumentListIter end_doc_it = doc_list.end();
    size_t doc_list_size = doc_list.size();
    if (doc_list_size == 0) {
        LOG(ERROR) << "train doc list is emtpy";
        return false;
    }

    int tokenizer_cnt = 0;
    // tokenize
    for (QPredictDocumentListIter doc_it = doc_list.begin(); doc_it != end_doc_it; ++doc_it) {
        ++tokenizer_cnt;
        if (tokenizer_cnt % 10000 == 0) {
            std::cerr << "tokenizer cnt " << tokenizer_cnt << std::endl;
        }
        //QPredictNgram ngram(2);
        //ngram.terms_count(doc_it->raw_data, doc_it->terms);
        tokenizer->terms_count(doc_it->raw_data, doc_it->terms);
        doc_it->raw_data.clear();
    }

    // feature selector
    for (QPredictDocumentListIter doc_it = doc_list.begin(); doc_it != end_doc_it; ++doc_it) {
        const std::string &class_label = doc_it->class_label;
        if (class_label2id.find(class_label) == class_label2id.end()) {
            std::cerr << "no label " << class_label << " define in config\n";
            return false;
        }
        uint32_t class_index = class_label2id[class_label];
        selector.docs_per_class[class_index]++;

        doc_it->class_index = class_index;

        TermHash &terms = doc_it->terms;
        // computer df
        for (TermHashIter term_it = terms.begin(); term_it != terms.end(); ++term_it) {
            TermHashIter df_term_it = df_term_hash.find(term_it->first);
            if (df_term_it == df_term_hash.end()) {
                QPredictTerm term;
                term.df = 1;
                term.df_per_class[class_index]++;
                term.value = term_it->first;
                df_term_hash[term_it->first] = term;
            } else {
                df_term_it->second.df += 1;
                df_term_it->second.df_per_class[class_index]++;
            }
        }
    }

    selector.num_of_docs = doc_list_size;
    selector.num_of_classes =  class_id2label.size();

    std::vector<QPredictTerm *> term_list;
    for (TermHashIter df_it = df_term_hash.begin(); df_it != df_term_hash.end(); ++df_it) {
        QPredictTerm *term = &(df_it->second);

        double df_ratio = static_cast<double>(term->df) / static_cast<double>(doc_list_size);
        //if (df_ratio < 0.0001 || (df_ratio > 0.6 && term->value.size() <= 3)) {
        if (term->value.size() <= 3 || df_ratio > 0.6) {
            std::cout << "too rare or too frequent term : " << term->value << "\n";
            continue;
        }

        term->feature_score = selector.chi_square_score(df_it->second);
        term_list.push_back(term);
    }

    std::sort(term_list.begin(), term_list.end(), QPredictFeatureSelector::feature_select_compare);

    int term_list_len = term_list.size();
    fprintf(stderr, "term size: %d\n", term_list_len);

    term_feature_size = term_list_len > term_feature_size_limit ? term_feature_size_limit : term_list_len;

    for (int i = 0; i < term_feature_size; ++i) {
        int id = i + 1;

        // TODO computer idf, should load from a file
        double idf = log(static_cast<double>(doc_list.size())
            / static_cast<double>(term_list[i]->df + 1));

        term_list[i]->id = id;

        term_feature_dict.insert(term_list[i]->value, id, idf);
    }

    // TODO output the feature
    for (int i = 0; i < term_feature_size; ++i) {
        term_list[i]->debug_print();
    }

    for (QPredictDocumentListIter doc_it = doc_list.begin(); doc_it != doc_list.end(); ++doc_it) {

        TermHash &terms = doc_it->terms;
        QPredictFeatureList &feature_list = doc_it->feature_list;
        for (TermHashIter term_it = terms.begin(); term_it != terms.end(); ++term_it) {
            int id; double idf;
            if (term_feature_dict.query(term_it->first, &id, &idf)) {
                QPredictFeature feature;
                feature.id = id;
                feature.value = term_it->second.tf * idf;
                feature_list.push_back(feature);
            }
        }
        // lda inf
        // add lda topics featuer
        for (int i = 0; i < lda_feature_size; ++i) {
            //int id = term_feature_size + i + 1;
            //double value = lda.inf[]
        }
    }

    learner->train(doc_list);

    return true;
}

bool QPredict::valid(QPredictDocumentList &doc_list)
{

    int class_size = class_id2label.size();

    double class_stat[QPREDICT_MAX_CLASSES];
    double predict_class_stat[QPREDICT_MAX_CLASSES];
    double total_stat[QPREDICT_MAX_CLASSES][QPREDICT_MAX_CLASSES];
    memset(class_stat, 0, sizeof(double) * QPREDICT_MAX_CLASSES);
    memset(predict_class_stat, 0, sizeof(double) * QPREDICT_MAX_CLASSES);
    memset(total_stat, 0, sizeof(double) * QPREDICT_MAX_CLASSES * QPREDICT_MAX_CLASSES);

    for (QPredictDocumentListIter doc_it = doc_list.begin(); doc_it != doc_list.end(); ++doc_it) {
        const std::string &class_label = doc_it->class_label;
        if (class_label2id.find(class_label) == class_label2id.end()) {
            std::cerr << "no label " << class_label << " define in config\n";
            return false;
        }
        doc_it->class_index = class_label2id[class_label];
        predict(*doc_it);
    }

    int right_total = 0;
    int wrong_total = 0;

    for (QPredictDocumentListIter doc_it = doc_list.begin(); doc_it != doc_list.end(); ++doc_it) {
        ++class_stat[doc_it->class_index];
        ++predict_class_stat[doc_it->predict_class_index];
        ++total_stat[doc_it->class_index][doc_it->predict_class_index];

        if (doc_it->class_index != doc_it->predict_class_index) {
            wrong_total++;
        } else {
            right_total++;
        }
    }

    for (int i = 0; i < class_size; i++) {
        fprintf(stderr, "%s:\t", class_id2label[i].c_str());
        for (int j = 0; j < class_size; j++) {
            fprintf(stderr, "%d(%.2f%%)\t", \
                    static_cast<int>(total_stat[i][j]), total_stat[i][j]*100/class_stat[i]);
        }

        double right = total_stat[i][i] / predict_class_stat[i];
        double recall = total_stat[i][i] / class_stat[i];
        fprintf(stderr, "right:%.2f recall:%.2f F:%.2f\n", right, recall, right * recall * 2 / (right + recall));
    }

    if (doc_list.size() > 0) {
        std::cerr << "right_ratio: " <<  (double)(right_total) / (double)doc_list.size()  << std::endl;
    } else {
        std::cerr << "No document gived!" << std::endl;
    }


    //if (doc_list.size() > 0) {
    //    std::cerr << "right_ratio: " <<  (double)(right_total) / (double)doc_list.size()  << std::endl;
    //} else {
    //    std::cerr << "No document gived!" << std::endl;
    //}

    return true;
}

bool QPredict::predict(QPredictDocument &doc)
{
    // tokenizer
    //QPredictNgram ngram(2);
    //ngram.terms_count(doc.raw_data, doc.terms);
    tokenizer->terms_count(doc.raw_data, doc.terms);

    // feature selector
    TermHash &terms = doc.terms;
    QPredictFeatureList &feature_list = doc.feature_list;
    // computer df
    for (TermHashIter term_it = terms.begin(); term_it != terms.end(); ++term_it) {
        int id;
        double idf;
        if (term_feature_dict.query(term_it->first, &id, &idf)) {
            QPredictFeature feature;
            feature.id = id;
            feature.value = term_it->second.tf * idf;
            feature_list.push_back(feature);
        }
    }

    // TODO lda inf
    // add lda topics featuer
    for (int i = 0; i < lda_feature_size; ++i) {
        //int id = term_feature_size + i + 1;
        //double value = lda.inf[]
    }

    learner->predict(doc);

    return true;
}

bool QPredict::save_qpredict_model()
{
    bool ret;
    // save term feature and idf
    term_feature_dict.set_path(term_feature_dict_file);
    ret = term_feature_dict.save();
    if (!ret) {
        LOG(ERROR) << "save term feature dict fail";
        return false;
    }

    // save learner model
    ret = learner->save_model();
    if (!ret) {
        LOG(ERROR) << "save learner model";
        return false;
    }

    return true;
}


bool QPredict::load_qpredict_model()
{
    bool ret;
    // load term feature dict
    term_feature_dict.set_path(term_feature_dict_file);
    ret = term_feature_dict.load();
    if (!ret) {
        LOG(ERROR) << "load term feature dict fail";
        return false;
    }

    // load learner model
    ret = learner->load_model();
    if (!ret) {
        LOG(ERROR) << "load learner model faile";
    }

    int total_features = learner->num_of_features();
    assert(total_features > 0);

    // get the size of term feature
    term_feature_size = total_features - lda_feature_size;

    return true;
}

} // end for namespace qcontent


