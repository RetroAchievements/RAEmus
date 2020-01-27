/****************************************************************************
File    : GraphicsDlg.cpp
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# CGraphicsDlg implementation file
@(#) #BY# Richard Lawrence, Tomasz Szymankowski
@(#) #LM# 13.10.2003
*/

#include "StdAfx.h"
#include "Atari800Win.h"
#include "Helpers.h"
#include "GraphicsAdvDlg.h"
#include "GraphicsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define IDC_GRAPHICS_FIRST	IDC_GRAPHICS_320X200CLIP
#define IDC_GRAPHICS_LAST	IDC_GRAPHICS_CANCEL


/////////////////////////////////////////////////////////////////////////////
// Static objects

static CGraphicsDlg::GfxModeInfo_t s_aGfxModeInfo[] =
{
	{ IDC_GRAPHICS_320X200CLIP,     MODE_320_200,  SM_FRES_320_200,  IDI_NO_OVERSCAN,   "320x200",  "320x200x256 DDraw",  TRUE  },
	{ IDC_GRAPHICS_320X240PARTCLIP, MODE_320_240,  SM_FRES_320_240,  IDI_VERT_OVERSCAN, "320x240",  "320x240x256 DDraw",  TRUE  },
	{ IDC_GRAPHICS_320X400CLIP,     MODE_320_400,  SM_FRES_320_400,  IDI_NO_OVERSCAN,   "320x400",  "320x400x256 DDraw",  TRUE  },
	{ IDC_GRAPHICS_400X300,         MODE_400_300,  SM_FRES_400_300,  IDI_FULL_OVERSCAN, "400x300",  "400x300x256 DDraw",  TRUE  },
	{ IDC_GRAPHICS_512X384,         MODE_512_384,  SM_FRES_512_384,  IDI_FULL_OVERSCAN, "512x384",  "512x384x256 DDraw",  TRUE  },
	{ IDC_GRAPHICS_640X400CLIP,     MODE_640_400,  SM_FRES_640_400,  IDI_NO_OVERSCAN,   "640x400",  "640x400x256 DDraw",  TRUE  },
	{ IDC_GRAPHICS_640X480PARTCLIP, MODE_640_480,  SM_FRES_640_480,  IDI_VERT_OVERSCAN, "640x480",  "640x480x256 DDraw",  TRUE  },
	{ IDC_GRAPHICS_800X600,         MODE_800_600,  SM_FRES_800_600,  IDI_FULL_OVERSCAN, "800x600",  "800x600x256 DDraw",  TRUE  },
	{ IDC_GRAPHICS_1024X768,        MODE_1024_768, SM_FRES_1024_768, IDI_FULL_OVERSCAN, "1024x768", "1024x768x256 DDraw", TRUE  },
	{ IDC_GRAPHICS_336X240WINDOWED, 0,             SM_WRES_NORMAL,   IDI_FULL_OVERSCAN, "normal",   "336x240 Windowed",   FALSE },
	{ IDC_GRAPHICS_672X480WINDOWED, 0,             SM_WRES_DOUBLE,   IDI_FULL_OVERSCAN, "double",   "672x480 Windowed",   FALSE }
};

static const int s_nGfxModeInfoNo = sizeof(s_aGfxModeInfo)/sizeof(s_aGfxModeInfo[0]);

CGraphicsDlg::GfxModeInfo_t *CGraphicsDlg::m_pGfxModeInfo = s_aGfxModeInfo;


/////////////////////////////////////////////////////////////////////////////
// CGraphicsDlg dialog

