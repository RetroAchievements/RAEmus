/****************************************************************************
File    : RomImagesDlg.cpp
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# CRomImagesDlg implementation file
@(#) #BY# Tomasz Szymankowski, Richard Lawrence
@(#) #LM# 27.09.2002
*/

#include "StdAfx.h"
#include "Atari800Win.h"
#include "Helpers.h"
#include "FileService.h"
#include "RomImagesBase.h"
#include "RomImagesDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define IDC_ROMIMAGES_FIRST		IDC_ROMIMAGES_OSA
#define IDC_ROMIMAGES_LAST		IDC_ROMIMAGES_CANCEL

#define CS_ATTACHED				0
#define CS_DETACHED				1


/////////////////////////////////////////////////////////////////////////////
// CRomImagesDlg dialog

BEGIN_MESSAGE_MAP(CRomImagesDlg, CCommonDlg)
	//{{AFX_MSG_MAP(CRomImagesDlg)
	ON_BN_CLICKED(IDC_ROMIMAGES_OSA, OnOsa)
	ON_BN_CLICKED(IDC_ROMIMAGES_OSB, OnOsb)
	ON_BN_CLICKED(IDC_ROMIMAGES_XLXE, OnXlxe)
	ON_BN_CLICKED(IDC_ROMIMAGES_5200, On5200)
	ON_BN_CLICKED(IDC_ROMIMAGES_BASIC, OnBasic)
	ON_BN_CLICKED(IDC_ROMIMAGES_CARTRIDGE, OnCartridge)
	ON_BN_CLICKED(IDC_ROMIMAGES_OSROM, OnOsRom)
	ON_EN_KILLFOCUS(IDC_ROMIMAGES_EDITOSA, OnKillfocusEditOsa)
	ON_EN_KILLFOCUS(IDC_ROMIMAGES_EDITOSB, OnKillfocusEditOsb)
	ON_EN_KILLFOCUS(IDC_ROMIMAGES_EDITXLXE, OnKillfocusEditXlxe)
	ON_EN_KILLFOCUS(IDC_ROMIMAGES_EDIT5200, OnKillfocusEdit5200)
	ON_EN_KILLFOCUS(IDC_ROMIMAGES_EDITBASIC, OnKillfocusEditBasic)
	ON_EN_KILLFOCUS(IDC_ROMIMAGES_EDITOSROM, OnKillfocusEditOsRom)
	ON_CBN_SELCHANGE(IDC_ROMIMAGES_MACHINETYPE, OnSelchangeMachineType)
	ON_BN_CLICKED(IDC_ROMIMAGES_SEARCH, OnSearch)
	ON_BN_CLICKED(IDC_ROMIMAGES_INFO, OnInfo)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_ROMIMAGES_OK, OnOK)
	ON_BN_CLICKED(IDC_ROMIMAGES_CANCEL, CCommonDlg::OnCancel)
END_MESSAGE_MAP()

/*========================================================
Method   : CRomImagesDlg::CRomImagesDlg
=========================================================*/
/* #FN#
   Standard constructor */
CRomImagesDlg::
CRomImagesDlg(
	CWnd *pParent /*=NULL*/ /* #IN# Pointer to the parent window */
)
	: CCommonDlg( (g_Screen.ulState & SM_ATTR_SMALL_DLG ? IDD_ROMIMAGES_SMALL : CRomImagesDlg::IDD), pParent ),
	  CRomImagesBase()
{
	//{{AFX_DATA_INIT(CRomImagesDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_bExitPass  = FALSE;
	m_eReboot    = RBT_NONE;
	m_nFirstCtrl = IDC_ROMIMAGES_FIRST;
	m_nLastCtrl  = IDC_ROMIMAGES_LAST;

} /* #OF# CRomImagesDlg::CRomImagesDlg */

/*========================================================
Method   : CRomImagesDlg::DoDataExchange
=========================================================*/
/* #FN#
   Dynamic Data Exchange (not used) */
void
/* #AS#
   Nothing */
CRomImagesDlg::
DoDataExchange(
	CDataExchange *pDX /* #IN# Pointer to CDataExchange object */
)
{
	CCommonDlg::DoDataExchange( pDX );
	CRomImagesBase::DataExchange( pDX );
	//{{AFX_DATA_MAP(CRomImagesDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP

} /* #OF# CRomImagesDlg::DoDataExchange */


/////////////////////////////////////////////////////////////////////////////
// CRomImagesDlg implementation

