/****************************************************************************
File    : Atari800WinView.h
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Interface of the CAtari800WinView class
@(#) #BY# Richard Lawrence, Tomasz Szymankowski
@(#) #LM# 17.09.2002
*/

#ifndef __ATARI800WINVIEW_H__
#define __ATARI800WINVIEW_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


/////////////////////////////////////////////////////////////////////////////
// CAtari800WinView:
// See Atari800WinView.cpp for the implementation of this class
//

class CAtari800WinView : public CView
{
// Construction/destruction
protected:
	// Create from serialization only
	DECLARE_DYNCREATE(CAtari800WinView)

	CAtari800WinView();
	virtual ~CAtari800WinView();

// Interface: operations
public:

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Interface: attributes
public:
	CAtari800WinDoc* GetDocument();

// Implementation: operations
private:
	void StartAtariMachine();
	void EraseView ( CDC *pDC );
	void RedrawView( CDC *pDC, BOOL bAllowRedraw = TRUE );

// Implementation: overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAtari800WinView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual void OnInitialUpdate();
	protected:
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation: attributes
private:
	CAtari800WinApp *m_pMainApp;
	CMainFrame      *m_pMainWindow;

// Generated message map functions
protected:
	//{{AFX_MSG(CAtari800WinView)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnDestroy();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in Atari800WinView.cpp
inline CAtari800WinDoc* CAtari800WinView::GetDocument()
   { return (CAtari800WinDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif //__ATARI800WINVIEW_H__
