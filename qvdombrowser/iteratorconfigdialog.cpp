#include "iteratorconfigdialog.h"
//#include <QDebug>

IteratorConfigDialog::IteratorConfigDialog(QWidget *parent): QDialog(parent) {
    QCompleter *completer = new QCompleter(this);
    completer->setModel(new QDirModel(completer));

    QVBoxLayout* layout = new QVBoxLayout(this);
    m_formGroup = new QGroupBox(tr("&Enable URL iterator"), this);
    m_formGroup->setCheckable(true);
    m_formGroup->setChecked(false);

    QGridLayout* formLayout = new QGridLayout(m_formGroup);
    m_formGroup->setLayout(formLayout);
    //m_formGroup->setFlat(false);

    QLabel *label = new QLabel(tr("&URL list file"), this);
    formLayout->addWidget(label, 0, 0);

    m_listFileEdit = new QLineEdit(this);
    m_listFileEdit->setCompleter(completer);
    formLayout->addWidget(m_listFileEdit, 0, 1);
    label->setBuddy(m_listFileEdit);

    QPushButton* button = new QPushButton(tr("Browse..."), this);
    connect(button, SIGNAL(clicked()),
            this, SLOT(browseListFile()));
    formLayout->addWidget(button, 0, 2);

    formLayout->setSpacing(20);

    layout->addWidget(m_formGroup);

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
    setFixedSize(QSize(700, 150));
    setWindowTitle(tr("URL Iterator Configuration"));
}

void IteratorConfigDialog::accept() {
    //qDebug() << "Checking form values...\n";
    if (m_formGroup->isChecked()) {
        QString listFile = m_listFileEdit->text().trimmed();
        if (listFile.isEmpty()) {
            croak(tr("URL list file path is empty."));
            m_listFileEdit->selectAll();
            return;
        }
        if (! QFile::exists(listFile)) {
            croak(tr("URL list file \"%1\" not found.").arg(listFile));
            m_listFileEdit->selectAll();
            return;
        }
        QFile::Permissions perms = QFile::permissions(listFile);
        if (! (perms & QFile::ReadUser)) {
            croak(tr("URL list file is not readable."));
            m_listFileEdit->selectAll();
            return;
        }
    }
    QDialog::accept();
}

void IteratorConfigDialog::browseListFile() {
     const QString& fileName = QFileDialog::getOpenFileName(
         this, tr("URL List File"),
         0, tr("Executable files (*.txt *.list);;Any Files (*)"));
     if (!fileName.isEmpty()) {
         m_listFileEdit->setText(fileName);
     }
}

