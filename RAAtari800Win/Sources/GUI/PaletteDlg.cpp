/****************************************************************************
File    : PaletteDlg.cpp
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# CPaletteDlg implementation file
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 23.11.2003
*/

#include "StdAfx.h"
#include "Atari800Win.h"
#include "Helpers.h"
#include "FileService.h"
#include "PaletteDlg.h"
#include "cfg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define IDC_PALETTE_FIRST	IDC_PALETTE_BLACKLEVEL
#define IDC_PALETTE_LAST	IDC_PALETTE_CANCEL

#define MIN_BLACK			0
#define MAX_BLACK			255
#define MIN_WHITE			0
#define MAX_WHITE			255
#define MIN_SATURATION		0
#define MAX_SATURATION		100
#define MIN_CONTRAST		0
#define MAX_CONTRAST		100
#define MIN_BRIGHTNESS		0
#define MAX_BRIGHTNESS		100
#define MIN_GAMMA			0
#define MAX_GAMMA			100

#define BAR_LINES_NO		16
#define BAR_ENTRIES_NO		(PAL_ENTRIES_NO / BAR_LINES_NO)

/////////////////////////////////////////////////////////////////////////////
// CPaletteDlg dialog

BEGIN_MESSAGE_MAP(CPaletteDlg, CCommonDlg)
	//{{AFX_MSG_MAP(CPaletteDlg)
	ON_NOTIFY(UDN_DELTAPOS, IDC_PALETTE_BLACKSPIN, OnDeltaposBlackSpin)
	ON_EN_KILLFOCUS(IDC_PALETTE_BLACKLEVEL, OnKillfocusBlackLevel)
	ON_NOTIFY(UDN_DELTAPOS, IDC_PALETTE_WHITESPIN, OnDeltaposWhiteSpin)
	ON_EN_KILLFOCUS(IDC_PALETTE_WHITELEVEL, OnKillfocusWhiteLevel)
	ON_NOTIFY(UDN_DELTAPOS, IDC_PALETTE_SATURATIONSPIN, OnDeltaposSaturationSpin)
	ON_EN_KILLFOCUS(IDC_PALETTE_SATURATION, OnKillfocusSaturation)
	ON_NOTIFY(UDN_DELTAPOS, IDC_PALETTE_CONTRASTSPIN, OnDeltaposContrastSpin)
	ON_EN_KILLFOCUS(IDC_PALETTE_CONTRAST, OnKillfocusContrast)
	ON_NOTIFY(UDN_DELTAPOS, IDC_PALETTE_BRIGHTNESSSPIN, OnDeltaposBrightnessSpin)
	ON_EN_KILLFOCUS(IDC_PALETTE_BRIGHTNESS, OnKillfocusBrightness)
	ON_NOTIFY(UDN_DELTAPOS, IDC_PALETTE_GAMMASPIN, OnDeltaposGammaSpin)
	ON_EN_KILLFOCUS(IDC_PALETTE_GAMMA, OnKillfocusGamma)
	ON_BN_CLICKED(IDC_PALETTE_BROWSE, OnBrowse)
	ON_EN_KILLFOCUS(IDC_PALETTE_EDIT, OnKillfocusEdit)
	ON_BN_CLICKED(IDC_PALETTE_USEEXTERNAL, OnUseExternal)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_PALETTE_OK, OnOK)
	ON_BN_CLICKED(IDC_PALETTE_CANCEL, OnCancel)
END_MESSAGE_MAP()

/*========================================================
Method   : CPaletteDlg::CPaletteDlg
=========================================================*/
/* #FN#
   Standard constructor */
CPaletteDlg::
CPaletteDlg(
	CWnd *pParent /*=NULL*/ /* #IN# Pointer to the parent window */
)
	: CCommonDlg( _IsFlagSet( g_Screen.ulState, SM_MODE_FULL ) ? IDD_PALETTE_SMALL : CPaletteDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPaletteDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_bExitPass  = FALSE;
	m_nFirstCtrl = IDC_PALETTE_FIRST;
	m_nLastCtrl  = IDC_PALETTE_LAST;

} /* #OF# CPaletteDlg::CPaletteDlg */

/*========================================================
Method   : CPaletteDlg::DoDataExchange
=========================================================*/
/* #FN#
   Dynamic Data Exchange (not used) */
void
/* #AS#
   Nothing */