/*========================================================
Method   : CRomImagesDlg::SetDlgState
=========================================================*/
/* #FN#
   Sets up the state of the dialog controls */
void
/* #AS#
   Nothing */
CRomImagesDlg::
SetDlgState()
{
	if( !m_bStateInit )
	{
		/* Set an appropriate combo item */
		_SetSelCbox( IDC_ROMIMAGES_CARTRIDGESTATE, m_nCartState );
	}
	if( m_bSmallMode )
	{
		RomType rtType = RTI_XLE;
		/* Get the default system OS type */
		switch( Atari800_machine_type )
		{
			case Atari800_MACHINE_OSA:
				rtType = RTI_OSA;
				break;
			case Atari800_MACHINE_OSB:
				rtType = RTI_OSB;
				break;
			case Atari800_MACHINE_5200:
				rtType = RTI_A52;
				break;
			default:
				break;
		}
		_SetSelCbox( IDC_ROMIMAGES_MACHINETYPE, rtType );
		SetDlgItemText( m_pRomData[ rtType ].nCtrlID, m_pRomData[ rtType ].szNewName );
		SetDlgItemText( m_pRomData[ RTI_BAS ].nCtrlID, m_pRomData[ RTI_BAS ].szNewName );
	}
	else
	{
		/* Set OS ROM and BASIC path edit windows */
		for( int i = 0; i < ROM_TYPES_NO; i++ )
		{
			SetDlgItemText( m_pRomData[ i ].nCtrlID, m_pRomData[ i ].szNewName );
		}
	}
	/* Set cartridge path edit window */
	SetDlgItemText( IDC_ROMIMAGES_EDITCARTRIDGE, m_szCartName );

#ifdef WIN_NETWORK_GAMES
	if( ST_KAILLERA_ACTIVE )
	{
		_EnableCtrl( IDC_ROMIMAGES_CARTRIDGESTATE, FALSE );
		_RdOnlyEdit( IDC_ROMIMAGES_EDITBASIC,      TRUE  );
		_RdOnlyEdit( IDC_ROMIMAGES_EDITCARTRIDGE,  TRUE  );
		_EnableCtrl( IDC_ROMIMAGES_BASIC,          FALSE );
		_EnableCtrl( IDC_ROMIMAGES_CARTRIDGE,      FALSE );
		_EnableCtrl( IDC_ROMIMAGES_SEARCH,         FALSE );

		if( m_bSmallMode )
		{
			_RdOnlyEdit( IDC_ROMIMAGES_EDITOSROM,  TRUE  );
			_EnableCtrl( IDC_ROMIMAGES_OSROM,      FALSE );
		}
		else
		{
			_RdOnlyEdit( IDC_ROMIMAGES_EDITOSA,    TRUE  );
			_RdOnlyEdit( IDC_ROMIMAGES_EDITOSB,    TRUE  );
			_RdOnlyEdit( IDC_ROMIMAGES_EDITXLXE,   TRUE  );
			_RdOnlyEdit( IDC_ROMIMAGES_EDIT5200,   TRUE  );
			_EnableCtrl( IDC_ROMIMAGES_OSA,        FALSE );
			_EnableCtrl( IDC_ROMIMAGES_OSB,        FALSE );
			_EnableCtrl( IDC_ROMIMAGES_XLXE,       FALSE );
			_EnableCtrl( IDC_ROMIMAGES_5200,       FALSE );
		}
	}
#endif
	m_bStateInit = TRUE;

} /* #OF# CRomImagesDlg::SetDlgState */


/////////////////////////////////////////////////////////////////////////////
// CRomImagesDlg message handlers

/*========================================================
Method   : CRomImagesDlg::OnInitDialog
=========================================================*/
/* #FN#
   Performs special processing when the dialog box is initialized */
BOOL
/* #AS#
   TRUE unless you set the focus to a control */
