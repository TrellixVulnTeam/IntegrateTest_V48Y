#include "StdAfx.h"

#include ".\workmanager.h"
#include "ErrorCode.h"
#include "RecvPacketModel.h"
#include "iostream"
#include "string"
#include "ws2tcpip.h"


//#define CY_FOR_VERSION "V3.0.1"  //chaiye 2017.11.12
//#define CY_FOR_VERSION "V3.0.2"  //chaiye 2018.02.05
// 1�����������ṩ��һ����������ʱ��������ӿڣ�
// 2���޸���֮ǰ�ж��ļ���>64��������⣬����ʱתΪ�̶���temp�ļ��������ᳬ��64�����Դ��жϷ������ԡ�

//#define CY_FOR_VERSION "V3.1.0"  //chaiye 2018.05.09
// 1.���������и��˸���PID��¼���㷨  10001+pid*10,���Բ�����SCTF��BPOK��������10021�����ƾ��Ƕ��̴߳������Ľ������

//#define CY_FOR_VERSION "V3.1.1" // chaiye 2018.6.4 
// Ϊ�˲����·���ȡ��־���������Cmdִ�еĴ�ӡ
// ���Ǹ�����㷢�Ĳ��԰汾����ӡbuffer��ʵ��û�б�Ҫ��ȡ��,��Ϊ�ܶ��������PC��ַ����ӡҲ�����룬���������ᣬ

#define CY_FOR_VERSION "V3.2.0" //chaiye 2018.6.5  ����5G��64λ�汾
// ���������������ʱ����һ�����԰汾��û�İ汾�ţ�����V3.1.0��֮��ͳһ��V3.2.0


CWorkManager::CWorkManager(void)
{
	m_pLog = NULL;
	m_hRecvHandle = NULL;
	m_strTempFilePath = "D:\\temp\\"; // ��ʼֵ�����Ե��ýӿڽ�������
	FindUseDisk();//��m_strTempFilePath���д������û�����ݣ�����Ȼʹ��D:\\temp\\����Ȼ����tempʵ�ʲ���ʹ��
	m_strTransName="tempfile";

	InitializeCriticalSection(&m_cs); 
}

CWorkManager::~CWorkManager(void)
{
	DeleteCriticalSection(&m_cs); 
}

// ��ʼ��
int CWorkManager::TestCaseLibInit()
{
	LogInit();
	CString strInfo("");
	strInfo.Format("CALLINTERFACE TestCase_Init (%s)",CY_FOR_VERSION);
	Log(LOGINFO,strInfo);
	if(TESTCASE_SUCCESS != SockInit())
	{
		return INIT_SOCKET_FAIL;
	}
	
	if(!MemberInit())
	{
		return INT_OTHER_FAIL;
	}

	m_hSigRspEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
	m_hTaskRspEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
	m_hRsp1Event = CreateEvent(NULL,FALSE,FALSE,NULL);
	m_hRsp2Event = CreateEvent(NULL,FALSE,FALSE,NULL);
	m_hFileTransEvent = CreateEvent(NULL,FALSE,FALSE,NULL);

    m_nWaitTimeMSec = WAIT_RSP_TIMEOUT;// �����õ�Ĭ��ֵ��RSP1��RSP2֧���û��޸�
	m_nWaitTimeMSecForFtp = WAIT_RSP_DOWNLOAD; // Ĭ��10����
	m_nFileTransId = 3;// ����ԭ��

	g_pTestCaseOutputFunc = NULL;

	RecvThreadStart();

	return TESTCASE_SUCCESS;
}

// ע��
int CWorkManager::TestCaseLibRegisterProc(char* csProName,char* csIP,s32 s32Pid,s32 s32SlotNum, s32 s32ProcId)
{
	LogReset(); // �ڴ��ж�����־��С

	CString strInfo("");
	strInfo.Format("CALLINTERFACE TestCase_RegisterTargetProc(Ex) Proc(%s) IP(%s) PID(%d) SlotNum(%d) ProcId(%d)"
		,csProName,csIP,s32Pid,s32SlotNum,s32ProcId);
	Log(LOGINFO,strInfo);
	
	std::map<CString, CSockInfo>::iterator iter = m_NameAddrMap.find(csProName);
	if(iter != m_NameAddrMap.end())
	{
		Log(LOGERROR,"�������Ѿ���ʹ��");
		return REGISTER_NAMEINUSE_FAIL;
	}

	if(AddrAlreadyInUse(csIP,s32Pid))
	{
		Log(LOGERROR,"�˵�ַ�Ѿ���ʹ��");
		return REGISTER_IPINUSE_FAIL;
	}

	CSockInfo sockInfo;
	sockInfo.SetPid(s32Pid);
	sockInfo.strIp = csIP;
	// (1)����Server
	SOCKET clientSock = ClientConnect(csIP,sockInfo.nPort, s32SlotNum, s32ProcId);
	if(INVALID_SOCKET == clientSock)
	{
		return REGISTER_CONNECT_FAIL;
	}
	
	sockInfo.sock = clientSock;
	m_NameAddrMap[csProName] = sockInfo;

	CRecvPacketModel model;
	model.m_pClassLog = m_pLog;
	model.ResetModel();
	m_NameDataMap[csProName]= model;
	
	// ��2������¼ָ��ǲ��Ȼ�Ӧ
	SendMsg_LogOn(csProName);
	if (WAIT_TIMEOUT == WaitForSingleObject(m_hSigRspEvent, WAIT_RSP_TIMEOUT)) 
	{
		return ECMD_CTRL_RSP_TIMEOUT_ERROR; 
	}
	SendMsg_ServReq(csProName,TESTCASE_SERVICE_ID);
	if (WAIT_TIMEOUT == WaitForSingleObject(m_hSigRspEvent, WAIT_RSP_TIMEOUT)) 
	{
		return ECMD_CTRL_RSP_TIMEOUT_ERROR; 
	}
	SendMsg_ServReq(csProName,UAGT_FILE_TRANS_SERVICE_ID);
	if (WAIT_TIMEOUT == WaitForSingleObject(m_hSigRspEvent, WAIT_RSP_TIMEOUT))
	{
		return ECMD_CTRL_RSP_TIMEOUT_ERROR; 
	}
	SendMsg_ServReq(csProName,UAGT_CMD_EXECUTE_SERVICE_ID);
	if (WAIT_TIMEOUT == WaitForSingleObject(m_hSigRspEvent, WAIT_RSP_TIMEOUT)) 
	{
		return ECMD_CTRL_RSP_TIMEOUT_ERROR; 
	}

	return TESTCASE_SUCCESS;
}

// ȡ��ע��
int CWorkManager::TestCaseLibUnRegisterProc(char* csProName,char* csIP,s32 s32Pid)
{
	CString strInfo("");
	strInfo.Format("CALLINTERFACE TestCase_UnregisterTargetProc Proc(%s) IP(%s) PID(%d)",csProName,csIP,s32Pid);
	Log(LOGINFO,strInfo);	
	
    EnterCriticalSection(&m_cs);
	std::map<CString,CSockInfo>::iterator iterNameSock = m_NameAddrMap.find(csProName);
	if(iterNameSock == m_NameAddrMap.end())
	{
		LeaveCriticalSection(&m_cs);
		return TESTCASE_SUCCESS; // û�����ֵ�����ȡ��ע�ᣬ��Ϊ��Щ�˺���ע�����ȵ�ȡ��ע��
	}
	else
	{
		// �����ֵļ��IP�Ƿ�ƥ�䣬��ƥ�䲻�ܶϿ�����
		if(!IsInfoMatch(csProName,csIP,s32Pid))
		{
			LeaveCriticalSection(&m_cs);
			Log(LOGERROR,"��������Ϣ���ַ��Ϣ������");	
			return TESTCASE_FAIL;
		}

		SOCKET sock = m_NameAddrMap[csProName].sock;
		ClientClose(sock);
		m_NameAddrMap.erase(iterNameSock);
	}
	LeaveCriticalSection(&m_cs);

	std::map<CString,CRecvPacketModel>::iterator iterNameBuff = m_NameDataMap.find(csProName);
	if(iterNameBuff != m_NameDataMap.end())
	{
		m_NameDataMap.erase(iterNameBuff);
		TRACE("ERASE DATA\n");
	}
	else
	{
		return TESTCASE_FAIL; // �����ϲ���
	}

	return TESTCASE_SUCCESS;
}



/* ���
typedef struct tag_STRU_TESTCASE_EXECUTECMD_PARAS
{ 
	s32  s32ParaType; //�������ͣ���ֵ/���ݻ�����
	s32  s32ParaVal;  // ��ֵ/���ݻ���������
	void *pParaBuf;   // ���ݻ������׵�ַ
}STRU_TESTCASE_EXECUTECMD_PARAS;
*/

