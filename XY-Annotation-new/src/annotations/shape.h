#ifndef SHAPE_H
#define SHAPE_H
#include <QString>
#include <QColor>
#include <QPainter>
#include <QJsonObject>
#include <QPointF>
#include <QVector>
#include <QObject>
#include <memory>

QT_FORWARD_DECLARE_CLASS(LabelManager)
QT_FORWARD_DECLARE_CLASS(LabelProperty)


class ShapeException : public std::exception{
public:
    ShapeException() : m_msg("ShapeError.") {};
    ShapeException(const std::string& msg) : m_msg(msg) {}
    ~ShapeException() throw() {}
    const char* what() const throw() { return m_msg.c_str(); }
private:
    std::string m_msg;
};

namespace YShape {
enum draw_mode_e{
    Rectangle,
    Line,
    Curve,
    Polygon,
    None,
};

struct YPolygonPoints
{
private:
    QPolygonF m_imgPoints;//图像点-坐标原点在图像左上角
    QPolygonF m_oriPoints;//原始点-坐标原点在图像中心
    QPointF offset;
public:
    void setImgWH(const QSize &size) {
        offset.setX(size.width());
        offset.setY(size.height());
    }
    void append(const QPointF& point) {
        m_oriPoints.append(point);
        m_imgPoints.append(point + offset);
    }
    const QPolygonF getImgPoints() const {
        return m_imgPoints;
    }
    const QPolygonF getOriPoints() const {
        return m_oriPoints;
    }
    inline const int size() const {
        return m_oriPoints.size();
    }
    inline const QPointF at(const int& idx);
    inline void replace(const QPointF& point, int& pos);
    inline void insert(const QPointF& point, int& pos);
    
    void erase(const int& idx);

    bool containsPoint(const QPointF& point);
};


class Shape : public QObject
{
    Q_OBJECT
public:
    explicit Shape(const draw_mode_e& type);
    virtual ~Shape(){}
    static std::string drawModeToStr(const draw_mode_e &type);
    QString name()  const;
    int id()        const;
    QColor color()  const;
    bool isClosed() const;
    bool isDrag()   const;
    bool isSelect() const;
    LabelProperty *label() const;

    void setIsClosed(bool isClosed);
    void setColor(const QColor &color);
    void setLabel(const LabelProperty &label);
    void setIsDrag(bool isDrag);
    void setIsSelect(bool isSelect);

    virtual void draw(QPainter *p, bool isdisEndPt=false, bool fill=true)=0;

//    QJsonObject toJsonObject();

    virtual bool isPtsHavPt(const QPointF &pt,int &pos);
    virtual bool isEdgeHavPt(const QPointF &pt,int &pos);
    virtual bool isAreaHavPt(const QPointF &pt);
    virtual bool updatePoint(const QPointF &pt,int &pos);
    virtual bool insertPoint(const QPointF &pt,int &pos);

    static QString typeToString(draw_mode_e type);
    static draw_mode_e stringToType(QString &str,bool &result);

    int lineWidth() const;
    void setLineWidth(int newLineWidth);

    int pointSize() const;
    void setPointSize(int newPointSize);

    bool move(const QPointF &point);
    draw_mode_e type() const;
    void setType(draw_mode_e newType);

    void setName(const QString &newName);
    void appendPoint(const QPointF &point);
    void updateEndPt(const QPointF &point);

    void deletePoint(const int &index);

    const YPolygonPoints &points() const;

    void setimgWH(QSize size = QSize(640,512)) { m_points.setImgWH(size); };
protected:
    YPolygonPoints m_points;
    draw_mode_e m_type{YShape::None};
    QColor m_color;
    QString m_name{""};

    int m_id{0};

    bool m_isClosed{false};
    bool m_isDrag{false};
    bool m_isSelect{false};


    int m_senseDis{8};
    int m_lineWidth{3};
    int m_pointSize{3};

    QPointF m_disPoit;//Move the mouse to display the point
    LabelProperty *m_label{nullptr};
};
}


#endif // SHAPE_H
