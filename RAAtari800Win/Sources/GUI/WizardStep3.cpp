/****************************************************************************
File    : WizardStep3.cpp
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# CWizardStep3 implementation file
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 30.09.2002
*/

#include "StdAfx.h"
#include "Atari800Win.h"
#include "WizardDlg.h"
#include "FileAssociationsBase.h"
#include "WizardStep3.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define IDC_WIZARD_STEP3_FIRST		IDC_WIZARD_STEP3_ATR
#define IDC_WIZARD_STEP3_LAST		IDC_WIZARD_STEP3_REMOVEALL


/////////////////////////////////////////////////////////////////////////////
// CWizardStep3 dialog

BEGIN_MESSAGE_MAP(CWizardStep3, CWizardPage)
	//{{AFX_MSG_MAP(CWizardStep3)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/*========================================================
Method   : CWizardStep3::CWizardStep3
=========================================================*/
/* #FN#
   Standard constructor */
CWizardStep3::
CWizardStep3(
	CWnd *pParent /*=NULL*/ /* #IN# Pointer to the parent window */
)
	: CWizardPage( CWizardStep3::IDD, pParent ),
	  CFileAssociationsBase()
{
	//{{AFX_DATA_INIT(CWizardStep3)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_nFirstCtrl = IDC_WIZARD_STEP3_FIRST;
	m_nLastCtrl  = IDC_WIZARD_STEP3_LAST;

} /* #OF# CWizardStep3::CWizardStep3 */

/*========================================================
Method   : CWizardStep3::DoDataExchange
=========================================================*/
/* #FN#
   Dynamic Data Exchange (not used) */
void
CWizardStep3::
DoDataExchange(
	CDataExchange *pDX /* #IN# Pointer to CDataExchange object */
)
{
	CWizardPage::DoDataExchange( pDX );
	CFileAssociationsBase::DataExchange( pDX );

	//{{AFX_DATA_MAP(CWizardStep3)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP

} /* #OF# CWizardStep3::DoDataExchange */


/////////////////////////////////////////////////////////////////////////////
// CWizardStep3 implementation

/*========================================================
Method   : CWizardStep3::SetDlgState
=========================================================*/
/* #FN#
   Sets up the state of the dialog controls */
void
/* #AS#
   Nothing */
CWizardStep3::
SetDlgState()
{
	for( int i = 0; i < m_nFileExtInfoNo; i++ )
	{
		_SetChkBttn( m_pFileExtInfo[ i ].nCtrlID, _IsFlagSet( m_ulFileAssociations, m_pFileExtInfo[ i ].dwRegFlag ) );
	}
} /* #OF# CWizardStep3::SetDlgState */

/*========================================================
Method   : CWizardStep3::TypeButton
=========================================================*/
/* #FN#
   Handles clicking on the file type button */
BOOL
/* #AS#
   TRUE if the requested button has been recognised as a file type button, otherwise FALSE */
CWizardStep3::
TypeButton(
	UINT nButtonID,  /* #IN# Resource ID of the button */
	HWND hwndButton, /* #IN# Handle of the button */
	int  nButton /*=-1*/
)
{
	if( -1 == nButton )
		for( int i = 0; i < m_nFileExtInfoNo; i++ )
		{
			if( nButtonID == m_pFileExtInfo[ i ].nCtrlID ) 
			{
				nButton = i;
				break;
			}
		}

	if( -1 != nButton )
	{
		_ClickButton( m_pFileExtInfo[ nButton ].nCtrlID, m_ulFileAssociations, m_pFileExtInfo[ nButton ].dwRegFlag );
		return TRUE;
	}
	return FALSE;

} /* #OF# CWizardStep3::TypeButton */

/*========================================================
Method   : CWizardStep3::Commit
=========================================================*/
/* #FN#
   Saves changes the user has made using the page */
void
/* #AS#
   Nothing */
CWizardStep3::
Commit()
{
	/* Write dialog status only if necessary */
	if( m_ulFileAssociations != g_Misc.ulFileAssociations )
	{
		g_Misc.ulFileAssociations = m_ulFileAssociations;
		WriteRegDWORD( NULL, REG_FILE_ASSOCIATIONS, g_Misc.ulFileAssociations );

		WriteRegFileExt( g_Misc.ulFileAssociations, m_szHomeDir, g_szCmdLine );
	}
} /* #OF# CWizardStep3::Commit */


/////////////////////////////////////////////////////////////////////////////
// CWizardStep3 message handlers

/*========================================================
Method   : CWizardStep3::OnInitDialog
=========================================================*/
/* #FN#
   Performs special processing when the dialog box is initialized */
BOOL
/* #AS#
   TRUE unless you set the focus to a control */
CWizardStep3::
OnInitDialog()
{
	CWizardPage::OnInitDialog();
	CFileAssociationsBase::InitDialog();

	const UINT anCtrls[ /*m_nFileExtInfoNo*/ ] =
	{
		IDC_WIZARD_STEP3_ATR,
		IDC_WIZARD_STEP3_XFD,
		IDC_WIZARD_STEP3_DCM,
		IDC_WIZARD_STEP3_ATZ,
		IDC_WIZARD_STEP3_XFZ,
		IDC_WIZARD_STEP3_XEX,
		IDC_WIZARD_STEP3_CAS,
		IDC_WIZARD_STEP3_ROM,
		IDC_WIZARD_STEP3_BIN,
		IDC_WIZARD_STEP3_CAR,
		IDC_WIZARD_STEP3_A8S
	};

	/* Set the appropriate controls IDs */
	for( int i = 0; i < m_nFileExtInfoNo; i++ )
		m_pFileExtInfo[ i ].nCtrlID = anCtrls[ i ];

	SetDlgState();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
} /* #OF# CWizardStep3::OnInitDialog */

/*========================================================
Method   : CWizardStep3::OnCommand
=========================================================*/
/* #FN#
   The framework calls this member function when the user selects an item
   from a menu, when a child control sends a notification message, or when
   an accelerator keystroke is translated */
BOOL
/* #AS#
   Nonzero if the application processes this message; otherwise 0 */
CWizardStep3::
OnCommand(
	WPARAM wParam,
	LPARAM lParam
)
{
	/* This could also be handled by many ButtonClicked individual
	   procedures, but this seems a little more elegant */
	if( BN_CLICKED == HIWORD(wParam) )
	{
		UINT nCtrlID = (UINT)LOWORD(wParam);

		if( !TypeButton( nCtrlID, (HWND)lParam ) )
		{
			if( IDC_WIZARD_STEP3_SELECTALL == nCtrlID )
				SelectAll();
			else
			if( IDC_WIZARD_STEP3_REMOVEALL == nCtrlID )
				RemoveAll();

			for( int i = 0; i < m_nFileExtInfoNo; i++ )
				TypeButton( 0, NULL, i );
		}
		return TRUE;
	}
	return CWizardPage::OnCommand( wParam, lParam );

} /* #OF# CDriveDlg::OnCommand */
