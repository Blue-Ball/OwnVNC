/*-----------------------------
How to use, 
1. Declare CResizeWnd as a member variable
2. Create it at the create time (OnCreate()), and send pointer "this" as parent. 
3. call SetWindowPos() in OnInitDialog(), OnSize() and OnMove(), with the "SWP_NOACTIVATE" to avoid CResizeWnd took dialog focus
-----------------------------*/

#pragma once

// The minimum dialog size
#define	DLG_MIN_WIDTH				(1)
#define	DLG_MIN_HEIGHT				(1)

enum CURSOR_INDEX { CursorArrow, CursorHand, CursorNo, CursorSizeNWSE, CursorSizeNESW, CursorSizeWE, CursorSizeNS, CURSOR_NUM };

// CResizeWnd
class CResizeWnd : public CWnd
{
    DECLARE_DYNAMIC(CResizeWnd)
public:
	
	enum { OFFSET_THRESHOLD = 15, WND_W = 10, WND_H = 10 };
	enum {RESIZE_CORNER_WIDTH = 10, RESIZE_BORDER_WIDTH = 5};

	CResizeWnd();
    virtual ~CResizeWnd();
    BOOL Create(CWnd* pParent);
private:
	void LoadCursorArray();

	CRect	m_rcWindow;
    bool	m_bSizing;		// is Resizing

	CWnd*	m_pParent;
	
	enum RESIZE_STYLE {SE, SW, NW, NE, E, S, W, N, RESIZE_STYLE_COUNT};
	RESIZE_STYLE m_ResizeFrom;
	CRect m_rcResizeArea[RESIZE_STYLE_COUNT];

protected:
    DECLARE_MESSAGE_MAP()
    afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	void SetRegion();
	CURSOR_INDEX GetCursorStyle(CPoint point);
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	CWnd* GetParent();
};
