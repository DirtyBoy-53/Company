#include "CanvasView.h"

#include <QGraphicsScene>
#include <QUndoGroup>
#include <QDebug>

#include "Canvas2D.h"
#include "CanvasBase.h"
#include "shapefactory.h"
#include "commands.h"

CanvasView::CanvasView(QWidget* parent)
	: QGraphicsView(parent)
	, m_scene(new QGraphicsScene(this))
	, m_undoGroup(new QUndoGroup(this))
{
	try {
		m_canvas = new Canvas2D(this);
	} catch (const std::exception& e) {
		qWarning() << e.what();
	}
	
	initUI();
	initConnect();
}

CanvasView::~CanvasView()
{
}

void CanvasView::adjustFitWindow()
{
	m_canvas->setSize(size());
	m_canvas->setPos(mapToScene(rect().center()));
}

void CanvasView::addShape()
{
	qInfo("addShape");
	ShapePtr newShape = ShapeFactory::create(m_canvas->getDrawMode());
	newShape->setimgWH(m_canvas->getImage().size());
	m_canvas->undoStack()->push(new AddShapeCommand(m_canvas, newShape));
}

void CanvasView::removeShape()
{
	QString shapeName = m_canvas->currentShapeName();
	if (shapeName.isEmpty()) return;
	m_canvas->undoStack()->push(new RemoveShapeCommand(m_canvas, shapeName));
}

void CanvasView::addPoint(const QPointF& point)
{
	auto shape = m_canvas->currentShape();
	if (nullptr == shape) return;
	m_canvas->undoStack()->push(new AddPointCommand(m_canvas, shape, point));
}

void CanvasView::removePoint()
{
}

void CanvasView::saveFile()
{
}

void CanvasView::loadImage(const QImage& img)
{
	m_canvas->setImage(img);
	m_scene->setSceneRect(m_canvas->boundingRect());
	adjustFitWindow();
}

void CanvasView::clean()
{
	m_scene->setBackgroundBrush(Qt::white);
	m_scene->update();
}

void CanvasView::initUI()
{
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	m_scene->setBackgroundBrush(Qt::white);
	m_scene->addItem(m_canvas);

	setScene(m_scene);

	m_undoGroup->setActiveStack(m_canvas->undoStack());
}

void CanvasView::initConnect()
{
	connect(m_canvas, &Canvas2D::sigAddShape, this, &CanvasView::addShape);
	connect(m_canvas, &Canvas2D::sigAddPoint, this, &CanvasView::addPoint);
}

void CanvasView::resizeEvent(QResizeEvent* event)
{
	Q_UNUSED(event)
	try {
		adjustFitWindow();
	} catch (const std::exception& e) {
		qWarning() << e.what();
	} catch (...) {
		qWarning() << "unknown error";
	}
	
}
