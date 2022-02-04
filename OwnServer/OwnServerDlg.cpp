
// OwnServerDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "OwnServer.h"
#include "OwnServerDlg.h"
#include "afxdialogex.h"
#include "Global.h"
#include "xxhash32.h"
#include "rfb/keysym.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define FPS 25.0f
#define UPDATE_INTERVAL (1.0f/FPS)
#define WM_FIRST_SHOWN WM_USER + 100
#define WM_SHOWTASK WM_USER + 101

struct keymap_t {
	uint32_t keysym;
	uint8_t vk;
	bool extended;
};

static keymap_t keymap[] = {

	// TTY functions

	{ XK_BackSpace,        VK_BACK, 0 },
	{ XK_Tab,              VK_TAB, 0 },
	{ XK_Clear,            VK_CLEAR, 0 },
	{ XK_Return,           VK_RETURN, 0 },
	{ XK_Pause,            VK_PAUSE, 0 },
	{ XK_Escape,           VK_ESCAPE, 0 },
	{ XK_Delete,           VK_DELETE, 1 },

	// Japanese stuff - almost certainly wrong...
	//	[v1.0.2-jp1 fix] IOport's patch (Correct definition of Japanese key)
	//{ XK_Kanji,            VK_KANJI, 0 },
	//{ XK_Kana_Shift,       VK_KANA, 0 },
	// Japanese key
	{ XK_Kanji,            VK_KANJI, 0 },				/* 0x19: Kanji, Kanji convert */
	{ XK_Muhenkan,         VK_NONCONVERT, 0 },		/* 0x1d: Cancel Conversion */
	{ XK_Mae_Koho,         VK_CONVERT, 0 },			/* 0x1c: Previous Candidate */

	// Cursor control & motion

	{ XK_Home,             VK_HOME, 1 },
	{ XK_Left,             VK_LEFT, 1 },
	{ XK_Up,               VK_UP, 1 },
	{ XK_Right,            VK_RIGHT, 1 },
	{ XK_Down,             VK_DOWN, 1 },
	{ XK_Page_Up,          VK_PRIOR, 1 },
	{ XK_Page_Down,        VK_NEXT, 1 },
	{ XK_End,              VK_END, 1 },

	// Misc functions

	{ XK_Select,           VK_SELECT, 0 },
	{ XK_Print,            VK_SNAPSHOT, 0 },
	{ XK_Execute,          VK_EXECUTE, 0 },
	{ XK_Insert,           VK_INSERT, 1 },
	{ XK_Help,             VK_HELP, 0 },
	{ XK_Break,            VK_CANCEL, 1 },

	// Keypad Functions, keypad numbers

	{ XK_KP_Space,         VK_SPACE, 0 },
	{ XK_KP_Tab,           VK_TAB, 0 },
	{ XK_KP_Enter,         VK_RETURN, 1 },
	{ XK_KP_F1,            VK_F1, 0 },
	{ XK_KP_F2,            VK_F2, 0 },
	{ XK_KP_F3,            VK_F3, 0 },
	{ XK_KP_F4,            VK_F4, 0 },
	{ XK_KP_Home,          VK_HOME, 0 },
	{ XK_KP_Left,          VK_LEFT, 0 },
	{ XK_KP_Up,            VK_UP, 0 },
	{ XK_KP_Right,         VK_RIGHT, 0 },
	{ XK_KP_Down,          VK_DOWN, 0 },
	{ XK_KP_End,           VK_END, 0 },
	{ XK_KP_Page_Up,       VK_PRIOR, 0 },
	{ XK_KP_Page_Down,     VK_NEXT, 0 },
	{ XK_KP_Begin,         VK_CLEAR, 0 },
	{ XK_KP_Insert,        VK_INSERT, 0 },
	{ XK_KP_Delete,        VK_DELETE, 0 },
	// XXX XK_KP_Equal should map in the same way as ascii '='
	{ XK_KP_Multiply,      VK_MULTIPLY, 0 },
	{ XK_KP_Add,           VK_ADD, 0 },
	{ XK_KP_Separator,     VK_SEPARATOR, 0 },
	{ XK_KP_Subtract,      VK_SUBTRACT, 0 },
	{ XK_KP_Decimal,       VK_DECIMAL, 0 },
	{ XK_KP_Divide,        VK_DIVIDE, 1 },

	{ XK_KP_0,             VK_NUMPAD0, 0 },
	{ XK_KP_1,             VK_NUMPAD1, 0 },
	{ XK_KP_2,             VK_NUMPAD2, 0 },
	{ XK_KP_3,             VK_NUMPAD3, 0 },
	{ XK_KP_4,             VK_NUMPAD4, 0 },
	{ XK_KP_5,             VK_NUMPAD5, 0 },
	{ XK_KP_6,             VK_NUMPAD6, 0 },
	{ XK_KP_7,             VK_NUMPAD7, 0 },
	{ XK_KP_8,             VK_NUMPAD8, 0 },
	{ XK_KP_9,             VK_NUMPAD9, 0 },

	// Auxilliary Functions

	{ XK_F1,               VK_F1, 0 },
	{ XK_F2,               VK_F2, 0 },
	{ XK_F3,               VK_F3, 0 },
	{ XK_F4,               VK_F4, 0 },
	{ XK_F5,               VK_F5, 0 },
	{ XK_F6,               VK_F6, 0 },
	{ XK_F7,               VK_F7, 0 },
	{ XK_F8,               VK_F8, 0 },
	{ XK_F9,               VK_F9, 0 },
	{ XK_F10,              VK_F10, 0 },
	{ XK_F11,              VK_F11, 0 },
	{ XK_F12,              VK_F12, 0 },
	{ XK_F13,              VK_F13, 0 },
	{ XK_F14,              VK_F14, 0 },
	{ XK_F15,              VK_F15, 0 },
	{ XK_F16,              VK_F16, 0 },
	{ XK_F17,              VK_F17, 0 },
	{ XK_F18,              VK_F18, 0 },
	{ XK_F19,              VK_F19, 0 },
	{ XK_F20,              VK_F20, 0 },
	{ XK_F21,              VK_F21, 0 },
	{ XK_F22,              VK_F22, 0 },
	{ XK_F23,              VK_F23, 0 },
	{ XK_F24,              VK_F24, 0 },

	// Modifiers

  { XK_Shift_L,          VK_SHIFT, 0 },
  { XK_Shift_R,          VK_RSHIFT, 0 },
  { XK_Control_L,        VK_CONTROL, 0 },
  { XK_Control_R,        VK_CONTROL, 1 },
  { XK_Alt_L,            VK_MENU, 0 },
  { XK_Alt_R,            VK_RMENU, 1 },

  // Left & Right Windows keys & Windows Menu Key

	{ XK_Super_L,			VK_LWIN, 0 },
	{ XK_Super_R,			VK_RWIN, 0 },
	{ XK_Menu,			VK_APPS, 0 },

};

