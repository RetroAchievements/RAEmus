/****************************************************************************
File    : PerformanceDlg.cpp
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# CPerformanceDlg implementation file
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 01.10.2003
*/

#include "StdAfx.h"
#include "Atari800Win.h"
#include "Helpers.h"
#include "PerformanceDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define IDC_PERFORMANCE_FIRST		IDC_PERFORMANCE_REFRESHRATE
#define IDC_PERFORMANCE_LAST		IDC_PERFORMANCE_CANCEL

#define MIN_REFRESH_RATE			1
#define MAX_REFRESH_RATE			99


/////////////////////////////////////////////////////////////////////////////
// Static objects

static const int s_anAvailableSpeed[] =
{
	 10,  20,  30,  40,  50,  60,  70,  80,  90, 100,
	110, 120, 130, 140, 150, 160, 170, 180, 190, 200
};

static const int s_nAvailableSpeedNo = sizeof(s_anAvailableSpeed)/sizeof(s_anAvailableSpeed[0]);


/////////////////////////////////////////////////////////////////////////////
// CPerformanceDlg dialog

BEGIN_MESSAGE_MAP(CPerformanceDlg, CCommonDlg)
	//{{AFX_MSG_MAP(CPerformanceDlg)
	ON_CBN_SELCHANGE(IDC_PERFORMANCE_SPEEDPERCENT, OnSelchangeSpeedPercent)
	ON_BN_CLICKED(IDC_PERFORMANCE_FULLSPEED, OnFullSpeed)
	ON_EN_KILLFOCUS(IDC_PERFORMANCE_REFRESHRATE, OnKillfocusRefreshRate)
	ON_NOTIFY(UDN_DELTAPOS, IDC_PERFORMANCE_REFRESHSPIN, OnDeltaposRefreshSpin)
	ON_BN_CLICKED(IDC_PERFORMANCE_DOUBLEREFRESHCHECK, OnDoubleRefreshCheck)
	ON_EN_KILLFOCUS(IDC_PERFORMANCE_DOUBLEREFRESHRATE, OnKillfocusDoubleRefreshRate)
	ON_NOTIFY(UDN_DELTAPOS, IDC_PERFORMANCE_DOUBLEREFRESHSPIN, OnDeltaposDoubleRefreshSpin)
	ON_BN_CLICKED(IDC_PERFORMANCE_SPEEDREFRESHCHECK, OnSpeedRefreshCheck)
	ON_EN_KILLFOCUS(IDC_PERFORMANCE_SPEEDREFRESHRATE, OnKillfocusSpeedRefreshRate)
	ON_NOTIFY(UDN_DELTAPOS, IDC_PERFORMANCE_SPEEDREFRESHSPIN, OnDeltaposSpeedRefreshSpin)
	ON_BN_CLICKED(IDC_PERFORMANCE_NODRAWDISPLAY, OnNoDrawDisplay)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_PERFORMANCE_OK, OnOK)
	ON_BN_CLICKED(IDC_PERFORMANCE_CANCEL, CCommonDlg::OnCancel)
END_MESSAGE_MAP()

/*========================================================
Method   : CPerformanceDlg::CPerformanceDlg
=========================================================*/
/* #FN#
   Standard constructor */
CPerformanceDlg::
CPerformanceDlg(
	CWnd *pParent /*=NULL*/ /* #IN# Pointer to the parent window */
)
	: CCommonDlg( CPerformanceDlg::IDD, pParent )
{
	//{{AFX_DATA_INIT(CPerformanceDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_nFirstCtrl = IDC_PERFORMANCE_FIRST;
	m_nLastCtrl  = IDC_PERFORMANCE_LAST;

} /* #OF# CPerformanceDlg::CPerformanceDlg */

/*========================================================
Method   : CPerformanceDlg::DoDataExchange
=========================================================*/
/* #FN#
   Dynamic Data Exchange (not used) */
void
/* #AS#
   Nothing */
CPerformanceDlg::
DoDataExchange(
	CDataExchange *pDX /* #IN# Pointer to CDataExchange object */
)
{
	CCommonDlg::DoDataExchange( pDX );
	//{{AFX_DATA_MAP(CPerformanceDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP

} /* #OF# CPerformanceDlg::DoDataExchange */


