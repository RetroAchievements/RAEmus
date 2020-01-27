/****************************************************************************
File    : PaletteDlg.h
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Interface of the CGraphicsDlg class
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 03.08.2002
*/

#ifndef __PALETTEDLG_H__
#define __PALETTEDLG_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


/////////////////////////////////////////////////////////////////////////////
// CPaletteDlg dialog

class CPaletteDlg : public CCommonDlg
{
// Construction
public:
	CPaletteDlg( CWnd *pParent = NULL );   // standard constructor

// Interface: operations
public:

// Interface: attributes
public:

// Dialog Data
	//{{AFX_DATA(CPaletteDlg)
	enum { IDD = IDD_PALETTE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Implementation: operations
private:
	void SetDlgState   ();
	BOOL PreparePalette( LPSTR pszPaletteFile );
	BOOL PaintPalette  ( CDC *pDC );
	void RestorePalette();
	void ReceiveFocused();

// Implementation: overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPaletteDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_VIRTUAL

// Implementation: attributes
private:
	BOOL  m_bSmallMode;
	ULONG m_ulMiscState;
	char  m_szPaletteFile[ MAX_PATH + 1 ];
	CRect m_rcPalBar;
	int   m_nBlackLevel;
	int   m_nWhiteLevel;
	int   m_nSaturation;
	int   m_nContrast;
	int	  m_nBrightness;
	int   m_nGamma;
	BOOL  m_bExitPass;

// Generated message map functions
protected:
	//{{AFX_MSG(CPaletteDlg)
	afx_msg void OnPaint();
	afx_msg void OnDeltaposBlackSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusBlackLevel();
	afx_msg void OnDeltaposWhiteSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusWhiteLevel();
	afx_msg void OnDeltaposSaturationSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusSaturation();
	afx_msg void OnDeltaposContrastSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusContrast();
	afx_msg void OnDeltaposBrightnessSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusBrightness();
	afx_msg void OnDeltaposGammaSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKillfocusGamma();
	afx_msg void OnBrowse();
	afx_msg void OnKillfocusEdit();
	afx_msg void OnUseExternal();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif //__PALETTEDLG_H__
