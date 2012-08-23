#ifndef FIELD_DIALOG_H
#define FIELD_DIALOG_H

#include <QDialog>
#include <QtGui>
#include <QVariant>

QT_BEGIN_NAMESPACE
class QAction;
class QDialoButtonBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QMenu;
class QMenuBar;
class QPushButton;
class QTextEdit;
class QTableWidget;
QT_END_NAMESPACE

class FieldDialog : public QDialog
{
    Q_OBJECT

public:
    FieldDialog(QWidget *parent = 0);
    bool refresh(const QVariant&);
    QVariant getValue();

signals:
    void addField();
    void deleteField();
    void overrideField();

private:
    QGroupBox *buttonsGroupBox;
    QGroupBox *formGroupBox;

    QPushButton *addBtn;
    QPushButton *deleteBtn;
    QPushButton *overrideBtn;
    QPushButton *cancelBtn;

    QTextEdit *selectedText;
    QTextEdit *wantText;
    QComboBox *nameCombo;
    QComboBox *typeCombo;

    QList<QComboBox> *fieldTypeComboList;
    QTableWidget *filtersTable;

    QTableWidget *xpathsTable;
    QDialoButtonBox *buttonBox;

    QVariant fieldJson;
};

#endif