BEGIN_MESSAGE_MAP(CGraphicsDlg, CCommonDlg)
	//{{AFX_MSG_MAP(CGraphicsDlg)
	ON_BN_CLICKED(IDC_GRAPHICS_320X200CLIP, OnMode320x200Clip)
	ON_BN_CLICKED(IDC_GRAPHICS_320X240PARTCLIP, OnMode320x240PartClip)
	ON_BN_CLICKED(IDC_GRAPHICS_320X400CLIP, OnMode320x400Clip)
	ON_BN_CLICKED(IDC_GRAPHICS_400X300, OnMode400x300)
	ON_BN_CLICKED(IDC_GRAPHICS_512X384, OnMode512x384)
	ON_BN_CLICKED(IDC_GRAPHICS_640X400CLIP, OnMode640x400Clip)
	ON_BN_CLICKED(IDC_GRAPHICS_640X480PARTCLIP, OnMode640x480PartClip)
	ON_BN_CLICKED(IDC_GRAPHICS_800X600, OnMode800x600)
	ON_BN_CLICKED(IDC_GRAPHICS_1024X768, OnMode1024x768)
	ON_BN_CLICKED(IDC_GRAPHICS_336X240WINDOWED, On336x240Windowed)
	ON_BN_CLICKED(IDC_GRAPHICS_672X480WINDOWED, On672x480Windowed)
	ON_CBN_SELCHANGE(IDC_GRAPHICS_COMBO, OnSelchangeCombo)
	ON_CBN_SELCHANGE(IDC_GRAPHICS_STRETCHMODE, OnSelchangeStretchMode)
	ON_BN_CLICKED(IDC_GRAPHICS_USEGDI, OnUseGDI)
	ON_BN_CLICKED(IDC_GRAPHICS_HIDECURSOR, OnHideCursor)
	ON_BN_CLICKED(IDC_GRAPHICS_USESYSPALETTE, OnUseSysPalette)
	ON_BN_CLICKED(IDC_GRAPHICS_ADVSETTING, OnAdvSetting)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_GRAPHICS_OK, OnOK)
	ON_BN_CLICKED(IDC_GRAPHICS_CANCEL, CCommonDlg::OnCancel)
END_MESSAGE_MAP()

/*========================================================
Method   : CGraphicsDlg::CGraphicsDlg
=========================================================*/
/* #FN#
   Standard constructor */
CGraphicsDlg::
CGraphicsDlg(
	CWnd *pParent /*=NULL*/ /* #IN# Pointer to the parent window */
)
	: CCommonDlg( _IsFlagSet( g_Screen.ulState, SM_ATTR_SMALL_DLG ) ? IDD_GRAPHICS_SMALL : CGraphicsDlg::IDD, pParent )
{
	//{{AFX_DATA_INIT(CGraphicsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_bModeChanged = FALSE;
	m_nFirstCtrl   = IDC_GRAPHICS_FIRST;
	m_nLastCtrl    = IDC_GRAPHICS_LAST;

} /* #OF# CGraphicsDlg::CGraphicsDlg */

/*========================================================
Method   : CGraphicsDlg::DoDataExchange
=========================================================*/
/* #FN#
   Dynamic Data Exchange (not used) */
void
/* #AS#
   Nothing */
CGraphicsDlg::
DoDataExchange(
	CDataExchange *pDX /* #IN# Pointer to CDataExchange object */
)
{
	CCommonDlg::DoDataExchange( pDX );
	//{{AFX_DATA_MAP(CGraphicsDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP

} /* #OF# CGraphicsDlg::DoDataExchange */


/////////////////////////////////////////////////////////////////////////////
// CFileAssociationsDlg implementation

/*========================================================
Method   : CGraphicsDlg::SetBtnState
=========================================================*/
/* #FN#
   Sets pointed radio button state */
BOOL
/* #AS#
   TRUE if the mode has been checked off */
