/****************************************************************************
File    : WizardDlg.cpp
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# CWizardDlg implementation file
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 30.09.2002
*/

#include "StdAfx.h"
#include "Atari800Win.h"
#include "WizardDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define IDC_WIZARD_FIRST	IDC_WIZARD_BACK

/* For handling context help request invoked by Shift+F1 there
   is necessary to include all the wizard pages' controls here.
   Unfortunately, the context help request invoked by question-
   pointer is handled in the standard way by the CCommonDlg.
*/
#define IDC_WIZARD_LAST		IDC_WIZARD_STEP3_REMOVEALL /*IDC_WIZARD_CANCEL*/


/////////////////////////////////////////////////////////////////////////////
// CWizardPage class

/*========================================================
Method   : CWizardPage::CWizardPage
=========================================================*/
/* #FN#
   Standard constructor */
CWizardPage::
CWizardPage(
	UINT  nIDTemplate,
	CWnd *pParent /*=NULL*/ /* #IN# Pointer to the parent window */
)
	: CCommonDlg( nIDTemplate, pParent ),
	  m_bReboot( FALSE )
{
	//{{AFX_DATA_INIT(CWizardPage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

} /* #OF# CWizardPage::CWizardPage */


/////////////////////////////////////////////////////////////////////////////
// CWizardDlg dialog

BEGIN_MESSAGE_MAP(CWizardDlg, CCommonDlg)
	//{{AFX_MSG_MAP(CWizardDlg)
	ON_BN_CLICKED(IDC_WIZARD_NEXT, OnNext)
	ON_BN_CLICKED(IDC_WIZARD_BACK, OnBack)
	ON_BN_CLICKED(IDC_WIZARD_FINISH, OnFinish)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_WIZARD_CANCEL, CWizardDlg::OnCancel)
END_MESSAGE_MAP()

/*========================================================
Method   : CWizardDlg::CWizardDlg
=========================================================*/
/* #FN#
   Standard constructor */
CWizardDlg::
CWizardDlg(
	CWnd *pParent /*=NULL*/ /* #IN# Pointer to the parent window */
)
	: CCommonDlg( CWizardDlg::IDD, pParent ),
	  m_nCurrentIndex( -1 ),
	  m_strTitle( "" )
{
	//{{AFX_DATA_INIT(CWizardDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_nFirstCtrl = IDC_WIZARD_FIRST;
	m_nLastCtrl  = IDC_WIZARD_LAST;

} /* #OF# CWizardDlg::CWizardDlg */

/*========================================================
Method   : CWizardDlg::~CWizardDlg
=========================================================*/
/* #FN#
   Destructor */
CWizardDlg::
~CWizardDlg()
{
	m_arrPages.RemoveAll();

} /* #OF# CWizardDlg::~CWizardDlg */

/*========================================================
Method   : CWizardDlg::DoDataExchange
=========================================================*/
/* #FN#
   Dynamic Data Exchange */
void
/* #AS#
   Nothing */
CWizardDlg::
DoDataExchange(
	CDataExchange *pDX /* #IN# Pointer to CDataExchange object */
)
{
	CCommonDlg::DoDataExchange( pDX );
	//{{AFX_DATA_MAP(CWizardDlg)
	DDX_Control(pDX, IDC_WIZARD_FRAME, m_wndPanel);
	//}}AFX_DATA_MAP

} /* #OF# CWizardDlg::DoDataExchange */


/////////////////////////////////////////////////////////////////////////////
// CWizardDlg implementation

/*========================================================
Method   : CWizardDlg::SetWizardState
=========================================================*/
/* #FN#
   Sets up the state of the wizard controls */
void
/* #AS#
   Nothing */
CWizardDlg::
SetWizardState()
{
	CButton *pBack   = (CButton *)GetDlgItem( IDC_WIZARD_BACK );
	CButton *pNext   = (CButton *)GetDlgItem( IDC_WIZARD_NEXT );
	CButton *pFinish = (CButton *)GetDlgItem( IDC_WIZARD_FINISH );

	BOOL bFirst = (BOOL)(!m_nCurrentIndex);
	BOOL bLast  = (BOOL)(m_nCurrentIndex == m_arrPages.GetUpperBound());

	ASSERT(NULL != pBack && NULL != pNext && NULL != pFinish);

	pBack->EnableWindow( !bFirst );
	pNext->EnableWindow( !bLast );

	if( bLast )
	{
		SetDefID( IDC_WIZARD_FINISH );
		GotoDlgCtrl( pFinish );
	}
	else
	{
		SetDefID( IDC_WIZARD_NEXT );
		if( bFirst )
			GotoDlgCtrl( pNext );
	}
	/* Set wizard dialog title */
	m_strTitle.Format( IDS_WIZARD_TITLE, m_nCurrentIndex + 1, m_arrPages.GetUpperBound() != -1 ? m_arrPages.GetUpperBound() + 1 : 1 );
	SetWindowText( m_strTitle );

} /* #OF# CWizardDlg::SetWizardState */

/*========================================================
Method   : CWizardDlg::AddPage
=========================================================*/
/* #FN#
   Adds a page to the wizard */
void
/* #AS#
   Nothing */
CWizardDlg::
AddPage(
	CWizardPage *pPage /* #IN# Pointer to a page to add */
)
{
	m_arrPages.Add( (void *)pPage );

} /* #OF# CWizardDlg::AddPage */

/*========================================================
Method   : CWizardDlg::GetPage
=========================================================*/
/* #FN#
   Makes a page with a given index visible to the user */
void
/* #AS#
   Nothing */
