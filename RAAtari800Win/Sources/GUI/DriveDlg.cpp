/****************************************************************************
File    : DriveDlg.cpp
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# CDriveDlg implementation file
@(#) #BY# Richard Lawrence, Tomasz Szymankowski
@(#) #LM# 28.12.2003
*/

#include "StdAfx.h"
#include "Atari800Win.h"
#include "Helpers.h"
#include "FileService.h"
#include "NewDiskImageDlg.h"
#include "DriveDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define CSD_READERROR		0
#define CSD_COMPRESSED		1
#define CSD_XFDREADWRITE	2
#define CSD_ATRREADONLY		3
#define CSD_ATRREADWRITE	4
#define CSD_PCREADONLY		5
#define CSD_DRIVEOFF		6
#define CSD_NODISKIMAGE		7

#define IDC_DRIVE_FIRST		IDC_DRIVE_BUTTON1
#define IDC_DRIVE_LAST		IDC_DRIVE_CANCEL


/////////////////////////////////////////////////////////////////////////////
// Static objects

static CDriveDlg::DiskData_t s_aDiskData[ SIO_MAX_DRIVES ] =
{
	{ SIO_filename[ 0 ], "Off", 0, 0, 0, SIO_OFF },
	{ SIO_filename[ 1 ], "Off", 0, 0, 0, SIO_OFF },
	{ SIO_filename[ 2 ], "Off", 0, 0, 0, SIO_OFF },
	{ SIO_filename[ 3 ], "Off", 0, 0, 0, SIO_OFF },
	{ SIO_filename[ 4 ], "Off", 0, 0, 0, SIO_OFF },
	{ SIO_filename[ 5 ], "Off", 0, 0, 0, SIO_OFF },
	{ SIO_filename[ 6 ], "Off", 0, 0, 0, SIO_OFF },
	{ SIO_filename[ 7 ], "Off", 0, 0, 0, SIO_OFF }
};


/////////////////////////////////////////////////////////////////////////////
// CDriveDlg dialog

BEGIN_MESSAGE_MAP(CDriveDlg, CCommonDlg)
	//{{AFX_MSG_MAP(CDriveDlg)
	ON_BN_CLICKED(IDC_DRIVE_CLEARALL, OnClearAll)
	ON_BN_CLICKED(IDC_DRIVE_NEWIMAGE, OnNewImage)
	ON_CBN_SELCHANGE(IDC_DRIVE_NUMBER, OnSelchangeNumber)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_DRIVE_OK, OnOK)
	ON_BN_CLICKED(IDC_DRIVE_CANCEL, CCommonDlg::OnCancel)
	ON_BN_CLICKED(IDC_DRIVE_READONLY, OnBnClickedDriveReadonly)
END_MESSAGE_MAP()

/*========================================================
Method   : CDriveDlg::CDriveDlg
=========================================================*/
/* #FN#
   Standard constructor */
CDriveDlg::
CDriveDlg(
	CWnd *pParent /*=NULL*/ /* #IN# Pointer to the parent window */
)
	: CCommonDlg( (g_Screen.ulState & SM_ATTR_SMALL_DLG ? IDD_DRIVE_SMALL : CDriveDlg::IDD), pParent )
{
	//{{AFX_DATA_INIT(CDriveDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_pDiskData  = s_aDiskData;
	m_nFirstCtrl = IDC_DRIVE_FIRST;
	m_nLastCtrl  = IDC_DRIVE_LAST;

} /* #OF# CDriveDlg::CDriveDlg */

/*========================================================
Method   : CDriveDlg::DoDataExchange
=========================================================*/
/* #FN#
   Dynamic Data Exchange (not used) */
void
/* #AS#
   Nothing */
CDriveDlg::
DoDataExchange(
	CDataExchange *pDX /* #IN# Pointer to CDataExchange object */
)
{
	CCommonDlg::DoDataExchange( pDX );
	//{{AFX_DATA_MAP(CDriveDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP

} /* #OF# CDriveDlg::DoDataExchange */


/////////////////////////////////////////////////////////////////////////////
// CDriveDlg implementation

/*========================================================
Method   : CDriveDlg::CheckSelectedDisk
=========================================================*/
/* #FN#
   Checks attributes of the pointed disk image */
SIO_UnitStatus
/* #AS#
   State of the tested disk image */