CGraphicsDlg::
SetBtnState(
	ULONG ulButtonID,  /* #IN# Button resource ID */
	ULONG ulModeFlag,  /* #IN# Sreen mode bit     */
	ULONG ulResFlag,   /* #IN# Resolution bit     */
	BOOL  bFullScreen, /* #IN# Full screen flag   */
	BOOL  bSetFocus /*=FALSE*/
)
{
	CButton *pButton = (CButton *)GetDlgItem( ulButtonID );
	BOOL bChecked = _IsFlagSet( m_ulScreenState, ulResFlag ) && (bFullScreen ? _IsFlagSet( m_ulScreenState, SM_MODE_FULL ) : _IsFlagSet( m_ulScreenState, SM_MODE_WIND ));

	ASSERT(NULL != pButton);

	pButton->EnableWindow( bFullScreen ? _IsFlagSet( g_Screen.ulModesAvail, ulModeFlag ) : TRUE );
	pButton->SetCheck( bChecked );
	if( bChecked && bSetFocus )
		pButton->SetFocus();

	return _IsFlagSet( m_ulScreenState, ulResFlag );

} /* #OF# CGraphicsDlg::SetBtnState */

/*========================================================
Method   : CGraphicsDlg::GetFullScreenMode
=========================================================*/
/* #FN#
   Returns a full screen mode flag on the basis of screen state flags */
ULONG
CGraphicsDlg::
GetFullScreenMode(
	ULONG ulScreenState
)
{
	DWORD dwScreenMode = 0;

	for( int i = 0; i < s_nGfxModeInfoNo; i++ )
	{
		if( _IsFlagSet( ulScreenState, m_pGfxModeInfo[ i ].dwRes & SM_FRES_MASK ) )
		{
			dwScreenMode = m_pGfxModeInfo[ i ].dwMode;
			break;
		}
	}
	return dwScreenMode;

} /* #OF# CGraphicsDlg::GetFullScreenMode */

/*========================================================
Method   : CGraphicsDlg::SetDlgState
=========================================================*/
/* #FN#
   Sets up the state of the dialog controls */
void
/* #AS#
   Nothing */
CGraphicsDlg::
SetDlgState(
	BOOL bSetFocus /*=FALSE*/
)
{
	CComboBox *pCombo  = NULL;
	CStatic   *pStatic = NULL;
	HANDLE	   hIcon   = NULL;

	if( m_bSmallMode )
	{
		pCombo = (CComboBox *)GetDlgItem( IDC_GRAPHICS_COMBO );
		int nSelect = 0;
		int nIndex  = 0;
		ASSERT(NULL != pCombo);

		pCombo->ResetContent();

		for( int i = 0; i < s_nGfxModeInfoNo; i++ )
		{
			if( m_pGfxModeInfo[ i ].bFullScr ? _IsFlagSet( g_Screen.ulModesAvail, m_pGfxModeInfo[ i ].dwMode ) : TRUE )
			{
				nIndex = pCombo->AddString( m_pGfxModeInfo[ i ].pszItem );
				/* Remember internal id of the inserted item */
				pCombo->SetItemData( nIndex, (DWORD)i );
			}
			if( _IsFlagSet( m_ulScreenState, m_pGfxModeInfo[ i ].dwRes ) )
			{
				if( m_pGfxModeInfo[ i ].bFullScr )
				{
					if( _IsFlagSet( m_ulScreenState, SM_MODE_FULL ) )
					{
						hIcon = AfxGetApp()->LoadIcon( m_pGfxModeInfo[ i ].dwIconId );
						nSelect = nIndex;
					}
				}
				else
				{
					if( _IsFlagSet( m_ulScreenState, SM_MODE_WIND ) )
					{
						hIcon = AfxGetApp()->LoadIcon( m_pGfxModeInfo[ i ].dwIconId );
						nSelect = nIndex;
					}
				}
			}
		}
		pCombo->SetCurSel( nSelect );
	}
	else
	{
		for( int i = 0; i < s_nGfxModeInfoNo; i++ )
		{
			if( SetBtnState( m_pGfxModeInfo[ i ].dwCtrlId,
								m_pGfxModeInfo[ i ].dwMode,
								m_pGfxModeInfo[ i ].dwRes,
								m_pGfxModeInfo[ i ].bFullScr,
								bSetFocus ) )
			{
				if( _IsFlagSet( m_ulScreenState, m_pGfxModeInfo[ i ].dwRes ) )
				{
					if( m_pGfxModeInfo[ i ].bFullScr )
					{
						pStatic = (CStatic *)GetDlgItem( IDC_GRAPHICS_FULLSCREEN );
						if( _IsFlagSet( m_ulScreenState, SM_MODE_FULL ) )
							hIcon = AfxGetApp()->LoadIcon( m_pGfxModeInfo[ i ].dwIconId );
					}
					else
					{
						pStatic = (CStatic *)GetDlgItem( IDC_GRAPHICS_WINDOWED );
						if( _IsFlagSet( m_ulScreenState, SM_MODE_WIND ) )
							hIcon = AfxGetApp()->LoadIcon( m_pGfxModeInfo[ i ].dwIconId );
					}
					ASSERT(NULL != pStatic);
					pStatic->SetWindowText( m_pGfxModeInfo[ i ].pszDesc );
				}
			}
		}
	}
	/* Set the stretching mode combo */
	_SetSelCbox( IDC_GRAPHICS_STRETCHMODE,   (BOOL)m_nStretchMode );
	_SetChkBttn( IDC_GRAPHICS_USEGDI,        _IsFlagSet( m_ulScreenState, SM_OPTN_USE_GDI     ) );
	_SetChkBttn( IDC_GRAPHICS_HIDECURSOR,    _IsFlagSet( m_ulScreenState, SM_OPTN_HIDE_CURSOR ) );
	_SetChkBttn( IDC_GRAPHICS_USESYSPALETTE, _IsFlagSet( m_ulScreenState, SM_OPTN_USE_SYSPAL  ) );

	pStatic = (CStatic *)GetDlgItem( IDC_GRAPHICS_ICON );
	if( hIcon && pStatic )
		pStatic->SetIcon( (HICON)hIcon );

} /* #OF# CGraphicsDlg::SetDlgState */


