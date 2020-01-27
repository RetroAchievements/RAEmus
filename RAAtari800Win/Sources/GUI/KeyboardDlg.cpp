/****************************************************************************
File    : KeyboardDlg.cpp
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# CKeyboardDlg implementation file
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 16.10.2002
*/

#include "StdAfx.h"
#include "Atari800Win.h"
#include "Helpers.h"
#include "FileService.h"
#include "KeyTemplateDlg.h"
#include "KeyboardDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define IDC_KEYBOARD_FIRST			IDC_KEYBOARD_ARROWKEYS
#define IDC_KEYBOARD_LAST			IDC_KEYBOARD_CANCEL


/////////////////////////////////////////////////////////////////////////////
// Static objects

static int s_anKBTable[ KEYBOARD_TABLE_SIZE ];


/////////////////////////////////////////////////////////////////////////////
// CKeyboardDlg dialog

BEGIN_MESSAGE_MAP(CKeyboardDlg, CCommonDlg)
	//{{AFX_MSG_MAP(CKeyboardDlg)
	ON_CBN_SELCHANGE(IDC_KEYBOARD_ARROWKEYS, OnSelchangeArrowKeys)
	ON_BN_CLICKED(IDC_KEYBOARD_TYPEMATICRATE, OnTypematicRate)
	ON_BN_CLICKED(IDC_KEYBOARD_CAPTURECTRLESC, OnCaptureCtrlEsc)
	ON_BN_CLICKED(IDC_KEYBOARD_EDITTEMPLATE, OnEditTemplate)
	ON_BN_CLICKED(IDC_KEYBOARD_LOADTEMPLATE, OnLoadTemplate)
	ON_BN_CLICKED(IDC_KEYBOARD_USETEMPLATE, OnUseTemplate)
	ON_EN_KILLFOCUS(IDC_KEYBOARD_TEMPLATEFILE, OnKillfocusTemplateFile)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_KEYBOARD_CANCEL, OnCancel)
	ON_BN_CLICKED(IDC_KEYBOARD_OK, OnOK)
END_MESSAGE_MAP()

/*========================================================
Method   : CKeyboardDlg::CKeyboardDlg
=========================================================*/
/* #FN#
   Standard constructor */
CKeyboardDlg::
CKeyboardDlg(
	CWnd *pParent /*=NULL*/ /* #IN# Pointer to the parent window */
)
	: CCommonDlg( CKeyboardDlg::IDD, pParent )
{
	//{{AFX_DATA_INIT(CKeyboardDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_bExitPass  = FALSE;
	m_nFirstCtrl = IDC_KEYBOARD_FIRST;
	m_nLastCtrl  = IDC_KEYBOARD_LAST;

} /* #OF# CKeyboardDlg::CKeyboardDlg */

/*========================================================
Method   : CKeyboardDlg::DoDataExchange
=========================================================*/
/* #FN#
   Dynamic Data Exchange (not used) */
void
/* #AS#
   Nothing */
CKeyboardDlg::
DoDataExchange(
	CDataExchange *pDX /* #IN# Pointer to CDataExchange object */
)
{
	CCommonDlg::DoDataExchange( pDX );
	//{{AFX_DATA_MAP(CKeyboardDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP

} /* #OF# CKeyboardDlg::DoDataExchange */


/////////////////////////////////////////////////////////////////////////////
// CKeyboardDlg implementation

/*========================================================
Method   : CKeyboardDlg::PrepareTemplate
=========================================================*/
/* #FN#
   Reads a keyboard template from a given file */
BOOL
/* #AS#
   TRUE if succeeded, otherwise FALSE */