CRomImagesDlg::
OnInitDialog()
{
	CCommonDlg::OnInitDialog();
	CRomImagesBase::InitDialog();

	const UINT anCtrlsLarge[ ROM_TYPES_NO ] =
	{
		IDC_ROMIMAGES_EDITOSA,
		IDC_ROMIMAGES_EDITOSB,
		IDC_ROMIMAGES_EDITXLXE,
		IDC_ROMIMAGES_EDIT5200,
		IDC_ROMIMAGES_EDITBASIC
	};

	const UINT anCtrlsSmall[ ROM_TYPES_NO ] =
	{
		IDC_ROMIMAGES_EDITOSROM,
		IDC_ROMIMAGES_EDITOSROM,
		IDC_ROMIMAGES_EDITOSROM,
		IDC_ROMIMAGES_EDITOSROM,
		IDC_ROMIMAGES_EDITBASIC
	};

	m_bSmallMode = g_Screen.ulState & SM_ATTR_SMALL_DLG;

	for( int i = 0; i < ROM_TYPES_NO; i++ )
	{
		/* Set the appropriate controls IDs */
		m_pRomData[ i ].nCtrlID = (m_bSmallMode ? anCtrlsSmall[ i ] : anCtrlsLarge[ i ]);
		/* Backup ROM paths */
		_strncpy( m_pRomData[ i ].szNewName, m_pRomData[ i ].pszName, MAX_PATH );
	}
	/* Backup cart path */
	_strncpy( m_szCartName, g_szOtherRom, MAX_PATH );
	/* Remember the cartridge state */
	m_nCartState = (CARTRIDGE_NONE == CARTRIDGE_type ? CS_DETACHED : CS_ATTACHED);

	SetDlgState();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
} /* #OF# CRomImagesDlg::OnInitDialog */

/*========================================================
Method   : CRomImagesDlg::OnSelchangeMachineType
=========================================================*/
/* #FN#
	Sets a state of the object regarding to an appropriate combo box */
void
/* #AS#
   Nothing */
CRomImagesDlg::
OnSelchangeMachineType()
{
	int nType = _GetSelCbox( IDC_ROMIMAGES_MACHINETYPE );

	SetDlgItemText( m_pRomData[ nType ].nCtrlID, m_pRomData[ nType ].szNewName );

} /* #OF# CRomImagesDlg::OnSelchangeMachineType */

/*========================================================
Method   : CRomImagesDlg::OnOsa
=========================================================*/
/* #FN#
   Allows selecting an Atari OS-A ROM image using FileDialog window */
void
/* #AS#
   Nothing */
CRomImagesDlg::
OnOsa() 
{
	BrowseRomFile( RTI_OSA );

} /* #OF# CRomImagesDlg:: */

/*========================================================
Method   : CRomImagesDlg::OnOsb
=========================================================*/
/* #FN#
   Allows selecting an Atari OS-B ROM image using FileDialog window */
void
/* #AS#
   Nothing */
CRomImagesDlg::
OnOsb()
{
	BrowseRomFile( RTI_OSB );

} /* #OF# CRomImagesDlg:: */

/*========================================================
Method   : CRomImagesDlg::OnXlxe
=========================================================*/
/* #FN#
   Allows selecting an Atari XL/XE ROM image using FileDialog window */
void
/* #AS#
   Nothing */
CRomImagesDlg::
OnXlxe()
{
	BrowseRomFile( RTI_XLE );

} /* #OF# CRomImagesDlg:: */

/*========================================================
Method   : CRomImagesDlg::On5200
=========================================================*/
/* #FN#
   Allows selecting an Atari 5200 ROM image using FileDialog window */
void
/* #AS#
   Nothing */
CRomImagesDlg::
On5200()
{
	BrowseRomFile( RTI_A52 );

} /* #OF# CRomImagesDlg:: */

/*========================================================
Method   : CRomImagesDlg::OnBasic
=========================================================*/
/* #FN#
   Allows selecting a BASIC image using FileDialog window */
void
/* #AS#
   Nothing */
CRomImagesDlg::
OnBasic()
{
	BrowseRomFile( RTI_BAS );

} /* #OF# CRomImagesDlg::OnBasic */

/*========================================================
Method   : CRomImagesDlg::OnCartridge
=========================================================*/
/* #FN#
   Allows selecting and attaching a cartridge image using FileDialog window */
void
/* #AS#
   Nothing */
CRomImagesDlg::
OnCartridge()
{
	char szNewCart[ MAX_PATH + 1 ];

	GetDlgItemText( IDC_ROMIMAGES_EDITCARTRIDGE, szNewCart, MAX_PATH );

	if( PickCartridge( szNewCart, this ) )
	{
		_NullPathToNone( szNewCart );
		/* If the names and cart types are equal then don't process */
		if( _stricmp( m_szCartName, szNewCart ) != 0 )
		{
			strcpy( m_szCartName, szNewCart );
			/* Apply changes to dialog window */
			SetDlgItemText( IDC_ROMIMAGES_EDITCARTRIDGE, m_szCartName );
		}
	}
} /* #OF# CRomImagesDlg::OnCartridge */

