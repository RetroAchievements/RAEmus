/****************************************************************************
File    : WarningDlg.h
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Interface of the CWarningDlg class
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 03.08.2002
*/

#ifndef __WARNINGDLG_H__
#define __WARNINGDLG_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


/////////////////////////////////////////////////////////////////////////////
// CWarningDlg dialog

class CWarningDlg : public CDialog
{
// Construction
public:
	CWarningDlg( CWnd *pParent = NULL );   // standard constructor

// Interface: operations
public:

// Interface: attributes
public:
	char m_szWarningText[ LOADSTRING_SIZE_L + 1 ];
	UINT m_nWarningID;
	UINT m_nWarningFlag;
	BOOL m_bCancel;

// Dialog Data
	//{{AFX_DATA(CWarningDlg)
	enum { IDD = IDD_WARNING };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Implementation: operations
private:
	void SaveDontShowFlag();

// Implementation: attributes
private:

// Implementation: overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWarningDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_VIRTUAL

// Generated message map functions
protected:
	//{{AFX_MSG(CWarningDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif //__WARNINGDLG_H__
