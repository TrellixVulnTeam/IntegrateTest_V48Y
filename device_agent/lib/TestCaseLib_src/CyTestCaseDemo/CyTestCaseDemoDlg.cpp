// CyTestCaseDemoDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "CyTestCaseDemo.h"
#include "CyTestCaseDemoDlg.h"
#include ".\cytestcasedemodlg.h"

#include "..\include\TestCaseFuncs.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CCyTestCaseDemoDlg �Ի���



CCyTestCaseDemoDlg::CCyTestCaseDemoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCyTestCaseDemoDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CCyTestCaseDemoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CCyTestCaseDemoDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON4, OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON5, OnBnClickedButton5)
	ON_BN_CLICKED(IDC_BUTTON6, OnBnClickedButton6)
	ON_BN_CLICKED(IDC_BUTTON7, OnBnClickedButton7)
	ON_BN_CLICKED(IDC_BUTTON8, OnBnClickedButton8)
END_MESSAGE_MAP()


// CCyTestCaseDemoDlg ��Ϣ�������

BOOL CCyTestCaseDemoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ��\������...\���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	
	ContentInit();
	return TRUE;  // ���������˿ؼ��Ľ��㣬���򷵻� TRUE
}

void CCyTestCaseDemoDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CCyTestCaseDemoDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ��������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù����ʾ��
HCURSOR CCyTestCaseDemoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CCyTestCaseDemoDlg::OnBnClickedOk()
{
	CString strProcName("");
	CString strCmd("");
	CString strIp("");
	CString strPid("");
	GetDlgItem(IDC_EDIT_NAME)->GetWindowText(strProcName);
	GetDlgItem(IDC_EDIT_CMD)->GetWindowText(strCmd);
	GetDlgItem(IDC_IPADDRESS1)->GetWindowText(strIp);
	GetDlgItem(IDC_EDIT_PID)->GetWindowText(strPid);
	ExecuteCmd(strProcName,strIp,strCmd,strPid);
	//OnOK();//���˳�
}

// ���ýӿڲ���
void CCyTestCaseDemoDlg::ExecuteCmd(CString strProcName,CString strIp,CString strCmd,CString strPid)
{
	int result = TestCase_Init();
	CString t1("");
	t1.Format("TestCase_Init:%d\n",result);
	TRACE(t1);

	int nPid = atoi(strPid);
	result = TestCase_RegisterTargetProc_Pid((const u8*)strProcName.GetBuffer(),(const u8*)strIp.GetBuffer(),nPid);
	CString t2("");
	t2.Format("TestCase_RegisterTargetProc_Ex:%d\n",result);
	TRACE(t2);

	int s32RetVal = 0;
	//strCmd = "fdd_hal_epld_write 0x16,0x4c";
    result = TestCase_ExcuteCmd((const u8*)strProcName.GetBuffer(),(const u8*)strCmd.GetBuffer(),NULL,0,&s32RetVal);
	CString t3("");
	t3.Format("TestCase_ExcuteCmd:return %d  value %d\n",result,s32RetVal);
	TRACE(t3);

	result = TestCase_UnregisterTargetProc_Pid((const u8*)strProcName.GetBuffer(),(const u8*)strIp.GetBuffer(),nPid);
	CString t4("");
	t4.Format("TestCase_UnregisterTargetProc:%d\n",result);
	TRACE(t4);
}

// ������Ϣ��ʼ��
void CCyTestCaseDemoDlg::ContentInit()
{
	GetDlgItem(IDC_EDIT_NAME)->SetWindowText("SCTF");
	GetDlgItem(IDC_IPADDRESS1)->SetWindowText("");
	GetDlgItem(IDC_EDIT_CMD)->SetWindowText("");//"fdd_hal_epld_read"
	GetDlgItem(IDC_EDIT_PCPATH)->SetWindowText("D:\\ChaiyeTestCase\\a.so"); // ���Ҳ���
	GetDlgItem(IDC_EDIT_BOARDPATH)->SetWindowText("/ramDisk/a.so"); // ������/ �����\\������ok����û�зŵ�ramDisk��
}

