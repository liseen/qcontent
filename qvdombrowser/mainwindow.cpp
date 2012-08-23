#include "mainwindow.h"
#include "webpage.h"
#include "webview.h"
#include <stdlib.h>

const static int MAX_FILE_LINE_LEN = 2048;

MainWindow::MainWindow(const QString& url): currentZoom(100) {
    m_iterLabel = new QLabel(this);

    QDesktopServices::setUrlHandler(QLatin1String("http"), this, "loadUrl");
    m_hunterConfig = new HunterConfigDialog(this);
    connect(m_hunterConfig, SIGNAL(accepted()),
            this, SLOT(saveHunterConfig()));

    connect(&m_hunter, SIGNAL(readyReadStandardOutput()),
            this, SLOT(emitHunterStdout()));
    connect(&m_hunter, SIGNAL(readyReadStandardError()),
            this, SLOT(emitHunterStderr()));
    connect(&m_hunter, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(hunterFinished(int, QProcess::ExitStatus)));
    connect(&m_hunter, SIGNAL(started()), this, SLOT(hunterStarted()));

    m_huntButton = new QPushButton(tr("Hun&t"), this);
    connect(m_huntButton, SIGNAL(clicked()), SLOT(huntOnly()));

    m_iteratorConfig = new IteratorConfigDialog(this);
    connect(m_iteratorConfig, SIGNAL(accepted()),
            this, SLOT(saveIteratorConfig()));

    m_iterPrevButton = new QPushButton(tr("P&rev"), this);
    connect(m_iterPrevButton, SIGNAL(clicked()), SLOT(iterPrev()));
    m_iterNextButton = new QPushButton(tr("&Next"), this);
    connect(m_iterNextButton, SIGNAL(clicked()), SLOT(iterNext()));

    m_settings = new QSettings(
        QSettings::UserScope,
        qApp->organizationDomain(),
        qApp->applicationName(),
        this
    );
    readSettings();

    setupUI();
    m_callProc = new QProcess();

    connect(m_view->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()),
            this, SLOT(populateJavaScriptWindowObject()));

    connect(m_callProc, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(processFinished(int, QProcess::ExitStatus)));

    QUrl qurl;
    qurl.setEncodedUrl(url.toUtf8(), QUrl::StrictMode);
    if (qurl.isValid()) {
        m_urlEdit->setText(qurl.toEncoded());
        m_view->page()->mainFrame()->load(qurl);

        // the zoom values are chosen to be like in Mozilla Firefox 3
        zoomLevels << 30 << 50 << 67 << 80 << 90;
        zoomLevels << 100;
        zoomLevels << 110 << 120 << 133 << 150 << 170 << 200 << 240 << 300;
    }
}

void MainWindow::changeLocation() {
    //QUrl url = guessUrlFromString(urlEdit->text());
    //
    QString urlStr = m_urlEdit->text().trimmed();
    QRegExp test("^[a-zA-Z]+\\:.*");

    // Check if it looks like a qualified URL. Try parsing it and see.
    bool hasSchema = test.exactMatch(urlStr);
    if (!hasSchema) {
        urlStr = "http://" + urlStr;
    }

    QUrl url;
    url.setEncodedUrl(urlStr.toUtf8(), QUrl::StrictMode);
    m_urlEdit->setText(url.toEncoded());
    // we might have enabled JS for UI interaction...
    //qDebug() << "Loading URL " << url.toEncoded() << "..." << endl;
    loadUrl(url);
}

