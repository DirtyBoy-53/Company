#ifndef I2C_ADDR_H
#define I2C_ADDR_H

// 设备信息页
#define DEVICE_INFO_BASE                0x0000
#define DEVICE_INFO_MFR                 (DEVICE_INFO_BASE + 0x01)           //厂家信息 0x01:高德  0x4c:睿创 len 2
#define DEVICE_INFO_PLATFORM            (DEVICE_INFO_BASE + 0x02)           //平台信息 0：asic1.0  1:asic2.0
#define DEVICE_INFO_VIDEO_COL           (DEVICE_INFO_BASE + 0x03)           //视频分辨率 行
#define DEVICE_INFO_VIDEO_ROW           (DEVICE_INFO_BASE + 0x04)           //视频分辨率 列
#define DEVICE_INFO_FPS                 (DEVICE_INFO_BASE + 0x05)           //帧率
#define DEVICE_INFO_APP_TYPE_H          (DEVICE_INFO_BASE + 0x10)           //类型  ASCII:    SY：送样  XD：项目代号
#define DEVICE_INFO_APP_TYPE_L          (DEVICE_INFO_BASE + 0x11)           //类型
#define DEVICE_INFO_APP_LOAD            (DEVICE_INFO_BASE + 0x16)           //APP加载位 1：A区   2：B区
#define DEVICE_INFO_APP_WRITE           (DEVICE_INFO_BASE + 0x17)           //APP存储位 1：A区   2：B区 
#define DEVICE_INFO_BOOT_VERSION        (DEVICE_INFO_BASE + 0x1C)           //boot版本号
#define DEVICE_INFO_SOFTWARE_VERSION0   (DEVICE_INFO_BASE + 0x20)           //软件版本号    len 8 年
#define DEVICE_INFO_SOFTWARE_VERSION1   (DEVICE_INFO_BASE + 0x21)           //软件版本号    len 8 月 + 日
#define DEVICE_INFO_SOFTWARE_VERSION2   (DEVICE_INFO_BASE + 0x22)           //软件版本号    len 8 主版本 + 次版本
#define DEVICE_INFO_SOFTWARE_VERSION3   (DEVICE_INFO_BASE + 0x23)           //软件版本号    len 8 修订版本 + 保留
#define DEVICE_INFO_HARDWARE_VERSION    (DEVICE_INFO_BASE + 0x28)           //硬件版本号    len 8
#define DEVICE_INFO_SN_CODE             (DEVICE_INFO_BASE + 0x30)           //序列号

#define DEVICE_INFO_HARDWARE_ERROR_CODE         (DEVICE_INFO_BASE + 0x60)           //硬件错误码    len 4
#define DEVICE_INFO_SHUTTER_ERROR_CODE          (DEVICE_INFO_BASE + 0x64)           //快门错误码    len 2
#define DEVICE_INFO_SENSEOR_INIT_ERROR_CODE     (DEVICE_INFO_BASE + 0x66)           //传感器初始化错误码    len 2
#define DEVICE_INFO_SENSEOR_ERROR_CODE          (DEVICE_INFO_BASE + 0x68)           //传感器错误码  len 2
// #define DEVICE_INFO_TEMP_ERROR_CODE             (DEVICE_INFO_BASE + 0x6A)           //错误码