CWizardDlg::
SetPage(
	const int nIndex /* #IN# Index of the page to be shown */
)
{
	CWizardPage *pPage = NULL;

	if( m_nCurrentIndex >= 0 )
	{
		pPage = GetPage( m_nCurrentIndex );
		if( pPage )
		{
			if( !pPage->UpdateData() )
				return;
			pPage->ShowWindow( SW_HIDE );
		}
	}
	m_nCurrentIndex = nIndex;

	pPage = GetPage( m_nCurrentIndex );
	if( pPage )
		pPage->ShowWindow( SW_SHOW );

} /* #OF# CWizardDlg::SetPage */

/*========================================================
Method   : CWizardDlg::GetPage
=========================================================*/
/* #FN#
   Returns pointer to a page with a given index */
CWizardPage*
/* #AS#
   Pointer to the requested page */
CWizardDlg::
GetPage(
	const int nIndex /* #IN# Index of the requested page */
)
{
	ASSERT(nIndex <= m_arrPages.GetUpperBound());
	ASSERT(nIndex >= 0);

	return (CWizardPage *)m_arrPages.GetAt( nIndex );

} /* #OF# CWizardDlg::GetPage */

/*========================================================
Method   : CWizardDlg::ClosePages
=========================================================*/
/* #FN#
   Closes all the pages of the wizard */
void
/* #AS#
   Nothing */
CWizardDlg::
ClosePages()
{
	CWizardPage *pPage = NULL;
	for( int i = 0; i < m_arrPages.GetUpperBound() + 1; i++ )
	{
		pPage = GetPage( i );
		if( pPage )
			pPage->DestroyWindow();
	}
} /* #OF# CWizardDlg::ClosePages */

/*========================================================
Method   : CWizardDlg::Reboot
=========================================================*/
/* #FN#
   Checks if reboot of an emulated Atari is necessary */
BOOL
/* #AS#
   TRUE if reboot is necessary, otherwise FALSE */
CWizardDlg::
Reboot()
{
	BOOL bResult = FALSE;

	CWizardPage *pPage = NULL;
	for( int i = 0; i < m_arrPages.GetUpperBound() + 1; i++ )
	{
		pPage = GetPage( i );
		if( pPage && pPage->GetReboot() )
		{
			bResult = TRUE;
			break;
		}
	}
	return bResult;

} /* #OF# CWizardDlg::Reboot */

/*========================================================
Method   : CWizardDlg::Commit
=========================================================*/
/* #FN#
   Saves all the changes the user has made */
void
/* #AS#
   Nothing */
CWizardDlg::
Commit()
{
	CWizardPage *pPage = NULL;
	for( int i = 0; i < m_arrPages.GetUpperBound() + 1; i++ )
	{
		pPage = GetPage( i );
		if( pPage )
			pPage->Commit();
	}
} /* #OF# CWizardDlg::Commit */


/////////////////////////////////////////////////////////////////////////////
// CWizardDlg message handlers

/*========================================================
Method   : CWizardDlg::OnInitDialog
=========================================================*/
/* #FN#
   Performs special processing when the dialog box is initialized */
BOOL
/* #AS#
   TRUE unless you set the focus to a control */
CWizardDlg::
OnInitDialog()
{
	CCommonDlg::OnInitDialog();

	if( m_arrPages.GetUpperBound() != -1 )
	{
		CWizardPage *pPage = NULL;
		CRect rc;

		m_wndPanel.GetWindowRect( rc );
		ScreenToClient( rc );

		for( int i = 0; i < m_arrPages.GetUpperBound() + 1; i++ )
		{
			pPage = GetPage( i );
			if( pPage )
			{
				pPage->Create( pPage->GetIDTemplate(), this );
				pPage->SetWindowPos( NULL, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOZORDER );
			}			
		}
		SetPage( 0 );
		SetWizardState();
	}
	return FALSE;  // return TRUE unless you set the focus to a control
	               // EXCEPTION: OCX Property Pages should return FALSE
} /* #OF# CWizardDlg::OnInitDialog */

/*========================================================
Method   : CWizardDlg::OnNext
=========================================================*/
/* #FN#
   Called when the user clicks the wizard's NEXT button */
void
/* #AS#
   Nothing */
CWizardDlg::
OnNext() 
{
	if( m_nCurrentIndex < m_arrPages.GetUpperBound() )
		SetPage( m_nCurrentIndex + 1 );

	SetWizardState();

} /* #OF# CWizardDlg::OnNext */

/*========================================================
Method   : CWizardDlg::OnBack
=========================================================*/
/* #FN#
   Called when the user clicks the wizard's BACK button */
void
/* #AS#
   Nothing */
CWizardDlg::
OnBack()
{
	if( m_nCurrentIndex > 0 )
		SetPage( m_nCurrentIndex - 1 );

	SetWizardState();

} /* #OF# CWizardDlg::OnBack */

/*========================================================
Method   : CWizardDlg::OnFinish
=========================================================*/
/* #FN#
   Called when the user clicks the wizard's FINISH button */
void
/* #AS#
   Nothing */
CWizardDlg::
OnFinish() 
{
	CWizardPage *pPage = NULL;
	for( int i = 0; i < m_arrPages.GetUpperBound() + 1; i++ )
	{
		pPage = GetPage( i );
		if( pPage )
			pPage->UpdateData();
	}
	ClosePages();

	CCommonDlg::OnOK();

} /* #OF# CWizardDlg::OnFinish */

/*========================================================
Method   : CWizardDlg::OnCancel
=========================================================*/
/* #FN#
   Called when the user clicks the wizard's CANCEL button */
void
/* #AS#
   Nothing */
CWizardDlg::
OnCancel()
{
	ClosePages();
	CCommonDlg::OnCancel();

} /* #OF# CWizardDlg::OnCancel */
