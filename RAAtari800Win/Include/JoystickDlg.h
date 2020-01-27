/****************************************************************************
File    : JoystickDlg.h
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Interface of the CJoystickDlg class
@(#) #BY# Richard Lawrence, Tomasz Szymankowski
@(#) #LM# 28.08.2002
*/

#ifndef __JOYSTICKDLG_H__
#define __JOYSTICKDLG_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


/////////////////////////////////////////////////////////////////////////////
// CJoystickDlg dialog

class CJoystickDlg : public CCommonDlg
{
// Construction
public:
	CJoystickDlg( CWnd *pParent = NULL );   // standard constructor

// Interface: operations
public:

// Interface: attributes
public:

// Dialog Data
	//{{AFX_DATA(CJoystickDlg)
	enum { IDD = IDD_JOYSTICK };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Implementation: operations
private:
	void SetDlgState   ();
	void SelchangeStick( int nStickNum );
	void Autofire      ( int nStickNum );

// Implementation: overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CJoystickDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_VIRTUAL

// Implementation: attributes
private:
	BOOL  m_bSmallMode;
	int   m_anDevSelected[ MAX_ATARI_JOYPORTS ];
	ULONG m_ulInputState;
	ULONG m_ulAutoSticks;
	int   m_nAutoMode;
	int   m_nBlockOpposite;
	int   m_nMultiJoy;

// Generated message map functions
protected:
	//{{AFX_MSG(CJoystickDlg)
	afx_msg void OnSelchangePort();
	afx_msg void OnSelchangeStick();
	afx_msg void OnSelchangeStick1();
	afx_msg void OnSelchangeStick2();
	afx_msg void OnSelchangeStick3();
	afx_msg void OnSelchangeStick4();
	afx_msg void OnKeysetA();
	afx_msg void OnKeysetB();
	afx_msg void OnAutofire();
	afx_msg void OnAutofire1();
	afx_msg void OnAutofire2();
	afx_msg void OnAutofire3();
	afx_msg void OnAutofire4();
	afx_msg void OnSelchangeAutofireMode();
	afx_msg void OnAdvSetting();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif //__JOYSTICKDLG_H__