// 系统控制页
#define SYSTEM_CONTROL_BASE                 0x0100
#define SYSTEM_CONTROL_RESET_CFG            (SYSTEM_CONTROL_BASE + 0x01)            //出厂设置
#define SYSTEM_CONTROL_SCENE_MODE           (SYSTEM_CONTROL_BASE + 0x05)            //软件复位控制 写1触发软件复位
#define SYSTEM_CONTROL_SR_UNLOCK            (SYSTEM_CONTROL_BASE + 0x10)            //特殊寄存器解锁
#define SYSTEM_CONTROL_SR_READ_END          (SYSTEM_CONTROL_BASE + 0x12)            //特殊寄存器-读完成后写1
#define SYSTEM_CONTROL_SR_WRITE_RESULT      (SYSTEM_CONTROL_BASE + 0x16)            //特殊寄存器-写完成后 结果
#define SYSTEM_CONTROL_ALG_TYPE             (SYSTEM_CONTROL_BASE + 0x1A)            //算法参数读类型
#define SYSTEM_CONTROL_SENSOR_FLASH_PAGE    (SYSTEM_CONTROL_BASE + 0x1B)            //探测器读寄存器页控制
#define SYSTEM_CONTROL_SENSOR_FLASH_COUNT   (SYSTEM_CONTROL_BASE + 0x1C)            //探测器读寄存器页数量
#define SYSTEM_CONTROL_SHUTTER_SWITCH       (SYSTEM_CONTROL_BASE + 0x21)            //快门开关 0：工作模式 1：休眠模式
#define SYSTEM_CONTROL_HEAT_MODE            (SYSTEM_CONTROL_BASE + 0x24)            //加热片模式
#define SYSTEM_CONTROL_HEAT_NTC_TEMP        (SYSTEM_CONTROL_BASE + 0x25)            //加热片NTC温度
#define SYSTEM_CONTROL_BIG_RING_TEMP        (SYSTEM_CONTROL_BASE + 0x2A)            //大热环温度
#define SYSTEM_CONTROL_NUC_TEMP_MAX         (SYSTEM_CONTROL_BASE + 0x2B)            //NUC温度阈值
#define SYSTEM_CONTROL_BOTTOM_TEMP_MAX      (SYSTEM_CONTROL_BASE + 0x2C)            //本地替换温度阈值

#define SYSTEM_CONTROL_AUTO_COMP_TIME       (SYSTEM_CONTROL_BASE + 0x30)            //自动补偿时间
#define SYSTEM_CONTROL_AUTO_COMP_SW         (SYSTEM_CONTROL_BASE + 0x31)            //自适应补偿开关
#define SYSTEM_CONTROL_VIDEO_FORMAT         (SYSTEM_CONTROL_BASE + 0x40)            //图像数据格式
#define SYSTEM_CONTROL_DEBUG_MODE           (SYSTEM_CONTROL_BASE + 0x41)            //调试模式开关

//图像设置页
#define IMAGE_SETTING_BASE                  0x0200
#define IMAGE_SETTING_SET_COLOR             (IMAGE_SETTING_BASE + 0x00)            //设置图像伪彩  0：白热 1: 黑热 12：彩虹 21：绿热 22：蓝热
#define IMAGE_SETTING_FREEZE                (IMAGE_SETTING_BASE + 0x10)            //图像冻结
#define IMAGE_SETTING_TEST                  (IMAGE_SETTING_BASE + 0x11)            //测试画面-0：实时画面 1：棋盘格画面 2：行渐变 3：列渐变


#define IMAGE_SETTING_ISP_PARAM_SW          (IMAGE_SETTING_BASE + 0x5F)            //切换ISP参数
#define IMAGE_SETTING_ISP_AUTO_SW           (IMAGE_SETTING_BASE + 0x60)            //ISP自动切档开关
#define IMAGE_SETTING_ISP_CONDITION_SW      (IMAGE_SETTING_BASE + 0x61)            //ISP切档条件设置 0:焦温判断 1:动态范围判断
#define IMAGE_SETTING_ISP__VALUE_SW         (IMAGE_SETTING_BASE + 0x62)            //ISP切档当前档位 0：默认isp参数 1：高温isp参数
#define IMAGE_SETTING_ISP_INTERVAL_SW       (IMAGE_SETTING_BASE + 0x63)            //ISP切档时间间隔
#define IMAGE_SETTING_ISP_TEMP_MAX_SW       (IMAGE_SETTING_BASE + 0x65)            //ISP切档温度阈值

