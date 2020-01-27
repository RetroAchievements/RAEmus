/****************************************************************************
File    : display_win.c
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Implementation of display handling API
@(#) #BY# Richard Lawrence, Tomasz Szymankowski, Piotr Fusik
@(#) #LM# 03.02.2004
*/

#include <stdio.h>
#include <crtdbg.h>
#include <ddraw.h>
#include <mmsystem.h>
#include <windows.h>
#include <windowsx.h>
#include "WinConfig.h"
#include "Resource.h"
#include "Helpers.h"
#include "Debug.h"
#include "atari800.h"
#include "globals.h"
#include "macros.h"
#include "registry.h"
#include "sound_win.h"
#include "misc_win.h"
#include "input_win.h"
#include "avisave.h"
#include "directdraw.h"
#include "display_win.h"
#include "screen.h"
#include "hq.h"


#define FULL_SUP_MODES_NO			9
#define WIND_SUP_MODES_NO			2

/* Stretching routines have been optimized by Piotr Fusik. */

#define PFUSIK_ASM
#pragma warning (disable: 4731)

/* WARNING!: Robust interpolation routines in C seem to be faster
   on Pentium Pro systems (Celeron, Pentium II, Pentium III etc.)
*/
#define INTEL_ASM

/* Public objects */

struct ScreenCtrl_t g_Screen =
{
	DEF_SCREEN_STATE,
	0L,
	0L,
	DEF_STRETCH_MODE,
	DEF_MEMORY_TYPE,
	NULL,
	DEF_SHOW_CURSOR,
	20,
	0,
	0,
	FALSE,
	NULL,
	/* Palette */
	{
		DEF_CLR_BLACK_LEVEL,
		DEF_CLR_WHITE_LEVEL,
		DEF_CLR_SATURATION,
		DEF_CLR_CONTRAST,
		DEF_CLR_BRIGHTNESS,
		DEF_CLR_GAMMA,
		TRUE
	}
};

struct s_Buffer_t s_Buffer =
{
	NULL, NULL, NULL, NULL, NULL
};

void (*Atari_DisplayScreen_ptr)( void );

/* Private objects */

static LPBITMAPINFO s_lpbmi = NULL;
static PALETTEENTRY s_pe[ PAL_ENTRIES_NO ];

static LONG  s_lWndStyleBack = 0;
static ULONG s_ulScreenMode  = 0;
static int   s_nStretchMode  = 0;
static int   s_nMemoryType   = 0;
static int   s_nRequestRate  = 0;
static BOOL  s_bModeChanging = FALSE;
static BOOL  s_bModeChanged  = FALSE;
static BOOL  s_bIColorMap    = FALSE;
static RECT  s_rcSource;
static RECT  s_rcDestin;

static struct
{
	HDC     hDC;		/* Compatible device context */
	HBITMAP hBmp;		/* Display screenshot bitmap */
	int     nWidth;		/* Width of this bitmap      */
	int     nHeight;	/* Height of this bitmap     */
}
s_Redraw =
{
	NULL, NULL, 0, 0
};

static struct
{
	LONG nPitch;
	LONG nWidth;
	LONG nHeight;
}
s_Surface =
{
	0L, Screen_WIDTH, Screen_HEIGHT + 16
};

#define FLIPPED_BUFFERS_NO	2 /* Triple-buffering */

static struct
{
	/* Drawing flags and other params of DirectDraw */
	DWORD    dwBlitFlags;
	DWORD    dwFlipFlags;
	DWORD    dwDDFxFlags[ BLIT_EFFECTS_NO ];
	DDBLTFX  fxBltEffects; /* The FX effects for blits */
	DDBLTFX *pfxBltEffects;
	int      nFlipBuffers;
}
s_DDraw =
{
	DDBLT_WAIT, DDFLIP_WAIT,
	{
		DDBLTFX_MIRRORLEFTRIGHT,
		DDBLTFX_MIRRORUPDOWN,
		DDBLTFX_NOTEARING
	},
	{ 0 }, NULL, 0
};

static UBYTE s_anIColorMap[ PAL_ENTRIES_NO ][ PAL_ENTRIES_NO ];

static void Screen_DDraw                       ( void );
static void Screen_DDraw_Double                ( void );
static void Screen_DDraw_Double_Interpolation  ( void );
static void Screen_DDraw_Double_Scanlines      ( void );
static void Screen_DDraw_Double_HiEnd          ( void );
static void Screen_DDraw_Full_Blt              ( void );
static void Screen_DDraw_Full_Blt_Interpolation( void );
static void Screen_DDraw_Full_Blt_Scanlines    ( void );
static void Screen_DDraw_1024_Interpolation    ( void );
static void Screen_DDraw_1024_Scanlines        ( void );
static void Screen_GDI                         ( void );
static void Screen_GDI_Double                  ( void );
static void Screen_GDI_Double_Interpolation    ( void );
static void Screen_GDI_Double_Scanlines        ( void );
static void Screen_GDI_Double_HiEnd            ( void );

/* data for HiEnd stretching mode */

static unsigned char *hqSource = NULL, *hqTarget = NULL;
static LPBITMAPINFO hqlpbmi = NULL;

/* Macro-definitions */

#ifdef _DEBUG
#define ServeDDError( nUID, hResult, bQuit ) \
		ShowDDError( nUID, hResult, bQuit, __FILE__, __LINE__ )
#else /*_DEBUG*/
#define ServeDDError( nUID, hResult, bQuit ) \
		ShowDDError( nUID, hResult, bQuit )
#endif /*_DEBUG*/


/*========================================================
Function : ShowDDError
=========================================================*/
/* #FN#
   Shows DirectDraw error */
static
void
/* #AS#
   Nothing */
ShowDDError(
	UINT    nUID,    /* #IN# */
	HRESULT hResult, /* #IN# */
	BOOL    bQuit    /* #IN# */
#ifdef _DEBUG
  , char   *pszFile,
	DWORD   dwLine
#endif /*_DEBUG*/
)
{
	char szError [ LOADSTRING_SIZE_S + 1 ];
	char szAction[ LOADSTRING_SIZE_L + 1 ];

#ifdef _DEBUG
	Log_print( "DirectDraw error: %s@%ld", pszFile, dwLine );
#endif /*_DEBUG*/

	/* Get us back to a GDI display and stop making noises */
	Screen_SetSafeDisplay( TRUE );
	Sound_Clear( FALSE, FALSE );

	/* Get the error string and present it to the user */
	DD_GetErrorString( hResult, szError, LOADSTRING_SIZE_S );
	DisplayMessage( NULL, IDS_DDERR_PROMPT, IDS_DDERR_HDR, MB_ICONSTOP | MB_OK, _LoadStringLx( nUID, szAction ), szError );

	/* Start a quit (this will end up in Atari_Exit()) */
	if( bQuit )
	{
		/* Make sure the atari is turned off */
		g_ulAtariState = ATARI_UNINITIALIZED;
		PostMessage( g_hMainWnd, WM_CLOSE, 0, 0L );
	}
	else
		Sound_Restart();

} /* #OF# ShowDDError */

/*========================================================
Function : Screen_GetInterParms
=========================================================*/
/* #FN#
   Returns screen information structure */
void
/* #AS#
   Nothing */
Screen_GetInterParms(
	struct ScreenInterParms_t *pInfo
)
{
	/* Send a pointer to the BITMAPINFO structure */
	if( pInfo->dwMask & DIP_BITMAPINFO )
		pInfo->pBitmapInfo = s_lpbmi;

	/* Make copy of sample size value */
	if( pInfo->dwMask & DIP_PALENTRIES )
		CopyMemory( &pInfo->pePalette, &s_pe, sizeof(PALETTEENTRY) * PAL_ENTRIES_NO );

} /* #OF# Screen_GetInterParms */

/*========================================================
Function : AtariPlot
=========================================================*/
/* #FN#
   Routines for plotting on the Atari screen (taken out of ui.c, which
   I don't use currently */
static
void
/* #AS#
   Nothing */
AtariPlot(
	UBYTE *pScreen, /* #IN# */
	int    nFg,     /* #IN# */
	int    nBg,     /* #IN# */
	int    nCh,     /* #IN# */
	int    x,       /* #IN# */
	int    y        /* #IN# */
)
{
	int   nOffset = (nCh & 0x07f) * 8;
	int   i, j;
	char  szCharset[ 1024 ];
	char *ptr;

	if( !_IsFlagSet( g_ulAtariState, ATARI_RUNNING ) )
		return;

	MEMORY_GetCharset( szCharset );

	ptr = &pScreen[ 32 ]; //[24 * Screen_WIDTH + 32 ];

	for( i = 0; i < 8; i++ )
	{
		UBYTE data = szCharset[ nOffset++ ];

		for( j = 0; j < 8; j++ )
		{
			int nPixel;

			if( data & 0x80 )
				nPixel = nFg;
			else
				nPixel = nBg;

			ptr[ (y * 8 + i) * Screen_WIDTH + (x * 8 + j) ] = nPixel;

			data = data << 1;
		}
	}
} /* #OF# AtariPlot */

/*========================================================
Function : AtariPrint
=========================================================*/
/* #FN#
   Routines for printing on the Atari screen (taken out of ui.c, which
   I don't use currently */
static
void
/* #AS#
   Nothing */
AtariPrint(
	UBYTE *pScreen,   /* #IN# */
	int    nFg,       /* #IN# */
	int    nBg,       /* #IN# */
	char  *pszString, /* #IN# */
	int    x,         /* #IN# */
	int    y          /* #IN# */
)
{
	while( *pszString )
	{
		AtariPlot( pScreen, nFg, nBg, *pszString++, x, y );
		x++;
	}
} /* #OF# AtariPrint */

/*========================================================
Function : CenterPrint
=========================================================*/
/* #FN#
   Routines for printing on the Atari screen (taken out of ui.c, which
   I don't use currently */
static
void
/* #AS#
   Nothing */
CenterPrint(
	UBYTE *pScreen,   /* #IN# */
	int    nFg,       /* #IN# */
	int    nBg,       /* #IN# */
	char  *pszString, /* #IN# */
	int    y          /* #IN# */
)
{
	AtariPrint( pScreen, nFg, nBg, pszString, (40 - strlen( pszString )) / 2, y );

} /* #OF# CenterPrint */

/*========================================================
Function : DrawPaused
=========================================================*/
/* #FN#
   Draws paused screen */
static
void
/* #AS#
   Nothing */
DrawPaused(
	UBYTE *pScreen  /* #IN# */
)
{
	char   szBuffer[ LOADSTRING_SIZE_S + 1 ];
	int    i;

	sprintf( szBuffer, _LoadStringSx( IDS_TITLE_PAUSED, szBuffer ), "" );
	CenterPrint( pScreen, 255, 0, szBuffer, 0 );

	for( i = 0; i < s_DDraw.nFlipBuffers + 1 /* + primary surface */; i++ )
		/* We have to draw the text also on a GDI surface */
		PLATFORM_DisplayScreen();

} /* #OF# DrawPaused */

/*========================================================
Function : RestoreSurfaces
=========================================================*/
/* #FN#
   Restores all the DirectDraw Surfaces */
static
BOOL
/* #AS#
   TRUE if succeeded, otherwise FALSE */
RestoreSurfaces( void )
{
	HRESULT hResult;

	/* Restore Memory Surface */
	if( DD_IsSurfaceCreated( stOffscrn ) &&
		FAILED(hResult = DD_SurfaceIsLost ( stOffscrn )) &&
		FAILED(hResult = DD_SurfaceRestore( stOffscrn )) )
	{
		return FALSE;
	}
	/* Restore Primary Surface */
	if( DD_IsSurfaceCreated( stPrimary ) &&
		FAILED(hResult = DD_SurfaceIsLost ( stPrimary )) &&
		FAILED(hResult = DD_SurfaceRestore( stPrimary )) )
	{
		return FALSE;
	}
	/* It is a good place to clear screen here because there may
	   be some graphics glitches after the surfaces restoring */
	Screen_Clear( FALSE, TRUE );

	return TRUE;

} /* #OF# RestoreSurfaces */

/*========================================================
Function : SetupPrimarySurface
=========================================================*/
/* #FN#
   Sets up DirectDraw Primary Surface */
static
BOOL
/* #AS#
   TRUE if succeeded, otherwise FALSE */
SetupPrimarySurface( void )
{
	HRESULT hResult;

	DD_SurfaceParams_t ddsp;
	ZeroMemory( &ddsp, sizeof(DD_SurfaceParams_t) );
	ddsp.dwFlags = DDSD_CAPS;
	ddsp.dwCaps  = DDSCAPS_PRIMARYSURFACE;

	if( s_DDraw.nFlipBuffers )
	{
		ddsp.dwFlags |= DDSD_BACKBUFFERCOUNT;
		ddsp.dwCaps  |= DDSCAPS_FLIP | DDSCAPS_COMPLEX;
		ddsp.dwBackBufferCount = s_DDraw.nFlipBuffers;
	}
	/* Create the primary DirectDraw surface */
	hResult = DD_SurfaceCreate( &ddsp, stPrimary );
	if( FAILED(hResult) && s_DDraw.nFlipBuffers > 1 )
	{
		for( ; s_DDraw.nFlipBuffers > 1; )
		{
			Log_print( "Initialization of %d-buffering failed...", s_DDraw.nFlipBuffers );
			ddsp.dwBackBufferCount = --s_DDraw.nFlipBuffers;
			if( SUCCEEDED(hResult = DD_SurfaceCreate( &ddsp, stPrimary )) )
				break;
		}
	}
	if( FAILED(hResult) )
	{
		DD_SurfaceRelease( stPrimary );
		ServeDDError( IDS_DDERR_ALLOC_PRIMARY, hResult, FALSE );
		return FALSE;
	}
	/* Get the back buffer when flipping the surfaces is active */
	if( s_DDraw.nFlipBuffers )
	{
		DD_SurfaceGetAttached( DDSCAPS_BACKBUFFER );
		if( FAILED(hResult) )
		{
			DD_SurfaceRelease( stPrimary );
			ServeDDError( IDS_DDERR_ALLOC_PRIMARY, hResult, FALSE );
			return FALSE;
		}
	}
	/* Create and attach the clipper object when in DirectDraw windowed */
	if( g_hViewWnd && _IsFlagSet( g_Screen.ulState, SM_MODE_WIND ) )
	{
		hResult = DD_ClipperCreate( 0 );
		if( SUCCEEDED(hResult) )
		{
			hResult = DD_ClipperSetHWnd( 0, g_hViewWnd );
			/* Attach the clipper to a primary surface */
			if( SUCCEEDED(hResult) )
				hResult = DD_SurfaceSetClipper( stPrimary, TRUE );
		}
		if( FAILED(hResult) )
		{
			DD_ClipperRelease();
			DD_SurfaceRelease( stPrimary );
			ServeDDError( IDS_DDERR_ALLOC_PRIMARY, hResult, FALSE );
			return FALSE;
		}
	}
	return TRUE;

} /* #OF# SetupPrimarySurface */

/*========================================================
Function : SetupMemorySurface
=========================================================*/
/* #FN#
   Sets up DirectDraw Memory Surface */
static
BOOL
/* #AS#
   TRUE if succeeded, otherwise FALSE */
SetupMemorySurface( void )
{
	HRESULT hResult = DD_OK; /* Important! */

	DD_SurfaceParams_t ddsp;
	ZeroMemory( &ddsp, sizeof(DD_SurfaceParams_t) );
	ddsp.dwHeight = s_Surface.nHeight;
	ddsp.dwWidth  = s_Surface.nWidth;

	if( _IsFlagSet( g_Screen.ulState, SM_MODE_FULL ) )
	{
		ddsp.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
		ddsp.ddpfFormat.dwSize        = sizeof(DDPIXELFORMAT);
		ddsp.ddpfFormat.dwFlags       = DDPF_RGB | DDPF_PALETTEINDEXED8;
		ddsp.ddpfFormat.dwRGBBitCount = 8;
	}
	else
		ddsp.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;

	/* Try to allocate a video memory for offscreen surface */
	if( MEMORY_VIDEO == g_Screen.nMemoryType )
	{
		ddsp.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY | DDSCAPS_LOCALVIDMEM;

		hResult = DD_SurfaceCreate( &ddsp, stOffscrn );
		if( FAILED(hResult) )
			Log_print( "Video memory allocation failed, trying AGP memory..." );
	}
	/* Try to allocate an AGP memory for offscreen surface */
	if( MEMORY_AGP == g_Screen.nMemoryType || FAILED(hResult) )
	{
		ddsp.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY | DDSCAPS_NONLOCALVIDMEM;

		hResult = DD_SurfaceCreate( &ddsp, stOffscrn );
		if( FAILED(hResult) )
			Log_print( "AGP memory allocation failed, trying system memory..." );
	}
	/* Try to allocate a system memory for offscreen surface */
	if( MEMORY_SYSTEM == g_Screen.nMemoryType || FAILED(hResult) )
	{
		ddsp.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;

		hResult = DD_SurfaceCreate( &ddsp, stOffscrn );
		if( FAILED(hResult) )
		{
			DD_SurfaceRelease( stOffscrn );
			ServeDDError( IDS_DDERR_ALLOC_OFFSCR, hResult, FALSE );

			Log_print( "Could not allocate system memory surface!" );
			return FALSE;
		}
	}
	return TRUE;

} /* #OF# SetupMemorySurface */

/*========================================================
Function : SetDirectPointers
=========================================================*/
/* #FN#
   Sets up pointers directly to memory surfaces */
static
BOOL
/* #AS#
   TRUE if succeeded, otherwise FALSE */
SetDirectPointers( void )
{
	if( _IsFlagSet( g_Screen.ulState, SM_MODE_FULL ) && !_IsFlagSet( g_Screen.ulState, SM_OPTN_SAFE_MODE ) )
	{
		HRESULT hResult;
		LPVOID  pSurface = NULL;



		hResult = DD_SurfaceLock( stOffscrn, DDLOCK_WAIT, &pSurface, &s_Surface.nPitch );
		if( DDERR_SURFACELOST == hResult && RestoreSurfaces() )
			/* Try to lock the surface once again */
			hResult = DD_SurfaceLock( stOffscrn, DDLOCK_WAIT, &pSurface, &s_Surface.nPitch );

		if( FAILED(hResult) )
		{
			DD_SurfaceRelease( stOffscrn );
			ServeDDError( IDS_DDERR_ALLOC_OFFSCR, hResult, FALSE );
			return FALSE;
		}
		/* It is VERY dangerous method, especially when using video
		   memory for off-screen surfaces (unfortunately DirectX 3.0
		   doesn't support client memory surfaces). But the method is
		   still the fastest one available here.
		*/
		if( _IsFlagSet( g_Screen.ulState, SM_ATTR_STRETCHED ) && g_Screen.nStretchMode != STRETCH_PIXELDOUBLING )
		{
			/* Set pointer to the offscreen surface buffer */
			s_Buffer.pTarget = pSurface;
		}
		else
			s_Buffer.pSource = pSurface;

		DD_SurfaceUnlock( stOffscrn );
	}
	return TRUE;

} /* #OF# SetDirectPointers */

/*========================================================
Function : ReleaseObjects
=========================================================*/
/* #FN#
   Releases all the DirectDraw Objects */
static
void
/* #AS#
   Nothing */
ReleaseObjects(
	BOOL bReleaseObject /* #IN# */
)
{
	/* Release Palette Object */
	DD_PaletteRelease(); /* It decreases only the palette's reference count */

	/* Release Clipper Object */
	DD_ClipperRelease(); /* It decreases only the clipper's reference count */
	DD_SurfaceSetClipper( stPrimary, FALSE );

	/* Release surfaces */
	DD_SurfaceRelease( stOffscrn );
	DD_SurfaceRelease( stPrimary );

	/* Release DirectDraw object and restore display mode */
	if( bReleaseObject )
		DD_ObjectRelease();

} /* #OF# ReleaseObjects */

/*========================================================
Function : BlitSurface
=========================================================*/
/* #FN#
   Performs a bit block transfer to the primary surface from a memory one */
static
HRESULT
/* #AS#
   DD_OK if succeeded */
BlitSurface( void )
{
	HRESULT hResult = DD_OK;

	if( _IsFlagSet( g_Screen.ulState, SM_OPTN_DDVBL_WAIT ) )
		DD_WaitForVBlank( DDWAITVB_BLOCKBEGIN );

	if( SUCCEEDED(hResult = DD_SurfaceBlit( stPrimary, &s_rcDestin, stOffscrn, &s_rcSource, s_DDraw.dwBlitFlags, s_DDraw.pfxBltEffects )) )
	{
		if( _IsFlagSet( g_Screen.ulState, SM_OPTN_DDVBL_WAIT ) && (g_Screen.nCurrentRate > 85 || g_Screen.nRequestRate < 0) )
			DD_WaitForVBlank( DDWAITVB_BLOCKBEGIN );
	}
	return hResult;

} /* #OF# BlitSurface */

/*========================================================
Function : FlipSurface
=========================================================*/
/* #FN#
   Writes the next frame of video to a new surface */
static
HRESULT
/* #AS#
   DD_OK if succeeded */
FlipSurface( void )
{
	HRESULT hResult = DD_OK;

	if( SUCCEEDED(hResult = DD_SurfaceBlit( stBackBuf, &s_rcDestin, stOffscrn, &s_rcSource, s_DDraw.dwBlitFlags, s_DDraw.pfxBltEffects )) )
	{
		/* Flipping is performed by switching pointers that the display device
		   uses for referencing memory, not by copying surface memory */
		if( _IsFlagSet( g_Screen.ulState, SM_OPTN_DDVBL_WAIT ) && (g_Screen.nCurrentRate > 85 || g_Screen.nRequestRate < 0) )
		{
			if( _IsFlagSet( g_Screen.ulVideoCaps, DD_CAPS_FLIPINTERVAL ) )
				hResult = DD_SurfaceFlip( s_DDraw.dwFlipFlags | DDFLIP_INTERVAL2 );
			else
			{
				hResult = DD_SurfaceFlip( s_DDraw.dwFlipFlags );
				if( SUCCEEDED(hResult) )
					hResult = DD_SurfaceBlitFast( stBackBuf, stPrimary, DDBLTFAST_WAIT );
				if( SUCCEEDED(hResult) )
					hResult = DD_SurfaceFlip( s_DDraw.dwFlipFlags );
			}
		}
		else
			hResult = DD_SurfaceFlip( s_DDraw.dwFlipFlags );
	}
	return hResult;

} /* #OF# FlipSurface */