CDriveDlg::
CheckSelectedDisk(
	LPCSTR pszDiskName, /* #IN# Name of the disk image to check */
	UINT  *pInfo /*= NULL*/
)
{
	SIO_UnitStatus usResult = SIO_READ_ONLY;
	UINT unInfo = CSD_READERROR;

	if( strcmp( pszDiskName, "Off" ) == 0 )
	{
		usResult = SIO_OFF;
		unInfo = CSD_DRIVEOFF;
	}
	else
	{
		CFileStatus fsStatus;
		/* That's a static method, don't panic */
		if( CFile::GetStatus( pszDiskName, fsStatus ) )
		{
			if( !IsCompressedFile( pszDiskName ) )
			{
				if( !(fsStatus.m_attribute & CFile::readOnly) )
				{
					int fd = _open( pszDiskName, O_RDONLY | O_BINARY, 0777 );
					/* If there is an ATR image check header's writeprotect member */
					if( fd >= 0 )
					{
						AFILE_ATR_Header header;
						_lseek( fd, 0L, SEEK_SET );
						if( (-1 != _read( fd, &header, sizeof(AFILE_ATR_Header) )) &&
							header.magic1 == AFILE_ATR_MAGIC1 && header.magic2 == AFILE_ATR_MAGIC2 )
						{
							if( !header.writeprotect )
							{
								unInfo = CSD_ATRREADWRITE;
								if ( !_IsFlagSet( g_Misc.ulState, MS_DRIVE_READONLY) )
									usResult = SIO_READ_WRITE;
							}
							else
								unInfo = CSD_ATRREADONLY;
						}
						else
						{
							if (!_IsFlagSet(g_Misc.ulState, MS_DRIVE_READONLY))
								usResult = SIO_READ_WRITE;
							unInfo = CSD_XFDREADWRITE;
						}
						_close( fd );
					}
				}
				else
					unInfo = CSD_PCREADONLY;
			}
			else
				unInfo = CSD_COMPRESSED;
		}
		else
		{
			usResult = SIO_NO_DISK;
			unInfo = CSD_NODISKIMAGE;
		}
	}
	if( pInfo )
		*pInfo = unInfo;

	return usResult;

} /* #OF# CDriveDlg::CheckSelectedDisk */

/*========================================================
Method   : CDriveDlg::GetDiskImage
=========================================================*/
/* #FN#
   Allows selecting a disk image for the pointed disk drive */
BOOL
/* #AS#
   Nothing */
CDriveDlg::
GetDiskImage(
	int nDrive /* #IN# Number of the disk drive an image is selected for */
)
{
	char    szDiskName[ MAX_PATH + 1 ];
	CString strPrompt;
	BOOL	bResult = FALSE;

	/* Get the most recently used file name to use in a file dialog */
	_strncpy( szDiskName, m_pDiskData[ nDrive ].szNewName, MAX_PATH );
	strPrompt.Format( IDS_SELECT_DSK_LOAD, nDrive + 1 );

	if( PickFileName( TRUE, szDiskName, strPrompt, IDS_FILTER_DSK,
					  NULL, PF_LOAD_FLAGS, TRUE, FILE_NONE, this ) &&
		*szDiskName != '\0' )
	{
		SetDlgItemText( m_pDiskData[ nDrive ].nEditID, szDiskName );
		_strncpy( m_pDiskData[ nDrive ].szNewName, szDiskName, MAX_PATH );
		m_pDiskData[ nDrive ].usStatus = CheckSelectedDisk( szDiskName );

		bResult = TRUE;
	}
	return bResult;

} /* #OF# CDriveDlg::GetDiskImage */

/*========================================================
Method   : CDriveDlg::SetDlgState
=========================================================*/
/* #FN#
   We set the drive edit boxes to the the appropriate values,
   and set the status ComboBox */
void
/* #AS#
   Nothing */
