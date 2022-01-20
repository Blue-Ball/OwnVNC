
// OwnClientDlg.h : header file
//

#pragma once

#include "ResizeWnd.h"
#include <rfb/rfbclient.h>
#include "ximage.h"

// COwnClientDlg dialog
class COwnClientDlg : public CDialogEx
{
// Construction
public:
	COwnClientDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_OWNCLIENT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMove(int x, int y);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	virtual BOOL DestroyWindow();
	LRESULT OnDialogShown(WPARAM, LPARAM);

public:
	CResizeWnd			m_WndResize;
	rfbClient* m_pClient;
	CxImage				m_imgDraw;
	CPoint				ConvertPointToClient(CPoint pt);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
};
