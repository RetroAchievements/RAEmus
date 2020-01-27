/****************************************************************************
File    : NewTapeImageDlg.h
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Interface of the CNewTapeImageDlg class
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 11.01.2004
*/

#ifndef __NEWTAPEIMAGEDLG_H__
#define __NEWTAPEIMAGEDLG_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


/////////////////////////////////////////////////////////////////////////////
// CNewTapeImageDlg dialog

class CNewTapeImageDlg : public CCommonDlg
{
// Construction
public:
	CNewTapeImageDlg( CWnd *pParent = NULL );   // standard constructor

// Interface: operations
public:

// Interface: attributes
public:
	char m_szTapeName[ MAX_PATH + 1 ];
	int  m_bAttachTape;

// Dialog Data
	//{{AFX_DATA(CNewTapeImageDlg)
	enum { IDD = IDD_NEWTAPEIMAGE };
	//}}AFX_DATA

// Implementation: operations
private:
	void SetDlgState   ();
	void ReceiveFocused();

// Implementation: overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNewTapeImageDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_VIRTUAL

// Implementation: attributes
private:
	char m_szTapeDesc[ CASSETTE_DESCRIPTION_MAX ];

// Generated message map functions
protected:
	//{{AFX_MSG(CNewTapeImageDlg)
	afx_msg void OnKillfocusDescription();
	afx_msg void OnAttachTape();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif //__NEWTAPEIMAGEDLG_H__
