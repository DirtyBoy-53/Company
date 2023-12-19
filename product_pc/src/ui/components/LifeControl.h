#ifndef LIFECONTROL_H
#define LIFECONTROL_H

#include <QWidget>
#include <QLabel>
#include <QMap>
#include "lifecontrolsetup.h"

namespace Ui {
class LifeControl;
}


class LifeControl : public QLabel
{
    Q_OBJECT
public:
    explicit LifeControl(QWidget *parent = nullptr);
    ~LifeControl();
    void mouseDoubleClickEvent(QMouseEvent *event);
signals:
    void sigOutLimitation(bool lifeOver);

public slots:
    void updateStatusLife(bool usrPlusOne = true);
private:

private:
    Ui::LifeControl *ui;
    LifeControlSetup* m_lifeSetup;
};

#endif // LIFECONTROL_H