void MainWindow::loadFinished(bool done) {
    if (!done) {
        statusBar()->showMessage(
            QString("Failed to open resource %1.")
                .arg(m_urlEdit->text().trimmed()));
        return;
    }
    //qDebug() << "Loaded URL " << m_view->url().toEncoded() << "." << endl;
    m_urlEdit->setText(m_view->url().toEncoded());
    addUrlToList();

    if (!m_injectedJSFiles.isEmpty()) {
        for (int i = 0; i < m_injectedJSFiles.count(); i++) {
            const QString& fileName = m_injectedJSFiles[i];
            //qDebug() << QString("Checking if file %1 is readable...")
                //.arg(fileName).toUtf8() << endl;
            if (!QFile::exists(fileName)) {
                qDebug() <<
                    QString("js file \"%1\" not found.\n").arg(fileName).toUtf8().data();
                continue;
            }

            QFile file(fileName);
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                qDebug() <<
                    QString("Failed to load js file %1: %2\n")
                        .arg(fileName).arg(file.errorString()).toUtf8();
                file.close();
                continue;
            }
            qDebug() << "Loading JS file " << fileName << "...\n";
            QString injectedJS = QString::fromUtf8(file.readAll()) + "\n";
            QVariant res = evalJS(injectedJS + "true");
            qDebug() << "injecting JS res: " << res << endl;
        }
    }

    if (m_hunterEnabled) {
        ::vdom::Window window;
        m_webvdom->buildVdom(&window);
        std::string utf8_vdom = window.Utf8DebugString();
        QByteArray vdom(utf8_vdom.c_str(), utf8_vdom.size());
        QFile file(m_vdomPath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::warning(this, tr("VDOM Dumper"),
                QString("Failed to open file ") +
                m_vdomPath + " for writing: " +
                file.errorString(), QMessageBox::NoButton);
            return;
        }
        if (file.write(vdom) == -1) {
            QMessageBox::warning(this, tr("VDOM Dumper"),
                QString("Failed to write VDOM dump to file ") +
                m_vdomPath + ": " +
                file.errorString(), QMessageBox::NoButton);
            file.close();
            return;
        }
        file.close();

        // execute the external hunter program 
        m_hunter.close();
        QStringList args;
        args << m_vdomPath;
        m_itemInfoEdit->clear();
        m_pageInfoEdit->clear();
        m_hunterLabel->hide();
        statusBar()->showMessage("Starting " + m_hunterPath + "...");
        m_hunter.start(m_hunterPath, args);
    }
}

void MainWindow::setupUI() {
    createCentralWidget();
    createProgressBar();
    createUrlEdit();
    createToolBar();
    createMenus();
    //m_hunterConfig->hide();
}

void MainWindow::createCentralWidget() {
    createSidebar();
    createWebView();

    m_mainSplitter = new QSplitter(Qt::Horizontal, this);
    m_mainSplitter->addWidget(m_view);
    m_mainSplitter->addWidget(m_sidebar);

    setCentralWidget(m_mainSplitter);

    m_settings->beginGroup("MainWindow");
    //qDebug() << "splitter state fron settings: " << m_settings->value("sidebarSplitterSizes") << endl;
    //qDebug() << "restore main splitter: " << m_mainSplitter->restoreState(m_settings->value("mainSplitterSizes").toByteArray()) << endl;
    m_settings->endGroup();
}

void MainWindow::createWebView() {
    //m_view = new QWebView(this);
    //QWebPage* page = new QWebPage(m_view);
    m_view = new WebView(this);
    WebPage* page = webPage();

    page->settings()->setAttribute(QWebSettings::JavascriptEnabled, m_enableJavascript);
    page->settings()->setAttribute(QWebSettings::PluginsEnabled, m_enablePlugins);
    page->settings()->setAttribute(QWebSettings::AutoLoadImages, m_enableImages);
    page->settings()->setAttribute(QWebSettings::JavaEnabled, m_enableJava);

    m_view->setPage(page);
    m_webvdom = new QWebVDom(page->mainFrame());
    //page->setUserAgent("Mozilla/5.0 (Windows; U; Windows NT 5.1; zh-CN; rv:1.9.0.10) Gecko/2009042316 Firefox/3.0.10");
    //page->setUserAgent("Mozilla/5.0 (Windows; U; Windows NT 5.1; sv-SE) AppleWebKit/528.16 (KHTML, like Gecko) Version/4.0 Safari/528.16");
    page->setUserAgent("Mozilla/5.0 (Windows; U; Windows NT 5.1; zh-CN) AppleWebKit/528.16 (KHTML, like Gecko) Version/4.0 Safari/528.16");

    connect(m_view->page(), SIGNAL(loadFinished(bool)),
            this, SLOT(loadFinished(bool)));
    connect(m_view, SIGNAL(titleChanged(const QString&)),
            this, SLOT(setWindowTitle(const QString&)));
    connect(m_view->page(), SIGNAL(linkHovered(const QString&, const QString&, const QString &)),
            this, SLOT(showLinkHover(const QString&, const QString&)));
    connect(m_view->page(), SIGNAL(windowCloseRequested()), this, SLOT(deleteLater()));
    connect(m_view, SIGNAL(urlChanged(const QUrl&)), this, SLOT(updateUrl(const QUrl&)));
    connect(m_view, SIGNAL(linkClicked(const QUrl&)), this, SLOT(loadUrl(const QUrl&)));


    m_view->pageAction(QWebPage::Back)->setShortcut(QKeySequence::Back);
    m_view->pageAction(QWebPage::Stop)->setShortcut(Qt::Key_Escape);
    m_view->pageAction(QWebPage::Forward)->setShortcut(QKeySequence::Forward);
    m_view->pageAction(QWebPage::Reload)->setShortcut(Qt::Key_F5);
    m_view->pageAction(QWebPage::Undo)->setShortcut(QKeySequence::Undo);
    m_view->pageAction(QWebPage::Redo)->setShortcut(QKeySequence::Redo);
    m_view->pageAction(QWebPage::Cut)->setShortcut(QKeySequence::Cut);
    m_view->pageAction(QWebPage::Copy)->setShortcut(QKeySequence::Copy);
    m_view->pageAction(QWebPage::Paste)->setShortcut(QKeySequence::Paste);
    m_view->pageAction(QWebPage::ToggleBold)->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_B));
    m_view->pageAction(QWebPage::ToggleItalic)->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_I));
    m_view->pageAction(QWebPage::ToggleUnderline)->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_U));
}