CDriveDlg::
SetDlgState()
{
	CComboBox *pCombo = NULL;
	CEdit     *pEdit  = NULL;
	int        nDrive = 0;

	_SetChkBttn( IDC_DRIVE_READONLY, _IsFlagSet( g_Misc.ulState, MS_DRIVE_READONLY) );

	if( m_bSmallMode )
	{
		nDrive = _GetSelCbox( IDC_DRIVE_NUMBER );
	}
	for( int i = 0; i < SIO_MAX_DRIVES; i++ )
	{
		if( !m_bSmallMode || (m_bSmallMode && i == nDrive ) )
		{
			/* The szNewName field is not really needed */
			SetDlgItemText( m_pDiskData[ i ].nEditID, m_pDiskData[ i ].szNewName );

			pCombo = (CComboBox *)GetDlgItem( m_pDiskData[ i ].nComboID );
			ASSERT(NULL != pCombo);
			pEdit = (CEdit *)GetDlgItem( m_pDiskData[ i ].nEditID );
			ASSERT(NULL != pEdit);
			
			switch( m_pDiskData[ i ].usStatus )
			{
				case SIO_OFF:
					pCombo->SetCurSel( 0 );
					pEdit->SetReadOnly();
					break;

				case SIO_READ_WRITE:
					pCombo->SetCurSel( 1 );
					pEdit->SetReadOnly( FALSE );
					break;

				case SIO_READ_ONLY:
					pCombo->SetCurSel( 2 );
					pEdit->SetReadOnly( FALSE );
					break;

				case SIO_NO_DISK:
					pCombo->SetCurSel( 3 );
					pEdit->SetReadOnly( FALSE );
					break;
			}
		}
	}
#ifdef WIN_NETWORK_GAMES
	if( ST_KAILLERA_ACTIVE )
	{
		if( m_bSmallMode )
		{
			if( 0 == nDrive )
			{
				_RdOnlyEdit( IDC_DRIVE_EDIT,   TRUE  );
				_EnableCtrl( IDC_DRIVE_BUTTON, FALSE );
				_EnableCtrl( IDC_DRIVE_COMBO,  FALSE );
			}
		}
		else
		{
			_RdOnlyEdit( IDC_DRIVE_EDIT1,   TRUE  );
			_EnableCtrl( IDC_DRIVE_BUTTON1, FALSE );
			_EnableCtrl( IDC_DRIVE_COMBO1,  FALSE );
		}
		_EnableCtrl( IDC_DRIVE_CLEARALL, FALSE );
	}
#endif
} /* #OF# CDriveDlg::SetDlgState */


/////////////////////////////////////////////////////////////////////////////
// CDriveDlg message handlers

/*========================================================
Method   : CDriveDlg::OnInitDialog
=========================================================*/
/* #FN#
   Performs special processing when the dialog box is initialized */
BOOL
/* #AS#
   TRUE unless you set the focus to a control */
CDriveDlg::
OnInitDialog()
{
	CCommonDlg::OnInitDialog();

	const UINT anCtrlsLarge[ SIO_MAX_DRIVES ][ 3 ] =
	{
		{ IDC_DRIVE_BUTTON1, IDC_DRIVE_EDIT1, IDC_DRIVE_COMBO1 },
		{ IDC_DRIVE_BUTTON2, IDC_DRIVE_EDIT2, IDC_DRIVE_COMBO2 },
		{ IDC_DRIVE_BUTTON3, IDC_DRIVE_EDIT3, IDC_DRIVE_COMBO3 },
		{ IDC_DRIVE_BUTTON4, IDC_DRIVE_EDIT4, IDC_DRIVE_COMBO4 },
		{ IDC_DRIVE_BUTTON5, IDC_DRIVE_EDIT5, IDC_DRIVE_COMBO5 },
		{ IDC_DRIVE_BUTTON6, IDC_DRIVE_EDIT6, IDC_DRIVE_COMBO6 },
		{ IDC_DRIVE_BUTTON7, IDC_DRIVE_EDIT7, IDC_DRIVE_COMBO7 },
		{ IDC_DRIVE_BUTTON8, IDC_DRIVE_EDIT8, IDC_DRIVE_COMBO8 }
	};

	m_bSmallMode = g_Screen.ulState & SM_ATTR_SMALL_DLG;

	for( int i = 0; i < SIO_MAX_DRIVES; i++ )
	{
		/* Set the appropriate controls IDs */
		m_pDiskData[ i ].nButtonID = (m_bSmallMode ? IDC_DRIVE_BUTTON : anCtrlsLarge[ i ][ 0 ]);
		m_pDiskData[ i ].nEditID   = (m_bSmallMode ? IDC_DRIVE_EDIT   : anCtrlsLarge[ i ][ 1 ]);
		m_pDiskData[ i ].nComboID  = (m_bSmallMode ? IDC_DRIVE_COMBO  : anCtrlsLarge[ i ][ 2 ]);
		/* Backup drives paths */
		_strncpy( m_pDiskData[ i ].szNewName, m_pDiskData[ i ].pszName, MAX_PATH );
		m_pDiskData[ i ].usStatus = SIO_drive_status[ i ];
	}
	if( m_bSmallMode )
	{
		_SetSelCbox( IDC_DRIVE_NUMBER, 0 );
	}
	SetDlgState();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
} /* #OF# CDriveDlg::OnInitDialog */