CMap<uint32_t, uint32_t&, uint8_t, uint8_t&> vkMap;
CMap<uint32_t, uint32_t&, bool, bool&> extendedMap;

// COwnServerDlg dialog
COwnServerDlg* g_pDlg = NULL;

COwnServerDlg::COwnServerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_OWNSERVER_DIALOG, pParent)
	, m_nPort(0)
	, m_nWidth(0)
	, m_nHeight(0)
	, m_strPwd(_T(""))
	, m_strName(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_bVisible = FALSE;
}

void COwnServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_PORT, m_nPort);
	DDX_Text(pDX, IDC_EDIT_WIDTH, m_nWidth);
	DDX_Text(pDX, IDC_EDIT_HEIGHT, m_nHeight);
	DDX_Text(pDX, IDC_EDIT_PWD, m_strPwd);
	DDX_Text(pDX, IDC_EDIT_NAME, m_strName);
}

BEGIN_MESSAGE_MAP(COwnServerDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_COMMAND(ID_MENU_EXIT, &COwnServerDlg::OnMenuExit)
	ON_MESSAGE(WM_SHOWTASK, onShowTask)
	ON_MESSAGE(WM_FIRST_SHOWN, OnDialogShown)
//	ON_WM_TIMER()
	ON_COMMAND(ID_MENU_SETTINGS, &COwnServerDlg::OnMenuSettings)
	ON_BN_CLICKED(IDOK, &COwnServerDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &COwnServerDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BTN_GETSCREEN, &COwnServerDlg::OnBnClickedBtnGetscreen)
	ON_WM_WINDOWPOSCHANGING()
END_MESSAGE_MAP()


// COwnServerDlg message handlers

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

BOOL COwnServerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	PostMessage(WM_FIRST_SHOWN);

	HDC hdcDesk = ::GetDC(HWND_DESKTOP);

	m_nScreenWidth = GetDeviceCaps(hdcDesk, HORZRES);
	m_nScreenHeight = GetDeviceCaps(hdcDesk, VERTRES);

	::ReleaseDC(HWND_DESKTOP, hdcDesk);

	m_pScreen = NULL;
	g_pDlg = this;
	m_nChecksum = 0xFFFFFF;

	for (int i = 0; i < sizeof(keymap) / sizeof(keymap_t); i++) {
		vkMap[keymap[i].keysym] = keymap[i].vk;
		extendedMap[keymap[i].keysym] = keymap[i].extended;
	}
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void COwnServerDlg::OnPaint()
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
HCURSOR COwnServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

static int			nPrevX = 0, nPrevY = 0, nPrevButtonMask = 0;
static void doptr(int buttonMask, int nX, int nY, rfbClientPtr cl)
{
	OutputA("Ptr: mouse button mask 0x%x at %d,%d\n", buttonMask, nX, nY);

	DWORD flags = MOUSEEVENTF_ABSOLUTE;

	if (nX != nPrevX || nY != nPrevY)
		flags |= MOUSEEVENTF_MOVE;
	if ((buttonMask & rfbButton1Mask) != (nPrevButtonMask & rfbButton1Mask))
	{
		if (GetSystemMetrics(SM_SWAPBUTTON))
			flags |= (buttonMask & rfbButton1Mask) ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_RIGHTUP;
		else
			flags |= (buttonMask & rfbButton1Mask) ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP;
	}
	if ((buttonMask & rfbButton2Mask) != (nPrevButtonMask & rfbButton2Mask))
	{
		flags |= (buttonMask & rfbButton2Mask) ? MOUSEEVENTF_MIDDLEDOWN : MOUSEEVENTF_MIDDLEUP;
	}
	if ((buttonMask & rfbButton3Mask) != (nPrevButtonMask & rfbButton3Mask))
	{
		if (GetSystemMetrics(SM_SWAPBUTTON))
			flags |= (buttonMask & rfbButton3Mask) ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP;
		else
			flags |= (buttonMask & rfbButton3Mask) ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_RIGHTUP;
	}

	DWORD wheel_movement = 0;
	if (buttonMask & rfbWheelUpMask) {
		flags |= MOUSEEVENTF_WHEEL;
		wheel_movement = WHEEL_DELTA;
	}
	if (buttonMask & rfbWheelDownMask) {
		flags |= MOUSEEVENTF_WHEEL;
		wheel_movement = -WHEEL_DELTA;
	}

	CPoint		curPos;
	GetCursorPos(&curPos);

	// SetCursorPos(nX, nY);

	OutputA("flags: %d, x: %d, y: %d, curX: %d, curY: %d, wheel: %d", flags, nX, nY, curPos.x, curPos.y, wheel_movement);

	unsigned long x = ((nX + 0) * 65535) / (g_pDlg->m_nScreenWidth - 1);
	unsigned long y = ((nY + 0) * 65535) / (g_pDlg->m_nScreenHeight - 1);

	OutputA("flags: %d, x: %d, y: %d, curX: %d, curY: %d, wheel: %d", flags, x, y, curPos.x, curPos.y, wheel_movement);

	::mouse_event(flags, (DWORD)x, (DWORD)y, wheel_movement, 0);

	nPrevX = x;
	nPrevY = y;
	nPrevButtonMask = buttonMask;

	rfbDefaultPtrAddEvent(buttonMask, x, y, cl);
}

static void doKey(rfbBool down, rfbKeySym keySym, rfbClientPtr cl)
{
	OutputA("down: %d, keySym: %X", down, keySym);

	INPUT input;
	DWORD flags = 0;
	BYTE vkCode;

	input.type = INPUT_KEYBOARD;
	input.ki.wScan = 0;
	input.ki.time = 0;
	input.ki.dwExtraInfo = 0;

	if ((keySym >= 32 && keySym <= 126) ||
		(keySym >= 160 && keySym <= 255))
	{
		SHORT s = VkKeyScan(keySym);
		input.ki.wVk = s;

		if (!down)
			flags |= KEYEVENTF_KEYUP;
		
		input.ki.dwFlags = flags; // there is no KEYEVENTF_KEYDOWN
		
		::SendInput(1, &input, sizeof(INPUT));
	}
	else
	{
		if (!vkMap.Lookup(keySym, vkCode)) {
		}
		else
		{
			vkCode = vkMap[keySym];

			if (extendedMap[keySym])
				flags |= KEYEVENTF_EXTENDEDKEY;
			
			input.ki.wVk = vkCode;

			if (!down)
				flags |= KEYEVENTF_KEYUP;

			input.ki.dwFlags = flags; // there is no KEYEVENTF_KEYDOWN

			::SendInput(1, &input, sizeof(INPUT));
		}
	}

	

}

typedef struct ClientData {
	rfbBool bDrawCursor;
	uint32_t nChecksum;
	uint32_t nUpdatedCount;
} ClientData;

static void clientgone(rfbClientPtr cl)
{
	free(cl->clientData);
	cl->clientData = NULL;
}

static enum rfbNewClientAction newclient(rfbClientPtr cl)
{
	cl->clientData = calloc(sizeof(ClientData), 1);
	cl->clientGoneHook = clientgone;

	((ClientData*)cl->clientData)->bDrawCursor = FALSE;
	((ClientData*)cl->clientData)->nChecksum = 0xFFFFFF;
	((ClientData*)cl->clientData)->nUpdatedCount = 0;

	return RFB_CLIENT_ACCEPT;
}

static void gettimeofday(struct timeval* tv, char* dummy)
{
	SYSTEMTIME t;
	GetSystemTime(&t);
	tv->tv_sec = t.wHour * 3600 + t.wMinute * 60 + t.wSecond;
	tv->tv_usec = t.wMilliseconds * 1000;
}

int TimeToTakePicture() {
	static struct timeval now = { 0,0 }, then = { 0,0 };
	double elapsed, dnow, dthen;

	gettimeofday(&now, NULL);

	dnow = now.tv_sec + (now.tv_usec / 1000000.0);
	dthen = then.tv_sec + (then.tv_usec / 1000000.0);
	elapsed = dnow - dthen;

	if (elapsed > UPDATE_INTERVAL)
		memcpy((char*)&then, (char*)&now, sizeof(struct timeval));
	return elapsed > UPDATE_INTERVAL;
}

void ThreadProc(COwnServerDlg* pDlg)
{
	long usec;

	while (pDlg->m_pScreen && rfbIsActive(pDlg->m_pScreen)) {
		if (TimeToTakePicture())
		{
			BOOL		bCaptured = FALSE;
			BOOL		bUpdateScreen = FALSE;

			rfbClientPtr cl;
			rfbClientIteratorPtr iter = rfbGetClientIterator(pDlg->m_pScreen);
			while ((cl = rfbClientIteratorNext(iter)))
			{
				if (!bCaptured)
				{
					pDlg->CaptureScreen(pDlg->m_pScreen, pDlg->m_pScreen->width, pDlg->m_pScreen->height);
					bCaptured = TRUE;
				}

				if (((ClientData*)cl->clientData)->nChecksum != pDlg->m_nChecksum || 
					((ClientData*)cl->clientData)->nUpdatedCount < 3)
				{
					((ClientData*)cl->clientData)->nChecksum = pDlg->m_nChecksum;
					((ClientData*)cl->clientData)->nUpdatedCount++;
					bUpdateScreen = TRUE;
				}
				else
				{
					((ClientData*)cl->clientData)->nUpdatedCount = 0;
				}

//				rfbMarkRectAsModified(cl->screen, 0, 0, cl->screen->width - 1, cl->screen->height - 1);
			}
			rfbReleaseClientIterator(iter);
			if (bUpdateScreen)
			{
				rfbMarkRectAsModified(pDlg->m_pScreen, 0, 0, pDlg->m_pScreen->width - 1, pDlg->m_pScreen->height - 1);
			}
		}

		Sleep(40);
//		rfbProcessEvents(pDlg->m_pScreen, 40000);
	}
}

LRESULT COwnServerDlg::OnDialogShown(WPARAM, LPARAM)
{
	ToTray();
	ShowWindow(SW_HIDE);

	SetWindowTextA(m_hWnd, g_param.szName);

	m_pScreen = rfbGetScreen(NULL, NULL, g_param.nWidth, g_param.nHeight, 8, 3, 4);
	m_pScreen->desktopName = g_param.szName;
	m_pScreen->port = g_param.nPort;
	m_pScreen->frameBuffer = (char*)malloc(g_param.nWidth * g_param.nHeight * 4);
	m_pScreen->alwaysShared = TRUE;
	m_pScreen->ptrAddEvent = doptr;
	m_pScreen->kbdAddEvent = doKey;
	m_pScreen->newClientHook = newclient;
	
	if (strlen(g_param.szPassword) == 0)
	{
		m_pScreen->authPasswdData = NULL;
	}
	else
	{
		char** passwds = (char**)malloc(sizeof(char**) * 2);
		passwds[0] = g_param.szPassword;
		passwds[1] = NULL;
		m_pScreen->authPasswdData = (void*)passwds;
		m_pScreen->passwordCheck = rfbCheckPasswordByList;
	}
	

	rfbInitServer(m_pScreen);

	// SetTimer(0, UPDATE_INTERVAL, NULL);
	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ThreadProc, this, NULL, NULL);

	// 	while (1) {
// 		rfbProcessEvents(server, UPDATE_INTERVAL*1000);
// 	}

 	rfbRunEventLoop(m_pScreen, -1, TRUE);

	return 0;
}

