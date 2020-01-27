/****************************************************************************
File    : HarddiskDlg.h
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Interface of the CHarddiskDlg class
@(#) #BY# Richard Lawrence, Tomasz Szymankowski
@(#) #LM# 14.07.2003
*/

#ifndef __HARDDISKDLG_H__
#define __HARDDISKDLG_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


/////////////////////////////////////////////////////////////////////////////
// CHarddiskDlg dialog

class CHarddiskDlg : public CCommonDlg
{
// Type definitions
public:
	struct DriveData_t
	{
		LPSTR pszDir;
		char  szNewDir[ MAX_PATH + 1 ];
		UINT  nCtrlID;
		LPSTR pszRegName;
	};

// Construction
public:
	CHarddiskDlg( CWnd *pParent = NULL );   // standard constructor

// Interface: operations
public:

// Interface: attributes
public:

// Dialog Data
	//{{AFX_DATA(CHarddiskDlg)
	enum { IDD = IDD_HARDDISK };
	//}}AFX_DATA

// Implementation: operations
private:
	void SetDlgState    ();
	void ReceiveFocused ();
	void SetToDrive     ( int nDriveIndex );
	void KillfocusHdEdit( int nDriveIndex );

// Implementation: overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHarddiskDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_VIRTUAL

// Implementation: attributes
private:
	DriveData_t *m_pDriveData;

	char m_szHardExePath[ FILENAME_MAX ];

// Generated message map functions
protected:
	// Generated message map functions
	//{{AFX_MSG(CHarddiskDlg)
	afx_msg void OnHd1();
	afx_msg void OnHd2();
	afx_msg void OnHd3();
	afx_msg void OnHd4();
	afx_msg void OnKillfocusHd2Edit();
	afx_msg void OnKillfocusHd1Edit();
	afx_msg void OnKillfocusHd3Edit();
	afx_msg void OnKillfocusHd4Edit();
	afx_msg void OnKillfocusExePath();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif //__HARDDISKDLG_H__
