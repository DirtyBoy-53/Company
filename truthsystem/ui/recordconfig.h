#ifndef RECORDCONFIG_H
#define RECORDCONFIG_H

#include <QWidget>
#include "recorder.h"

#define MAXREC 8

namespace Ui {
class RecordConfig;
}

class RecordConfig : public QWidget
{
    Q_OBJECT

public:
    explicit RecordConfig(QWidget *parent = 0);
    ~RecordConfig();

private slots:
    void on_pushButton_clicked();

    void on_btSelFile_clicked();

private:
    Ui::RecordConfig *ui;
    Recorder m_recorder[MAXREC];
};

#endif // RECORDCONFIG_H