/////////////////////////////////////////////////////////////////////////////
// CPerformanceDlg implementation

/*========================================================
Method   : CPerformanceDlg::SetDlgState
=========================================================*/
/* #FN#
   Sets up the state of the dialog controls */
void
/* #AS#
   Nothing */
CPerformanceDlg::
SetDlgState()
{
	CComboBox *pCombo = (CComboBox *)GetDlgItem( IDC_PERFORMANCE_SPEEDPERCENT );
	ASSERT(NULL != pCombo);

	char szItem[ 4 ];
	int  nSel = 5;

	/* Set up "Speed Percent" combo */
	pCombo->ResetContent();
	for( int i = 0; i < s_nAvailableSpeedNo; i++ )
	{
		sprintf( szItem, "%d", s_anAvailableSpeed[ i ] );
		pCombo->AddString( szItem );
		if( s_anAvailableSpeed[ i ] == m_nSpeedPercent )
			nSel = i;
	}
	pCombo->SetCurSel( nSel );
	pCombo->EnableWindow( !_IsFlagSet( m_ulMiscState, MS_FULL_SPEED ) );

	_EnableCtrl( IDC_PERFORMANCE_SPEEDPERCENT_LABEL, !_IsFlagSet( m_ulMiscState, MS_FULL_SPEED ) );

	/* Set up check buttons states */
	_SetChkBttn( IDC_PERFORMANCE_FULLSPEED,          _IsFlagSet( m_ulMiscState, MS_FULL_SPEED ) );
	_SetChkBttn( IDC_PERFORMANCE_DOUBLEREFRESHCHECK, _IsFlagSet( m_ulMiscState, MS_USE_DOUBLEWND_REFRESH ) );
	_SetChkBttn( IDC_PERFORMANCE_SPEEDREFRESHCHECK,  _IsFlagSet( m_ulMiscState, MS_USE_FULLSPEED_REFRESH ) );

	/* Set up refresh rate stuff */
	SetDlgItemInt( IDC_PERFORMANCE_REFRESHRATE, m_nRefreshRate, FALSE );

	/* For double-windowed modes */
	_EnableCtrl( IDC_PERFORMANCE_DOUBLEREFRESHRATE,  _IsFlagSet( m_ulMiscState, MS_USE_DOUBLEWND_REFRESH ) );
	_EnableCtrl( IDC_PERFORMANCE_DOUBLEREFRESHSPIN,  _IsFlagSet( m_ulMiscState, MS_USE_DOUBLEWND_REFRESH ) );
	SetDlgItemInt( IDC_PERFORMANCE_DOUBLEREFRESHRATE, m_nRefreshDoubleWnd, FALSE );

	/* For full-speed modes */
	_EnableCtrl( IDC_PERFORMANCE_SPEEDREFRESHRATE,   _IsFlagSet( m_ulMiscState, MS_USE_FULLSPEED_REFRESH ) );
	_EnableCtrl( IDC_PERFORMANCE_SPEEDREFRESHSPIN,   _IsFlagSet( m_ulMiscState, MS_USE_FULLSPEED_REFRESH ) );
	SetDlgItemInt( IDC_PERFORMANCE_SPEEDREFRESHRATE, m_nRefreshFullSpeed, FALSE );

	_SetChkBttn( IDC_PERFORMANCE_NODRAWDISPLAY, _IsFlagSet( m_ulMiscState, MS_NO_DRAW_DISPLAY ) );

#ifdef WIN_NETWORK_GAMES
	if( ST_KAILLERA_ACTIVE )
	{
		_EnableCtrl( IDC_PERFORMANCE_NODRAWDISPLAY, FALSE );
	}
	else
#endif
		_EnableCtrl( IDC_PERFORMANCE_NODRAWDISPLAY, m_nRefreshRate > 1 ||
					(_IsFlagSet( m_ulMiscState, MS_USE_DOUBLEWND_REFRESH ) && m_nRefreshDoubleWnd > 1) ||
					(_IsFlagSet( m_ulMiscState, MS_USE_FULLSPEED_REFRESH ) && m_nRefreshFullSpeed > 1) );

} /* #OF# CPerformanceDlg::SetDlgState */


/////////////////////////////////////////////////////////////////////////////
// CPerformanceDlg message handlers

