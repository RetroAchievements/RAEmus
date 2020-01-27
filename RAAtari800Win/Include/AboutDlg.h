/****************************************************************************
File    : AboutDlg.h
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Interface of the CAboutDlg class
@(#) #BY# Richard Lawrence, Tomasz Szymankowski
@(#) #LM# 16.03.2002
*/

#ifndef __ABOUTDLG_H__
#define __ABOUTDLG_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog

class CAboutDlg : public CDialog
{
// Construction
public:
	CAboutDlg( CWnd *pParent = NULL );   // standard constructor

// Interface: operations
public:

// Interface: attributes
public:

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

// Implementation: operations
private:

// Implementation: attributes
private:
	static UINT m_nLastIcon;

// Implementation: overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Generated message map functions
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif //__ABOUTDLG_H__
