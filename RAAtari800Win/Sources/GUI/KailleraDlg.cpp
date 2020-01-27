/****************************************************************************
File    : KailleraDlg.cpp
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# CKailleraDlg implementation file
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 30.09.2002
*/

#include "StdAfx.h"

#ifdef WIN_NETWORK_GAMES

#include "Atari800Win.h"
#include "Helpers.h"
#include "FileService.h"
#include "KailleraDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define IDC_KAILLERA_FIRST		IDC_KAILLERA_GAMENAME
#define IDC_KAILLERA_LAST		IDC_KAILLERA_CANCEL

#define MIN_FRAME_SKIP			1
#define MAX_FRAME_SKIP			3

#define MAX_BOOT_IMAGES_NO		5


/////////////////////////////////////////////////////////////////////////////
// CKailleraDlg dialog

BEGIN_MESSAGE_MAP(CKailleraDlg, CCommonDlg)
	//{{AFX_MSG_MAP(CKailleraDlg)
	ON_EN_KILLFOCUS(IDC_KAILLERA_GAMENAME, OnKillfocusGameName)
	ON_CBN_SELCHANGE(IDC_KAILLERA_BOOTIMAGE, OnSelchangeBootImage)
	ON_CBN_SELCHANGE(IDC_KAILLERA_PORT, OnSelchangePort)
	ON_BN_CLICKED(IDC_KAILLERA_SKIPFRAMECHECK, OnSkipFrameCheck)
	ON_EN_KILLFOCUS(IDC_KAILLERA_SKIPFRAMEVALUE, OnKillfocusSkipFrameValue)
	ON_NOTIFY(UDN_DELTAPOS, IDC_KAILLERA_SKIPFRAMESPIN, OnDeltaposSkipFrameSpin)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_KAILLERA_OK, OnOK)
	ON_BN_CLICKED(IDC_KAILLERA_CANCEL, CCommonDlg::OnCancel)
END_MESSAGE_MAP()

/*========================================================
Method   : CKailleraDlg::CKailleraDlg
=========================================================*/
/* #FN#
   Standard constructor */
CKailleraDlg::
CKailleraDlg(
	CWnd *pParent /*=NULL*/ /* #IN# Pointer to the parent window */
)
	: CCommonDlg( CKailleraDlg::IDD, pParent )
{
	//{{AFX_DATA_INIT(CKailleraDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_unBootImage = 0;
	m_nFirstCtrl  = IDC_KAILLERA_FIRST;
	m_nLastCtrl   = IDC_KAILLERA_LAST;

} /* #OF# CKailleraDlg::CKailleraDlg */

/*========================================================
Method   : CKailleraDlg::DoDataExchange
=========================================================*/
/* #FN#
   Dynamic Data Exchange (not used) */
void
/* #AS#
   Nothing */
CKailleraDlg::
DoDataExchange(
	CDataExchange *pDX /* #IN# Pointer to CDataExchange object */
)
{
	CCommonDlg::DoDataExchange( pDX );
	//{{AFX_DATA_MAP(CKailleraDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP

} /* #OF# CKailleraDlg::DoDataExchange */


/////////////////////////////////////////////////////////////////////////////
// CKailleraDlg implementation

/*========================================================
Method   : CKailleraDlg::SetDlgState
=========================================================*/
/* #FN#
   Sets up the state of the dialog controls */
void
/* #AS#
   Nothing */