/*========================================================
Method   : CPerformanceDlg::OnInitDialog
=========================================================*/
/* #FN#
   Performs special processing when the dialog box is initialized */
BOOL
/* #AS#
   TRUE unless you set the focus to a control */
CPerformanceDlg::
OnInitDialog()
{
	CCommonDlg::OnInitDialog();

	m_ulMiscState       = g_Misc.ulState;
	m_nSpeedPercent     = g_Misc.nSpeedPercent;
	m_nRefreshRate      = Atari800_refresh_rate;
	m_nRefreshDoubleWnd = g_Misc.Refresh.nDoubleWnd;
	m_nRefreshFullSpeed = g_Misc.Refresh.nFullSpeed;

	SetDlgState();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
} /* #OF# CPerformanceDlg::OnInitDialog */

/*========================================================
Method   : CPerformanceDlg::OnSelchangeSpeedPercent
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate combo box */
void
/* #AS#
   Nothing */
CPerformanceDlg::
OnSelchangeSpeedPercent()
{
	m_nSpeedPercent = s_anAvailableSpeed[ _GetSelCbox( IDC_PERFORMANCE_SPEEDPERCENT ) ];

} /* #OF# CPerformanceDlg::OnSelchangeSpeedPercent */

/*========================================================
Method   : CPerformanceDlg::OnFullSpeed
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate check box */
void
/* #AS#
   Nothing */
CPerformanceDlg::
OnFullSpeed() 
{
	_ClickButton( IDC_PERFORMANCE_FULLSPEED, m_ulMiscState, MS_FULL_SPEED );
	SetDlgState();

} /* #OF# CPerformanceDlg::OnFullSpeed */

/*========================================================
Method   : CPerformanceDlg::OnDeltaposRefreshSpin
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate spin control */
void
/* #AS#
   Nothing */
CPerformanceDlg::
OnDeltaposRefreshSpin(
	NMHDR   *pNMHDR, /* #IN#  */
	LRESULT *pResult /* #OUT# */
)
{
	_DeltaposSpin( pNMHDR, IDC_PERFORMANCE_REFRESHRATE, m_nRefreshRate, MIN_REFRESH_RATE, MAX_REFRESH_RATE );
	SetDlgState();

	*pResult = 0;

} /* #OF# CPerformanceDlg::OnDeltaposRefreshSpin */

/*========================================================
Method   : CPerformanceDlg::OnKillfocusRefreshRate
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate edit control */
void
/* #AS#
   Nothing */
CPerformanceDlg::
OnKillfocusRefreshRate()
{
	_KillfocusSpin( IDC_PERFORMANCE_REFRESHRATE, m_nRefreshRate, MIN_REFRESH_RATE, MAX_REFRESH_RATE );
	SetDlgState();

} /* #OF# CPerformanceDlg::OnKillfocusRefreshRate */

/*========================================================
Method   : CPerformanceDlg::OnDoubleRefreshCheck
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate check box */
void
/* #AS#
   Nothing */
CPerformanceDlg::
OnDoubleRefreshCheck()
{
	_ClickButton( IDC_PERFORMANCE_DOUBLEREFRESHCHECK, m_ulMiscState, MS_USE_DOUBLEWND_REFRESH );
	SetDlgState();

} /* #OF# CPerformanceDlg::OnDoubleRefreshCheck */

/*========================================================
Method   : CPerformanceDlg::OnDeltaposDoubleRefreshSpin
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate spin control */
void
/* #AS#
   Nothing */
CPerformanceDlg::
OnDeltaposDoubleRefreshSpin(
	NMHDR   *pNMHDR, /* #IN#  */
	LRESULT *pResult /* #OUT# */
)
{
	_DeltaposSpin( pNMHDR, IDC_PERFORMANCE_DOUBLEREFRESHRATE, m_nRefreshDoubleWnd, MIN_REFRESH_RATE, MAX_REFRESH_RATE );
	SetDlgState();

	*pResult = 0;

} /* #OF# CPerformanceDlg::OnDeltaposDoubleRefreshSpin */

/*========================================================
Method   : CPerformanceDlg::OnKillfocusDoubleRefreshRate
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate edit control */
void
/* #AS#
   Nothing */
