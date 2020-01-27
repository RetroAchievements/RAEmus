/****************************************************************************
File    : JoystickDlg.cpp
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# CJoystickDlg implementation file
@(#) #BY# Richard Lawrence, Tomasz Szymankowski
@(#) #LM# 30.09.2002
*/

#include "StdAfx.h"
#include "Atari800Win.h"
#include "Helpers.h"
#include "KeysetDlg.h"
#include "JoystickAdvDlg.h"
#include "JoystickDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define IDC_JOYSTICK_FIRST		IDC_JOYSTICK_PORT
#define IDC_JOYSTICK_LAST		IDC_JOYSTICK_CANCEL


/////////////////////////////////////////////////////////////////////////////
// Static objects

static DWORD s_dwStickIds[ MAX_ATARI_JOYPORTS ] =
{
	IDC_JOYSTICK_STICK1,
	IDC_JOYSTICK_STICK2,
	IDC_JOYSTICK_STICK3,
	IDC_JOYSTICK_STICK4
};

static DWORD s_dwAfireIds[ MAX_ATARI_JOYPORTS ] =
{
	IDC_JOYSTICK_AUTOFIRE1,
	IDC_JOYSTICK_AUTOFIRE2,
	IDC_JOYSTICK_AUTOFIRE3,
	IDC_JOYSTICK_AUTOFIRE4
};


/////////////////////////////////////////////////////////////////////////////
// CJoystickDlg dialog

BEGIN_MESSAGE_MAP(CJoystickDlg, CCommonDlg)
	//{{AFX_MSG_MAP(CJoystickDlg)
	ON_CBN_SELCHANGE(IDC_JOYSTICK_PORT, OnSelchangePort)
	ON_CBN_SELCHANGE(IDC_JOYSTICK_STICK1, OnSelchangeStick1)
	ON_CBN_SELCHANGE(IDC_JOYSTICK_STICK2, OnSelchangeStick2)
	ON_CBN_SELCHANGE(IDC_JOYSTICK_STICK3, OnSelchangeStick3)
	ON_CBN_SELCHANGE(IDC_JOYSTICK_STICK4, OnSelchangeStick4)
	ON_CBN_SELCHANGE(IDC_JOYSTICK_STICK, OnSelchangeStick)
	ON_CBN_SELCHANGE(IDC_JOYSTICK_AUTOFIREMODE, OnSelchangeAutofireMode)
	ON_BN_CLICKED(IDC_JOYSTICK_AUTOFIRE1, OnAutofire1)
	ON_BN_CLICKED(IDC_JOYSTICK_AUTOFIRE2, OnAutofire2)
	ON_BN_CLICKED(IDC_JOYSTICK_AUTOFIRE3, OnAutofire3)
	ON_BN_CLICKED(IDC_JOYSTICK_AUTOFIRE4, OnAutofire4)
	ON_BN_CLICKED(IDC_JOYSTICK_AUTOFIRE, OnAutofire)
	ON_BN_CLICKED(IDC_JOYSTICK_KEYSETA, OnKeysetA)
	ON_BN_CLICKED(IDC_JOYSTICK_KEYSETB, OnKeysetB)
	ON_BN_CLICKED(IDC_JOYSTICK_ADVSETTING, OnAdvSetting)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_JOYSTICK_OK, OnOK)
	ON_BN_CLICKED(IDC_JOYSTICK_CANCEL, CCommonDlg::OnCancel)
END_MESSAGE_MAP()

/*========================================================
Method   : CJoystickDlg::CJoystickDlg
=========================================================*/
/* #FN#
   Standard constructor */
CJoystickDlg::
CJoystickDlg(
	CWnd *pParent /*=NULL*/
)
	: CCommonDlg( _IsFlagSet( g_Screen.ulState, SM_ATTR_SMALL_DLG ) ? IDD_JOYSTICK_SMALL : CJoystickDlg::IDD, pParent )
{
	//{{AFX_DATA_INIT(CJoystickDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_nFirstCtrl = IDC_JOYSTICK_FIRST;
	m_nLastCtrl  = IDC_JOYSTICK_LAST;

} /* #OF# CJoystickDlg::CJoystickDlg */