CKailleraDlg::
SetDlgState()
{
	int i;
	CComboBox *pCombo = (CComboBox *)GetDlgItem( IDC_KAILLERA_BOOTIMAGE );
	ASSERT(NULL != pCombo);
	ASSERT(pCombo->GetCount() <= MAX_BOOT_IMAGES_NO);

	/* Name of the network game */
	SetNetworkGameName( m_unBootImage ); /* Display name of the selected image */
	SetDlgItemText( IDC_KAILLERA_GAMENAME, m_szGameName );

	_RdOnlyEdit( IDC_KAILLERA_GAMENAME, TRUE );

	/* Booting device */
	for( i = 0; i < pCombo->GetCount(); i++ )
		if( m_unBootImage == pCombo->GetItemData( i ) )
		{
			pCombo->SetCurSel( i );
			break;
		}
	ASSERT(i < pCombo->GetCount());
	pCombo->EnableWindow( pCombo->GetCount() > 1 );

	/* Skip frame */
	_EnableCtrl( IDC_KAILLERA_SKIPFRAMECHECK, m_bImageAttached );
	_SetChkBttn( IDC_KAILLERA_SKIPFRAMECHECK, _IsFlagSet( m_ulKailleraState, KS_SKIP_FRAMES ) );
	_EnableCtrl( IDC_KAILLERA_SKIPFRAMEVALUE, _IsFlagSet( m_ulKailleraState, KS_SKIP_FRAMES ) && m_bImageAttached );
	_EnableCtrl( IDC_KAILLERA_SKIPFRAMESPIN,  _IsFlagSet( m_ulKailleraState, KS_SKIP_FRAMES ) );

	SetDlgItemInt( IDC_KAILLERA_SKIPFRAMEVALUE, m_nFrameSkip, FALSE );

	/* Local port */
	_SetSelCbox( IDC_KAILLERA_PORT, m_nLocalPort );
	_EnableCtrl( IDC_KAILLERA_PORT, m_bImageAttached );

	if(	NO_JOYSTICK != g_Input.anDevSelected[ m_nLocalPort ] )
	{
		_SetSelCbox( IDC_KAILLERA_STICK, g_Input.anDevSelected[ m_nLocalPort ] + NUM_KBJOY_DEVICES );
	}
	else
		_SetSelCbox( IDC_KAILLERA_STICK, g_Input.nDevFoundNum + NUM_KBJOY_DEVICES );

	/* Connect button */
	_EnableCtrl( IDC_KAILLERA_OK, m_bImageAttached );

} /* #OF# CKailleraDlg::SetDlgState */


/////////////////////////////////////////////////////////////////////////////
// CKailleraDlg message handlers

/*========================================================
Method   : CKailleraDlg::OnInitDialog
=========================================================*/
/* #FN#
   Performs special processing when the dialog box is initialized */
BOOL
/* #AS#
   TRUE unless you set the focus to a control */
CKailleraDlg::
OnInitDialog()
{
	CCommonDlg::OnInitDialog();

	char szItem[ LOADSTRING_SIZE_S + 1 ];
	UINT unFileType = IAF_ATARI_FILE;
	int  nIndex     = 0;

	CComboBox *pCombo = NULL;

	m_ulKailleraState = g_Kaillera.ulState;
	m_nLocalPort      = g_Kaillera.nLocalPort;
	m_nFrameSkip      = g_Kaillera.nFrameSkip;

	/* Set default name of the network game */
	SetNetworkGameName();

	pCombo = (CComboBox *)GetDlgItem( IDC_KAILLERA_STICK );
	ASSERT(NULL != pCombo);

	pCombo->AddString( _LoadStringSx( IDS_JOY_NUMPAD,   szItem ) );
	pCombo->AddString( _LoadStringSx( IDS_JOY_ARROWS,   szItem ) );
	pCombo->AddString( _LoadStringSx( IDS_JOY_KEYSET_A, szItem ) );
	pCombo->AddString( _LoadStringSx( IDS_JOY_KEYSET_B, szItem ) );

	/* Available input devices */
	for( int i = 0; i < g_Input.nDevFoundNum; i++ )
		pCombo->AddString( g_Input.acDevNames[ i ] );

	pCombo->AddString( _LoadStringSx( IDS_JOY_NONE, szItem ) );

	pCombo = (CComboBox *)GetDlgItem( IDC_KAILLERA_BOOTIMAGE );
	ASSERT(NULL != pCombo);

	if( GetBootFileInfo( NULL, 0, &unFileType ) )
	{
		DWORD adwData[ MAX_BOOT_IMAGES_NO ][ 2 ] =
		{
			{ IAF_A8S_IMAGE, IDS_IMAGE_A8S },
			{ IAF_ROM_IMAGE, IDS_IMAGE_ROM },
			{ IAF_BIN_IMAGE, IDS_IMAGE_BIN },
			{ IAF_DSK_IMAGE, IDS_IMAGE_DSK },
			{ IAF_CAS_IMAGE, IDS_IMAGE_CAS }
		};
		for( int i = 0; i < MAX_BOOT_IMAGES_NO; i++ )
		{
			if( 0 != (unFileType & adwData[ i ][ 0 ]) )
			{
				nIndex = pCombo->AddString( _LoadStringSx( adwData[ i ][ 1 ], szItem ) );
				pCombo->SetItemData( nIndex, adwData[ i ][ 0 ] );
			}
		}
	}
	else
	{
		nIndex = pCombo->AddString( _LoadStringSx( IDS_IMAGE_NONE, szItem ) );
		pCombo->SetItemData( nIndex, 0 );
	}	
	SetDlgState();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
} /* #OF# CKailleraDlg::OnInitDialog */

