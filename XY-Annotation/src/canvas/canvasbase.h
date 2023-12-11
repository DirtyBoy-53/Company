#ifndef CANVASBASE_H
#define CANVASBASE_H

#include <QObject>
#include <QWidget>


enum TaskMode{
    DETECTION, SEGMENTATION, DETECTION3D, SEGMENTATION3D
};

enum CanvasMode{
    DRAW, SELECT, MOVE
};

enum DrawMode{
    RECTANGLE,
    CONTOUR, SQUAREPEN, CIRCLEPEN, POLYGEN
};

class AnnotationContainer;
class LabelManager;
class CanvasBase : public QWidget
{
    Q_OBJECT
public:
    explicit CanvasBase(const LabelManager *LabelManager, const AnnotationContainer *AnnoContainer, QWidget *parent = nullptr);

    TaskMode getTaskMode() const { return task; }
    CanvasMode getCanvasMode() const { return mode; }
    DrawMode getDrawMode() const { return drawMode; }
    qreal getScale() const { return scale; }
    int getLastPenWidth() const { return lastPenWidth; }


    QSize sizeHint() const override { return minimumSizeHint(); }
    QSize minimumSizeHint() const override = 0;

    virtual QSize sizeUnscaled() const = 0;

    virtual QString modeString() const;



signals:
    void modeChanged(QString mode);

public slots:
    virtual void setScale(qreal newScale) = 0;
    virtual void setPenWidth(int) = 0;
    virtual void changeTask(TaskMode _task) = 0;
    virtual void changeCanvasMode(CanvasMode _mode) = 0;
    virtual void changeDrawMode(DrawMode _draw) = 0;

    virtual void close() = 0;

protected:

    qreal scale;


    TaskMode task;
    CanvasMode mode;
    DrawMode drawMode;


    const AnnotationContainer *pAnnoContainer;
    const LabelManager* pLabelManager;

    int lastPenWidth;
    int curPenWidth;
};

#endif // CANVASBASE_H