//图像校正页
#define IMAGE_CORRECTION_BASE               0x0300
#define IMAGE_CORRECTION_SCENE_COMP         (IMAGE_CORRECTION_BASE + 0x00)        //场景补偿
#define IMAGE_CORRECTION_SHUTTER_COMP       (IMAGE_CORRECTION_BASE + 0x01)        //图像校正参数
#define IMAGE_CORRECTION_NUC                (IMAGE_CORRECTION_BASE + 0x02)        //NUC
#define IMAGE_CORRECTION_SCENE_NUM          (IMAGE_CORRECTION_BASE + 0x03)        //场景NUC

#define IMAGE_CORRECTION_BL_COMP            (IMAGE_CORRECTION_BASE + 0x10)        //BL补偿
#define IMAGE_CORRECTION_BH_COMP            (IMAGE_CORRECTION_BASE + 0x11)        //BH补偿
#define IMAGE_CORRECTION_K_CAL              (IMAGE_CORRECTION_BASE + 0x12)        //K计算
#define IMAGE_CORRECTION_K_SAVE             (IMAGE_CORRECTION_BASE + 0x13)        //K保存
#define IMAGE_CORRECTION_K_LOAD             (IMAGE_CORRECTION_BASE + 0x14)        //K加载
#define IMAGE_CORRECTION_INIT_K_LOAD        (IMAGE_CORRECTION_BASE + 0x15)        //初K加载

#define IMAGE_CORRECTION_SFFC_ENABLE        (IMAGE_CORRECTION_BASE + 0x20)        //SFFC使能
#define IMAGE_CORRECTION_SFFC_COLLECT       (IMAGE_CORRECTION_BASE + 0x21)        //SFFC采集
#define IMAGE_CORRECTION_SFFC_SAVE          (IMAGE_CORRECTION_BASE + 0x22)        //SFFC保存
#define IMAGE_CORRECTION_SFFC_LOAD          (IMAGE_CORRECTION_BASE + 0x23)        //SFFC加载

#define IMAGE_CORRECTION_CROSS_SW                   (IMAGE_CORRECTION_BASE + 0x30)        //十字光标
#define IMAGE_CORRECTION_CROSS_X                    (IMAGE_CORRECTION_BASE + 0x31)        //光标X
#define IMAGE_CORRECTION_CROSS_Y                    (IMAGE_CORRECTION_BASE + 0x32)        //光标Y
#define IMAGE_CORRECTION_BAD_PT_PRO                 (IMAGE_CORRECTION_BASE + 0x33)        //坏点处理 1：添加坏点 2：添加坏行（保留） 3：添加坏列（保留） 4：清空坏点列表 5：删除上一个坏点 0xFF：自定义代码 (自测使用) 
#define IMAGE_CORRECTION_SAVE_BAD_PT                (IMAGE_CORRECTION_BASE + 0x34)        //保存坏点
#define IMAGE_CORRECTION_Y16_CHECK_BAD_PT           (IMAGE_CORRECTION_BASE + 0x35)        //Y16一键坏点检测
#define IMAGE_CORRECTION_Y16_CHECK_BAD_PT_STATUS    (IMAGE_CORRECTION_BASE + 0x36)        //Y16一键坏点检测状态
#define IMAGE_CORRECTION_K_CHECK_BAD_PT             (IMAGE_CORRECTION_BASE + 0x37)        //K值坏点检测
#define IMAGE_CORRECTION_K_CHECK_BAD_PT_STATUS      (IMAGE_CORRECTION_BASE + 0x38)        //K值坏点检测状态
#define IMAGE_CORRECTION_BAD_PT_NUM                 (IMAGE_CORRECTION_BASE + 0x39)        //坏点个数
#define IMAGE_CORRECTION_CUR_BAD_PT_NUM             (IMAGE_CORRECTION_BASE + 0x3A)        //实时坏点个数

