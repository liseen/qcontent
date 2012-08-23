#ifndef ITERATOR_H
#define ITERATOR_H

#include <QDebug>

class Iterator {
public:
    Iterator() {
        m_count = 0;
        m_cur = -1;
    }

    void setCount (int count) {
        //qDebug() << "Setting count to " << count << endl;
        m_count = count;
        m_cur = -1;
    }

    int prev() {
        if (m_count <= 0) {
            return (m_cur = -1);
        }
        if (m_cur == -1 || m_cur == 0) {
            return (m_cur = m_count - 1);
        }
        return --m_cur;
    }

    int next() {
        if (m_count <= 0) {
            return (m_cur = -1);
        }
        if (m_cur == -1 || m_cur == m_count - 1) {
            return (m_cur = 0);
        }
        return ++m_cur;
    }

    int cur() {
        return m_cur;
    }

    void setCur(int cur) {
        m_cur = cur;
    }

private:
    int m_cur;
    int m_count;
};

#endif // ITERATOR_H

