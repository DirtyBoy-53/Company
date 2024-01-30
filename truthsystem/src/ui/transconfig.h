#ifndef TRANSCONFIG_H
#define TRANSCONFIG_H

#include <QWidget>
#include <QVector>
#include <QSignalMapper>
#include <QMutex>
#include "ArtosynNet.h"
#include "sharedmembuffer.h"
#include <QProcess>



namespace Ui {
class TransConfig;
}


class TransConfig : public QWidget
{
    Q_OBJECT

public:
    explicit TransConfig(QWidget *parent = 0);
    ~TransConfig();

private slots:

    void on_connectTypeChanged(QWidget *w);

    void on_connectProChanged(QWidget *w);
    void on_connectEditChanged(QWidget *w);
    void onWCEditChanged(QWidget *w);
    void onAlgoEditChanged(QWidget *w);
    void onBtnClicked(QWidget* w);
signals:
    void sigUiChange(int idx);
private:
    void removeItem(int row);
    int getRowIdx(QWidget * w, int col);

    Ui::TransConfig *ui;

    QVector <QString> m_configList;
    QSignalMapper * m_mapper;
    QSignalMapper * m_mapperLine;
    QSignalMapper * m_mapModeSwitch;
    QSignalMapper * m_mapWC;
    QSignalMapper * m_mapAlgo;
    ArtosynNet* m_artosVideo;
    QProcess m_pythonProcess;
};

#endif // TRANSCONFIG_H
