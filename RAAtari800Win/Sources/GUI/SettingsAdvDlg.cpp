/****************************************************************************
File    : SettingsAdvDlg.cpp
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# CSettingsAdvDlg implementation file
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 27.09.2002
*/

#include "StdAfx.h"
#include "Atari800Win.h"
#include "SettingsAdvDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define IDC_SETTINGSADV_FIRST		IDC_SETTINGSADV_REBOOTCART
#define IDC_SETTINGSADV_LAST		IDC_SETTINGSADV_CANCEL


/////////////////////////////////////////////////////////////////////////////
// CSettingsAdvDlg dialog

BEGIN_MESSAGE_MAP(CSettingsAdvDlg, CCommonDlg)
	//{{AFX_MSG_MAP(CSettingsAdvDlg)
	ON_BN_CLICKED(IDC_SETTINGSADV_REBOOTCART, OnRebootCart)
	ON_BN_CLICKED(IDC_SETTINGSADV_REBOOTVIDEO, OnRebootVideo)
	ON_BN_CLICKED(IDC_SETTINGSADV_TURNDRIVESOFF, OnTurnDrivesOff)
	ON_BN_CLICKED(IDC_SETTINGSADV_NOATTRACTMODE, OnNoAttractMode)
	ON_BN_CLICKED(IDC_SETTINGSADV_RTIME, OnRTime)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_SETTINGSADV_OK, OnOK)
	ON_BN_CLICKED(IDC_SETTINGSADV_CANCEL, CCommonDlg::OnCancel)
	ON_BN_CLICKED(IDC_SETTINGSADV_REBOOTBASIC, OnRebootBasic)
END_MESSAGE_MAP()

/*========================================================
Method   : CSettingsAdvDlg::CSettingsAdvDlg
=========================================================*/
/* #FN#
   Standard constructor */
CSettingsAdvDlg::
CSettingsAdvDlg(
	ULONG *pMiscState,       /* #IN# Pointer to misc state flags  */
	int   *pEnableRTime,     /* #IN# Pointer to R-Time8 flag      */
	CWnd  *pParent /*=NULL*/ /* #IN# Pointer to the parent window */
)
	: CCommonDlg( CSettingsAdvDlg::IDD, pParent )
{
	//{{AFX_DATA_INIT(CSettingsAdvDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	ASSERT(NULL != pMiscState && NULL != pEnableRTime);

	m_pMiscState   = pMiscState;
	m_pEnableRTime = pEnableRTime;
	m_nFirstCtrl   = IDC_SETTINGSADV_FIRST;
	m_nLastCtrl    = IDC_SETTINGSADV_LAST;

} /* #OF# CSettingsAdvDlg::CSettingsAdvDlg */

/*========================================================
Method   : CSettingsAdvDlg::DoDataExchange
=========================================================*/
/* #FN#
   Dynamic Data Exchange (not used) */
void
/* #AS#
   Nothing */
CSettingsAdvDlg::
DoDataExchange(
	CDataExchange *pDX /* #IN# Pointer to CDataExchange object */
)
{
	CCommonDlg::DoDataExchange( pDX );
	//{{AFX_DATA_MAP(CSettingsAdvDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP

} /* #OF# CSettingsAdvDlg::DoDataExchange */


/////////////////////////////////////////////////////////////////////////////
// CSettingsAdvDlg implementation

/*========================================================
Method   : CSettingsAdvDlg::SetDlgState
=========================================================*/
/* #FN#
   Sets up the state of the dialog controls */
void
/* #AS#
   Nothing */
CSettingsAdvDlg::
SetDlgState()
{
	/* Set up check buttons states */
	_SetChkBttn( IDC_SETTINGSADV_REBOOTCART,    _IsFlagSet( m_ulMiscState, MS_REBOOT_WHEN_CART ) );
	_SetChkBttn( IDC_SETTINGSADV_REBOOTVIDEO,   _IsFlagSet( m_ulMiscState, MS_REBOOT_WHEN_VIDEO ) );
	_SetChkBttn( IDC_SETTINGSADV_REBOOTBASIC,   _IsFlagSet( m_ulMiscState, MS_REBOOT_WHEN_BASIC ) );
	_SetChkBttn( IDC_SETTINGSADV_TURNDRIVESOFF, _IsFlagSet( m_ulMiscState, MS_TURN_DRIVES_OFF ) );
	_SetChkBttn( IDC_SETTINGSADV_NOATTRACTMODE, _IsFlagSet( m_ulMiscState, MS_NO_ATTRACT_MODE ) );
	_SetChkBttn( IDC_SETTINGSADV_RTIME,         (BOOL)m_nEnableRTime );

#ifdef WIN_NETWORK_GAMES
	if( ST_KAILLERA_ACTIVE )
	{
		_EnableCtrl( IDC_SETTINGSADV_NOATTRACTMODE, FALSE );
		_EnableCtrl( IDC_SETTINGSADV_RTIME,         FALSE );
	}
#endif
} /* #OF# CSettingsAdvDlg::SetDlgState */


