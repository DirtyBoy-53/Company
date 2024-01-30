#ifndef TOOLVECTOR_H
#define TOOLVECTOR_H
#include <QWidget>
#include <QVector>
#include <QToolBar>
#include <QSignalMapper>

class ToolVector:public QWidget
{
    Q_OBJECT
public:
    ToolVector(QToolBar * tool);
    void add(QWidget * pw, QString name);
    void connectT();
private slots:
    void click(QWidget * pw);
private:
    QSignalMapper * m_mapper;
    QToolBar * m_tool;
};

#endif // TOOLVECTOR_H
