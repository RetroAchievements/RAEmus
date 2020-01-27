/****************************************************************************
File    : MouseDlg.h
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Interface of the CMouseDlg class
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 04.08.2002
*/

#ifndef __MOUSEDLG_H__
#define __MOUSEDLG_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


/////////////////////////////////////////////////////////////////////////////
// CMouseDlg dialog

class CMouseDlg : public CCommonDlg
{
// Construction
public:
	CMouseDlg( CWnd *pParent = NULL );   // standard constructor

// Interface: operations
public:

// Interface: attributes
public:

// Dialog Data
	//{{AFX_DATA(CMouseDlg)
	enum { IDD = IDD_MOUSE };
	//}}AFX_DATA

// Implementation: operations
private:
	void SetDlgState   ();
	void ReceiveFocused();

// Implementation: overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMouseDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_VIRTUAL

// Implementation: attributes
private:
	ULONG m_ulInputState;
	int   m_nMouseMode;
	int   m_nMousePort;
	int   m_nMouseSpeed;
	int   m_nPotMin;
	int   m_nPotMax;
	int   m_nJoyInertia;
	int   m_nPenOffsetX;
	int   m_nPenOffsetY;

// Generated message map functions
protected:
	//{{AFX_MSG(CMouseDlg)
	afx_msg void OnDeltaposPenHorzSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusPenHorzOffset();
	afx_msg void OnDeltaposPenVertSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusPenVertOffset();
	afx_msg void OnDeltaposMouseSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusMouseSpeed();
	afx_msg void OnDeltaposMinSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusMinPot();
	afx_msg void OnDeltaposMaxSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusMaxPot();
	afx_msg void OnDeltaposJoySpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusJoyInertia();
	afx_msg void OnSelchangeEmulatedDevice();
	afx_msg void OnSelchangePort();
	afx_msg void OnMouseCapture();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif //__MOUSEDLG_H__
