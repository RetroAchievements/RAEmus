/****************************************************************************
File    : display_win.c
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Implementation of DirectDraw wrappers
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 28.10.2003
*/

#include <stdio.h>
#include "WinConfig.h"
#include "macros.h"
#include "directdraw.h"


/* Private objects */

/* Use DirectX 3 for backward compatibility with NT 4.0 */
static LPDIRECTDRAW         s_pDirectDraw  = NULL;
static LPDIRECTDRAWSURFACE  s_apSurfaces [ 4 ] = { NULL, NULL, NULL, NULL };
/* Use advanced features of the DirectX 7 if available */
static LPDIRECTDRAW7        s_pDirectDraw7 = NULL;
static LPDIRECTDRAWSURFACE7 s_apSurfaces7[ 4 ] = { NULL, NULL, NULL, NULL };
/* Common interfaces */
static LPDIRECTDRAWCLIPPER  s_pDDClipper   = NULL;
static LPDIRECTDRAWPALETTE  s_pDDPalette   = NULL;

/* We are going to remember some screen modes information here */
static struct
{
	DWORD  dwMode;
	DWORD  dwWidth;
	DWORD  dwHeight;
	DWORD  dwRefreshRates[ MAX_REFRESH_RATE_NO + 1 ];
}
s_aScreenModeInfo[] =
{
	{ MODE_320_200,  320,  200, { 0 } },
	{ MODE_320_240,  320,  240, { 0 } },
	{ MODE_320_400,  320,  400, { 0 } },
	{ MODE_400_300,  400,  300, { 0 } },
	{ MODE_512_384,  512,  384, { 0 } },
	{ MODE_640_400,  640,  400, { 0 } },
	{ MODE_640_480,  640,  480, { 0 } },
	{ MODE_800_600,  800,  600, { 0 } },
	{ MODE_1024_768, 1024, 768, { 0 } }
};

static const int s_nScreenModeInfoNo = sizeof(s_aScreenModeInfo)/sizeof(s_aScreenModeInfo[0]);


/*========================================================
Function : DD_ObjectCreate
=========================================================*/
/* #FN#
   Creates a DirectDraw object(s) */
HRESULT
/* #AS#
   TRUE if succeeded, otherwise FALSE */
DD_ObjectCreate( void )
{
	HRESULT hResult = DD_OK;

	/* Get the object if not already allocated */
	if( !s_pDirectDraw )
	{
		hResult = DirectDrawCreate( NULL, &s_pDirectDraw, 0 );
		if( SUCCEEDED(hResult) )
		{
			/* Some advanced features need a newer DirectDraw interface */
			if( FAILED(IDirectDraw_QueryInterface( s_pDirectDraw, &IID_IDirectDraw7, (LPVOID *)&s_pDirectDraw7 )) )
			{
				s_pDirectDraw7 = NULL;
			}
		}
	}
	return hResult;

} /* #OF# DD_ObjectCreate */

/*========================================================
Function : DD_ObjectRelease
=========================================================*/
/* #FN#
   Releases a DirectDraw object(s) */
void
/* #AS#
   Nothing */
DD_ObjectRelease( void )
{
	/* The Release method decreases the reference count of the objects by 1 */
	if( s_pDirectDraw7 )
	{
		IDirectDraw7_RestoreDisplayMode( s_pDirectDraw7 );
		IDirectDraw7_Release( s_pDirectDraw7 );
		s_pDirectDraw7 = NULL;
	}
	else if( s_pDirectDraw )
		IDirectDraw_RestoreDisplayMode( s_pDirectDraw );

	if( s_pDirectDraw )
	{
		IDirectDraw_Release( s_pDirectDraw );
		s_pDirectDraw = NULL;
	}
} /* #OF# DD_ObjectRelease */

/*========================================================
Function : EnumModesCallback
=========================================================*/
/* #FN#
   Checks available display modes */
static
HRESULT WINAPI
/* #AS#
   Always DDENUMRET_OK */
EnumModesCallback(
	LPDDSURFACEDESC pDDSurfaceDesc, /* #IN# */
	LPVOID          pContext        /* #IN# */
)
{
	if( pDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount == 8 )
	{
		ULONG *pScreenModes = pContext;

		int i;
		for( i = 0; i < s_nScreenModeInfoNo; i++ )
		{
			if( pDDSurfaceDesc->dwHeight == s_aScreenModeInfo[ i ].dwHeight &&
				pDDSurfaceDesc->dwWidth  == s_aScreenModeInfo[ i ].dwWidth )
			{
				if( pScreenModes )
					*pScreenModes |= s_aScreenModeInfo[ i ].dwMode;
				break;
			}
		}
	}
	return DDENUMRET_OK;

} /* #OF# EnumModesCallback */

/*========================================================
Function : EnumModesCallbackEx
=========================================================*/
/* #FN#
   Checks available display modes (extended version) */
static
HRESULT WINAPI
/* #AS#
   Always DDENUMRET_OK */
EnumModesCallbackEx(
	LPDDSURFACEDESC2 pDDSurfaceDesc, /* #IN# */
	LPVOID           pContext        /* #IN# */
)
{
	if( pDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount == 8 )
	{
		ULONG *pScreenModes = pContext;

		int i, j;
		for( i = 0; i < s_nScreenModeInfoNo; i++ )
		{
			if( pDDSurfaceDesc->dwHeight == s_aScreenModeInfo[ i ].dwHeight &&
				pDDSurfaceDesc->dwWidth  == s_aScreenModeInfo[ i ].dwWidth )
			{
				if( pScreenModes )
					*pScreenModes |= s_aScreenModeInfo[ i ].dwMode;

				for( j = 0; j < MAX_REFRESH_RATE_NO; j++ )
				{
					if( 0 == s_aScreenModeInfo[ i ].dwRefreshRates[ j ] )
					{
						s_aScreenModeInfo[ i ].dwRefreshRates[ j ]     = pDDSurfaceDesc->dwRefreshRate;
						s_aScreenModeInfo[ i ].dwRefreshRates[ j + 1 ] = 0;
						break;
					}
				}
				break;
			}
		}
	}
	return DDENUMRET_OK;

} /* #OF# EnumModesCallbackEx */

/*========================================================
Function : DD_EnumDisplayModes
=========================================================*/
/* #FN#
   Enumerates all of the display modes the hardware exposes */
HRESULT
/* #AS#
   DD_OK if succeeded */
DD_EnumDisplayModes(
	ULONG *pScreenModes
)
{
	HRESULT hResult = DD_OK;

	if( s_pDirectDraw7 )
	{
		DDSURFACEDESC2 ddsd;

		ZeroMemory( &ddsd, sizeof(DDSURFACEDESC2) );
		ddsd.dwSize  = sizeof(DDSURFACEDESC2);
		ddsd.dwFlags = DDSD_PIXELFORMAT;
		ddsd.ddpfPixelFormat.dwRGBBitCount = 8;

		/* Enumerate graphics modes with different refresh rates */
		hResult = IDirectDraw7_EnumDisplayModes( s_pDirectDraw7, DDEDM_REFRESHRATES, &ddsd, pScreenModes, EnumModesCallbackEx );
	}
	else if( s_pDirectDraw )
	{
		DDSURFACEDESC ddsd;

		ZeroMemory( &ddsd, sizeof(DDSURFACEDESC) );
		ddsd.dwSize  = sizeof(DDSURFACEDESC);
		ddsd.dwFlags = DDSD_PIXELFORMAT;
		ddsd.ddpfPixelFormat.dwRGBBitCount = 8;

		hResult = IDirectDraw_EnumDisplayModes( s_pDirectDraw, 0, &ddsd, pScreenModes, EnumModesCallback );
	}
	return hResult;

} /* #OF# DD_EnumDisplayModes */

