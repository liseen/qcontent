#ifndef QCRALER_WEBPAGE_H
#define QCRALER_WEBPAGE_H

#include <QtWebKit>
#include <QWebPage>

namespace qcontent {

class QHtml2VdomPage : public QWebPage {
    Q_OBJECT

public:
    void setUserAgent(const QString& userAgent);
//    bool crawlerLoad(const QUrl &qurl);

    bool setRawHtmlContent(const std::string &url, const std::string &raw_html_content);
    bool setRawHtml(const std::string &url, const std::string &raw_html);

protected:
    QString chooseFile(QWebFrame *frame, const QString& suggestedFile);
    void javaScriptConsoleMessage(const QString& message, int lineNumber, const QString& sourceID);
    bool javaScriptPrompt(QWebFrame* frame, const QString& msg, const QString& defaultValue, QString* result);
    void javaScriptAlert(QWebFrame* frame, const QString& msg);
    bool javaScriptConfirm(QWebFrame* frame, const QString& msg);
    QString userAgentForUrl(const QUrl& url) const;

    QString mUserAgent;
};

} // end namespace qcontent

#endif