int CWorkManager::TestCaseLibExcuteCmd(char* csProName,char* csFunction,STRU_TESTCASE_EXECUTECMD_PARAS *pstruParas,int nParaNum,int *pnRetVal,unsigned int *pu32SymbolAddr)
{
	CString strInfo("");
	strInfo.Format("CALLINTERFACE TestCase_ExcuteCmd(Ex) Proc(%s) Func(%s) ParaNum(%d)",csProName,csFunction,nParaNum);
	Log(LOGINFO,strInfo);
	
	if(NULL == pnRetVal || NULL == pu32SymbolAddr)
	{
		Log(LOGERROR,"Return Point is NULL");
		return TESTCASE_FAIL;
	}

	// ����У��
	if(nParaNum > CMD_MAX_PARA_NUM)
	{
		return EXECUTECMD_PARA_TOOMANY;
	}

	int nCmdParaNum = nParaNum;
	if(nCmdParaNum < 0)
	{
		nCmdParaNum = 0;
	}

	CString strProName(csProName);
	CString strFunciton(csFunction);
	strProName = strProName.Trim();
	strFunciton = strFunciton.Trim();


	STRU_TESTCASE_EXECUTECMD_PARAS cmdParas[CMD_MAX_PARA_NUM];
	memset((char*)cmdParas,0,sizeof(STRU_TESTCASE_EXECUTECMD_PARAS) * CMD_MAX_PARA_NUM);

	// ���1 ָ��������
	if(nCmdParaNum>0)
	{
		if(NULL == pstruParas)
		{
			Log(LOGERROR,"Parameter number is not 0, But pstruParas is NULL!");
			return TESTCASE_FAIL;
		}
		
		memcpy((char*)cmdParas,(char*)pstruParas,sizeof(STRU_TESTCASE_EXECUTECMD_PARAS)*nParaNum);
		int nResult = SendMsg_ExecuteCmd(strProName,strFunciton,cmdParas,nCmdParaNum,pnRetVal,pu32SymbolAddr);
		return nResult;
	}

	// ���2 �޲���
	int nPos = strFunciton.Find(" ");
	if(-1 == nPos)
	{
		int nResult = SendMsg_ExecuteCmd(strProName,strFunciton,NULL,0,pnRetVal,pu32SymbolAddr);
		return nResult;
	}

	// ���3 �������Ͷ������д��һ����
	CString strParaAll = strFunciton.Mid(nPos+1,strFunciton.GetLength()-nPos);
	strFunciton = strFunciton.Left(nPos);
	nPos = strParaAll.Find(",");
	int paraIndex = 0;
	while(nPos > 0) //�������
	{
		CString strPara = strParaAll.Left(nPos);
		cmdParas[paraIndex].s32ParaType = TESTCASE_EXECUTECMD_PARATYPE_NUMBER;
		cmdParas[paraIndex].s32ParaVal =  strtoul(strPara.GetBuffer(),NULL,16);
		paraIndex++;
		strParaAll = strParaAll.Mid(nPos+1,strParaAll.GetLength()-nPos);
		nPos = strParaAll.Find(",");
	}
	cmdParas[paraIndex].s32ParaType = TESTCASE_EXECUTECMD_PARATYPE_NUMBER;
	cmdParas[paraIndex].s32ParaVal =  strtoul(strParaAll.GetBuffer(),NULL,16);
	nCmdParaNum = paraIndex+1;
	int nResult = SendMsg_ExecuteCmd(strProName,strFunciton,cmdParas,nCmdParaNum,pnRetVal,pu32SymbolAddr);
	return nResult;
}

int CWorkManager::TestCaseLibSetTaskPrio(char* csProName,int nPrio)
{
	CString strInfo("");
	strInfo.Format("CALLINTERFACE TestCase_SetExecuteTaskPrio Proc(%s) Prio(%d)",csProName,nPrio);
	Log(LOGINFO,strInfo);
	
	int nResult = SendMsg_SetTaskPrio(csProName,nPrio);
	if(sizeof(UAGT_CMD_SET_TASK_PRIO_REQ) != nResult)
	{
		strInfo.Format("Set Proc(%s) Task Priority(%d) Fail",csProName,nPrio);
		Log(LOGERROR,strInfo);
		return TESTCASE_FAIL;
	}
	
	if (WAIT_TIMEOUT == WaitForSingleObject(m_hTaskRspEvent, WAIT_RSP_TIMEOUT)) // �Ȼ�Ӧ������Ӧ���˲����κδ���
	{
		return ECMD_CTRL_RSP_TIMEOUT_ERROR;  // ��Ϣ��ɫ��cmdȫ��ff
	}
	return TESTCASE_SUCCESS;
}

int CWorkManager::TestCaseLibSetCmdTimeOut(u32 u32CmdTimeOutSec)
{
	CString strInfo("");
	strInfo.Format("CALLINTERFACE TestCase_SetCmdTimeOut SEC(%d)",u32CmdTimeOutSec);
	Log(LOGINFO,strInfo);
	m_nWaitTimeMSec = u32CmdTimeOutSec*1000;
	return TESTCASE_SUCCESS;
}

int CWorkManager::TestCaseLibSetFtpTimeOut(u32 u32CmdTimeOutSec)
{
	CString strInfo("");
	strInfo.Format("CALLINTERFACE TestCase_SetFtpTimeOut SEC(%d)",u32CmdTimeOutSec);
	Log(LOGINFO,strInfo);
	m_nWaitTimeMSecForFtp = u32CmdTimeOutSec*1000;
	return TESTCASE_SUCCESS;
}

int CWorkManager::TestCaseLibRestartExecuteTask(char* csProcName)
{
	// ��Restart��Ϣ������Ӧ����Ӧ��������
	CString strInfo("");
	strInfo.Format("CALLINTERFACE TestCase_RestartExecuteTask Proc(%s)",csProcName);
	Log(LOGINFO,strInfo);
	
	int nResult = SendMsg_RestartTaskReq(csProcName);
	if(sizeof(UAGT_CMD_SET_TASK_PRIO_REQ) != nResult)
	{
		strInfo.Format("Restart Proc(%s) Task Fail",csProcName);
		Log(LOGERROR,strInfo);
		return TESTCASE_FAIL;
	}

	if (WAIT_TIMEOUT == WaitForSingleObject(m_hTaskRspEvent, WAIT_RSP_TIMEOUT))
	{
		return ECMD_CTRL_RSP_TIMEOUT_ERROR;  
	}
	return TESTCASE_SUCCESS;
}

int CWorkManager::TestCaseLibDownloadFile(char* csProcName, char* csPCFilePath, char* csBoardFilePath, int nLoad,int nInflate)
{
	CString strInfo("");
	strInfo.Format("CALLINTERFACE TestCase_DownloadFile Proc(%s) PCFilePath(%s) BoardFilePath(%s) Load(%d) Inflate(%d)",
		csProcName,csPCFilePath,csBoardFilePath,nLoad,nInflate);
	Log(LOGINFO,strInfo);
	int nFileLen = 0;

	// ���-�ļ��Ƿ����
	SECURITY_ATTRIBUTES sa;
	sa.nLength              =   sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor =   NULL;
	sa.bInheritHandle       =   TRUE;
	HANDLE hFile = CreateFile((LPCTSTR)csPCFilePath,
		GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		&sa,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		strInfo.Format("File Not Exist %s",csPCFilePath);
		Log(LOGERROR,strInfo);
		return TESTCASE_FAIL;
	}
	nFileLen = GetFileSize(hFile, NULL);
	CloseHandle(hFile);

	//// ���-·���Ƿ񳬹�����
	//CString strLocalPath(csPCFilePath);
	//if (strLocalPath.GetLength()>64)
	//{
	//	strInfo.Format("File Length %d > 64",strLocalPath.GetLength());
	//	Log(LOGERROR,strInfo);
	//	return TESTCASE_FAIL;
	//}

	// ��������ļ�������D:\\Temp����Ϊtempfile��Ȼ������tempfile����ļ����忨��Ϊָ��������
	// ������Ϣ���ݵ�PC·������һ���ģ����Ȳ�����64�����Դ˴����ñ�����ע�͵�����Ĵ���
	CString strCmd("");
	if (nInflate == 0)
	{
		// �����ѹ����ֱ��ʹ��
		strCmd.Format("copy /Y \"%s\" \"%s%s\"",csPCFilePath,m_strTempFilePath,m_strTransName);
	}
	else
	{
		// ���ѹ���ͽ�ѹʹ��
		strCmd.Format("inflate \"%s\" \"%s%s\"", csPCFilePath, m_strTempFilePath,m_strTransName);
	}
	int nResult = system(strCmd.GetBuffer());//Windows�µ�system��������0
	if(0 != nResult)
	{
		int nError = GetLastError();
		strInfo.Format("Execute System Cmd Fail! Result(%d) Error(%d) Cmd(%s)",nResult,nError,strCmd);
		Log(LOGERROR,strInfo);
		return TESTCASE_FAIL;
	}
	
	// ������
	int nSendTransId = m_nFileTransId;
	m_nFileTransId = m_nFileTransId*2;// ���´���׼������֪����վ�������û���жϣ�Ϊɶ�Ҳ���+1����Ҫ������ô����
	if( nLoad != 0 )
	{
		// ��Ҫ����
		nResult = SendMsg_FileTransReq(csProcName,m_strTempFilePath,csBoardFilePath,m_strTransName,UAGT_FILE_TRANS_LOADMODULE,nFileLen,nSendTransId);
	}
	else
	{
		nResult = SendMsg_FileTransReq(csProcName,m_strTempFilePath,csBoardFilePath,m_strTransName,UAGT_FILE_TRANS_DOWNLOAD,nFileLen,nSendTransId);
	}
	if(nResult < 0)
	{
		strInfo.Format("Send UAGT_FILE_TRANS_REQ DownLoad Fail! Proc(%s)",csProcName);
		Log(LOGERROR,strInfo);
		DelTempFile();
		return TESTCASE_FAIL;
	}
	// �Ȼ�Ӧ
	if (WAIT_TIMEOUT == WaitForSingleObject(m_hFileTransEvent, m_nWaitTimeMSecForFtp)) 
	{
		strInfo.Format("Wait UAGT_FILE_TRANS_RSP Fail! Proc(%s)",csProcName);
		Log(LOGERROR,strInfo);
		DelTempFile();
		return TESTCASE_FAIL; 
	}
	std::map<CString,UAGT_FILE_TRANS_RSP>::iterator iterNameRspTrans = m_NameFileTransRspMap.find(csProcName);
	if(iterNameRspTrans == m_NameFileTransRspMap.end())
	{
		strInfo.Format("UnLogic %d",__LINE__);
		Log(LOGERROR,strInfo);
		DelTempFile();
		return TESTCASE_FAIL;
	}
	u32 u32TransId = ntohl(iterNameRspTrans->second.u32TransId);
	s32 s32TransResult = ntohl(iterNameRspTrans->second.s32TransResult);
    if(UAGT_FILE_TRANS_OK != s32TransResult)
	{
		strInfo.Format("File Trans Fail! TransResult(%d) Proc(%s)",s32TransResult,csProcName);
		Log(LOGERROR,strInfo);
		DelTempFile();
		return TESTCASE_FAIL;
	}
	// ����Ƿ���ƥ������
	if(u32TransId != nSendTransId*2) //��վ��*2�ظ� �������
	{
		strInfo.Format("File Trans Fail! TransId Not Match,Send(%d) Recv(%d) Proc(%s)",nSendTransId,u32TransId,csProcName);
		Log(LOGERROR,strInfo);
		DelTempFile();
		return TESTCASE_FAIL;
	}

	// ɾ����ʱ�ļ�
	DelTempFile();
	return TESTCASE_SUCCESS;
}

