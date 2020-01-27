/****************************************************************************
File    : HarddiskDlg.cpp
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# CHarddiskDlg implementation file
@(#) #BY# Tomasz Szymankowski, Richard Lawrence
@(#) #LM# 15.07.2003
*/

#include "StdAfx.h"
#include "Atari800Win.h"
#include "Helpers.h"
#include "FileService.h"
#include "FileSmallDlg.h"
#include "HarddiskDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define IDC_HARDDISK_FIRST		IDC_HARDDISK_BUTTON1
#define IDC_HARDDISK_LAST		IDC_HARDDISK_CANCEL

#define HD_DRIVES_NO		4


/////////////////////////////////////////////////////////////////////////////
// Static objects

static CHarddiskDlg::DriveData_t s_aDriveData[ HD_DRIVES_NO ] =
{
	{ Devices_atari_h_dir[0], "", IDC_HARDDISK_EDIT1, REG_HD1 },
	{ Devices_atari_h_dir[1], "", IDC_HARDDISK_EDIT2, REG_HD2 },
	{ Devices_atari_h_dir[2], "", IDC_HARDDISK_EDIT3, REG_HD3 },
	{ Devices_atari_h_dir[3], "", IDC_HARDDISK_EDIT4, REG_HD4 }
};


/////////////////////////////////////////////////////////////////////////////
// CHarddiskDlg dialog

BEGIN_MESSAGE_MAP(CHarddiskDlg, CCommonDlg)
	//{{AFX_MSG_MAP(CHarddiskDlg)
	ON_BN_CLICKED(IDC_HARDDISK_BUTTON1, OnHd1)
	ON_BN_CLICKED(IDC_HARDDISK_BUTTON2, OnHd2)
	ON_BN_CLICKED(IDC_HARDDISK_BUTTON3, OnHd3)
	ON_BN_CLICKED(IDC_HARDDISK_BUTTON4, OnHd4)
	ON_EN_KILLFOCUS(IDC_HARDDISK_EDIT2, OnKillfocusHd2Edit)
	ON_EN_KILLFOCUS(IDC_HARDDISK_EDIT1, OnKillfocusHd1Edit)
	ON_EN_KILLFOCUS(IDC_HARDDISK_EDIT3, OnKillfocusHd3Edit)
	ON_EN_KILLFOCUS(IDC_HARDDISK_EDIT4, OnKillfocusHd4Edit)
	ON_EN_KILLFOCUS(IDC_HARDDISK_EXEPATH, OnKillfocusExePath)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_HARDDISK_OK, OnOK)
	ON_BN_CLICKED(IDC_HARDDISK_CANCEL, CCommonDlg::OnCancel)
END_MESSAGE_MAP()

/*========================================================
Method   : CHarddiskDlg::CHarddiskDlg
=========================================================*/
/* #FN#
   Standard constructor */
CHarddiskDlg::
CHarddiskDlg(
	CWnd *pParent /*=NULL*/ /* #IN# Pointer to the parent window */
)
	: CCommonDlg( CHarddiskDlg::IDD, pParent )
{
	//{{AFX_DATA_INIT(CHarddiskDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_pDriveData = s_aDriveData;
	m_nFirstCtrl = IDC_HARDDISK_FIRST;
	m_nLastCtrl  = IDC_HARDDISK_LAST;

} /* #OF# CHarddiskDlg::CHarddiskDlg */

/*========================================================
Method   : CHarddiskDlg::DoDataExchange
=========================================================*/
/* #FN#
   Dynamic Data Exchange (not used) */
void
/* #AS#
   Nothing */
CHarddiskDlg::
DoDataExchange(
	CDataExchange *pDX /* #IN# Pointer to CDataExchange object */
)
{
	CCommonDlg::DoDataExchange( pDX );
	//{{AFX_DATA_MAP(CHarddiskDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP

} /* #OF# CHarddiskDlg::DoDataExchange */


/////////////////////////////////////////////////////////////////////////////
// CHarddiskDlg implementation

/*========================================================
Method   : CHarddiskDlg::SetToDrive
=========================================================*/
/* #FN#
   Allows selecting a disk image for a given disk drive.
   Small window mode */
void
/* #AS#
   Nothing */
