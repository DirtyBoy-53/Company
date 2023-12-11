#ifndef SEGANNOTATIONITEM_H
#define SEGANNOTATIONITEM_H

#include <QString>
#include <QList>
#include <QPoint>
#include <QJsonArray>
#include <QPainter>
#include <QByteArray>
#include <QVector>
#include "annotationitem.h"
struct SegStroke{
    QString type;
    int penWidth;
    int ptDistance{10};
    QVector<QPointF> points;
    QByteArray picHex;
    SegStroke(): type(),penWidth(-1),points(),picHex(){}

    void fromJsonObject(QJsonObject json);
    QJsonObject toJsonObject();
    void drawSelf(QPainter &p,QColor color,bool fill=true);

    bool isPtsHavPoint(const QPoint& point,int& pos);
    bool isEdgeHavPt(const QPoint& point);
    void udpatePoint(const QPoint& point,int& pos);

};


class AnnotationContainer;
template<class stroke_type>
class Basic_SegAnnotationItem : public AnnotationItem{
    friend AnnotationContainer;
public:
    typedef std::shared_ptr<Basic_SegAnnotationItem> Basic_segPtr;
    static Basic_segPtr castPointer(std::shared_ptr<AnnotationItem> ptr){
        return std::static_pointer_cast<Basic_SegAnnotationItem>(ptr);
    }
    Basic_SegAnnotationItem()=default;
    Basic_SegAnnotationItem(const QList<stroke_type>& strokes, QString label, int id):
        AnnotationItem(label,id),strokes(strokes) {}

    const QList<stroke_type>& getStrokes() const { return strokes; }

    QString toStr() const { return label+" "+QString::number(id); }
    QJsonObject toJsonObject() const{
        QJsonObject json = AnnotationItem::toJsonObject();
        QJsonArray array;
        for (auto stroke: strokes){
            array.append(stroke.toJsonObject());
        }
        json.insert("strokes", array);
        return json;
    }
    void fromJsonObject(const QJsonObject &json){
        AnnotationItem::fromJsonObject(json);
        if (json.contains("strokes")){
            QJsonValue value = json.value("strokes");
            if (value.isArray()){
                strokes.clear();
                QJsonArray array = value.toArray();
                for (int i=0;i<array.size();i++){
                    stroke_type stroke;
                    stroke.fromJsonObject(array[i].toObject());
                    strokes.push_back(stroke);
                }
            }
        }
    }

protected:
    QList<stroke_type> strokes;
};

using SegAnnotationItem = Basic_SegAnnotationItem<SegStroke>;

class AnnotationContainer;
extern QImage drawColorImage(const QSize &size,
                             const AnnotationContainer *pAnnoContainer,
                             const LabelManager *pLabelManager);

extern QImage drawLabelIdImage(const QSize &size,
                               const AnnotationContainer *pAnnoContainer,
                               const LabelManager *pLabelManager);


#endif // SEGANNOTATIONITEM_H