/*========================================================
Function : SetupPalette
=========================================================*/
/* #FN#
   Creates and attaches a palette to DirectDraw surfaces */
static
BOOL
/* #AS#
   TRUE if succeeded, otherwise FALSE */
SetupPalette( void )
{
	if( _IsFlagSet( g_Screen.ulState, SM_MODE_FULL ) )
	{
		HRESULT hResult;

		if( DD_IsObjectCreated() )
		{
			/* Release Palette Object if necessary */
			DD_PaletteRelease();

			hResult = DD_PaletteCreate( DDPCAPS_8BIT | DDPCAPS_ALLOW256, (LPPALETTEENTRY)&s_pe );
			if( FAILED(hResult) )
			{
				ServeDDError( IDS_DDERR_ALLOC_PALETTE, hResult, FALSE );
				return FALSE;
			}
		}
		else
			return FALSE; /* Something went wrong, e.g. registry are corrupted */

		/* This avoids some glitches that may appear during the palette changing */
//		DD_WaitForVBlank( DDWAITVB_BLOCKBEGIN );

		if( DD_IsSurfaceCreated( stPrimary ) )
		{
			DD_SurfaceSetPalette( stPrimary, FALSE );
			hResult = DD_SurfaceSetPalette( stPrimary, TRUE );

			if( DDERR_SURFACELOST == hResult && RestoreSurfaces() )
				hResult = DD_SurfaceSetPalette( stPrimary, TRUE );

			if( FAILED(hResult) )
			{
				ServeDDError( IDS_DDERR_SETPAL_PRIMARY, hResult, FALSE );
				return FALSE;
			}
		}
		if( DD_IsSurfaceCreated( stOffscrn ) )
		{
			DD_SurfaceSetPalette( stOffscrn, FALSE );
			/* Don't actually want to set a palette for this if it's windowed
			   DDRAW, because that's what we use this surface for - a colorspace
			   conversion to whatever the primary surface is via StretchDIB */
			hResult = DD_SurfaceSetPalette( stOffscrn, TRUE );

			if( DDERR_SURFACELOST == hResult && RestoreSurfaces() )
				hResult = DD_SurfaceSetPalette( stOffscrn, TRUE );

			if( FAILED(hResult) )
			{
				ServeDDError( IDS_DDERR_SETPAL_MEMORY, hResult, FALSE );
				return FALSE;
			}
		}
		/* Using of this may seem questionable but helps under 2000 with double buffering */
		DD_PaletteSetEntries( 0, 0, PAL_ENTRIES_NO, (LPPALETTEENTRY)&s_pe );
	}
	return TRUE;

} /* #OF# SetupPalette */

/*========================================================
Function : Screen_UseSystemPalette
=========================================================*/
/* #FN#
   Forces using system palette in full-screen modes */
BOOL
/* #AS#
   TRUE if succeeded, otherwise FALSE */
Screen_UseSystemPalette( void )
{
	if( _IsFlagSet( g_Screen.ulState, SM_MODE_FULL ) )
	{
		if( _IsFlagSet( g_Screen.ulState, SM_OPTN_USE_SYSPAL ) && !g_Screen.Pal.bUseSysCol )
		{
			PALETTEENTRY peEntry = { 196, 196, 196, PC_NOCOLLAPSE };

			/* Copy system palette entries for menus and dialogs looking a little
			   bit better in 8-bit full-screen modes */
//			GetSystemPaletteEntries( NULL, 0,   10, &s_pe[   0 ] );
//			GetSystemPaletteEntries( NULL, 246, 10, &s_pe[ 246 ] );

			/* We do not need all the system colors; 10 of them should be enough */
			GetSystemPaletteEntries( NULL, 1,   1, &s_pe[   1 ] );
			GetSystemPaletteEntries( NULL, 4,   2, &s_pe[   4 ] );
			GetSystemPaletteEntries( NULL, 7,   1, &s_pe[   7 ] );

			memcpy( &s_pe[ 8 ], &peEntry, sizeof(PALETTEENTRY) );
			memcpy( &s_pe[ 9 ], &peEntry, sizeof(PALETTEENTRY) );

			GetSystemPaletteEntries( NULL, 246, 3, &s_pe[ 246 ] );
			GetSystemPaletteEntries( NULL, 252, 1, &s_pe[ 252 ] );
			GetSystemPaletteEntries( NULL, 254, 2, &s_pe[ 254 ] );

			DD_PaletteSetEntries( 0, 0, PAL_ENTRIES_NO, (LPPALETTEENTRY)&s_pe );

			return (g_Screen.Pal.bUseSysCol = TRUE);
		}
	}
	return FALSE;

} /* #OF# Screen_UseSystemPalette */

/*========================================================
Function : Screen_UseAtariPalette
=========================================================*/
/* #FN#
   Forces using Atari800 palette in Windows environment */
BOOL
/* #AS#
   TRUE if succeeded, otherwise FALSE */
Screen_UseAtariPalette(
	BOOL bForceUse
)
{
	if( s_lpbmi )
	{
		if( bForceUse || g_Screen.Pal.bUseSysCol )
		{
			int	nRGB;
			int i;

			for( i = 0; i < PAL_ENTRIES_NO; i++ )
			{
				/* Get color value from kernel table */
				nRGB = Colours_table[ i ];

				s_lpbmi->bmiColors[ i ].rgbRed      = s_pe[ i ].peRed   = (nRGB & 0x00ff0000) >> 16;
				s_lpbmi->bmiColors[ i ].rgbGreen    = s_pe[ i ].peGreen = (nRGB & 0x0000ff00) >> 8;
				s_lpbmi->bmiColors[ i ].rgbBlue     = s_pe[ i ].peBlue  =  nRGB & 0x000000ff;
				s_lpbmi->bmiColors[ i ].rgbReserved = s_pe[ i ].peFlags =  0;
			}
			/* Make the changes visible for DirectDraw full-screen modes */
			if( _IsFlagSet( g_Screen.ulState, SM_MODE_FULL ) )
				DD_PaletteSetEntries( 0, 0, PAL_ENTRIES_NO, (LPPALETTEENTRY)&s_pe );

			return !(g_Screen.Pal.bUseSysCol = FALSE);
		}
	}
	return FALSE;

} /* #OF# Screen_UseAtariPalette */

/*========================================================
Function : Screen_FlipToGDI
=========================================================*/
/* #FN#
   Makes the surface that GDI writes to the primary surface */
BOOL
/* #AS#
   Nothing */
Screen_FlipToGDI(
	BOOL bUseSysPal
)
{
	if( s_DDraw.nFlipBuffers && DD_IsObjectCreated() )
	{
		/* Flip to GDI surface if multiple-buffered mode is active */
		HRESULT hResult = DD_FlipToGDISurface();
		if( FAILED(hResult) )
		{
			ServeDDError( IDS_DDERR_FLIP_TO_GDI, hResult, FALSE );
			return FALSE;
		}
	}
	if( bUseSysPal )
		/* Set the standard sytem colors */
		Screen_UseSystemPalette();

	return TRUE;

} /* #OF# Screen_FlipToGDI */

/*========================================================
Function : Screen_ShowMenuBar
=========================================================*/
/* #FN#
   Redraws a non-client area (menu bar, status bar and borders) */
void
/* #AS#
   Nothing */
Screen_ShowMenuBar(
	BOOL bForceShow /* #IN# Forces displaying the menu */
)
{
	if( g_hMainWnd && _IsFlagSet( g_Screen.ulState, SM_MODE_FULL ) &&
		(bForceShow || !ST_MENUBAR_HIDDEN) )
	{


//		DrawMenuBar( g_hMainWnd );
		/* We need menu with borders and status bar on the screen */
		RedrawWindow( g_hMainWnd,
					  NULL, NULL,
					  RDW_FRAME |
					  RDW_INVALIDATE |
					  RDW_UPDATENOW |
					  RDW_NOERASE );
	}
} /* #OF# Screen_ShowMenuBar */

/*========================================================
Function : Screen_PrepareRedraw
=========================================================*/
/* #FN#
   Prepares a GDI bitmap for repairing screen damages */
BOOL
/* #AS#
   TRUE if succeeded, otherwise FALSE */
Screen_PrepareRedraw(
	BOOL bForcePrep /* #IN# Forces preparing a bitmap */
)
{
	BOOL bResult = FALSE;

	if( _IsFlagSet( g_Screen.ulState, SM_MODE_FULL ) )
	{
		if( bForcePrep || NULL == s_Redraw.hDC )
		{
			HDC hDC = NULL;	/* GDI-compatible device context for the surface */
			RECT rc;

			/* Release the redrawing stuff if necessary */
			Screen_FreeRedraw();

			GetWindowRect( g_hViewWnd, &rc );
			rc.left += CX_BORDER; rc.top += CY_BORDER;

			s_Redraw.nWidth  = rc.right - rc.left - CX_BORDER;
			s_Redraw.nHeight = rc.bottom - rc.top - CY_BORDER;

			if( hDC = GetDC( g_hViewWnd ) )
			{
				if( (s_Redraw.hBmp = CreateCompatibleBitmap( hDC, s_Redraw.nWidth, s_Redraw.nHeight )) != NULL )
				{
					/* The bitmap is empty, so let's copy the contents of the surface to it.
					   For that we need to select it into a device context. We create one. */
					if( (s_Redraw.hDC = CreateCompatibleDC( hDC )) != NULL )
					{
						/* Select OffscrBmp into OffscrDC */
						HBITMAP hOldBmp = (HBITMAP)SelectObject( s_Redraw.hDC, s_Redraw.hBmp );
						/* Now we can copy the contents of the surface to the offscreen bitmap */
						BitBlt( s_Redraw.hDC, 0, 0, s_Redraw.nWidth, s_Redraw.nHeight, hDC, 0, 0 /*rc.left, rc.top*/, SRCCOPY );
						/* De-select OffscrBmp */
						SelectObject( s_Redraw.hDC, hOldBmp );

						bResult = TRUE;
					}
					else
						Screen_FreeRedraw();
				}
				/* We do not need hDC anymore. Free it */
				ReleaseDC( g_hViewWnd, hDC );
			}
		}
	}
	return bResult;

} /* #OF# Screen_PrepareRedraw */

/*========================================================
Function : Screen_FreeRedraw
=========================================================*/
/* #FN#
   Releases the redrawing stuff */
void
/* #AS#
   Nothing */
Screen_FreeRedraw( void )
{
	if( s_Redraw.hDC != NULL )
	{
		DeleteDC( s_Redraw.hDC );
		s_Redraw.hDC = NULL;
	}
	if( s_Redraw.hBmp != NULL )
	{
		DeleteObject( s_Redraw.hBmp );
		s_Redraw.hBmp = NULL;
	}
} /* #OF# Screen_FreeRedraw */

/*========================================================
Function : Screen_Redraw
=========================================================*/
/* #FN#
   Redraws an emulated Atari screen */
void
/* #AS#
   Nothing */
Screen_Redraw(
	HDC hDC /* #IN# Handle of the device context to be used for rendering an image */
)
{
	/* For windowed modes we may redraw the screen in the very simple way */
	if( _IsFlagSet( g_Screen.ulState, SM_MODE_WIND ) )
	{
		HDC hBackDC = g_Screen.hDC;
		g_Screen.hDC = hDC;

		PLATFORM_DisplayScreen();

		g_Screen.hDC = hBackDC;
	}
	/* For DirectDraw fullscreen modes we have to use a prepared bitmap */
	else if( s_Redraw.hBmp )
	{
		/* Select OffscrBmp into OffscrDC */
		HBITMAP hOldBmp = (HBITMAP)SelectObject( s_Redraw.hDC, s_Redraw.hBmp );
		/* Now we can copy the contents of the surface to the offscreen bitmap */
		BitBlt( hDC, 0, 0, s_Redraw.nWidth, s_Redraw.nHeight, s_Redraw.hDC, 0, 0, SRCCOPY );
		/* De-select OffscrBmp */
		SelectObject( s_Redraw.hDC, hOldBmp );
	}
} /* #OF# Screen_Redraw */

/*========================================================
Function : Screen_DrawPaused
=========================================================*/
/* #FN#
   Paints paused screen after changing a display mode */
void
/* #AS#
   Nothing */
Screen_DrawPaused(
	BOOL bForceMenu, /* #IN# Forces displaying the menu */
	BOOL bUseSysPal, /* #IN# Forces setting the system palette */
	BOOL bForcePrep,
	BOOL bRedraw
)
{
	if( _IsFlagSet( g_ulAtariState, ATARI_PAUSED ) && s_bModeChanged )
	{
		if( bRedraw ) {
			DrawPaused( s_Buffer.pMainScr );
			DrawPaused( s_Buffer.pBackScr );
		}

		Screen_FlipToGDI( bUseSysPal );
		Screen_PrepareRedraw( bForcePrep );

		if( bForceMenu || ST_MENUBAR_HIDDEN )
			Screen_ShowMenuBar( TRUE );
	}
} /* #OF# Screen_DrawPaused */

/*========================================================
Function : Screen_DrawFrozen
=========================================================*/
/* #FN#
   Paints lastly built screen */
void
/* #AS#
   Nothing */
Screen_DrawFrozen(
	BOOL bForceMenu, /* #IN# Forces displaying the menu */
	BOOL bUseSysPal, /* #IN# Forces setting the system palette */
	BOOL bForcePrep,
	BOOL bRedraw
)
{
	if( bRedraw && !_IsFlagSet( g_ulAtariState, ATARI_UNINITIALIZED ) )
	{
		int i;
		for( i = 0; i < s_DDraw.nFlipBuffers + 1 /* + primary surface */; i++ )
			/* We have to redraw the screen also on a GDI surface */
			PLATFORM_DisplayScreen();
	}
	Screen_FlipToGDI( bUseSysPal );
	Screen_PrepareRedraw( bForcePrep );

	if( bForceMenu || ST_MENUBAR_HIDDEN )
		Screen_ShowMenuBar( TRUE );

} /* #OF# Screen_DrawFrozen */

/*========================================================
Function : Screen_SetWindowSize
=========================================================*/
/* #FN#
   Sets main window size and position */
void
/* #AS#
   Nothing */
Screen_SetWindowSize(
	HWND hView,
	UINT nFlags
)
{
	if( g_hMainWnd )
	{
		int  nMenuHeight = GetSystemMetrics( SM_CYMENU );
		int  nHeight = Screen_HEIGHT;
		int  nWidth  = ATARI_VIS_WIDTH;
		RECT rc;

		if( _IsFlagSet( g_Screen.ulState, SM_WRES_DOUBLE ) )
		{
			nHeight = ATARI_DOUBLE_HEIGHT;
			nWidth  = ATARI_DOUBLE_VIS_WIDTH;
		}
		if( !hView )
			hView = g_hViewWnd;

		/* Set main window size and position */
		for( ;; )
		{
			SetWindowPos( g_hMainWnd, HWND_NOTOPMOST, g_nStartX, g_nStartY,
				nWidth +
					GetSystemMetrics( SM_CXDLGFRAME ) * 2 +
					GetSystemMetrics( SM_CXEDGE ) * 2,
				nHeight + g_Screen.nStatusSize +
					nMenuHeight +
					GetSystemMetrics( SM_CYCAPTION ) +
					GetSystemMetrics( SM_CYDLGFRAME ) * 2 +
					GetSystemMetrics( SM_CYEDGE ) * 2,
				nFlags );

			if( hView )
			{
				/* Add extra space to the window's height when a menu bar wraps
				   to two or more rows; GetMenuBarInfo and GetMenuInfo require
				   Windows 98 or "better"... */
				GetClientRect( hView, &rc );

				if( rc.bottom != nHeight )
					nMenuHeight += nHeight - rc.bottom;
				else
					break;
			}
			else
				break;
		}
	}
} /* #OF# Screen_SetWindowSize */

/*========================================================
Function : SetupWindowedDisplay
=========================================================*/
/* #FN#
   Sets up windowed mode display */
static
BOOL
/* #AS#
   TRUE if succeeded, otherwise FALSE */
SetupWindowedDisplay( void )
{
	_ASSERT(_IsFlagSet( g_Screen.ulState, SM_MODE_WIND ));

	/* If we are recovering from a DirectDraw mode, the window frame will be
	   whatever size that mode was instead of what we want it to be, so set
	   window size here */
	Screen_SetWindowSize( g_hViewWnd, SWP_SHOWWINDOW | SWP_NOSENDCHANGING );

	s_rcSource.left   = 0;
	s_rcSource.top    = 0;
	s_rcSource.right  = ATARI_VIS_WIDTH;
	s_rcSource.bottom = Screen_HEIGHT;

	Screen_visible_x1 = ATARI_HORZ_CLIP;
	Screen_visible_x2 = ATARI_HORZ_CLIP + ATARI_VIS_WIDTH;
	Screen_visible_y1 = 0;
	Screen_visible_y2 = Screen_HEIGHT;

	s_lpbmi->bmiHeader.biWidth  =  ATARI_VIS_WIDTH;
	s_lpbmi->bmiHeader.biHeight = -Screen_HEIGHT;	/* Negative because we are a top-down bitmap */
	hqlpbmi->bmiHeader.biWidth  =  ATARI_VIS_WIDTH;
	hqlpbmi->bmiHeader.biHeight = -Screen_HEIGHT;	/* Negative because we are a top-down bitmap */

	if( _IsFlagSet( g_Screen.ulState, SM_OPTN_USE_GDI ) )
	{
		/* Specifics for non-DirectDraw windowed modes */
		if( _IsFlagSet( g_Screen.ulState, SM_WRES_DOUBLE ) )
		{
			_SetFlag( g_Screen.ulState, SM_ATTR_STRETCHED );

			s_lpbmi->bmiHeader.biWidth  =  ATARI_DOUBLE_VIS_WIDTH;
			s_lpbmi->bmiHeader.biHeight = -ATARI_DOUBLE_HEIGHT;
			hqlpbmi->bmiHeader.biWidth  =  ATARI_DOUBLE_VIS_WIDTH;
			hqlpbmi->bmiHeader.biHeight = -ATARI_DOUBLE_HEIGHT;

			switch (g_Screen.nStretchMode) {
				case STRETCH_HIEND:
					Atari_DisplayScreen_ptr = Screen_GDI_Double_HiEnd;
					break;
				case STRETCH_SCANLINES:
					Atari_DisplayScreen_ptr = Screen_GDI_Double_Scanlines;
					break;
				case STRETCH_INTERPOLATION:
					Atari_DisplayScreen_ptr = Screen_GDI_Double_Interpolation;
					break;
				default:
					Atari_DisplayScreen_ptr = Screen_GDI_Double;
			}
		}
		else
			Atari_DisplayScreen_ptr = Screen_GDI;
	}
	else
	{
		/* Specifics for DirectDraw windowed modes */
		if( _IsFlagSet( g_Screen.ulState, SM_WRES_DOUBLE ) )
		{
			_SetFlag( g_Screen.ulState, SM_ATTR_STRETCHED );

			switch (g_Screen.nStretchMode) {
				case STRETCH_HIEND:
					hqlpbmi->bmiHeader.biHeight = -ATARI_DOUBLE_HEIGHT;
					s_rcSource.bottom = ATARI_DOUBLE_HEIGHT;
					s_Surface.nHeight *= 2;
					hqlpbmi->bmiHeader.biWidth = ATARI_DOUBLE_VIS_WIDTH;
					s_rcSource.right = ATARI_DOUBLE_VIS_WIDTH;
					s_Surface.nWidth = ATARI_DOUBLE_VIS_WIDTH;
					Atari_DisplayScreen_ptr = Screen_DDraw_Double_HiEnd;
					break;
				case STRETCH_SCANLINES:
					s_lpbmi->bmiHeader.biHeight = -ATARI_DOUBLE_HEIGHT;
					s_rcSource.bottom = ATARI_DOUBLE_HEIGHT;
					s_Surface.nHeight *= 2;
					Atari_DisplayScreen_ptr = Screen_DDraw_Double_Scanlines;
					break;
				case STRETCH_INTERPOLATION:
					s_lpbmi->bmiHeader.biHeight = -ATARI_DOUBLE_HEIGHT;
					s_rcSource.bottom = ATARI_DOUBLE_HEIGHT;
					s_Surface.nHeight *= 2;
					s_lpbmi->bmiHeader.biWidth = ATARI_DOUBLE_VIS_WIDTH;
					s_rcSource.right = ATARI_DOUBLE_VIS_WIDTH;
					s_Surface.nWidth = ATARI_DOUBLE_VIS_WIDTH;
					Atari_DisplayScreen_ptr = Screen_DDraw_Double_Interpolation;
					break;
				default:
					Atari_DisplayScreen_ptr = Screen_DDraw_Double;
			}
		}
		else
			Atari_DisplayScreen_ptr = Screen_DDraw;

		Screen_ComputeClipArea( g_hViewWnd );
	}
	return TRUE;

} /* #OF# SetupWindowedDisplay */

/*========================================================
Function : SetupFullScreenDisplay
=========================================================*/
/* #FN#
   Sets up DirectDraw full screen mode display */
static
BOOL
/* #AS#
   TRUE if succeeded, otherwise FALSE */
