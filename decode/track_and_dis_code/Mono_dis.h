#pragma once
#include <iostream>
#include <algorithm>
#include <vector>

typedef struct ExternalParam2
{
	float m_pitch;
	float m_yaw;
	float m_roll;
	float m_install_height;
	float h;
}external_param_t;

typedef struct IRPoint
{
	float x;
	float y;
}m_irPoint;

typedef struct IRPoint3f
{
	float x;
	float y;
	float z;
}m_irPoint3f;

typedef struct
{
	int obj_id;//Ŀ������
	int  track_id;//Ŀ�����id

	//����
	float  detectbox_x_left;
	float  detectbox_y_top;
	float  detectbox_x_right;
	float  detectbox_y_bottom;

	float detectbox_width_height_rate;
	//�����
	float y_res;
	float x_res;
	float x_left_res;
	float x_right_res;

	//����м���
	float model_dist_y;
	float width_dist_y;
	float height_dist_y;
	float hight_dist_people_y;

	float ai_dist;
}detectbox_info_m;

typedef struct
{
	long long frame_id;//ͼ��ʱ���
	int car_speed;//����
	int car_PDCU_ShiftLvlPosn;//����ʵ�ʵ�λλ��

	float imu_pitch;//IMU�Ƕ��ź�
	float imu_yaw;
	float imu_roll;

	bool flag_dist_switch;

	int samelane_car_trackId;//-1,

	std::vector<detectbox_info_m> detectbox_info;//һ��ͼ�����м�����Ϣ

}img_detect_info_m;

class IRCalib
{
public:
	static IRCalib* Instance();

	int calc_distance(img_detect_info_m& img_detect_info, float m_intrinsic_param[14], external_param_t* external_param, float static_people_rate, float max_width_dist_y);

	~IRCalib();
private:
	int  calc_distance_by_camera_model(float x_left, float y_top, float x_right, float y_bottom, float& y_dist, float m_intrinsic_param[14], external_param_t* external_param_t);

	//������ұ�
	int calc_targetPoint(std::vector<m_irPoint3f> targetPointList, std::vector<m_irPoint>& objPointList, float* m_intrinsic_param, external_param_t* external_param_t);
	void calcWidthByYaw(int Ly, float Lx, std::vector<m_irPoint>& detectBoxList, float m_intrinsic_param[14], external_param_t external_param_t);

	void distortPoints(m_irPoint inputPoint, m_irPoint& distPoint, float* m_intrinsic_param);
	void solveUndistortedPoints(m_irPoint inputPoint, m_irPoint& undistoredPoint, float* m_intrinsic_param);

	IRCalib();
	IRCalib(const IRCalib&);
	IRCalib& operator=(const IRCalib&);
};