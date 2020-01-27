/****************************************************************************
File    : TapeDlg.cpp
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# CTapeDlg implementation file
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 19.01.2004
*/

#include "StdAfx.h"
#include "Atari800Win.h"
#include "Helpers.h"
#include "FileService.h"
#include "NewTapeImageDlg.h"
#include "TapeDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define IDC_TAPE_FIRST		IDC_TAPE_BROWSE
#define IDC_TAPE_LAST		IDC_TAPE_CANCEL

#define LABEL_NOCASIMAGE	0
#define LABEL_NOTAPE		1
#define LABEL_ENDOFTAPE		2
#define LABEL_BLOCKNO		3
#define LABEL_EMPTYTAPE		4


/////////////////////////////////////////////////////////////////////////////
// CTapeDlg dialog

BEGIN_MESSAGE_MAP(CTapeDlg, CCommonDlg)
	//{{AFX_MSG_MAP(CTapeDlg)
	ON_BN_CLICKED(IDC_TAPE_BROWSE, OnBrowse)
	ON_BN_CLICKED(IDC_TAPE_EJECT, OnEject)
	ON_BN_CLICKED(IDC_TAPE_REWIND, OnRewind)
	ON_BN_CLICKED(IDC_TAPE_NEWIMAGE, OnNewImage)
	ON_EN_KILLFOCUS(IDC_TAPE_FILE, OnKillfocusFile)
	ON_WM_HSCROLL()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_TAPE_OK, OnOK)
	ON_BN_CLICKED(IDC_TAPE_CANCEL, OnCancel)
END_MESSAGE_MAP()

/*========================================================
Method   : CTapeDlg::CTapeDlg
=========================================================*/
/* #FN#
   Standard constructor */
CTapeDlg::
CTapeDlg(
	CWnd *pParent /*=NULL*/ /* #IN# Pointer to the parent window */
)
	: CCommonDlg( CTapeDlg::IDD, pParent )
{
	//{{AFX_DATA_INIT(CTapeDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_bExitPass  = FALSE;
	m_nFirstCtrl = IDC_TAPE_FIRST;
	m_nLastCtrl  = IDC_TAPE_LAST;
	m_bCasImage  = TRUE;

} /* #OF# CTapeDlg::CTapeDlg */

/*========================================================
Method   : CTapeDlg::DoDataExchange
=========================================================*/
/* #FN#
   Dynamic Data Exchange (not used) */
void
/* #AS#
   Nothing */
CTapeDlg::
DoDataExchange(
	CDataExchange *pDX /* #IN# Pointer to CDataExchange object */
)
{
	CCommonDlg::DoDataExchange( pDX );
	//{{AFX_DATA_MAP(CTapeDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP

} /* #OF# CTapeDlg::DoDataExchange */


/////////////////////////////////////////////////////////////////////////////
// CTapeDlg implementation

/*========================================================
Method   : CTapeDlg::SetPosInfo
=========================================================*/
/* #FN#
   Sets up tape position information */
void
/* #AS#
   Nothing */
CTapeDlg::
SetPosInfo(
	UINT nPos /* #IN# Position of the tape slider */
)
{
	CWnd *pStatic = GetDlgItem( IDC_TAPE_POSITION );
	ASSERT(NULL != pStatic);

	static char szInfo[ LOADSTRING_SIZE_S + 1 ];

	if( cassette_max_block > 1 )
	{
		if( (cassette_max_block + 1) == (int)nPos )
		{
			/* Set the "End of tape" label */
			_strncpy( szInfo, m_szLabels[ LABEL_ENDOFTAPE ], LOADSTRING_SIZE_S );
		}
		else
			/* Set the "Block x of y" label */
			sprintf( szInfo, m_szLabels[ LABEL_BLOCKNO ], nPos, cassette_max_block );
	}
	else if( m_bCasImage )
	{
		/* Set the "Empty tape" label */
		_strncpy( szInfo, m_szLabels[ LABEL_EMPTYTAPE ], LOADSTRING_SIZE_S );
	}
	else
		/* Set the "No tape attached" label */
		_strncpy( szInfo, m_szLabels[ LABEL_NOTAPE ], LOADSTRING_SIZE_S );

	pStatic->SetWindowText( szInfo );

} /* #OF# CTapeDlg::SetPosInfo */

/*========================================================
Method   : CTapeDlg::SetDlgState
=========================================================*/
/* #FN#
   Sets up the state of the dialog controls */
