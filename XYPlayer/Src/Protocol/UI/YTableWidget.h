#ifndef YTABLEMODEL_H
#define YTABLEMODEL_H

#include <QTableWidget>

class YTableWidget : public QTableWidget
{
    Q_OBJECT
public:
    explicit YTableWidget(QWidget *parent = nullptr);
    void initUI();
    void updateTable();
    void readTable();
public slots:
    void setData(QMap<QString,QVariant> &data);
    void getData(QMap<QString, QVariant> &data);
public:
    QMap<QString,QVariant> mData;
};

#endif // YTABLEMODEL_H