/*========================================================
Method   : CJoystickDlg::DoDataExchange
=========================================================*/
/* #FN#
   Dynamic Data Exchange (not used) */
void
/* #AS#
   Nothing */
CJoystickDlg::
DoDataExchange(
	CDataExchange *pDX /* #IN# Pointer to CDataExchange object */
)
{
	CCommonDlg::DoDataExchange( pDX );
	//{{AFX_DATA_MAP(CJoystickDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP

} /* #OF# CJoystickDlg::DoDataExchange */


/////////////////////////////////////////////////////////////////////////////
// CHarddiskDlg implementation

/*========================================================
Method   : CJoystickDlg::SetDlgState
=========================================================*/
/* #FN#
   Sets up the state of the dialog controls */
void
/* #AS#
   Nothing */
CJoystickDlg::
SetDlgState()
{
	CComboBox *pCombo = NULL;
	char szItem[ LOADSTRING_SIZE_S + 1 ];

	for( int i = 0; i < MAX_ATARI_JOYPORTS; i++ )
	{
		pCombo = (CComboBox *)GetDlgItem( m_bSmallMode ? IDC_JOYSTICK_STICK : s_dwStickIds[ i ] );
		ASSERT(NULL != pCombo);
		pCombo->ResetContent();

		/* Keyboard input */
		pCombo->AddString( _LoadStringSx( IDS_JOY_NUMPAD,   szItem ) );
		pCombo->AddString( _LoadStringSx( IDS_JOY_ARROWS,   szItem ) );
		pCombo->AddString( _LoadStringSx( IDS_JOY_KEYSET_A, szItem ) );
		pCombo->AddString( _LoadStringSx( IDS_JOY_KEYSET_B, szItem ) );

		/* Available input devices */
		for( int j = 0; j < g_Input.nDevFoundNum; j++ )
			pCombo->AddString( g_Input.acDevNames[ j ] );

		/* No input device */
		pCombo->AddString( _LoadStringSx( IDS_JOY_NONE, szItem ) );

		if(	m_anDevSelected[ i ] != NO_JOYSTICK )
		{
			pCombo->SetCurSel( m_anDevSelected[ i ] + NUM_KBJOY_DEVICES );
		}
		else
			pCombo->SetCurSel( g_Input.nDevFoundNum + NUM_KBJOY_DEVICES );

		if( _IsFlagSet( m_ulAutoSticks, (1 << i) ) )
		{
			_SetChkBttn( m_bSmallMode ? IDC_JOYSTICK_AUTOFIRE : s_dwAfireIds[ i ], TRUE );
		}
		if( m_bSmallMode )
		{
			_SetSelCbox( IDC_JOYSTICK_PORT, 0 );
			/* There is one combo only in a small version of the window */
			break;
		}
	}
	_SetSelCbox( IDC_JOYSTICK_AUTOFIREMODE, m_nAutoMode );

} /* #OF# CJoystickDlg::SetDlgState */


/////////////////////////////////////////////////////////////////////////////
// CJoystickDlg message handlers

/*========================================================
Method   : CJoystickDlg::OnInitDialog
=========================================================*/
/* #FN#
   Performs special processing when the dialog box is initialized */
BOOL
/* #AS#
   TRUE unless you set the focus to a control */
CJoystickDlg::
OnInitDialog()
{
	CCommonDlg::OnInitDialog();

	m_ulInputState   = g_Input.ulState;
	m_ulAutoSticks   = g_Input.Joy.ulAutoSticks;
	m_nAutoMode      = g_Input.Joy.nAutoMode;
	m_nBlockOpposite = INPUT_joy_block_opposite_directions;
	m_nMultiJoy      = INPUT_joy_multijoy;

	m_bSmallMode = _IsFlagSet( g_Screen.ulState, SM_ATTR_SMALL_DLG );

	for( int i = 0; i < MAX_ATARI_JOYPORTS; i++ )
		m_anDevSelected[ i ] = g_Input.anDevSelected[ i ];

	SetDlgState();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
} /* #OF# CJoystickDlg::OnInitDialog */

