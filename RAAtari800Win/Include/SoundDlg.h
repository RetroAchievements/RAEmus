/****************************************************************************
File    : SoundDlg.h
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Interface of the CSoundDlg class
@(#) #BY# Richard Lawrence, Tomasz Szymankowski
@(#) #LM# 01.03.2003
*/

#ifndef __SOUNDDLG_H__
#define __SOUNDDLG_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


/////////////////////////////////////////////////////////////////////////////
// CSoundDlg dialog

class CSoundDlg : public CCommonDlg
{
// Type definitions
public:
	struct SndModeInfo_t
	{
		int nSoundRate;
		int nLowLimit;
	};

// Construction
public:
	CSoundDlg( CWnd *pParent = NULL );   // standard constructor

// Interface: operations
public:

// Interface: attributes
public:
	BOOL m_bModeChanged;

// Dialog Data
	//{{AFX_DATA(CGraphicsDlg)
	enum { IDD = IDD_SOUND };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Implementation: operations
private:
	void SetDlgState    ();
	void SetPokeyDivisor();
	void ReceiveFocused ();

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
	ULONG m_ulSoundState;
	int   m_nSoundRate;
	int   m_nSoundVol;
	int   m_nSkipUpdate;
	int   m_nSoundLatency;
	int   m_nSoundQuality;
	BOOL  m_bDigitized;
	BOOL  m_bEnableStereo;
	int	  m_nLowSkipLimit;
	int   m_nHighSkipLimit;

	SndModeInfo_t *m_pSndModeInfo;

// Generated message map functions
protected:
	//{{AFX_MSG(CSoundDlg)
	afx_msg void OnReleasedCaptureVolumeSlider(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSelchangeDriverType();
	afx_msg void OnKillfocusSoundLatency();
	afx_msg void OnDeltaposSoundLatencySpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSoundStereo();
	afx_msg void OnSoundDigitized();
	afx_msg void OnSelchangeSoundPlayback();
	afx_msg void OnKillfocusPokeyDivisor();
	afx_msg void OnDeltaposPokeyDivisorSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangeSoundResolution();
	afx_msg void OnKillfocusSoundQuality();
	afx_msg void OnDeltaposSoundQualitySpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSoundMute();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif //__SOUNDDLG_H__
