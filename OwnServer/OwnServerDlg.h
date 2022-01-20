
// OwnServerDlg.h : header file
//

#pragma once

#include <rfb/rfb.h>
#include "ximage.h"

// COwnServerDlg dialog
class COwnServerDlg : public CDialogEx
{
// Construction
public:
	COwnServerDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_OWNSERVER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

// Implementation
protected:
	HICON m_hIcon;
	NOTIFYICONDATA m_nid;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	virtual BOOL DestroyWindow();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnMenuExit();
	afx_msg void OnMenuSettings();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	DECLARE_MESSAGE_MAP()

public:
	LRESULT OnDialogShown(WPARAM, LPARAM);
	LRESULT onShowTask(WPARAM wParam, LPARAM lParam);
	void ToTray();
	void CaptureScreen(rfbScreenInfoPtr rfbScreen, int nWidth, int nHeight);

public:
	rfbScreenInfoPtr	m_pScreen;
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	int m_nPort;
	int m_nWidth;
	int m_nHeight;
	int	m_nScreenWidth;
	int	m_nScreenHeight;
	CString m_strPwd;
	BOOL		m_bVisible;
	afx_msg void OnBnClickedBtnGetscreen();
	afx_msg void OnWindowPosChanging(WINDOWPOS* lpwndpos);
};
