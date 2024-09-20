#include "ParamLineHandler.h"

ParamLineHandler::ParamLineHandler()
{
    mParamStr   << "快门状态" << "快门工作模式" << "大闭环标志" << "本地NUC标志" << "AD使能标志"
                << "自适应补偿开关" << "伪彩类型" << "冻结状态" << "焦温" << "数据格式" << "场景模式"
                << "补偿触发类型" << "宽动态范围" << "平均AD" << "ADC0" << "ADC1" << "加热环温度"
                << "PCB温度" << "加热环模式" << "加热环开关" << "故障码";
}

ParamLineInfo ParamLineHandler::parseParamLine(QByteArray data)
{
    if(data.size() < 400 ) return ParamLineInfo();

    m_paramInfo.shutterFlag = data.at(4);
    m_paramInfo.shutterMode = data.at(5);
    m_paramInfo.closeFlag = ((data.at(7) & 0xff) << 8) | (data.at(6) & 0xff);
    m_paramInfo.nucFlag = ((data.at(9) & 0xff) << 8) | (data.at(8) & 0xff);
    m_paramInfo.adFlag = data.at(10);
    m_paramInfo.autoFlag = data.at(11);
    m_paramInfo.colorType = data.at(12);
    m_paramInfo.freezeStatus = data.at(13);
    m_paramInfo.jTemp = ((data.at(15) & 0xff) << 8) | (data.at(14) & 0xff);
    m_paramInfo.outputMode = data.at(16);
    m_paramInfo.sceneMode = data.at(17);
    m_paramInfo.comType = ((data.at(19) & 0xff) << 8) | (data.at(18) & 0xff);
    m_paramInfo.hist = ((data.at(23) & 0xff) << 24) |((data.at(22) & 0xff) << 16) |
             ((data.at(21) & 0xff) << 8) | (data.at(20) & 0xff);
    m_paramInfo.avgAd = ((data.at(27) & 0xff) << 24) |((data.at(26) & 0xff) << 16) |
             ((data.at(25) & 0xff) << 8) | (data.at(24) & 0xff);
    m_paramInfo.adc0 = ((data.at(31) & 0xff) << 8) | (data.at(30) & 0xff);
    m_paramInfo.adc1 = ((data.at(33) & 0xff) << 8) | (data.at(32) & 0xff);
    m_paramInfo.ringTemp = ((data.at(45) & 0xff) << 8) | (data.at(44) & 0xff);
    m_paramInfo.pcbTemp = ((data.at(47) & 0xff) << 8) | (data.at(46) & 0xff);
    m_paramInfo.ringState = data.at(42);
    m_paramInfo.ringSwitch = data.at(43);

    for(auto i = 108,j=0;i <= 115; ++i,++j){
        m_paramInfo.errCode[j] = data.at(i);
    }

    m_paramInfo.bigGear = data.at(159);
    m_paramInfo.smallGearL = data.at(161);
    m_paramInfo.smallGearH = data.at(160);
    m_paramInfo.bDiff = ((data.at(167) & 0xff) << 8) | (data.at(166) & 0xff);

    m_paramInfo.contrast = data.at(400);
    m_paramInfo.light = data.at(401);
    m_paramInfo.doNoise = data.at(402);
    m_paramInfo.enhance = data.at(403);
    return m_paramInfo;
}

ParamLineInfo ParamLineHandler::getParamInfo()
{
    return m_paramInfo;
}

QStringList &ParamLineHandler::getParamStr()
{
    return mParamStr;
}
