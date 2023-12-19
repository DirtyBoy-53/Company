#include "asiccontrol.h"
#include "socketmanager.h"
#include "messageid.h"
#include <QJsonArray>

AsicControl::AsicControl(SocketManager* socket)
{
}

AsicControl::AsicControl(QString sIp)
{
    m_tcpClient = new TcpClient;
    m_tcpClient->init(sIp, "6001");
}

AsicControl::~AsicControl()
{
    if(m_tcpClient) {
        delete m_tcpClient;
    }
}

void AsicControl::Reset()
{
    m_tcpClient->sendCommand(MESSAGE_ID_OPERATE_RESET, MSG_ACTION_SET, m_sOut);
}

void AsicControl::Save()
{
    m_tcpClient->sendCommand(MESSAGE_ID_OPERATE_SAVE, MSG_ACTION_SET, m_sOut);
}

void AsicControl::StartWindowHot(const int mode, const int max)
{
    QJsonObject jsonNode;
    jsonNode.insert("mode", mode);
    jsonNode.insert("temp_max", max);
    m_tcpClient->sendCommand(MESSAGE_ID_OPERATE_HEATING, MSG_ACTION_SET, jsonNode, m_sOut);
}

bool AsicControl::GetSffcValue()
{
    if (m_tcpClient->sendCommand(MESSAGE_ID_ADJUST_IMG_SFFC_COLLECT, MSG_ACTION_SET, m_sOut)) {
        return resultConfirm(MESSAGE_ID_ADJUST_IMG_SFFC_COLLECT, m_sOut);
    }

    return false;
}

bool AsicControl::SaveSffcValue()
{
    if (m_tcpClient->sendCommand(MESSAGE_ID_ADJUST_IMG_SFFC_SAVE, MSG_ACTION_SET, m_sOut)) {
        return resultConfirm(MESSAGE_ID_ADJUST_IMG_SFFC_SAVE, m_sOut);
    }

    return false;
}

bool AsicControl::ChangeWorkMode(const bool isSleep)
{
    QJsonObject jsonNode;
    jsonNode.insert("is_sleep", isSleep ? 1 : 0);
    if (m_tcpClient->sendCommand(MESSAGE_ID_OPERATE_SHUTTER, MSG_ACTION_SET, jsonNode, m_sOut)) {
        return resultConfirm(MESSAGE_ID_OPERATE_SHUTTER, m_sOut);
    }

    return false;
}

bool AsicControl::ChangeMrtdSwitch(const int id, const int value)
{
    QJsonObject jsonNode;
    jsonNode.insert("id", id);
    jsonNode.insert("value", value);
    if (m_tcpClient->sendCommand(MESSAGE_ID_MRTD_SWITCH, MSG_ACTION_SET, jsonNode, m_sOut)) {
        return resultConfirm(MESSAGE_ID_MRTD_SWITCH, m_sOut);
    }

    return false;
}

bool AsicControl::BhComp()
{
    if (m_tcpClient->sendCommand(MESSAGE_ID_ADJUST_IMG_BH_COMP, MSG_ACTION_SET, m_sOut)) {
        return resultConfirm(MESSAGE_ID_ADJUST_IMG_BH_COMP, m_sOut);
    }

    return false;
}

bool AsicControl::BlComp()
{
    if (m_tcpClient->sendCommand(MESSAGE_ID_ADJUST_IMG_BL_COMP, MSG_ACTION_SET, m_sOut)) {
        return resultConfirm(MESSAGE_ID_ADJUST_IMG_BL_COMP, m_sOut);
    }

    return false;
}

bool AsicControl::CaluteK()
{
    if (m_tcpClient->sendCommand(MESSAGE_ID_ADJUST_IMG_CALCULATE_K, MSG_ACTION_SET, m_sOut)) {
        return resultConfirm(MESSAGE_ID_ADJUST_IMG_CALCULATE_K, m_sOut);
    }

    return false;
}

bool AsicControl::LoadDefaultK()
{
    if (m_tcpClient->sendCommand(MESSAGE_ID_ADJUST_IMG_LOAD_DEFAULT_K, MSG_ACTION_SET, m_sOut)) {
        return resultConfirm(MESSAGE_ID_ADJUST_IMG_LOAD_DEFAULT_K, m_sOut);
    }
    return false;
}

