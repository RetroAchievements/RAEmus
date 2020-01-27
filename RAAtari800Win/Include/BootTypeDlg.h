/****************************************************************************
File    : BootTypeDlg.h
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Interface of the CBootTypeDlg class
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 25.03.2002
*/

#ifndef __BOOTTYPE_H__
#define __BOOTTYPE_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


/////////////////////////////////////////////////////////////////////////////
// CBootTypeDlg dialog

class CBootTypeDlg : public CCommonDlg
{
// Type definitions
public:
	enum BootType{ Logical = XFD2ATR_BOOT_LOGICAL, Physical = XFD2ATR_BOOT_PHYSICAL, Sio2Pc = XFD2ATR_BOOT_SIO2PC };

// Construction
public:
	CBootTypeDlg( CWnd *pParent = NULL );   // standard constructor

// Interface: operations
public:
	BootType GetBootType() { return m_btSelOpt; }

// Interface: attributes
public:

// Dialog Data
	//{{AFX_DATA(CBootTypeDlg)
	enum { IDD = IDD_BOOTTYPE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Implementation: operations
private:
	void SetDlgState();

// Implementation: overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBootTypeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_VIRTUAL

// Implementation: attributes
private:
	BootType m_btSelOpt;

// Generated message map functions
protected:
	//{{AFX_MSG(CBootTypeDlg)
	afx_msg void OnLogical();
	afx_msg void OnPhysical();
	afx_msg void OnSio2Pc();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif //__BOOTTYPE_H__