/*========================================================
Method   : CJoystickDlg::OnSelchangePort
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate combo box.
   Small window mode */
void
/* #AS#
   Nothing */
CJoystickDlg::
OnSelchangePort()
{
	int nStickNum = 0;

	if( CB_ERR == (nStickNum = _GetSelCbox( IDC_JOYSTICK_PORT )) )
	{
		_SetSelCbox( IDC_JOYSTICK_PORT, 0 );
		nStickNum = 0;
	}
	if( m_anDevSelected[ nStickNum ] != NO_JOYSTICK )
	{
		/* Set an appropriate device if any has been selected */
		_SetSelCbox( IDC_JOYSTICK_STICK, m_anDevSelected[ nStickNum ] + NUM_KBJOY_DEVICES );
	}
	else
		_SetSelCbox( IDC_JOYSTICK_STICK, g_Input.nDevFoundNum + NUM_KBJOY_DEVICES );

	_SetChkBttn( IDC_JOYSTICK_AUTOFIRE, _IsFlagSet( m_ulAutoSticks, (1 << nStickNum) ) );

} /* #OF# CJoystickDlg::OnSelchangePort */

/*========================================================
Method   : CJoystickDlg::OnSelchangeStick
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate combo box.
   Small window mode */
void
/* #AS#
   Nothing */
CJoystickDlg::
OnSelchangeStick()
{
	int nStickNum = 0;

	if( CB_ERR == (nStickNum = _GetSelCbox( IDC_JOYSTICK_PORT )) )
	{
		_SetSelCbox( IDC_JOYSTICK_PORT, 0 );
		nStickNum = 0;
	}
	SelchangeStick( nStickNum );

} /* #OF# CJoystickDlg::OnSelchangeStick */

/*========================================================
Method   : CJoystickDlg::OnAutofire
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate combo box.
   Small window mode */
void
/* #AS#
   Nothing */
CJoystickDlg::
OnAutofire()
{
	int nStickNum = 0;

	if( CB_ERR == (nStickNum = _GetSelCbox( IDC_JOYSTICK_PORT )) )
	{
		_SetSelCbox( IDC_JOYSTICK_PORT, 0 );
		nStickNum = 0;
	}
	Autofire( nStickNum );

} /* #OF# CJoystickDlg::OnAutofire */

/*========================================================
Method   : CJoystickDlg::SelchangeStick
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate combo box */
void
/* #AS#
   Nothing */
CJoystickDlg::
SelchangeStick(
	int nStickNum
)
{
	CComboBox *pStick = (CComboBox *)GetDlgItem( m_bSmallMode ? IDC_JOYSTICK_STICK : s_dwStickIds[ nStickNum ] );
	int nJoy;

	ASSERT(NULL != pStick);

	/* Get the joystick selection */
	if( CB_ERR == (nJoy = pStick->GetCurSel()) )
		return;

	if( nJoy <= g_Input.nDevFoundNum + NUM_KBJOY_DEVICES - 1  /* Zero based index */ )
	{
		nJoy -= NUM_KBJOY_DEVICES;

		for( int i = 0; i < MAX_ATARI_JOYPORTS; i++ )
		{
			if( (nJoy == m_anDevSelected[ i ]) && i != nStickNum )
			{
				if( IDYES == DisplayMessage( GetSafeHwnd(), IDS_WARN_STICK_CONFLICT, 0, MB_ICONQUESTION | MB_YESNO, i + 1, nStickNum + 1 ) )
				{
					if( !m_bSmallMode )
					{
						_SetSelCbox( s_dwStickIds[ i ], g_Input.nDevFoundNum + NUM_KBJOY_DEVICES );
					}
					m_anDevSelected[ i ] = NO_JOYSTICK;
				}
				else
					nJoy = m_anDevSelected[ nStickNum ];

				/* There may be only one the same device associated
				   to a port, break */
				break;
			}
		}
	}
	else
		nJoy = NO_JOYSTICK;

	/* Set the selected device to use */
	m_anDevSelected[ nStickNum ] = nJoy;
	/* Synchronize a device combo box */
	pStick->SetCurSel( (NO_JOYSTICK == nJoy ? g_Input.nDevFoundNum : nJoy) + NUM_KBJOY_DEVICES );

} /* #OF# CJoystickDlg::SelchangeStick */

