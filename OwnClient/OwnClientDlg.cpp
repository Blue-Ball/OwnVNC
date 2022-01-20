
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
		int			nTempImage, nTempFrame;

		for (i = y; i < y + h; i++)
		{
			nTempImage = (client->height - i - 1) * client->width * 3;
			nTempFrame = i * client->width * 4;
			for (j = x; j < x + w; j++)
			{
   				memcpy(g_pDlg->m_imgDraw.GetBits() + nTempImage + j * 3,
   					client->frameBuffer + nTempFrame + j * 4, 3);
			}
		}
		OutputA("%d - Width: %d, Height %d", (int)time(0), client->width, client->height);
		OutputA("%d - Received an update for %d, %d, %d, %d", (int)time(0), x, y, w, h);

// 		RECT		rt;
// 		SetRect(&rt, x, y, x + w, y + h);
// 		g_pDlg->InvalidateRect(&rt, FALSE);
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
// 	return HTCAPTION;
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
	m_pClient->appData.compressLevel = theApp.m_param.nCompressLevel;
	m_pClient->appData.qualityLevel = theApp.m_param.nQualityLevel;
	// m_pClient->appData.encodingsString = "tight zrle ultra copyrect hextile zlib corre rre raw";

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
//		SendFramebufferUpdateRequest(m_pClient, 0, 0, m_pClient->width, m_pClient->height, TRUE);

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
//	KillTimer(0);
	if (m_pClient)
	{
		if (m_pClient->frameBuffer)
			free(m_pClient->frameBuffer);
	}

	return CDialogEx::DestroyWindow();
}

CPoint COwnClientDlg::ConvertPointToClient(CPoint pt)
{
	RECT		rt;
	CPoint		ptRet;

	GetClientRect(&rt);
	if (m_pClient)
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

LRESULT COwnClientDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: Add your specialized code here and/or call the base class
	int			x, y;
	CPoint		ptInput, ptConvert;
	DWORD		keyflags;
	int			state;

	if (m_pClient)
	{
		switch (message)
		{
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MOUSEMOVE:

			x = LOWORD(lParam);
			y = HIWORD(lParam);

			ptInput.x = x;
			ptInput.y = y;
			ptConvert = ConvertPointToClient(ptInput);

			keyflags = MAKEWPARAM(LOWORD(wParam), 0);

			buttonMask = (((keyflags & MK_LBUTTON) ? rfbButton1Mask : 0) |
				((keyflags & MK_MBUTTON) ? rfbButton2Mask : 0) |
				((keyflags & MK_RBUTTON) ? rfbButton3Mask : 0));

			if ((short)HIWORD(keyflags) > 0) {
				buttonMask |= rfbButton4Mask;
			}
			else if ((short)HIWORD(keyflags) < 0) {
				buttonMask |= rfbButton5Mask;
			}

			SendPointerEvent(m_pClient, ptConvert.x, ptConvert.y, buttonMask);

			if ((short)HIWORD(keyflags) != 0) {
				// Immediately send a "button-up" after mouse wheel event.
				buttonMask &= !(rfbButton4Mask | rfbButton5Mask);
				SendPointerEvent(m_pClient, ptConvert.x, ptConvert.y, buttonMask);
			}
			break;
		case WM_LBUTTONDBLCLK:
			x = LOWORD(lParam);
			y = HIWORD(lParam);

			ptInput.x = x;
			ptInput.y = y;
			ptConvert = ConvertPointToClient(ptInput);

			SendPointerEvent(m_pClient, ptConvert.x, ptConvert.y, 1);
			SendPointerEvent(m_pClient, ptConvert.x, ptConvert.y, 0);
			SendPointerEvent(m_pClient, ptConvert.x, ptConvert.y, 1);
			SendPointerEvent(m_pClient, ptConvert.x, ptConvert.y, 0);
			break;
		case WM_MOUSEWHEEL:
			x = LOWORD(lParam);
			y = HIWORD(lParam);

			ptInput.x = x;
			ptInput.y = y;
			ptConvert = ConvertPointToClient(ptInput);

			int delta = (SHORT)HIWORD(wParam);
			int wheelMask = rfbWheelUpMask;
			if (delta < 0) {
				wheelMask = rfbWheelDownMask;
				delta = -delta;
			}
			while (delta > 0) {
				SendPointerEvent(m_pClient, ptConvert.x, ptConvert.y,  wheelMask);
				delta -= 120;
			}
			break;
		}
	}


	return CDialogEx::WindowProc(message, wParam, lParam);
}