CHarddiskDlg::
SetToDrive(
	int nDriveIndex
)
{
	char   *pszDrivePath = m_pDriveData[ nDriveIndex ].szNewDir;
	CString	strPath = pszDrivePath;

	CFileSmallDlg dlgFileSmall( TRUE, NULL, NULL, strPath, TRUE, this );

	_CursorBusy();
	if( IDOK == dlgFileSmall.DoModal() )
	{
		strPath = dlgFileSmall.GetPathName();
		if( strPath.IsEmpty() )
			strPath = DEFAULT_HDD;
		strcpy( pszDrivePath, strPath );
		if( pszDrivePath[ strlen( pszDrivePath ) - 1 ] == '\\' )
			pszDrivePath[ strlen( pszDrivePath ) - 1 ] = '\0';

		SetDlgItemText( m_pDriveData[ nDriveIndex ].nCtrlID, pszDrivePath );
	}
	_CursorFree();

} /* #OF# CHarddiskDlg::SetToDrive */

/*========================================================
Method   : CHarddiskDlg::SetDlgState
=========================================================*/
/* #FN#
   Sets up the state of the dialog controls */
void
/* #AS#
   Nothing */
CHarddiskDlg::
SetDlgState()
{
#ifdef WIN_NETWORK_GAMES
	if( ST_KAILLERA_ACTIVE )
	{
		_RdOnlyEdit( IDC_HARDDISK_EDIT1,   TRUE  );
		_RdOnlyEdit( IDC_HARDDISK_EDIT2,   TRUE  );
		_RdOnlyEdit( IDC_HARDDISK_EDIT3,   TRUE  );
		_RdOnlyEdit( IDC_HARDDISK_EDIT4,   TRUE  );
		_EnableCtrl( IDC_HARDDISK_BUTTON1, FALSE );
		_EnableCtrl( IDC_HARDDISK_BUTTON2, FALSE );
		_EnableCtrl( IDC_HARDDISK_BUTTON3, FALSE );
		_EnableCtrl( IDC_HARDDISK_BUTTON4, FALSE );
		_RdOnlyEdit( IDC_HARDDISK_EXEPATH, TRUE  );
	}
#endif
} /* #OF# CHarddiskDlg::SetDlgState */


/////////////////////////////////////////////////////////////////////////////
// CHardDisk message handlers

/*========================================================
Method   : CHarddiskDlg::OnInitDialog
=========================================================*/
/* #FN#
   Performs special processing when the dialog box is initialized */
BOOL
/* #AS#
   TRUE unless you set the focus to a control */
CHarddiskDlg::
OnInitDialog()
{
	CCommonDlg::OnInitDialog();

	/* Backup hard disk paths */
	for( int i = 0; i < HD_DRIVES_NO; i++ )
	{
		_strncpy( m_pDriveData[ i ].szNewDir, m_pDriveData[ i ].pszDir, MAX_PATH );
		SetDlgItemText( m_pDriveData[ i ].nCtrlID, m_pDriveData[ i ].szNewDir );
	}
	/* Backup executables path */
	_strncpy( m_szHardExePath, g_szHardExePath, FILENAME_MAX - 1 );
	SetDlgItemText( IDC_HARDDISK_EXEPATH, m_szHardExePath );

	SetDlgState();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
} /* #OF# CHarddiskDlg::OnInitDialog */

/*========================================================
Method   : CHarddiskDlg::OnHd1
=========================================================*/
/* #FN#
   Called when first hard disk button is clicked by the user */
void
/* #AS#
   Nothing */
CHarddiskDlg::
OnHd1()
{
	SetToDrive( 0 );

} /* #OF# CHarddiskDlg::OnHd1 */

/*========================================================
Method   : CHarddiskDlg::OnHd2
=========================================================*/
/* #FN#
   Called when second hard disk button is clicked by the user */
void
/* #AS#
   Nothing */
CHarddiskDlg::
OnHd2()
{
	SetToDrive( 1 );

} /* #OF# CHarddiskDlg::OnHd2 */

/*========================================================
Method   : CHarddiskDlg::OnHd3
=========================================================*/
/* #FN#
   Called when third hard disk button is clicked by the user */
void
/* #AS#
   Nothing */
CHarddiskDlg::
OnHd3()
{
	SetToDrive( 2 );

} /* #OF# CHarddiskDlg::OnHd3 */

/*========================================================
Method   : CHarddiskDlg::OnHd4
=========================================================*/
/* #FN#
   Called when forth hard disk button is clicked by the user */
void
/* #AS#
   Nothing */
CHarddiskDlg::
OnHd4()
{
	SetToDrive( 3 );

} /* #OF# CHarddiskDlg::OnHd4 */

/*========================================================
Method   : CHarddiskDlg::KillfocusHdEdit
=========================================================*/
/* #FN#
   Stores a path to the pointed hard disk */
void
/* #AS#
   Nothing */
