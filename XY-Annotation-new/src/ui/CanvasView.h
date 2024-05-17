#ifndef CANVASVIEW_H
#define CANVASVIEW_H

#include <QGraphicsView>

QT_FORWARD_DECLARE_CLASS(CanvasBase)
QT_FORWARD_DECLARE_CLASS(QUndoGroup)

class CanvasView : public QGraphicsView
{
    Q_OBJECT

public:
    CanvasView(QWidget* parent = nullptr);
    ~CanvasView();

    void adjustFitWindow();
    CanvasBase* canvas() const { return m_canvas; };
    QUndoGroup* undoGroup() const { return m_undoGroup; };


public slots:
    void addShape();
    void removeShape();
    void addPoint(const QPointF& point);
    void removePoint();
    void saveFile();
    void loadImage(const QString& fileName);

protected:
    void initUI();
    void initConnect();

    virtual void resizeEvent(QResizeEvent* event) override;

private:
    QGraphicsScene      *m_scene; // 定义一个场景，设置背景色为红色
    CanvasBase          *m_canvas{ nullptr };
    QUndoGroup          *m_undoGroup{ nullptr };
};
#endif // WIDGET_H
