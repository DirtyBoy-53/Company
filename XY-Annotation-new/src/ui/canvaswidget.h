#ifndef CANVASWIDGET_H
#define CANVASWIDGET_H

#include <QScrollArea>
#include <QUndoGroup>
#include <QAction>
#include "documentbase.h"
#include "document2d.h"
#include "labelmanager.h"
class CanvasWidget : public QScrollArea
{
    Q_OBJECT
public:
    explicit CanvasWidget(QWidget *parent = nullptr);
    ~CanvasWidget();

    qreal scaleFitWindow() const;
    void adjustFitWindow();
public slots:
    void addShape();
    void removeShape();
protected:
    void initUI();
    void initConnect();
signals:
public slots:
    void loadPixmap();
public:
    Document2D *m_doc{nullptr};
    QUndoGroup *m_undoGroup{nullptr};
    LabelManager m_labelManager;
};

#endif // CANVASWIDGET_H