/*========================================================
Function : DD_SetCooperativeLevel
=========================================================*/
/* #FN#
   Determines the top-level behavior of the application */
HRESULT
/* #AS#
   DD_OK if succeeded */
DD_SetCooperativeLevel(
	HWND  hWnd,
	DWORD dwFlags
)
{
	HRESULT hResult = DD_OK;

	if( s_pDirectDraw7 )
	{
		hResult = IDirectDraw7_SetCooperativeLevel( s_pDirectDraw7, hWnd, dwFlags );
	}
	else if( s_pDirectDraw )
	{
		hResult = IDirectDraw_SetCooperativeLevel( s_pDirectDraw, hWnd, dwFlags );
	}
	return hResult;

} /* #OF# DD_SetCooperativeLevel */

/*========================================================
Function : DD_SetDisplayMode
=========================================================*/
/* #FN#
   Sets the mode of the display-device hardware */
HRESULT
/* #AS#
   DD_OK if succeeded */
DD_SetDisplayMode(
	DWORD dwWidth,
	DWORD dwHeight,
	DWORD dwBPP,
	DWORD dwRefreshRate
)
{
	HRESULT hResult = DD_OK;

	if( s_pDirectDraw7 )
	{
		hResult = IDirectDraw7_SetDisplayMode( s_pDirectDraw7, dwWidth, dwHeight, dwBPP, dwRefreshRate, 0 );
	}
	else if( s_pDirectDraw )
	{
		hResult = IDirectDraw_SetDisplayMode( s_pDirectDraw, dwWidth, dwHeight, dwBPP );
	}
	return hResult;

} /* #OF# DD_SetDisplayMode */

/*========================================================
Function : DD_RestoreDisplayMode
=========================================================*/
/* #FN#
   Resets the mode of the display device hardware */
HRESULT
/* #AS#
   DD_OK if succeeded */
DD_RestoreDisplayMode( void )
{
	HRESULT hResult = DD_OK;

	if( s_pDirectDraw7 )
	{
		hResult = IDirectDraw7_RestoreDisplayMode( s_pDirectDraw7 );
	}
	else if( s_pDirectDraw )
	{
		hResult = IDirectDraw_RestoreDisplayMode( s_pDirectDraw );
	}
	return hResult;

} /* #OF# DD_RestoreDisplayMode */

/*========================================================
Function : DD_FlipToGDISurface
=========================================================*/
/* #FN#
   Makes the surface that the GDI writes to the primary surface */
HRESULT
/* #AS#
   DD_OK if succeeded */
DD_FlipToGDISurface( void )
{
	HRESULT hResult = DD_OK;

	if( s_pDirectDraw7 )
	{
		hResult = IDirectDraw7_FlipToGDISurface( s_pDirectDraw7 );
	}
	else if( s_pDirectDraw )
	{
		hResult = IDirectDraw_FlipToGDISurface( s_pDirectDraw );
	}
	return hResult;

} /* #OF# DD_FlipToGDISurface */

/*========================================================
Function : DD_GetMonitorFrequency
=========================================================*/
/* #FN#
   Retrieves the frequency of the monitor being driven by the DirectDraw object */
HRESULT
/* #AS#
   DD_OK if succeeded */
DD_GetMonitorFrequency(
	ULONG *pRefreshRate
)
{
	HRESULT hResult = DD_OK;

	if( s_pDirectDraw7 )
	{
		hResult = IDirectDraw7_GetMonitorFrequency( s_pDirectDraw7, pRefreshRate );
	}
	else if( s_pDirectDraw )
	{
		hResult = IDirectDraw_GetMonitorFrequency( s_pDirectDraw, pRefreshRate );
	}
	return hResult;

} /* #OF# DD_GetMonitorFrequency */

/*========================================================
Function : DD_GetVideoCaps
=========================================================*/
/* #FN#
   Fills in the capabilities of the device driver for the hardware */
HRESULT
/* #AS#
   DD_OK if succeeded */
DD_GetVideoCaps(
	ULONG *pVideoCaps
)
{
	HRESULT hResult = DD_OK;

	if( s_pDirectDraw7 )
	{
		DDCAPS caps;
		ZeroMemory( &caps, sizeof(DDCAPS) );
		caps.dwSize = sizeof(DDCAPS);

		hResult = IDirectDraw7_GetCaps( s_pDirectDraw7, &caps, NULL );
		if( SUCCEEDED(hResult) )
		{
			if( caps.dwCaps & DDCAPS_VBI )
				*pVideoCaps |= DD_CAPS_VBI;
			if( caps.dwCaps2 & DDCAPS2_FLIPINTERVAL )
				*pVideoCaps |= DD_CAPS_FLIPINTERVAL;
			if( caps.dwCaps2 & DDCAPS2_FLIPNOVSYNC )
				*pVideoCaps |= DD_CAPS_FLIPNOVSYNC;
		}
	}
	else if( s_pDirectDraw )
	{
		DDCAPS caps;
		ZeroMemory( &caps, sizeof(DDCAPS) );
		caps.dwSize = sizeof(DDCAPS);

		hResult = IDirectDraw_GetCaps( s_pDirectDraw, &caps, NULL );
		if( SUCCEEDED(hResult) )
		{
			if( caps.dwCaps & DDCAPS_VBI )
				*pVideoCaps |= DD_CAPS_VBI;
		}
	}
	return hResult;

} /* #OF# DD_GetVideoCaps */

/*========================================================
Function : DD_WaitForVBlank
=========================================================*/
/* #FN#
   Helps the application synchronize itself with the vertical-blank interval */
HRESULT
/* #AS#
   DD_OK if succeeded */
DD_WaitForVBlank(
	DWORD dwFlags
)
{
	HRESULT hResult = DD_OK;

	if( s_pDirectDraw7 )
	{
		hResult = IDirectDraw7_WaitForVerticalBlank( s_pDirectDraw7, DDWAITVB_BLOCKBEGIN, NULL );
	}
	else if( s_pDirectDraw )
	{
		hResult = IDirectDraw_WaitForVerticalBlank( s_pDirectDraw, DDWAITVB_BLOCKBEGIN, NULL );
	}
	return hResult;

} /* #OF# DD_WaitForVBlank */

/*========================================================
Function : DD_SurfaceCreate
=========================================================*/
/* #FN#
   Creates a DirectDraw surface object for DirectDraw object */
HRESULT
/* #AS#
   DD_OK if succeeded */
