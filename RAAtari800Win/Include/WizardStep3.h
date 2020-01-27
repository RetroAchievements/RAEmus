/****************************************************************************
File    : WizardStep3.h
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Interface of the CWizardStep3 class
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 09.03.2001
*/

#ifndef __WIZARDSTEP3_H__
#define __WIZARDSTEP3_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


/////////////////////////////////////////////////////////////////////////////
// CWizardStep3 dialog

class CWizardStep3 : public CWizardPage, public CFileAssociationsBase
{
// Construction
public:
	CWizardStep3( CWnd *pParent = NULL );   // standard constructor

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
	//{{AFX_DATA(CWizardStep3)
	enum { IDD = IDD_WIZARD_STEP3 };
	//}}AFX_DATA

// Implementation: operations
private:
	void SetDlgState();
	BOOL TypeButton ( UINT nButtonID, HWND hwndButton, int nButton = -1 );

// Implementation: overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWizardStep3)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation: attributes
private:

// Generated message map functions
protected:
	//{{AFX_MSG(CWizardStep3)
	afx_msg void OnSelectAll();
	afx_msg void OnRemoveAll();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif //__WIZARDSTEP3_H__
