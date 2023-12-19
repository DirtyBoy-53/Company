#include "showlifelabel.h"

ShowLifeLabel::ShowLifeLabel(){
    QString style = "color:rgb(255,255,255);\nbackground-color:rgb(0,200,0);\nfont-size:16pt";
    setStyleSheet(style);
}

void ShowLifeLabel::mouseDoubleClickEvent(QMouseEvent *event)
{
    sigClicked();
}

void ShowLifeLabel::setColor(int color) {
    m_color = color;
    //        QString style = "color:rgb(255,255,255);\nfont-size:16pt";
    //        if (color == QColor::gr) {
    //            style += ";background-color:rgb(0, 200, 0)";
    //        } else if (color == QColor::yellow()) {
    //            style += ";background-color:rgb(250, 250, 0)";
    //        } else {
    //            style += ";background-color:rgb(220, 0, 0)";
    //        }
    //setStyleSheet(style);
}
