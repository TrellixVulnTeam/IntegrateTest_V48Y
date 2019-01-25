#pragma once

#include "TestCaseTypeDef.h"

//--------------------TCPĬ�϶˿�------------------------
//AAU BPOI SCTA SCTE
#define CONNECTTCPSERVER_PORT_OLD                      10001 
// SCTF BPOK
#define CONNECTTCPSERVER_PORT_NEW                      10021 
#define CONNECTTCPSERVER_PORT_NULL                     0 // ��������֮ǰ
/*���˽�*/ 

//---------------------���ݽṹ---------------------------
#define SERVICE_CONTROL_SIGNAL 0 // ��0��̫���ˣ��ֺ���,�����֪��ɶ��˼�������Լ�����
//#define SERVICE_DATA_SIGNAL 0// ���ݵĲ�֪����ɶ

typedef struct 
{
	u32 u32ServiceId;
	u32 u32MsgSize;
}UAGENT_MSG_HEAD; // ͨ��


typedef struct
{
	UAGENT_MSG_HEAD  msgHead;
	u32              u32SigType;
}UAGENT_SIG_HEAD;// ���� ������ܸ�������


// �������½�忨����
typedef struct
{
	UAGENT_SIG_HEAD  sigHead; /* u32SigType = 0 */ 
}UAGENT_SIG_LOGON_REQ;




/* ���ݰ�����(�ǿ�������) */
typedef struct 
{
	UAGENT_MSG_HEAD  msgHead;
	u32              u32TransId; 
}UAGENT_DATA_MSG; // ���� // �����������

typedef struct tag_STRU_CMD_EXCUTE_PARAS
{ 
	s32  s32ParaType;       /* �������ͣ���ֵ/���ݻ����� */ 
	s32  s32ParaVal;        /* ��ֵ/���ݻ���������  */ 
	s32  s32ParaDataIndex;  /* ������������ */
	s32  s32ParaLen;        /* ��������     */
}STRU_CMD_EXCUTE_PARAS;// ����Ϊ16

#define CMD_MAX_PARA_NUM 32 //���10������������Ϣ�ṹ����������32��


typedef struct
{
	UAGENT_SIG_HEAD         sigHead;//uagentͷ����4��sigtype��5 ����Ч  
	u32                     u32CmdId;
	u32                     u32CmdAttr;
	u32                     u32ParaNum;
	STRU_CMD_EXCUTE_PARAS   struCmdPara[CMD_MAX_PARA_NUM];
	u32                     u32DataAreaLen;
}UAGT_CMD_EXECUTE_REQ_HEAD;

typedef struct
{
	UAGENT_SIG_HEAD         sigHead;//uagentͷ����4��sigtype��5 ����Ч // ���Լ�����header
	u32                     u32CmdId;
	u32                     u32CmdAttr;
	u32                     u32ParaNum;
	STRU_CMD_EXCUTE_PARAS   struCmdPara[CMD_MAX_PARA_NUM];
	u32                     u32DataAreaLen;
	char                    u8DataArea[102400]; //�Ҹĵ�
}UAGT_CMD_EXECUTE_REQ; // �����������


#define UAGENT_TCP_PACKET_SIGN 0x5a //�綨��



/* ��½�忨���� */
#define UAGENT_SIGTYPE_LOGON_REQ                   0
/* ��½�忨��Ӧ */
#define UAGENT_SIGTYPE_LOGON_RSP                   1
/* ��������� */
#define UAGENT_SIGTYPE_ALIVE_REQ                   2
/* �������Ӧ */
#define UAGENT_SIGTYPE_ALIVE_RSP                   3
/* �������� */
#define UAGENT_SIGTYPE_SERVICE_REQ                 4
/* ����������Ӧ */
#define UAGENT_SIGTYPE_SERVICE_RSP                 5
/* ����Ӧ��(ACK) */
#define UAGENT_SIGTYPE_DATA_ACK                    6





// �����������ȼ�����
typedef struct 
{
	UAGENT_DATA_MSG header;//�Ҳ���
	u32                     u32CmdId; //0xFFFFFFFF
	u32                     u32Prio;
}UAGT_CMD_SET_TASK_PRIO_REQ;


// @@ serviceID ����Ҿ��þ�������ķ���
//service = UAgent_Register_Service(ipAddr, TESTCASE_SERVICE_ID, UAGENT_PROTOCOL_TCP, 1, (u8*)" ", (IMsgHandler*)pMsgHandler);

#define TESTCASE_SERVICE_ID            2 //�ű���
/* �ļ������������ */
#define UAGT_FILE_TRANS_SERVICE_ID     3 //�ļ������

#define UAGT_CMD_EXECUTE_SERVICE_ID     4 //����ִ�е�

#define CONTROL_SIGNAL_SERVICE_ID 0 // �Ҳ���0


/* �������� */
typedef struct
{
	UAGENT_SIG_HEAD  sigHead;    /* u32SigType = 4 */	//12
	u32              u32ServiceId;                      //16
	u8               u8Options; // ����һ���ո�
	u8               pad[3];//�������Ҫ
}UAGENT_SIG_SERVICE_REQ;



