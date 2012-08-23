#ifndef QHTML2VDOM_H
#define QHTML2VDOM_H

#include <qcrawler_processor.h>
#include <qcontent_record.h>

class QWebFrame;
class QWebVDom;

namespace qcontent {

class QHtml2VdomPage;

class QHtml2Vdom : public QCrawlerProcessor
{

public:
    QHtml2Vdom();
    ~QHtml2Vdom();
    virtual int process(QCrawlerRecord &record);

private:
    QHtml2VdomPage *m_page;
    QWebFrame *m_frame;
    QWebVDom *m_webvdom;
};

} // end namespace qcontent

#endif
