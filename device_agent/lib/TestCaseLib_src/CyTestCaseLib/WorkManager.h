#pragma once

#include <WinSock2.h>
#include <map>
#include "FunctionDefine.h"
#include "MessageFormat.h"
#include "RecvPacketModel.h"
#include "ws2ipdef.h"

class CSockInfo
{
public:
	CSockInfo()
	{
		nPid = -1;// ����ԭ�ӿڴ�ֵΪ-1
	}

	CString strIp;
	int nPid;
	int nPort;
	SOCKET sock;

	void SetPid(int nPidPara)
	{
		if(nPidPara<0)
		{
			nPid = -1;
			nPort = CONNECTTCPSERVER_PORT_NULL;
		}
		else
		{
			nPid = nPidPara;
			nPort = CONNECTTCPSERVER_PORT_OLD + 10*nPid;
		}
	}
};

class CWorkManager
{
public:
	CWorkManager(void);
	~CWorkManager(void);
	int TestCaseLibInit();
	// 2018-5-9 Start
	//int TestCaseLibRegisterProc(char* csProName,char* csIP);
	//int TestCaseLibUnRegisterProc(char* csProName,char* csIP);
	int TestCaseLibRegisterProc(char* csProName,char* csIP,s32 s32Pid, s32 s32SlutNum=0, s32 s32ProcId=0);
	int TestCaseLibUnRegisterProc(char* csProName,char* csIP, s32 s32Pid);

	int TestCaseLibExcuteCmd(char* csProName,char* csFunction,STRU_TESTCASE_EXECUTECMD_PARAS *pstruParas,int nParaNum,int *pnRetVal,unsigned int *pu32SymbolAddr);
	int TestCaseLibSetTaskPrio(char* csProName,int nPrio);
	int TestCaseLibSetCmdTimeOut(u32 u32CmdTimeOutSec);// ����Ϊ�� RSP1 RSP2
	int TestCaseLibSetFtpTimeOut(u32 u32CmdTimeOutSec);// ����Ϊ�� FTP
	int TestCaseLibRestartExecuteTask(char* csProcName);
	int TestCaseLibDownloadFile(char* csProcName, char* csPCFilePath, char* csBoardFilePath, int nLoad,int nInflate);
	int TestCaseLibUploadFile(char* csProcName, char* csPCFilePath, char* csBoardFilePath);
	int TestCaseLibRegisterOutputFunc(TESTCASE_OUTPUT_FUNC pTestCaseOutputFunc);
	// �����ӵ���ʱ�ӿ�
	void TestCaseConfigTempfilePath(char* csTempfilePath);

private:

	TESTCASE_OUTPUT_FUNC g_pTestCaseOutputFunc;

	// �����еĴ���������
	//std::map<CString,CString> m_IpNameMap;//������IP������
	// 2018-5-10 Strat
	std::map<CString, CSockInfo> m_NameAddrMap;//<���������֣���ַ��Ϣ��>
	//std::map<CString,SOCKET> m_NameSockMap;//���������ֺ�socket

	// ���������������
	std::map<CString,CRecvPacketModel> m_NameDataMap;//<���������֣�������Ϣ>

	// ��Ӧ���ظ�����
	std::map<CString,UAGT_CMD_EXECUTE_RSP1> m_NameRsp1Map;//�������erase��ʵʱ���¾Ϳ���
	std::map<CString,UAGT_CMD_EXECUTE_RSP2_FORMAP> m_NameRsp2Map;// ����erase
	std::map<CString,UAGT_FILE_TRANS_RSP> m_NameFileTransRspMap;

	// ֪ͨ���¼�
	//HANDLE m_hLogonRspEvent;// logon �� �������ȼ���rsp,û�����֣�ȫ��20����Ϣ��12��sigtypeΪ1��cmdId��ȫf 
	                          //AAU��������͵�¼������,��ȫ����������Ϊһ��
	//HANDLE m_hServiceRspEvent;
	HANDLE m_hSigRspEvent;
    HANDLE m_hTaskRspEvent;// �����������ȼ�����Ļ�Ӧ��һ�� chaiye 20171122

