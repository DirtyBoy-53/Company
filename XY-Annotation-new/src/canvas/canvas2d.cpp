#include "Canvas2D.h"
#include <QKeyEvent>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include <QCursor>
#include "shape.h"
#include "labelmanager.h"
Canvas2D::Canvas2D(QObject* parent)
    : CanvasBase(parent)
{

}

void Canvas2D::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (getImage().isNull()) return;
    QPointF pixPos = bindImgEdge(event->pos());

    if (m_task == CanvasBase::detection) {

    }
    else if (m_task == CanvasBase::segmentation) {
        if (event->button() == Qt::LeftButton) {
            if (m_operat == CanvasBase::draw) {
                auto shape = currentShape();
                if (m_currentIndex >= 0 && !shape) return;
                if (m_currentIndex == -1 ||
                    m_shapeList.count() == 0 ||
                    shape->isClosed()) {//队列为空 || 都已经封闭
                    emit sigAddShape();
                    return;
                }
                if (m_draw == YShape::Polygon) {
                    emit sigAddPoint(pixPos);
                }
            }
            else if (m_operat == CanvasBase::edit) {
                if (m_mouseDrag == true) return;
                m_activePoint = -1; m_insertPoint = -1;
                if (event->button() == Qt::LeftButton) {
                    for (ShapePtr &shape : getShapeList()) {

                        if (!shape->isClosed() || !shape->label()->m_visible) continue;

                        if (shape->isEdgeHavPt(pixPos, m_insertPoint)) {
                            m_mouseDrag = true;
                            shape->setIsPress(true);
                            shape->setControlPtActive(m_insertPoint);
                            break;
                        }

                        if (shape->isPtsHavPt(pixPos, m_activePoint)) {
                            m_mouseDrag = true;
                            shape->setIsPress(true);
                            shape->setIsSelect(false);
                            break;
                        }

                        if (shape->isAreaHavPt(pixPos)) {
                            setCursor(Qt::ClosedHandCursor);
                            shape->setIsDrag(true);
                            shape->setIsSelect(true);
                            m_lastPressPos = pixPos;
                            break;
                        }
                        else {
                            shape->setIsSelect(false);
                        }
   
                        

                    }
                }
            }
            update();
        }
    } else throw CanvasException("Unable to draw current pattern");

}

void Canvas2D::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    try{
        if (getImage().isNull()) return;
        QPointF pixPos = bindImgEdge(event->pos());

        if (m_task == task_mode_e::segmentation) {
            if (m_operat == CanvasBase::draw) {

            }
            else if (m_operat == CanvasBase::edit) {
                if (m_draw == draw_mode_e::Polygon) {
                    if (m_mouseDrag) {
                        m_mouseDrag = false; m_activePoint = -1;
                    }
                    for (ShapePtr shape : getShapeList()) {
                        if (!shape->isClosed() || !shape->label()->m_visible) continue;

                        shape->setControlPtActive(-1);

                        if (shape->isPress()) shape->setIsPress(false);
                        shape->setIsDrag(false);
                        if (!shape->isAreaHavPt(pixPos)) setCursor(Qt::ArrowCursor);
                        else setCursor(Qt::OpenHandCursor);
                    }
                }
            }
        } 
    }catch (const std::exception& e) {
        qWarning() << e.what();
    }

    update();
}

void Canvas2D::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    try {
        if (m_task == CanvasBase::segmentation) {
            if (m_operat == CanvasBase::draw) {
                //结束绘制，弹出属性窗口
                ShapePtr shape = currentShape();
                if (!shape) return;
                if (shape->pointCount() > 2) {
                    shape->deletePoint(shape->pointCount() - 1);
                    shape->setIsClosed(true);
                    emit sigSetProperty(shape);
                }
                else {

                }
            }
        }
    }
    catch (const std::exception& e) {
        qWarning() << e.what();
    }
    CanvasBase::mouseDoubleClickEvent(event);
}

//void Canvas2D::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
//{
//
//}

void Canvas2D::wheelEvent(QGraphicsSceneWheelEvent* event)
{
	CanvasBase::wheelEvent(event);
}

