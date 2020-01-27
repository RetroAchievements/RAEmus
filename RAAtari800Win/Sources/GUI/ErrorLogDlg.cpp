/****************************************************************************
File    : ErrorLogDlg.cpp
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# CErrorLogDlg implementation file
@(#) #BY# Tomasz Szymankowski, Richard Lawrence
@(#) #LM# 17.03.2002
*/

#include "StdAfx.h"
#include "Atari800Win.h"
#include "ErrorLogDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CErrorLogDlg dialog

BEGIN_MESSAGE_MAP(CErrorLogDlg, CDialog)
	//{{AFX_MSG_MAP(CErrorLogDlg)
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_NCHITTEST()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/*========================================================
Method   : CErrorLogDlg::CErrorLogDlg
=========================================================*/
/* #FN#
   Standard constructor */
CErrorLogDlg::
CErrorLogDlg(
	CWnd *pParent /*=NULL*/ /* #IN# Pointer to the parent window */
)
	: CDialog( CErrorLogDlg::IDD, pParent )
{
	//{{AFX_DATA_INIT(CErrorLogDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

} /* #OF# CErrorLogDlg::CErrorLogDlg */

/*========================================================
Method   : CErrorLogDlg::DoDataExchange
=========================================================*/
/* #FN#
   Dynamic Data Exchange */
void
/* #AS#
   Nothing */
CErrorLogDlg::
DoDataExchange(
	CDataExchange *pDX /* #IN# Pointer to CDataExchange object */
)
{
	CDialog::DoDataExchange( pDX );
	//{{AFX_DATA_MAP(CErrorLogDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP

} /* #OF# CErrorLogDlg::DoDataExchange */


/////////////////////////////////////////////////////////////////////////////
// CErrorLogDlg message handlers

/*========================================================
Method   : CErrorLogDlg::OnInitDialog
=========================================================*/
/* #FN#
   Performs special processing when the dialog box is initialized */
BOOL
/* #AS#
   TRUE unless you set the focus to a control */
CErrorLogDlg::
OnInitDialog() 
{
	CDialog::OnInitDialog();

	CRect rcDlg, rcCtrl;
	CWnd *pCtrl = NULL;

	GetClientRect( rcDlg );

	pCtrl = GetDlgItem( IDC_ERRORLOG_EDIT );
	if( pCtrl )
	{
		pCtrl->GetWindowRect( rcCtrl );
		ScreenToClient( rcCtrl );
		m_nEditYSub = rcDlg.bottom - rcCtrl.bottom + rcCtrl.top;
		m_nEditXSub = rcDlg.right - rcCtrl.right + rcCtrl.left;
	}
	pCtrl = GetDlgItem( IDOK );
	if( pCtrl )
	{
		pCtrl->GetWindowRect( rcCtrl );
		ScreenToClient( rcCtrl );
		m_nButnYSub = rcDlg.bottom - rcCtrl.top;
		m_nButnXSub = rcDlg.right - rcCtrl.left;
	}
	/* The dialog template is very small because dialog's size
	   could be resized before showing on the screen in other way.
       That's why we have to enlarge dialog window size here. */
	SetWindowPos( NULL, 0, 0, rcDlg.Width() + 60, rcDlg.Height() + 80, SWP_NOMOVE );
	
	SetDlgItemText( IDC_ERRORLOG_EDIT, (const char *)Log_buffer );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
} /* #OF# CErrorLogDlg::OnInitDialog */

/*========================================================
Method   : CErrorLogDlg::OnSize
=========================================================*/
/* #FN#
   The framework calls this member function after the window’s size has
   changed */
void
/* #AS#
   Nothing */
CErrorLogDlg::
OnSize(
	UINT nType, /* #IN# Type of resizing requested    */
	int cx,     /* #IN# New width of the client area  */
	int cy      /* #IN# New height of the client area */
)
{
	CWnd *pCtrl = NULL;

	CDialog::OnSize( nType, cx, cy );

	/* When enlarging a dialog box we need to erase the old gripper */
	InvalidateRect( m_rcGripper );

	pCtrl = GetDlgItem( IDC_ERRORLOG_EDIT );
	if( pCtrl )
	{
		pCtrl->SetWindowPos( NULL,
			0, 0,
			cx - m_nEditXSub,
			cy - m_nEditYSub,
			SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE );
	}
	pCtrl = GetDlgItem( IDOK );
	if( pCtrl )
	{
		pCtrl->SetWindowPos( NULL,
			cx - m_nButnXSub,
			cy - m_nButnYSub,
			0, 0,
			SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );
	}
} /* #OF# CErrorLogDlg::OnSize */

/*========================================================
Method   : CErrorLogDlg::OnPaint
=========================================================*/
/* #FN#
   The framework calls this member function when Windows or an application
   makes a request to repaint a portion of an application’s window */
void
/* #AS#
   Nothing */
CErrorLogDlg::
OnPaint()
{
	/* Draw a resizing gripper at the lower right corner */
	CRect rc;
	GetClientRect( rc );

	rc.left = rc.right  - ::GetSystemMetrics( SM_CXHSCROLL );
	rc.top  = rc.bottom - ::GetSystemMetrics( SM_CYVSCROLL );
	m_rcGripper = rc;

	CClientDC dc( this );
	dc.DrawFrameControl( rc, DFC_SCROLL, DFCS_SCROLLSIZEGRIP );

	/* It is not a good idea to redraw OK button here, but
	   I have not time to do it wiser at the moment */
	CWnd *pCtrl = GetDlgItem( IDOK );
	if( pCtrl )
		pCtrl->RedrawWindow();

	CDialog::OnPaint();

} /* #OF# CErrorLogDlg::OnPaint */

/*========================================================
Method   : CErrorLogDlg::OnNcHitTest
=========================================================*/
/* #FN#
   The framework calls this member function for the window object that
   contains the cursor every time the mouse is moved */
LRESULT
/* #AS#
   One of mouse hit-test enumerated values */
CErrorLogDlg::
OnNcHitTest(
	CPoint point /* #IN# Contains the x- and y-coordinates of the cursor */
)
{
	LRESULT ht = CDialog::OnNcHitTest( point );

	if( ht == HTCLIENT )
	{
		CRect rc;
		GetWindowRect( rc );

		rc.left = rc.right  - GetSystemMetrics( SM_CXHSCROLL );
		rc.top  = rc.bottom - GetSystemMetrics( SM_CYVSCROLL );
		if( rc.PtInRect( point ) )
		{
			ht = HTBOTTOMRIGHT;
		}
	}
	return ht;

} /* #OF# CErrorLogDlg::OnNcHitTest */
