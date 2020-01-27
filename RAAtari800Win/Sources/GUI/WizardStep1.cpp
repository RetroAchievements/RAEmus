/****************************************************************************
File    : WizardStep1.cpp
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# CWizardStep1 implementation file
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 30.09.2002
*/

#include "StdAfx.h"
#include "Atari800Win.h"
#include "WizardDlg.h"
#include "WizardStep1.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define IDC_WIZARD_STEP1_FIRST		IDC_WIZARD_STEP1_SHORTMENU
#define IDC_WIZARD_STEP1_LAST		IDC_WIZARD_STEP1_SHORTDESK


/////////////////////////////////////////////////////////////////////////////
// Static objects

static CWizardStep1::ShortInfo_t s_aShortInfo[] =
{
	{ "Atari800Win.exe", VERSION_INFO".lnk",      "Atari800Win PLus Emulator" },
	{ "Atari800Win.hlp", VERSION_INFO" Help.lnk", "Atari800Win PLus Help"     },
	{ "Atari800Win.url", "Website.lnk",           "Atari800Win PLus Website"},
	{ "Uninstall.exe", "Uninstall.lnk",         "Atari800Win PLus Uninstall"}
};

const int s_nShortInfoNo = sizeof(s_aShortInfo)/sizeof(s_aShortInfo[0]);


/////////////////////////////////////////////////////////////////////////////
// CWizardStep1 dialog

BEGIN_MESSAGE_MAP(CWizardStep1, CWizardPage)
	//{{AFX_MSG_MAP(CWizardStep1)
	ON_BN_CLICKED(IDC_WIZARD_STEP1_SHORTMENU, OnShortMenu)
	ON_BN_CLICKED(IDC_WIZARD_STEP1_SHORTDESK, OnShortDesk)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/*========================================================
Method   : CWizardStep1::CWizardStep1
=========================================================*/
/* #FN#
   Standard constructor */
CWizardStep1::
CWizardStep1(
	CWnd *pParent /*=NULL*/ /* #IN# Pointer to the parent window */
)
	: CWizardPage( CWizardStep1::IDD, pParent )
{
	//{{AFX_DATA_INIT(CWizardStep1)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_nFirstCtrl = IDC_WIZARD_STEP1_FIRST;
	m_nLastCtrl  = IDC_WIZARD_STEP1_LAST;

	m_pShortInfo = s_aShortInfo;
	m_bInitCOM   = FALSE;
	m_bShortMenu = FALSE;
	m_bShortDesk = FALSE;

} /* #OF# CWizardStep1::CWizardStep1 */

/*========================================================
Method   : CWizardStep1::DoDataExchange
=========================================================*/
/* #FN#
   Dynamic Data Exchange (not used) */
void
CWizardStep1::
DoDataExchange(
	CDataExchange *pDX /* #IN# Pointer to CDataExchange object */
)
{
	CWizardPage::DoDataExchange( pDX );
	//{{AFX_DATA_MAP(CWizardStep1)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP

} /* #OF# CWizardStep1::DoDataExchange */


/////////////////////////////////////////////////////////////////////////////
// CWizardStep2 implementation

/*========================================================
Method   : CWizardStep1::SetDlgState
=========================================================*/
/* #FN#
   Sets up the state of the dialog controls */
void
/* #AS#
   Nothing */
CWizardStep1::
SetDlgState()
{
	_SetChkBttn( IDC_WIZARD_STEP1_SHORTMENU, m_bShortMenu );
	_SetChkBttn( IDC_WIZARD_STEP1_SHORTDESK, m_bShortDesk );

} /* #OF# CWizardStep1::SetDlgState */

/*========================================================
Method   : CWizardStep1::CreateLink
=========================================================*/
/* #FN#
   Uses the shell's IShellLink and IPersistFile interfaces 
   to create and store a shortcut to the specified object */
HRESULT
/* #AS#
   Result of calling member functions of the interfaces */
