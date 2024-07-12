#ifndef CANVASBASE_H
#define CANVASBASE_H

#include <QGraphicsItem>
#include <QUndoStack>
#include <QObject>
#include <exception>
#include <string>

#include "shape.h"
using namespace YShape;
typedef std::shared_ptr<Shape> ShapePtr;
class CanvasException : public std::exception
{
public:
    CanvasException() : m_msg("CanvasError.") {};
    CanvasException(const std::string& msg) : m_msg(msg) {}
    ~CanvasException() throw() {}
	const char* what() const throw() { return m_msg.c_str(); }
private:
	std::string m_msg;
};

class CanvasBase : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
public:
    enum task_mode_e {
        detection,
        segmentation,
    };
    enum operat_mode_e {
        draw,
        edit,
    };
    CanvasBase(QObject *parent = nullptr);

    void    setSize(QSize size);

    QPointF mapToImg(const QPointF& pt) const;
    QPointF mapFromImg(const QPointF& pt) const;
    QPointF bindImgEdge(const QPointF& pt);
    QRectF boundingRect() const ;

    QRectF imgRectF() const;
    int imgWidth() const { return getImage().width(); };
    int imgHeight() const { return getImage().height(); };
    QImage getImage() const;
    void setImage(const QImage & img);

    const task_mode_e& getTaskMode() const { return m_task; }
    const operat_mode_e& getCanvasMode() const { return m_operat; }
    const draw_mode_e& getDrawMode() const { return m_draw; }
    void changeTask(const task_mode_e& task) { m_task = task; };
    void changeOperatMode(const operat_mode_e &opt) { m_operat = opt;   };
    void changeDrawMode(const draw_mode_e &draw) { m_draw = draw; };

    QUndoStack* undoStack() { return m_undoStack; }
    ShapePtr currentShape() const;
    QString currentShapeName() const;
    QVector<ShapePtr> &getShapeList() { return m_shapeList; };


    QString addShape(const ShapePtr shape);
    void deleteShape(const QString& shapeName);

    QString addPoint(ShapePtr shape, const QPointF& point);
    void deletePoint(ShapePtr shape, const QString& pointName);

    ShapePtr shape(const QString& shapeName) const;

    void setCurrentShape(int index);
    int indexOf(const QString& shapeName) const;
    QString uniqueName(const QString& name) const;

signals:
    void sigCurShapeChanged(const QString& shapeName);
    void sigAddShape();
    void sigAddPoint(const QPointF& point);

    void sigSetProperty(ShapePtr shape);
public slots:
    void clean();
    void zoomIn();
    void zoomOut();

    int getContrast() const { return m_contrast; }
    void setContrast(int contrast);
    int getBrightness() const { return m_brightness; }
    void setBrightness(int brightness);

protected:
    // Í¨¹ý QGraphicsItem ¼Ì³Ð
    void    paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
    void    wheelEvent(QGraphicsSceneWheelEvent* event); 

    virtual void drawShape(QPainter* painter);

    QImage lightContrastImage(const QImage& img,const int &light,const int &contrast);
protected:
    QImage              m_image;
    QImage              m_disImg;
    int                 m_penLineWidth{ 2 };
    const qreal         m_scaleStep{ 0.05 };
    int                 m_currentIndex{ -1 };
    QVector<ShapePtr>   m_shapeList;
    qreal               m_scaleValue{ 1.0f };

    task_mode_e         m_task;
    operat_mode_e       m_operat;
    draw_mode_e         m_draw;
    QUndoStack*         m_undoStack{ nullptr };

    QPointF             m_lastPressPos;
    QPointF             m_mousePos;
    int                 m_activePoint{ -1 };
    bool                m_mouseDrag{ false };
    int                 m_insertPoint{ -1 };

    int                 m_contrast{ 150 };
    int                 m_brightness{ 150 };
};

#endif // CANVASBASE_H
