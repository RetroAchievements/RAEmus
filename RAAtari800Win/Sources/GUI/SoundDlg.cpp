/****************************************************************************
File    : SoundDlg.cpp
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# CSoundDlg implementation file
@(#) #BY# Richard Lawrence, Tomasz Szymankowski
@(#) #LM# 30.03.2003
*/

#include "StdAfx.h"
#include "Atari800Win.h"
#include "Helpers.h"
#include "SoundDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define IDC_SOUND_FIRST		IDC_SOUND_VOLUMESLIDER
#define IDC_SOUND_LAST		IDC_SOUND_CANCEL

#define MIN_SOUND_LATENCY	2
#define MAX_SOUND_LATENCY	6
#define MIN_SOUND_QUALITY	1
#define MAX_SOUND_QUALITY	4

#define DT_WAVEOUT			0
#define DT_DIRECTX			1

#define SR_8BIT				0
#define SR_16BIT			1


/////////////////////////////////////////////////////////////////////////////
// Static objects

static CSoundDlg::SndModeInfo_t s_aSndModeInfo[] =
{
	{ 8000,  6 },
	{ 11025, 4 },
	{ 21280, 2 },
	{ 22050, 2 },
	{ 31920, 2 },
	{ 44100, 1 },
	{ 48000, 1 }
};

static const int s_nSndModeInfoNo = sizeof(s_aSndModeInfo)/sizeof(s_aSndModeInfo[0]);


/////////////////////////////////////////////////////////////////////////////
// CSoundDlg dialog

BEGIN_MESSAGE_MAP(CSoundDlg, CCommonDlg)
	//{{AFX_MSG_MAP(CSoundDlg)
	ON_WM_HSCROLL()
	ON_CBN_SELCHANGE(IDC_SOUND_DRIVERTYPE, OnSelchangeDriverType)
	ON_EN_KILLFOCUS(IDC_SOUND_LATENCY, OnKillfocusSoundLatency)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SOUND_LATENCYSPIN, OnDeltaposSoundLatencySpin)
	ON_BN_CLICKED(IDC_SOUND_STEREO, OnSoundStereo)
	ON_BN_CLICKED(IDC_SOUND_VOLUMEONLY, OnSoundDigitized)
	ON_CBN_SELCHANGE(IDC_SOUND_PLAYBACK, OnSelchangeSoundPlayback)
	ON_EN_KILLFOCUS(IDC_SOUND_DIVISOR, OnKillfocusPokeyDivisor)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SOUND_DIVISORSPIN, OnDeltaposPokeyDivisorSpin)
	ON_CBN_SELCHANGE(IDC_SOUND_RESOLUTION, OnSelchangeSoundResolution)
	ON_EN_KILLFOCUS(IDC_SOUND_QUALITY, OnKillfocusSoundQuality)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SOUND_QUALITYSPIN, OnDeltaposSoundQualitySpin)
	ON_BN_CLICKED(IDC_SOUND_MUTE, OnSoundMute)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_SOUND_OK, OnOK)
	ON_BN_CLICKED(IDC_SOUND_CANCEL, CCommonDlg::OnCancel)
END_MESSAGE_MAP()

/*========================================================
Method   : CSoundDlg::CSoundDlg
=========================================================*/
/* #FN#
   Standard constructor */
CSoundDlg::
CSoundDlg(
	CWnd *pParent /*=NULL*/ /* #IN# Pointer to the parent window */
)
	: CCommonDlg( CSoundDlg::IDD, pParent )
{
	//{{AFX_DATA_INIT(CSoundDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_pSndModeInfo = s_aSndModeInfo;
	m_bModeChanged = FALSE;
	m_nFirstCtrl   = IDC_SOUND_FIRST;
	m_nLastCtrl    = IDC_SOUND_LAST;

} /* #OF# CSoundDlg::CSoundDlg */

/*========================================================
Method   : CSoundDlg::DoDataExchange
=========================================================*/
/* #FN#
   Dynamic Data Exchange (not used) */
void
/* #AS#
   Nothing */
CSoundDlg::
DoDataExchange(
	CDataExchange *pDX /* #IN# Pointer to CDataExchange object */
)
{
	CCommonDlg::DoDataExchange( pDX );
	//{{AFX_DATA_MAP(CSoundDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP

} /* #OF# CSoundDlg::DoDataExchange */


/////////////////////////////////////////////////////////////////////////////
// CSoundDlg implementation

/*========================================================
Method   : CSoundDlg::SetDlgState
=========================================================*/
/* #FN#
   Sets up the state of the dialog controls */
