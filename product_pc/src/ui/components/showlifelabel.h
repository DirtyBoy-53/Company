#ifndef SHOWLIFELABEL_H
#define SHOWLIFELABEL_H

#include <QLabel>

class ShowLifeLabel : QLabel {
    Q_OBJECT
public:
    ShowLifeLabel();

    void mouseDoubleClickEvent(QMouseEvent* event);

    void setColor(int color);
signals:
    void sigClicked();
private:
    // int m_level; // 0 manager, 1 farmer
    int m_color; // red, green, yellow
};

#endif // SHOWLIFELABEL_H
