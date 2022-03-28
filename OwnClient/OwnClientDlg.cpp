
// OwnClientDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "OwnClient.h"
#include "OwnClientDlg.h"
#include "afxdialogex.h"
#include <time.h>
#include "ximage.h"
#include "xxhash32.h"

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
//	ON_WM_TIMER()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

int OutputW(const WCHAR* format, ...)
{
	return 0;

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
// 		uint32_t	nTempChecksum = XXHash32::hash(client->frameBuffer, client->width * client->height * 4, 0);
// 		if (g_pDlg->m_nChecksum == nTempChecksum)
// 			return;
// 		g_pDlg->m_nChecksum = nTempChecksum;

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

		RECT		rt;
		g_pDlg->GetClientRect(&rt);

		int		nClientWidth = rt.right - rt.left;
		int		nClientHeight = rt.bottom - rt.top;

		FLOAT	fScaleX = (FLOAT)nClientWidth / w;
		FLOAT	fScaleY = (FLOAT)nClientHeight / h;

		SetRect(&rt, x * fScaleX, y * fScaleY, (x + w) * fScaleX, (y + h) * fScaleY);
		// g_pDlg->InvalidateRect(&rt, FALSE);
		g_pDlg->Invalidate(FALSE);
	}
	else
	{
		OutputA("Error Update");
	}
}

static void gotCursorShape(rfbClient* client, int xhot, int yhot, int width, int height, int bytesPerPixel)
{
	OutputA("Got Cursor Shape \n");
}