/////////////////////////////////////////////////////////////////////////////
// CGraphicsDlg message handlers

/*========================================================
Method   : CGraphicsDlg::OnInitDialog
=========================================================*/
/* #FN#
   Performs special processing when the dialog box is initialized */
BOOL
/* #AS#
   TRUE unless you set the focus to a control */
CGraphicsDlg::
OnInitDialog() 
{
	CCommonDlg::OnInitDialog();

	CRect rcCenterArea, rcCtrl;
	CWnd *pCtrl = NULL;

	m_ulScreenState = g_Screen.ulState;
	m_nStretchMode  = g_Screen.nStretchMode;
	m_nMemoryType   = g_Screen.nMemoryType;
	m_nRequestRate  = g_Screen.nRequestRate;

	m_bSmallMode = _IsFlagSet( g_Screen.ulState, SM_ATTR_SMALL_DLG );

	/* Center the coverage icon */
	pCtrl = GetDlgItem( IDC_GRAPHICS_ICONAREA );
	ASSERT(NULL != pCtrl);
	pCtrl->GetWindowRect( rcCenterArea );
	ScreenToClient( rcCenterArea );

	pCtrl = GetDlgItem( IDC_GRAPHICS_ICON );
	ASSERT(NULL != pCtrl);
	pCtrl->GetWindowRect( rcCtrl );

	rcCtrl.left = rcCenterArea.left + (rcCenterArea.Width() - rcCtrl.Width()) / 2;
	rcCtrl.top  = rcCenterArea.top + (rcCenterArea.Height() - rcCtrl.Height()) / 2;

	pCtrl->SetWindowPos( NULL,
		rcCtrl.left,
		rcCtrl.top,
		0, 0,
		SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );
	
	SetDlgState();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
} /* #OF# CGraphicsDlg::OnInitDialog */

/*========================================================
Method   : CGraphicsDlg::OnMode320x200Clip
=========================================================*/
/* #FN#
   Sets DirectDraw full mode 320x200 */