// ��Ϣ�ṹ
#define TESTCASE_EXECUTECMD_PARATYPE_NUMBER        1
#define TESTCASE_EXECUTECMD_PARATYPE_BUFFER        2

/* �������� */
#define UAGT_CMD_PARATYPE_NUMBER        1
#define UAGT_CMD_PARATYPE_BUFFER        2

typedef struct tag_STRU_TESTCASE_EXECUTECMD_PARAS
{ 
	s32  s32ParaType; //�������ͣ���ֵ/���ݻ�����
	s32  s32ParaVal;  // ��ֵ/���ݻ���������
	void *pParaBuf;   // ���ݻ������׵�ַ
}STRU_TESTCASE_EXECUTECMD_PARAS;


// ��Ӧ����RSP1��s32CmdExecResult
#define UAGT_CMD_RSP1_OK                    0
#define UAGT_CMD_RSP1_NO_SYMBOL            -1
#define UAGT_CMD_RSP1_NONE_EXECUTE_SYMBOL  -2
#define UAGT_CMD_RSP1_BUSY                 -3
#define UAGT_CMD_RSP1_INVALID_CMDID        -4
#define UAGT_CMD_RSP1_BUF_FULL             -5
#define UAGT_CMD_RSP1_INVALID_SIG          -6
typedef struct
{
	UAGENT_DATA_MSG         cmdHead;// �����
	u32                     u32CmdId;
	s32                     s32CmdExecResult;// ������ڿ�������һλ������վ�Ķ��壬���벻��Ӱ����һ����
	u32                     u32Addr;//
	s32                     s32Val; /* ��������Ϊ�ǿ�ִ�з���ʱ��Ч */
}UAGT_CMD_EXECUTE_RSP1;//cy:RSP1�����ǹ̶��ģ�RSP2����u8DataArea��ĳ����ǹ̶��ģ����Ȳ��ܴ����Կ��Կ�������������Ϣ����Ϊ���ǵ���Ϣͷû������

/*  ����RSP1���жϣ�
    RSP1��RSP2û�б�־λ���������Դӳ����ж�
	�����ι�ͨ��վ����u32MsgSize���㷨����վ�ǲ������ݲ��ֽṹ��+u32TransId���� = 24+4 =28
	��long long��32λҲ��8�������Ż�վ���룬��֪�����Ƕ���ĵ�����ɶ
{
u32                     u32CmdId;
s32                     s32CmdExecResult;
long long               u32Addr;
s32                     s32Val;
PADλ��
}

�����վҪ��ʹ�ú�ͷ�Ľṹ-ͷ�������Ⱦ�Ӧ��Ϊ40-8 =32,��Ϊ��ͷ��PAD��λ��һ����

{
UAGENT_DATA_MSG         cmdHead;  ������3��int����u32CmdId����һ����
u32                     u32CmdId;
s32                     s32CmdExecResult;
PADλ��
long long               u32Addr;
s32                     s32Val;
PADλ��
}
*/

#define MSGSIZE_RSP1_32BIT 20   // 16+4
#define MSGSIZE_RSP1_64BIT 28   // 24+4

typedef struct
{
	UAGENT_DATA_MSG         cmdHead;
	u32                     u32CmdId;
	s32                     s32CmdExecResult;
    u8                      u8Addr[8];
	s32                     s32Val; 
	u32                     pad;
}UAGT_CMD_EXECUTE_RSP1_64BIT; // ģ���վ�ṹ����Ҫ��u8Addr����������ת�������򿽱���long long�м���


#define UAGT_CMD_RSP2_OK                    0
#define UAGT_CMD_RSP2_TIMEOUT              -1
/* ����ִ��֮��Ļظ�����, ��������REQ���ֶԳƣ�u8DataAreaƫ����ͬ���Ա���忨�˶�����ڴ濽������ */ // ԭע�� �Ǻ�
typedef struct
{
	UAGENT_DATA_MSG         cmdHead;// �����
	u32                     u32CmdId;
	s32                     s32CmdExecResult;
	s32                     s32CmdRetVal;
	STRU_CMD_EXCUTE_PARAS   struCmdPara[32]; /* �����������壬Ϊ������REQ��ԳƵĽṹ�������� */ //�Ǻ���
	u32                     u32DataAreaLen;
	u8                      u8DataArea[4]; // �Ҹĵģ�����ʵ������ж�
}UAGT_CMD_EXECUTE_RSP2;

typedef struct
{
	UAGENT_DATA_MSG         cmdHead;// �����
	u32                     u32CmdId;
	s32                     s32CmdExecResult;
	s32                     s32CmdRetVal;
	STRU_CMD_EXCUTE_PARAS   struCmdPara[32]; /* �����������壬Ϊ������REQ��ԳƵĽṹ�������� */ //�Ǻ���
	u32                     u32DataAreaLen;
	u8                      u8DataArea[102400]; // �Ҹĵģ�����ʵ������ж�
}UAGT_CMD_EXECUTE_RSP2_FORMAP;