static rfbBool handleCursorPos(rfbClient* client, int x, int y)
{
	OutputA("HandleCursorPos\n");
	return TRUE;
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
	m_nChecksum = 0xFFFFFF;

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
	if (GetAsyncKeyState(VK_CONTROL))
	{
		ShowCursor(FALSE);
		return CDialogEx::OnNcHitTest(point);
	}
	ShowCursor(TRUE);
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

void ThreadProc(COwnClientDlg* pDlg)
{
	int		i;

	while (1) {
		i = WaitForMessage(pDlg->m_pClient, 40000);
		if (i < 0)
		{
			rfbClientCleanup(pDlg->m_pClient);
			pDlg->m_pClient = NULL;
			pDlg->EndDialog(IDCANCEL);
			break;
		}
		if (i)
		{
			if (!HandleRFBServerMessage(pDlg->m_pClient))
			{
				rfbClientCleanup(pDlg->m_pClient);
				pDlg->m_pClient = NULL;
				pDlg->EndDialog(IDCANCEL);
				break;
			}
		}
		Sleep(40);
	}
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

	// SetTimer(0, UPDATE_INTERVAL, NULL);
	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ThreadProc, this, NULL, NULL);

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

//void COwnClientDlg::OnTimer(UINT_PTR nIDEvent)
//{
//	// TODO: Add your message handler code here and/or call default
//	if(m_pClient)
//	{
//		SendFramebufferUpdateRequest(m_pClient, 0, 0, m_pClient->width, m_pClient->height, TRUE);
//
//		int			i;
//		i = WaitForMessage(m_pClient, UPDATE_INTERVAL * 1000);
//		if (i < 0)
//		{
//			rfbClientCleanup(m_pClient);
//			m_pClient = NULL;
//			EndDialog(IDCANCEL);
//			return;
//		}
//		if (i)
//		{
//			if (!HandleRFBServerMessage(m_pClient))
//			{
//				rfbClientCleanup(m_pClient);
//				m_pClient = NULL;
//				EndDialog(IDCANCEL);
//				return;
//			}
//		}
//
//		Invalidate(FALSE);
//	}
//
//	CDialogEx::OnTimer(nIDEvent);
//}


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

static rfbKeySym VKey2rfbKeySym(WPARAM wParam) {
	rfbKeySym k = 0;
	uint8_t sym = wParam;

	switch (sym) {
	case VK_BACK: k = XK_BackSpace; break;
	case VK_TAB: k = XK_KP_Tab; break;
	case VK_CLEAR: k = XK_Clear; break;
	case VK_RETURN: k = XK_Return; break;
	case VK_PAUSE: k = XK_Pause; break;
	case VK_ESCAPE: k = XK_Escape; break;
	case VK_DELETE: k = XK_Delete; break;
	case VK_NUMPAD0: k = XK_KP_0; break;
	case VK_NUMPAD1: k = XK_KP_1; break;
	case VK_NUMPAD2: k = XK_KP_2; break;
	case VK_NUMPAD3: k = XK_KP_3; break;
	case VK_NUMPAD4: k = XK_KP_4; break;
	case VK_NUMPAD5: k = XK_KP_5; break;
	case VK_NUMPAD6: k = XK_KP_6; break;
	case VK_NUMPAD7: k = XK_KP_7; break;
	case VK_NUMPAD8: k = XK_KP_8; break;
	case VK_NUMPAD9: k = XK_KP_9; break;
	case VK_DECIMAL: k = XK_KP_Decimal; break;
	case VK_DIVIDE: k = XK_KP_Divide; break;
	case VK_MULTIPLY: k = XK_KP_Multiply; break;
	case VK_SUBTRACT: k = XK_KP_Subtract; break;
	case VK_ADD: k = XK_KP_Add; break;
// 	case SDLK_KP_ENTER: k = XK_KP_Enter; break;
// 	case SDLK_KP_EQUALS: k = XK_KP_Equal; break;
	case VK_SEPARATOR: k = XK_KP_Separator; break;
	case VK_UP: k = XK_Up; break;
	case VK_DOWN: k = XK_Down; break;
	case VK_RIGHT: k = XK_Right; break;
	case VK_LEFT: k = XK_Left; break;
	case VK_INSERT: k = XK_Insert; break;
	case VK_HOME: k = XK_Home; break;
	case VK_END: k = XK_End; break;
	case VK_PRIOR: k = XK_Page_Up; break;
	case VK_NEXT: k = XK_Page_Down; break;
	case VK_F1: k = XK_F1; break;
	case VK_F2: k = XK_F2; break;
	case VK_F3: k = XK_F3; break;
	case VK_F4: k = XK_F4; break;
	case VK_F5: k = XK_F5; break;
	case VK_F6: k = XK_F6; break;
	case VK_F7: k = XK_F7; break;
	case VK_F8: k = XK_F8; break;
	case VK_F9: k = XK_F9; break;
	case VK_F10: k = XK_F10; break;
	case VK_F11: k = XK_F11; break;
	case VK_F12: k = XK_F12; break;
	case VK_F13: k = XK_F13; break;
	case VK_F14: k = XK_F14; break;
	case VK_F15: k = XK_F15; break;
	case VK_NUMLOCK: k = XK_Num_Lock; break;
	case VK_CAPITAL: k = XK_Caps_Lock; break;
	case VK_SCROLL: k = XK_Scroll_Lock; break;
	case VK_RSHIFT: k = XK_Shift_R; break;
	case VK_SHIFT: k = XK_Shift_L; break;
	case VK_RCONTROL: k = XK_Control_R; break;
	case VK_LCONTROL: k = XK_Control_L; break;
 	case VK_RMENU: k = XK_Alt_R; break;
	case VK_MENU: k = XK_Alt_L; break;
// 	case SDLK_LGUI: k = XK_Super_L; break;
// 	case SDLK_RGUI: k = XK_Super_R; break;
#if 0
// 	case SDLK_COMPOSE: k = XK_Compose; break;
#endif
	case VK_MODECHANGE: k = XK_Mode_switch; break;
	case VK_HELP: k = XK_Help; break;
	case VK_SNAPSHOT: k = XK_Print; break;
// 	case SDLK_SYSREQ: k = XK_Sys_Req; break;
	case VK_CANCEL: k = XK_Break; break;
	case VK_EXECUTE: k = XK_Execute; break;
	case VK_KANJI: k = XK_Kanji; break;
	case VK_NONCONVERT: k = XK_Muhenkan; break;
	case VK_CONVERT: k = XK_Mae_Koho; break;
	case VK_SELECT: k = XK_Select; break;
	case VK_SPACE: k = XK_KP_Space; break;
	case VK_LWIN: k = XK_Super_L; break;
	case VK_RWIN: k = XK_Super_R; break;
	case VK_APPS: k = XK_Menu; break;
	default: break;
	}
	/* SDL_TEXTINPUT does not generate characters if ctrl is down, so handle those here */
	if (k == 0 && sym > 0x0 && sym < 0x100 /*&& e->keysym.mod & KMOD_CTRL*/)
		k = sym;

	return k;
}

LRESULT COwnClientDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: Add your specialized code here and/or call the base class
	int			x, y;
	CPoint		ptInput, ptConvert;
	DWORD		keyflags;
	int			state;
	CURSORINFO cursor = { sizeof(cursor) };
	int wheelMask;
	int delta;

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

			GetCursorInfo(&cursor);

			if (keyflags & MK_CONTROL)
			{
				if (cursor.flags == CURSOR_SHOWING) {
					ShowCursor(FALSE);
				}

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
			}
			else
			{
				if (cursor.flags != CURSOR_SHOWING) {
					ShowCursor(TRUE);
				}
			}
			::DestroyIcon(cursor.hCursor);
			
			break;
		case WM_LBUTTONDBLCLK:
			if (keyflags & MK_CONTROL)
			{
				x = LOWORD(lParam);
				y = HIWORD(lParam);

				ptInput.x = x;
				ptInput.y = y;
				ptConvert = ConvertPointToClient(ptInput);

				SendPointerEvent(m_pClient, ptConvert.x, ptConvert.y, 1);
				SendPointerEvent(m_pClient, ptConvert.x, ptConvert.y, 0);
				SendPointerEvent(m_pClient, ptConvert.x, ptConvert.y, 1);
				SendPointerEvent(m_pClient, ptConvert.x, ptConvert.y, 0);
			}
			break;
		case WM_MOUSEWHEEL:
			x = LOWORD(lParam);
			y = HIWORD(lParam);

			ptInput.x = x;
			ptInput.y = y;
			ptConvert = ConvertPointToClient(ptInput);

			delta = (SHORT)HIWORD(wParam);
			wheelMask = rfbWheelUpMask;
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


void COwnClientDlg::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class

	// CDialogEx::OnOK();
}


BOOL COwnClientDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->hwnd == m_hWnd)
	{
		switch (pMsg->message)
		{
		case WM_KEYDOWN:
		case WM_KEYUP:
			SendKeyEvent(m_pClient, VKey2rfbKeySym(pMsg->wParam), pMsg->message == WM_KEYDOWN ? TRUE : FALSE);
			break;
		}
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}