void
/* #AS#
   Nothing */
CGraphicsDlg::
OnMode320x200Clip()
{
	/* There is a strange problem with messages routing during the dialog
	   navigation using keyboard. This is a strange solution: */
	static BOOL bWarning = FALSE;

	CButton *pButton = (CButton *)GetDlgItem( IDC_GRAPHICS_320X200CLIP );
	if( bWarning || pButton && !pButton->GetCheck() )
		return;

	if( !_IsFlagSet( m_ulScreenState, SM_FRES_320_200 ) || !_IsFlagSet( m_ulScreenState, SM_MODE_FULL ) )
	{
		bWarning = TRUE;
		if( DisplayWarning( IDS_WARN_MENU, DONT_SHOW_MENU_WARN, TRUE ) )
		{
			_ClrFlag( m_ulScreenState, SM_FRES_MASK | SM_MODE_MASK );
			_SetFlag( m_ulScreenState, SM_FRES_320_200 | SM_MODE_FULL );

			SetDlgState( FALSE );
		}
		else
			SetDlgState( TRUE );
		bWarning = FALSE;
	}
} /* #OF# CGraphicsDlg::OnMode320x200Clip */

/*========================================================
Method   : CGraphicsDlg::OnMode320x240Partclip
=========================================================*/
/* #FN#
   Sets DirectDraw full mode 320x240 */
void
/* #AS#
   Nothing */
CGraphicsDlg::
OnMode320x240PartClip()
{
	_ClrFlag( m_ulScreenState, SM_FRES_MASK | SM_MODE_MASK );
	_SetFlag( m_ulScreenState, SM_FRES_320_240 | SM_MODE_FULL );

	SetDlgState();

} /* #OF# CGraphicsDlg::OnMode320x240Partclip */

/*========================================================
Method   : CGraphicsDlg::OnMode320x400Clip
=========================================================*/
/* #FN#
   Sets DirectDraw full mode 320x400 */
void
/* #AS#
   Nothing */
CGraphicsDlg::
OnMode320x400Clip()
{
	/* There is a strange problem with messages routing during the dialog
	   navigation using keyboard. This is a strange solution: */
	static BOOL bWarning = FALSE;

	CButton *pButton = (CButton *)GetDlgItem( IDC_GRAPHICS_320X400CLIP );
	if( bWarning || pButton && !pButton->GetCheck() )
		return;

	if( !_IsFlagSet( m_ulScreenState, SM_FRES_320_400 ) || !_IsFlagSet( m_ulScreenState, SM_MODE_FULL ) )
	{
		bWarning = TRUE;
		if( DisplayWarning( IDS_WARN_MENU, DONT_SHOW_MENU_WARN, TRUE ) )
		{
			_ClrFlag( m_ulScreenState, SM_FRES_MASK | SM_MODE_MASK );
			_SetFlag( m_ulScreenState, SM_FRES_320_400 | SM_MODE_FULL );

			SetDlgState( FALSE );
		}
		else
			SetDlgState( TRUE );
		bWarning = FALSE;
	}
} /* #OF# CGraphicsDlg::OnMode320x400Partclip */

/*========================================================
Method   : CGraphicsDlg::OnMode400x300
=========================================================*/
/* #FN#
   Sets DirectDraw full mode 400x300 */
void
/* #AS#
   Nothing */
CGraphicsDlg::
OnMode400x300()
{
	_ClrFlag( m_ulScreenState, SM_FRES_MASK | SM_MODE_MASK );
	_SetFlag( m_ulScreenState, SM_FRES_400_300 | SM_MODE_FULL );

	SetDlgState();

} /* #OF# CGraphicsDlg::OnMode400x300 */

/*========================================================
Method   : CGraphicsDlg::OnMode512x384
=========================================================*/
/* #FN#
   Sets DirectDraw full mode 512x384 */
void
/* #AS#
   Nothing */
