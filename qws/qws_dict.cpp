#include "qws_dict.h"

#include <cstdio>
#include <fstream>
#include <iostream>

#include "qws_util.h"

BEGIN_NAMESPACE_QWS

QWSDict::QWSDict()
{
}

QWSDict::QWSDict(const std::string &path)
{
    m_path = path;
    load();
}


QWSDict::~QWSDict()
{
}


bool QWSDict::load()
{
    std::ifstream fin;
    fin.open(m_path.c_str());
    int lineno = 0;
    if (fin.good()) {
        std::string line;
        while(std::getline(fin, line)) {
            ++lineno;
            if (line.empty()) {
                continue;
            }
            std::string word;
            int value = lineno;
            size_t pos = line.find("\t");
            if (pos != std::string::npos) {
                word = line.substr(0, pos);
                value = atoi(line.substr(pos).c_str());
            } else {
                word = line;
            }
            m_map[word] = value;
            /*
            std::string word16;
            if (qcontent::QWSUtil::utf8_to_utf16(word, word16)) {
                m_map[word16] = value;
            } else {
                std::cerr << "error line(" << lineno << "): " << word << std::endl;
            }
            */
        }
        fin.close();
    } else {
        fprintf(stderr, "load dict error");
        return false;
    }

    return true;
}

bool QWSDict::save()
{
    //std::ofstream fout;

    return true;
}

void QWSDict::insert(const std::string &word, int value)
{
    m_map[word] = value;
}

bool QWSDict::query(const char* word, int size, int *value)
{
    std::string w;
    w.append(word, size);
    if (m_map.find(w) != m_map.end()) {
        *value = m_map[w];
        return true;
    } else {
        return false;
    }
}

bool QWSDict::query(const std::string &word, int *value)
{
    if (m_map.find(word) != m_map.end()) {
        *value = m_map[word];
        return true;
    } else {
        return false;
    }
}

END_NAMESPACE_QWS
