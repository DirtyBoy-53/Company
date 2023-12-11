#include "common.h"

#include <QIcon>
#include <cmath>
#include <ctime>
#include <cstdlib>


//! Reference: https://gist.github.com/ialhashim/b39a68cf48a0d2e66621
QList<QColor> ColorUtils::randomColors(int count){
    static qreal currentHue = static_cast<qreal>(qrand())/RAND_MAX;
    QList<QColor> colors;
    for (int i = 0; i < count; i++){
        currentHue += 0.618033988749895;
        currentHue = std::fmod(currentHue, 1.0);
        colors.push_back( QColor::fromHslF(currentHue, 1.0, 0.5) );
    }
    return colors;
}

QColor ColorUtils::randomColor(){
    static qreal currentHue = static_cast<qreal>(qrand())/RAND_MAX;
    currentHue += 0.618033988749895;
    currentHue = std::fmod(currentHue, 1.0);
    return QColor::fromHslF(currentHue, 1.0, 0.5);
}

QIcon ColorUtils::iconFromColor(QColor color, QSize size)
{
    QPixmap pixmap(size);
    if (color.isValid()){
        pixmap.fill(color);
    }else{
        pixmap.fill(Qt::white);
    }
    return QIcon(pixmap);
}

bool CanvasUtils::onRectTop(QPoint pos, QRect rect){
    return rect.left()<=pos.x() && pos.x()<=rect.right() && abs(pos.y()-rect.top())<=pixEps;
}

bool CanvasUtils::onRectBottom(QPoint pos, QRect rect){
    return rect.left()<=pos.x() && pos.x()<=rect.right() && abs(pos.y()-rect.bottom())<=pixEps;
}

bool CanvasUtils::onRectLeft(QPoint pos, QRect rect){
    return rect.top()<=pos.y() && pos.y()<=rect.bottom() && abs(pos.x()-rect.left())<=pixEps;
}

bool CanvasUtils::onRectRight(QPoint pos, QRect rect){
    return rect.top()<=pos.y() && pos.y()<=rect.bottom() && abs(pos.x()-rect.right())<=pixEps;
}

DrawMode StringConstants::getDrawModeFromText(const QString &text){
    for (auto item: drawModeText)
        if (item.second==text)
            return item.first;
    throw "can not find text "+text+" for map drawModeText";
}

TaskMode StringConstants::getTaskFromText(const QString &text){
    for (auto item: taskText)
        if (item.second==text)
            return item.first;
    throw "can not find text "+text+" for map taskText";
}