// Board·�������ǡ�/��д��\������
int CWorkManager::TestCaseLibUploadFile(char* csProcName, char* csPCFilePath, char* csBoardFilePath)
{
	CString strInfo("");
	strInfo.Format("CALLINTERFACE TestCase_UploadFile Proc(%s) PCFilePath(%s) BoardFilePath(%s)",
		csProcName,csPCFilePath,csBoardFilePath);
	Log(LOGINFO,strInfo);
	int nResult = 0;
	int nError = 0;

	// ����Ϣ
	int nSendTransId = m_nFileTransId;
	m_nFileTransId = m_nFileTransId*2;

	// ����boardʲô�ļ�������������tempfile,�Ǻ���
	nResult = SendMsg_FileTransReq(csProcName,m_strTempFilePath,csBoardFilePath,m_strTransName,UAGT_FILE_TRANS_UPLOAD,0,nSendTransId);
	
	if(nResult < 0)
	{
		strInfo.Format("Send UAGT_FILE_TRANS_REQ UpLoad Fail! Proc(%s)",csProcName);
		Log(LOGERROR,strInfo);
		return TESTCASE_FAIL;
	}
	// �Ȼ�Ӧ
	if (WAIT_TIMEOUT == WaitForSingleObject(m_hFileTransEvent, m_nWaitTimeMSecForFtp)) 
	{
		strInfo.Format("Wait UAGT_FILE_TRANS_RSP Upload Fail! Proc(%s)",csProcName);
		Log(LOGERROR,strInfo);
		return TESTCASE_FAIL; 
	}
	std::map<CString,UAGT_FILE_TRANS_RSP>::iterator iterNameRspTrans = m_NameFileTransRspMap.find(csProcName);
	if(iterNameRspTrans == m_NameFileTransRspMap.end())
	{
		strInfo.Format("UnLogic %d",__LINE__);
		Log(LOGERROR,strInfo);
		return TESTCASE_FAIL;
	}
	u32 u32TransId = ntohl(iterNameRspTrans->second.u32TransId);
	s32 s32TransResult = ntohl(iterNameRspTrans->second.s32TransResult);
	if(UAGT_FILE_TRANS_OK != s32TransResult)
	{
		strInfo.Format("File Trans Fail! TransResult(%d) Proc(%s)",s32TransResult,csProcName);
		Log(LOGERROR,strInfo);
		return TESTCASE_FAIL;
	}
	// ����Ƿ���ƥ������
	if(u32TransId != nSendTransId*2)
	{
		strInfo.Format("File Trans Fail! TransId Not Match,Send(%d) Recv(%d) Proc(%s)",nSendTransId,u32TransId,csProcName);
		Log(LOGERROR,strInfo);
		return TESTCASE_FAIL;
	}
	// �����ļ���Ŀ��·��
	CString strCmd("");
	strCmd.Format("copy /Y \"%s%s\" \"%s\"",m_strTempFilePath,m_strTransName,csPCFilePath); 
	nResult = system(strCmd.GetBuffer());
	if(0 != nResult)
	{
		nError = GetLastError();
		strInfo.Format("System Cmd Fail!Cmd(%s) Result(%d) Error(%d)",strCmd,nResult,nError);
		return TESTCASE_FAIL;
	}

	// �����ߺ�ɾ����ʱ�ļ�
	DelTempFile();
	return TESTCASE_SUCCESS;
}

int CWorkManager::TestCaseLibRegisterOutputFunc(TESTCASE_OUTPUT_FUNC pTestCaseOutputFunc)
{
	Log(LOGERROR,"CALLINTERFACE TestCase_RegisterOutputFunc(TEMP)");
	return TESTCASE_SUCCESS;

	//TODO
	if (NULL == pTestCaseOutputFunc)
	{
		Log(LOGERROR,"TESTCASE_OUTPUT_FUNC is NULL");
		return TESTCASE_FAIL;
	}
	else
	{
		g_pTestCaseOutputFunc = pTestCaseOutputFunc;
		return TESTCASE_SUCCESS;
	}
}

// ��ʱ�ӿ�
void CWorkManager::TestCaseConfigTempfilePath(char* csTempfilePath)
{
	m_strTempFilePath.Format("%s",csTempfilePath);
	if((m_strTempFilePath.Right(1)!= "\\") && (m_strTempFilePath.Right(1)!= '/'))
	{
		m_strTempFilePath += "\\";
	}
}

//////////////////////////////////////////////////////////////////////////
int CWorkManager::SockInit()
{
	WSADATA wsaData;
    WORD wVersionRequested = MAKEWORD( 2, 2 );
	int err = WSAStartup(wVersionRequested, &wsaData);
	if ( err != 0 ) 
	{
		Log(LOGERROR,"WSAStartup Fail");
		return INIT_SOCKET_FAIL;
	}
	return TESTCASE_SUCCESS;
}

BOOL CWorkManager::LogInit()
{
	if(m_pLog!=NULL)
	{
		fclose(m_pLog);
		m_pLog = NULL;
	}
	CreateDirectory(m_strTempFilePath.GetBuffer(),NULL);
	SYSTEMTIME curTime;
	GetLocalTime(&curTime);
	m_strLogName.Format("%sTestCaseLib_%d%02d%02d-%02d%02d%02d.log",m_strTempFilePath,
		curTime.wYear,curTime.wMonth,curTime.wDay,curTime.wHour,curTime.wMinute,curTime.wSecond);
	m_pLog = fopen(m_strLogName.GetBuffer(),"at"); 
	if(NULL == m_pLog)
	{
		return FALSE;
	}
	return TRUE;
}

BOOL CWorkManager::MemberInit()
{
	m_NameAddrMap.clear();
	m_NameDataMap.clear();
	return TRUE;
}


