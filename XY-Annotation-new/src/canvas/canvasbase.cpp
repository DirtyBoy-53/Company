#include "CanvasBase.h"
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QUndoStack>
CanvasBase::CanvasBase(QObject* parent) 
    : QObject(parent)
    , m_undoStack(new QUndoStack)
{
    setAcceptHoverEvents(true);
    setFlags(QGraphicsItem::ItemIsFocusable);
}

void CanvasBase::setSize(QSize size)
{
    m_scaleValue = 1;

    qreal a1 = size.width() / (imgWidth() * 1.0);
    qreal a2 = size.height() / (imgHeight() * 1.0);
    m_scaleValue = (a1 < a2 ? a1 : a2)*0.98;
    setScale(m_scaleValue);
}

QPointF CanvasBase::mapToImg(const QPointF& pt) const
{
    QPointF offset(imgWidth() / 2, imgHeight() / 2);
    return pt + offset;
}

QPointF CanvasBase::mapFromImg(const QPointF& pt) const
{
    QPointF offset(imgWidth() / 2, imgHeight() / 2);
    return pt - offset;
}

QPointF CanvasBase::bindImgEdge(const QPointF& pt)
{
    auto point = mapToImg(pt);
    auto w = point.x(), h = point.y();
    double edge = 2 * m_penLineWidth / 3.0;
    w = w >= (imgWidth() - edge) ? (imgWidth() - edge) : w;
    w = w <= edge ? edge : w;
    h = h >= (imgHeight() - edge) ? (imgHeight() - edge) : h;
    h = h <= edge ? edge : h;
    return mapFromImg(QPointF(w, h));
}

QRectF CanvasBase::boundingRect() const
{
    if (m_image.isNull())
        return QRectF(-640/2, -512/2, 640, 512);
    return QRectF(-imgWidth() / 2, -imgHeight() / 2, imgWidth(), imgHeight());
}

QRectF CanvasBase::imgRectF() const
{
    if(m_image.isNull())
		return QRectF(-640, -512, 640/2, 512/2);
    return QRectF(-imgWidth() / 2, -imgHeight() / 2, imgWidth()/2, imgHeight()/2);
}

QImage CanvasBase::getImage() const
{
    if (m_image.isNull())
        throw CanvasException(std::string(__FUNCDNAME__) + ":image is null");
    return m_image;
}

void CanvasBase::setImage(const QImage& img)
{
    if (img.isNull())
        throw CanvasException(std::string(__FUNCDNAME__) + ":image is null");
    m_image = img;
    m_disImg = m_image;
    update();
}

ShapePtr CanvasBase::currentShape() const
{
    if (m_currentIndex == -1)
        return nullptr;
    return m_shapeList.at(m_currentIndex);
}

QString CanvasBase::currentShapeName() const
{
    if (m_currentIndex == -1)
        return QString();
    return m_shapeList.at(m_currentIndex).get()->name();
}

QString CanvasBase::addShape(const ShapePtr shape)
{
    QString name = Shape::typeToString(shape.get()->type());
    name = uniqueName(name);

    m_shapeList.append(shape);
    m_shapeList[m_shapeList.count() - 1].get()->setName(name);
    setCurrentShape(m_shapeList.count() - 1);
    update();
    return name;
}

void CanvasBase::deleteShape(const QString& shapeName)
{
    int index = indexOf(shapeName);
    if (index == -1)
        return;
    m_shapeList.removeAt(index);

    if (index <= m_currentIndex) {
        m_currentIndex = -1;
        if (index == m_shapeList.count())
            --index;
        setCurrentShape(index);
    }
    update();
}

QString CanvasBase::addPoint(ShapePtr shape, const QPointF& point)
{
    if (shape == nullptr) return "";
    shape.get()->appendPoint(point);
    int curPos = shape.get()->points().size() - 1;
    update();
    return QString::number(curPos);
}

void CanvasBase::deletePoint(ShapePtr shape, const QString& pointName)
{
    auto curPos = pointName.toInt();
    shape.get()->deletePoint(curPos);
    update();
}