CPaletteDlg::
DoDataExchange(
	CDataExchange *pDX /* #IN# Pointer to CDataExchange object */
)
{
	CCommonDlg::DoDataExchange( pDX );
	//{{AFX_DATA_MAP(CPaletteDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP

} /* #OF# CPaletteDlg::DoDataExchange */


/////////////////////////////////////////////////////////////////////////////
// CPaletteDlg implementation

/*========================================================
Method   : CPaletteDlg::PreparePalette
=========================================================*/
/* #FN#
   Generates new or reads and format an external palette from a file */
BOOL
/* #AS#
   Nothing */
CPaletteDlg::
PreparePalette(
	LPSTR pszPaletteFile
)
{
	BOOL bResult = FALSE;

	if( _IsFlagSet( m_ulMiscState, MS_USE_EXT_PALETTE ) &&
		/* Load an external palette only if there
		   is an appropriate option checked off */
		_IsPathAvailable( pszPaletteFile ) )
	{
		if( !Palette_Read( pszPaletteFile ) )
		{
			DisplayMessage( GetSafeHwnd(), IDS_ERROR_ACT_READ, 0, MB_ICONEXCLAMATION | MB_OK );
			strcpy( pszPaletteFile, FILE_NONE );
			_ClrFlag( m_ulMiscState, MS_USE_EXT_PALETTE );
		}
		else
		{
			bResult = TRUE;
		}
	}

	Palette_Generate( m_nBlackLevel, m_nWhiteLevel, m_nSaturation, m_nContrast, m_nBrightness, m_nGamma, _IsFlagSet( m_ulMiscState, MS_USE_EXT_PALETTE ) );

	/* Palette bar is drawn in windowed modes only */
	if( !m_bSmallMode )
	{
		CWnd *pStatic = GetDlgItem( IDC_PALETTE_BAR );
		RECT  rc;

		ASSERT(NULL != pStatic);
		/* There is no reason to make the whole window invalidated */
		pStatic->GetWindowRect( &rc );
		ScreenToClient( &rc );
		/* This makes the changes visible */
		InvalidateRect( &rc, FALSE );
	}
	return bResult;

} /* #OF# CPaletteDlg::PreparePalette */

/*========================================================
Method   : CPaletteDlg::PaintPalette
=========================================================*/
/* #FN#
   Fills the palette bar */
BOOL
/* #AS#
   TRUE if succeeded, otherwise FALSE */
CPaletteDlg::
PaintPalette(
	CDC *pDC
)
{
	CPalette     palPalette;
	CPalette    *pOldPalette;
	CBrush      *pBrush;
	CBrush      *pOldBrush;
	int          nRGB;
	HANDLE       hLogPal;    /* Handle to a logical palette */
	LPLOGPALETTE lpPal;      /* Pointer to a logical palette */
	BOOL         bResult;

	/* Allocate memory block for logical palette */
	hLogPal = ::GlobalAlloc( GHND/*LPTR*/, sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) * PAL_ENTRIES_NO );
	/* If not enough memory, clean up and return NULL */
	if( !hLogPal )
		return FALSE;

	lpPal = (LPLOGPALETTE)::GlobalLock( (HGLOBAL)hLogPal );
	/* Init palette header info */
	lpPal->palVersion    = 0x300;
	lpPal->palNumEntries = (WORD)PAL_ENTRIES_NO;

	/* Copy entries */
	for( int i = 0; i < PAL_ENTRIES_NO; i++ )
	{
		nRGB = Colours_table[ i ];
		
		lpPal->palPalEntry[ i ].peRed   = (nRGB & 0x00ff0000) >> 16;
		lpPal->palPalEntry[ i ].peGreen = (nRGB & 0x0000ff00) >> 8;
		lpPal->palPalEntry[ i ].peBlue  = nRGB & 0x000000ff;
		lpPal->palPalEntry[ i ].peFlags = 0;
	}
	/* Create palette */
	bResult = (BOOL)(palPalette.CreatePalette( lpPal ));

	::GlobalUnlock( (HGLOBAL) hLogPal );
	::GlobalFree( (HGLOBAL) hLogPal );

	if( bResult )
	{
		int nWidth   = (m_rcPalBar.Width() - BAR_ENTRIES_NO) / BAR_ENTRIES_NO;
		int nHeight  = (m_rcPalBar.Height() - BAR_LINES_NO) / BAR_LINES_NO;

		int nOffsetX = (m_rcPalBar.Width() - (nWidth + 1) * BAR_ENTRIES_NO) / 2 + 1;
		int nOffsetY = (m_rcPalBar.Height() - (nHeight + 1) * BAR_LINES_NO) / 2 + 1;

		/* Select and realize palette */
		pOldPalette = pDC->SelectPalette( &palPalette, TRUE );
		pDC->RealizePalette();

		for( int i = 0; i < BAR_LINES_NO; i++ ) /* Eight lines x 32 color picks */
		{
			/* Draw each stripe */
			for( int j = 0; j < BAR_ENTRIES_NO; j++ )
			{
				pBrush = new CBrush;
				/* A palette-relative pBrush is used for drawing */
				if( TRUE == pBrush->CreateSolidBrush( PALETTEINDEX( j + (i * BAR_ENTRIES_NO) ) ) )
				{
					/* Setup pBrush */
					pOldBrush = pDC->SelectObject( pBrush );
					pBrush->UnrealizeObject();
					/* Draw stripe */
					pDC->PatBlt( m_rcPalBar.left + nOffsetX + j * nWidth + j,
								 m_rcPalBar.top + nOffsetY + i * nHeight + i,
								 nWidth, nHeight, PATCOPY );
					/* Restore old brush */
					pDC->SelectObject( pOldBrush );
				}
				delete pBrush;
			}
		}
		/* Back to old palette */
		pDC->SelectPalette( pOldPalette, TRUE );
		pDC->RealizePalette();
	}
	return bResult;

} /* #OF# CPaletteDlg::PaintPalette */

