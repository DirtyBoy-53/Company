#ifndef COMMANDS_H
#define COMMANDS_H

#include <QUndoCommand>
#include "CanvasBase.h"
#include "CanvasBase.h"

class AddShapeCommand : public QUndoCommand
{
public:
    AddShapeCommand(CanvasBase *canvas, const ShapePtr shape,
                    QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;

private:
    CanvasBase *m_canvas;
    ShapePtr m_shape;
    QString m_shapeName;
};

class RemoveShapeCommand : public QUndoCommand
{
public:
    RemoveShapeCommand(CanvasBase *canvas, const QString shapeName,
                    QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;

private:
    CanvasBase *m_canvas;
    ShapePtr m_shape;
    QString m_shapeName;
};

class AddPointCommand : public QUndoCommand
{
public:
    AddPointCommand(CanvasBase *canvas, const ShapePtr shape, const QPointF &point,
                    QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;

private:
    CanvasBase *m_canvas;
    ShapePtr m_shape;
    QPointF m_point;
    QString m_pointName;
};

class RemovePointCommand : public QUndoCommand
{
public:
    RemovePointCommand(CanvasBase *canvas, const ShapePtr shape, const QPointF &point,
                       QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;

private:
    CanvasBase *m_canvas;
    ShapePtr m_shape;
    QPointF m_point;
    QString m_pointName;
};
#endif // COMMANDS_H
