#ifndef WATERMARK_TOOL_H
#define WATERMARK_TOOL_H

#include <QWidget>
#include <QList>
#include "YStruct.h"

QT_FORWARD_DECLARE_CLASS(QStandardItemModel)
QT_FORWARD_DECLARE_CLASS(QTableWidget)



class WatermarkTool : public QWidget
{
    Q_OBJECT

public:
    explicit WatermarkTool(QWidget *parent = nullptr);
    ~WatermarkTool();
    void initUI();
    void initConnect();

private slots:
    void on_btnAdd_clicked();

    void on_btnDelete_clicked();

    void on_btnConfirm_clicked();
signals:
    void sigWatermark(QList<CustomWatermarkInfo> &list);

private:
    QTableWidget*               mTableWidget;
    QStandardItemModel*         m_model;
    QList<CustomWatermarkInfo>  m_lastWatermark;
    QList<CustomWatermarkInfo>  m_curWatermark;
};

#endif // WATERMARK_TOOL_H
