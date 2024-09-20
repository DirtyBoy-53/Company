#ifndef CUSTOM_WIDGET_H
#define CUSTOM_WIDGET_H

#include <QWidget>
#include <QFrame>
#include "Media.h"


QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QLineEdit)
QT_FORWARD_DECLARE_CLASS(QPushButton)
QT_FORWARD_DECLARE_CLASS(QTextEdit)


class I2CRegisterOptBar : public QFrame
{
public:
    enum RegOpt{
        REG_OPT_READ,   // 读取操作
        REG_OPT_WRITE,  // 写入操作
    };

    explicit I2CRegisterOptBar(QWidget *parent=nullptr);
    ~I2CRegisterOptBar();

    void setRegOpt(RegOpt opt);

    void initUI();
    void updateUI();
    void initConnect();
    

    QLabel*         mDevAddrLabel;
    QLabel*         mRegAddrLabel;
    QLabel*         mDataLabel;
    QLabel*         mlabelTips;

    QLineEdit*      mDevAddr;
    QLineEdit*      mRegAddr;
    QLineEdit*      mData;
    QPushButton*    mBtn;

    QTextEdit*      mTextBox;

    RegOpt          mRegOpt{REG_OPT_READ};


};

QT_FORWARD_DECLARE_CLASS(QCheckBox)

class CustomWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CustomWidget(QWidget *parent = nullptr);
    ~CustomWidget();

    void initUI();
    void initConnect();
    void setMedia(Media &media);
public slots:
    void on_checkBox_clicked(bool cleck);
    void on_btn_clicked();
private:
    I2CRegisterOptBar* mRegOptBar;

    // 读写切换
    QCheckBox*      checkBox_read;
    QCheckBox*      checkBox_write;

    Media           mMedia;
};


#endif // CUSTOM_WIDGET_H
