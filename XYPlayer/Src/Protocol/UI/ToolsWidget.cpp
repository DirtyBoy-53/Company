#include "ToolsWidget.h"
#include "WatermarkTool.h"

#include <QVBoxLayout>
#include <QHBoxLayout>

ToolsWidget::ToolsWidget(QWidget *parent)
{
    initUI();
    initConnect();
}

ToolsWidget::~ToolsWidget()
{
}

void ToolsWidget::initUI()
{
    watermarkTool = new WatermarkTool();
    auto layout = new QVBoxLayout(this);
    layout->addWidget(watermarkTool);
    layout->addStretch();
}

void ToolsWidget::initConnect()
{
}

void ToolsWidget::setMedia(Media &media)
{
    mMedia = media;
}