void MainWindow::createSidebar() {
    //m_sidebar = new QWidget(this);
    //QVBoxLayout* sidebarLayout = new QVBoxLayout(m_sidebar);
    //m_sidebar->setLayout(sidebarLayout);

    m_sidebar = new QSplitter(Qt::Vertical, this);

    QWidget* item = new QWidget(this);
    QWidget* page = new QWidget(this);

    QVBoxLayout* itemLayout = new QVBoxLayout(item);

    QVBoxLayout* pageLayout = new QVBoxLayout(page);

    QLabel* label = new QLabel(tr("Debug Info"), item);
    itemLayout->addWidget(label);
    m_itemInfoEdit = new QTextEdit(m_sidebar);
    m_itemInfoEdit->setReadOnly(true);
    itemLayout->addWidget(m_itemInfoEdit);

    label = new QLabel(tr("Page Info"), page);
    pageLayout->addWidget(label);
    m_pageInfoEdit = new QTextEdit(m_sidebar);
    m_pageInfoEdit->setReadOnly(true);
    pageLayout->addWidget(m_pageInfoEdit);

    item->setLayout(itemLayout);
    page->setLayout(pageLayout);

    m_sidebar->addWidget(item);
    m_sidebar->addWidget(page);

    m_settings->beginGroup("MainWindow");
    //qDebug() << "splitter state fron settings: " << m_settings->value("sidebarSplitterSizes") << endl;
    //qDebug() << "restore side bar splitter: " << m_sidebar->restoreState(m_settings->value("sidebarSplitterSizes").toByteArray()) << endl;
    m_settings->endGroup();

    //sidebarLayout->addWidget(label);
    //m_sidebar->addWidget(splitter);
}

void MainWindow::createUrlEdit() {
    m_urlEdit = new LineEdit(this);
    m_urlEdit->setSizePolicy(QSizePolicy::Expanding,
            m_urlEdit->sizePolicy().verticalPolicy());

    connect(m_urlEdit, SIGNAL(returnPressed()),
            SLOT(changeLocation()));

    QCompleter *completer = new QCompleter(this);
    m_urlEdit->setCompleter(completer);
    completer->setModel(&m_urlCompleterModel);
}

void MainWindow::createProgressBar() {
    m_progress = new QProgressBar(this);
    m_progress->setRange(0, 100);
    m_progress->setMinimumSize(100, 20);
    m_progress->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    m_progress->hide();
    statusBar()->addPermanentWidget(m_progress);

    m_iterLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    statusBar()->addPermanentWidget(m_iterLabel);

    m_hunterLabel = new QLabel(this);
    m_hunterLabel->hide();
    m_hunterLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    statusBar()->addPermanentWidget(m_hunterLabel);

    //m_iterLabel->show();
    connect(m_view, SIGNAL(loadProgress(int)), m_progress, SLOT(show()));
    connect(m_view, SIGNAL(loadProgress(int)), m_progress, SLOT(setValue(int)));
    connect(m_view, SIGNAL(loadFinished(bool)), m_progress, SLOT(hide()));
}