// ѭ��ִ��
void CCyTestCaseDemoDlg::OnBnClickedButton1()
{
	CString strNum("");
	GetDlgItem(IDC_EDIT_NUM)->GetWindowText(strNum);
	int nNum = atoi(strNum.GetBuffer());

	CString strProcName("");
	CString strCmd("");
	CString strIp("");
	GetDlgItem(IDC_EDIT_NAME)->GetWindowText(strProcName);
	GetDlgItem(IDC_EDIT_CMD)->GetWindowText(strCmd);
	GetDlgItem(IDC_IPADDRESS1)->GetWindowText(strIp);

	int result = TestCase_Init();
	CString t1("");
	t1.Format("TestCase_Init:%d\n",result);
	TRACE(t1);

	for(int i = 0;i<nNum;i++)
	{
		Sleep(1000);//�ҵĲ��Գ���
		TRACE("\n\n\n%d\n",i);
		result = TestCase_RegisterTargetProc_Ex((const u8*)strProcName.GetBuffer(),(const u8*)strIp.GetBuffer());
		CString t2("");
		t2.Format("TestCase_RegisterTargetProc_Ex:%d\n",result);
		TRACE(t2);

		int s32RetVal = 0;
		//strCmd = "fdd_hal_epld_write 0x16,0x4c";
		result = TestCase_ExcuteCmd((const u8*)strProcName.GetBuffer(),(const u8*)strCmd.GetBuffer(),NULL,0,&s32RetVal);
		CString t3("");
		t3.Format("TestCase_ExcuteCmd:return %d  value %d\n",result,s32RetVal);
		TRACE(t3);

		result = TestCase_UnregisterTargetProc((const u8*)strProcName.GetBuffer(),(const u8*)strIp.GetBuffer());
		CString t4("");
		t4.Format("TestCase_UnregisterTargetProc:%d\n",result);
		TRACE(t4);
	}
}

void CCyTestCaseDemoDlg::OnBnClickedButton2()
{
	CString strProcName("");
	CString strIp("");
	GetDlgItem(IDC_EDIT_NAME)->GetWindowText(strProcName);
	GetDlgItem(IDC_IPADDRESS1)->GetWindowText(strIp);

	int result = TestCase_Init();
	CString t1("");
	t1.Format("TestCase_Init:%d\n",result);
	TRACE(t1);

	result = TestCase_RegisterTargetProc_Ex((const u8*)strProcName.GetBuffer(),(const u8*)strIp.GetBuffer());
	CString t2("");
	t2.Format("TestCase_RegisterTargetProc_Ex:%d\n",result);
	TRACE(t2);

	result = TestCase_SetExecuteTaskPrio((const u8*)strProcName.GetBuffer(),52);
	CString t3("");
	t3.Format("TestCase_SetExecuteTaskPrio:%d\n",result);
	TRACE(t3);

	result = TestCase_UnregisterTargetProc((const u8*)strProcName.GetBuffer(),(const u8*)strIp.GetBuffer());
	CString t4("");
	t4.Format("TestCase_UnregisterTargetProc:%d\n",result);
	TRACE(t4);
	
}

// ģ�������忨����
void CCyTestCaseDemoDlg::OnBnClickedButton3()
{
	CString strPro1("SCTF");
	CString strPro2("BPOI");//246.6
	CString strIP1("172.27.245.92");
	CString strIP2("172.27.246.6");
	CString strCmd("i");

	int result = TestCase_Init();
	CString t("");
	t.Format("TestCase_Init:%d\n",result);
	TRACE(t);

	result = TestCase_RegisterTargetProc_Ex((const u8*)strPro1.GetBuffer(),(const u8*)strIP1.GetBuffer());
	t.Format("TestCase_RegisterTargetProc_Ex:%d\n",result);
	TRACE(t);

	result = TestCase_RegisterTargetProc_Ex((const u8*)strPro2.GetBuffer(),(const u8*)strIP2.GetBuffer());
	t.Format("TestCase_RegisterTargetProc_Ex:%d\n",result);
	TRACE(t);

	/*result = TestCase_SetExecuteTaskPrio((const u8*)strProcName.GetBuffer(),52);
	CString t3("");
	t3.Format("TestCase_SetExecuteTaskPrio:%d\n",result);
	TRACE(t3);*/

	result = TestCase_UnregisterTargetProc((const u8*)strPro1.GetBuffer(),(const u8*)strIP1.GetBuffer());
	t.Format("TestCase_UnregisterTargetProc:%d\n",result);
	TRACE(t);

	result = TestCase_UnregisterTargetProc((const u8*)strPro2.GetBuffer(),(const u8*)strIP2.GetBuffer());
	t.Format("TestCase_UnregisterTargetProc:%d\n",result);
	TRACE(t);
}