/*========================================================
Method   : CKailleraDlg::SetNetworkGameName
=========================================================*/
/* #FN#
   Establishes name of the network game */
BOOL
/* #AS#
   TRUE if the game name has been determined, otherwise FALSE */
CKailleraDlg::
SetNetworkGameName(
	UINT unBootImage /*=IAF_ATARI_FILE*/
)
{
	const int nBufferLen = max(MAX_PATH, FILENAME_MAX);
	char szFilePath[ nBufferLen + 1 ];
	UINT unFileType = unBootImage;

	m_bImageAttached = GetBootFileInfo( szFilePath, nBufferLen, &unFileType );
	m_unBootImage = 0;

	if( m_bImageAttached )
	{
		char szTemp[ _MAX_PATH  + 1 ];
		char szFile[ _MAX_FNAME + 1 ];

		/* Split disk full path */
		_tsplitpath( szFilePath, szTemp, szTemp, szFile, szTemp );
		_strncpy( m_szGameName, szFile, GAME_NAME_LENGTH );

		/* Store the booting image type */
		m_unBootImage = unFileType;
	}
	else
		LoadStringEx( IDS_KAILLERA_ATTACH_IMAGE, m_szGameName, GAME_NAME_LENGTH );

	return m_bImageAttached;

} /* #OF# CKailleraDlg::SetNetworkGameName */

/*========================================================
Method   : CKailleraDlg::OnKillfocusGameName
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate edit control */
void
/* #AS#
   Nothing */
CKailleraDlg::
OnKillfocusGameName()
{
	GetDlgItemText( IDC_KAILLERA_GAMENAME, m_szGameName, GAME_NAME_LENGTH );

} /* #OF# CKailleraDlg::OnKillfocusGameName */

/*========================================================
Method   : CKailleraDlg::OnSelchangeBootImage
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate combo box */
void
/* #AS#
   Nothing */
CKailleraDlg::
OnSelchangeBootImage()
{
	CComboBox *pCombo = (CComboBox *)GetDlgItem( IDC_KAILLERA_BOOTIMAGE );
	ASSERT(NULL != pCombo);
	int nIndex = pCombo->GetCurSel();

	if( CB_ERR != nIndex )
	{
		m_unBootImage = pCombo->GetItemData( nIndex );
	}
	else
	{
		pCombo->SetCurSel( 0 );
		m_unBootImage = pCombo->GetItemData( 0 );
	}
	SetDlgState();

} /* #OF# CKailleraDlg::OnSelchangeBootImage */

/*========================================================
Method   : CKailleraDlg::OnSelchangePort
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate combo box */
void
/* #AS#
   Nothing */
CKailleraDlg::
OnSelchangePort()
{
	if( CB_ERR == (m_nLocalPort = _GetSelCbox( IDC_KAILLERA_PORT )) )
	{
		_SetSelCbox( IDC_KAILLERA_PORT, 0 );
		m_nLocalPort = 0;
	}
	SetDlgState();

} /* #OF# CKailleraDlg::OnSelchangePort */

