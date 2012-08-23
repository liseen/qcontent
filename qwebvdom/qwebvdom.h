/*
 * Copyright 2011 Qunar Inc.
 * BSD license
 * Author
 *    xunxin.wan (万珣新) <xunxin.wan@qunar.com>
 */

#ifndef QWEBVDOM_H
#define QWEBVDOM_H

#include <QtCore/qobject.h>
#include <QtCore/qurl.h>
#include <QtCore/qvariant.h>
#include <QtGui/qicon.h>
#include <QtScript/qscriptengine.h>
#if QT_VERSION >= 0x040400
#include <QtNetwork/qnetworkaccessmanager.h>
#endif

#include "qwebkitglobal.h"

#include <list>
#include <vdom.h>

class QWebFrame;
class QWebVDomPrivate;

class QWEBKIT_EXPORT QWebVDom : public QObject
{
    //Q_OBJECT
public:
    QWebVDom(QWebFrame* qframe);
    ~QWebVDom();

    bool buildVdom(vdom::Window *vdom_win);
    int httpStatusCode();
    int lastModified();
    int expires();
    std::string mimeType();

private:
    QWebFrame *m_qframe;
};

#endif // QWEBVDOM_H
