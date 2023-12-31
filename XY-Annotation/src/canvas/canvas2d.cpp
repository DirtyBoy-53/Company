﻿#include "canvas2d.h"
#include <QPainter>
#include <QMouseEvent>
#include <algorithm>
#include "segannotationitem.h"
#include "annotationcontainer.h"
#include "common.h"
using namespace CanvasUtils;
Canvas2D::Canvas2D(const LabelManager *pLabelManager, const AnnotationContainer *pAnnoContainer, QWidget *parent)
    : CanvasBase (pLabelManager, pAnnoContainer, parent)
    , pixmap()
{
    mousePos=QPoint(0,0);
    setMouseTracking(true);
    setFocusPolicy(Qt::WheelFocus);
}

void Canvas2D::paintEvent(QPaintEvent *event)
{
    if (pixmap.isNull()){
        QWidget::paintEvent(event);
        return;
    }
    QPainter p(this);
    /**********************启用抗锯齿功能**************************/
//     p.setRenderHint(QPainter::Antialiasing);
//     p.setRenderHint(QPainter::HighQualityAntialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
//    p.setRenderHint(QPainter::Antialiasing);

    p.scale(scale, scale);
    p.translate(offsetToCenter());

    p.drawPixmap(0, 0, pixmap);

    p.setPen(QPen(Qt::white));

    if (task == DETECTION){
//        if (mode == DRAW){
//            for (int i=0;i<pAnnoContainer->length();i++){
//                auto item = RectAnnotationItem::castPointer((*pAnnoContainer)[i]);
//                QRect rect=item->getRect();
//                QString label=item->getLabel();

//                if (pLabelManager->hasLabel(label) && (*pLabelManager)[label].visible==false)
//                    continue;

//                if (pLabelManager->hasLabel(label) && (*pLabelManager)[label].color.isValid()){
//                    QColor color = (*pLabelManager)[label].color;
//                    drawRectAnnotation(p, rect, color, 0.2, color, 0.5);
//                }else{
//                    p.drawRect(rect);
//                }
//                QFont font("Helvetica"); font.setFamily("Times"); font.setPixelSize(LABEL_PIXEL_SIZE);
//                p.setFont(font);
//                p.drawText(rect.topLeft()-QPoint(0,LABEL_PIXEL_SIZE/2), label);
//            }
//            if (curPoints.length()>0){
//                p.drawRect(QRect(curPoints[0], curPoints[1]).normalized());
//            }
//            p.end();
//        }else if (mode==SELECT){
//            for (int i=0;i<pAnnoContainer->length();i++){
//                if (i==pAnnoContainer->getSelectedIdx()) continue;

//                auto item = RectAnnotationItem::castPointer((*pAnnoContainer)[i]);
//                QRect rect=item->getRect();
//                QString label=item->getLabel();

//                if (pLabelManager->hasLabel(label) && (*pLabelManager)[label].visible==false)
//                    continue;

//                if (pLabelManager->hasLabel(label) && (*pLabelManager)[label].color.isValid()){
//                    QColor color = (*pLabelManager)[label].color;
//                    drawRectAnnotation(p, rect, color, 0.1, color, 0.2);
//                }else{
//                    p.drawRect(rect);
//                }
//            }
//            QString selectedLabel = pAnnoContainer->getSelectedItem()->getLabel();
//            QRect drawedRect = rectEditing?editedRect:RectAnnotationItem::castPointer(pAnnoContainer->getSelectedItem())->getRect();
//            p.save();
//            QColor color = (*pLabelManager)[selectedLabel].color;
//            color.setAlphaF(0.2); QBrush brush(color); p.setBrush(brush);
//            QPen pen(Qt::white); pen.setStyle(Qt::DashLine); p.setPen(pen);
//            p.drawRect(drawedRect);
//            p.restore();

//            QFont font("Helvetica"); font.setFamily("Times"); font.setPixelSize(LABEL_PIXEL_SIZE);
//            p.setFont(font);
//            p.drawText(drawedRect.topLeft()-QPoint(0,LABEL_PIXEL_SIZE/2), selectedLabel);
//        }
    }else if (task == SEGMENTATION){

        if (mode == DRAW){
            QPixmap colorMap(pixmap.size());
            colorMap.fill(QColor(0,0,0,0));
            QPainter p0(&colorMap);
//            for (int i=0;i<pAnnoContainer->length();i++){
//                auto item = SegAnnotationItem::castPointer((*pAnnoContainer)[i]);
//                QString label = item->getLabel();

//                if (pLabelManager->hasLabel(label) && (*pLabelManager)[label].visible==false)
//                    continue;

//                QColor color = (*pLabelManager)[label].color;
//                for (auto stroke: item->getStrokes())
//                    stroke.drawSelf(p0,color);
//            }

            if (curStrokes.length()>0){
                QColor color = Qt::black;
                for (int i=0;i<curStrokes.length()-1;i++){
                    curStrokes[i].drawSelf(p0,color);
                }

                if (strokeDrawing)
                    curStrokes.back().drawSelf(p0, color, true);
                else
                    curStrokes.back().drawSelf(p0, color, false);
            }
            p0.end();

            p.setOpacity(0.5);
            p.drawPixmap(0,0,colorMap);

            if (!strokeDrawing && (drawMode==SQUAREPEN || drawMode==CIRCLEPEN) && !outOfPixmap(mousePos)){
                p.save();
                if (drawMode==SQUAREPEN){
                    p.setPen(QPen(Qt::white, curPenWidth, Qt::SolidLine, Qt::SquareCap, Qt::BevelJoin));
                }else if (drawMode==CIRCLEPEN){
                    p.setPen(QPen(Qt::white, curPenWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                }
                p.drawPoint(mousePos);
                p.restore();
            }

            p.end();
        }else if (mode == SELECT){
            QPixmap colorMap(pixmap.size());
            colorMap.fill(QColor(0,0,0,0));
            QPainter p0(&colorMap);
//            for (int i=0;i<pAnnoContainer->length();i++){
//                auto item = SegAnnotationItem::castPointer((*pAnnoContainer)[i]);
//                QString label = item->getLabel();

//                if (pLabelManager->hasLabel(label) && (*pLabelManager)[label].visible==false)
//                    continue;

//                QColor color = (*pLabelManager)[label].color;
//                for (auto stroke: item->getStrokes())
//                    stroke.drawSelf(p0,color);
//            }
            if (curStrokes.length()>0){
                QColor color = Qt::white;
                for (int i=0;i<curStrokes.length()-1;i++)
                    curStrokes[i].drawSelf(p0,color);
                if (strokeDrawing)
                    curStrokes.back().drawSelf(p0, color, true);
                else
                    curStrokes.back().drawSelf(p0, color, false);
            }
            p0.end();

            p.setOpacity(0.5);
            p.drawPixmap(0,0,colorMap);

            if (!strokeDrawing && (drawMode==SQUAREPEN || drawMode==CIRCLEPEN) && !outOfPixmap(mousePos)){
                p.save();
                if (drawMode==SQUAREPEN){
                    p.setPen(QPen(Qt::white, curPenWidth, Qt::SolidLine, Qt::SquareCap, Qt::BevelJoin));
                }else if (drawMode==CIRCLEPEN){
                    p.setPen(QPen(Qt::white, curPenWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                }
                p.drawPoint(mousePos);
                p.restore();
            }

            p.end();
//            QPixmap colorMap(pixmap.size());
//            colorMap.fill(QColor(0,0,0,0));
//            QPainter p0(&colorMap);
//            for (int i=0;i<pAnnoContainer->length();i++){
//                auto item = SegAnnotationItem::castPointer((*pAnnoContainer)[i]);
//                QString label = item->getLabel();

//                if (pLabelManager->hasLabel(label) && (*pLabelManager)[label].visible==false)
//                    continue;

//                QColor color = (*pLabelManager)[label].color;
//                color = i == pAnnoContainer->getSelectedIdx() ? color: color.lighter();
//                for (auto stroke: item->getStrokes())
//                    stroke.drawSelf(p0,color);
//            }
//            p0.end();

//            p.setOpacity(0.5);
//            p.drawPixmap(0,0,colorMap);
//            p.end();
        }
    }
}


void Canvas2D::mousePressEvent(QMouseEvent *event)
{
    if (pixmap.isNull()){
        QWidget::mousePressEvent(event);
        return;
    }
    QPoint pixPos = pixelPos(event->pos());
    QPoint boundedPixPos = boundedPixelPos(event->pos());
    emit mouseMoved(boundedPixPos);

    if (task == TaskMode::DETECTION){
//        if (mode == CanvasMode::DRAW){
//            if (drawMode == DrawMode::RECTANGLE){
//                if (event->button() == Qt::LeftButton){
//                    if (curPoints.length()==0){
//                        if (!outOfPixmap(pixPos)){
//                            curPoints.push_back(pixPos);
//                            curPoints.push_back(pixPos);
//                            update();
//                        }
//                    } else if (curPoints.length()==2){
//                        curPoints[1] = boundedPixPos;
//                        emit newRectangleAnnotated(QRect(curPoints[0], curPoints[1]).normalized());
//                        curPoints.clear();
//                    } else {
//                        throw "Anomaly length of curPoints of new rectangle";
//                    }
//                }else if (event->button() == Qt::RightButton){
//                    if (curPoints.length()==0){
//                        int selectedIdx = selectShape(pixPos);
//                        if (selectedIdx!=-1)
//                            emit removeRectRequest(selectedIdx);
//                    } else if (curPoints.length()==2){
//                        curPoints.clear();
//                        update();
//                    } else {
//                        throw "Anomaly length of curPoints of new rectangle";
//                    }
//                }
//            }
//        } else if (mode == CanvasMode::SELECT){
//            if (event->button() == Qt::LeftButton){             // 左键开始拖动矩形的边
//                auto item = RectAnnotationItem::castPointer((*pAnnoContainer)[pAnnoContainer->getSelectedIdx()]);
//                QRect selectedRect = item->getRect();
//                if (onRectTop(pixPos, selectedRect)){
//                    rectEditing=true; editedRect = selectedRect;
//                    editedRectEdge = TOP;
//                }else if (onRectBottom(pixPos, selectedRect)){
//                    rectEditing=true; editedRect = selectedRect;
//                    editedRectEdge = BOTTOM;
//                }else if (onRectLeft(pixPos, selectedRect)){
//                    rectEditing=true; editedRect = selectedRect;
//                    editedRectEdge = LEFT;
//                }else if (onRectRight(pixPos, selectedRect)){
//                    rectEditing=true; editedRect = selectedRect;
//                    editedRectEdge = RIGHT;
//                }
//            }
//        }
    }else if (task == TaskMode::SEGMENTATION){
        if (mode == DRAW){
            if (event->button()==Qt::LeftButton){
                if (drawMode!=POLYGEN){
                    SegStroke stroke;
                    stroke.penWidth = curPenWidth;
                    switch(drawMode){
                    case CONTOUR: stroke.type="contour"; break;
                    case SQUAREPEN: stroke.type="square_pen"; break;
                    case CIRCLEPEN: stroke.type="circle_pen"; break;
                    default: throw "abnormal draw mode when segmentation";
                    }
                    stroke.points.push_back(boundedPixPos);
                    curStrokes.push_back(stroke);
                    strokeDrawing=true;
                    update();
                }else{  // drawMode == POLYGEN
                    if (!strokeDrawing){
                        SegStroke stroke;
                        stroke.penWidth = curPenWidth;
                        stroke.type = "contour";
                        stroke.points.push_back(boundedPixPos);
                        stroke.points.push_back(boundedPixPos);
                        curStrokes.push_back(stroke);
                        strokeDrawing=true;
                        update();
                    }else{
                        curStrokes.back().points.push_back(boundedPixPos);
                        update();
                    }
                }
            }else if (event->button()==Qt::RightButton){
                if (drawMode!=POLYGEN || strokeDrawing==false){
                    if (curStrokes.length()>0){
                        curStrokes.pop_back();
                        update();
                    }
                }else{ //drawMode == POLYGEN && strokeDrawing
                    strokeDrawing=false;
                    curStrokes.pop_back();
                    update();
                }
            }
        }
        else if(mode = SELECT){
            if(m_mouseDrag == true) return;
            m_activePoint = -1;
            m_insertPoint = -1;
            for(auto &strokes : curStrokes){
                if(strokes.isPtsHavPoint(pixPos,m_activePoint)){
                    m_mouseDrag = true;
                    m_curStroke = &strokes;
                    break;
                }
                if(strokes.isEdgeHavPt(pixPos,m_insertPoint)){
                    m_mouseDrag = true;
                    m_curStroke = &strokes;
                    break;
                }
            }
        }
    }
}

void Canvas2D::mouseMoveEvent(QMouseEvent *event)
{
    if (pixmap.isNull()){
        QWidget::mouseMoveEvent(event);
        return;
    }
    mousePos = pixelPos(event->pos());
    QPoint pixPos = boundedPixelPos(event->pos());
    emit mouseMoved(pixPos);
    if (task == TaskMode::DETECTION){
        if (mode == CanvasMode::DRAW){
            if (drawMode == DrawMode::RECTANGLE){
                if (curPoints.length()==2){
                    curPoints[1] = pixPos;
                    update();
                } else if (curPoints.length()!=0){
                    throw "Anomaly length of curPoints of new rectangle";
                }
            }
        }else if (mode == CanvasMode::SELECT){
            if(m_curStroke&& m_activePoint > -1){
                m_curStroke->updatePoint(pixPos,m_activePoint);
                update();
            }

//            if (rectEditing){
//                switch (editedRectEdge) {
//                case TOP:
//                    editedRect.setTop(pixPos.y());
//                    break;
//                case BOTTOM:
//                    editedRect.setBottom(pixPos.y());
//                    break;
//                case LEFT:
//                    editedRect.setLeft(pixPos.x());
//                    break;
//                case RIGHT:
//                    editedRect.setRight(pixPos.x());
//                    break;
//                }
//                update();
//            }
        }
    }else if (task == SEGMENTATION){
        if (mode == DRAW){
            if (drawMode!=POLYGEN){
                if (strokeDrawing){
                    if (curStrokes.back().points.length()==0 || curStrokes.back().points.back()!=pixPos){
                        curStrokes.back().points.push_back(pixPos);
                        update();
                    }
                }
                if (!strokeDrawing && (drawMode==SQUAREPEN || drawMode==CIRCLEPEN)){
                    update();
                }
            }else{
                if (strokeDrawing){
                    curStrokes.back().points.back()=pixPos;
                    update();
                }
            }
        }
        else if(mode = SELECT){
            if(m_insertPoint >= 0 && m_curStroke){
                m_curStroke->insertPoint(pixPos,m_insertPoint);
                m_activePoint =m_insertPoint;
                m_insertPoint = -1;
            }
            if(m_mouseDrag && m_curStroke){
                m_curStroke->updatePoint(pixPos,m_activePoint);
                update();
            }
        }
    }
}

void Canvas2D::mouseReleaseEvent(QMouseEvent *event){
    if (pixmap.isNull()){
        QWidget::mouseReleaseEvent(event);
        return;
    }
    if (task == TaskMode::DETECTION){
        if (mode == CanvasMode::SELECT){
            if (rectEditing){
                emit modifySelectedRectRequest(pAnnoContainer->getSelectedIdx(), editedRect.normalized());
                rectEditing = false;
            }
        }
    }else if (task == TaskMode::SEGMENTATION){
        if (mode == DRAW){
            if (drawMode!=POLYGEN){
                strokeDrawing=false;
                update();
            }
        }
        else if(mode == SELECT){
            if(drawMode == POLYGEN){
                if(m_mouseDrag){
                    m_mouseDrag = false;
                    m_curStroke = nullptr;
                    if(m_curStroke){
                        m_curStroke = nullptr;
                        m_activePoint = -1;
                    }
                }
            }
        }
    }
}

void Canvas2D::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (pixmap.isNull()){
        QWidget::mouseDoubleClickEvent(event);
        return;
    }
    if (task==SEGMENTATION)
        if (mode == DRAW){
            if (drawMode==POLYGEN){
                strokeDrawing=false;
                update();
            }
        }
}

void Canvas2D::keyPressEvent(QKeyEvent *event)
{
    if (task == SEGMENTATION){
        if (mode == DRAW){
            if (event->key()==Qt::Key_Return || event->key()==Qt::Key_Enter){
                if (drawMode==POLYGEN){
                    strokeDrawing=false;
                }
                if (curStrokes.length()>0){
                    emit newStrokesAnnotated(curStrokes);
                    curStrokes.back().m_isClosed = true;
//                    curStrokes.clear();
                    update();
                }
                return;
            }
        }
    }
    QWidget::keyPressEvent(event);
}

void Canvas2D::changeTask(TaskMode _task) {
    if (task == _task) return;
    task = _task;
    switch(task){
    case DETECTION:
        mode = CanvasMode::DRAW;
        drawMode = DrawMode::RECTANGLE;
        break;
    case SEGMENTATION:
        mode = CanvasMode::DRAW;
        drawMode = DrawMode::CIRCLEPEN;
        curPenWidth=lastPenWidth;
        break;
    default:
        throw "3d task cannot be set to canvas 2d";
    }
    rectEditing = false;
    strokeDrawing=false;
    curStrokes.clear();
    emit modeChanged(modeString());
}

void Canvas2D::changeCanvasMode(CanvasMode _mode)
{
    if (mode == _mode) return;
    if (_mode == MOVE)
        throw "move mode cannot be set to canvas 2d";
    mode = _mode;
    if (mode == SELECT) rectEditing = false;
    emit modeChanged(modeString());
}

void Canvas2D::changeDrawMode(DrawMode _draw)
{
    drawMode=_draw;
    switch (drawMode) {
    case RECTANGLE:
        curPoints.clear();
        rectEditing=false;
        break;
    case CIRCLEPEN:
    case SQUAREPEN:
        if (strokeDrawing==true){
            curStrokes.pop_back();
            strokeDrawing=false;
        }
        curPenWidth = lastPenWidth;
        break;
    case CONTOUR:
    case POLYGEN:
        if (strokeDrawing==true){
            curStrokes.pop_back();
            strokeDrawing=false;
        }
        curPenWidth=1;
        break;
    }
    emit modeChanged(modeString());
}

void Canvas2D::setPenWidth(int width) {
    curPenWidth = width;
    if (drawMode==CIRCLEPEN || drawMode==SQUAREPEN)
        lastPenWidth = width;
    update();
}

void Canvas2D::close() {
    pixmap = QPixmap();
    curPoints.clear();
    rectEditing=false;
    strokeDrawing=false;
    curStrokes.clear();
    adjustSize();
    update();
}

void Canvas2D::loadPixmap(QPixmap new_pixmap)
{
    pixmap = new_pixmap;
    adjustSize();
    update();
}

void Canvas2D::setScale(qreal new_scale)
{
    scale = new_scale;
    adjustSize();
    update();
}

int Canvas2D::selectShape(QPoint pos)
{
    if (task==DETECTION){
//        for (int i=pAnnoContainer->length()-1;i>=0;i--){
//            auto item = RectAnnotationItem::castPointer((*pAnnoContainer)[i]);
//            QRect rect=item->getRect();
//            rect.setTopLeft(rect.topLeft()-QPoint(2,2));
//            rect.setBottomRight(rect.bottomRight()+QPoint(2,2));
//            if (rect.contains(pos))
//                return i;
//        }
        return -1;
    }
    return -1;
}

QPoint Canvas2D::offsetToCenter()
{
    qreal s = scale;
    int w = int(pixmap.width() * s), h=int(pixmap.height() * s);
    int aw = this->size().width(), ah = this->size().height();
    int x = aw > w ? int((aw - w) / (2 * s)) : 0;
    int y = ah > h ? int((ah - h) / (2 * s)) : 0;
    return QPoint(x,y);
}

QSize Canvas2D::minimumSizeHint() const
{
    if (!pixmap.isNull())
        return scale * pixmap.size();
    return QWidget::minimumSizeHint();
}

QPoint Canvas2D::pixelPos(QPoint pos)
{
    return pos / scale - offsetToCenter();
}

QPoint Canvas2D::boundedPixelPos(QPoint pos)
{
    pos = pos / scale - offsetToCenter();
    pos.setX(std::min(std::max(pos.x(), 0), pixmap.width()-1));
    pos.setY(std::min(std::max(pos.y(), 0), pixmap.height()-1));
    return pos;
}

bool Canvas2D::outOfPixmap(QPoint pos)
{
    int w = pixmap.width(), h= pixmap.height();
    return !(0<=pos.x() && pos.x()<w && 0<=pos.y() && pos.y()<h);
}
