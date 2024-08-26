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
        offset.setX(size.width()/2.0);
        offset.setY(size.height()/2.0);
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

    YPolygonPoints& operator = (const YPolygonPoints & other) {
        if (this == &other) return *this;
        m_imgPoints = other.m_imgPoints;
        m_oriPoints = other.m_oriPoints;
        offset      = other.offset;
    }
};


class Shape : public QObject
{
    Q_OBJECT
public:
    explicit            Shape(const draw_mode_e& type);
    virtual             ~Shape(){}
    static std::string  drawModeToStr(const draw_mode_e &type);
    QString             name()      const;
    int                 id()        const;
    QColor              color()     const;
    bool                isClosed()  const;
    bool                isDrag()    const;
    bool                isPress()   const;
    bool                isActive()  const;
    bool                isSelect()  const;
    LabelProperty       *label()    const;

    void                setIsClosed(bool isClosed);
    void                setColor(const QColor &color);
    void                setLabel(const LabelProperty &label);
    void                setIsDrag(bool isDrag);
    void                setIsPress(bool isPress);
    void                setIsActive(bool active);
    void                setIsSelect(bool select);
    void                setControlPtActive(int active);

    virtual void        draw(QPainter *p, bool isdisEndPt=false, bool fill=true)=0;

//    QJsonObject toJsonObject();

    virtual bool        isPtsHavPt(const QPointF &pt,int &pos);
    virtual bool        isEdgeHavPt(const QPointF &pt,int &pos);
    virtual bool        isAreaHavPt(const QPointF &pt);
    virtual bool        updatePoint(const QPointF &pt,int &pos);
    virtual bool        insertPoint(const QPointF &pt,int &pos);

    static QString      typeToString(draw_mode_e type);
    static draw_mode_e  stringToType(QString &str,bool &result);

    qreal               lineWidth()     const;
    void                setLineWidth(qreal newLineWidth);

    int                 pointSize()     const;
    void                setPointSize(int newPointSize);

    int                 pointCount()    const;

    bool                move(const QPointF &point);
    draw_mode_e         type()          const;
    void                setType(draw_mode_e newType);

    void                setName(const QString &newName);
    void                appendPoint(const QPointF &point);
    void                updateEndPt(const QPointF &point);

    void                deletePoint(const int &index);

    const YPolygonPoints&     points() const;

    void                setimgWH(QSize size = QSize(640,512)) { m_points.setImgWH(size); };
    void                setClosed() { m_isClosed = true; }
protected:
    YPolygonPoints      m_points;
    draw_mode_e         m_type{YShape::None};
    QColor              m_color;
    QString             m_name{""};

    int                 m_id{0};

    bool                m_isClosed{false};
    bool                m_isDrag{false};
    bool                m_isPress{false};
    bool                m_isActive{false};
    bool                m_isSelect{ false };
    int                 m_controlPtActive{-1};

    int                 m_senseDis{8};      //鼠标敏感距离
    qreal               m_lineWidth{1};     //线宽
    qreal               m_pointSize{2.5};   //点大小

    QPointF             m_disPoit;//Move the mouse to display the point
    LabelProperty       *m_label{nullptr};
};
}


#endif // SHAPE_H