SetupFullScreenDisplay( void )
{
	HRESULT hResult;

	_ASSERT(_IsFlagSet( g_Screen.ulState, SM_MODE_FULL ));

	Atari_DisplayScreen_ptr = Screen_DDraw_Full_Blt;

	if( _IsFlagSet( g_Screen.ulState, SM_FRES_320_200 ) )
	{
		hResult = DD_SetDisplayMode( 320, 200, 8, max( g_Screen.nRequestRate, 0 ) );
		if( FAILED(hResult) )
		{
			ServeDDError( IDS_DDERR_INIT_RES, hResult, FALSE );
			return FALSE;
		}
		s_rcDestin.left   = 0;
		s_rcDestin.top    = 0;
		s_rcDestin.right  = 320;
		s_rcDestin.bottom = 200;
		s_rcSource.left   = ATARI_HORZ_CLIP + ((ATARI_VIS_WIDTH - 320) >> 1);
		s_rcSource.top    = (Screen_HEIGHT - 200) >> 1;
		s_rcSource.right  = Screen_WIDTH - s_rcSource.left;
		s_rcSource.bottom = Screen_HEIGHT - s_rcSource.top;

		Screen_visible_x1 = s_rcSource.left;
		Screen_visible_x2 = s_rcSource.right;
		Screen_visible_y1 = s_rcSource.top;
		Screen_visible_y2 = s_rcSource.bottom;

		_SetFlag( g_Screen.ulState, SM_ATTR_SMALL_DLG | SM_ATTR_NO_MENU );
	}
	else
	if( _IsFlagSet( g_Screen.ulState, SM_FRES_320_240 ) )
	{
		hResult = DD_SetDisplayMode( 320, 240, 8, max( g_Screen.nRequestRate, 0 ) );
		if( FAILED(hResult) )
		{
			ServeDDError( IDS_DDERR_INIT_RES, hResult, FALSE );
			return FALSE;
		}
		s_rcDestin.left   = 0;
		s_rcDestin.top    = 0;
		s_rcDestin.right  = 320;
		s_rcDestin.bottom = 240;
		s_rcSource.left   = ATARI_HORZ_CLIP + ((ATARI_VIS_WIDTH - 320) >> 1);
		s_rcSource.top    = 0;
		s_rcSource.right  = Screen_WIDTH - s_rcSource.left;
		s_rcSource.bottom = Screen_HEIGHT;

		Screen_visible_x1 = s_rcSource.left;
		Screen_visible_x2 = s_rcSource.right;
		Screen_visible_y1 = s_rcSource.top;
		Screen_visible_y2 = s_rcSource.bottom;

		_SetFlag( g_Screen.ulState, SM_ATTR_SMALL_DLG | SM_ATTR_NO_MENU );
	}
	else
	if( _IsFlagSet( g_Screen.ulState, SM_FRES_320_400 ) )
	{
		hResult = DD_SetDisplayMode( 320, 400, 8, max( g_Screen.nRequestRate, 0 ) );
		if( FAILED(hResult) )
		{
			ServeDDError( IDS_DDERR_INIT_RES, hResult, FALSE );
			return FALSE;
		}
		s_rcDestin.left   = 0;
		s_rcDestin.top    = 0;
		s_rcDestin.right  = 320;
		s_rcDestin.bottom = 400;
		s_rcSource.left   = ATARI_HORZ_CLIP + ((ATARI_VIS_WIDTH - 320) >> 1);
		s_rcSource.top    = (Screen_HEIGHT - 200) >> 1;
		s_rcSource.right  = Screen_WIDTH - s_rcSource.left;
		s_rcSource.bottom = Screen_HEIGHT - s_rcSource.top;

		Screen_visible_x1 = s_rcSource.left;
		Screen_visible_x2 = s_rcSource.right;
		Screen_visible_y1 = s_rcSource.top;
		Screen_visible_y2 = s_rcSource.bottom;

		_SetFlag( g_Screen.ulState, SM_ATTR_SMALL_DLG | SM_ATTR_STRETCHED | SM_ATTR_NO_MENU );

		if( STRETCH_PIXELDOUBLING != g_Screen.nStretchMode )
		{
			s_rcSource.top     = (ATARI_DOUBLE_HEIGHT - 400) >> 1;
			s_rcSource.bottom  = ATARI_DOUBLE_HEIGHT - s_rcSource.top;
			s_Surface.nHeight *= 2;

			if( STRETCH_INTERPOLATION == g_Screen.nStretchMode )
			{
				s_rcSource.left  = (ATARI_VIS_WIDTH - 320) >> 1;
				s_rcSource.right = ATARI_VIS_WIDTH - s_rcSource.left;
				s_Surface.nWidth = ATARI_VIS_WIDTH;

				Atari_DisplayScreen_ptr = Screen_DDraw_Full_Blt_Interpolation;
			}
			else
				Atari_DisplayScreen_ptr = Screen_DDraw_Full_Blt_Scanlines;
		}
	}
	else
	if( _IsFlagSet( g_Screen.ulState, SM_FRES_400_300 ) )
	{
		hResult = DD_SetDisplayMode( 400, 300, 8, max( g_Screen.nRequestRate, 0 ) );
		if( FAILED(hResult) )
		{
			ServeDDError( IDS_DDERR_INIT_RES, hResult, FALSE );
			return FALSE;
		}
		s_rcDestin.left   = (400 - ATARI_VIS_WIDTH) >> 1;
		s_rcDestin.top    = (300 - Screen_HEIGHT) >> 1;
		s_rcDestin.right  = s_rcDestin.left + ATARI_VIS_WIDTH;
		s_rcDestin.bottom = s_rcDestin.top + Screen_HEIGHT;
		s_rcSource.left   = ATARI_HORZ_CLIP;
		s_rcSource.top    = 0;
		s_rcSource.right  = Screen_WIDTH - ATARI_HORZ_CLIP;
		s_rcSource.bottom = Screen_HEIGHT;

		Screen_visible_x1 = s_rcSource.left;
		Screen_visible_x2 = s_rcSource.right;
		Screen_visible_y1 = s_rcSource.top;
		Screen_visible_y2 = s_rcSource.bottom;

		_SetFlag( g_Screen.ulState, SM_ATTR_SMALL_DLG );
	}
	else
	if( _IsFlagSet( g_Screen.ulState, SM_FRES_512_384 ) )
	{
		hResult = DD_SetDisplayMode( 512, 384, 8, max( g_Screen.nRequestRate, 0 ) );
		if( FAILED(hResult) )
		{
			ServeDDError( IDS_DDERR_INIT_RES, hResult, FALSE );
			return FALSE;
		}
		s_rcDestin.left   = (512 - ATARI_VIS_WIDTH) >> 1;
		s_rcDestin.top    = (384 - Screen_HEIGHT) >> 1;
		s_rcDestin.right  = s_rcDestin.left + ATARI_VIS_WIDTH;
		s_rcDestin.bottom = s_rcDestin.top + Screen_HEIGHT;
		s_rcSource.left   = ATARI_HORZ_CLIP;
		s_rcSource.top    = 0;
		s_rcSource.right  = Screen_WIDTH - ATARI_HORZ_CLIP;
		s_rcSource.bottom = Screen_HEIGHT;

		Screen_visible_x1 = s_rcSource.left;
		Screen_visible_x2 = s_rcSource.right;
		Screen_visible_y1 = s_rcSource.top;
		Screen_visible_y2 = s_rcSource.bottom;
	}
	else
	if( _IsFlagSet( g_Screen.ulState, SM_FRES_640_400 ) )
	{
		hResult = DD_SetDisplayMode( 640, 400, 8, max( g_Screen.nRequestRate, 0 ) );
		if( FAILED(hResult) )
		{
			ServeDDError( IDS_DDERR_INIT_RES, hResult, FALSE );
			return FALSE;
		}
		s_rcDestin.left   = 0;
		s_rcDestin.top    = 0;
		s_rcDestin.right  = 640;
		s_rcDestin.bottom = 400;
		s_rcSource.left   = ATARI_HORZ_CLIP + ((ATARI_VIS_WIDTH - 320) >> 1);
		s_rcSource.top    = (Screen_HEIGHT - 200) >> 1;
		s_rcSource.right  = Screen_WIDTH - s_rcSource.left;
		s_rcSource.bottom = Screen_HEIGHT - s_rcSource.top;

		Screen_visible_x1 = s_rcSource.left;
		Screen_visible_x2 = s_rcSource.right;
		Screen_visible_y1 = s_rcSource.top;
		Screen_visible_y2 = s_rcSource.bottom;

		_SetFlag( g_Screen.ulState, SM_ATTR_STRETCHED | SM_ATTR_NO_MENU );

		if( STRETCH_PIXELDOUBLING != g_Screen.nStretchMode )
		{
			s_rcSource.top     = (ATARI_DOUBLE_HEIGHT - 400) >> 1;
			s_rcSource.bottom  = ATARI_DOUBLE_HEIGHT - s_rcSource.top;
			s_Surface.nHeight *= 2;

			if( STRETCH_INTERPOLATION == g_Screen.nStretchMode )
			{
				s_rcSource.left  = (ATARI_DOUBLE_VIS_WIDTH - 640) >> 1;
				s_rcSource.right = ATARI_DOUBLE_VIS_WIDTH - s_rcSource.left;
				s_Surface.nWidth = ATARI_DOUBLE_VIS_WIDTH;

				Atari_DisplayScreen_ptr = Screen_DDraw_Full_Blt_Interpolation;
			}
			else
				Atari_DisplayScreen_ptr = Screen_DDraw_Full_Blt_Scanlines;
		}
	}
	else
	if( _IsFlagSet( g_Screen.ulState, SM_FRES_640_480 ) )
	{
		hResult = DD_SetDisplayMode( 640, 480, 8, max( g_Screen.nRequestRate, 0 ) );
		if( FAILED(hResult) )
		{
			ServeDDError( IDS_DDERR_INIT_RES, hResult, FALSE );
			return FALSE;
		}
		s_rcDestin.left   = 0;
		s_rcDestin.top    = 0;
		s_rcDestin.right  = 640;
		s_rcDestin.bottom = 480;
		s_rcSource.left   = ATARI_HORZ_CLIP + ((ATARI_VIS_WIDTH - 320) >> 1);
		s_rcSource.top    = 0;
		s_rcSource.right  = Screen_WIDTH - s_rcSource.left;
		s_rcSource.bottom = Screen_HEIGHT;

		Screen_visible_x1 = s_rcSource.left;
		Screen_visible_x2 = s_rcSource.right;
		Screen_visible_y1 = s_rcSource.top;
		Screen_visible_y2 = s_rcSource.bottom;

		_SetFlag( g_Screen.ulState, SM_ATTR_STRETCHED | SM_ATTR_NO_MENU );

		if( STRETCH_PIXELDOUBLING != g_Screen.nStretchMode )
		{
			s_rcSource.bottom  = ATARI_DOUBLE_HEIGHT;
			s_Surface.nHeight *= 2;

			if( STRETCH_INTERPOLATION == g_Screen.nStretchMode )
			{
				s_rcSource.left  = (ATARI_DOUBLE_VIS_WIDTH - 640) >> 1;
				s_rcSource.right = ATARI_DOUBLE_VIS_WIDTH - s_rcSource.left;
				s_Surface.nWidth = ATARI_DOUBLE_VIS_WIDTH;

				Atari_DisplayScreen_ptr = Screen_DDraw_Full_Blt_Interpolation;
			}
			else
				Atari_DisplayScreen_ptr = Screen_DDraw_Full_Blt_Scanlines;
		}
	}
	else
	if( _IsFlagSet( g_Screen.ulState, SM_FRES_800_600 ) )
	{
		hResult = DD_SetDisplayMode( 800, 600, 8, max( g_Screen.nRequestRate, 0 ) );
		if( FAILED(hResult) )
		{
			ServeDDError( IDS_DDERR_INIT_RES, hResult, FALSE );
			return FALSE;
		}
		s_rcDestin.left   = (800 - ATARI_DOUBLE_VIS_WIDTH) >> 1;
		s_rcDestin.top    = (600 - ATARI_DOUBLE_HEIGHT) >> 1;
		s_rcDestin.right  = s_rcDestin.left + ATARI_DOUBLE_VIS_WIDTH;
		s_rcDestin.bottom = s_rcDestin.top + ATARI_DOUBLE_HEIGHT;
		s_rcSource.left   = ATARI_HORZ_CLIP;
		s_rcSource.top    = 0;
		s_rcSource.right  = Screen_WIDTH - ATARI_HORZ_CLIP;
		s_rcSource.bottom = Screen_HEIGHT;

		Screen_visible_x1 = s_rcSource.left;
		Screen_visible_x2 = s_rcSource.right;
		Screen_visible_y1 = s_rcSource.top;
		Screen_visible_y2 = s_rcSource.bottom;

		_SetFlag( g_Screen.ulState, SM_ATTR_STRETCHED );

		if( STRETCH_PIXELDOUBLING != g_Screen.nStretchMode )
		{
			s_rcSource.bottom  = ATARI_DOUBLE_HEIGHT;
			s_Surface.nHeight *= 2;

			if( STRETCH_INTERPOLATION == g_Screen.nStretchMode )
			{
				s_rcSource.left  = 0;
				s_rcSource.right = ATARI_DOUBLE_VIS_WIDTH;
				s_Surface.nWidth = ATARI_DOUBLE_VIS_WIDTH;

				Atari_DisplayScreen_ptr = Screen_DDraw_Full_Blt_Interpolation;
			}
			else
				Atari_DisplayScreen_ptr = Screen_DDraw_Full_Blt_Scanlines;
		}
	}
	else
	if( _IsFlagSet( g_Screen.ulState, SM_FRES_1024_768 ) )
	{
		hResult = DD_SetDisplayMode( 1024, 768, 8, max( g_Screen.nRequestRate, 0 ) );
		if( FAILED(hResult) )
		{
			ServeDDError( IDS_DDERR_INIT_RES, hResult, FALSE );
			return FALSE;
		}
		s_rcDestin.left   = (1024 - ATARI_TRIPLE_VIS_WIDTH) >> 1;
		s_rcDestin.top    = (768 - ATARI_TRIPLE_HEIGHT) >> 1;
		s_rcDestin.right  = s_rcDestin.left + ATARI_TRIPLE_VIS_WIDTH;
		s_rcDestin.bottom = s_rcDestin.top + ATARI_TRIPLE_HEIGHT;
		s_rcSource.left   = ATARI_HORZ_CLIP;
		s_rcSource.top    = 0;
		s_rcSource.right  = Screen_WIDTH - ATARI_HORZ_CLIP;
		s_rcSource.bottom = Screen_HEIGHT;

		Screen_visible_x1 = s_rcSource.left;
		Screen_visible_x2 = s_rcSource.right;
		Screen_visible_y1 = s_rcSource.top;
		Screen_visible_y2 = s_rcSource.bottom;

		_SetFlag( g_Screen.ulState, SM_ATTR_STRETCHED );

		if( STRETCH_PIXELDOUBLING != g_Screen.nStretchMode )
		{
			s_rcSource.bottom  = ATARI_TRIPLE_HEIGHT;
			s_Surface.nHeight *= 3;

			if( STRETCH_INTERPOLATION == g_Screen.nStretchMode )
			{
				s_rcSource.left  = 0;
				s_rcSource.right = ATARI_TRIPLE_VIS_WIDTH;
				s_Surface.nWidth = ATARI_TRIPLE_VIS_WIDTH;

				Atari_DisplayScreen_ptr = Screen_DDraw_1024_Interpolation;
			}
			else
				Atari_DisplayScreen_ptr = Screen_DDraw_1024_Scanlines;
		}
	}
	return TRUE;

} /* #OF# SetupFullScreenDisplay */

/*========================================================
Function : Screen_ToggleWindowed
=========================================================*/
/* #FN#
   Toggles between a single and double windowed mode */
BOOL
/* #AS#
   TRUE if succeeded, otherwise FALSE */
Screen_ToggleWindowed( void )
{
	if( _IsFlagSet( g_Screen.ulState, SM_WRES_DOUBLE ) )
	{
		_ClrFlag( g_Screen.ulState, SM_WRES_MASK );
		_SetFlag( g_Screen.ulState, SM_WRES_NORMAL );
	}
	else
	{
		_ClrFlag( g_Screen.ulState, SM_WRES_MASK );
		_SetFlag( g_Screen.ulState, SM_WRES_DOUBLE );
	}
	WriteRegDWORD( NULL, REG_SCREEN_STATE, g_Screen.ulState );
	/* See CMainFrame::OnSize method */
//	g_nTestVal = _GetRefreshRate() - 1;

	return Screen_ChangeMode( FALSE );

} /* #OF# Screen_ToggleWindowed */

/*========================================================
Function : Screen_ToggleModes
=========================================================*/
/* #FN#
   Toggles between a windowed and full screen modes */
BOOL
/* #AS#
   TRUE if succeeded, otherwise FALSE */
Screen_ToggleModes( void )
{
	if( _IsFlagSet( g_Screen.ulState, SM_MODE_FULL ) )
	{
		Screen_ShowMousePointer( TRUE );

		_ClrFlag( g_Screen.ulState, SM_MODE_FULL );
		_SetFlag( g_Screen.ulState, SM_MODE_WIND );
	}
	else
	{
		_ASSERT(_IsFlagSet( g_Screen.ulState, SM_MODE_WIND ));

		_ClrFlag( g_Screen.ulState, SM_MODE_WIND );
		_SetFlag( g_Screen.ulState, SM_MODE_FULL );
	}
	WriteRegDWORD( NULL, REG_SCREEN_STATE, g_Screen.ulState );
	/* See CMainFrame::OnSize method */
//	g_nTestVal = _GetRefreshRate() - 1;

	return Screen_ChangeMode( FALSE );

} /* #OF# Screen_ToggleModes */

/*========================================================
Function : Screen_InitialiseDisplay
=========================================================*/
/* #FN#
   Initializes DirectDraw/GDI display */
BOOL
/* #AS#
   TRUE if succeeded, otherwise FALSE */
