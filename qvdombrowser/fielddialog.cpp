
#include <QtGui>

#include "fielddialog.h"

FieldDialog::FieldDialog(QWidget* parent) : QDialog(parent)
{

    QList<QComboBox> *fieldTypeComboList = new QList<QComboBox>();
    QVBoxLayout *mainLayout = new QVBoxLayout;

    formGroupBox = new QGroupBox(tr("Field"));
    QFormLayout *formLayout = new QFormLayout;

    selectedText = new QTextEdit();
    selectedText->setReadOnly(true);
    formLayout->addRow(new QLabel(tr("Text:")), selectedText);

    wantText = new QTextEdit();
    formLayout->addRow(new QLabel(tr("Want:")), wantText);

    nameCombo = new QComboBox();
    formLayout->addRow(new QLabel(tr("Name: ")), nameCombo);

    typeCombo = new QComboBox();
    typeCombo->addItem("image");
    typeCombo->addItem("text");

    typeCombo->setEditable(false);

    formLayout->addRow(new QLabel(tr("Type: ")), typeCombo);

    filtersTable = new QTableWidget(0, 3);
    QStringList list;
    list << tr("Filter Type") << tr("Filter Arg") << tr("Delete");
    filtersTable->setHorizontalHeaderLabels (list);
    formLayout->addRow(new QLabel(tr("Filters: ")), filtersTable);

    xpathsTable = new QTableWidget(0, 3);
    xpathsTable->setHorizontalHeaderLabels ( QStringList() << tr("Xpath") << tr("Start") << tr("End"));
    formLayout->addRow(new QLabel(tr("Xpath: ")), xpathsTable);
/*
    fieldTypeCombo = new QComboBox();
    fieldTypeCombo->addItem(tr("suffix"));
    fieldTypeCombo->addItem(tr("prefix"));
    qDebug() << filtersTable->rowCount();
    qDebug() << filtersTable->currentRow();
    qDebug() << filtersTable->currentColumn();

    filtersTable->setCellWidget(0, 0, fieldTypeCombo);

    filtersTable->setCellWidget(1, 0, new QComboBox(fieldTypeCombo));
    filtersTable->setCellWidget(2, 0, new QComboBox(fieldTypeCombo));
    */


    formGroupBox->setLayout(formLayout);

    mainLayout->addWidget(formGroupBox);

    buttonsGroupBox = new QGroupBox(tr("Process: "));
    QHBoxLayout *hboxLayout = new QHBoxLayout;
    addBtn = new QPushButton(tr("Add"));
    hboxLayout->addWidget(addBtn);
    overrideBtn = new QPushButton(tr("Override"));
    hboxLayout->addWidget(overrideBtn);
    deleteBtn = new QPushButton(tr("Delete"));
    hboxLayout->addWidget(deleteBtn);

    cancelBtn = new QPushButton(tr("Cancel"));
    connect(cancelBtn, SIGNAL(clicked()), this, SLOT(reject()));
    hboxLayout->addWidget(cancelBtn);
    buttonsGroupBox->setLayout(hboxLayout);

    mainLayout->addWidget(buttonsGroupBox);


    setLayout(mainLayout);

    setWindowTitle(tr("Field Panel"));

    connect(addBtn, SIGNAL(clicked()), this, SIGNAL(addField()));
    connect(overrideBtn, SIGNAL(clicked()), this, SIGNAL(overrideField()));
    connect(deleteBtn, SIGNAL(clicked()), this, SIGNAL(deleteField()));


}

bool FieldDialog::refresh(const QVariant& selected)
{
    qDebug() << selected;
    Q_ASSERT(selected.toMap().type() == QVariant::Map);

    fieldJson = selected;

    QVariantMap fieldMap = selected.toMap();

    QString type = fieldMap["type"].toString();
    qDebug() << "type:" << type;
    typeCombo->setCurrentIndex(typeCombo->findText(type));

    QString text = fieldMap["text"].toString();
    selectedText->setPlainText(text);

    QString want = fieldMap["want"].toString();
    wantText->setPlainText(want);


    xpathsTable->clear();
    QVariantList xpathList = fieldMap["xpaths"].toList();
    int len = xpathList.length();
    for (int i = 0; i < len; i++) {
        QVariant var = xpathList.at(i);

        QVariantMap xpathMap = var.toMap();

        QString xpath = xpathMap["xpath"].toString();
        int start = xpathMap["start"].toInt();
        int end = xpathMap["end"].toInt();
        xpathsTable->setRowCount(xpathsTable->rowCount() + 1);
        //xpathsTable->insertRow(1);

        xpathsTable->setItem(i, 0, new  QTableWidgetItem(xpath));
        xpathsTable->setItem(i, 1, new  QTableWidgetItem(QString::number(start)));
        xpathsTable->setItem(i, 2, new  QTableWidgetItem(QString::number(end)));
    }
    // nameCombo
    // typeCombo
    // 
    // fieldsTable
    return true;
}

QVariant FieldDialog::getValue()
{
    return fieldJson;
}
