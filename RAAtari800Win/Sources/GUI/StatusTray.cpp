/****************************************************************************
File    : StatusTray.cpp
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# CStatusTray implementation file
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 04.11.2002
*/

#include "StdAfx.h"
#include "Atari800Win.h"
#include "Helpers.h"
#include "StatusTray.h"


/////////////////////////////////////////////////////////////////////////////
// Static objects

static UINT GetRunBmpID();
static UINT GetVideoBmpID();
static UINT GetDiskBmpID();
static UINT GetMouseBmpID();
static UINT GetAutofireBmpID();
static UINT GetCameraBmpID();
static UINT GetTapeBmpID();
#ifdef WIN_NETWORK_GAMES
static UINT GetNetGameBmpID();
#endif

static CStatusTray::StatusIconInfo_t s_aStatusIconInfo[] =
{
	{ ID_INDICATOR_RUN, GetRunBmpID      },
	{ ID_INDICATOR_VID, GetVideoBmpID    },
	{ ID_INDICATOR_SIO, GetDiskBmpID     },
	{ ID_INDICATOR_MSE, GetMouseBmpID    },
	{ ID_INDICATOR_JOY, GetAutofireBmpID },
	{ ID_INDICATOR_AVI, GetCameraBmpID   },
	{ ID_INDICATOR_WAV, GetTapeBmpID     }
#ifdef WIN_NETWORK_GAMES
   ,{ ID_INDICATOR_NET, GetNetGameBmpID  }
#endif
};

static const int s_nStatusIconInfoNo = sizeof(s_aStatusIconInfo)/sizeof(s_aStatusIconInfo[0]);


/////////////////////////////////////////////////////////////////////////////
// CStatusTray class

BEGIN_MESSAGE_MAP(CStatusTray, CStatusBar)
	//{{AFX_MSG_MAP(CStatusTray)
	ON_WM_LBUTTONDOWN()
	ON_WM_MBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MBUTTONUP()
	ON_WM_RBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_MBUTTONDBLCLK()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/*========================================================
Method   : CStatusTray::CStatusTray
=========================================================*/
/* #FN#
   Standard constructor */
CStatusTray::
CStatusTray()
{
	m_bInitilized = FALSE;

	m_pIcons = new CBitmap[ s_nStatusIconInfoNo ];
	if( NULL != m_pIcons )
	{
		m_pBmpIDs = new UINT[ s_nStatusIconInfoNo ];
		if( NULL != m_pBmpIDs )
		{
			for( int i = 0; i < s_nStatusIconInfoNo; i++ )
				m_pBmpIDs[ i ] = 0;

			m_bInitilized = TRUE;
		}
	}
	m_pParent = NULL;

} /* #OF# CStatusTray::CStatusTray */

/*========================================================
Method   : CStatusTray::~CStatusTray
=========================================================*/
/* #FN#
   Destructor */
CStatusTray::
~CStatusTray()
{
	if( NULL != m_pBmpIDs )
	{
		delete [] m_pBmpIDs;
		m_pBmpIDs = NULL;
	}
	if( NULL != m_pIcons )
	{
		delete [] m_pIcons;
		m_pIcons = NULL;
	}
} /* #OF# CStatusTray::~CStatusTray */


/////////////////////////////////////////////////////////////////////////////
// CStatusTray implementation

/*========================================================
Function : GetRunBmpID
=========================================================*/
/* #FN#
   Returns an appropriate resource ID for the 'run' indicator */
static
UINT
/* #AS#
   Resource ID of the bitmap */
GetRunBmpID()
{
	static UINT nBmpID[] =
	{
		IDB_SB_RUN_D, IDB_SB_RUN_E, IDB_SB_RUNFAST_D, IDB_SB_RUNFAST_E
	};
	int nIdx = 1;

	if( _IsFlagSet( g_ulAtariState, ATARI_UNINITIALIZED | ATARI_PAUSED | ATARI_CRASHED | ATARI_MONITOR ) ||
		_IsFlagSet( g_Misc.ulState, MS_STOP_WHEN_NO_FOCUS ) && _IsFlagSet( g_ulAtariState, ATARI_NO_FOCUS ) )
		nIdx = 0;

	if( _IsFlagSet( g_Misc.ulState, MS_FULL_SPEED ) )
		nIdx += 2;

	return nBmpID[ nIdx ];

} /* #OF# GetRunBmpID */

