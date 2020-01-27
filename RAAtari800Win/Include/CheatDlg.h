/****************************************************************************
File    : CheatDlg.h
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Interface of the CCheatDlg class
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 15.09.2002
*/

#ifndef __CHEATDLG_H__
#define __CHEATDLG_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


class CCheatServer;

/////////////////////////////////////////////////////////////////////////////
// CCheatDlg dialog

class CCheatDlg : public CCommonDlg
{
// Construction/destruction
public:
	CCheatDlg( CCheatServer *pCheatServer, CWnd *pParent = NULL );   // standard constructor
	~CCheatDlg();

// Interface: operations
public:
	static BOOL ReadTrainerSnapshot( LPCSTR pszFileName, int *pMemo, int *pSearch, int *pLockAt, BOOL *pLock, PDWORD *ppValues, int *pCount, HWND hWnd = NULL );
	static BOOL SaveTrainerSnapshot( LPCSTR pszFileName, int nMemo, int nSearch, int nLock, BOOL bLock, PDWORD pValues, int nCount );
		
// Interface: attributes
public:
	BOOL m_bTrainerChanged;

// Dialog Data
	//{{AFX_DATA(CCheatDlg)
	enum { IDD = IDD_CHEAT };
	CListCtrl	m_listPoke;
	//}}AFX_DATA

// Implementation: operations
private:
	void SetDlgState       ();
	int  CheckedItemsCount ( BOOL bCheckIfAny = FALSE );
	void EnableListUpdates ( BOOL bEnable = TRUE );
	BOOL SetTrainerFileName();
	void ReceiveFocused    ();

// Implementation: overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCheatDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_VIRTUAL

// Implementation: attributes
private:
	ULONG m_ulMiscState;
	ULONG m_ulCollisions;
	int   m_nMemo;
	int   m_nSearch;
	int   m_nLock;
	BOOL  m_bListNotify;
	BOOL  m_bFontCreated;
	CFont m_fontList;

	char  m_szTrainerFile[ MAX_PATH + 1 ];

	CCheatServer *m_pCheatServer;

// Generated message map functions
protected:
	//{{AFX_MSG(CCheatDlg)
	afx_msg void OnDisableCollisions();
	afx_msg void OnPlayerPlayer();
	afx_msg void OnPlayerPlayfield();
	afx_msg void OnMissilePlayer();
	afx_msg void OnMissilePlayfield();
	afx_msg void OnSearchButton();
	afx_msg void OnMemoButton();
	afx_msg void OnDeltaposMemoSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusMemo();
	afx_msg void OnDeltaposSearchSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusSearch();
	afx_msg void OnDeltaposLockSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusLock();
	afx_msg void OnItemchangedPokeList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLockCheck();
	afx_msg void OnLoadButton();
	afx_msg void OnSaveButton();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif //__CHEATDLG_H__