Screen_InitialiseDisplay(
	BOOL bForceInit /* #IN# Forces display initialization */
)
{
	HRESULT	hResult;

	if( !s_lpbmi )
	{
		int	nRGB, i;
		if( !(s_lpbmi = (LPBITMAPINFO)calloc( 1, sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * PAL_ENTRIES_NO )) )
			return FALSE;

		s_lpbmi->bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
		s_lpbmi->bmiHeader.biWidth       =  ATARI_VIS_WIDTH;
		s_lpbmi->bmiHeader.biHeight      = -Screen_HEIGHT;	/* Negative because we are a top-down bitmap */
		s_lpbmi->bmiHeader.biPlanes      = 1;
		s_lpbmi->bmiHeader.biBitCount    = 8;				/* Each byte stands for a color value */
		s_lpbmi->bmiHeader.biCompression = BI_RGB;			/* Uncompressed format */
		s_lpbmi->bmiHeader.biSizeImage   = ATARI_VIS_WIDTH * Screen_HEIGHT - (sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * PAL_ENTRIES_NO);
		s_lpbmi->bmiHeader.biClrUsed     = PAL_ENTRIES_NO;

		for( i = 0; i < PAL_ENTRIES_NO; i++ )
		{
			/* Get color value from kernel table */
			nRGB = Colours_table[ i ];

			s_lpbmi->bmiColors[ i ].rgbRed      = s_pe[ i ].peRed   = (nRGB & 0x00ff0000) >> 16;
			s_lpbmi->bmiColors[ i ].rgbGreen    = s_pe[ i ].peGreen = (nRGB & 0x0000ff00) >> 8;
			s_lpbmi->bmiColors[ i ].rgbBlue     = s_pe[ i ].peBlue  =  nRGB & 0x000000ff;
			s_lpbmi->bmiColors[ i ].rgbReserved = s_pe[ i ].peFlags =  0;
		}
		Screen_PrepareInterp( FALSE );
	}

	/* HiEnd modes */
	hqxInit();
	if (hqSource)
		free(hqSource);
	hqSource = malloc(ATARI_VIS_WIDTH * Screen_HEIGHT * 4);
	if (hqTarget)
		free(hqTarget);
	hqTarget = malloc(ATARI_VIS_WIDTH * Screen_HEIGHT * 4 * 4); // ONLY for hq2x
	if( !hqlpbmi )
	{
		if( !(hqlpbmi = (LPBITMAPINFO)malloc( sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) )) )
			return FALSE;

		hqlpbmi->bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
		hqlpbmi->bmiHeader.biWidth       = ATARI_VIS_WIDTH;
		hqlpbmi->bmiHeader.biHeight      = -Screen_HEIGHT;	/* Negative because we are a top-down bitmap */
		hqlpbmi->bmiHeader.biPlanes      = 1;
		hqlpbmi->bmiHeader.biBitCount    = 32;
		hqlpbmi->bmiHeader.biCompression = BI_RGB;			/* Uncompressed format */
	}

	/* Allocate screen buffers */
	if( !s_Buffer.pMainScr )
		s_Buffer.pMainScr = malloc( Screen_WIDTH * (Screen_HEIGHT + 16) );
	if( !s_Buffer.pWorkScr )
		s_Buffer.pWorkScr = malloc( ATARI_TRIPLE_VIS_SCREEN_SIZE + 1 );
	if( !s_Buffer.pBackScr )
		s_Buffer.pBackScr = malloc( Screen_WIDTH * (Screen_HEIGHT + 16) );

	if( !s_Buffer.pMainScr || !s_Buffer.pWorkScr )
		return FALSE;

	if( !bForceInit && DD_IsObjectCreated() &&
		s_ulScreenMode == g_Screen.ulState &&
		s_nStretchMode == g_Screen.nStretchMode &&
		s_nMemoryType  == g_Screen.nMemoryType &&
		(g_Screen.nRequestRate < 0 || s_nRequestRate == g_Screen.nRequestRate) )
		return TRUE;

	s_bModeChanged  = FALSE;
	s_bModeChanging = TRUE;

	_ClrFlag( g_Screen.ulState, SM_ATTR_MASK );

	/* Close Video Output file */
	StopVideoRecording( 0, TRUE );

	if( s_Buffer.pBackScr && s_Buffer.pSource && !_IsFlagSet( g_Screen.ulState, SM_OPTN_SAFE_MODE ) )
 		/* Save the generated atari screen */
		CopyMemory( s_Buffer.pBackScr, s_Buffer.pSource, Screen_WIDTH * (Screen_HEIGHT + 16) );

	/* There is the same schema of using buffers for all the windowed modes;
	   when in fullscreen the offscreen surface is used as source or target */
	s_Buffer.pSource = s_Buffer.pMainScr;
	s_Buffer.pTarget = s_Buffer.pWorkScr;

	/* Set DirectDraw control params */
	s_DDraw.pfxBltEffects = NULL;
	s_DDraw.nFlipBuffers  = 0;
	s_DDraw.dwBlitFlags   = DDBLT_WAIT;
	s_DDraw.dwFlipFlags   = DDFLIP_WAIT;

	if( !_IsFlagSet( g_Screen.ulState, SM_OPTN_DDVBL_WAIT ) )
	{
		s_DDraw.dwFlipFlags |= DDFLIP_NOVSYNC;
	}
	/* Generic size of the offscreen surface for the DirectDraw */
	s_Surface.nHeight = Screen_HEIGHT + 16;
	s_Surface.nWidth  = Screen_WIDTH;
	s_Surface.nPitch  = 0L;

	if( !s_lWndStyleBack )
		s_lWndStyleBack = GetWindowLong( g_hMainWnd, GWL_STYLE );

	/* Release also DirectDraw object if windowed non-DirectDraw mode is chosen */
	ReleaseObjects( _IsFlagSet( g_Screen.ulState, SM_MODE_WIND ) &&
		            _IsFlagSet( g_Screen.ulState, SM_OPTN_USE_GDI ) );

	g_Screen.nCurrentRate = 0;

	/* Handle windowed DirectDraw or non-DirectDraw modes */
	if( _IsFlagSet( g_Screen.ulState, SM_MODE_WIND ) )
	{
		/* Restore oryginal window style */
		if( g_hMainWnd && s_lWndStyleBack )
			SetWindowLong( g_hMainWnd, GWL_STYLE, s_lWndStyleBack );

		/* Restore display mode */
		DD_RestoreDisplayMode();

		/* Specifics for DirectDraw windowed modes */
		if( !_IsFlagSet( g_Screen.ulState, SM_OPTN_USE_GDI ) )
		{
			hResult = DD_ObjectCreate();
			if( FAILED(hResult) )
			{
				ServeDDError( IDS_DDERR_ALLOC_OBJ, hResult, FALSE );
				g_ulAtariState  = ATARI_UNINITIALIZED;
				s_bModeChanging = FALSE;
				return FALSE;
			}
			/* Set DirectDraw cooperative level */
			hResult = DD_SetCooperativeLevel( g_hMainWnd, DDSCL_NORMAL | DDSCL_NOWINDOWCHANGES );
			if( FAILED(hResult) )
			{
				ServeDDError( IDS_DDERR_SET_MODE, hResult, FALSE );
				s_bModeChanging = FALSE;
				return FALSE;
			}
		}
		if( !SetupWindowedDisplay() )
		{
			s_bModeChanging = FALSE;
			return FALSE;
		}
	}
	else /* Handle fullscreen (exclusive) DirectDraw modes */
	{
		_ASSERT(_IsFlagSet( g_Screen.ulState, SM_MODE_FULL ));

		hResult = DD_ObjectCreate();
		if( FAILED(hResult) )
		{
			ServeDDError( IDS_DDERR_ALLOC_OBJ, hResult, FALSE );
			g_ulAtariState = ATARI_UNINITIALIZED;
			s_bModeChanging = FALSE;
			return FALSE;
		}
		if( g_hMainWnd )
		{
			ShowWindow( g_hMainWnd, SW_HIDE );
			SetWindowLong( g_hMainWnd, GWL_STYLE, WS_BORDER );
			ShowWindow( g_hMainWnd, SW_SHOW );
		}
		/* Set DirectDraw cooperative level */
		hResult = DD_SetCooperativeLevel( g_hMainWnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN /*| DDSCL_ALLOWREBOOT*/ );
		if( FAILED(hResult) )
		{
			ServeDDError( IDS_DDERR_SET_MODE, hResult, FALSE );
			s_bModeChanging = FALSE;
			return FALSE;
		}
		if( !SetupFullScreenDisplay() )
		{
			s_bModeChanging = FALSE;
			return FALSE;
		}
		/* Flipped buffers are available for full screen without menu only */
		s_DDraw.nFlipBuffers = _IsFlagSet( g_Screen.ulState, SM_OPTN_FLIP_BUFFERS ) ? FLIPPED_BUFFERS_NO : 0;
	}
	/* Continue handling DirectDraw modes (windowed and fullscreen) */
	if( _IsFlagSet( g_Screen.ulState, SM_MODE_FULL ) || !_IsFlagSet( g_Screen.ulState, SM_OPTN_USE_GDI ) )
	{
		/* Primary surface for DirectDraw modes */
		if( !SetupPrimarySurface() )
		{
			s_bModeChanging = FALSE;
			return FALSE;
		}
		/* Memory surface is used for a full screen and both the windowed
		   modes now */
		if( !SetupMemorySurface() )
		{
			s_bModeChanging = FALSE;
			return FALSE;
		}
		/* Set direct pointers to the memory surface */
		if( !SetDirectPointers() )
		{
			s_bModeChanging = FALSE;
			return FALSE;
		}
		/* Create and set a palette to the created DirectDraw surfaces */
		if( !SetupPalette() )
		{
			s_bModeChanging = FALSE;
			return FALSE;
		}
		/* DirectDraw special effects have been enabled */
		if( _IsFlagSet( g_Screen.ulState, SM_DDFX_MASK ) )
		{
			ULONG ulIndex = (g_Screen.ulState >> 28) & 0x0f;
			int   i;

			ZeroMemory( &s_DDraw.fxBltEffects, sizeof(DDBLTFX) );
			s_DDraw.fxBltEffects.dwSize = sizeof(DDBLTFX);
			for( i = 0; i < BLIT_EFFECTS_NO; i++ )
				if( ulIndex & (1 << i) )
				{
					s_DDraw.fxBltEffects.dwDDFX = s_DDraw.dwDDFxFlags[ i ];
					break;
				}
			s_DDraw.pfxBltEffects = &s_DDraw.fxBltEffects;
			s_DDraw.dwBlitFlags |= DDBLT_DDFX;
		}
		/* Information about the current monitor refresh rate */
		if( FAILED(DD_GetMonitorFrequency( &g_Screen.nCurrentRate )) )
			g_Screen.nCurrentRate = 0;

		/* Print some information */
		Log_print( "%s", !s_DDraw.nFlipBuffers ? "Blitting to primary surface" : (1 == s_DDraw.nFlipBuffers ? "Double-buffering" : "Triple-buffering") );
		Log_print( "Monitor refresh rate: %ld", g_Screen.nCurrentRate );
	}
	/* Set kernel pointer to our screen buffer */
	Screen_atari = (ULONG *)s_Buffer.pSource;

	if( _IsFlagSet( g_Input.ulState, IS_CAPTURE_CTRLESC ) &&
		_IsFlagSet( g_Screen.ulState, SM_MODE_WIND ) && _IsFlagSet( s_ulScreenMode, SM_MODE_FULL ) )
	{
		/* Re-enable a Ctrl+Esc keystroke capture for a windowed mode we are switching to */
		Input_EnableEscCapture( TRUE );
	}
	if( ST_FLIPPED_BUFFERS && !_IsFlagSet( g_ulAtariState, ATARI_PAUSED ) )
		/* Needed for multiple-buffering modes to clean the menus */
		Screen_ShowMenuBar( TRUE );

	/* Clean up the screen */
	Screen_Clear( FALSE, FALSE );

	if( s_Buffer.pBackScr && s_Buffer.pSource && !_IsFlagSet( g_Screen.ulState, SM_OPTN_SAFE_MODE ) )
		/* Restore the generated atari screen */
		CopyMemory( s_Buffer.pSource, s_Buffer.pBackScr, Screen_WIDTH * (Screen_HEIGHT + 16) );

	s_ulScreenMode  = g_Screen.ulState;
	s_nStretchMode  = g_Screen.nStretchMode;
	s_nMemoryType   = g_Screen.nMemoryType;
	s_nRequestRate  = g_Screen.nRequestRate;
	s_bModeChanging = FALSE;
	s_bModeChanged  = TRUE;

	return TRUE;

} /* #OF# Screen_InitialiseDisplay */

/*========================================================
Function : Screen_ComputeClipArea
=========================================================*/
/* #FN#
   Computes destination area size and position */
void
/* #AS#
   Nothing */
Screen_ComputeClipArea(
	HWND hViewWnd
)
{
	if( hViewWnd && /* Don't need to worry about this for GDI screens */
		!_IsFlagSet( g_Screen.ulState, SM_OPTN_USE_GDI ) && !_IsFlagSet( g_Screen.ulState, SM_MODE_FULL ) )
	{
		GetWindowRect( hViewWnd, &s_rcDestin );

		s_rcDestin.left += GetSystemMetrics( SM_CXEDGE );
		s_rcDestin.top  += GetSystemMetrics( SM_CYEDGE );

		s_rcDestin.right  = s_rcDestin.left + (_IsFlagSet( g_Screen.ulState, SM_WRES_DOUBLE ) ? ATARI_DOUBLE_VIS_WIDTH : ATARI_VIS_WIDTH);
		s_rcDestin.bottom = s_rcDestin.top  + (_IsFlagSet( g_Screen.ulState, SM_WRES_DOUBLE ) ? ATARI_DOUBLE_HEIGHT : Screen_HEIGHT);
	}
} /* #OF# Screen_ComputeClipArea */

/*========================================================
Function : Screen_SetSafeDisplay
=========================================================*/
/* #FN#
   Sets safe display screen for compatibility */
BOOL
/* #AS#
   TRUE if succeeded, otherwise FALSE */
Screen_SetSafeDisplay(
	BOOL bForceGDI /* #IN# Use GDI flag */
)
{
	BOOL bResult = FALSE;

	Screen_ShowMousePointer( TRUE );

	_ClrFlag( g_Screen.ulState, SM_MODE_MASK );
	/* Set windowed mode */
	_SetFlag( g_Screen.ulState, SM_MODE_WIND );
	if( bForceGDI )
		_SetFlag( g_Screen.ulState, SM_OPTN_USE_GDI );

	bResult = Screen_InitialiseDisplay( TRUE );
	/* If there still are some problems */
	if( !bResult )
	{
		_ClrFlag( g_Screen.ulState, SM_WRES_MASK );
		_SetFlag( g_Screen.ulState, SM_WRES_NORMAL | SM_OPTN_USE_GDI );

		if( !Screen_InitialiseDisplay( TRUE ) )
			/* Set normal windowed size */
			Screen_SetWindowSize( g_hViewWnd, SWP_SHOWWINDOW | SWP_NOSENDCHANGING );
	}
	WriteRegDWORD( NULL, REG_SCREEN_STATE, g_Screen.ulState );
	/* Force screen refreshing */
	g_nTestVal = _GetRefreshRate() - 1;

	return bResult;

} /* #OF# Screen_SetSafeDisplay */

/*========================================================
Function : Screen_Clear
=========================================================*/
/* #FN#
   Clears screen buffers */
void
/* #AS#
   Nothing */
Screen_Clear(
	BOOL bPermanent, /* #IN# */
	BOOL bDeepClear
)
{
	if( bDeepClear )
	{
		if( s_Buffer.pMainScr && !_IsFlagSet( g_ulAtariState, ATARI_PAUSED ) )
			ZeroMemory( s_Buffer.pMainScr, Screen_WIDTH * (Screen_HEIGHT + 16) );
	}
	if( s_Buffer.pWorkScr )
		ZeroMemory( s_Buffer.pWorkScr, ATARI_TRIPLE_VIS_SCREEN_SIZE );

	/* Force screen refreshing */
	g_nTestVal = _GetRefreshRate() - 1;

	/* Restore the Atari palette if necessary */
	Screen_UseAtariPalette( FALSE );

	if( bDeepClear || (!ST_ATARI_FAILED &&
		/* If paused clear only when switching between modes */
		(!_IsFlagSet( g_ulAtariState, ATARI_PAUSED ) || s_bModeChanging)) )
	{
		DDBLTFX ddbltfx;

		ZeroMemory( &ddbltfx, sizeof(DDBLTFX) );
		ddbltfx.dwSize      = sizeof(DDBLTFX);
		ddbltfx.dwFillColor = 0;

		if( bDeepClear )
		{
			if( DD_IsSurfaceCreated( stOffscrn ) )
				DD_SurfaceBlit( stOffscrn, NULL, stNone, NULL, DDBLT_COLORFILL, &ddbltfx );
		}
		if( _IsFlagSet( g_Screen.ulState, SM_MODE_FULL ) )
		{
			if( DD_IsSurfaceCreated( stPrimary ) )
			{
				DD_WaitForVBlank( DDWAITVB_BLOCKBEGIN );

				if( s_DDraw.nFlipBuffers && DD_IsSurfaceCreated( stBackBuf ) )
				{
					/* Clear all the buffers if a flipping-surfaces mode is used */
					int i;
					for( i = 0; i < s_DDraw.nFlipBuffers + 1; i++ )
					{
						DD_SurfaceBlit( stBackBuf, NULL, stNone, NULL, DDBLT_COLORFILL, &ddbltfx );
						DD_SurfaceFlip( DDFLIP_WAIT | DDFLIP_NOVSYNC );
					}
				}
				else if( !_IsFlagSet( g_ulAtariState, ATARI_PAUSED ) || !s_bModeChanging )
				{
					/* Clear the primary buffer */
					DD_SurfaceBlit( stPrimary, NULL, stNone, NULL, DDBLT_COLORFILL, &ddbltfx );

					/* WARNING: There is a non-client area (a menu bar, status bar and borders)
					   to redraw after cleaning when in a full-screen mode with visible menus
					*/
					Screen_ShowMenuBar( FALSE );
				}
			}
		}
	}
	if( bPermanent )
	{
		/* Release DirectDraw Objects */
		ReleaseObjects( TRUE );

		/* Release allocated buffers */
		if( s_Buffer.pMainScr )
		{
			free( s_Buffer.pMainScr );
			s_Buffer.pMainScr = s_Buffer.pSource = NULL;
		}
		if( s_Buffer.pWorkScr )
		{
			free( s_Buffer.pWorkScr );
			s_Buffer.pWorkScr = s_Buffer.pTarget = NULL;
		}
		if( s_Buffer.pBackScr )
		{
			free( s_Buffer.pBackScr );
			s_Buffer.pBackScr = NULL;
		}
		if( s_lpbmi )
		{
			free( s_lpbmi );
			s_lpbmi = NULL;
		}
	}
	/* Clear redrawing stuff if necessary */
	if( !_IsFlagSet( g_ulAtariState, ATARI_PAUSED ) || bPermanent )
	{
		/* Release redraw bitmap after using */
		Screen_FreeRedraw();
	}
} /* #OF# Screen_Clear */

/*========================================================
Function : Screen_ChangeMode
=========================================================*/
/* #FN#
   Changes display mode */
BOOL
/* #AS#
   TRUE if succeeded, otherwise FALSE */
Screen_ChangeMode(
	BOOL bForceInit /* #IN# Forces display reinitialization */
)
{
	BOOL bResult = Screen_InitialiseDisplay( bForceInit );

	if( !bResult )
	{
		Screen_SetSafeDisplay( TRUE );
	}
	/* Refresh paused screen if necessary */
	Screen_DrawPaused( _IsFlagSet( g_Screen.ulState, SM_MODE_FULL ), FALSE, TRUE, TRUE );

	return bResult;

} /* #OF# Screen_ChangeMode */

/*========================================================
Function : Screen_CheckDDrawModes
=========================================================*/
/* #FN#
   Checks DirectDraw modes */
void
/* #AS#
   Nothing */
Screen_CheckDDrawModes( void )
{
	if( !DD_IsObjectCreated() )
	{
		if( SUCCEEDED(DD_ObjectCreate()) )
		{
			DD_EnumDisplayModes( &g_Screen.ulModesAvail );
			DD_GetVideoCaps( &g_Screen.ulVideoCaps );
		}
	}
	else
	{
		DD_EnumDisplayModes( &g_Screen.ulModesAvail );
		DD_GetVideoCaps( &g_Screen.ulVideoCaps );
	}
	/* Print some information */
	Log_print( "DirectDraw7: %s", DD_IsObjectCreated() == 2 ? "yes" : "no" );
	Log_print( "Can delay a flip: %s", _IsFlagSet( g_Screen.ulVideoCaps, DD_CAPS_FLIPINTERVAL ) ? "yes" : "no" );

} /* #OF# Screen_CheckDDrawModes */

/*========================================================
Function : Screen_ShowMousePointer
=========================================================*/
/* #FN#
   Makes the mouse pointer visible or hidden */
void
/* #AS#
   Nothing */
Screen_ShowMousePointer(
	BOOL bShow /* #IN# Show mouse pointer flag */
)
{
	if( ST_MOUSE_HIDDEN )
	{
		if( bShow )
		{
			if( g_Screen.nShowCursor <= 0 )
				ShowCursor( TRUE );

			if( _IsFlagSet( g_Input.ulState, IS_CAPTURE_MOUSE ) )
			{
				ClipCursor( NULL );
				g_Screen.nShowCursor = 10;
			}
			else
				g_Screen.nShowCursor = DEF_SHOW_CURSOR;
		}
		else
		{
			ShowCursor( FALSE ); /* Hide the mouse cursor */
			/* Re-capture the cursor */
			Input_RefreshBounds( g_hViewWnd, TRUE );
		}
	}
} /* #OF# Screen_ShowMousePointer */

/*========================================================
Function : Screen_GetBuffer
=========================================================*/
/* #FN#
   Returns pointer to the screen buffer */
UBYTE*
/* #AS#
   Pointer to the screen buffer */
Screen_GetBuffer( void )
{
	return s_Buffer.pSource; /* Screen buffer */

} /* #OF# Screen_GetBuffer */

/*========================================================
Function : Screen_FreeInterp
=========================================================*/
/* #FN#
   Invalidates a interpolation table with palette indexes */
void
/* #AS#
   Nothing */
Screen_FreeInterp( void )
{
	s_bIColorMap = FALSE;

} /* #OF# Screen_FreeInterp */

//#pragma optimize("at", on)

/*========================================================
Function : Screen_PrepareInterp
=========================================================*/
/* #FN#
   Prepares an interpolation table with palette indexes */
void
/* #AS#
   Nothing */
Screen_PrepareInterp(
	BOOL bForcePrep
)
{
	if( bForcePrep ||
		STRETCH_INTERPOLATION == g_Screen.nStretchMode && !s_bIColorMap )
	{
		/* Build a new interpolation map */
		UBYTE pixel;
		int r1, g1, b1, r2, g2, b2;
		int	nRGB;
		int i, j;

		HANDLE       hLogPal  = NULL;  /* Handle to a logical palette */
		LPLOGPALETTE lpPal    = NULL;  /* Pointer to a logical palette */
		HPALETTE     hPalette = NULL;

		hLogPal = GlobalAlloc( GHND/*LPTR*/, sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) * PAL_ENTRIES_NO );
		/* If not enough memory, clean up and return NULL */
		if( !hLogPal )
			return;

		lpPal = (LPLOGPALETTE)GlobalLock( (HGLOBAL)hLogPal );
		/* Init palette header info */
		lpPal->palVersion    = 0x300;
		lpPal->palNumEntries = (WORD)PAL_ENTRIES_NO;

		/* Copy entries */
		for( i = 0; i < PAL_ENTRIES_NO; i++ )
		{
			/* Get color value from kernel table */
			nRGB = Colours_table[ i ];

			lpPal->palPalEntry[ i ].peRed   = (nRGB & 0x00ff0000) >> 16;
			lpPal->palPalEntry[ i ].peGreen = (nRGB & 0x0000ff00) >> 8;
			lpPal->palPalEntry[ i ].peBlue  =  nRGB & 0x000000ff;
			lpPal->palPalEntry[ i ].peFlags =  0;
		}
		/* Create palette */
		hPalette = CreatePalette( lpPal );

		GlobalUnlock( (HGLOBAL) hLogPal );
		GlobalFree( (HGLOBAL) hLogPal );

		if( hPalette )
		{
			for( i = 0; i < PAL_ENTRIES_NO; i++ )
			{
				r1 = s_pe[ i ].peRed;
				g1 = s_pe[ i ].peGreen;
				b1 = s_pe[ i ].peBlue;

				for( j = i; j < PAL_ENTRIES_NO; j++ )
				{
					if( i == j )
					{
						s_anIColorMap[ i ][ j ] = i;
					}
					else
					{
						r2 = s_pe[ j ].peRed;
						g2 = s_pe[ j ].peGreen;
						b2 = s_pe[ j ].peBlue;

						pixel = GetNearestPaletteIndex( hPalette, RGB((r1 + r2) / 2, (g1 + g2) / 2, (b1 + b2) / 2) );

						s_anIColorMap[ i ][ j ] = pixel;
						s_anIColorMap[ j ][ i ] = pixel;
					}
				}
			}
			DeleteObject( hPalette );
			s_bIColorMap = TRUE;
		}
	}
} /* #OF# Screen_PrepareInterp */