/*========================================================
Function : GetVideoBmpID
=========================================================*/
/* #FN#
   Returns an appropriate resource ID for the 'tv system' indicator */
static
UINT
/* #AS#
   Resource ID of the bitmap */
GetVideoBmpID()
{
	return (Atari800_TV_PAL == Atari800_tv_mode ? IDB_SB_TVPAL_E : IDB_SB_TVNTSC_E);

} /* #OF# GetVideoBmpID */

/*========================================================
Function : GetDiskBmpID
=========================================================*/
/* #FN#
   Returns an appropriate resource ID for the 'fast disk' indicator */
static
UINT
/* #AS#
   Resource ID of the bitmap */
GetDiskBmpID()
{
	return (ESC_enable_sio_patch ? IDB_SB_DISK_E : IDB_SB_DISK_D);

} /* #OF# GetDiskBmpID */

/*========================================================
Function : GetMouseBmpID
=========================================================*/
/* #FN#
   Returns an appropriate resource ID for the 'mouse device' indicator */
static
UINT
/* #AS#
   Resource ID of the bitmap */
GetMouseBmpID()
{
	static UINT nBmpID[] =
	{
		IDB_SB_NONE_E,        IDB_SB_PADDLES_E,
		IDB_SB_TOUCHTABLET_E, IDB_SB_KOALAPAD_E,
		IDB_SB_LIGHTPEN_E,    IDB_SB_LIGHTGUN_E,
		IDB_SB_AMIGAMOUSE_E,  IDB_SB_STMOUSE_E,
		IDB_SB_TRAKBALL_E,    IDB_SB_JOYSTICK_E,
		IDB_SB_NONE_D,        IDB_SB_PADDLES_D,
		IDB_SB_TOUCHTABLET_D, IDB_SB_KOALAPAD_D,
		IDB_SB_LIGHTPEN_D,    IDB_SB_LIGHTGUN_D,
		IDB_SB_AMIGAMOUSE_D,  IDB_SB_STMOUSE_D,
		IDB_SB_TRAKBALL_D,    IDB_SB_JOYSTICK_D
	};

	return nBmpID[ _IsFlagSet( g_Input.ulState, IS_CAPTURE_MOUSE ) ?
						INPUT_mouse_mode :
						INPUT_mouse_mode + 10 ];
} /* #OF# GetMouseBmpID */

/*========================================================
Function : GetAutofireBmpID
=========================================================*/
/* #FN#
   Returns an appropriate resource ID for the 'autofire mode' indicator */
static
UINT
/* #AS#
   Resource ID of the bitmap */
GetAutofireBmpID()
{
	return (g_Input.Joy.nAutoMode == INPUT_AUTOFIRE_FIRE ? IDB_SB_AUTOFIRE1_E :
		   (g_Input.Joy.nAutoMode == INPUT_AUTOFIRE_CONT ? IDB_SB_AUTOFIRE2_E : IDB_SB_AUTOFIRE_D));

} /* #OF# GetAutofireBmpID */

/*========================================================
Function : GetCameraBmpID
=========================================================*/
/* #FN#
   Returns an appropriate resource ID for the 'video recording' indicator */
static
UINT
/* #AS#
   Resource ID of the bitmap */
GetCameraBmpID()
{
	return (g_Screen.pfOutput != NULL ? IDB_SB_CAMERA_E : IDB_SB_CAMERA_D);

} /* #OF# GetCameraBmpID */

/*========================================================
Function : GetTapeBmpID
=========================================================*/
/* #FN#
   Returns an appropriate resource ID for the 'sound recording' indicator */
static
UINT
/* #AS#
   Resource ID of the bitmap */
GetTapeBmpID()
{
	return (g_Sound.pfOutput != NULL  ? IDB_SB_TAPE_E : IDB_SB_TAPE_D);

} /* #OF# GetTapeBmpID */

#ifdef WIN_NETWORK_GAMES
/*========================================================
Function : GetNetGameBmpID
=========================================================*/
/* #FN#
   Returns an appropriate resource ID for the 'network game' indicator */
static
UINT
/* #AS#
   Resource ID of the bitmap */
