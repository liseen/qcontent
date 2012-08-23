#include "webview.h"

WebView::WebView(QWidget *parent)
    :QWebView(parent),
    m_page(new WebPage(this))
{
    setPage(m_page);
}

void WebView::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = new QMenu(this);

    QWebHitTestResult r = page()->mainFrame()->hitTestContent(event->pos());
    //if (!page()->selectedText().isNull()) {
    //    qDebug() << " selected Text is null()";
    //}
    /*
    if (!page()->selectedText().isEmpty()) {
        if (!menu->isEmpty())
            menu->addSeparator();
        menu->addAction(tr("&Add Text Field"), this, SIGNAL(showTextFieldDialog()));
    }

    //if (!r.imageUrl().isEmpty()) {
        if (!menu->isEmpty())
            menu->addSeparator();
        hitPos = event->pos();
        menu->addAction(tr("&Add Image Field"), this, SIGNAL(showImageFieldDialog()));
   // }

    if (!menu->isEmpty())
            menu->addSeparator();
        menu->addAction(tr("&Viwie"), this, SIGNAL(showViwieDialog()));

    */
    if (menu->isEmpty()) {
        delete menu;
        menu = page()->createStandardContextMenu();
    }
    menu->exec(mapToGlobal(event->pos()));
    delete menu;

    QWebView::contextMenuEvent(event);
}