bool AsicControl::LoadK()
{
    if (m_tcpClient->sendCommand(MESSAGE_ID_ADJUST_IMG_LOAD_K, MSG_ACTION_SET, m_sOut)) {
        return resultConfirm(MESSAGE_ID_ADJUST_IMG_LOAD_K, m_sOut);
    }

    return false;
}

bool AsicControl::SaveK()
{
    if (m_tcpClient->sendCommand(MESSAGE_ID_ADJUST_IMG_SAVE_K, MSG_ACTION_SET, m_sOut)) {
        return resultConfirm(MESSAGE_ID_ADJUST_IMG_SAVE_K, m_sOut);
    }
    return false;
}

bool AsicControl::SceneComp()
{
    if (m_tcpClient->sendCommand(MESSAGE_ID_ADJUST_IMG_SCENE_COMP, MSG_ACTION_SET, m_sOut)) {
        return resultConfirm(MESSAGE_ID_ADJUST_IMG_SCENE_COMP, m_sOut);
    }

    return false;
}

bool AsicControl::ShutterComp()
{
    if (m_tcpClient->sendCommand(MESSAGE_ID_ADJUST_IMG_SHUTTER_COMP, MSG_ACTION_SET, m_sOut)) {
        return resultConfirm(MESSAGE_ID_ADJUST_IMG_SHUTTER_COMP, m_sOut);
    }

    return false;
}

bool AsicControl::GetAd(int &ad)
{
    if (m_tcpClient->sendCommand(MESSAGE_ID_SENSOR_AVERAGE_AD_VALUE, MSG_ACTION_REQUEST, m_sOut)) {
        QJsonParseError jsonError;
        QJsonDocument document = QJsonDocument::fromJson(m_sOut.toStdString().data(),
                                                            &jsonError);
        if (document.isNull() || !document.isObject() || jsonError.error != QJsonParseError::NoError) {
            return false;
        }

        QJsonObject object = document.object();
        if (object.value("retcode").toInt() != 0 ||
                object.value("id").toInt() != MESSAGE_ID_SENSOR_AVERAGE_AD_VALUE) {
            return false;
        }
        if (object.contains("content")) {
            QJsonObject content = object.value("content").toObject();
            if (content.contains("ad_value")) {
                ad = content.value("ad_value").toInt(-1);
                if (ad == -1)
                    return false;
                return true;
            }
        }
    }

    return false;
}

void AsicControl::AddPoint(int x, int y)
{
    QJsonObject jsonNode;
    jsonNode.insert("x", x);
    jsonNode.insert("y", y);
    m_tcpClient->sendCommand(MESSAGE_ID_ADJUST_IMG_CURRENT_POINT, MSG_ACTION_SET, jsonNode, m_sOut);
}

void AsicControl::AddBadPoint(int x, int y)
{
    QJsonObject jsonNode;
    jsonNode.insert("x", x);
    jsonNode.insert("y", y);
    m_tcpClient->sendCommand(MESSAGE_ID_ADJUST_IMG_ADD_BAD_POINT, MSG_ACTION_SET, jsonNode, m_sOut);
}

bool AsicControl::GetArtOsVersion(QString &version)
{
    if (m_tcpClient->sendCommand(MESSAGE_ID_CUSTOM_GET_VERSION, MSG_ACTION_REQUEST, m_sOut)) {
        QJsonParseError jsonError;
        QJsonDocument document = QJsonDocument::fromJson(m_sOut.toStdString().data(),
                                                            &jsonError);
        if (document.isNull() || !document.isObject() || jsonError.error != QJsonParseError::NoError) {
            return false;
        }

        QJsonObject object = document.object();
        if (object.value("retcode").toInt() != 0 ||
                object.value("id").toInt() != MESSAGE_ID_CUSTOM_GET_VERSION) {
            return false;
        }
        if (object.contains("content")) {
            QJsonObject content = object.value("content").toObject();
            if (content.contains("version")) {
                version = content.value("version").toString();
                return true;
            }
        }
    }

    return false;
}

