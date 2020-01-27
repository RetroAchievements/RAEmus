/****************************************************************************
File    : Atari800WinView.cpp
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# CAtari800WinView implementation file
@(#) #BY# Richard Lawrence, Tomasz Szymankowski, Piotr Fusik
@(#) #LM# 01.10.2003
*/

#include "StdAfx.h"
#include "Atari800Win.h"
#include "Atari800WinDoc.h"
#include "MainFrame.h"
#include "RomImagesBase.h"
#include "RomImagesDlg.h"
#include "Helpers.h"
#include "Atari800WinView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CAtari800WinView

IMPLEMENT_DYNCREATE(CAtari800WinView, CView)

BEGIN_MESSAGE_MAP(CAtari800WinView, CView)
	//{{AFX_MSG_MAP(CAtari800WinView)
	ON_WM_ERASEBKGND()
	ON_WM_DESTROY()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDBLCLK()
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, OnFilePrintPreview)
END_MESSAGE_MAP()

/*========================================================
Method   : CAtari800WinView::CAtari800WinView
=========================================================*/
/* #FN#
   Standard constructor */
CAtari800WinView::
CAtari800WinView()
{
	m_pMainWindow = NULL;
	m_pMainApp    = NULL;

} /* #OF# CAtari800WinView::CAtari800WinView */

/*========================================================
Method   : CAtari800WinView::~CAtari800WinView
=========================================================*/
/* #FN#
   Destructor */
CAtari800WinView::
~CAtari800WinView()
{
} /* #OF# CAtari800WinView::~CAtari800WinView */


/////////////////////////////////////////////////////////////////////////////
// CAtari800WinView implementation

/*========================================================
Method   : CAtari800WinView::StartAtariMachine
=========================================================*/
/* #FN#
   (Re)initializes an emulated Atari; invoked from OnDraw when
   g_hViewWnd is NULL */
void
/* #AS#
   Nothing */
CAtari800WinView::
StartAtariMachine()
{
	g_hViewWnd = GetSafeHwnd();
	if( !g_Screen.hDC )
		g_Screen.hDC = ::GetDC( g_hViewWnd );
	
	/* Cache boundaries of the view window */
	Input_RefreshBounds( g_hViewWnd, TRUE );

	if( Screen_InitialiseDisplay( FALSE ) &&
		Sound_Initialise( TRUE ) &&
		Input_Initialise( TRUE ) )
	{
		g_ulAtariState = ATARI_RUNNING;
		/* Right before we actually start the Atari, we write out a boot
		   failure message to the registry. During _normal_ closing of
		   windows we clear the "failure" mark in the WM_CLOSE handler.
		   This way if the Atari causes the Windows program to crash it's
		   easy to tell by testing for boot failure upon reading the
		   value from the registry initially*/
		_SetFlag( g_Misc.ulState, MS_LAST_BOOT_FAILED );

		WriteRegDWORD( NULL, REG_MISC_STATE, g_Misc.ulState );
	}
	else
	{
		g_ulAtariState = ATARI_UNINITIALIZED;
		Invalidate();
	}
	m_pMainWindow->UpdateStatus();

} /* #OF# CAtari800WinView::StartAtariMachine */


/////////////////////////////////////////////////////////////////////////////
// CAtari800WinView drawing

/*========================================================
Method   : CAtari800WinView::OnEraseBkgnd
=========================================================*/
/* #FN#
   The framework calls this member function when the CWnd object
   background needs erasing (for example, when resized) */
BOOL
/* #AS#
   Nonzero if it erases the background; otherwise 0 */
CAtari800WinView::
OnEraseBkgnd(
	CDC *pDC /* #IN# Specifies the device-context object */
)
{
	/* We need to repair the Atari screen when the user moves the main
	   emulator window around and parts of it are not visible on the
	   screen; also popup-menu have to be properly erased under 98/2000 */
	RedrawView( pDC, !ST_ATARI_FAILED );

	return TRUE;

} /* #OF# CAtari800WinView::OnEraseBkgnd */

/*========================================================
Method   : CAtari800WinView::OnDraw
=========================================================*/
/* #FN#
   Called by the framework to render an image of the document */
void
/* #AS#
   Nothing */