/*========================================================
Method   : CJoystickDlg::OnSelchangeStick1
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate combo box */
void
/* #AS#
   Nothing */
CJoystickDlg::
OnSelchangeStick1()
{
	SelchangeStick( 0 );

} /* #OF# CJoystickDlg::OnSelchangeStick1 */

/*========================================================
Method   : CJoystickDlg::OnSelchangeStick2
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate combo box */
void
/* #AS#
   Nothing */
CJoystickDlg::
OnSelchangeStick2()
{
	SelchangeStick( 1 );

} /* #OF# CJoystickDlg::OnSelchangeStick2 */

/*========================================================
Method   : CJoystickDlg::OnSelchangeStick3
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate combo box */
void
/* #AS#
   Nothing */
CJoystickDlg::
OnSelchangeStick3()
{
	SelchangeStick( 2 );

} /* #OF# CJoystickDlg::OnSelchangeStick3 */

/*========================================================
Method   : CJoystickDlg::OnSelchangeStick4
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate combo box */
void
/* #AS#
   Nothing */
CJoystickDlg::
OnSelchangeStick4()
{
	SelchangeStick( 3 );

} /* #OF# CJoystickDlg::OnSelchangeStick4 */

/*========================================================
Method   : CJoystickDlg::Autofire
=========================================================*/
void
/* #AS#
   Nothing */
CJoystickDlg::
Autofire(
	int nStickNum
)
{
	_ClickButton( m_bSmallMode ? IDC_JOYSTICK_AUTOFIRE : s_dwAfireIds[ nStickNum ], m_ulAutoSticks, 1 << nStickNum );

} /* #OF# CJoystickDlg::Autofire */

/*========================================================
Method   : CJoystickDlg::OnAutofire1
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate check box */
void
/* #AS#
   Nothing */
CJoystickDlg::
OnAutofire1()
{
	Autofire( 0 );

} /* #OF# CJoystickDlg::OnAutofire1 */

/*========================================================
Method   : CJoystickDlg::OnAutofire2
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate check box */
void
/* #AS#
   Nothing */
CJoystickDlg::
OnAutofire2()
{
	Autofire( 1 );

} /* #OF# CJoystickDlg::OnAutofire2 */

/*========================================================
Method   : CJoystickDlg::OnAutofire3
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate check box */
void
/* #AS#
   Nothing */
CJoystickDlg::
OnAutofire3()
{
	Autofire( 2 );

} /* #OF# CJoystickDlg::OnAutofire3 */

/*========================================================
Method   : CJoystickDlg::OnAutofire4
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate check box */
void
/* #AS#
   Nothing */
CJoystickDlg::
OnAutofire4()
{
	Autofire( 3 );

} /* #OF# CJoystickDlg::OnAutofire4 */

/*========================================================
Method   : CJoystickDlg::OnSelchangeAutofireMode
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate combo box */
void
/* #AS#
   Nothing */
CJoystickDlg::
OnSelchangeAutofireMode()
{
	m_nAutoMode = _GetSelCbox( IDC_JOYSTICK_AUTOFIREMODE );

} /* #OF# CJoystickDlg::OnSelchangeAutofireMode */

/*========================================================
Method   : CJoystickDlg::OnKeysetA
=========================================================*/
/* #FN#
   Displays "Configure Keyset A" dialog box */
void
/* #AS#
   Nothing */
CJoystickDlg::
OnKeysetA()
{
	CKeysetDlg dlgKeyset;

	if( _IsFlagSet( m_ulInputState, IS_JOY_DONT_EXCLUDE ) ||
		DisplayWarning( IDS_WARN_KBJOY, DONT_SHOW_KBJOY_WARN, TRUE ) )
	{
		dlgKeyset.SetTitlePostfix( " A" );
		dlgKeyset.SetCurrentKeyset( KEYS_A_JOYSTICK + NUM_KBJOY_DEVICES );
		dlgKeyset.DoModal();
	}
} /* #OF# CJoystickDlg::OnKeysetA */

