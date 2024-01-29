#include "producemanager.h"
#include <burningde1000.h>
#include <LeakageDE1006.h>
#include <inputmessagebox.h>
#include <burningmcucb1000.h>
#include <burningarmce1001.h>
#include "autoprocesscheckarm.h"
#include <autoprocesscheckSensor.h>
#include <autocalibinde1010.h>
#include <comproduce.h>
#include <mtfcl1004.h>
#include <NonUniformDE1008.h>
#include "autoprocesseleclock.h"
#include "autoprocessguide.h"
#include "httppassstation.h"
#include "productpacking.h"
#include "autofpccheck.h"
#include <armcb1002.h>
#include "dispensingcuring.h"
#include "dustimagedetection.h"
#include "autoprocesseleclockCB.h"
static ProduceInfo g_produces[100] = {
    {"电路板调试烧录","DE1000", 4},
    {"电路板调试烧录","DE1100", 4},
    {"窗口组件安装","DE1001", 8},
    {"镜头组件-窗口组件安装","DE1002", 7},
    {"机芯组件安装","DE1003", 8},
    {"侧面导热片安装","DE1004", 8},
    {"摄像头后壳安装","DE1005", 8},
    {"气密性检测","DE1006", 5},
    {"板对线连接器安装","DE1007", 8},
    {"甩线线缆安装安装","DE1107", 8},
    {"非均匀性校正","DE1008", 6},
    {"麻点","DE1009", 6},
    {"内参标定","DE1010", 1},
    {"摄像头支架安装","DE1011", 8},
    {"老化测试","DE1012", 7},
    {"EOL检测","DE1013", 1},
    {"成品包装","DE1014", 9},
    {"主板MCU烧录调试","CB1000", 4},
    {"主板ARM程序烧录","CB1001", 4},
    {"控制盒组装","CB1002", 7},
    {"控制盒主板安装","CB1102", 8},
    {"控制盒下盖安装","CB1202", 8},
    {"老化测试","CB1003", 7},
    {"EOL检测","CB1004", 3},
    {"成品包装","CB1005", 9},
    {"镜头安装","CL1000", 1},
    {"快门安装","CL1001", 8},
    {"镜头支架组件安装","CL1101", 8},
    {"探测器安装","CL1002", 8},
    {"前灰尘检测","CL1102", 7},
    {"快门线点胶","CL1003", 7},
    {"镜头调焦及MTF测试","CL1004", 10},
    {"镜头点胶","CL1005", 7},
    {"EOL检测","CL1006", 7},
    {"产品包装","CL1007", 9},
    {"GP12","GP1000", 7},
    {"产品包装","IsPackage", 9},
    {"前后壳点胶","DE2001", 7},
    {"前壳窗片点胶","DE2002", 7},
    {"快门组件锁付","AA1007",8},
    {"ASIC锁付","AA1008",8},
    {"快门焊接","AA1009",8},
    {"",""},
};

ProduceManager::ProduceManager()
{
    for(int i = 0; i < 100; i ++) {
        if(g_produces[i].sName.length() > 0 &&
                g_produces[i].sCode.length() > 0) {
            m_produces.append(g_produces[i]);
        }else {
            break;
        }
    }
}

QVector<ProduceInfo> ProduceManager::produces() const
{
    return m_produces;
}

int ProduceManager::getUiTemplate(QString sCode)
{
    int iS = 0;
    for(int i = 0; i < 100; i ++) {
        if(g_produces[i].sName.length() > 0 &&
                g_produces[i].sCode.length() > 0) {
            if(sCode == g_produces[i].sCode) {
                iS = g_produces[i].uiComponent;
            }
        } else {
            break;
        }
    }
    return iS;
}

BaseProduce *ProduceManager::getCurProduce()
{
    BaseProduce * pPro = nullptr;
    QString sCode = ConfigInfo::getInstance()->cacheInfo().sCodeId;
    if(sCode == "DE1000" || sCode == "DE1100") {
        pPro = new BurningDe1000;
    }else if(sCode == "CB1000") {
        pPro = new BurningMcuCB1000;
    }else if(sCode == "CB1001") {
        pPro = new BurningArmCE1001;
    }else if(sCode == "CB1002") {
        pPro = new ArmCb1002();
    }else if(sCode == "CB1102" || sCode == "CB1202") {
        // 2024年1月11日 向华 新增控制盒主板安装和下盖安装
        pPro = new AutoProcessElecLockCB();
    }else if(sCode == "CB1004") {
        pPro = new AutoProcessCheckArm;
    } else if(sCode == "DE1013") {
        pPro = new AutoProcessCheckSensor;
    } else if(sCode == "DE1010") {
        pPro = new AutoCalibInDe1010;
    } else if(sCode == "DE1006") {
        pPro = new LeakageDE1006;
    } else if(sCode == "CL1004") {
        pPro = new MtfCL1004;
    } else if(sCode == "DE1008" || sCode == "DE1009") {
        pPro = new NonUniform;
    } else if(sCode == "DE1001") {
        pPro = new AutoProcessElecLock;
    } else if(sCode == "DE1002") {
        pPro = new AutoFpcCheck;
    } else if(sCode=="GP1000" ||
               sCode=="CB1003" || sCode=="DE1012" ) {
        pPro = new AutoProcessComGuide(0);
    } else if(sCode == "CL1003" || sCode == "CL1005" || sCode == "DE2001" || sCode == "DE2002"){
        pPro = new DispensingCuring();
    } else if(sCode == "DE1003") {
        pPro = new AutoProcessElecLock;
    } else if(sCode == "DE1004") {
        pPro = new AutoProcessElecLock;
    } else if(sCode == "DE1005") {
        pPro = new AutoProcessElecLock;
    } else if(sCode == "DE1007") {
        pPro = new AutoProcessElecLock;
    } else if(sCode == "DE1107") {
        pPro = new AutoProcessElecLock;
    } else if(sCode == "DE1011") {
        pPro = new AutoProcessElecLock;
    } else if(sCode == "CL1000") {
        pPro = new HttpPassStation;
    } else if(sCode == "CL1001"  || sCode == "AA1007" ||
               sCode == "AA1008" || sCode == "AA1009" ) {
        pPro = new AutoProcessElecLock;
    } else if(sCode == "CL1101") {
        pPro = new AutoProcessElecLock;
    } else if(sCode == "CL1002") {
        pPro = new AutoProcessElecLock;
    } else if(sCode == "CL1006" || sCode == "CL1102") {
        pPro = new AutoProcessComGuide(1);
    } else if(sCode == "DE1014" || sCode == "CB1005" || sCode == "CL1007" || sCode == "IsPackage") {
        pPro = new productpacking;
    } else {
        pPro = new ComProduce();
    }
    return pPro;
}

QString ProduceManager::getTitle(QString sCode)
{
    QString title = sCode;
    for(int i = 0; i < 100; i ++) {
        if(g_produces[i].sName.length() > 0 &&
                g_produces[i].sCode.length() > 0) {
            if(sCode == g_produces[i].sCode) {
                title = sCode + "-"+ g_produces[i].sName;
            }
        } else {
            break;
        }
    }
    return title;
}

