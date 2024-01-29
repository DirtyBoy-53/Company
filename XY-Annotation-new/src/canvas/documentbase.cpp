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
