#ifndef QCRAWLER_PROCESSOR_H
#define QCRAWLER_PROCESSOR_H

#include "qcontent_record.h"

namespace qcontent {

class QCrawlerProcessor
{
public:
    virtual int process(QCrawlerRecord &record) = 0;

};

}

#endif
