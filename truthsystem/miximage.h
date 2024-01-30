#ifndef MIXIMAGE_H
#define MIXIMAGE_H

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <QObject>

//crop param
struct CropParam {
    int crop_x_ir;
    int crop_y_ir;
    int crop_w_ir;
    int crop_h_ir;

    int crop_x_vis;
    int crop_y_vis;
    int crop_w_vis;
    int crop_h_vis;
};

class MixImage
{
public:
    MixImage();
    bool mixImg(cv::Mat irMat, cv::Mat vlMat, cv::Mat& mixImg);
    bool setMapPath(QString path);

private:
    bool readMapInfo(std::string mapInfo_path, CropParam& m_crop_param);
    int irvis_imgReg(CropParam m_crop_param, cv::Mat ir_img, cv::Mat vis_img, cv::Mat& imgReg_ir, cv::Mat& imgReg_vis);

    int m_offsetX, m_offsetY;//offset
    cv::Mat mapx_ir, mapy_ir, mapx_vis, mapy_vis;//map info
    QString m_path;
    bool m_isInit = false;
    CropParam m_cropParam;
};

#endif // MIXIMAGE_H
