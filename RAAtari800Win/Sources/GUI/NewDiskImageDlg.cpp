/****************************************************************************
File    : NewDiskImageDlg.cpp
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# CNewDiskImageDlg implementation file
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 28.12.2003
*/

#include "StdAfx.h"
#include "Atari800Win.h"
#include "Helpers.h"
#include "FileService.h"
#include "NewDiskImageDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define IDC_NEWDISKIMAGE_FIRST		IDC_NEWDISKIMAGE_90K
#define IDC_NEWDISKIMAGE_LAST		IDC_NEWDISKIMAGE_CANCEL

#define IMAGE_FORMATS_NO			4
#define MAX_SECTORS_NO				65535


/////////////////////////////////////////////////////////////////////////////
// CNewDiskImageDlg dialog

BEGIN_MESSAGE_MAP(CNewDiskImageDlg, CCommonDlg)
	//{{AFX_MSG_MAP(CNewDiskImageDlg)
	ON_NOTIFY(UDN_DELTAPOS, IDC_NEWDISKIMAGE_SECTORSPIN, OnDeltaposSectorSpin)
	ON_EN_KILLFOCUS(IDC_NEWDISKIMAGE_SECTORS, OnKillfocusSectors)
	ON_BN_CLICKED(IDC_NEWDISKIMAGE_90K, OnNewDiskImage90k)
	ON_BN_CLICKED(IDC_NEWDISKIMAGE_130K, OnNewDiskImage130k)
	ON_BN_CLICKED(IDC_NEWDISKIMAGE_180K, OnNewDiskImage180k)
	ON_BN_CLICKED(IDC_NEWDISKIMAGE_OTHER, OnNewDiskImageOther)
	ON_BN_CLICKED(IDC_NEWDISKIMAGE_ATTACHDISK_CHECK, OnAttachDiskCheck)
	ON_CBN_SELCHANGE(IDC_NEWDISKIMAGE_ATTACHDISK_COMBO, OnSelchangeAttachDiskCombo)
	ON_CBN_SELCHANGE(IDC_NEWDISKIMAGE_BYTESPERSEC, OnSelchangeBytesPerSector)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_NEWDISKIMAGE_OK, OnOK)
	ON_BN_CLICKED(IDC_NEWDISKIMAGE_CANCEL, CCommonDlg::OnCancel)
END_MESSAGE_MAP()

/*========================================================
Method   : CNewDiskImageDlg::CNewDiskImageDlg
=========================================================*/
/* #FN#
   Standard constructor */
CNewDiskImageDlg::
CNewDiskImageDlg(
	CWnd *pParent /*=NULL*/ /* #IN# Pointer to the parent window */
)
	: CCommonDlg( CNewDiskImageDlg::IDD, pParent )
{
	//{{AFX_DATA_INIT(CNewDiskImageDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_nFirstCtrl = IDC_NEWDISKIMAGE_FIRST;
	m_nLastCtrl  = IDC_NEWDISKIMAGE_LAST;

} /* #OF# CNewDiskImageDlg::CNewDiskImageDlg */

/*========================================================
Method   : CNewDiskImageDlg::DoDataExchange
=========================================================*/
/* #FN#
   Dynamic Data Exchange (not used) */
void
/* #AS#
   Nothing */
CNewDiskImageDlg::
DoDataExchange(
	CDataExchange *pDX /* #IN# Pointer to CDataExchange object */
)
{
	CCommonDlg::DoDataExchange( pDX );
	//{{AFX_DATA_MAP(CNewDiskImageDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP

} /* #OF# CNewDiskImageDlg::DoDataExchange */


/////////////////////////////////////////////////////////////////////////////
// CNewDiskImageDlg implementation

/*========================================================
Method   : CNewDiskImageDlg::SetDlgState
=========================================================*/
/* #FN#
   Sets up the state of the dialog controls */
void
/* #AS#
   Nothing */
