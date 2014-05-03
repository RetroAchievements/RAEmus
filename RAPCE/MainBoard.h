/*******************************************************************************
	[MainBoard.h]

	Copyright (C) 2004 Ki

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*******************************************************************************/
#ifndef MAIN_BOARD_H
#define MAIN_BOARD_H

#include <stdio.h>
#include "TypeDefs.h"
#include "CPU.h"
#include "VDC.h"
#include "APU.h"
#include "PSG.h"
#include "IntCtrl.h"
#include "Timer.h"
#include "Cartridge.h"
#include "JoyPad.h"
#include "Mouse.h"
#include "MB128.h"
#include "CDROM.h"
#include "ADPCM.h"

//Kitao追加。ステートセーブのバージョン。古いものを区別して読み込めるようにするために必要。
//v0.50は1。v0.51から2に。v0.57から3に。v0.58から4に。v0.60から5に。v0.61から6に。
//v0.62から7に。v0.64から8に。v0.65から9に。v0.72から10に。v0.74から11に。v0.78から12に。
//v0.79から13に。v0.80から14に。v0.82から15に。v0.87から16に。v0.89から17に。v0.91から18に。
//v0.92から19に。v0.94から20に。v0.95から21に。v1.00から22に。v1.02から23に。v1.03から24に。
//v1.08から25に。v1.11から26に。v1.15から27に。v1.30から28に。v1.31から29に。v1.38から30に。
//v1.40から31に。v1.43から32に。(33,34はv1.61beta)。v1.61から35に。v1.63から36に。
//v1.65から37に。v2.00から40に。v2.08から41に。v2.27から42に。v2.28から43に。v2.47betaから44に。
//v2.47から45に。
#define SAVE_STATE_VERSION	45


enum RamType
{
	MAINBOARD_MAINRAM = 0,
	MAINBOARD_BUFFERRAM,
	MAINBOARD_ARCADERAM
};


BOOL
MAINBOARD_Init(
	const char*		pGameName);

void
MAINBOARD_Deinit();

void
MAINBOARD_Pause(
	BOOL	bPause);

//Kitao追加
void
MAINBOARD_PauseNoMessage(
	BOOL	bPause);

BOOL
MAINBOARD_SaveBRAM(
	const char*		pathName);

BOOL
MAINBOARD_LoadBRAM(
	const char*		pathName);


void
MAINBOARD_Reset();


void
MAINBOARD_AdvanceFrame();

Sint32
MAINBOARD_AdvanceInstruction();


//Kitao追加
void
MAINBOARD_ResetFastForwardingCount();


BOOL
MAINBOARD_ChangeScreenMode(
	Sint32		screenWidth,
	Sint32		screenHeight,
	Sint32		magnification,
	BOOL		bFullScreen,
	Uint32		bFullScreenColor); //Kitao更新。bFullScreenColor…フルスクリーン時のbitsPerPixel


BOOL
MAINBOARD_ChangeSoundMode(
	Uint32		bufferSize,
	Uint32		sampleRate,
	Uint32		masterVolume,
	BOOL		bReverb);

BOOL
MAINBOARD_SaveState(
	FILE*		p);

BOOL
MAINBOARD_LoadState(
	FILE*		p);

void
MAINBOARD_ChangeMemoryValue(
	Sint32		ramType,
	Uint32		addr,
	Uint8		data);

//Kitao追加
void
MAINBOARD_ScreenUpdate(
	BOOL	bVsync);

//Kitao追加
void
MAINBOARD_ScreenEncord();

//Kitao追加
void
MAINBOARD_ScreenUpdateFast();

//Kitao追加
void
MAINBOARD_DrawScreenshot();

//Kitao追加
void
MAINBOARD_ScreenClear();

//Kitao追加
BOOL
MAINBOARD_GetPause();

//Kitao追加
void
MAINBOARD_SetBigSoundBuffer(
	Sint32 n);

//Kitao追加
Sint32
MAINBOARD_GetMagnification();

//Kitao追加
Sint32
MAINBOARD_GetScanLine();

//Kitao追加
void
MAINBOARD_SetSuperGrafx(
	Sint32	superGrafx);