/* ��λ�������� */
typedef struct 
{
	UAGENT_SIG_HEAD  sigHead;//�Ҳ�ͷ
	u32                     u32CmdId; /* 0xFFFFFFFE */ // ע�⣬���������ڲ���ȫf�����һ����e
}UAGT_CMD_TASK_RESTART_REQ;

/* ��λ������Ӧ */
typedef struct 
{
	UAGENT_SIG_HEAD  sigHead;// �Ҳ�ͷ
	u32                     u32CmdId; /* 0xFFFFFFFE */ 
}UAGT_CMD_TASK_RESTART_RSP;


typedef struct
{
	UAGENT_DATA_MSG head;// �Ҳ�ͷ
	u32 u32TransId;
	u32 u32TransAttr;
	u32 u32FileSize;
	u8  u8ServIp[16];
	u8  u8RemoteFilePath[128];
	u8  u8RemoteFileName[64];
	u8  u8LocalFileName[64];		
}UAGT_FILE_TRANS_REQ;

typedef struct
{
	UAGENT_DATA_MSG head;// �Ҳ�ͷ
	u32 u32TransId;
	s32 s32TransResult;
}UAGT_FILE_TRANS_RSP;


// �Ӱ忨�ϴ�
#define UAGT_FILE_TRANS_UPLOAD         0x00000001
// ��忨����
#define UAGT_FILE_TRANS_DOWNLOAD         0x00000000 // ������Ҳ���

/* Attr: ��1��ʾ���ظ��ļ� */
#define UAGT_FILE_TRANS_LOADMODULE     0x00000002



/* s32TransResult: �ļ�����ɹ� */
#define UAGT_FILE_TRANS_OK                              0
/* s32TransResult: ����ȷǷ� */
#define UAGT_FILE_TRANS_REQ_MSGLEN_INVALID              -1
/* s32TransResult: ���������ļ�ʧ�� */
#define UAGT_FILE_TRANS_CREAT_FILE_FAIL                 -2
/* s32TransResult: ����FTP����ʧ�� */
#define UAGT_FILE_TRANS_FTPXFER_FAIL                    -3
/* s32TransResult: ���������Ӷ�ȡ���� */
#define UAGT_FILE_TRANS_READ_DATASOCK_ERROR             -4
/* s32TransResult: д�����ļ�ʧ�� */
#define UAGT_FILE_TRANS_WRITE_FILE_ERROR                -5
/* s32TransResult: �ļ�����������ֹ�����쳣 */                      
#define UAGT_FILE_TRANS_UNEXCEPT_END                    -6
/* s32TransResult: ���ظ��ļ�ʱ�������� */                      
#define UAGT_FILE_TRANS_LOADMODULE_ERROR                -7
/* s32TransResult: �򿪱����ļ�ʧ�� */                      
#define UAGT_FILE_TRANS_OPEN_LOCALFILE_FAIL             -8
/* s32TransResult: ��ȡ�����ļ�ʧ�� */                      
#define UAGT_FILE_TRANS_READ_LOCALFILE_FAIL             -9
/* s32TransResult: д�뵽�������Ӵ��� */                      
#define UAGT_FILE_TRANS_WRITE_DATASOCK_ERROR            -10


// ����ָ��
typedef void (*TESTCASE_OUTPUT_FUNC)(u8* pu8OutputMsg, s32 s32OutputMsgType, s32 s32DebugLevel);
#define WAIT_RSP_DOWNLOAD  10*60*1000 // ԭ���е�10����
// #define WAIT_RSP_UPLOAD    5*60*1000 // ��5����  ����ʱ�����ƺ󣬸�ֵ����ʹ��

// ��־����
#define SHOW_LEVEL_MINOR         0 // ��Ҫ��
#define SHOW_LEVEL_MAJOR         1 // ��Ҫ��
#define SHOW_LEVEL_CRITICAL      2 // ���ص�
#define SHOW_LEVEL_CATASTROPHIC  3 // �����Ե�

//����----------------------------------------------------
#define MY_LOG_MAX_LENGTH 1024*1024*5 //10M��ʵҲ�� 
#define WAIT_RSP_TIMEOUT  30*1000// ��Ϣ�ĳ�ʱʱ�䣬��ͳһ�� RSP2��ĵ�6s������RSP1�ɿ죬�����ǻظ��ĵط���һ����RSP2��ĺ���



// RSP�ĳ�ʱʱ��֧���û�����
// FTP�ĵȴ�ʱ��ܳ�

typedef enum
{
	LOGDEBUG,
	LOGINFO,
	LOGERROR
}LOGENUM;

#define CONNECTTCPSERVER_RETRY_NUMBER 10 //��������,������һ�㣬��Ϊ����ȷʵ�������ϵ������һ���ֺ���
#define CONNECTTCPSERVER_RETRY_WAITTIME 1000 // �����ȴ�ʱ�䣺��λ���룬����Sleep


#define BOARD_MAX_MCU 300 
#define BOARD_SEND_MCU 260