CGraphicsDlg::
OnMode512x384()
{
	_ClrFlag( m_ulScreenState, SM_FRES_MASK | SM_MODE_MASK );
	_SetFlag( m_ulScreenState, SM_FRES_512_384 | SM_MODE_FULL );

	SetDlgState();

} /* #OF# CGraphicsDlg::OnMode512x384 */

/*========================================================
Method   : CGraphicsDlg::OnMode640x400Clip
=========================================================*/
/* #FN#
   Sets DirectDraw full mode 640x400 */
void
/* #AS#
   Nothing */
CGraphicsDlg::
OnMode640x400Clip()
{
	/* There is a strange problem with messages routing during the dialog
	   navigation using keyboard. This is a strange solution: */
	static BOOL bWarning = FALSE;

	CButton *pButton = (CButton *)GetDlgItem( IDC_GRAPHICS_640X400CLIP );
	if( bWarning || pButton && !pButton->GetCheck() )
		return;

	if( !_IsFlagSet( m_ulScreenState, SM_FRES_640_400 ) || !_IsFlagSet( m_ulScreenState, SM_MODE_FULL ) )
	{
		bWarning = TRUE;
		if( DisplayWarning( IDS_WARN_MENU, DONT_SHOW_MENU_WARN, TRUE ) )
		{
			_ClrFlag( m_ulScreenState, SM_FRES_MASK | SM_MODE_MASK );
			_SetFlag( m_ulScreenState, SM_FRES_640_400 | SM_MODE_FULL );

			SetDlgState( FALSE );
		}
		else
			SetDlgState( TRUE );
		bWarning = FALSE;
	}
} /* #OF# CGraphicsDlg::OnMode640x400Clip */

/*========================================================
Method   : CGraphicsDlg::OnMode640x480PartClip
=========================================================*/
/* #FN#
   Sets DirectDraw full mode 640x480 */
void
/* #AS#
   Nothing */
CGraphicsDlg::
OnMode640x480PartClip()
{
	_ClrFlag( m_ulScreenState, SM_FRES_MASK | SM_MODE_MASK );
	_SetFlag( m_ulScreenState, SM_FRES_640_480 | SM_MODE_FULL );

	SetDlgState();

} /* #OF# CGraphicsDlg::OnMode640x480PartClip */

/*========================================================
Method   : CGraphicsDlg::OnMode800x600
=========================================================*/
/* #FN#
   Sets DirectDraw full mode 800x600 */
void
/* #AS#
   Nothing */
CGraphicsDlg::
OnMode800x600()
{
	_ClrFlag( m_ulScreenState, SM_FRES_MASK | SM_MODE_MASK );
	_SetFlag( m_ulScreenState, SM_FRES_800_600 | SM_MODE_FULL );

	SetDlgState();

} /* #OF# CGraphicsDlg::OnMode800x600 */

/*========================================================
Method   : CGraphicsDlg::OnMode1024x768
=========================================================*/
/* #FN#
   Sets DirectDraw full mode 1024x768 */
void
/* #AS#
   Nothing */
CGraphicsDlg::
OnMode1024x768()
{
	_ClrFlag( m_ulScreenState, SM_FRES_MASK | SM_MODE_MASK );
	_SetFlag( m_ulScreenState, SM_FRES_1024_768 | SM_MODE_FULL );

	SetDlgState();

} /* #OF# CGraphicsDlg::OnMode1024x768 */

/*========================================================
Method   : CGraphicsDlg::On336x240Windowed
=========================================================*/
/* #FN#
   Sets DirectDraw windowed mode 384x240 */
void
/* #AS#
   Nothing */
CGraphicsDlg::
On336x240Windowed()
{
	_ClrFlag( m_ulScreenState, SM_WRES_MASK | SM_MODE_MASK );
	_SetFlag( m_ulScreenState, SM_WRES_NORMAL | SM_MODE_WIND );

	SetDlgState();

} /* #OF# CGraphicsDlg::On336x240Windowed */

