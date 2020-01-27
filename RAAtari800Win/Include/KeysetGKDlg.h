/****************************************************************************
File    : KeysetGKDlg.h
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Interface of the CKeysetGKDlg class
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 15.04.2000
*/

#ifndef __KEYSETGKDLG_H__
#define __KEYSETGKDLG_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


/////////////////////////////////////////////////////////////////////////////
// CKeysetGKDlg dialog

class CKeysetGKDlg : public CDialog
{
// Construction
public:
	CKeysetGKDlg( WORD *pwKeyset, int nKeyIndex, CWnd *pParent = NULL );   // standard constructor

// Interface: operations
public:

// Interface: attributes
public:

// Dialog Data
	//{{AFX_DATA(CKeysetGKDlg)
	enum { IDD = IDD_KEYSETGETKEY };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Implementation: operations
private:

// Implementation: overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CKeysetGKDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation: attributes
private:
	int   m_nKeyIndex;
	WORD *m_pwKeyset;

// Generated message map functions
protected:
	//{{AFX_MSG(CKeysetGKDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg UINT OnGetDlgCode();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif //__KEYSETGKDLG_H__