void MainWindow::createToolBar() {
    QToolBar *bar = addToolBar("Navigation");
    bar->addAction(m_view->pageAction(QWebPage::Back));
    bar->addAction(m_view->pageAction(QWebPage::Forward));
    bar->addAction(m_view->pageAction(QWebPage::Reload));
    bar->addAction(m_view->pageAction(QWebPage::Stop));
    bar->addWidget(m_urlEdit);

    QPushButton* loadButton = new QPushButton(tr("&Load"), this);
    connect(loadButton, SIGNAL(clicked()), SLOT(changeLocation()));
    bar->addWidget(loadButton);

    bar->addWidget(m_huntButton);
    bar->addWidget(m_iterPrevButton);
    bar->addWidget(m_iterNextButton);
}

void MainWindow::createMenus() {
    createFileMenu();
    createEditMenu();
    createViewMenu();
    createSettingsMenu();
    createHelpMenu();
}

void MainWindow::createFileMenu() {
    QMenu *fileMenu = menuBar()->addMenu("&File");
    QAction *newWindow = fileMenu->addAction(tr("New Window"), this, SLOT(newWindow()));
    newWindow->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_N));

    QAction* focusAddressBar = new QAction(tr("Open Location..."), this);
    // Add the location bar shortcuts familiar to users from other browsers
    QList<QKeySequence> editUrlShortcuts;
    editUrlShortcuts.append(QKeySequence(Qt::ControlModifier + Qt::Key_L));
    editUrlShortcuts.append(QKeySequence(Qt::AltModifier + Qt::Key_O));
    editUrlShortcuts.append(QKeySequence(Qt::AltModifier + Qt::Key_D));
    focusAddressBar->setShortcuts(editUrlShortcuts);
    connect(focusAddressBar, SIGNAL(triggered()),
            this, SLOT(selectLineEdit()));
    fileMenu->addAction(focusAddressBar);

    fileMenu->addAction(tr("Print"), this, SLOT(print()));
    fileMenu->addAction(tr("Close"), this, SLOT(close()));
}

void MainWindow::createEditMenu() {
    QMenu *editMenu = menuBar()->addMenu("&Edit");
    editMenu->addAction(m_view->pageAction(QWebPage::Undo));
    editMenu->addAction(m_view->pageAction(QWebPage::Redo));
    editMenu->addSeparator();
    editMenu->addAction(m_view->pageAction(QWebPage::Cut));
    editMenu->addAction(m_view->pageAction(QWebPage::Copy));
    editMenu->addAction(m_view->pageAction(QWebPage::Paste));
    //editMenu->addSeparator();
    //QAction *setEditable = editMenu->addAction(tr("Set Editable"), this, SLOT(setEditable(bool)));
    //setEditable->setCheckable(true);
}

void MainWindow::createViewMenu() {
    QMenu *viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(m_view->pageAction(QWebPage::Stop));
    viewMenu->addAction(m_view->pageAction(QWebPage::Reload));
    viewMenu->addSeparator();

    QAction *zoomIn = viewMenu->addAction(tr("Zoom &In"), this, SLOT(zoomIn()));
    zoomIn->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Plus));

    QAction *zoomOut = viewMenu->addAction(tr("Zoom &Out"), this, SLOT(zoomOut()));
    zoomOut->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Minus));

    QAction *resetZoom = viewMenu->addAction(tr("Reset Zoom"), this, SLOT(resetZoom()));
    resetZoom->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_0));

    QAction *zoomTextOnly = viewMenu->addAction(tr("Zoom Text Only"), this, SLOT(toggleZoomTextOnly(bool)));
    zoomTextOnly->setCheckable(true);
    zoomTextOnly->setChecked(false);

    //viewMenu->addSeparator();
    //viewMenu->addAction("Dump HTML", this, SLOT(dumpHtml()));
}

