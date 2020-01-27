/****************************************************************************
File    : RomImagesDlg.h
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Interface of the CRomImagesDlg class
@(#) #BY# Richard Lawrence, Tomasz Szymankowski
@(#) #LM# 03.08.2002
*/

#ifndef __ROMIMAGESDLG_H__
#define __ROMIMAGESDLG_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


/////////////////////////////////////////////////////////////////////////////
// CRomImagesDlg dialog

class CRomImagesDlg : public CCommonDlg, public CRomImagesBase
{
// Type definitions
public:
	enum RebootType { RBT_NONE = 0, RBT_COLDSTART, RBT_RESTART };

// Construction
public:
	CRomImagesDlg( CWnd *pParent = NULL );   // standard constructor

// Interface: operations
public:
	// Returns pointer to the window inherited from C<xxx>Base
	virtual CWnd* GetWnd() { return this; };

// Interface: attributes
public:
	RebootType m_eReboot;

// Dialog Data
	//{{AFX_DATA(CRomImagesDlg)
	enum { IDD = IDD_ROMIMAGES };
	//}}AFX_DATA

// Implementation: operations
private:
	void SetDlgState   ();
	void ReceiveFocused();

// Implementation: overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRomImagesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_VIRTUAL

// Implementation: attributes
private:
	BOOL m_bSmallMode;
	int  m_nCartState;
	BOOL m_bExitPass;
	char m_szCartName[ MAX_PATH + 1 ];

// Generated message map functions
protected:
	//{{AFX_MSG(CRomImagesDlg)
	afx_msg void OnOsa();
	afx_msg void OnOsb();
	afx_msg void OnXlxe();
	afx_msg void On5200();
	afx_msg void OnBasic();
	afx_msg void OnCartridge();
	afx_msg void OnOsRom();
	afx_msg void OnKillfocusEditOsa();
	afx_msg void OnKillfocusEditOsb();
	afx_msg void OnKillfocusEditXlxe();
	afx_msg void OnKillfocusEdit5200();
	afx_msg void OnKillfocusEditBasic();
	afx_msg void OnKillfocusEditOsRom();
	afx_msg void OnSelchangeMachineType();
	afx_msg void OnSearch();
	afx_msg void OnInfo();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif //__ROMIMAGESDLG_H__