CHarddiskDlg::
KillfocusHdEdit(
	int nDriveIndex
)
{
	GetDlgItemText( m_pDriveData[ nDriveIndex ].nCtrlID, m_pDriveData[ nDriveIndex ].szNewDir, MAX_PATH );

} /* #OF# CHarddiskDlg::KillfocusHdEdit */

/*========================================================
Method   : CHarddiskDlg::OnKillfocusHd1Edit
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate edit control */
void
/* #AS#
   Nothing */
CHarddiskDlg::
OnKillfocusHd1Edit()
{
	KillfocusHdEdit( 0 );

} /* #OF# CHarddiskDlg::OnKillfocusHd1Edit */

/*========================================================
Method   : CHarddiskDlg::OnKillfocusHd2Edit
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate edit control */
void
/* #AS#
   Nothing */
CHarddiskDlg::
OnKillfocusHd2Edit()
{
	KillfocusHdEdit( 1 );

} /* #OF# CHarddiskDlg::OnKillfocusHd2Edit */

/*========================================================
Method   : CHarddiskDlg::OnKillfocusHd3Edit
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate edit control */
void
/* #AS#
   Nothing */
CHarddiskDlg::
OnKillfocusHd3Edit()
{
	KillfocusHdEdit( 2 );

} /* #OF# CHarddiskDlg::OnKillfocusHd3Edit */

/*========================================================
Method   : CHarddiskDlg::OnKillfocusHd4Edit
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate edit control */
void
/* #AS#
   Nothing */
CHarddiskDlg::
OnKillfocusHd4Edit()
{
	KillfocusHdEdit( 3 );

} /* #OF# CHarddiskDlg::OnKillfocusHd4Edit */

/*========================================================
Method   : CHarddiskDlg::OnKillfocusExePath
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate edit control */
void
/* #AS#
   Nothing */
CHarddiskDlg::OnKillfocusExePath() 
{
	GetDlgItemText( IDC_HARDDISK_EXEPATH, m_szHardExePath, FILENAME_MAX - 1 );

} /* #OF# CHarddiskDlg::OnKillfocusExePath */

/*========================================================
Method   : CHarddiskDlg::ReceiveFocused
=========================================================*/
/* #FN#
   Receives the edit controls content again. The user could press
   'Enter' or 'Alt-O' and then all changes he's made in the last
   edited control would be lost. */
void
/* #AS#
   Nothing */
CHarddiskDlg::
ReceiveFocused()
{
	CWnd *pWnd    = GetFocus();
	UINT  nCtrlID = pWnd ? pWnd->GetDlgCtrlID() : 0;

	if( IDC_HARDDISK_EXEPATH == nCtrlID )
	{
		OnKillfocusExePath();
	}
	else
	{
		for( int i = 0; i < HD_DRIVES_NO; i++ )
		{
			/* Receive the edit contents again. The user could press 'Enter' or 'Alt-O'
			   and then all changes he's made in the last edited control would be lost */
			if( m_pDriveData[ i ].nCtrlID == nCtrlID )
			{
				KillfocusHdEdit( i );
				break;
			}
		}
	}
} /* #OF# CHarddiskDlg::ReceiveFocused */

/*========================================================
Method   : CHarddiskDlg::OnOK
=========================================================*/
/* #FN#
   Called when the user clicks the OK button */
void
/* #AS#
   Nothing */
CHarddiskDlg::
OnOK()
{
	/* Unfortunately, edit controls do not lose the focus before
	   handling this when the user uses accelerators */
	ReceiveFocused();
	
	/* Hard disk paths */
	for( int i = 0; i < HD_DRIVES_NO; i++ )
	{
		if( *m_pDriveData[ i ].szNewDir == '\0' )
			strcpy( m_pDriveData[ i ].szNewDir, PATH_NONE );

		if( _stricmp( m_pDriveData[ i ].pszDir, m_pDriveData[ i ].szNewDir ) != 0 )
		{
			strcpy( m_pDriveData[ i ].pszDir, m_pDriveData[ i ].szNewDir );
			WriteRegString( NULL, m_pDriveData[ i ].pszRegName, m_pDriveData[ i ].pszDir );
		}
	}
	/* Executables path */
	if( *m_szHardExePath == '\0' )
		strcpy( m_szHardExePath, DEFAULT_H_PATH );

	if( _stricmp( g_szHardExePath, m_szHardExePath ) != 0 )
	{
		strcpy( g_szHardExePath, m_szHardExePath );
		WriteRegString( NULL, REG_HDE_PATH, g_szHardExePath );
	}
	CCommonDlg::OnOK();

} /* #OF# CHarddiskDlg::OnOK */
