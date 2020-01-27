/****************************************************************************
File    : KeyTemplateDlg.h
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Interface of the CKeyTemplateDlg class
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 03.08.2002
*/

#ifndef __KEYTEMPLATEDLG_H__
#define __KEYTEMPLATEDLG_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


/////////////////////////////////////////////////////////////////////////////
// CKeyTemplateDlg dialog

class CKeyTemplateDlg : public CCommonDlg
{
// Construction
public:
	CKeyTemplateDlg( LPSTR pszTemplateFile, LPSTR pszTemplateDesc, int *pKBTable, CWnd *pParent = NULL );   // standard constructor

// Interface: operations
public:
	static BOOL ReadKeyTemplate( LPCSTR pszFileName, LPSTR  pszFileDesc, int *pKBTable, HWND hWnd = NULL );
	static BOOL SaveKeyTemplate( LPCSTR pszFileName, LPCSTR pszFileDesc, int *pKBTable );

// Interface: attributes
public:

// Dialog Data
	//{{AFX_DATA(CKeyTemplateDlg)
	enum { IDD = IDD_KEYTEMPLATE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Implementation: operations
private:
	void SetDlgState   ();
	void SetAtariSelect();
	void ReceiveFocused();

// Implementation: overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CKeyTemplateDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_VIRTUAL

// Implementation: attributes
private:
	int   m_nPCSelect;
	int   m_nA8Select;
	int  *m_pKBTable;

	char *m_pszTemplateFile;
	char  m_szTemplateFile[ MAX_PATH + 1 ];
	char *m_pszTemplateDesc;
	char  m_szTemplateDesc[ TEMPLATE_DESC_LENGTH + 1 ];

	BOOL  m_bChanged;

// Generated message map functions
protected:
	//{{AFX_MSG(CKeyTemplateDlg)
	afx_msg void OnSelchangePcKeys();
	afx_msg void OnSelchangeAtariKeys();
	afx_msg void OnCtrl();
	afx_msg void OnShift();
	afx_msg void OnSaveTemplate();
	afx_msg void OnKillfocusDescription();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif //__KEYTEMPLATEDLG_H__
