/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

/*
 * qpedict tool for train and predict
 *
 */

#include <iostream>

#include <getopt.h>

#include <strtk.hpp>

#include <qcontent_config.h>

#include "qpredict_log.h"
#include "qpredict.h"

using namespace std;
using namespace qcontent;

static void print_usage(FILE* stream, int exit_code) {
    fprintf(stream, "Usage: qpredict options \n");
    fprintf(stream,
            "  -h --help               Display this usage information.\n"
            "  -t --train              Training documents\n"
            "  -v --valid              Validing documents\n"
            "  -c --conf <file>        Config file\n");

    exit(exit_code);
}


int main(int argc, char *argv[])
{
    //google::InitGoogleLogging(argv[0]);

    bool train_mode = false;
    bool valid_mode = false;

    std::string config_file = "./qpredict.conf";

    const char* const short_options = "htvc:";
    const struct option long_options[] = {
        { "help",     0, NULL, 'h' },
        { "train",   0, NULL, 't' },
        { "valid",   0, NULL, 'v' },
        { "conf",    1, NULL, 'c' },
        { NULL,       0, NULL, 0   }
    };

    int next_option;
    do {
        next_option = getopt_long (argc, argv, short_options,
                               long_options, NULL);
        switch (next_option) {
            case 't':
                train_mode = true;
                break;
            case 'v':
                valid_mode = true;
                break;
            case 'h':
                print_usage(stdout, 0);
                break;
            case 'c':
                config_file = optarg;
                break;
            case -1:
                break;
            case '?':
                print_usage(stderr, 1);
            default:
                print_usage(stderr, 1);
        }
    } while (next_option != -1);


    qcontent::QPredict qpredict;

    qcontent::QContentConfig *qcontent_config = qcontent::QContentConfig::get_instance();
    if (!qcontent_config->parse_file(config_file)) {
        LOG(FATAL) << "parse config file error";
        exit(-1);
    }

    QPredictConfig config(qcontent_config->values_map());

    bool init_ret = qpredict.init(&config);
    if (!init_ret) {
        LOG(FATAL) << "init qpredict fatal";
    }

    if (train_mode) {
        QPredictDocumentList doc_list;
        int lineno = 0;
        string line;
        while (std::getline(std::cin, line)) {
            ++lineno;
            if (line.empty()) {
                LOG(WARNING) << "empty line no: " << lineno;
                continue;
            }

            QPredictDocument doc;

            bool valid = strtk::parse(line, "\t", doc.raw_data, doc.class_label);
            if (!valid) {
                LOG(WARNING) << "invalid line no: " << lineno;
                continue;
            }

            doc_list.push_back(doc);
        }

        bool train_ret = qpredict.train(doc_list);
        if (!train_ret) {
            LOG(FATAL) << "train fatal";
        }

        bool save_ret = qpredict.save_qpredict_model();
        if (!save_ret) {
            LOG(FATAL) << "save fatal";
        }
    } else if (valid_mode) {
        bool load_ret = qpredict.load_qpredict_model();
        if (!load_ret) {
            LOG(FATAL) << "load fatal";
        }

        QPredictDocumentList doc_list;
        int lineno = 0;
        string line;
        while (std::getline(std::cin, line)) {
            ++lineno;
            if (line.empty()) {
                LOG(WARNING) << "empty line no: " << lineno;
                continue;
            }

            QPredictDocument doc;

            bool valid = strtk::parse(line, "\t", doc.raw_data, doc.class_label);
            if (!valid) {
                LOG(WARNING) << "invalid line no: " << lineno;
                continue;
            }

            doc_list.push_back(doc);
        }

        qpredict.valid(doc_list);
    } else {
        bool load_ret = qpredict.load_qpredict_model();
        if (!load_ret) {
            LOG(FATAL) << "load fatal";
        }

        int lineno = 0;
        string line;
        while (std::getline(std::cin, line)) {
            ++lineno;
            if (line.empty()) {
                LOG(WARNING) << "empty line no: " << lineno;
                continue;
            }

            QPredictDocument doc;
            bool valid = strtk::parse(line, "\t", doc.raw_data, doc.class_label);
            if (!valid) {
                doc.raw_data = line;
            }

            if (doc.raw_data.empty()) {
                LOG(WARNING) << "invalid line no: " << lineno;
                continue;
            }

            bool predict_ret = qpredict.predict(doc);
            if (!predict_ret) {
                LOG(WARNING) << "predict line no: " << lineno << "fail";
                continue;
            }

            std::cout << line << "\t" << qpredict.class_id2label[doc.predict_class_index];
            for (size_t i = 0; i < qpredict.class_id2label.size(); i++) {
                std::cout << "\t" << qpredict.class_id2label[i] << ":" << doc.predict_class_probs[i];

            }
            std::cout << "\n";
        }
    }

    return 0;
}
