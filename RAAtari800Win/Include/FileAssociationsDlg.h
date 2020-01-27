/****************************************************************************
File    : FileAssociationsDlg.h
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Interface of the CFileAssociationsDlg class
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 13.08.2000
*/

#ifndef __FILEASSOCIATIONSDLG_H__
#define __FILEASSOCIATIONSDLG_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


/////////////////////////////////////////////////////////////////////////////
// CFileAssociationsDlg dialog

class CFileAssociationsDlg : public CCommonDlg, public CFileAssociationsBase
{
// Construction
public:
	CFileAssociationsDlg( CWnd *pParent = NULL );   // standard constructor

// Interface: operations
public:
	// Returns pointer to the window inherited from C<xxx>Base
	virtual CWnd* GetWnd() { return this; };

// Interface: attributes
public:

// Dialog Data
	//{{AFX_DATA(CFileAssociationsDlg)
	enum { IDD = IDD_FILEASSOCIATIONS };
	//}}AFX_DATA

// Implementation: operations
private:
	void SetDlgState();

// Implementation: overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFileAssociationsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_VIRTUAL

// Implementation: attributes
private:
	ULONG m_ulMiscState;

// Generated message map functions
protected:
	//{{AFX_MSG(CFileAssociationsDlg)
	afx_msg void OnSelectAll();
	afx_msg void OnRemoveAll();
	afx_msg void OnAutoReg();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif //__FILEASSOCIATIONSDLG_H__

