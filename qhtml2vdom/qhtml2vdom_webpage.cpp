#include "qhtml2vdom_webpage.h"
#include "qhtml2vdom_eventloop.h"

#include <QSignalSpy>

namespace qcontent {

void QHtml2VdomPage::setUserAgent(const QString& userAgent)
{
    mUserAgent = userAgent;
}


QString QHtml2VdomPage::chooseFile(QWebFrame* /*frame*/, const QString& /*suggestedFile*/)
{
  return QString::null;
}

bool QHtml2VdomPage::javaScriptConfirm(QWebFrame* /*frame*/, const QString& /*msg*/)
{
  return true;
}

bool QHtml2VdomPage::javaScriptPrompt(QWebFrame* /*frame*/,
                           const QString& /*msg*/,
                           const QString& /*defaultValue*/,
                           QString* /*result*/)
{
  return true;
}

void QHtml2VdomPage::javaScriptConsoleMessage(const QString& /*message*/,
                                   int /*lineNumber*/,
                                   const QString& /*sourceID*/)
{
  // noop
}

void QHtml2VdomPage::javaScriptAlert(QWebFrame* /*frame*/, const QString& /*msg*/)
{
  // noop
}



QString QHtml2VdomPage::userAgentForUrl(const QUrl& url) const
{
  if (!mUserAgent.isNull())
    return mUserAgent;

  return QWebPage::userAgentForUrl(url);
}
/*
bool QHtml2VdomPage::crawlerLoad(const QUrl &url)
{
    int mTimeout = 60 * 1000;
    //int mTimeout = -1;
    if (url.isValid()) {
        QHtml2VdomEventLoop loop;
        QObject::connect(this, SIGNAL(loadFinished(bool)), &loop,
                SLOT(quitWithRes(bool)));

        QTimer timer;
        QSignalSpy timeoutSpy(&timer, SIGNAL(timeout()));
        if (mTimeout > 0) {
            QObject::connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
            timer.setSingleShot(true);
            timer.start(mTimeout);
        }
        this->mainFrame()->load(url);
        loop.exec();

        if (!timeoutSpy.isEmpty()) {
            qlog_warn("crawl_timeout", url.toString().toUtf8().data());
            this->blockSignals(true);
            this->triggerAction(QWebPage::Stop);
            this->blockSignals(false);
            return false;
        }

        if (!loop.getRes()) {
            qlog_warn("cannot_crawl", url.toString().toUtf8().data());
            return false;
        }

        return true;
    } else {
        return false;
    }
}
*/
bool QHtml2VdomPage::setRawHtml(const std::string &url, const std::string &raw_html)
{
    int m_timeout = 60 * 1000;
    QUrl qurl;
    qurl.setEncodedUrl(QByteArray(url.c_str()));

    QHtml2VdomEventLoop loop;
    QObject::connect(this, SIGNAL(loadFinished(bool)), &loop,
                SLOT(quitWithRes(bool)));

    mainFrame()->setHtml(QString::fromUtf8(raw_html.c_str(), raw_html.size()), qurl);

    if (!loop.checkFinished()) {
        QTimer timer;
        QSignalSpy timeoutSpy(&timer, SIGNAL(timeout()));
        if (m_timeout > 0) {
            QObject::connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
            timer.setSingleShot(true);
            timer.start(m_timeout);
        }

        loop.exec();

        if (!timeoutSpy.isEmpty()) {
            this->blockSignals(true);
            this->triggerAction(QWebPage::Stop);
            this->blockSignals(false);
            return false;
        } else if (!loop.getRes()) {
            return false;
        }
    }

    return true;
}

bool QHtml2VdomPage::setRawHtmlContent(const std::string &url, const std::string &raw_html_content)
{
    int m_timeout = 60 * 1000;
    QUrl qurl;
    qurl.setEncodedUrl(QByteArray(url.c_str()));

    QHtml2VdomEventLoop loop;
    QObject::connect(this, SIGNAL(loadFinished(bool)), &loop,
                SLOT(quitWithRes(bool)));

    mainFrame()->setContent(QByteArray(raw_html_content.c_str(), raw_html_content.size()), QString("text/html"), qurl);

    if (!loop.checkFinished()) {
        QTimer timer;
        QSignalSpy timeoutSpy(&timer, SIGNAL(timeout()));
        if (m_timeout > 0) {
            QObject::connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
            timer.setSingleShot(true);
            timer.start(m_timeout);
        }

        loop.exec();

        if (!timeoutSpy.isEmpty()) {
            this->blockSignals(true);
            this->triggerAction(QWebPage::Stop);
            this->blockSignals(false);
            return false;
        } else if (!loop.getRes()) {
            return false;
        }
    }

    return true;
}

} // end namespace qcontent
