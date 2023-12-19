
#ifndef _POST_PROCESS_YOLO5_POST_PROCESS_H_
#define _POST_PROCESS_YOLO5_POST_PROCESS_H_


#define MAX_OBJECT_NUMBER 30

typedef struct
{
    float m_pitch;		  //俯仰角
    float m_yaw;		  //偏航角
    float install_height; //安装高度
} camera_external_t;


//车辆角速度
typedef struct
{
    float wx; //侧倾角
    float wy; //俯仰角
    float wz; //横摆角
} an_spd_t;




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
    bool is_alarm_frame;			   //单帧画面报警
    unsigned char car_fire_state;	   //车辆点火状态
    float car_speed;				   //车辆速度
    long long obj_frame_id;			   //当前帧id
    an_spd_t veh_angle;				   //车辆偏角
    camera_external_t camera_external; //镜头外参信息
    BBox_t bbox[MAX_OBJECT_NUMBER];
} BoxMsg_t;


#endif //
