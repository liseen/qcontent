/*
 * Copyright 2007-2008 Benjamin C. Meyer <ben@meyerhome.net>
 * Copyright 2008 Matvey Kozhev <sikon@ubuntu.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

#include "aboutdialog.h"

#include <qdialogbuttonbox.h>
#include <qfile.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qtextedit.h>
#include <qtextstream.h>

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);
    setWindowTitle(tr("About %1").arg(qApp->applicationName()));
    //logo->setPixmap(qApp->windowIcon().pixmap(128, 128));
    name->setText(qApp->applicationName());
    version->setText(qApp->applicationVersion());
}

void AboutDialog::setupUi(QDialog *AboutDialog) {
    if (AboutDialog->objectName().isEmpty())
        AboutDialog->setObjectName(QString::fromUtf8("AboutDialog"));
    AboutDialog->resize(454, 236);
    AboutDialog->setModal(true);
    verticalLayout = new QVBoxLayout(AboutDialog);
    verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
    //logo = new QLabel(AboutDialog);
    //logo->setObjectName(QString::fromUtf8("logo"));
    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    //sizePolicy.setHeightForWidth(logo->sizePolicy().hasHeightForWidth());
    //logo->setSizePolicy(sizePolicy);
    //logo->setScaledContents(false);
    //logo->setAlignment(Qt::AlignCenter);

    //verticalLayout->addWidget(logo);

    name = new QLabel(AboutDialog);
    name->setObjectName(QString::fromUtf8("name"));
    QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Fixed);
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(name->sizePolicy().hasHeightForWidth());
    name->setSizePolicy(sizePolicy1);
    QFont font;
    font.setPointSize(16);
    font.setBold(true);
    font.setWeight(75);
    name->setFont(font);
    name->setAlignment(Qt::AlignCenter);

    verticalLayout->addWidget(name);

    version = new QLabel(AboutDialog);
    version->setObjectName(QString::fromUtf8("version"));
    sizePolicy1.setHeightForWidth(version->sizePolicy().hasHeightForWidth());
    version->setSizePolicy(sizePolicy1);
    QFont font1;
    font1.setPointSize(12);
    font1.setBold(false);
    font1.setWeight(50);
    version->setFont(font1);
    version->setAlignment(Qt::AlignCenter);

    verticalLayout->addWidget(version);

    description = new QLabel(AboutDialog);
    description->setObjectName(QString::fromUtf8("description"));
    description->setAlignment(Qt::AlignCenter);

    verticalLayout->addWidget(description);

    author = new QLabel(AboutDialog);
    author->setObjectName(QString::fromUtf8("author"));
    sizePolicy1.setHeightForWidth(author->sizePolicy().hasHeightForWidth());
    author->setSizePolicy(sizePolicy1);
    author->setAlignment(Qt::AlignCenter);
    author->setOpenExternalLinks(true);

    verticalLayout->addWidget(author);

    sitelink = new QLabel(AboutDialog);
    sitelink->setObjectName(QString::fromUtf8("sitelink"));
    sizePolicy1.setHeightForWidth(sitelink->sizePolicy().hasHeightForWidth());
    sitelink->setSizePolicy(sizePolicy1);
    sitelink->setAlignment(Qt::AlignCenter);
    sitelink->setOpenExternalLinks(true);

    verticalLayout->addWidget(sitelink);

    horizontalLayout = new QHBoxLayout();
    horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
    horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    horizontalLayout->addItem(horizontalSpacer);

    closeButton = new QPushButton(AboutDialog);
    closeButton->setObjectName(QString::fromUtf8("closeButton"));
    closeButton->setDefault(true);

    horizontalLayout->addWidget(closeButton);

    horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    horizontalLayout->addItem(horizontalSpacer_2);


    verticalLayout->addLayout(horizontalLayout);


    retranslateUi(AboutDialog);
    QObject::connect(closeButton, SIGNAL(clicked()), AboutDialog, SLOT(accept()));

    QMetaObject::connectSlotsByName(AboutDialog);
} // setupUi

void AboutDialog::retranslateUi(QDialog *AboutDialog) {
    //logo->setText(QString());
    name->setText(QString());
    version->setText(QString());
    description->setText(QApplication::translate("AboutDialog", "QtWebKit-based VDOM Browser for X Hunter", 0, QApplication::UnicodeUTF8));
    author->setText(QApplication::translate("AboutDialog", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\"font-size:9pt;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">Copyright \302\251 2009 Agent Zhang (agentzh) &lt;<a href=\"mailto:agentzh@yahoo.cn\"><span style=\" text-decoration: underline; color:#0057ae;\">agentzh@yahoo.cn</span></a>&gt;</p></body></html>", 0, QApplication::UnicodeUTF8));
    sitelink->setText(QApplication::translate("AboutDialog", "<a href=\"http://www.eeeeworks.org/#posts/1\">http://www.eeeeworks.org</a>", 0, QApplication::UnicodeUTF8));
    closeButton->setText(QApplication::translate("AboutDialog", "Close", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(AboutDialog);
} // retranslateUi

