/****************************************************************************
File    : WizardStep2.h
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Interface of the CWizardStep2 class
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 09.03.2001
*/

#ifndef __WIZARDSTEP2_H__
#define __WIZARDSTEP2_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


/////////////////////////////////////////////////////////////////////////////
// CWizardStep2 dialog

class CWizardStep2 : public CWizardPage, public CRomImagesBase
{
// Construction
public:
	CWizardStep2( CWnd *pParent = NULL );   // standard constructor

// Interface: operations
public:

// Interface: overrides
public:
	// Returns pointer to the window inherited from C<xxx>Base
	virtual CWnd* GetWnd() { return this; };
	virtual void  Commit();

// Interface: attributes
public:

// Dialog Data
	//{{AFX_DATA(CWizardStep2)
	enum { IDD = IDD_WIZARD_STEP2 };
	//}}AFX_DATA

// Implementation: operations
private:
	void SetDlgState();

// Implementation: overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWizardStep2)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation: attributes
private:

// Generated message map functions
protected:
	//{{AFX_MSG(CWizardStep2)
	afx_msg void OnOsa();
	afx_msg void OnOsb();
	afx_msg void OnXlxe();
	afx_msg void On5200();
	afx_msg void OnBasic();
	afx_msg void OnKillfocusEditOsa();
	afx_msg void OnKillfocusEditOsb();
	afx_msg void OnKillfocusEditXlxe();
	afx_msg void OnKillfocusEdit5200();
	afx_msg void OnKillfocusEditBasic();
	afx_msg void OnSearch();
	afx_msg void OnInfo();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif //__WIZARDSTEP2_H__