/*========================================================
Method   : CRomImagesDlg::OnOsRom
=========================================================*/
/* #FN#
   Allows selecting an Atari ROM image using FileDialog window.
   Small window mode */
void
/* #AS#
   Nothing */
CRomImagesDlg::
OnOsRom()
{
	BrowseRomFile( (RomType)_GetSelCbox( IDC_ROMIMAGES_MACHINETYPE ) );

} /* #OF# CRomImagesDlg::OnOsRom */

/*========================================================
Method   : CRomImagesDlg::OnKillfocusEditOsa
=========================================================*/
/* #FN#
   The framework calls this function before an edit losing input focus */
void
/* #AS#
   Nothing */
CRomImagesDlg::
OnKillfocusEditOsa()
{
	if( !m_bExitPass )
		KillfocusEditRom( RTI_OSA );

} /* #OF# CRomImagesDlg::OnKillfocusEditOsa */

/*========================================================
Method   : CRomImagesDlg::OnKillfocusEditOsb
=========================================================*/
/* #FN#
   The framework calls this function before an edit losing input focus */
void
/* #AS#
   Nothing */
CRomImagesDlg::
OnKillfocusEditOsb()
{
	if( !m_bExitPass )
		KillfocusEditRom( RTI_OSB );

} /* #OF# CRomImagesDlg::OnKillfocusEditOsb */

/*========================================================
Method   : CRomImagesDlg::OnKillfocusEditXlxe
=========================================================*/
/* #FN#
   The framework calls this function before an edit losing input focus */
void
/* #AS#
   Nothing */
CRomImagesDlg::
OnKillfocusEditXlxe()
{
	if( !m_bExitPass )
		KillfocusEditRom( RTI_XLE );

} /* #OF# CRomImagesDlg::OnKillfocusEditXlxe */

/*========================================================
Method   : CRomImagesDlg::OnKillfocusEdit5200
=========================================================*/
/* #FN#
   The framework calls this function before an edit losing input focus */
void
/* #AS#
   Nothing */
CRomImagesDlg::
OnKillfocusEdit5200()
{
	if( !m_bExitPass )
		KillfocusEditRom( RTI_A52 );

} /* #OF# CRomImagesDlg::OnKillfocusEdit5200 */

/*========================================================
Method   : CRomImagesDlg::OnKillfocusEditBasic
=========================================================*/
/* #FN#
   The framework calls this function before an edit losing input focus */
void
/* #AS#
   Nothing */
CRomImagesDlg::
OnKillfocusEditBasic()
{
	if( !m_bExitPass )
		KillfocusEditRom( RTI_BAS );

} /* #OF# CRomImagesDlg::OnKillfocusEditBasic */

/*========================================================
Method   : CRomImagesDlg::OnKillfocusEditOsRom
=========================================================*/
/* #FN#
   The framework calls this function before an edit losing input focus */
void
/* #AS#
   Nothing */
CRomImagesDlg::
OnKillfocusEditOsRom()
{
	if( !m_bExitPass )
		KillfocusEditRom( (RomType)_GetSelCbox( IDC_ROMIMAGES_MACHINETYPE ) );

} /* #OF# CRomImagesDlg::OnKillfocusEditOsRom */

/*========================================================
Method   : CRomImagesDlg::OnSearch
=========================================================*/
/* #FN#
   Searches Atari ROM images automatically */
void
/* #AS#
   Nothing */
CRomImagesDlg::
OnSearch()
{
	if( SearchRomImages() )
		SetDlgState();

} /* #OF# CRomImagesDlg::OnSearch */

/*========================================================
Method   : CRomImagesDlg::OnInfo
=========================================================*/
/* #FN#
   Displays information about the using Atari ROM images */
void
/* #AS#
   Nothing */
CRomImagesDlg::
OnInfo()
{
	DisplayRomInfo();

} /* #OF# CRomImagesDlg::OnInfo */

/*========================================================
Method   : CRomImagesDlg::ReceiveFocused
=========================================================*/
/* #FN#
   Receives the edit controls content again. The user could press
   'Enter' or 'Alt-O' and then all changes he's made in the last
   edited control would be lost. */
void
/* #AS#
   Nothing */
