#include "documentbase.h"


DocumentBase::DocumentBase(LabelManager *labelManager, QWidget *parent)
    : QWidget(parent)
    , m_labelManager(labelManager)
    , m_undoStack(new QUndoStack(this))
{

}


QString DocumentBase::currentShapeName() const
{
    if(m_currentIndex == -1)
        return QString();
    return m_shapeList.at(m_currentIndex).get()->name();
}

ShapePtr DocumentBase::currentShape() const
{
    if(m_currentIndex == -1)
        return nullptr;
    return m_shapeList.at(m_currentIndex);
}

QVector<ShapePtr> DocumentBase::getShapeList() const
{
    return m_shapeList;
}

draw_mode_e DocumentBase::getDraw() const
{
    return m_draw;
}