/*========================================================
Method   : CDriveDlg::DriveButton
=========================================================*/
/* #FN#
   Called when a drive button is pressed */
BOOL
/* #AS#
   TRUE if succeeded, otherwise FALSE */
CDriveDlg::
DriveButton(
	UINT nButtonID, /* #IN# Resource ID of a drive button control */
	HWND /*hwndButton*/
)
{
	int nDrive = -1;

	if( m_bSmallMode )
	{
		if( nButtonID == IDC_DRIVE_BUTTON )
		{
			nDrive = _GetSelCbox( IDC_DRIVE_NUMBER );
		}
	}
	else
	{
		for( int i = 0; i < SIO_MAX_DRIVES; i++ )
		{
			if( nButtonID == m_pDiskData[ i ].nButtonID ) 
			{
				nDrive = i;
				break;
			}
		}
	}
	if( -1 == nDrive || CB_ERR == nDrive ) /* It wasn't a drive button */
		return FALSE;

	if( GetDiskImage( nDrive ) )
		SetDlgState();

	return TRUE;

} /* #OF# CDriveDlg::DriveButton */

/*========================================================
Method   : CDriveDlg::StatusSelChange
=========================================================*/
/* #FN#
   Called when a drive combo box item is selected */
BOOL
/* #AS#
   TRUE if succeeded, otherwise FALSE */
CDriveDlg::
StatusSelChange(
	UINT nComboID, /* #IN# Resource ID of a drive combo box control */
	HWND /*hwndCombo*/
)
{
	UINT       unDiskInf =  0;
	SIO_UnitStatus usRealMode;
	int        nDrive    = -1;

	if( m_bSmallMode )
	{
		if( nComboID == IDC_DRIVE_COMBO )
		{
			nDrive = _GetSelCbox( IDC_DRIVE_NUMBER );
		}
	}
	else
	{
		for( int i = 0; i < SIO_MAX_DRIVES; i++ )
		{
			if( nComboID == m_pDiskData[ i ].nComboID )
			{
				nDrive = i;
				break;
			}
		}
	}
	if( -1 == nDrive || CB_ERR == nDrive ) /* It wasn't a drive button */
		return FALSE;

	usRealMode = CheckSelectedDisk( m_pDiskData[ nDrive ].szNewName, &unDiskInf );

	switch( _GetSelCbox( m_pDiskData[ nDrive ].nComboID ) )
	{
		case 0: /* Combo box indicates "Off" */
		{
			m_pDiskData[ nDrive ].usStatus = SIO_OFF;
			break;
		}
		case 1: /* Combo box indicates "Read/Write" */
		{
			if( usRealMode == SIO_READ_ONLY )
			{
				switch( unDiskInf )
				{
					case CSD_COMPRESSED:
						DisplayMessage( GetSafeHwnd(), IDS_DRIVE_MSG1, 0, MB_ICONINFORMATION | MB_OK );
						break;

					case CSD_ATRREADONLY:
						if( IDYES == DisplayMessage( GetSafeHwnd(), IDS_DRIVE_MSG2, 0, MB_ICONQUESTION | MB_YESNO ) )
						{
							int fd = _open( m_pDiskData[ nDrive ].szNewName, O_WRONLY | O_BINARY, 0777 );
							if( fd >= 0 )
							{
								if( -1 != _lseek( fd, 15L, SEEK_SET ) &&
									-1 != _write( fd, "\000", 1 ) )
								{
									usRealMode = SIO_READ_WRITE;
								}
								_close( fd );
							}
						}
						break;

					case CSD_PCREADONLY:
						DisplayMessage( GetSafeHwnd(), IDS_DRIVE_MSG3, 0, MB_ICONINFORMATION | MB_OK );
						break;

					default:
						usRealMode = SIO_READ_WRITE;
				}
			}
			m_pDiskData[ nDrive ].usStatus = usRealMode;
			break;
		}
		case 2: /* Combo box indicates "Read Only" */
		{
			if( CSD_ATRREADWRITE == unDiskInf )
			{
				if( IDYES == DisplayMessage( GetSafeHwnd(), IDS_DRIVE_MSG4, 0, MB_ICONQUESTION | MB_YESNO ) )
				{
					int fd = _open( m_pDiskData[ nDrive ].szNewName, O_WRONLY | O_BINARY, 0777 );
					if( fd >= 0 )
					{
						if( -1 != _lseek( fd, 15L, SEEK_SET ) )
						{
							_write( fd, "\001", 1 );
						}
						_close( fd );
					}
				}
				usRealMode = SIO_READ_ONLY;
			}
			else
				DisplayMessage( GetSafeHwnd(), IDS_DRIVE_MSG5, 0, MB_ICONINFORMATION | MB_OK );

			m_pDiskData[ nDrive ].usStatus = usRealMode;
			break;
		}
		case 3: /* Combo box indicates "No Disk" */
		{
			m_pDiskData[ nDrive ].usStatus = SIO_NO_DISK;
			break;
		}
	}
	SetDlgState();

	return TRUE;

} /* #OF# CDriveDlg::StatusSelChange */

