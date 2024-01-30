#ifndef EVENTWIDGET_H
#define EVENTWIDGET_H

#include <QWidget>
struct BtnClickTimes {
    int btnReboot = 0;
    int btnAlarmError = 0;
    int btnAlarmMissed = 0;
    int btnCheckError = 0;
    int btnCheckMissed = 0;
    int btnCustom = 0;
};
namespace Ui {
class EventWidget;
}

class EventWidget : public QWidget
{
    Q_OBJECT

public:
    explicit EventWidget(QWidget *parent = nullptr);
    ~EventWidget();
    void addTag(QString value, int index);
    QString createFileDir(QString filePath);
private slots:
    void slot_btn_clicked();
private:
    Ui::EventWidget *ui;
    QString m_tagFile;
    BtnClickTimes m_times;
};

#endif // EVENTWIDGET_H
