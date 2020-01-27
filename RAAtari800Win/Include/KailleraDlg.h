/****************************************************************************
File    : KailleraDlg.h
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Interface of the CKailleraDlg class
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 25.09.2002
*/

#ifndef __KAILLERADLG_H__
#define __KAILLERADLG_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


/////////////////////////////////////////////////////////////////////////////
// CKailleraDlg dialog

class CKailleraDlg : public CCommonDlg
{
// Construction
public:
	CKailleraDlg( CWnd *pParent = NULL );   // standard constructor

// Interface: operations
public:

// Interface: attributes
public:

// Dialog Data
	//{{AFX_DATA(CKailleraDlg)
	enum { IDD = IDD_KAILLERA };
	//}}AFX_DATA

// Implementation: operations
private:
	void SetDlgState       ();
	BOOL SetNetworkGameName( UINT unBootImage = IAF_ATARI_FILE );
	void ReceiveFocused    ();

// Implementation: overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CKailleraDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_VIRTUAL

// Implementation: attributes
private:
	ULONG m_ulKailleraState;
	char  m_szGameName[ GAME_NAME_LENGTH + 2 ];
	int   m_nLocalPort;
	int   m_nFrameSkip;
	UINT  m_unBootImage;
	BOOL  m_bImageAttached;

// Generated message map functions
protected:
	//{{AFX_MSG(CKailleraDlg)
	afx_msg void OnKillfocusGameName();
	afx_msg void OnSelchangeBootImage();
	afx_msg void OnSelchangePort();
	afx_msg void OnSkipFrameCheck();
	afx_msg void OnKillfocusSkipFrameValue();
	afx_msg void OnDeltaposSkipFrameSpin(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif //__KAILLERADLG_H__
