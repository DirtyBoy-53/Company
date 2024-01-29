#include "document2d.h"

#include <algorithm>
#include <QMouseEvent>

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
    QPointF boundedPixPos = boundedPixelPos(event->pos());
    if(m_task == DocumentBase::detection){

    }else if(m_task == DocumentBase::segmentation){
        if (event->button()==Qt::LeftButton){
            if(m_operat == DocumentBase::draw){
                if(m_currentIndex == -1 && m_shapeList.count() == 0){//队列为空 || 都已经封闭
                    emit sigAddShape();
                    return;
                }
                if(m_draw == YShape::Polygon){
                    m_shapeList.at(m_currentIndex).get()->appendPoint(pixPos);
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
                //结束绘制，弹出属性窗口
            }
        }
    }
}

void Document2D::mouseMoveEvent(QMouseEvent *event)
{
    if (m_pixmap.isNull()){
        QWidget::mouseMoveEvent(event);
        return;
    }
    QPointF pixPos = pixelPos(event->pos());
    QPointF boundedPixPos = boundedPixelPos(event->pos());

    if(m_task == DocumentBase::detection){

    }else if(m_task == DocumentBase::segmentation){
        if(m_operat == DocumentBase::draw){
            if(m_currentIndex == -1 && m_shapeList.count() == 0){//队列为空 || 都已经封闭
                return;
            }
            if(m_draw == YShape::Polygon){
//                if(m_shapeList.at(m_currentIndex).get()->points().size() > 1){
                    m_shapeList.at(m_currentIndex).get()->updateEndPt(pixPos);
                    update();
//                }

            }
        }

    }
    else throw "Unable to draw current pattern";

}

void Document2D::wheelEvent(QWheelEvent *event)
{
    if (m_pixmap.isNull()){
        QWidget::wheelEvent(event);
        return;
    }
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
            }
        }
    }
    QWidget::keyPressEvent(event);
}

void Document2D::paintEvent(QPaintEvent *event)
{
    if (m_pixmap.isNull()){
        QWidget::paintEvent(event);
        return;
    }
    QPainter p(this);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    p.scale(m_scale,m_scale);
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
                shape.get()->draw(p0);
            }

            p0.end();
            p.setOpacity(0.5);
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
    qreal x = aw > w ? int((aw - w) / (2.0f * s)) : 0.0f;
    qreal y = ah > h ? int((ah - h) / (2.0f * s)) : 0.0f;
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
        const ShapePtr curShape = m_shapeList.at(m_currentIndex);
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

