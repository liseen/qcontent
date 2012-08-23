/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

#include "qpredict_dict.h"
#include <fstream>

namespace qcontent {

QPredictDict::QPredictDict()
{
}


QPredictDict::QPredictDict(const std::string &path) :m_path(path)
{
}

QPredictDict::~QPredictDict()
{
}

bool QPredictDict::load()
{
    std::ifstream fin;
    fin.open(m_path.c_str());
    if (fin.good()) {
        int lineno = 0;
        std::string line;
        while (std::getline(fin, line)) {
            ++lineno;
            std::string w;
            int id; double val;
            if (parse_line(line, &id, &w, &val)) {
                insert(w, id, val);
            }
        }
        fin.close();
        return true;
    } else {
        LOG(ERROR) << "load term feature idf dict fail";
        return false;
    }
}

bool QPredictDict::save()
{
    std::ofstream fout;
    fout.open(m_path.c_str());
    if (fout.good()) {
        for (QPredictDictWordMapIter  it = words.begin(); it != words.end(); ++it) {
            fout << it->second.id << "\t" << it->first << "\t" << it->second.idf << "\n";
        }
        fout.close();
        return true;
    } else {
        LOG(ERROR) << "save term feature idf dict fail";
        return false;
    }
}


void QPredictDict::insert(const std::string &w, int id, double value)
{
    QPredictDictWord dw(id, value);
    words[w] = dw;
}

bool QPredictDict::query(const std::string &w, int *id, double *value)
{
    QPredictDictWordMapIter it = words.find(w);
    if (it != words.end()) {
        *id = it->second.id;
        *value = it->second.idf;
        return true;
    } else {
        return false;
    }
}

}