bool AsicControl::SaveBadPoint()
{
    if (m_tcpClient->sendCommand(MESSAGE_ID_ADJUST_IMG_SAVE_BAD_POINT, MSG_ACTION_SET, m_sOut)) {
        return resultConfirm(MESSAGE_ID_ADJUST_IMG_SAVE_BAD_POINT, m_sOut);
    }
    return false;
}

bool AsicControl::DeleteBadPoint()
{
    if (m_tcpClient->sendCommand(MESSAGE_ID_ADJUST_IMG_DELETE_BAD_POINT, MSG_ACTION_SET, m_sOut)) {
        return resultConfirm(MESSAGE_ID_ADJUST_IMG_DELETE_BAD_POINT, m_sOut);
    }
    return false;
}

void AsicControl::GetWindowTemp()
{
    m_tcpClient->sendCommand(MESSAGE_ID_OPERATE_HEAT_TEMP, MSG_ACTION_REQUEST, m_sOut);
}

bool AsicControl::GetAsicVersion(QString &version)
{
    if (m_tcpClient->sendCommand(MESSAGE_ID_GET_SYS_SOFT_WARE_VERSION, MSG_ACTION_REQUEST, m_sOut)) {
        QJsonParseError jsonError;
        QJsonDocument document = QJsonDocument::fromJson(m_sOut.toStdString().data(),
                                                            &jsonError);
        if (document.isNull() || !document.isObject() || jsonError.error != QJsonParseError::NoError) {
            return false;
        }

        QJsonObject object = document.object();
        if (object.value("retcode").toInt() != 0 ||
                object.value("id").toInt() != MESSAGE_ID_GET_SYS_SOFT_WARE_VERSION) {
            return false;
        }
        if (object.contains("content")) {
            QJsonObject content = object.value("content").toObject();
            if (content.contains("asic_version")) {
                version = content.value("asic_version").toString();
                return true;
            }
        }
    }

    return false;
}

bool AsicControl::GetDeviceName(QString &deviceName)
{
    if (m_tcpClient->sendCommand(MESSAGE_ID_GET_SYS_DEV_MODE, MSG_ACTION_REQUEST, m_sOut)) {
        QJsonParseError jsonError;
        QJsonDocument document = QJsonDocument::fromJson(m_sOut.toStdString().data(),
                                                            &jsonError);
        if (document.isNull() || !document.isObject() || jsonError.error != QJsonParseError::NoError) {
            return false;
        }

        QJsonObject object = document.object();
        if (object.value("retcode").toInt() != 0 ||
                object.value("id").toInt() != MESSAGE_ID_GET_SYS_DEV_MODE) {
            return false;
        }
        if (object.contains("content")) {
            QJsonObject content = object.value("content").toObject();
            if (content.contains("dev_mode")) {
                deviceName = content.value("dev_mode").toString();
                return true;
            }
        }
    }

    return false;
}

bool AsicControl::SetKBadPoint(int max)
{
    QJsonObject json;
    json.insert("max", max);
    if (m_tcpClient->sendCommand(MESSAGE_ID_ADJUST_IMG_K_BAD_POINT, MSG_ACTION_SET, json, m_sOut)) {
        return resultConfirm(MESSAGE_ID_ADJUST_IMG_K_BAD_POINT, m_sOut);
    }

    return false;
}

bool AsicControl::SetY16BadPoint(int max)
{
    QJsonObject json;
    json.insert("max", max);
    if (m_tcpClient->sendCommand(MESSAGE_ID_ADJUST_IMG_Y16_BAD_POINT, MSG_ACTION_SET, json, m_sOut)) {
        return resultConfirm(MESSAGE_ID_ADJUST_IMG_Y16_BAD_POINT, m_sOut);
    }

    return false;
}

void AsicControl::ManulNuc()
{
    m_tcpClient->sendCommand(MESSAGE_ID_ADJUST_IMG_MANUL_NUC, MSG_ACTION_SET, m_sOut);
}

void AsicControl::SffcEnable(const int enable)
{
    m_tcpClient->sendCommand(MESSAGE_ID_ADJUST_IMG_SFFC_ENABLE, MSG_ACTION_SET, m_sOut);
}