/////////////////////////////////////////////////////////////////////////////
// CSettingsAdvDlg message handlers

/*========================================================
Method   : CSettingsAdvDlg::OnInitDialog
=========================================================*/
/* #FN#
   Performs special processing when the dialog box is initialized */
BOOL
/* #AS#
   TRUE unless you set the focus to a control */
CSettingsAdvDlg::
OnInitDialog()
{
	CCommonDlg::OnInitDialog();

	m_ulMiscState  = *m_pMiscState;
	m_nEnableRTime = *m_pEnableRTime;

	SetDlgState();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
} /* #OF# CSettingsAdvDlg::OnInitDialog */

/*========================================================
Method   : CSettingsAdvDlg::OnRebootCart
=========================================================*/
/* #FN#
	Sets a state of the object regarding to an appropriate check box */
void
/* #AS#
   Nothing */
CSettingsAdvDlg::
OnRebootCart()
{
	_ClickButton( IDC_SETTINGSADV_REBOOTCART, m_ulMiscState, MS_REBOOT_WHEN_CART );

} /* #OF# CSettingsAdvDlg::OnRebootCart */

/*========================================================
Method   : CSettingsAdvDlg::OnRebootVideo
=========================================================*/
/* #FN#
	Sets a state of the object regarding to an appropriate check box */
void
/* #AS#
   Nothing */
CSettingsAdvDlg::
OnRebootVideo()
{
	_ClickButton( IDC_SETTINGSADV_REBOOTVIDEO, m_ulMiscState, MS_REBOOT_WHEN_VIDEO );

} /* #OF# CSettingsAdvDlg::OnRebootVideo */

/*========================================================
Method   : CSettingsAdvDlg::OnRebootBasic
=========================================================*/
/* #FN#
	Sets a state of the object regarding to an appropriate check box */
void CSettingsAdvDlg::OnRebootBasic()
{
	_ClickButton( IDC_SETTINGSADV_REBOOTBASIC, m_ulMiscState, MS_REBOOT_WHEN_BASIC );

} /* #OF# CSettingsAdvDlg::OnRebootBasic */

/*========================================================
Method   : CSettingsAdvDlg::OnTurnDrivesOff
=========================================================*/
/* #FN#
	Sets a state of the object regarding to an appropriate check box */
void
/* #AS#
   Nothing */
CSettingsAdvDlg::
OnTurnDrivesOff()
{
	_ClickButton( IDC_SETTINGSADV_TURNDRIVESOFF, m_ulMiscState, MS_TURN_DRIVES_OFF );

} /* #OF# CSettingsAdvDlg::OnTurnDrivesOff */

/*========================================================
Method   : CSettingsAdvDlg::OnNoAttractMode
=========================================================*/
/* #FN#
	Sets a state of the object regarding to an appropriate check box */
void
/* #AS#
   Nothing */
CSettingsAdvDlg::
OnNoAttractMode()
{
	_ClickButton( IDC_SETTINGSADV_NOATTRACTMODE, m_ulMiscState, MS_NO_ATTRACT_MODE );

} /* #OF# CSettingsAdvDlg::OnNoAttractMode */

/*========================================================
Method   : CSettingsAdvDlg::OnRTime
=========================================================*/
/* #FN#
	Sets a state of the object regarding to an appropriate check box */
void
/* #AS#
   Nothing */
CSettingsAdvDlg::
OnRTime()
{
	m_nEnableRTime = _GetChkBttn( IDC_SETTINGSADV_RTIME );

} /* #OF# CSettingsAdvDlg::OnRTime */

/*========================================================
Method   : CSettingsAdvDlg::OnOK
=========================================================*/
/* #FN#
   Called when the user clicks the OK button */
void
/* #AS#
   Nothing */
CSettingsAdvDlg::
OnOK() 
{
	if( m_nEnableRTime != *m_pEnableRTime )
		*m_pEnableRTime = m_nEnableRTime;

	if( m_ulMiscState != *m_pMiscState )
		*m_pMiscState = m_ulMiscState;

	CCommonDlg::OnOK();

} /* #OF# CSettingsAdvDlg::OnOK */