/*========================================================
Method   : CJoystickDlg::OnKeysetB
=========================================================*/
/* #FN#
   Displays "Configure Keyset B" dialog box */
void
/* #AS#
   Nothing */
CJoystickDlg::
OnKeysetB()
{
	CKeysetDlg dlgKeyset;

	if( _IsFlagSet( m_ulInputState, IS_JOY_DONT_EXCLUDE ) ||
		DisplayWarning( IDS_WARN_KBJOY, DONT_SHOW_KBJOY_WARN, TRUE ) )
	{
		dlgKeyset.SetTitlePostfix( " B" );
		dlgKeyset.SetCurrentKeyset( KEYS_B_JOYSTICK + NUM_KBJOY_DEVICES );
		dlgKeyset.DoModal();
	}
} /* #OF# CJoystickDlg::OnKeysetB */

/*========================================================
Method   : CJoystickDlg::OnAdvSetting
=========================================================*/
/* #FN#
   Displays "Advanced Joystick Options" dialog box */
void
/* #AS#
   Nothing */
CJoystickDlg::
OnAdvSetting()
{
	CJoystickAdvDlg dlgJoystickAdv( &m_ulInputState, &m_nBlockOpposite, &m_nMultiJoy, this );

	dlgJoystickAdv.DoModal();

} /* #OF# CSettingsDlg::OnAdvSetting */

/*========================================================
Method   : CJoystickDlg::OnOK
=========================================================*/
/* #FN#
   Called when the user clicks the OK button */
void
/* #AS#
   Nothing */
CJoystickDlg::
OnOK()
{
	BOOL bChanged = FALSE;

	if( m_ulInputState != g_Input.ulState )
	{
		g_Input.ulState = m_ulInputState;
		WriteRegDWORD( NULL, REG_INPUT_STATE, g_Input.ulState );
	}
	if( m_nAutoMode != g_Input.Joy.nAutoMode )
	{
		g_Input.Joy.nAutoMode = m_nAutoMode;
		WriteRegDWORD( NULL, REG_AUTOFIRE_MODE, g_Input.Joy.nAutoMode );

		/* Update main window's indicator */
		UpdateIndicator( ID_INDICATOR_JOY );
	}
	if( m_ulAutoSticks != g_Input.Joy.ulAutoSticks )
	{
		g_Input.Joy.ulAutoSticks = m_ulAutoSticks;
		WriteRegDWORD( NULL, REG_AUTOFIRE_STICKS, g_Input.Joy.ulAutoSticks );
	}
	if( m_nBlockOpposite != INPUT_joy_block_opposite_directions )
	{
		INPUT_joy_block_opposite_directions = m_nBlockOpposite;
		WriteRegDWORD( NULL, REG_BLOCK_OPPOSITE, INPUT_joy_block_opposite_directions );
	}
	if( m_nMultiJoy != INPUT_joy_multijoy )
	{
		INPUT_joy_multijoy = m_nMultiJoy;
		WriteRegDWORD( NULL, REG_MULTI_JOY, INPUT_joy_multijoy );
	}

	for( int i = 0; i < MAX_ATARI_JOYPORTS; i++ )
	{
		if( m_anDevSelected[ i ] != g_Input.anDevSelected[ i ] )
			bChanged = TRUE;
	}
	if( bChanged )
	{
		g_Input.Joy.ulSelected  = (ULONG)((UBYTE)m_anDevSelected[ 3 ]) << 24;
		g_Input.Joy.ulSelected |= (ULONG)((UBYTE)m_anDevSelected[ 2 ]) << 16;
		g_Input.Joy.ulSelected |= (ULONG)((UBYTE)m_anDevSelected[ 1 ]) << 8;
		g_Input.Joy.ulSelected |= (ULONG)((UBYTE)m_anDevSelected[ 0 ]);

		WriteRegDWORD( NULL, REG_JOYSTICKS, g_Input.Joy.ulSelected );
	}
	Input_Initialise( bChanged );

	CCommonDlg::OnOK();

} /* #OF# CJoystickDlg::OnOK */