// �ϴ�����
void CCyTestCaseDemoDlg::OnBnClickedButton4()
{
	CString strPath("");
	GetDlgItem(IDC_EDIT_PCPATH )->GetWindowText(strPath);
	CString strBoardPath("");
	GetDlgItem(IDC_EDIT_BOARDPATH )->GetWindowText(strBoardPath);

	CString strProcName("");
	CString strIp("");
	GetDlgItem(IDC_EDIT_NAME)->GetWindowText(strProcName);
	GetDlgItem(IDC_IPADDRESS1)->GetWindowText(strIp);

	

	int result = TestCase_Init();
	CString t("");
	t.Format("TestCase_Init:%d\n",result);
	TRACE(t);

	result = TestCase_RegisterTargetProc_Ex((const u8*)strProcName.GetBuffer(),(const u8*)strIp.GetBuffer());
	t.Format("TestCase_RegisterTargetProc_Ex:%d\n",result);
	TRACE(t);

	result = TestCase_UploadFile((const u8*)strProcName.GetBuffer(),(const u8*)strPath.GetBuffer(),(const u8*)strBoardPath.GetBuffer());
	t.Format("TestCase_UploadFile:%d\n",result);
	TRACE(t);

	result = TestCase_UnregisterTargetProc((const u8*)strProcName.GetBuffer(),(const u8*)strIp.GetBuffer());
	t.Format("TestCase_UnregisterTargetProc:%d\n",result);
	TRACE(t);

}

// ���ز���
void CCyTestCaseDemoDlg::OnBnClickedButton5()
{
	CString strPath("");
	GetDlgItem(IDC_EDIT_PCPATH )->GetWindowText(strPath);
	CString strBoardPath("");
	GetDlgItem(IDC_EDIT_BOARDPATH )->GetWindowText(strBoardPath);

	CString strProcName("");
	CString strIp("");
	GetDlgItem(IDC_EDIT_NAME)->GetWindowText(strProcName);
	GetDlgItem(IDC_IPADDRESS1)->GetWindowText(strIp);

	int result = TestCase_Init();
	CString t("");
	t.Format("TestCase_Init:%d\n",result);
	TRACE(t);

	result = TestCase_RegisterTargetProc_Ex((const u8*)strProcName.GetBuffer(),(const u8*)strIp.GetBuffer());
	t.Format("TestCase_RegisterTargetProc_Ex:%d\n",result);
	TRACE(t);

	result = TestCase_DownloadFile((const u8*)strProcName.GetBuffer(),(const u8*)strPath.GetBuffer(),(const u8*)strBoardPath.GetBuffer(),0,0);
	t.Format("TestCase_DownloadFile:%d\n",result);
	TRACE(t);

	result = TestCase_UnregisterTargetProc((const u8*)strProcName.GetBuffer(),(const u8*)strIp.GetBuffer());
	t.Format("TestCase_UnregisterTargetProc:%d\n",result);
	TRACE(t);
	
}

