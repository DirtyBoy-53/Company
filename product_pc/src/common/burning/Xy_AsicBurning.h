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
	/* �ӿ����ƣ�checkVersion
	** �ӿ�˵������ϵͳ��ʼ��֮ǰ���е��ã�������֤�����Ƿ�Ϊ��Ȩ����
	**			��ע�⡿�̲߳���ȫ�����߳���������⡣
	** ����˵����[IN]com - const char*
	**			 �����������Ʒ��¼���Զ�Ӧ�Ĵ��ں�
	** �������ͣ�bool
	**			 true - ��ʼOK����֮Ϊ false
	*/
	virtual bool checkVersion() = 0;

	/* �ӿ����ƣ�xyInitialize
	** �ӿ�˵������ϵͳ��ʼ��֮ǰ���е��ã�����ȷ���������Ի����Ƿ�׼����ɡ�
	** ����˵����[IN]com - const char*
	**			 �����������Ʒ��¼���Զ�Ӧ�Ĵ��ں�
	** �������ͣ�bool
	**			 true - ��ʼOK����֮Ϊ false
	*/
	virtual bool xyInitialize(const char* com) = 0;


	/* �ӿ����ƣ�xyDeInitial
	** �ӿ�˵�������˳�ϵͳʱ���е��ã��ͷ������Դ��
	** �������ͣ�bool
	*/
	virtual bool xyDeInitial() = 0;

	/* �ӿ����ƣ�xyStartBurning
	** �ӿ�˵������ÿ����¼����ʱ���ã�ϸ��Ϊ����¼��CRCУ�飬SNд����ȶԣ��汾�űȶԡ�
	**			 ����֮ǰ��Ҫ��ǰ�����ϵ�ص������������ִ��ʧ�ܡ�
	** ����˵����[IN]sn			- const char*	��ǰ��Ҫд�������SN
	**			 [IN]version	- const char*	��ǰ��¼�İ汾��
	**			 [IN]filepath	- const char*	��ǰ��¼�ļ������ļ���
	** �������ͣ�bool
	**			 true - ��ʼOK����֮Ϊ false
	*/
	virtual bool xyStartBurning(const char* sn, const char* version, const char* filepath) = 0;


	/* �ӿ����ƣ�xyStop
	** �ӿ�˵������δʵ��
	*/
	virtual bool xyStop() = 0;

	/* �ӿ����ƣ�xySetPowerOnOffPtr
	** �ӿ�˵�������µ�ص��������ýӿڣ���Ҫ�û�ʵ�֣���������ָ��ͨ������������ݸ����ʹ��
	** ����˵����[IN]func - std::function<bool(bool)>
	**			 bool���Ͳ���Ϊtrue��ʾ�ϵ磬false��ʾ�µ�
	** �������ͣ�bool
	**			 true - �ϵ�ִ��OK
	*/
	virtual bool xySetPowerOnOffPtr(std::function<bool(bool)> func) = 0;

	/* �ӿ����ƣ�xySetProcessLogPtr
	** �ӿ�˵�������Թ��̴�ӡ�ص��������ýӿڣ���Ҫ�û�ʵ�֣���������ָ��ͨ������������ݸ����ʹ��
	** ����˵����[IN]func - std::function<void(char*)>
	**			 char* ���Ͳ���Ϊ����������ļ�ʱ����log
	** �������ͣ�void
	*/
	virtual bool xySetProcessLogPtr(std::function<void(char*)> func) = 0; 

	/* �ӿ����ƣ�xySetCheckVersionFlag[�ڲ�ʹ��]
	** �ӿ�˵�������Թ����Ƿ���Ҫִ��дSN���ȶ԰汾�Ĳ���
	** ����˵����[IN]flag - bool
	**			 true ִ�а汾�ȶԺ�SNд�����
	**			 false ��ִ��
	** �������ͣ�void
	*/
	virtual void xySetCheckVersionFlag(bool flag) = 0;

	/* �ӿ����ƣ�xyGetLastError
	** �ӿ�˵������ȡ���Խ��
	** ����˵����[OUT]errString std::string
	**			 ���ؾ���Ĵ�����Ϣ
	** �������ͣ�int ���ؾ���Ĵ������
	*/
	virtual int xyGetLastError(std::string& errString) = 0;

	/* �ӿ����ƣ�xySetLogPath
	** �ӿ�˵������δʵ��
	*/
	virtual void xySetLogPath(char * path) = 0;

	/* �ӿ����ƣ�xyRelease
	** �ӿ�˵����ϵͳ�˳�֮ǰ���ͷ�ϵͳ�ӿ�ʱʹ�ã������getXyObject�ӿ�ʹ��
	*/
	virtual void xyRelease() = 0;
};

#endif