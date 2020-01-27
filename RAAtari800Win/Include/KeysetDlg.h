/****************************************************************************
File    : KeysetDlg.h
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Interface of the CKeysetDlg class
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 18.03.2001
*/

#ifndef __KEYSETDLG_H__
#define __KEYSETDLG_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


/////////////////////////////////////////////////////////////////////////////
// CKeysetDlg dialog

class CKeysetDlg : public CCommonDlg
{
// Construction
public:
	CKeysetDlg( CWnd *pParent = NULL );   // standard constructor

// Interface: operations
public:
	void SetTitlePostfix ( LPCSTR pszPostfix );
	void SetCurrentKeyset( int nKeyset );

// Interface: attributes
public:

// Dialog Data
	//{{AFX_DATA(CKeysetDlg)
	enum { IDD = IDD_KEYSET };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Implementation: operations
private:
	void SetDlgState();

// Implementation: overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CKeysetDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_VIRTUAL

// Implementation: attributes
private:
	CString m_strTitlePostfix;
	int		m_nCurrentKeyset;
	BOOL	m_bChanged;

// Generated message map functions
protected:
	//{{AFX_MSG(CKeysetDlg)
	afx_msg void OnKeysetNw();
	afx_msg void OnKeysetNorth();
	afx_msg void OnKeysetNe();
	afx_msg void OnKeysetEast();
	afx_msg void OnKeysetSe();
	afx_msg void OnKeysetSouth();
	afx_msg void OnKeysetSw();
	afx_msg void OnKeysetWest();
	afx_msg void OnKeysetFire();
	afx_msg void OnKeysetCentre();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif //__KEYSETDLG_H__
