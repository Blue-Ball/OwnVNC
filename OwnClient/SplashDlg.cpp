
// OwnServerDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "OwnClient.h"
#include "SplashDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CSplashDlg dialog

CSplashDlg::CSplashDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SPLASH_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSplashDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CSplashDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_ERASEBKGND()
	ON_WM_TIMER()
END_MESSAGE_MAP()


BOOL CSplashDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	m_imgSplash.LoadResource(FindResource(GetModuleHandle(0), MAKEINTRESOURCE(IDB_PNG_LOGO), _T("PNG")), CXIMAGE_FORMAT_PNG);

	MoveWindow(0, 0, m_imgSplash.GetWidth(), m_imgSplash.GetHeight());

	CRgn crRgn, crRgnTmp;

	crRgn.CreateRectRgn(0, 0, 0, 0);
	
	int iX = 0;
	int iY;
	for (iY = 0; iY < m_imgSplash.GetHeight(); iY++)
	{
		do
		{
			//skip over transparent pixels at start of lines.
			while (iX <= m_imgSplash.GetWidth() && m_imgSplash.AlphaGet(iX, iY) == 0)
				iX++;
			//remember this pixel
			int iLeftX = iX;
			//now find first non transparent pixel
			while (iX <= m_imgSplash.GetWidth() && m_imgSplash.AlphaGet(iX, iY) != 0)
				++iX;
			//create a temp region on this info
			crRgnTmp.CreateRectRgn(iLeftX, (m_imgSplash.GetHeight()-1)-iY, iX, (m_imgSplash.GetHeight() - 1) - (iY + 1));
			//combine into main region.
			crRgn.CombineRgn(&crRgn, &crRgnTmp, RGN_OR);
			//delete the temp region for next pass (otherwise you'll get an ASSERT)
			crRgnTmp.DeleteObject();
		} while (iX < m_imgSplash.GetWidth());
		iX = 0;
	}

	SetWindowRgn(crRgn, TRUE);

	CenterWindow();
	UpdateWindow();

	SetTimer(0, 3000, NULL);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSplashDlg::OnPaint()
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
	m_imgSplash.Draw(dc.m_hDC);
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CSplashDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

BOOL CSplashDlg::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default
	return CDialogEx::OnEraseBkgnd(pDC);
}

void CSplashDlg::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class

	// CDialogEx::OnOK();
}


void CSplashDlg::OnCancel()
{
	// TODO: Add your specialized code here and/or call the base class

	// CDialogEx::OnCancel();
}


void CSplashDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	KillTimer(0);
	EndDialog(IDOK);
	CDialogEx::OnTimer(nIDEvent);
}