/*========================================================
Method   : CGraphicsDlg::On672x480Windowed
=========================================================*/
/* #FN#
   Sets DirectDraw windowed mode 768x480 */
void
/* #AS#
   Nothing */
CGraphicsDlg::
On672x480Windowed()
{
	/* There is a strange problem with messages routing during the dialog
	   navigation using keyboard. This is a strange solution: */
	static BOOL bWarning = FALSE;

	CButton *pButton = (CButton *)GetDlgItem( IDC_GRAPHICS_672X480WINDOWED );
	if( bWarning || pButton && !pButton->GetCheck() )
		return;

	if( !_IsFlagSet( m_ulScreenState, SM_WRES_DOUBLE ) || !_IsFlagSet( m_ulScreenState, SM_MODE_WIND ) )
	{
		bWarning = TRUE;
		if( DisplayWarning( IDS_WARN_DOUBLE, DONT_SHOW_DOUBLE_WARN, TRUE ) )
		{
			_ClrFlag( m_ulScreenState, SM_WRES_MASK | SM_MODE_MASK );
			_SetFlag( m_ulScreenState, SM_WRES_DOUBLE | SM_MODE_WIND );

			SetDlgState( FALSE );
		}
		else
			SetDlgState( TRUE );
		bWarning = FALSE;
	}
} /* #OF# CGraphicsDlg::On672x480Windowed */

/*========================================================
Method   : CGraphicsDlg::OnSelchangeCombo
=========================================================*/
/* #FN#
   Sets graphics mode selected by using "Display Mode" combo.
   Small window mode */
void
/* #AS#
   Nothing */
CGraphicsDlg::
OnSelchangeCombo()
{
	CComboBox *pCombo = (CComboBox *)GetDlgItem( IDC_GRAPHICS_COMBO );
	ASSERT(NULL != pCombo);

	switch( pCombo->GetItemData( pCombo->GetCurSel() ) )
	{
		case 0:
			OnMode320x200Clip();
			break;
		case 1:
			OnMode320x240PartClip();
			break;
		case 2:
			OnMode320x400Clip();
			break;
		case 3:
			OnMode400x300();
			break;
		case 4:
			OnMode512x384();
			break;
		case 5:
			OnMode640x400Clip();
			break;
		case 6:
			OnMode640x480PartClip();
			break;
		case 7:
			OnMode800x600();
			break;
		case 8:
			OnMode1024x768();
			break;
		case 9:
			On336x240Windowed();
			break;
		case 10:
			On672x480Windowed();
			break;

		default:
			OnMode640x480PartClip();
			break;
	}
} /* #OF# CGraphicsDlg::OnSelchangeCombo */

/*========================================================
Method   : CGraphicsDlg::OnSelchangeStretchMode
=========================================================*/
/* #FN#
   Sets graphics mode selected by using "Stretching Mode" combo */
void
/* #AS#
   Nothing */
CGraphicsDlg::
OnSelchangeStretchMode()
{
	m_nStretchMode = _GetSelCbox( IDC_GRAPHICS_STRETCHMODE );

} /* #OF# CGraphicsDlg::OnSelchangeStretchMode */

/*========================================================
Method   : CGraphicsDlg::OnUseGDI
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate check box */
void
/* #AS#
   Nothing */
CGraphicsDlg::
OnUseGDI()
{
	_ClickButton( IDC_GRAPHICS_USEGDI, m_ulScreenState, SM_OPTN_USE_GDI );

} /* #OF# CGraphicsDlg::OnUseGDI */

/*========================================================
Method   : CGraphicsDlg::OnHideCursor
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate check box */
void
/* #AS#
   Nothing */
CGraphicsDlg::
OnHideCursor()
{
	_ClickButton( IDC_GRAPHICS_HIDECURSOR, m_ulScreenState, SM_OPTN_HIDE_CURSOR );

} /* #OF# CGraphicsDlg::OnHideCursor */