void
/* #AS#
   Nothing */
CSoundDlg::
SetDlgState()
{
	CSliderCtrl *pSlider = (CSliderCtrl *)GetDlgItem( IDC_SOUND_VOLUMESLIDER );
	ASSERT(NULL != pSlider);

	/* Set up Volume slider */
	pSlider->EnableWindow( Sound_VolumeCapable() );
	pSlider->SetRange( 0, 100, FALSE );
	pSlider->SetTicFreq( 10 );
	pSlider->SetPageSize( 10 );
	pSlider->SetPos( 100 + m_nSoundVol );

	/* Enable/Disable appriopriate controls */
	_EnableCtrl( IDC_SOUND_LATENCY,       _IsFlagSet( m_ulSoundState, SS_DS_SOUND ) );
	_EnableCtrl( IDC_SOUND_LATENCY_LABEL, _IsFlagSet( m_ulSoundState, SS_DS_SOUND ) );
	_EnableCtrl( IDC_SOUND_LATENCYSPIN,   _IsFlagSet( m_ulSoundState, SS_DS_SOUND ) );

	_SetChkBttn( IDC_SOUND_MUTE,          _IsFlagSet( m_ulSoundState, SS_NO_SOUND ) );
	_SetChkBttn( IDC_SOUND_STEREO,        m_bEnableStereo );
	_SetChkBttn( IDC_SOUND_VOLUMEONLY,    m_bDigitized    );
	_EnableCtrl( IDC_SOUND_VOLUMEONLY,    1 == m_nSoundQuality );

	/* Set up Driver Type and Sound Resolution combos */
	_SetSelCbox( IDC_SOUND_DRIVERTYPE,    _IsFlagSet( m_ulSoundState, SS_MM_SOUND    ) ? DT_WAVEOUT : DT_DIRECTX );
	_SetSelCbox( IDC_SOUND_RESOLUTION,    _IsFlagSet( m_ulSoundState, SS_16BIT_AUDIO ) ? SR_16BIT   : SR_8BIT    );

	/* Set up Playback Rate combo */
	switch( m_nSoundRate )
	{
		case 8000:
			_SetSelCbox( IDC_SOUND_PLAYBACK, 0 );
			m_nLowSkipLimit = 6;
			break;

		case 11025:
			_SetSelCbox( IDC_SOUND_PLAYBACK, 1 );
			m_nLowSkipLimit = 4;
			break;

		case 21280:
			_SetSelCbox( IDC_SOUND_PLAYBACK, 2 );
			m_nLowSkipLimit = 2;
			break;

		case 22050:
			_SetSelCbox( IDC_SOUND_PLAYBACK, 3 );
			m_nLowSkipLimit = 2;
			break;

		case 31920:
			_SetSelCbox( IDC_SOUND_PLAYBACK, 4 );
			m_nLowSkipLimit = 2;
			break;

		case 44100:
			_SetSelCbox( IDC_SOUND_PLAYBACK, 5 );
			m_nLowSkipLimit = 1;
			break;

		case 48000:
			_SetSelCbox( IDC_SOUND_PLAYBACK, 6 );
			m_nLowSkipLimit = 1;
			break;

		default:
			_SetSelCbox( IDC_SOUND_PLAYBACK, 5 );
			m_nSoundRate = 44100;
			m_nLowSkipLimit = 1;
	}
	SetPokeyDivisor();

	SetDlgItemInt( IDC_SOUND_LATENCY, m_nSoundLatency, FALSE );
	SetDlgItemInt( IDC_SOUND_QUALITY, m_nSoundQuality, FALSE );

} /* #OF# CSoundDlg::SetDlgState */

/*========================================================
Method   : CSoundDlg::SetPokeyDivisor
=========================================================*/
/* #FN#
   Sets the proper Pokey update divisor value */
void
/* #AS#
   Nothing */
CSoundDlg::
SetPokeyDivisor()
{
	if( m_nSkipUpdate < m_nLowSkipLimit )
		m_nSkipUpdate = m_nLowSkipLimit;
	if( m_nSkipUpdate > m_nHighSkipLimit )
		m_nSkipUpdate = m_nHighSkipLimit;

	SetDlgItemInt( IDC_SOUND_DIVISOR, m_nSkipUpdate, FALSE );

} /* #OF# CSoundDlg::SetPokeyDivisor */


/////////////////////////////////////////////////////////////////////////////
// CSoundDlg message handlers

/*========================================================
Method   : CSoundDlg::OnInitDialog
=========================================================*/
/* #FN#
   Performs special processing when the dialog box is initialized */
