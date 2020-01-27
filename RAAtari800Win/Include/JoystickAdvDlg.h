/****************************************************************************
File    : JoystickAdvDlg.h
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Interface of the CJoystickAdvDlg class
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 28.08.2002
*/

#ifndef __JOYSTICKADVDLG_H__
#define __JOYSTICKADVDLG_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


/////////////////////////////////////////////////////////////////////////////
// CJoystickAdvDlg dialog

class CJoystickAdvDlg : public CCommonDlg
{
// Construction
public:
	CJoystickAdvDlg( ULONG *pInputState, int *pBlockOpposite, int *pMultiJoy, CWnd *pParent = NULL );   // standard constructor

// Interface: operations
public:

// Interface: attributes
public:

// Dialog Data
	//{{AFX_DATA(CJoystickAdvDlg)
	enum { IDD = IDD_JOYSTICKADV };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Implementation: operations
private:
	void SetDlgState();

// Implementation: overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CJoystickAdvDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_VIRTUAL

// Implementation: attributes
private:
	ULONG *m_pInputState;
	ULONG  m_ulInputState;
	int   *m_pBlockOpposite;
	int    m_nBlockOpposite;
	int   *m_pMultiJoy;
	int    m_nMultiJoy;

// Generated message map functions
protected:
	//{{AFX_MSG(CJoystickAdvDlg)
	afx_msg void OnButtonsFire();
	afx_msg void OnRecenterStick();
	afx_msg void OnBlockOpposite();
	afx_msg void OnMultiJoy();
	afx_msg void OnExitPause();
	afx_msg void OnDontExclude();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif //__JOYSTICKADVDLG_H__
