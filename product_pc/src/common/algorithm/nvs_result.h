#ifndef NVS_RESULT_H
#define NVS_RESULT_H

//#include "bpu_predict_extension.h"
#ifdef __cplusplus
extern "C"
{
#endif
#define MAX_OBJECT_NUMBER 30

typedef struct
{
    float m_pitch;		  //俯仰角
    float m_yaw;		  //偏航角
    float install_height; //安装高度
} camera_external_t;

//车辆加速度
typedef struct
{
    float ax; //纵向加速度信号
    float ay; //横向加速度信号
    float az; //垂向加速度信号
} ac_spd_t;

//车辆角速度
typedef struct
{
    float wx; //侧倾角
    float wy; //俯仰角
    float wz; //横摆角
} an_spd_t;

//年月日时分妙
typedef struct
{
    unsigned int year;	  //年
    unsigned int month;  //月
    unsigned int date;	  //日
    unsigned int hour;	  //时
    unsigned int minute; // 分
    unsigned int second; //秒
} Data_and_time_t;

//车辆实时状态信息
typedef struct
{
    unsigned char BCM_HighBeamStatus;			//远光灯状态
    unsigned char BCM_FrontFoglightStatus;		//前雾灯状态
    float AC_AmbTemp;							//环境温度
    unsigned char BCM_AutoLowBeam;				//环境光照度
    unsigned char BCM_FrontWiperWorkStatus;		//前雨刮挡位输出状态
    unsigned char NVC_SwitchStatus;				//夜视系统开关状态
    unsigned char Ignition_switch_signal;		//点火开关信号
    float ESC_VehSpd;							//车速信号
    signed char ESC_StaticSlope;				//坡度信息
    float EPS_SAS_SteeringAngle;				//转向角信号
    float EPS_SAS_SteeringAngleSpd;				//转向角速度信号
    unsigned char EMS_BrakePedalSignal;			//制动踏板开关
    unsigned char Air_suspension_travel_signal; //空气悬挂行程信号
    unsigned char Target_type_mode;				//目标类型模式
    unsigned int Total_mileage;					//总里程
    float rpm;									//发动机转速
    unsigned char Driving_permission;			//行车允许
    unsigned char SOC;							//剩余电量
    unsigned char DTC;							//故障码

    float acceleratorPos; //加速
    float brakePos;		  //制动
    float Angle;		  //方向盘转角
    unsigned char YawValid;
    float Yaw;

    ac_spd_t accelerated_speed;	   //加速度
    an_spd_t angular_speed;		   //角速度
    Data_and_time_t Data_and_time; //日期和时间

} vehicle_state_info_t;

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
} BoxMsg_t;

#ifdef __cplusplus
}
#endif

#endif
