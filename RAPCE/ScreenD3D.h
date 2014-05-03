/******************************************************************************
	[ScreenD3D.h]

		Define Direct3D screen interface.

******************************************************************************/
#ifndef SCREEND3D_H_INCLUDED
#define SCREEND3D_H_INCLUDED

#include "TypeDefs.h"


BOOL
SCREEND3D_Init(
	Sint32		width,
	Sint32		height,
	Sint32		magnification, //Kitao追加
	Uint32		flags); //Kitao追加

BOOL
SCREEND3D_ChangeMode(
	Sint32		width,
	Sint32		height,
	Sint32		magnification, //Kitao追加
	Uint32		flags);

//Kitao更新
BOOL
SCREEND3D_WaitVBlank(
	BOOL	bDraw);

void
SCREEND3D_PutPixel(
	Sint32		x,
	Sint32		y,
	Uint32		pixel);

void*
SCREEND3D_GetBuffer();

const Sint32
SCREEND3D_GetBufferPitch();

void
SCREEND3D_FillRect(
	Sint32		x,
	Sint32		y,
	Sint32		width,
	Sint32		height,
	Uint32		color);

//Kitao追加
void
SCREEND3D_Clear();

//Kitao更新
void
SCREEND3D_Blt(
	Uint32*		pSrc,
	Sint32		srcX,
	Sint32		srcY,
	Uint16*		pTvW,
	Sint32		srcH,
	Sint32		executeCode);

void
SCREEND3D_Deinit();

//Kitao追加
void
SCREEND3D_SetSyncTo60HzScreen(
	BOOL	bSyncTo60HzScreen);

//Kitao追加
BOOL
SCREEND3D_GetSyncTo60HzScreen();

//Kitao追加。v2.65
void
SCREEND3D_UpdateWindowsAutoVSync();

//Kitao追加
void
SCREEND3D_SetMessageText(
	char*	pText);

//Kitao追加
DWORD
SCREEND3D_GetLastTimeSyncTime();

//Kitao追加
void
SCREEND3D_WriteScreenshot(
	FILE*	fp);

//Kitao追加
void
SCREEND3D_SetMagnification(
	Sint32	magnification);

//Kitao追加
Sint32
SCREEND3D_GetMagnification();

//Kitao追加
BOOL
SCREEND3D_GetOldVideoChip();

//Kitao追加
void
SCREEND3D_SetWindowsAero(
	BOOL	bWindowsAero);


#endif // SCREEND3D_H_INCLUDED
