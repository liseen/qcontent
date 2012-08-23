#ifndef URLLOADER_H
#define URLLOADER_H

#include "qwebview.h"
#include <QFile>
#include <QVector>
#include <QTextStream>
#include <QtCore>

class URLLoader : public QObject
{
    Q_OBJECT
public:
    URLLoader(QWebView* view, const QString& inputFileName)
        : m_view(view)
        , m_stdOut(stdout)
    {
        init(inputFileName);
    }

public slots:
    void loadNext();

private:
    void init(const QString& inputFileName);

    bool getUrl(QString& qstr)
    {
        if (m_index == m_urls.size())
            return false;

        qstr = m_urls[m_index++];
        return true;
    }

private:
    QVector<QString> m_urls;
    int m_index;
    QWebView* m_view;
    QTextStream m_stdOut;
};

#endif

