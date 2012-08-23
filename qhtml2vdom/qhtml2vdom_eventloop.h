#ifndef QCRAWLER_EVENTLOOP_H
#define QCRAWLER_EVENTLOOP_H

#include <QEventLoop>

namespace qcontent {

class QHtml2VdomEventLoop: public QEventLoop {
    Q_OBJECT

public:
    QHtml2VdomEventLoop():isFinished(false) {}

public Q_SLOTS:
    void quitWithRes(bool res) {
        m_res = res;
        isFinished = true;
        quit();
    }

    bool getRes() {
        return m_res;
    }

    bool checkFinished() {
        return isFinished;
    }

private:
    bool m_res;
    bool isFinished;
};

} // end namespace qcontent

#endif