void MainWindow::createSettingsMenu() {
    QMenu *prefMenu = menuBar()->addMenu(tr("&Preferences"));

    {
        QAction *enableJS = prefMenu->addAction(tr("Enable &Javascript"), this, SLOT(toggleEnableJavascript(bool)));
        enableJS->setCheckable(true);
        enableJS->setChecked(m_enableJavascript);
    }

    {
        QAction *enableParseJS = prefMenu->addAction(tr("Enable &Parse Javascript"), this, SLOT(toggleEnableParseJavascript(bool)));
        enableParseJS->setCheckable(true);
        enableParseJS->setChecked(m_enableParseJavascript);
    }

    {
        QAction *enableImages = prefMenu->addAction(tr("Enable &Images"), this, SLOT(toggleEnableImages(bool)));
        enableImages->setCheckable(true);
        enableImages->setChecked(m_enableImages);
    }
    {
        QAction *enablePlugins = prefMenu->addAction(tr("Enable &Plugins"), this, SLOT(toggleEnablePlugins(bool)));
        enablePlugins->setCheckable(true);
        enablePlugins->setChecked(m_enablePlugins);
    }
    {
        QAction *enableJava = prefMenu->addAction(tr("Enable J&ava"), this, SLOT(toggleEnableJava(bool)));
        enableJava->setCheckable(true);
        enableJava->setChecked(m_enableJava);
    }
    prefMenu->addSeparator();
    prefMenu->addAction(tr("X &Hunter"), this, SLOT(execHunterConfig()));
    prefMenu->addAction(tr("&URL Iterator"), this, SLOT(execIteratorConfig()));
}

void MainWindow::createHelpMenu() {
    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(tr("About &Qt"), qApp, SLOT(aboutQt()));
    helpMenu->addAction(tr("&About ") + qApp->applicationName(),
            this, SLOT(aboutMe()));
}


void MainWindow::writeSettings() {
    m_settings->beginGroup("MainWindow");

    m_settings->setValue("size", size());
    m_settings->setValue("pos", pos());

    m_settings->setValue("enableJavascript", QVariant(m_enableJavascript));
    m_settings->setValue("enableParseJavascript", QVariant(m_enableParseJavascript));
    m_settings->setValue("enablePlugins", QVariant(m_enablePlugins));
    m_settings->setValue("enableImages", QVariant(m_enableImages));
    m_settings->setValue("enableJava", QVariant(m_enableJava));

    m_settings->setValue("hunterEnabled", QVariant(m_hunterEnabled));
    m_settings->setValue("hunterPath", m_hunterPath);
    m_settings->setValue("vdomPath", m_vdomPath);

    m_settings->setValue("iteratorEnabled", QVariant(m_iteratorEnabled));
    m_settings->setValue("urlListFile", QVariant(m_urlListFile));
    //m_settings->setValue("iteratorCurrentIndex", QVariant(m_iterator.cur()));
    m_settings->setValue("sidebarSplitterSizes", m_sidebar->saveState());
    m_settings->setValue("mainSplitterSizes", m_mainSplitter->saveState());

    m_settings->endGroup();
}

void MainWindow::readSettings() {
    m_settings->beginGroup("MainWindow");

    resize(m_settings->value("size", QSize(800, 600)).toSize());
    move(m_settings->value("pos", QPoint(200, 200)).toPoint());

    m_enableJavascript = m_settings->value("enableJavascript").toBool();
    m_enableParseJavascript = m_settings->value("enableParseJavascript").toBool();
    //QWebVDom::setEnabledParseJavascript(m_enableParseJavascript);

    m_enablePlugins = m_settings->value("enablePlugins").toBool();
    m_enableImages = m_settings->value("enableImages").toBool();
    m_enableJava = m_settings->value("enableJava").toBool();

    m_hunterEnabled = m_settings->value("hunterEnabled").toBool();
    m_huntButton->setEnabled(m_hunterEnabled);

    m_hunterPath = m_settings->value("hunterPath").toString();
    m_vdomPath   = m_settings->value("vdomPath").toString();
    initHunterConfig();

    m_iteratorEnabled = m_settings->value("iteratorEnabled").toBool();
    m_urlListFile = m_settings->value("urlListFile").toString();
    initIteratorConfig();

    //m_iterator.setCur(m_settings->value("iteratorCurrentIndex", 0).toInt());
    initIterator();
    if (m_iteratorEnabled) {
        m_iterLabel->show();
    }

    m_settings->endGroup();
}

void MainWindow::closeEvent(QCloseEvent *event) {
    writeSettings();
    event->accept();
}

void MainWindow::saveHunterConfig() {
    m_hunterEnabled = m_hunterConfig->hunterEnabled();
    m_huntButton->setEnabled(m_hunterEnabled);

    //m_hunterPath = m_hunterConfig->progPath();
    m_hunterPath = m_hunterConfig->progPath();
    m_vdomPath   = m_hunterConfig->vdomPath();
    //qDebug() << "Saving hunter config... (hunter: " << m_hunterPath << ")";
}

