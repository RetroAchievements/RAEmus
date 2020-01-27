/****************************************************************************
File    : TapeDlg.h
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Interface of the CTapeDlg class
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 11.01.2004
*/

#ifndef __TAPEDLG_H__
#define __TAPEDLG_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


/////////////////////////////////////////////////////////////////////////////
// CTapeDlg dialog

class CTapeDlg : public CCommonDlg
{
// Construction
public:
	CTapeDlg( CWnd *pParent = NULL );   // standard constructor

// Interface: operations
public:

// Interface: attributes
public:

// Dialog Data
	//{{AFX_DATA(CTapeDlg)
	enum { IDD = IDD_TAPE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Implementation: operations
private:
	void SetDlgState   ();
	BOOL PrepareTape   ( LPSTR pszTapeFile, BOOL bCheckIfExists = TRUE, BOOL bForceInsert = FALSE );
	void EjectTape     ();
	void ReceiveFocused();
	void SetPosInfo    ( UINT nPos );

// Implementation: overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTapeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_VIRTUAL

// Implementation: attributes
private:
	char m_szTapeFile[ FILENAME_MAX ];
	char m_szTapeLast[ FILENAME_MAX ];
	char m_szTapeBack[ FILENAME_MAX ];
	int  m_nCurrentBack;
	BOOL m_bCasImage;
	BOOL m_bExitPass;

	char m_szLabels[ 5 ][ LOADSTRING_SIZE_S + 1 ];

// Generated message map functions
protected:
	//{{AFX_MSG(CTapeDlg)
	afx_msg void OnBrowse();
	afx_msg void OnEject();
	afx_msg void OnRewind();
	afx_msg void OnNewImage();
	afx_msg void OnKillfocusFile();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif //__TAPEDLG_H__
