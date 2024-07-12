#ifndef DOCUMENT2D_H
#define DOCUMENT2D_H
#include <QImage>

#include "CanvasBase.h"

//QT_FORWARD_DECLARE_CLASS(QGraphicsSceneMouseEvent)


class Canvas2D : public CanvasBase
{
public:
    Canvas2D(QObject* parent = nullptr);

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
    virtual void wheelEvent(QGraphicsSceneWheelEvent* event) override;
    virtual void hoverMoveEvent(QGraphicsSceneHoverEvent* event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    virtual void keyPressEvent(QKeyEvent* event) override;
    virtual void drawShape(QPainter* painter);

};

#endif // DOCUMENT2D_H
