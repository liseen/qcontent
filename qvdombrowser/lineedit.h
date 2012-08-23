#ifndef LINEEDIT_H
#define LINEEDIT_H

#include <QLineEdit>

class LineEdit: public QLineEdit {
public:
   LineEdit(QWidget* const w): QLineEdit(w) {}
protected:
   //void focusInEvent(QFocusEvent *);
};

#endif