void
/* #AS#
   Nothing */
CTapeDlg::
SetDlgState()
{
	CSliderCtrl *pSlider = (CSliderCtrl *)GetDlgItem( IDC_TAPE_SLIDER );
	CFileStatus  fsStatus;

	ASSERT(NULL != pSlider);

	/* Check if there is the pointed cassette file */
	BOOL bTape = _IsPathAvailable( m_szTapeFile ) &&
				 CFile::GetStatus( m_szTapeFile, fsStatus );

	pSlider->SetRange( 1, cassette_max_block + 1, FALSE );
	pSlider->SetTicFreq( (cassette_max_block / 30) + 1 );
	pSlider->SetPageSize( (cassette_max_block / 30) + 1 );
	pSlider->SetPos( cassette_current_block );
	pSlider->EnableWindow( bTape );

	SetDlgItemText( IDC_TAPE_FILE, m_szTapeFile );
	SetDlgItemText( IDC_TAPE_DESC, m_bCasImage ? cassette_description : (bTape ? m_szLabels[ LABEL_NOCASIMAGE ] : "") );

	SetPosInfo( cassette_current_block );

#ifdef WIN_NETWORK_GAMES
	if( ST_KAILLERA_ACTIVE )
	{
		_RdOnlyEdit( IDC_TAPE_FILE,   TRUE  );
		_EnableCtrl( IDC_TAPE_BROWSE, FALSE );
		_EnableCtrl( IDC_TAPE_EJECT,  FALSE );
		_EnableCtrl( IDC_TAPE_REWIND, FALSE );
		_EnableCtrl( IDC_TAPE_SLIDER, FALSE );
	}
	else
#endif
	{
		_EnableCtrl( IDC_TAPE_EJECT,  bTape );
		_EnableCtrl( IDC_TAPE_REWIND, bTape );
	}
} /* #OF# CTapeDlg::SetDlgState */


/////////////////////////////////////////////////////////////////////////////
// CTapeDlg message handlers

/*========================================================
Method   : CTapeDlg::OnInitDialog
=========================================================*/
/* #FN#
   Performs special processing when the dialog box is initialized */
BOOL
/* #AS#
   TRUE unless you set the focus to a control */
CTapeDlg::
OnInitDialog()
{
	CCommonDlg::OnInitDialog();
	
	/* Cache the kernel values */
	_strncpy( m_szTapeFile, cassette_filename, FILENAME_MAX - 1 );

	/* Backup a current tape file name and position */
	_strncpy( m_szTapeBack, cassette_filename, FILENAME_MAX - 1 );
	_strncpy( m_szTapeLast, cassette_filename, FILENAME_MAX - 1 );
	m_nCurrentBack = cassette_current_block;

	/* Check if the tape image is a cassette file */
	UINT unFileType = IAF_CAS_IMAGE;
	m_bCasImage = IsAtariFile( m_szTapeFile, &unFileType );

	/* Cache some descriptions */
	_LoadStringSx( IDS_TAPE_NO_CAS_IMAGE, m_szLabels[ LABEL_NOCASIMAGE ] );
	_LoadStringSx( IDS_TAPE_NO_TAPE,      m_szLabels[ LABEL_NOTAPE ]     );
	_LoadStringSx( IDS_TAPE_END_OF_TAPE,  m_szLabels[ LABEL_ENDOFTAPE ]  );
	_LoadStringSx( IDS_TAPE_BLOCK_NO,     m_szLabels[ LABEL_BLOCKNO ]    );
	_LoadStringSx( IDS_TAPE_EMPTY_TAPE,   m_szLabels[ LABEL_EMPTYTAPE ]  );

	SetDlgState();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
} /* #OF# CTapeDlg::OnInitDialog */

/*========================================================
Method   : CTapeDlg::OnBrowse
=========================================================*/
/* #FN#
   Allows selecting a cassette image using FileDialog window */
void
/* #AS#
   Nothing */
CTapeDlg::
OnBrowse()
{
	if( PickFileName( TRUE, m_szTapeFile, IDS_SELECT_CAS_LOAD, IDS_FILTER_CAS,
					  "cas", PF_LOAD_FLAGS, TRUE, DEFAULT_TAP, this ) &&
		*m_szTapeFile != '\0' )
	{
		SetDlgItemText( IDC_TAPE_FILE, m_szTapeFile );
		/* Open a tape image */
		PrepareTape( m_szTapeFile, FALSE );

		SetDlgState();
	}
} /* #OF# CTapeDlg::OnBrowse */