CWizardStep1::
CreateLink(
	LPCSTR lpszPathObj,  /* #IN# Address of a buffer containing the path of the object */
	LPCSTR lpszPathLink, /* #IN# Address of a buffer containing the path where the shell link is to be stored */
	LPCSTR lpszDesc      /* #IN# Address of a buffer containing the description of the shell link */
)
{
	HRESULT hResult;
	IShellLink* psl;

	// Get a pointer to the IShellLink interface. 
	hResult = CoCreateInstance( CLSID_ShellLink,
							 NULL, 
							 CLSCTX_INPROC_SERVER,
							 IID_IShellLink,
							 (LPVOID *)&psl );
	if( SUCCEEDED(hResult) )
	{
		IPersistFile *ppf;

		// Set the path to the shortcut target and add the description.
		psl->SetPath( lpszPathObj );
		psl->SetDescription( lpszDesc );

		// Query IShellLink for the IPersistFile interface for saving the
		// shortcut in persistent storage.
		hResult = psl->QueryInterface( IID_IPersistFile, (LPVOID *)&ppf );

		if( SUCCEEDED(hResult) )
		{
			WCHAR wsz[ MAX_PATH + 1 ];

			// Ensure that the string is Unicode. 
			MultiByteToWideChar( CP_ACP, 0, lpszPathLink, -1, wsz, MAX_PATH );

			// Save the link by calling IPersistFile::Save.
			hResult = ppf->Save( wsz, TRUE );
			ppf->Release();
		}
		psl->Release();
	}
	return hResult;

} /* #OF# CWizardStep1::CreateLink */

/*========================================================
Method   : CWizardStep1::GetSpecialFolder
=========================================================*/
/* #FN#
   Gets a requested special system folder path */
void
/* #AS#
   Nothing */
CWizardStep1::
GetSpecialFolder(
	int   nFolder,        /* #IN#  Special folder key */
	LPSTR szSpecialFolder /* #OUT# Path to the requested special folder */
)
{
    LPITEMIDLIST pidl = NULL;
    LPMALLOC pMalloc = NULL;

    SHGetMalloc( &pMalloc );

	SHGetSpecialFolderLocation( GetSafeHwnd(), nFolder, &pidl );
	SHGetPathFromIDList( pidl, szSpecialFolder );
	if( pidl )
		pMalloc->Free( pidl );

    pMalloc->Release();

} /* #OF# CWizardStep1::GetSpecialFolder */

/*========================================================
Method   : CWizardStep1::GetMenuFolder
=========================================================*/
/* #FN#
   Gets a path to the emulator folder located in 'Programs' menu */
void
/* #AS#
   Nothing */
CWizardStep1::
GetMenuFolder(
	LPSTR szMenuPath /* #OUT# Path to the requested menu object */
)
{
	GetSpecialFolder( CSIDL_PROGRAMS, szMenuPath );
	strcat( szMenuPath, "\\Atari800Win PLus" );

} /* #OF# CWizardStep1::GetMenuFolder */

/*========================================================
Method   : CWizardStep1::GetDeskFolder
=========================================================*/
/* #FN#
   Gets a path to 'Desktop' folder */
void
/* #AS#
   Nothing */
CWizardStep1::
GetDeskFolder(
	LPSTR szDeskPath /* #OUT# Path to the requested desktop object */
)
{
	GetSpecialFolder( CSIDL_DESKTOP, szDeskPath );

} /* #OF# CWizardStep1::GetDeskFolder */

/*========================================================
Method   : CWizardStep1::CreateShortcut
=========================================================*/
/* #FN#
   Creates a shortcut to an emulator file */
BOOL
/* #AS#
   TRUE if succeeded, otherwise FALSE */
CWizardStep1::
CreateShortcut(
	ShortInfo_t *pShortInfo,
	LPCSTR pszShortPath /* #IN# Path to a shortcut to be created */
)
{
	char szSource[ MAX_PATH + 1 ];
	char szTarget[ MAX_PATH + 1 ];
	BOOL bResult = TRUE;
	CFileStatus fsStatus;

	sprintf( szSource, "%s\\%s", m_szHomeDir, pShortInfo->pszFile );
	sprintf( szTarget, "%s\\%s", pszShortPath, pShortInfo->pszShortcut );

	if ( CFile::GetStatus( szSource, fsStatus) )
		if( FAILED(CreateLink( szSource, szTarget, pShortInfo->pszDesc )) )
			bResult = FALSE;

	return bResult;

} /* #OF# CWizardStep1::CreateShortcut */

/*========================================================
Method   : CWizardStep1::DeleteShortcut
=========================================================*/
/* #FN#
   Deletes a given shortcut to an emulator file */