// 图像配置页
#define IMAGE_CONFIG_BASE                           0x0400
#define IMAGE_CONFIG_SWITCH_ALG                     (IMAGE_CONFIG_BASE + 0x00)        //一键开关算法
#define IMAGE_CONFIG_REDUCE_BAD_PT                  (IMAGE_CONFIG_BASE + 0x01)        //去坏点算法Bpr 
#define IMAGE_CONFIG_EDGE_ENHANCE_ALG               (IMAGE_CONFIG_BASE + 0x02)        //边缘增强算法Ee  
#define IMAGE_CONFIG_CONTRAST_ENHANCE_ALG           (IMAGE_CONFIG_BASE + 0x03)        //对比度增强算法Lwdr                                   
#define IMAGE_CONFIG_REDUCE_LID_ALG                 (IMAGE_CONFIG_BASE + 0x04)        //去锅盖算法Sffc
#define IMAGE_CONFIG_REDUCE_SPACE_NOISE_ALG         (IMAGE_CONFIG_BASE + 0x05)        //空域降噪算法（含两级Dns）
#define IMAGE_CONFIG_REDUCE_VGRAIN_ALG              (IMAGE_CONFIG_BASE + 0x06)        //去竖纹算法Fpnc
#define IMAGE_CONFIG_REDUCE_HGRAIN_ALG              (IMAGE_CONFIG_BASE + 0x07)        //去横纹算法Rfc
#define IMAGE_CONFIG_REDUCE_Y16_NOISE_ALG           (IMAGE_CONFIG_BASE + 0x08)        //3d去噪y16算法Nr3draw
#define IMAGE_CONFIG_REDUCE_Y8_NOISE_ALG            (IMAGE_CONFIG_BASE + 0x09)        //3d去噪y8算法Nr3dy

#define IMAGE_CONFIG_BRIGHTNESS             (IMAGE_CONFIG_BASE + 0x20)        //亮度
#define IMAGE_CONFIG_CONTRAST               (IMAGE_CONFIG_BASE + 0x21)        //对比度
#define IMAGE_CONFIG_DETAILS                (IMAGE_CONFIG_BASE + 0x22)        //细节增益
#define IMAGE_CONFIG_DIMMING                (IMAGE_CONFIG_BASE + 0x23)        //非线性调光系数
#define IMAGE_CONFIG_WIDEDYNAMIC            (IMAGE_CONFIG_BASE + 0x24)        //宽动态对比度增强
#define IMAGE_CONFIG_ENHANCEMENT1           (IMAGE_CONFIG_BASE + 0x25)        //边缘增强系数1
#define IMAGE_CONFIG_ENHANCEMENT2           (IMAGE_CONFIG_BASE + 0x26)        //边缘增强系数2
#define IMAGE_CONFIG_ENHANCEMENT3           (IMAGE_CONFIG_BASE + 0x27)        //边缘增强系数3
#define IMAGE_CONFIG_TIMEDOMAIN             (IMAGE_CONFIG_BASE + 0x28)        //时域降噪系统
#define IMAGE_CONFIG_SPACE_DOMAIN1          (IMAGE_CONFIG_BASE + 0x29)        //空域降噪系数1
#define IMAGE_CONFIG_SPACE_DOMAIN2          (IMAGE_CONFIG_BASE + 0x2A)        //空域降噪系数2
#define IMAGE_CONFIG_CLOSE_VGRAIN_VALUE     (IMAGE_CONFIG_BASE + 0x2B)        //去竖纹系数

// 探测器配置页
#define DETECTOR_CONFIG_BASE                0x0600
#define DETECTOR_CONFIG_RASEL               (DETECTOR_CONFIG_BASE + 0x00)        //RASEL
#define DETECTOR_CONFIG_INT                 (DETECTOR_CONFIG_BASE + 0x01)        //INT
#define DETECTOR_CONFIG_GAIN                (DETECTOR_CONFIG_BASE + 0x02)        //GAIN
#define DETECTOR_CONFIG_HSSD                (DETECTOR_CONFIG_BASE + 0x03)        //HSSD
#define DETECTOR_CONFIG_GFID                (DETECTOR_CONFIG_BASE + 0x04)        //GFID
#define DETECTOR_CONFIG_RC                  (DETECTOR_CONFIG_BASE + 0x05)        //RC
#define DETECTOR_CONFIG_RD                  (DETECTOR_CONFIG_BASE + 0x06)        //RD

