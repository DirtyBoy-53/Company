#include "ycommon.h"

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

