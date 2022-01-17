
// OwnClient.cpp : Defines the class behaviors for the application.
//

#include "pch.h"
#include "framework.h"
#include "OwnClient.h"
#include "OwnClientDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// COwnClientApp

BEGIN_MESSAGE_MAP(COwnClientApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// COwnClientApp construction

COwnClientApp::COwnClientApp()
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only COwnClientApp object

COwnClientApp theApp;


// COwnClientApp initialization

BOOL COwnClientApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	// Create the shell manager, in case the dialog contains
	// any shell tree view or shell list view controls.
	CShellManager *pShellManager = new CShellManager;

	// Activate "Windows Native" visual manager for enabling themes in MFC controls
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	if (m_param.nX == -1 && m_param.nY == -1)
	{
		return FALSE;
	}

	COwnClientDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "Warning: dialog creation failed, so application is terminating unexpectedly.\n");
		TRACE(traceAppMsg, 0, "Warning: if you are using MFC controls on the dialog, you cannot #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS.\n");
	}

	// Delete the shell manager created above.
	if (pShellManager != nullptr)
	{
		delete pShellManager;
	}

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
	ControlBarCleanUp();
#endif

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

void COwnClientApp::ParseCommandLine(CCommandLineInfo & rCmdInfo)
{
	COwnClientApp* pApp = (COwnClientApp*)AfxGetApp();
	if (__argc != 8)
	{
		MessageBox(NULL, L"Runtime error : CS 22367", L"Error", MB_OK);
		// AfxMessageBox(szTemp);

		pApp->m_param.nPort = 5900;
		pApp->m_param.nX = -1;
		pApp->m_param.nY = -1;
		pApp->m_param.nWidth = -1;
		pApp->m_param.nHeight = -1;

		return;
	}

	pApp->m_param.szHost = (char*)malloc(MAX_PATH);
	sprintf(pApp->m_param.szHost, "%S", __targv[1]);
	pApp->m_param.nPort = _ttoi(__targv[2]);

	pApp->m_param.nX = _ttoi(__targv[3]);
	pApp->m_param.nY = _ttoi(__targv[4]);

	pApp->m_param.nWidth = _ttoi(__targv[5]);
	pApp->m_param.nHeight = _ttoi(__targv[6]);

	pApp->m_param.szPassword = (char*)malloc(MAX_PATH);
	sprintf(pApp->m_param.szPassword, "%S", __targv[7]);
}