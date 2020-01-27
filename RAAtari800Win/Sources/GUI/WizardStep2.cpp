/****************************************************************************
File    : WizardStep2.cpp
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# CWizardStep2 implementation file
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 22.09.2002
*/

#include "StdAfx.h"
#include "Atari800Win.h"
#include "Helpers.h"
#include "FileService.h"
#include "WizardDlg.h"
#include "RomImagesBase.h"
#include "WizardStep2.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define IDC_WIZARD_STEP2_FIRST		IDC_WIZARD_STEP2_OSA
#define IDC_WIZARD_STEP2_LAST		IDC_WIZARD_STEP2_INFO


/////////////////////////////////////////////////////////////////////////////
// CWizardStep2 dialog

BEGIN_MESSAGE_MAP(CWizardStep2, CWizardPage)
	//{{AFX_MSG_MAP(CWizardStep2)
	ON_BN_CLICKED(IDC_WIZARD_STEP2_OSA, OnOsa)
	ON_BN_CLICKED(IDC_WIZARD_STEP2_OSB, OnOsb)
	ON_BN_CLICKED(IDC_WIZARD_STEP2_XLXE, OnXlxe)
	ON_BN_CLICKED(IDC_WIZARD_STEP2_5200, On5200)
	ON_BN_CLICKED(IDC_WIZARD_STEP2_BASIC, OnBasic)
	ON_EN_KILLFOCUS(IDC_WIZARD_STEP2_EDITOSA, OnKillfocusEditOsa)
	ON_EN_KILLFOCUS(IDC_WIZARD_STEP2_EDITOSB, OnKillfocusEditOsb)
	ON_EN_KILLFOCUS(IDC_WIZARD_STEP2_EDITXLXE, OnKillfocusEditXlxe)
	ON_EN_KILLFOCUS(IDC_WIZARD_STEP2_EDIT5200, OnKillfocusEdit5200)
	ON_EN_KILLFOCUS(IDC_WIZARD_STEP2_EDITBASIC, OnKillfocusEditBasic)
	ON_BN_CLICKED(IDC_WIZARD_STEP2_SEARCH, OnSearch)
	ON_BN_CLICKED(IDC_WIZARD_STEP2_INFO, OnInfo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/*========================================================
Method   : CWizardStep2::CWizardStep2
=========================================================*/
/* #FN#
   Standard constructor */
CWizardStep2::
CWizardStep2(
	CWnd *pParent /*=NULL*/ /* #IN# Pointer to the parent window */
)
	: CWizardPage( CWizardStep2::IDD, pParent ),
	  CRomImagesBase()
{
	//{{AFX_DATA_INIT(CWizardStep2)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_nFirstCtrl = IDC_WIZARD_STEP2_FIRST;
	m_nLastCtrl  = IDC_WIZARD_STEP2_LAST;

} /* #OF# CWizardStep2::CWizardStep2 */

/*========================================================
Method   : CWizardStep2::DoDataExchange
=========================================================*/
/* #FN#
   Dynamic Data Exchange (not used) */
void
CWizardStep2::
DoDataExchange(
	CDataExchange *pDX /* #IN# Pointer to CDataExchange object */
)
{
	CWizardPage::DoDataExchange( pDX );
	CRomImagesBase::DataExchange( pDX );

	//{{AFX_DATA_MAP(CWizardStep2)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP

} /* #OF# CWizardStep2::DoDataExchange */


/////////////////////////////////////////////////////////////////////////////
// CWizardStep2 implementation

/*========================================================
Method   : CWizardStep2::SetDlgState
=========================================================*/
/* #FN#
   Sets up the state of the dialog controls */
void
/* #AS#
   Nothing */
CWizardStep2::
SetDlgState()
{
	/* Set OS ROM and BASIC path edit windows */
	for( int i = 0; i < ROM_TYPES_NO; i++ )
		SetDlgItemText( m_pRomData[ i ].nCtrlID, m_pRomData[ i ].szNewName );

} /* #OF# CWizardStep2::SetDlgState */

/*========================================================
Method   : CWizardStep2::Commit
=========================================================*/
/* #FN#
   Saves changes the user has made using the page */
void
/* #AS#
   Nothing */
CWizardStep2::
Commit()
{
	/* Check if OS ROMs were changed */
	for( int i = 0; i < ROM_TYPES_NO; i++ )
	{
		_NullPathToNone( m_pRomData[ i ].szNewName );
		if( _stricmp( m_pRomData[ i ].pszName, m_pRomData[ i ].szNewName ) != 0 )
		{
			/* If the ROM was changed for an active system */
			if( i == Atari800_machine_type || i == RTI_BAS )
			{
				int nCFOut;
				if( CheckFile( m_pRomData[ i ].pszName, &nCFOut ) !=
					CheckFile( m_pRomData[ i ].szNewName, &nCFOut ) )
				{
					/* If the user doesn't want to stop streaming, break the Commit operation */
					if( !StreamWarning( IDS_WARN_RECORD_SYSTEM, SRW_VIDEO_STREAM | SRW_SOUND_STREAM ) )
						return;

					m_bReboot = TRUE;
				}
			}
			/* The function "strncpy" has given strange effects here */
			strcpy( m_pRomData[ i ].pszName, m_pRomData[ i ].szNewName );
			WriteRegString( NULL, m_pRomData[ i ].pszRegName, m_pRomData[ i ].pszName );
		}
	}
} /* #OF# CWizardStep2::Commit */


/////////////////////////////////////////////////////////////////////////////
// CWizardStep2 message handlers

/*========================================================
Method   : CWizardStep2::OnInitDialog
=========================================================*/
/* #FN#
   Performs special processing when the dialog box is initialized */
BOOL
/* #AS#
   TRUE unless you set the focus to a control */
CWizardStep2::
OnInitDialog()
{
	CWizardPage::OnInitDialog();
	CRomImagesBase::InitDialog();

	const UINT anCtrls[ ROM_TYPES_NO ] =
	{
		IDC_WIZARD_STEP2_EDITOSA,
		IDC_WIZARD_STEP2_EDITOSB,
		IDC_WIZARD_STEP2_EDITXLXE,
		IDC_WIZARD_STEP2_EDIT5200,
		IDC_WIZARD_STEP2_EDITBASIC
	};

	for( int i = 0; i < ROM_TYPES_NO; i++ )
	{
		/* Set the appropriate controls IDs */
		m_pRomData[ i ].nCtrlID = anCtrls[ i ];
		/* Backup ROM paths */
		_strncpy( m_pRomData[ i ].szNewName, m_pRomData[ i ].pszName, MAX_PATH );
	}
	SetDlgState();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
} /* #OF# CWizardStep2::OnInitDialog */

/*========================================================
Method   : CWizardStep2::OnOsa
=========================================================*/
/* #FN#
   Allows selecting an Atari OS-A ROM image using FileDialog window */
void
/* #AS#
   Nothing */
CWizardStep2::
OnOsa()
{
	BrowseRomFile( RTI_OSA );

} /* #OF# CWizardStep2::OnOsa */

/*========================================================
Method   : CWizardStep2::OnOsb
=========================================================*/
/* #FN#
   Allows selecting an Atari OS-B ROM image using FileDialog window */
void
/* #AS#
   Nothing */
CWizardStep2::
OnOsb()
{
	BrowseRomFile( RTI_OSB );

} /* #OF# CWizardStep2::OnOsb */

/*========================================================
Method   : CWizardStep2::OnXlxe
=========================================================*/
/* #FN#
   Allows selecting an Atari XL/XE ROM image using FileDialog window */
void
/* #AS#
   Nothing */
CWizardStep2::
OnXlxe()
{
	BrowseRomFile( RTI_XLE );

} /* #OF# CWizardStep2::OnXlxe */

/*========================================================
Method   : CWizardStep2::On5200
=========================================================*/
/* #FN#
   Allows selecting an Atari 5200 ROM image using FileDialog window */
void
/* #AS#
   Nothing */
CWizardStep2::
On5200()
{
	BrowseRomFile( RTI_A52 );

} /* #OF# CWizardStep2::On5200 */

/*========================================================
Method   : CWizardStep2::OnBasic
=========================================================*/
/* #FN#
   Allows selecting a BASIC image using FileDialog window */
void
/* #AS#
   Nothing */
CWizardStep2::
OnBasic()
{
	BrowseRomFile( RTI_BAS );

} /* #OF# CWizardStep2::OnBasic */

/*========================================================
Method   : CWizardStep2::OnKillfocusEditOsa
=========================================================*/
/* #FN#
   The framework calls this function before an edit losing input focus */
void
/* #AS#
   Nothing */
CWizardStep2::
OnKillfocusEditOsa()
{
	KillfocusEditRom( RTI_OSA );

} /* #OF# CWizardStep2::OnKillfocusEditOsa */

/*========================================================
Method   : CWizardStep2::OnKillfocusEditOsb
=========================================================*/
/* #FN#
   The framework calls this function before an edit losing input focus */
void
/* #AS#
   Nothing */
CWizardStep2::
OnKillfocusEditOsb()
{
	KillfocusEditRom( RTI_OSB );

} /* #OF# CWizardStep2::OnKillfocusEditOsb */

/*========================================================
Method   : CWizardStep2::OnKillfocusEditXlxe
=========================================================*/
/* #FN#
   The framework calls this function before an edit losing input focus */
void
/* #AS#
   Nothing */
CWizardStep2::
OnKillfocusEditXlxe()
{
	KillfocusEditRom( RTI_XLE );

} /* #OF# CWizardStep2::OnKillfocusEditXlxe */

/*========================================================
Method   : CWizardStep2::OnKillfocusEdit5200
=========================================================*/
/* #FN#
   The framework calls this function before an edit losing input focus */
void
/* #AS#
   Nothing */
CWizardStep2::
OnKillfocusEdit5200()
{
	KillfocusEditRom( RTI_A52 );

} /* #OF# CWizardStep2::OnKillfocusEdit5200 */

/*========================================================
Method   : CWizardStep2::OnKillfocusEditBasic
=========================================================*/
/* #FN#
   The framework calls this function before an edit losing input focus */
void
/* #AS#
   Nothing */
CWizardStep2::
OnKillfocusEditBasic()
{
	KillfocusEditRom( RTI_BAS );

} /* #OF# CWizardStep2::OnKillfocusEditBasic */

/*========================================================
Method   : CWizardStep2::OnSearch
=========================================================*/
/* #FN#
   Searches Atari ROM images automatically */
void
/* #AS#
   Nothing */
CWizardStep2::
OnSearch()
{
	if( SearchRomImages() )
		SetDlgState();

} /* #OF# CWizardStep2::OnSearch */

/*========================================================
Method   : CWizardStep2::OnInfo
=========================================================*/
/* #FN#
   Displays information about the using Atari ROM images */
void
/* #AS#
   Nothing */
CWizardStep2::
OnInfo()
{
	DisplayRomInfo();

} /* #OF# CWizardStep2::OnInfo */
