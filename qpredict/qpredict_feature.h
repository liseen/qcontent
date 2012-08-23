/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 *
 * qpredict_feature.h
 *
 */

#ifndef QPREDICT_FEATURE_H
#define QPREDICT_FEATURE_H

#include <string>
#include <vector>

namespace qcontent
{

struct QPredictFeature
{
    QPredictFeature(): id(0), value(0.0) {
    }

    uint32_t id;
    double value;
    static bool feature_compare(const QPredictFeature &f1, const QPredictFeature &f2) {
        return f1.id < f2.id;
    }
};

typedef std::vector<QPredictFeature> QPredictFeatureList;
typedef std::vector<QPredictFeature>::iterator QPredictFeatureListIter;


} // end for namespace qcontent

#endif