CRomImagesDlg::
ReceiveFocused()
{
	CWnd *pWnd    = GetFocus();
	UINT  nCtrlID = pWnd ? pWnd->GetDlgCtrlID() : 0;

	if( m_bSmallMode )
	{
		if( IDC_ROMIMAGES_EDITOSROM == nCtrlID )
		{
			KillfocusEditRom( (RomType)_GetSelCbox( IDC_ROMIMAGES_MACHINETYPE ) );
		}
		else
		if( IDC_ROMIMAGES_EDITBASIC == nCtrlID )
			KillfocusEditRom( RTI_BAS );
	}
	else
	for( int i = 0; i < ROM_TYPES_NO; i++ )
	{
		if( m_pRomData[ i ].nCtrlID == nCtrlID )
		{
			KillfocusEditRom( (RomType)i );
			break;
		}
	}
} /* #OF# CRomImagesDlg::ReceiveFocused */

/*========================================================
Method   : CRomImagesDlg::OnOK
=========================================================*/
/* #FN#
   Called when the user clicks the OK button */
void
/* #AS#
   Nothing */
CRomImagesDlg::
OnOK()
{
	int nNewCartState = m_nCartState;

	/* There is a problem with messages routing when the dialog is
	   closed with Enter/Alt-O key. KILLFOCUS message arrives
       to late and we have to invoke KillFocus handlers in OnOK
       method by ourselves. That's why we use this member. */
	m_bExitPass = TRUE;

	/* Unfortunately, edit controls do not lose the focus before
	   handling this when the user uses accelerators */
	ReceiveFocused();

	/* Get cartridge state the user has set */
	if( CB_ERR == (nNewCartState = _GetSelCbox( IDC_ROMIMAGES_CARTRIDGESTATE )) )
	{
		_SetSelCbox( IDC_ROMIMAGES_CARTRIDGESTATE, CS_DETACHED );
		nNewCartState = CS_DETACHED;
	}

	/* Check if OS ROMs were changed */
	for( int i = 0; i < ROM_TYPES_NO; i++ )
	{
		_NullPathToNone( m_pRomData[ i ].szNewName );
		if( _stricmp( m_pRomData[ i ].pszName, m_pRomData[ i ].szNewName ) != 0 )
		{
			/* If the ROM was changed for an active system */
			if( i == Atari800_machine_type ||
			    i == RTI_BAS && Atari800_machine_type != Atari800_MACHINE_5200 )
			{
				int nCFOut;
				if( CheckFile( m_pRomData[ i ].pszName, &nCFOut ) !=
					CheckFile( m_pRomData[ i ].szNewName, &nCFOut ) )
				{
					/* If the user doesn't want to stop streaming, cancel the OK operation */
					if( !StreamWarning( IDS_WARN_RECORD_SYSTEM, SRW_VIDEO_STREAM | SRW_SOUND_STREAM ) )
					{
						m_bExitPass = FALSE;
						return;
					}
					m_eReboot = RBT_RESTART;
				}
			}
			/* The function "strncpy" gives strange effects here */
			strcpy( m_pRomData[ i ].pszName, m_pRomData[ i ].szNewName );
			WriteRegString( NULL, m_pRomData[ i ].pszRegName, m_pRomData[ i ].pszName );
		}
	}
	/* Check if cartridge ROM was changed */
	/* This is the simplest way to resolve the KillFocus handling problems :) */
	GetDlgItemText( IDC_ROMIMAGES_EDITCARTRIDGE, m_szCartName, MAX_PATH );

	_NullPathToNone( m_szCartName );
	if( _stricmp( m_szCartName, g_szOtherRom ) != 0 )
	{
		strcpy( g_szOtherRom, m_szCartName );
		WriteRegString( NULL, REG_ROM_OTHER, g_szOtherRom );

		if( CS_ATTACHED == nNewCartState )
			m_eReboot = RBT_COLDSTART;
	}

	if( RBT_NONE != m_eReboot || nNewCartState != m_nCartState )
	{
		if( CS_ATTACHED == nNewCartState )
		{
			/* Attach the selected cartridge */
			AttachCartridge( g_szOtherRom, CARTRIDGE_NONE );
		}
		else
		{
			ASSERT(CS_DETACHED == nNewCartState);

			CARTRIDGE_Remove();
			strcpy( g_szCurrentRom, FILE_NONE );
		}
		/* Update the registry with new cartridge's state */
		WriteRegString( NULL, REG_ROM_CURRENT, g_szCurrentRom );
		WriteRegDWORD ( NULL, REG_CART_TYPE,   CARTRIDGE_type );

		if( RBT_RESTART != m_eReboot )
			m_eReboot = RBT_COLDSTART;
	}
	CCommonDlg::OnOK();

} /* #OF# CRomImagesDlg::OnOK */
