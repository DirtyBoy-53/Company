#include "canvaswidget.h"
#include <QFileDialog>
#include "shape.h"
#include "shapefactory.h"
#include "commands.h"
CanvasWidget::CanvasWidget(QWidget *parent)
    : QScrollArea(parent)
    , m_undoGroup (new QUndoGroup(this))
{
    m_doc = new Document2D(&m_labelManager);
    initUI();
    initConnect();
}

CanvasWidget::~CanvasWidget()
{

}

qreal CanvasWidget::scaleFitWindow() const
{
    int w1 = this->width() - 2; // -2 So that no scrollbars are generated.
    int h1 = this->height() - 2;
    qreal a1 = static_cast<qreal>(w1)/h1;
    int w2 = m_doc->sizeUnscaled().width();
    int h2 = m_doc->sizeUnscaled().height();
    qreal a2 = static_cast<qreal>(w2)/h2;
    return a2>=a1 ? static_cast<qreal>(w1)/w2 : static_cast<qreal>(h1)/h2;
}

void CanvasWidget::adjustFitWindow()
{
    m_doc->setScale(scaleFitWindow());
}

void CanvasWidget::addShape()
{
    qInfo("addShape");
    ShapePtr newShape = ShapeFactory::create(m_doc->getDrawMode());
    m_doc->undoStack()->push(new AddShapeCommand(m_doc,newShape));
}

void CanvasWidget::removeShape()
{
    QString shapeName = m_doc->currentShapeName();
    if (shapeName.isEmpty()) return;
    m_doc->undoStack()->push(new RemoveShapeCommand(m_doc,shapeName));
}

void CanvasWidget::addPoint(const QPointF &point)
{
    auto shape = m_doc->currentShape();
    if(nullptr == shape) return;
    m_doc->undoStack()->push(new AddPointCommand(m_doc, shape, point));

}

void CanvasWidget::removePoint()
{

}

void CanvasWidget::initUI()
{
    setAlignment(Qt::AlignCenter);
    setWidget(m_doc);
    m_doc->setVisible(true);
    m_doc->setEnabled(true);

    m_undoGroup->setActiveStack(m_doc->undoStack());

}

void CanvasWidget::initConnect()
{
    connect(m_doc,&Document2D::sigAddShape,this,&CanvasWidget::addShape);
    connect(m_doc,&Document2D::sigAddPoint,this,&CanvasWidget::addPoint);
}

void CanvasWidget::loadPixmap()
{
    QString fileName = QFileDialog::getOpenFileName(this, "open a file", "/",
                                                    "Image Files (*.jpg *.png *.bmp);;JPEG Files (*.jpg);;PNG Files (*.png);;BMP Files (*.bmp)");
    if (!fileName.isNull() && !fileName.isEmpty()){

        m_doc->loadPixmap(fileName);
        adjustFitWindow();
    }
}