SOCKET CWorkManager::ClientConnect(CString strIP,int nPort, s32 s32SlotNum, s32 s32ProcId)
{
	// ���㱾�ض˿�
//	int srcPortv6 = 30000 + s32SlotNum * 64 + s32ProcId;
//	int srcPort = 30000 + s32SlotNum * 64 + s32ProcId;
	int srcPortv6 = 30000 + s32SlotNum * 64 + s32ProcId + 16 * 64; // Ϊ����OSP���߱����ͻ
	int srcPort = 30000 + s32SlotNum * 64 + s32ProcId + 16 * 64;

	//int srcPort = 5000;
	//int srcPortv6 = 10030;

	std::string::size_type idx;
	CString v6 = ":";
	idx = strIP.Find(v6);
	if(idx == std::string::npos)
	{
		// client
		SOCKADDR_IN clientAddr;
		clientAddr.sin_family = AF_INET;
		clientAddr.sin_addr = in4addr_any;//INADDR_ANY;
		clientAddr.sin_port = htons(srcPort);
  	
  	
		// 2018-5-9
		if(CONNECTTCPSERVER_PORT_NULL != nPort)
		{
			SOCKET clientSock = INVALID_SOCKET;
			int nError = 0;
			int nResult = 0;
			CString strLog("");
  	
			SOCKADDR_IN  srvAddr;
			srvAddr.sin_family = AF_INET; 
			srvAddr.sin_port = htons(nPort);
			srvAddr.sin_addr.s_addr = inet_addr(strIP.GetBuffer());
  	
			int nSrvAddrSize = sizeof (srvAddr); 
  	
  	
			for(int i = 0; i< CONNECTTCPSERVER_RETRY_NUMBER; i++) 
			{
				clientSock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
				if(clientSock == INVALID_SOCKET)
				{
					nError = WSAGetLastError();
					strLog.Format("Create New Socket Fail Error(%d)",nError);
					Log(LOGERROR,strLog);
					return INVALID_SOCKET;
				}
  	
				int optval = 1;
				nResult = setsockopt(clientSock,IPPROTO_TCP,TCP_NODELAY,(char *)&optval,sizeof(optval)); // ֮ǰ�ڶ���дΪSOCKET������û��Ч�����ֻ���TCP BUG:ֻ���TCP
				if(0 != nResult)
				{
					nError = WSAGetLastError();
					strLog.Format("Setsockopt TCP_NODELAY Fail Error(%d)",nError);
					Log(LOGERROR,strLog);
				}
  	
				strLog.Format("Socket bind info: port=%d, htons(port)=%d",srcPort, htons(srcPort));
				Log(LOGINFO,strLog);
				// bind
				if(bind(clientSock, (struct sockaddr*)&clientAddr, sizeof(clientAddr)) != 0)
				{
					strLog.Format("Socket bind error, port=%d, htons(port)=%d",srcPort, htons(srcPort));
					Log(LOGERROR,strLog);
					return SOCKET_ERROR;
				}
  	
				// ����
				nResult = connect(clientSock, (LPSOCKADDR)&srvAddr, nSrvAddrSize); 
				if(0 == nResult)
				{
					strLog.Format("Connect IP(%s) Port(%d) Success",strIP,nPort);
					Log(LOGINFO,strLog);
					return clientSock;
				}
  	
				nError = WSAGetLastError();
				strLog.Format("Connect IP(%s) Port(%d) Fail Error(%d)",strIP,nPort,nError);
				Log(LOGERROR,strLog);
  	
				Sleep(CONNECTTCPSERVER_RETRY_WAITTIME);
			}//for
  	
			strLog.Format("Connect IP(%s) %d Times Fail)",strIP,nPort);
			Log(LOGERROR,strLog);
  	
			return INVALID_SOCKET;
  	
		}
		else
		{
			SOCKET clientSock = INVALID_SOCKET;
			int nError = 0;
			int nResult = 0;
			CString strLog("");
  	
			SOCKADDR_IN  srvAddr;
			srvAddr.sin_family = AF_INET; 
			srvAddr.sin_port = htons(CONNECTTCPSERVER_PORT_OLD);
			srvAddr.sin_addr.s_addr = inet_addr(strIP.GetBuffer());
  	
			int nSrvAddrSize = sizeof (srvAddr); 
  	
			for(int i = 0; i< CONNECTTCPSERVER_RETRY_NUMBER; i++) 
			{
				clientSock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
				if(clientSock == INVALID_SOCKET)
				{
					nError = WSAGetLastError();
					strLog.Format("Create New Socket Fail Error(%d)",nError);
					Log(LOGERROR,strLog);
					return INVALID_SOCKET;
				}
  	
				int optval = 1;
				nResult = setsockopt(clientSock,IPPROTO_TCP,TCP_NODELAY,(char *)&optval,sizeof(optval)); // ֮ǰ�ڶ���дΪSOCKET������û��Ч�����ֻ���TCP BUG:ֻ���TCP
				if(0 != nResult)
				{
					nError = WSAGetLastError();
					strLog.Format("Setsockopt TCP_NODELAY Fail Error(%d)",nError);
					Log(LOGERROR,strLog);
				}
  	
				strLog.Format("Socket bind info: port=%d, htons(port)=%d",srcPort, htons(srcPort));
				Log(LOGINFO,strLog);
				// bind
				if(bind(clientSock, (struct sockaddr*)&clientAddr, sizeof(clientAddr)) != 0)
				{
					strLog.Format("Socket bind error, port=%d, htons(port)=%d",srcPort, htons(srcPort));
					Log(LOGERROR,strLog);
					return SOCKET_ERROR;
				}
  	
				// ����
				nResult = connect(clientSock, (LPSOCKADDR)&srvAddr, nSrvAddrSize); 
				if(0 == nResult)
				{
					strLog.Format("Connect IP(%s) Port(%d) Success",strIP,CONNECTTCPSERVER_PORT_OLD);
					Log(LOGINFO,strLog);
					return clientSock;
				}
  	
				nError = WSAGetLastError();
				strLog.Format("Connect IP(%s) Port(%d) Fail Error(%d)",strIP,CONNECTTCPSERVER_PORT_OLD,nError);
				Log(LOGERROR,strLog);
  	
				srvAddr.sin_port = htons(CONNECTTCPSERVER_PORT_NEW);
				nResult = connect(clientSock, (LPSOCKADDR)&srvAddr, nSrvAddrSize);
				if(0 == nResult)
				{
					strLog.Format("Connect IP(%s) Port(%d) Success",strIP,CONNECTTCPSERVER_PORT_NEW);
					Log(LOGINFO,strLog);
					return clientSock;
				}
				nError = WSAGetLastError();
				strLog.Format("Connect IP(%s) Port(%d) Fail Error(%d)",strIP,CONNECTTCPSERVER_PORT_NEW,nError);
				Log(LOGERROR,strLog);
  	
				Sleep(CONNECTTCPSERVER_RETRY_WAITTIME);
			}//for
  	
			strLog.Format("Connect IP(%s) %d Times Fail)",strIP,CONNECTTCPSERVER_RETRY_NUMBER);
			Log(LOGERROR,strLog);
  	
			return INVALID_SOCKET;
		}
	}
	else
	{
		// client
		SOCKADDR_IN6 clientAddr;
		memset(&clientAddr, 0, sizeof(clientAddr));
		clientAddr.sin6_family = AF_INET6;
		clientAddr.sin6_addr = in6addr_any;
		clientAddr.sin6_port = htons(srcPortv6);
		
  	
  	
		// 2018
		if(CONNECTTCPSERVER_PORT_NULL != nPort)
		{
			SOCKET clientSock = INVALID_SOCKET;
			int nError = 0;
			int nResult = 0;
			CString strLog("");
  	
			SOCKADDR_IN6  srvAddr;
			memset(&srvAddr, 0, sizeof(srvAddr));
			srvAddr.sin6_family = AF_INET6; 
			srvAddr.sin6_port = htons(nPort);
			//srvAddr.sin6_addr = in6addr_any;
			inet_pton(AF_INET6,strIP.GetBuffer(),(unsigned char *)&srvAddr.sin6_addr);
  	
			int nSrvAddrSize = sizeof (srvAddr); 
  	
  	
			for(int i = 0; i< CONNECTTCPSERVER_RETRY_NUMBER; i++) 
			{
				clientSock = socket(AF_INET6,SOCK_STREAM,IPPROTO_TCP);
				if(clientSock == INVALID_SOCKET)
				{
					nError = WSAGetLastError();
					strLog.Format("Create New Socket Fail Error(%d)",nError);
					Log(LOGERROR,strLog);
					return INVALID_SOCKET;
				}
  	
				int optval = 1;
				nResult = setsockopt(clientSock, IPPROTO_TCP,TCP_NODELAY,(char *)&optval,sizeof(optval)); // ֮ǰ�ڶ���дΪSOCKET������û��Ч�����ֻ���TCP BUG:ֻ���TCP
				if(0 != nResult)
				{
					nError = WSAGetLastError();
					strLog.Format("Setsockopt TCP_NODELAY Fail Error(%d)",nError);
					Log(LOGERROR,strLog);
				}
  	
				strLog.Format("Socket bind info: port=%d, htons(port)=%d",srcPortv6, htons(srcPortv6));
				Log(LOGERROR,strLog);
				// bind
				if(bind(clientSock, (struct sockaddr*)&clientAddr, sizeof(clientAddr)) != 0)
				{
					strLog.Format("!Socket bind error, port=%d, htons(port)=%d",srcPort, htons(srcPort));
					Log(LOGERROR,strLog);
					//return SOCKET_ERROR;
				}
  	
				// ����
				nResult = connect(clientSock, (LPSOCKADDR)&srvAddr, nSrvAddrSize); 
				if(0 == nResult)
				{
					strLog.Format("!Connect IP(%s) Port(%d) Success",strIP,nPort);
					Log(LOGINFO,strLog);
					return clientSock;
				}
  	
				nError = WSAGetLastError();
				strLog.Format("Connect IP(%s) Port(%d) Fail Error(%d)",strIP,nPort,nError);
				Log(LOGERROR,strLog);
  	
				Sleep(CONNECTTCPSERVER_RETRY_WAITTIME);
			}//for
  	
			strLog.Format("Connect IP(%s) %d Times Fail)",strIP,nPort);
			Log(LOGERROR,strLog);
  	
			return INVALID_SOCKET;
  	
		}
		
		else
		{
			SOCKET clientSock = INVALID_SOCKET;
			int nError = 0;
			int nResult = 0;
			CString strLog("");
			
			
  	
			SOCKADDR_IN6  srvAddr;
			memset(&srvAddr, 0, sizeof(srvAddr));
			srvAddr.sin6_family = AF_INET6; 
			srvAddr.sin6_port = htons(CONNECTTCPSERVER_PORT_OLD);
			//srvAddr.sin6_addr = in6addr_any;
			inet_pton(AF_INET6, strIP.GetBuffer(),(unsigned char *)&srvAddr.sin6_addr);
  	
			int nSrvAddrSize = sizeof (srvAddr); 
  	
			for(int i = 0; i< CONNECTTCPSERVER_RETRY_NUMBER; i++) 
			{
				//clientSock = socket(AF_INET6,SOCK_STREAM,IPPROTO_TCP);
				clientSock = socket(AF_INET6, SOCK_STREAM, IPPROTO_IP);
				if(clientSock == INVALID_SOCKET)
				{
					nError = WSAGetLastError();
					strLog.Format("!!!Create New Socket Fail Error(%d)",nError);
					Log(LOGERROR,strLog);
					return INVALID_SOCKET;
				}
  	
				int optval = 1;
				nResult = setsockopt(clientSock, IPPROTO_TCP,TCP_NODELAY,(char *)&optval,sizeof(optval)); // ֮ǰ�ڶ���дΪSOCKET������û��Ч�����ֻ���TCP BUG:ֻ���TCP
				if(0 != nResult)
				{
					nError = WSAGetLastError();
					strLog.Format("!!!Setsockopt TCP_NODELAY Fail Error(%d)",nError);
					Log(LOGERROR,strLog);
				}
  	
				strLog.Format("!!!Socket bind info: port=%d, htons(port)=%d",srcPortv6, htons(srcPortv6));
				Log(LOGERROR,strLog);
				// bind
				
				if(bind(clientSock, (struct sockaddr*)&clientAddr, sizeof(clientAddr)) != 0)
				{
					int n = WSAGetLastError();
					strLog.Format("!!!error = %d", n);
					Log(LOGERROR, strLog);
					strLog.Format("!!!Socket bind error, port=%d, htons(port)=%d",srcPortv6, htons(srcPortv6));
					Log(LOGERROR,strLog);
					return SOCKET_ERROR;
				}

				// ����
				nResult = connect(clientSock, (LPSOCKADDR)&srvAddr, nSrvAddrSize); 
				if(0 == nResult)
				{
					strLog.Format("!!!Connect IP(%s) Port(%d) Success",strIP,CONNECTTCPSERVER_PORT_OLD);
					Log(LOGINFO,strLog);
					return clientSock;
				}
  	
				nError = WSAGetLastError();
				strLog.Format("Connect IP(%s) Port(%d) Fail Error(%d)",strIP,CONNECTTCPSERVER_PORT_OLD,nError);
				Log(LOGERROR,strLog);
  	
				srvAddr.sin6_port = htons(CONNECTTCPSERVER_PORT_NEW);
				nResult = connect(clientSock, (LPSOCKADDR)&srvAddr, nSrvAddrSize);
				if(0 == nResult)
				{
					strLog.Format("Connect IP(%s) Port(%d) Success",strIP,CONNECTTCPSERVER_PORT_NEW);
					Log(LOGINFO,strLog);
					return clientSock;
				}
				nError = WSAGetLastError();
				strLog.Format("Connect IP(%s) Port(%d) Fail Error(%d)",strIP,CONNECTTCPSERVER_PORT_NEW,nError);
				Log(LOGERROR,strLog);
  	
				Sleep(CONNECTTCPSERVER_RETRY_WAITTIME);
			}//for
  	
			strLog.Format("Connect IP(%s) %d Times Fail)",strIP,CONNECTTCPSERVER_RETRY_NUMBER);
			Log(LOGERROR,strLog);
  	
			return INVALID_SOCKET;
		}
	}
}

BOOL CWorkManager::ClientClose(SOCKET nSock)
{
	if(INVALID_SOCKET == nSock)
	{
		// ��¼
		return TRUE;
	}
	TRACE("Close socket %d\n",nSock);
	int nResult = closesocket(nSock);
	return TRUE;
}