CKeyboardDlg::
PrepareTemplate(
	LPSTR pszTemplateFile,
	LPSTR pszTemplateDesc,
	BOOL  bCheckIfExists /*=TRUE*/
)
{
	BOOL bResult = FALSE;

	if( _IsPathAvailable( pszTemplateFile ) )
	{
		CFileStatus fsStatus;

		if( !bCheckIfExists || CFile::GetStatus( pszTemplateFile, fsStatus ) )
		{
			if( !CKeyTemplateDlg::ReadKeyTemplate( pszTemplateFile, pszTemplateDesc, s_anKBTable, GetSafeHwnd() ) )
			{
				DisplayMessage( GetSafeHwnd(), IDS_ERROR_A8K_LOAD, 0, MB_ICONEXCLAMATION | MB_OK );
				strcpy( pszTemplateFile, FILE_NONE );
			}
			else
				bResult = TRUE;
		}
	}
	/* Clear the template description */
	if( !bResult )
		*pszTemplateDesc = '\0';

	return bResult;

} /* #OF# CKeyboardDlg::PrepareTemplate */

/*========================================================
Method   : CKeyboardDlg::SetDlgState
=========================================================*/
/* #FN#
   Sets up the state of the dialog controls */
void
/* #AS#
   Nothing */
CKeyboardDlg::
SetDlgState()
{
	CFileStatus fsStatus;

	/* Check if there is the pointed template file */
	BOOL bTemplate = _IsPathAvailable( m_szTemplateFile ) &&
					 CFile::GetStatus( m_szTemplateFile, fsStatus );

	if( !bTemplate )
		/* Clear "Use keyboard template" flag if there is no file */
		_ClrFlag( m_ulInputState, IS_KEY_USE_TEMPLATE );

	/* Arrow keys mode */
	if( CB_ERR == _SetSelCbox( IDC_KEYBOARD_ARROWKEYS, m_nArrowsMode ) )
		_SetSelCbox( IDC_KEYBOARD_ARROWKEYS, 0 );

	/* Under Win 9x the Ctrl+Esc keystroke capturing may be turn off only in
	   windowed modes. This feature is always enabled for full-screen modes */
	_EnableCtrl( IDC_KEYBOARD_CAPTURECTRLESC, _IsFlagSet( g_Screen.ulState, SM_MODE_WIND ) || _IsFlagSet( g_Misc.unSystemInfo, SYS_WIN_NT ) );
	_SetChkBttn( IDC_KEYBOARD_CAPTURECTRLESC, ST_CTRLESC_CAPTURED );
	_SetChkBttn( IDC_KEYBOARD_TYPEMATICRATE,  _IsFlagSet( m_ulInputState, IS_KEY_TYPEMATIC_RATE ) );
	_EnableCtrl( IDC_KEYBOARD_USETEMPLATE,    bTemplate );
	_SetChkBttn( IDC_KEYBOARD_USETEMPLATE,    _IsFlagSet( m_ulInputState, IS_KEY_USE_TEMPLATE ) );
	_EnableCtrl( IDC_KEYBOARD_EDITTEMPLATE,   bTemplate );

	SetDlgItemText( IDC_KEYBOARD_TEMPLATEFILE, m_szTemplateFile );
	SetDlgItemText( IDC_KEYBOARD_TEMPLATEDESC, m_szTemplateDesc );

} /* #OF# CKeyboardDlg::SetDlgState */


/////////////////////////////////////////////////////////////////////////////
// CKeyboardDlg message handlers

/*========================================================
Method   : CKeyboardDlg::OnInitDialog
=========================================================*/
/* #FN#
   Performs special processing when the dialog box is initialized */
BOOL
/* #AS#
   TRUE unless you set the focus to a control */
CKeyboardDlg::
OnInitDialog() 
{
	CCommonDlg::OnInitDialog();

	m_ulInputState = g_Input.ulState;
	m_nArrowsMode  = g_Input.Key.nArrowsMode;

	_strncpy( m_szTemplateFile, g_szTemplateFile, MAX_PATH );
	_strncpy( m_szTemplateDesc, g_szTemplateDesc, TEMPLATE_DESC_LENGTH );

	/* Read the template from a file */
	PrepareTemplate( m_szTemplateFile, m_szTemplateDesc );

	SetDlgState();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
} /* #OF# CKeyboardDlg::OnInitDialog */

/*========================================================
Method   : CKeyboardDlg::OnSelchangeArrowKeys
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate combo box */
void
/* #AS#
   Nothing */
