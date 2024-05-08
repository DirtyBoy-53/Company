#include "document2d.h"

#include <algorithm>
#include <QMouseEvent>
#include <QApplication>
#include <QDebug>
#include "labelmanager.h"
#include "labeldialog.h"

Document2D::Document2D(LabelManager *labelManager,QWidget *parent)
    : DocumentBase(labelManager,parent)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::WheelFocus);
}

QString Document2D::addShape(const ShapePtr shape)
{
    QString name = Shape::typeToString(shape.get()->type());
    name = uniqueName(name);

    m_shapeList.append(shape);
    m_shapeList[m_shapeList.count()-1].get()->setName(name);
    setCurrentShape(m_shapeList.count()-1);
    update();
    return name;
}

void Document2D::deleteShape(const QString &shapeName)
{
    int index = indexOf(shapeName);
    if(index == -1)
        return;
    m_shapeList.removeAt(index);

    if(index <= m_currentIndex){
        m_currentIndex = -1;
        if(index == m_shapeList.count())
            --index;
        setCurrentShape(index);
    }
    update();
}

QString Document2D::addPoint(ShapePtr shape, const QPointF &point)
{
    if(shape == nullptr) return "";
    shape.get()->appendPoint(point);
    int curPos = shape.get()->points().size()-1;
    update();
    return QString::number(curPos);
}

void Document2D::deletePoint(ShapePtr shape, const QString &pointName)
{
    auto curPos = pointName.toInt();
    shape.get()->deletePoint(curPos);
    update();
}

ShapePtr Document2D::shape(const QString &shapeName) const
{
    int index = indexOf(shapeName);
    if (index == -1)
        return nullptr;
    return m_shapeList.at(index);
}


void Document2D::mousePressEvent(QMouseEvent *event)
{
    if (m_pixmap.isNull()){
        QWidget::mousePressEvent(event);
        return;
    }
    QPointF pixPos = pixelPos(event->pos());
//    QPointF boundedPixPos = boundedPixelPos(event->pos());
    if(m_task == DocumentBase::detection){

    }else if(m_task == DocumentBase::segmentation){
        if (event->button()==Qt::LeftButton){
            if(m_operat == DocumentBase::draw){
                auto shape = currentShape();
                if(m_currentIndex >=0 && !shape) return;
                if(m_currentIndex   == -1 ||
                    m_shapeList.count() == 0  ||
                    shape->isClosed() ){//队列为空 || 都已经封闭
                    emit sigAddShape();
                    return;
                }
                if(m_draw == YShape::Polygon){
                    emit sigAddPoint(pixPos);
                }
            }else if(m_operat == DocumentBase::edit){
                if(m_mouseDrag == true) return;
                m_activePoint = -1; m_insertPoint = -1;
                if (event->button()==Qt::LeftButton){
                    for (ShapePtr shape : getShapeList()){

                        if(!shape->isClosed() || !shape->label()->m_visible) continue;

                        if(shape->isAreaHavPt(pixPos)){
                            setCursor(Qt::ClosedHandCursor);
                            shape->setIsDrag(true);
                            m_lastPressPos = pixPos;
                        }else{
                            setCursor(Qt::ArrowCursor);
                            shape->setIsDrag(false);
                        }
                        if(shape->isPtsHavPt(pixPos,m_activePoint)){
                            m_mouseDrag = true;
                            shape->setIsSelect(true);
                            break;
                        }
                        if(shape->isEdgeHavPt(pixPos,m_insertPoint)){
                            m_mouseDrag = true;
                            shape->setIsSelect(true);
                            break;
                        }
                    }
                }
            }
        }

    }
    else throw "Unable to draw current pattern";
    update();
}

void Document2D::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_pixmap.isNull()){
        QWidget::mouseReleaseEvent(event);
        return;
    }
    QPointF pixPos = pixelPos(event->pos());
    QPointF boundedPixPos = boundedPixelPos(event->pos());

    if(m_task == task_mode_e::segmentation){
        if(m_operat == DocumentBase::draw){

        }else if(m_operat == DocumentBase::edit){
            if(m_draw == draw_mode_e::Polygon){
                if(m_mouseDrag){
                    m_mouseDrag = false; m_activePoint = -1;
                }
                for(ShapePtr shape : getShapeList()){
                    if(!shape->isClosed() || !shape->label()->m_visible) continue;
                    if(shape->isSelect()) shape->setIsSelect(false);
                    shape->setIsDrag(false);
                    if(!shape->isAreaHavPt(pixPos)) setCursor(Qt::ArrowCursor);
                    else setCursor(Qt::OpenHandCursor);
                }
            }
        }
    }

    update();
}