bool AsicControl::SffcLoad()
{
    if (m_tcpClient->sendCommand(MESSAGE_ID_ADJUST_IMG_SFFC_LOAD, MSG_ACTION_SET, m_sOut)) {
        return resultConfirm(MESSAGE_ID_ADJUST_IMG_SFFC_LOAD, m_sOut);
    }

    return false;
}

void AsicControl::AutoBadPoint()
{
    m_tcpClient->sendCommand(MESSAGE_ID_SYS_SN, MSG_ACTION_SET, m_sOut);
}

bool AsicControl::SetPattle(const int pattle)
{
    QJsonObject jsonNode;
    jsonNode.insert("pattle", pattle);
    if(m_tcpClient->sendCommand(MESSAGE_ID_VIDEO_PATTLE, MSG_ACTION_SET, jsonNode, m_sOut)){
        return resultConfirm(MESSAGE_ID_VIDEO_PATTLE, m_sOut);
    }

    return false;
}

bool AsicControl::AutoAdapteComp(const int mode)
{
    QJsonObject jsonNode;
    jsonNode.insert("auto_comp", mode);
    if(m_tcpClient->sendCommand(MESSAGE_ID_OPERATE_AUTO_COMP_ENABLE, MSG_ACTION_SET, jsonNode, m_sOut)){
        return resultConfirm(MESSAGE_ID_OPERATE_AUTO_COMP_ENABLE, m_sOut);
    }

    return false;
}

bool AsicControl::GetFocusTemp(float &ftemp)
{
    if (m_tcpClient->sendCommand(MESSAGE_ID_SENSOR_FOCUS_TEMP, MSG_ACTION_REQUEST, m_sOut)) {
        QJsonParseError jsonError;
        QJsonDocument document = QJsonDocument::fromJson(m_sOut.toStdString().data(),
                                                            &jsonError);
        if (document.isNull() || !document.isObject() || jsonError.error != QJsonParseError::NoError) {
            return false;
        }

        QJsonObject object = document.object();
        if (object.value("retcode").toInt() != 0 ||
                object.value("id").toInt() != MESSAGE_ID_SENSOR_FOCUS_TEMP) {
            return false;
        }
        if (object.contains("content")) {
            QJsonObject content = object.value("content").toObject();
            if (content.contains("focus_temp")) {
                ftemp = content.value("focus_temp").toDouble() / 100;
                return true;
            }
        }
    }

    return false;
}

void AsicControl::SetAlgorithm(AsicAlgorithmInfo info)
{
    bool isChange = false;
    QJsonObject jsonNode;
    if(m_algorithmPage.castUp != info.castUp) {
        jsonNode.insert("cast_up_ratio", info.castUp);
        isChange = true;
    }
    if(m_algorithmPage.castDown != info.castDown) {
        jsonNode.insert("cast_down_ratio", info.castDown);
        isChange = true;
    }
    if(m_algorithmPage.noiseGrade0 != info.noiseGrade0) {
        jsonNode.insert("remove_noise_grade_th0", info.noiseGrade0);
        isChange = true;
    }
    if(m_algorithmPage.noiseGrade1 != info.noiseGrade1) {
        jsonNode.insert("remove_noise_grade_th1", info.noiseGrade1);
        isChange = true;
    }
    if(m_algorithmPage.ideDetails != info.ideDetails) {
        jsonNode.insert("ide_details", info.ideDetails);
        isChange = true;
    }
    if(isChange) {
        m_tcpClient->sendCommand(MESSAGE_ID_ASIC_ALGORITHM, MSG_ACTION_SET, jsonNode, m_sOut);
        m_algorithmPage = info;
    }
}

bool AsicControl::SceneNuc()
{
    if (m_tcpClient->sendCommand(MESSAGE_ID_ADJUST_IMG_SCENE_NUC, MSG_ACTION_SET, m_sOut)) {
        return resultConfirm(MESSAGE_ID_ADJUST_IMG_SCENE_NUC, m_sOut);
    }
    return false;
}

bool AsicControl::setTransmitFps(int fps)
{
    QJsonObject jsonNode;
    jsonNode.insert("fps", fps);
    if (m_tcpClient->sendCommand(MESSAGE_ID_CUSTOM_SET_FPS, MSG_ACTION_SET, jsonNode, m_sOut)) {
        return resultConfirm(MESSAGE_ID_CUSTOM_SET_FPS, m_sOut);
    }
    return false;
}

