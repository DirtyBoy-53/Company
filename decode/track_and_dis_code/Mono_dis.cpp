#include "Mono_dis.h"

IRCalib* IRCalib::Instance()
{
	static IRCalib config;
	return &config;
}

IRCalib::IRCalib()
{

}

IRCalib::~IRCalib()
{

}

void sortPoint(std::vector<float> dist_temp, std::vector<int>& index)
{
	index.resize(dist_temp.size(), 0);
	for (int ii = 0; ii != index.size(); ii++)
	{
		index[ii] = ii;
	}
	sort(index.begin(), index.end(),
		[&](const int& a, const int& b)
		{
			return (dist_temp[a] < dist_temp[b]);
		}
	);
}

int IRCalib::calc_targetPoint(std::vector<m_irPoint3f> targetPointList, std::vector<m_irPoint>& objPointList, float* m_intrinsic_param, external_param_t* external_param_t)
{
	float m_pitch = external_param_t->m_pitch;
	float m_yaw = -1 * external_param_t->m_yaw;
	float m_roll = external_param_t->m_roll;
	//float m_yaw = external_param_t->m_yaw;
	float m_install_height = external_param_t->m_install_height;
	float m_tan_pitch = tan(m_pitch);
	float m_sin_yaw = sin(m_yaw);
	float m_cos_yaw = cos(m_yaw);
	float m_sin_roll = sin(m_roll);
	float m_cos_roll = cos(m_roll);

	objPointList.clear();

	//根据pitch计算距离阈值, 小于0是是指pitch向下吗
	float distLimt;
	if (m_pitch <= 0)
	{
		distLimt = 999;
	}
	else
	{
		distLimt = m_install_height / tan(m_pitch);
	}
	m_irPoint pp, distPoint;
	for (int i = 0; i < targetPointList.size(); i++)
	{
		targetPointList[i].x = targetPointList[i].x / 1000;
		targetPointList[i].y = targetPointList[i].y / 1000;
		targetPointList[i].z = targetPointList[i].z / 1000;

		// 消除roll和yaw角度的影响
		float x0 = m_cos_roll * targetPointList[i].x - m_sin_roll * targetPointList[i].y;// 这里在修正 roll角顺时针为正，逆时针为负
		float h0 = m_sin_roll * targetPointList[i].x + m_cos_roll * targetPointList[i].y;

		float x1 = m_cos_yaw * x0 - m_sin_yaw * targetPointList[i].z; // yaw角左转为正，右转为负
		float y1 = m_sin_yaw * x0 + m_cos_yaw * targetPointList[i].z;

		if (targetPointList[i].y == 0)
		{
			if (m_pitch <= 0)//代表相机有仰起的角度，此时测距会偏近
			{
				pp.y = tan(atan(m_install_height / targetPointList[i].z) + abs(m_pitch)) * m_intrinsic_param[4] + m_intrinsic_param[5];
			}
			else
			{
				if (targetPointList[i].z < distLimt) // 角度俯下，此时测距会偏远
				{
					pp.y = m_intrinsic_param[5] + tan(atan(m_install_height / targetPointList[i].z) - m_pitch) * m_intrinsic_param[4];
				}
				else
				{
					pp.y = m_intrinsic_param[5] - tan(m_pitch - atan(m_install_height / targetPointList[i].z)) * m_intrinsic_param[4];
				}
			}
			pp.x = m_intrinsic_param[2] + x1 * sqrt((m_intrinsic_param[0] * m_intrinsic_param[0] + (pp.y - m_intrinsic_param[5]) * (pp.y - m_intrinsic_param[5])) / (y1 * y1 + m_install_height * m_install_height));
			distortPoints(pp, distPoint, m_intrinsic_param);
			objPointList.push_back(distPoint);
		}
		else
		{
			if (targetPointList[i].y > m_install_height)
			{
				if (m_pitch <= 0)
				{
					float distLimt2 = m_install_height + tan(abs(m_pitch)) * targetPointList[i].z;
					if (targetPointList[i].y > distLimt2)
					{
						pp.y = m_intrinsic_param[5] - tan(atan((targetPointList[i].y - m_install_height) / targetPointList[i].z) - abs(m_pitch)) * m_intrinsic_param[4];
					}
					else if (targetPointList[i].y < distLimt2)
					{
						pp.y = m_intrinsic_param[5] + tan(abs(m_pitch) - atan((targetPointList[i].y - m_install_height) / targetPointList[i].z)) * m_intrinsic_param[4];
					}
					else
					{
						pp.y = m_intrinsic_param[5];
					}
				}
				else
				{
					pp.y = m_intrinsic_param[5] - tan(m_pitch + atan((targetPointList[i].y - m_install_height) / targetPointList[i].z)) * m_intrinsic_param[4];
				}
				pp.x = m_intrinsic_param[2] + x1 * sqrt((m_intrinsic_param[0] * m_intrinsic_param[0] + (pp.y - m_intrinsic_param[5]) * (pp.y - m_intrinsic_param[5])) / (y1 * y1 + m_install_height * m_install_height));
				distortPoints(pp, distPoint, m_intrinsic_param);
				objPointList.push_back(distPoint);
			}
		}
	}
	return 0;
}

void IRCalib::calcWidthByYaw(int Ly, float Lx, std::vector<m_irPoint>& detectBoxList, float m_intrinsic_param[14], external_param_t external_param)
{
	int car_width = 1878;
	int car_length = 5256;
	int car_height = 1776;
	detectBoxList.clear();
	std::vector<m_irPoint3f> pplist;
	m_irPoint3f m_irpp;
	for (int i = -1; i <= 1; )
	{
		for (int j = 0; j <= 1; j++)
		{
			for (int k = -1; k <= 1;)
			{
				m_irpp.x = i * 1.0 / 2 * car_width;
				m_irpp.y = j * 1.0 * car_height;
				m_irpp.z = k * 1.0 / 2 * car_length + Ly;
				pplist.push_back(m_irpp);
				k = k + 2;
			}
		}
		i = i + 2;
	}

	for (int j = 0; j < pplist.size(); j++)
	{
		pplist[j].x += Lx * 1000;
	}

	std::vector<m_irPoint> res_pplist;
	//3D框投影到成像面上, 
	calc_targetPoint(pplist, res_pplist, m_intrinsic_param, &external_param);

	std::vector<float> x_PointList, y_PointList;
	std::vector<int> x_index;
	for (int i = 0; i < res_pplist.size(); i++)
	{
		x_PointList.push_back(res_pplist[i].x);
		y_PointList.push_back(res_pplist[i].y);
		x_index.push_back(i);
	}
	sortPoint(x_PointList, x_index);
	m_irPoint pp_leftUp, pp_rightDown;
	pp_leftUp.x = res_pplist[x_index[0]].x;
	pp_rightDown.x = res_pplist[x_index[x_index.size() - 1]].x;

	sortPoint(y_PointList, x_index);
	pp_leftUp.y = res_pplist[x_index[0]].y;
	pp_rightDown.y = res_pplist[x_index[x_index.size() - 1]].y;

	detectBoxList.push_back(pp_leftUp);
	detectBoxList.push_back(pp_rightDown);

	x_index.clear();
	std::vector<int>().swap(x_index);// 使用swap来释放内存

	x_PointList.clear();
	std::vector<float>().swap(x_PointList);

	y_PointList.clear();
	std::vector<float>().swap(y_PointList);

	res_pplist.clear();
	std::vector<m_irPoint>().swap(res_pplist);

	pplist.clear();
	std::vector<m_irPoint3f>().swap(pplist);
}