CPerformanceDlg::
OnKillfocusDoubleRefreshRate()
{
	_KillfocusSpin( IDC_PERFORMANCE_DOUBLEREFRESHRATE, m_nRefreshDoubleWnd, MIN_REFRESH_RATE, MAX_REFRESH_RATE );
	SetDlgState();

} /* #OF# CPerformanceDlg::OnKillfocusDoubleRefreshRate */

/*========================================================
Method   : CPerformanceDlg::OnSpeedRefreshCheck
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate check box */
void
/* #AS#
   Nothing */
CPerformanceDlg::
OnSpeedRefreshCheck()
{
	_ClickButton( IDC_PERFORMANCE_SPEEDREFRESHCHECK, m_ulMiscState, MS_USE_FULLSPEED_REFRESH );
	SetDlgState();

} /* #OF# CPerformanceDlg::OnSpeedRefreshCheck */

/*========================================================
Method   : CPerformanceDlg::OnDeltaposSpeedRefreshSpin
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate spin control */
void
/* #AS#
   Nothing */
CPerformanceDlg::
OnDeltaposSpeedRefreshSpin(
	NMHDR   *pNMHDR, /* #IN#  */
	LRESULT *pResult /* #OUT# */
)
{
	_DeltaposSpin( pNMHDR, IDC_PERFORMANCE_SPEEDREFRESHRATE, m_nRefreshFullSpeed, MIN_REFRESH_RATE, MAX_REFRESH_RATE );
	SetDlgState();

	*pResult = 0;

} /* #OF# CPerformanceDlg::OnDeltaposSpeedRefreshSpin */

/*========================================================
Method   : CPerformanceDlg::OnKillfocusSpeedRefreshRate
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate edit control */
void
/* #AS#
   Nothing */
CPerformanceDlg::
OnKillfocusSpeedRefreshRate()
{
	_KillfocusSpin( IDC_PERFORMANCE_SPEEDREFRESHRATE, m_nRefreshFullSpeed, MIN_REFRESH_RATE, MAX_REFRESH_RATE );
	SetDlgState();

} /* #OF# CPerformanceDlg::OnKillfocusSpeedRefreshRate */

/*========================================================
Method   : CPerformanceDlg::OnNoDrawDisplay
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate check box */
void
/* #AS#
   Nothing */
CPerformanceDlg::
OnNoDrawDisplay()
{
	_ClickButton( IDC_PERFORMANCE_NODRAWDISPLAY, m_ulMiscState, MS_NO_DRAW_DISPLAY );
	SetDlgState();

} /* #OF# CPerformanceDlg::OnNoDrawDisplay */

/*========================================================
Method   : CPerformanceDlg::ReceiveFocused
=========================================================*/
/* #FN#
   Receives the edit controls content again. The user could press
   'Enter' or 'Alt-O' and then all changes he's made in the last
   edited control would be lost. */
void
/* #AS#
   Nothing */
CPerformanceDlg::
ReceiveFocused()
{
	CWnd *pWnd    = GetFocus();
	UINT  nCtrlID = pWnd ? pWnd->GetDlgCtrlID() : 0;

	switch( nCtrlID )
	{
		case IDC_PERFORMANCE_REFRESHRATE:
			OnKillfocusRefreshRate();
			break;
		case IDC_PERFORMANCE_DOUBLEREFRESHRATE:
			OnKillfocusDoubleRefreshRate();
			break;
		case IDC_PERFORMANCE_SPEEDREFRESHRATE:
			OnKillfocusSpeedRefreshRate();
			break;
	}
} /* #OF# CPerformanceDlg::ReceiveFocused */

/* These macros had been taken from macros.h and then modified */
#define PREVIEW_DOUBLEWND_REFRESH \
	(m_ulMiscState & MS_USE_DOUBLEWND_REFRESH && \
	 g_Screen.ulState & SM_MODE_WIND && \
	 g_Screen.ulState & SM_WRES_DOUBLE)

#define PREVIEW_FULLSPEED_REFRESH \
	(m_ulMiscState & MS_USE_FULLSPEED_REFRESH && \
	 m_ulMiscState & MS_FULL_SPEED)

