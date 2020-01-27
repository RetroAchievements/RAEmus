/****************************************************************************
File    : BootTypeDlg.cpp
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# CBootTypeDlg implementation file
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 30.09.2002
*/

#include "StdAfx.h"
#include "Atari800Win.h"
#include "Helpers.h"
#include "BootTypeDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define IDC_BOOTTYPE_FIRST		IDC_BOOTTYPE_LOGICAL
#define IDC_BOOTTYPE_LAST		IDC_BOOTTYPE_CANCEL


/////////////////////////////////////////////////////////////////////////////
// CBootTypeDlg dialog

BEGIN_MESSAGE_MAP(CBootTypeDlg, CCommonDlg)
	//{{AFX_MSG_MAP(CBootTypeDlg)
	ON_BN_CLICKED(IDC_BOOTTYPE_LOGICAL, OnLogical)
	ON_BN_CLICKED(IDC_BOOTTYPE_PHYSICAL, OnPhysical)
	ON_BN_CLICKED(IDC_BOOTTYPE_SIO2PC, OnSio2Pc)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BOOTTYPE_OK, OnOK)
	ON_BN_CLICKED(IDC_BOOTTYPE_CANCEL, CCommonDlg::OnCancel)
END_MESSAGE_MAP()

/*========================================================
Method   : CBootTypeDlg::CBootTypeDlg
=========================================================*/
/* #FN#
   Standard constructor */
CBootTypeDlg::
CBootTypeDlg(
	CWnd *pParent /*=NULL*/ /* #IN# Pointer to the parent window */
)
	: CCommonDlg( CBootTypeDlg::IDD, pParent )
{
	//{{AFX_DATA_INIT(CBootTypeDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_nFirstCtrl = IDC_BOOTTYPE_FIRST;
	m_nLastCtrl  = IDC_BOOTTYPE_LAST;

} /* #OF# CBootTypeDlg::CBootTypeDlg */

/*========================================================
Method   : CBootTypeDlg::DoDataExchange
=========================================================*/
/* #FN#
   Dynamic Data Exchange (not used) */
void
/* #AS#
   Nothing */
CBootTypeDlg::
DoDataExchange(
	CDataExchange *pDX /* #IN# Pointer to CDataExchange object */
)
{
	CCommonDlg::DoDataExchange( pDX );
	//{{AFX_DATA_MAP(CBootTypeDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP

} /* #OF# CBootTypeDlg::DoDataExchange */


/////////////////////////////////////////////////////////////////////////////
// CBootTypeDlg implementation

/*========================================================
Method   : CBootTypeDlg::SetDlgState
=========================================================*/
/* #FN#
   Sets up the state of the dialog controls */
void
/* #AS#
   Nothing */
CBootTypeDlg::
SetDlgState()
{
	char  szBuffer[ LOADSTRING_SIZE_L + 1 ];
	int   nCtrlID = 0;
	CWnd *pWnd    = NULL;

	switch( m_btSelOpt )
	{
		case Logical:
			_LoadStringLx( IDS_BOOT_DESC_LOGICAL, szBuffer );
			nCtrlID = IDC_BOOTTYPE_LOGICAL;
			break;

		case Physical:
			_LoadStringLx( IDS_BOOT_DESC_PHYSICAL, szBuffer );
			nCtrlID = IDC_BOOTTYPE_PHYSICAL;
			break;

		case Sio2Pc:
			_LoadStringLx( IDS_BOOT_DESC_SIO2PC, szBuffer );
			nCtrlID = IDC_BOOTTYPE_SIO2PC;
			break;
	}
	/* Set an appropriate radio button */
	CheckRadioButton( IDC_BOOTTYPE_LOGICAL, IDC_BOOTTYPE_SIO2PC, nCtrlID );

	/* Set an option description */
	pWnd = GetDlgItem( IDC_BOOTTYPE_DESCRIPTION );
	ASSERT(NULL != pWnd);
	pWnd->SetWindowText( szBuffer );

} /* #OF# CBootTypeDlg::SetDlgState */


/////////////////////////////////////////////////////////////////////////////
// CBootTypeDlg message handlers

/*========================================================
Method   : CBootTypeDlg::OnInitDialog
=========================================================*/
/* #FN#
   Performs special processing when the dialog box is initialized */
BOOL
/* #AS#
   TRUE unless you set the focus to a control */
CBootTypeDlg::
OnInitDialog() 
{
	CCommonDlg::OnInitDialog();

	m_btSelOpt = Logical;

	SetDlgState();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
} /* #OF# CBootTypeDlg::OnInitDialog */

/*========================================================
Method   : CBootTypeDlg::OnLogical
=========================================================*/
/* #FN#
	Sets a state of the object regarding to an appropriate radio button */
void
/* #AS#
   Nothing */
CBootTypeDlg::
OnLogical()
{
	m_btSelOpt = Logical;
	SetDlgState();

} /* #OF# CBootTypeDlg::OnLogical */

/*========================================================
Method   : CBootTypeDlg::OnPhysical
=========================================================*/
/* #FN#
	Sets a state of the object regarding to an appropriate radio button */
void
/* #AS#
   Nothing */
CBootTypeDlg::
OnPhysical()
{
	m_btSelOpt = Physical;
	SetDlgState();

} /* #OF# CBootTypeDlg::OnPhysical */

/*========================================================
Method   : CBootTypeDlg::OnSio2Pc
=========================================================*/
/* #FN#
	Sets a state of the object regarding to an appropriate radio button */
void
/* #AS#
   Nothing */
CBootTypeDlg::
OnSio2Pc()
{
	m_btSelOpt = Sio2Pc;
	SetDlgState();

} /* #OF# CBootTypeDlg::OnSio2Pc */

/*========================================================
Method   : CBootTypeDlg::OnOK
=========================================================*/
/* #FN#
   Called when the user clicks the OK button */
void
/* #AS#
   Nothing */
CBootTypeDlg::
OnOK() 
{
	CCommonDlg::OnOK();

} /* #OF# CBootTypeDlg::OnOK */