CNewDiskImageDlg::
SetDlgState()
{
	BOOL abEnabled[][ IMAGE_FORMATS_NO ] =
	{
		{ FALSE, FALSE, TRUE,  TRUE },
		{ FALSE, FALSE, FALSE, TRUE },
		{ FALSE, FALSE, FALSE, TRUE }
	};
	UINT anContent[][ IMAGE_FORMATS_NO ] =
	{
		{ 0/*128*/,  0/*128*/, 0/*128*/ },
		{ 0/*128*/,  0/*128*/, 1/*256*/ },
		{ 720,       1040,     720      }
	};

	CheckRadioButton( IDC_NEWDISKIMAGE_90K, IDC_NEWDISKIMAGE_OTHER,
					  IDC_NEWDISKIMAGE_90K + m_nFormat );

	/* Set up activity of the controls */
	_EnableCtrl( IDC_NEWDISKIMAGE_BYTESINBOOT,       abEnabled[ 0 ][ m_nFormat ] );
	_EnableCtrl( IDC_NEWDISKIMAGE_BYTESINBOOT_LABEL, abEnabled[ 0 ][ m_nFormat ] );
	_EnableCtrl( IDC_NEWDISKIMAGE_BYTESPERSEC,       abEnabled[ 1 ][ m_nFormat ] );
	_EnableCtrl( IDC_NEWDISKIMAGE_BYTESPERSEC_LABEL, abEnabled[ 1 ][ m_nFormat ] );
	_EnableCtrl( IDC_NEWDISKIMAGE_SECTORS,           abEnabled[ 2 ][ m_nFormat ] );
	/* There is the spin/label not enabled/disabled yet */
	_EnableCtrl( IDC_NEWDISKIMAGE_SECTORS_LABEL,     abEnabled[ 2 ][ m_nFormat ] );
	_EnableCtrl( IDC_NEWDISKIMAGE_SECTORSPIN,        abEnabled[ 2 ][ m_nFormat ] );

	/* Set up parameters */
	if( IMAGE_FORMATS_NO - 1 > m_nFormat )
	{
		_SetSelCbox( IDC_NEWDISKIMAGE_BYTESINBOOT, anContent[ 0 ][ m_nFormat ] );
		_SetSelCbox( IDC_NEWDISKIMAGE_BYTESPERSEC, anContent[ 1 ][ m_nFormat ] );

		m_unSectors = anContent[ 2 ][ m_nFormat ];
		SetDlgItemInt( IDC_NEWDISKIMAGE_SECTORS, m_unSectors, FALSE );
	}
	else
	{
		int nSelect = _GetSelCbox( IDC_NEWDISKIMAGE_BYTESPERSEC );
		if( CB_ERR != nSelect )
		{
			/* Verify state of Bytes In Boot combo */
			_SetSelCbox( IDC_NEWDISKIMAGE_BYTESINBOOT,       0 /*nSelect*/ ); /* Always suggest 128 */
			_EnableCtrl( IDC_NEWDISKIMAGE_BYTESINBOOT,       (BOOL)nSelect );
			_EnableCtrl( IDC_NEWDISKIMAGE_BYTESINBOOT_LABEL, (BOOL)nSelect );
		}
	}
	/* Set up "attach to drive" group */
	_SetSelCbox( IDC_NEWDISKIMAGE_ATTACHDISK_COMBO, m_nDriveNumber );
	_EnableCtrl( IDC_NEWDISKIMAGE_ATTACHDISK_COMBO, m_bAttachDisk );
	_SetChkBttn( IDC_NEWDISKIMAGE_ATTACHDISK_CHECK, m_bAttachDisk );

#ifdef WIN_NETWORK_GAMES
	if( ST_KAILLERA_ACTIVE )
	{
		_EnableCtrl( IDC_NEWDISKIMAGE_ATTACHDISK_CHECK, FALSE );
	}
#endif
} /* #OF# CNewDiskImageDlg::SetDlgState */


/////////////////////////////////////////////////////////////////////////////
// CNewDiskImageDlg message handlers

/*========================================================
Method   : CNewDiskImageDlg::OnInitDialog
=========================================================*/
/* #FN#
   Performs special processing when the dialog box is initialized */
BOOL
/* #AS#
   TRUE unless you set the focus to a control */
