/****************************************************************************
File    : SettingsAdvDlg.h
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Interface of the CSettingsAdvDlg class
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 26.04.2002
*/

#ifndef __SETTINGSADVDLG_H__
#define __SETTINGSADVDLG_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


/////////////////////////////////////////////////////////////////////////////
// CSettingsAdvDlg dialog

class CSettingsAdvDlg : public CCommonDlg
{
// Construction
public:
	CSettingsAdvDlg( ULONG *pMiscState, int *pEnableRTime, CWnd *pParent = NULL );   // standard constructor

// Interface: operations
public:

// Interface: attributes
public:
	BOOL m_bReboot;

// Dialog Data
	//{{AFX_DATA(CSettingsAdvDlg)
	enum { IDD = IDD_SETTINGSADV };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Implementation: operations
private:
	void SetDlgState();

// Implementation: overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSettingsAdvDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_VIRTUAL

// Implementation: attributes
private:
	ULONG *m_pMiscState;
	ULONG  m_ulMiscState;
	int   *m_pEnableRTime;
	int    m_nEnableRTime;

// Generated message map functions
protected:
	//{{AFX_MSG(CSettingsAdvDlg)
	afx_msg void OnRebootCart();
	afx_msg void OnRebootVideo();
	afx_msg void OnRebootBasic();
	afx_msg void OnTurnDrivesOff();
	afx_msg void OnNoAttractMode();
	afx_msg void OnRTime();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
};

/////////////////////////////////////////////////////////////////////////////

#endif //__SETTINGSADVDLG_H__
