#ifndef CANVAS2D_H
#define CANVAS2D_H

#include "canvasbase.h"
#include "segannotationitem.h"
#include "common.h"
#include <QRect>

class Canvas2D : public CanvasBase
{
    Q_OBJECT
public:
    explicit Canvas2D(const LabelManager *pLabelManager, const AnnotationContainer *pAnnoContainer, QWidget *parent=nullptr);

    QSize minimumSizeHint() const override;
    QSize sizeUnscaled() const override { return pixmap.size(); }

    const QPixmap &getPixmap() const { return pixmap; }

    void deleteShape(const QString &shapeName);
protected:
    void paintEvent(QPaintEvent*) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

signals:

    void mouseMoved(QPoint pos);

    void newRectangleAnnotated(QRect newRect);

    void newStrokesAnnotated(const QList<SegStroke> &strokes);

    void modifySelectedRectRequest(int idx, QRect rect);

    void removeRectRequest(int idx);

public slots:
    void setScale(qreal) override;
    void setPenWidth(int width) override;
    void changeTask(TaskMode _task) override;
    void changeCanvasMode(CanvasMode _mode) override;
    void changeDrawMode(DrawMode _draw) override;
    void close() override;

    void loadPixmap(QPixmap);

private:
    QPixmap pixmap;
    QPoint mousePos;

    QPoint offsetToCenter();

    QPoint pixelPos(QPoint pos);

    QPoint boundedPixelPos(QPoint pos);

    bool outOfPixmap(QPoint pos);


    QList<QPoint> curPoints;


    bool rectEditing;
    QRect editedRect;
    CanvasUtils::EditingRectEdge editedRectEdge;

    int selectShape(QPoint pos);


    bool strokeDrawing;
    int m_activePoint{-1};
    bool m_mouseDrag{false};
    int m_insertPoint{-1};
    SegStroke *m_curStroke{nullptr};
    QList<SegStroke> curStrokes;
};

#endif // CANVAS2D_H