void MainWindow::saveIteratorConfig() {
    m_iteratorEnabled = m_iteratorConfig->iteratorEnabled();
    m_iterPrevButton->setEnabled(m_iteratorEnabled);
    m_iterNextButton->setEnabled(m_iteratorEnabled);
    m_urlListFile = m_iteratorConfig->listFile();
    if (m_iteratorEnabled) {
        m_iterLabel->show();
    } else {
        m_iterLabel->hide();
    }

    initIterator();
}

void MainWindow::hunterFinished(int exitCode, QProcess::ExitStatus) {
    if (exitCode != 0) {
        QString msg = QString("Failed to spawn X Hunter %1: %2: "
                "Process returns exit code %3.")
                .arg(m_hunterPath)
                .arg(m_hunter.errorString())
                .arg(m_hunter.exitCode());
        m_itemInfoEdit->append(msg);
        QMessageBox::warning(this, tr("Hunter runner"),
                msg, QMessageBox::NoButton);
        return;
    }
    statusBar()->showMessage(
        QString("Finished running X Hunter %1. (exit code: %2)")
                .arg(m_hunterPath).arg(exitCode));


    const QVariantMap root;
    QWebFrame* frame = m_view->page()->mainFrame();

    QVariant groupsVar = root["groups"];
    if (!groupsVar.isNull() && groupsVar.canConvert<QVariantList>()) {
        QVariantList groups = groupsVar.toList();
        if ( ! groups.isEmpty() ) {
            if (!m_enableJavascript) {
                m_view->page()->settings()->setAttribute(QWebSettings::JavascriptEnabled, true);
            }
            frame->addToJavaScriptWindowObject("itemInfoEdit", m_itemInfoEdit);
            frame->addToJavaScriptWindowObject("statusBar", statusBar());

            if (!m_enableJavascript) {
                m_view->page()->settings()->setAttribute(QWebSettings::JavascriptEnabled, false);
            }
            //annotateWebPage(groups);
        }
    }

    QVariant programMeta = root["program"];
    if (!programMeta.isNull() && programMeta.canConvert<QString>()) {
        m_hunterLabel->setText(programMeta.toString());
    } else {
        m_hunterLabel->setText(tr("Unknown hunter"));
    }
    m_hunterLabel->show();

    QVariant jumpTo = root["jump_to"];
    if (!jumpTo.isNull() && jumpTo.canConvert<QVariantMap>()) {
        QVariantMap point = jumpTo.toMap();
        QVariant x = point["x"];
        QVariant y = point["y"];
        if (!x.isNull() && x.canConvert<int>() &&
                !y.isNull() && y.canConvert<int>()) {
            //qDebug() << "Scroll the page to point (" << x.toInt() <<
                //"," << y.toInt() << ")" << endl;
            frame->setScrollPosition(QPoint(x.toInt(), y.toInt()));
        }
    }

    /* pageinfo from stdout
    QVariant summary = root["summary"];
    m_pageInfoEdit->clear();
    if (!summary.isNull() && summary.canConvert<QString>()) {
        QString txt = summary.toString();
        m_pageInfoEdit->setText(txt);
    }
    */
}

QVariant MainWindow::evalJS(const QString& js) {
    return m_view->page()->mainFrame()->evaluateJavaScript(js);
}