DD_SurfaceCreate(
	DD_SurfaceParams_t *pParams,
	eSurfaceType        stSurface
)
{
	HRESULT hResult = DD_OK;

	if( s_pDirectDraw7 )
	{
		DDSURFACEDESC2 ddsd;

		ZeroMemory( &ddsd, sizeof(DDSURFACEDESC2) );
		ddsd.dwSize                        = sizeof(DDSURFACEDESC2);
		ddsd.dwHeight                      = pParams->dwHeight;
		ddsd.dwWidth                       = pParams->dwWidth;
		ddsd.dwFlags                       = pParams->dwFlags;
		ddsd.ddsCaps.dwCaps                = pParams->dwCaps;
		ddsd.dwBackBufferCount             = pParams->dwBackBufferCount;
		ddsd.ddpfPixelFormat.dwSize        = pParams->ddpfFormat.dwSize;
		ddsd.ddpfPixelFormat.dwFlags       = pParams->ddpfFormat.dwFlags;
		ddsd.ddpfPixelFormat.dwRGBBitCount = pParams->ddpfFormat.dwRGBBitCount;

		hResult = IDirectDraw7_CreateSurface( s_pDirectDraw7, &ddsd, &s_apSurfaces7[ stSurface ], NULL );
	}
	else if( s_pDirectDraw )
	{
		DDSURFACEDESC ddsd;

		ZeroMemory( &ddsd, sizeof(DDSURFACEDESC) );
		ddsd.dwSize                        = sizeof(DDSURFACEDESC);
		ddsd.dwHeight                      = pParams->dwHeight;
		ddsd.dwWidth                       = pParams->dwWidth;
		ddsd.dwFlags                       = pParams->dwFlags;
		ddsd.ddsCaps.dwCaps                = pParams->dwCaps;
		ddsd.dwBackBufferCount             = pParams->dwBackBufferCount;
		ddsd.ddpfPixelFormat.dwSize        = pParams->ddpfFormat.dwSize;
		ddsd.ddpfPixelFormat.dwFlags       = pParams->ddpfFormat.dwFlags;
		ddsd.ddpfPixelFormat.dwRGBBitCount = pParams->ddpfFormat.dwRGBBitCount;

		hResult = IDirectDraw_CreateSurface( s_pDirectDraw, &ddsd, &s_apSurfaces[ stSurface ], NULL );
	}
	return hResult;

} /* #OF# DD_SurfaceCreate */

/*========================================================
Function : DD_SurfaceRelease
=========================================================*/
/* #FN#
   Releases the DirectDraw surface object */
ULONG
/* #AS#
   The new reference count to the object */
DD_SurfaceRelease(
	eSurfaceType stSurface
)
{
	ULONG ulResult = 0;

	if( s_pDirectDraw7 )
	{
		if( s_apSurfaces7[ stSurface ] )
		{
			ulResult = IDirectDrawSurface7_Release( s_apSurfaces7[ stSurface ] );
			s_apSurfaces7[ stSurface ] = NULL;

			if( stPrimary == stSurface )
				s_apSurfaces7[ stBackBuf ] = NULL;
		}
	}
	else if( s_pDirectDraw )
	{
		if( s_apSurfaces[ stSurface ] )
		{
			ulResult = IDirectDrawSurface_Release( s_apSurfaces[ stSurface ] );
			s_apSurfaces[ stSurface ] = NULL;

			if( stPrimary == stSurface )
				s_apSurfaces[ stBackBuf ] = NULL;
		}
	}
	return ulResult;

} /* #OF# DD_SurfaceRelease */

/*========================================================
Function : DD_SurfaceIsLost
=========================================================*/
/* #FN#
   Determines whether the surface memory associated with
   a DirectDrawSurface object has been freed */
HRESULT
/* #AS#
   DD_OK if succeeded */
DD_SurfaceIsLost(
	eSurfaceType stSurface
)
{
	HRESULT hResult = DD_OK;

	if( s_pDirectDraw7 )
	{
		if( s_apSurfaces7[ stSurface ] )
			hResult = IDirectDrawSurface7_IsLost( s_apSurfaces7[ stSurface ] );
	}
	else if( s_pDirectDraw )
	{
		if( s_apSurfaces[ stSurface ] )
			hResult = IDirectDrawSurface_IsLost( s_apSurfaces[ stSurface ] );
	}
	return hResult;

} /* #OF# DD_SurfaceIsLost */

/*========================================================
Function : DD_SurfaceRestore
=========================================================*/
/* #FN#
   Releases a DirectDraw surface object */
HRESULT
/* #AS#
   DD_OK if succeeded */
DD_SurfaceRestore(
	eSurfaceType stSurface
)
{
	HRESULT hResult = DD_OK;

	if( s_pDirectDraw7 )
	{
		if( s_apSurfaces7[ stSurface ] )
			hResult = IDirectDrawSurface7_Restore( s_apSurfaces7[ stSurface ] );
	}
	else if( s_pDirectDraw )
	{
		if( s_apSurfaces[ stSurface ] )
			hResult = IDirectDrawSurface_Restore( s_apSurfaces[ stSurface ] );
	}
	return hResult;

} /* #OF# DD_SurfaceRestore */

/*========================================================
Function : DD_SurfaceLock
=========================================================*/
/* #FN#
   Obtains a pointer to the surface memory */
HRESULT
/* #AS#
   DD_OK if succeeded */
DD_SurfaceLock(
	eSurfaceType stSurface,
	DWORD        dwFlags,
	LPVOID      *ppBuffer,
	DWORD       *pPitch
)
{
	HRESULT hResult = DD_OK;

	if( s_pDirectDraw7 )
	{
		if( s_apSurfaces7[ stSurface ] )
		{
			DDSURFACEDESC2 ddsd;
			ZeroMemory( &ddsd, sizeof(DDSURFACEDESC2) );
			ddsd.dwSize = sizeof(DDSURFACEDESC2);

			hResult = IDirectDrawSurface7_Lock( s_apSurfaces7[ stSurface ], NULL, &ddsd, dwFlags, NULL );

			*ppBuffer = ddsd.lpSurface;
			*pPitch   = ddsd.lPitch;
		}
	}
	else if( s_pDirectDraw )
	{
		if( s_apSurfaces[ stSurface ] )
		{
			DDSURFACEDESC ddsd;
			ZeroMemory( &ddsd, sizeof(DDSURFACEDESC) );
			ddsd.dwSize = sizeof(DDSURFACEDESC);

			hResult = IDirectDrawSurface_Lock( s_apSurfaces[ stSurface ], NULL, &ddsd, dwFlags, NULL );

			*ppBuffer = ddsd.lpSurface;
			*pPitch   = ddsd.lPitch;
		}
	}
	return hResult;

} /* #OF# DD_SurfaceLock */

/*========================================================
Function : DD_SurfaceUnlock
=========================================================*/
/* #FN#
   Notifies DirectDraw that the direct surface manipulations are complete */
HRESULT
/* #AS#
   DD_OK if succeeded */