CAtari800WinView::
OnDraw(
	CDC *pDC /* #IN# Points to the device context to be used for rendering an image of the document */
)
{
	if( !_IsFlagSet( g_ulAtariState, ATARI_CLOSING ) )
	{
		if( ST_ATARI_FAILED )
		{
			/* First of all, clean up/redraw the Atari screen */
			RedrawView( pDC, _IsFlagSet( g_ulAtariState, ATARI_RUNNING ) && _IsFlagSet( g_ulAtariState, ATARI_CRASHED ) );

			if( _IsFlagSet( g_ulAtariState, ATARI_LOAD_FAILED ) )
			{
				_ClrFlag( g_ulAtariState, ATARI_LOAD_FAILED );

				if( IDYES == DisplayMessage( NULL, IDS_ERROR_BOOT, 0, MB_ICONWARNING | MB_YESNO ) )
				{
					CRomImagesDlg dlgRomImages;

					/* Use system palette if needed, we are on the GDI screen yet */
					Screen_UseSystemPalette();

					/* Show the "ROM Images" dialog box */
					if( IDOK == dlgRomImages.DoModal() )
					{
						if( CRomImagesDlg::RBT_NONE != dlgRomImages.m_eReboot || _IsFlagSet( g_ulAtariState, ATARI_UNINITIALIZED ) )
						{
							RestartEmulation( TRUE );
						}
					}
					Screen_UseAtariPalette( FALSE );
				}
			}
			m_pMainWindow->UpdateStatus( TRUE, 0 ); /* Force the status bar redrawing, speed 0 */
		}
		else
		{
			if( !g_hViewWnd )
			{
				EraseView( pDC ); /* Clear the screen */

				/* Restart the emulation */
				StartAtariMachine();
			}
			else if( _IsFlagSet( g_ulAtariState, ATARI_PAUSED | ATARI_NO_FOCUS ) )
			{
				RedrawView( pDC );
			}
		}
	}
} /* #OF# CAtari800WinView::OnDraw */

/*========================================================
Method   : CAtari800WinView::EraseView
=========================================================*/
/* #FN#
   Cleans up the view area */
inline
void
/* #AS#
   Nothing */
CAtari800WinView::
EraseView(
	CDC *pDC /* #IN# Points to the device context to be used for rendering an image of the document */
)
{
	RECT rc;
	GetClientRect( &rc );
	pDC->PatBlt( rc.left, rc.top, rc.right, rc.bottom, BLACKNESS );
}

/*========================================================
Method   : CAtari800WinView::RedrawView
=========================================================*/
/* #FN#
   Redraw the view area */
inline
void
/* #AS#
   Nothing */
CAtari800WinView::
RedrawView(
	CDC  *pDC, /* #IN# Points to the device context to be used for rendering an image of the document */
	BOOL bAllowRedraw /*=TRUE*/
)
{
	if( Screen_GetBuffer() && bAllowRedraw )
	{
		/* Redraw the Atari screen contents */
		Screen_Redraw( pDC->m_hDC );
	}
	else
		EraseView( pDC );
}

/////////////////////////////////////////////////////////////////////////////
// CAtari800WinView diagnostics

#ifdef _DEBUG
void
CAtari800WinView::
AssertValid() const
{
	CView::AssertValid();
}

void
CAtari800WinView::
Dump( CDumpContext& dc ) const
{
	CView::Dump( dc );
}

CAtari800WinDoc*
CAtari800WinView::
GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CAtari800WinDoc)));
	return (CAtari800WinDoc*)m_pDocument;
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
// CAtari800WinView message handlers

/*========================================================
Method   : CAtari800WinView::OnDestroy
=========================================================*/
/* #FN#
   The framework calls this member function to inform the CWnd
   object that it is being destroyed */
void
/* #AS#
   Nothing */
CAtari800WinView::
OnDestroy()
{
	g_ulAtariState = ATARI_UNINITIALIZED | ATARI_CLOSING;

	/* Clear last boot failed message so on reboot we don't think we crashed */
	_ClrFlag( g_Misc.ulState, MS_LAST_BOOT_FAILED );
	WriteRegDWORD( NULL, REG_MISC_STATE, g_Misc.ulState );

	if( _IsFlagSet( g_Input.ulState, IS_CAPTURE_MOUSE ) )
		ClipCursor( NULL );

	if( _IsFlagSet( g_Input.ulState, IS_CAPTURE_CTRLESC ) )
		/* Enable task switching */
		Input_EnableEscCapture( FALSE );

	Atari_Exit( 0 );

	if( g_Screen.hDC && g_hViewWnd )
		::ReleaseDC( g_hViewWnd, g_Screen.hDC );
	g_Screen.hDC = NULL;
	g_hViewWnd = NULL;

	CView::OnDestroy();

} /* #OF# CAtari800WinView::OnDestroy */

/*========================================================
Method   : CAtari800WinView::OnPrint
=========================================================*/
/* #FN#
   Called by the framework to print or preview a page of the document */
void
/* #AS#
   Nothing */