void Document2D::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (m_pixmap.isNull()){
        QWidget::mouseDoubleClickEvent(event);
        return;
    }
    if(m_task == DocumentBase::segmentation){
        if(m_operat == DocumentBase::draw){
            if (event->button()==Qt::LeftButton){
                //结束绘制，弹出属性窗口 暂不使用
            }
        }
    }
    update();
}

void Document2D::mouseMoveEvent(QMouseEvent *event)
{
    if (m_pixmap.isNull()){
        QWidget::mouseMoveEvent(event);
        return;
    }
    QPointF pixPos = pixelPos(event->pos());
    QPointF m_mousePos = pixPos;

    if(m_task == DocumentBase::detection){

    }else if(m_task == DocumentBase::segmentation){
        if(m_operat == DocumentBase::draw){
            ShapePtr shape = currentShape();
            if(!shape) return;
            if(m_currentIndex == -1 ||
                    m_shapeList.count() == 0 ||
                    shape->isClosed()){//队列为空 || 都已经封闭
                return;
            }

            if(m_draw == YShape::Polygon){
                shape->updateEndPt(pixPos);

            }
        }else if(m_operat == DocumentBase::edit){
            for(ShapePtr shape : getShapeList()){
                if(!shape->isClosed() || !shape->label()->m_visible) continue;
                if(shape->isSelect()) {
                    setCursor(Qt::ArrowCursor);
                } else {
                    if(shape->isPtsHavPt(pixPos, m_activePoint) ||
                        shape->isEdgeHavPt(pixPos, m_insertPoint)){
                        setCursor(Qt::PointingHandCursor);
                    }else if(shape->isDrag() && shape->isAreaHavPt(pixPos)){
                        setCursor(Qt::ClosedHandCursor);
                        QPointF movePos = m_mousePos - m_lastPressPos;
                        shape->move(movePos);
                        m_lastPressPos = m_mousePos;
                    }else if(shape->isAreaHavPt(pixPos)){
                        setCursor(Qt::OpenHandCursor);
                    }else setCursor(Qt::ArrowCursor);
                }
                if(m_insertPoint >= 0 && shape->isSelect()){
                    shape->insertPoint(pixPos, m_insertPoint);
                    m_activePoint = m_insertPoint;
                    m_insertPoint = -1;
                }
                if(m_mouseDrag && shape->isSelect()) shape->updatePoint(pixPos, m_activePoint);
            }
        }
    }
    else throw "Unable to draw current pattern";
    update();
}

void Document2D::wheelEvent(QWheelEvent *event)
{
    if (m_pixmap.isNull()){
        QWidget::wheelEvent(event);
        return;
    }
    if(Qt::ControlModifier == QApplication::keyboardModifiers()){
        if(event->delta() > 0){
            zoomIn();
        }else{
            zoomOut();
        }
    }
    update();
}

void Document2D::keyPressEvent(QKeyEvent *event)
{
    if (m_pixmap.isNull()){
        QWidget::keyPressEvent(event);
        return;
    }
    if(m_task == DocumentBase::segmentation){
        if(m_operat == DocumentBase::draw){
            if(event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter){
                //结束绘制，弹出属性窗口
                ShapePtr shape = currentShape();
                if(!shape) return;
                if(shape->pointSize() > 2){
                    shape->setIsClosed(true);
                    emit sigSetProperty(shape);
//                    LabelDialog::instance()->show();
//                    shape->m_label->setProperty();
                }
                else{

                }
            }
        }

    }
    update();
    QWidget::keyPressEvent(event);
}

