#ifndef VDOMWEBVIEW_H
#define VDOMWEBVIEW_H


#include <qwebview.h>
#include <webpage.h>
#include <QDebug>
#include <QtGui>
#include <qwebframe.h>
#include "fielddialog.h"
#include "viwiedialog.h"


class WebView : public QWebView
{
    Q_OBJECT

public:
    WebView(QWidget *parent = 0);

    WebPage *webPage() const { return m_page; }

signals:
    void showImageFieldDialog();
    void showTextFieldDialog();

    void showViwieDialog();

public:
    QPoint hitPos;

protected:
    void contextMenuEvent(QContextMenuEvent *event);
    WebPage* m_page;
};

#endif
