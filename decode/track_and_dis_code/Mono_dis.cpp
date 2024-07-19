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

	//����pitch���������ֵ, С��0����ָpitch������
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

		// ����roll��yaw�Ƕȵ�Ӱ��
		float x0 = m_cos_roll * targetPointList[i].x - m_sin_roll * targetPointList[i].y;// ���������� roll��˳ʱ��Ϊ������ʱ��Ϊ��
		float h0 = m_sin_roll * targetPointList[i].x + m_cos_roll * targetPointList[i].y;

		float x1 = m_cos_yaw * x0 - m_sin_yaw * targetPointList[i].z; // yaw����תΪ������תΪ��
		float y1 = m_sin_yaw * x0 + m_cos_yaw * targetPointList[i].z;

		if (targetPointList[i].y == 0)
		{
			if (m_pitch <= 0)//�������������ĽǶȣ���ʱ����ƫ��
			{
				pp.y = tan(atan(m_install_height / targetPointList[i].z) + abs(m_pitch)) * m_intrinsic_param[4] + m_intrinsic_param[5];
			}
			else
			{
				if (targetPointList[i].z < distLimt) // �Ƕȸ��£���ʱ����ƫԶ
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
	//3D��ͶӰ����������, 
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
	std::vector<int>().swap(x_index);// ʹ��swap���ͷ��ڴ�

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

//ȥ����
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


		// pitch����0ʱ��������� v - m_intrinsic_param[5] < 0����� -1 * ((v - m_intrinsic_param[5]) / m_intrinsic_param[4])������㷽ʽҲ���˸о����Ի�Ӧ�ð���������������
		if (external_param->m_pitch == 0)
		{
			y_dist = ((v - m_intrinsic_param[5]) > 0) ? (external_param->m_install_height - h) / ((v - m_intrinsic_param[5]) / m_intrinsic_param[4]) : 150;
		}
		else if (external_param->m_pitch > 0)//��������
		{
			tempValue = ((v - m_intrinsic_param[5]) > 0) ? ((v - m_intrinsic_param[5]) / m_intrinsic_param[4]) : -1 * ((v - m_intrinsic_param[5]) / m_intrinsic_param[4]);
			if (v < m_intrinsic_param[5])
			{
				if ((m_tan_pitch - tempValue) >= 0) // С�ڵ���0����
				{
					y_dist = (external_param->m_install_height - h) * (1 + m_tan_pitch * tempValue) / (m_tan_pitch - tempValue);
					//(1 + m_tan_pitch * tempValue) / (m_tan_pitch - tempValue) ����һ�����Ǻ����任 tan(x-y) = (tanx-tany) / (1+tanx*tany)
				}
				else
				{
					y_dist = 150;//����Զ��
				}
			}
			else // �˴���ʾ�����Ͻ���λ�ã�����ֱ��ʹ��
			{
				y_dist = (external_param->m_install_height - h) * (1 - m_tan_pitch * tempValue) / (m_tan_pitch + tempValue);
			}
		}
		else//��������
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



float car_width_x[29][15] = { 0 };//����yaw��x���ұ�

float car_L[] = { 7.37,    9.37,   11.37,   13.37,   15.37,   17.37,   19.37,   21.37,   23.37,   25.37,   27.37,   29.37,   31.37,   33.37,   35.37,   37.37,   39.37,   41.37,   43.37,   45.37,   47.37,   49.37,   51.37,   53.37,   55.37,   57.37,   59.37,   61.37,   63.37 };
float car_realHeight_m[] = { 1420,1520,1620,1720,1820 };
// car_height_m ��ʾ��ͬ�ĳ����߶���
float car_height_m[][29] = {
	144.03,  113.56,   93.69,   79.74,   69.39,   61.42,   55.09,   49.94,   45.67,   42.08,   39.01,   36.35,   34.04,   32.00,   30.19,   28.57,   27.12,   25.81,   24.62,   23.54,   22.54,   21.63,   20.79,   20.01,   19.29,   18.62,   17.99,   17.40,   16.85,
	154.11,  121.52,  100.28,   85.34,   74.27,   65.74,   58.97,   53.46,   48.89,   45.04,   41.75,   38.91,   36.43,   34.25,   32.31,   30.59,   29.03,   27.63,   26.36,   25.19,   24.13,   23.15,   22.25,   21.42,   20.65,   19.93,   19.25,   18.63,   18.04,
	164.16,  129.47,  106.85,   90.94,   79.15,   70.06,   62.84,   56.97,   52.10,   48.00,   44.50,   41.47,   38.83,   36.50,   34.44,   32.60,   30.94,   29.45,   28.09,   26.85,   25.72,   24.68,   23.72,   22.83,   22.00,   21.24,   20.52,   19.85,   19.23,
	174.19,  137.42,  113.42,   96.54,   84.02,   74.38,   66.71,   60.48,   55.32,   50.96,   47.24,   44.03,   41.22,   38.75,   36.56,   34.61,   32.85,   31.26,   29.82,   28.51,   27.31,   26.20,   25.18,   24.24,   23.36,   22.55,   21.79,   21.08,   20.41,
	184.19,  145.34,  119.98,  102.13,   88.89,   78.69,   70.59,   63.99,   58.53,   53.92,   49.99,   46.59,   43.62,   41.01,   38.69,   36.62,   34.76,   33.08,   31.56,   30.17,   28.89,   27.72,   26.64,   25.65,   24.72,   23.86,   23.05,   22.30,   21.60
};

img_detect_info_m pre_img_detect_info;//�洢ǰ1֡���м��ͼ�������Ϣ����ʵ���˶�������ʹ�ã��������2֡�ĸ���id��������Ϣ����������ǰ������������2֡�����<2m
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

	float h = external_param->h;//Ŀ��߶�
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

	// ͳһ��𣬾ܾ��������
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

	// ���¸߶�-�����ұ�
	update_car_height_m(m_intrinsic_param[4]);

	if (car_width_x[0][0] == 0)
	{
		//����ʵ�ʾ��� y ��Χ10-66m��������2m
		//����ʵ�ʾ��� x ��Χ -3.5m����3.5m��������0.5m �ò��ұ�Ϊ�ɽ���Զ
		//���ݲ�ͬ x��y��ɵĵ㣬����x���ұ�
		std::vector<m_irPoint> predict_detectBoxList;
		int t = 0;
		for (float x = -3.5; x <= 3.5;)
		{
			for (int L = 0; L < (66 - 10) / 2 + 1; L++)
			{
				// calcWidthByYaw �����������ݸ����ĵ㹹��һ3D������Ȼ��֮ͶӰ��ƽ���ϣ����ظó�����ƽ���ϵĺ�������
				calcWidthByYaw((10 + L * 2) * 1000, x, predict_detectBoxList, m_intrinsic_param, *external_param);
				car_width_x[L][t] = (predict_detectBoxList[1].x + predict_detectBoxList[0].x) / 2;
			}
			x = x + 0.5;
			t = t + 1;
		}
		predict_detectBoxList.clear();
		std::vector<m_irPoint>().swap(predict_detectBoxList);
	}

	//Ĭ��0 �������㷨
	//1����̬���ԣ�ʹ�ó����������pitch����̬���˲�ʹ�ø߶ȹ����������
	//             �ȴ����̵�
	//������ֹפͣ����̬�����ԣ���ʹ�����˸߶Ƚ��в�࣬��������֡��ϵ��
   //2���˶�
   //   ��ֹʱIMU�ǶȽ�Ϊ��ȷ�����ݾ�ֹIMU
	std::vector<float> model_dist_y, width_dist_y, height_dist_y, width_car_x_err, hight_dist_people_y;//ģ�ͼ�������������ȼ������������߶ȼ�����
	std::vector<int> width_dist_id, hight_dist_people_id, hight_dist_car_id;

	//�ĵ�����  ����ʹ�ÿ�ȹ���
	std::vector<int> car_body_id;

	//�����ڵ�  
	//car �ڵ�
	std::vector<int> car_hide_id;
	//people�ڵ�
	std::vector<int> people_hide_id;
	float width_height_rate = 0;
	int sameLane_car_trackId = -1;

	if (detectbox_info_list.size() > 0)
	{
		//1. model��⡢��ȹ��� init ��ʼ�� ���� width/height �ж��Ƿ��ڵ����ĵ�����
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
			// ��ģ�Ͳ�෨��pitch�������ŷǳ���Ҫ����Ҫʮ�־�ȷ��ģ�Ͳ�෨����û��ʹ�õ�yaw�ǣ�����һ��Ĭ���𣿻����ҿ�©��ʲô
			int calc_dist_by_model_returnvalue = calc_distance_by_camera_model(x_left, y_top, x_right, y_bottom, y_dist, m_intrinsic_param, external_param);
			if (calc_dist_by_model_returnvalue == 0)
			{
				model_dist_y.push_back(y_dist);
			}

			//   width/height ��߱��ж��ڵ�
			width_height_rate = (x_right - x_left) / (y_bottom - y_top);
			detectbox_info_list[ii].detectbox_width_height_rate = width_height_rate;

			if (detectbox_info_list[ii].obj_id == 0)//car
			{
				if (width_height_rate < 1)
				{
					car_hide_id.push_back(ii);
				}
			}
			else if (detectbox_info_list[ii].obj_id == 1)//������  ��2500mm����3040mm   bus�� ��2500mm����3700mm  
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
				//�����ڵ��ж�
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
				//�����ڵ��ж�
			}

			//width ���ڿ�Ƚ��в�࣬���ֲ�෽��������������ǰ�������ĳ���
			float x_detect_center = (x_left + x_right) * 1.0 / 2;
			float x_err = abs(car_width_x[28 / 2 - 1][7] - x_detect_center);
			float y0 = 0; // �ǲ�࣬�����ǰ�����Ͱ������������ν��в�����
			if (x_err < std::min(abs(car_width_x[28][4] - car_width_x[28 / 2 - 1][7]), abs(car_width_x[28][10] - car_width_x[28 / 2 - 1][7])))
			{
				//Ϊͬһ���������������ο�ȹ���
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

			//���ڼ���߶ȹ��Ƴ���������� ����ʹ�õ��Ǹ߶Ȳ�࣬������Ӧ�������������η��ó��ģ��ͽ���ǿ��أ�������ô����Ϊ�˶�ֵ�أ�
			float w_detect_height = y_bottom - y_top;
			if (detectbox_info_list[ii].obj_id > 6 && detectbox_info_list[ii].obj_id < 9)//����
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
			else if (detectbox_info_list[ii].obj_id == 1)//������  ��2500mm����3040mm   bus�� ��2500mm����3700mm  
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


		//2.���г����жϣ���̬���ٶ�=0��P��������̬���̵ƣ��ٶ�=0��N����������λ�����˶�״̬���������ж�
		//if (img_detect_info.car_PDCU_ShiftLvlPosn == 5) //P��  ͣ������̬���ԣ�����Ϊ0
		//{
		//	if (img_detect_info.car_speed <= 1)  //��̬���ԣ��������ˡ�������̬���ԣ�����ʹ�����˸߶Ƚ��о�������ں�
		//	{
		//		float y_dist = 0;
		//		//������ͬ�������������нǶ�����
		//		if (width_dist_id.size() >= 1)
		//		{
		//			//���ж������������x_err��Сʱid��Ϊ��׼
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

		//			//�ж�x_err��С�ĳ���   |ģ����ⷨ-��ȹ��ƾ���|<0.5m

		//			//��һ�����¶���ҪУ��
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
		//					//���ڿ�ȸ���pitch
		//					float y1 = width_dist_y[t_x_err_min] * m_cos_yaw;
		//					float v = detectbox_info_list[width_dist_id[t_x_err_min]].detectbox_y_bottom;
		//					float u = (detectbox_info_list[width_dist_id[t_x_err_min]].detectbox_x_left + detectbox_info_list[width_dist_id[t_x_err_min]].detectbox_x_right) / 2;
		//					/*float v = static_car_30.y;
		//					float u = static_car_30.x;*/
		//					float tempValue = ((v - m_intrinsic_param[5]) > 0) ? ((v - m_intrinsic_param[5]) / m_intrinsic_param[4]) : -1 * ((v - m_intrinsic_param[5]) / m_intrinsic_param[4]);
		//					if (v > m_intrinsic_param[5])
		//					{
		//						//��Ϊ2�֣��������Ϻ�����
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
		//						//�������£��ҳ���Խ��cy
		//						external_param->m_pitch = atan(external_param->m_install_height / y1) + atan(tempValue);
		//					}
		//					/*double x0 = (m_intrinsic_param[2] - u)*sqrt((external_param->m_install_height - h)*(external_param->m_install_height - h) + y_dist * y_dist) / (sqrt(m_intrinsic_param[0] * m_intrinsic_param[0] + (v - m_intrinsic_param[5])*(v - m_intrinsic_param[5])));
		//					external_param->m_yaw = atan(-1 * x0 / y_dist);*/
		//					std::cout << external_param->m_pitch * 180 / 3.1415 << "\t" << external_param->m_yaw * 180 / 3.1415 << std::endl;
		//					model_dist_y.clear();
		//					//������νǶȻ������Ƕ� ����model_dist_y
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

		//		//��̬���ԣ�����ں�
		//		for (int t = 0; t < detectbox_info_list.size(); t++)
		//		{
		//			if (detectbox_info_list[t].obj_id >= 7 && detectbox_info_list[t].obj_id <= 8)//�˺�������
		//			{
		//				if (model_dist_y[t] < 0 || model_dist_y[t]>150)
		//				{
		//					detectbox_info_list[t].y_res = abs(m_intrinsic_param[0] * 1.75 / (detectbox_info_list[t].detectbox_y_bottom - detectbox_info_list[t].detectbox_y_top));
		//				}
		//				else
		//				{
		//					//detectbox_info_list[t].y_res = 0.5*model_dist_y[t]+ 0.5*abs(m_intrinsic_param[0] * 1.75 / (detectbox_info_list[t].detectbox_y_bottom - detectbox_info_list[t].detectbox_y_top));    //��̬����ֻʹ��ģ����ⷨ
		//					detectbox_info_list[t].y_res = static_people_rate * model_dist_y[t] + (1 - static_people_rate) * abs(m_intrinsic_param[0] * 1.75 / (detectbox_info_list[t].detectbox_y_bottom - detectbox_info_list[t].detectbox_y_top));    //��̬����ֻʹ��ģ����ⷨ
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
		//						detectbox_info_list[t].y_res = static_people_rate * model_dist_y[t] + (1 - static_people_rate) * abs(m_intrinsic_param[0] * 1.5 / (detectbox_info_list[t].detectbox_y_bottom - detectbox_info_list[t].detectbox_y_top));    //��̬����ֻʹ��ģ����ⷨ
		//					}
		//					else
		//					{
		//						//detectbox_info_list[t].y_res = abs(m_intrinsic_param[0] * 0.6 / (detectbox_info_list[t].detectbox_y_bottom - detectbox_info_list[t].detectbox_y_top));
		//						detectbox_info_list[t].y_res = static_people_rate * model_dist_y[t] + (1 - static_people_rate) * abs(m_intrinsic_param[0] * 0.6 / (detectbox_info_list[t].detectbox_y_bottom - detectbox_info_list[t].detectbox_y_top));    //��̬����ֻʹ��ģ����ⷨ
		//					}
		//					//detectbox_info_list[t].y_res = 0.5*model_dist_y[t]+ 0.5*abs(m_intrinsic_param[0] * 1.75 / (detectbox_info_list[t].detectbox_y_bottom - detectbox_info_list[t].detectbox_y_top));    //��̬����ֻʹ��ģ����ⷨ

		//				}
		//			}
		//		}
		//	}
		//	else
		//	{
		//		return -1;//���뵵λ��Ϣ�ͳ�����Ϣ����
		//	}
		//}
		if (true)// D����N����R��  ������ʻ
		{
			//ͬ���������жϣ�������ͬ��������������pitch�������������¼�������Ŀ���model_dist
			if (width_dist_id.size() >= 1)
			{
				//���ж������������x_err��Сʱid��Ϊ��׼
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

				//�ж�x_err��С�ĳ���   |ģ����ⷨ-��ȹ��ƾ���|<0.5m
				float y_dist = 0;
				int calc_dist_by_model_returnvalue = calc_distance_by_camera_model(detectbox_info_list[width_dist_id[t_x_err_min]].detectbox_x_left, detectbox_info_list[width_dist_id[t_x_err_min]].detectbox_y_top, detectbox_info_list[width_dist_id[t_x_err_min]].detectbox_x_right, detectbox_info_list[width_dist_id[t_x_err_min]].detectbox_y_bottom, y_dist, m_intrinsic_param, external_param);
				if (calc_dist_by_model_returnvalue == 0)
				{
					if (abs(y_dist - width_dist_y[t_x_err_min]) > 1)
					{
						//���ڿ�ȸ���pitch
						float y1 = width_dist_y[t_x_err_min] * m_cos_yaw; //��һ���Ĵ���Ƚ��ر�һ����˵��תҲ������������
						float v = detectbox_info_list[width_dist_id[t_x_err_min]].detectbox_y_bottom;

						float tempValue = ((v - m_intrinsic_param[5]) > 0) ? ((v - m_intrinsic_param[5]) / m_intrinsic_param[4]) : -1 * ((v - m_intrinsic_param[5]) / m_intrinsic_param[4]);
						if (v > m_intrinsic_param[5])
						{
							//��Ϊ2�֣��������Ϻ�����
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
							//�������£��ҳ���Խ��cy
							external_param->m_pitch = atan(external_param->m_install_height / y1) + atan(tempValue);
						}

						model_dist_y.clear();
						//������νǶȻ������Ƕ� ����model_dist_y
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

			//if (img_detect_info.car_speed <= 1)//�ٶ�=0��������ͣ�ȴ�������Ⱥ��̵ƣ��ɻ�ȡIMU�Ƕȣ��Ƿ�ʹ�ú��̵ƴ���
			//{
			//	//����IMU�ھ�ֹ������ۼ����Ϊ0����
			//	//����ͬ��������
			//	if (width_dist_id.size() >= 1)
			//	{
			//		//model_dist �ɿ�������ںϣ���̬���ֻʹ��ģ����ⷨ
			//		for (int t = 0; t < detectbox_info_list.size(); t++)
			//		{
			//			if (detectbox_info_list[t].obj_id >= 7 && detectbox_info_list[t].obj_id <= 8)//�˺�������
			//			{
			//				if (model_dist_y[t] < 0 || model_dist_y[t]>150)
			//				{
			//					detectbox_info_list[t].y_res = abs(m_intrinsic_param[0] * 1.75 / (detectbox_info_list[t].detectbox_y_bottom - detectbox_info_list[t].detectbox_y_top));
			//				}
			//				else
			//				{
			//					detectbox_info_list[t].y_res = 1.0 * model_dist_y[t];    //��̬����ֻʹ��ģ����ⷨ
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
			//		//������νǶȻ������Ƕ� ����model_dist_y
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

			//		//��ͬ����������imu_pitch �Ƿ����ʹ�ã�IMU�ǶȲ��ԣ�������̬ʱIMU�Ƕ������ϴ󣬳����жϲ���ʹ��
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
			//					//��
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
			//						detectbox_info_list[ii1].y_res = 0.1 * model_dist_y[ii1] + (1 - 0.1) * abs(m_intrinsic_param[0] * 1.5 / (detectbox_info_list[ii1].detectbox_y_bottom - detectbox_info_list[ii1].detectbox_y_top));    //��̬����ֻʹ��ģ����ⷨ
			//					}
			//					else
			//					{
			//						detectbox_info_list[ii1].y_res = 0.1 * model_dist_y[ii1] + (1 - 0.1) * abs(m_intrinsic_param[0] * 0.6 / (detectbox_info_list[ii1].detectbox_y_bottom - detectbox_info_list[ii1].detectbox_y_top));    //��̬����ֻʹ��ģ����ⷨ
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
			if (true)  //������ʻ 
				//ģ�Ͳ�෨Ŀǰ�����׽�Ŀ���ķǳ�Զ�����������Ҫ�о�һ�¡�
			{
				if (width_dist_id.size() >= 1)
				{
					//
					for (int t = 0; t < detectbox_info_list.size(); t++)
					{
						if (detectbox_info_list[t].obj_id >= 7 && detectbox_info_list[t].obj_id <= 8)//�˺�������
						{
							if (model_dist_y[t] < 0 || model_dist_y[t]>150)
							{
								detectbox_info_list[t].y_res = m_intrinsic_param[0] * 1.75 / (detectbox_info_list[t].detectbox_y_bottom - detectbox_info_list[t].detectbox_y_top);
							}
							else
							{
								detectbox_info_list[t].y_res = 1.0 * model_dist_y[t];    //��̬����ֻʹ��ģ����ⷨ
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
									//�����жϣ�model dist��һ��׼ȷ����ģ����ⷨ�����ζȵ���
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
												detectbox_info_list[t].y_res = 1.0 * model_dist_y[t];    //��̬����ֻʹ��ģ����ⷨ
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
					//��ͬ����������imu_pitch �Ƿ����ʹ�ã�IMU�ǶȲ��ԣ�������̬ʱIMU�Ƕ������ϴ󣬳����жϲ���ʹ��
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
								//��
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
									detectbox_info_list[ii1].y_res = 0.1 * model_dist_y[ii1] + (1 - 0.1) * abs(m_intrinsic_param[0] * 1.5 / (detectbox_info_list[ii1].detectbox_y_bottom - detectbox_info_list[ii1].detectbox_y_top));    //��̬����ֻʹ��ģ����ⷨ
								}
								else
								{
									detectbox_info_list[ii1].y_res = 0.1 * model_dist_y[ii1] + (1 - 0.1) * abs(m_intrinsic_param[0] * 0.6 / (detectbox_info_list[ii1].detectbox_y_bottom - detectbox_info_list[ii1].detectbox_y_top));    //��̬����ֻʹ��ģ����ⷨ
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
		//3.����֡�ж�����,������¼ǰһ֡�Ĳ��������Ҳֻ�Ǽ�¼һ֡
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

				//�������һ���ִ��룬�Ա�ǰ����Ĳ��Ч����������ƽ�������������ǵ�֡��Ч��һ
				for (int ii = 0; ii < img_detect_info.detectbox_info.size(); ii++)
				{
					for (int jj = 0; jj < pre_img_detect_info.detectbox_info.size(); jj++)
					{
						if (img_detect_info.detectbox_info[ii].track_id == pre_img_detect_info.detectbox_info[jj].track_id)
						{
							if ((!flag_pre_width_dist_y && !flag_img_width_dist_y) || (flag_pre_width_dist_y && !flag_img_width_dist_y))
							{
								//��������ٶȲ�Ϊ120km/h  ������֡ʱ��Ϊ0.06s  ��������֡ 120/3.6*0.06=2m
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
								//�����������
								img_detect_info.flag_dist_switch = true;
							}
						}
					}
				}

				pre_img_detect_info = img_detect_info;
			}
		}


		//4.x������
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