#ifndef QTTOOPENCV_H
#define QTTOOPENCV_H

#include "opencv2/opencv.hpp"
#include <QObject>

class QtToOpencv
{
public:
    QtToOpencv();
    QImage ResizeRgb(const QImage &image, double value);
    cv::Mat QImageToCvMat(const QImage &image, bool inCloneImageData = true);
    QImage CvMatToQImage(const cv::Mat &mat);
};

#endif // QTTOOPENCV_H
