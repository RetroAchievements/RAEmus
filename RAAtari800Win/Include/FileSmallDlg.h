/****************************************************************************
File    : FileSmallDlg.h
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Interface of the CFileSmallDlg class
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 30.01.2001
*/

#ifndef __FILESMALLDLG_H__
#define __FILESMALLDLG_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


#include "ShellTree.h"

/////////////////////////////////////////////////////////////////////////////
// CFileSmallDlg dialog

class CFileSmallDlg : public CDialog
{
// Construction
public:
	CFileSmallDlg( BOOL bOpenFileDialog, LPCSTR pszDefExt = NULL, LPCSTR pszTitle = NULL, LPCSTR pszStateFile = NULL, BOOL bFoldersOnly = FALSE, CWnd *pParent = NULL );   // standard constructor

// Interface: operations
public:
	CString	GetPathName();

// Interface: attributes
public:

// Dialog Data
	//{{AFX_DATA(CFileSmallDlg)
	enum { IDD = IDD_FILE_SMALL };
	CShellTree	m_treeShell;
	//}}AFX_DATA

// Implementation: operations
private:

// Implementation: overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFileSmallDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_VIRTUAL

// Implementation: attributes
private:
	BOOL    m_bOpenFileDialog;
	CString m_strDefExt;
	BOOL    m_bFoldersOnly;
	CString m_strDlgTitle;
	CString m_strSelectedPath;
	CRect   m_rcGripper;

	int  m_nTreeXSub;
	int  m_nTreeYSub;
	int  m_nEditXSub;
	int  m_nEditYSub;
	int  m_nEditX;
	int  m_nEditY;
	int  m_nOkXSub;
	int  m_nOkYSub;
	int  m_nCancelXSub;
	int  m_nCancelYSub;

// Generated message map functions
protected:
	//{{AFX_MSG(CFileSmallDlg)
	afx_msg void OnItemExpanding(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangedTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeleteItemTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg LRESULT OnNcHitTest(CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif //__FILESMALLDLG_H__
