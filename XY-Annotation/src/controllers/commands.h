#ifndef COMMANDS_H
#define COMMANDS_H

#include <QUndoCommand>
#include <canvas2d.h>

class AddShapeCommand : public QUndoCommand
{
public:
    AddShapeCommand(Canvas2D *doc, const SegStroke &shape,
                    QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;

private:
    Canvas2D *m_doc;
    SegStroke m_shape;
    QString m_shapeName;
};
#endif // COMMANDS_H