/*========================================================
Method   : CPaletteDlg::RestorePalette
=========================================================*/
/* #FN#
   Restores the palette which the dialog was invoked with */
void
/* #AS#
   Nothing */
CPaletteDlg::
RestorePalette()
{
	int nRGB;

	struct ScreenInterParms_t dipInfo;
	dipInfo.dwMask = DIP_BITMAPINFO;

	Screen_GetInterParms( &dipInfo );

	/* Restore old palette */
	for( int i = 0; i < PAL_ENTRIES_NO; i++ )
	{
		nRGB  = (dipInfo.pBitmapInfo->bmiColors[ i ].rgbRed   & 0xff) << 16;
		nRGB |= (dipInfo.pBitmapInfo->bmiColors[ i ].rgbGreen & 0xff) <<  8;
		nRGB |= (dipInfo.pBitmapInfo->bmiColors[ i ].rgbBlue  & 0xff);

		Colours_table[ i ] = nRGB;
	}
} /* #OF# CPaletteDlg::RestorePalette */

/*========================================================
Method   : CPaletteDlg::SetDlgState
=========================================================*/
/* #FN#
   Sets up the state of the dialog controls */
void
/* #AS#
   Nothing */
CPaletteDlg::
SetDlgState()
{
	CFileStatus fsStatus;

	/* Check if there is the pointed palette file */
	BOOL bPalette = _IsPathAvailable( m_szPaletteFile ) &&
					CFile::GetStatus( m_szPaletteFile, fsStatus );

	if( !bPalette )
		/* Clear "Use external palette" flag if there is no file */
		_ClrFlag( m_ulMiscState, MS_USE_EXT_PALETTE );

	/* Set up check buttons states and activity */
	_SetChkBttn( IDC_PALETTE_USEEXTERNAL,      _IsFlagSet( m_ulMiscState, MS_USE_EXT_PALETTE ) );
	_EnableCtrl( IDC_PALETTE_USEEXTERNAL,      bPalette );

	SetDlgItemInt( IDC_PALETTE_BLACKLEVEL, m_nBlackLevel, FALSE );
	SetDlgItemInt( IDC_PALETTE_WHITELEVEL, m_nWhiteLevel, FALSE );
	SetDlgItemInt( IDC_PALETTE_SATURATION, m_nSaturation, FALSE );
	SetDlgItemInt( IDC_PALETTE_CONTRAST,   m_nContrast,   FALSE );
	SetDlgItemInt( IDC_PALETTE_BRIGHTNESS, m_nBrightness, FALSE );
	SetDlgItemInt( IDC_PALETTE_GAMMA,      m_nGamma,      FALSE );

	SetDlgItemText( IDC_PALETTE_EDIT, m_szPaletteFile );

} /* #OF# CPaletteDlg::SetDlgState */


/////////////////////////////////////////////////////////////////////////////
// CPaletteDlg message handlers