void Document2D::paintEvent(QPaintEvent *event)
{
    if (m_pixmap.isNull()){
        QWidget::paintEvent(event);
        return;
    }
    QPainter p(this);
//    p.setRenderHint(QPainter::SmoothPixmapTransform);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::HighQualityAntialiasing, true);

    p.scale(m_scale,m_scale);
    qDebug() << "center:" << offsetToCenter();
    p.translate(offsetToCenter());
    p.drawPixmap(0,0,m_pixmap);
    p.setPen(QPen(Qt::white));

    if(m_task == DocumentBase::detection){

    }else if(m_task == DocumentBase::segmentation){
        if(m_operat == DocumentBase::draw){
            QPixmap colorMap(m_pixmap.size());
            colorMap.fill(QColor(0,0,0,0));
            QPainter p0(&colorMap);
            for(int i = 0;i < m_shapeList.count();++i){
                ShapePtr shape = m_shapeList.at(i);
                shape.get()->draw(p0,!shape->isClosed());
            }

            p0.end();
//            p.setOpacity(0.5);
            p.drawPixmap(0,0,colorMap);
            p.end();
        }else if(m_operat == DocumentBase::edit){
            QPixmap colorMap(m_pixmap.size());
            colorMap.fill(QColor(0,0,0,0));
            QPainter p0(&colorMap);
            for(int i = 0;i < m_shapeList.count();++i){
                ShapePtr shape = m_shapeList.at(i);
                shape.get()->draw(p0);
            }

            p0.end();
            p.drawPixmap(0,0,colorMap);
            p.end();
        }
    }
    else throw "Unable to draw current pattern";

}

QSize Document2D::sizeUnscaled() const
{
    return m_pixmap.size();
}

QSize Document2D::minimumSizeHint() const
{
    if(!m_pixmap.isNull())
        return m_scale*m_pixmap.size();
    return QWidget::minimumSizeHint();
}

void Document2D::setScale(qreal newScale)
{
    m_scale = newScale;
    adjustSize();
    update();
}

void Document2D::changeTask(task_mode_e task)
{
    if(m_task == task) return;
    m_task = task;
    switch(m_task){
    case DocumentBase::detection :
        m_operat = DocumentBase::draw;
        m_draw = YShape::Rectangle;
        break;
    case DocumentBase::segmentation :
        m_operat = DocumentBase::draw;
        m_draw = YShape::Polygon;
        break;
    default :
        throw "cannot set task.";
    }
    m_shapeList.clear();
}

void Document2D::changeOperatMode(operat_mode_e opt)
{
    if(m_operat == opt) return;
    m_operat = opt;
}

void Document2D::changeDrawMode(draw_mode_e draw)
{
    if(m_draw == draw) return;
    switch(m_draw){
    case YShape::Rectangle :
        break;
    case YShape::Line :
        break;
    case YShape::Curve :
        break;
    case YShape::Polygon :
        break;
    }
}

void Document2D::clean()
{
    m_pixmap = QPixmap();
    adjustSize();
    update();
}



QPointF Document2D::offsetToCenter()
{
    qreal s = m_scale;
    qreal w = int(m_pixmap.width() * s), h=int(m_pixmap.height() * s);
    qreal aw = this->size().width(), ah = this->size().height();
    qreal x = aw > w ? qreal((aw - w) / (2.0f * s)) : 0.0f;
    qreal y = ah > h ? qreal((ah - h) / (2.0f * s)) : 0.0f;
    return QPointF(x,y);
}

QPointF Document2D::pixelPos(QPointF pos)
{
    return pos / m_scale - offsetToCenter();
}

QPointF Document2D::boundedPixelPos(QPointF pos)
{
    pos = pos / m_scale - offsetToCenter();
    pos.setX(std::min(std::max(pos.x(), 0.0),  m_pixmap.width()  - 1.0));
    pos.setY(std::min(std::max(pos.y(), 0.0),  m_pixmap.height() - 1.0));
    return pos;
}

bool Document2D::outOfPixmap(QPointF pos)
{
    int w = m_pixmap.width(), h= m_pixmap.height();
    return !(0.0f<=pos.x() && pos.x()<w && 0.0f<=pos.y() && pos.y()<h);
}

void Document2D::setCurrentShape(int index)
{
    QString currentName;
    m_currentIndex = index;
    if(m_currentIndex != -1){
        const ShapePtr curShape = currentShape();
        currentName = curShape.get()->name();
    }
    emit currentShapeChanged(currentName);
}

int Document2D::indexOf(const QString &shapeName) const
{
    for(int i = 0;i < m_shapeList.count(); ++i){
        if(m_shapeList.at(i).get()->name() == shapeName)
            return i;
    }
    return -1;
}

QString Document2D::uniqueName(const QString &name) const
{
    QString unique;
    for(int i = 0;;++i){
        unique = name;
        if(i > 0)
            unique += QString::number(i);
        if(indexOf(unique)==-1)
            break;
    }
    return unique;
}

void Document2D::zoomIn()
{
    setScale(getScale()*1.05);
}

void Document2D::zoomOut()
{
    setScale(getScale()*0.95);
}
