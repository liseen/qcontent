#include "urlloader.h"

void URLLoader::loadNext() {
    QString qstr;
    if (getUrl(qstr)) {
        QUrl url;
        url.setEncodedUrl(qstr.toUtf8(), QUrl::StrictMode);
        if (url.isValid()) {
            m_stdOut << "Loading " << qstr << " ......" << endl;
            m_view->load(url);
        } else
            loadNext();
    } else
        disconnect(m_view, 0, this, 0);
}

void URLLoader::init(const QString& inputFileName) {
    QFile inputFile(inputFileName);
    if (inputFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream stream(&inputFile);
        QString line;
        while (true) {
            line = stream.readLine();
            if (line.isNull())
                break;
            m_urls.append(line);
        }
    } else {
        qDebug() << "Cant't open list file";
        exit(0);
    }
    m_index = 0;
    inputFile.close();
}

