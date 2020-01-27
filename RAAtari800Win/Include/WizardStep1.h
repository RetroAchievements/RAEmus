/****************************************************************************
File    : WizardStep1.h
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Interface of the CWizardStep1 class
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 03.08.2002
*/

#ifndef __WIZARDSTEP1_H__
#define __WIZARDSTEP1_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


/////////////////////////////////////////////////////////////////////////////
// CWizardStep1 dialog

class CWizardStep1 : public CWizardPage
{
// Type definitions
public:
	struct ShortInfo_t
	{
		LPCSTR pszFile;	/* Pointers to static texts */
		LPCSTR pszShortcut;
		LPCSTR pszDesc;
	};

// Construction
public:
	CWizardStep1( CWnd *pParent = NULL );   // standard constructor

// Interface: operations
public:

// Interface: overrides
public:
	virtual void Commit();

// Interface: attributes
public:

// Dialog Data
	//{{AFX_DATA(CWizardStep1)
	enum { IDD = IDD_WIZARD_STEP1 };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Implementation: operations
private:
	void SetDlgState     ();
	void GetSpecialFolder( int nFolder, LPSTR szSpecialFolder );
	void GetMenuFolder   ( LPSTR szMenuPath );
	void GetDeskFolder   ( LPSTR szDeskPath );

	BOOL CreateShortcut( ShortInfo_t *pShortInfo, LPCSTR pszShortPath );
	BOOL DeleteShortcut( ShortInfo_t *pShortInfo, LPCSTR pszShortPath );

	HRESULT CreateLink( LPCSTR lpszPathObj, LPCSTR lpszPathLink, LPCSTR lpszDesc );

// Implementation: overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWizardStep1)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation: attributes
private:
	char m_szHomeDir[ MAX_PATH + 1 ];
	ShortInfo_t *m_pShortInfo;

	BOOL m_bInitCOM;
	BOOL m_bShortMenu;
	BOOL m_bShortDesk;

// Generated message map functions
protected:
	//{{AFX_MSG(CWizardStep1)
	afx_msg void OnShortMenu();
	afx_msg void OnShortDesk();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif //__WIZARDSTEP1_H__