/*========================================================
Method   : CGraphicsDlg::OnUseSysPalette
=========================================================*/
/* #FN#
   Sets a state of the object regarding to an appropriate check box */
void
/* #AS#
   Nothing */
CGraphicsDlg::
OnUseSysPalette()
{
	_ClickButton( IDC_GRAPHICS_USESYSPALETTE, m_ulScreenState, SM_OPTN_USE_SYSPAL );

} /* #OF# CGraphicsDlg::OnUseSysPalette */

/*========================================================
Method   : CGraphicsDlg::OnAdvSetting
=========================================================*/
/* #FN#
   Displays "Advanced Graphics Options" dialog box */
void
/* #AS#
   Nothing */
CGraphicsDlg::
OnAdvSetting()
{
	CGraphicsAdvDlg dlgGraphicsAdv( &m_ulScreenState, &m_nMemoryType, &m_nRequestRate, GetFullScreenMode( m_ulScreenState ), this );

	dlgGraphicsAdv.DoModal();

} /* #OF# CGraphicsDlg::OnAdvSetting */

/*========================================================
Method   : CGraphicsDlg::OnOK
=========================================================*/
/* #FN#
   Called when the user clicks the OK button */
void
/* #AS#
   Nothing */
CGraphicsDlg::
OnOK() 
{
	BOOL bCommit = TRUE;

	/* Validate the selected refresh rate value */
	if( m_nRequestRate > 0 )
	{
		int  anRefreshRates[ MAX_REFRESH_RATE_NO + 1 ] = { 0 };
		BOOL bValid = FALSE;

		DD_GetRefreshRates( GetFullScreenMode( m_ulScreenState ), &anRefreshRates[ 0 ], MAX_REFRESH_RATE_NO );

		for( int i = 0; i < MAX_REFRESH_RATE_NO && anRefreshRates[ i ]; i++ )
		{
			if( anRefreshRates[ i ] == m_nRequestRate )
			{
				bValid = TRUE;
				break;
			}
		}
		if( !bValid )
			m_nRequestRate = 0; /* Set refresh rate to default */
	}

	/* Check if the video recording should be stopped */
	if( m_ulScreenState != g_Screen.ulState      ||
		m_nStretchMode  != g_Screen.nStretchMode ||
		m_nMemoryType   != g_Screen.nMemoryType  ||
		m_nRequestRate  != g_Screen.nRequestRate )
	{
		bCommit = StreamWarning( IDS_WARN_RECORD_DISPLAY, SRW_VIDEO_STREAM );
	}
	if( bCommit )
	{
		if( m_ulScreenState != g_Screen.ulState )
		{
			g_Screen.ulState = m_ulScreenState;
			WriteRegDWORD( NULL, REG_SCREEN_STATE, g_Screen.ulState );

			/* We'll change the graphic mode after closing the dialog */
			m_bModeChanged = TRUE;
		}
		if( m_nStretchMode != g_Screen.nStretchMode )
		{
			g_Screen.nStretchMode = m_nStretchMode;
			WriteRegDWORD( NULL, REG_STRETCH_MODE, g_Screen.nStretchMode );

			/* Prepare interpolation stuff if needed */
			Screen_PrepareInterp( FALSE );

			m_bModeChanged = TRUE;
		}
		if( m_nMemoryType != g_Screen.nMemoryType )
		{
			g_Screen.nMemoryType = m_nMemoryType;
			WriteRegDWORD( NULL, REG_MEMORY_TYPE, g_Screen.nMemoryType );

			m_bModeChanged = TRUE;
		}
		if( m_nRequestRate != g_Screen.nRequestRate )
		{
			g_Screen.nRequestRate = m_nRequestRate;
			WriteRegDWORD( NULL, REG_REQUEST_RATE, g_Screen.nRequestRate );

			m_bModeChanged = TRUE;
		}
		CCommonDlg::OnOK();
	}
} /* #OF# CGraphicsDlg::OnOK */
