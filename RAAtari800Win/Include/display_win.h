/****************************************************************************
File    : display_win.h
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# DisplayWin public methods and objects prototypes
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 30.10.2003
*/

#ifndef __DISPLAY_WIN_H__
#define __DISPLAY_WIN_H__

#include <vfw.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Constants declarations */

#define ATARI_HORZ_CLIP					24
#define ATARI_FULL_HORZ_CLIP			(ATARI_HORZ_CLIP * 2)
#define ATARI_VIS_WIDTH					(Screen_WIDTH - ATARI_FULL_HORZ_CLIP)
#define ATARI_DOUBLE_WIDTH				(Screen_WIDTH * 2)
#define ATARI_DOUBLE_VIS_WIDTH			(ATARI_VIS_WIDTH * 2)
#define ATARI_DOUBLE_HEIGHT				(Screen_HEIGHT * 2)
#define ATARI_TRIPLE_WIDTH				(Screen_WIDTH * 3)
#define ATARI_TRIPLE_VIS_WIDTH			(ATARI_VIS_WIDTH * 3)
#define ATARI_TRIPLE_HEIGHT				(Screen_HEIGHT * 3)

#define ATARI_SCREEN_SIZE				(Screen_WIDTH * Screen_HEIGHT)
#define ATARI_VIS_SCREEN_SIZE			(ATARI_VIS_WIDTH * Screen_HEIGHT)
#define ATARI_DOUBLE_VIS_SCREEN_SIZE	(ATARI_DOUBLE_VIS_WIDTH * ATARI_DOUBLE_HEIGHT)
#define ATARI_TRIPLE_VIS_SCREEN_SIZE	(ATARI_TRIPLE_VIS_WIDTH * ATARI_TRIPLE_HEIGHT)

#define SM_FRES_320_200				0x00000001	/* Full screen resolutions */
#define SM_FRES_320_240				0x00000002
#define SM_FRES_320_400				0x00000004
#define SM_FRES_400_300				0x00000008
#define SM_FRES_512_384				0x00000010
#define SM_FRES_640_400				0x00000020
#define SM_FRES_640_480				0x00000040
#define SM_FRES_800_600				0x00000080
#define SM_FRES_1024_768			0x00000100
#define SM_FRES_MASK				0x000001FF

#define SM_WRES_NORMAL				0x00000200	/* Windowed resolutions */
#define SM_WRES_DOUBLE				0x00000400
#define SM_WRES_MASK				0x00000600

#define SM_MODE_WIND				0x00000800	/* DirectDraw or GDI windowed mode */
#define SM_MODE_FULL				0x00001000	/* DirectDraw Full screen mode */
#define SM_MODE_MASK				0x00001800

#define SM_ATTR_SMALL_DLG			0x00002000	/* Display mode attributes */
#define SM_ATTR_NO_MENU				0x00004000
#define SM_ATTR_STRETCHED			0x00008000
#define SM_ATTR_MASK				0x0000E000

#define SM_OPTN_DDVBL_WAIT 			0x00010000	/* Display options */
#define SM_OPTN_USE_GDI				0x00020000
#define SM_OPTN_FLIP_BUFFERS		0x00040000
#define SM_OPTN_HIDE_CURSOR			0x00080000
#define SM_OPTN_USE_SYSPAL			0x00100000
#define SM_OPTN_SAFE_MODE			0x00200000
#define SM_OPTN_USE_MMX				0x00400000
#define SM_OPTN_OPTIMIZE_PRO		0x00800000
#define SM_OPTN_MASK				0x00FF0000

#define SM_DDFX_MIRRORLEFTRIGHT		0x10000000	/* FX effects for blits */
#define SM_DDFX_MIRRORUPDOWN		0x20000000
#define SM_DDFX_NOTEARING 			0x40000000
#define SM_DDFX_MASK				0x70000000

#define DEF_SCREEN_STATE			(SM_MODE_WIND | SM_FRES_640_480 | SM_WRES_NORMAL | SM_OPTN_USE_GDI | SM_OPTN_HIDE_CURSOR)

#define STRETCH_PIXELDOUBLING		0	/* Stretching modes */
#define STRETCH_INTERPOLATION		1
#define STRETCH_SCANLINES			2
#define STRETCH_HIEND				3

#define MEMORY_SYSTEM				0	/* Type of memory for offscreen surface */
#define MEMORY_AGP					1
#define MEMORY_VIDEO				2