void AsicControl::SetEnhanceGrade(const int value)
{
    QJsonObject jsonNode;
    jsonNode.insert("enhance_grade", value);
    m_tcpClient->sendCommand(MESSAGE_ID_OPERATE_ENHANCE_GRADE, MSG_ACTION_SET, jsonNode, m_sOut);
}

bool AsicControl::SetSn(QString sn)
{
    QJsonObject jsonNode;
    jsonNode.insert("sn_code", sn);
    if (m_tcpClient->sendCommand(MESSAGE_ID_SYS_SN, MSG_ACTION_SET, jsonNode, m_sOut)) {
        return resultConfirm(MESSAGE_ID_SYS_SN, m_sOut);
    }

    return false;
}

QString AsicControl::GetSn()
{
    QString sn;
    if(m_tcpClient->sendCommand(MESSAGE_ID_SYS_SN, MSG_ACTION_REQUEST, m_sOut)){
        QJsonParseError jsonError;
        QJsonDocument document = QJsonDocument::fromJson(m_sOut.toStdString().data(),
                                                            &jsonError);
        if(document.isNull() || !document.isObject() || jsonError.error != QJsonParseError::NoError) {
            return "";
        }

        QJsonObject object = document.object();
        if(object.contains("content")) {
            QJsonObject content = object.value("content").toObject();
            if(content.contains("sn_code")) {
                sn = content.value("sn_code").toString().toUpper();
                return sn;
            }
        }

    }

    return "";
}

void AsicControl::DeleteCurrentBadPoint()
{
    m_tcpClient->sendCommand(MESSAGE_ID_ADJUST_IMG_DELETE_CURRENT_BAD_POINT, MSG_ACTION_SET, m_sOut);
}

bool AsicControl::SendCustom(int addr, int value)
{
    QJsonObject jsonNode;
    jsonNode.insert("addr", addr);
    jsonNode.insert("value", value);
    if (m_tcpClient->sendCommand(MESSAGE_ID_CUSTOM_I2C_WRITE, MSG_ACTION_SET, jsonNode, m_sOut)) {
        return resultConfirm(MESSAGE_ID_CUSTOM_I2C_WRITE, m_sOut);
    }
    return false;
}

bool AsicControl::ReadCustom(int addr, int & value)
{
    QJsonObject jsonNode;
    jsonNode.insert("addr", addr);
    // {\"action\":2,\"content\":{\"read_value\":63},\"id\":1501,\"retcode\":0}
    if(m_tcpClient->sendCommand(MESSAGE_ID_CUSTOM_I2C_READ, MSG_ACTION_REQUEST, jsonNode, m_sOut)) {
        QJsonParseError jsonError;
        QJsonDocument document = QJsonDocument::fromJson(m_sOut.toStdString().data(),
                                                            &jsonError);
        if (document.isNull() || !document.isObject() || jsonError.error != QJsonParseError::NoError) {
            return false;
        }

        QJsonObject object = document.object();
        if (object.value("retcode").toInt() != 0 ||
                object.value("id").toInt() != MESSAGE_ID_CUSTOM_I2C_READ) {
            return false;
        }
        if (object.contains("content")) {
            QJsonObject content = object.value("content").toObject();
            if (content.contains("read_value")) {
                value = content.value("read_value").toInt();
                return true;
            }
        }
    }

    return false;
}

// Y16 3
bool AsicControl::changeSensorFormat(const int mode)
{
    QJsonObject jsonNode;
    jsonNode.insert("video_format", mode);
    if (m_tcpClient->sendCommand(MESSAGE_ID_OPERATE_CHANGE_VIDEO_FORMAT, MSG_ACTION_SET, jsonNode, m_sOut)) {
        return resultConfirm(MESSAGE_ID_OPERATE_CHANGE_VIDEO_FORMAT, m_sOut);
    }
    return false;
}

void AsicControl::ChangeSensorCfg(const int type)
{
    QJsonObject jsonNode;
    jsonNode.insert("sensor_type", type);
    m_tcpClient->sendCommand(MESSAGE_ID_SENSOR_CFG, MSG_ACTION_SET, jsonNode, m_sOut);
}

