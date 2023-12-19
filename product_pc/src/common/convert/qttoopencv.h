#ifndef QTTOOPENCV_H
#define QTTOOPENCV_H
#include <comdll.h>
#include <QObject>

class Q_API_FUNCTION QtToOpencv
{
public:
    QtToOpencv();
    static QImage ResizeRgb(const QImage &image, double value);
};

#endif // QTTOOPENCV_H
