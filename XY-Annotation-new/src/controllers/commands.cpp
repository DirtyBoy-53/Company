#include "commands.h"


/******************************************************************************
** AddShapeCommand
*/
AddShapeCommand::AddShapeCommand(Document2D *doc, const ShapePtr shape, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_doc(doc), m_shape(shape)
{}

void AddShapeCommand::undo()
{
    m_doc->deleteShape(m_shapeName);
}

void AddShapeCommand::redo()
{
    // A shape only gets a name when it is inserted into a document
    m_shapeName = m_doc->addShape(m_shape);
    setText(QObject::tr("Add %1").arg(m_shapeName));
}


/******************************************************************************
** RemoveShapeCommand
*/
RemoveShapeCommand::RemoveShapeCommand(Document2D *doc, const QString shapeName, QUndoCommand *parent)
    : QUndoCommand(parent), m_doc(doc), m_shape(doc->shape(shapeName))
    , m_shapeName(shapeName)
{
    setText(QObject::tr("Remove %1").arg(shapeName));
}

void RemoveShapeCommand::undo()
{
    m_shapeName = m_doc->addShape(m_shape);
}

void RemoveShapeCommand::redo()
{
    m_doc->deleteShape(m_shapeName);
}