/*========================================================
Method   : CDriveDlg::KillfocusEditDrive
=========================================================*/
/* #FN#
   Called when a drive edit control text is changed */
BOOL
/* #AS#
   TRUE if succeeded, otherwise FALSE */
CDriveDlg::
KillfocusEditDrive(
	UINT nEditID, /* #IN# Resource ID of a drive edit control */
	HWND /*hwndEdit*/
)
{
	char szDiskName[ MAX_PATH + 1 ];
	int  nDrive = -1;

	if( m_bSmallMode )
	{
		if( nEditID == IDC_DRIVE_EDIT )
		{
			nDrive = _GetSelCbox( IDC_DRIVE_NUMBER );
		}
	}
	else
	{
		for( int i = 0; i < SIO_MAX_DRIVES; i++ )
		{
			if( nEditID == m_pDiskData[ i ].nEditID )
			{
				nDrive = i;
				break;
			}
		}
	}
	if( -1 == nDrive || CB_ERR == nDrive ) /* It wasn't a drive edit */
		return FALSE;

	GetDlgItemText( m_pDiskData[ nDrive ].nEditID, szDiskName, MAX_PATH );
	strcpy( m_pDiskData[ nDrive ].szNewName, szDiskName );
	m_pDiskData[ nDrive ].usStatus = CheckSelectedDisk( szDiskName );

	SetDlgState();

	return TRUE;

} /* #OF# CDriveDlg::KillfocusEditDrive */

/*========================================================
Method   : CDriveDlg::OnCommand
=========================================================*/
/* #FN#
   The framework calls this member function when the user selects
   an item from a menu, when a child control sends a notification
   message, or when an accelerator keystroke is translated */
BOOL
/* #AS#
   Nonzero if the application processes this message; otherwise 0 */
CDriveDlg::
OnCommand(
	WPARAM wParam,
	LPARAM lParam
)
{
	/* This could also be handled by eight OnSelChanged plus eight
	   ButtonClicked individual procedures, but this seems a little
	   more elegant */
	switch( HIWORD(wParam) )
	{
		case BN_CLICKED:
		{
			if( DriveButton( (UINT)LOWORD(wParam), (HWND)lParam ) )
				return TRUE;
			break;
		}
		case EN_KILLFOCUS:
		{
			if( KillfocusEditDrive( (UINT)LOWORD(wParam), (HWND)lParam ) )
				return TRUE;
			break;
		}
		case CBN_SELCHANGE:
		{
			if( StatusSelChange( (UINT)LOWORD(wParam), (HWND)lParam ) )
				return TRUE;
			break;
		}
	}
	return CCommonDlg::OnCommand( wParam, lParam );

} /* #OF# CDriveDlg::OnCommand */

/*========================================================
Method   : CDriveDlg::OnClearAll
=========================================================*/
/* #FN#
   Removes disks from all drives and turns them off */
void
/* #AS#
   Nothing */
CDriveDlg::
OnClearAll()
{
	for( int i = 0; i < SIO_MAX_DRIVES; i++ )
	{
		m_pDiskData[ i ].usStatus = SIO_OFF;
		strcpy( m_pDiskData[ i ].szNewName, "Off" );
	}
	SetDlgState();

} /* #OF# CDriveDlg::OnClearAll */

/*========================================================
Method   : CDriveDlg::OnNewImage
=========================================================*/
/* #FN#
   Displays New Disk Image dialog box */
