#ifndef XYALGORITHMDEBUG_H
#define XYALGORITHMDEBUG_H

#include <QDialog>

namespace Ui {
class XyAlgorithmDebug;
}

class XyAlgorithmDebug : public QDialog
{
    Q_OBJECT

public:
    explicit XyAlgorithmDebug(QWidget *parent = nullptr);
    ~XyAlgorithmDebug();

private slots:
    void on_btnStartRecordFT_clicked();

    void on_btnStopRecordFT_clicked();

    void on_btnStartCollectY16_clicked();

    void on_btnStopCollectY16_clicked();

    void on_btnCollectMtf_clicked();

private:
    Ui::XyAlgorithmDebug *ui;
};

#endif // XYALGORITHMDEBUG_H
