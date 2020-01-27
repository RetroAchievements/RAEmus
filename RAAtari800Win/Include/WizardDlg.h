/****************************************************************************
File    : WizardDlg.h
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Interface of the CWizardDlg/CWizardPage classes
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 07.03.2001
*/

#ifndef __WIZARDDLG_H__
#define __WIZARDDLG_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


/////////////////////////////////////////////////////////////////////////////
// CWizardPage dialog

class CWizardPage : public CCommonDlg
{
// Construction
public:
	CWizardPage( UINT nIDTemplate, CWnd *pParent = NULL );   // standard constructor
	virtual ~CWizardPage() {};

// Interface: operations
public:

// Interface: overrides
public:
	virtual void Commit() = 0;
	virtual BOOL GetReboot() { return m_bReboot; };

// Implementation: attributes
protected:
	BOOL m_bReboot;
};


/////////////////////////////////////////////////////////////////////////////
// CWizardDlg dialog

class CWizardDlg : public CCommonDlg
{
// Construction
public:
	CWizardDlg( CWnd *pParent = NULL );   // standard constructor
	virtual ~CWizardDlg();

// Interface: operations
public:
	void AddPage( CWizardPage *pPage );
	void Commit();
	BOOL Reboot();

// Interface: overrides
public:

// Interface: attributes
public:

// Dialog Data
	//{{AFX_DATA(CWizardDlg)
	enum { IDD = IDD_WIZARD };
	CStatic	m_wndPanel;
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Implementation: operations
private:
	CWizardPage* GetPage( const int nIndex );
	void         SetPage( const int nIndex );
	void         ClosePages();

	void SetWizardState();

// Implementation: overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWizardDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	//}}AFX_VIRTUAL

// Implementation: attributes
private:
	CPtrArray m_arrPages;
	int       m_nCurrentIndex;
	CString   m_strTitle;

// Generated message map functions
protected:
	//{{AFX_MSG(CWizardDlg)
	afx_msg void OnNext();
	afx_msg void OnBack();
	afx_msg void OnFinish();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif //__WIZARDDLG_H__
