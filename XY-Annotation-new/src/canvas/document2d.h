#ifndef DOCUMENT2D_H
#define DOCUMENT2D_H
#include <QPixmap>

#include "documentbase.h"

class Document2D : public DocumentBase
{
    Q_OBJECT
public:
    Document2D(LabelManager *labelManager,QWidget *parent=nullptr);

    QString addShape(const ShapePtr shape);
    void deleteShape(const QString &shapeName);

    QString addPoint(ShapePtr shape, const QPointF &point);
    void deletePoint(ShapePtr shape, const QString &pointName);

    ShapePtr shape(const QString &shapeName) const;


private:
    // QWidget interface
protected:
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void mouseDoubleClickEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void wheelEvent(QWheelEvent *event) override;
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void paintEvent(QPaintEvent *event) override;

    // DocumentBase interface
public:
    virtual QSize sizeUnscaled() const override;
    virtual QSize minimumSizeHint() const override;


public slots:
    virtual void setScale(qreal newScale) override;
    virtual void changeTask(task_mode_e task) override;
    virtual void changeOperatMode(operat_mode_e opt) override;
    virtual void changeDrawMode(draw_mode_e draw) override;
    virtual void clean() override;

    void zoomIn();
    void zoomOut();

private:
    /* 坐标相关的函数 */
    // pixmap绘制位置的偏移量，这是由于当pixmap的大小小于画布（窗口）大小时，将pixmap绘制在中央
    QPointF offsetToCenter();
    // 将画布坐标转化为pixmap的像素坐标，具体为 pos / scale - offsetToCenter();
    QPointF pixelPos(QPointF pos);
    // 将画布坐标转化为pixmap的像素坐标，当超出pixmap的范围时，将其限制在pixmap边界上
    QPointF boundedPixelPos(QPointF pos);
    // 判断像素坐标pos是否超出pixmap的范围
    bool outOfPixmap(QPointF pos);



    void setCurrentShape(int index);
    int indexOf(const QString &shapeName) const;
    QString uniqueName(const QString &name) const;


};

#endif // DOCUMENT2D_H
