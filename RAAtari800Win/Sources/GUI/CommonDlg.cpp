/****************************************************************************
File    : CommonDlg.cpp
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# CCommonDlg implementation file
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 17.03.2002
*/

#include "StdAfx.h"
#include "Atari800Win.h"
#include "CommonDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CCommonDlg Class

BEGIN_MESSAGE_MAP(CCommonDlg, CDialog)
	//{{AFX_MSG_MAP(CCommonDlg)
	ON_WM_HELPINFO()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/*========================================================
Metoda   : CCommonDlg::CCommonDlg
=========================================================*/
/* #FN#
   Standard constructor */
CCommonDlg::
CCommonDlg(
	UINT nIDTemplate,
	CWnd *pParentWnd /*=NULL*/
)
	: CDialog( nIDTemplate, pParentWnd )
{
	m_nIDTemplate = nIDTemplate;
	m_nFirstCtrl = 0;
	m_nLastCtrl  = 0;
	m_bStateInit = FALSE;

} /* #OF# CCommonDlg::CCommonDlg */

/*========================================================
Metoda   : CCommonDlg::~CCommonDlg
=========================================================*/
/* #FN#
   Destructor */
CCommonDlg::
~CCommonDlg()
{
} /* #OF# CCommonDlg::~CCommonDlg */


/////////////////////////////////////////////////////////////////////////////
// CCommonDlg message handlers

/*========================================================
Method   : CCommonDlg::OnInitDialog
=========================================================*/
/* #FN#
   Performs special processing when the dialog box is initialized */
BOOL
/* #AS#
   TRUE unless you set the focus to a control */
CCommonDlg::
OnInitDialog()
{
	CDialog::OnInitDialog();

	/* Unfortunately, the context help is available only in windowed modes */
	if( g_Screen.ulState & SM_MODE_WIND )
		SetWindowLong( GetSafeHwnd(), GWL_EXSTYLE, WS_EX_CONTEXTHELP | GetWindowLong( GetSafeHwnd(), GWL_EXSTYLE ) );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
} /* #OF# CCommonDlg::OnInitDialog */

/*========================================================
Function : CCommonDlg::HelpInfo
=========================================================*/
/* #FN#
   Context help handler of dialog windows */
void
/* #AS#
   Nothing */
CCommonDlg::
HelpInfo(
	HELPINFO *pHelpInfo,
	UINT      nFirstCtrl,
	UINT      nLastCtrl
)
{
	/* Check to see if the F1+SHIFT keystroke is used */
	if( !_IsKeyPressed( VK_F1 ) || _IsKeyPressed( VK_SHIFT ) )
	{
		HWND hCtrl   = ::WindowFromPoint( pHelpInfo->MousePos );
		UINT nCtrlId = (hCtrl ? ::GetDlgCtrlID( hCtrl ) : pHelpInfo->iCtrlId);

		/* Static and disabled windows are not indicated by
		   the WindowFromPoint method */
		if( GetSafeHwnd() == hCtrl && pHelpInfo->hItemHandle )
		{
			CRect rc;
			::GetWindowRect( (HWND)pHelpInfo->hItemHandle, rc );

			if( rc.PtInRect( pHelpInfo->MousePos ) )
				nCtrlId = pHelpInfo->iCtrlId;
		}
		if( nCtrlId &&
			nCtrlId >= nFirstCtrl &&
			nCtrlId <= nLastCtrl )
		{
			AfxGetApp()->WinHelp( HID_BASE_CONTROL + nCtrlId, HELP_CONTEXTPOPUP );
			return;
		}
	}
	AfxGetApp()->WinHelp( HID_BASE_RESOURCE + m_nIDTemplate );

} /* #OF# CCommonDlg::HelpInfo */

/*========================================================
Function : CCommonDlg::OnHelpInfo
=========================================================*/
/* #FN#
   Called by the framework when the user presses the F1 key */
BOOL
/* #AS#
   Always TRUE */
CCommonDlg::
OnHelpInfo(
	HELPINFO *pHelpInfo /* #IN# Information about the object for which help is requested */
)
{
	if( g_Screen.ulState & SM_MODE_WIND )
		HelpInfo( pHelpInfo, m_nFirstCtrl, m_nLastCtrl );

	return TRUE; //CDialog::OnHelpInfo( pHelpInfo );

} /* #OF# CCommonDlg::OnHelpInfo */
