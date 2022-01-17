
// OwnClient.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// COwnClientApp:
// See OwnClient.cpp for the implementation of this class
//

typedef struct _PARAM_
{
	char	*szHost;
	int		nPort;
	int		nX;
	int		nY;
	int		nWidth;
	int		nHeight;
} PARAM;

class COwnClientApp : public CWinApp
{
public:
	COwnClientApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
	PARAM	m_param;
	void ParseCommandLine(CCommandLineInfo& rCmdInfo);

	DECLARE_MESSAGE_MAP()
};

extern COwnClientApp theApp;
