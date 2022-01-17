
// OwnClientDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "OwnClient.h"
#include "OwnClientDlg.h"
#include "afxdialogex.h"
#include <time.h>
#include "ximage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// COwnClientDlg dialog

#define FPS 25
#define UPDATE_INTERVAL (CLOCKS_PER_SEC/FPS)
#define WM_FIRST_SHOWN WM_USER + 100

COwnClientDlg* g_pDlg = NULL;
static int buttonMask = 0;

COwnClientDlg::COwnClientDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_OWNCLIENT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pClient = NULL;
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
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
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
	if (g_pDlg->m_imgDraw.IsValid() && 
		g_pDlg->m_imgDraw.GetWidth() == client->width && 
		g_pDlg->m_imgDraw.GetHeight() == client->height)
	{
		int			nTemp = rand() % 256;
		int			i, j;

		for (i = y; i < y + h; i++)
		{
			for (j = x; j < x + w; j++)
			{
//				memset(g_pDlg->m_imgDraw.GetBits() + (client->height - i - 1) * client->width * 3 + j * 3, nTemp, 3);

//				int b = *(BYTE*)(client->frameBuffer + i * client->width * 4 + j * 4);

   				memcpy(g_pDlg->m_imgDraw.GetBits() + (client->height-i-1) * client->width * 3 + j * 3,
   					client->frameBuffer + i * client->width * 4 + j * 4, 3);
			}
		}
//		memcpy(g_pDlg->m_imgDraw.GetBits(), client->frameBuffer, client->width * client->height * 3);
		OutputA("%d - Width: %d, Height %d", (int)time(0), client->width, client->height);
		OutputA("%d - Received an update for %d, %d, %d, %d", (int)time(0), x, y, w, h);
	}
	else
	{
		OutputA("Error Update");
	}
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
	CPaintDC dc(this); // device context for painting
	if (IsIconic())
	{
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

	if (m_imgDraw.IsValid())
	{
		RECT		rt;
		GetClientRect(&rt);
		m_imgDraw.Draw(dc.m_hDC, rt);
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

static char* doReadPassword(rfbClient* client) {
	char* p = (char *)calloc(1, 9);
	strncpy(p, theApp.m_param.szPassword, 8);
	p[8] = 0;

	return p;
}

int COwnClientDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialogEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	g_pDlg = this;

	// TODO:  Add your specialized creation code here
	m_pClient = rfbGetClient(8, 3, 4);
	m_pClient->serverHost = theApp.m_param.szHost;
	m_pClient->serverPort = theApp.m_param.nPort;
	m_pClient->GotFrameBufferUpdate = PrintRect;
	m_pClient->GetPassword = doReadPassword;

	try
	{
		if (!rfbInitClient(m_pClient, NULL, NULL))
		{
			m_pClient = NULL;
			AfxMessageBox(L"Can't connect to server");
			EndDialog(IDCANCEL);
			return -1;
		}
	}
	catch (...)
	{
		m_pClient = NULL;
		AfxMessageBox(L"Can't connect to server");
		EndDialog(IDCANCEL);
		return -1;
	}

	SetWindowTextA(m_hWnd, m_pClient->desktopName);

	m_imgDraw.Create(m_pClient->width, m_pClient->height, 24);
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
	if(m_pClient)
	{
		int			i;
		i = WaitForMessage(m_pClient, UPDATE_INTERVAL * 1000);
		if (i < 0)
		{
			rfbClientCleanup(m_pClient);
			m_pClient = NULL;
			EndDialog(IDCANCEL);
			return;
		}
		if (i)
		{
			if (!HandleRFBServerMessage(m_pClient))
			{
				rfbClientCleanup(m_pClient);
				m_pClient = NULL;
				EndDialog(IDCANCEL);
				return;
			}
		}

		Invalidate(FALSE);
	}

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
	if (m_pClient)
	{
		if (m_pClient->frameBuffer)
			free(m_pClient->frameBuffer);
	}

	return CDialogEx::DestroyWindow();
}


void COwnClientDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if (m_pClient)
	{
		CPoint	ptConvert;
		int		x, y, state;

		ptConvert = ConvertPointToClient(point);
		x = ptConvert.x;
		y = ptConvert.y;
		state = rfbButton1Mask;
		buttonMask |= state;

		SendPointerEvent(m_pClient, x, y, buttonMask);
		buttonMask &= ~(rfbButton4Mask | rfbButton5Mask);
	}
	CDialogEx::OnLButtonDown(nFlags, point);
}

CPoint COwnClientDlg::ConvertPointToClient(CPoint pt)
{
	RECT		rt;
	CPoint		ptRet;
	
	GetClientRect(&rt);
	if(m_pClient)
	{
		ptRet.x = (FLOAT)pt.x / (rt.right - rt.left) * m_pClient->width;
		ptRet.y = (FLOAT)pt.y / (rt.bottom - rt.top) * m_pClient->height;
	}
	else
	{
		ptRet = pt;
	}
	return ptRet;
}

void COwnClientDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if (m_pClient)
	{
		CPoint	ptConvert;
		int		x, y, state;

		ptConvert = ConvertPointToClient(point);
		x = ptConvert.x;
		y = ptConvert.y;
		state = 0;

		SendPointerEvent(m_pClient, x, y, buttonMask);
		buttonMask &= ~(rfbButton4Mask | rfbButton5Mask);
	}
	CDialogEx::OnMouseMove(nFlags, point);
}
