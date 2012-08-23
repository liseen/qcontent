#ifndef ITERATOR_CONFIG_DIALOG_H
#define ITERATOR_CONFIG_DIALOG_H

#include <qdialog.h>
#include <QtGui>
//#include <QDebug>

class IteratorConfigDialog: public QDialog {
    Q_OBJECT

public:
    IteratorConfigDialog(QWidget *parent = 0);

    void setIteratorEnabled(bool enabled) {
        m_formGroup->setChecked(enabled);
    }

    void setListFile(const QString& path) {
        m_listFileEdit->setText(path.trimmed());
    }

    bool iteratorEnabled() {
        return m_formGroup->isChecked();
    }

    QString listFile() const {
        return m_listFileEdit->text().trimmed();
    }

public slots:
    virtual void accept();
    void browseListFile();

private:
    void croak(const QString& msg) {
        QMessageBox::warning(this, tr("X Hunter Configuration"),
            msg, QMessageBox::NoButton);
    }
    QLineEdit* m_listFileEdit;
    QGroupBox* m_formGroup;
};

#endif // HUNTER_CONFIG_DIALOG_H

