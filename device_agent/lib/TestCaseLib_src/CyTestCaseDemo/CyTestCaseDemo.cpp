// CyTestCaseDemo.cpp : ����Ӧ�ó��������Ϊ��
//

#include "stdafx.h"
#include "CyTestCaseDemo.h"
#include "CyTestCaseDemoDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CCyTestCaseDemoApp

BEGIN_MESSAGE_MAP(CCyTestCaseDemoApp, CWinApp)
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()


// CCyTestCaseDemoApp ����

CCyTestCaseDemoApp::CCyTestCaseDemoApp()
{
}


// Ψһ��һ�� CCyTestCaseDemoApp ����

CCyTestCaseDemoApp theApp;


// CCyTestCaseDemoApp ��ʼ��

BOOL CCyTestCaseDemoApp::InitInstance()
{
	// ���һ�������� Windows XP �ϵ�Ӧ�ó����嵥ָ��Ҫ
	// ʹ�� ComCtl32.dll �汾 6 ����߰汾�����ÿ��ӻ���ʽ��
	//����Ҫ InitCommonControls()�����򣬽��޷��������ڡ�
	InitCommonControls();

	CWinApp::InitInstance();

	AfxEnableControlContainer();
	SetRegistryKey(_T("Ӧ�ó��������ɵı���Ӧ�ó���"));

	CCyTestCaseDemoDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		
	}
	else if (nResponse == IDCANCEL)
	{
		
	}

	// ���ڶԻ����ѹرգ����Խ����� FALSE �Ա��˳�Ӧ�ó���
	// ����������Ӧ�ó������Ϣ�á�
	return FALSE;
}
