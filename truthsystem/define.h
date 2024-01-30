#ifndef DEFINE_H
#define DEFINE_H

#ifndef HAVE_PF_RING
#define HAVE_PF_RING
#endif

typedef  unsigned char uchar8_t;
typedef  char  char8_t;
typedef  unsigned short uint16_t;
typedef  short int16_t;
typedef  unsigned int uint32_t;
typedef  int int32_t;
typedef  unsigned long  ulong64_t;
typedef  long long64_t;
typedef unsigned char byte;

#define CAMERA_NUM      9   //相机数量 
#define PANE_NUM        9   //窗格数量
#define HEADSIZE        128   
#define TYPENONE        0   //无
#define TYPEI2CDIRECT   1   //IIC
#define TYPEJIGUANG     2   //lidar

#define MAX_PANLE_NUM   9
#define MAX_ITEM_NUM    9

#define PI              3.141592654
#define AT128_DATASIZE_MAX  384000  //AT128单帧点云大小

#define RADAR_ARS408_OBJNUM_MAX 1000 //ARS408目标数量最多500条]
#define RADAR_ARS408_TABLE_MAX 250  //ARS408标签显示，最大250个

#endif // DEFINE_H
