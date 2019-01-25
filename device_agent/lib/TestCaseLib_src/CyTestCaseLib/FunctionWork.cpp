#include "stdafx.h"
#include "WorkManager.h"
#include "FunctionDefine.h"

CWorkManager mgr;



// ��ʼ��
extern "C" TESTCASE_DLL_API int TestCase_Init()
{
	int nResult = mgr.TestCaseLibInit();
	return nResult;
}

// ע��
extern "C" TESTCASE_DLL_API int TestCase_RegisterTargetProc_Ex(const u8* pu8ProcName, const u8* pu8ProcIpAddr)
{
	int nResult = mgr.TestCaseLibRegisterProc((char*)pu8ProcName,(char*)pu8ProcIpAddr,-1);
	return nResult;
}

// ע��
extern "C" TESTCASE_DLL_API int TestCase_RegisterTargetProc(const u8* pu8ProcName, const u8* pu8ProcIpAddr)
{
	// ����_Ex�ģ������Ǳ��ò���Ex��
	// ����EX���Ƿ���ע�ᣬ��EX����ֻע������ʹ����
	int nResult = mgr.TestCaseLibRegisterProc((char*)pu8ProcName,(char*)pu8ProcIpAddr,-1);
	return nResult;
}

// ȡ��ע��
extern "C" TESTCASE_DLL_API int TestCase_UnregisterTargetProc(const u8* pu8ProcName, const u8* pu8ProcIpAddr)
{
	int nResult = mgr.TestCaseLibUnRegisterProc((char*)pu8ProcName,(char*)pu8ProcIpAddr,-1);
	return nResult;
}

// ע�ᡪ������pid����
// ��ΪҪ����������������Ϊ_Pid
extern "C" TESTCASE_DLL_API int TestCase_RegisterTargetProc_Pid(const u8* pu8ProcName, const u8* pu8ProcIpAddr, u32 u32Pid,s32 s32SlotNum, s32 s32ProcId)
{
	// ����_Ex�ģ������Ǳ��ò���Ex��
	// ����EX���Ƿ���ע�ᣬ��EX����ֻע������ʹ����
	int nResult = mgr.TestCaseLibRegisterProc((char*)pu8ProcName,(char*)pu8ProcIpAddr,u32Pid,s32SlotNum, s32ProcId);
	return nResult;
}

// ȡ��ע�ᡪ������pid����
extern "C" TESTCASE_DLL_API int TestCase_UnregisterTargetProc_Pid(const u8* pu8ProcName, const u8* pu8ProcIpAddr,u32 u32Pid)
{
	int nResult = mgr.TestCaseLibUnRegisterProc((char*)pu8ProcName,(char*)pu8ProcIpAddr,u32Pid);
	return nResult;
}

// ִ������
extern "C" TESTCASE_DLL_API int TestCase_ExcuteCmd(const u8 *pu8ProcName, const u8 *pu8Funcname,STRU_TESTCASE_EXECUTECMD_PARAS *pstruParas,u32 u32ParaNum,s32 *ps32RetVal)
{
	unsigned int u32SymbolAddr = 0;
	int nResult = mgr.TestCaseLibExcuteCmd((char*)pu8ProcName,(char*)pu8Funcname,pstruParas,u32ParaNum,ps32RetVal,&u32SymbolAddr);
	return nResult;
}

// EX���Ƕ�����ṩһ��������ַ
// 2018.6.4 ִ��������һ���ݣ����е��������pu32SymbolAddr �����ܻ�վ64λ��Ӱ��
// ���������ps32RetVal��pu32SymbolAddr��δ����ʹ��
// ���Դ��ڽ����������RSP1���з�����������ͬ����
extern "C" TESTCASE_DLL_API int TestCase_ExcuteCmd_Ex(const u8 *pu8ProcName, const u8 *pu8Funcname,STRU_TESTCASE_EXECUTECMD_PARAS *pstruParas,u32 u32ParaNum,s32 *ps32RetVal,u32 *pu32SymbolAddr)
{
	int nResult = mgr.TestCaseLibExcuteCmd((char*)pu8ProcName,(char*)pu8Funcname,pstruParas,u32ParaNum,ps32RetVal,pu32SymbolAddr);
	return nResult;
}

// �������ȼ�
extern "C" TESTCASE_DLL_API int TestCase_SetExecuteTaskPrio(const u8 *pu8ProcName, u32 u32Prio)
{
	int nResult = mgr.TestCaseLibSetTaskPrio((char*)pu8ProcName,u32Prio);
	return nResult;
}

// �������ʱʱ��
extern "C" TESTCASE_DLL_API int TestCase_SetCmdTimeOut(u32 u32CmdTimeOutSec)
{
	int nResult = mgr.TestCaseLibSetCmdTimeOut(u32CmdTimeOutSec);//��λ���룬����RSP1��RSP2�ĵȴ�ʱ��
	return nResult;
}

// ��λ����
extern "C" TESTCASE_DLL_API int TestCase_RestartExecuteTask(const u8 *pu8ProcName)
{
	int nResult = mgr.TestCaseLibRestartExecuteTask((char*)pu8ProcName);
	return nResult;
}


/* �����ļ� */
extern "C" TESTCASE_DLL_API int TestCase_DownloadFile(const u8* pu8ProcName, 
										   const u8* pu8PCFilePath, 
										   const u8* pu8BoardFilePath, 
										   s32 s32Load,s32 s32Inflate)
{
	int nResult = mgr.TestCaseLibDownloadFile((char*)pu8ProcName,(char*)pu8PCFilePath,(char*)pu8BoardFilePath,s32Load,s32Inflate);
	return nResult;
}
/* �ϴ��ļ� */
extern "C" TESTCASE_DLL_API int TestCase_UploadFile(const u8* pu8ProcName, 
										 const u8* pu8PCFilePath, 
										 const u8* pu8BoardFilePath)
{
	int nResult = mgr.TestCaseLibUploadFile((char*)pu8ProcName,(char*)pu8PCFilePath,(char*)pu8BoardFilePath);
	return nResult;
}

//ע������������
extern "C" TESTCASE_DLL_API int TestCase_RegisterOutputFunc(TESTCASE_OUTPUT_FUNC pTestCaseOutputFunc)
{
	int nResult = mgr.TestCaseLibRegisterOutputFunc(pTestCaseOutputFunc);
	return nResult;
}

// ��ʱ�ӿڣ�����Ӧ��û��D�̵����
// ��Ҫ��TestCase_Init֮ǰ���ã�������־��¼����
// ĩβ��Ҫ����"\\"
extern "C" TESTCASE_DLL_API void TestCaseConfig_TempfilePath(char* csTempfilePath)
{
	mgr.TestCaseConfigTempfilePath(csTempfilePath);
}

// ����FTP�ϴ����س�ʱʱ��
extern "C" TESTCASE_DLL_API int TestCase_SetFtpTimeOut(u32 u32CmdTimeOutSec)
{
	int nResult = mgr.TestCaseLibSetFtpTimeOut(u32CmdTimeOutSec);//��λ���룬����FTP�ϴ����صĵȴ�ʱ��
	return nResult;
}