void COwnServerDlg::OnMenuExit()
{
	EndDialog(IDCANCEL);
	// TODO: Add your command handler code here
}

LRESULT COwnServerDlg::onShowTask(WPARAM wParam, LPARAM lParam)
{
	if (wParam != IDR_MAINFRAME)
		return 1;
	switch (lParam)
	{
	case WM_RBUTTONUP:
	{
		CPoint pt;
		CMenu menu;

		GetCursorPos(&pt);
		menu.LoadMenuW(IDR_MENU1);
		menu.GetSubMenu(0)->TrackPopupMenu(TPM_BOTTOMALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, pt.x, pt.y, this);
	}
	break;
	case WM_LBUTTONDBLCLK:
	case WM_LBUTTONDOWN:
	{
	}
	break;
	}
	return 0;
}

void COwnServerDlg::ToTray() {
	m_nid.cbSize = (DWORD)sizeof(NOTIFYICONDATA);
	m_nid.hWnd = this->m_hWnd;
	m_nid.uID = IDR_MAINFRAME;
	m_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	m_nid.uCallbackMessage = WM_SHOWTASK;

	m_nid.hIcon = m_hIcon;
	lstrcpy(m_nid.szTip, L"VUBIG_VNC");
	Shell_NotifyIcon(NIM_ADD, &m_nid);
}

BOOL COwnServerDlg::DestroyWindow()
{
	// TODO: Add your specialized code here and/or call the base class
	Shell_NotifyIcon(NIM_DELETE, &m_nid);

	if (m_pScreen->authPasswdData)
		free(m_pScreen->authPasswdData);

	if (m_pScreen)
		free(m_pScreen->frameBuffer);

	rfbScreenCleanup(m_pScreen);

	return CDialogEx::DestroyWindow();
}

BOOL COwnServerDlg::CaptureScreen(rfbScreenInfoPtr rfbScreen, int nWidth, int nHeight)
{
	HDC hdcDesk = ::GetDC(HWND_DESKTOP);
	HDC hdcCopy = CreateCompatibleDC(hdcDesk);
	HBITMAP hBm = CreateCompatibleBitmap(hdcDesk, m_nScreenWidth, m_nScreenHeight);
	HGDIOBJ hOldObject = SelectObject(hdcCopy, hBm);
	BitBlt(hdcCopy, 0, 0, m_nScreenWidth, m_nScreenHeight, hdcDesk, 0, 0, SRCCOPY);

	CURSORINFO cursor = { sizeof(cursor) };
	ICONINFO info = { sizeof(info) };
	if (GetCursorInfo(&cursor) && cursor.flags == CURSOR_SHOWING) {
		RECT rect;
		::GetWindowRect(::GetDesktopWindow(), &rect);
		GetIconInfo(cursor.hCursor, &info);
		const int x = cursor.ptScreenPos.x - rect.left - rect.left - info.xHotspot;
		const int y = cursor.ptScreenPos.y - rect.top - rect.top - info.yHotspot;
		BITMAP bmpCursor = { 0 };
		GetObject(info.hbmColor, sizeof(bmpCursor), &bmpCursor);
		DrawIconEx(hdcCopy, x, y, cursor.hCursor, bmpCursor.bmWidth, bmpCursor.bmHeight,
			0, NULL, DI_NORMAL);

		::DeleteObject(info.hbmColor);
		::DeleteObject(info.hbmMask);
		::DestroyIcon(cursor.hCursor);
	}

	// create a CxImage from the screen grab
	CxImage* image = new CxImage(m_nScreenWidth, m_nScreenHeight, 24);
	GetDIBits(hdcDesk, hBm, 0, m_nScreenHeight, image->GetBits(),
		(LPBITMAPINFO)image->GetDIB(), DIB_RGB_COLORS);

	// clean up the bitmap and dcs
	SelectObject(hdcCopy, hOldObject);

	::ReleaseDC(HWND_DESKTOP, hdcDesk);
	DeleteDC(hdcCopy);
	DeleteObject(hBm);

	uint32_t	nTempChecksum = XXHash32::hash(image->GetBits(), image->GetWidth() * image->GetHeight() * 3, 0);

	if (nTempChecksum == m_nChecksum)
	{
		delete image;
		return FALSE;
	}
	m_nChecksum = nTempChecksum;

	if(nWidth != m_nScreenWidth || nHeight != m_nScreenHeight)
		image->Resample(nWidth, nHeight);

	int				i, j;
	int				nTempFrame, nTempImage;
	for (i = 0; i < nHeight; i++)
	{
		nTempFrame = i * rfbScreen->paddedWidthInBytes;
		nTempImage = (nHeight - i - 1) * nWidth * 3;
		for (j = 0; j < nWidth; j++)
		{
			memcpy(rfbScreen->frameBuffer + nTempFrame + j * 4,
				image->GetBits() + nTempImage + j * 3, 3);
		}
	}

//	m_nChecksum = XXHash32::hash(rfbScreen->frameBuffer, rfbScreen->paddedWidthInBytes * nHeight, 0);

	delete image;

	return TRUE;
}

//void COwnServerDlg::OnTimer(UINT_PTR nIDEvent)
//{
//	// TODO: Add your message handler code here and/or call default
//	if (m_pScreen)
//	{
//		if (rfbIsActive(m_pScreen))
//		{			
//			BOOL		bCaptured = FALSE;
//
//			rfbClientPtr cl;
//			rfbClientIteratorPtr iter = rfbGetClientIterator(m_pScreen);
//			while ((cl = rfbClientIteratorNext(iter)))
//			{
//				OutputA("%d, %d\n", cl->enableCursorShapeUpdates, cl->enableCursorPosUpdates);
//
//				if (!bCaptured)
//				{
//					CaptureScreen(m_pScreen, m_pScreen->width, m_pScreen->height);
//					bCaptured = TRUE;
// 					rfbMarkRectAsModified(m_pScreen, 0, 0, m_pScreen->width - 1, m_pScreen->height - 1);
//					break;
//				}
//
//				rfbMarkRectAsModified(cl->screen, 0, 0, cl->screen->width - 1, cl->screen->height - 1);
//			}
//			rfbReleaseClientIterator(iter);
//
//			rfbProcessEvents(m_pScreen, UPDATE_INTERVAL * 1000);
//		}
//
//	}
//	CDialogEx::OnTimer(nIDEvent);
//}


void COwnServerDlg::OnMenuSettings()
{
	// TODO: Add your command handler code here
	m_nPort = g_param.nPort;
	m_nWidth = g_param.nWidth;
	m_nHeight = g_param.nHeight;
	m_strPwd.Format(L"%S", g_param.szPassword);
	m_strName.Format(L"%S", g_param.szName);
	UpdateData(FALSE);
	m_bVisible = TRUE;
	ShowWindow(SW_SHOW);
}


void COwnServerDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	AfxMessageBox(L"This will be applied next launch.");
	g_param.nPort = m_nPort;
	g_param.nWidth = m_nWidth;
	if (g_param.nWidth & 3)
		g_param.nWidth += 4 - (g_param.nWidth & 3);
	g_param.nHeight = m_nHeight;
	sprintf(g_param.szPassword, "%S", m_strPwd.GetBuffer());
	sprintf(g_param.szName, "%S", m_strName.GetBuffer());
	SaveSettings();
	m_bVisible = FALSE;
	ShowWindow(SW_HIDE);
}


void COwnServerDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	m_bVisible = FALSE;
	ShowWindow(SW_HIDE);
}


void COwnServerDlg::OnBnClickedBtnGetscreen()
{
	// TODO: Add your control notification handler code here
	HDC hdcDesk = ::GetDC(HWND_DESKTOP);
	m_nWidth = ::GetDeviceCaps(hdcDesk, HORZRES);
	m_nHeight = ::GetDeviceCaps(hdcDesk, VERTRES);
	::ReleaseDC(HWND_DESKTOP, hdcDesk);

	UpdateData(FALSE);
}

void COwnServerDlg::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
	if (!m_bVisible)
		lpwndpos->flags &= ~SWP_SHOWWINDOW;

	CDialogEx::OnWindowPosChanging(lpwndpos);

	// TODO: Add your message handler code here
}