	HANDLE m_hRsp1Event; // ��ʵ��ѭ���ж�mapҲ���ԣ������ϲ�����ֳ�ͻ
	HANDLE m_hRsp2Event;
	HANDLE m_hFileTransEvent;//�ļ����� 

	BOOL m_b64Bit; // ��վ�Ƿ���64λϵͳ

	FILE *m_pLog;

	HANDLE m_hRecvHandle;

	int m_nWaitTimeMSec; // ����  �ȴ�RSP1 �ȴ�RSP2
	int m_nWaitTimeMSecForFtp;// ����, �ȴ�FTP�ϴ��������أ�Ĭ��ֵΪ WAIT_RSP_DOWNLOAD 10����

	CString m_strTempFilePath;// ��־��ź���ʱ�ļ���ŵ�λ��,����β��\\��;
	CString m_strTransName;//����Ϣ���ǹ̶����ļ���
	unsigned int m_nFileTransId;// �ļ�������������Ϣ���

	CString m_strLogName;

	int SockInit();
	BOOL LogInit();
	BOOL MemberInit();// ��Ա���
	BOOL RecvThreadStart();
	static DWORD WINAPI RecvThread(LPVOID lpParameter);	

	// ��Ϊ�ǵȴ��ģ������첽�ģ������Լ�ʹ����忨Ҳ������ִ�����ͬһʱ��ֻ����һ���ڵ���Ϣ
	// ��ʱ�յ�����Ϣֻ��������忨��

	// ����IP���ɹ��򷵻�socket��ʧ�ܷ���INVALID_SOCKET
	// 2018-5-9 ClientConnect���Ӳ���nPort,nPortΪ0��CONNECTTCPSERVER_PORT_NULL�����򻹰�ԭ���ķ�ʽ�������˿��ֻ��ظ����ӣ���nPort��0,��ֻ����nPortָ���˿�
	SOCKET ClientConnect(CString strIP,int nPort, s32 s32SlotNum=0, s32 s32ProcId=0);
	BOOL ClientClose(SOCKET nSock);

    // ������Ϣ
	int SendMsg(CString strProName,char* pMsg, int nSendSize);
	// ������Ϣ
	void DealRecvBuff(CString strProcName,char* pData,int nDataLen);
	int DealMsg(CString strProcName,UAGENT_SIG_HEAD* pMsg);
	void RecvMsg();

	void Log(LOGENUM type,CString strInfo);
	void LogReset();// �ļ���С�жϣ���ֹ��־����
	
    int SendMsg_LogOn(CString strProName);
    int SendMsg_ServReq(CString strProName,int nServiceId);// �����󡪡�����ִ������
    int SendMsg_SetTaskPrio(CString strProName,int nPrio);
	// ֻ��ExecuteCmd�Ǻ��ȴ�����ģ������ľ������洦��
	int SendMsg_ExecuteCmd(CString strProName,CString strFunction,STRU_TESTCASE_EXECUTECMD_PARAS *pstruParas,int nParaNum,int *pnRetVal,unsigned int *pu32SymbolAddr);
	int SendMsg_RestartTaskReq(CString strProName);
	// �������ء��ϴ��Ĳ�ͬ��д
	int SendMsg_FileTransReq(CString strProName,CString strPCPath,CString strBoardPath,CString strFileName,int nTransAttr,int nFileLen,int nTransId);

	// ɾ����ʱ�ļ�
	void DelTempFile();

	// �ٽ���
	CRITICAL_SECTION m_cs; 

	// Ϊ����֮ǰ���ʹ�ã������������ȷ����D�̻���C�̻���E�̣���Ȼ����Ҳ֧������
	void FindUseDisk();

	// 2018-5-10 ����ַ�Ƿ��Ѿ�ʹ��
	BOOL AddrAlreadyInUse(CString strIp,int nPid);
	// ��Ϣ�Ƿ�ƥ��
	BOOL IsInfoMatch(CString strProc,CString strIp,int nPid);
};
