
// UPDATE.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CUPDATEApp: 
// �йش����ʵ�֣������ UPDATE.cpp
//

class CUPDATEApp : public CWinApp
{
public:
	CUPDATEApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CUPDATEApp theApp;