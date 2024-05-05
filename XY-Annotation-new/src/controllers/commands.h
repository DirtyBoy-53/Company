#ifndef COMMANDS_H
#define COMMANDS_H

#include <QUndoCommand>
#include "documentbase.h"
#include "document2d.h"

class AddShapeCommand : public QUndoCommand
{
public:
    AddShapeCommand(Document2D *doc, const ShapePtr shape,
                    QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;

private:
    Document2D *m_doc;
    ShapePtr m_shape;
    QString m_shapeName;
};

class RemoveShapeCommand : public QUndoCommand
{
public:
    RemoveShapeCommand(Document2D *doc, const QString shapeName,
                    QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;

private:
    Document2D *m_doc;
    ShapePtr m_shape;
    QString m_shapeName;
};

class AddPointCommand : public QUndoCommand
{
public:
    AddPointCommand(Document2D *doc, const ShapePtr shape, const QPointF &point,
                    QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;

private:
    Document2D *m_doc;
    ShapePtr m_shape;
    QPointF m_point;
    QString m_pointName;
};

class RemovePointCommand : public QUndoCommand
{
public:
    RemovePointCommand(Document2D *doc, const ShapePtr shape, const QPointF &point,
                       QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;

private:
    Document2D *m_doc;
    ShapePtr m_shape;
    QPointF m_point;
    QString m_pointName;
};
#endif // COMMANDS_H