/*========================================================
Method   : CPaletteDlg::OnInitDialog
=========================================================*/
/* #FN#
   Performs special processing when the dialog box is initialized */
BOOL
/* #AS#
   TRUE unless you set the focus to a control */
CPaletteDlg::
OnInitDialog()
{
	CCommonDlg::OnInitDialog();
	
	m_bSmallMode  = _IsFlagSet( g_Screen.ulState, SM_MODE_FULL );
	m_ulMiscState = g_Misc.ulState;
	m_nBlackLevel = g_Screen.Pal.nBlackLevel;
	m_nWhiteLevel = g_Screen.Pal.nWhiteLevel;
	m_nSaturation = g_Screen.Pal.nSaturation;
	m_nContrast   = g_Screen.Pal.nContrast;
	m_nBrightness = g_Screen.Pal.nBrightness;
	m_nGamma      = g_Screen.Pal.nGamma;

	_strncpy( m_szPaletteFile, g_szPaletteFile, MAX_PATH );

	if( !m_bSmallMode )
	{
		GetDlgItem( IDC_PALETTE_BAR )->GetWindowRect( m_rcPalBar );
		ScreenToClient( m_rcPalBar );
		m_rcPalBar.DeflateRect( CX_BORDER, CY_BORDER );

		/* Read the palette from a file or/and format it */
		PreparePalette( m_szPaletteFile );
	}
	SetDlgState();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
} /* #OF# CPaletteDlg::OnInitDialog */

/*========================================================
Method   : CPaletteDlg::OnDeltaposBlackSpin
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate spin control */
void
/* #AS#
   Nothing */
CPaletteDlg::
OnDeltaposBlackSpin(
	NMHDR   *pNMHDR, /* #IN#  */
	LRESULT *pResult /* #OUT# */
)
{
	_DeltaposSpin( pNMHDR, IDC_PALETTE_BLACKLEVEL, m_nBlackLevel, MIN_BLACK, MAX_BLACK );
	/* Read the palette from a file or/and format it */
	PreparePalette( m_szPaletteFile );

	*pResult = 0;

} /* #OF# CPaletteDlg::OnDeltaposBlackSpin */

/*========================================================
Method   : CPaletteDlg::OnKillfocusBlackLevel
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate edit control */
void
/* #AS#
   Nothing */
CPaletteDlg::
OnKillfocusBlackLevel()
{
	_KillfocusSpin( IDC_PALETTE_BLACKLEVEL, m_nBlackLevel, MIN_BLACK, MAX_BLACK );
	if( !m_bExitPass )
		/* Read the palette from a file or/and format it */
		PreparePalette( m_szPaletteFile );

} /* #OF# CPaletteDlg::OnKillfocusBlackLevel */

/*========================================================
Method   : CPaletteDlg::OnDeltaposWhiteSpin
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate spin control */
void
/* #AS#
   Nothing */
CPaletteDlg::
OnDeltaposWhiteSpin(
	NMHDR   *pNMHDR, /* #IN#  */
	LRESULT *pResult /* #OUT# */
)
{
	_DeltaposSpin( pNMHDR, IDC_PALETTE_WHITELEVEL, m_nWhiteLevel, MIN_WHITE, MAX_WHITE );
	/* Read the palette from a file or/and format it */
	PreparePalette( m_szPaletteFile );

	*pResult = 0;

} /* #OF# CPaletteDlg::OnDeltaposWhiteSpin */

/*========================================================
Method   : CPaletteDlg::OnKillfocusWhiteLevel
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate edit control */
void
/* #AS#
   Nothing */
CPaletteDlg::
OnKillfocusWhiteLevel()
{
	_KillfocusSpin( IDC_PALETTE_WHITELEVEL, m_nWhiteLevel, MIN_WHITE, MAX_WHITE );
	if( !m_bExitPass )
		/* Read the palette from a file or/and format it */
		PreparePalette( m_szPaletteFile );

} /* #OF# CPaletteDlg::OnKillfocusWhiteLevel */

/*========================================================
Method   : CPaletteDlg::OnDeltaposSaturationSpin
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate spin control */
void
/* #AS#
   Nothing */
CPaletteDlg::
OnDeltaposSaturationSpin(
	NMHDR   *pNMHDR, /* #IN#  */
	LRESULT *pResult /* #OUT# */
)
{
	_DeltaposSpin( pNMHDR, IDC_PALETTE_SATURATION, m_nSaturation, MIN_SATURATION, MAX_SATURATION );
	/* Read the palette from a file or/and format it */
	PreparePalette( m_szPaletteFile );

	*pResult = 0;

} /* #OF# CPaletteDlg::OnDeltaposSaturationSpin */

