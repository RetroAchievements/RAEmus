/****************************************************************************
File    : AboutDlg.cpp
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# CAboutDlg implementation file
@(#) #BY# Richard Lawrence, Tomasz Szymankowski
@(#) #LM# 03.08.2002
*/

#include "StdAfx.h"
#include "Atari800Win.h"
#include "Helpers.h"
#include "AboutDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/* Static objects of the class */

UINT CAboutDlg::m_nLastIcon = IDI_ATARIXE;

/*========================================================
Method   : CAboutDlg::CAboutDlg
=========================================================*/
/* #FN#
   Standard constructor */
CAboutDlg::
CAboutDlg(
	CWnd *pParent /*=NULL*/ /* #IN# Pointer to the parent window */
)
	: CDialog( CAboutDlg::IDD, pParent )
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT

} /* #OF# CAboutDlg::CAboutDlg */

/*========================================================
Method   : CAboutDlg::DoDataExchange
=========================================================*/
/* #FN#
   Dynamic Data Exchange */
void
/* #AS#
   Nothing */
CAboutDlg::
DoDataExchange(
	CDataExchange *pDX /* #IN# Pointer to CDataExchange object */
)
{
	CDialog::DoDataExchange( pDX );
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP

} /* #OF# CAboutDlg::DoDataExchange */


/////////////////////////////////////////////////////////////////////////////
// CCheatDlg message handlers

/*========================================================
Method   : CAboutDlg::OnInitDialog
=========================================================*/
/* #FN#
   Performs special processing when the dialog box is initialized */
BOOL
/* #AS#
   TRUE unless you set the focus to a control */
CAboutDlg::
OnInitDialog()
{
	CDialog::OnInitDialog();

	CStatic *pStatic = (CStatic *)GetDlgItem( IDC_ABOUT_ICON );
	HANDLE   hIcon   = AfxGetApp()->LoadIcon( m_nLastIcon = (m_nLastIcon == IDR_MAINFRAME ? IDI_ATARIXE : IDR_MAINFRAME) );

	char szTextL[ LOADSTRING_SIZE_L + 1 ];
	char szTextS[ LOADSTRING_SIZE_S + 1 ];
	
	SetDlgItemText( IDC_ABOUT_COPYRIGHT, _LoadStringLx( IDS_COPYRIGHT, szTextL ) );
	SetDlgItemText( IDC_ABOUT_WEBPAGE, _LoadStringSx( IDS_WEB_PAGE, szTextS ) );

	sprintf( szTextL, _LoadStringSx( IDS_PRG_NAME, szTextS ), VERSION_INFO );
	SetDlgItemText( IDC_ABOUT_PRGNAME, szTextL );
			  
	if( hIcon && pStatic )
		pStatic->SetIcon( (HICON)hIcon );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
} /* #OF# CAboutDlg::OnInitDialog */
