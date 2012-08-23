#ifndef PAGE_TAG_DIALOG_H
#define PAGE_TAG_DIALOG_H

#include <QDialog>
#include <QtGui>
#include <QVariant>

QT_BEGIN_NAMESPACE
class QAction;
class QDialogButtonBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QMenu;
class QMenuBar;
class QPushButton;
class QTextEdit;
class QTableWidget;
QT_END_NAMESPACE

class ViwieDialog : public QDialog
{
    Q_OBJECT

public:
    ViwieDialog(QWidget *parent = 0);

    bool refresh(const QVariant&);
    QVariant getValue();

public slots:
    void emitShowDetailField() {
        QVariant detail;

        emit showDetailField(detail);
    }

signals:
    void permanentSave();
    void tmpSave();

    void showDetailField(const QVariant& detail);

private:
    QGroupBox *formGroupBox;
    QGroupBox *buttonsGroupBox;

    QLineEdit *urlEditor;
    QComboBox *catCombo;

    QTableWidget *fieldsTable;
    QPushButton *tmpSaveBtn;
    QPushButton *permSaveBtn;
    QPushButton *cancelBtn;

    QDialogButtonBox *buttonBox;

    QVariant viwieJson;
};

#endif