/*========================================================
Method   : CTapeDlg::PrepareTape
=========================================================*/
/* #FN#
   Prepares the tape to be accessed or removed */
BOOL
/* #AS#
   Nothing */
CTapeDlg::
PrepareTape(
	LPSTR pszTapeFile,
	BOOL  bCheckIfExists /*=TRUE*/,
	BOOL  bForceInsert /*=FALSE*/
)
{
	BOOL bResult = FALSE;

	if( _IsPathAvailable( pszTapeFile ) )
	{
		CFileStatus fsStatus;

		if( !bCheckIfExists || CFile::GetStatus( pszTapeFile, fsStatus ) )
		{
			UINT unFileType = IAF_CAS_IMAGE;
			m_bCasImage = IsAtariFile( pszTapeFile, &unFileType );

			bForceInsert = bForceInsert || m_bCasImage || _stricmp( m_szTapeLast, pszTapeFile ) == 0;

			if( bForceInsert ||
				DisplayWarning( IDS_WARN_CASFILE, DONT_SHOW_CASFILE_WARN, TRUE ) )
			{
				/* Prevents from resource leaks */
				CASSETTE_Remove();
				/* We have to attach a selected file because there is need
				   to update some kernel-depending cassette parameters */
				if( CASSETTE_Insert( pszTapeFile ) )
				{
					/* A tape image has been properly inserted */
					bResult = TRUE;
				}
				else
					DisplayMessage( GetSafeHwnd(), IDS_ERROR_CAS_READ, 0, MB_ICONEXCLAMATION | MB_OK );
			}
		}
	}	
	/* It is a good idea to remove the tape if no success */
	if( !bResult )
		EjectTape();

	/* Save a name of the last used image */
	_strncpy( m_szTapeLast, pszTapeFile, FILENAME_MAX - 1 );

	return bResult;

} /* #OF# CTapeDlg::PrepareTape */

/*========================================================
Method   : CTapeDlg::OnEject
=========================================================*/
/* #FN#
   Called when the user clicks the EJECT button */
void
/* #AS#
   Nothing */
CTapeDlg::
OnEject()
{
	/* Eject an inserted tape image */
	EjectTape();
	/* Set the dialog controls */
	SetDlgState();

} /* #OF# CTapeDlg::OnEject */

/*========================================================
Method   : CTapeDlg::EjectTape
=========================================================*/
/* #FN#
   Detaches a tape file */
void
/* #AS#
   Nothing */
CTapeDlg::
EjectTape()
{
	CASSETTE_Remove();
	strcpy( m_szTapeFile, FILE_NONE );

	/* Restore the standard settings */
	cassette_current_block = 1;
	cassette_max_block = 1;

	m_bCasImage = TRUE;

} /* #OF# CTapeDlg::EjectTape */

/*========================================================
Method   : CTapeDlg::OnHScroll
=========================================================*/
/* #FN#
   The framework calls this member function when the user
   clicks a window’s horizontal scroll bar */
void
/* #AS#
   Nothing */
CTapeDlg::
OnHScroll(
	UINT nSBCode,
	UINT nPos,
	CScrollBar *pScrollBar
)
{
	int nSliderPos = nPos;

	if( TB_THUMBTRACK != nSBCode && SB_THUMBPOSITION != nSBCode )
	{
		if( NULL != pScrollBar )
			nSliderPos = ((CSliderCtrl *)pScrollBar)->GetPos();
	}
	/* Set the cassette position */
	SetPosInfo( cassette_current_block = nSliderPos );

	CCommonDlg::OnHScroll( nSBCode, nPos, pScrollBar );

} /* #OF# CTapeDlg::OnHScroll */

/*========================================================
Method   : CTapeDlg::OnRewind
=========================================================*/
/* #FN#
   Called when the user clicks the REWIND button */
void
/* #AS#
   Nothing */
CTapeDlg::
OnRewind()
{
	cassette_current_block = 1;
	/* Display the new tape position */
	SetDlgState();

} /* #OF# CTapeDlg::OnRewind */

/*========================================================
Method   : CTapeDlg::OnNewImage
=========================================================*/
/* #FN#
   Displays New Tape Image dialog box */
void
/* #AS#
   Nothing */
