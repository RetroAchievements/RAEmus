/****************************************************************************
File    : FileSmallDlg.cpp
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# CFileSmallDlg implementation file
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 30.09.2002
*/

#include "StdAfx.h"
#include "Atari800Win.h"
#include "Helpers.h"
#include "FileService.h"
#include "FileSmallDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CFileSmallDlg dialog

BEGIN_MESSAGE_MAP(CFileSmallDlg, CDialog)
	//{{AFX_MSG_MAP(CFileSmallDlg)
	ON_NOTIFY(TVN_ITEMEXPANDING, IDC_FILE_SMALL_TREE, OnItemExpanding)
	ON_NOTIFY(TVN_SELCHANGED, IDC_FILE_SMALL_TREE, OnSelchangedTree)
	ON_NOTIFY(TVN_DELETEITEM, IDC_FILE_SMALL_TREE, OnDeleteItemTree)
	ON_NOTIFY(NM_DBLCLK, IDC_FILE_SMALL_TREE, OnDblclkTree)
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_NCHITTEST()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/*========================================================
Method   : CFileSmallDlg::CFileSmallDlg
=========================================================*/
/* #FN#
   Standard constructor */
CFileSmallDlg::
CFileSmallDlg(
	BOOL   bOpenFileDialog,         /* #IN# */
	LPCSTR pszDefExt,    /*=NULL*/  /* #IN# */
	LPCSTR pszTitle,     /*=NULL*/  /* #IN# */
	LPCSTR pszStateFile, /*=NULL*/  /* #IN# */
	BOOL   bFoldersOnly, /*=FALSE*/ /* #IN# */
	CWnd  *pParent       /*=NULL*/  /* #IN# */
)
	: CDialog( CFileSmallDlg::IDD, pParent ),
	  m_bOpenFileDialog( bOpenFileDialog ),
	  m_strDefExt( pszDefExt ),
	  m_strDlgTitle( pszTitle ),
	  m_strSelectedPath( pszStateFile ),
	  m_bFoldersOnly( bFoldersOnly )
{
	//{{AFX_DATA_INIT(CFileSmallDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	ASSERT(!bFoldersOnly || bFoldersOnly && bOpenFileDialog);

} /* #OF# CFileSmallDlg::CFileSmallDlg */

/*========================================================
Method   : CFileSmallDlg::DoDataExchange
=========================================================*/
/* #FN#
   Dynamic Data Exchange */
void
/* #AS#
   Nothing */
CFileSmallDlg::
DoDataExchange(
	CDataExchange *pDX /* #IN# Pointer to CDataExchange object */
)
{
	CDialog::DoDataExchange( pDX );
	//{{AFX_DATA_MAP(CFileSmallDlg)
	DDX_Control(pDX, IDC_FILE_SMALL_TREE, m_treeShell);
	//}}AFX_DATA_MAP

} /* #OF# CFileSmallDlg::DoDataExchange */


/////////////////////////////////////////////////////////////////////////////
// CFileSmallDlg message handlers

/*========================================================
Method   : CFileSmallDlg::OnInitDialog
=========================================================*/
/* #FN#
   Performs special processing when the dialog box is initialized */
BOOL
/* #AS#
   TRUE unless you set the focus to a control */
