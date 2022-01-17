
// OwnServer.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// COwnServerApp:
// See OwnServer.cpp for the implementation of this class
//

typedef struct _PARAM_
{
	int		nPort;
	int		nWidth;
	int		nHeight;
	char* szPassword;
} PARAM;

class COwnServerApp : public CWinApp
{
public:
	COwnServerApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
	PARAM	m_param;
	void ParseCommandLine(CCommandLineInfo& rCmdInfo);

	DECLARE_MESSAGE_MAP()
};

extern COwnServerApp theApp;
