#ifndef OPENMEDIADLG_H
#define OPENMEDIADLG_H

#include <QDialog>
#include <QMap>
#include "Media.h"

QT_FORWARD_DECLARE_CLASS(QTabWidget)
QT_FORWARD_DECLARE_CLASS(QLineEdit)
QT_FORWARD_DECLARE_CLASS(QPushButton)
QT_FORWARD_DECLARE_CLASS(QComboBox)

class FileTab : public QWidget
{
    Q_OBJECT
public:
    explicit FileTab(int id, QWidget *parent = nullptr);

public:
    QLineEdit*      edit;
    QPushButton*    btnBrowse;
    int             m_id;
    int             m_channle{0};
};

class NetworkTab : public QWidget
{
    Q_OBJECT
public:
    explicit NetworkTab(int id, QWidget *parent = nullptr);

public:
    QLineEdit*      edit;
    int             m_id;
    int             m_channle{0};
};

class PcieCaptureTab : public QWidget
{
    Q_OBJECT
public:
    explicit PcieCaptureTab(int id, QWidget *parent = nullptr);
    void initConnect();

public:
    QMap<QString,QString>       m_confMap;
    QComboBox*                  m_confBox{nullptr};
    QComboBox*                  m_channelBox{nullptr};
    QString                     m_chooseConf;
    QString                     m_curConf;
    int                         m_chooseCh{0};
    int                         m_id;
};


class OpenMediaDlg : public QDialog
{
    Q_OBJECT
public:
    explicit OpenMediaDlg(int id, QWidget *parent = nullptr);

public slots:
    virtual void accept();

protected:
    void initUI();
    void initConnect();

public:
    QTabWidget*     tab{nullptr};
    Media           media;
    int             m_id;
};

#endif // OPENMEDIADLG_H