CKeyboardDlg::
OnSelchangeArrowKeys()
{
	m_nArrowsMode = _GetSelCbox( IDC_KEYBOARD_ARROWKEYS );

} /* #OF# CKeyboardDlg::OnSelchangeArrowKeys */

/*========================================================
Method   : CKeyboardDlg::OnTypematicRate
=========================================================*/
/* #FN#
	Sets a state of the object regarding to an appropriate check box */
void
/* #AS#
   Nothing */
CKeyboardDlg::
OnTypematicRate()
{
	_ClickButton( IDC_KEYBOARD_TYPEMATICRATE, m_ulInputState, IS_KEY_TYPEMATIC_RATE );

} /* #OF# CKeyboardDlg::OnTypematicRate */

/*========================================================
Method   : CKeyboardDlg::OnCaptureCtrlEsc
=========================================================*/
/* #FN#
	Sets a state of the object regarding to an appropriate check box */
void
/* #AS#
   Nothing */
CKeyboardDlg::
OnCaptureCtrlEsc()
{
	_ClickButton( IDC_KEYBOARD_CAPTURECTRLESC, m_ulInputState, IS_CAPTURE_CTRLESC );

} /* #OF# CKeyboardDlg::OnCaptureCtrlEsc */

/*========================================================
Method   : CKeyboardDlg::OnKillfocusTemplateFile
=========================================================*/
/* #FN#
   The framework calls this function before an edit losing input focus */
void
/* #AS#
   Nothing */
CKeyboardDlg::
OnKillfocusTemplateFile()
{
	char szTemplateOld[ MAX_PATH + 1 ];

	strcpy( szTemplateOld, m_szTemplateFile );
	GetDlgItemText( IDC_KEYBOARD_TEMPLATEFILE, m_szTemplateFile, MAX_PATH );

	if( !m_bExitPass &&
		_stricmp( szTemplateOld, m_szTemplateFile ) != 0 )
	{
		/* Read the template from a file */
		PrepareTemplate( m_szTemplateFile, m_szTemplateDesc );
		/* Set the dialog controls */
		SetDlgState();
	}
} /* #OF# CKeyboardDlg::OnKillfocusTemplateFile */

/*========================================================
Method   : CKeyboardDlg::OnLoadTemplate
=========================================================*/
/* #FN#
   Allows selecting a keyboard template using FileDialog window */
void
/* #AS#
   Nothing */
CKeyboardDlg::
OnLoadTemplate()
{
	if( PickFileName( TRUE, m_szTemplateFile, IDS_SELECT_A8K_LOAD, IDS_FILTER_A8K,
					  "a8k", PF_LOAD_FLAGS, FALSE, DEFAULT_A8K, this ) &&
		*m_szTemplateFile != '\0' )
	{
		/* Read the template from a file */
		PrepareTemplate( m_szTemplateFile, m_szTemplateDesc, FALSE );
		/* Set the dialog controls */
		SetDlgState();
	}
} /* #OF# CKeyboardDlg::OnLoadTemplate */

/*========================================================
Method   : CKeyboardDlg::OnEditTemplate
=========================================================*/
/* #FN#
   Displays "Keyboard Template" dialog box */
void
/* #AS#
   Nothing */
CKeyboardDlg::
OnEditTemplate()
{
	CKeyTemplateDlg dlgKeyTemplate( m_szTemplateFile, m_szTemplateDesc, s_anKBTable, this );

	dlgKeyTemplate.DoModal();

	/* Read the template from a file */
	PrepareTemplate( m_szTemplateFile, m_szTemplateDesc );
	/* Set the dialog controls */
	SetDlgState();

} /* #OF# CKeyboardDlg::OnEditTemplate */

/*========================================================
Method   : CKeyboardDlg::OnUseTemplate
=========================================================*/
/* #FN#
	Sets a state of the object regarding to an appropriate check box */
void
/* #AS#
   Nothing */
