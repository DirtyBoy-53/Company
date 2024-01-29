#include "segannotationitem.h"
#include "annotationcontainer.h"
#include <QColor>
#include <QPainterPath>
#include <QPen>
#include <QBrush>
#include <QPainter>


void SegStroke::fromJsonObject(QJsonObject json)
{
    if (json.contains("type")){
        QJsonValue value = json.value("type");
        if (value.isString()){
            type = value.toString();
            if (type!="contour" && type!="square_pen" && type!="circle_pen"){
                throw JsonException("value of <type> is illegal");
            }
        }else{
            throw JsonException("value of <type> is illegal");
        }
    }else{
        throw JsonException("no data <type>");
    }
    if (json.contains("pen_width")){
        QJsonValue value = json.value("pen_width");
        if (value.isDouble()){
            penWidth = static_cast<int>(value.toDouble());
        }else{
            throw JsonException("value of <pen_width> is illegal");
        }
    }else{
        throw JsonException("no data <pen_width>");
    }
    if (json.contains("points")){
        QJsonValue value = json.value("points");
        if (value.isArray()){
            QJsonArray pointsArray = value.toArray();
            points.clear();
            for (int i=0;i<pointsArray.size();i++){
                QJsonArray point = pointsArray.at(i).toArray();
                if (!point.at(0).isDouble() || !point.at(1).isDouble()){
                    throw JsonException("value of <points> is illegal");
                }
                int x=static_cast<int>(point.at(0).toDouble());
                int y=static_cast<int>(point.at(1).toDouble());
                points.push_back(QPoint(x,y));
            }
        }else{
            throw JsonException("value of <points> is illegal");
        }
    }else{
        throw JsonException("no data <points>");
    }
    if (json.contains("base64")){

    }else{
        throw JsonException("no data <base64>");
    }
}

QJsonObject SegStroke::toJsonObject()
{
    QJsonObject json;
    json.insert("shape_type", type);
    QJsonArray array;
    for (auto point : points){
        QJsonArray pointArray;
        pointArray.append(QString::number(point.x(),'f',13));
        pointArray.append(QString::number(point.y(),'f',13));
        array.append(pointArray);
    }
    json.insert("points", array);
//    if (penWidth!=-1)
//        json.insert("pen_width", penWidth);

    return json;
}

void SegStroke::addJsonObject(QJsonArray &json,QJsonObject &obj)
{
    QJsonArray array;
    for (auto point : points){
        QJsonArray pointArray;
        pointArray.append(QString::number(point.x(),'f',13));
        pointArray.append(QString::number(point.y(),'f',13));
        array.append(pointArray);
    }
    obj.insert("points", array);
    obj.insert("shape_type", type);
}

void SegStroke::drawSelf(QPainter &p, QColor color, bool fill)
{
    QPainterPath path;

    QPolygonF polygon(points);
    path.addPolygon(polygon);
    if(m_isClosed){
        path.closeSubpath();
//        p.setBrush(QColor(color.red(), color.green(), color.blue(),100));
        p.setPen(QPen(color, m_lineWidth, Qt::SolidLine));
    }else{
        p.setBrush(QColor(10, 0, 0,100));
        p.setPen(QPen(Qt::green, m_lineWidth, Qt::SolidLine));
    }

    if (fill) path.setFillRule(Qt::WindingFill);
    p.save();


    p.setRenderHint(QPainter::Antialiasing);
    p.drawPath(path);



//    if (type=="contour"){
//        p.setPen(QPen(color));
//        if (fill)
//            p.setBrush(QBrush(color));
//    }else if (type == "square_pen"){
//        p.setPen(QPen(color, penWidth, Qt::SolidLine, Qt::SquareCap, Qt::BevelJoin));
//    }else if (type == "circle_pen"){
//        p.setPen(QPen(color, penWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
//    }


    if(m_isClosed){
        // Draw the control points
        p.setBrush(QColor(color.red(), color.green(), color.blue(),255));
        p.setPen(QPen(color));
        for (int i=0; i<points.length(); ++i) {
            QPointF pos = points.at(i);
            p.drawEllipse(QRectF(pos.x() - m_pointSize,
                                        pos.y() - m_pointSize,
                                        m_pointSize*2, m_pointSize*2));
        }
        p.setPen(QPen(color, m_lineWidth, Qt::SolidLine));
        p.setBrush(Qt::NoBrush);
        p.drawPolyline(points);
    }else{
        p.setPen(QColor(0, 255, 0, 255));
        p.setBrush(QColor(0, 255, 0, 255));
        for (int i=0; i<points.length(); ++i) {
            QPointF pos = points.at(i);
            p.drawEllipse(QRectF(pos.x() - m_pointSize,
                                 pos.y() - m_pointSize,
                                 m_pointSize*2, m_pointSize*2));
        }
    }


    p.restore();
}

bool SegStroke::isPtsHavPoint(const QPointF &point,int& pos)
{
    for(auto i = 0;i < points.length();++i){
        auto dis = QPointF(points.at(i)-point).manhattanLength();
        if(dis <= m_resDistance){
            pos = i;
            return true;
        }
    }
    return false;
}