#define	Inter1a( pDst, pSrc1, pSrc2 ) \
{\
	*pDst++ = *pSrc1;\
	/* Fill in the gap with the average of the pixels either side */\
	*pDst++ = s_anIColorMap[ *pSrc1++ ][ *pSrc2++ ];\
}
#define	Inter4a( pDst, pSrc1, pSrc2 ) \
{\
	Inter1a( pDst, pSrc1, pSrc2 );\
	Inter1a( pDst, pSrc1, pSrc2 );\
	Inter1a( pDst, pSrc1, pSrc2 );\
	Inter1a( pDst, pSrc1, pSrc2 );\
}
#define	Inter16a( pDst, pSrc1, pSrc2 ) \
{\
	Inter4a( pDst, pSrc1, pSrc2 );\
	Inter4a( pDst, pSrc1, pSrc2 );\
	Inter4a( pDst, pSrc1, pSrc2 );\
	Inter4a( pDst, pSrc1, pSrc2 );\
}
#define	Inter64a( pDst, pSrc1, pSrc2 ) \
{\
	Inter16a( pDst, pSrc1, pSrc2 );\
	Inter16a( pDst, pSrc1, pSrc2 );\
	Inter16a( pDst, pSrc1, pSrc2 );\
	Inter16a( pDst, pSrc1, pSrc2 );\
}
#define	Inter2Line( pDst, pSrc1, pSrc2 ) \
{\
	Inter64a( pDst, pSrc1, pSrc2 );\
	Inter64a( pDst, pSrc1, pSrc2 );\
	Inter64a( pDst, pSrc1, pSrc2 );\
	Inter64a( pDst, pSrc1, pSrc2 );\
	Inter64a( pDst, pSrc1, pSrc2 );\
	Inter4a( pDst, pSrc1, pSrc2 );\
	Inter4a( pDst, pSrc1, pSrc2 );\
	Inter4a( pDst, pSrc1, pSrc2 );\
	Inter1a( pDst, pSrc1, pSrc2 );\
	Inter1a( pDst, pSrc1, pSrc2 );\
	Inter1a( pDst, pSrc1, pSrc2 );\
	/* The last pixel is a simple copy of the one before it */\
	*pDst = *pSrc1;\
}

/*========================================================
Function : Interpolate2
=========================================================*/
/* #FN#
   Doubles the Atari screen using interpolation */
_inline
static
void
/* #AS#
   Nothing */
Interpolate2(
	int    xs, /* #IN# x pos of the source left-top corner */
	int    ws, /* #IN# Source width       */
	int    hs, /* #IN# Source height      */
	int    wd, /* #IN# Destination width  */
	int    hd, /* #IN# Destination height */
	UBYTE *pSource,
	UBYTE *pTarget,
	int    nPitch,
	BOOL   bVertFlip
)
{
	UBYTE *pSourceLine = (UBYTE*)(pSource + xs);
	UBYTE *pTargetLine = (UBYTE*)pTarget;

	/* Get the scan line widths of each DIB */

	const int nIncS = ws + xs + xs;

	int nIncD1 = nPitch ? nPitch : wd;
	int nIncD2 = nIncD1 + nIncD1;
	int l;

#ifndef PFUSIK_ASM
	UBYTE *pd, *ps1, *ps2;
	int i, j;
#endif

	if( bVertFlip )
	{
		/* Get pointers to the start points in the source and destination
		   DIBs. Note that this will be the bottom left corner of the DIB
		   as the scan lines are reversed in memory */
		pTargetLine = (UBYTE*)(pTarget + (hd - 1) * nIncD1);
		nIncD1 = -nIncD1;
		nIncD2 = -nIncD2;
	}

#ifdef PFUSIK_ASM
	if( wd > ws )
	{
		/* First line is interpolated horizontally:
		   Interpolate ws bytes starting from pSourceLine
		   to 2 * ws bytes starting from pTargetLine */
		_asm
		{
			mov		esi, dword ptr [pSourceLine]		;; [---eax---] [---edx---]
			mov		edi, dword ptr [pTargetLine]		;; al ah -- -- dl dh -- -- bl
			mov		ecx, dword ptr [ws]
			shr		ecx, 1
			dec		ecx
			xor		edx, edx							;; .. .. .. .. 00 00 00 00 ..
			mov		bl, byte ptr [esi]					;; .. .. .. .. 00 00 00 00 AA
			inc		esi
		int_horz:
		;; [esi] BB CC
		;; [edi] AA AB BB BC
			mov		dx, word ptr [esi]					;; .. .. .. .. BB CC 00 00 AA
			mov		al, dl								;; BB .. .. .. BB CC 00 00 AA
			mov		ah, byte ptr [edx + s_anIColorMap]	;; BB BC .. .. BB CC 00 00 AA
			add		esi, 2
			shl		eax, 16								;; 00 00 BB BC BB CC 00 00 AA
			mov		al, bl								;; AA 00 BB BC BB CC 00 00 AA
			mov		bl, dh								;; AA 00 BB BC BB CC 00 00 CC
			mov		dh, al								;; AA 00 BB BC BB AA 00 00 CC
			mov		ah, byte ptr [edx + s_anIColorMap]	;; AA AB BB BC BB AA 00 00 CC
			mov		dword ptr [edi], eax
			add		edi, 4
			dec		ecx
			jnz		int_horz
		;; [esi] BB
		;; [edi] AA AB BB BB
			mov		dl, byte ptr [esi]					;; .. .. .. .. BB .. 00 00 AA
			mov		dh, bl								;; .. .. .. .. BB AA 00 00 AA
			mov		al, dl								;; BB .. .. .. BB AA 00 00 AA
			mov		ah, dl								;; BB BB .. .. BB AA 00 00 AA
			shl		eax, 16								;; 00 00 BB BB BB AA 00 00 AA
			mov		ah, byte ptr [edx + s_anIColorMap]	;; 00 AB BB BB BB AA 00 00 AA
			mov		al, bl								;; AA AB BB BB BB AA 00 00 AA
			mov		dword ptr [edi], eax
		}
		/* Interpolate horizontally and vertically */
		if( _IsFlagSet( g_Screen.ulState, SM_OPTN_OPTIMIZE_PRO ) )
		{
			for( l = 1; l < hs; l++ )
			{
				pSourceLine += nIncS;
				_asm
				{
					mov		esi, dword ptr [pSourceLine]	;; The source pointer
					mov		edi, dword ptr [pTargetLine]	;; The dest pointer
					mov		eax, dword ptr [nIncD1]			;; Offset between stretched lines
					mov		ecx, dword ptr [ws]
					push	ebp
					mov		ebp, eax
					shr		ecx, 1
					dec		ecx
					push	ecx
				;; [esi]            AA BB CC
				;; [edi]            PP   PQ   QQ   QR
				;; [edi + ebp]     PPAA PQAB QQBB QRBC
				;; [edi + 2 * ebp]  AA   AB   BB   BC
				int_both_pro:
																		;;           [---eax---] [-------edx-------]
																		;; [bx] [cl] al ah [] [] [dl] [dh] [--] [--]
					movzx	ebx, byte ptr [esi + 1]						;;  bb
					movzx	ecx, byte ptr [esi + 2]						;;       CC
					shl		ebx, 8										;;  BB
					movzx	eax, byte ptr [ebx + ecx + s_anIColorMap]	;;           BC 00 00 00
					shl		eax, 8										;;           00 BC 00 00
					movzx	ecx, byte ptr [edi + 3]						;;       QR
					movzx	edx, byte ptr [eax + ecx + s_anIColorMap]	;;                       QRBC  00   00   00
					shl		edx, 8										;;                        00  QRBC  00   00
					movzx	ecx, byte ptr [edi + 2]						;;       QQ
					mov		dl, byte ptr [ebx + ecx + s_anIColorMap]	;;                       QQBB
					mov		al, bh										;;           BB
					movzx	ecx, byte ptr [esi]							;;       AA
					shl		eax, 8										;;           00 BB BC 00
					mov		al, byte ptr [ebx + ecx + s_anIColorMap]	;;           AB
					shl		eax, 8										;;           00 AB BB BC
					movzx	ebx, byte ptr [edi]							;;  pp
					add		eax, ecx									;;           AA
					shl		ebx, 8										;;  PP
					mov		dword ptr [edi + 2 * ebp], eax				;; Store AA AB BB BC
					movzx	ecx, byte ptr [ebx + ecx + s_anIColorMap]	;;      PPAA
					movzx	ebx, dl										;; qqbb
					shl		edx, 8										;;                        00  QQBB QRBC  00
					shl		ebx, 8										;; QQBB
					add		esi, 2										;; Increment source pointer
					mov		dl, byte ptr [ebx + ecx + s_anIColorMap]	;;                       PQAB
					shl		edx, 8										;;                        00  PQAB QQBB QRBC
					add		edx, ecx									;;                       PPAA
					mov		dword ptr [edi + ebp], edx					;; Store PPAA PQAB QQBB QRBC
					add		edi, 4										;; Increment dest pointer
					dec		[esp]										;; Decrement loop counter
					jnz		int_both_pro

					pop		eax											;; Pop decrement loop counter
				;; [esi]            AA BB
				;; [edi]            PP   PQ   QQ   QQ
				;; [edi + ebp]     PPAA PQAB QQBB QQBB
				;; [edi + 2 * ebp]  AA   AB   BB   BB
					movzx	ebx, byte ptr [esi + 1]						;;  bb
					movzx	ecx, byte ptr [edi + 2]						;;       QQ
					mov		eax, ebx									;;           BB 00 00 00
					shl		ebx, 8										;;  BB
					add		eax, ebx									;;              BB
					movzx	edx, byte ptr [ebx + ecx + s_anIColorMap]	;;                       QQBB  00   00   00
					mov		dh, dl										;;                            QQBB
					movzx	ecx, byte ptr [esi]							;;       AA
					shl		eax, 8										;;           00 BB BB 00
					mov		al, byte ptr [ebx + ecx + s_anIColorMap]	;;           AB
					movzx	ebx, byte ptr [edi]							;;  pp
					shl		eax, 8										;;           00 AB BB BB
					shl		ebx, 8										;;  PP
					add		eax, ecx									;;           AA
					mov		dword ptr [edi + 2 * ebp], eax				;; Store AA AB BB BB
					movzx	ecx, byte ptr [ebx + ecx + s_anIColorMap]	;;      PPAA
					mov		ebx, 0ff00h
					and		ebx, edx									;; QQBB
					shl		edx, 8										;;                        00  QQBB QQBB
					mov		dl, byte ptr [ebx + ecx + s_anIColorMap]	;;                       PQAB QQBB QQBB
					shl		edx, 8										;;                        00  PQAB QQBB QQBB
					add		edx, ecx									;;                       PPAA PQAB QQBB QQBB
					mov		dword ptr [edi + ebp], edx	;; Store PPAA PQAB QQBB QQBB

					pop		ebp
				}
				pTargetLine += nIncD2;
			}
		}
		else
		{
			for( l = 1; l < hs; l++ )
			{
				pSourceLine += nIncS;
				/* Interpolate ws bytes starting from pSourceLine
				   to 2 * ws bytes starting from pTargetLine + 2 * nIncD1
				   Then interpolate these 2 * ws bytes starting from pTargetLine + 2 * nIncD1
				   with 2 * ws bytes starting from pTargetLine
				   to 2 * ws bytes starting from pTargetLine + nIncD1 */
				_asm
				{
					mov		esi, dword ptr [pSourceLine]	;; The source pointer
					mov		edi, dword ptr [pTargetLine]	;; The dest pointer
				;; Here's a little trick:
				;; s_anIColorMap[y][x] is accessed by [ebx + ecx]
				;; cl = x, bh = y
				;; both ecx and ebx contain parts of s_anIColorMap offset,
				;; ecx contains loop count in high word
				;; i.e.
				;; ecx = (s_anIColorMap & 0x0000ff00) + (loopCount << 16) + x;
				;; ebx = (s_anIColorMap & 0xffff00ff) - (loopCount << 16) + (y << 8);
					mov		ecx, dword ptr [ws]			;; Loop count = number of words - 2
					shr		ecx, 1						;; (loop count is tested for < 0, not == 0
					dec		ecx							;; and last word is processed after the loop)
					dec		ecx
					shl		ecx, 16						;; Loop count in high word of ecx
					mov		ebx, offset s_anIColorMap	;; Interpolation lookup table
					mov		ch, bh
					sub		ebx, ecx
					mov		eax, dword ptr [nIncD1]		;; Offset between stretched lines
					push	ebp
					mov		ebp, eax
				;; [esi]            AA BB CC
				;; [edi]            PP   PQ   QQ   QR
				;; [edi + ebp]     PPAA PQAB QQBB QRBC
				;; [edi + 2 * ebp]  AA   AB   BB   BC
				int_both:
														;;           [---eax---] [-------edx-------]
														;; [bh] [cl] al ah [] [] [dl] [dh] [--] [--]
					mov		eax, ebx					;; Copy ebx to eax, so [eax + ecx] can be used
					mov		bh, byte ptr [esi + 1]		;;  BB
					mov		cl, byte ptr [esi + 2]		;;       CC
					mov		ah, byte ptr [ebx + ecx]	;;              BC
					mov		cl, byte ptr [edi + 3]		;;       QR
					mov		dh, byte ptr [eax + ecx]	;;                            QRBC
					mov		cl, byte ptr [edi + 2]		;;       QQ
					mov		dl, byte ptr [ebx + ecx]	;;                       QQBB
					mov		al, bh						;;           BB
					mov		cl, byte ptr [esi]			;;       AA
					shl		eax, 16						;;           00 00 BB BC
					mov		ah, byte ptr [ebx + ecx]	;;              AB
					mov		bh, byte ptr [edi]			;;  PP
					mov		al, cl						;;           AA
					mov		dword ptr [edi + 2 * ebp], eax	;; Store AA AB BB BC
					mov		cl, byte ptr [ebx + ecx]	;;      PPAA
					mov		bh, dl						;; QQBB
					shl		edx, 16						;;                        00   00  QQBB QRBC
					add		esi, 2						;; Increment source pointer
					mov		dh, byte ptr [ebx + ecx]	;;                            PQAB
					add		ebx, 10000h
					mov		dl, cl						;;                       PPAA
					mov		dword ptr [edi + ebp], edx	;; Store PPAA PQAB QQBB QRBC
					add		edi, 4						;; Increment dest pointer
					sub		ecx, 10000h
					jae		int_both

				;; [esi]            AA BB
				;; [edi]            PP   PQ   QQ   QQ
				;; [edi + ebp]     PPAA PQAB QQBB QQBB
				;; [edi + 2 * ebp]  AA   AB   BB   BB
					mov		bh, byte ptr [esi + 1]		;;  BB
					mov		cl, byte ptr [edi + 2]		;;       QQ
					mov		ah, bh						;;              BB
					mov		dh, byte ptr [ebx + ecx]	;;                            QQBB
					mov		al, bh						;;           BB
					mov		dl, dh						;;                       QQBB
					mov		cl, byte ptr [esi]			;;       AA
					shl		eax, 16						;;           00 00 BB BB
					mov		ah, byte ptr [ebx + ecx]	;;              AB
					mov		bh, byte ptr [edi]			;;  PP
					mov		al, cl						;;           AA
					mov		dword ptr [edi + 2 * ebp], eax	;; Store AA AB BB BB
					mov		cl, byte ptr [ebx + ecx]	;;      PPAA
					mov		bh, dl						;; QQBB
					shl		edx, 16						;;                        00   00  QQBB QQBB
					mov		dh, byte ptr [ebx + ecx]	;;                            PQAB
					mov		dl, cl						;;                       PPAA
					mov		dword ptr [edi + ebp], edx	;; Store PPAA PQAB QQBB QQBB
					pop		ebp
				}
				pTargetLine += nIncD2;
			}
		}
	}
	else
	{
		/* Interpolate vertically only */
		memcpy( pTargetLine, pSourceLine, wd);

		if( _IsFlagSet( g_Screen.ulState, SM_OPTN_OPTIMIZE_PRO ) )
		{
			/* Optimized for Pentium Pro, Pentium II etc. */
			for( l = 1; l < hs; l++ )
			{
				pSourceLine += nIncS;
				memcpy( pTargetLine + nIncD2, pSourceLine, wd);
				_asm
				{
					mov		edi, dword ptr [pTargetLine]
					mov		ebx, dword ptr [nIncD1]
					mov		ecx, dword ptr [wd]
					shr		ecx, 1
				int_vert_pro:
					movzx	esi, byte ptr [edi]
					movzx	edx, byte ptr [edi + 2 * ebx]
					shl		esi, 8
					mov		al, byte ptr [esi + edx + s_anIColorMap]
					mov		byte ptr [edi + ebx], al
					inc		edi
					movzx	esi, byte ptr [edi]
					movzx	edx, byte ptr [edi + 2 * ebx]
					shl		esi, 8
					mov		al, byte ptr [esi + edx + s_anIColorMap]
					mov		byte ptr [edi + ebx], al
					inc		edi
					dec		ecx
					jnz		int_vert_pro
				}
				pTargetLine += nIncD2;
			}
		}
		else
		{
			/* Optimized for Pentium MMX, Pentium, 486, etc. */
			for( l = 1; l < hs; l++ )
			{
				pSourceLine += nIncS;
				/* Move wd bytes starting from pSourceLine
				   to wd bytes starting from pTargetLine + 2 * nIncD1
				   Then interpolate these wd bytes starting from pTargetLine + 2 * nIncD1
				   with wd bytes starting from pTargetLine
				   to wd bytes starting from pTargetLine + nIncD1 */
				_asm
				{
					mov		esi, dword ptr [pSourceLine]		;; [---eax---] [---edx---]
					mov		edi, dword ptr [pTargetLine]		;; al ah -- -- dl dh -- --
					mov		ebx, dword ptr [nIncD1]
					mov		ecx, dword ptr [wd]
					shr		ecx, 1
					xor		eax, eax							;; 00 00 00 00 .. .. .. ..
					xor		edx, edx							;; 00 00 00 00 00 00 00 00
				int_vert:
				;; [esi]           RR SS
				;; [edi]           PP QQ
				;; [edi + ebx]     PR QS
				;; [edi + 2 * ebx] RR SS
					mov		ax, word ptr [esi]					;; RR SS 00 00 .. .. 00 00
					mov		dx, word ptr [edi]					;; RR SS 00 00 PP QQ 00 00
					mov		word ptr [edi + 2 * ebx], ax
					xchg	ah, dl								;; RR PP 00 00 SS QQ 00 00
					add		esi, 2
					mov		al, byte ptr [eax + s_anIColorMap]	;; PR PP 00 00 SS QQ 00 00
					mov		ah, byte ptr [edx + s_anIColorMap]	;; PR QS 00 00 SS QQ 00 00
					mov		word ptr [edi + ebx], ax
					add		edi, 2
					dec		ecx
					jnz		int_vert
				}
				pTargetLine += nIncD2;
			}
		}
	}
	memcpy( pTargetLine + nIncD1, pTargetLine, wd );
#else /* PFUSIK_ASM */
	if( wd > ws )
	{
		/* Process each source line */
		for( l = 0; l < hs; l++ )
		{
			/* Copy the source pixels to every other destination pixel */
			ps1 = pSourceLine;
			ps2 = pSourceLine + 1;
			pd  = pTargetLine;

			Inter2Line( pd, ps1, ps2 );

			pTargetLine += nIncD2;
			pSourceLine += nIncS;
		}
	}
	else
	{
		/* Process each source line */
		for( l = 0; l < hs; l++ )
		{
			/* Copy the source lines */
			memcpy( pTargetLine, pSourceLine, wd );

			pTargetLine += nIncD2;
			pSourceLine += nIncS;
		}
	}
	/* Process the missing lines in the destination */
	pSourceLine = (UBYTE*)(bVertFlip ? (pTarget + (hd - 1) * -nIncD1) : pTarget);

	for( l = 0, j = wd >> 4; l < hs - 1; l++ )
	{
		ps1 = pSourceLine;
		ps2 = pSourceLine + nIncD2;
		pd  = pSourceLine + nIncD1;

		for( i = 0; i < j; i++ )
		{
			*pd++ = s_anIColorMap[ *ps1++ ][ *ps2++ ];
			*pd++ = s_anIColorMap[ *ps1++ ][ *ps2++ ];
			*pd++ = s_anIColorMap[ *ps1++ ][ *ps2++ ];
			*pd++ = s_anIColorMap[ *ps1++ ][ *ps2++ ];
			*pd++ = s_anIColorMap[ *ps1++ ][ *ps2++ ];
			*pd++ = s_anIColorMap[ *ps1++ ][ *ps2++ ];
			*pd++ = s_anIColorMap[ *ps1++ ][ *ps2++ ];
			*pd++ = s_anIColorMap[ *ps1++ ][ *ps2++ ];
			*pd++ = s_anIColorMap[ *ps1++ ][ *ps2++ ];
			*pd++ = s_anIColorMap[ *ps1++ ][ *ps2++ ];
			*pd++ = s_anIColorMap[ *ps1++ ][ *ps2++ ];
			*pd++ = s_anIColorMap[ *ps1++ ][ *ps2++ ];
			*pd++ = s_anIColorMap[ *ps1++ ][ *ps2++ ];
			*pd++ = s_anIColorMap[ *ps1++ ][ *ps2++ ];
			*pd++ = s_anIColorMap[ *ps1++ ][ *ps2++ ];
			*pd++ = s_anIColorMap[ *ps1++ ][ *ps2++ ];
		}
		pSourceLine += nIncD2;
	}
	/* The last line is a simple copy of the one before it */
	memcpy( pSourceLine + (bVertFlip ? -nIncD1 : nIncD1), pSourceLine, wd );
#endif /* PFUSIK_ASM */

} /* #OF# Interpolate2 */

