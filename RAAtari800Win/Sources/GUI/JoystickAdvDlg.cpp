/****************************************************************************
File    : JoystickAdvDlg.cpp
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# CJoystickAdvDlg implementation file
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 30.09.2002
*/

#include "StdAfx.h"
#include "Atari800Win.h"
#include "JoystickAdvDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define IDC_JOYSTICKADV_FIRST		IDC_JOYSTICKADV_BUTTONSFIRE
#define IDC_JOYSTICKADV_LAST		IDC_JOYSTICKADV_CANCEL


/////////////////////////////////////////////////////////////////////////////
// CJoystickAdvDlg dialog

BEGIN_MESSAGE_MAP(CJoystickAdvDlg, CCommonDlg)
	//{{AFX_MSG_MAP(CJoystickAdvDlg)
	ON_BN_CLICKED(IDC_JOYSTICKADV_BUTTONSFIRE, OnButtonsFire)
	ON_BN_CLICKED(IDC_JOYSTICKADV_RECENTERSTICK, OnRecenterStick)
	ON_BN_CLICKED(IDC_JOYSTICKADV_BLOCKOPPOSITE, OnBlockOpposite)
	ON_BN_CLICKED(IDC_JOYSTICKADV_MULTIJOY, OnMultiJoy)
	ON_BN_CLICKED(IDC_JOYSTICKADV_EXITPAUSE, OnExitPause)
	ON_BN_CLICKED(IDC_JOYSTICKADV_DONTEXCLUDE, OnDontExclude)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_JOYSTICKADV_OK, OnOK)
	ON_BN_CLICKED(IDC_JOYSTICKADV_CANCEL, CCommonDlg::OnCancel)
END_MESSAGE_MAP()

/*========================================================
Method   : CJoystickAdvDlg::CJoystickAdvDlg
=========================================================*/
/* #FN#
   Standard constructor */
CJoystickAdvDlg::
CJoystickAdvDlg(
	ULONG *pInputState,    /* #IN# Pointer to input state flags */
	int   *pBlockOpposite, /* #IN# Pointer to block opposite flag */
	int   *pMultiJoy,      /* #IN# Pointer to multi joy flag */
	CWnd  *pParent /*=NULL*/
)
	: CCommonDlg( CJoystickAdvDlg::IDD, pParent )
{
	//{{AFX_DATA_INIT(CJoystickAdvDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	ASSERT(NULL != pInputState);

	m_pInputState    = pInputState;
	m_pBlockOpposite = pBlockOpposite;
	m_pMultiJoy      = pMultiJoy;
	m_nFirstCtrl     = IDC_JOYSTICKADV_FIRST;
	m_nLastCtrl      = IDC_JOYSTICKADV_LAST;

} /* #OF# CJoystickAdvDlg::CJoystickAdvDlg */

/*========================================================
Method   : CJoystickAdvDlg::DoDataExchange
=========================================================*/
/* #FN#
   Dynamic Data Exchange (not used) */
void
/* #AS#
   Nothing */
CJoystickAdvDlg::
DoDataExchange(
	CDataExchange *pDX /* #IN# Pointer to CDataExchange object */
)
{
	CCommonDlg::DoDataExchange( pDX );
	//{{AFX_DATA_MAP(CJoystickAdvDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP

} /* #OF# CJoystickAdvDlg::DoDataExchange */


/////////////////////////////////////////////////////////////////////////////
// CHarddiskDlg implementation

/*========================================================
Method   : CJoystickAdvDlg::SetDlgState
=========================================================*/
/* #FN#
   Sets up the state of the dialog controls */
void
/* #AS#
   Nothing */
CJoystickAdvDlg::
SetDlgState()
{
	_SetChkBttn( IDC_JOYSTICKADV_BUTTONSFIRE,   _IsFlagSet( m_ulInputState, IS_JOY_FIRE_ONLY     ) );
	_SetChkBttn( IDC_JOYSTICKADV_RECENTERSTICK, _IsFlagSet( m_ulInputState, IS_JOY_STICK_RELEASE ) );
	_SetChkBttn( IDC_JOYSTICKADV_EXITPAUSE,     _IsFlagSet( m_ulInputState, IS_JOY_EXIT_PAUSE    ) );
	_SetChkBttn( IDC_JOYSTICKADV_DONTEXCLUDE,   _IsFlagSet( m_ulInputState, IS_JOY_DONT_EXCLUDE  ) );

	_SetChkBttn( IDC_JOYSTICKADV_BLOCKOPPOSITE, m_nBlockOpposite );
	_SetChkBttn( IDC_JOYSTICKADV_MULTIJOY,      m_nMultiJoy      );

} /* #OF# CJoystickAdvDlg::SetDlgState */


