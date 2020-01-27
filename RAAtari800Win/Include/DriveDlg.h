/****************************************************************************
File    : DriveDlg.h
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Interface of the CDriveDlg class
@(#) #BY# Richard Lawrence, Tomasz Szymankowski
@(#) #LM# 03.08.2002
*/

#ifndef __DRIVEDLG_H__
#define __DRIVEDLG_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


/////////////////////////////////////////////////////////////////////////////
// CDriveDlg dialog

class CDriveDlg : public CCommonDlg
{
// Type definitions
public:
	struct DiskData_t
	{
		LPSTR      pszName;
		char       szNewName[ MAX_PATH + 1 ];
		UINT       nButtonID;
		UINT       nEditID;
		UINT       nComboID;
		SIO_UnitStatus usStatus;
	};

// Construction
public:
	CDriveDlg( CWnd *pParent = NULL );   // standard constructor

// Interface: operations
public:

// Interface: attributes
public:

// Dialog Data
	//{{AFX_DATA(CDriveDlg)
	enum { IDD = IDD_DRIVE };
	//}}AFX_DATA

// Implementation: operations
private:
	void       SetDlgState       ();
	void       ReceiveFocused    ();
	BOOL       GetDiskImage      ( int nDrive );
	BOOL       DriveButton       ( UINT nButtonID, HWND hwndButton );
	BOOL       KillfocusEditDrive( UINT nEditID,   HWND hwndEdit );
	BOOL       StatusSelChange   ( UINT nComboID,  HWND hwndCombo );
	SIO_UnitStatus CheckSelectedDisk ( LPCSTR pszDiskName, UINT *pInfo = NULL );

// Implementation: overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDriveDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_VIRTUAL

// Implementation: attributes
private:
	BOOL        m_bSmallMode;
	DiskData_t *m_pDiskData;

// Generated message map functions
protected:
	//{{AFX_MSG(CDriveDlg)
	afx_msg void OnClearAll();
	afx_msg void OnNewImage();
	afx_msg void OnSelchangeNumber();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedDriveReadonly();
};

/////////////////////////////////////////////////////////////////////////////

#endif //__DRIVEDLG_H__