#define	Inter1b( pDst, pSrc1, pSrc2 ) \
{\
	*pDst++ = *pSrc1;\
	/* Fill in the gap with the average of the pixels either side */\
	*pDst++ = s_anIColorMap[ *pSrc1++ ][ *pSrc2++ ];\
	*pDst++ = *(pDst - 1);\
}
#define	Inter4b( pDst, pSrc1, pSrc2 ) \
{\
	Inter1b( pDst, pSrc1, pSrc2 );\
	Inter1b( pDst, pSrc1, pSrc2 );\
	Inter1b( pDst, pSrc1, pSrc2 );\
	Inter1b( pDst, pSrc1, pSrc2 );\
}
#define	Inter16b( pDst, pSrc1, pSrc2 ) \
{\
	Inter4b( pDst, pSrc1, pSrc2 );\
	Inter4b( pDst, pSrc1, pSrc2 );\
	Inter4b( pDst, pSrc1, pSrc2 );\
	Inter4b( pDst, pSrc1, pSrc2 );\
}
#define	Inter64b( pDst, pSrc1, pSrc2 ) \
{\
	Inter16b( pDst, pSrc1, pSrc2 );\
	Inter16b( pDst, pSrc1, pSrc2 );\
	Inter16b( pDst, pSrc1, pSrc2 );\
	Inter16b( pDst, pSrc1, pSrc2 );\
}
#define	Inter3Line( pDst, pSrc1, pSrc2 ) \
{\
	Inter64b( pDst, pSrc1, pSrc2 );\
	Inter64b( pDst, pSrc1, pSrc2 );\
	Inter64b( pDst, pSrc1, pSrc2 );\
	Inter64b( pDst, pSrc1, pSrc2 );\
	Inter64b( pDst, pSrc1, pSrc2 );\
	Inter4b( pDst, pSrc1, pSrc2 );\
	Inter4b( pDst, pSrc1, pSrc2 );\
	Inter4b( pDst, pSrc1, pSrc2 );\
	Inter1b( pDst, pSrc1, pSrc2 );\
	Inter1b( pDst, pSrc1, pSrc2 );\
	Inter1b( pDst, pSrc1, pSrc2 );\
	/* The last pixel is a simple copy of the one before it */\
	*pDst++ = *pSrc1;\
	*pDst   = *pSrc1;\
}

/*========================================================
Function : Interpolate3
=========================================================*/
/* #FN#
   Triples the Atari screen using interpolation */
_inline
static
void
/* #AS#
   Nothing */
Interpolate3(
	int    xs, /* #IN# x pos of the source left-top corner */
	int    ws, /* #IN# Source width       */
	int    hs, /* #IN# Source height      */
	int    wd, /* #IN# Destination width  */
	int    hd, /* #IN# Destination height */
	UBYTE *pSource,
	UBYTE *pTarget,
	int    nPitch
)
{
	UBYTE *pSourceLine = (UBYTE*)(pSource + xs);
	UBYTE *pTargetLine = (UBYTE*)pTarget;

	UBYTE *pd, *ps1, *ps2;

	/* Get the scan line widths of each DIB */
	const int nIncS  = ws + xs + xs;
	const int nIncD1 = nPitch ? nPitch : wd;
	const int nIncD2 = nIncD1 + nIncD1;
	const int nIncD3 = nIncD2 + nIncD1;

	int i, j, l;

	/* Process each source line */
	for( l = 0; l < hs; l++ )
	{
		/* Copy the source pixels to every other destination pixel */
		ps1 = pSourceLine;
		ps2 = pSourceLine + 1;
		pd  = pTargetLine;

		Inter3Line( pd, ps1, ps2 );

		pTargetLine += nIncD3;
		pSourceLine += nIncS;
	}
	/* Process the missing lines in the destination */
	pSourceLine = (UBYTE*)pTarget;

	for( l = 0, j = wd >> 4; l < hs - 1; l++ )
	{
		ps1 = pSourceLine;
		ps2 = pSourceLine + nIncD3;
		pd  = pSourceLine + nIncD2;

		memcpy( pSourceLine + nIncD1, ps1, wd );

		for( i = 0; i < j; i++ )
		{
			*pd++ = s_anIColorMap[ *ps1++ ][ *ps2++ ];
			*pd++ = s_anIColorMap[ *ps1++ ][ *ps2++ ];
			*pd++ = s_anIColorMap[ *ps1++ ][ *ps2++ ];
			*pd++ = s_anIColorMap[ *ps1++ ][ *ps2++ ];
			*pd++ = s_anIColorMap[ *ps1++ ][ *ps2++ ];
			*pd++ = s_anIColorMap[ *ps1++ ][ *ps2++ ];
			*pd++ = s_anIColorMap[ *ps1++ ][ *ps2++ ];
			*pd++ = s_anIColorMap[ *ps1++ ][ *ps2++ ];
			*pd++ = s_anIColorMap[ *ps1++ ][ *ps2++ ];
			*pd++ = s_anIColorMap[ *ps1++ ][ *ps2++ ];
			*pd++ = s_anIColorMap[ *ps1++ ][ *ps2++ ];
			*pd++ = s_anIColorMap[ *ps1++ ][ *ps2++ ];
			*pd++ = s_anIColorMap[ *ps1++ ][ *ps2++ ];
			*pd++ = s_anIColorMap[ *ps1++ ][ *ps2++ ];
			*pd++ = s_anIColorMap[ *ps1++ ][ *ps2++ ];
			*pd++ = s_anIColorMap[ *ps1++ ][ *ps2++ ];
		}
		pSourceLine += nIncD3;
	}
	/* The two last lines are a simple copy of the one before them */
	memcpy( pSourceLine + nIncD1, pSourceLine, wd );
	memcpy( pSourceLine + nIncD2, pSourceLine, wd );

} /* #OF# Interpolate3 */

//#pragma optimize("", on)

/*========================================================
Function : LockMemorySurface
=========================================================*/
/* #FN#
   Locks the DirectDraw memory surface for direct accessing to be safe */
_inline
static
BOOL
/* #AS#
   TRUE if succeeded, otherwise FALSE */
LockMemorySurface(
	BOOL bLock
)
{
	if( bLock )
	{
		HRESULT hResult;

		hResult = DD_SurfaceLock( stOffscrn, DDLOCK_WAIT, &s_Buffer.pTarget, &s_Surface.nPitch );
		if( DDERR_SURFACELOST == hResult && RestoreSurfaces() )
			/* Try to lock the surface once again */
			hResult = DD_SurfaceLock( stOffscrn, DDLOCK_WAIT, &s_Buffer.pTarget, &s_Surface.nPitch );

		if( FAILED(hResult) )
			return FALSE;
	}
	else
		DD_SurfaceUnlock( stOffscrn );

	return TRUE;

} /* #OF# LockMemorySurface */

/*========================================================
Function : PLATFORM_DisplayScreen
=========================================================*/
void
PLATFORM_DisplayScreen( void ) {
	Atari_DisplayScreen_ptr();
}

/*========================================================
Function : Screen_DDraw
=========================================================*/
/* #FN#
   Draws Atari screen in DirectDraw windowed mode x 1 */
static
void
/* #AS#
   Nothing */
Screen_DDraw( void )
{
	HRESULT hResult;
	HDC hdc;

	/* We have to copy the full size atari screen to another buffer to chop
	   off the left and right overflow (reason: allowing StretchDIB to cut the
	   rectangle for us is VERY slow). It so happens everything involved is
	   DWORD aligned, and the offsets and such are constants, so I'm sticking
	   this asm routine in here... */
	_asm
	{
		mov		esi, dword ptr [s_Buffer.pSource]	;; The source pointer
		add		esi, ATARI_HORZ_CLIP
		mov		edi, dword ptr [s_Buffer.pTarget]	;; The dest pointer
		mov		eax, Screen_HEIGHT					;; Number of lines
	scan_line:
		mov		ecx, 054h		;; Our count of DWORDs to copy (ATARI_VIS_WIDTH/4)
		rep		movsd			;; Move that string
		add		esi, 030h		;; Add in the offset to the next scan line for src

		dec		eax
		jnz		scan_line
	}

	if( SUCCEEDED(hResult = DD_SurfaceGetDC( &hdc )) )
	{
		/* This StretchDIB basically does only the color space conversion to the memory surface */
		StretchDIBits( hdc,
					   0, 0, ATARI_VIS_WIDTH, Screen_HEIGHT,
					   0, 0, ATARI_VIS_WIDTH, Screen_HEIGHT,
					   s_Buffer.pTarget, s_lpbmi, DIB_RGB_COLORS, SRCCOPY );

		DD_SurfaceReleaseDC( hdc );
	}
	else
	{
		ServeDDError( IDS_DDERR_SURFACE_LOCK, hResult, FALSE );
		return;
	}

	/* Blit to the primary surface from a memory one */
	if( FAILED(hResult = BlitSurface()) )
	{
		if( hResult == DDERR_WASSTILLDRAWING )
			return;

		if( hResult == DDERR_SURFACELOST )
			RestoreSurfaces();
		else
			ServeDDError( IDS_DDERR_SURFACE_BLT, hResult, FALSE );
	}
} /* #OF# Screen_DDraw */

/*========================================================
Function : Screen_DDraw_Double
=========================================================*/
/* #FN#
   Draws Atari screen in DirectDraw windowed mode x 2 */
static
void
/* #AS#
   Nothing */
Screen_DDraw_Double( void )
{
	HRESULT	hResult;
	HDC hdc;

	_asm
	{
		mov		esi, dword ptr [s_Buffer.pSource]	;; The source pointer
		add		esi, ATARI_HORZ_CLIP
		mov		edi, dword ptr [s_Buffer.pTarget]	;; The dest pointer
		mov		eax, Screen_HEIGHT					;; Number of lines
	scan_line:
		mov		ecx, 054h		;; Our count of DWORDs to copy (ATARI_VIS_WIDTH/4)
		rep		movsd			;; Move that string
		add		esi, 030h		;; Add in the offset to the next scan line for src

		dec		eax
		jnz		scan_line
	}

	if( SUCCEEDED(hResult = DD_SurfaceGetDC( &hdc )) )
	{
		/* This StretchDIB basically does only the color space conversion to the memory surface */
		StretchDIBits( hdc,
					   0, 0, ATARI_VIS_WIDTH, Screen_HEIGHT,
					   0, 0, ATARI_VIS_WIDTH, Screen_HEIGHT,
					   s_Buffer.pTarget, s_lpbmi, DIB_RGB_COLORS, SRCCOPY );

		DD_SurfaceReleaseDC( hdc );
	}
	else
	{
		ServeDDError( IDS_DDERR_SURFACE_LOCK, hResult, FALSE );
		return;
	}

	/* Blit to the primary surface from a memory one */
	if( FAILED(hResult = BlitSurface()) )
	{
		if( hResult == DDERR_WASSTILLDRAWING )
			return;

		if( hResult == DDERR_SURFACELOST )
			RestoreSurfaces();
		else
			ServeDDError( IDS_DDERR_SURFACE_BLT, hResult, FALSE );
	}
} /* #OF# Screen_DDraw_Double */

/*========================================================
Function : Screen_DDraw_Double_Interpolation
=========================================================*/
/* #FN#
   Draws Atari screen in windowed mode x 2 with interpolation */
static
void
/* #AS#
   Nothing */
Screen_DDraw_Double_Interpolation( void )
{
	HRESULT	hResult;
	HDC	hdc;

	Interpolate2( ATARI_HORZ_CLIP, ATARI_VIS_WIDTH, Screen_HEIGHT,
				  ATARI_DOUBLE_VIS_WIDTH, ATARI_DOUBLE_HEIGHT,
				  s_Buffer.pSource, s_Buffer.pTarget, 0, FALSE );

	if( SUCCEEDED(hResult = DD_SurfaceGetDC( &hdc )) )
	{
		/* This StretchDIB basically does only the color space conversion to the memory surface */
		StretchDIBits( hdc,
					   0, 0, ATARI_DOUBLE_VIS_WIDTH, ATARI_DOUBLE_HEIGHT,
					   0, 0, ATARI_DOUBLE_VIS_WIDTH, ATARI_DOUBLE_HEIGHT,
					   s_Buffer.pTarget, s_lpbmi, DIB_RGB_COLORS, SRCCOPY );

		DD_SurfaceReleaseDC( hdc );
	}
	else
	{
		ServeDDError( IDS_DDERR_SURFACE_LOCK, hResult, FALSE );
		return;
	}

	/* Blit to the primary surface from a memory one */
	if( FAILED(hResult = BlitSurface()) )
	{
		if( hResult == DDERR_WASSTILLDRAWING )
			return;

		if( hResult == DDERR_SURFACELOST )
			RestoreSurfaces();
		else
			ServeDDError( IDS_DDERR_SURFACE_BLT, hResult, FALSE );
	}
} /* #OF# Screen_DDraw_Double_Interpolation */

/*========================================================
Function : Screen_DDraw_Double_Scanlines
=========================================================*/
/* #FN#
   Draws Atari screen in windowed mode x 2 with scan lines */
static
void
/* #AS#
   Nothing */
Screen_DDraw_Double_Scanlines( void )
{
	HRESULT	hResult;
	HDC	hdc;

#ifdef PFUSIK_ASM
	if( _IsFlagSet( g_Screen.ulState, SM_OPTN_USE_MMX ) )
	{
		static ULONG colMask[2] = {0xf0f0f0f0, 0xf0f0f0f0};
		static ULONG lumMask[2] = {0x07070707, 0x07070707};
		_asm
		{
			mov		esi, dword ptr [s_Buffer.pSource]	;; The source pointer
			add		esi, ATARI_HORZ_CLIP
			mov		edi, dword ptr [s_Buffer.pTarget]	;; The dest pointer
			mov		ebx, Screen_HEIGHT		;; Number of line pairs (ATARI_HEIGHT)
			mov		edx, ATARI_VIS_WIDTH	;; Offset from normal line to dark line
			movq	mm2, qword ptr [colMask]
			movq	mm3, qword ptr [lumMask]
		mmx_line:
			mov		ecx, 0x15				;; Count of OWORDs (ATARI_VIS_WIDTH/16)
		mmx_oword:
			movq	mm0, qword ptr [esi]	;; Load original pixels
			movq	mm4, qword ptr [esi + 8]
			add		esi, 16					;; Increment source pointer
			movq	mm1, mm0				;; Copy original pixels
			movq	mm5, mm4
			movq	qword ptr [edi], mm0	;; Store original pixels
			movq	qword ptr [edi + 8], mm4
			psrlq	mm1, 1					;; Darken brightness
			psrlq	mm5, 1
			pand	mm0, mm2				;; Extract colors
			pand	mm4, mm2
			pand	mm1, mm3				;; Extract darkened brightness
			pand	mm5, mm3
			por		mm0, mm1				;; Combine color with brightness
			por		mm4, mm5
			movq	qword ptr [edi + edx], mm0	;; Store darkened pixels
			movq	qword ptr [edi + edx + 8], mm4
			add		edi, 16					;; Increment dest pointer
			dec		ecx						;; Decrement QWORD counter
			jnz		mmx_oword				;; Back to start of QWORD operation

			add		esi, 0x30				;; Add in the offset to the next scan line for src
			add		edi, edx				;; Skip over the dark pixels
			dec		ebx
			jnz		mmx_line

			emms							;; Empty MMX state
		}
	}
	else
	{
		_asm
		{
			mov		esi, dword ptr [s_Buffer.pSource]	;; The source pointer
			add		esi, ATARI_HORZ_CLIP
			mov		edi, dword ptr [s_Buffer.pTarget]	;; The dest pointer
			mov		ebx, Screen_HEIGHT		;; Number of line pairs (ATARI_HEIGHT)
		new_line:
			mov		ecx, 0x54				;; Our count of DWORDs to copy (ATARI_VIS_WIDTH/4)
			rep		movsd					;; Move that string

			sub		esi, ATARI_VIS_WIDTH	;; Move back to origin of original scan line
			mov		ecx, 0x54				;; Count of DWORDs (ATARI_VIS_WIDTH/4)
		dark_line:
			mov		eax, dword ptr [esi]	;; Make copy of original pixels
			mov		edx, eax				;; Another copy of original pixels
			and		eax, 0xf0f0f0f0			;; Extract high nybbles - colors
			shr		edx, 1					;; Darken brightness
			and		edx, 0x07070707			;; Extract darkened brightness
			add		eax, edx				;; Combine color with brightness
			add		esi, 4					;; Increment source pointer
			mov		dword ptr [edi], eax	;; Store darkened pixels
			add		edi, 4					;; Increment dest pointer
			dec		ecx						;; Decrement our DWORD counter
			jnz		dark_line				;; Back to start of DWORD operation

			add		esi, 0x30				;; Add in the offset to the next scan line for src
			dec		ebx
			jnz		new_line
		}
	}
#else
	_asm
	{
		mov		esi, dword ptr [s_Buffer.pSource]	;; The source pointer
		add		esi, ATARI_HORZ_CLIP
		mov		edi, dword ptr [s_Buffer.pTarget]	;; The dest pointer
		mov		eax, Screen_HEIGHT					;; Number of line pairs (ATARI_HEIGHT)
	new_line:
		mov		ecx, 054h				;; Our count of DWORDs to copy (ATARI_VIS_WIDTH/4)
		rep		movsd					;; Move that string

		sub		esi, ATARI_VIS_WIDTH	;; Move back to origin of original scan line
		mov		edx, ATARI_VIS_WIDTH	;; The pixel counter
	dark_line:
		mov		bl, byte ptr [esi]		;; Make copy of original pixel
		mov		cl, bl					;; Another copy of original pixel
		and		bl, 0x0f				;; Bottom nybble of this pixel
		shr		bl, 1					;; nybble / 2
		and		cl, 0xf0				;; Top nybble of this pixel
		add		cl, bl					;; Make darkened pixel by adding nybbles
		inc		esi						;; Move to next source pixel
		mov		byte ptr [edi], cl		;; Copy darkened pixel into destination point
		inc		edi
		dec		edx     				;; Decrement our pixel counter
		jnz		dark_line				;; Back to start of pixel operation

		add		esi, 030h				;; Add in the offset to the next scan line for src
		dec		eax
		jnz		new_line
	}
#endif
	if( SUCCEEDED(hResult = DD_SurfaceGetDC( &hdc )) )
	{
		/* This StretchDIB basically does only the color space conversion to the memory surface */
		StretchDIBits( hdc,
					   0, 0, ATARI_VIS_WIDTH, ATARI_DOUBLE_HEIGHT,
					   0, 0, ATARI_VIS_WIDTH, ATARI_DOUBLE_HEIGHT,
					   s_Buffer.pTarget, s_lpbmi, DIB_RGB_COLORS, SRCCOPY );

		DD_SurfaceReleaseDC( hdc );
	}
	else
	{
		ServeDDError( IDS_DDERR_SURFACE_LOCK, hResult, FALSE );
		return;
	}

	/* Blit to the primary surface from a memory one */
	if( FAILED(hResult = BlitSurface()) )
	{
		if( hResult == DDERR_WASSTILLDRAWING )
			return;

		if( hResult == DDERR_SURFACELOST )
			RestoreSurfaces();
		else
			ServeDDError( IDS_DDERR_SURFACE_BLT, hResult, FALSE );
	}
} /* #OF# Screen_DDraw_Double_Scanlines */

/*========================================================
Function : Screen_DDraw_Double_HiEnd
=========================================================*/
/* #FN#
   Draws Atari screen in windowed mode HiEnd stretched */
static
void
/* #AS#
   Nothing */
Screen_DDraw_Double_HiEnd( void )
{
	HRESULT	hResult;
	HDC	hdc;
	int i,j;
	unsigned char *c = s_Buffer.pSource + ATARI_HORZ_CLIP;
	unsigned long *s = (unsigned long *)hqSource;
	unsigned short r,g,b;

	for (j=0; j<Screen_HEIGHT; j++) {
		for (i=0; i<ATARI_VIS_WIDTH; i++) {
			r = s_lpbmi->bmiColors[ *c ].rgbRed;
			g = s_lpbmi->bmiColors[ *c ].rgbGreen;
			b = s_lpbmi->bmiColors[ *c ].rgbBlue;
			*s = (r << 16) + (g << 8) + b;
			c++;
			s++;
		}
		c+=ATARI_FULL_HORZ_CLIP;
	}

	hq2x_32(hqSource, hqTarget, ATARI_VIS_WIDTH, Screen_HEIGHT);

	if( SUCCEEDED(hResult = DD_SurfaceGetDC( &hdc )) )
	{
		StretchDIBits( hdc,
					   0, 0, ATARI_DOUBLE_VIS_WIDTH, ATARI_DOUBLE_HEIGHT,
					   0, 0, ATARI_DOUBLE_VIS_WIDTH, ATARI_DOUBLE_HEIGHT,
					   hqTarget, hqlpbmi, DIB_RGB_COLORS, SRCCOPY );

		DD_SurfaceReleaseDC( hdc );
	}
	else
	{
		ServeDDError( IDS_DDERR_SURFACE_LOCK, hResult, FALSE );
		return;
	}

	/* Blit to the primary surface from a memory one */
	if( FAILED(hResult = BlitSurface()) )
	{
		if( hResult == DDERR_WASSTILLDRAWING )
			return;

		if( hResult == DDERR_SURFACELOST )
			RestoreSurfaces();
		else
			ServeDDError( IDS_DDERR_SURFACE_BLT, hResult, FALSE );
	}

} /* #OF# Screen_DDraw_Double_HiEnd */

/*========================================================
Function : Screen_DDraw_Full_Blt
=========================================================*/
/* #FN#
   Draws Atari screen in full screen mode */
static
void
/* #AS#
   Nothing */
Screen_DDraw_Full_Blt( void )
{
	HRESULT	hResult;

	if( _IsFlagSet( g_Screen.ulState, SM_OPTN_SAFE_MODE ) )
	{
		if( !LockMemorySurface( TRUE ) )
			return;

		if( s_Surface.nPitch != s_Surface.nWidth )
		{
			UCHAR *pSource = s_Buffer.pSource;
			UCHAR *pTarget = s_Buffer.pTarget;
			int i;

			for( i = 0; i < Screen_HEIGHT; i++ )
			{
				CopyMemory( pTarget, pSource, Screen_WIDTH );
				pTarget += s_Surface.nPitch;
				pSource += Screen_WIDTH;
			}
		}
		else
			CopyMemory( s_Buffer.pTarget, s_Buffer.pSource, ATARI_SCREEN_SIZE );

		LockMemorySurface( FALSE );
	}

	if( s_DDraw.nFlipBuffers )
	{
		/* Blit-and-flip the DirectDraw surfaces */
		hResult = FlipSurface();
	}
	else
		/* Blit to the primary surface from a memory one */
		hResult = BlitSurface();

	if( FAILED(hResult) )
	{
		if( hResult == DDERR_WASSTILLDRAWING )
			return;

		if( hResult == DDERR_SURFACELOST )
			RestoreSurfaces();
		else
			ServeDDError( IDS_DDERR_SURFACE_BLT, hResult, FALSE );
	}
} /* #OF# Screen_DDraw_Full_Blt */

