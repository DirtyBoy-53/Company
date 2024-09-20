#include "CentralWidget.h"
#include "MulWindow.h"
#include <QSplitter>
#include <QHBoxLayout>
#include "YFunction.h"
#include "styles.h"
#include "LSideWindow.h"
CentralWidget::CentralWidget(QWidget *parent) : QWidget(parent)
{
    initUI();
    initConnect();
}

CentralWidget::~CentralWidget()
{

}

void CentralWidget::initUI()
{
    m_lsideWind = new LSideWindow;
    m_rsideWind = new QWidget;
    m_mulWind = new MulWindow;

    QSplitter *split = new QSplitter(Qt::Horizontal);
    split->addWidget(m_lsideWind);
    split->addWidget(m_mulWind);
    split->addWidget(m_rsideWind);

    m_lsideWind->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    m_lsideWind->setMinimumWidth(LSIDE_MIN_WIDTH);
    m_lsideWind->setMaximumWidth(LSIDE_MAX_WIDTH);

    m_mulWind->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_mulWind->setMinimumWidth(MV_MIN_WIDTH);
    m_mulWind->setMinimumHeight(MV_MIN_HEIGHT);

    m_rsideWind->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    m_rsideWind->setMinimumWidth(RSIDE_MIN_WIDTH);
    m_rsideWind->setMaximumWidth(RSIDE_MAX_WIDTH);

    split->setStretchFactor(0, 1);
    split->setStretchFactor(1, 1);
    split->setStretchFactor(2, 1);

    m_lsideWind->setVisible(false);
    m_rsideWind->setVisible(false);

    QHBoxLayout *hbox = genHBoxLayout();
    hbox->addWidget(split);
    setLayout(hbox);
}

void CentralWidget::initConnect()
{

}
