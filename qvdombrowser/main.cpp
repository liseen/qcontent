#include "version.h"
#include "webpage.h"
#include "mainwindow.h"
#include "urlloader.h"

#include <qwebview.h>
#include <qwebframe.h>
#include <qwebsettings.h>

#include <QtGui>
#include <QDebug>
#include <QPrintPreviewDialog>

//#include <QVector>
#include <iostream>
#include <QTextStream>
#include <QTextCodec>
#include <QFile>
#include <cstdio>

static void help(int status_code);
static void showVersion(const QApplication& app);

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QTextCodec *tc = QTextCodec::codecForName("utf8");
    QTextCodec::setCodecForCStrings(tc);
    QString url = "http://www.yahoo.cn";

    QWebSettings::setMaximumPagesInCache(20);

    app.setApplicationName(VB_PRODUCT_NAME);
    app.setApplicationVersion(
        QString("%1.%2.%3")
            .arg(VB_MAJORVERSION_NUMBER)
            .arg(VB_MINORVERSION_NUMBER)
            .arg(VB_PATCHLEVEL_NUMBER)
    );
    QCoreApplication::setOrganizationName("Yahoo China EEEE");
    QCoreApplication::setOrganizationDomain("eeeeworks.org");
    QCoreApplication::setApplicationName(VB_PRODUCT_NAME);

    QWebSettings::setObjectCacheCapacities((16*1024*1024) / 8, (16*1024*1024) / 8, 16*1024*1024);

    QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);

    const QStringList args = app.arguments();
    QStringList jsFiles;

    for (int i = 1; i < args.count(); i++) {
        QString arg = args.at(i);
        if (arg == "-h" || arg == "--help") {
            //argHelp = true;
            help(0);
        } else if (arg.indexOf("--js=") == 0) {
            QString jsFile = arg.split("=").at(1);
            jsFiles.push_back(jsFile);
        } else if (arg == "-v" || arg == "--version") {
            showVersion(app);
            return 0;
        } else if (arg.indexOf("-") == 0) {
            fprintf(stderr, "Invalid command-line option: %s\n\n", arg.toUtf8().data());
            exit(1);
        } else {
            url = arg;
        }
    }

    MainWindow window(url);
    //qDebug() << "js files: " << jsFiles << endl;
    if (jsFiles.count() > 0) {
        window.setJSFiles(jsFiles);
    }
    window.show();
    return app.exec();
}

static void help(int status_code) {
    fprintf(status_code == 0 ? stdout : stderr,
        "Usage: VdomBrowser [Options] [URL]\n"
        "Options:\n"
        "  -h\n"
        "  --help           Print this help page and exit\n"
        "  --js <.js file>  JavaScript file executed after loading each web\n"
        "                   page. Multiple --js are allowed and would run\n"
        "                   in order.\n"
        "  -v\n"
        "  --version        Display version number.\n"
    );
    exit(status_code);
}

static void showVersion (const QApplication& app) {
    std::cout << QString("VdomBrowser version %1\n"
        "Copyright (c) 2009 by Yahoo! China EEEE Works, Alibaba Inc.\n"
        ).arg(app.applicationVersion()).toUtf8().data();
}

