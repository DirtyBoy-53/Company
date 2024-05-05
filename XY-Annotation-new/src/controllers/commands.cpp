#include "commands.h"


/******************************************************************************
 * AddShapeCommand
 */
AddShapeCommand::AddShapeCommand(Document2D *doc, const ShapePtr shape, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_doc(doc), m_shape(shape)
{}

void AddShapeCommand::undo()
{
    m_doc->deleteShape(m_shapeName);
    setText(QObject::tr("形状 %1").arg(m_shapeName));
}

void AddShapeCommand::redo()
{
    // A shape only gets a name when it is inserted into a document
    m_shapeName = m_doc->addShape(m_shape);
    setText(QObject::tr("形状 %1").arg(m_shapeName));
}


/******************************************************************************
 * RemoveShapeCommand
 */
RemoveShapeCommand::RemoveShapeCommand(Document2D *doc, const QString shapeName, QUndoCommand *parent)
    : QUndoCommand(parent), m_doc(doc), m_shape(doc->shape(shapeName))
    , m_shapeName(shapeName)
{}

void RemoveShapeCommand::undo()
{
    m_shapeName = m_doc->addShape(m_shape);
     setText(QObject::tr("形状 %1").arg(m_shapeName));
}

void RemoveShapeCommand::redo()
{
    m_doc->deleteShape(m_shapeName);
    setText(QObject::tr("形状 %1").arg(m_shapeName));
}

/******************************************************************************
 * AddPointCommand
 */
AddPointCommand::AddPointCommand(Document2D *doc, const ShapePtr shape,
                                 const QPointF &point, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_doc(doc), m_shape(shape), m_point(point)
{}

void AddPointCommand::undo()
{
    m_doc->deletePoint(m_shape, m_pointName);
    setText(QObject::tr("点 %1").arg(m_pointName));
}

void AddPointCommand::redo()
{
    m_pointName = m_doc->addPoint(m_shape, m_point);
    setText(QObject::tr("点 %1").arg(m_pointName));
}


/******************************************************************************
 * RemovePointCommand
 */

RemovePointCommand::RemovePointCommand(Document2D *doc, const ShapePtr shape,
                                       const QPointF &point, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_doc(doc), m_shape(shape), m_point(point)
{ }

void RemovePointCommand::undo()
{

}

void RemovePointCommand::redo()
{

}
