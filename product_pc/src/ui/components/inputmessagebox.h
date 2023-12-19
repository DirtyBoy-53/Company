#ifndef INPUTMESSAGEBOX_H
#define INPUTMESSAGEBOX_H

#include <QWidget>
#include <CButtonSelector.h>
#include <thread>

namespace Ui {
class InputMessageBox;
}

typedef enum {
    InputBox_Input = 0,
    InputBox_NgTips = 1,
    InputBox_OkNg  = 2,
    InputBox_OkTips = 3,
    InputBox_OkTips1 = 4,
    InputBox_ShowOnly = 5
}InputBox_Status;

class InputMessageBox : public QWidget
{
    Q_OBJECT

public:
    explicit InputMessageBox(QWidget *parent = nullptr);
    ~InputMessageBox();

    void setMsg(QString sMesg, int type = 0);

protected:
    void keyPressEvent(QKeyEvent *event);

signals:
    void signalConfirmClicked(QString txt);
    void signalClickedFinish(bool result);
	
private slots:

    void on_btnSuc_clicked();

    void on_btnFail_clicked();

private:
    void onOut();


private:
    Ui::InputMessageBox *ui;
    QString m_msg = "";
    //bool m_useSelector = false;
    bool m_notifyStop = false;
    //CButtonSelector* m_btnSelector;
    std::thread* m_threadBtn;
    // QWidget interface
    int m_type;
protected:
    void closeEvent(QCloseEvent *event);

    // QWidget interface
protected:
    void showEvent(QShowEvent *event);
};

#endif // INPUTMESSAGEBOX_H
