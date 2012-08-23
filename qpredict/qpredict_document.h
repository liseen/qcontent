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

#ifndef QPREDICT_DOCUMENT_H
#define QPREDICT_DOCUMENT_H

#include <string>
#include <vector>

#include "qpredict_define.h"
#include "qpredict_term.h"
#include "qpredict_feature.h"

namespace qcontent
{

class QPredictDocument;

typedef std::vector<QPredictDocument> QPredictDocumentList;
typedef std::vector<QPredictDocument>::iterator QPredictDocumentListIter;

class QPredictDocument {
public:
    QPredictDocument();
    virtual ~QPredictDocument();

    //bool terms_count();
    //bool parse_term_from_line(const std::string &line);
    //bool parse_feature_from_line(const std::string &line);

    //bool parse_from_zdcpline(const std::string &line);
    //bool parse_from_json(const std::string &line);
/*
    const std::string &get_field(const std::string &field) {
        if (fields.find(field) != fields.end()) {
            return fields[field];
        } else {
            return string("");
        }
    }
*/
    std::string raw_data;
    TermHash terms;

    QPredictFeatureList feature_list;


    int class_index;
    std::string class_label;

    int predict_class_index;
    std::string predict_class_label;
    double predict_class_probs[QPREDICT_MAX_CLASSES];

//    unordered_map<std::string, std::string> fields;
};

} // end for namespace qcontent

#endif
