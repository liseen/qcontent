#include "viwiedialog.h"


ViwieDialog::ViwieDialog(QWidget *parent)
    :QDialog(parent)
{

    QVBoxLayout *mainLayout = new QVBoxLayout;
    formGroupBox = new QGroupBox(tr("Page"));

    QFormLayout *formLayout = new QFormLayout();
    urlEditor = new QLineEdit();
    urlEditor->setText("http://www.google.com");
    urlEditor->setReadOnly(true);
    formLayout->addRow(new QLabel(tr("Url: ")), urlEditor);

    catCombo = new QComboBox();
    catCombo->addItem("cat1");
    catCombo->addItem("cat2");
    formLayout->addRow(new QLabel(tr("Category: ")), catCombo);
    fieldsTable = new QTableWidget(0, 3);
    QStringList list;
    list << tr("Field Name") << tr("Field Type") << tr("Detail");
    fieldsTable->setHorizontalHeaderLabels (list);
    formLayout->addRow(new QLabel(tr("Fields: ")), fieldsTable);

    formGroupBox->setLayout(formLayout);
    mainLayout->addWidget(formGroupBox);

    buttonsGroupBox = new QGroupBox(tr("Process: "));
    QHBoxLayout *hboxLayout = new QHBoxLayout;
    QPushButton *tmpSaveBtn = new QPushButton(tr("Temp Save"));
    QPushButton *permSaveBtn = new QPushButton(tr("Permanent Save"));
    QPushButton *cancelBtn = new QPushButton(tr("Cancel"));
    connect(cancelBtn, SIGNAL(clicked()), this, SLOT(reject()));

    hboxLayout->addWidget(tmpSaveBtn);
    hboxLayout->addWidget(permSaveBtn);
    hboxLayout->addWidget(cancelBtn);

    buttonsGroupBox->setLayout(hboxLayout);

    mainLayout->addWidget(buttonsGroupBox);
    setLayout(mainLayout);
    setWindowTitle(tr("Vision-based Web Information Extraction "));
}

QVariant ViwieDialog::getValue()
{

    return viwieJson;
}

bool ViwieDialog::refresh(const QVariant& viwieTags)
{
    QVariantMap viwieMap = viwieTags.toMap();

    QString url = viwieMap["url"].toString();
    urlEditor->setText(url);

    fieldsTable->clear();
    QVariantList fieldsList = viwieMap["fields"].toList();
    int len = fieldsList.length();
    for (int i = 0; i < len; i++) {
        QVariantMap fieldMap = fieldsList.at(i).toMap();

        QString name = fieldMap["name"].toString();
        QString type = fieldMap["type"].toString();
        fieldsTable->setRowCount(fieldsTable->rowCount() + 1);

        fieldsTable->setItem(i, 0, new  QTableWidgetItem(name));
        fieldsTable->setItem(i, 1, new  QTableWidgetItem(type));
    }

    return true;
}
