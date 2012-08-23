#include "webpage.h"
#include "mainwindow.h"
#include <QtUiTools/QUiLoader>

QWebPage *WebPage::createWindow(QWebPage::WebWindowType)
{
    MainWindow *mw = new MainWindow;
    mw->show();
    return mw->webPage();
}

QObject *WebPage::createPlugin(const QString &classId, const QUrl &url, const QStringList &paramNames, const QStringList &paramValues)
{
    Q_UNUSED(url);
    Q_UNUSED(paramNames);
    Q_UNUSED(paramValues);
    QUiLoader loader;
    return loader.createWidget(classId, view());
}

void WebPage::javaScriptConsoleMessage ( const QString & message, int lineNumber, const QString & sourceID ) {
    qDebug() << "[Error Console] " << sourceID << ": line " << lineNumber <<
        ": " << message << endl;
}

QString WebPage::userAgentForUrl(const QUrl& url) const {

  if (!m_userAgent.isNull())
    return m_userAgent;

  return QWebPage::userAgentForUrl(url);
}

void WebPage::setUserAgent(const QString& userAgent) {
    m_userAgent = userAgent;
}

