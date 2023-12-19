#ifndef INTERFACE_CONSTDEF_H
#define INTERFACE_CONSTDEF_H

#include <QObject>
#include <QByteArray>

/**
 * @brief The InterfaceErrorCode enum 接口函数返回的错误码
 */
enum InterfaceErrorCode {
    INTERFACE_ERROR_CODE_SUC = 0,
    INTERFACE_ERROR_CODE_FAILD = 1,
};

/**
 * @brief The ResultInfo struct 数据上报的节点结构体
 * key = result, 是上报工作总结果, value是描述, notes是错误码
 * key = tips, 是修改框架提示, value是提示文字, notes是背景颜色
 * ......
 * key = 其他, 是上报momo的值
 */
struct ResultInfo {
    QString key;
    QString value;
    QString expect; // 期望值
    int result = 0; // 结果
    int notes = 0;  // 备注信息
};

/**
 * @brief The PlatformInfo struct 工装平台信息结构体
 * 例如通知框架arm版本号、探测器sn等
 * key = "sn_code"; desc = "探测器SN号"; value = sn号;
 * key = "arm_version"; desc = "arm版本号"; value = 版本号;
 * key = "version"; desc = "arm app版本号"; value = 版本号;
 * .....
 */
struct PlatformInfo {
    QString key;   //
    QString value;
    QString desc;  // key的描述
};

/**
 * @brief The SocketHandleIndex enum 治具号
 */
enum SocketHandleIndex {
    SOCKET_HANDLE_1 = 0,
    SOCKET_HANDLE_2,
    SOCKET_HANDLE_3,
    SOCKET_HANDLE_4
};

/**
 * @brief The CheckItemType enum 检测结果类型
 */
enum CheckItemType {
    CHECK_TYPE_STRING = 0, // 字符串比较
    CHECK_TYPE_VALUE,  // 最小值、最大值比较
    CHECK_TYPE_PASS_FAIL,  // 直接执行
};

enum CheckWorkState {
    CHECK_WORK_TESTING = 0,
    CHECK_WORK_PASS,
    CHECK_WORK_FAIL
};


/**
 * 工装平台信息回调函数体，支持多治具上传
 */
typedef QMap<int, QList<PlatformInfo>> MapPlatformInfo;
typedef bool(*PlatformInfoCallback)(MapPlatformInfo info, void* parent);

/**
 * momo数据上报回调函数体
 */
typedef bool(*ReportResultCallback)(QString sCodeId, QList<ResultInfo> result, void* parent);
typedef bool(*StandResultCallback)(QString sCodeId, QList<ResultInfo> &result, void* parent);
typedef bool(*EnterProduce)(QString sId, QList<ResultInfo> &result, void* parent);
typedef bool(*OutProduce)(QString sId,QList<ResultInfo> &result, void* parent);
typedef bool(*DownloadData)(QString &sInToken, QByteArray &arrayData);
typedef bool(*UploadData)(QString &sOutToken, QString fileName, QByteArray &arrayData);
#endif // INTERFACE_CONSTDEF_H