/*========================================================
Method   : CPaletteDlg::OnKillfocusSaturation
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate edit control */
void
/* #AS#
   Nothing */
CPaletteDlg::
OnKillfocusSaturation()
{
	_KillfocusSpin( IDC_PALETTE_SATURATION, m_nSaturation, MIN_SATURATION, MAX_SATURATION );
	if( !m_bExitPass )
		/* Read the palette from a file or/and format it */
		PreparePalette( m_szPaletteFile );

} /* #OF# CPaletteDlg::OnKillfocusSaturation */

/*========================================================
Method   : CPaletteDlg::OnDeltaposContrastSpin
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate spin control */
void
/* #AS#
   Nothing */
CPaletteDlg::
OnDeltaposContrastSpin(
	NMHDR   *pNMHDR, /* #IN#  */
	LRESULT *pResult /* #OUT# */
)
{
	_DeltaposSpin( pNMHDR, IDC_PALETTE_CONTRAST, m_nContrast, MIN_CONTRAST, MAX_CONTRAST );
	/* Read the palette from a file or/and format it */
	PreparePalette( m_szPaletteFile );

	*pResult = 0;

} /* #OF# CPaletteDlg::OnDeltaposContrastSpin */

/*========================================================
Method   : CPaletteDlg::OnKillfocusContrast
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate edit control */
void
/* #AS#
   Nothing */
CPaletteDlg::
OnKillfocusContrast()
{
	_KillfocusSpin( IDC_PALETTE_CONTRAST, m_nContrast, MIN_CONTRAST, MAX_CONTRAST );
	if( !m_bExitPass )
		/* Read the palette from a file or/and format it */
		PreparePalette( m_szPaletteFile );

} /* #OF# CPaletteDlg::OnKillfocusContrast */

/*========================================================
Method   : CPaletteDlg::OnDeltaposBrightnessSpin
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate spin control */
void
/* #AS#
   Nothing */
CPaletteDlg::
OnDeltaposBrightnessSpin(
	NMHDR   *pNMHDR, /* #IN#  */
	LRESULT *pResult /* #OUT# */
)
{
	_DeltaposSpin( pNMHDR, IDC_PALETTE_BRIGHTNESS, m_nBrightness, MIN_BRIGHTNESS, MAX_BRIGHTNESS );
	/* Read the palette from a file or/and format it */
	PreparePalette( m_szPaletteFile );

	*pResult = 0;

} /* #OF# CPaletteDlg::OnDeltaposBrightnessSpin */

/*========================================================
Method   : CPaletteDlg::OnKillfocusBrightness
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate edit control */
void
/* #AS#
   Nothing */
CPaletteDlg::
OnKillfocusBrightness()
{
	_KillfocusSpin( IDC_PALETTE_BRIGHTNESS, m_nBrightness, MIN_BRIGHTNESS, MAX_BRIGHTNESS );
	if( !m_bExitPass )
		/* Read the palette from a file or/and format it */
		PreparePalette( m_szPaletteFile );

} /* #OF# CPaletteDlg::OnKillfocusBrightness */

/*========================================================
Method   : CPaletteDlg::OnDeltaposGammaSpin
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate spin control */
void
/* #AS#
   Nothing */
CPaletteDlg::
OnDeltaposGammaSpin(
	NMHDR   *pNMHDR, /* #IN#  */
	LRESULT *pResult /* #OUT# */
)
{
	_DeltaposSpin( pNMHDR, IDC_PALETTE_GAMMA, m_nGamma, MIN_GAMMA, MAX_GAMMA );
	/* Read the palette from a file or/and format it */
	PreparePalette( m_szPaletteFile );

	*pResult = 0;

} /* #OF# CPaletteDlg::OnDeltaposGammaSpin */

/*========================================================
Method   : CPaletteDlg::OnKillfocusGamma
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate edit control */
void
/* #AS#
   Nothing */
CPaletteDlg::
OnKillfocusGamma()
{
	_KillfocusSpin( IDC_PALETTE_GAMMA, m_nGamma, MIN_GAMMA, MAX_GAMMA );
	if( !m_bExitPass )
		/* Read the palette from a file or/and format it */
		PreparePalette( m_szPaletteFile );

} /* #OF# CPaletteDlg::OnKillfocusGamma */