BOOL
/* #AS#
   TRUE unless you set the focus to a control */
CSoundDlg::
OnInitDialog()
{
	CCommonDlg::OnInitDialog();

	m_nLowSkipLimit  = 1;
	m_nHighSkipLimit = Atari800_tv_mode;
	m_ulSoundState   = g_Sound.ulState;
	m_nSoundRate     = g_Sound.nRate;
	m_nSoundVol      = g_Sound.nVolume;
	m_nSkipUpdate    = g_Sound.nSkipUpdate;
	m_nSoundLatency  = g_Sound.nLatency;
	m_nSoundQuality  = g_Sound.nQuality;
	m_bDigitized     = g_Sound.nDigitized;
	m_bEnableStereo  = POKEYSND_stereo_enabled;

	SetDlgState();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
} /* #OF# CSoundDlg::OnInitDialog */

/*========================================================
Method   : CSoundDlg::OnHScroll
=========================================================*/
/* #FN#
   The framework calls this member function when the user
   clicks a window’s horizontal scroll bar */
void
/* #AS#
   Nothing */
CSoundDlg::
OnHScroll(
	UINT        nSBCode,   /* #IN# A scroll-bar code that indicates the user’s scrolling request */
	UINT        nPos,      /* #IN# Specifies the scroll-box position */
	CScrollBar *pScrollBar /* #IN# A pointer to the scroll-bar control control */
)
{
	if( TB_THUMBTRACK == nSBCode || SB_THUMBPOSITION == nSBCode )
		m_nSoundVol = nPos - 100;
	else
	{
//		CSliderCtrl *pSlider = (CSliderCtrl *)GetDlgItem( IDC_SOUND_VOLUMESLIDER );
//		ASSERT(NULL != pSlider);
		if( NULL != pScrollBar )
			m_nSoundVol = ((CSliderCtrl *)pScrollBar)->GetPos() - 100;
	}
	CCommonDlg::OnHScroll( nSBCode, nPos, pScrollBar );

} /* #OF# CSoundDlg::OnHScroll */

/*========================================================
Method   : CSoundDlg::OnSelchangeSoundPlayback
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate combo box */
void
/* #AS#
   Nothing */
CSoundDlg::
OnSelchangeSoundPlayback()
{
	m_nLowSkipLimit = m_pSndModeInfo[ _GetSelCbox( IDC_SOUND_PLAYBACK ) ].nLowLimit;
	m_nSoundRate    = m_pSndModeInfo[ _GetSelCbox( IDC_SOUND_PLAYBACK ) ].nSoundRate;
	SetPokeyDivisor();

	_ModifyFlag( 22050 != m_nSoundRate, m_ulSoundState, SS_CUSTOM_RATE );

} /* #OF# CSoundDlg::OnSelchangeSoundPlayback */

/*========================================================
Method   : CSoundDlg::OnSelchangeSoundResolution
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate combo box */
void
/* #AS#
   Nothing */
CSoundDlg::
OnSelchangeSoundResolution()
{
	int nSelection = _GetSelCbox( IDC_SOUND_RESOLUTION );

	if( CB_ERR == nSelection )
		return;

	_ModifyFlag( SR_16BIT == nSelection, m_ulSoundState, SS_16BIT_AUDIO );

} /* #OF# CSoundDlg::OnSelchangeSoundResolution */

/*========================================================
Method   : CSoundDlg::OnSoundMute
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate check box */
void
/* #AS#
   Nothing */
CSoundDlg::
OnSoundMute()
{
	_ClickButton( IDC_SOUND_MUTE, m_ulSoundState, SS_NO_SOUND );

} /* #OF# CSoundDlg::OnSoundMute */

/*========================================================
Method   : CSoundDlg::OnSelchangeDriverType
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate combo box */
void
/* #AS#
   Nothing */
CSoundDlg::
OnSelchangeDriverType()
{
	int nSelection = _GetSelCbox( IDC_SOUND_DRIVERTYPE );

	if( CB_ERR == nSelection )
		return;

	if( DT_WAVEOUT == nSelection )
	{
		_ClrFlag( m_ulSoundState, SS_DS_SOUND );
		_SetFlag( m_ulSoundState, SS_MM_SOUND );
	}
	else
	{
		ASSERT(DT_DIRECTX == nSelection);

		_ClrFlag( m_ulSoundState, SS_MM_SOUND );
		_SetFlag( m_ulSoundState, SS_DS_SOUND );
	}
	SetDlgState();

} /* #OF# CSoundDlg::OnSelchangeDriverType */

