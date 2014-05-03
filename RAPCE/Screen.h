/******************************************************************************
	[Screen.h]
		画面出力系のインタフェイスを定義します。

		Define screen interface.

	Copyright (C) 2004 Ki

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
******************************************************************************/
#ifndef SCREEN_H_INCLUDED
#define SCREEN_H_INCLUDED

#include "TypeDefs.h"

#define SCREEN_FDEFAULT					0
#define SCREEN_FFULLSCREEN				1
#define SCREEN_FHARDWAREACCELERATION	2

//Kitao追加
void
SCREEN_SaveWindowPosition();

//Kitao追加
void
SCREEN_LoadWindowPosition();

//Kitao更新。SCREEN_ChangeMode()から名前を変更した。v2.28
BOOL
SCREEN_Init(
	Sint32		width,
	Sint32		height,
	Sint32		magnification, //Kitao追加
	Uint32		bitsPerPixel,
	Uint32		flags);

void
SCREEN_Deinit();

BOOL
SCREEN_ToggleFullScreen();

//Kitao更新
BOOL
SCREEN_WaitVBlank(
	BOOL	bDraw);

void*
SCREEN_GetBuffer();

const Sint32
SCREEN_GetBufferPitch();

void
SCREEN_FillRect(
	Sint32		x,
	Sint32		y,
	Sint32		width,
	Sint32		height,
	Uint32		color);

//Kitao追加
void
SCREEN_Clear();

//Kitao更新
void
SCREEN_Blt(
	Uint32*		pSrc,
	Sint32		srcX,
	Sint32		srcY,
	Uint16*		pSrcW,
	Sint32		srcH,
	Sint32		executeCode);

//Kitao追加
void
SCREEN_SetSyncTo60HzScreen(
	BOOL	bSyncTo60HzScreen);

//Kitao追加
BOOL
SCREEN_GetSyncTo60HzScreen();

//Kitao追加
void
SCREEN_SetMessageText(
	char*	pText);

//Kitao追加
void
SCREEN_SetGamma(
	Sint32	scanLineType,
	Sint32	scanLineDensity,
	BOOL	bTvMode);

//Kitao追加
DWORD
SCREEN_GetLastTimeSyncTime();

//Kitao追加
void
SCREEN_WriteScreenshot(
	FILE*	fp);

//Kitao追加
void
SCREEN_SetMagnification(
	Sint32	magnification);

//Kitao追加
Sint32
SCREEN_GetMagnification();

//Kitao追加
void
SCREEN_SetPixelMagnification(
	Sint32*	wMag,
	Sint32*	hMag);

//Kitao追加
Sint32
SCREEN_GetVerticalScanFrequency();

#endif // SCREEN_H_INCLUDED