CAtari800WinView::
OnPrint(
	CDC        *pDC,  /* #IN# Points to the printer device context */
	CPrintInfo *pInfo /* #IN# Points to structure that describes the current print job */
)
{
	int xRes;	/* Horizontal resolution of printer */
	int nMult;	/* How many times to stretch the Atari screen */

	/* We need it to clip and flip the Atari screen vertically,
	   in case StretchDIBits can't do it. */
	/* Alloc temporary buffer for flipped Atari screen */
	UBYTE *pTmpScr = (UBYTE *)malloc(ATARI_VIS_WIDTH * Screen_HEIGHT);
	UBYTE *pSource = Screen_GetBuffer() + ATARI_HORZ_CLIP;
	UBYTE *pTarget = pTmpScr + ATARI_VIS_WIDTH * (Screen_HEIGHT - 1);
	int y;

	struct ScreenInterParms_t dipInfo;
	dipInfo.dwMask = DIP_BITMAPINFO;

	Screen_GetInterParms( &dipInfo );

	ASSERT(NULL != pTmpScr);
	/* We are going to use g_Screen.lpbmi, where we already have the
	   Atari palette. We will only modify biWidth and biHeight. */
	/* Make backup of original values */
	int nWidth  = dipInfo.pBitmapInfo->bmiHeader.biWidth;
	int nHeight = dipInfo.pBitmapInfo->bmiHeader.biHeight;

	/* Set the size of the temporary screen */
	dipInfo.pBitmapInfo->bmiHeader.biWidth  = ATARI_VIS_WIDTH;
	dipInfo.pBitmapInfo->bmiHeader.biHeight = Screen_HEIGHT;

	/* Make flipped screen */
	for( y = 0; y < Screen_HEIGHT; y++ )
	{
		memcpy( pTarget, pSource, ATARI_VIS_WIDTH );
		pSource += Screen_WIDTH;
		pTarget -= ATARI_VIS_WIDTH;
	}
	/* Find the largest integral multiple of width on the page */
	xRes  = pDC->GetDeviceCaps( HORZRES );
	nMult = xRes / ATARI_VIS_WIDTH;
	/* And stretch the original DIB to that size, centering it
	   horizontally on the page */
	StretchDIBits( pDC->m_hDC, (xRes - ATARI_VIS_WIDTH * nMult) / 2, 0,
				   ATARI_VIS_WIDTH * nMult, Screen_HEIGHT * nMult,
				   0, 0, ATARI_VIS_WIDTH, Screen_HEIGHT,
				   pTmpScr, dipInfo.pBitmapInfo, DIB_RGB_COLORS, SRCCOPY );

	/* Restore original values */
	dipInfo.pBitmapInfo->bmiHeader.biWidth  = nWidth;
	dipInfo.pBitmapInfo->bmiHeader.biHeight = nHeight;
	free(pTmpScr);

	m_pMainWindow->CleanScreen();
//	CView::OnPrint( pDC, pInfo );

} /* #OF# CAtari800WinView::OnPrint */

/*========================================================
Method   : CAtari800WinView::OnPreparePrinting
=========================================================*/
/* #FN#
   Called by the framework before a document is printed or previewed */
BOOL
/* #AS#
   Nothing */
CAtari800WinView::
OnPreparePrinting(
	CPrintInfo *pInfo /* #IN# Points to structure that describes the current print job */
)
{
	/* Disable selection of pages in the Print dialog */
	pInfo->m_pPD->m_pd.Flags |= PD_NOPAGENUMS;

	BOOL bResult = DoPreparePrinting( pInfo );
	m_pMainWindow->CleanScreen();

	return bResult;

} /* #OF# CAtari800WinView::OnPreparePrinting */

/*========================================================
Method   : CAtari800WinView::OnInitialUpdate
=========================================================*/
/* #FN#
   Called by the framework after the view is first attached to the
   document, but before the view is initially displayed */
void
/* #AS#
   Nothing */
CAtari800WinView::
OnInitialUpdate()
{
	CView::OnInitialUpdate();

	m_pMainWindow = (CMainFrame *)AfxGetMainWnd();
	ASSERT(NULL != m_pMainWindow);
	m_pMainWindow->SetMainView( this );

	m_pMainApp = (CAtari800WinApp *)AfxGetApp();
	ASSERT(NULL != m_pMainApp);

	Screen_SetWindowSize( GetSafeHwnd(), SWP_NOMOVE | /*SWP_SHOWWINDOW |*/ SWP_NOSENDCHANGING );

} /* #OF# CAtari800WinView::OnInitialUpdate */

/*========================================================
Method   : CAtari800WinView::OnMouseMove
=========================================================*/
/* #FN#
   The framework calls this member function when the mouse cursor moves */
void
/* #AS#
   Nothing */