/*========================================================
Method   : CPaletteDlg::OnUseExternal
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate check box */
void
/* #AS#
   Nothing */
CPaletteDlg::
OnUseExternal()
{
	_ClickButton( IDC_PALETTE_USEEXTERNAL, m_ulMiscState, MS_USE_EXT_PALETTE );
	/* Read the palette from a file or/and format it */
	PreparePalette( m_szPaletteFile );

	SetDlgState();

} /* #OF# CPaletteDlg::OnUseExternal */

/*========================================================
Method   : CPaletteDlg::OnBrowse
=========================================================*/
/* #FN#
   Allows selecting a palette file using FileDialog window */
void
/* #AS#
   Nothing */
CPaletteDlg::
OnBrowse()
{
	if( PickFileName( TRUE, m_szPaletteFile, IDS_SELECT_ACT_LOAD, IDS_FILTER_ACT,
					  "act", PF_LOAD_FLAGS, TRUE, DEFAULT_ACT, this ) &&
		*m_szPaletteFile != '\0' )
	{
		SetDlgItemText( IDC_PALETTE_EDIT, m_szPaletteFile );
		/* Read the palette from a file or/and format it */
		PreparePalette( m_szPaletteFile);

		SetDlgState();
	}
} /* #OF# CPaletteDlg::OnBrowse */

/*========================================================
Method   : CPaletteDlg::OnKillfocusEdit
=========================================================*/
/* #FN#
   The framework calls this function before an edit losing input focus */
void
/* #AS#
   Nothing */
CPaletteDlg::
OnKillfocusEdit()
{
	char szPaletteOld[ MAX_PATH + 1 ];

	strcpy( szPaletteOld, m_szPaletteFile );
	GetDlgItemText( IDC_PALETTE_EDIT, m_szPaletteFile, MAX_PATH );

	if( !m_bExitPass &&
		_stricmp( szPaletteOld, m_szPaletteFile ) != 0 )
	{
		/* Read the palette from a file or/and format it */
		PreparePalette( m_szPaletteFile );
		/* Set the dialog controls */
		SetDlgState();
	}
} /* #OF# CPaletteDlg::OnKillfocusEdit */

/*========================================================
Method   : CPaletteDlg::OnPaint
=========================================================*/
/* #FN#
   The framework calls this member function when Windows or an application
   makes a request to repaint a portion of an application’s window */
void
/* #AS#
   Nothing */
CPaletteDlg::
OnPaint()
{
	CPaintDC dc( this ); /* Device context for painting */

	if( !m_bSmallMode )
	{
		/* Paint the palette bar */
		PaintPalette( &dc );
	}
} /* #OF# CPaletteDlg::OnPaint */

/*========================================================
Method   : CPaletteDlg::ReceiveFocused
=========================================================*/
/* #FN#
   Receives the edit controls content again. The user could press
   'Enter' or 'Alt-O' and then all changes he's made in the last
   edited control would be lost. */
void
/* #AS#
   Nothing */
CPaletteDlg::
ReceiveFocused()
{
	CWnd *pWnd    = GetFocus();
	UINT  nCtrlID = pWnd ? pWnd->GetDlgCtrlID() : 0;

	switch( nCtrlID )
	{
		case IDC_PALETTE_BLACKLEVEL:
			OnKillfocusBlackLevel();
			break;
		case IDC_PALETTE_WHITELEVEL:
			OnKillfocusWhiteLevel();
			break;
		case IDC_PALETTE_SATURATION:
			OnKillfocusSaturation();
			break;
		case IDC_PALETTE_CONTRAST:
			OnKillfocusContrast();
			break;
		case IDC_PALETTE_BRIGHTNESS:
			OnKillfocusBrightness();
			break;
		case IDC_PALETTE_GAMMA:
			OnKillfocusGamma();
			break;
		case IDC_PALETTE_EDIT:
			OnKillfocusEdit();
			break;
	}
} /* #OF# CPaletteDlg::ReceiveFocused */

/*========================================================
Method   : CPaletteDlg::OnOK
=========================================================*/
/* #FN#
   Called when the user clicks the OK button */
void
/* #AS#
   Nothing */
