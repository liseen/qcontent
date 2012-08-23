/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

/*
 * qpredict_feature_selector.h
 *
 */

#ifndef QPREDICT_FEATURE_SELECTOR_H
#define QPREDICT_FEATURE_SELECTOR_H

namespace qcontent
{

class QPredictFeatureSelector
{

public:
    QPredictFeatureSelector() :num_of_classes(0), num_of_docs(0) {
        for (uint32_t i = 0; i < QPREDICT_MAX_CLASSES; ++i) {
            docs_per_class[i] = 0;
        }
    }

    uint32_t docs_per_class[QPREDICT_MAX_CLASSES];
    uint32_t num_of_classes;
    uint32_t num_of_docs;

    double chi_square_score(const QPredictTerm &term)
    {
        double score = 0.0;
        double sum = 0.0;
        double num = 0.0;
        double per = 0.0;

        for (uint32_t i = 0; i < num_of_classes; ++i) {
            sum += term.df_per_class[i];
        }

        uint32_t loop_size = num_of_classes;
        if (loop_size == 2) {
            loop_size = 1;
        }

        for (uint32_t i = 0; i < loop_size; ++i) {
            int docs_of_class = docs_per_class[i];
            if (docs_of_class == 0) {
                continue;
            }

            per = static_cast<double>(docs_of_class);
            //rate = per / static_cast<double>(num_of_docs);
            num = static_cast<double>(term.df_per_class[i]);
/*
            // CHI Square
            double A = num;
            double B = sum - num;
            double C = per - num;
            double D = static_cast<double>(num_of_docs) - A - B - C;
            double AD_CB = A * D - C * B; // a replacement

            std::cout << "value: " << term.value
                << " sum: " << sum
                << " A: " << A
                << " B: " << B
                << " C: " << C
                << " D: " << D
                << " AD_CB: " << AD_CB << "\n";

            // ChiSquare Formula
            double current = (num_of_docs * AD_CB * AD_CB) /
                             ((A + C) * (B + D) * (A + B) * (C + D));

*/
            // k(i,0) = numbers of documents in class i not containing term t
            // k(i,1) = numbers of documents in class i containing term t
            double k00 = per - num;
            double k01 = num;
            double k11 = sum - num;
            double k10 = static_cast<double>(num_of_docs) - k00 - k01 - k11;

            double k1100_k1001 = k11 * k00 - k10 * k01;

            double current = (num_of_docs * k1100_k1001 * k1100_k1001) /
                             ((k11 + k10) * (k01 + k00) * (k11 + k01) * (k10 + k00))
                             ;
            // max score
            if (current > score) {
                score = current;
            }

            std::cout << "value: " << term.value
                << " k00: " << k00
                << " k01: " << k01
                << " k10: " << k10
                << " k11: " << k11
                << " score: " << score
                << "\n";


        }

        return score;
    }

    static bool feature_select_compare(QPredictTerm *t1, QPredictTerm *t2) {
        return t1->feature_score > t2->feature_score;
    }
};

} // end for namespace qcontent


#endif
