/****************************************************************************
File    : StatusTray.h
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Interface of the CStatusTray class
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 04.10.2001
*/

#ifndef __STATUSTRAY_H__
#define __STATUSTRAY_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


/////////////////////////////////////////////////////////////////////////////
// CStatusTray control

class CStatusTray : public CStatusBar
{
// Type definitions
public:
	struct StatusIconInfo_t
	{
		UINT  nPane;
		UINT (*pfGetBmpID)();
	};

// Construction
public:
	CStatusTray();
	~CStatusTray();

// Interface: operations
public:
	void ConfigureTray   ( UINT *pIndicators, int nPaneNo, BOOL bRedraw = TRUE, CWnd *pParent = NULL );
	void UpdatePane      ( int nPane );
	int  GetPaneFromPoint( CPoint &pt );

	virtual void DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct );

// Interface: attributes
public:

// Implementation: operations
private:
	BOOL LoadMappedBitmap( UINT dwBmpID, int nPane );

// Implementation: attributes
private:
	CWnd    *m_pParent;
	CBitmap *m_pIcons;
	UINT    *m_pBmpIDs;
	BOOL     m_bInitilized;

// Generated message map functions
protected:
	//{{AFX_MSG(CStatusTray)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif //__STATUSTRAY_H__