/*========================================================
Function : Screen_DDraw_Full_Blt_Interpolation
=========================================================*/
/* #FN#
   Draws Atari screen in full mode with interpolation */
static
void
/* #AS#
   Nothing */
Screen_DDraw_Full_Blt_Interpolation( void )
{
	HRESULT	hResult;

	if( MEMORY_SYSTEM != g_Screen.nMemoryType )
	{
		Interpolate2( ATARI_HORZ_CLIP, ATARI_VIS_WIDTH, Screen_HEIGHT,
					  s_Surface.nWidth, ATARI_DOUBLE_HEIGHT,
					  s_Buffer.pSource, s_Buffer.pWorkScr, 0, FALSE );

		if( _IsFlagSet( g_Screen.ulState, SM_OPTN_SAFE_MODE ) )
		{
			if( !LockMemorySurface( TRUE ) )
				return;
		}
		if( s_Surface.nPitch != s_Surface.nWidth )
		{
			UCHAR *pSource = s_Buffer.pWorkScr;
			UCHAR *pTarget = s_Buffer.pTarget;
			int i;

			for( i = 0; i < ATARI_DOUBLE_HEIGHT; i++ )
			{
				CopyMemory( pTarget, pSource, s_Surface.nWidth );
				pTarget += s_Surface.nPitch;
				pSource += s_Surface.nWidth;
			}
		}
		else
			CopyMemory( s_Buffer.pTarget, s_Buffer.pWorkScr, s_Surface.nWidth * ATARI_DOUBLE_HEIGHT );
	}
	else
	{
		if( _IsFlagSet( g_Screen.ulState, SM_OPTN_SAFE_MODE ) )
		{
			if( !LockMemorySurface( TRUE ) )
				return;
		}
		Interpolate2( ATARI_HORZ_CLIP, ATARI_VIS_WIDTH, Screen_HEIGHT,
					  s_Surface.nWidth, ATARI_DOUBLE_HEIGHT,
					  s_Buffer.pSource, s_Buffer.pTarget, s_Surface.nPitch, FALSE );
	}
	if( _IsFlagSet( g_Screen.ulState, SM_OPTN_SAFE_MODE ) )
		LockMemorySurface( FALSE );

	if( s_DDraw.nFlipBuffers )
	{
		/* Blit-and-flip the DirectDraw surfaces */
		hResult = FlipSurface();
	}
	else
		/* Blit to the primary surface from a memory one */
		hResult = BlitSurface();

	if( FAILED(hResult) )
	{
		if( hResult == DDERR_WASSTILLDRAWING )
			return;

		if( hResult == DDERR_SURFACELOST )
			RestoreSurfaces();
		else
			ServeDDError( IDS_DDERR_SURFACE_BLT, hResult, FALSE );
	}
} /* #OF# Screen_DDraw_Full_Blt_Interpolation */

/*========================================================
Function : Screen_DDraw_Full_Blt_Scanlines
=========================================================*/
/* #FN#
   Draws Atari screen in full mode with scan lines */
static
void
/* #AS#
   Nothing */
Screen_DDraw_Full_Blt_Scanlines( void )
{
	HRESULT	hResult;
	int nCache;

	if( _IsFlagSet( g_Screen.ulState, SM_OPTN_SAFE_MODE ) )
	{
		if( !LockMemorySurface( TRUE ) )
			return;
	}
	/* Calculate cache size of the memory surface */
	nCache = s_Surface.nPitch - s_Surface.nWidth;

#ifdef PFUSIK_ASM
	_asm
	{
		mov		esi, dword ptr [s_Buffer.pSource]	;; The source pointer
		add		esi, ATARI_HORZ_CLIP
		mov		edi, dword ptr [s_Buffer.pTarget]	;; The dest pointer
		add		edi, ATARI_HORZ_CLIP
		add		nCache, 0x30			;; Skip margins in dest
		mov		ebx, Screen_HEIGHT		;; Number of line pairs (ATARI_HEIGHT)
	new_line:
		mov		ecx, 0x54				;; Our count of DWORDs to copy (ATARI_VIS_WIDTH/4)
		rep		movsd					;; Move that string

		sub		esi, ATARI_VIS_WIDTH	;; Move back to origin of original scan line
		add		edi, 0x30				;; Skip margins in dest
		mov		ecx, 0x54				;; Count of DWORDs (ATARI_VIS_WIDTH/4)
	dark_line:
		mov		eax, dword ptr [esi]	;; Make copy of original pixels
		mov		edx, eax				;; Another copy of original pixels
		and		eax, 0xf0f0f0f0			;; Extract high nybbles - colors
		shr		edx, 1					;; Darken brightness
		and		edx, 0x07070707			;; Extract darkened brightness
		add		eax, edx				;; Combine color with brightness
		add		esi, 4					;; Increment source pointer
		mov		dword ptr [edi], eax	;; Store darkened pixels
		add		edi, 4					;; Increment dest pointer
		dec		ecx						;; Decrement our DWORD counter
		jnz		dark_line				;; Back to start of DWORD operation

		add		esi, 0x30				;; Add in the offset to the next scan line for src
		add		edi, nCache
		dec		ebx
		jnz		new_line
	}
#else
	_asm
	{
		mov		esi, dword ptr [s_Buffer.pSource]	;; The source pointer
		mov		edi, dword ptr [s_Buffer.pTarget]	;; The dest pointer
		mov		eax, Screen_HEIGHT					;; Number of line pairs (ATARI_HEIGHT)
	new_line:
		mov		ecx, 060h				;; Our count of DWORDs to copy (Screen_WIDTH/4)
		rep		movsd					;; Move that string

		sub		esi, Screen_WIDTH		;; Move back to origin of original scan line
		mov		edx, Screen_WIDTH		;; The pixel counter
	dark_line:
		mov		bl, byte ptr [esi]		;; Make copy of original pixel
		mov		cl, bl					;; Another copy of original pixel
		and		bl, 0x0f				;; Bottom nybble of this pixel
		shr		bl, 1					;; nybble / 2
		and		cl, 0xf0				;; Top nybble of this pixel
		add		cl, bl					;; Make darkened pixel by adding nybbles
		inc		esi						;; Move to next source pixel
		mov		byte ptr [edi], cl		;; Copy darkened pixel into destination point
		add		edi, nCache
		dec		edx						;; Decrement our pixel counter
		jnz		dark_line				;; Back to start of pixel operation

		dec		eax
		jnz		new_line
	}
#endif
	if( _IsFlagSet( g_Screen.ulState, SM_OPTN_SAFE_MODE ) )
		LockMemorySurface( FALSE );

	if( s_DDraw.nFlipBuffers )
	{
		/* Blit-and-flip the DirectDraw surfaces */
		hResult = FlipSurface();
	}
	else
		/* Blit to the primary surface from a memory one */
		hResult = BlitSurface();

	if( FAILED(hResult) )
	{
		if( hResult == DDERR_WASSTILLDRAWING )
			return;

		if( hResult == DDERR_SURFACELOST )
			RestoreSurfaces();
		else
			ServeDDError( IDS_DDERR_SURFACE_BLT, hResult, FALSE );
	}
} /* #OF# Screen_DDraw_Full_Blt_Scanlines */

/*========================================================
Function : Screen_DDraw_1024_Interpolation
=========================================================*/
/* #FN#
   Draws Atari screen in full 1024 mode with interpolation */
static
void
/* #AS#
   Nothing */
Screen_DDraw_1024_Interpolation( void )
{
	HRESULT	hResult;

	if( MEMORY_SYSTEM != g_Screen.nMemoryType )
	{
		Interpolate3( ATARI_HORZ_CLIP, ATARI_VIS_WIDTH, Screen_HEIGHT,
					  ATARI_TRIPLE_VIS_WIDTH, ATARI_TRIPLE_HEIGHT,
					  s_Buffer.pSource, s_Buffer.pWorkScr, 0 );

		if( _IsFlagSet( g_Screen.ulState, SM_OPTN_SAFE_MODE ) )
		{
			if( !LockMemorySurface( TRUE ) )
				return;
		}
		if( s_Surface.nPitch != s_Surface.nWidth )
		{
			UCHAR *pSource = s_Buffer.pWorkScr;
			UCHAR *pTarget = s_Buffer.pTarget;
			int i;

			for( i = 0; i < ATARI_TRIPLE_HEIGHT; i++ )
			{
				CopyMemory( pTarget, pSource, ATARI_TRIPLE_VIS_WIDTH );
				pTarget += s_Surface.nPitch;
				pSource += ATARI_TRIPLE_VIS_WIDTH;
			}
		}
		else
			CopyMemory( s_Buffer.pTarget, s_Buffer.pWorkScr, ATARI_TRIPLE_VIS_SCREEN_SIZE );
	}
	else
	{
		if( _IsFlagSet( g_Screen.ulState, SM_OPTN_SAFE_MODE ) )
		{
			if( !LockMemorySurface( TRUE ) )
				return;
		}
		Interpolate3( ATARI_HORZ_CLIP, ATARI_VIS_WIDTH, Screen_HEIGHT,
					  ATARI_TRIPLE_VIS_WIDTH, ATARI_TRIPLE_HEIGHT,
					  s_Buffer.pSource, s_Buffer.pTarget, s_Surface.nPitch );
	}
	if( _IsFlagSet( g_Screen.ulState, SM_OPTN_SAFE_MODE ) )
		LockMemorySurface( FALSE );

	if( s_DDraw.nFlipBuffers )
	{
		/* Blit-and-flip the DirectDraw surfaces */
		hResult = FlipSurface();
	}
	else
		/* Blit to the primary surface from a memory one */
		hResult = BlitSurface();

	if( FAILED(hResult) )
	{
		if( hResult == DDERR_WASSTILLDRAWING )
			return;

		if( hResult == DDERR_SURFACELOST )
			RestoreSurfaces();
		else
			ServeDDError( IDS_DDERR_SURFACE_BLT, hResult, FALSE );
	}
} /* #OF# Screen_DDraw_1024_Interpolation */

/*========================================================
Function : Screen_DDraw_1024_Scanlines
=========================================================*/
/* #FN#
   Draws Atari screen in full 1024 mode with scan lines */
static
void
/* #AS#
   Nothing */
Screen_DDraw_1024_Scanlines( void )
{
	HRESULT	hResult;
	int nCache;

	if( _IsFlagSet( g_Screen.ulState, SM_OPTN_SAFE_MODE ) )
	{
		if( !LockMemorySurface( TRUE ) )
			return;
	}
	/* Calculate cache size of the memory surface */
	nCache = s_Surface.nPitch - s_Surface.nWidth;

#ifdef PFUSIK_ASM
	_asm
	{
		mov		esi, dword ptr [s_Buffer.pSource]	;; The source pointer
		add		esi, ATARI_HORZ_CLIP
		mov		edi, dword ptr [s_Buffer.pTarget]	;; The dest pointer
		add		edi, ATARI_HORZ_CLIP
		add		nCache, 0x30			;; Skip margins in dest
		mov		ebx, Screen_HEIGHT		;; Number of line pairs (ATARI_HEIGHT)
	new_line:
		mov		ecx, 0x54				;; Our count of DWORDs to copy (ATARI_VIS_WIDTH/4)
		rep		movsd					;; Move that string

		sub		esi, ATARI_VIS_WIDTH	;; Move back to origin of original scan line
		add		edi, 0x30				;; Skip margins in dest
		mov		ecx, 0x54				;; Our count of DWORDs to copy (ATARI_VIS_WIDTH/4)
		rep		movsd					;; Move that string

		sub		esi, ATARI_VIS_WIDTH	;; Move back to origin of original scan line
		add		edi, 0x30				;; Skip margins in dest
		mov		ecx, 0x54				;; Count of DWORDs (ATARI_VIS_WIDTH/4)
	dark_line:
		mov		eax, dword ptr [esi]	;; Make copy of original pixels
		mov		edx, eax				;; Another copy of original pixels
		and		eax, 0xf0f0f0f0			;; Extract high nybbles - colors
		shr		edx, 1					;; Darken brightness
		and		edx, 0x07070707			;; Extract darkened brightness
		add		eax, edx				;; Combine color with brightness
		add		esi, 4					;; Increment source pointer
		mov		dword ptr [edi], eax	;; Store darkened pixels
		add		edi, 4					;; Increment dest pointer
		dec		ecx						;; Decrement our DWORD counter
		jnz		dark_line				;; Back to start of DWORD operation

		add		esi, 0x30				;; Add in the offset to the next scan line for src
		add		edi, nCache
		dec		ebx
		jnz		new_line
	}
#else
	_asm
	{
		;; Order in this routine has been manipulated to maximize pairing on a Pentium processor
		mov		esi, dword ptr [s_Buffer.pSource]	;; the source pointer
		mov		edi, dword ptr [s_Buffer.pTarget]	;; the dest pointer
		mov		eax, Screen_HEIGHT					;; Number of line pairs (ATARI_HEIGHT)
	new_line:
		mov		ecx, 060h			;; Our count of DWORDs to copy (Screen_WIDTH/4)
		rep		movsd				;; Move that string
		sub		esi, Screen_WIDTH	;; Move back to origin of this scan line
		mov		ecx, 060h			;; Going to move another set of words...
		rep		movsd				;; Make another copy of scanline

		sub		esi, Screen_WIDTH	;; Move back to origin of original scan line
		mov		edx, Screen_WIDTH	;; The pixel counter
	dark_line:
		mov		bl, byte ptr [esi]	;; Make copy of original pixel
		mov		cl, bl				;; Another copy of original pixel
		and		bl, 0x0f			;; Bottom nybble of this pixel
		shr		bl, 1				;; nybble / 2
		and		cl, 0xf0			;; Top nybble of this pixel
		add		cl, bl				;; Make darkened pixel by adding nybbles
		inc		esi					;; move to next source pixel
		mov		byte ptr [edi], cl	;; Copy darkened pixel into destination point
		add		edi, nCache
		dec		edx					;; Decrement our pixel counter
		jnz		dark_line			;; Back to start of pixel operation

		dec		eax					;; Decrement our line counter
		jnz		new_line
	}
#endif
	if( _IsFlagSet( g_Screen.ulState, SM_OPTN_SAFE_MODE ) )
		LockMemorySurface( FALSE );

	if( s_DDraw.nFlipBuffers )
	{
		/* Blit-and-flip the DirectDraw surfaces */
		hResult = FlipSurface();
	}
	else
		/* Blit to the primary surface from a memory one */
		hResult = BlitSurface();

	if( FAILED(hResult) )
	{
		if( hResult == DDERR_WASSTILLDRAWING )
			return;

		if( hResult == DDERR_SURFACELOST )
			RestoreSurfaces();
		else
			ServeDDError( IDS_DDERR_SURFACE_BLT, hResult, FALSE );
	}
} /* #OF# Screen_DDraw_Full_Blt_Scanlines */

/*========================================================
Function : Screen_GDI_Double
=========================================================*/
/* #FN#
   Draws Atari screen in GDI windowed mode x 2 */
static
void
/* #AS#
   Nothing */
Screen_GDI_Double( void )
{
	UBYTE *pSourceLine = (UBYTE *)s_Buffer.pSource + ATARI_HORZ_CLIP;
	UBYTE *pTargetLine = (UBYTE *)s_Buffer.pTarget;
#ifdef PFUSIK_ASM
	int nIncD = ATARI_DOUBLE_VIS_WIDTH;
#else
	int nIncD = 0;
#endif

	if( g_Screen.pfOutput )
	{
		/* We have to flip the top-down bitmap verticaly, because
		   video compressors do not support bitmap mirroring
		*/
		pTargetLine += ATARI_DOUBLE_VIS_SCREEN_SIZE - 2 * ATARI_DOUBLE_VIS_WIDTH;
#ifdef PFUSIK_ASM
		nIncD = -3 * ATARI_DOUBLE_VIS_WIDTH;
#else
		nIncD = -4 * ATARI_DOUBLE_VIS_WIDTH;
#endif
	}

#ifdef PFUSIK_ASM
	if( _IsFlagSet( g_Screen.ulState, SM_OPTN_USE_MMX ) )
	{
#ifdef INTEL_ASM
		int nWidth = ATARI_VIS_WIDTH;
		int nSrcWidth = Screen_WIDTH;
		int nDestWidth = ATARI_DOUBLE_VIS_WIDTH;
		int nHeight = Screen_HEIGHT;
		/* Size: 2x2, Mode: DUPLICATE, Cpu: MMX */
		_ASSERT(nWidth % 16 == 0);
		_asm
		{
			mov		ebx, dword ptr [pSourceLine]
			mov		eax, dword ptr [pTargetLine]
			sub		ebx, 32
			sub		eax, 64
			mov		ecx, dword ptr [nWidth]
			mov		edi, dword ptr [nDestWidth]
			and		ecx, 0ffffffe0h
			mov		esi, dword ptr [nHeight]
			lea		ecx, [eax + 2 * ecx]
		pixel_2x2_duplicate_mmx:
			movq	mm0, qword ptr [ebx + 32]
			movq	mm1, mm0
			mov		edx, dword ptr [eax + 64]	; Cache read -- better performance
			add		ebx, 32
			mov		edx, dword ptr [eax + 96]	; Cache read -- better performance
			add		eax, 64
			punpcklbw	mm0, mm0
			movq	mm2, qword ptr [ebx + 8]	; 2nd read
			punpckhbw	mm1, mm1
			movq	qword ptr [eax + edi], mm0	; 2a write -- row no. 2
			movq	qword ptr [eax + edi + 8], mm1	; 2b write -- row no. 2
			movq	mm3, mm2
			movq	qword ptr [eax], mm0		; 1a write -- row no. 1
			punpcklbw	mm2, mm2
			movq	qword ptr [eax + 8], mm1	; 1b write
			movq	qword ptr [eax + 16], mm2	; 1c write
			punpckhbw	mm3, mm3
			movq	mm4, qword ptr [ebx + 16]	; 3rd read
			movq	qword ptr [eax + 24], mm3	; 1d write
			movq	mm5, mm4
			movq	qword ptr [eax + edi + 16], mm2	; 2c write
			punpcklbw	mm4, mm4
			movq	qword ptr [eax + edi + 24], mm3	; 2d write
			punpckhbw	mm5, mm5
			movq	qword ptr [eax + 32], mm4	; 1e write
			movq	mm6, qword ptr [ebx + 24]	; 4th read
			movq	qword ptr [eax + 40], mm5	; 1f write
			movq	mm7, mm6
			movq	qword ptr [eax + edi + 32], mm4	; 2e write
			punpcklbw	mm6, mm6
			movq	qword ptr [eax + edi + 40], mm5	; 2f write
			punpckhbw	mm7, mm7
			movq	qword ptr [eax + 48], mm6	; 1g write
			movq	qword ptr [eax + 56], mm7	; 1h write
			movq	qword ptr [eax + edi + 48], mm6	; 2g write
			movq	qword ptr [eax + edi + 56], mm7	; 2h write
			cmp		ecx, eax
			jne		pixel_2x2_duplicate_mmx

			test	dword ptr [nWidth], 16
			jz		cont_2x2_duplicate_mmx
			movq	mm0, qword ptr [ebx + 32]
			movq	mm1, mm0
			mov		edx, dword ptr [eax + 64]	; Cache read -- better performance
			add		ebx, 16
			mov		edx, dword ptr [eax + 96]	; Cache read -- better performance
			add		eax, 32
			punpcklbw	mm0, mm0
			movq	mm2, qword ptr [ebx + 24]	; 2nd read
			punpckhbw	mm1, mm1
			movq	qword ptr [eax + edi + 32], mm0	; 2a write -- row no. 2
			movq	qword ptr [eax + edi + 40], mm1	; 2b write -- row no. 2
			movq	mm3, mm2
			movq	qword ptr [eax + 32], mm0		; 1a write -- row no. 1
			punpcklbw	mm2, mm2
			movq	qword ptr [eax + 40], mm1	; 1b write
			movq	qword ptr [eax + 48], mm2	; 1c write
			punpckhbw	mm3, mm3
			movq	qword ptr [eax + 56], mm3	; 1d write
			movq	qword ptr [eax + edi + 48], mm2	; 2c write
			movq	qword ptr [eax + edi + 56], mm3	; 2d write

		cont_2x2_duplicate_mmx:
			mov		edx, dword ptr [nWidth]
			lea		ecx, [ecx + 2 * edi]
			sub		eax, edx
			sub		ebx, edx
			sub		eax, edx
			add		ebx, dword ptr [nSrcWidth]
			lea		eax, [eax + 2 * edi]
			dec		esi
			jnz		pixel_2x2_duplicate_mmx

			emms
		}
#else /*INTEL_ASM*/
		_asm
		{
			mov		esi, dword ptr [pSourceLine]	;; The source pointer
			mov		edi, dword ptr [pTargetLine]	;; The dest pointer
			mov		ebx, Screen_HEIGHT				;; Number of line pairs (ATARI_HEIGHT)
		mmx_line:
			mov		ecx, 0x15						;; Count of OWORDs (ATARI_VIS_WIDTH/16)
		mmx_oword:
			movq	mm0, qword ptr [esi]			;; mm0=HGFEDCBA
			movq	mm2, qword ptr [esi + 8]		;; mm2=PONMLKJI
			movq	mm1, mm0						;; mm1=HGFEDCBA
			movq	mm3, mm2						;; mm3=PONMLKJI
			punpcklbw	mm0, mm0					;; mm0=DDCCBBAA
			punpckhbw	mm1, mm1					;; mm1=HHGGFFEE
			punpcklbw	mm2, mm2					;; mm2=LLKKJJII
			punpckhbw	mm3, mm3					;; mm3=PPOONNMM
			movq	qword ptr [edi], mm0			;; Store upper copies
			movq	qword ptr [edi + 8], mm1
			movq	qword ptr [edi + 16], mm2
			movq	qword ptr [edi + 24], mm3
			movq	qword ptr [edi + ATARI_DOUBLE_VIS_WIDTH], mm0	;; Store lower copies
			movq	qword ptr [edi + ATARI_DOUBLE_VIS_WIDTH + 8], mm1
			movq	qword ptr [edi + ATARI_DOUBLE_VIS_WIDTH + 16], mm2
			movq	qword ptr [edi + ATARI_DOUBLE_VIS_WIDTH + 24], mm3
			add		esi, 16							;; Increment source pointer
			add		edi, 32							;; Increment dest pointer
			dec		ecx								;; Decrement OWORD counter
			jnz		mmx_oword						;; Back to start of OWORD->HWORD operation

			add		esi, 0x30						;; Add in the offset to the next scan line for src
			add		edi, dword ptr [nIncD]			;; Top-down: ATARI_DOUBLE_VIS WIDTH, Bottom-up: -3 * ATARI_DOUBLE_VIS_WIDTH
			dec		ebx
			jnz		mmx_line

			emms									;; Empty MMX state
		}
#endif /*INTEL_ASM*/
	}
	else
	{
		int lineCount;
		_asm
		{
			mov		esi, dword ptr [pSourceLine]	;; The source pointer
			mov		edi, dword ptr [pTargetLine]	;; The dest pointer
			mov		dword ptr [lineCount], Screen_HEIGHT
		new_line_2:
			mov		ecx, 0x54
		copy_scanline_2:
			mov		eax, dword ptr [esi]
			add		esi, 4
			add		edi, 8
			mov		edx, eax
			mov		ebx, eax
			bswap	edx
			and		eax, 00000ffffh
			and		edx, 0ffff0000h
			add		edx, eax
			mov		eax, ebx
			bswap	eax
			rol		edx, 8
			mov		dword ptr [edi - 8], edx
			and		ebx, 0ffff0000h
			and		eax, 00000ffffh
			mov		dword ptr [edi - 8 + ATARI_DOUBLE_VIS_WIDTH], edx
			add		ebx, eax
			ror		ebx, 8
			dec		ecx
			mov		[edi - 4], ebx
			mov		[edi - 4 + ATARI_DOUBLE_VIS_WIDTH], ebx
			jnz		copy_scanline_2
			add		esi, 0x30						;; Add in the offset to the next scan line for src
			add		edi, dword ptr [nIncD]			;; Top-down: ATARI_DOUBLE_VIS WIDTH, Bottom-up: -3 * ATARI_DOUBLE_VIS_WIDTH
			dec		dword ptr [lineCount]
			jnz		new_line_2
		}
	}
#else
	_asm
	{
		;; Order in this routine has been manipulated to maximize pairing on a Pentium processor
		mov		ebx, dword ptr [pSourceLine]	;; The source pointer
		mov		edi, dword ptr [pTargetLine]	;; The dest pointer
		mov		eax, 0f00000h		;; Number of scan lines in the source (shifted 16 bits)
	new_line_1:
		mov		ecx, 0150h			;; We're going to copy ATARI_VIS_WIDTH pixel pairs
	copy_bytes_1:
		mov		dl, byte ptr [ebx]	;; Get this pixel for doubling
		mov		byte ptr [edi], dl	;; Store copy 1
		inc		edi
		inc		ebx					;; Next source
		mov		byte ptr [edi], dl	;; Store copy 2
		inc		edi					;; Next destination

		dec		ecx
		jnz		copy_bytes_1		;; Do this until ATARI_VIS_WIDTH pairs are done

		mov		esi, edi			;; Current destination pointer into source index
		mov		ecx, 0a8h			;; Our count of DWORDs to copy (ATARI_VIS_WIDTH * 2 / 4)
		sub		esi, 02a0h			;; Move back ATARI_VIS_WIDTH * 2 (beginning of pixel-doubled scan line)
		add		ebx, 030h			;; Add in the offset to the next scan line for src
		rep		movsd				;; Move that string
		add		edi, dword ptr [nIncD]	;; Top-down: 0, Bottom-up: -4 * ATARI_DOUBLE_VIS_WIDTH

		sub		eax, 10000h			;; Decrement high word of eax
		jnz		new_line_1			;; Jump if we're not done
	}
#endif
	if( g_Screen.pfOutput )
		/* Save the DIB to video output stream */
		Video_SaveFrame( s_Buffer.pTarget, ATARI_DOUBLE_VIS_SCREEN_SIZE, NULL, 0 );

	/* Make the prepared bitmap visible on the screen */
	StretchDIBits( g_Screen.hDC,
				   0, 0, ATARI_DOUBLE_VIS_WIDTH, ATARI_DOUBLE_HEIGHT,
				   0, 0, ATARI_DOUBLE_VIS_WIDTH, ATARI_DOUBLE_HEIGHT,
				   s_Buffer.pTarget, s_lpbmi, DIB_RGB_COLORS, SRCCOPY );

} /* #OF# Screen_GDI_Double */

