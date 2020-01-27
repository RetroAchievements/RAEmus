/****************************************************************************
File    : directdraw.h
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Interface of DirectDraw wrappers
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 28.10.2003
*/

#ifndef __DIRECTDRAW_H__
#define __DIRECTDRAW_H__

#include <ddraw.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Constants declarations */

#define MODE_1024_768				0x0100	/* States for the available mode test */
#define MODE_800_600				0x0080
#define MODE_640_480				0x0040
#define MODE_640_400				0x0020
#define MODE_512_384				0x0010
#define MODE_400_300				0x0008
#define MODE_320_400				0x0004
#define MODE_320_240				0x0002
#define MODE_320_200				0x0001

#define DD_CAPS_VBI					0x0001
#define DD_CAPS_FLIPINTERVAL		0x0002
#define DD_CAPS_FLIPNOVSYNC			0x0004

#define MAX_REFRESH_RATE_NO			128

typedef struct _DD_SurfaceParams_t
{
	DWORD         dwHeight;
	DWORD         dwWidth;
	DWORD         dwFlags;
	DWORD         dwCaps;
	DWORD         dwBackBufferCount;
	DDPIXELFORMAT ddpfFormat;
} DD_SurfaceParams_t;

typedef enum {
	stNone = 0, stPrimary = 1, stOffscrn = 2, stBackBuf = 3
} eSurfaceType;

/* Exported methods */

HRESULT DD_ObjectCreate       ( void );
void    DD_ObjectRelease      ( void );
HRESULT DD_EnumDisplayModes   ( ULONG *pScreenModes );
HRESULT DD_SetCooperativeLevel( HWND hWnd, DWORD dwFlags );
HRESULT DD_SetDisplayMode     ( DWORD dwWidth, DWORD dwHeight, DWORD dwBPP, DWORD dwRefreshRate );
HRESULT DD_RestoreDisplayMode ( void );
HRESULT DD_FlipToGDISurface   ( void );
HRESULT DD_GetMonitorFrequency( ULONG *pRefreshRate );
HRESULT DD_GetVideoCaps       ( ULONG *pVideoCaps );
HRESULT DD_WaitForVBlank      ( DWORD dwFlags );

HRESULT DD_SurfaceCreate      ( DD_SurfaceParams_t *pParams, eSurfaceType stSurface );
ULONG   DD_SurfaceRelease     ( eSurfaceType stSurface );
HRESULT DD_SurfaceIsLost      ( eSurfaceType stSurface );
HRESULT DD_SurfaceRestore     ( eSurfaceType stSurface );
HRESULT DD_SurfaceLock        ( eSurfaceType stSurface, DWORD dwFlags, LPVOID *ppBuffer, DWORD *pPitch );
HRESULT DD_SurfaceUnlock      ( eSurfaceType stSurface );
HRESULT DD_SurfaceGetDC       ( HDC *pDC );
HRESULT DD_SurfaceReleaseDC   ( HDC hDC );
HRESULT DD_SurfaceBlit        ( eSurfaceType stDestSurface, RECT *pDestRect, eSurfaceType stSrcSurface, RECT *pSrcRect, DWORD dwFlags, LPDDBLTFX pDDBltFx );
HRESULT DD_SurfaceBlitFast    ( eSurfaceType stDestSurface, eSurfaceType stSrcSurface, DWORD dwTrans );
HRESULT DD_SurfaceFlip        ( DWORD dwFlags );
HRESULT DD_SurfaceGetAttached ( DWORD dwCaps );
HRESULT DD_SurfaceSetClipper  ( eSurfaceType stSurface, BOOL bEnable );
HRESULT DD_SurfaceSetPalette  ( eSurfaceType stSurface, BOOL bEnable );

HRESULT DD_ClipperCreate      ( DWORD dwFlags );
ULONG   DD_ClipperRelease     ( void );
HRESULT DD_ClipperSetHWnd     ( DWORD dwFlags, HWND hWnd );

HRESULT DD_PaletteCreate      ( DWORD dwFlags, PALETTEENTRY *pEntries );
ULONG   DD_PaletteRelease     ( void );
HRESULT DD_PaletteSetEntries  ( DWORD dwFlags, DWORD dwStartingEntry, DWORD dwCount, PALETTEENTRY *pEntries );

int     DD_IsObjectCreated    ( void );
BOOL    DD_IsSurfaceCreated   ( eSurfaceType stSurface );

void    DD_GetRefreshRates    ( DWORD dwScreenMode, int *pRefreshRates, int nBufferLen );
BOOL    DD_GetErrorString     ( HRESULT hResult, LPSTR pszErrorBuff, DWORD dwError );

#ifdef __cplusplus
}
#endif

#endif /*__DIRECTDRAW_H__*/
