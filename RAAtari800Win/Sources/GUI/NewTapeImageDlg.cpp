/****************************************************************************
File    : NewTapeImageDlg.cpp
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# CNewTapeImageDlg implementation file
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 19.01.2004
*/

#include "StdAfx.h"
#include "Atari800Win.h"
#include "Helpers.h"
#include "FileService.h"
#include "NewTapeImageDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define IDC_NEWTAPEIMAGE_FIRST		IDC_NEWTAPEIMAGE_DESCRIPTION
#define IDC_NEWTAPEIMAGE_LAST		IDC_NEWTAPEIMAGE_CANCEL


/////////////////////////////////////////////////////////////////////////////
// CNewTapeImageDlg dialog

BEGIN_MESSAGE_MAP(CNewTapeImageDlg, CCommonDlg)
	//{{AFX_MSG_MAP(CNewTapeImageDlg)
	ON_EN_KILLFOCUS(IDC_NEWTAPEIMAGE_DESCRIPTION, OnKillfocusDescription)
	ON_BN_CLICKED(IDC_NEWTAPEIMAGE_ATTACHTAPE, OnAttachTape)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_NEWTAPEIMAGE_OK, OnOK)
	ON_BN_CLICKED(IDC_NEWTAPEIMAGE_CANCEL, CCommonDlg::OnCancel)
END_MESSAGE_MAP()

/*========================================================
Method   : CNewTapeImageDlg::CNewTapeImageDlg
=========================================================*/
/* #FN#
   Standard constructor */
CNewTapeImageDlg::
CNewTapeImageDlg(
	CWnd *pParent /*=NULL*/ /* #IN# Pointer to the parent window */
)
	: CCommonDlg( CNewTapeImageDlg::IDD, pParent )
{
	//{{AFX_DATA_INIT(CNewTapeImageDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_nFirstCtrl = IDC_NEWTAPEIMAGE_FIRST;
	m_nLastCtrl  = IDC_NEWTAPEIMAGE_LAST;

} /* #OF# CNewTapeImageDlg::CNewTapeImageDlg */

/*========================================================
Method   : CNewTapeImageDlg::DoDataExchange
=========================================================*/
/* #FN#
   Dynamic Data Exchange (not used) */
void
/* #AS#
   Nothing */
CNewTapeImageDlg::
DoDataExchange(
	CDataExchange *pDX /* #IN# Pointer to CDataExchange object */
)
{
	CCommonDlg::DoDataExchange( pDX );
	//{{AFX_DATA_MAP(CNewTapeImageDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP

} /* #OF# CNewTapeImageDlg::DoDataExchange */


/////////////////////////////////////////////////////////////////////////////
// CNewTapeImageDlg implementation

/*========================================================
Method   : CNewTapeImageDlg::SetDlgState
=========================================================*/
/* #FN#
   Sets up the state of the dialog controls */
void
/* #AS#
   Nothing */
CNewTapeImageDlg::
SetDlgState()
{
	/* Set up "attach to drive" group */
	_SetChkBttn( IDC_NEWTAPEIMAGE_ATTACHTAPE, m_bAttachTape );

#ifdef WIN_NETWORK_GAMES
	if( ST_KAILLERA_ACTIVE )
	{
		_EnableCtrl( IDC_NEWTAPEIMAGE_ATTACHTAPE, FALSE );
	}
#endif
} /* #OF# CNewTapeImageDlg::SetDlgState */


/////////////////////////////////////////////////////////////////////////////
// CNewTapeImageDlg message handlers

/*========================================================
Method   : CNewTapeImageDlg::OnInitDialog
=========================================================*/
/* #FN#
   Performs special processing when the dialog box is initialized */
BOOL
/* #AS#
   TRUE unless you set the focus to a control */
CNewTapeImageDlg::
OnInitDialog()
{
	CCommonDlg::OnInitDialog();

	strcpy( m_szTapeName, "None" );
	*m_szTapeDesc = '\0';

#ifdef WIN_NETWORK_GAMES
	if( ST_KAILLERA_ACTIVE )
	{
		m_bAttachTape = FALSE;
	}
	else
#endif
		m_bAttachTape = TRUE;

	SetDlgState();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
} /* #OF# CNewTapeImageDlg::OnInitDialog */

/*========================================================
Method   : CNewTapeImageDlg::OnKillfocusDescription
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate edit control */
void
/* #AS#
   Nothing */
CNewTapeImageDlg::
OnKillfocusDescription()
{
	GetDlgItemText( IDC_NEWTAPEIMAGE_DESCRIPTION, m_szTapeDesc, CASSETTE_DESCRIPTION_MAX );

} /* #OF# CNewTapeImageDlg::OnKillfocusDescription */

/*========================================================
Method   : CNewTapeImageDlg::OnAttachTape
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate check box */
void
/* #AS#
   Nothing */
CNewTapeImageDlg::
OnAttachTape()
{
	m_bAttachTape = _GetChkBttn( IDC_NEWTAPEIMAGE_ATTACHTAPE );

} /* #OF# CNewTapeImageDlg::OnAttachTape */

/*========================================================
Method   : CNewTapeImageDlg::ReceiveFocused
=========================================================*/
/* #FN#
   Receives the edit controls content again. The user could press
   'Enter' or 'Alt-O' and then all changes he's made in the last
   edited control would be lost. */
void
/* #AS#
   Nothing */
CNewTapeImageDlg::
ReceiveFocused()
{
	CWnd *pWnd    = GetFocus();
	UINT  nCtrlID = pWnd ? pWnd->GetDlgCtrlID() : 0;

	if( IDC_NEWTAPEIMAGE_DESCRIPTION == nCtrlID )
	{
		OnKillfocusDescription();
	}
} /* #OF# CNewTapeImageDlg::ReceiveFocused */

/*========================================================
Method   : CNewTapeImageDlg::OnOK
=========================================================*/
/* #FN#
   Called when the user clicks the OK button */
void
/* #AS#
   Nothing */
CNewTapeImageDlg::
OnOK()
{
	/* Unfortunately, edit controls do not lose the focus before
	   handling this when the user uses accelerators */
	ReceiveFocused();

	if( PickFileName( FALSE, m_szTapeName, IDS_SELECT_CAS_SAVE, IDS_FILTER_CAS,
					  "cas", PF_SAVE_FLAGS, FALSE, DEFAULT_TAP, this ) &&
		*m_szTapeName != '\0' )
	{
		FILE *pfImage = NULL;
		/* Create the image file */
		if( NULL != (pfImage = fopen( m_szTapeName, "wb" )) )
		{
			BOOL bIsCas = FALSE;
			/* Set description of the tape */
			_strncpy( cassette_description, m_szTapeDesc, CASSETTE_DESCRIPTION_MAX - 1 );
			/* Write cassette header */
			CASSETTE_CreateFile( m_szTapeName, &pfImage, &bIsCas );
			/* Close the image file */
			fclose( pfImage );
		}
		else
			DisplayMessage( GetSafeHwnd(), IDS_ERROR_FILE_OPEN, 0, MB_ICONEXCLAMATION | MB_OK, m_szTapeName );
	}
	CCommonDlg::OnOK();

} /* #OF# CNewTapeImageDlg::OnOK */