CFileSmallDlg::
OnInitDialog()
{
	CDialog::OnInitDialog();

	CRect  rcDlg, rcCtrl;
	CWnd  *pCtrl = NULL;
	CEdit *pEdit = NULL;
	char   szPrompt[ LOADSTRING_SIZE_S + 1 ];

	/* Get data for repositioning routine */
	GetClientRect( rcDlg );

	pCtrl = GetDlgItem( IDC_FILE_SMALL_TREE );
	if( pCtrl )
	{
		pCtrl->GetWindowRect( rcCtrl );
		ScreenToClient( rcCtrl );
		m_nTreeYSub = rcDlg.bottom - rcCtrl.bottom + rcCtrl.top;
		m_nTreeXSub = rcDlg.right - rcCtrl.right + rcCtrl.left;
	}
	pCtrl = GetDlgItem( IDC_FILE_SMALL_EDIT );
	if( pCtrl )
	{
		pCtrl->GetWindowRect( rcCtrl );
		ScreenToClient( rcCtrl );
		m_nEditYSub = rcDlg.bottom - rcCtrl.top;
		m_nEditXSub = rcDlg.right - rcCtrl.right + rcCtrl.left;
		m_nEditY    = rcCtrl.Height();
		m_nEditX    = rcCtrl.left;
	}
	pCtrl = GetDlgItem( IDOK );
	if( pCtrl )
	{
		pCtrl->GetWindowRect( rcCtrl );
		ScreenToClient( rcCtrl );
		m_nOkYSub = rcDlg.bottom - rcCtrl.top;
		m_nOkXSub = rcDlg.right - rcCtrl.left;
	}
	pCtrl = GetDlgItem( IDCANCEL );
	if( pCtrl )
	{
		pCtrl->GetWindowRect( rcCtrl );
		ScreenToClient( rcCtrl );
		m_nCancelYSub = rcDlg.bottom - rcCtrl.top;
		m_nCancelXSub = rcDlg.right - rcCtrl.left;
	}
	/* The dialog template is very small because dialog's size
	   could be resized before showing on the screen in other way.
       That's why we have to enlarge dialog window size here. */
	SetWindowPos( NULL, 0, 0, rcDlg.Width() + 60, rcDlg.Height() + 80, SWP_NOMOVE );

	if( m_strDlgTitle.IsEmpty() )
		m_strDlgTitle = m_bFoldersOnly ?
						_LoadStringSx( IDS_SELECT_FOLDER, szPrompt ) :
						_LoadStringSx( IDS_SELECT_FILE, szPrompt );
	SetWindowText( m_strDlgTitle );

	m_treeShell.SetRedraw( FALSE ); /* Not really needed? */
	m_treeShell.SetFolderMode( m_bFoldersOnly );
	m_treeShell.EnableImages();
	m_treeShell.PopulateTree();
//	m_treeShell.TunnelTree( m_strSelectedPath );
	m_treeShell.SetRedraw();

	pEdit = (CEdit *)GetDlgItem( IDC_FILE_SMALL_EDIT );
	ASSERT(NULL != pEdit);
	pEdit->SetFocus();
	pEdit->SetSel( (DWORD)MAKELONG(0, -1) );

	return FALSE;  // return TRUE unless you set the focus to a control
				   // EXCEPTION: OCX Property Pages should return FALSE
} /* #OF# CFileSmallDlg::OnInitDialog */

/*========================================================
Method   : CFileSmallDlg::OnSize
=========================================================*/
/* #FN#
   The framework calls this member function after the window’s size has
   changed */
void
/* #AS#
   Nothing */
CFileSmallDlg::
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

	pCtrl = GetDlgItem( IDC_FILE_SMALL_TREE );
	if( pCtrl )
	{
		pCtrl->SetWindowPos( NULL,
			0, 0,
			cx - m_nTreeXSub,
			cy - m_nTreeYSub,
			SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE );
	}
	pCtrl = GetDlgItem( IDC_FILE_SMALL_EDIT );
	if( pCtrl )
	{
		pCtrl->SetWindowPos( NULL,
			m_nEditX,
			cy - m_nEditYSub,
			cx - m_nEditXSub,
			m_nEditY,
			SWP_NOZORDER | SWP_NOACTIVATE );
	}
	pCtrl = GetDlgItem( IDOK );
	if( pCtrl )
	{
		pCtrl->SetWindowPos( NULL,
			cx - m_nOkXSub,
			cy - m_nOkYSub,
			0, 0,
			SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOCOPYBITS );
	}
	pCtrl = GetDlgItem( IDCANCEL );
	if( pCtrl )
	{
		pCtrl->SetWindowPos( NULL,
			cx - m_nCancelXSub,
			cy - m_nCancelYSub,
			0, 0,
			SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOCOPYBITS );
	}
} /* #OF# CFileSmallDlg::OnSize */