void IRCalib::distortPoints(m_irPoint inputPoint, m_irPoint& distPoint, float* m_intrinsic_param)
{
	double fx_ = m_intrinsic_param[0];
	double fy_ = m_intrinsic_param[4];
	double cx_ = m_intrinsic_param[2];
	double cy_ = m_intrinsic_param[5];
	double k1_ = m_intrinsic_param[9];
	double k2_ = m_intrinsic_param[10];
	double p1_ = m_intrinsic_param[11];
	double p2_ = m_intrinsic_param[12];
	double k3_ = m_intrinsic_param[13];

	double x = (inputPoint.x - cx_) / fx_;
	double y = (inputPoint.y - cy_) / fy_;

	double r2 = x * x + y * y;

	// Radial distorsion
	double xDistort = x * (1 + k1_ * r2 + k2_ * r2 * r2 + k3_ * r2 * r2 * r2);
	double yDistort = y * (1 + k1_ * r2 + k2_ * r2 * r2 + k3_ * r2 * r2 * r2);

	// Tangential distorsion
	xDistort = xDistort + (2 * p1_ * x * y + p2_ * (r2 + 2 * x * x));
	yDistort = yDistort + (p1_ * (r2 + 2 * y * y) + 2 * p2_ * x * y);

	// Back to absolute coordinates.
	xDistort = xDistort * fx_ + cx_;
	yDistort = yDistort * fy_ + cy_;
	distPoint.x = (float)xDistort;
	distPoint.y = (float)yDistort;
}

//去畸变
void IRCalib::solveUndistortedPoints(m_irPoint inputPoint, m_irPoint& undistoredPoint, float* m_intrinsic_param)
{
	double error = 0.0;
	int maxN = 20;
	std::vector<double> err_list, x_list, y_list;

	double x2 = (inputPoint.x - m_intrinsic_param[2]) / m_intrinsic_param[0];
	double y2 = (inputPoint.y - m_intrinsic_param[5]) / m_intrinsic_param[4];
	double r2 = 0;
	double x1 = x2;
	double y1 = y2;
	double x1_temp, y1_temp;
	for (int j = 0;; j++)
	{
		r2 = x1 * x1 + y1 * y1;
		x1_temp = (x2 - (2 * m_intrinsic_param[11] * x1 * y1 + m_intrinsic_param[12] * (r2 + 2 * x1 * x1))) / (1 + m_intrinsic_param[9] * r2 + m_intrinsic_param[10] * r2 * r2 + m_intrinsic_param[13] * r2 * r2 * r2);
		y1_temp = (y2 - (m_intrinsic_param[11] * (r2 + 2 * y1 * y1) + 2 * m_intrinsic_param[12] * x1 * y1)) / (1 + m_intrinsic_param[9] * r2 + m_intrinsic_param[10] * r2 * r2 + m_intrinsic_param[13] * r2 * r2 * r2);
		error = sqrt(pow(x1 - x1_temp, 2) + pow(y1 - y1_temp, 2));

		err_list.push_back(error);
		x_list.push_back(x1);
		y_list.push_back(y1);
		if (error < 0.00001)
		{
			break;
		}
		if (j > maxN)
		{
			double err_min = err_list[0];
			int index = 0;
			for (int i = 0; i < err_list.size(); i++)
			{
				if (err_list[i] < err_min)
				{
					index = i;
					err_min = err_list[i];
				}
			}
			x1 = x_list[index];
			y1 = y_list[index];
			break;
		}

		x1 = x1_temp;
		y1 = y1_temp;
	}
	undistoredPoint.x = m_intrinsic_param[0] * x1 + m_intrinsic_param[2];
	undistoredPoint.y = m_intrinsic_param[4] * y1 + m_intrinsic_param[5];

	err_list.clear();
	std::vector<double>().swap(err_list);
	x_list.clear();
	std::vector<double>().swap(x_list);
	y_list.clear();
	std::vector<double>().swap(y_list);
}

int IRCalib::calc_distance_by_camera_model(float x_left, float y_top, float x_right, float y_bottom, float& y_dist, float m_intrinsic_param[14], external_param_t* external_param)
{
	float m_tan_pitch = tan(external_param->m_pitch);
	float m_sin_yaw = sin(external_param->m_yaw);
	float m_cos_yaw = cos(external_param->m_yaw);
	float h = 0;
	float tempValue = 0;
	if (x_right > x_left && y_bottom > y_top)
	{
		m_irPoint ir_point1, ir_point2;
		ir_point1.x = (x_left + x_right) * 1.0 / 2; ir_point1.y = y_bottom;
		solveUndistortedPoints(ir_point1, ir_point2, m_intrinsic_param);

		float u = ir_point2.x;
		float v = ir_point2.y;


		// pitch等于0时，不会出现 v - m_intrinsic_param[5] < 0的情况 -1 * ((v - m_intrinsic_param[5]) / m_intrinsic_param[4])这个解算方式也让人感觉到迷惑不应该按照上限来处理吗
		if (external_param->m_pitch == 0)
		{
			y_dist = ((v - m_intrinsic_param[5]) > 0) ? (external_param->m_install_height - h) / ((v - m_intrinsic_param[5]) / m_intrinsic_param[4]) : 150;
		}
		else if (external_param->m_pitch > 0)//光轴向下
		{
			tempValue = ((v - m_intrinsic_param[5]) > 0) ? ((v - m_intrinsic_param[5]) / m_intrinsic_param[4]) : -1 * ((v - m_intrinsic_param[5]) / m_intrinsic_param[4]);
			if (v < m_intrinsic_param[5])
			{
				if ((m_tan_pitch - tempValue) >= 0) // 小于等于0更好
				{
					y_dist = (external_param->m_install_height - h) * (1 + m_tan_pitch * tempValue) / (m_tan_pitch - tempValue);
					//(1 + m_tan_pitch * tempValue) / (m_tan_pitch - tempValue) 这是一个三角函数变换 tan(x-y) = (tanx-tany) / (1+tanx*tany)
				}
				else
				{
					y_dist = 150;//无穷远处
				}
			}
			else // 此处表示车辆较近的位置，可以直接使用
			{
				y_dist = (external_param->m_install_height - h) * (1 - m_tan_pitch * tempValue) / (m_tan_pitch + tempValue);
			}
		}
		else//光轴向上
		{
			if (v - m_intrinsic_param[5] < 0) {
				y_dist = 150;
			}
			else {
				y_dist = (external_param->m_install_height - h) / tan(atan(abs(v - m_intrinsic_param[5]) / m_intrinsic_param[4]) - abs(external_param->m_pitch));
			}

		}
	}
	else
	{
		return -1;
	}
	return 0;
}



float car_width_x[29][15] = { 0 };//基于yaw的x查找表

float car_L[] = { 7.37,    9.37,   11.37,   13.37,   15.37,   17.37,   19.37,   21.37,   23.37,   25.37,   27.37,   29.37,   31.37,   33.37,   35.37,   37.37,   39.37,   41.37,   43.37,   45.37,   47.37,   49.37,   51.37,   53.37,   55.37,   57.37,   59.37,   61.37,   63.37 };
float car_realHeight_m[] = { 1420,1520,1620,1720,1820 };
// car_height_m 表示不同的车辆高度下
float car_height_m[][29] = {
	144.03,  113.56,   93.69,   79.74,   69.39,   61.42,   55.09,   49.94,   45.67,   42.08,   39.01,   36.35,   34.04,   32.00,   30.19,   28.57,   27.12,   25.81,   24.62,   23.54,   22.54,   21.63,   20.79,   20.01,   19.29,   18.62,   17.99,   17.40,   16.85,
	154.11,  121.52,  100.28,   85.34,   74.27,   65.74,   58.97,   53.46,   48.89,   45.04,   41.75,   38.91,   36.43,   34.25,   32.31,   30.59,   29.03,   27.63,   26.36,   25.19,   24.13,   23.15,   22.25,   21.42,   20.65,   19.93,   19.25,   18.63,   18.04,
	164.16,  129.47,  106.85,   90.94,   79.15,   70.06,   62.84,   56.97,   52.10,   48.00,   44.50,   41.47,   38.83,   36.50,   34.44,   32.60,   30.94,   29.45,   28.09,   26.85,   25.72,   24.68,   23.72,   22.83,   22.00,   21.24,   20.52,   19.85,   19.23,
	174.19,  137.42,  113.42,   96.54,   84.02,   74.38,   66.71,   60.48,   55.32,   50.96,   47.24,   44.03,   41.22,   38.75,   36.56,   34.61,   32.85,   31.26,   29.82,   28.51,   27.31,   26.20,   25.18,   24.24,   23.36,   22.55,   21.79,   21.08,   20.41,
	184.19,  145.34,  119.98,  102.13,   88.89,   78.69,   70.59,   63.99,   58.53,   53.92,   49.99,   46.59,   43.62,   41.01,   38.69,   36.62,   34.76,   33.08,   31.56,   30.17,   28.89,   27.72,   26.64,   25.65,   24.72,   23.86,   23.05,   22.30,   21.60
};

