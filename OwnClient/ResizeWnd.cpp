// ResizeWnd.cpp : implementation file
//

#include "framework.h"
#include "ResizeWnd.h"
HCURSOR g_hCursorArray[CURSOR_NUM];

// CResizeWnd
IMPLEMENT_DYNAMIC(CResizeWnd, CWnd)
CResizeWnd::CResizeWnd()
{
	m_bSizing = false;
	m_pParent = NULL;
}

CResizeWnd::~CResizeWnd()
{
}

BEGIN_MESSAGE_MAP(CResizeWnd, CWnd)
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_SIZE()
END_MESSAGE_MAP()

// CResizeWnd message handlers
BOOL CResizeWnd::Create(CWnd* pParent)
{
	// Register window class
	CString csClassName = AfxRegisterWndClass(CS_OWNDC|CS_HREDRAW|CS_VREDRAW, ::LoadCursor(NULL, IDC_ARROW), CBrush(::GetSysColor(COLOR_BTNFACE)));
	m_pParent = pParent;

	BOOL bSuccess = CreateEx(NULL,  // Extended style
		csClassName,                // Classname
		_T("CResizeWnd"),			// Title
		WS_POPUP,					// style
		CRect(0, 0, 10, 10),
		pParent,
		NULL,
		NULL);

	if (!bSuccess) {
		return FALSE;
	}

	LoadCursorArray();
	return TRUE;
}

void CResizeWnd::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	dc.FillSolidRect(0, 0, 10, 10, RGB(255, 255, 255));
}

void CResizeWnd::OnSize(UINT nType, int cx, int cy)
{
	SetRegion();
}

int CResizeWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1) {
		return -1;
	}

	::SetWindowLong(GetSafeHwnd(), GWL_EXSTYLE, GetWindowLong(GetSafeHwnd(), GWL_EXSTYLE) | WS_EX_LAYERED);
	::SetLayeredWindowAttributes(this->m_hWnd, 0, 1, LWA_ALPHA);

	return 0;
}

void CResizeWnd::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if (!m_bSizing && !m_pParent->IsZoomed()) {
		SetCursor(g_hCursorArray[GetCursorStyle(point)]);
		SetCapture();
		m_bSizing=true;

		for (auto i = 0; i < RESIZE_STYLE_COUNT; ++i) {
			if (m_rcResizeArea[i].PtInRect(point)) {
				m_ResizeFrom = (RESIZE_STYLE)i;
				break;
			}
		}
	}
	CWnd::OnLButtonDown(nFlags, point);
}

void CResizeWnd::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if (m_bSizing) {
		m_bSizing=false;
		ReleaseCapture();
	}
	CWnd::OnLButtonUp(nFlags, point);
}

void CResizeWnd::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (m_bSizing) {
		CWnd *pParentWnd = m_pParent;
		CRect rc(0, 0, 0, 0);
		CPoint pt(point);
		if (pParentWnd) {
			pParentWnd->GetWindowRect(&rc);
			ClientToScreen(&pt);

			if (SE == m_ResizeFrom) {
				rc.right += pt.x - rc.right;
				rc.bottom += pt.y - rc.bottom;
			} else if (SW == m_ResizeFrom) {
				rc.left += pt.x - rc.left;
				rc.bottom += pt.y - rc.bottom;
			} else if (NW == m_ResizeFrom) {
				rc.left += pt.x - rc.left;
				rc.top += pt.y - rc.top;
			} else if (NE == m_ResizeFrom) {
				rc.right += pt.x - rc.right;
				rc.top += pt.y - rc.top;
			} else if (E == m_ResizeFrom) {
				rc.right += pt.x - rc.right;
			} else if (S == m_ResizeFrom) {
				rc.bottom += pt.y - rc.bottom;
			} else if (W == m_ResizeFrom) {
				rc.left += pt.x - rc.left;
			} else if (N == m_ResizeFrom) {
				rc.top += pt.y - rc.top;
			}

			// minimum dialog size
			int nMinWidth = DLG_MIN_WIDTH;
			int nMinHeight = DLG_MIN_HEIGHT;

			if (rc.Width() < nMinWidth) {
				if (SE == m_ResizeFrom || NE == m_ResizeFrom || E == m_ResizeFrom) {
					rc.right = rc.left + nMinWidth;
				} else if (SW == m_ResizeFrom || NW == m_ResizeFrom || W == m_ResizeFrom) {
					rc.left = rc.right - nMinWidth;
				}
			}

			if (rc.Height() < nMinHeight) {
				if (SE == m_ResizeFrom || SW == m_ResizeFrom || S == m_ResizeFrom) {
					rc.bottom = rc.top + nMinHeight;
				} else if (NE == m_ResizeFrom || NW == m_ResizeFrom || N == m_ResizeFrom) {
					rc.top = rc.bottom - nMinHeight;
				}
			}

			pParentWnd->SetWindowPos(NULL, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOACTIVATE);
		}
		TRACE(_T("rc(%d,%d,%d,%d) point(%d,%d)\n"), rc.left, rc.top, rc.right, rc.bottom, point.x, point.y);
	} else if (!m_pParent->IsZoomed()) {
		SetCursor(g_hCursorArray[GetCursorStyle(point)]);
	}

	CWnd::OnMouseMove(nFlags, point);
}

