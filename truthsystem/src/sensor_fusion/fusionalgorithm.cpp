#include "fusionalgorithm.h"
#include <thread>
#include <string>
#include <QString>
#include <../common.h>
#include <../utils/global.hpp>
void writePCD(pcl::PointCloud<PointXYZIT> data,std::string name)
{
	pcl::PCDWriter writer;
	try{

		writer.write(name,data,true);

	}catch (pcl::PCLException &e){
		Log::Logger->error("pcl::PCLException:{}",e.what());
	}
}
void FusionAlgorithm::writeParam(std::vector<double> value)
{
    if(value.size() >= 6){
        m_v1 = value.at(0);
        m_v2 = value.at(1);
        m_v3 = value.at(2);
        m_v4 = value.at(3);
        m_v5 = value.at(4);
        m_v6 = value.at(5);
    }
        
    std::cout << "value:" << m_v1<< "\n";
    std::cout << "value:" << m_v2<< "\n";
    std::cout << "value:" << m_v3<< "\n";
    std::cout << "value:" << m_v4<< "\n";
    std::cout << "value:" << m_v5<< "\n";
    std::cout << "value:" << m_v6<< "\n";
        
}
void FusionAlgorithm::projectpcd(pcl::PointCloud<PointXYZIT> pcds, cv::Mat img,cv::Mat& retImg) {
    
    std::vector<cv::Point3f> objectPoints;
    for (int i = 0; i < pcds.points.size();) {
        if (pcds.points[i].x <100 && pcds.points[i].x > 5) {
            objectPoints.push_back(cv::Point3f(pcds.points[i].x, pcds.points[i].y, pcds.points[i].z));
        }
        i += 10;
    }
    // cout << "there are " << objectPoints.size() << " points" << endl;

    cv::Mat intrisicMat(3, 3, cv::DataType<float>::type);
    intrisicMat.at<float>(0, 0) = 765.362;
    intrisicMat.at<float>(1, 0) = 0;
    intrisicMat.at<float>(2, 0) = 0;
    intrisicMat.at<float>(0, 1) = 0;
    intrisicMat.at<float>(1, 1) = 765.362;
    intrisicMat.at<float>(2, 1) = 0;
    intrisicMat.at<float>(0, 2) = 306.55;
    intrisicMat.at<float>(1, 2) = 181.0144;
    intrisicMat.at<float>(2, 2) = 1;

    cv::Mat rVec(3, 1, cv::DataType<float>::type);
    rVec.at<float>(0) =  1.14960728;
    rVec.at<float>(1) = -1.13551925;
    rVec.at<float>(2) =  1.25224818;

    cv::Mat tVec(3, 1, cv::DataType<float>::type); 
    tVec.at<float>(0) = -0.19083475;
    tVec.at<float>(1) = -0.08228526;
    tVec.at<float>(2) = 0.23415749;

    cv::Mat distCoeffs(5, 1, cv::DataType<float>::type);   // Distortion vector
    distCoeffs.at<float>(0) = -0.351782;
    distCoeffs.at<float>(1) =  0.06771;
    distCoeffs.at<float>(2) = 0;
    distCoeffs.at<float>(3) = 0;
    distCoeffs.at<float>(4) = 0;

    std::vector<cv::Point2f> projectedPoints;
    cv::projectPoints(objectPoints, rVec, tVec, intrisicMat, distCoeffs, projectedPoints);

    // int cn1=0, cn2=0, cn3=0, cn4=0;
    for (int i = 0; i < projectedPoints.size(); i++) {
        if (projectedPoints[i].x < 0 || projectedPoints[i].x >= 639 || projectedPoints[i].y < 0 || projectedPoints[i].y >= 511) {
            continue;
        }

        int depth = int(objectPoints[i].x / 100.0 * 255.0);
        // cv::Scalar color(0, 1022-4*depth, 255);
        // cv::circle(img, projectedPoints[i], 2, color, -1);
        if (depth < 63) {
            cv::Scalar color(255, 254 - 4* depth, 0);
            cv::circle(img, projectedPoints[i], 1, color, -1);
        }
        else if (depth < 127){
            cv::Scalar color(510 - 4*depth, 4*depth - 254, 0);
            cv::circle(img, projectedPoints[i], 1, color, -1);
        }
        else if (depth < 191) {
            cv::Scalar color(0, 255, 4*depth-510);
            cv::circle(img, projectedPoints[i], 1, color, -1);
        }
        else if(depth <= 255)
        {
            cv::Scalar color(0, 1022-4*depth, 255);
            cv::circle(img, projectedPoints[i], 1, color, -1);
        }
    }
    retImg = img.clone();
}

void FusionAlgorithm::radar2camera(std::vector<StructRadarInfo> radarData, cv::Mat& cameraData,cv::Mat& retImg) {
	cv::Mat m = (cv::Mat_<double>(3, 3) << 219.2131, 101.8629, 269.8319,
                                        -8.0768, 55.9542, 472.5958,
                                        -0.0302, 0.2890 ,1);
	for (int i = 0; i < radarData.size(); i++) {
		cv::Mat xy = (cv::Mat_<double>(3, 1) << -radarData[i]._y, radarData[i]._x, 1);
		cv::Mat uv = cv::Mat_<double>(3, 1);
		uv = m * xy;
		auto u = uv.at<double>(0, 0) / uv.at<double>(2, 0);
		auto v = uv.at<double>(1, 0) / uv.at<double>(2, 0);
		// cv::line(cameraData, cv::Point(int(u), int(v)), cv::Point(int(u), int(v + 50)), cv::Scalar(0, 0, 255), 2, cv::LINE_8);
        cv::circle(cameraData,cv::Point(int(u), int(v)),4,cv::Scalar(0, 0, 255));
	}
    retImg = cameraData.clone();
    // static uint32_t count = 0;
    // std::string name = QString("./out/%1.jpg").arg(count).toStdString();
    // cv::imwrite(name, cameraData);
    // count++;
}