img_detect_info_m pre_img_detect_info;//存储前1帧所有检测和计算结果信息，在实车运动过程中使用，针对连续2帧的跟踪id，检测框信息进行修正，前置条件，相邻2帧测距结果<2m
m_irPoint static_car_30;
int count_static_car_30 = 0;

void update_car_height_m(float focal_length_y) {
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 30; j++) {
			car_height_m[i][j] = focal_length_y * car_realHeight_m[i] / (1000 * car_L[j]);
		}
	}
}

int  IRCalib::calc_distance(img_detect_info_m& img_detect_info, float m_intrinsic_param[14], external_param_t* external_param, float static_people_rate, float max_width_dist_y)
{
	std::vector<detectbox_info_m> detectbox_info_list;

	float h = external_param->h;//目标高度
	float x_dist = 0, y_dist = 0, x_left_dist = 0, x_right_dist = 0;

	float m_tan_pitch = tan(external_param->m_pitch);
	float m_sin_yaw = sin(external_param->m_yaw);
	float m_cos_yaw = cos(external_param->m_yaw);

	int car_width = 1878;
	int bus_width = 2500;
	int truck_width = 2300;

	int car_height = 1776;
	int bus_height = 3500;
	int truck_height = 3600;

	// 统一类别，拒绝类别跳变
	for (int ii = 0; ii < img_detect_info.detectbox_info.size(); ii++)
	{
		for (int jj = 0; jj < pre_img_detect_info.detectbox_info.size(); jj++) {
			if (img_detect_info.detectbox_info[ii].track_id == pre_img_detect_info.detectbox_info[jj].track_id)
			{
				img_detect_info.detectbox_info[ii].obj_id = pre_img_detect_info.detectbox_info[jj].obj_id;
				break;
			}
		}
	}
	detectbox_info_list = img_detect_info.detectbox_info;

	// 更新高度-测距查找表
	update_car_height_m(m_intrinsic_param[4]);

	if (car_width_x[0][0] == 0)
	{
		//纵向实际距离 y 范围10-66m，距离间隔2m
		//横向实际距离 x 范围 -3.5m——3.5m，距离间隔0.5m 该查找表为由近到远
		//根据不同 x，y组成的点，生成x查找表
		std::vector<m_irPoint> predict_detectBoxList;
		int t = 0;
		for (float x = -3.5; x <= 3.5;)
		{
			for (int L = 0; L < (66 - 10) / 2 + 1; L++)
			{
				// calcWidthByYaw 这个函数会根据给出的点构建一3D辆车，然后将之投影到平面上，返回该车辆在平面上的横纵坐标
				calcWidthByYaw((10 + L * 2) * 1000, x, predict_detectBoxList, m_intrinsic_param, *external_param);
				car_width_x[L][t] = (predict_detectBoxList[1].x + predict_detectBoxList[0].x) / 2;
			}
			x = x + 0.5;
			t = t + 1;
		}
		predict_detectBoxList.clear();
		std::vector<m_irPoint>().swap(predict_detectBoxList);
	}

	//默认0 不进行算法
	//1：静态测试，使用车辆宽度修正pitch，静态行人不使用高度估计纵向距离
	//             等待红绿灯
	//车辆静止驻停（静态测距测试，不使用行人高度进行测距，考虑连续帧关系）
   //2：运动
   //   静止时IMU角度较为精确，根据静止IMU
	std::vector<float> model_dist_y, width_dist_y, height_dist_y, width_car_x_err, hight_dist_people_y;//模型计算结果，车辆宽度计算结果，车辆高度计算结果
	std::vector<int> width_dist_id, hight_dist_people_id, hight_dist_car_id;

	//拍到车身  不能使用宽度估计
	std::vector<int> car_body_id;

	//存在遮挡  
	//car 遮挡
	std::vector<int> car_hide_id;
	//people遮挡
	std::vector<int> people_hide_id;
	float width_height_rate = 0;
	int sameLane_car_trackId = -1;

	if (detectbox_info_list.size() > 0)
	{
		//1. model求解、宽度估计 init 初始化 计算 width/height 判断是否遮挡或拍到车身
		for (int ii = 0; ii < detectbox_info_list.size(); ii++)
		{
			if (detectbox_info_list[ii].obj_id > 8) {
				continue;
			}
			float x_right = detectbox_info_list[ii].detectbox_x_right;
			float x_left = detectbox_info_list[ii].detectbox_x_left;
			float y_bottom = detectbox_info_list[ii].detectbox_y_bottom;
			float y_top = detectbox_info_list[ii].detectbox_y_top;

			float y_dist = 0;
			// 在模型测距法中pitch的正负号非常重要，需要十分精确，模型测距法好像没有使用到yaw角，这是一种默认吗？还是我看漏了什么
			int calc_dist_by_model_returnvalue = calc_distance_by_camera_model(x_left, y_top, x_right, y_bottom, y_dist, m_intrinsic_param, external_param);
			if (calc_dist_by_model_returnvalue == 0)
			{
				model_dist_y.push_back(y_dist);
			}

			//   width/height 宽高比判断遮挡
			width_height_rate = (x_right - x_left) / (y_bottom - y_top);
			detectbox_info_list[ii].detectbox_width_height_rate = width_height_rate;

			if (detectbox_info_list[ii].obj_id == 0)//car
			{
				if (width_height_rate < 1)
				{
					car_hide_id.push_back(ii);
				}
			}
			else if (detectbox_info_list[ii].obj_id == 1)//公交车  宽2500mm，高3040mm   bus车 宽2500mm，高3700mm  
			{
				if (width_height_rate < 0.6)
				{
					car_hide_id.push_back(ii);
				}
			}
			else if (detectbox_info_list[ii].obj_id == 2)//truck
			{
				if (width_height_rate < 0.8)
				{
					car_hide_id.push_back(ii);
				}
			}
			else if (detectbox_info_list[ii].obj_id >= 3 && detectbox_info_list[ii].obj_id <= 6)
			{
				//不做遮挡判断
			}
			else if (detectbox_info_list[ii].obj_id == 7)//people
			{
				if (width_height_rate < 0.2)
				{
					people_hide_id.push_back(ii);
				}
				if (width_height_rate > 1) {
					people_hide_id.push_back(ii);
				}
			}
			else if (detectbox_info_list[ii].obj_id == 8)//cyclist
			{
				//不做遮挡判断
			}

			//width 基于宽度进行测距，这种测距方案仅仅适用于正前方车道的车辆
			float x_detect_center = (x_left + x_right) * 1.0 / 2;
			float x_err = abs(car_width_x[28 / 2 - 1][7] - x_detect_center);
			float y0 = 0; // 是测距，如果是前车，就按照相似三角形进行测距估计
			if (x_err < std::min(abs(car_width_x[28][4] - car_width_x[28 / 2 - 1][7]), abs(car_width_x[28][10] - car_width_x[28 / 2 - 1][7])))
			{
				//为同一车道，相似三角形宽度估计
				if (detectbox_info_list[ii].obj_id < 6)
				{
					if (detectbox_info_list[ii].obj_id == 1 && width_height_rate > 0.6 && width_height_rate < 1.4)//bus
					{
						y0 = m_intrinsic_param[0] * bus_width / (abs(x_left - x_right)) / 1000;
						if (y0 <= max_width_dist_y)
						{
							width_dist_id.push_back(ii);
							width_dist_y.push_back(y0);
							width_car_x_err.push_back(x_err);
						}

					}
					else if (detectbox_info_list[ii].obj_id == 2 && width_height_rate > 0.8 && width_height_rate < 1.4)//truck
					{
						y0 = m_intrinsic_param[0] * truck_width / (abs(x_left - x_right)) / 1000;
						if (y0 <= max_width_dist_y)
						{
							width_dist_id.push_back(ii);
							width_dist_y.push_back(y0);
							width_car_x_err.push_back(x_err);
						}
					}
					else if (detectbox_info_list[ii].obj_id == 0 && width_height_rate > 0.9 && width_height_rate < 1.3)//car
					{
						y0 = m_intrinsic_param[0] * car_width / (abs(x_left - x_right)) / 1000;
						if (y0 <= max_width_dist_y)
						{
							width_dist_id.push_back(ii);
							width_dist_y.push_back(y0);
							width_car_x_err.push_back(x_err);
						}
					}
				}
			}
			else if (x_err > abs(car_width_x[28][4] - car_width_x[28 / 2 - 1][7]) && x_err < abs(car_width_x[28][4] - car_width_x[0][6]))
			{
				if (detectbox_info_list[ii].obj_id < 6)
				{
					if (detectbox_info_list[ii].obj_id == 1 && width_height_rate > 0.6 && width_height_rate < 1.4)//bus
					{
						y0 = m_intrinsic_param[0] * bus_width / (abs(x_left - x_right)) / 1000;
					}
					else if (detectbox_info_list[ii].obj_id == 2 && width_height_rate > 0.8 && width_height_rate < 1.4)//truck
					{
						y0 = m_intrinsic_param[0] * truck_width / (abs(x_left - x_right)) / 1000;
					}
					else if (detectbox_info_list[ii].obj_id == 0 && width_height_rate > 0.9 && width_height_rate < 1.3)//car
					{
						y0 = m_intrinsic_param[0] * car_width / (abs(x_left - x_right)) / 1000;
					}
					if (y0 > 0 && y0 <= max_width_dist_y)
					{
						int t = 0;
						for (; t < 29; t++)
						{
							if (car_L[t] > y0)
							{
								break;
							}
						}
						if (abs(car_width_x[28 / 2 - 1][7] - x_detect_center) < abs(car_width_x[28 / 2 - 1][7] - car_width_x[t][5]))
						{
							width_dist_id.push_back(ii);
							width_dist_y.push_back(y0);
							width_car_x_err.push_back(x_err);
						}
					}
				}
			}

			//基于检测框高度估计车辆纵向距离 这里使用的是高度测距，这个测距应该是相似三角形法得出的，和焦距强相关，这里怎么设置为了定值呢？
			float w_detect_height = y_bottom - y_top;
			if (detectbox_info_list[ii].obj_id > 6 && detectbox_info_list[ii].obj_id < 9)//行人
			{
				//1.75 131.6
				hight_dist_people_y.push_back(m_intrinsic_param[0] * 1.75 / w_detect_height);
				hight_dist_people_id.push_back(ii);
			}
			int index = 0;
			float y3 = 0;
			if (detectbox_info_list[ii].obj_id == 0 || (detectbox_info_list[ii].obj_id >= 3 && detectbox_info_list[ii].obj_id <= 6))//car
			{
				for (; index < 29 - 1; index++)
				{
					if (w_detect_height <= car_height_m[3][index] && w_detect_height >= car_height_m[3][index + 1])
					{
						float k = (car_L[index] - car_L[index + 1]) / (car_height_m[3][index] - car_height_m[3][index + 1]);
						float b = car_L[index] - k * car_height_m[3][index];
						y3 = k * w_detect_height + b;
						break;
					}
				}
				if (w_detect_height < car_height_m[3][28])
				{
					y3 = car_L[28] * car_height_m[3][28] / w_detect_height;
				}
				if (w_detect_height > car_height_m[3][0])
				{
					y3 = car_L[0] / (w_detect_height / car_height_m[3][0]);
				}
				height_dist_y.push_back(y3);
				hight_dist_car_id.push_back(ii);
			}
			else if (detectbox_info_list[ii].obj_id == 1)//公交车  宽2500mm，高3040mm   bus车 宽2500mm，高3700mm  
			{
				y3 = m_intrinsic_param[0] * bus_height / (abs(w_detect_height)) / 1000;
				height_dist_y.push_back(y3);
				hight_dist_car_id.push_back(ii);
			}
			else if (detectbox_info_list[ii].obj_id == 2)//truck
			{
				y3 = m_intrinsic_param[0] * truck_height / (abs(w_detect_height)) / 1000;
				height_dist_y.push_back(y3);
				hight_dist_car_id.push_back(ii);
			}
		}


		//2.进行场景判断（静态（速度=0，P挡），静态红绿灯（速度=0，N挡，其他档位），运动状态），场景判断
		//if (img_detect_info.car_PDCU_ShiftLvlPosn == 5) //P挡  停车，静态测试，车速为0
		//{
		//	if (img_detect_info.car_speed <= 1)  //静态测试，包含行人、车辆静态测试，不能使用行人高度进行距离估计融合
		//	{
		//		float y_dist = 0;
		//		//若存在同车道车辆，进行角度修正
		//		if (width_dist_id.size() >= 1)
		//		{
		//			//若有多辆车，则计算x_err最小时id作为基准
		//			int t_x_err_min = 0;
		//			if (width_dist_id.size() > 1)
		//			{
		//				std::vector<int> index_x_err;
		//				sortPoint(width_car_x_err, index_x_err);
		//				t_x_err_min = index_x_err[0];

		//				index_x_err.clear();
		//				std::vector<int>().swap(index_x_err);
		//				sameLane_car_trackId = detectbox_info_list[width_dist_id[t_x_err_min]].track_id;
		//			}
		//			else if (width_dist_id.size() == 1)
		//			{
		//				sameLane_car_trackId = detectbox_info_list[width_dist_id[t_x_err_min]].track_id;
		//			}

		//			//判断x_err最小的车辆   |模型求解法-宽度估计距离|<0.5m

		//			//任一距离下都需要校正
		//			int calc_dist_by_model_returnvalue = calc_distance_by_camera_model(detectbox_info_list[width_dist_id[t_x_err_min]].detectbox_x_left, detectbox_info_list[width_dist_id[t_x_err_min]].detectbox_y_top, detectbox_info_list[width_dist_id[t_x_err_min]].detectbox_x_right, detectbox_info_list[width_dist_id[t_x_err_min]].detectbox_y_bottom, y_dist, m_intrinsic_param, external_param);
		//			if (calc_dist_by_model_returnvalue == 0)
		//			{
		//				if (abs(y_dist - width_dist_y[t_x_err_min]) > 0.2 && width_dist_y[t_x_err_min] < 50 && width_dist_y[t_x_err_min]>15)
		//				{
		//					/*count_static_car_30++;
		//					static_car_30.x= (static_car_30.x*(count_static_car_30 - 1)+(detectbox_info_list[width_dist_id[t_x_err_min]].detectbox_x_left+ detectbox_info_list[width_dist_id[t_x_err_min]].detectbox_x_right)/2)/count_static_car_30;
		//					static_car_30.y = (static_car_30.y*(count_static_car_30 - 1) + detectbox_info_list[width_dist_id[t_x_err_min]].detectbox_y_bottom ) / count_static_car_30;*/
		//					std::cout << (detectbox_info_list[width_dist_id[t_x_err_min]].detectbox_x_left + detectbox_info_list[width_dist_id[t_x_err_min]].detectbox_x_right) / 2 << "\t";
		//					std::cout << detectbox_info_list[width_dist_id[t_x_err_min]].detectbox_y_bottom << "\t";
		//					std::cout << width_dist_y[t_x_err_min] << "\t" << y_dist << "\t";
		//					//基于宽度更新pitch
		//					float y1 = width_dist_y[t_x_err_min] * m_cos_yaw;
		//					float v = detectbox_info_list[width_dist_id[t_x_err_min]].detectbox_y_bottom;
		//					float u = (detectbox_info_list[width_dist_id[t_x_err_min]].detectbox_x_left + detectbox_info_list[width_dist_id[t_x_err_min]].detectbox_x_right) / 2;
		//					/*float v = static_car_30.y;
		//					float u = static_car_30.x;*/
		//					float tempValue = ((v - m_intrinsic_param[5]) > 0) ? ((v - m_intrinsic_param[5]) / m_intrinsic_param[4]) : -1 * ((v - m_intrinsic_param[5]) / m_intrinsic_param[4]);
		//					if (v > m_intrinsic_param[5])
		//					{
		//						//分为2种，光轴向上和向下
		//						if (atan(external_param->m_install_height / y1) > atan(tempValue))
		//						{
		//							external_param->m_pitch = atan(external_param->m_install_height / y1) - atan(tempValue);
		//						}
		//						else
		//						{
		//							external_param->m_pitch = atan(external_param->m_install_height / y1) - atan(tempValue);
		//						}
		//					}
		//					else
		//					{
		//						//光轴向下，且成像越过cy
		//						external_param->m_pitch = atan(external_param->m_install_height / y1) + atan(tempValue);
		//					}
		//					/*double x0 = (m_intrinsic_param[2] - u)*sqrt((external_param->m_install_height - h)*(external_param->m_install_height - h) + y_dist * y_dist) / (sqrt(m_intrinsic_param[0] * m_intrinsic_param[0] + (v - m_intrinsic_param[5])*(v - m_intrinsic_param[5])));
		//					external_param->m_yaw = atan(-1 * x0 / y_dist);*/
		//					std::cout << external_param->m_pitch * 180 / 3.1415 << "\t" << external_param->m_yaw * 180 / 3.1415 << std::endl;
		//					model_dist_y.clear();
		//					//根据外参角度或修正角度 计算model_dist_y
		//					float y_dist = 0;
		//					for (int t = 0; t < detectbox_info_list.size(); t++)
		//					{
		//						int calc_dist_by_model_returnvalue = calc_distance_by_camera_model(detectbox_info_list[t].detectbox_x_left, detectbox_info_list[t].detectbox_y_top, detectbox_info_list[t].detectbox_x_right, detectbox_info_list[t].detectbox_y_bottom, y_dist, m_intrinsic_param, external_param);
		//						if (calc_dist_by_model_returnvalue == 0)
		//						{
		//							model_dist_y.push_back(y_dist);
		//						}
		//					}
		//				}
		//			}
		//		}

		//		//静态测试，测距融合
		//		for (int t = 0; t < detectbox_info_list.size(); t++)
		//		{
		//			if (detectbox_info_list[t].obj_id >= 7 && detectbox_info_list[t].obj_id <= 8)//人和骑行人
		//			{
		//				if (model_dist_y[t] < 0 || model_dist_y[t]>150)
		//				{
		//					detectbox_info_list[t].y_res = abs(m_intrinsic_param[0] * 1.75 / (detectbox_info_list[t].detectbox_y_bottom - detectbox_info_list[t].detectbox_y_top));
		//				}
		//				else
		//				{
		//					//detectbox_info_list[t].y_res = 0.5*model_dist_y[t]+ 0.5*abs(m_intrinsic_param[0] * 1.75 / (detectbox_info_list[t].detectbox_y_bottom - detectbox_info_list[t].detectbox_y_top));    //静态测试只使用模型求解法
		//					detectbox_info_list[t].y_res = static_people_rate * model_dist_y[t] + (1 - static_people_rate) * abs(m_intrinsic_param[0] * 1.75 / (detectbox_info_list[t].detectbox_y_bottom - detectbox_info_list[t].detectbox_y_top));    //静态测试只使用模型求解法
		//				}
		//			}
		//			else if (detectbox_info_list[t].obj_id >= 0 && detectbox_info_list[t].obj_id <= 6)
		//			{
		//				bool flag = false;
		//				for (int index_width_dist_id = 0; index_width_dist_id < width_dist_id.size(); index_width_dist_id++)
		//				{
		//					if (t == width_dist_id[index_width_dist_id])
		//					{
		//						detectbox_info_list[t].y_res = 0.6 * width_dist_y[index_width_dist_id] + 0.4 * model_dist_y[t];
		//						flag = true;
		//						break;
		//					}
		//				}
		//				if (!flag)
		//				{
		//					detectbox_info_list[t].y_res = 1.0 * model_dist_y[t];
		//				}
		//			}

		//			else if (detectbox_info_list[t].obj_id >= 10)
		//			{
		//				if (model_dist_y[t] < 0 || model_dist_y[t]>150)
		//				{
		//					if (detectbox_info_list[t].obj_id >= 11)
		//					{
		//						detectbox_info_list[t].y_res = abs(m_intrinsic_param[0] * 1.5 / (detectbox_info_list[t].detectbox_y_bottom - detectbox_info_list[t].detectbox_y_top));
		//					}
		//					else
		//					{
		//						detectbox_info_list[t].y_res = abs(m_intrinsic_param[0] * 0.6 / (detectbox_info_list[t].detectbox_y_bottom - detectbox_info_list[t].detectbox_y_top));
		//					}
		//				}
		//				else
		//				{
		//					if (detectbox_info_list[t].obj_id >= 11)
		//					{
		//						//detectbox_info_list[t].y_res = abs(m_intrinsic_param[0] * 1.5 / (detectbox_info_list[t].detectbox_y_bottom - detectbox_info_list[t].detectbox_y_top));
		//						detectbox_info_list[t].y_res = static_people_rate * model_dist_y[t] + (1 - static_people_rate) * abs(m_intrinsic_param[0] * 1.5 / (detectbox_info_list[t].detectbox_y_bottom - detectbox_info_list[t].detectbox_y_top));    //静态测试只使用模型求解法
		//					}
		//					else
		//					{
		//						//detectbox_info_list[t].y_res = abs(m_intrinsic_param[0] * 0.6 / (detectbox_info_list[t].detectbox_y_bottom - detectbox_info_list[t].detectbox_y_top));
		//						detectbox_info_list[t].y_res = static_people_rate * model_dist_y[t] + (1 - static_people_rate) * abs(m_intrinsic_param[0] * 0.6 / (detectbox_info_list[t].detectbox_y_bottom - detectbox_info_list[t].detectbox_y_top));    //静态测试只使用模型求解法
		//					}
		//					//detectbox_info_list[t].y_res = 0.5*model_dist_y[t]+ 0.5*abs(m_intrinsic_param[0] * 1.75 / (detectbox_info_list[t].detectbox_y_bottom - detectbox_info_list[t].detectbox_y_top));    //静态测试只使用模型求解法

		//				}
		//			}
		//		}
		//	}
		//	else
		//	{
		//		return -1;//输入档位信息和车速信息不对
		//	}
		//}
		if (true)// D挡、N挡，R挡  正常行驶
		{
			//同车道车辆判断，若存在同车道车辆，进行pitch角修正，并从新计算所有目标的model_dist
			if (width_dist_id.size() >= 1)
			{
				//若有多辆车，则计算x_err最小时id作为基准
				int t_x_err_min = 0;
				if (width_dist_id.size() > 1)
				{
					std::vector<int> index_x_err;
					sortPoint(width_car_x_err, index_x_err);
					t_x_err_min = index_x_err[0];

					index_x_err.clear();
					std::vector<int>().swap(index_x_err);

					sameLane_car_trackId = detectbox_info_list[width_dist_id[t_x_err_min]].track_id;
				}
				else if (width_dist_id.size() == 1)
				{
					sameLane_car_trackId = detectbox_info_list[width_dist_id[t_x_err_min]].track_id;
				}

				//判断x_err最小的车辆   |模型求解法-宽度估计距离|<0.5m
				float y_dist = 0;
				int calc_dist_by_model_returnvalue = calc_distance_by_camera_model(detectbox_info_list[width_dist_id[t_x_err_min]].detectbox_x_left, detectbox_info_list[width_dist_id[t_x_err_min]].detectbox_y_top, detectbox_info_list[width_dist_id[t_x_err_min]].detectbox_x_right, detectbox_info_list[width_dist_id[t_x_err_min]].detectbox_y_bottom, y_dist, m_intrinsic_param, external_param);
				if (calc_dist_by_model_returnvalue == 0)
				{
					if (abs(y_dist - width_dist_y[t_x_err_min]) > 1)
					{
						//基于宽度更新pitch
						float y1 = width_dist_y[t_x_err_min] * m_cos_yaw; //这一步的处理比较特别，一般来说旋转也不是这样做的
						float v = detectbox_info_list[width_dist_id[t_x_err_min]].detectbox_y_bottom;

						float tempValue = ((v - m_intrinsic_param[5]) > 0) ? ((v - m_intrinsic_param[5]) / m_intrinsic_param[4]) : -1 * ((v - m_intrinsic_param[5]) / m_intrinsic_param[4]);
						if (v > m_intrinsic_param[5])
						{
							//分为2种，光轴向上和向下
							if (atan(external_param->m_install_height / y1) > atan(tempValue))
							{
								external_param->m_pitch = atan(external_param->m_install_height / y1) - atan(tempValue);
							}
							else
							{
								external_param->m_pitch = atan(external_param->m_install_height / y1) - atan(tempValue);
							}
						}
						else
						{
							//光轴向下，且成像越过cy
							external_param->m_pitch = atan(external_param->m_install_height / y1) + atan(tempValue);
						}

						model_dist_y.clear();
						//根据外参角度或修正角度 计算model_dist_y
						float y_dist = 0;
						for (int t = 0; t < detectbox_info_list.size(); t++)
						{
							if (detectbox_info_list[t].obj_id > 8) {
								model_dist_y.push_back(0);
								continue;
							}
							int calc_dist_by_model_returnvalue = calc_distance_by_camera_model(detectbox_info_list[t].detectbox_x_left, detectbox_info_list[t].detectbox_y_top, detectbox_info_list[t].detectbox_x_right, detectbox_info_list[t].detectbox_y_bottom, y_dist, m_intrinsic_param, external_param);
							if (calc_dist_by_model_returnvalue == 0)
							{
								if (y_dist < 0 || y_dist>100)
								{
									y_dist = 100;
								}
								model_dist_y.push_back(y_dist);
							}
						}
					}
				}
			}

			//if (img_detect_info.car_speed <= 1)//速度=0，车辆暂停等待，例如等红绿灯，可获取IMU角度，是否使用红绿灯待定
			//{
			//	//由于IMU在静止情况下累计误差为0，在
			//	//存在同车道车辆
			//	if (width_dist_id.size() >= 1)
			//	{
			//		//model_dist 可靠，测距融合，静态测距只使用模型求解法
			//		for (int t = 0; t < detectbox_info_list.size(); t++)
			//		{
			//			if (detectbox_info_list[t].obj_id >= 7 && detectbox_info_list[t].obj_id <= 8)//人和骑行人
			//			{
			//				if (model_dist_y[t] < 0 || model_dist_y[t]>150)
			//				{
			//					detectbox_info_list[t].y_res = abs(m_intrinsic_param[0] * 1.75 / (detectbox_info_list[t].detectbox_y_bottom - detectbox_info_list[t].detectbox_y_top));
			//				}
			//				else
			//				{
			//					detectbox_info_list[t].y_res = 1.0 * model_dist_y[t];    //静态测试只使用模型求解法
			//				}
			//			}
			//			else if (detectbox_info_list[t].obj_id >= 0 && detectbox_info_list[t].obj_id <= 6)
			//			{
			//				bool flag = false;
			//				for (int index_width_dist_id = 0; index_width_dist_id < width_dist_id.size(); index_width_dist_id++)
			//				{
			//					if (t == width_dist_id[index_width_dist_id])
			//					{
			//						detectbox_info_list[t].y_res = 0.6 * width_dist_y[index_width_dist_id] + 0.4 * model_dist_y[t];
			//						flag = true;
			//						break;
			//					}
			//				}
			//				if (!flag)
			//				{
			//					detectbox_info_list[t].y_res = 1.0 * model_dist_y[t];
			//				}
			//			}
			//			detectbox_info_list[t].model_dist_y = model_dist_y[t];
			//		}
			//	}
			//	else
			//	{
			//		model_dist_y.clear();
			//		//根据外参角度或修正角度 计算model_dist_y
			//		float y_dist = 0;
			//		for (int t = 0; t < detectbox_info_list.size(); t++)
			//		{
			//			int calc_dist_by_model_returnvalue = calc_distance_by_camera_model(detectbox_info_list[t].detectbox_x_left, detectbox_info_list[t].detectbox_y_top, detectbox_info_list[t].detectbox_x_right, detectbox_info_list[t].detectbox_y_bottom, y_dist, m_intrinsic_param, external_param);
			//			if (calc_dist_by_model_returnvalue == 0)
			//			{
			//				if (y_dist < 0 || y_dist>150)
			//				{
			//					y_dist = 150;
			//				}
			//				model_dist_y.push_back(y_dist);
			//			}
			//		}

			//		//无同车道车辆，imu_pitch 是否可以使用？IMU角度测试，车辆静态时IMU角度跳动较大，初步判断不能使用
			//		for (int ii1 = 0; ii1 < detectbox_info_list.size(); ii1++)
			//		{
			//			if (model_dist_y[ii1] > 0 && model_dist_y[ii1] <= 200)
			//			{

			//				if (detectbox_info_list[ii1].obj_id > -1 && detectbox_info_list[ii1].obj_id <= 6)
			//				{
			//					for (int ii2 = 0; ii2 < hight_dist_car_id.size(); ii2++)
			//					{
			//						if (ii1 == hight_dist_car_id[ii2])
			//						{
			//							detectbox_info_list[ii1].y_res = 0.4 * model_dist_y[ii1] + 0.6 * height_dist_y[ii2];
			//						}
			//					}
			//				}
			//				else if (detectbox_info_list[ii1].obj_id >= 7 && detectbox_info_list[ii1].obj_id <= 9)
			//				{
			//					//人
			//					for (int ii2 = 0; ii2 < hight_dist_people_id.size(); ii2++)
			//					{
			//						if (ii1 == hight_dist_people_id[ii2])
			//						{
			//							detectbox_info_list[ii1].y_res = 0.1 * model_dist_y[ii1] + 0.9 * (m_intrinsic_param[0] * 1.75 / (detectbox_info_list[ii1].detectbox_y_bottom - detectbox_info_list[ii1].detectbox_y_top));
			//						}
			//					}
			//				}
			//				else if (detectbox_info_list[ii1].obj_id >= 10)
			//				{
			//					if (detectbox_info_list[ii1].obj_id >= 11)
			//					{
			//						detectbox_info_list[ii1].y_res = 0.1 * model_dist_y[ii1] + (1 - 0.1) * abs(m_intrinsic_param[0] * 1.5 / (detectbox_info_list[ii1].detectbox_y_bottom - detectbox_info_list[ii1].detectbox_y_top));    //静态测试只使用模型求解法
			//					}
			//					else
			//					{
			//						detectbox_info_list[ii1].y_res = 0.1 * model_dist_y[ii1] + (1 - 0.1) * abs(m_intrinsic_param[0] * 0.6 / (detectbox_info_list[ii1].detectbox_y_bottom - detectbox_info_list[ii1].detectbox_y_top));    //静态测试只使用模型求解法
			//					}
			//				}
			//			}
			//			else
			//			{
			//				if (detectbox_info_list[ii1].obj_id > -1 && detectbox_info_list[ii1].obj_id <= 6)
			//				{
			//					for (int ii2 = 0; ii2 < hight_dist_car_id.size(); ii2++)
			//					{
			//						if (ii1 == hight_dist_car_id[ii2])
			//						{
			//							detectbox_info_list[ii1].y_res = height_dist_y[ii2];

			//						}
			//					}
			//				}
			//				else if (detectbox_info_list[ii1].obj_id >= 7 && detectbox_info_list[ii1].obj_id <= 9)
			//				{
			//					for (int ii2 = 0; ii2 < hight_dist_people_id.size(); ii2++)
			//					{
			//						if (ii1 == hight_dist_people_id[ii2])
			//						{
			//							detectbox_info_list[ii1].y_res = 1.0 * (m_intrinsic_param[0] * 1.75 / (detectbox_info_list[ii1].detectbox_y_bottom - detectbox_info_list[ii1].detectbox_y_top));
			//						}
			//					}
			//				}
			//				else if (detectbox_info_list[ii1].obj_id >= 10)
			//				{
			//					if (detectbox_info_list[ii1].obj_id >= 11)
			//					{
			//						detectbox_info_list[ii1].y_res = abs(m_intrinsic_param[0] * 1.5 / (detectbox_info_list[ii1].detectbox_y_bottom - detectbox_info_list[ii1].detectbox_y_top));
			//					}
			//					else
			//					{
			//						detectbox_info_list[ii1].y_res = abs(m_intrinsic_param[0] * 0.6 / (detectbox_info_list[ii1].detectbox_y_bottom - detectbox_info_list[ii1].detectbox_y_top));
			//					}
			//				}

			//			}
			//		}
			//	}
			//}
			if (true)  //正常行驶 
				//模型测距法目前很容易将目标测的非常远，这个问题需要研究一下。
			{
				if (width_dist_id.size() >= 1)
				{
					//
					for (int t = 0; t < detectbox_info_list.size(); t++)
					{
						if (detectbox_info_list[t].obj_id >= 7 && detectbox_info_list[t].obj_id <= 8)//人和骑行人
						{
							if (model_dist_y[t] < 0 || model_dist_y[t]>150)
							{
								detectbox_info_list[t].y_res = m_intrinsic_param[0] * 1.75 / (detectbox_info_list[t].detectbox_y_bottom - detectbox_info_list[t].detectbox_y_top);
							}
							else
							{
								detectbox_info_list[t].y_res = 1.0 * model_dist_y[t];    //静态测试只使用模型求解法
							}
						}
						else if (detectbox_info_list[t].obj_id >= 0 && detectbox_info_list[t].obj_id <= 6)
						{
							bool flag = false;
							for (int index_width_dist_id = 0; index_width_dist_id < width_dist_id.size(); index_width_dist_id++)
							{
								if (t == width_dist_id[index_width_dist_id])
								{
									detectbox_info_list[t].y_res = 0.7 * width_dist_y[index_width_dist_id] + 0.3 * model_dist_y[t];
									flag = true;
									break;
								}
							}
							if (!flag)
							{
								if (model_dist_y[t] < 0 || model_dist_y[t]>100)
								{
									for (int ii2 = 0; ii2 < hight_dist_car_id.size(); ii2++)
									{
										if (t == hight_dist_car_id[ii2])
										{
											detectbox_info_list[t].y_res = height_dist_y[ii2];
										}
									}
								}
								else
								{
									//增加判断，model dist不一定准确，对模型求解法的信任度低于
									for (int ii2 = 0; ii2 < hight_dist_car_id.size(); ii2++)
									{
										if (t == hight_dist_car_id[ii2])
										{
											if (abs(model_dist_y[t] - height_dist_y[ii2]) > 10)
											{
												detectbox_info_list[t].y_res = height_dist_y[ii2];
											}
											else
											{
												detectbox_info_list[t].y_res = 1.0 * model_dist_y[t];    //静态测试只使用模型求解法
											}
										}
									}
								}
							}
						}
						detectbox_info_list[t].model_dist_y = model_dist_y[t];
					}
				}
				else
				{
					//无同车道车辆，imu_pitch 是否可以使用？IMU角度测试，车辆静态时IMU角度跳动较大，初步判断不能使用
					for (int ii1 = 0; ii1 < detectbox_info_list.size(); ii1++)
					{
						if (model_dist_y[ii1] > 0 && model_dist_y[ii1] <= 100)
						{

							if (detectbox_info_list[ii1].obj_id > -1 && detectbox_info_list[ii1].obj_id <= 6)
							{
								for (int ii2 = 0; ii2 < hight_dist_car_id.size(); ii2++)
								{
									if (ii1 == hight_dist_car_id[ii2])
									{
										if (abs(model_dist_y[ii1] - height_dist_y[ii2]) > 10) {
											detectbox_info_list[ii1].y_res = 0.1 * model_dist_y[ii1] + 0.9 * height_dist_y[ii2];
										}
										else {
											detectbox_info_list[ii1].y_res = 0.4 * model_dist_y[ii1] + 0.6 * height_dist_y[ii2];
										}

									}
								}
							}
							else if (detectbox_info_list[ii1].obj_id >= 7 && detectbox_info_list[ii1].obj_id <= 9)
							{
								//人
								for (int ii2 = 0; ii2 < hight_dist_people_id.size(); ii2++)
								{
									if (ii1 == hight_dist_people_id[ii2])
									{
										if (abs(model_dist_y[ii1] - (m_intrinsic_param[0] * 1.75 / (detectbox_info_list[ii1].detectbox_y_bottom - detectbox_info_list[ii1].detectbox_y_top))) < 10) {
											detectbox_info_list[ii1].y_res = 0.1 * model_dist_y[ii1] + 0.9 * (m_intrinsic_param[0] * 1.75 / (detectbox_info_list[ii1].detectbox_y_bottom - detectbox_info_list[ii1].detectbox_y_top));
										}
										else {
											detectbox_info_list[ii1].y_res = (m_intrinsic_param[0] * 1.75 / (detectbox_info_list[ii1].detectbox_y_bottom - detectbox_info_list[ii1].detectbox_y_top));
										}
									}
								}
							}
							else if (detectbox_info_list[ii1].obj_id >= 10)
							{
								if (detectbox_info_list[ii1].obj_id >= 11)
								{
									detectbox_info_list[ii1].y_res = 0.1 * model_dist_y[ii1] + (1 - 0.1) * abs(m_intrinsic_param[0] * 1.5 / (detectbox_info_list[ii1].detectbox_y_bottom - detectbox_info_list[ii1].detectbox_y_top));    //静态测试只使用模型求解法
								}
								else
								{
									detectbox_info_list[ii1].y_res = 0.1 * model_dist_y[ii1] + (1 - 0.1) * abs(m_intrinsic_param[0] * 0.6 / (detectbox_info_list[ii1].detectbox_y_bottom - detectbox_info_list[ii1].detectbox_y_top));    //静态测试只使用模型求解法
								}
							}
						}
						else
						{

							if (detectbox_info_list[ii1].obj_id > -1 && detectbox_info_list[ii1].obj_id <= 6)
							{
								for (int ii2 = 0; ii2 < hight_dist_car_id.size(); ii2++)
								{
									if (ii1 == hight_dist_car_id[ii2])
									{
										detectbox_info_list[ii1].y_res = height_dist_y[ii2];

									}
								}
							}
							else if (detectbox_info_list[ii1].obj_id >= 7 && detectbox_info_list[ii1].obj_id <= 9)
							{
								for (int ii2 = 0; ii2 < hight_dist_people_id.size(); ii2++)
								{
									if (ii1 == hight_dist_people_id[ii2])
									{
										detectbox_info_list[ii1].y_res = 1.0 * (m_intrinsic_param[0] * 1.75 / (detectbox_info_list[ii1].detectbox_y_bottom - detectbox_info_list[ii1].detectbox_y_top));
									}
								}
							}
							else if (detectbox_info_list[ii1].obj_id >= 10)
							{
								if (detectbox_info_list[ii1].obj_id >= 11)
								{
									detectbox_info_list[ii1].y_res = abs(m_intrinsic_param[0] * 1.5 / (detectbox_info_list[ii1].detectbox_y_bottom - detectbox_info_list[ii1].detectbox_y_top));
								}
								else
								{
									detectbox_info_list[ii1].y_res = abs(m_intrinsic_param[0] * 0.6 / (detectbox_info_list[ii1].detectbox_y_bottom - detectbox_info_list[ii1].detectbox_y_top));
								}
							}
						}
					}
				}
			}
		}
		
		//for (int t = 0; t < detectbox_info_list.size(); t++) {
		//	detectbox_info_list[t].y_res = detectbox_info_list[t].ai_dist;
		//}

		//
		for (int i = 0; i < model_dist_y.size(); i++)
		{
			detectbox_info_list[i].model_dist_y = model_dist_y[i];
			detectbox_info_list[i].width_dist_y = 0;
			detectbox_info_list[i].hight_dist_people_y = 0;
			detectbox_info_list[i].height_dist_y = 0;
		}
		for (int i = 0; i < width_dist_id.size(); i++)
		{
			detectbox_info_list[width_dist_id[i]].width_dist_y = width_dist_y[i];
		}
		for (int i = 0; i < hight_dist_people_id.size(); i++)
		{
			detectbox_info_list[hight_dist_people_id[i]].hight_dist_people_y = hight_dist_people_y[i];
		}
		for (int i = 0; i < hight_dist_car_id.size(); i++)
		{
			detectbox_info_list[hight_dist_car_id[i]].height_dist_y = height_dist_y[i];
		}

		img_detect_info.detectbox_info = detectbox_info_list;
		img_detect_info.flag_dist_switch = false;
		img_detect_info.samelane_car_trackId = sameLane_car_trackId;
		//3.连续帧判断修正,这里会记录前一帧的测距结果，但也只是记录一帧
		//if (true)
		if (img_detect_info.car_speed > 1 && img_detect_info.car_PDCU_ShiftLvlPosn != 5)
		{
			if (pre_img_detect_info.frame_id < 1)
			{
				pre_img_detect_info = img_detect_info;
			}
			else
			{
				bool flag_pre_width_dist_y = false;
				for (int ii = 0; ii < pre_img_detect_info.detectbox_info.size(); ii++)
				{
					if (pre_img_detect_info.detectbox_info[ii].width_dist_y > 0)
					{
						flag_pre_width_dist_y = true;
						break;
					}
				}

				bool flag_img_width_dist_y = false;
				for (int ii = 0; ii < img_detect_info.detectbox_info.size(); ii++)
				{
					if (img_detect_info.detectbox_info[ii].width_dist_y > 0)
					{
						flag_img_width_dist_y = true;
						break;
					}
				}

				//很特殊的一部分代码，对比前后真的测距效果，并进行平滑处理，但仅仅是单帧的效果一
				for (int ii = 0; ii < img_detect_info.detectbox_info.size(); ii++)
				{
					for (int jj = 0; jj < pre_img_detect_info.detectbox_info.size(); jj++)
					{
						if (img_detect_info.detectbox_info[ii].track_id == pre_img_detect_info.detectbox_info[jj].track_id)
						{
							if ((!flag_pre_width_dist_y && !flag_img_width_dist_y) || (flag_pre_width_dist_y && !flag_img_width_dist_y))
							{
								//设置最大速度差为120km/h  相邻两帧时间为0.06s  则相邻两帧 120/3.6*0.06=2m
								if (pre_img_detect_info.detectbox_info[jj].y_res > max_width_dist_y) {
									continue;
								}
								if (abs(img_detect_info.detectbox_info[ii].y_res - pre_img_detect_info.detectbox_info[jj].y_res) > 2)
								{
									float img_detect_box_height = img_detect_info.detectbox_info[ii].detectbox_y_bottom - img_detect_info.detectbox_info[ii].detectbox_y_top;
									float pre_img_detetct_box_height = pre_img_detect_info.detectbox_info[jj].detectbox_y_bottom - pre_img_detect_info.detectbox_info[jj].detectbox_y_top;
									img_detect_info.detectbox_info[ii].y_res = pre_img_detect_info.detectbox_info[jj].y_res * pre_img_detetct_box_height / img_detect_box_height;

									if (abs(img_detect_info.detectbox_info[ii].y_res - pre_img_detect_info.detectbox_info[jj].y_res) > 2)
									{
										if (img_detect_box_height > pre_img_detetct_box_height)
										{
											img_detect_info.detectbox_info[ii].y_res = pre_img_detect_info.detectbox_info[jj].y_res - 2;
										}
										else
										{
											img_detect_info.detectbox_info[ii].y_res = pre_img_detect_info.detectbox_info[jj].y_res + 2;
										}
									}
								}
							}
							else if (!flag_pre_width_dist_y && flag_img_width_dist_y)
							{
								//测距结果会跳变
								img_detect_info.flag_dist_switch = true;
							}
						}
					}
				}

				pre_img_detect_info = img_detect_info;
			}
		}


		//4.x方向测距
		for (int ii = 0; ii < img_detect_info.detectbox_info.size(); ii++)
		{
			float x_left = img_detect_info.detectbox_info[ii].detectbox_x_left;
			float x_right = img_detect_info.detectbox_info[ii].detectbox_x_right;
			float u = (x_left + x_right) * 1.0 / 2;
			float v = img_detect_info.detectbox_info[ii].detectbox_y_bottom;

			m_irPoint ir_point1, ir_point2;
			ir_point1.x = u; ir_point1.y = v;
			solveUndistortedPoints(ir_point1, ir_point2, m_intrinsic_param);
			u = ir_point2.x;
			v = ir_point2.y;
			x_dist = (m_intrinsic_param[2] - u) * sqrt((external_param->m_install_height - h) * (external_param->m_install_height - h) + img_detect_info.detectbox_info[ii].y_res * img_detect_info.detectbox_info[ii].y_res) / (sqrt(m_intrinsic_param[0] * m_intrinsic_param[0] + (v - m_intrinsic_param[5]) * (v - m_intrinsic_param[5])));
			x_left_dist = (m_intrinsic_param[2] - x_left) * sqrt((external_param->m_install_height - h) * (external_param->m_install_height - h) + img_detect_info.detectbox_info[ii].y_res * img_detect_info.detectbox_info[ii].y_res) / (sqrt(m_intrinsic_param[0] * m_intrinsic_param[0] + (v - m_intrinsic_param[5]) * (v - m_intrinsic_param[5])));
			x_right_dist = (m_intrinsic_param[2] - x_right) * sqrt((external_param->m_install_height - h) * (external_param->m_install_height - h) + img_detect_info.detectbox_info[ii].y_res * img_detect_info.detectbox_info[ii].y_res) / (sqrt(m_intrinsic_param[0] * m_intrinsic_param[0] + (v - m_intrinsic_param[5]) * (v - m_intrinsic_param[5])));

			if (img_detect_info.detectbox_info[ii].obj_id <= 2)
			{
				float vehicle_width_f = 1.878;
				if (img_detect_info.detectbox_info[ii].obj_id == 0)
				{
					vehicle_width_f = 1.878;
				}
				else if (img_detect_info.detectbox_info[ii].obj_id == 1)
				{
					vehicle_width_f = 2.5;
				}
				else if (img_detect_info.detectbox_info[ii].obj_id == 2)
				{
					vehicle_width_f = 2.3;
				}

				if (x_left_dist < 0 && x_right_dist < 0)
				{
					if (x_right < 640 - 10)
					{
						x_left_dist = -(abs(x_right_dist) - vehicle_width_f);
					}
				}
				if (x_left_dist > 0 && x_right_dist > 0)
				{
					if (x_left > 10)
					{
						x_right_dist = x_left_dist - vehicle_width_f;
					}
				}
			}

			img_detect_info.detectbox_info[ii].y_res = -m_sin_yaw * x_dist + m_cos_yaw * detectbox_info_list[ii].y_res;
			img_detect_info.detectbox_info[ii].x_res = m_cos_yaw * x_dist + m_sin_yaw * detectbox_info_list[ii].y_res;

			img_detect_info.detectbox_info[ii].x_left_res = m_cos_yaw * x_left_dist + m_sin_yaw * detectbox_info_list[ii].y_res;

			img_detect_info.detectbox_info[ii].x_right_res = m_cos_yaw * x_right_dist + m_sin_yaw * detectbox_info_list[ii].y_res;
		}
	}	
	return 0;
}