void
/* #AS#
   Nothing */
CDriveDlg::
OnNewImage()
{
	CNewDiskImageDlg dlgNewDiskImage;

	if( IDOK == dlgNewDiskImage.DoModal() &&
		/* Attach disk image to any drive if it was expected */
		dlgNewDiskImage.m_bAttachDisk )
	{
		_strncpy( m_pDiskData[ dlgNewDiskImage.m_nDriveNumber ].szNewName, dlgNewDiskImage.m_szDiskName, MAX_PATH );
		m_pDiskData[ dlgNewDiskImage.m_nDriveNumber ].usStatus = CheckSelectedDisk( dlgNewDiskImage.m_szDiskName );
	}
	SetDlgState();

} /* #OF# CDriveDlg::OnNewImage */

/*========================================================
Method   : CDriveDlg::OnSelchangeNumber
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate combo box */
void
/* #AS#
   Nothing */
CDriveDlg::
OnSelchangeNumber()
{
	SetDlgState();

} /* #OF# CDriveDlg::OnSelchangeNumber */

/*========================================================
Method   : CDriveDlg::ReceiveFocused
=========================================================*/
/* #FN#
   Receives the edit controls content again. The user could press
   'Enter' or 'Alt-O' and then all changes he's made in the last
   edited control would be lost. */
void
/* #AS#
   Nothing */
CDriveDlg::
ReceiveFocused()
{
	CWnd *pWnd    = GetFocus();
	UINT  nCtrlID = pWnd ? pWnd->GetDlgCtrlID() : 0;

	for( int i = 0; i < SIO_MAX_DRIVES; i++ )
	{
		/* Receive the edit contents again. A user could use 'Enter' or 'Alt-O'
		   and then all changes made in the last edited control would be lost */
		if( m_pDiskData[ i ].nEditID == nCtrlID )
		{
			KillfocusEditDrive( nCtrlID, (HWND)NULL );
			break;
		}
	}
} /* #OF# CDriveDlg::ReceiveFocused */

/*========================================================
Method   : CDriveDlg::OnOK
=========================================================*/
/* #FN#
   Called when the user clicks the OK button */
void
/* #AS#
   Nothing */
CDriveDlg::
OnOK()
{
	char szFileName[ MAX_PATH + 1 ];
	BOOL bChanged = FALSE;

	/* Unfortunately, edit controls do not lose the focus before
	   handling this when the user uses accelerators */
	ReceiveFocused();

	for( int i = 0; i < SIO_MAX_DRIVES; i++ )
	{
		if( m_pDiskData[ i ].usStatus == SIO_OFF )
		{
			strcpy( szFileName, "Off" );
		}
		else if( m_pDiskData[ i ].usStatus == SIO_NO_DISK )
		{
			strcpy( szFileName, "Empty" );
		}
		else
			strcpy( szFileName, m_pDiskData[ i ].szNewName );

		/* The reason for doing this here instead of just doing it in
		   GetDiskImage is to preserve the ability to cancel the entire
		   drive operation with one click of the cancel button. */
		if( _stricmp( szFileName, m_pDiskData[ i ].pszName ) != 0 )
		{
			SIO_Dismount( i + 1 );
			strcpy( m_pDiskData[ i ].pszName, szFileName );

			if( strcmp( szFileName, "Empty" ) != 0 &&
				strcmp( szFileName, "Off" ) != 0 )
			{
				if( !SIO_Mount( i + 1, szFileName, m_pDiskData[ i ].usStatus == SIO_READ_ONLY ) )
				{
					/* There was an error with SIO mounting */
					strcpy( m_pDiskData[ i ].pszName, "Empty" );
					m_pDiskData[ i ].usStatus = SIO_NO_DISK;
				}
			}
			bChanged = TRUE;
		}
		if( m_pDiskData[ i ].usStatus != SIO_drive_status[ i ] )
		{
			SIO_drive_status[ i ] = m_pDiskData[ i ].usStatus;
			bChanged = TRUE;
		}
	}
	if( bChanged )
		WriteRegDrives( NULL );

	CCommonDlg::OnOK();

} /* #OF# CDriveDlg::OnOK */

void CDriveDlg::OnBnClickedDriveReadonly()
{
	_ModifyFlag( _GetChkBttn( IDC_DRIVE_READONLY ), g_Misc.ulState, MS_DRIVE_READONLY);
}
