#pragma once
#include "sensordata.hpp"
#include "../singleton.h"
#include <vector>
class FusionAlgorithm : public Singleton<FusionAlgorithm>
{
public:
    FusionAlgorithm()=default;
    void writeParam(std::vector<double> value);
    void projectpcd(pcl::PointCloud<PointXYZIT> pcds, cv::Mat img,cv::Mat& retImg);
    void radar2camera(std::vector<StructRadarInfo> radarData, cv::Mat& cameraData,cv::Mat& retImg);

private:
    double m_v1{1.18460728};
    double m_v2{-1.13551925};
    double m_v3{1.25224818};
    
    double m_v4{0.48083475};
    double m_v5{0.10228526};
    double m_v6{0.23415749};
};


