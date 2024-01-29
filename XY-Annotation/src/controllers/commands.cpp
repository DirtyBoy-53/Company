#include "commands.h"


AddShapeCommand::AddShapeCommand(Canvas2D *doc, const SegStroke &shape, QUndoCommand *parent)
    : QUndoCommand(parent), m_doc(doc), m_shape(shape)
{}

void AddShapeCommand::undo()
{
//    m_doc
}

void AddShapeCommand::redo()
{

}