/////////////////////////////////////////////////////////////////////////////
// CJoystickAdvDlg message handlers

/*========================================================
Method   : CJoystickAdvDlg::OnInitDialog
=========================================================*/
/* #FN#
   Performs special processing when the dialog box is initialized */
BOOL
/* #AS#
   TRUE unless you set the focus to a control */
CJoystickAdvDlg::
OnInitDialog()
{
	CCommonDlg::OnInitDialog();

	m_ulInputState   = *m_pInputState;
	m_nBlockOpposite = *m_pBlockOpposite;
	m_nMultiJoy      = *m_pMultiJoy;

	SetDlgState();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
} /* #OF# CJoystickAdvDlg::OnInitDialog */

/*========================================================
Method   : CJoystickAdvDlg::OnRecenterStick
=========================================================*/
/* #FN#
	Sets a state of the object regarding to an appropriate check box */
void
/* #AS#
   Nothing */
CJoystickAdvDlg::
OnRecenterStick()
{
   _ClickButton( IDC_JOYSTICKADV_RECENTERSTICK, m_ulInputState, IS_JOY_STICK_RELEASE );

} /* #OF# CJoystickAdvDlg::OnRecenterStick */

/*========================================================
Method   : CJoystickAdvDlg::OnButtonsFire
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate check box */
void
/* #AS#
   Nothing */
CJoystickAdvDlg::
OnButtonsFire()
{
	_ClickButton( IDC_JOYSTICKADV_BUTTONSFIRE, m_ulInputState, IS_JOY_FIRE_ONLY );

} /* #OF# CJoystickAdvDlg::OnButtonsFire */

/*========================================================
Method   : CJoystickAdvDlg::OnBlockOpposite
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate check box */
void
/* #AS#
   Nothing */
CJoystickAdvDlg::
OnBlockOpposite()
{
	m_nBlockOpposite = _GetChkBttn( IDC_JOYSTICKADV_BLOCKOPPOSITE );

} /* #OF# CJoystickAdvDlg::OnBlockOpposite */

/*========================================================
Method   : CJoystickAdvDlg::OnMultiJoy
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate check box */
void
/* #AS#
   Nothing */
CJoystickAdvDlg::
OnMultiJoy()
{
	m_nMultiJoy = _GetChkBttn( IDC_JOYSTICKADV_MULTIJOY );

} /* #OF# CJoystickAdvDlg::OnMultiJoy */

/*========================================================
Method   : CJoystickAdvDlg::OnExitPause
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate check box */
void
/* #AS#
   Nothing */
CJoystickAdvDlg::
OnExitPause()
{
	_ClickButton( IDC_JOYSTICKADV_EXITPAUSE, m_ulInputState, IS_JOY_EXIT_PAUSE );

} /* #OF# CJoystickAdvDlg::OnExitPause */

/*========================================================
Method   : CJoystickAdvDlg::OnDontExclude
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate check box */
void
/* #AS#
   Nothing */
CJoystickAdvDlg::
OnDontExclude()
{
	_ClickButton( IDC_JOYSTICKADV_DONTEXCLUDE, m_ulInputState, IS_JOY_DONT_EXCLUDE );

} /* #OF# CJoystickAdvDlg::OnDontExclude */

/*========================================================
Method   : CJoystickAdvDlg::OnOK
=========================================================*/
/* #FN#
   Called when the user clicks the OK button */
void
/* #AS#
   Nothing */
CJoystickAdvDlg::
OnOK()
{
	if( m_ulInputState != *m_pInputState )
		*m_pInputState = m_ulInputState;

	if( m_nBlockOpposite != *m_pBlockOpposite )
		*m_pBlockOpposite = m_nBlockOpposite;

	if( m_nMultiJoy != *m_pMultiJoy )
		*m_pMultiJoy = m_nMultiJoy;

	CCommonDlg::OnOK();

} /* #OF# CJoystickAdvDlg::OnOK */
