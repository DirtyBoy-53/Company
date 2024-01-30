#include "toolvector.h"
#include <QPushButton>
#include <QSignalMapper>

ToolVector::ToolVector(QToolBar * tool)
{
    m_mapper = new QSignalMapper(tool);
    m_tool = tool;
}

void ToolVector::add(QWidget *pw, QString name)
{
    QPushButton * pPushButton = new QPushButton(name);
    pPushButton->setGeometry(0,0,50,60);
    connect(pPushButton, SIGNAL(clicked()),
            m_mapper, SLOT(map()));
    m_mapper->setMapping(pPushButton, pw);
    m_tool->addWidget(pPushButton);
}

void ToolVector::connectT()
{
    connect(m_mapper, SIGNAL(mapped(QWidget*)), this, SLOT(click(QWidget*)));
}

void ToolVector::click(QWidget *pw)
{
    if(pw->isHidden()) {
        pw->show();
    }else {
        pw->hide();
    }
}
