/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

#ifndef QCONTENT_PROCESSOR_H
#define QCONTENT_PROCESSOR_H

#include "qcontent_record.h"

namespace qcontent {

class QContentProcessor
{
public:
    virtual int process(QContentRecord &record) = 0;
};

} // end namespace qcontent

#endif