void CResizeWnd::SetRegion()
{
	if (!m_pParent) {
		return;
	}

	CWnd *pParentWnd = m_pParent;
	CRect rcMain;
	pParentWnd->GetWindowRect(&rcMain);
	ScreenToClient(rcMain);

	HRGN hRgnBorder;
	hRgnBorder = CreateRectRgn(0, 0, 0, 0);

	HRGN hRgnCorner;

	// set upper-left corner
	m_rcResizeArea[NW] = CRect(0, 0, RESIZE_CORNER_WIDTH, RESIZE_CORNER_WIDTH);
	hRgnCorner = CreateRectRgnIndirect(m_rcResizeArea[NW]);
	::CombineRgn(hRgnBorder, hRgnBorder, hRgnCorner, RGN_OR);

	// set bottom-right corner
	m_rcResizeArea[SE] = CRect(rcMain.right-RESIZE_CORNER_WIDTH, rcMain.bottom-RESIZE_CORNER_WIDTH, rcMain.right, rcMain.bottom);
	hRgnCorner = CreateRectRgnIndirect(m_rcResizeArea[SE]);
	::CombineRgn(hRgnBorder, hRgnBorder, hRgnCorner, RGN_OR);

	// set upper-right corner
	m_rcResizeArea[NE] = CRect(rcMain.right-RESIZE_CORNER_WIDTH, 0, rcMain.right, RESIZE_CORNER_WIDTH);
	hRgnCorner = CreateRectRgnIndirect(m_rcResizeArea[NE]);
	::CombineRgn(hRgnBorder, hRgnBorder, hRgnCorner, RGN_OR);

	// set bottom-left corner
	m_rcResizeArea[SW] = CRect(0, rcMain.bottom-RESIZE_CORNER_WIDTH, RESIZE_CORNER_WIDTH, rcMain.bottom);
	hRgnCorner = CreateRectRgnIndirect(m_rcResizeArea[SW]);
	::CombineRgn(hRgnBorder, hRgnBorder, hRgnCorner, RGN_OR);

	// set upper border
	m_rcResizeArea[N] = CRect(0, 0, rcMain.right, RESIZE_BORDER_WIDTH);
	hRgnCorner = CreateRectRgnIndirect(m_rcResizeArea[N]);
	::CombineRgn(hRgnBorder, hRgnBorder, hRgnCorner, RGN_OR);

	// set bottom border
	m_rcResizeArea[S] = CRect(0, rcMain.bottom-RESIZE_BORDER_WIDTH, rcMain.right, rcMain.bottom);
	hRgnCorner = CreateRectRgnIndirect(m_rcResizeArea[S]);
	::CombineRgn(hRgnBorder, hRgnBorder, hRgnCorner, RGN_OR);

	// set left border
	m_rcResizeArea[W] = CRect(0, 0, RESIZE_BORDER_WIDTH, rcMain.bottom);
	hRgnCorner = CreateRectRgnIndirect(m_rcResizeArea[W]);
	::CombineRgn(hRgnBorder, hRgnBorder, hRgnCorner, RGN_OR);

	// set right border
	m_rcResizeArea[E] = CRect(rcMain.right-RESIZE_BORDER_WIDTH, 0, rcMain.right, rcMain.bottom);
	hRgnCorner = CreateRectRgnIndirect(m_rcResizeArea[E]);
	::CombineRgn(hRgnBorder, hRgnBorder, hRgnCorner, RGN_OR);
	
	SetWindowRgn(hRgnBorder, FALSE);
}

CURSOR_INDEX CResizeWnd::GetCursorStyle(CPoint point)
{
	// LoadCursor IDC_SIZENWSE
	if (m_rcResizeArea[NW].PtInRect(point) || m_rcResizeArea[SE].PtInRect(point)) {
		return CursorSizeNWSE;
	}

	// LoadCursor IDC_SIZENESW
	if (m_rcResizeArea[NE].PtInRect(point) || m_rcResizeArea[SW].PtInRect(point)) {
		return CursorSizeNESW;
	}

	// LoadCursor IDC_SIZENS
	if (m_rcResizeArea[N].PtInRect(point) || m_rcResizeArea[S].PtInRect(point)) {
		return CursorSizeNS;
	}

	// LoadCursor IDC_SIZEWE
	if (m_rcResizeArea[W].PtInRect(point) || m_rcResizeArea[E].PtInRect(point)) {
		return CursorSizeWE;
	}

	return CursorArrow;
}

void CResizeWnd::LoadCursorArray()
{
	g_hCursorArray[CursorArrow]=::LoadCursor(NULL, IDC_ARROW);
	g_hCursorArray[CursorHand]=::LoadCursor(NULL, MAKEINTRESOURCE(32649));
	g_hCursorArray[CursorNo]=::LoadCursor(NULL, IDC_NO);
	g_hCursorArray[CursorSizeNWSE]=::LoadCursor(NULL, IDC_SIZENWSE);
	g_hCursorArray[CursorSizeNESW]=::LoadCursor(NULL, IDC_SIZENESW);
	g_hCursorArray[CursorSizeWE]=::LoadCursor(NULL, IDC_SIZEWE);
	g_hCursorArray[CursorSizeNS]=::LoadCursor(NULL, IDC_SIZENS);
}

CWnd *CResizeWnd::GetParent()
{
	return m_pParent;
}