CNewDiskImageDlg::
OnInitDialog()
{
	CCommonDlg::OnInitDialog();

	strcpy( m_szDiskName, "Empty" );
	m_nFormat      = 0;
	m_nDriveNumber = 1;

#ifdef WIN_NETWORK_GAMES
	if( ST_KAILLERA_ACTIVE )
	{
		m_bAttachDisk = FALSE;
	}
	else
#endif
		m_bAttachDisk = TRUE;

	SetDlgState();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
} /* #OF# CNewDiskImageDlg::OnInitDialog */

/*========================================================
Method   : CNewDiskImageDlg::OnDeltaposSectorSpin
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate spin control */
void
/* #AS#
   Nothing */
CNewDiskImageDlg::
OnDeltaposSectorSpin(
	NMHDR   *pNMHDR, /* #IN#  */
	LRESULT *pResult /* #OUT# */
)
{
	_DeltaposSpin( pNMHDR, IDC_NEWDISKIMAGE_SECTORS, m_unSectors, 1, MAX_SECTORS_NO );
	*pResult = 0;

} /* #OF# CNewDiskImageDlg::OnDeltaposSectorSpin */

/*========================================================
Method   : CNewDiskImageDlg::OnKillfocusSectors
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate spin control */
void
/* #AS#
   Nothing */
CNewDiskImageDlg::
OnKillfocusSectors()
{
	_KillfocusSpin( IDC_NEWDISKIMAGE_SECTORS, m_unSectors, 1, MAX_SECTORS_NO );

} /* #OF# CNewDiskImageDlg::OnKillfocusSectors */

/*========================================================
Method   : CNewDiskImageDlg::OnNewDiskImage90k
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate radio button */
void
/* #AS#
   Nothing */
CNewDiskImageDlg::
OnNewDiskImage90k()
{
	m_nFormat = 0;
	SetDlgState();

} /* #OF# CNewDiskImageDlg::OnNewDiskImage90k */

/*========================================================
Method   : CNewDiskImageDlg::OnNewDiskImage130k
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate radio button */
void
/* #AS#
   Nothing */
CNewDiskImageDlg::
OnNewDiskImage130k()
{
	m_nFormat = 1;
	SetDlgState();

} /* #OF# CNewDiskImageDlg::OnNewDiskImage130k */

/*========================================================
Method   : CNewDiskImageDlg::OnNewDiskImage180k
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate radio button */
void
/* #AS#
   Nothing */
CNewDiskImageDlg::
OnNewDiskImage180k()
{
	m_nFormat = 2;
	SetDlgState();

} /* #OF# CNewDiskImageDlg::OnNewDiskImage180k */

/*========================================================
Method   : CNewDiskImageDlg::OnNewDiskImageOther
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate radio button */
void
/* #AS#
   Nothing */
CNewDiskImageDlg::
OnNewDiskImageOther()
{
	m_nFormat = 3;
	SetDlgState();

} /* #OF# CNewDiskImageDlg::OnNewDiskImageOther */

/*========================================================
Method   : CNewDiskImageDlg::OnSelchangeBytesPerSector
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate combo box */
void
/* #AS#
   Nothing */
CNewDiskImageDlg::
OnSelchangeBytesPerSector()
{
	SetDlgState();

} /* #OF# CNewDiskImageDlg::OnSelchangeBytesPerSector */

/*========================================================
Method   : CNewDiskImageDlg::OnAttachDiskCheck
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate check box */
void
/* #AS#
   Nothing */
CNewDiskImageDlg::
OnAttachDiskCheck()
{
	_EnableCtrl( IDC_NEWDISKIMAGE_ATTACHDISK_COMBO, m_bAttachDisk = _GetChkBttn( IDC_NEWDISKIMAGE_ATTACHDISK_CHECK ) );

} /* #OF# CNewDiskImageDlg::OnAttachDiskCheck */

/*========================================================
Method   : CNewDiskImageDlg::OnSelchangeAttachDiskCombo
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate combo box */
void
/* #AS#
   Nothing */
CNewDiskImageDlg::
OnSelchangeAttachDiskCombo() 
{
	m_nDriveNumber = _GetSelCbox( IDC_NEWDISKIMAGE_ATTACHDISK_COMBO );

} /* #OF# CNewDiskImageDlg::OnSelchangeAttachDiskCombo */

