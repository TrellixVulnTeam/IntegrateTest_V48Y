/*******************************************************************************
* COPYRIGHT DaTang Mobile Communications Equipment CO.,LTD
********************************************************************************
* �ļ�����:  TestCaseFuncs.h
* ��    ��:  ���ڲ��Խű��ĺ����ӿ�
* ��    ��:  V0.1
* ��д����:  2009/01/17
* ˵    ��:
* �޸���ʷ:
* 001 2009/01/17  ����ǿ �ļ�����
*******************************************************************************/


/******************************** ͷ�ļ�������ͷ ******************************/
#ifndef TESTCASE_FUNCS_H
#define TESTCASE_FUNCS_H
/******************************** �����ļ����� ********************************/
#define TESTCASE_SCRIPT_ERRBASE                                    -100
#define TESTCASE_FILETRANS_ERRBASE                                 -200
#define TESTCASE_EXECUTECMD_ERRBASE                                -300

/******************************** ��ͳ������� ********************************/

#define DllImport   __declspec( dllimport )
#define DllExport   __declspec( dllexport )


/* �Զ����������� */
typedef unsigned int            u32;
/* �Զ����������� */
typedef unsigned short          u16;
/* �Զ����������� */
typedef unsigned char           u8;
/* �Զ����������� */
typedef int                     s32;
/* �Զ����������� */
typedef signed short            s16;
/* �Զ����������� */
typedef signed char             s8;
/* �Զ����������� */
typedef volatile unsigned short vu16;

