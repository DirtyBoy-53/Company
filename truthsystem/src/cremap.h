#ifndef CREMAP_H
#define CREMAP_H

#include <QObject>
#include <opencv2/opencv.hpp>

class CRemap
{
public:
    CRemap();
    void init(QString path);
    QImage remap(QImage srcImg);

private:
    bool m_flag = false;
    cv::Mat m_mapXLoaded;
    cv::Mat m_mapYLoaded;
};

#endif // CREMAP_H
