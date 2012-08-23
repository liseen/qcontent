#include <qwebvdom.h>

#include "qhtml2vdom.h"
#include "qhtml2vdom_webpage.h"
#include <QDebug>
#include <iostream>

namespace qcontent {

QHtml2Vdom::QHtml2Vdom()
{
    m_page = new QHtml2VdomPage();
    m_page->settings()->setAttribute(QWebSettings::JavascriptEnabled, false);
    m_page->settings()->setAttribute(QWebSettings::AutoLoadImages, false);
    m_page->settings()->setAttribute(QWebSettings::JavaEnabled, false);
    //default charset
    QWebSettings::globalSettings()->setDefaultTextEncoding("GB18030-0");
    m_page->settings()->setDefaultTextEncoding("GB18030-0");
    m_page->settings()->setAttribute(QWebSettings::JavaEnabled, false);
    m_page->settings()->setAttribute(QWebSettings::PluginsEnabled, false);

    m_page->settings()->setAttribute(QWebSettings::JavascriptCanOpenWindows, false);
    m_page->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, false);
    m_page->settings()->setAttribute(QWebSettings::PrintElementBackgrounds, false);
    m_page->settings()->setMaximumPagesInCache(0);
    //page->settings()->setObjectCacheCapacities(0, 0, 0);
    m_page->setViewportSize(QSize(947, 843));

    m_page->setUserAgent(QString("Mozilla/5.0 QunarBot/1.0"));
    m_frame = m_page->mainFrame();
    m_webvdom = new QWebVDom(m_frame);
}

QHtml2Vdom::~QHtml2Vdom()
{
    delete m_page;
    delete m_webvdom;
}

int QHtml2Vdom::process(QCrawlerRecord &record)
{
    bool ret;
    if (!record.redirect_url.empty()) {
        ret = m_page->setRawHtmlContent(record.redirect_url, record.raw_content);
    } else {
        ret = m_page->setRawHtmlContent(record.url, record.raw_content);
    }

    if (ret) {
        if (record.win != NULL) {
            m_webvdom->buildVdom(record.win);
            record.win->SerializeToString(&record.vdom);
        } else {
            vdom::Window win;
            m_webvdom->buildVdom(&win);
            win.SerializeToString(&record.vdom);
        }
        return 0;
    } else {
        return 1;
    }
}

} // end namespace qcontent

