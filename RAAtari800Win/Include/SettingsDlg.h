/****************************************************************************
File    : SettingsDlg.h
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Interface of the CSettingsDlg class
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 03.08.2002
*/

#ifndef __SETTINGSDLG_H__
#define __SETTINGSDLG_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


/////////////////////////////////////////////////////////////////////////////
// CSettingsDlg dialog

class CSettingsDlg : public CCommonDlg
{
// Construction
public:
	CSettingsDlg( CWnd *pParent = NULL );   // standard constructor

// Interface: operations
public:

// Interface: attributes
public:
	BOOL m_bReboot;

// Dialog Data
	//{{AFX_DATA(CSettingsDlg)
	enum { IDD = IDD_SETTINGS };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Implementation: operations
private:
	void SetDlgState   ();
	void ReceiveFocused();

// Implementation: overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSettingsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_VIRTUAL

// Implementation: attributes
private:
	ULONG m_ulMiscState;
	ULONG m_ulCollisions;
	int   m_nDisableBasic;
	int   m_nEnableSIOPatch;
	int   m_nEnableHPatch;
	int   m_nEnablePPatch;
	int   m_nEnableRPatch;
	int   m_nEnableRTime;
	int   m_nHardReadOnly;
	char  m_szPrintCommand[ PRINT_CMD_LENGTH + 1 ];

// Generated message map functions
protected:
	//{{AFX_MSG(CSettingsDlg)
	afx_msg void OnDisableBasic();
	afx_msg void OnEnableSio();
	afx_msg void OnEnableH();
	afx_msg void OnEnableP();
	afx_msg void OnEnableR();
	afx_msg void OnHardReadOnly();
	afx_msg void OnPrintCheck();
	afx_msg void OnKillfocusPrintCmd();
	afx_msg void OnAdvSetting();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif //__SETTINGSDLG_H__