/*========================================================
Function : Screen_GDI_Double_Interpolation
=========================================================*/
/* #FN#
   Draws Atari screen in GDI windowed mode x 2 with interpolation */
static
void
/* #AS#
   Nothing */
Screen_GDI_Double_Interpolation( void )
{
	if( g_Screen.pfOutput )
	{
		/* We have to flip the top-down bitmap verticaly, because
		   video compressors do not support bitmap mirroring
		*/
		Interpolate2( ATARI_HORZ_CLIP, ATARI_VIS_WIDTH, Screen_HEIGHT,
					  ATARI_DOUBLE_VIS_WIDTH, ATARI_DOUBLE_HEIGHT,
					  s_Buffer.pSource, s_Buffer.pTarget, 0, TRUE );
		/* Save the DIB to video output stream */
		Video_SaveFrame( s_Buffer.pTarget, ATARI_DOUBLE_VIS_SCREEN_SIZE, NULL, 0 );
	}
	else
	{
		Interpolate2( ATARI_HORZ_CLIP, ATARI_VIS_WIDTH, Screen_HEIGHT,
					  ATARI_DOUBLE_VIS_WIDTH, ATARI_DOUBLE_HEIGHT,
					  s_Buffer.pSource, s_Buffer.pTarget, 0, FALSE );
	}
	/* Make the prepared bitmap visible on the screen */
	StretchDIBits( g_Screen.hDC,
				   0, 0, ATARI_DOUBLE_VIS_WIDTH, ATARI_DOUBLE_HEIGHT,
				   0, 0, ATARI_DOUBLE_VIS_WIDTH, ATARI_DOUBLE_HEIGHT,
				   s_Buffer.pTarget, s_lpbmi, DIB_RGB_COLORS, SRCCOPY );

} /* #OF# Screen_GDI_Double_Interpolation */

/*========================================================
Function : Screen_GDI_Double_Scanlines
=========================================================*/
/* #FN#
   Draws Atari screen in GDI windowed mode x 2 with scan lines */
static
void
/* #AS#
   Nothing */
Screen_GDI_Double_Scanlines( void )
{
	UBYTE *pSourceLine = (UBYTE *)s_Buffer.pSource + ATARI_HORZ_CLIP;
	UBYTE *pTargetLine = (UBYTE *)s_Buffer.pTarget;
	int nIncD = ATARI_DOUBLE_VIS_WIDTH;

	if( g_Screen.pfOutput )
	{
		/* We have to flip the top-down bitmap verticaly, because
		   video compressors do not support bitmap mirroring
		*/
		pTargetLine += ATARI_DOUBLE_VIS_SCREEN_SIZE - 2 * ATARI_DOUBLE_VIS_WIDTH;
		nIncD = -3 * ATARI_DOUBLE_VIS_WIDTH;
	}

#ifdef PFUSIK_ASM
	if( _IsFlagSet( g_Screen.ulState, SM_OPTN_USE_MMX ) )
	{
		static ULONG colMask[2] = {0xf0f0f0f0, 0xf0f0f0f0};
		static ULONG lumMask[2] = {0x07070707, 0x07070707};
		_asm
		{
			mov		esi, dword ptr [pSourceLine]	;; The source pointer
			mov		edi, dword ptr [pTargetLine]	;; The dest pointer
			mov		ebx, Screen_HEIGHT				;; Number of line pairs (ATARI_HEIGHT)
			movq	mm2, qword ptr [colMask]
			movq	mm3, qword ptr [lumMask]
		mmx_line:
			mov		ecx, 0x2a				;; Count of QWORDs (ATARI_VIS_WIDTH/8)
		mmx_qword:
			movq	mm0, qword ptr [esi]	;; Load 8 source pixels
			movq	mm4, mm0
			punpcklbw	mm0, mm0			;; Double the pixels
			punpckhbw	mm4, mm4
			movq	mm1, mm0				;; Copy original pixels
			movq	mm5, mm4
			movq	qword ptr [edi], mm0	;; Store normal pixels
			movq	qword ptr [edi + 8], mm4
			pand	mm0, mm2				;; Extract colors
			pand	mm4, mm2
			psrlq	mm1, 1					;; Darken brightness
			psrlq	mm5, 1
			add		esi, 8					;; Increment source pointer
			pand	mm1, mm3				;; Extract darkened brightness
			pand	mm5, mm3
			por		mm0, mm1				;; Combine color with brightness
			por		mm4, mm5
			movq	qword ptr [edi + ATARI_DOUBLE_VIS_WIDTH], mm0	;; Store lower copies
			movq	qword ptr [edi + ATARI_DOUBLE_VIS_WIDTH + 8], mm4
			add		edi, 16					;; Increment dest pointer
			dec		ecx						;; Decrement DWORD counter
			jnz		mmx_qword				;; Back to start of QWORD->OWORD operation

			add		esi, 0x30				;; Add in the offset to the next scan line for src
			add		edi, dword ptr [nIncD]	;; Top-down: ATARI_DOUBLE_VIS WIDTH, Bottom-up: -3 * ATARI_DOUBLE_VIS_WIDTH
			dec		ebx
			jnz		mmx_line

			emms							;; Empty MMX state
		}
	}
	else
	{
		_asm
		{
			mov		esi, dword ptr [pSourceLine]	;; The source pointer
			mov		edi, dword ptr [pTargetLine]	;; The dest pointer
			mov		ebx, Screen_HEIGHT				;; Number of line pairs (ATARI_HEIGHT)
		new_line_1:
			mov		ecx, 0xa8						;; Count of WORDs to copy (ATARI_VIS_WIDTH/2)
		copy_scanline_1:
			xor		edx, edx						;; edx=0000
			mov		ax, word ptr [esi]				;; eax=..HL
			mov		dl, al							;; edx=000L
			mov		al, ah							;; eax=..HH
			mov		dh, dl							;; edx=00LL
			shl		eax, 16							;; eax=HH00
			or		eax, edx						;; eax=HHLL
			mov		dword ptr [edi], eax			;; Store normal pixels
			mov		edx, eax						;; Copy of normal pixels
			and		eax, 0xf0f0f0f0					;; Extract high nybbles - colors
			shr		edx, 1							;; Darken brightness
			add		esi, 2							;; Increment source pointer
			and		edx, 0x07070707					;; Extract darkened brightness
			add		eax, edx						;; Combine color with brightness
			mov		dword ptr [edi + ATARI_DOUBLE_VIS_WIDTH], eax	;; Store darkened pixels
			add		edi, 4							;; Increment dest pointer
			dec		ecx								;; Decrement our WORD counter
			jnz		copy_scanline_1					;; Back to start of WORD operation

			add		esi, 0x30						;; Add in the offset to the next scan line for src
			add		edi, dword ptr [nIncD]			;; Top-down: ATARI_DOUBLE_VIS WIDTH, Bottom-up: -3 * ATARI_DOUBLE_VIS_WIDTH
			dec		ebx
			jnz		new_line_1
		}
	}
#else
	_asm
	{
		;; Order in this routine has been manipulated to maximize pairing on a Pentium processor
		mov		esi, dword ptr [pSourceLine]	;; The source pointer
		mov		edi, dword ptr [pTargetLine]	;; The dest pointer
		dec		edi
		mov		eax, 0f00000h			;; Number of scan lines in the source (shifted 16 bits)
	new_line_1:
		mov		ax, ATARI_VIS_WIDTH		;; We're going to copy ATARI_VIS_WIDTH pixel pairs
	copy_scanline_1:
		mov		dl, byte ptr [esi]								;; Get this pixel
		inc		edi												;; Next destination
		mov		cl, dl											;; Make a copy of original pixel
		mov		bl, dl											;; Another copy of original pixel
		and		bl, 0x0f										;; Bottom nybble of this pixel
		mov		byte ptr [edi], dl								;; Store copy 1 of regular pixel
		shr		bl, 1											;; bits / 2
		and		cl, 0xf0										;; Top nybble of this pixel
		add		bl, cl											;; (pixel val / 16) * 16 + (val % 16) / 2
		mov		byte ptr [edi + ATARI_DOUBLE_VIS_WIDTH], bl	;; Store darkline pixel 1
		inc		esi												;; Next source pixel
		inc		edi												;; Next destination
		mov		byte ptr [edi], dl								;; Store copy 2 of regular pixel
		mov		byte ptr [edi + ATARI_DOUBLE_VIS_WIDTH], bl	;; Store darkline pixel 2

		dec		ax					;; Decrement counter
		jnz		copy_scanline_1		;; Do this until ATARI_VIS_WIDTH pairs are done

		add		edi, dword ptr [nIncD]			;; Top-down: ATARI_DOUBLE_VIS WIDTH, Bottom-up: -3 * ATARI_DOUBLE_VIS_WIDTH
		add		esi, 030h			;; Add in the offset to the next scan line for src

		sub		eax, 10000h			;; Decrement high word of eax
		jnz		new_line_1			;; Jump if we're not done
	}
#endif
	if( g_Screen.pfOutput )
		/* Save the DIB to video output stream */
		Video_SaveFrame( s_Buffer.pTarget, ATARI_DOUBLE_VIS_SCREEN_SIZE, NULL, 0 );

	/* Make the prepared bitmap visible on the screen */
	StretchDIBits( g_Screen.hDC,
				   0, 0, ATARI_DOUBLE_VIS_WIDTH, ATARI_DOUBLE_HEIGHT,
				   0, 0, ATARI_DOUBLE_VIS_WIDTH, ATARI_DOUBLE_HEIGHT,
				   s_Buffer.pTarget, s_lpbmi, DIB_RGB_COLORS, SRCCOPY );

} /* #OF# Screen_GDI_Double_Scanlines */

/*========================================================
Function : Screen_GDI_Double_HiEnd
=========================================================*/
/* #FN#
   Draws Atari screen in GDI windowed mode HiEnd stretched */
static
void
/* #AS#
   Nothing */
Screen_GDI_Double_HiEnd( void )
{
	int i,j;
	unsigned char *c = s_Buffer.pSource + ATARI_HORZ_CLIP;
	unsigned long *s = (unsigned long *)hqSource;
	unsigned short r,g,b;

	for (j=0; j<Screen_HEIGHT; j++) {
		for (i=0; i<ATARI_VIS_WIDTH; i++) {
			r = s_lpbmi->bmiColors[ *c ].rgbRed;
			g = s_lpbmi->bmiColors[ *c ].rgbGreen;
			b = s_lpbmi->bmiColors[ *c ].rgbBlue;
			*s = (r << 16) + (g << 8) + b;
			c++;
			s++;
		}
		c+=ATARI_FULL_HORZ_CLIP;
	}

	hq2x_32(hqSource, hqTarget, ATARI_VIS_WIDTH, Screen_HEIGHT);

	StretchDIBits( g_Screen.hDC,
				   0, 0, ATARI_DOUBLE_VIS_WIDTH, ATARI_DOUBLE_HEIGHT,
				   0, 0, ATARI_DOUBLE_VIS_WIDTH, ATARI_DOUBLE_HEIGHT,
				   hqTarget, hqlpbmi, DIB_RGB_COLORS, SRCCOPY );

} /* #OF# Screen_GDI_Double_HiEnd */

/*========================================================
Function : Screen_GDI
=========================================================*/
/* #FN#
   Draws Atari screen in GDI windowed mode x 1 */
static
void
/* #AS#
   Nothing */
Screen_GDI( void )
{
	if( g_Screen.pfOutput )
	{
		/* We have to flip the top-down bitmap verticaly, because
		   video compressors do not support bitmap mirroring
		*/
		_asm
		{
			mov		esi, dword ptr [s_Buffer.pSource]	;; The source pointer
			add		esi, ATARI_HORZ_CLIP
			mov		edi, dword ptr [s_Buffer.pTarget]	;; The dest pointer
			add		edi, 0139b0h						;; ATARI_VIS_SCREEN_SIZE - ATARI_VIS_WIDTH
			mov		eax, Screen_HEIGHT					;; Number of lines
		scan_line_1:
			mov		ecx, 054h		;; Our count of DWORDs to copy (ATARI_VIS_WIDTH/4)
			rep		movsd			;; Move that string
			add		esi, 030h		;; Add in the offset to the next scan line for source
			sub		edi, 02a0h		;; 2 * ATARI_VIS_WIDTH

			dec		eax
			jnz		scan_line_1
		}
		/* Save the DIB to video output stream */
		Video_SaveFrame( s_Buffer.pTarget, ATARI_VIS_SCREEN_SIZE, NULL, 0 );
	}
	else
	{
		/* The top-down bitmap will be mirrored by StretchDIBits */
		_asm
		{
			mov		esi, dword ptr [s_Buffer.pSource]	;; the source pointer
			add		esi, ATARI_HORZ_CLIP
			mov		edi, dword ptr [s_Buffer.pTarget]	;; the dest pointer
			mov		eax, Screen_HEIGHT					;; number of lines
		scan_line_2:
			mov		ecx, 054h		;; Our count of DWORDs to copy (ATARI_VIS_WIDTH/4)
			rep		movsd			;; Move that string
			add		esi, 030h		;; add in the offset to the next scan line for src

			dec		eax
			jnz		scan_line_2
		}
	}
	/* Make the prepared bitmap visible on the screen */
	StretchDIBits( g_Screen.hDC,
				   0, 0, ATARI_VIS_WIDTH, Screen_HEIGHT,
				   0, 0, ATARI_VIS_WIDTH, Screen_HEIGHT,
				   s_Buffer.pTarget, s_lpbmi, DIB_RGB_COLORS, SRCCOPY );

} /* #OF# Screen_GDI */


/* TO DO:
   The methods below should be used EVERY time when the screen
   freezing for GUI is needed. Maybe in the next release...
*/

/*========================================================
Function : Screen_AllocUserScreen
=========================================================*/
/* #FN#
   Prepares the screen for using dialogs safely */
UINT
/* #AS#
   Flags to use as the input of the FreeUserScreen method */
Screen_AllocUserScreen( void )
{
	UINT uiResult = 0;

	if( _IsFlagSet( g_Screen.ulState, SM_MODE_FULL ) )
	{
		if( ST_ATARI_STOPPED )
		{
			/* It may be a good idea to handle the case here,
			   for example when the DisplayMessage is called
			   after cleaning the screen using 'deep' flag */
			if( !g_Screen.bLocked )
			{
				Screen_FlipToGDI( FALSE );

				if( ST_MENUBAR_HIDDEN )
					Screen_ShowMenuBar( TRUE );
			}
			_TRACE0("Screen_AllocUserScreen:FlipToGDI\n");
		}
		/* There is a valid bitmap to prepare */
		else if( NULL == s_Redraw.hBmp )
		{
			/* Prepare a screen bitmap for redrawing */
			Screen_DrawFrozen( FALSE, FALSE, FALSE, TRUE );
			uiResult |= AF_BITMAP;

			_TRACE0("Screen_AllocUserScreen:AF_BITMAP\n");
		}
		/* Set a system palette if possible and necessary */
		if( !g_Screen.Pal.bUseSysCol && Screen_UseSystemPalette() )
			uiResult |= AF_COLORS;

		/* Set the 'menu-active' semaphore */
		if( !g_Screen.bLocked )
		{
			g_Screen.bLocked = TRUE;
			uiResult |= AF_LOCKED;

			_TRACE0("Screen_AllocUserScreen:AF_LOCKED\n");
		}
	}
	return uiResult;

} /* #OF# Screen_AllocUserScreen */

/*========================================================
Function : Screen_FreeUserScreen
=========================================================*/
/* #FN#
   Frees the screen allocated by AllocUserScreen */
void
/* #AS#
   Nothing */
Screen_FreeUserScreen(
	UINT uiAlloc
)
{
	if( _IsFlagSet( uiAlloc, AF_BITMAP ) )
	{
		Screen_FreeRedraw();

		/* TO DO:
		   I'm not sure if the screen cleaning should be placed here;
		   Used temporarily for 2.8
		*/
		if( ST_FLIPPED_BUFFERS && !_IsFlagSet( g_Screen.ulState, SM_ATTR_NO_MENU ) )
			/* Needed for multiple-buffering modes to clean the menus */
			Screen_ShowMenuBar( TRUE );

		/* Clean up the screen */
		Screen_Clear( FALSE, FALSE );
	}
	if( _IsFlagSet( uiAlloc, AF_COLORS ) )
		Screen_UseAtariPalette( FALSE );

	if( _IsFlagSet( uiAlloc, AF_LOCKED ) )
		g_Screen.bLocked = FALSE;

} /* #OF# Screen_FreeUserScreen */
