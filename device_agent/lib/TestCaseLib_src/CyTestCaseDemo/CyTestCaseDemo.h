// CyTestCaseDemo.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error �ڰ������� PCH �Ĵ��ļ�֮ǰ������stdafx.h��
#endif

#include "resource.h"		// ������


// CCyTestCaseDemoApp:
// �йش����ʵ�֣������ CyTestCaseDemo.cpp
//

class CCyTestCaseDemoApp : public CWinApp
{
public:
	CCyTestCaseDemoApp();

// ��д
	public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CCyTestCaseDemoApp theApp;
