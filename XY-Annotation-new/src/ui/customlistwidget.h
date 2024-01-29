#ifndef CUSTOMLISTWIDGET_H
#define CUSTOMLISTWIDGET_H

#include <QListWidget>
#include <QString>
#include <QColor>


class CustomListWidget : public QListWidget
{
public:
    CustomListWidget(QWidget *parent=nullptr);

protected:
    void mousePressEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);

public slots:
    void addCustomItem(QString label, QColor color, bool checked);
    void insertCustomItem(QString label, QColor color, bool checked, int idx);

    void addCustomItemUncheckable(QString label, QColor color);

    void insertCustomItemUncheckable(QString label, QColor color, int idx);

    void removeCustomItem(QString label);

    void changeIconColor(QString label,QColor color);

    void changeCheckState(QString label, bool checked);

    void removeCustomItemByIdx(int idx);

    void changeIconColorByIdx(int idx,QColor color);

    void changeTextByIdx(int idx, QString text);

private:
    QListWidgetItem* _findItemByText(QString label);
};

#endif // CUSTOMLISTWIDGET_H