/*========================================================
Method   : CSoundDlg::OnKillfocusPokeyDivisor
=========================================================*/
/* #FN#
	Sets a state of the object regarding to an appropriate edit control */
void
/* #AS#
   Nothing */
CSoundDlg::
OnKillfocusPokeyDivisor()
{
	BOOL bResult;
	m_nSkipUpdate = GetDlgItemInt( IDC_SOUND_DIVISOR, &bResult, FALSE );

	SetPokeyDivisor();

} /* #OF# CSoundDlg::OnKillfocusPokeyDivisor */

/*========================================================
Method   : CSoundDlg::OnKillfocusSoundLatency
=========================================================*/
/* #FN#
	Sets a state of the object regarding to an appropriate edit control */
void
/* #AS#
   Nothing */
CSoundDlg::
OnKillfocusSoundLatency()
{
	_KillfocusSpin( IDC_SOUND_LATENCY, m_nSoundLatency, MIN_SOUND_LATENCY, MAX_SOUND_LATENCY );

} /* #OF# CSoundDlg::OnKillfocusSoundLatency */

/*========================================================
Method   : CSoundDlg::OnKillfocusSoundQuality
=========================================================*/
/* #FN#
	Sets a state of the object regarding to an appropriate edit control */
void
/* #AS#
   Nothing */
CSoundDlg::
OnKillfocusSoundQuality()
{
	_KillfocusSpin( IDC_SOUND_QUALITY, m_nSoundQuality, MIN_SOUND_QUALITY, MAX_SOUND_QUALITY );
	SetDlgState();

} /* #OF# CSoundDlg::OnKillfocusSoundQuality */

  /*========================================================
Method   : CSoundDlg::OnDeltaposPokeyDivisorSpin
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate spin control */
void
/* #AS#
   Nothing */
CSoundDlg::
OnDeltaposPokeyDivisorSpin(
	NMHDR   *pNMHDR,
	LRESULT *pResult
)
{
	NM_UPDOWN *pNMUpDown = (NM_UPDOWN *)pNMHDR;
	m_nSkipUpdate -= pNMUpDown->iDelta;

	SetPokeyDivisor();

	*pResult = m_nSkipUpdate;

} /* #OF# CSoundDlg::OnDeltaposPokeyDivisorSpin */

/*========================================================
Method   : CSoundDlg::OnDeltaposSoundLatencySpin
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate spin control */
void
/* #AS#
   Nothing */
CSoundDlg::
OnDeltaposSoundLatencySpin(
	NMHDR   *pNMHDR, /* #IN#  */
	LRESULT *pResult /* #OUT# */
)
{
	_DeltaposSpin( pNMHDR, IDC_SOUND_LATENCY, m_nSoundLatency, MIN_SOUND_LATENCY, MAX_SOUND_LATENCY );
	*pResult = 0;

} /* #OF# CSoundDlg::OnDeltaposSoundLatencySpin */

/*========================================================
Method   : CSoundDlg::OnDeltaposSoundQualitySpin
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate spin control */
void
/* #AS#
   Nothing */
CSoundDlg::
OnDeltaposSoundQualitySpin(
	NMHDR   *pNMHDR, /* #IN#  */
	LRESULT *pResult /* #OUT# */
)
{
	_DeltaposSpin( pNMHDR, IDC_SOUND_QUALITY, m_nSoundQuality, MIN_SOUND_QUALITY, MAX_SOUND_QUALITY );
	SetDlgState();

	*pResult = 0;

} /* #OF# CSoundDlg::OnDeltaposSoundQualitySpin */

/*========================================================
Method   : CSoundDlg::OnSoundDigitized
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate check box */
void
/* #AS#
   Nothing */
CSoundDlg::
OnSoundDigitized()
{
	m_bDigitized = _GetChkBttn( IDC_SOUND_VOLUMEONLY );

} /* #OF# CSoundDlg::OnSoundDigitized */

/*========================================================
Method   : CSoundDlg::OnSoundStereo
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate check box */
void
/* #AS#
   Nothing */
CSoundDlg::
OnSoundStereo()
{
	m_bEnableStereo = _GetChkBttn( IDC_SOUND_STEREO );

} /* #OF# CSoundDlg::OnSoundStereo */

/*========================================================
Method   : CSoundDlg::ReceiveFocused
=========================================================*/
/* #FN#
   Receives the edit controls content again. The user could press
   'Enter' or 'Alt-O' and then all changes he's made in the last
   edited control would be lost. */
void
/* #AS#
   Nothing */