// ���������߳�
BOOL CWorkManager::RecvThreadStart()
{
	if(NULL != m_hRecvHandle)
	{
		TerminateThread(m_hRecvHandle,0);
		m_hRecvHandle = NULL;
	}

	DWORD dwThreadId;
	m_hRecvHandle = CreateThread(NULL,
		0,
		(LPTHREAD_START_ROUTINE)RecvThread, 
		(LPVOID)this, 
		0, 
		&dwThreadId);

	return TRUE;
}

DWORD WINAPI CWorkManager::RecvThread(LPVOID lpParameter)
{
	CWorkManager* pMgr = (CWorkManager*)lpParameter;
	pMgr->RecvMsg();
	return 0;
}

int CWorkManager::SendMsg(CString strProName,char* pMsg, int nSendSize)
{
	SOCKET clientSock = m_NameAddrMap[strProName].sock;
	int nError = 0;
	CString strInfo("");
	//�����־
	char csPacketSign = UAGENT_TCP_PACKET_SIGN;
	int nSendLen = send(clientSock, (char*)&csPacketSign, sizeof(char), 0); 
	if(1 != nSendLen)
	{
		nError = WSAGetLastError();
		strInfo.Format("send 0x5a to %s fail(%d)",strProName,nError);
		Log(LOGERROR,strInfo);
		return EXECUTECMD_SEND_FAIL;
	}
	// ������Ϣ
	int nPoint = 0;
	int nTcpSend = nSendSize;
	while(nTcpSend > BOARD_SEND_MCU)
	{
		nSendLen = send(clientSock,pMsg+nPoint,BOARD_SEND_MCU,0);
		nPoint += nSendLen;
		if(nSendLen != BOARD_SEND_MCU)
		{
			nError = WSAGetLastError();
			strInfo.Format("send %s fail(%d) len(%d)",strProName,nError,BOARD_SEND_MCU);
			Log(LOGERROR,strInfo);
			return EXECUTECMD_SEND_FAIL;
		}
		nTcpSend = nTcpSend - BOARD_SEND_MCU;
	}

	if(nTcpSend > 0)
	{
		nSendLen = send(clientSock,pMsg+nPoint,nTcpSend,0);
		nPoint += nSendLen;
		if(nSendLen != nTcpSend)
		{
			nError = WSAGetLastError();
			strInfo.Format("send %s fail(%d) len(%d)",strProName,nError,nTcpSend);
			Log(LOGERROR,strInfo);
			return EXECUTECMD_SEND_FAIL;
		}
	}
	return nSendSize;
}

int CWorkManager::DealMsg(CString strProcName,UAGENT_SIG_HEAD* pMsg)
{
	CString strInfo("");
	//Log(LOGDEBUG,"DealMsg In");
	u32 u32ServiceId = ntohl(pMsg->msgHead.u32ServiceId);
	u32 u32MsgSize = ntohl(pMsg->msgHead.u32MsgSize);
	u32 u32SigType = ntohl(pMsg->u32SigType);
	int nTransId = ntohl(((UAGENT_DATA_MSG*)pMsg)->u32TransId);// ��ʵ��Ϣ�ṹһ��������u32SigTypeһ����ֵ
	
	strInfo.Format("Deal Message ServiceId %d MsgSize %d SigType %d",u32ServiceId,u32MsgSize,u32SigType);
	Log(LOGINFO,strInfo);

	if(CONTROL_SIGNAL_SERVICE_ID == u32ServiceId)//serviceid��0��Ϊ����
	{
		switch (u32SigType)
		{
		case UAGENT_SIGTYPE_LOGON_REQ: //0 PC����¼���󣬻�վ����������
			{
				strInfo.Format("RECV UAGENT_SIGTYPE_LOGON_REQ Proc(%s)",strProcName);
				Log(LOGINFO,strInfo);
				SetEvent(m_hSigRspEvent); //Ҳ�������豸��û��
			}
			break;
		case UAGENT_SIGTYPE_LOGON_RSP: //1
			{
				strInfo.Format("RECV UAGENT_SIGTYPE_LOGON_REQ Proc(%s)",strProcName);
				Log(LOGINFO,strInfo);
				// ��¼�����ȼ������������Ӧ
				SetEvent(m_hSigRspEvent);
			}
			break;
		case UAGENT_SIGTYPE_ALIVE_REQ://2 Ӧ�ò�ʹ���ˣ�PC������NBҲ��Ӧ�û�
			{
				strInfo.Format("RECV UAGENT_SIGTYPE_LOGON_REQ Proc(%s)",strProcName);
				Log(LOGINFO,strInfo);
			}
			break;
		case UAGENT_SIGTYPE_ALIVE_RSP://3
			{
				strInfo.Format("RECV UAGENT_SIGTYPE_LOGON_REQ Proc(%s)",strProcName);
				Log(LOGINFO,strInfo);
			}
			break;
		case UAGENT_SIGTYPE_SERVICE_REQ://4
			{
				strInfo.Format("RECV UAGENT_SIGTYPE_LOGON_REQ Proc(%s)",strProcName);
				Log(LOGINFO,strInfo);
			}
			break;
		case UAGENT_SIGTYPE_SERVICE_RSP: //5
			{
				strInfo.Format("RECV UAGENT_SIGTYPE_LOGON_REQ Proc(%s)",strProcName);
				Log(LOGINFO,strInfo);
				SetEvent(m_hSigRspEvent);
			}
			break;
		case UAGENT_SIGTYPE_DATA_ACK://6
			{
				strInfo.Format("RECV UAGENT_SIGTYPE_LOGON_REQ Proc(%s)",strProcName);
				Log(LOGINFO,strInfo);
			}
			break;
		default:
			{
				CString strInfo("");
				strInfo.Format("RECV Unknown CONTROL_SIGNAL_SERVICE Msg SigType(%d) Proc(%s)",u32SigType,strProcName);
				Log(LOGERROR,strInfo);
			}
			break;
		}
	}
	else if(UAGT_FILE_TRANS_SERVICE_ID == u32ServiceId)
	{
		if(u32MsgSize != sizeof(UAGT_FILE_TRANS_RSP)-sizeof(UAGENT_MSG_HEAD))
		{
			strInfo.Format("RECV Unknown UAGT_FILE_TRANS_SERVICE Msg SigType/TransId(%d) Proc(%s)",u32SigType,strProcName);
			Log(LOGERROR,strInfo);
			return 0;
		}
		UAGT_FILE_TRANS_RSP rsp;
		memcpy((void*)&rsp,pMsg,sizeof(UAGT_FILE_TRANS_RSP));
		m_NameFileTransRspMap[strProcName] = rsp;
		SetEvent(m_hFileTransEvent);
	}
	else if( UAGT_CMD_EXECUTE_SERVICE_ID == u32ServiceId)
	{
		// ���ڿ�����͵Ļ�����serid��4
		// ��վ���͵��ǣ�1��0x5a ��2��UAGENT_DATA_MSG+������ ��ʽ����RSP1��RSP2
		if(u32MsgSize == (sizeof(UAGT_CMD_SET_TASK_PRIO_REQ)-sizeof(UAGENT_MSG_HEAD)))
		{
			strInfo.Format("RECV UAGT_CMD_TASK_RSP Proc(%s)",strProcName);
			Log(LOGINFO,strInfo);
			SetEvent(m_hTaskRspEvent);

		}
		// �ó������ж�RSP1
		//else if(u32MsgSize == (sizeof(UAGT_CMD_EXECUTE_RSP1)- sizeof(UAGENT_MSG_HEAD)))//2018.6.5 ��1����һ���Ϊlonglong����64λ�³��ȱ仯��2����վ�������嶨��Ϊһ���ṹ���뺬ͷ�Ľṹ���壬�ڱ�����ռλ��ͬ
		else if(u32MsgSize == MSGSIZE_RSP1_32BIT)
		{
			UAGT_CMD_EXECUTE_RSP1 rsp1;
			memcpy((void*)&rsp1,pMsg,sizeof(UAGT_CMD_EXECUTE_RSP1));
			m_NameRsp1Map[strProcName] = rsp1;

			strInfo.Format("RECV UAGT_CMD_EXECUTE_RSP1 Proc(%s) Id(%d) Result(%d) Addr(%d) Val(%d)",
				strProcName,ntohl(rsp1.u32CmdId),ntohl(rsp1.s32CmdExecResult),ntohl(rsp1.u32Addr),ntohl(rsp1.s32Val));
			Log(LOGINFO,strInfo);
            m_b64Bit = FALSE;
			SetEvent(m_hRsp1Event);
		}
		else if (u32MsgSize == MSGSIZE_RSP1_64BIT)
		{
			// Ҫ�ĵĻ���Щ�ṹ�����С��ת�����øģ������Ķ�ʹ�õ�ȡֵҲû��Ӱ�죬��Ϊ�漰�������������˭Ҳ����
			UAGT_CMD_EXECUTE_RSP1 rsp1;
			memcpy((void*)&rsp1,pMsg,sizeof(UAGT_CMD_EXECUTE_RSP1));
			m_NameRsp1Map[strProcName] = rsp1;

			strInfo.Format("RECV UAGT_CMD_EXECUTE_RSP1 64bit Proc(%s) Id(%d) Result(%d) Addr(%d) Val(%d)",
				strProcName,ntohl(rsp1.u32CmdId),ntohl(rsp1.s32CmdExecResult),ntohl(rsp1.u32Addr),ntohl(rsp1.s32Val));
			Log(LOGINFO,strInfo);
			m_b64Bit = TRUE;
			SetEvent(m_hRsp1Event);
		}
		else //RSP2
		{
			UAGT_CMD_EXECUTE_RSP2_FORMAP rsp2;
			//strInfo.Format("rsp2 size %d recv %d",sizeof(UAGT_CMD_EXECUTE_RSP2_FORMAP),u32MsgSize+sizeof(UAGENT_MSG_HEAD));
			//Log(LOGDEBUG,strInfo);
			memset(&rsp2,0,sizeof(UAGT_CMD_EXECUTE_RSP2_FORMAP));
			memcpy((void*)&rsp2,pMsg,u32MsgSize+sizeof(UAGENT_MSG_HEAD));
			m_NameRsp2Map[strProcName] = rsp2;
			strInfo.Format("RECV UAGT_CMD_EXECUTE_RSP2 Proc(%s) Id(%d) Result(%d) RetVal(%d) DataLen(%d)",
				strProcName,  ntohl(rsp2.u32CmdId), ntohl(rsp2.s32CmdExecResult), ntohl(rsp2.s32CmdRetVal), ntohl(rsp2.u32DataAreaLen));
			Log(LOGINFO,strInfo);
			SetEvent(m_hRsp2Event);			
		}
	}
	else
	{
		strInfo.Format("RECV Unknown Msg ServiceId(%d) SigType/TransId(%d) Len(%d) Proc(%s)",u32ServiceId,u32SigType,u32MsgSize,strProcName);
		Log(LOGERROR,strInfo);
	}

	//Log(LOGDEBUG,"DealMsg Out");
	return 0;
}