GetNetGameBmpID()
{
	static UINT nBmpID[] =
	{
		IDB_SB_KAILLERA_D,  IDB_SB_KAILLERA_E,
		IDB_SB_KAILLERA1_E, IDB_SB_KAILLERA2_E,
		IDB_SB_KAILLERA3_E, IDB_SB_KAILLERA4_E
	};

	return nBmpID[ ST_KAILLERA_ACTIVE ? (ST_NETGAME_ACTIVE ? g_Kaillera.nLocalPort + 2 : 1) : 0 ];

} /* #OF# GetNetGameBmpID */
#endif

/*========================================================
Method   : CStatusTray::ConfigureTray
=========================================================*/
/* #FN#
   Sets indicators on the status bar */
void
/* #AS#
   Nothing */
CStatusTray::
ConfigureTray(
	UINT *pIndicators,
	int   nPaneNo,
	BOOL  bRedraw /*=TRUE*/,
	CWnd *pParent /*=NULL*/
)
{
	static int nMsgPaneWidth = 0;

	UINT nID, nStyle;
	int  nIconWidth = _IsFlagSet( g_Misc.ulState, MS_SHOW_INDICATORS ) ? 12 : 0;
	int  nWidth, nTrayWidth = 0;

	if( NULL != pParent )
		m_pParent = pParent;

	/* Set style for icon status panes */
	for( int i = 1; i < nPaneNo; i++ )
	{
		GetPaneInfo( i, nID, nStyle, nWidth );
		_ModifyFlag( !nIconWidth, nStyle, SBPS_NOBORDERS );
		SetPaneInfo( i, nID, nStyle | SBPS_OWNERDRAW, nIconWidth );
	}
	GetPaneInfo( ID_INDICATOR_MSG, nID, nStyle, nWidth );
	if( !nMsgPaneWidth )
		nMsgPaneWidth = nWidth;
	/* Size of the row of icons should be calculated more properly... Maybe in the future... */
	SetPaneInfo( ID_INDICATOR_MSG, nID, nStyle, nMsgPaneWidth - (nIconWidth ? int(nMsgPaneWidth * 0.6) : -1024) );

	if( bRedraw )
		/* Redraw status bar */
		Invalidate();

} /* #OF# CStatusTray::ConfigureTray */

/*========================================================
Method   : CStatusTray::UpdatePane
=========================================================*/
/* #FN#
   Redraws the pointed indicator */
void
/* #AS#
   Nothing */
CStatusTray::
UpdatePane(
	int nPane /* #IN# Code of the indicator to redraw */
)
{
	RECT rc;
	GetItemRect( nPane, &rc );
	/* It sometimes doesn't work when accelerator keys are used under
	   2000 (it seems that RDW_UPDATENOW is the key to the success) */
//	InvalidateRect( &rc, FALSE );

	if( m_pParent )
	{
		UINT nFlags = RDW_INVALIDATE | RDW_NOERASE | RDW_FRAME;

		if( _IsFlagSet( g_Misc.unSystemInfo, SYS_WIN_NT5 ) )
			/* There is a fatal crash "performed" under 95 when using this flag */
			nFlags |= RDW_UPDATENOW;

		ClientToScreen( &rc );
		m_pParent->ScreenToClient( &rc );
		m_pParent->RedrawWindow( &rc, NULL, nFlags );
	}
} /* #OF# CStatusTray::UpdatePane */

/*========================================================
Method   : CStatusTray::LoadMappedBitmap
=========================================================*/
/* #FN#
   Loads a bitmap for the pointed indicator */
BOOL
/* #AS#
   TRUE if succeeded, otherwise FALSE */
CStatusTray::
LoadMappedBitmap(
	UINT nBmpID, /* #IN# Resource ID of the bitmap to load */
	int  nPane   /* #IN# Code of the indicator the bitmap is loading for */
)
{
	BOOL bResult = TRUE;

	ASSERT(nPane <= s_nStatusIconInfoNo);

	if( m_pBmpIDs[ nPane ] != nBmpID )
	{
		m_pIcons[ nPane ].DeleteObject();
		m_pBmpIDs [ nPane ] = 0;

		/* Load a pointed bitmap to the cache */
		if( m_pIcons[ nPane ].LoadMappedBitmap( nBmpID ) )
		{
			/* Save the respective bitmap id */
			m_pBmpIDs[ nPane ] = nBmpID;
//			TRACE0("CStatusTray::LoadMappedBitmap cache hit failure\n");
		}
		else
			bResult = FALSE;
	}
	return bResult;

} /* #OF# CStatusTray::LoadMappedBitmap */

