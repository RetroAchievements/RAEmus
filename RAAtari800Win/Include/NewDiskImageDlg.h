/****************************************************************************
File    : NewDiskImageDlg.h
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Interface of the CNewDiskImageDlg class
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 28.12.2003
*/

#ifndef __NEWDISKIMAGEDLG_H__
#define __NEWDISKIMAGEDLG_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


/////////////////////////////////////////////////////////////////////////////
// CNewDiskImageDlg dialog

class CNewDiskImageDlg : public CCommonDlg
{
// Construction
public:
	CNewDiskImageDlg( CWnd *pParent = NULL );   // standard constructor

// Interface: operations
public:

// Interface: attributes
public:
	char m_szDiskName[ MAX_PATH + 1 ];
	int  m_bAttachDisk;
	int  m_nDriveNumber;

// Dialog Data
	//{{AFX_DATA(CNewDiskImageDlg)
	enum { IDD = IDD_NEWDISKIMAGE };
	//}}AFX_DATA

// Implementation: operations
private:
	void SetDlgState   ();
	void ReceiveFocused();

// Implementation: overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNewDiskImageDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_VIRTUAL

// Implementation: attributes
private:
	int  m_nFormat;
	UINT m_unSectors;

// Generated message map functions
protected:
	//{{AFX_MSG(CNewDiskImageDlg)
	afx_msg void OnDeltaposSectorSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusSectors();
	afx_msg void OnNewDiskImage90k();
	afx_msg void OnNewDiskImage130k();
	afx_msg void OnNewDiskImage180k();
	afx_msg void OnNewDiskImageOther();
	afx_msg void OnAttachDiskCheck();
	afx_msg void OnSelchangeAttachDiskCombo();
	afx_msg void OnSelchangeBytesPerSector();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif //__NEWDISKIMAGEDLG_H__
