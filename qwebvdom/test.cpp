#include <QWebView>
#include <QWebPage>
#include <QWebFrame>
#include <QDebug>
#include <QUrl>
#include <QTime>
#include <QApplication>

#include <vdom.h>
#include "qwebvdom.h"

using namespace vdom;
#include <iostream>
using namespace std;

int main(int argc, char* argv[])
{
    QWebSettings::globalSettings()->setAttribute(QWebSettings::DnsPrefetchEnabled, true);
    QWebSettings::globalSettings()->setAttribute(QWebSettings::JavascriptEnabled, false);
    QWebSettings::globalSettings()->setAttribute(QWebSettings::AutoLoadImages, false);
    QWebSettings::globalSettings()->setAttribute(QWebSettings::JavaEnabled, false);
    QWebSettings::globalSettings()->setAttribute(QWebSettings::AutoLoadImages, false);
    //QWebSettings::globalSettings()->setDefaultTextEncoding("GB18030-0");
    QWebSettings::globalSettings()->setDefaultTextEncoding("GB18030-0");
    //QWebSettings::globalSettings()->setAttribute(QWebSettings::JavaEnabled, false);
    QWebSettings::globalSettings()->setAttribute(QWebSettings::PluginsEnabled, false);
    QWebSettings::globalSettings()->setAttribute(QWebSettings::JavascriptCanOpenWindows, false);
    //QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, false);
    //QWebSettings::globalSettings()->setAttribute(QWebSettings::PrintElementBackgrounds, false);

    QApplication app(argc, argv);
    QWebView* view = new QWebView(NULL);
    QWebPage* page = view->page();

    page->setViewportSize(QSize(947, 843));
    QEventLoop loop;
    QUrl url("http://www.baidu.com");
    if (argc > 1) {
        url = QUrl::fromEncoded(QByteArray(argv[1]));
    }

    view->load(url);
    QObject::connect(page, SIGNAL(loadFinished(bool)), &loop,
SLOT(quit()));
    loop.exec();
    QWebFrame * frame = view->page()->mainFrame();
    QWebVDom *webvdom = new QWebVDom(frame);
    ::vdom::Window window;

    QTime begin_time;
    begin_time.start();
    webvdom->buildVdom(&window);
    std::cout<<"elapsed: "<< begin_time.elapsed() << endl;
    std::cout << window.Utf8DebugString() << std::endl;
    std::cout << "code: " << webvdom->httpStatusCode() << std::flush;
    std::cout << "mimetype: " << webvdom->mimeType() << std::endl;
    std::cout << "last_modified: " << webvdom->lastModified() << std::endl;
    std::cout << "expires: " << webvdom->expires() << std::endl;
    std::cout << std::flush;
    view->show();
    app.exec();
}