#define _PreviewRefreshRate() \
	(PREVIEW_FULLSPEED_REFRESH ? m_nRefreshFullSpeed : \
	(PREVIEW_DOUBLEWND_REFRESH ? m_nRefreshDoubleWnd : m_nRefreshRate))

/*========================================================
Method   : CPerformanceDlg::OnOK
=========================================================*/
/* #FN#
   Called when the user clicks the OK button */
void
/* #AS#
   Nothing */
CPerformanceDlg::
OnOK()
{
	BOOL bCommit = TRUE;

	/* Unfortunately, edit controls do not lose the focus before
	   handling this when the user uses accelerators */
	ReceiveFocused();

	/* Check if the video recording should be stopped */
	if( _PreviewRefreshRate() != _GetRefreshRate() )
	{
		bCommit = StreamWarning( IDS_WARN_RECORD_REFRESH, SRW_VIDEO_STREAM );
	}
	else if( (m_ulMiscState & MS_FULL_SPEED) != (g_Misc.ulState & MS_FULL_SPEED) )
	{
		bCommit = StreamWarning( IDS_WARN_RECORD_FULLSPEED, SRW_VIDEO_STREAM );
	}
	else if( m_nSpeedPercent != g_Misc.nSpeedPercent )
	{
		bCommit = StreamWarning( IDS_WARN_RECORD_SPEED, SRW_VIDEO_STREAM );
	}
	if( bCommit )
	{
		if( m_nRefreshRate != Atari800_refresh_rate )
		{
			Atari800_refresh_rate = m_nRefreshRate;
			WriteRegDWORD( NULL, REG_REFRESH_RATE, Atari800_refresh_rate );

			if( !ST_DOUBLEWND_REFRESH && !ST_FULLSPEED_REFRESH )
				g_nTestVal = Atari800_refresh_rate - 1;
		}
		if( m_nRefreshDoubleWnd != g_Misc.Refresh.nDoubleWnd )
		{
			g_Misc.Refresh.nDoubleWnd = m_nRefreshDoubleWnd;
			WriteRegDWORD( NULL, REG_REFRESH_DOUBLEWND, g_Misc.Refresh.nDoubleWnd );

			if( ST_DOUBLEWND_REFRESH && !ST_FULLSPEED_REFRESH )
				g_nTestVal = g_Misc.Refresh.nDoubleWnd - 1;
		}
		if( m_nRefreshFullSpeed != g_Misc.Refresh.nFullSpeed )
		{
			g_Misc.Refresh.nFullSpeed = m_nRefreshFullSpeed;
			WriteRegDWORD( NULL, REG_REFRESH_FULLSPEED, g_Misc.Refresh.nFullSpeed );

			if( ST_FULLSPEED_REFRESH )
				g_nTestVal = g_Misc.Refresh.nFullSpeed - 1;
		}
		if( m_ulMiscState != g_Misc.ulState )
		{
			BOOL bFullSpeed = _IsFlagSet( g_Misc.ulState, MS_FULL_SPEED );

			g_Misc.ulState = m_ulMiscState;
			WriteRegDWORD( NULL, REG_MISC_STATE, g_Misc.ulState );

			if( !bFullSpeed && _IsFlagSet( g_Misc.ulState, MS_FULL_SPEED ) )
				Sound_Clear( FALSE, FALSE );
			else
			if( bFullSpeed && !_IsFlagSet( g_Misc.ulState, MS_FULL_SPEED ) )
				Sound_Restart();
		}
		if( m_nSpeedPercent != g_Misc.nSpeedPercent )
		{
			g_Misc.nSpeedPercent = m_nSpeedPercent;
			WriteRegDWORD( NULL, REG_SPEED_PERCENT, g_Misc.nSpeedPercent );

			g_Timer.nPalFreq  = (DEF_PAL_FREQUENCY  * m_nSpeedPercent) / 100;
			g_Timer.nNtscFreq = (DEF_NTSC_FREQUENCY * m_nSpeedPercent) / 100;

			/* The Atari timer stuff has to be reinitialised */
			Timer_Reset();

			/* Sound depends on the timer settings */
			Sound_Initialise( FALSE );

			/* Start an Atari timer */
			Timer_Start( FALSE );
		}
		CCommonDlg::OnOK();
	}
} /* #OF# CPerformanceDlg::OnOK */