void MainWindow::annotateWebPage(QVariantList& groups) {
    QVariantList::iterator itI;
    int i = 0;
    QString js = "window.addEventListener('mousedown',"
        "function (e) {"
          "if (window._vdom_selected) return;"
          "if (!e.target) return;"
          "if (e.target.className && "
                "/\\bvdom-result\\b/.test(e.target.className)) {"
            "window._vdom_selected = e.target;"
          "}"
        "}, true);"
        "window.addEventListener('keydown',"
          "function (e) {"
            "if (e.keyCode == 27 && window._vdom_selected) {"
              "var node = window._vdom_selected;"
              "delete window._vdom_selected;"
              "var event = document.createEvent('MouseEvents');"
              "event.initMouseEvent('mouseout', true, true);"
              "node.dispatchEvent(event);"
              "return;"
            "}"
          "}, true);";
    QVariant res = evalJS(js + "true");

    for (itI = groups.begin(); itI != groups.end(); i++, itI++) {
        QVariant groupVar = *itI;
        if (groupVar.canConvert<QVariantList>()) {
            QVariantList group = groupVar.toList();
            QVariantList::iterator itJ;
            for (itJ = group.begin(); itJ != group.end(); itJ++) {
                QVariant itemVar = *itJ;
                if (itemVar.canConvert<QVariantMap>()) {
                    QVariantMap item = itemVar.toMap();
                    //qDebug() << item << endl;
                    if (item["borderWidth"].isNull()) {
                        item["borderWidth"] = QVariant(2);
                    }
                    if (item["borderColor"].isNull()) {
                        item["borderColor"] = QVariant("red");
                    }
                    if (item["borderStyle"].isNull()) {
                        item["borderStyle"] = QVariant("solid");
                    }
                    QString js = QString(
            "var box = document.createElement('div');"
            "var style = box.style;"
            "style.borderWidth = '%1px';"
            "style.borderColor = '%2';"
            "style.borderStyle = '%3';"
            "style.position = 'absolute';"
            "style.left   = '%4px';"
            "style.top    = '%5px';"
            "style.width  = '%6px';"
            "style.height = '%7px';"
            "box.className = 'vdom-result vdom-group-%8';"
            "document.body.appendChild(box);")
                        .arg(item["borderWidth"].toString())
                        .arg(item["borderColor"].toString())
                        .arg(item["borderStyle"].toString())
                        .arg(item["x"].toInt())
                        .arg(item["y"].toInt())
                        .arg(item["w"].toInt())
                        .arg(item["h"].toInt())
                        .arg(i);
                    QVariant noHighlight = item["noHighlight"];
                    if (noHighlight.isNull() ||
                            (noHighlight.canConvert<bool>() &&
                            !noHighlight.toBool())) {
                        QVariant descVar = item["desc"];
                        if (descVar.isNull() || !descVar.canConvert<QString>()) {
                            descVar = QVariant("");
                        }
                        QVariant titleVar = item["title"];
                        if (titleVar.isNull() || !titleVar.canConvert<QString>()) {
                            titleVar = QVariant("");
                        }
                        js += QString("box.addEventListener('mouseover',"
                            "function (e) {"
                              "if (window._vdom_selected) return;"
                              "itemInfoEdit.plainText = %1;"
                              "statusBar.showMessage(%2);"
                              "nodes = document.getElementsByClassName('vdom-group-%3');"
                              "for (var i = 0; i < nodes.length; i++) {"
                                "var node = nodes[i];"
                                "node.setAttribute('_vdom_color', node.style.borderColor);"
                                "node.style.borderColor = 'yellow';"
                              "}"
                            "},"
                            "true);")
                                .arg("1")
                                .arg("2")
                                //.arg(m_webvdom->dumpStrAsJson(
                                //        descVar.toString()))
                                //.arg(m_webvdom->dumpStrAsJson(
                                //        titleVar.toString()))
                                .arg(i);
                        //qDebug() << "JSON: " << js << endl;
                        js += QString("box.addEventListener('mouseout',"
                            "function (e) {"
                              "if (window._vdom_selected) return;"
                              "nodes = document.getElementsByClassName('vdom-group-%1');"
                              "for (var i = 0; i < nodes.length; i++) {"
                                "var node = nodes[i];"
                                "node.style.borderColor = node.getAttribute('_vdom_color');"
                              "}"
                            "},"
                            "true);").arg(i);
                        //qDebug() << js << endl;
                    }
                    //qDebug() << i << ":" << j << ": " << js << endl;
                    QVariant res = evalJS(js + "true");
                    //if (!res.isNull()) {
                    //qDebug() << "res: " << res << endl;
                    //}
                }
            }
        }
    }
    m_view->update();
    m_view->page()->settings()->setAttribute(QWebSettings::JavascriptEnabled, true);
}

void MainWindow::huntOnly() {
    if (!m_hunterEnabled) {
        QMessageBox::warning(this, tr("Hunt Only"),
            QString("X Hunter is not enabled in the preferences."),
            QMessageBox::NoButton);
        return;
    }
    QString js = "var nodes = document.getElementsByClassName('vdom-result');"
        "for (var i = 0; i < nodes.length; i++) {"
          "nodes[i].style.display = 'none';"
        "} true";
    //qDebug() << js << endl;
    evalJS(js);
    //qDebug() <<  << endl;
    //m_view->update();
    //QMessageBox::warning(this, "hi", "Done!", QMessageBox::NoButton);
    loadFinished(true);
}

void MainWindow::initHunterConfig() {
    m_hunterConfig->setHunterEnabled(m_hunterEnabled);
    m_hunterConfig->setProgPath(m_hunterPath);
    m_hunterConfig->setVdomPath(m_vdomPath);
}