/*========================================================
Method   : CKailleraDlg::OnSkipFrameCheck
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate check box */
void
/* #AS#
   Nothing */
CKailleraDlg::
OnSkipFrameCheck()
{
	_ClickButton( IDC_KAILLERA_SKIPFRAMECHECK, m_ulKailleraState, KS_SKIP_FRAMES );
	SetDlgState();

} /* #OF# CKailleraDlg::OnSkipFrameCheck */

/*========================================================
Method   : CKailleraDlg::OnDeltaposSkipFrameSpin
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate spin control */
void
/* #AS#
   Nothing */
CKailleraDlg::
OnDeltaposSkipFrameSpin(
	NMHDR   *pNMHDR, /* #IN#  */
	LRESULT *pResult /* #OUT# */
)
{
	_DeltaposSpin( pNMHDR, IDC_KAILLERA_SKIPFRAMEVALUE, m_nFrameSkip, MIN_FRAME_SKIP, MAX_FRAME_SKIP );
	SetDlgState();

	*pResult = 0;

} /* #OF# CKailleraDlg::OnDeltaposSkipFrameSpin */

/*========================================================
Method   : CKailleraDlg::OnKillfocusSkipFrameValue
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate edit control */
void
/* #AS#
   Nothing */
CKailleraDlg::
OnKillfocusSkipFrameValue()
{
	_KillfocusSpin( IDC_KAILLERA_SKIPFRAMEVALUE, m_nFrameSkip, MIN_FRAME_SKIP, MAX_FRAME_SKIP );
	SetDlgState();

} /* #OF# CKailleraDlg::OnKillfocusSkipFrameValue */

/*========================================================
Method   : CKailleraDlg::ReceiveFocused
=========================================================*/
/* #FN#
   Receives the edit controls content again. The user could press
   'Enter' or 'Alt-O' and then all changes he's made in the last
   edited control would be lost. */
void
/* #AS#
   Nothing */
CKailleraDlg::
ReceiveFocused()
{
	CWnd *pWnd    = GetFocus();
	UINT  nCtrlID = pWnd ? pWnd->GetDlgCtrlID() : 0;

	switch( nCtrlID )
	{
		case IDC_KAILLERA_GAMENAME:
			OnKillfocusGameName();
			break;
		case IDC_KAILLERA_SKIPFRAMEVALUE:
			OnKillfocusSkipFrameValue();
			break;
	}
} /* #OF# CKailleraDlg::ReceiveFocused */

/*========================================================
Method   : CKailleraDlg::OnOK
=========================================================*/
/* #FN#
   Called when the user clicks the OK button */
void
/* #AS#
   Nothing */
CKailleraDlg::
OnOK()
{
	/* Unfortunately, edit controls do not lose the focus before
	   handling this when the user uses accelerators */
	ReceiveFocused();

	if( m_ulKailleraState != g_Kaillera.ulState )
	{
		g_Kaillera.ulState = m_ulKailleraState;
		WriteRegDWORD( NULL, REG_KAILLERA_STATE, g_Kaillera.ulState );
	}
	if( m_nLocalPort != g_Kaillera.nLocalPort )
	{
		g_Kaillera.nLocalPort = m_nLocalPort;
		WriteRegDWORD( NULL, REG_KAILLERA_LOCAL_PORT, g_Kaillera.nLocalPort );
	}
	if( m_nFrameSkip != g_Kaillera.nFrameSkip )
	{
		g_Kaillera.nFrameSkip = m_nFrameSkip;
		WriteRegDWORD( NULL, REG_KAILLERA_FRAME_SKIP, g_Kaillera.nFrameSkip );
	}
	g_Kaillera.unBootImage = m_unBootImage;

	_strncpy( g_Kaillera.szGameName, m_szGameName, GAME_NAME_LENGTH );
	g_Kaillera.szGameName[ strlen( g_Kaillera.szGameName ) + 1 ] = '\0';

	CCommonDlg::OnOK();

} /* #OF# CKailleraDlg::OnOK */

#endif /*WIN_NETWORK_GAMES*/