//QPointF GetFootOfPerpendicular(
//    const QPointF &pt,
//    const QPointF &begin,
//    const QPointF &end)
//{
//    double dx = begin.x() - end.x();
//    double dy = begin.y() - end.y();
//    if(fabs(dx)<0x00000001 && fabs(dy) < 0x00000001){
//        return begin;
//    }
//    double u = (pt.x() - begin.x())*(begin.x() - end.x())+
//               (pt.y()-begin.y())*(begin.y()-end.y());
//    u = u/((dx*dx)+(dy*dy));
//    double x = begin.x() + u*dx;
//    double y = begin.y() + u*dy;
//    return QPointF(x,y);
//}

////圆
//struct Circle{
//public:
//    Circle()=default;
//    Circle(double r,const QPointF& pt):r(r),x(pt.x()),y(pt.y()){}
//    double r,x,y;
//};
//bool judge(QPointF p1,QPointF p2,Circle c)
//{
//    bool flag1=(p1.x()-c.x)*(p1.x()-c.x)+(p1.y()-c.y)*(p1.y()-c.y)<=c.r*c.r;
//    bool flag2=(p2.x()-c.x)*(p2.x()-c.x)+(p2.y()-c.y)*(p2.y()-c.y)<=c.r*c.r;

//    if(flag1&&flag2)	//情况一、两点都在圆内 :一定不相交
//      return false;
//    else if(flag1||flag2) //情况二、一个点在圆内，一个点在圆外：一定相交
//      return false;//尽管相交，但是会导致逻辑混乱，不予认可
//    else //情况三、两个点都在圆外
//    {
//        double A,B,C,dist1,dist2,angle1,angle2;
//        //将直线p1p2化为一般式：Ax+By+C=0的形式。先化为两点式，然后由两点式得出一般式
//        A=p1.y()-p2.y();
//        B=p2.x()-p1.x();
//        C=p1.x()*p2.y()-p2.x()*p1.y();
//        //使用距离公式判断圆心到直线ax+by+c=0的距离是否大于半径
//        dist1=A*c.x+B*c.y+C;
//        dist1*=dist1;
//        dist2=(A*A+B*B)*c.r*c.r;
//        if(dist1>dist2)//圆心到直线p1p2的距离大于半径，不相交
//            return false;
//        angle1=(c.x-p1.x())*(p2.x()-p1.x())+(c.y-p1.y())*(p2.y()-p1.y());
//        angle2=(c.x-p2.x())*(p1.x()-p2.x())+(c.y-p2.y())*(p1.y()-p2.y());
//        if(angle1>0&&angle2>0)//余弦为正，则是锐角，一定相交
//            return true;
//        else
//            return false;
//    }
//    return false;
//}


bool SegStroke::isEdgeHavPt(const QPointF &point,int& pos)
{
//    auto size = points.size();
//    if(size < 2) return false;
//    for(auto i = 0;i < size;i++){
//        QPointF begin = points.at(i);
//        QPointF end;
//        if(i+1 < size)
//            end = points.at(i+1);
//        else end = points.at(0);
//        if(!judge(begin,end,Circle(m_resDistance,point))) continue;
//        QPointF FootPt = GetFootOfPerpendicular(point,begin,end);
//        auto dis = (point-FootPt).manhattanLength();
//        qDebug("all indx=%d dis:%lf",i,dis);
//        if(dis <= m_resDistance){
//            pos = i+1;
//            qDebug("success indx=%d dis:%lf",pos,dis);
//            qDebug() << "CurPoint:" << point;
//            return true;
//        }
//    }
    return false;
}

bool SegStroke::isContains(const QPointF &point)
{
    QPolygonF polygon(points);

    return polygon.containsPoint(point,Qt::WindingFill);
}

void SegStroke::updatePoint(const QPointF &point, int &pos)
{
    if(pos < points.length() && pos >=0){
        points.replace(pos,point);
    }
}

void SegStroke::insertPoint(const QPointF &point, int &pos)
{
    if(pos < points.length() && pos >=0){
        points.insert(pos,point);
    }else if(pos == points.length()){
        points.append(point);
    }
}

void SegStroke::move(const QPointF &point)
{
    for(auto i = 0;i < points.size();i++){
        points.replace(i,(points.at(i)+point));
    }
}

QImage drawColorImage(const QSize &size, const AnnotationContainer *pAnnoContainer, const LabelManager *pLabelManager)
{
    QImage image(size, QImage::Format_RGB32);
    image.fill(QColor(0,0,0));
    QPainter p(&image);
    for (int i=0;i<pAnnoContainer->length();i++){
        auto item = SegAnnotationItem::castPointer((*pAnnoContainer)[i]);
        QString label = item->getLabel();
        if ((*pLabelManager)[label].visible){
            QColor color = (*pLabelManager)[label].color;
            for (auto stroke: item->getStrokes())
                stroke.drawSelf(p,color);
        }
    }
    p.end();
    return image;
}

QImage drawLabelIdImage(const QSize &size, const AnnotationContainer *pAnnoContainer, const LabelManager *pLabelManager)
{
    QImage image(size, QImage::Format_Grayscale8);
    image.fill(QColor(0,0,0));
    QPainter p(&image);
    for (int i=0;i<pAnnoContainer->length();i++){
        auto item = SegAnnotationItem::castPointer((*pAnnoContainer)[i]);
        QString label = item->getLabel();
        int labelId = (*pLabelManager)[label].id;
        if ((*pLabelManager)[label].visible){
            QColor color = QColor(labelId, labelId, labelId);
            for (auto stroke: item->getStrokes())
                stroke.drawSelf(p,color);
        }
    }
    p.end();
    return image;
}