#ifdef __cplusplus
extern "C" { 
#endif

typedef void (*TESTCASE_OUTPUT_FUNC)(u8* pu8OutputMsg, s32 s32OutputMsgType, s32 s32DebugLevel);
typedef void (*TESTCASE_ALERT_FUNC)(u8* pu8AlertInfo);
typedef s32 (*TESTCASE_CONFIRM_FUNC)(u8* pu8ConfirmInfo);

#define TESTCASE_EXECUTECMD_PARATYPE_NUMBER        1
#define TESTCASE_EXECUTECMD_PARATYPE_BUFFER        2

typedef struct tag_STRU_TESTCASE_EXECUTECMD_PARAS
{ 
	s32  s32ParaType; /* �������ͣ���ֵ/���ݻ����� */ 
	s32  s32ParaVal;  /* ��ֵ/���ݻ���������  */ 
	void *pParaBuf;   /* ���ݻ������׵�ַ */
}STRU_TESTCASE_EXECUTECMD_PARAS;


/******************************** �������� ************************************/
/* ���ʼ������ */
extern DllImport s32 TestCase_Init();
/* ��½ */
extern DllImport s32 TestCase_RegisterTargetProc(const u8* pu8ProcName, 
												 const u8* pu8ProcIpAddr);
extern DllImport s32 TestCase_RegisterTargetProc_Pid(const u8* pu8ProcName, 
												 const u8* pu8ProcIpAddr,
												 int s32Pid,
												 s32 s32SlotNum=0, 
												 s32 s32ProcId=0);
/* ��½ */
extern DllImport s32 TestCase_RegisterTargetProc_Ex(const u8* pu8ProcName, 
												    const u8* pu8ProcIpAddr);

/* chaiye 2017-10-18 �����ӿڣ���忨��ʹ�ö˿ڲ�ͬ�����ǵ������ԣ����Ӵ˽ӿ� */
/* �˿�Ĭ��10021,������BPOK��SCTF ���˽ӿڵ����Կ�ʹ��TestCase_RegisterTargetProc_Ex*/
/* AAUʹ��10001����ʹ�ô˽ӿ� */
extern DllImport s32 TestCase_RegisterTargetProc_Ex_CfgPort(const u8* pu8ProcName, 
													const u8* pu8ProcIpAddr,int nPort);
/* ��½Udp */
extern DllImport s32 TestCase_RegisterTargetProc_Udp(const u8* pu8ProcName, 
													 const u8* pu8ProcIpAddr);

/* ����½TestCase,��"OldStyle"���� */
extern DllImport s32 TestCase_RegisterTargetProc_Ex2(const u8* pu8ProcName,
													 const u8* pu8ProcIpAddr);

/* �����������Դ��� */
extern DllImport s32 TestCase_SetRegisterTargetProcRetryTimes(u32 u32Times);

/* �˳� */
extern DllImport s32 TestCase_UnregisterTargetProc(const u8* pu8ProcName, 
												   const u8* pu8ProcIpAddr);
extern DllImport s32 TestCase_UnregisterTargetProc_Pid(const u8* pu8ProcName, 
												   const u8* pu8ProcIpAddr,
												   int s32Pid);
/* �����ļ� */
extern DllImport s32 TestCase_DownloadFile(const u8* pu8ProcName, 
										   const u8* pu8PCFilePath, 
										   const u8* pu8BoardFilePath, 
										   s32 s32Load,s32 s32Inflate);
/* �ϴ��ļ� */
extern DllImport s32 TestCase_UploadFile(const u8* pu8ProcName, 
										 const u8* pu8PCFilePath, 
										 const u8* pu8BoardFilePath);
/* ������־·�� old */
extern DllImport s32 TestCase_SetReportFile(const u8* pu8ReportFile);
/* �����ѱ���ű� old */
extern DllImport s32 TestCase_SetCompiledScriptFile(const u8* pu8CompiledScriptFile);
/* ����Դ�ű� old */
extern DllImport s32 TestCase_SetSrcScriptFile(const u8* pu8SrcScriptFile);
/* ִ�����в������� old */
extern DllImport s32 TestCase_RunTestCaseScript(void);
/* ִ��һ��������������������ִ�н�� old */
extern DllImport s32 TestCase_RunOneTestCase(const u8* pu8TestCaseName);

/* ע������������ */
extern DllImport s32 TestCase_RegisterOutputFunc(TESTCASE_OUTPUT_FUNC pTestCaseOutputFunc);
/* ע��Alert���� */
extern DllImport s32 TestCase_RegisterAlertFunc(TESTCASE_ALERT_FUNC pTestCaseAlertFunc);
/* ע��Confirm���� */
extern DllImport s32 TestCase_RegisterConfirmFunc(TESTCASE_CONFIRM_FUNC pTestCaseConfirmFunc);
/* ��������ִ��ģʽ old */
extern DllImport s32 TestCase_EnableFilterMode(void);
/* �رղ���ִ��ģʽ old */
extern DllImport s32 TestCase_DisableFilterMode(void);
/* ���ò���ִ��ģʽ���˹��� old */
extern DllImport s32 TestCase_ResetFilter(void);
/* ��Ӳ�������������ִ��ģʽ���� old */
extern DllImport s32 TestCase_ActiveTestCase(const u8* pstrTestCase);
/* ��ȡ�ѱ���ű��ļ��еĲ����������� old */
extern DllImport s32 TestCase_GetTestCaseName(u32 u32StartLine, 
											  u8* pstrTestCaseName, 
											  u32 u32BufLen);
/* ���������ѱ���Ľű��ļ� old */
extern DllImport s32 TestCase_RegenerateCompiledScriptFile(const u8* pu8FileName);
/* �������ؽű� old */
extern DllImport s32 TestCase_AppendCompiledTestCaseScriptFile(const u8* pu8FileName);
/* ���¼��ؽű� old */
extern DllImport s32 TestCase_ReloadCompiledTestCaseScriptFile(const u8* pu8FileName);

/* ִ�в������� */
extern DllImport s32 TestCase_ExcuteCmd(const u8 *pu8ProcName, 
										const u8 *pu8Funcname, 
										STRU_TESTCASE_EXECUTECMD_PARAS *pstruParas, 
										u32 u32ParaNum, 
										s32 *ps32RetVal);

extern DllImport s32 TestCase_ExcuteCmd_Ex(const u8 *pu8ProcName, 
										   const u8 *pu8Funcname, 
										   STRU_TESTCASE_EXECUTECMD_PARAS *pstruParas, 
										   u32 u32ParaNum, 
										   s32 *ps32RetVal,
										   u32 *pu32SymbolAddr);

extern DllImport s32 TestCase_ExecuteCmd_Addr(const u8 *pu8ProcName,
											  u32 u32FuncAddr,
											  STRU_TESTCASE_EXECUTECMD_PARAS *pstruParas, 
											  u32 u32ParaNum, 
											  s32 *ps32RetVal);

/* ִ�в��������ϵ�� */
extern DllImport s32 TestCase_ExecuteCmd_Series(s32 s32Series,
												const u8 *pu8ProcName,
												const u8 *pu8Funcname,
												u32 u32FuncAddr,
												STRU_TESTCASE_EXECUTECMD_PARAS *pstruParas, 
												u32 u32ParaNum, 
												s32 *ps32RetVal);

/* ��λ���� */
extern DllImport s32 TestCase_RestartExecuteTask(const u8 *pu8ProcName);

/* �����������ȼ� */
extern DllImport s32 TestCase_SetExecuteTaskPrio(const u8 *pu8ProcName, u32 u32Prio);

/* �������ʱʱ�� */
extern DllImport s32 TestCase_SetCmdTimeOut(u32 u32CmdTimeOutSec);

/* ��ȡ��ǰ����״̬ */
extern DllImport s32 TestCase_GetConnState(const u8 *pu8ProcName);

/* �������־��������Ϣ */
extern DllImport s32 TestCase_LogMsg(u8* pu8OutputMsg);

/* �ȴ��忨����Ϣ */
extern DllImport s32 TestCase_WaitRemoteData(u32 u32BufferSize, u32 u32SecTimeOut, u8* pu8ProcFrom, u8* pu8Data, u32* pu32ReceivedSize);

#ifdef __cplusplus
}
#endif

/******************************** ͷ�ļ�������β ******************************/

#endif /* TESTCASE_FUNCS_H */
/******************************** ͷ�ļ����� **********************************/









