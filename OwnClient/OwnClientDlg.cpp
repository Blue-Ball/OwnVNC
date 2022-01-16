
// OwnClientDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "OwnClient.h"
#include "OwnClientDlg.h"
#include "afxdialogex.h"
#include <time.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// COwnClientDlg dialog

#define FPS 25
#define UPDATE_INTERVAL (CLOCKS_PER_SEC/FPS)

#define WM_FIRST_SHOWN WM_USER + 100

COwnClientDlg::COwnClientDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_OWNCLIENT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void COwnClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(COwnClientDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_NCHITTEST()
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_MOVE()
	ON_MESSAGE(WM_FIRST_SHOWN, OnDialogShown)
	ON_WM_TIMER()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

int OutputW(const WCHAR* format, ...)
{
	static WCHAR s_printf_buf[1024];
	va_list args;
	va_start(args, format);
	_vsnwprintf(s_printf_buf, sizeof(s_printf_buf), format, args);
	va_end(args);
	OutputDebugStringW(s_printf_buf);
	OutputDebugStringW(L"\n");
	return 0;
}

int OutputA(const char* format, ...)
{
	static char s_printf_buf[1024];
	va_list args;
	va_start(args, format);
	_vsnprintf(s_printf_buf, sizeof(s_printf_buf), format, args);
	va_end(args);
	OutputDebugStringA(s_printf_buf);
	OutputDebugStringA("\n");
	return 0;
}

static void PrintRect(rfbClient* client, int x, int y, int w, int h) {

	OutputA("%d - Received an update for %d,%d,%d,%d.", (int)time(0), x, y, w, h);
}

// COwnClientDlg message handlers

BOOL COwnClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	CRect rcWindow;
	GetWindowRect(&rcWindow);
	m_WndResize.SetWindowPos(&wndNoTopMost, rcWindow.left, rcWindow.top, rcWindow.Width(), rcWindow.Height(), SWP_NOACTIVATE);

	PostMessage(WM_FIRST_SHOWN);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void COwnClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR COwnClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



LRESULT COwnClientDlg::OnNcHitTest(CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	return HTCAPTION;
	return CDialogEx::OnNcHitTest(point);
}



int COwnClientDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialogEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	m_pClient = rfbGetClient(8, 3, 4);
	m_pClient->serverHost = theApp.m_param.szHost;
	m_pClient->serverPort = theApp.m_param.nPort;
	m_pClient->GotFrameBufferUpdate = PrintRect;

	try
	{
		if (!rfbInitClient(m_pClient, NULL, NULL))
		{
			AfxMessageBox(L"False");
			EndDialog(IDCANCEL);
			return -1;
		}
	}
	catch (...)
	{
		AfxMessageBox(L"Error");
		EndDialog(IDCANCEL);
		return -1;
	}

	m_WndResize.Create(this);
	m_WndResize.ShowWindow(SW_SHOW);

	return 0;
}

void COwnClientDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	CRect rcWindow;
	GetWindowRect(&rcWindow);
	m_WndResize.SetWindowPos(&wndNoTopMost, rcWindow.left, rcWindow.top, rcWindow.Width(), rcWindow.Height(), SWP_NOACTIVATE);
}

LRESULT COwnClientDlg::OnDialogShown(WPARAM, LPARAM)
{
	COwnClientApp* pApp = (COwnClientApp*)AfxGetApp();
	if (pApp->m_param.nWidth == -1 && pApp->m_param.nHeight == -1)
	{
		CRect		rtTemp;
		GetClientRect(&rtTemp);
		pApp->m_param.nWidth = rtTemp.Width();
		pApp->m_param.nHeight = rtTemp.Height();
		CenterWindow();
	}
	else
	{
		MoveWindow(pApp->m_param.nX, pApp->m_param.nY, pApp->m_param.nWidth, pApp->m_param.nHeight);
	}

	SetTimer(0, UPDATE_INTERVAL, NULL);
	return 0;
}

void COwnClientDlg::OnMove(int x, int y)
{
	CDialogEx::OnMove(x, y);

	// TODO: Add your message handler code here
	CRect rcWindow;
	GetWindowRect(&rcWindow);
	m_WndResize.SetWindowPos(&wndNoTopMost, rcWindow.left, rcWindow.top, rcWindow.Width(), rcWindow.Height(), SWP_NOACTIVATE);
}


void COwnClientDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	int			i;
	i = WaitForMessage(m_pClient, UPDATE_INTERVAL * 1000);
	if (i < 0)
	{
		EndDialog(IDCANCEL);
		return;
	}
	if (i)
	{
		if (!HandleRFBServerMessage(m_pClient))
		{
			EndDialog(IDCANCEL);
			return;
		}
	}

	Invalidate(FALSE);

	CDialogEx::OnTimer(nIDEvent);
}


BOOL COwnClientDlg::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default
	return FALSE;
	return CDialogEx::OnEraseBkgnd(pDC);
}


BOOL COwnClientDlg::DestroyWindow()
{
	// TODO: Add your specialized code here and/or call the base class
	KillTimer(0);
	if (m_pClient->frameBuffer)
		free(m_pClient->frameBuffer);

	return CDialogEx::DestroyWindow();
}
