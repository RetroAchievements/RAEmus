/****************************************************************************
File    : ErrorLogDlg.h
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Interface of the CErrorLogDlg class
@(#) #BY# Tomasz Szymankowski, Richard Lawrence
@(#) #LM# 30.01.2001
*/

#ifndef __ERRORLOGDLG_H__
#define __ERRORLOGDLG_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


/////////////////////////////////////////////////////////////////////////////
// CErrorLogDlg dialog

class CErrorLogDlg : public CDialog
{
// Construction
public:
	CErrorLogDlg( CWnd *pParent = NULL );   // standard constructor

// Interface: operations
public:

// Interface: attributes
public:

// Dialog Data
	//{{AFX_DATA(CErrorLogDlg)
	enum { IDD = IDD_ERRORLOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Implementation: operations
private:

// Implementation: overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CErrorLogDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation: attributes
private:
	CRect m_rcGripper;
	int   m_nEditXSub;
	int   m_nEditYSub;
	int   m_nButnXSub;
	int   m_nButnYSub;

// Generated message map functions
protected:
	//{{AFX_MSG(CErrorLogDlg)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg LRESULT OnNcHitTest(CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif //__ERRORLOGDLG_H__
