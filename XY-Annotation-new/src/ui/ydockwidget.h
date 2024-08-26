#ifndef YDOCKWIDGET_H
#define YDOCKWIDGET_H

#include <QObject>
#include "customlistwidget.h"
#include <QDockWidget>

class YTitleWidget : public QWidget
{
    Q_OBJECT
public:
    explicit YTitleWidget(QString title="title",QWidget *parent = nullptr);
private:
    void initUI(QString title);
};

class YDockWidget : public QObject
{
    Q_OBJECT
public:
    explicit YDockWidget(QObject *parent = nullptr);




    QDockWidget &annoDockWidget();
    QDockWidget &labelDockWidget();
    QDockWidget &fileDockWidget();

    CustomListWidget &annoListWidget();
    CustomListWidget &labelListWidget();
    CustomListWidget &fileListWidget();

private:
    void initUI();
    QDockWidget m_annoDockWidget;
    QDockWidget m_labelDockWidget;
    QDockWidget m_fileDockWidget;

    CustomListWidget m_annoListWidget ;
    CustomListWidget m_labelListWidget;
    CustomListWidget m_fileListWidget ;
signals:

public slots:
};

#endif // YDOCKWIDGET_H