DD_SurfaceUnlock(
	eSurfaceType stSurface
)
{
	HRESULT hResult = DD_OK;

	if( s_pDirectDraw7 )
	{
		if( s_apSurfaces7[ stSurface ] )
			hResult = IDirectDrawSurface7_Unlock( s_apSurfaces7[ stSurface ], NULL );
	}
	else if( s_pDirectDraw )
	{
		if( s_apSurfaces[ stSurface ] )
			hResult = IDirectDrawSurface_Unlock( s_apSurfaces[ stSurface ], NULL/*TO DO: &ddsd*/ );
	}
	return hResult;

} /* #OF# DD_SurfaceUnlock */

/*========================================================
Function : DD_SurfaceGetDC
=========================================================*/
/* #FN#
   Creates a GDI-compatible handle of a device context for the surface */
HRESULT
/* #AS#
   DD_OK if succeeded */
DD_SurfaceGetDC(
	HDC *phDC
)
{
	HRESULT hResult = DD_OK;

	if( s_pDirectDraw7 )
	{
		if( s_apSurfaces7[ stOffscrn ] )
			hResult = IDirectDrawSurface7_GetDC( s_apSurfaces7[ stOffscrn ], phDC );
	}
	else if( s_pDirectDraw )
	{
		if( s_apSurfaces[ stOffscrn ] )
			hResult = IDirectDrawSurface_GetDC( s_apSurfaces[ stOffscrn ], phDC );
	}
	return hResult;

} /* #OF# DD_SurfaceGetDC */

/*========================================================
Function : DD_SurfaceReleaseDC
=========================================================*/
/* #FN#
   Releases the handle of a device context previously obtained */
HRESULT
/* #AS#
   DD_OK if succeeded */
DD_SurfaceReleaseDC(
	HDC hDC
)
{
	HRESULT hResult = DD_OK;

	if( s_pDirectDraw7 )
	{
		if( s_apSurfaces7[ stOffscrn ] )
			hResult = IDirectDrawSurface7_ReleaseDC( s_apSurfaces7[ stOffscrn ], hDC );
	}
	else if( s_pDirectDraw )
	{
		if( s_apSurfaces[ stOffscrn ] )
			hResult = IDirectDrawSurface_ReleaseDC( s_apSurfaces[ stOffscrn ], hDC );
	}
	return hResult;

} /* #OF# DD_SurfaceReleaseDC */

/*========================================================
Function : DD_SurfaceBlit
=========================================================*/
/* #FN#
   Performs a bit block transfer (blit) */
HRESULT
/* #AS#
   DD_OK if succeeded */
DD_SurfaceBlit(
	eSurfaceType stDestSurface,
	RECT        *pDestRect,
	eSurfaceType stSrcSurface,
	RECT        *pSrcRect,
	DWORD        dwFlags,
	LPDDBLTFX    pDDBltFx
)
{
	HRESULT hResult = DD_OK;

	if( s_pDirectDraw7 )
	{
		if( s_apSurfaces7[ stDestSurface ] )
			hResult = IDirectDrawSurface7_Blt( s_apSurfaces7[ stDestSurface ], pDestRect, s_apSurfaces7[ stSrcSurface ], pSrcRect, dwFlags, pDDBltFx );
	}
	else if( s_pDirectDraw )
	{
		if( s_apSurfaces[ stDestSurface ] )
			hResult = IDirectDrawSurface_Blt( s_apSurfaces[ stDestSurface ], pDestRect, s_apSurfaces[ stSrcSurface ], pSrcRect, dwFlags, pDDBltFx );
	}
	return hResult;

} /* #OF# DD_SurfaceBlit */

/*========================================================
Function : DD_SurfaceBlitFast
=========================================================*/
/* #FN#
   Performs a bit block transfer (fast) */
HRESULT
/* #AS#
   DD_OK if succeeded */
DD_SurfaceBlitFast(
	eSurfaceType stDestSurface,
	eSurfaceType stSrcSurface,
	DWORD        dwTrans
)
{
	HRESULT hResult = DD_OK;

	if( s_pDirectDraw7 )
	{
		if( s_apSurfaces7[ stDestSurface ] )
			hResult = IDirectDrawSurface7_BltFast( s_apSurfaces7[ stDestSurface ], 0, 0, s_apSurfaces7[ stSrcSurface ], NULL, dwTrans );
	}
	else if( s_pDirectDraw )
	{
		if( s_apSurfaces[ stDestSurface ] )
			hResult = IDirectDrawSurface_BltFast( s_apSurfaces[ stDestSurface ], 0, 0, s_apSurfaces[ stSrcSurface ], NULL, dwTrans );
	}
	return hResult;

} /* #OF# DD_SurfaceBlitFast */

/*========================================================
Function : DD_SurfaceFlip
=========================================================*/
/* #FN#
   Makes the surface memory associated with the DDSCAPS_BACKBUFFER
   surface become associated with the front-buffer surface */
HRESULT
/* #AS#
   DD_OK if succeeded */
DD_SurfaceFlip(
	DWORD dwFlags
)
{
	HRESULT hResult = DD_OK;

	if( s_pDirectDraw7 )
	{
		hResult = IDirectDrawSurface7_Flip( s_apSurfaces7[ stPrimary ], NULL, dwFlags );
	}
	else if( s_pDirectDraw )
	{
		hResult = IDirectDrawSurface_Flip( s_apSurfaces[ stPrimary ], NULL, dwFlags );
	}
	return hResult;

} /* #OF# DD_SurfaceFlip */

/*========================================================
Function : DD_SurfaceGetAttached
=========================================================*/
/* #FN#
   Obtains the attached surface that has the specified capabilities
   and increments the reference count of the retrieved interface */
HRESULT
/* #AS#
   DD_OK if succeeded */
DD_SurfaceGetAttached(
	DWORD dwCaps
)
{
	HRESULT hResult = DD_OK;

	if( s_apSurfaces7[ stPrimary ] )
	{
		DDSCAPS2 caps;
		ZeroMemory( &caps, sizeof(DDSCAPS2) );
		caps.dwCaps = dwCaps;

		hResult = IDirectDrawSurface7_GetAttachedSurface( s_apSurfaces7[ stPrimary ], &caps, &s_apSurfaces7[ stBackBuf ] );
	}
	else if( s_apSurfaces[ stPrimary ] )
	{
		DDSCAPS caps;
		ZeroMemory( &caps, sizeof(DDSCAPS) );
		caps.dwCaps = dwCaps;

		hResult = IDirectDrawSurface_GetAttachedSurface( s_apSurfaces[ stPrimary ], &caps, &s_apSurfaces[ stBackBuf ] );
	}
	return hResult;

} /* #OF# DD_SurfaceGetAttached */

/*========================================================
Function : DD_SurfaceSetClipper
=========================================================*/
/* #FN#
   Attaches a clipper object to or deletes one from a surface */
HRESULT
/* #AS#
   DD_OK if succeeded */
DD_SurfaceSetClipper(
	eSurfaceType stSurface,
	BOOL         bEnable
)
{
	HRESULT hResult = DD_OK;

	if( !bEnable || s_pDDClipper )
	{
		if( s_pDirectDraw7 )
		{
			if( s_apSurfaces7[ stSurface ] )
				hResult = IDirectDrawSurface7_SetClipper( s_apSurfaces7[ stSurface ], bEnable ? s_pDDClipper : NULL );
		}
		else if( s_pDirectDraw )
		{
			if( s_apSurfaces[ stSurface ] )
				hResult = IDirectDrawSurface_SetClipper( s_apSurfaces[ stSurface ], bEnable ? s_pDDClipper : NULL );
		}
	}
	return hResult;

} /* #OF# DD_SurfaceSetClipper */