CTapeDlg::
OnNewImage()
{
	CNewTapeImageDlg dlgNewTapeImage;

	if( IDOK == dlgNewTapeImage.DoModal() &&
		/* Attach tape image to the tape recorder if it was expected */
		dlgNewTapeImage.m_bAttachTape )
	{
		/* Set a name of the created image */
		_strncpy( m_szTapeFile, dlgNewTapeImage.m_szTapeName, FILENAME_MAX - 1 );
		SetDlgItemText( IDC_TAPE_FILE, m_szTapeFile );
		/* Open a tape image */
		PrepareTape( m_szTapeFile, FALSE, TRUE );

		SetDlgState();
	}
} /* #OF# CTapeDlg::OnNewImage */

/*========================================================
Method   : CTapeDlg::OnKillfocusFile
=========================================================*/
/* #FN#
   The framework calls this function before an edit losing input focus */
void
/* #AS#
   Nothing */
CTapeDlg::
OnKillfocusFile()
{
	char szTapeOld[ FILENAME_MAX ];

	strcpy( szTapeOld, m_szTapeFile );
	GetDlgItemText( IDC_TAPE_FILE, m_szTapeFile, FILENAME_MAX - 1 );

	if( !m_bExitPass &&
		_stricmp( szTapeOld, m_szTapeFile ) != 0 )
	{
		/* Open a tape image */
		PrepareTape( m_szTapeFile );
		/* Set the dialog controls */
		SetDlgState();
	}
} /* #OF# CTapeDlg::OnKillfocusFile */

/*========================================================
Method   : CTapeDlg::ReceiveFocused
=========================================================*/
/* #FN#
   Receives the edit controls content again. The user could press
   'Enter' or 'Alt-O' and then all changes he's made in the last
   edited control would be lost. */
void
/* #AS#
   Nothing */
CTapeDlg::
ReceiveFocused()
{
	CWnd *pWnd    = GetFocus();
	UINT  nCtrlID = pWnd ? pWnd->GetDlgCtrlID() : 0;

	switch( nCtrlID )
	{
		case IDC_TAPE_FILE:
			OnKillfocusFile();
			break;
	}
} /* #OF# CTapeDlg::ReceiveFocused */

/*========================================================
Method   : CTapeDlg::OnOK
=========================================================*/
/* #FN#
   Called when the user clicks the OK button */
void
/* #AS#
   Nothing */
CTapeDlg::
OnOK()
{
	char szTapeOld[ FILENAME_MAX ];

	/* There is a problem with messages routing when the dialog is
	   closed with Enter/Alt-O key. KILLFOCUS message arrives
       to late and we have to invoke KillFocus handlers in OnOK
       method by ourselves. That's why we use this member. */
	m_bExitPass = TRUE;

	/* We have to indicate the 'last minute' changes */
	strcpy( szTapeOld, m_szTapeFile );

	/* Unfortunately, edit controls do not lose the focus before
	   handling this when the user uses accelerators */
	ReceiveFocused();

	if( _stricmp( m_szTapeFile, m_szTapeBack ) != 0 ||
		_stricmp( szTapeOld,    m_szTapeFile ) != 0 ||
		/* The tape position was changed, so reattach the tape
		   to allow reading it */
		m_nCurrentBack != cassette_current_block )
	{
		int nCurrentBlock = cassette_current_block;
		/* Always reattach the selected tape */
		PrepareTape( m_szTapeFile );
		/* Restore a tape position */
		cassette_current_block = nCurrentBlock;

		if( _stricmp( m_szTapeFile, m_szTapeBack ) != 0 )
		{
			/* Update the registry */
			WriteRegString( NULL, REG_FILE_TAPE, m_szTapeFile );
		}
	}
	CCommonDlg::OnOK();

} /* #OF# CTapeDlg::OnOK */

/*========================================================
Method   : CTapeDlg::OnCancel
=========================================================*/
/* #FN#
   Called when the user clicks the CANCEL button */
void
/* #AS#
   Nothing */
CTapeDlg::
OnCancel()
{
	/* Restore the original tape */
	if( _stricmp( m_szTapeFile, m_szTapeBack ) != 0 )
	{
		if( !PrepareTape( m_szTapeBack, TRUE, TRUE ) )
			m_nCurrentBack = 1;
	}
	/* Restore the pointer to a current block */
	cassette_current_block = m_nCurrentBack;

	CCommonDlg::OnCancel();

} /* #OF# CTapeDlg::OnCancel */