#define DETECTOR_CONFIG_NUC_ENABLE          (DETECTOR_CONFIG_BASE + 0x40)        //NUC使能
#define DETECTOR_CONFIG_NUC_X_MAX           (DETECTOR_CONFIG_BASE + 0x41)        //NUC校正依据X上门限
#define DETECTOR_CONFIG_NUC_X_MIN           (DETECTOR_CONFIG_BASE + 0x42)        //NUC校正依据X下门限

#define DETECTOR_CONFIG_TEST_FRAME          (DETECTOR_CONFIG_BASE + 0x50)        //探测器棋盘格
#define DETECTOR_CONFIG_AD_ENABLE           (DETECTOR_CONFIG_BASE + 0x60)        //平均AD使能
#define DETECTOR_CONFIG_AD_CENTER           (DETECTOR_CONFIG_BASE + 0x61)        //中心AD使能
#define DETECTOR_CONFIG_AD_X                (DETECTOR_CONFIG_BASE + 0x62)        //单点ad x坐标
#define DETECTOR_CONFIG_AD_Y                (DETECTOR_CONFIG_BASE + 0x63)        //单点ad y坐标
#define DETECTOR_CONFIG_GET_AD_VALUE        (DETECTOR_CONFIG_BASE + 0x64)        //AD值

#define DETECTOR_CONFIG_FOCUS_TEMP          (DETECTOR_CONFIG_BASE + 0x70)        //数字焦温
#define DETECTOR_CONFIG_ENVIRONMENT_TEMP    (DETECTOR_CONFIG_BASE + 0x74)        //环温

#define DETECTOR_CONFIG_FLASH_PAGE          (DETECTOR_CONFIG_BASE + 0xF0)        //探测器寄存器页地址 
#define DETECTOR_CONFIG_FLASH_ADDR          (DETECTOR_CONFIG_BASE + 0xF1)        //探测器寄存器地址
#define DETECTOR_CONFIG_FLASH_VALUE         (DETECTOR_CONFIG_BASE + 0xF2)        //探测器寄存器值

//特殊功能寄存器
#define SPECIAL_CONFIG_BASE                 0xF000
#define SPECIAL_CONFIG_BOOT                 (SPECIAL_CONFIG_BASE + 0x01)        //boot程序
#define SPECIAL_CONFIG_APP                  (SPECIAL_CONFIG_BASE + 0x02)        //应用程序
#define SPECIAL_CONFIG_COLOR_TABLE          (SPECIAL_CONFIG_BASE + 0x03)        //伪彩表

#define SPECIAL_CONFIG_CALIB_IN             (SPECIAL_CONFIG_BASE + 0x10)        //内参值
#define SPECIAL_CONFIG_CALIB_EX             (SPECIAL_CONFIG_BASE + 0x11)        //外参值
#define SPECIAL_CONFIG_ISP_ALG              (SPECIAL_CONFIG_BASE + 0x12)        //算法参数
#define SPECIAL_CONFIG_SENSOR_FLASH         (SPECIAL_CONFIG_BASE + 0x14)        //探测器参数

#define SPECIAL_CONFIG_DEFAULT_BAD_PT_LIST  (SPECIAL_CONFIG_BASE + 0x20)        //初始化坏点列表
#define SPECIAL_CONFIG_ADD_BAD_PT_LIST      (SPECIAL_CONFIG_BASE + 0x21)        //新增坏点列表
#define SPECIAL_CONFIG_FLASH_BAD_PT_LIST    (SPECIAL_CONFIG_BASE + 0x22)        //flash坏点列表

#define SPECIAL_CONFIG_K_IMG                (SPECIAL_CONFIG_BASE + 0x30)        //K图像
#define SPECIAL_CONFIG_BL_IMG               (SPECIAL_CONFIG_BASE + 0x31)        //BL图像
#define SPECIAL_CONFIG_BH_IMG               (SPECIAL_CONFIG_BASE + 0x32)        //BH图像
#define SPECIAL_CONFIG_SFFC_IMG             (SPECIAL_CONFIG_BASE + 0x33)        //SFFC图像

#endif // I2C_ADDR_H
