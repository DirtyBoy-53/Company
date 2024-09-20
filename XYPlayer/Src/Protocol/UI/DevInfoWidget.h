#ifndef DEVINFOWIDGET_H
#define DEVINFOWIDGET_H

#include <QWidget>
#include "Media.h"

QT_FORWARD_DECLARE_CLASS(QLineEdit)
QT_FORWARD_DECLARE_CLASS(QPushButton)

class DevInfoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DevInfoWidget(QWidget *parent = nullptr);
    ~DevInfoWidget();

    void initUI();
    void initConnect();
    void setMedia(Media &media);
signals:
public slots:
    void on_btn_clicked();
private:
    QLineEdit*      mPlatformType;
    QLineEdit*      mAsicVersion;
    QLineEdit*      mReolution;
    QLineEdit*      mProductName;

    QPushButton*    mBtnRead;

    Media           mMedia;
};

#endif // DEVINFOWIDGET_H
