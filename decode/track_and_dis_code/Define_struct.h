#pragma once
#ifndef NVS_RESULT_H
#define NVS_RESULT_H
//#include "bpu_predict_extension.h"
#ifdef __cplusplus
extern "C"
{
#endif
#define MAX_OBJECT_NUMBER 30

	typedef struct ExternalParam
	{
		float m_pitch;
		float m_yaw;
		float m_roll;
		float m_install_height;
		float h;
	}external_param_cal;

	//typedef struct IRPoint
	//{
	//	float x;
	//	float y;
	//}m_irPoint;

	typedef struct
	{
		float  person_min;
		float  person_max;
		float  vehicle_min;
		float  vehicle_max;
	}scope_person_vehicle;

	//�������ٶ�
	typedef struct
	{
		float wx; /* �����: ����m/s^2 */
		float wy; /* ������: ����m/s^2 */
		float wz; /* ��ڽ�: ����m/s^2 */
	} an_spd_t;

	//�������ٶ�
	typedef struct
	{
		float ax; /* Ax ������ٶ��ź�: ����m/s^2 */
		float ay; /* Ay ������ٶ��ź�: ����m/s^2 */
		float az; /* Az ������ٶ��ź�: ����m/s^2 */
	} ac_spd_t;

	//������ʱ����
	typedef struct
	{
		unsigned char hour;	  //ʱ
		unsigned char minute; // ��
		unsigned char second; //��
	} Data_and_time_t;

	//����ʵʱ״̬��Ϣ
	typedef struct
	{
		unsigned char BCM_HighBeamStatus;			/* Զ���״̬: ��:0x01 ��:0x00 */
		unsigned char BCM_FrontFoglightStatus;		/* ǰ���: ��:0x01 ��:0x00 */
		float AC_AmbTemp;							/* �����¶�: �������϶� */
		signed char ESC_StaticSlope;				/* �¶���Ϣ: �������� */
		unsigned char ESC_StaticSlopeValid;   /* �¶��ź�״̬: 0x0:INVALID 0x1:VALID */
		unsigned char EMS_BrakePedalSignal;					/* �ƶ�״̬: 0:OFF 1:ON 2:���� 3:��Ч */
		unsigned char Ignition_switch_signal;		/* ��𿪹��ź�: 0x01/0x02/0x03:�� 0x00:�� */
		float ESC_VehSpd;							/* �����ź�: km/h */
		unsigned char ESC_VehSpdValid;				/* �����ź�״̬: 0x0:INVALID 0x1:VALID */
		unsigned char EPS_VehicleDrivingDirection;	/* ������ʻ����: 0:Undefined��standstill�� 1:Front toward 2:Rear toward 3:Invalid */
		float EPS_SAS_SteeringAngle;				/* ת����ź�: �������� */
		float EPS_SAS_SteeringAngleSpd;				/* ת����ٶ��ź�: ������/s */
		unsigned char Air_suspension_travel_signal;  /* ���������г��ź�: mm */
		unsigned char Target_type_mode;				/* Ŀ������ģʽ: 0x00 ��Ŀ�����ͣ�0x01 Ŀ�������л������ˣ���0x02 Ŀ�������л����������� 0x03 Ŀ�������л�������+������*/
		unsigned char NVC_Alarm_Enable;				/* ҹ�ӱ������ÿ���: 0:�ر� 1:������Ĭ�ϣ� */
		unsigned char time_valid;					/* ʱ����Чλ: 0:Valid 1:Not Valid */
		unsigned char WashCarMode;					/* 0x01 ����ϴ��ģʽ 0x00 �˳�ϴ��ģʽ */
		unsigned char DTC;							/* ������Ϣ: ��Ӧbitλ0Ϊ��,1Ϊ�� bit0:VIUL��ʧ0xA2 bit1:IVI��ʧ0x3F7 bit2:�� bit3:SCC��ʧ0x309 */
		unsigned char BCM_AutoLowBeam;			/* �����״̬: ��:0x01 ��:0x00 */
		unsigned char BCM_FrontWiperWorkStatus;		/* ǰ��ι���״̬: ��:0x01 ��:0x00 */
		unsigned char NVC_SwitchStatus;				/* ҹ��ϵͳ���ذ���״̬: ��:0x01 ��:0x00 */
		unsigned char ExtrinsicCalibReq;			/* ��α궨����: "0�������� 1���궨����" */
		unsigned char PDCU_ShiftLvlPosn;     /*����ʵ�ʵ�λ 0:N 1:D 5:P 7:R*/

		ac_spd_t accelerated_speed;	   //���ٶ�
		an_spd_t angular_speed;		   //�Ƕ�
		Data_and_time_t Data_and_time; //���ں�ʱ��

	} vehicle_state_info_t;


	typedef struct
	{
		float m_pitch;		  //������
		float m_yaw;		  //ƫ����
		float install_height; //��װ�߶�
	} camera_external_t;

	typedef struct
	{
		float x1;
		float y1;
		float x2;
		float y2;
	} coordinates_t;

	//�����������
	typedef struct
	{
		float x;
		float y;

		float x_left_res;
		float x_right_res;

		float model_dist_y;
		float width_dist_y;
		float height_dist_y;
		float hight_dist_people_y;

	} position_t;

	//���������ٶ�
	typedef struct
	{
		float v1;
		float v2;
	} velocity_t;




	typedef struct
	{
		bool is_alarm;		   //�Ƿ񱨾�
		unsigned int class_id; //Ŀ������
		unsigned int track_id; //Ŀ��ĸ���id
		// float confidence;  //Ŀ���ĵķ�
		// int tracking_time;  //ͬһĿ�걻���֡��
		float ttc;				   //��������ٶ�
		position_t position;	   //Ŀ��ľ�����Ϣ
		velocity_t velocity;	   //Ŀ����ٶ���Ϣ
		coordinates_t coordinates; //Ŀ������ϽǺ����½�����
	} BBox_t;

	typedef struct
	{
		int obj_number;					   //Ŀ�����
		char is_alarm_frame;			   //��֡���汨�� 0XFF������״�� 0X00��ʾ����������ͼ�꣬������3s�� 0X01��ʾ�˱���ͼ�꣬������3s�� 0X02��ʾ���ﱨ��ͼ�꣬������3s
		unsigned char car_fire_state;	   //�������״̬
		float car_speed;				   //�����ٶ�
		long long obj_frame_id;			   //��ǰ֡id
		an_spd_t veh_angle;				   //����ƫ��
		camera_external_t camera_external; //��ͷ�����Ϣ
		BBox_t bbox[MAX_OBJECT_NUMBER];
		char* error_code[9];
		float EPS_SAS_SteeringAngle;
		unsigned char PDCU_ShiftLvlPosn;
	} BoxMsg_t;


#ifdef __cplusplus
}
#endif
#endif