CKeyboardDlg::
OnUseTemplate()
{
	_ClickButton( IDC_KEYBOARD_USETEMPLATE, m_ulInputState, IS_KEY_USE_TEMPLATE );
	/* Read the template from a file */
	if( _IsFlagSet( m_ulInputState, IS_KEY_USE_TEMPLATE ) )
	{
		PrepareTemplate( m_szTemplateFile, m_szTemplateDesc );
	}
	/* Set the dialog controls */
	SetDlgState();

} /* #OF# CKeyboardDlg::OnUseTemplate */

/*========================================================
Method   : CKeyboardDlg::ReceiveFocused
=========================================================*/
/* #FN#
   Receives the edit controls content again. The user could press
   'Enter' or 'Alt-O' and then all changes he's made in the last
   edited control would be lost. */
void
/* #AS#
   Nothing */
CKeyboardDlg::
ReceiveFocused()
{
	CWnd *pWnd    = GetFocus();
	UINT  nCtrlID = pWnd ? pWnd->GetDlgCtrlID() : 0;

	if( IDC_KEYBOARD_TEMPLATEFILE == nCtrlID )
	{
		OnKillfocusTemplateFile();
	}
} /* #OF# CKeyboardDlg::ReceiveFocused */

/*========================================================
Method   : CKeyboardDlg::OnOK
=========================================================*/
/* #FN#
   Called when the user clicks the OK button */
void
/* #AS#
   Nothing */
CKeyboardDlg::
OnOK() 
{
	/* There is a problem with messages routing when the dialog is
	   closed with Enter/Alt-O key. KILLFOCUS message arrives
       to late and we have to invoke KillFocus handlers in OnOK
       method by ourselves. That's why we use this member. */
	m_bExitPass = TRUE;

	/* Unfortunately, edit controls do not lose the focus before
	   handling this when the user uses accelerators */
	ReceiveFocused();

	if( m_nArrowsMode != g_Input.Key.nArrowsMode )
	{
		g_Input.Key.nArrowsMode = m_nArrowsMode;
		WriteRegDWORD( NULL, REG_ARROWS_MODE, g_Input.Key.nArrowsMode );

		Input_SetArrowKeys( g_Input.Key.nArrowsMode );
	}
	/* Always reload the selected template */
	if( !PrepareTemplate( m_szTemplateFile, m_szTemplateDesc ) )
	{
		if( _IsFlagSet( m_ulInputState, IS_KEY_USE_TEMPLATE ) )
		{
			_ClrFlag( m_ulInputState, IS_KEY_USE_TEMPLATE );
			DisplayMessage( GetSafeHwnd(), IDS_ERROR_NO_KEYTEMP, 0, MB_ICONEXCLAMATION | MB_OK, m_szTemplateFile );
		}
	}
	else
		/* Fill the template table */
		CopyMemory( g_Input.Key.anKBTable, s_anKBTable, KEYBOARD_TABLE_SIZE * sizeof(int) );

	if( _stricmp( g_szTemplateDesc, m_szTemplateDesc ) != 0 )
		strcpy( g_szTemplateDesc, m_szTemplateDesc );

	/* Windows NT doesn't like empty strings in Registry */
	if( *m_szTemplateFile == '\0' )
		strcpy( m_szTemplateFile, DEFAULT_A8K );

	if( _stricmp( g_szTemplateFile, m_szTemplateFile ) != 0 )
	{
		strcpy( g_szTemplateFile, m_szTemplateFile );
		WriteRegString( NULL, REG_FILE_TEMPLATE, g_szTemplateFile );
	}
	/* Check the miscellanous states */
	if( m_ulInputState != g_Input.ulState )
	{
		if( g_Input.ulState & IS_CAPTURE_CTRLESC ^
			m_ulInputState & IS_CAPTURE_CTRLESC )
		{
			Input_EnableEscCapture( _IsFlagSet( m_ulInputState, IS_CAPTURE_CTRLESC ) );
		}
		g_Input.ulState = m_ulInputState;
		WriteRegDWORD( NULL, REG_INPUT_STATE, g_Input.ulState);
	}
	CCommonDlg::OnOK();

} /* #OF# CKeyboardDlg::OnOK */