#define DEF_SHOW_CURSOR				50
#define DEF_STRETCH_MODE			STRETCH_PIXELDOUBLING
#define DEF_MEMORY_TYPE				MEMORY_SYSTEM
#define DEF_REQUEST_RATE			0
#define DEF_CLR_BLACK_LEVEL			16
#define DEF_CLR_WHITE_LEVEL			235
#define DEF_CLR_SATURATION			50
#define DEF_CLR_CONTRAST			50
#define DEF_CLR_BRIGHTNESS			50
#define DEF_CLR_GAMMA				65

#define PAL_ENTRIES_NO				256
#define BLIT_EFFECTS_NO				3

#define AF_BITMAP					0x01 /* Flags for use with Alloc/FreeUserScreen */
#define AF_COLORS					0x02
#define AF_LOCKED					0x04

/* Exported methods */

BOOL   Screen_InitialiseDisplay ( BOOL bForceInit );
void   Screen_CheckDDrawModes ( void );
BOOL   Screen_ChangeMode      ( BOOL bForceInit );
void   Screen_Clear           ( BOOL bPermanent, BOOL bDeepClear );
UINT   Screen_AllocUserScreen ( void );
void   Screen_FreeUserScreen  ( UINT uiAlloc );
BOOL   Screen_PrepareRedraw   ( BOOL bForcePrep );
void   Screen_Redraw          ( HDC hDC );
void   Screen_FreeRedraw      ( void );
void   Screen_DrawFrozen      ( BOOL bForceMenu, BOOL bUseSysPal, BOOL bForcePrep, BOOL bRedraw );
void   Screen_DrawPaused      ( BOOL bForceMenu, BOOL bUseSysPal, BOOL bForcePrep, BOOL bRedraw );
BOOL   Screen_FlipToGDI       ( BOOL bUseSysPal );
BOOL   Screen_ToggleModes     ( void );
BOOL   Screen_ToggleWindowed  ( void );
void   Screen_SetWindowSize   ( HWND hView, UINT nFlags );
void   Screen_ComputeClipArea ( HWND hViewWnd );
void   Screen_ShowMenuBar     ( BOOL bForceShow );
BOOL   Screen_SetSafeDisplay  ( BOOL bForceGDI );
UBYTE* Screen_GetBuffer       ( void );
BOOL   Screen_UseAtariPalette ( BOOL bForceUse );
BOOL   Screen_UseSystemPalette( void );
void   Screen_PrepareInterp   ( BOOL bForcePrep );
void   Screen_FreeInterp      ( void );
void   Screen_ShowMousePointer( BOOL bShow );

/* Internal state info */

#define DIP_BITMAPINFO		0x01
#define DIP_PALENTRIES		0x02

struct ScreenInterParms_t
{
	LPBITMAPINFO pBitmapInfo;	/* Bitmap info for our screen bitmap */
	PALETTEENTRY pePalette[ PAL_ENTRIES_NO ];
	DWORD dwMask;
};

void Screen_GetInterParms( struct ScreenInterParms_t *pInfo );

/* Exported globals */

struct ScreenCtrl_t
{
	ULONG    ulState;
	ULONG    ulModesAvail;
	ULONG    ulVideoCaps;
	int      nStretchMode;
	int      nMemoryType;
	HDC      hDC;
	int      nShowCursor;
	int      nStatusSize;
	int      nCurrentRate;
	int      nRequestRate;
	BOOL     bLocked;
	PAVIFILE pfOutput;
	/* Palette */
	struct PalCtrl_t
	{
		int  nBlackLevel;
		int  nWhiteLevel;
		int  nSaturation;
		int  nContrast;
		int  nBrightness;
		int  nGamma;
		BOOL bUseSysCol;
	} Pal;
};
extern struct ScreenCtrl_t g_Screen;

struct s_Buffer_t
{
	UCHAR *pSource;		/* The kernel makes his writes into this buffer  */
	UCHAR *pTarget;		/* We make the writes here by using asm routines */
	UCHAR *pMainScr;	/* Pointer to the primary buffer   */
	UCHAR *pWorkScr;	/* Pointer to the secondary buffer */
	UCHAR *pBackScr;	/* Pointer to the backup buffer    */
};
extern struct s_Buffer_t s_Buffer;

extern void (*Atari_DisplayScreen_ptr)( void );

#ifdef __cplusplus
}
#endif

#endif /*__DISPLAY_WIN_H__*/
