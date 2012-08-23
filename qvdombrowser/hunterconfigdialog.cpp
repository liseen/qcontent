#include "hunterconfigdialog.h"
//#include <QDebug>

HunterConfigDialog::HunterConfigDialog(QWidget *parent): QDialog(parent) {
    QCompleter *completer = new QCompleter(this);
    completer->setModel(new QDirModel(completer));

    QVBoxLayout* layout = new QVBoxLayout(this);
    formGroup = new QGroupBox(tr("&Enable X Hunter"), this);
    formGroup->setCheckable(true);
    formGroup->setChecked(false);

    QGridLayout* formLayout = new QGridLayout(formGroup);
    formGroup->setLayout(formLayout);
    //formGroup->setFlat(false);

    QLabel *label = new QLabel(tr("&Hunter program path"), this);
    formLayout->addWidget(label, 0, 0);

    progPathEdit = new QLineEdit(this);
    progPathEdit->setCompleter(completer);
    formLayout->addWidget(progPathEdit, 0, 1);
    //connect(progPathEdit, SIGNAL(returnPressed()), this, SLOT(browseProgFile()));
    label->setBuddy(progPathEdit);

    QPushButton* button = new QPushButton(tr("Browse..."), this);
    connect(button, SIGNAL(clicked()),
            this, SLOT(browseProgFile()));
    formLayout->addWidget(button, 0, 2);

    label = new QLabel(tr("&VDOM output path"), this);
    formLayout->addWidget(label, 1, 0);

    vdomPathEdit = new QLineEdit(this);
    vdomPathEdit->setCompleter(completer);
    formLayout->addWidget(vdomPathEdit, 1, 1);
    //connect(vdomPathEdit, SIGNAL(returnPressed()), this, SLOT(browseVdomFile()));
    label->setBuddy(vdomPathEdit);

    button = new QPushButton(tr("Browse..."), this);
    connect(button, SIGNAL(clicked()),
            this, SLOT(browseVdomFile()));
    formLayout->addWidget(button, 1, 2);

    formLayout->setSpacing(20);

    layout->addWidget(formGroup);

    QHBoxLayout* buttonsLayout = new QHBoxLayout;
    buttonsLayout->addSpacing(450);

    button = new QPushButton(tr("&Save"), this);
    connect(button, SIGNAL(clicked()),
            this, SLOT(accept()));
    buttonsLayout->addWidget(button);

    button = new QPushButton(tr("&Cancel"), this);
    connect(button, SIGNAL(clicked()),
            this, SLOT(reject()));
    buttonsLayout->addWidget(button);
    buttonsLayout->addStretch();

    layout->addLayout(buttonsLayout);
    //layout->addStretch();

    setLayout(layout);
    setFixedSize(QSize(700, 200));
    setWindowTitle(tr("X Hunter Configuration"));
}

void HunterConfigDialog::accept() {
    //qDebug() << "Checking form values...\n";
    if (formGroup->isChecked()) {
        QString progPath = progPathEdit->text().trimmed();
        if (progPath.isEmpty()) {
            croak(tr("Hunter Program Path is empty."));
            progPathEdit->selectAll();
            return;
        }
        if (! QFile::exists(progPath)) {
            croak(tr("Hunter Program File \"%1\" not found.").arg(progPath));
            progPathEdit->selectAll();
            return;
        }
        QFile::Permissions perms = QFile::permissions(progPath);
        if (! (perms & QFile::ExeUser)) {
            croak(tr("Hunter Program File \"%1\" is not executable.").arg(progPath));
            progPathEdit->selectAll();
            return;
        }

        QString vdomPath = vdomPathEdit->text().trimmed();
        if (vdomPath.isEmpty()) {
            croak(tr("VDOM Output File Path is empty."));
            vdomPathEdit->selectAll();
            return;
        }
        if (QFile::exists(vdomPath)) {
            //qDebug() << "VDOM Path " << vdomPath << " exists.\n";
            perms = QFile::permissions(vdomPath);
            if (! (perms & QFile::WriteUser)) {
                croak(tr("VDOM Output File \"%1\" is not writable.").arg(vdomPath));
                vdomPathEdit->selectAll();
                return;
            }
            //qDebug() << "VDOM Path " << vdomPath << " is writable.\n";
        }
    }
    QDialog::accept();
}

void HunterConfigDialog::browseProgFile() {
     const QString& fileName = QFileDialog::getOpenFileName(
         this, tr("Hunter Program File"),
         0, tr("Executable files (*.bat *.pl *.sh *.exe);;Any Files (*)"));
     if (!fileName.isEmpty()) {
         progPathEdit->setText(fileName);
     }
}

void HunterConfigDialog::browseVdomFile() {
     const QString& fileName = QFileDialog::getSaveFileName(
         this, tr("VDOM Output File"),
         0, tr("Text Files (*.txt *.dom *.vdom)"));
     if (!fileName.isEmpty()) {
         vdomPathEdit->setText(fileName);
     }
}

