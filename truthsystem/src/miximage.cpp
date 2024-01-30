#include "miximage.h"
#include <QDebug>
#include <QCoreApplication>

MixImage::MixImage()
{
    m_isInit = false;
}

bool MixImage::readMapInfo(std::string mapInfo_path, CropParam &m_crop_param)
{
    int irimg_width = 640;
    int irimg_height = 512;
    int visimg_width = 1920;
    int visimg_height = 1280;

    std::ifstream ifread(mapInfo_path);
    if (!ifread.is_open()) {
        return false;
    }
    std::string info_str;
    std::vector<float> info_list;
    while (ifread >> info_str) {
        info_list.push_back(atof(info_str.c_str()));
    }

    if (info_list.size() == (2 * irimg_width*irimg_height + 2 * visimg_width*visimg_height + 10)) {
        mapx_ir = cv::Mat::zeros(cv::Size(irimg_width, irimg_height), CV_32FC1);
        mapy_ir = mapx_ir.clone();

        mapx_vis = cv::Mat::zeros(cv::Size(visimg_width, visimg_height), CV_32FC1);
        mapy_vis = mapx_vis.clone();

        for (int ii = 0; ii < mapx_ir.rows; ii++)
        {
            for (int jj = 0; jj < mapx_ir.cols; jj++)
            {
                mapx_ir.at<float>(ii, jj) = info_list[ii*mapx_ir.cols + jj];
                mapy_ir.at<float>(ii, jj) = info_list[mapx_ir.cols * mapx_ir.rows + ii*mapy_ir.cols + jj];
            }
        }

        for (int ii = 0; ii < mapx_vis.rows; ii++)
        {
            for (int jj = 0; jj < mapx_vis.cols; jj++)
            {
                mapx_vis.at<float>(ii, jj) = info_list[2 * mapx_ir.cols*mapx_ir.rows + ii*mapx_vis.cols + jj];
                mapy_vis.at<float>(ii, jj) = info_list[2 * mapx_ir.cols*mapx_ir.rows + mapx_vis.cols * mapx_vis.rows + ii*mapx_vis.cols + jj];;
            }
        }

        //crop offset
        m_crop_param.crop_x_ir = info_list[2 * mapx_ir.cols*mapx_ir.rows + 2 * mapx_vis.cols * mapx_vis.rows];
        m_crop_param.crop_y_ir = info_list[2 * mapx_ir.cols*mapx_ir.rows + 2 * mapx_vis.cols * mapx_vis.rows + 1];
        m_crop_param.crop_w_ir = info_list[2 * mapx_ir.cols*mapx_ir.rows + 2 * mapx_vis.cols * mapx_vis.rows + 2];
        m_crop_param.crop_h_ir = info_list[2 * mapx_ir.cols*mapx_ir.rows + 2 * mapx_vis.cols * mapx_vis.rows + 3];

        m_crop_param.crop_x_vis = info_list[2 * mapx_ir.cols*mapx_ir.rows + 2 * mapx_vis.cols * mapx_vis.rows + 4];
        m_crop_param.crop_y_vis = info_list[2 * mapx_ir.cols*mapx_ir.rows + 2 * mapx_vis.cols * mapx_vis.rows + 5];
        m_crop_param.crop_w_vis = info_list[2 * mapx_ir.cols*mapx_ir.rows + 2 * mapx_vis.cols * mapx_vis.rows + 6];
        m_crop_param.crop_h_vis = info_list[2 * mapx_ir.cols*mapx_ir.rows + 2 * mapx_vis.cols * mapx_vis.rows + 7];

        //offset
        m_offsetX = info_list[2 * mapx_ir.cols*mapx_ir.rows + 2 * mapx_vis.cols * mapx_vis.rows + 8];
        m_offsetY = info_list[2 * mapx_ir.cols*mapx_ir.rows + 2 * mapx_vis.cols * mapx_vis.rows + 9];
    } else {
        return false;
    }
    return true;
}

int MixImage::irvis_imgReg(CropParam m_crop_param, cv::Mat ir_img, cv::Mat vis_img, cv::Mat &imgReg_ir, cv::Mat &imgReg_vis)
{
    if (!ir_img.empty() || !vis_img.empty()) {
        cv::Mat rectifyImage_ir, rectifyImage_vis;

        cv::Mat img_x = mapx_ir.clone();
        cv::remap(ir_img, rectifyImage_ir, mapx_ir, mapy_ir, cv::INTER_LINEAR);
        cv::remap(vis_img, rectifyImage_vis, mapx_vis, mapy_vis, cv::INTER_LINEAR);

        if (m_crop_param.crop_x_ir + m_crop_param.crop_w_ir <= rectifyImage_ir.cols && m_crop_param.crop_y_ir + m_crop_param.crop_h_ir <= rectifyImage_ir.rows)
        {
            imgReg_ir = rectifyImage_ir(cv::Rect(m_crop_param.crop_x_ir, m_crop_param.crop_y_ir, m_crop_param.crop_w_ir, m_crop_param.crop_h_ir));
        }
        else
        {
            return -3;//ir crop param error
        }

        if (m_crop_param.crop_x_vis + m_crop_param.crop_w_vis <= rectifyImage_vis.cols && m_crop_param.crop_y_vis + m_crop_param.crop_h_vis <= rectifyImage_vis.rows)
        {
            imgReg_vis = rectifyImage_vis(cv::Rect(m_crop_param.crop_x_vis, m_crop_param.crop_y_vis, m_crop_param.crop_w_vis, m_crop_param.crop_h_vis));
        }
        else
        {
            return -4;//vis crop param error
        }
        return 0;
    }
    else
    {
        return -2;//input image empty
    }
}

bool MixImage::mixImg(cv::Mat irMat, cv::Mat vlMat, cv::Mat& mixImg)
{
    if(!m_isInit) return false;

    cv::Mat irRegMat, vlRetMat;
    int returnValue = irvis_imgReg(m_cropParam,irMat, vlMat, irRegMat, vlRetMat);
    if (returnValue == 0) {
        cv::Mat vlCropMat = vlRetMat(cv::Rect(m_offsetX, m_offsetY, irRegMat.cols, irRegMat.rows));
        vlCropMat = 0.5*irRegMat + 0.5*vlCropMat;
        vlCropMat.copyTo(vlRetMat(cv::Rect(m_offsetX, m_offsetY, irRegMat.cols, irRegMat.rows)));
        mixImg = vlRetMat.clone();
        return true;
    } else {
        return false;
    }

}

bool MixImage::setMapPath(QString path)
{
    if(m_isInit) return true;
    qDebug() << "map info path is " << path;

    if(!readMapInfo(path.toStdString(), m_cropParam)) {
        qDebug() << "read map info fail ";
        return false;
    }
    m_isInit = true;
    return true;
}