//
//  AAU��Eeprom
//
void CCyTestCaseDemoDlg::OnBnClickedButton6()
{
	CString strProcName("AIU");
	CString strIp("172.27.45.250");
	CString strCmd("Product_Test_Rru_Read_Eeprom");
	int nParaNum = 5;

	int result = TestCase_Init();
	CString t("");
	t.Format("TestCase_Init:%d\n",result);
	TRACE(t);

	result = TestCase_RegisterTargetProc_Ex((const u8*)strProcName.GetBuffer(),(const u8*)strIp.GetBuffer());
	t.Format("TestCase_RegisterTargetProc_Ex:%d\n",result);
	TRACE(t);

	int nRetVal = 0;

	

    STRU_TESTCASE_EXECUTECMD_PARAS para[32];
	memset(para,0,sizeof(STRU_TESTCASE_EXECUTECMD_PARAS)*32);
	//#define TESTCASE_EXECUTECMD_PARATYPE_NUMBER        1
    //#define TESTCASE_EXECUTECMD_PARATYPE_BUFFER        2



	char buff1[4096];
	char buff2[4096];
	char buff3[4096];
	char buff4[4096];
	memset(buff1,0,4096);
	memset(buff2,0,4096);
	memset(buff3,0,4096);
	memset(buff4,0,4096);

	/////////////////////////////////////////////////////////////////
	// ��һ��
	para[0].s32ParaType = TESTCASE_EXECUTECMD_PARATYPE_NUMBER;
	para[0].s32ParaVal = 99;

	para[1].s32ParaType = TESTCASE_EXECUTECMD_PARATYPE_NUMBER;
	para[1].s32ParaVal = 0;

	para[2].s32ParaType = TESTCASE_EXECUTECMD_PARATYPE_NUMBER;
	para[2].s32ParaVal = 0;

	para[3].s32ParaType = TESTCASE_EXECUTECMD_PARATYPE_NUMBER;
	para[3].s32ParaVal = 4096;

	para[4].s32ParaType = TESTCASE_EXECUTECMD_PARATYPE_BUFFER;
	para[4].s32ParaVal = 4096;
	para[4].pParaBuf = buff1;
	
	result = TestCase_ExcuteCmd((const u8*)strProcName.GetBuffer(),(const u8*)strCmd.GetBuffer(),para,nParaNum,&nRetVal);
	t.Format("TestCase_ExcuteCmd:%d\n",result);
	TRACE(t);

	/////////////////////////////////////////////////////////////////
	// �ڶ���
	para[0].s32ParaType = TESTCASE_EXECUTECMD_PARATYPE_NUMBER;
	para[0].s32ParaVal = 99;

	para[1].s32ParaType = TESTCASE_EXECUTECMD_PARATYPE_NUMBER;
	para[1].s32ParaVal = 0;

	para[2].s32ParaType = TESTCASE_EXECUTECMD_PARATYPE_NUMBER;
	para[2].s32ParaVal = 4096*1;

	para[3].s32ParaType = TESTCASE_EXECUTECMD_PARATYPE_NUMBER;
	para[3].s32ParaVal = 4096;

	para[4].s32ParaType = TESTCASE_EXECUTECMD_PARATYPE_BUFFER;
	para[4].s32ParaVal = 4096;
	para[4].pParaBuf = buff2;

	result = TestCase_ExcuteCmd((const u8*)strProcName.GetBuffer(),(const u8*)strCmd.GetBuffer(),para,nParaNum,&nRetVal);
	t.Format("TestCase_ExcuteCmd:%d\n",result);
	TRACE(t);

	/////////////////////////////////////////////////////////////////
	// ������
	para[0].s32ParaType = TESTCASE_EXECUTECMD_PARATYPE_NUMBER;
	para[0].s32ParaVal = 99;

	para[1].s32ParaType = TESTCASE_EXECUTECMD_PARATYPE_NUMBER;
	para[1].s32ParaVal = 0;

	para[2].s32ParaType = TESTCASE_EXECUTECMD_PARATYPE_NUMBER;
	para[2].s32ParaVal = 4096*2;

	para[3].s32ParaType = TESTCASE_EXECUTECMD_PARATYPE_NUMBER;
	para[3].s32ParaVal = 4096;

	para[4].s32ParaType = TESTCASE_EXECUTECMD_PARATYPE_BUFFER;
	para[4].s32ParaVal = 4096;
	para[4].pParaBuf = buff3;

	result = TestCase_ExcuteCmd((const u8*)strProcName.GetBuffer(),(const u8*)strCmd.GetBuffer(),para,nParaNum,&nRetVal);
	t.Format("TestCase_ExcuteCmd:%d\n",result);
	TRACE(t);

	/////////////////////////////////////////////////////////////////
	// ���Ĵ�
	para[0].s32ParaType = TESTCASE_EXECUTECMD_PARATYPE_NUMBER;
	para[0].s32ParaVal = 99;

	para[1].s32ParaType = TESTCASE_EXECUTECMD_PARATYPE_NUMBER;
	para[1].s32ParaVal = 0;

	para[2].s32ParaType = TESTCASE_EXECUTECMD_PARATYPE_NUMBER;
	para[2].s32ParaVal = 4096*3;

	para[3].s32ParaType = TESTCASE_EXECUTECMD_PARATYPE_NUMBER;
	para[3].s32ParaVal = 4096;

	para[4].s32ParaType = TESTCASE_EXECUTECMD_PARATYPE_BUFFER;
	para[4].s32ParaVal = 4096;
	para[4].pParaBuf = buff4;

	result = TestCase_ExcuteCmd((const u8*)strProcName.GetBuffer(),(const u8*)strCmd.GetBuffer(),para,nParaNum,&nRetVal);
	t.Format("TestCase_ExcuteCmd:%d\n",result);
	TRACE(t);


	result = TestCase_UnregisterTargetProc((const u8*)strProcName.GetBuffer(),(const u8*)strIp.GetBuffer());
	t.Format("TestCase_UnregisterTargetProc:%d\n",result);
	TRACE(t);
}