/*========================================================
Method   : CFileSmallDlg::OnPaint
=========================================================*/
/* #FN#
   The framework calls this member function when Windows or an application
   makes a request to repaint a portion of an application’s window */
void
/* #AS#
   Nothing */
CFileSmallDlg::
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

	/* It is not a good idea to redraw Cancel button here, but
	   I have not time to do it wiser at the moment */
	CWnd *pCtrl = GetDlgItem( IDCANCEL );
	if( pCtrl )
		pCtrl->RedrawWindow();

	CDialog::OnPaint();

} /* #OF# CFileSmallDlg::OnPaint */

/*========================================================
Method   : CFileSmallDlg::OnNcHitTest
=========================================================*/
/* #FN#
   The framework calls this member function for the window object that
   contains the cursor every time the mouse is moved */
LRESULT
/* #AS#
   One of mouse hit-test enumerated values */
CFileSmallDlg::
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

} /* #OF# CFileSmallDlg::OnNcHitTest */

/*========================================================
Method   : CFileSmallDlg::OnItemExpanding
=========================================================*/
/* #FN#
   Called when a tree item is expanded by the user */
void
/* #AS#
   Nothing */
CFileSmallDlg::
OnItemExpanding(
	NMHDR   *pNMHDR, /* #IN# */
	LRESULT *pResult /* #OUT# */
)
{
//	NM_TREEVIEW *pNMTreeView = (NM_TREEVIEW *)pNMHDR;

	m_treeShell.OnFolderExpanding( pNMHDR, pResult );
	*pResult = 0;

} /* #OF# CFileSmallDlg::OnItemExpanding */

/*========================================================
Method   : CFileSmallDlg::OnSelchangedTree
=========================================================*/
/* #FN#
   Called when a tree item is selected by the user */
void
/* #AS#
   Nothing */
CFileSmallDlg::
OnSelchangedTree(
	NMHDR   *pNMHDR, /* #IN# */
	LRESULT *pResult /* #OUT# */
)
{
//	NM_TREEVIEW *pNMTreeView = (NM_TREEVIEW *)pNMHDR;

	CString szPath;
	if( m_treeShell.OnFolderSelected( pNMHDR, pResult, szPath ) )
	{
		CWnd *pEdit = GetDlgItem( IDC_FILE_SMALL_EDIT );
		ASSERT(NULL != pEdit);
		pEdit->SetWindowText( szPath );
	}
	*pResult = 0;

} /* #OF# CFileSmallDlg::OnSelchangedTree */

/*========================================================
Method   : CFileSmallDlg::OnDeleteItemTree
=========================================================*/
/* #FN#
   Deletes a tree node */
void
/* #AS#
   Nothing */
CFileSmallDlg::
OnDeleteItemTree(
	NMHDR   *pNMHDR, /* #IN# */
	LRESULT *pResult /* #OUT# */
)
{
//	NM_TREEVIEW *pNMTreeView = (NM_TREEVIEW *)pNMHDR;

	m_treeShell.OnDeleteShellItem( pNMHDR, pResult );
	*pResult = 0;

} /* #OF# CFileSmallDlg::OnDeleteItemTree */

/*========================================================
Method   : CFileSmallDlg::OnDblclkTree
=========================================================*/
/* #FN#
   Called when a tree item is double-clicked by the user */
void
/* #AS#
   Nothing */
CFileSmallDlg::
OnDblclkTree(
	NMHDR * /*pNMHDR*/, /* #IN#  */
	LRESULT *pResult    /* #OUT# */
)
{
	if( !m_bFoldersOnly )
		/* Try to end the file-selector session */
		OnOK();

	*pResult = 0;

} /* #OF# CFileSmallDlg::OnDblclkTree */

/*========================================================
Method   : CFileSmallDlg::GetPathName
=========================================================*/
/* #FN#
   Returns path name selected by the user */
