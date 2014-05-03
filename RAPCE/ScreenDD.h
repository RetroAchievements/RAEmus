/******************************************************************************
	[ScreenDD.h]

		Define DirectDraw screen interface.

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
#ifndef SCREENDD_H_INCLUDED
#define SCREENDD_H_INCLUDED

#include "TypeDefs.h"


BOOL
SCREENDD_Init(
	Sint32		width,
	Sint32		height,
	Sint32		magnification, //Kitao追加
	Sint32		bitsPerPixel,
	Uint32		flags); //Kitao追加

BOOL
SCREENDD_ChangeMode(
	Sint32		width,
	Sint32		height,
	Sint32		magnification, //Kitao追加
	Sint32		bitsPerPixel,
	Uint32		flags);

//Kitao更新
BOOL
SCREENDD_WaitVBlank(
	BOOL	bDraw);

void*
SCREENDD_GetBuffer();

const Sint32
SCREENDD_GetBufferPitch();

void
SCREENDD_FillRect(
	Sint32		x,
	Sint32		y,
	Sint32		width,
	Sint32		height,
	Uint32		color);

//Kitao追加
void
SCREENDD_Clear();

//Kitao更新
void
SCREENDD_Blt(
	Uint32*		pSrc,
	Sint32		srcX,
	Sint32		srcY,
	Uint16*		pTvW,
	Sint32		srcH,
	Sint32		executeCode);

void
SCREENDD_Deinit();

//Kitao追加
void
SCREENDD_SetSyncTo60HzScreen(
	BOOL	bSyncTo60HzScreen);

//Kitao追加
BOOL
SCREENDD_GetSyncTo60HzScreen();

//Kitao追加
void
SCREENDD_SetShowFPS(
	Sint32	showFPS);

//Kitao追加
Sint32
SCREENDD_GetShowFPS();

//Kitao追加
void
SCREENDD_SetMessageText(
	char*	pText);

//Kitao追加
DWORD
SCREENDD_GetLastTimeSyncTime();

//Kitao追加
void
SCREENDD_WriteScreenshot(
	FILE*	fp);

//Kitao追加
void
SCREENDD_SetMagnification(
	Sint32	magnification);

//Kitao追加
Sint32
SCREENDD_GetMagnification();

//Kitao追加
Sint32
SCREENDD_GetBitsPerPixel();


#endif // SCREENDD_H_INCLUDED
