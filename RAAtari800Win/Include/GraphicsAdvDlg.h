/****************************************************************************
File    : GraphicsAdvDlg.h
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Interface of the CGraphicsAdvDlg class
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 14.10.2003
*/

#ifndef __GRAPHICSADVDLG_H__
#define __GRAPHICSADVDLG_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


/////////////////////////////////////////////////////////////////////////////
// CGraphicsAdvDlg dialog

class CGraphicsAdvDlg : public CCommonDlg
{
// Construction
public:
	CGraphicsAdvDlg( ULONG *pScreenState, int *pMemoryType, int *pRefreshRate, ULONG ulScreenMode, CWnd *pParent = NULL );   // standard constructor

// Interface: operations
public:

// Interface: attributes
public:

// Dialog Data
	//{{AFX_DATA(CGraphicsAdvDlg)
	enum { IDD = IDD_GRAPHICSADV };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Implementation: operations
private:
	void PopulateRefreshRateList();
	void SetDlgState();

// Implementation: overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGraphicsAdvDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_VIRTUAL

// Implementation: attributes
private:
	ULONG *m_pScreenState;
	ULONG  m_ulScreenState;
	int   *m_pMemoryType;
	int    m_nMemoryType;
	int    m_nRefreshRate;
	int   *m_pRefreshRate;
	ULONG  m_ulScreenMode;

// Generated message map functions
protected:
	//{{AFX_MSG(CGraphicsAdvDlg)
	afx_msg void OnSelchangeBlitMode();
	afx_msg void OnSelchangeRefreshRate();
	afx_msg void OnWaitVbl();
	afx_msg void OnSafeMode();
	afx_msg void OnSelchangeMemoryType();
	afx_msg void OnSelchangeBlitEffects();
	afx_msg void OnOptimizePro();
	afx_msg void OnUseMMX();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif //__GRAPHICSADVDLG_H__