ShapePtr CanvasBase::shape(const QString& shapeName) const
{
    int index = indexOf(shapeName);
    if (index == -1)
        return nullptr;
    return m_shapeList.at(index);
}

void CanvasBase::setCurrentShape(int index)
{
    QString currentName;
    m_currentIndex = index;
    if (m_currentIndex != -1) {
        const ShapePtr curShape = currentShape();
        currentName = curShape.get()->name();
    }
    emit sigCurShapeChanged(currentName);
}

int CanvasBase::indexOf(const QString& shapeName) const
{
    for (int i = 0; i < m_shapeList.count(); ++i) {
        if (m_shapeList.at(i).get()->name() == shapeName)
            return i;
    }
    return -1;
}

QString CanvasBase::uniqueName(const QString& name) const
{
    QString unique;
    for (int i = 0;; ++i) {
        unique = name;
        if (i > 0)
            unique += QString::number(i);
        if (indexOf(unique) == -1)
            break;
    }
    return unique;
}

void CanvasBase::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    if(m_disImg.isNull()) return;
    painter->drawImage(imgRectF().topLeft(), m_disImg);
    drawShape(painter);
}


void CanvasBase::wheelEvent(QGraphicsSceneWheelEvent* event)
{ 
    qreal lastScale = m_scaleValue;
    if (event->delta() > 0) zoomIn();
    else                    zoomOut();
    if(lastScale == m_scaleValue) return;
    int dir = event->delta() > 0 ? -1 : 1;//获取滚轮滚动方向
    auto x = event->pos().x(), y = event->pos().y();
    auto scale = dir * lastScale * m_scaleStep;

    //使图片缩放的效果看起来像是以鼠标所在点为中心进行缩放的
    moveBy(x * scale, y * scale);
}

void CanvasBase::drawShape(QPainter* painter){ Q_UNUSED(painter) }

int Bound(int range_left, int data, int range_right)
{
    int index = data;
    if (data > range_right) {
        index = range_right;
    }
    else if (data < range_left) {
        index = range_left;
    }
    return index;
}

QImage CanvasBase::lightContrastImage(const QImage& img, const int& light, const int& contrast)
{
    QImage imgCopy;
    if (img.format() != QImage::Format_RGB888) {
        imgCopy = QImage(img).convertToFormat(QImage::Format_RGB888);
    }
    else {
        imgCopy = QImage(img);
    }
    uint8_t* rgb = imgCopy.bits();
    if (nullptr == rgb) {
        return QImage();
    }
    int r, g, b;
    int size = img.width() * img.height();
    for (int i = 0; i < size; i++) {
        r = light * 0.01 * rgb[i * 3] - 150 + contrast;
        g = light * 0.01 * rgb[i * 3 + 1] - 150 + contrast;
        b = light * 0.01 * rgb[i * 3 + 2] - 150 + contrast;
        r = Bound(0, r, 255);
        g = Bound(0, g, 255);
        b = Bound(0, b, 255);
        rgb[i * 3] = r;
        rgb[i * 3 + 1] = g;
        rgb[i * 3 + 2] = b;
    }
    return imgCopy;
}

//放大
void CanvasBase::zoomIn()
{
    if (m_scaleValue >= 10)//最大放大到原始图像的50倍
        return;
    m_scaleValue *= (1 + m_scaleStep);//每次放大1.05倍
    setScale(m_scaleValue);//设置缩放值
}

//缩小
void CanvasBase::zoomOut()
{
    if (m_scaleValue <= m_scaleStep)//最小缩小到原始图像的0.05倍
        return;
    m_scaleValue *= (1 - m_scaleStep);//每次缩小0.95倍
    setScale(m_scaleValue);//设置缩放值
}

void CanvasBase::setContrast(int contrast)
{
    m_contrast = contrast;
    m_disImg = lightContrastImage(m_image, m_brightness, m_contrast);
    update();
}

void CanvasBase::setBrightness(int brightness)
{
    m_brightness = brightness;
    m_disImg = lightContrastImage(m_image, m_brightness, m_contrast);
    update();
}

void CanvasBase::clean()
{
    m_image = QImage();
    update();
}