void CWorkManager::RecvMsg()
{
	//Log(LOGDEBUG,"RecvMsg In");
	int nError = 0;
	fd_set readfds;
	fd_set writefds;
	fd_set exceptfds;
	FD_ZERO(&writefds);
	FD_ZERO(&exceptfds);
	timeval outTime;
	outTime.tv_sec = 0;   //���õȴ�ʱ��Ϊ1s
	outTime.tv_usec= 10; //����

	char buffOnePack[BUFFER_DATA_RECV_LEN+1] = {0};
	std::map<CString,CSockInfo>::iterator iterNameSock;

	CString strInfo("");
	while(TRUE)
	{	
		FD_ZERO(&readfds);
		EnterCriticalSection(&m_cs); // �����ٽ���
		for(iterNameSock = m_NameAddrMap.begin();iterNameSock!=m_NameAddrMap.end();iterNameSock++)
		{			
			FD_SET((iterNameSock->second).sock,&readfds);			
		}
		LeaveCriticalSection(&m_cs);
		//maxfds��windows��û����
		int result = select(0,&readfds,&writefds,&exceptfds,&outTime);//��ʱ�������FDSET��û����Ӧ��������������readfdsʧЧ
		//int result = select(0,&readfds,&writefds,&exceptfds,NULL);
		// �����ڶ���һ���һ����յ����Ұѳ�ʱ���ö�һ�㣬�ѷ��ͼӵ�ʱ�ӣ���  ԭ��NULL��
		//BUG:һ��ʼû���ó�ʱ������������߳����ˣ���������û��֪ͨ���߳� timeout����Ҳ����
		if (result == SOCKET_ERROR)
		{
			nError = WSAGetLastError();
			//strInfo.Format("select fail error %d",nError);
			//Log(LOGERROR,strInfo);
			Sleep(100);
			continue;
		}
		else if(0 == result)
		{
			//Log(LOGDEBUG,"select 0");
			Sleep(100);
			continue;
		}
		else
		{
			//strInfo.Format("select %d",result);
			//Log(LOGDEBUG,strInfo);
		}

		EnterCriticalSection(&m_cs); // �����ٽ���
		for(iterNameSock = m_NameAddrMap.begin();iterNameSock!= m_NameAddrMap.end();iterNameSock++)
		{
			SOCKET sockTemp = (iterNameSock->second).sock;
			CString strName(iterNameSock->first);
			//strInfo.Format("recv %s %d",strName,sockTemp);
			//Log(LOGDEBUG,strInfo);
			if (FD_ISSET(sockTemp, &readfds))
			{
				int nRecvLen = recv(sockTemp,buffOnePack,BUFFER_DATA_RECV_LEN,0);
				//strInfo.Format("recv %d",nRecvLen);
				//Log(LOGDEBUG,strInfo);
				if(nRecvLen>0) // �����ж��socket���������ӣ���Ӧ��ֻ��һ��������Ϣ
				{
					DealRecvBuff(strName,buffOnePack,nRecvLen);
				}
			}
		}
		LeaveCriticalSection(&m_cs);
	}//while
	//Log(LOGDEBUG,"RecvMsg Out");
}

// ���������
void CWorkManager::DealRecvBuff(CString strProcName,char* pData,int nDataLen)
{
	//Log(LOGDEBUG,"DealRecvBuff In");
	CString strInfo("");
	// TEST --------------------->
//#ifdef _DEBUG
//	CString strDebug("");
//	strDebug.Format("�յ�%s��%d�ֽ�\n",strProcName,nDataLen);
//	TRACE(strDebug);
//	for(int i = 0;i<nDataLen;i++)
//	{
//		//if(pData[i] == 0xff)
//		char cTemp = 0xff;
//		if(pData[i] == cTemp)
//		{
//			int mm = 0;
//		}
//
//		if((i%4 == 0) && (i!= 0))
//			TRACE(" ");
//		if((i%16 == 0)&& (i!= 0))
//			TRACE("\n"); 
//		strDebug.Format("%02x",((u8*)pData)[i]); //ȫf
//		TRACE(strDebug);
//		
//	}
//	TRACE("\n");
//#endif
// TEST <---------------------

	std::map<CString,CRecvPacketModel>::iterator iter = m_NameDataMap.find(strProcName);
	if(iter == m_NameDataMap.end())
	{
		strInfo.Format("Unknown %s",strProcName);
		Log(LOGERROR,strInfo);
		return;
	}
	int nNextMsg =  iter->second.SetRecvBuffer(pData,nDataLen);
	//strInfo.Format("@1����%d",nNextMsg);
	//Log(LOGDEBUG,strInfo);
	//���ʣ�����0���϶�����һ�����ˣ���ʣ��Ϊ0��Ҳ���ܲ������ù�
	while(nNextMsg > 0)
	{
		DealMsg(strProcName,(UAGENT_SIG_HEAD*)iter->second.m_Buffer);	
		//Log(LOGDEBUG,"@��λ1");
		iter->second.ResetModel();
		nNextMsg = iter->second.SetRecvBuffer(pData+(nDataLen-nNextMsg),nNextMsg); 
		//strInfo.Format("@2����%d",nNextMsg);
		//Log(LOGDEBUG,strInfo);
	}
	// ʣ��Ϊ0�����
	if(iter->second.IsIntegrityMsg())
	{
		//Log(LOGDEBUG,"@����");
		DealMsg(strProcName,(UAGENT_SIG_HEAD*)iter->second.m_Buffer);
		//Log(LOGDEBUG,"@��λ2");
		iter->second.ResetModel();
	}
	// ��û���͵���һ��DealRecvBuff�ĵ���
	//Log(LOGDEBUG,"DealRecvBuff Out");
}

void CWorkManager::Log(LOGENUM type,CString strInfo)
{
	if(NULL != m_pLog)
	{
		SYSTEMTIME curTime;
		GetLocalTime(&curTime);
		CString strLine("");
		if(LOGDEBUG == type)
		{
			strLine.Format("DEBUG [%d%02d%02d-%02d:%02d:%02d] %s\n",
				curTime.wYear,curTime.wMonth,curTime.wDay,curTime.wHour,curTime.wMinute,curTime.wSecond,strInfo);
		}
		else if(LOGINFO == type)
		{
			strLine.Format("INFOR [%d%02d%02d-%02d:%02d:%02d] %s\n",
				curTime.wYear,curTime.wMonth,curTime.wDay,curTime.wHour,curTime.wMinute,curTime.wSecond,strInfo);
		}
		else
		{
			strLine.Format("ERROR [%d%02d%02d-%02d:%02d:%02d] %s\n",
				curTime.wYear,curTime.wMonth,curTime.wDay,curTime.wHour,curTime.wMinute,curTime.wSecond,strInfo);
		}
		fprintf(m_pLog,strLine.GetBuffer());
		fflush(m_pLog);
		//TRACE(strLine);
	}

	
	if(NULL == g_pTestCaseOutputFunc)
	{
		return;
	}
	try
	{
		// ��2��������0��1��2 Ҳ��֪��ɶ��˼�� ���Ҳû�и�ʽ���о�����ӿھ��Ǹı�D:Temp��һ�ַ���,ԭ������־��¼Ҳû��ͳһ��ʽ���е���"[%s]: %s\n"���������ּӻس�	
		if(LOGERROR == type)
		{
			g_pTestCaseOutputFunc((u8*)strInfo.GetBuffer(),1,SHOW_LEVEL_CRITICAL);
		}
		else if(LOGINFO == type)
		{
			g_pTestCaseOutputFunc((u8*)strInfo.GetBuffer(),1,SHOW_LEVEL_MAJOR);
		}
		else
		{
			g_pTestCaseOutputFunc((u8*)strInfo.GetBuffer(),1,SHOW_LEVEL_MINOR);
		}
	}
	catch(...)
	{
		if(NULL != m_pLog)
		{
			fprintf(m_pLog,"ERROR! TestCaseOutputFunc fail\n");
			fflush(m_pLog);
		}
	}
}

void CWorkManager::LogReset()
{
	// ������TestCase_Init�ڼ��������������в��Ե�ʱ��ֻ�ǵ�һ�Σ�������־��ܴ�
	// ���ڴ�����ע���ʱ���ж�һ�Σ��Ƿ���Ҫ�ؽ���־
	CFile f;
	f.Open(m_strLogName,CFile::modeRead|CFile::shareDenyNone);
	ULONGLONG len = f.GetLength();
	f.Close();
	if(len > MY_LOG_MAX_LENGTH)
	{
		SYSTEMTIME curTime;
		GetLocalTime(&curTime);
		CString strNewLogName("");
		strNewLogName.Format("%sTestCaseLib_%d%02d%02d-%02d%02d%02d.log",m_strTempFilePath,
			curTime.wYear,curTime.wMonth,curTime.wDay,curTime.wHour,curTime.wMinute,curTime.wSecond);
		CString strInfo("");
		strInfo.Format("NextLog:%s",strNewLogName);
		Log(LOGINFO,strInfo);

		if(NULL != m_pLog)
		{
			fclose(m_pLog);
			m_pLog = NULL;
		}
		m_pLog = fopen(strNewLogName.GetBuffer(),"at"); 
		strInfo.Format("PreLog:%s",m_strLogName);
		Log(LOGINFO,strInfo);
		m_strLogName = strNewLogName;
	}
}


