#ifndef TOOLS_WIDGET_H
#define TOOLS_WIDGET_H

#include <QWidget>
#include "Media.h"

QT_FORWARD_DECLARE_CLASS(WatermarkTool)

class ToolsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ToolsWidget(QWidget *parent = nullptr);
    ~ToolsWidget();

    void initUI();
    void initConnect();
    void setMedia(Media &media);

    WatermarkTool*      watermarkTool;
private:

    Media               mMedia;
};

#endif // TOOLS_WIDGET_H
