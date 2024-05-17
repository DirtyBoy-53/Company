#ifndef CANVASWIDGET_H
#define CANVASWIDGET_H

#include <QScrollArea>
#include <QUndoGroup>
#include <QAction>
#include "CanvasBase.h"
#include "Canvas2D.h"
#include "labelmanager.h"

class CanvasWidget : public QScrollArea
{
    Q_OBJECT
public:
    explicit CanvasWidget(QWidget *parent = nullptr);
    ~CanvasWidget();

    qreal scaleFitWindow() const;
    void adjustFitWindow();

    CanvasBase *canvas() const;
    QUndoGroup *undoGroup() const;

public slots:
    void addShape();
    void removeShape();
    void addPoint(const QPointF &point);
    void removePoint();
    void saveFile();
protected:
    void initUI();
    void initConnect();
signals:
public slots:
    void loadPixmap();
private:
    Canvas2D *m_doc{nullptr};
    QUndoGroup *m_undoGroup{nullptr};
    LabelManager m_labelManager;



    // QWidget interface
protected:
    virtual void wheelEvent(QWheelEvent *event) override;
};

#endif // CANVASWIDGET_H