// �����������Ȼ�Ӧ
int CWorkManager::SendMsg_ExecuteCmd(CString strProName,CString strFunction,STRU_TESTCASE_EXECUTECMD_PARAS *pstruParas,int nParaNum,int *pnRetVal,unsigned int *pu32SymbolAddr)
{
	//Log(LOGDEBUG,"SendMsg_ExecuteCmd In");
	CString strInfo("");
	int u32DataAreaLen = 0;
	UAGT_CMD_EXECUTE_REQ cmdMsg;
	char* pCmdMsg = (char*)&cmdMsg;
	memset(pCmdMsg,0,sizeof(UAGT_CMD_EXECUTE_REQ));
	cmdMsg.sigHead.msgHead.u32ServiceId = ntohl(UAGT_CMD_EXECUTE_SERVICE_ID);//4���ж�Ӧ��uagent��ͷ��serid������ʵ��
	//cmdMsg.sigHead.msgHead.u32MsgSize //�������nDataArea����д
	cmdMsg.sigHead.u32SigType = ntohl(0); // �̶����� BUG 20171122 ץ��ȷ�ϣ���e2prom

	cmdMsg.u32CmdId = ntohl(0);
	cmdMsg.u32ParaNum = ntohl(nParaNum);
	cmdMsg.u32CmdAttr = ntohl(0);//u32CmdAttr ���ڴ��ݺ�����ַ,�����޷���ѯ���ű��dsp���������ҿ����ڹ̶�Ϊ0
	u32DataAreaLen = strFunction.GetLength() + 1; //ԭ����Ϊstrlen +1,һ����ϣ������Ѿ�memset
	strcpy(cmdMsg.u8DataArea,strFunction.GetBuffer());// u8DataArea������������֣�ǰ��ĳ���������ָʾ
	
	// ������俪ʼ
	/*  BUFFER���͵Ķ����������ݺ�����
	typedef struct tag_STRU_CMD_EXCUTE_PARAS
	{ 
		s32  s32ParaType;       // �������ͣ���ֵ/���ݻ����� 
		s32  s32ParaVal;        // ��ֵ/���ݻ���������  
		s32  s32ParaDataIndex;  // ������������ 
		s32  s32ParaLen;        // ��������     
	}STRU_CMD_EXCUTE_PARAS;
	*/

	for (int i = 0; i < nParaNum; i++) //�ṹ���е�32���������������,�����������ֱ�
	{
		switch(pstruParas[i].s32ParaType)
		{
			// ��ֵ������ֱ����дs32ParaVal
		case TESTCASE_EXECUTECMD_PARATYPE_NUMBER:
			strInfo.Format("Parameter%d Value %d",i,pstruParas[i].s32ParaVal);
			Log(LOGINFO,strInfo);
			cmdMsg.struCmdPara[i].s32ParaType = htonl(UAGT_CMD_PARATYPE_NUMBER);
			cmdMsg.struCmdPara[i].s32ParaVal = htonl(pstruParas[i].s32ParaVal);
			break;

			/* ���������� */
		case TESTCASE_EXECUTECMD_PARATYPE_BUFFER: // ����ǻ��������͵���Ҫ�����ֺ�����ſ���
			/* ����Ϊ4�ֽڶ��� */
			if (0 != (u32DataAreaLen % 4))
			{
				u32DataAreaLen = (u32DataAreaLen / 4 + 1) * 4;
			}                
			cmdMsg.struCmdPara[i].s32ParaType = htonl(UAGT_CMD_PARATYPE_BUFFER);
			cmdMsg.struCmdPara[i].s32ParaLen = htonl(pstruParas[i].s32ParaVal);
			cmdMsg.struCmdPara[i].s32ParaDataIndex = htonl(u32DataAreaLen);

			strInfo.Format("Parameter%d BufferLen %d DataIndex %d",i,pstruParas[i].s32ParaVal,u32DataAreaLen);
			Log(LOGINFO,strInfo);

			if ((u32DataAreaLen + pstruParas[i].s32ParaVal) > 102400)
			{
				Log(LOGERROR,"over 102400");
				return ECMD_INVALID_ERROR;
			}
			memcpy((char*)(cmdMsg.u8DataArea + u32DataAreaLen), (char*)pstruParas[i].pParaBuf, pstruParas[i].s32ParaVal);//֮����ץ����ȫ�㣬������Ϊ�����paraBuf�������˳�ʼ��
			u32DataAreaLen += pstruParas[i].s32ParaVal;

			// ���Ӵ�ӡ�����ַ�����ʱ��������룬�����԰汾�ṩ��ץ��һ���Ľ��
			// strInfo.Format("Buff %s",pstruParas[i].pParaBuf);
			// Log(LOGINFO,strInfo);
			break;
		default:
			return ECMD_INVALID_ERROR;
		}
	}// for����������

	//TRACE("cmdMsg.sigHead.msgHead.u32MsgSize %d\n",sizeof(UAGT_CMD_EXECUTE_REQ_HEAD)+u32DataAreaLen-sizeof(UAGENT_MSG_HEAD));
	cmdMsg.sigHead.msgHead.u32MsgSize = ntohl(sizeof(UAGT_CMD_EXECUTE_REQ_HEAD)+u32DataAreaLen-sizeof(UAGENT_MSG_HEAD));
	cmdMsg.u32DataAreaLen = ntohl(u32DataAreaLen); //Bug 20171122 ��©��д
	strInfo.Format("SEND UAGT_CMD_EXECUTE_REQ To Proc(%s)",strProName);
	Log(LOGINFO,strInfo);
	SendMsg(strProName,(char*)&cmdMsg,sizeof(UAGT_CMD_EXECUTE_REQ_HEAD)+u32DataAreaLen);
 
     if(WAIT_TIMEOUT == WaitForSingleObject(m_hRsp1Event,m_nWaitTimeMSec))
	 {
		 strInfo.Format("Wait UAGT_CMD_EXECUTE_RSP1 timeout! Proc(%s)",strProName);
		 Log(LOGERROR,strInfo);
		return ECMD_RSP1_TIMEOUT_ERROR;
	 }
	 // ����RSP1
	 std::map<CString,UAGT_CMD_EXECUTE_RSP1>::iterator iterNameRsp1 = m_NameRsp1Map.find(strProName);
	 if(iterNameRsp1 == m_NameRsp1Map.end())
	 {
		 CString strInfo("");
		 strInfo.Format("UnLogic %d",__LINE__);
		 Log(LOGERROR,strInfo);
		 return TESTCASE_FAIL;
	 }
	 u32 u32CmdIdRsp1 = ntohl(iterNameRsp1->second.u32CmdId);
	 s32 s32CmdExecResultRsp1 = ntohl(iterNameRsp1->second.s32CmdExecResult);

	 // ������ֵ���ʹ�ã�����Ҫ����bit������
	 u32 u32AddrRsp1 = ntohl(iterNameRsp1->second.u32Addr);
	 s32 s32ValRsp1 = ntohl(iterNameRsp1->second.s32Val);
	 *pnRetVal = s32ValRsp1;
	 *pu32SymbolAddr = u32AddrRsp1;
      m_NameRsp1Map.erase(iterNameRsp1);// ���
	 // ����ֵ�ж�
	 switch(s32CmdExecResultRsp1)
	 {
	 case UAGT_CMD_RSP1_NO_SYMBOL:
		 {
			 strInfo.Format("Cmd Execute Result Fail(NO_SYMBOL)! Proc(%s)",strProName);
			 Log(LOGERROR,strInfo);
		 }
		 return ECMD_RSP1_NO_SYMBOL_ERROR;
	 case UAGT_CMD_RSP1_NONE_EXECUTE_SYMBOL:
		 {
			 strInfo.Format("Cmd Execute Result Fail(NONE_EXECUTE_SYMBOL)! Proc(%s)",strProName);
			 Log(LOGERROR,strInfo);
		 }
		 return ECMD_RSP1_NO_SYMBOL_ERROR;
	 case UAGT_CMD_RSP1_BUSY:
		 {
			 strInfo.Format("Cmd Execute Result Fail(TASK_BUSY)! Proc(%s)",strProName);
			 Log(LOGERROR,strInfo);
		 }
		 return ECMD_TASK_BUSY;
	 }

     // ����RSP2
	 if(WAIT_TIMEOUT == WaitForSingleObject(m_hRsp2Event,m_nWaitTimeMSec))
	 {
		 strInfo.Format("Wait UAGT_CMD_EXECUTE_RSP2 timeout! Proc(%s)",strProName);
		 Log(LOGERROR,strInfo);
		 return ECMD_RSP2_TIMEOUT_ERROR;
	 }
	 
	 std::map<CString,UAGT_CMD_EXECUTE_RSP2_FORMAP>::iterator iterNameRsp2 = m_NameRsp2Map.find(strProName);
	 if(iterNameRsp2 == m_NameRsp2Map.end())
	 {
		 strInfo.Format("UnLogic %d",__LINE__);
		 Log(LOGERROR,strInfo);
		 return TESTCASE_FAIL;
	 }
	u32 u32CmdIdRsp2 = ntohl(iterNameRsp2->second.u32CmdId);
	s32 s32CmdExecResultRsp2 = ntohl(iterNameRsp2->second.s32CmdExecResult);
	s32 s32CmdRetValRsp2 = ntohl(iterNameRsp2->second.s32CmdRetVal);// BUG 20171123 ���д��
	u32 u32DataAreaLenRsp2 = ntohl(iterNameRsp2->second.u32DataAreaLen);
	(*pnRetVal) = s32CmdRetValRsp2;
	// �������ã�����PC�������ã��ٰѻ�վ������д�أ�
	for(int i = 0;i<nParaNum;i++)
	{
		switch(pstruParas[i].s32ParaType)
		{
		case TESTCASE_EXECUTECMD_PARATYPE_NUMBER: // ��ֵ�Ĳ�д��
			break;
		case TESTCASE_EXECUTECMD_PARATYPE_BUFFER: // ��������д�أ�����Ҳ���Ϊ��Ӧ�Զ��������Ϣ�Ĵ���,���������Ҫ��PC���÷���Ҫ֪���ڴ泤�ȣ�ֱ���ɻ�վ�������Ȳ��Ϳ���
			{
				//strInfo.Format("offset %d len %d",ntohl(cmdMsg.struCmdPara[i].s32ParaDataIndex),pstruParas[i].s32ParaVal);
				//Log(LOGDEBUG,strInfo);
				memcpy(pstruParas[i].pParaBuf, iterNameRsp2->second.u8DataArea + ntohl(cmdMsg.struCmdPara[i].s32ParaDataIndex), pstruParas[i].s32ParaVal);
			}
			break;
		}
	}
	m_NameRsp2Map.erase(iterNameRsp2);// ���

	//Log(LOGDEBUG,"SendMsg_ExecuteCmd Out");
	return TESTCASE_SUCCESS;
}

