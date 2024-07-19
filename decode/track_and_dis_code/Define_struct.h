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

	//车辆角速度
	typedef struct
	{
		float wx; /* 侧倾角: 正负m/s^2 */
		float wy; /* 俯仰角: 正负m/s^2 */
		float wz; /* 横摆角: 正负m/s^2 */
	} an_spd_t;

	//车辆加速度
	typedef struct
	{
		float ax; /* Ax 纵向加速度信号: 正负m/s^2 */
		float ay; /* Ay 纵向加速度信号: 正负m/s^2 */
		float az; /* Az 纵向加速度信号: 正负m/s^2 */
	} ac_spd_t;

	//年月日时分妙
	typedef struct
	{
		unsigned char hour;	  //时
		unsigned char minute; // 分
		unsigned char second; //秒
	} Data_and_time_t;

	//车辆实时状态信息
	typedef struct
	{
		unsigned char BCM_HighBeamStatus;			/* 远光灯状态: 开:0x01 关:0x00 */
		unsigned char BCM_FrontFoglightStatus;		/* 前雾灯: 开:0x01 关:0x00 */
		float AC_AmbTemp;							/* 环境温度: 正负摄氏度 */
		signed char ESC_StaticSlope;				/* 坡度信息: 正负度数 */
		unsigned char ESC_StaticSlopeValid;   /* 坡度信号状态: 0x0:INVALID 0x1:VALID */
		unsigned char EMS_BrakePedalSignal;					/* 制动状态: 0:OFF 1:ON 2:错误 3:无效 */
		unsigned char Ignition_switch_signal;		/* 点火开关信号: 0x01/0x02/0x03:开 0x00:关 */
		float ESC_VehSpd;							/* 车速信号: km/h */
		unsigned char ESC_VehSpdValid;				/* 车速信号状态: 0x0:INVALID 0x1:VALID */
		unsigned char EPS_VehicleDrivingDirection;	/* 车辆行驶方向: 0:Undefined（standstill） 1:Front toward 2:Rear toward 3:Invalid */
		float EPS_SAS_SteeringAngle;				/* 转向角信号: 正负度数 */
		float EPS_SAS_SteeringAngleSpd;				/* 转向角速度信号: 正负度/s */
		unsigned char Air_suspension_travel_signal;  /* 空气悬挂行程信号: mm */
		unsigned char Target_type_mode;				/* 目标类型模式: 0x00 无目标类型；0x01 目标类型切换（行人）；0x02 目标类型切换（车辆）； 0x03 目标类型切换（行人+车辆）*/
		unsigned char NVC_Alarm_Enable;				/* 夜视报警设置开关: 0:关闭 1:开启（默认） */
		unsigned char time_valid;					/* 时间有效位: 0:Valid 1:Not Valid */
		unsigned char WashCarMode;					/* 0x01 进入洗车模式 0x00 退出洗车模式 */
		unsigned char DTC;							/* 故障信息: 对应bit位0为有,1为无 bit0:VIUL丢失0xA2 bit1:IVI丢失0x3F7 bit2:空 bit3:SCC丢失0x309 */
		unsigned char BCM_AutoLowBeam;			/* 近光灯状态: 开:0x01 关:0x00 */
		unsigned char BCM_FrontWiperWorkStatus;		/* 前雨刮工作状态: 开:0x01 关:0x00 */
		unsigned char NVC_SwitchStatus;				/* 夜视系统开关按键状态: 开:0x01 关:0x00 */
		unsigned char ExtrinsicCalibReq;			/* 外参标定触发: "0：无输入 1：标定触发" */
		unsigned char PDCU_ShiftLvlPosn;     /*车辆实际档位 0:N 1:D 5:P 7:R*/

		ac_spd_t accelerated_speed;	   //加速度
		an_spd_t angular_speed;		   //角度
		Data_and_time_t Data_and_time; //日期和时间

	} vehicle_state_info_t;


	typedef struct
	{
		float m_pitch;		  //俯仰角
		float m_yaw;		  //偏航角
		float install_height; //安装高度
	} camera_external_t;

	typedef struct
	{
		float x1;
		float y1;
		float x2;
		float y2;
	} coordinates_t;

	//横向，纵向距离
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

	//横向，纵向速度
	typedef struct
	{
		float v1;
		float v2;
	} velocity_t;




	typedef struct
	{
		bool is_alarm;		   //是否报警
		unsigned int class_id; //目标的类别
		unsigned int track_id; //目标的跟踪id
		// float confidence;  //目标框的的分
		// int tracking_time;  //同一目标被检测帧数
		float ttc;				   //车辆相对速度
		position_t position;	   //目标的距离信息
		velocity_t velocity;	   //目标的速度信息
		coordinates_t coordinates; //目标的左上角和右下角坐标
	} BBox_t;

	typedef struct
	{
		int obj_number;					   //目标个数
		char is_alarm_frame;			   //单帧画面报警 0XFF保持现状， 0X00显示车报警三角图标，并持续3s， 0X01显示人报警图标，并持续3s， 0X02显示动物报警图标，并持续3s
		unsigned char car_fire_state;	   //车辆点火状态
		float car_speed;				   //车辆速度
		long long obj_frame_id;			   //当前帧id
		an_spd_t veh_angle;				   //车辆偏角
		camera_external_t camera_external; //镜头外参信息
		BBox_t bbox[MAX_OBJECT_NUMBER];
		char* error_code[9];
		float EPS_SAS_SteeringAngle;
		unsigned char PDCU_ShiftLvlPosn;
	} BoxMsg_t;


#ifdef __cplusplus
}
#endif
#endif