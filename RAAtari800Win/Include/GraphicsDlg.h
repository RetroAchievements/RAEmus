/****************************************************************************
File    : GraphicsDlg.h
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Interface of the CGraphicsDlg class
@(#) #BY# Richard Lawrence, Tomasz Szymankowski
@(#) #LM# 10.10.2003
*/

#ifndef __GRAPHICSDLG_H__
#define __GRAPHICSDLG_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


/////////////////////////////////////////////////////////////////////////////
// CGraphicsDlg dialog

class CGraphicsDlg : public CCommonDlg
{
// Type definitions
public:
	struct GfxModeInfo_t
	{
		DWORD  dwCtrlId;
		DWORD  dwMode;
		DWORD  dwRes;
		DWORD  dwIconId;
		LPCSTR pszDesc;		/* The pointer to a static text */
		LPCSTR pszItem;		/* The pointer to a static text */
		BOOL   bFullScr;
	};

// Construction
public:
	CGraphicsDlg( CWnd *pParent = NULL );   // standard constructor

// Interface: operations
public:

// Interface: attributes
public:
	BOOL m_bModeChanged;

// Dialog Data
	//{{AFX_DATA(CGraphicsDlg)
	enum { IDD = IDD_GRAPHICS };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Implementation: operations
private:
	static ULONG GetFullScreenMode( ULONG ulScreenState );

	void SetDlgState( BOOL bSetFocus = FALSE );
	BOOL SetBtnState( ULONG ulButtonID, ULONG ulModeFlag, ULONG ulResFlag, BOOL bFullScreen, BOOL bSetFocus = FALSE );

// Implementation: overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGraphicsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_VIRTUAL

// Implementation: attributes
private:
	BOOL  m_bSmallMode;
	ULONG m_ulScreenState;
	int   m_nStretchMode;
	int   m_nMemoryType;
	int   m_nRequestRate;

	static GfxModeInfo_t *m_pGfxModeInfo;

// Generated message map functions
protected:
	//{{AFX_MSG(CGraphicsDlg)
	afx_msg void OnMode320x200Clip();
	afx_msg void OnMode320x240PartClip();
	afx_msg void OnMode320x400Clip();
	afx_msg void OnMode400x300();
	afx_msg void OnMode512x384();
	afx_msg void OnMode640x400Clip();
	afx_msg void OnMode640x480PartClip();
	afx_msg void OnMode800x600();
	afx_msg void OnMode1024x768();
	afx_msg void On336x240Windowed();
	afx_msg void On672x480Windowed();
	afx_msg void OnSelchangeCombo();
	afx_msg void OnSelchangeStretchMode();
	afx_msg void OnUseGDI();
	afx_msg void OnHideCursor();
	afx_msg void OnUseSysPalette();
	afx_msg void OnAdvSetting();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif //__GRAPHICSDLG_H__
