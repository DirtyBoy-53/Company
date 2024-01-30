#include "cremap.h"
#include "qttoopencv.h"
#include <QImage>
#include <QFileInfo>

CRemap::CRemap()
{

}

void CRemap::init(QString path)
{
    QFileInfo file(path);
    if(file.exists()) {
        cv::FileStorage fsLoaded(path.toStdString(), cv::FileStorage::READ);
        fsLoaded["mapx"] >> m_mapXLoaded;
        fsLoaded["mapy"] >> m_mapYLoaded;
        fsLoaded.release();
        m_flag = true;
    }
}

QImage CRemap::remap(QImage srcImg)
{
    if(m_flag) {
        QtToOpencv qConvert;
        cv::Mat srcMat, dstMat;
        srcMat = qConvert.QImageToCvMat(srcImg);
        cv::remap(srcMat, dstMat, m_mapXLoaded, m_mapYLoaded, cv::INTER_LINEAR);
        return qConvert.CvMatToQImage(dstMat);
    } else {
        return srcImg;
    }
}
