/*
 * Copyright 2007-2008 Benjamin C. Meyer <ben@meyerhome.net>
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

#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QtGui>
#include <qdialog.h>

class AboutDialog : public QDialog {
    Q_OBJECT

public:
    AboutDialog(QWidget *parent = 0);

private:
    QVBoxLayout *verticalLayout;
    QLabel *logo;
    QLabel *name;
    QLabel *version;
    QLabel *description;
    QLabel *author;
    QLabel *sitelink;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer;
    QPushButton *authorsButton;
    QPushButton *licenseButton;
    QPushButton *closeButton;
    QSpacerItem *horizontalSpacer_2;

protected:
    void setupUi(QDialog *AboutDialog);
    void retranslateUi(QDialog *AboutDialog);
};

#endif // ABOUTDIALOG_H