BOOL
/* #AS#
   TRUE if succeeded, otherwise FALSE */
CWizardStep1::
DeleteShortcut(
	ShortInfo_t *pShortInfo,
	LPCSTR pszShortPath /* #IN# Path to a shortcut to be deleted */
)
{
	char szSource[ MAX_PATH + 1 ];
	CFileStatus fsStatus;
	BOOL bResult = TRUE;

	sprintf( szSource, "%s\\%s", pszShortPath, pShortInfo->pszShortcut );
	if( CFile::GetStatus( szSource, fsStatus ) )
	{
		bResult = DeleteFile( szSource );
	}
	return bResult;

} /* #OF# CWizardStep1::DeleteShortcut */

/*========================================================
Method   : CWizardStep1::Commit
=========================================================*/
/* #FN#
   Saves changes the user has made using the page */
void
/* #AS#
   Nothing */
CWizardStep1::
Commit()
{
	char szShortPath[ MAX_PATH + 1 ];
	CFileStatus fsStatus;

	GetMenuFolder( szShortPath );
	/* Create shortcuts in the Start Menu */
	if( m_bShortMenu )
	{
		if( CFile::GetStatus( szShortPath, fsStatus ) || CreateDirectory( szShortPath, NULL ) )
			for( int i = 0; i < s_nShortInfoNo; i++ )
			{
				CreateShortcut( &m_pShortInfo[ i ], szShortPath );
			}
	}
	/* Remove shortcuts from the Start Menu */
	else
	{
		if( CFile::GetStatus( szShortPath, fsStatus ) )
			for( int i = 0; i < s_nShortInfoNo; i++ )
			{
				DeleteShortcut( &m_pShortInfo[ i ], szShortPath );
			}
		RemoveDirectory( szShortPath );
	}

	GetDeskFolder( szShortPath );
	/* Create shortcuts at the Desktop */
	if( m_bShortDesk )
	{
		CreateShortcut( &m_pShortInfo[ 0 ], szShortPath );
	}
	/* Remove shortcuts from the Desktop */
	else
	{
		DeleteShortcut( &m_pShortInfo[ 0 ], szShortPath );
	}
} /* #OF# CWizardStep1::Commit */


/////////////////////////////////////////////////////////////////////////////
// CWizardStep1 message handlers

/*========================================================
Method   : CWizardStep1::OnInitDialog
=========================================================*/
/* #FN#
   Performs special processing when the dialog box is initialized */
BOOL
/* #AS#
   TRUE unless you set the focus to a control */
CWizardStep1::
OnInitDialog()
{
	CWizardPage::OnInitDialog();

	char szShortPath[ MAX_PATH + 1 ];
	CFileStatus fsStatus;

	/* Init the COM library */
	if( !m_bInitCOM )
	{
		CoInitialize( NULL );
		m_bInitCOM = TRUE;
	}
	/* Prepare the shortcuts */
	if( !Misc_GetHomeDirectory( m_szHomeDir ) )
		GetCurrentDirectory( MAX_PATH, m_szHomeDir );

	/* We are satisfied if there is a folder for shortcuts in the Start Menu */
	GetMenuFolder( szShortPath );
	m_bShortMenu = CFile::GetStatus( szShortPath, fsStatus );

	GetDeskFolder( szShortPath );
	sprintf( szShortPath, "%s\\%s", szShortPath, m_pShortInfo[ 0 ].pszShortcut );
	m_bShortDesk = CFile::GetStatus( szShortPath, fsStatus );

	SetDlgState();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
} /* #OF# CWizardStep1::OnInitDialog */

/*========================================================
Method   : CWizardStep1::OnShortMenu
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate check box */
void
/* #AS#
   Nothing */
CWizardStep1::
OnShortMenu()
{
	m_bShortMenu = _GetChkBttn( IDC_WIZARD_STEP1_SHORTMENU );

} /* #OF# CWizardStep1::OnShortMenu */

/*========================================================
Method   : CWizardStep1::OnShortDesk
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate check box */
void
/* #AS#
   Nothing */
CWizardStep1::
OnShortDesk()
{
	m_bShortDesk = _GetChkBttn( IDC_WIZARD_STEP1_SHORTDESK );

} /* #OF# CWizardStep1::OnShortDesk */
