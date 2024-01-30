#ifndef USBCAN_ALYST_II_HPP
#define USBCAN_ALYST_II_HPP


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include "./tools/cxkj_can/include/controlcan.h"
#include <thread>
#include <iostream>
#include <chrono>
#include "./ars_40x/ars_40x_can.h"

#define msleep(ms) usleep((ms)*1000)
#define min(a,b)  (((a) < (b)) ? (a) : (b))

#define SEND_CMD  0

#define MAX_CHANNELS  2
#define CHECK_POINT  200
#define RX_WAIT_TIME  100
#define RX_BUFF_SIZE  500

class UsbCan_alyst_II
{
private:
    uint32_t a;
    uint32_t m_DevType{0};
    uint32_t m_DevIdx{0};
    uint32_t m_Debug{0};
public:
    UsbCan_alyst_II(uint32_t devidx,uint32_t devtype=4):m_DevType(devtype),m_DevIdx(devidx){}
    ~UsbCan_alyst_II(){}
    bool openFlag{false};

uint32_t reciveData(VCI_CAN_OBJ *vco,uint32_t bufCnt,uint32_t channel)
{
    uint32_t ret_cnt{0};
    auto retCnt = VCI_GetReceiveNum(m_DevType,m_DevIdx,channel);
    if(retCnt){
    ret_cnt = VCI_Receive(m_DevType, m_DevIdx, channel, vco,bufCnt,10);
    }else{
        // std::cout << "wait data......." << std::endl;
        // std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return ret_cnt;
}

bool sendData(UINT channel,UINT ID,BYTE remoteFlag,BYTE externFlag,const unsigned char *data,BYTE len){
    unsigned int dwRel;
    VCI_CAN_OBJ vco;
    vco.ID = ID ;
    vco.RemoteFlag = remoteFlag;
    vco.ExternFlag = externFlag;
    vco.DataLen = len;
    for(UINT j = 0;j < len;j++)
        vco.Data[j] = data[j];
    dwRel = VCI_Transmit(m_DevType, m_DevIdx, channel,&vco,1);
    if(dwRel>0){
        std::cout<<"send success."<<std::endl;
        return true;
    }
    else{
        printf("CAN%d TX failed: ID=%08x\n", channel, ID);
        return false;
    }
}

void getConfig(VCI_INIT_CONFIG &vic)
{
    vic.AccCode=0x80000008;
    vic.AccMask=0xFFFFFFFF;
    vic.Filter=1;
    vic.Mode=0;
    vic.Timing0=0x00;//baundRate 500k
    vic.Timing1=0x1c;

}
bool startCan(uint32_t channel){
    if(VCI_StartCAN(m_DevType, m_DevIdx, channel) !=1)
        return false;
    return true;
}
bool openCan(){
    //打开设备
    uint32_t dwRel = VCI_OpenDevice(m_DevType, m_DevIdx, 0);
    if (!dwRel) {
        printf("VCI_OpenDevice failed\n");
        return false;
    }
    printf("VCI_OpenDevice succeeded\n");
    openFlag = true;
    return openFlag;
}
bool usbCanInit(uint32_t channel){

    uint32_t dwRel{0};
    openCan();

    dwRel = VCI_ClearBuffer(m_DevType, m_DevIdx, channel);

    VCI_INIT_CONFIG _Config;
    getConfig(_Config);

    dwRel = VCI_InitCAN(m_DevType, m_DevIdx, channel, &_Config);

    if(!dwRel){
        return false;
        printf("VCI_InitCAN(%d,%d) failed\n",m_DevType, m_DevIdx);
    }  
    else
        printf("VCI_InitCAN(%d,%d) succeeded\n",m_DevType, m_DevIdx);

    // VCI_BOARD_INFO vbi;
    // dwRel = VCI_ReadBoardInfo(m_DevType, m_DevIdx, &vbi);
    // if(dwRel != 1)
    //     return false;
    // else
    //     // emit boardInfo(vbi);
    // return true;


    //启动通道
    if (!startCan(channel)) {
        printf("VCI_StartCAN(%d,%d) failed\n",m_DevType, m_DevIdx);
        openFlag = false;
        return false;
        
    }
    printf("VCI_StartCAN(%d,%d) succeeded\n",m_DevType, m_DevIdx);
    return true;
}

void usbCanClose(){
    if(openFlag)
        VCI_CloseDevice(m_DevType, m_DevIdx);
    printf("VCI_CloseDevice\n");
}

bool reSetCan(uint32_t channel){
    bool ret{false};
    if(VCI_ResetCAN(m_DevType, m_DevIdx, channel) !=1)
        ret = false;
    else ret = true;
    return ret;
}
static void searchCan(VCI_BOARD_INFO *info,uint32_t &cnt){
    cnt=VCI_FindUsbDevice2(info);
}

};
#endif