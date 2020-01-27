/****************************************************************************
File    : CommonDlg.h
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Interface of the CCommonDlg class
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 18.07.2002
*/

#ifndef __COMMONDLG_H__
#define __COMMONDLG_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


/////////////////////////////////////////////////////////////////////////////
// CCommonDlg dialog

class CCommonDlg : public CDialog
{
// Construction/destruction
public:
	CCommonDlg( UINT nIDTemplate, CWnd *pParentWnd = NULL );
	virtual ~CCommonDlg();

// Interface: operations
public:
	UINT GetIDTemplate() { return m_nIDTemplate; };

// Interface: attributes
public:

// Implementation: operations
private:
	void HelpInfo( HELPINFO *pHelpInfo, UINT nFirstCtrl, UINT nLastCtrl );

// Implementation: overrides
protected:
	virtual BOOL OnInitDialog();

// Implementation: attributes
private:
	UINT m_nIDTemplate;

protected:
	UINT m_nFirstCtrl;
	UINT m_nLastCtrl;
	BOOL m_bStateInit;

// Generated message map functions
protected:
	//{{AFX_MSG(CCommonDlg)
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif //__COMMONDLG_H__
