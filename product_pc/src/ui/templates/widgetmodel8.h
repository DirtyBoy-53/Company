#ifndef WIDGETMODEL8_H
#define WIDGETMODEL8_H

#include <QWidget>
#include "basecomponent.h"

namespace Ui {
class WidgetModel8;
}

class WidgetModel8 : public BaseComponent
{
    Q_OBJECT

public:
    explicit WidgetModel8(QWidget *parent = nullptr);
    ~WidgetModel8();

public slots:
    void updatePos(const int x, const int y);
    void updateOffset(const int offset);
    void updateEnable(const bool enable);
    void updateImg(QString path);
    void updateWorkResult(QString mnm, QString reticle);

private:
    Ui::WidgetModel8 *ui;
    QString m_imgPath;
};

#endif // WIDGETMODEL8_H