void Canvas2D::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{
    try {
        QPointF pixPos = bindImgEdge(event->pos());
        QPointF m_mousePos = pixPos;

        if (m_task == CanvasBase::detection) {

        } else if (m_task == CanvasBase::segmentation) {

            if (m_operat == CanvasBase::draw) {
                ShapePtr shape = currentShape();
                if (!shape) return;
                if (m_currentIndex == -1 ||
                    m_shapeList.count() == 0 ||
                    shape->isClosed()) {//队列为空 || 都已经封闭
                    return;
                }

                if (m_draw == YShape::Polygon) {
                    shape->updateEndPt(pixPos);
                    update();
                }
            } else if (m_operat == CanvasBase::edit) {

                for (ShapePtr shape : getShapeList()) {
                    
                    if (!shape->isClosed() || !shape->label()->m_visible) continue;
                    int pos;
                    if (shape->isPtsHavPt(pixPos, pos)) {
                        shape->setControlPtActive(pos);
                    } else {
                        shape->setControlPtActive(-1);
                    }
                    if (shape->isPress()) {
                        setCursor(Qt::OpenHandCursor);
                    } else {
                        auto pos1 = m_insertPoint, pos2 = m_activePoint;

                        if (shape->isPtsHavPt(pixPos, pos2) || shape->isEdgeHavPt(pixPos, pos1)) {
                            setCursor(Qt::PointingHandCursor);
                        } else if (shape->isAreaHavPt(pixPos)) {
                            shape->setIsActive(true);
                            setCursor(Qt::OpenHandCursor);
                        }
                        else {
                            shape->setIsActive(false);
                            setCursor(Qt::ArrowCursor);
                        }
                    }
                    update();
                }
            }
        }
    } catch (const std::exception& e) {
        qWarning() << e.what();
    } 
}

void Canvas2D::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    try {
        QPointF pixPos = bindImgEdge(event->pos());
        QPointF m_mousePos = pixPos;

        if (m_task == CanvasBase::detection) {

        }
        else if (m_task == CanvasBase::segmentation) {

            if (m_operat == CanvasBase::draw) {

            }
            else if (m_operat == CanvasBase::edit) {

                for (ShapePtr shape : getShapeList()) {

                    if (!shape->isClosed() || !shape->label()->m_visible) continue;
                    if (!shape->isAreaHavPt(pixPos)) {
                        shape->setIsActive(false);
                    }

                    if (m_insertPoint >= 0 && shape->isPress()) {
                        shape->insertPoint(pixPos, m_insertPoint);
                        m_activePoint = m_insertPoint;
                        m_insertPoint = -1;
                        break;
                    }
                    if (m_mouseDrag && shape->isPress()) {
                        shape->updatePoint(pixPos, m_activePoint);
                        break;
                    }

                    if (shape->isDrag() /*&& shape->isAreaHavPt(pixPos)*/) {
                        setCursor(Qt::ClosedHandCursor);
                        QPointF movePos = m_mousePos - m_lastPressPos;
                        shape->move(movePos);
                        m_lastPressPos = m_mousePos;
                        break;
                    }
                }
            }
        }
    }
    catch (const std::exception& e) {
        qWarning() << e.what();
    }

    update();
}

void Canvas2D::keyPressEvent(QKeyEvent* event)
{
    try {
        if (m_task == CanvasBase::segmentation) {
            if (m_operat == CanvasBase::draw) {
                if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
                    //结束绘制，弹出属性窗口
                    ShapePtr shape = currentShape();
                    if (!shape) return;
                    if (shape->pointCount() > 2) {
                        shape->setIsClosed(true);
                        emit sigSetProperty(shape);
                    } else {

                    }
                }
            }
        }
    }
    catch (const std::exception& e) {
        qWarning() << e.what();
    }
    CanvasBase::keyPressEvent(event);
}

void Canvas2D::drawShape(QPainter* painter)
{
    try{
        if (m_task == CanvasBase::detection) {

        }
        else if (m_task == CanvasBase::segmentation) {
            if (m_operat == CanvasBase::draw) {
                for (int i = 0; i < m_shapeList.count(); ++i) {
                    ShapePtr shape = m_shapeList.at(i);
                    shape.get()->draw(painter, !shape->isClosed());
                }
            }
            else if (m_operat == CanvasBase::edit) {
                for (int i = 0; i < m_shapeList.count(); ++i) {
                    ShapePtr shape = m_shapeList.at(i);
                    shape.get()->draw(painter);
                }
            }
        }
        else throw CanvasException("Unable to draw current pattern");
    }
    catch (const std::exception& e) {
        qWarning() << e.what();
    }
}