// ����CMD����
void CCyTestCaseDemoDlg::OnBnClickedButton7()
{
	CString strProcName("SCTF");
	CString strIp("172.27.245.92");
	CString strCmd("cmd");
	

	int result = TestCase_Init();
	CString t("");
	t.Format("TestCase_Init:%d\n",result);
	TRACE(t);

	result = TestCase_RegisterTargetProc_Ex((const u8*)strProcName.GetBuffer(),(const u8*)strIp.GetBuffer());
	t.Format("TestCase_RegisterTargetProc_Ex:%d\n",result);
	TRACE(t);

	int nRetVal = 0;



	STRU_TESTCASE_EXECUTECMD_PARAS para[32];
	memset(para,0,sizeof(STRU_TESTCASE_EXECUTECMD_PARAS)*32);
	//#define TESTCASE_EXECUTECMD_PARATYPE_NUMBER        1
	//#define TESTCASE_EXECUTECMD_PARATYPE_BUFFER        2



	char buff1[4096];
	memset(buff1,0,4096);
	
	//CString strTemp("ls /flashDev");
	CString strTemp("reboot");
	//CString strTemp("./firmware/upgrade_bios/firmware/spi_image_HV5.bin.");//��������Ҿ������Ǹ���д����
	strcat(buff1,strTemp.GetBuffer());


	para[0].s32ParaType = TESTCASE_EXECUTECMD_PARATYPE_BUFFER;
	para[0].s32ParaVal = strTemp.GetLength();
	para[0].pParaBuf = buff1;

    int nParaNum = 1;
	result = TestCase_ExcuteCmd((const u8*)strProcName.GetBuffer(),(const u8*)strCmd.GetBuffer(),para,nParaNum,&nRetVal);
	t.Format("TestCase_ExcuteCmd:%d\n",result);
	TRACE(t);

	result = TestCase_UnregisterTargetProc((const u8*)strProcName.GetBuffer(),(const u8*)strIp.GetBuffer());
	t.Format("TestCase_UnregisterTargetProc:%d\n",result);
	TRACE(t);
}

// ��̬���ز���  ���� Bsp_Get_BoardType
typedef int(*lpFun_TestCase_Init)(void);
typedef int(*lpFun_TestCase_RegisterTargetProc_Ex)(const unsigned char*, const unsigned char*);
typedef int(*lpFun_TestCase_ExcuteCmd)(const unsigned char*pu8ProcName, const unsigned char*pu8Funcname,STRU_TESTCASE_EXECUTECMD_PARAS *pstruParas,unsigned int u32ParaNum,int *ps32RetVal);

void CCyTestCaseDemoDlg::OnBnClickedButton8()
{
	HMODULE hModule = LoadLibrary("TestCaseLib.dll");
	lpFun_TestCase_Init fun_TestInit = (lpFun_TestCase_Init)GetProcAddress(hModule,"TestCase_Init");
	int nResult = fun_TestInit();
	TRACE("TestCase_Init %d\n",nResult);

	lpFun_TestCase_RegisterTargetProc_Ex fun_TestCase_RegisterTargetProc_Ex = (lpFun_TestCase_RegisterTargetProc_Ex)GetProcAddress(hModule,"TestCase_RegisterTargetProc_Ex");
	nResult = fun_TestCase_RegisterTargetProc_Ex((const unsigned char*)"SCTF",(const unsigned char*)"2001::11:5");
	TRACE("TestCase_RegisterTargetProc_Ex %d\n",nResult);

	lpFun_TestCase_ExcuteCmd fun_TestCase_ExcuteCmd = (lpFun_TestCase_ExcuteCmd)GetProcAddress(hModule,"TestCase_ExcuteCmd");
	int s32RetVal = 0;
	int* pType = new int;
	*pType = 0;
	STRU_TESTCASE_EXECUTECMD_PARAS para[32];
	memset(para,0,sizeof(STRU_TESTCASE_EXECUTECMD_PARAS)*32);
	para[0].s32ParaType = 2;//TESTCASE_EXECUTECMD_PARATYPE_BUFFER;
	para[0].s32ParaVal = sizeof(int);
	para[0].pParaBuf = (void*)pType; 
	nResult = fun_TestCase_ExcuteCmd((const unsigned char*)"SCTF",(const unsigned char*)"Bsp_Get_BoardType",para,1,&s32RetVal);
	TRACE("TestCase_ExcuteCmd %d\n",nResult);

	TRACE("END\n");
}

// ���õĲ�������
// Bsp_Monitor  �޲�����OspStudio�Ͽɼ�Ӳ����Ϣ��ӡ
// Bsp_Reboot  ��λ�汾����Щ�忨��֧�֣����ǿ�����OspStuido�Ͽ�������ԭ��
