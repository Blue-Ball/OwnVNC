
// OwnServerDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "OwnServer.h"
#include "OwnServerDlg.h"
#include "afxdialogex.h"
#include "Global.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define FPS 25
#define UPDATE_INTERVAL (CLOCKS_PER_SEC/FPS)
#define WM_FIRST_SHOWN WM_USER + 100
#define WM_SHOWTASK WM_USER + 101

// COwnServerDlg dialog
COwnServerDlg* g_pDlg = NULL;

COwnServerDlg::COwnServerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_OWNSERVER_DIALOG, pParent)
	, m_nPort(0)
	, m_nWidth(0)
	, m_nHeight(0)
	, m_strPwd(_T(""))
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
}

BEGIN_MESSAGE_MAP(COwnServerDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_COMMAND(ID_MENU_EXIT, &COwnServerDlg::OnMenuExit)
	ON_MESSAGE(WM_SHOWTASK, onShowTask)
	ON_MESSAGE(WM_FIRST_SHOWN, OnDialogShown)
	ON_WM_TIMER()
	ON_COMMAND(ID_MENU_SETTINGS, &COwnServerDlg::OnMenuSettings)
	ON_BN_CLICKED(IDOK, &COwnServerDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &COwnServerDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BTN_GETSCREEN, &COwnServerDlg::OnBnClickedBtnGetscreen)
	ON_WM_WINDOWPOSCHANGING()
END_MESSAGE_MAP()


// COwnServerDlg message handlers

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

BOOL COwnServerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	PostMessage(WM_FIRST_SHOWN);

	m_pScreen = NULL;
	g_pDlg = this;
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

LRESULT COwnServerDlg::OnDialogShown(WPARAM, LPARAM)
{
	ToTray();
	ShowWindow(SW_HIDE);

	m_pScreen = rfbGetScreen(NULL, NULL, g_param.nWidth, g_param.nHeight, 8, 3, 4);
	m_pScreen->desktopName = "OwnServer";
	m_pScreen->port = g_param.nPort;
	m_pScreen->frameBuffer = (char*)malloc(g_param.nWidth * g_param.nHeight * 4);
	m_pScreen->alwaysShared = TRUE;
	m_pScreen->ptrAddEvent = doptr;
	
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

	SetTimer(0, UPDATE_INTERVAL, NULL);

	// 	while (1) {
// 		rfbProcessEvents(server, UPDATE_INTERVAL*1000);
// 	}
	rfbRunEventLoop(m_pScreen, UPDATE_INTERVAL * 1000, TRUE);

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
	lstrcpy(m_nid.szTip, L"OwnServer");
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

void COwnServerDlg::CaptureScreen(rfbScreenInfoPtr rfbScreen, int nWidth, int nHeight)
{
	HDC hdcDesk = ::GetDC(HWND_DESKTOP);
	m_nScreenWidth = GetDeviceCaps(hdcDesk, HORZRES);
	m_nScreenHeight = GetDeviceCaps(hdcDesk, VERTRES);
	HDC hdcCopy = CreateCompatibleDC(hdcDesk);
	HBITMAP hBm = CreateCompatibleBitmap(hdcDesk, m_nScreenWidth, m_nScreenHeight);
	SelectObject(hdcCopy, hBm);
	BitBlt(hdcCopy, 0, 0, m_nScreenWidth, m_nScreenHeight, hdcDesk, 0, 0, SRCCOPY);

	// create a CxImage from the screen grab
	CxImage* image = new CxImage(m_nScreenWidth, m_nScreenHeight, 24);
	GetDIBits(hdcDesk, hBm, 0, m_nScreenHeight, image->GetBits(),
		(LPBITMAPINFO)image->GetDIB(), DIB_RGB_COLORS);
	// image->CreateFromHBITMAP(hBm);

	// clean up the bitmap and dcs
	::ReleaseDC(HWND_DESKTOP, hdcDesk);
	DeleteDC(hdcCopy);
	DeleteObject(hBm);

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

	delete image;
}
void COwnServerDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	if (m_pScreen)
	{
		if (rfbIsActive(m_pScreen))
		{
			int end = clock();
			
			BOOL		bCaptured = FALSE;

			rfbClientPtr cl;
			rfbClientIteratorPtr iter = rfbGetClientIterator(m_pScreen);
			while ((cl = rfbClientIteratorNext(iter)))
			{
				if (!bCaptured)
				{
					CaptureScreen(m_pScreen, m_pScreen->width, m_pScreen->height);
					bCaptured = TRUE;
 					rfbMarkRectAsModified(m_pScreen, 0, 0, m_pScreen->width - 1, m_pScreen->height - 1);
 					break;
				}
//				rfbMarkRectAsModified(cl->screen, 0, 0, cl->screen->width - 1, cl->screen->height - 1);
			}
			rfbReleaseClientIterator(iter);

			//rfbProcessEvents(m_pScreen, UPDATE_INTERVAL * 1000);
		}

	}
	CDialogEx::OnTimer(nIDEvent);
}


void COwnServerDlg::OnMenuSettings()
{
	// TODO: Add your command handler code here
	m_nPort = g_param.nPort;
	m_nWidth = g_param.nWidth;
	m_nHeight = g_param.nHeight;
	m_strPwd.Format(L"%S", g_param.szPassword);
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