void MainWindow::initIteratorConfig() {
    m_iteratorConfig->setIteratorEnabled(m_iteratorEnabled);
    m_iterPrevButton->setEnabled(m_iteratorEnabled);
    m_iterNextButton->setEnabled(m_iteratorEnabled);
    if (m_iteratorEnabled) {
        m_iterLabel->show();
    } else {
        m_iterLabel->hide();
    }
    //update();
    m_iteratorConfig->setListFile(m_urlListFile);
}

void MainWindow::addUrlToList() {
    QUrl::FormattingOptions opts;
    opts |= QUrl::RemoveScheme;
    opts |= QUrl::RemoveUserInfo;
    opts |= QUrl::StripTrailingSlash;
    QString s = m_view->url().toEncoded(opts);
    s = s.mid(2);
    if (!s.isEmpty()) {
        if (!m_urlList.contains(s))
            m_urlList += s;
        m_urlCompleterModel.setStringList(m_urlList);
    }
}

void MainWindow::iterPrev() {
    int ind = m_iterator.prev();
    if (ind < 0) {
        qDebug() << "Iterator index negative: " << ind << endl;
        return;
    }
    if (ind >= m_urlList.count()) {
        m_iterator.setCur(0);
        m_iterator.setCount(m_urlList.count());
        ind = 0;
    }
    m_iterLabel->setText("Page " + QString::number(ind));
    if (ind >= 0 && ind < m_urlList.count()) {
        QString url = "http://";
        url += m_urlList[ind];
        m_urlEdit->setText(url);
        loadUrl(url);
    }
}

void MainWindow::iterNext() {
    int ind = m_iterator.next();
    if (ind < 0) {
        qDebug() << "Iterator index negative: " << ind << endl;
        return;
    }
    if (ind >= m_urlList.count()) {
        m_iterator.setCur(0);
        m_iterator.setCount(m_urlList.count());
        ind = 0;
    }
    m_iterLabel->setText("Page " + QString::number(ind));
    if (ind >= 0 && ind < m_urlList.count()) {
        QString url = "http://";
        url += m_urlList[ind];
        m_urlEdit->setText(url);
        loadUrl(url);
    }
}

void MainWindow::initIterator() {
    if (!m_iteratorEnabled) {
        return;
    }
    QFile file(m_urlListFile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("URL List File Loader"),
            QString("Failed to load url list file %1: %2")
                .arg(m_urlListFile).arg(file.errorString()),
                QMessageBox::NoButton);
        file.close();
        return;
    }
    m_urlList.clear();
    QString line = QString::fromUtf8(file.readLine(MAX_FILE_LINE_LEN));
    QRegExp emptyLinePat("^\\s*$");
    while (!line.isEmpty()) {
        if (emptyLinePat.exactMatch(line)) {
            //qDebug() << "Empty line pattern found.\n";
        } else {
            //qDebug() << "Read line " << line;
            line = line.trimmed();
            line.replace(QRegExp("^[A-Za-z]+://"), "");
            //qDebug() << "URL: " << line << endl;
            m_urlList.push_back(line);
        }
        line = QString::fromUtf8(file.readLine(MAX_FILE_LINE_LEN));
    }
    //QString json = QString::fromUtf8(file.readAll());
    //qDebug() << "RAW JSON: " << json << endl;
    file.close();
    m_iterator.setCount(m_urlList.count());
    m_iterLabel->setText("Page " + QString::number(m_iterator.cur()));
}

void MainWindow::execHunterConfig() {
    initHunterConfig();
    m_hunterConfig->exec();
}

void MainWindow::execIteratorConfig() {
    initIteratorConfig();
    m_iteratorConfig->exec();
}

void MainWindow::loadUrl(const QUrl& url) {
    //fprintf(stderr, "Loading new url...");
    QWebPage* page = m_view->page();
    //page->blockSignals(true);
    m_view->stop();
    //page->blockSignals(false);

    if (!m_enableJavascript) {
        m_view->page()->settings()->setAttribute(QWebSettings::JavascriptEnabled, false);
    }

    page->mainFrame()->load(url);
    m_view->setFocus(Qt::OtherFocusReason);
}

void MainWindow::setJSFiles(QStringList& jsFiles) {
    m_injectedJSFiles = jsFiles;
        //qDebug() << "!!! JS: " << m_injectedJS << endl;
}

