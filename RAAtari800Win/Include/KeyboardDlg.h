/****************************************************************************
File    : KeyboardDlg.h
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Interface of the CKeyboardDlg class
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 01.09.2002
*/

#ifndef __KEYBOARDDLG_H__
#define __KEYBOARDDLG_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


/////////////////////////////////////////////////////////////////////////////
// CKeyboardDlg dialog

class CKeyboardDlg : public CCommonDlg
{
// Construction
public:
	CKeyboardDlg( CWnd *pParent = NULL );   // standard constructor

// Interface: operations
public:

// Interface: attributes
public:

// Dialog Data
	//{{AFX_DATA(CKeyboardDlg)
	enum { IDD = IDD_KEYBOARD };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Implementation: operations
private:
	void SetDlgState    ();
	void ReceiveFocused ();
	BOOL PrepareTemplate( LPSTR pszTemplateFile, LPSTR pszTemplateDesc, BOOL bCheckIfExists = TRUE );

// Implementation: overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CKeyboardDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_VIRTUAL

// Implementation: attributes
private:
	ULONG m_ulInputState;
	int   m_nArrowsMode;
	char  m_szTemplateFile[ MAX_PATH + 1 ];
	char  m_szTemplateDesc[ TEMPLATE_DESC_LENGTH + 1 ];
	BOOL  m_bExitPass;

// Generated message map functions
protected:
	//{{AFX_MSG(CKeyboardDlg)
	afx_msg void OnSelchangeArrowKeys();
	afx_msg void OnTypematicRate();
	afx_msg void OnCaptureCtrlEsc();
	afx_msg void OnEditTemplate();
	afx_msg void OnLoadTemplate();
	afx_msg void OnUseTemplate();
	afx_msg void OnKillfocusTemplateFile();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif //__KEYBOARDDLG_H__