void AsicControl::GetCurrentAd(const int x, const int y)
{
    QJsonObject jsonNode;
    jsonNode.insert("x", x);
    jsonNode.insert("y", y);
    m_tcpClient->sendCommand(MESSAGE_ID_SENSOR_AD_VALUE, MSG_ACTION_REQUEST, jsonNode, m_sOut);
}

void AsicControl::SetBigRingTemp(const float value)
{
    if(m_sensorPage.bigRingTempMax != value) {
        QJsonObject jsonNode;
        jsonNode.insert("value", value);
        m_tcpClient->sendCommand(MESSAGE_ID_OPERATE_BIG_RING_TEMP, MSG_ACTION_SET, jsonNode, m_sOut);
        m_sensorPage.bigRingTempMax = value;
    }
}

void AsicControl::SetNucTempMax(const float value)
{
    if(m_sensorPage.nucTempMax != value) {
        QJsonObject jsonNode;
        jsonNode.insert("value", value);
        m_tcpClient->sendCommand(MESSAGE_ID_SENSOR_NUC_TEMP_MAX, MSG_ACTION_SET, jsonNode, m_sOut);
        m_sensorPage.nucTempMax = value;
    }
}

void AsicControl::SetNucValue(const int value)
{
    if(m_sensorPage.nucTime != value) {
        QJsonObject jsonNode;
        jsonNode.insert("value", value);
        m_tcpClient->sendCommand(MESSAGE_ID_SENSOR_NUC_VALUE, MSG_ACTION_SET, jsonNode, m_sOut);
        m_sensorPage.nucTime = value;
    }
}

void AsicControl::SetBottomTempMax(const float value)
{
    if(m_sensorPage.localTemp != value) {
        QJsonObject jsonNode;
        jsonNode.insert("value", value);
        m_tcpClient->sendCommand(MESSAGE_ID_OPERATE_BOTTOM_TEMP, MSG_ACTION_SET, jsonNode, m_sOut);
        m_sensorPage.localTemp = value;
    }
}

void AsicControl::SetRasel(const int value)
{
    if(m_sensorPage.rasel != value) {
        QJsonObject jsonNode;
        jsonNode.insert("value", value);
        m_tcpClient->sendCommand(MESSAGE_ID_SENSOR_RASEL, MSG_ACTION_SET, jsonNode, m_sOut);
        m_sensorPage.rasel = value;
    }
}

void AsicControl::SetHssd(const int value)
{
    if(m_sensorPage.hssd != value) {
        QJsonObject jsonNode;
        jsonNode.insert("value", value);
        m_tcpClient->sendCommand(MESSAGE_ID_SENSOR_HSSD, MSG_ACTION_SET, jsonNode, m_sOut);
        m_sensorPage.hssd = value;
    }
}

void AsicControl::SetAutoCompTime(const int value)
{
    if(m_sensorPage.autoCompTime != value) {
        QJsonObject jsonNode;
        jsonNode.insert("value", value);
        m_tcpClient->sendCommand(MESSAGE_ID_OPERATE_AUTO_COMP_TIME, MSG_ACTION_SET, jsonNode, m_sOut);
        m_sensorPage.autoCompTime = value;
    }
}

void AsicControl::SetDebugMode(const int mode)
{
    QJsonObject jsonNode;
    jsonNode.insert("mode", mode);
    m_tcpClient->sendCommand(MESSAGE_ID_SYS_DEBUG_MODE, MSG_ACTION_SET, jsonNode, m_sOut);
}

bool AsicControl::SetBadPointList(const int mode, QString list)
{
    QJsonObject jsonNode;
    jsonNode.insert("mode", mode);
    jsonNode.insert("value", list);
    if (m_tcpClient->sendCommand(MESSAGE_ID_DATA_CONTROL_BAD_POINT_LIST, MSG_ACTION_SET, jsonNode, m_sOut)) {
        return resultConfirm(MESSAGE_ID_DATA_CONTROL_BAD_POINT_LIST, m_sOut);
    }
    return false;
}

