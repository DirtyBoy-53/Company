#ifndef _XY_ASIC_BURNING_ASIC_BURNING_H_
#define _XY_ASIC_BURNING_ASIC_BURNING_H_

#ifdef XY_AUTH_API
#define XY_API extern "C"  __declspec(dllexport)
#else
#define XY_API extern "C"  __declspec(dllimport)
#endif

#include <functional>

const char IID_XY_ASICBURNING[] = "{9A3946C0-8C77-4146-9ABD-FEF57D7CBE5C}";
#define DLLNAME_XY_ASIC_BURNING	"Xy_AsicBurning.dll"
#define APINAME_XY_ASIC_BURNING "getXyObject"
#define interface struct

XY_API bool getXyObject(const char* iid, void** p);

/* error code, error message
-1, "load datalink dll failed"
-2, "serial exception"
-2, "serial initial failed"
-3, "md5 check failed"
-4, "burning file failed"
-5, "power on failed"
-6, "restart dut failed"
-7, "re-burning dut failed"
-8, "re-burning restart dut failed"
-9, "crc compare failed"
-10, "crc command failed"
-11, "write sn failed"
-12, "read sn failed"
-13, "compare sn failed"
-14, "read version failed"
-15, "compare version failed"
*/
interface IXy_AsicBurning
{
	/* 接口名称：checkVersion
	** 接口说明：在系统初始化之前进行调用，用来验证环境是否为授权环境
	**			【注意】线程不安全，多线程需枷锁互斥。
	** 参数说明：[IN]com - const char*
	**			 输入参数，产品烧录调试对应的串口号
	** 返回类型：bool
	**			 true - 初始OK，反之为 false
	*/
	virtual bool checkVersion() = 0;

	/* 接口名称：xyInitialize
	** 接口说明：在系统初始化之前进行调用，用来确定环境测试环境是否准备完成。
	** 参数说明：[IN]com - const char*
	**			 输入参数，产品烧录调试对应的串口号
	** 返回类型：bool
	**			 true - 初始OK，反之为 false
	*/
	virtual bool xyInitialize(const char* com) = 0;


	/* 接口名称：xyDeInitial
	** 接口说明：在退出系统时进行调用，释放组件资源。
	** 返回类型：bool
	*/
	virtual bool xyDeInitial() = 0;

	/* 接口名称：xyStartBurning
	** 接口说明：在每轮烧录测试时调用，细分为：烧录，CRC校验，SN写入与比对，版本号比对。
	**			 测试之前需要提前传入上电回调函数，否则会执行失败。
	** 参数说明：[IN]sn			- const char*	当前需要写入的整机SN
	**			 [IN]version	- const char*	当前烧录的版本号
	**			 [IN]filepath	- const char*	当前烧录文件所在文件夹
	** 返回类型：bool
	**			 true - 初始OK，反之为 false
	*/
	virtual bool xyStartBurning(const char* sn, const char* version, const char* filepath) = 0;


	/* 接口名称：xyStop
	** 接口说明：暂未实现
	*/
	virtual bool xyStop() = 0;

	/* 接口名称：xySetPowerOnOffPtr
	** 接口说明：上下电回调函数设置接口，需要用户实现，并将函数指针通过这个函数传递给组件使用
	** 参数说明：[IN]func - std::function<bool(bool)>
	**			 bool类型参数为true表示上电，false表示下电
	** 返回类型：bool
	**			 true - 上电执行OK
	*/
	virtual bool xySetPowerOnOffPtr(std::function<bool(bool)> func) = 0;

	/* 接口名称：xySetProcessLogPtr
	** 接口说明：测试过程打印回调函数设置接口，需要用户实现，并将函数指针通过这个函数传递给组件使用
	** 参数说明：[IN]func - std::function<void(char*)>
	**			 char* 类型参数为组件传出来的即时过程log
	** 返回类型：void
	*/
	virtual bool xySetProcessLogPtr(std::function<void(char*)> func) = 0; 

	/* 接口名称：xySetCheckVersionFlag[内部使用]
	** 接口说明：测试过程是否需要执行写SN，比对版本的操作
	** 参数说明：[IN]flag - bool
	**			 true 执行版本比对和SN写入操作
	**			 false 不执行
	** 返回类型：void
	*/
	virtual void xySetCheckVersionFlag(bool flag) = 0;

	/* 接口名称：xyGetLastError
	** 接口说明：获取测试结果
	** 参数说明：[OUT]errString std::string
	**			 返回具体的错误信息
	** 返回类型：int 返回具体的错误代码
	*/
	virtual int xyGetLastError(std::string& errString) = 0;

	/* 接口名称：xySetLogPath
	** 接口说明：暂未实现
	*/
	virtual void xySetLogPath(char * path) = 0;

	/* 接口名称：xyRelease
	** 接口说明：系统退出之前，释放系统接口时使用，相对于getXyObject接口使用
	*/
	virtual void xyRelease() = 0;
};

#endif