CPaletteDlg::
OnOK()
{
	BOOL bChanged = FALSE;

	/* There is a problem with messages routing when the dialog is
	   closed with Enter/Alt-O key. KILLFOCUS message arrives
	   to late and we have to invoke KillFocus handlers in OnOK
	   method by ourselves. That's why we use this member. */
	m_bExitPass = TRUE;

	/* Unfortunately, edit controls do not lose the focus before
	   handling this when the user uses accelerators */
	ReceiveFocused();

	if( m_nBlackLevel != g_Screen.Pal.nBlackLevel )
	{
		g_Screen.Pal.nBlackLevel = m_nBlackLevel;
		WriteRegDWORD( NULL, REG_COLOR_BLACK, g_Screen.Pal.nBlackLevel );
		bChanged = TRUE;
	}
	if( m_nWhiteLevel != g_Screen.Pal.nWhiteLevel )
	{
		g_Screen.Pal.nWhiteLevel = m_nWhiteLevel;
		WriteRegDWORD( NULL, REG_COLOR_WHITE, g_Screen.Pal.nWhiteLevel );
		bChanged = TRUE;
	}
	if( m_nSaturation != g_Screen.Pal.nSaturation )
	{
		g_Screen.Pal.nSaturation = m_nSaturation;
		WriteRegDWORD( NULL, REG_COLOR_SATURATION, g_Screen.Pal.nSaturation );
		bChanged = TRUE;
	}
	if( m_nContrast != g_Screen.Pal.nContrast )
	{
		g_Screen.Pal.nContrast = m_nContrast;
		WriteRegDWORD( NULL, REG_COLOR_CONTRAST, g_Screen.Pal.nContrast );
		bChanged = TRUE;
	}
	if( m_nBrightness != g_Screen.Pal.nBrightness )
	{
		g_Screen.Pal.nBrightness = m_nBrightness;
		WriteRegDWORD( NULL, REG_COLOR_BRIGHTNESS, g_Screen.Pal.nBrightness );
		bChanged = TRUE;
	}
	if( m_nGamma != g_Screen.Pal.nGamma )
	{
		g_Screen.Pal.nGamma = m_nGamma;
		WriteRegDWORD( NULL, REG_COLOR_GAMMA, g_Screen.Pal.nGamma );
		bChanged = TRUE;
	}

	/* Always reload the selected palette 
	if( !PreparePalette( m_szPaletteFile ) )
	{
		if( _IsFlagSet( m_ulMiscState, MS_USE_EXT_PALETTE ) )
		{
			_ClrFlag( m_ulMiscState, MS_USE_EXT_PALETTE );
			DisplayMessage( GetSafeHwnd(), IDS_ERROR_NO_PALETTE, 0, MB_ICONEXCLAMATION | MB_OK, m_szPaletteFile );
		}
		RestorePalette();
	} */

	/* Windows NT doesn't like empty strings in Registry */
	if( '\0' == *m_szPaletteFile )
		strcpy( m_szPaletteFile, DEFAULT_ACT );

	if( _stricmp( g_szPaletteFile, m_szPaletteFile ) != 0 )
	{
		_strncpy( g_szPaletteFile, m_szPaletteFile, MAX_PATH );
		WriteRegString( NULL, REG_FILE_PALETTE, g_szPaletteFile );
		bChanged = TRUE;
	}
	/* Check the miscellanous states */
	if( m_ulMiscState != g_Misc.ulState )
	{
		g_Misc.ulState = m_ulMiscState;
		WriteRegDWORD( NULL, REG_MISC_STATE, g_Misc.ulState );
		bChanged = TRUE;
	}	
	/* Apply the changes to the main window */
	Screen_UseAtariPalette( TRUE );

	/* Prepare interpolation stuff if needed */
	if( bChanged )
	{
		Screen_FreeInterp();
		/* Now when the interpolation stuff was freed check
		   if there is a need for preparing a new one. If
		   not, the stuff will be prepared when later (e.g.
		   when "smooth' stretching will be checked off) */
		Screen_PrepareInterp( FALSE );
	}
	CCommonDlg::OnOK();

} /* #OF# CPaletteDlg::OnOK */

/*========================================================
Method   : CPaletteDlg::OnCancel
=========================================================*/
/* #FN#
   Called when the user clicks the CANCEL button */
void
/* #AS#
   Nothing */
CPaletteDlg::
OnCancel()
{
	/* Restore the original palette */
	RestorePalette();

	CCommonDlg::OnCancel();

} /* #OF# CPaletteDlg::OnCancel */