bool AsicControl::GetBadPointsCount(int &count)
{
    if(m_tcpClient->sendCommand(MESSAGE_ID_ADJUST_IMG_BAD_POINTS_COUNT, MSG_ACTION_REQUEST, m_sOut)) {
        QJsonParseError jsonError;
        QJsonDocument document = QJsonDocument::fromJson(m_sOut.toStdString().data(),
                                                            &jsonError);
        if (document.isNull() || !document.isObject() || jsonError.error != QJsonParseError::NoError) {
            return false;
        }

        QJsonObject object = document.object();
        if (object.value("retcode").toInt() != 0 ||
                object.value("id").toInt() != MESSAGE_ID_ADJUST_IMG_BAD_POINTS_COUNT) {
            return false;
        }
        if (object.contains("content")) {
            QJsonObject content = object.value("content").toObject();
            if (content.contains("flash_count")) {
                count = content.value("flash_count").toInt();
                return true;
            }
        }
    }

    return false;
}

bool AsicControl::GetBadPointsList(QString &list)
{
    list.clear();
    if(m_tcpClient->sendCommand(MESSAGE_ID_DATA_GET_BAD_POINT_LIST, MSG_ACTION_REQUEST, m_sOut)) {
        QJsonParseError jsonError;
        QJsonDocument document = QJsonDocument::fromJson(m_sOut.toStdString().data(),
                                                            &jsonError);
        if (document.isNull() || !document.isObject() || jsonError.error != QJsonParseError::NoError) {
            return false;
        }

        QJsonObject object = document.object();
        if (object.value("retcode").toInt() != 0 ||
                object.value("id").toInt() != MESSAGE_ID_DATA_GET_BAD_POINT_LIST) {
            return false;
        }
        if (object.contains("content")) {
            QJsonObject content = object.value("content").toObject();
            if (content.contains("bad_point_list")) {
                list = content.value("bad_point_list").toString();
                return true;
            }
        }
    }

    return false;
}

bool AsicControl::SendCustomUart(QString value)
{
    QJsonObject jsonNode;
    jsonNode.insert("value", value);
    if (m_tcpClient->sendCommand(MESSAGE_ID_CUSTOM_UART_WRITE, MSG_ACTION_SET, jsonNode, m_sOut)) {
        return resultConfirm(MESSAGE_ID_CUSTOM_UART_WRITE, m_sOut);
    }

    return false;
}

bool AsicControl::resultConfirm(int msgId, QString sOut)
{
    QJsonParseError jsonError;
    QJsonDocument document = QJsonDocument::fromJson(sOut.toStdString().data(), &jsonError);
    if(document.isNull() || !document.isObject() || jsonError.error != QJsonParseError::NoError) {
        return false;
    }

    QJsonObject map = document.object();
    if (map.contains("retcode") && map.contains("id")) {
        if ((map.value("retcode").toInt() == 0) && (map.value("id").toInt() == msgId)) {
            return true;
        }
    }

    return false;
}

void AsicControl::SetIspAlgValue(const int id, QString data)
{
    QJsonObject jsonNode;
    jsonNode.insert("isp_id", id);
    jsonNode.insert("data", data);
    m_tcpClient->sendCommand(MESSAGE_ID_DATA_CONTROL_ISP_VALUE, MSG_ACTION_SET, jsonNode, m_sOut);
}

void AsicControl::GetIspAlgValue(const int id, const int len)
{
    QJsonObject jsonNode;
    jsonNode.insert("isp_id", id);
    jsonNode.insert("len", len);
    m_tcpClient->sendCommand(MESSAGE_ID_DATA_CONTROL_ISP_VALUE, MSG_ACTION_REQUEST, jsonNode, m_sOut);
}

void AsicControl::ChangeNvsInfo(NvsInfo info)
{
    QJsonObject jsonNode;
    jsonNode.insert("person_max", info.personMax);
    jsonNode.insert("person_min", info.personMin);
    jsonNode.insert("vehicle_max", info.vehicleMax);
    jsonNode.insert("vehicle_min", info.vehicleMin);
    m_tcpClient->sendCommand(MESSAGE_ID_NVS_INFO, MSG_ACTION_SET, jsonNode, m_sOut);
}