//Kitao追加
Sint32
MAINBOARD_GetSuperGrafx();

//Kitao追加
void
MAINBOARD_SetArcadeCard(
	BOOL	arcadeCard);

//Kitao追加
BOOL
MAINBOARD_GetArcadeCard();

//Kitao追加
void
MAINBOARD_SetBackupFull(
	BOOL	backupFull);

//Kitao追加
BOOL
MAINBOARD_GetBackupFull();

//Kitao追加
void
MAINBOARD_SetStretched(
	BOOL	bStretched,
	BOOL	bVStretched);

//Kitao追加
Uint32
MAINBOARD_GetStateVersion();

//Kitao追加
BOOL
MAINBOARD_LoadScreenBuf(
	Sint32	num,
	FILE*	p);

//Kitao追加
void
MAINBOARD_RestoreScreenBuf();

//Kitao追加
void
MAINBOARD_ResetFastForwardingCount();

//Kitao追加
void
MAINBOARD_SetFastForwarding(
	Sint32	fastForwarding,
	BOOL	bSoundAjust,
	BOOL	bReset);

//Kitao追加
Sint32
MAINBOARD_GetFastForwarding();

//Kitao追加
Sint32
MAINBOARD_GetFastForwardingR();

//Kitao追加
BOOL
MAINBOARD_GetSystemInit();

//Kitao追加
void
MAINBOARD_TG16BitConvert();

//Kitao追加
void
MAINBOARD_OverDumpedConvert();

//Kitao追加
void
MAINBOARD_IncSystemTime();

//Kitao追加
void
MAINBOARD_SetGradiusII();

//Kitao追加
int
MAINBOARD_GetProcessingDelay();

//Kitao追加
int
MAINBOARD_GetDisplayedFrames();

//Kitao追加
void
MAINBOARD_ResetPrevTvW();

//Kitao追加
BOOL
MAINBOARD_GetResolutionChange();

//Kitao追加
void
MAINBOARD_SetResolutionChange(
	BOOL	bResolutionChange);

//Kitao追加
Sint32
MAINBOARD_GetShowOverscanTop();

//Kitao追加
Sint32
MAINBOARD_GetShowOverscanBottom();

//Kitao追加
Sint32
MAINBOARD_GetShowOverscanLeft();

//Kitao追加
Sint32
MAINBOARD_GetShowOverscanRight();

//Kitao追加
BOOL
MAINBOARD_GetShowOverscan();

//Kitao追加
void
MAINBOARD_SetShowOverscanTop(
	Sint32	showOverscanTop);

//Kitao追加
void
MAINBOARD_SetShowOverscanBottom(
	Sint32	showOverscanBottom);

//Kitao追加
void
MAINBOARD_SetShowOverscanLeft(
	Sint32	showOverscanLeft);

//Kitao追加
void
MAINBOARD_SetShowOverscanRight(
	Sint32	showOverscanRight);

//Kitao追加
BOOL
MAINBOARD_GetFourSplitScreen();

//Kitao追加
BOOL
MAINBOARD_GetMonoColor();

//Kitao追加
Sint32
MAINBOARD_GetForceMonoColor();

//Kitao追加
void
MAINBOARD_SetForceMonoColor(
	Sint32	forceMonoColor);

//Kitao追加
Uint8*
MAINBOARD_GetpMainRam();

//##RA
Uint8*
MAINBOARD_GetpMainROM();

//##RA
Uint32
MAINBOARD_GetpMainROMSize();

//Kitao追加
void
MAINBOARD_WriteROM(
	Uint32		mpr,
	Uint32		addr,
	Uint8		data);

//Kitao追加
void
MAINBOARD_SetContinuousWriteValue(
	BOOL		bContinuous,
	Uint32		mpr,
	Uint32		addr,
	Uint8		data);

//Kitao追加
BOOL
MAINBOARD_GetSystemCard();

//Kitao追加。v2.63
Uint32
MAINBOARD_GetSystemTime();

//Kitao追加。v2.63
void
MAINBOARD_ResetSystemTime();


#endif /* MAIN_BOARD_H */