CAtari800WinView::
OnMouseMove(
	UINT nFlags, /* #IN# Indicates whether various virtual keys are down */
	CPoint point /* #IN# Specifies the x- and y-coordinate of the cursor */
)
{
	if( !_IsFlagSet( g_Input.ulState, IS_CAPTURE_MOUSE ) )
		Screen_ShowMousePointer( TRUE );

	CView::OnMouseMove( nFlags, point );

} /* #OF# CAtari800WinView::OnMouseMove */

/*========================================================
Method   : CAtari800WinView::OnLButtonDown
=========================================================*/
/* #FN#
   The framework calls this member function when the user presses the left mouse button */
void
/* #AS#
   Nothing */
CAtari800WinView::
OnLButtonDown(
	UINT nFlags, /* #IN# Indicates whether various virtual keys are down */
	CPoint point /* #IN# Specifies the x- and y-coordinate of the cursor */
)
{
	if( _IsFlagSet( g_Input.ulState, IS_CAPTURE_MOUSE ) )
		INPUT_mouse_buttons |= 1;

	CView::OnLButtonDown( nFlags, point );

} /* #OF# CAtari800WinView::OnLButtonDown */

/*========================================================
Method   : CAtari800WinView::OnLButtonUp
=========================================================*/
/* #FN#
   The framework calls this member function when the user releases the left mouse button */
void
/* #AS#
   Nothing */
CAtari800WinView::
OnLButtonUp(
	UINT nFlags, /* #IN# Indicates whether various virtual keys are down */
	CPoint point /* #IN# Specifies the x- and y-coordinate of the cursor */
)
{
	if( _IsFlagSet( g_Input.ulState, IS_CAPTURE_MOUSE ) )
		INPUT_mouse_buttons &= ~1;
	
	CView::OnLButtonUp( nFlags, point );

} /* #OF# CAtari800WinView::OnLButtonUp */

/*========================================================
Method   : CAtari800WinView::OnLButtonDblClk
=========================================================*/
/* #FN#
   The framework calls this member function when the user double-clicks the left mouse button */
void
/* #AS#
   Nothing */
CAtari800WinView::
OnLButtonDblClk(
	UINT nFlags, /* #IN# Indicates whether various virtual keys are down */
	CPoint point /* #IN# Specifies the x- and y-coordinate of the cursor */
)
{
	if( _IsFlagSet( g_Input.ulState, IS_CAPTURE_MOUSE ) )
		INPUT_mouse_buttons |= 1;
	
	CView::OnLButtonDblClk( nFlags, point );

} /* #OF# CAtari800WinView::OnLButtonDblClk */

/*========================================================
Method   : CAtari800WinView::OnRButtonDown
=========================================================*/
/* #FN#
   The framework calls this member function when the user presses the right mouse button */
void
/* #AS#
   Nothing */
CAtari800WinView::
OnRButtonDown(
	UINT nFlags, /* #IN# Indicates whether various virtual keys are down */
	CPoint point /* #IN# Specifies the x- and y-coordinate of the cursor */
)
{
	if( _IsFlagSet( g_Input.ulState, IS_CAPTURE_MOUSE ) )
		INPUT_mouse_buttons |= 2;
	
	CView::OnRButtonDown( nFlags, point );

} /* #OF# CAtari800WinView::OnRButtonDown */

/*========================================================
Method   : CAtari800WinView::OnRButtonUp
=========================================================*/
/* #FN#
   The framework calls this member function when the user releases the right mouse button */
void
/* #AS#
   Nothing */
CAtari800WinView::
OnRButtonUp(
	UINT nFlags, /* #IN# Indicates whether various virtual keys are down */
	CPoint point /* #IN# Specifies the x- and y-coordinate of the cursor */
)
{
	if( _IsFlagSet( g_Input.ulState, IS_CAPTURE_MOUSE ) )
		INPUT_mouse_buttons &= ~2;
	
	CView::OnRButtonUp( nFlags, point );

} /* #OF# CAtari800WinView::OnRButtonUp */

/*========================================================
Method   : CAtari800WinView::OnRButtonDblClk
=========================================================*/
/* #FN#
   The framework calls this member function when the user double-clicks the right mouse button */
void
/* #AS#
   Nothing */
CAtari800WinView::
OnRButtonDblClk(
	UINT nFlags, /* #IN# Indicates whether various virtual keys are down */
	CPoint point /* #IN# Specifies the x- and y-coordinate of the cursor */
)
{
	if( _IsFlagSet( g_Input.ulState, IS_CAPTURE_MOUSE ) )
		INPUT_mouse_buttons |= 2;
	
	CView::OnRButtonDblClk( nFlags, point );

} /* #OF# CAtari800WinView::OnRButtonDblClk */