CString
/* #AS#
   Selected path name */
CFileSmallDlg::
GetPathName()
{
	return m_strSelectedPath;

} /* #OF# CFileSmallDlg::GetPathName */

/*========================================================
Method   : CFileSmallDlg::OnOK
=========================================================*/
/* #FN#
   Called when the user clicks the OK button */
void
/* #AS#
   Nothing */
CFileSmallDlg::
OnOK()
{
	char szPath[ MAX_PATH + 1 ];
	BOOL bDrive  = FALSE;
	BOOL bFailed = FALSE;

	CFileStatus fsStatus;

	CWnd *pEdit = GetDlgItem( IDC_FILE_SMALL_EDIT );
	ASSERT(NULL != pEdit);
	pEdit->GetWindowText( szPath, MAX_PATH );

	if( szPath[ strlen( szPath ) - 1 ] == '\\' )
		szPath[ strlen( szPath ) - 1 ] = '\0';

	/* Is it a drive? */
	bDrive = strlen( szPath ) == 2 && szPath[ 1 ] == ':';

	/* Add the default extension */
	if( !m_bFoldersOnly && !bDrive && !m_strDefExt.IsEmpty() &&
		(m_bOpenFileDialog ||
		!m_bOpenFileDialog && _stricmp( &szPath[ strlen( szPath ) - m_strDefExt.GetLength() ], (LPSTR)(LPCSTR)m_strDefExt ) != 0) &&
		!CFile::GetStatus( szPath, fsStatus ) )
	{
		strcat( szPath, PATH_NONE );
		strcat( szPath, (LPSTR)(LPCSTR)m_strDefExt );
	}

	if( m_bOpenFileDialog ) /* Open file/folder */
	{
		/* That's a static method, don't panic */
		if( CFile::GetStatus( szPath, fsStatus ) )
		{
			BOOL bFolder = fsStatus.m_attribute & CFile::directory;

			if( m_bFoldersOnly && !bFolder || !m_bFoldersOnly && bFolder )
				/* This is not appropriate object, do nothing */
				return;
		}
		else if( bDrive )
		{
			if( !m_bFoldersOnly )
				/* This is a drive, do nothing */
				return;
		}
		else
			bFailed = TRUE;
	}
	else /* Save file */
	{
		if( CFile::GetStatus( szPath, fsStatus ) )
		{
			if( fsStatus.m_attribute & CFile::directory )
			{
				/* This is a directory, do nothing */
				return;
			}
			else if( (IDYES != DisplayMessage( GetSafeHwnd(), IDS_WARN_FILE_EXISTS, 0, MB_ICONEXCLAMATION | MB_YESNO, szPath )) )
			{
				/* The file exists and must not be overwritten, exit */
				return;
			}
		}
		else if( bDrive )
		{
			/* This is a drive, do nothing */
			return;
		}
		else /* Check if the folder exists */
		{
			char szFolderPath[ MAX_PATH + 1 ];
			strcpy( szFolderPath, szPath );

			GetFolderPath( szFolderPath, NULL );
			if( CFile::GetStatus( szFolderPath, fsStatus ) )
			{
				if( !(fsStatus.m_attribute & CFile::directory) )
					bFailed = TRUE;
			}
			else if( strlen( szFolderPath ) != 3 || szPath[ 1 ] != ':' )
				bFailed = TRUE;
		}
	}
	/* Display message if failed */
	if( bFailed )
	{
		DisplayMessage( GetSafeHwnd(), (!m_bOpenFileDialog || m_bFoldersOnly ?
						IDS_ERROR_NO_PATH : IDS_ERROR_NO_FILE),
						0, MB_ICONEXCLAMATION | MB_OK, szPath );
		/* Don't close the file dialog */
		return;
	}
	/* Add ending backslash if it is a drive name */
	if( bDrive )
		strcat( szPath, "\\" );

	/* OK, save the selected path */
	m_strSelectedPath = szPath;

	CDialog::OnOK();

} /* #OF# CFileSmallDlg::OnOK */