QString AsicControl::GetParamIn()
{
    QString sOut;
    m_tcpClient->sendCommand(MESSAGE_ID_DATA_CONTROL_CALIB_IN, MSG_ACTION_REQUEST, sOut);
    QJsonParseError jsonError;
    QJsonDocument document = QJsonDocument::fromJson(sOut.toStdString().data(),
                                                        &jsonError);
    if (document.isNull() || !document.isObject() || jsonError.error != QJsonParseError::NoError) {
        return false;
    }

    QJsonObject msg = document.object().value("content").toObject();

    QString paramIn;
    if(msg.contains("k_array") && msg.contains("dist_coff")) {
        QJsonArray kArray;
        kArray = msg.value("k_array").toArray();
        for(int i=0; i<kArray.size(); i++) {
            paramIn += QString::number(kArray[i].toDouble());
            paramIn += ",";
        }
        QJsonArray distArray;
        distArray = msg.value("dist_coff").toArray();
        for(int i=0; i<distArray.size(); i++) {
            paramIn += QString::number(distArray[i].toDouble());
            paramIn += ",";
        }
        paramIn += QString::number(msg.value("rms").toDouble());
        paramIn += ",";
        paramIn += QString::number(msg.value("optical_dist_rate").toDouble());
    }

    return paramIn;
}

bool AsicControl::SetParamIn(float fx, float fy, float cx, float cy, float rms, float coff[])
{
    QJsonArray kArray;
    kArray.append(fx);
    kArray.append(0);
    kArray.append(cx);
    kArray.append(0);
    kArray.append(fy);
    kArray.append(cy);
    kArray.append(0);
    kArray.append(0);
    kArray.append(1);

    QJsonArray distCoff;
    for(int i=0; i<5; i++)
    {
        distCoff.insert(i, coff[i]);
    }

    QJsonObject jsonNode;
    jsonNode.insert("k_array", kArray);
    jsonNode.insert("dist_coff", distCoff);
    jsonNode.insert("rms", rms);
    jsonNode.insert("optical_dist_rate", 3);

    if (m_tcpClient->sendCommand(MESSAGE_ID_DATA_CONTROL_CALIB_IN, MSG_ACTION_SET, jsonNode, m_sOut)) {
        return resultConfirm(MESSAGE_ID_DATA_CONTROL_CALIB_IN, m_sOut);
    }

    return false;
}

bool AsicControl::ClearAgeErrorCode()
{
    QString sOut;
    m_tcpClient->sendCommand(MESSAGE_ID_CLEAR_AGE_ERROR_CODE, MSG_ACTION_SET, sOut);
    return true;
}

bool AsicControl::GetAgeErrorCode(int& errorCode)
{
    QString sOut;
    m_tcpClient->sendCommand(MESSAGE_ID_GET_AGE_ERROR_CODE, MSG_ACTION_REQUEST, sOut);

    QJsonParseError jsonError;
    QJsonDocument document = QJsonDocument::fromJson(sOut.toStdString().data(),
                                                        &jsonError);
    if (document.isNull() || !document.isObject() || jsonError.error != QJsonParseError::NoError) {
        return false;
    }

    QJsonObject msg = document.object().value("content").toObject();
    // 0未老化 1老化完成 2摄像头异常 3图像异常
    if(msg.contains("age_error_code")) {
        errorCode = msg.value("age_error_code").toInt();
    }

    return true;
}

bool AsicControl::ClearSensorErrorCode()
{
    QString sOut;
    m_tcpClient->sendCommand(MESSAGE_ID_CLEAR_SENSOR_ERROR_CODE, MSG_ACTION_SET, sOut);
    return true;
}

bool AsicControl::GetSensorErrorCode(int &errorCode)
{
    QString sOut;
    m_tcpClient->sendCommand(MESSAGE_ID_GET_SENSOR_ERROR_CODE, MSG_ACTION_REQUEST, sOut);

    QJsonParseError jsonError;
    QJsonDocument document = QJsonDocument::fromJson(sOut.toStdString().data(),
                                                        &jsonError);
    if (document.isNull() || !document.isObject() || jsonError.error != QJsonParseError::NoError) {
        return false;
    }

    QJsonObject msg = document.object().value("content").toObject();
    // 0无错误码，其他错误码看协议附录表
    if(msg.contains("sensor_error_code")) {
        errorCode = msg.value("sensor_error_code").toInt();
    }

    return true;
}