/*========================================================
Method   : CStatusTray::GetPaneFromPoint
=========================================================*/
/* #FN#
   Returns code of an indicator the mouse cursor is pointing to */
int
/* #AS#
   Code of the indicator */
CStatusTray::
GetPaneFromPoint(
	CPoint &pt /* #IN# Position of the mouse cursor */
)
{
	CRect rect;
	int nPaneNo = GetCount();

	for( int i = 0; i < nPaneNo; i++ )
	{
		GetItemRect( i, rect );
		if( rect.PtInRect( pt ) )
			return i;
	}
	return -1;

} /* #OF# CStatusTray::GetPaneFromPoint */


/////////////////////////////////////////////////////////////////////////////
// CStatusTray message handlers

/*========================================================
Method   : CStatusTray::DrawItem
=========================================================*/
/* #FN#
   This member function is called by the framework when a visual
   aspect of an owner-drawn status bar changes */
void
/* #AS#
   Nothing */
CStatusTray::
DrawItem(
	LPDRAWITEMSTRUCT lpDrawItemStruct /* #IN# Contains information about the type of drawing required */
)
{
//	TRACE0("CStatusTray::DrawItem invoked\n");
	if( m_bInitilized &&
		_IsFlagSet( g_Misc.ulState, MS_SHOW_INDICATORS ) )
	{
		CDC dc, dcMem;

		if( dc.Attach( lpDrawItemStruct->hDC ) )
		{
			if( dcMem.CreateCompatibleDC( &dc ) )
			{
				for( int i = 0; i < s_nStatusIconInfoNo; i++ )
				{
					if( lpDrawItemStruct->itemID == s_aStatusIconInfo[ i ].nPane )
					{
						/* Must be destroyed after LoadMappedBitmap */
						if( LoadMappedBitmap( s_aStatusIconInfo[ i ].pfGetBmpID(), i ) )
						{
							CBitmap *pOldBmp = (CBitmap *)dcMem.SelectObject( &m_pIcons[ i ] );
							if( pOldBmp )
							{
								CRect rc;
								GetItemRect( i, &rc );

								/* Paint the selected icon */
//								dc.BitBlt( lpDrawItemStruct->rcItem.left,
//										   lpDrawItemStruct->rcItem.top,
//										   16, 16, &dcMem, 0, 0, SRCCOPY );

								dc.StretchBlt( lpDrawItemStruct->rcItem.left,
											   lpDrawItemStruct->rcItem.top,
											   16, rc.Height() - 2, &dcMem,
											   0, 0, 16, 16, SRCCOPY );

								dcMem.SelectObject( pOldBmp );
							}
							break;
						}
					}
				}
				/* Now, clean up */
				dcMem.DeleteDC();
			}
			dc.Detach();
		}
	}
} /* #OF# CStatusTray::DrawItem */

#define NOTIFY_PARENT(pfHandler, uiEvent) \
	void CStatusTray::pfHandler( UINT nFlags, CPoint pt ) \
	{ \
		if( m_pParent ) \
			m_pParent->SendMessage( uiEvent, (WPARAM)nFlags, MAKELPARAM(pt.x, pt.y) ); \
		/*CStatusBar::pfHandler( nFlags, pt );*/ \
	}

NOTIFY_PARENT(OnLButtonDblClk, WM_LBUTTONDBLCLK)
NOTIFY_PARENT(OnMButtonDblClk, WM_LBUTTONDBLCLK)
NOTIFY_PARENT(OnRButtonDblClk, WM_RBUTTONDBLCLK)
/* The mouse pointer should be shown when
   the mouse is moved over the status bar */
NOTIFY_PARENT(OnMouseMove,     WM_MOUSEMOVE)
NOTIFY_PARENT(OnLButtonDown,   WM_LBUTTONDOWN)
NOTIFY_PARENT(OnMButtonDown,   WM_LBUTTONDOWN)
NOTIFY_PARENT(OnRButtonDown,   WM_RBUTTONDOWN)
NOTIFY_PARENT(OnLButtonUp,     WM_LBUTTONUP)
NOTIFY_PARENT(OnMButtonUp,     WM_RBUTTONUP)
NOTIFY_PARENT(OnRButtonUp,     WM_RBUTTONUP)

#undef NOTIFY_PARENT