CSoundDlg::
ReceiveFocused()
{
	CWnd *pWnd    = GetFocus();
	UINT  nCtrlID = pWnd ? pWnd->GetDlgCtrlID() : 0;

	switch( nCtrlID )
	{
		case IDC_SOUND_DIVISOR:
			OnKillfocusPokeyDivisor();
			break;
		case IDC_SOUND_LATENCY:
			OnKillfocusSoundLatency();
			break;
		case IDC_SOUND_QUALITY:
			OnKillfocusSoundQuality();
			break;
	}
} /* #OF# CSoundDlg::ReceiveFocused */

/*========================================================
Method   : CSoundDlg::OnOK
=========================================================*/
/* #FN#
   Called when the user clicks the OK button */
void
/* #AS#
   Nothing */
CSoundDlg::
OnOK()
{
	CSliderCtrl *pSlider = (CSliderCtrl *)GetDlgItem( IDC_SOUND_VOLUMESLIDER );
	ASSERT(NULL != pSlider);

	BOOL bCommit = TRUE;

	/* Unfortunately, edit controls do not lose the focus before
	   handling this when the user uses accelerators */
	ReceiveFocused();

	/* Check if the video recording should be stopped */
	if(	m_nSoundRate != g_Sound.nRate || m_bEnableStereo != (BOOL)POKEYSND_stereo_enabled )
	{
		if( _IsFlagSet( g_Misc.ulState, MS_VIDEO_AND_SOUND ) ) /* When streaming video with sound */
			bCommit = StreamWarning( IDS_WARN_RECORD_SNDOUT, SRW_VIDEO_STREAM );
	}
	if( bCommit )
	{
		if( m_nSoundRate    != g_Sound.nRate        ||
			m_bEnableStereo != (BOOL)POKEYSND_stereo_enabled ||
			m_ulSoundState  != g_Sound.ulState      ||
			m_nSkipUpdate   != g_Sound.nSkipUpdate  ||
			m_nSoundLatency != g_Sound.nLatency     ||
			m_nSoundQuality != g_Sound.nQuality )
		{
			bCommit = StreamWarning( IDS_WARN_RECORD_SNDOUT, SRW_SOUND_STREAM );
		}
	}
	if( bCommit )
	{
		if( m_ulSoundState != g_Sound.ulState )
		{
			g_Sound.ulState = m_ulSoundState;
			WriteRegDWORD( NULL, REG_SOUND_STATE, g_Sound.ulState );

			m_bModeChanged = TRUE;
		}
		if( m_nSoundRate != g_Sound.nRate )
		{
			g_Sound.nRate = m_nSoundRate;
			WriteRegDWORD( NULL, REG_SOUND_RATE, g_Sound.nRate );

			m_bModeChanged = TRUE;
		}
		if( m_nSkipUpdate != g_Sound.nSkipUpdate )
		{
			g_Sound.nSkipUpdate = m_nSkipUpdate;
			WriteRegDWORD( NULL, REG_SOUND_UPDATE, g_Sound.nSkipUpdate );

			m_bModeChanged = TRUE;
		}
		if( m_nSoundLatency != g_Sound.nLatency )
		{
			g_Sound.nLatency = m_nSoundLatency;
			WriteRegDWORD( NULL, REG_SOUND_LATENCY, g_Sound.nLatency );

			m_bModeChanged = TRUE;
		}
		if( m_nSoundQuality != g_Sound.nQuality )
		{
			g_Sound.nQuality = m_nSoundQuality;
			WriteRegDWORD( NULL, REG_SOUND_QUALITY, g_Sound.nQuality );

			m_bModeChanged = TRUE;
		}
		if( m_bDigitized != (BOOL)g_Sound.nDigitized )
		{
			g_Sound.nDigitized = m_bDigitized;
			WriteRegDWORD( NULL, REG_SOUND_DIGITIZED, g_Sound.nDigitized );

//			m_bModeChanged = TRUE;
		}
		if( m_bEnableStereo != (BOOL)POKEYSND_stereo_enabled )
		{
			POKEYSND_stereo_enabled = (int)m_bEnableStereo;
			WriteRegDWORD( NULL, REG_ENABLE_STEREO, POKEYSND_stereo_enabled );

			m_bModeChanged = TRUE;
		}
		/* Set new sound volume */
		if( m_nSoundVol != g_Sound.nVolume )
		{
			g_Sound.nVolume = m_nSoundVol;
			WriteRegDWORD( NULL, REG_SOUND_VOLUME, g_Sound.nVolume );

			Sound_SetVolume();
		}
		CCommonDlg::OnOK();
	}
} /* #OF# CSoundDlg::OnOK */