// ���͵�¼��Ϣ,���ط��ͳ���
int CWorkManager::SendMsg_LogOn(CString strProName)
{
	UAGENT_SIG_LOGON_REQ LogOnReqMsg;
	LogOnReqMsg.sigHead.msgHead.u32ServiceId = ntohl(CONTROL_SIGNAL_SERVICE_ID);
	LogOnReqMsg.sigHead.msgHead.u32MsgSize = htonl(sizeof(UAGENT_SIG_LOGON_REQ) - sizeof(UAGENT_MSG_HEAD));
	LogOnReqMsg.sigHead.u32SigType = htonl(UAGENT_SIGTYPE_LOGON_REQ);
	CString strInfo("");
	strInfo.Format("SEND UAGENT_SIG_LOGON_REQ To Proc(%s)",strProName);
	Log(LOGINFO,strInfo);
	int nResult = SendMsg(strProName,(char*)&LogOnReqMsg,sizeof(UAGENT_SIG_LOGON_REQ));
	return nResult;
}

int CWorkManager::SendMsg_ServReq(CString strProName,int nServiceId)
{
	UAGENT_SIG_SERVICE_REQ msg;
	int nSendSize = sizeof(UAGENT_SIG_SERVICE_REQ)-3; //������⣬��Ϊ�Ҽ���ɵ�
	msg.sigHead.msgHead.u32ServiceId = ntohl(CONTROL_SIGNAL_SERVICE_ID);
	msg.sigHead.msgHead.u32MsgSize = ntohl(sizeof(UAGENT_SIG_SERVICE_REQ)-3-sizeof(UAGENT_MSG_HEAD));// ��Ϊ���ֽڶ�����ɵ�
	msg.sigHead.u32SigType = ntohl(UAGENT_SIGTYPE_SERVICE_REQ);//���ṹ�������Ҫ��Ӧ����д4
	msg.u32ServiceId = ntohl(nServiceId);
	msg.u8Options = ' ';
	CString strInfo("");
	strInfo.Format("SEND UAGENT_SIG_SERVICE_REQ(%d) To Proc(%s)",nServiceId,strProName);
	Log(LOGINFO,strInfo);
	int nResult = SendMsg(strProName,(char*)&msg,nSendSize);
	return nResult;
}

int CWorkManager::SendMsg_SetTaskPrio(CString strProName,int nPrio)
{
	UAGT_CMD_SET_TASK_PRIO_REQ msg; 
	memset(&msg,0,sizeof(UAGT_CMD_SET_TASK_PRIO_REQ));
	msg.header.msgHead.u32ServiceId = ntohl(UAGT_CMD_EXECUTE_SERVICE_ID);// BUG 20171122 ��ץ��������ȶ�ȷ��
	msg.header.msgHead.u32MsgSize = ntohl(sizeof(UAGT_CMD_SET_TASK_PRIO_REQ)-sizeof(UAGENT_MSG_HEAD));
	msg.header.u32TransId = 0;//����д
	msg.u32CmdId = 0xFFFFFFFF;//Ҫ��̶�ȫF,��ʵת��ת��һ����
	msg.u32Prio = ntohl(nPrio);
	CString strInfo("");
	strInfo.Format("SEND UAGT_CMD_SET_TASK_PRIO_REQ To Proc(%s)",strProName);
	Log(LOGINFO,strInfo);
	int nResult = SendMsg(strProName,(char*)&msg,sizeof(UAGT_CMD_SET_TASK_PRIO_REQ));
	return nResult;
}

int CWorkManager::SendMsg_RestartTaskReq(CString strProName)
{
	UAGT_CMD_TASK_RESTART_REQ msg; // �����Ϣ�ֲ�Ҫ��Ϣͷ��
	memset(&msg,0,sizeof(UAGT_CMD_TASK_RESTART_REQ));
	msg.sigHead.msgHead.u32ServiceId = ntohl(UAGT_CMD_EXECUTE_SERVICE_ID);
	msg.sigHead.msgHead.u32MsgSize = ntohl(sizeof(UAGT_CMD_TASK_RESTART_REQ)-sizeof(UAGENT_MSG_HEAD));
	msg.sigHead.u32SigType = 0;//����дҪ��
	msg.u32CmdId = htonl(0xFFFFFFFE);//Ҫ��̶�ȫF
	CString strInfo("");
	strInfo.Format("SEND UAGT_CMD_TASK_RESTART_REQ To Proc(%s)",strProName);
	Log(LOGINFO,strInfo);
	int nResult = SendMsg(strProName,(char*)&msg,sizeof(UAGT_CMD_TASK_RESTART_REQ));
	return nResult;
}

int CWorkManager::SendMsg_FileTransReq(CString strProName,CString strPCPath,CString strBoardPath,CString strFileName,int nTransAttr,int nFileLen,int nTransId)
{
	CString strInfo("");

	UAGT_FILE_TRANS_REQ msg;
	memset(&msg,0,sizeof(UAGT_FILE_TRANS_REQ));
	msg.head.msgHead.u32ServiceId = ntohl(UAGT_FILE_TRANS_SERVICE_ID);
	msg.head.msgHead.u32MsgSize = ntohl(sizeof(UAGT_FILE_TRANS_REQ)- sizeof(UAGENT_MSG_HEAD));
	msg.head.u32TransId = 0;//�����ݲ���Ҫ��Ҫ��
	msg.u32TransId = ntohl(m_nFileTransId);//�и�ȫ�ֵ����ݣ�������ÿ��*2��Ӧ����Ҫ�����ֵ*2,Ҳ����������Ҫ��������
	msg.u32TransAttr = ntohl(nTransAttr);//Down 0 up 1 Downʱ�����Ҫload����2
	msg.u32FileSize = ntohl(nFileLen);
	//msg.u8ServIp //����

	// ���ӳ���У��
	if(strPCPath.GetLength()>128)
	{		
		strInfo.Format("u8RemoteFilePath Length %d > 128",strPCPath.GetLength());
		Log(LOGERROR,strInfo);
		return -1;
	}
	if(strFileName.GetLength()>64)
	{
		strInfo.Format("u8RemoteFileName Length %d > 64",strFileName.GetLength());
		Log(LOGERROR,strInfo);
		return -1;
	}
	if(strBoardPath.GetLength()>64)
	{
		strInfo.Format("u8LocalFileName Length %d > 64",strBoardPath.GetLength());
		Log(LOGERROR,strInfo);
		return -1;
	}

	memcpy(msg.u8RemoteFilePath,strPCPath.GetBuffer(),strPCPath.GetLength());
	memcpy(msg.u8RemoteFileName,strFileName.GetBuffer(),strFileName.GetLength());
	memcpy(msg.u8LocalFileName,strBoardPath.GetBuffer(),strBoardPath.GetLength());

	strInfo.Format("SEND UAGT_FILE_TRANS_REQ To Proc(%s)",strProName);
	Log(LOGINFO,strInfo);

	int nResult = SendMsg(strProName,(char*)&msg,sizeof(UAGT_FILE_TRANS_REQ));
	return nResult;
}

void CWorkManager::DelTempFile()
{
	CString strInfo("");
	CString strDelTemp("");
	strDelTemp.Format("del /F /Q \"%s%s\"",m_strTempFilePath,m_strTransName);//system("del /F /Q D:\\temp\\tempfile");	
	int nResult = system(strDelTemp.GetBuffer());
	if(0 != nResult)
	{
		int nError = GetLastError();
		strInfo.Format("system cmd fail(%s) result(%d) error(%d)",strDelTemp,nResult,nError);
		Log(LOGERROR,strInfo);
	}
}

// ����ԭ������ʵ��������˽ӿڿ������ã��������ô���
void CWorkManager::FindUseDisk()
{
	CString strTempForder[6];
	strTempForder[0] = "D:\\temp\\";
	strTempForder[1] = "C:\\temp\\";
	strTempForder[2] = "E:\\temp\\";

	strTempForder[3] = "F:\\temp\\";
	strTempForder[4] = "G:\\temp\\";
	strTempForder[5] = "H:\\temp\\";

	for(int i = 0;i<6;i++)
	{
		BOOL result = CreateDirectory(strTempForder[i].GetBuffer(),NULL);
		if(result)
		{
			m_strTempFilePath = strTempForder[i];
			return;
		}
		else
		{
			DWORD dwErr = GetLastError();
			if(ERROR_ALREADY_EXISTS == dwErr)
			{
				m_strTempFilePath = strTempForder[i];
				return;
			}
		}
	}//for
}

BOOL CWorkManager::AddrAlreadyInUse(CString strIp,int nPid)
{
	std::map<CString, CSockInfo>::iterator iter;
	for(iter = m_NameAddrMap.begin();iter!=m_NameAddrMap.end();iter++)
	{
		CSockInfo temp = iter->second;
		if((temp.strIp == strIp) && (temp.nPid == nPid))
		{
			return TRUE; // �Ѿ�ʹ��
		}
	}
	return FALSE;
}

BOOL CWorkManager::IsInfoMatch(CString strProc,CString strIp,int nPid)
{
	std::map<CString, CSockInfo>::iterator iter;
	for(iter = m_NameAddrMap.begin();iter!=m_NameAddrMap.end();iter++)
	{
		CSockInfo temp = iter->second;
		CString strName = iter->first;
		if((temp.strIp == strIp) && (temp.nPid == nPid) && (strName == strProc))
		{
			return TRUE; // ����ƥ��
		}
	}
	return FALSE;// ��һ�����ݲ�ƥ��
}