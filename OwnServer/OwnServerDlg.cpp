
// OwnServerDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "OwnServer.h"
#include "OwnServerDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define FPS 25
#define UPDATE_INTERVAL (CLOCKS_PER_SEC/FPS)
#define WM_FIRST_SHOWN WM_USER + 100
#define WM_SHOWTASK WM_USER+101

// COwnServerDlg dialog

COwnServerDlg::COwnServerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_OWNSERVER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void COwnServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(COwnServerDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_COMMAND(ID_MENU_EXIT, &COwnServerDlg::OnMenuExit)
	ON_MESSAGE(WM_SHOWTASK, onShowTask)
	ON_MESSAGE(WM_FIRST_SHOWN, OnDialogShown)
	ON_WM_TIMER()
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

static void doptr(int buttonMask, int x, int y, rfbClientPtr cl)
{
	char buffer[1024];
	if (buttonMask) {
		sprintf(buffer, "Ptr: mouse button mask 0x%x at %d,%d\n", buttonMask, x, y);
		printf("%s", buffer);
// 		output(cl->screen, buffer);
	}
	rfbDefaultPtrAddEvent(buttonMask, x, y, cl);
}

LRESULT COwnServerDlg::OnDialogShown(WPARAM, LPARAM)
{
	ToTray();
	ShowWindow(SW_HIDE);

	HDC hdcDesk = ::GetDC(HWND_DESKTOP);
	int nScreenWidth = ::GetDeviceCaps(hdcDesk, HORZRES);
	int nScreenHeight = ::GetDeviceCaps(hdcDesk, VERTRES);
	::ReleaseDC(HWND_DESKTOP, hdcDesk);

	if (theApp.m_param.nWidth == -1)
		theApp.m_param.nWidth = nScreenWidth;
	if (theApp.m_param.nHeight == -1)
		theApp.m_param.nHeight = nScreenHeight;

	if (theApp.m_param.nWidth & 3)
		theApp.m_param.nWidth += 4 - (theApp.m_param.nWidth & 3);

	m_pScreen = rfbGetScreen(NULL, NULL, theApp.m_param.nWidth, theApp.m_param.nHeight, 8, 3, 4);
	m_pScreen->port = theApp.m_param.nPort;
	m_pScreen->frameBuffer = (char*)malloc(theApp.m_param.nWidth * theApp.m_param.nHeight * 4);
	m_pScreen->alwaysShared = TRUE;
	m_pScreen->ptrAddEvent = doptr;
	
	char** passwds = (char **)malloc(sizeof(char**) * 2);
	passwds[0] = theApp.m_param.szPassword;
	passwds[1] = NULL;
	m_pScreen->authPasswdData = (void*)passwds;
	m_pScreen->passwordCheck = rfbCheckPasswordByList;

	rfbInitServer(m_pScreen);

	m_imgCapture.Create(theApp.m_param.nWidth, theApp.m_param.nHeight, 24);

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

	if(m_pScreen)
		free(m_pScreen->frameBuffer);

	return CDialogEx::DestroyWindow();
}

void COwnServerDlg::CaptureScreen(rfbScreenInfoPtr rfbScreen, int nWidth, int nHeight)
{
	HDC hdcDesk = ::GetDC(HWND_DESKTOP);
	int nScreenWidth = GetDeviceCaps(hdcDesk, HORZRES);
	int nScreenHeight = GetDeviceCaps(hdcDesk, VERTRES);
	HDC hdcCopy = CreateCompatibleDC(hdcDesk);
	HBITMAP hBm = CreateCompatibleBitmap(hdcDesk, nScreenWidth, nScreenHeight);
	SelectObject(hdcCopy, hBm);
	BitBlt(hdcCopy, 0, 0, nScreenWidth, nScreenHeight, hdcDesk, 0, 0, SRCCOPY);

	// create a CxImage from the screen grab
	CxImage* image = new CxImage(nScreenWidth, nScreenHeight, 24);
	GetDIBits(hdcDesk, hBm, 0, nScreenHeight, image->GetBits(),
		(LPBITMAPINFO)image->GetDIB(), DIB_RGB_COLORS);
	// image->CreateFromHBITMAP(hBm);

	// clean up the bitmap and dcs
	::ReleaseDC(HWND_DESKTOP, hdcDesk);
	DeleteDC(hdcCopy);
	DeleteObject(hBm);

	image->Resample(nWidth, nHeight);
	int				i, j;
	for (i = 0; i < nHeight; i++)
	{
		// memcpy(rfbScreen->frameBuffer + i * nWidth * 3, image->GetBits(nHeight - i - 1), nWidth * 3);
		for (j = 0; j < nWidth; j++)
		{
			memcpy(rfbScreen->frameBuffer + i * rfbScreen->paddedWidthInBytes + j * 4,
				image->GetBits() + (nHeight - i - 1) * nWidth * 3 + j * 3, 3);
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
			
			CaptureScreen(m_pScreen, theApp.m_param.nWidth, theApp.m_param.nHeight);

			rfbClientPtr cl;
			rfbClientIteratorPtr iter = rfbGetClientIterator(m_pScreen);
			while ((cl = rfbClientIteratorNext(iter)))
			{
				rfbMarkRectAsModified(cl->screen, 0, 0, cl->screen->width - 1, cl->screen->height - 1);
			}
			rfbReleaseClientIterator(iter);

			//rfbProcessEvents(m_pScreen, UPDATE_INTERVAL * 1000);
		}

	}
	CDialogEx::OnTimer(nIDEvent);
}