/*========================================================
Function : DD_SurfaceSetPalette
=========================================================*/
/* #FN#
   Attaches a palette object to or detaches one from a surface */
HRESULT
/* #AS#
   DD_OK if succeeded */
DD_SurfaceSetPalette(
	eSurfaceType stSurface,
	BOOL         bEnable
)
{
	HRESULT hResult = DD_OK;

	if( !bEnable || s_pDDPalette )
	{
		if( s_pDirectDraw7 )
		{
			if( s_apSurfaces7[ stSurface ] )
				hResult = IDirectDrawSurface7_SetPalette( s_apSurfaces7[ stSurface ], bEnable ? s_pDDPalette : NULL );
		}
		else if( s_pDirectDraw )
		{
			if( s_apSurfaces[ stSurface ] )
				hResult = IDirectDrawSurface_SetPalette( s_apSurfaces[ stSurface ], bEnable ? s_pDDPalette : NULL );
		}
	}
	return hResult;

} /* #OF# DD_SurfaceSetPalette */

/*========================================================
Function : DD_ClipperCreate
=========================================================*/
/* #FN#
   Creates a DirectDraw clipper object */
HRESULT
/* #AS#
   DD_OK if succeeded */
DD_ClipperCreate(
	DWORD dwFlags
)
{
	HRESULT hResult = DD_OK;

	if( !s_pDDClipper )
	{
		if( s_apSurfaces7[ stPrimary ] )
		{
			hResult = IDirectDraw7_CreateClipper( s_pDirectDraw7, dwFlags, &s_pDDClipper, NULL );
		}
		else if( s_apSurfaces[ stPrimary ] )
		{
			hResult = IDirectDraw_CreateClipper( s_pDirectDraw, dwFlags, &s_pDDClipper, NULL );
		}
	}
	return hResult;

} /* #OF# DD_ClipperCreate */

/*========================================================
Function : DD_ClipperRelease
=========================================================*/
/* #FN#
   Releases a DirectDraw clipper object */
ULONG
/* #AS#
   The new reference count to the object */
DD_ClipperRelease( void )
{
	ULONG ulResult = 0;

	if( s_pDDClipper )
	{
		ulResult = IDirectDrawClipper_Release( s_pDDClipper );
		s_pDDClipper = NULL;
	}
	return ulResult;

} /* #OF# DD_ClipperRelease */

/*========================================================
Function : DD_ClipperSetHWnd
=========================================================*/
/* #FN#
   Sets the window handle that the clipper object uses to obtain clipping information */
HRESULT
/* #AS#
   DD_OK if succeeded */
DD_ClipperSetHWnd(
	DWORD dwFlags,
	HWND  hWnd
)
{
	HRESULT hResult = DD_OK;

	if( s_pDDClipper )
	{
		hResult = IDirectDrawClipper_SetHWnd( s_pDDClipper, dwFlags, hWnd );
	}
	return hResult;

} /* #OF# DD_ClipperSetHWnd */

/*========================================================
Function : DD_ClipperCreate
=========================================================*/
/* #FN#
   Creates a DirectDraw palette object */
HRESULT
/* #AS#
   DD_OK if succeeded */
DD_PaletteCreate(
	DWORD         dwFlags,
	PALETTEENTRY *pEntries
)
{
	HRESULT hResult = DD_OK;

	if( !s_pDDPalette )
	{
		if( s_pDirectDraw7 )
		{
			hResult = IDirectDraw7_CreatePalette( s_pDirectDraw7, dwFlags, pEntries, &s_pDDPalette, NULL );
		}
		else if( s_pDirectDraw )
		{
			hResult = IDirectDraw_CreatePalette( s_pDirectDraw, dwFlags, pEntries, &s_pDDPalette, NULL );
		}
	}
	return hResult;

} /* #OF# DD_PaletteCreate */

/*========================================================
Function : DD_PaletteRelease
=========================================================*/
/* #FN#
   Releases a DirectDraw palette object */
ULONG
/* #AS#
   The new reference count to the object */
DD_PaletteRelease( void )
{
	ULONG ulResult = 0;

	if( s_pDDPalette )
	{
		ulResult = IDirectDrawPalette_Release( s_pDDPalette );
		s_pDDPalette = NULL;
	}
	return ulResult;

} /* #OF# DD_PaletteRelease */

/*========================================================
Function : DD_PaletteSetEntries
=========================================================*/
/* #FN#
   Changes entries in a DirectDrawPalette object immediately */
HRESULT
/* #AS#
   DD_OK if succeeded */
DD_PaletteSetEntries(
	DWORD         dwFlags,
	DWORD         dwStartingEntry,
	DWORD         dwCount,
	PALETTEENTRY *pEntries
)
{
	HRESULT hResult = DD_OK;

	if( s_pDDPalette )
	{
		hResult = IDirectDrawPalette_SetEntries( s_pDDPalette, dwFlags, dwStartingEntry, dwCount, pEntries );
	}
	return hResult;

} /* #OF# DD_PaletteSetEntries */

/*========================================================
Function : DD_IsObjectCreated
=========================================================*/
/* #FN#
   Checks if a DirectDraw object has been created */
int
/* #AS#
   1 or 2 if the object has been created, otherwise 0 */
DD_IsObjectCreated( void )
{
	int nResult = 0;

	if( s_pDirectDraw7 )
		nResult = 2;
	else
	if( s_pDirectDraw )
		nResult = 1;

	return nResult;

} /* #OF# DD_IsObjectCreated */

/*========================================================
Function : DD_IsSurfaceCreated
=========================================================*/
/* #FN#
   Checks if a DirectDraw surface has been created */
BOOL
/* #AS#
   TRUE, if the surface has been created, otherwise FALSE */
DD_IsSurfaceCreated(
	eSurfaceType stSurface
)
{
	BOOL bResult = FALSE;

	if( s_pDirectDraw7 )
	{
		bResult = NULL != s_apSurfaces7[ stSurface ];
	}
	else if( s_pDirectDraw )
	{
		bResult = NULL != s_apSurfaces[ stSurface ];
	}
	return bResult;

} /* #OF# DD_IsSurfaceCreated */

/*========================================================
Function : DD_GetRefreshRates
=========================================================*/
/* #FN#
   Gets valid screen refresh rates for a screen mode */
void
/* #AS#
   Nothing */
DD_GetRefreshRates(
	DWORD dwScreenMode,
	int  *pRefreshRates,
	int   nBufferLen
)
{
	int i, j;
	nBufferLen = min( nBufferLen, MAX_REFRESH_RATE_NO );

	for( i = 0; i < s_nScreenModeInfoNo; i++ )
	{
		if( dwScreenMode == s_aScreenModeInfo[ i ].dwMode )
		{
			for( j = 0; j < nBufferLen; j++ )
			{
				pRefreshRates[ j ] = s_aScreenModeInfo[ i ].dwRefreshRates[ j ];

				if( 0 == pRefreshRates[ j ] )
					break;
			}
			if( j == nBufferLen )
				pRefreshRates[ j - 1 ] = 0;
			break;
		}
	}
} /* #OF# DD_GetRefreshRates */

