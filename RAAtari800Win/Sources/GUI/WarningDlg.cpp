/****************************************************************************
File    : WarningDlg.cpp
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# CWarningDlg implementation file
@(#) #BY# Richard Lawrence, Tomasz Szymankowski
@(#) #LM# 30.09.2002
*/

#include "StdAfx.h"
#include "Atari800Win.h"
#include "Helpers.h"
#include "WarningDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CWarningDlg dialog

BEGIN_MESSAGE_MAP(CWarningDlg, CDialog)
	//{{AFX_MSG_MAP(CWarningDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/*========================================================
Method   : CWarningDlg::CWarningDlg
=========================================================*/
/* #FN#
   Standard constructor */
CWarningDlg::
CWarningDlg(
	CWnd *pParent /*=NULL*/ /* #IN# Pointer to the parent window */
)
	: CDialog( CWarningDlg::IDD, pParent ),
	  m_bCancel( TRUE ),
	  m_nWarningID( -1 ),
	  m_nWarningFlag( 0 )
{
	//{{AFX_DATA_INIT(CWarningDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	*m_szWarningText = '\0';

} /* #OF# CWarningDlg::CWarningDlg */

/*========================================================
Method   : CWarningDlg::DoDataExchange
=========================================================*/
/* #FN#
   Dynamic Data Exchange (not used) */
void
/* #AS#
   Nothing */
CWarningDlg::
DoDataExchange(
	CDataExchange *pDX /* #IN# Pointer to CDataExchange object */
)
{
	CDialog::DoDataExchange( pDX );
	//{{AFX_DATA_MAP(CWarningDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP

} /* #OF# CWarningDlg::DoDataExchange */


/////////////////////////////////////////////////////////////////////////////
// CWarningDlg implementation

/*========================================================
Method   : CWarningDlg::SaveDontShowFlag
=========================================================*/
/* #FN#
   Sets a 'dont show' flag if it's requested */
void
/* #AS#
   Nothing */
CWarningDlg::
SaveDontShowFlag()
{
	if( _GetChkBttn( IDC_WARNING_NOSHOW ) )
	{
		WriteRegDWORD( NULL, REG_DONT_SHOW, (g_Misc.ulDontShow |= m_nWarningFlag) );
	}
} /* #OF# CWarningDlg::SaveDontShowFlag */


/////////////////////////////////////////////////////////////////////////////
// CWarningDlg message handlers

/*========================================================
Method   : CWarningDlg::OnInitDialog
=========================================================*/
/* #FN#
   Performs special processing when the dialog box is initialized */
BOOL
/* #AS#
   TRUE unless you set the focus to a control */
CWarningDlg::
OnInitDialog()
{
	CDialog::OnInitDialog();

	if( !m_bCancel )
	{
		CRect rcCtrl;

		CButton *pButton = (CButton *)GetDlgItem( IDCANCEL );
		ASSERT(NULL != pButton);

		pButton->GetWindowRect( rcCtrl );
		ScreenToClient( rcCtrl );
		pButton->ShowWindow( SW_HIDE );

		pButton = (CButton *)GetDlgItem( IDOK );
		ASSERT(NULL != pButton);

		pButton->SetWindowPos( NULL,
			rcCtrl.TopLeft().x,
			rcCtrl.TopLeft().y,
			0, 0,
			SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOCOPYBITS );
	}

	if( '\0' == *m_szWarningText )
		_LoadStringLx( -1 != m_nWarningID ? m_nWarningID : IDS_WARN_ERROR, m_szWarningText );

	SetDlgItemText( IDC_WARNING_TEXT, m_szWarningText );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
} /* #OF# CWarningDlg::OnInitDialog */

/*========================================================
Method   : CWarningDlg::OnOK
=========================================================*/
/* #FN#
   Called when the user clicks the OK button */
void
/* #AS#
   Nothing */
CWarningDlg::
OnOK()
{
	SaveDontShowFlag();

	CDialog::OnOK();

} /* #OF# CWarningDlg::OnOK */

/*========================================================
Method   : CWarningDlg::OnCancel
=========================================================*/
/* #FN#
   Called when the user clicks the CANCEL button */
void
/* #AS#
   Nothing */
CWarningDlg::
OnCancel()
{
	SaveDontShowFlag();
	
	CDialog::OnCancel();

} /* #OF# CWarningDlg::OnCancel */
