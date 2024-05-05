#ifndef DOCUMENTBASE_H
#define DOCUMENTBASE_H

#include <QWidget>
#include <QUndoStack>
#include "shape.h"

using namespace YShape;
typedef std::shared_ptr<Shape> ShapePtr;
class DocumentBase : public QWidget
{
    Q_OBJECT
public:
    enum task_mode_e{
        detection,
        segmentation,
    };
    enum operat_mode_e{
        draw,
        edit,
    };

    explicit DocumentBase(LabelManager *labelManager,QWidget *parent = nullptr);

    const task_mode_e& getTaskMode() const { return m_task; }
    const operat_mode_e& getCanvasMode() const { return m_operat; }
    const draw_mode_e& getDrawMode() const { return m_draw; }
    const qreal& getScale() const { return m_scale; }
    QUndoStack *undoStack() { return m_undoStack;}
    void loadPixmap(QPixmap pix){ m_pixmap = pix;adjustSize();update();}
    const QPixmap &getPixmap() const { return m_pixmap; }
    QString currentShapeName() const;
    ShapePtr currentShape() const;


    // 这两个重载函数对于adjustSize()以及layout等是必须的
    QSize sizeHint() const override { return minimumSizeHint(); }
    QSize minimumSizeHint() const override = 0;

    // 返回未缩放情况下的画布大小，可用于适应窗口大小（fitWindow）等
    virtual QSize sizeUnscaled() const = 0;



signals:
    void currentShapeChanged(const QString &shapeName);
    void sigAddShape();
    void sigAddPoint(const QPointF &point);
public slots:
    virtual void setScale(qreal newScale) = 0;
    virtual void changeTask(task_mode_e task) = 0;
    virtual void changeOperatMode(operat_mode_e opt) = 0;
    virtual void changeDrawMode(draw_mode_e draw) = 0;
    virtual void clean() = 0;
protected:
    int m_currentIndex{-1};
    QVector<ShapePtr> m_shapeList;
    qreal m_scale{1.0f};
    QPixmap m_pixmap;
    task_mode_e m_task;
    operat_mode_e m_operat;
    draw_mode_e m_draw;
    QUndoStack *m_undoStack{nullptr};
    LabelManager *m_labelManager{nullptr};
};

#endif // DOCUMENTBASE_H
