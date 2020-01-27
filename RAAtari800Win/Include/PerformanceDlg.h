/****************************************************************************
File    : PerformanceDlg.h
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Interface of the CPerformanceDlg class
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 27.04.2002
*/

#ifndef __PERFORMANCEDLG_H__
#define __PERFORMANCEDLG_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


/////////////////////////////////////////////////////////////////////////////
// CPerformanceDlg dialog

class CPerformanceDlg : public CCommonDlg
{
// Construction
public:
	CPerformanceDlg( CWnd *pParent = NULL );   // standard constructor

// Interface: operations
public:

// Interface: attributes
public:

// Dialog Data
	//{{AFX_DATA(CPerformanceDlg)
	enum { IDD = IDD_PERFORMANCE };
	//}}AFX_DATA

// Implementation: operations
private:
	void SetDlgState   ();
	void ReceiveFocused();

// Implementation: overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPerformanceDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_VIRTUAL

// Implementation: attributes
private:
	ULONG m_ulMiscState;
	int   m_nRefreshRate;
	int   m_nRefreshDoubleWnd;
	int   m_nRefreshFullSpeed;
	int   m_nSpeedPercent;

// Generated message map functions
protected:
	//{{AFX_MSG(CPerformanceDlg)
	afx_msg void OnSelchangeSpeedPercent();
	afx_msg void OnFullSpeed();
	afx_msg void OnKillfocusRefreshRate();
	afx_msg void OnDeltaposRefreshSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDoubleRefreshCheck();
	afx_msg void OnKillfocusDoubleRefreshRate();
	afx_msg void OnDeltaposDoubleRefreshSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSpeedRefreshCheck();
	afx_msg void OnKillfocusSpeedRefreshRate();
	afx_msg void OnDeltaposSpeedRefreshSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNoDrawDisplay();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif //__PERFORMANCEDLG_H__