/*========================================================
Method   : CNewDiskImageDlg::ReceiveFocused
=========================================================*/
/* #FN#
   Receives the edit controls content again. The user could press
   'Enter' or 'Alt-O' and then all changes he's made in the last
   edited control would be lost. */
void
/* #AS#
   Nothing */
CNewDiskImageDlg::
ReceiveFocused()
{
	CWnd *pWnd    = GetFocus();
	UINT  nCtrlID = pWnd ? pWnd->GetDlgCtrlID() : 0;

	if( IDC_NEWDISKIMAGE_SECTORS == nCtrlID )
	{
		OnKillfocusSectors();
	}
} /* #OF# CNewDiskImageDlg::ReceiveFocused */

/*========================================================
Method   : CNewDiskImageDlg::OnOK
=========================================================*/
/* #FN#
   Called when the user clicks the OK button */
void
/* #AS#
   Nothing */
CNewDiskImageDlg::
OnOK()
{
	/* Unfortunately, edit controls do not lose the focus before
	   handling this when the user uses accelerators */
	ReceiveFocused();

	if( PickFileName( FALSE, m_szDiskName, IDS_SELECT_DSK_SAVE, IDS_FILTER_ATR_XFD,
					  "atr", PF_SAVE_FLAGS, FALSE, FILE_NONE, this ) &&
		*m_szDiskName != '\0' )
	{
		UINT  unBytesInBootSec;
		UINT  unBytesPerSector;
		UINT  unSectors;
		UINT  unImageLen;
		BOOL  bTrans;
		FILE *pfImage = NULL;
		char  szPostfix[ 5 ];

		/* Receive the parameters */
		unBytesInBootSec = (_GetSelCbox( IDC_NEWDISKIMAGE_BYTESINBOOT ) + 1) * 128;
		unBytesPerSector = (_GetSelCbox( IDC_NEWDISKIMAGE_BYTESPERSEC ) + 1) * 128;

		unSectors = GetDlgItemInt( IDC_NEWDISKIMAGE_SECTORS, &bTrans, FALSE );
		if( !bTrans )
		{
			Log_print( "Error while receiving content of dialog controls" );
			return;
		}
		/* Image length divided by 16 */
		if( unSectors <= 3 )
			unImageLen = unSectors * unBytesInBootSec / 16;
		else
			unImageLen = ((unSectors - 3) * unBytesPerSector + 3 * unBytesInBootSec) / 16;

		/* Open the image file */
		if( NULL == (pfImage = fopen( m_szDiskName, "wb" )) )
		{
			DisplayMessage( GetSafeHwnd(), IDS_ERROR_FILE_OPEN, 0, MB_ICONEXCLAMATION | MB_OK, m_szDiskName );
			return;
		}

		_strncpy( szPostfix, &m_szDiskName[ strlen( m_szDiskName ) - 4 ], 5 );
		if( _stricmp( szPostfix, ".atr" ) == 0 )
		{
			/* There is the definition of AFILE_ATR_Header structure in atari.h */
			struct AFILE_ATR_Header atrHeader;

			ZeroMemory( &atrHeader, sizeof(AFILE_ATR_Header) );
			atrHeader.magic1       = AFILE_ATR_MAGIC1;
			atrHeader.magic2       = AFILE_ATR_MAGIC2;
			atrHeader.secsizelo    =  unBytesPerSector   & 0x00ff;
			atrHeader.secsizehi    = (unBytesPerSector   & 0xff00) >> 8;
			atrHeader.seccountlo   =  LOWORD(unImageLen) & 0x00ff;
			atrHeader.seccounthi   = (LOWORD(unImageLen) & 0xff00) >> 8;
			atrHeader.hiseccountlo =  HIWORD(unImageLen) & 0x00ff;
			atrHeader.hiseccounthi = (HIWORD(unImageLen) & 0xff00) >> 8;

			fwrite( &atrHeader, sizeof(AFILE_ATR_Header), 1, pfImage );
		}
		/* Build an empty body of the image */
		for( int i = 0; i < (int)unImageLen; i++ )
			fwrite( "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000", 16, 1, pfImage );
		/* Close the image file */
		fflush( pfImage );
		fclose( pfImage );
	}
	CCommonDlg::OnOK();

} /* #OF# CNewDiskImageDlg::OnOK */