/*========================================================
Function : DD_GetErrorString
=========================================================*/
/* #FN#
   Outputs a debug string to debugger */
BOOL
/* #AS#
   TRUE if succeeded, otherwise FALSE */
DD_GetErrorString(
	HRESULT hResult,      /* #IN# */
	LPSTR   pszErrorBuff, /* #IN# */
	DWORD   dwError       /* #IN# */
)
{
	char  szMsg[ 256 ];
	LPSTR pszError;
	DWORD dwLen;

	/* Check parameters */
	if( !pszErrorBuff || !dwError )
	{
		/* Error, invalid parameters */
		return FALSE;
	}

	switch( hResult )
	{
		/* The request completed successfully */
		case DD_OK:
			pszError = "DD_OK";
			break;

		/* This object is already initialized */
		case DDERR_ALREADYINITIALIZED:
			pszError = "DDERR_ALREADYINITIALIZED";
			break;

		/* This surface can not be attached to the requested surface */
		case DDERR_CANNOTATTACHSURFACE:
			pszError = "DDERR_CANNOTATTACHSURFACE";
			break;

		/* This surface can not be detached from the requested surface */
		case DDERR_CANNOTDETACHSURFACE:
			pszError = "DDERR_CANNOTDETACHSURFACE";
			break;

		/* Support is currently not available */
		case DDERR_CURRENTLYNOTAVAIL:
			pszError = "DDERR_CURRENTLYNOTAVAIL";
			break;

		/* An exception was encountered while performing the requested operation */
		case DDERR_EXCEPTION:
			pszError = "DDERR_EXCEPTION";
			break;

		/* Generic failure */
		case DDERR_GENERIC:
			pszError = "DDERR_GENERIC";
			break;

		/* Height of rectangle provided is not a multiple of reqd alignment */
		case DDERR_HEIGHTALIGN:
			pszError = "DDERR_HEIGHTALIGN";
			break;

		/* Unable to match primary surface creation request with existing
		   primary surface */
		case DDERR_INCOMPATIBLEPRIMARY:
			pszError = "DDERR_INCOMPATIBLEPRIMARY";
			break;

		/* One or more of the caps bits passed to the callback are incorrect */
		case DDERR_INVALIDCAPS:
			pszError = "DDERR_INVALIDCAPS";
			break;

		/* DirectDraw does not support provided Cliplist */
		case DDERR_INVALIDCLIPLIST:
			pszError = "DDERR_INVALIDCLIPLIST";
			break;

		/* DirectDraw does not support the requested mode */
		case DDERR_INVALIDMODE:
			pszError = "DDERR_INVALIDMODE";
			break;

		/* DirectDraw received a pointer that was an invalid DIRECTDRAW object */
		case DDERR_INVALIDOBJECT:
			pszError = "DDERR_INVALIDOBJECT";
			break;

		/* One or more of the parameters passed to the callback function are
		   incorrect */
		case DDERR_INVALIDPARAMS:
			pszError = "DDERR_INVALIDPARAMS";
			break;

		/* Pixel format was invalid as specified */
		case DDERR_INVALIDPIXELFORMAT:
			pszError = "DDERR_INVALIDPIXELFORMAT";
			break;

		/* Rectangle provided was invalid */
		case DDERR_INVALIDRECT:
			pszError = "DDERR_INVALIDRECT";
			break;

		/* Operation could not be carried out because one or more surfaces are locked */
		case DDERR_LOCKEDSURFACES:
			pszError = "DDERR_LOCKEDSURFACES";
			break;

		/* There is no 3D present. */
		case DDERR_NO3D:
			pszError = "DDERR_NO3D";
			break;

		/* Operation could not be carried out because there is no alpha accleration
		   hardware present or available */
		case DDERR_NOALPHAHW:
			pszError = "DDERR_NOALPHAHW";
			break;

		/* no clip list available */
		case DDERR_NOCLIPLIST:
			pszError = "DDERR_NOCLIPLIST";
			break;

		/* Operation could not be carried out because there is no color conversion
		   hardware present or available */
		case DDERR_NOCOLORCONVHW:
			pszError = "DDERR_NOCOLORCONVHW";
			break;

		/* Create function called without DirectDraw object method SetCooperativeLevel
		   being called */
		case DDERR_NOCOOPERATIVELEVELSET:
			pszError = "DDERR_NOCOOPERATIVELEVELSET";
			break;

		/* Surface doesn't currently have a color key */
		case DDERR_NOCOLORKEY:
			pszError = "DDERR_NOCOLORKEY";
			break;

		/* Operation could not be carried out because there is no hardware support
		   of the dest color key */
		case DDERR_NOCOLORKEYHW:
			pszError = "DDERR_NOCOLORKEYHW";
			break;

		/* No DirectDraw support possible with current display driver */
		case DDERR_NODIRECTDRAWSUPPORT:
			pszError = "DDERR_NODIRECTDRAWSUPPORT";
			break;

		/* Operation requires the application to have exclusive mode but the
		   application does not have exclusive mode */
		case DDERR_NOEXCLUSIVEMODE:
			pszError = "DDERR_NOEXCLUSIVEMODE";
			break;

		/* Flipping visible surfaces is not supported */
		case DDERR_NOFLIPHW:
			pszError = "DDERR_NOFLIPHW";
			break;

		/* There is no GDI present */
		case DDERR_NOGDI:
			pszError = "DDERR_NOGDI";
			break;

		/* Operation could not be carried out because there is no hardware present
		   or available */
		case DDERR_NOMIRRORHW:
			pszError = "DDERR_NOMIRRORHW";
			break;

		/* Requested item was not found */
		case DDERR_NOTFOUND:
			pszError = "DDERR_NOTFOUND";
			break;

		/* Operation could not be carried out because there is no overlay hardware
		   present or available */
		case DDERR_NOOVERLAYHW:
			pszError = "DDERR_NOOVERLAYHW";
			break;

		/* Operation could not be carried out because there is no appropriate raster
		   op hardware present or available */
		case DDERR_NORASTEROPHW:
			pszError = "DDERR_NORASTEROPHW";
			break;

		/* Operation could not be carried out because there is no rotation hardware
		   present or available */
		case DDERR_NOROTATIONHW:
			pszError = "DDERR_NOROTATIONHW";
			break;

		/* Operation could not be carried out because there is no hardware support
		   for stretching */
		case DDERR_NOSTRETCHHW:
			pszError = "DDERR_NOSTRETCHHW";
			break;

		/* DirectDrawSurface is not in 4 bit color palette and the requested operation
		   requires 4 bit color palette */
		case DDERR_NOT4BITCOLOR:
			pszError = "DDERR_NOT4BITCOLOR";
			break;

		/* DirectDrawSurface is not in 4 bit color index palette and the requested
		   operation requires 4 bit color index palette */
		case DDERR_NOT4BITCOLORINDEX:
			pszError = "DDERR_NOT4BITCOLORINDEX";
			break;

		/* DirectDraw Surface is not in 8 bit color mode and the requested operation
		   requires 8 bit color */
		case DDERR_NOT8BITCOLOR:
			pszError = "DDERR_NOT8BITCOLOR";
			break;

		/* Operation could not be carried out because there is no texture mapping
		   hardware present or available */
		case DDERR_NOTEXTUREHW:
			pszError = "DDERR_NOTEXTUREHW";
			break;

		/* Operation could not be carried out because there is no hardware support
		   for vertical blank synchronized operations */
		case DDERR_NOVSYNCHW:
			pszError = "DDERR_NOVSYNCHW";
			break;

		/* Operation could not be carried out because there is no hardware support
		   for zbuffer blting */
		case DDERR_NOZBUFFERHW:
			pszError = "DDERR_NOZBUFFERHW";
			break;

		/* Overlay surfaces could not be z layered based on their BltOrder because
		   the hardware does not support z layering of overlays */
		case DDERR_NOZOVERLAYHW:
			pszError = "DDERR_NOZOVERLAYHW";
			break;

		/* The hardware needed for the requested operation has already been
		   allocated */
		case DDERR_OUTOFCAPS:
			pszError = "DDERR_OUTOFCAPS";
			break;

		/* DirectDraw does not have enough memory to perform the operation */
		case DDERR_OUTOFMEMORY:
			pszError = "DDERR_OUTOFMEMORY";
			break;

		/* DirectDraw does not have enough memory to perform the operation */
		case DDERR_OUTOFVIDEOMEMORY:
			pszError = "DDERR_OUTOFVIDEOMEMORY";
			break;

		/* Hardware does not support clipped overlays */
		case DDERR_OVERLAYCANTCLIP:
			pszError = "DDERR_OVERLAYCANTCLIP";
			break;

		/* Can only have ony color key active at one time for overlays */
		case DDERR_OVERLAYCOLORKEYONLYONEACTIVE:
			pszError = "DDERR_OVERLAYCOLORKEYONLYONEACTIVE";
			break;

		/* Access to this palette is being refused because the palette is already
		   locked by another thread */
		case DDERR_PALETTEBUSY:
			pszError = "DDERR_PALETTEBUSY";
			break;

		/* No src color key specified for this operation */
		case DDERR_COLORKEYNOTSET:
			pszError = "DDERR_COLORKEYNOTSET";
			break;

		/* This surface is already attached to the surface it is being attached to */
		case DDERR_SURFACEALREADYATTACHED:
			pszError = "DDERR_SURFACEALREADYATTACHED";
			break;

		/* This surface is already a dependency of the surface it is being made a
		   dependency of */
		case DDERR_SURFACEALREADYDEPENDENT:
			pszError = "DDERR_SURFACEALREADYDEPENDENT";
			break;

		/* Access to this surface is being refused because the surface is already
		   locked by another thread */
		case DDERR_SURFACEBUSY:
			pszError = "DDERR_SURFACEBUSY";
			break;

		/* Access to this surface is being refused because no driver exists
		   which can supply a pointer to the surface.
		   This is most likely to happen when attempting to lock the primary
		   surface when no DCI provider is present.
		   Will also happen on attempts to lock an optimized surface. */
		case DDERR_CANTLOCKSURFACE:
			pszError = "DDERR_CANTLOCKSURFACE";
			break;

		/* Access to Surface refused because Surface is obscured */
		case DDERR_SURFACEISOBSCURED:
			pszError = "DDERR_SURFACEISOBSCURED";
			break;

		/* Access to this surface is being refused because the surface is gone.
		   The DIRECTDRAWSURFACE object representing this surface should
		   have Restore called on it. */
		case DDERR_SURFACELOST:
			pszError = "DDERR_SURFACELOST";
			break;

		/* The requested surface is not attached */
		case DDERR_SURFACENOTATTACHED:
			pszError = "DDERR_SURFACENOTATTACHED";
			break;

		/* Height requested by DirectDraw is too large */
		case DDERR_TOOBIGHEIGHT:
			pszError = "DDERR_TOOBIGHEIGHT";
			break;

		/* Size requested by DirectDraw is too large. The individual height and
		   width are OK */
		case DDERR_TOOBIGSIZE:
			pszError = "DDERR_TOOBIGSIZE";
			break;

		/* Width requested by DirectDraw is too large */
		case DDERR_TOOBIGWIDTH:
			pszError = "DDERR_TOOBIGWIDTH";
			break;

		/* Action not supported */
		case DDERR_UNSUPPORTED:
			pszError = "DDERR_UNSUPPORTED";
			break;

		/* FOURCC format requested is unsupported by DirectDraw */
		case DDERR_UNSUPPORTEDFORMAT:
			pszError = "DDERR_UNSUPPORTEDFORMAT";
			break;

		/* Bitmask in the pixel format requested is unsupported by DirectDraw */
		case DDERR_UNSUPPORTEDMASK:
			pszError = "DDERR_UNSUPPORTEDMASK";
			break;

		/* Vertical blank is in progress */
		case DDERR_VERTICALBLANKINPROGRESS:
			pszError = "DDERR_VERTICALBLANKINPROGRESS";
			break;

		/* Informs DirectDraw that the previous Blt which is transfering information
		   to or from this Surface is incomplete */
		case DDERR_WASSTILLDRAWING:
			pszError = "DDERR_WASSTILLDRAWING";
			break;

		/* Rectangle provided was not horizontally aligned on reqd. boundary */
		case DDERR_XALIGN:
			pszError = "DDERR_XALIGN";
			break;

		/* The GUID passed to DirectDrawCreate is not a valid DirectDraw driver
		   identifier */
		case DDERR_INVALIDDIRECTDRAWGUID:
			pszError = "DDERR_INVALIDDIRECTDRAWGUID";
			break;

		/* A DirectDraw object representing this driver has already been created
		   for this process */
		case DDERR_DIRECTDRAWALREADYCREATED:
			pszError = "DDERR_DIRECTDRAWALREADYCREATED";
			break;

		/* A hardware only DirectDraw object creation was attempted but the driver
		   did not support any hardware */
		case DDERR_NODIRECTDRAWHW:
			pszError = "DDERR_NODIRECTDRAWHW";
			break;

		/* this process already has created a primary surface */
		case DDERR_PRIMARYSURFACEALREADYEXISTS:
			pszError = "DDERR_PRIMARYSURFACEALREADYEXISTS";
			break;

		/* software emulation not available */
		case DDERR_NOEMULATION:
			pszError = "DDERR_NOEMULATION";
			break;

		/* Region passed to Clipper::GetClipList is too small */
		case DDERR_REGIONTOOSMALL:
			pszError = "DDERR_REGIONTOOSMALL";
			break;

		/* An attempt was made to set a clip list for a clipper objec that
		   is already monitoring an hwnd */
		case DDERR_CLIPPERISUSINGHWND:
			pszError = "DDERR_CLIPPERISUSINGHWND";
			break;

		/* No clipper object attached to surface object */
		case DDERR_NOCLIPPERATTACHED:
			pszError = "DDERR_NOCLIPPERATTACHED";
			break;

		/* Clipper notification requires an HWND or
		   no HWND has previously been set as the CooperativeLevel HWND */
		case DDERR_NOHWND:
			pszError = "DDERR_NOHWND";
			break;

		/* HWND used by DirectDraw CooperativeLevel has been subclassed,
		   this prevents DirectDraw from restoring state */
		case DDERR_HWNDSUBCLASSED:
			pszError = "DDERR_HWNDSUBCLASSED";
			break;

		/* The CooperativeLevel HWND has already been set.
		   It can not be reset while the process has surfaces or palettes created */
		case DDERR_HWNDALREADYSET:
			pszError = "DDERR_HWNDALREADYSET";
			break;

		/* No palette object attached to this surface */
		case DDERR_NOPALETTEATTACHED:
			pszError = "DDERR_NOPALETTEATTACHED";
			break;

		/* No hardware support for 16 or 256 color palettes */
		case DDERR_NOPALETTEHW:
			pszError = "DDERR_NOPALETTEHW";
			break;

		/* If a clipper object is attached to the source surface passed into a
		   BltFast call */
		case DDERR_BLTFASTCANTCLIP:
			pszError = "DDERR_BLTFASTCANTCLIP";
			break;

		/* No blter */
		case DDERR_NOBLTHW:
			pszError = "DDERR_NOBLTHW";
			break;

		/* No DirectDraw ROP hardware */
		case DDERR_NODDROPSHW:
			pszError = "DDERR_NODDROPSHW";
			break;

		/* Returned when GetOverlayPosition is called on a hidden overlay */
		case DDERR_OVERLAYNOTVISIBLE:
			pszError = "DDERR_OVERLAYNOTVISIBLE";
			break;

		/* Returned when GetOverlayPosition is called on a overlay that UpdateOverlay
		   has never been called on to establish a destionation */
		case DDERR_NOOVERLAYDEST:
			pszError = "DDERR_NOOVERLAYDEST";
			break;

		/* returned when the position of the overlay on the destionation is no longer
		   legal for that destionation */
		case DDERR_INVALIDPOSITION:
			pszError = "DDERR_INVALIDPOSITION";
			break;

		/* Returned when an overlay member is called for a non-overlay surface */
		case DDERR_NOTAOVERLAYSURFACE:
			pszError = "DDERR_NOTAOVERLAYSURFACE";
			break;

		/* An attempt was made to set the cooperative level when it was already
		   set to exclusive */
		case DDERR_EXCLUSIVEMODEALREADYSET:
			pszError = "DDERR_EXCLUSIVEMODEALREADYSET";
			break;

		/* An attempt has been made to flip a surface that is not flippable */
		case DDERR_NOTFLIPPABLE:
			pszError = "DDERR_NOTFLIPPABLE";
			break;

		/* Can't duplicate primary & 3D surfaces, or surfaces that are implicitly
		   created */
		case DDERR_CANTDUPLICATE:
			pszError = "DDERR_CANTDUPLICATE";
			break;

		/* Surface was not locked.  An attempt to unlock a surface that was not
		   locked at all, or by this process, has been attempted */
		case DDERR_NOTLOCKED:
			pszError = "DDERR_NOTLOCKED";
			break;

		/* Windows can not create any more DCs */
		case DDERR_CANTCREATEDC:
			pszError = "DDERR_CANTCREATEDC";
			break;

		/* No DC was ever created for this surface */
		case DDERR_NODC:
			pszError = "DDERR_NODC";
			break;

		/* This surface can not be restored because it was created in a different
		   mode */
		case DDERR_WRONGMODE:
			pszError = "DDERR_WRONGMODE";
			break;

		/* This surface can not be restored because it is an implicitly created
		   surface */
		case DDERR_IMPLICITLYCREATED:
			pszError = "DDERR_IMPLICITLYCREATED";
			break;

		/* The surface being used is not a palette-based surface */
		case DDERR_NOTPALETTIZED:
			pszError = "DDERR_NOTPALETTIZED";
			break;

		/* The display is currently in an unsupported mode */
		case DDERR_UNSUPPORTEDMODE:
			pszError = "DDERR_UNSUPPORTEDMODE";
			break;

		/* Operation could not be carried out because there is no mip-map
		   texture mapping hardware present or available */
		case DDERR_NOMIPMAPHW:
			pszError = "DDERR_NOMIPMAPHW";
			break;

		/* The requested action could not be performed because the surface was of
		   the wrong type */
		case DDERR_INVALIDSURFACETYPE:
			pszError = "DDERR_INVALIDSURFACETYPE";
			break;

		/* Device does not support optimized surfaces, therefore no video memory optimized surfaces */
		case DDERR_NOOPTIMIZEHW:
			pszError = "DDERR_NOOPTIMIZEHW";
			break;

		/* Surface is an optimized surface, but has not yet been allocated any memory */
		case DDERR_NOTLOADED:
			pszError = "DDERR_NOTLOADED";
			break;

		/* A DC has already been returned for this surface. Only one DC can be
		   retrieved per surface */
		case DDERR_DCALREADYCREATED:
			pszError = "DDERR_DCALREADYCREATED";
			break;

		/* An attempt was made to allocate non-local video memory from a device
		   that does not support non-local video memory */
		case DDERR_NONONLOCALVIDMEM:
			pszError = "DDERR_NONONLOCALVIDMEM";
			break;

		/* The attempt to page lock a surface failed */
		case DDERR_CANTPAGELOCK:
			pszError = "DDERR_CANTPAGELOCK";
			break;

		/* The attempt to page unlock a surface failed */
		case DDERR_CANTPAGEUNLOCK:
			pszError = "DDERR_CANTPAGEUNLOCK";
			break;

		/* An attempt was made to page unlock a surface with no outstanding page locks */
		case DDERR_NOTPAGELOCKED:
			pszError = "DDERR_NOTPAGELOCKED";
			break;

		/* There is more data available than the specified buffer size could hold */
		case DDERR_MOREDATA:
			pszError = "DDERR_MOREDATA";
			break;

		/* The video port is not active */
		case DDERR_VIDEONOTACTIVE:
			pszError = "DDERR_VIDEONOTACTIVE";
			break;

		/* Surfaces created by one direct draw device cannot be used directly by
		   another direct draw device */
		case DDERR_DEVICEDOESNTOWNSURFACE:
			pszError = "DDERR_DEVICEDOESNTOWNSURFACE";
			break;

		/* An attempt was made to invoke an interface member of a DirectDraw object
		   created by CoCreateInstance() before it was initialized */
		case DDERR_NOTINITIALIZED:
			pszError = "DDERR_NOTINITIALIZED";
			break;

		/* Unknown DD/App Error */
		default:
			sprintf( szMsg, "Error #%ld", (DWORD)hResult );
			pszError = szMsg;
			break;
	}

	/* Copy DD Error string to buff */
	dwLen = strlen( pszError );
	if( dwLen >= dwError )
	{
		dwLen = dwError - 1;
	}
	if( dwLen )
	{
		_strncpy( pszErrorBuff, pszError, dwLen );
		pszErrorBuff[ dwLen ] = '\0';
	}
	return TRUE;

} /* #OF# DD_GetErrorString */
