/*-----------------------------------------------------------------------------
	[VDC.h]

	Copyright (C) 2004 Ki

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
-----------------------------------------------------------------------------*/
#ifndef VDC_H_INCLUDED
#define VDC_H_INCLUDED

#include <stdio.h>
#include "TypeDefs.h"


#define VDC_CYCLESPERLINE	453	//453.68124207858048162230671736375 Kitao更新。v0.81
#define MAX_SCANLINE		263	//v0.81更新


/*
** AR の値によってアクセスされるレジスタ
*/
#define		VDC_MAWR			0x00
#define		VDC_MARR			0x01
#define		VDC_VWR				0x02
#define		VDC_VRR				0x02
#define		VDC_CR				0x05
#define		VDC_RCR				0x06
#define		VDC_BXR				0x07
#define		VDC_BYR				0x08
#define		VDC_MWR				0x09
#define		VDC_HSR				0x0A
#define		VDC_HDR				0x0B
#define		VDC_VPR				0x0C
#define		VDC_VDW				0x0D
#define		VDC_VCR				0x0E
#define		VDC_DCR				0x0F
#define		VDC_SOUR			0x10
#define		VDC_DESR			0x11
#define		VDC_LENR			0x12
#define		VDC_SATB			0x13


/*
** VDC STATUS
*/
#define		VDC_STAT_CR				0x01  // sprite collision
#define		VDC_STAT_OR				0x02  // over detection
#define		VDC_STAT_RR				0x04  // raster detection
#define		VDC_STAT_DS				0x08  // DMA (VRAM-SATB) end
#define		VDC_STAT_DV				0x10  // DMA (VRAM-VRAM) end
#define		VDC_STAT_VD				0x20  // vertical retrace period
#define		VDC_STAT_BSY			0x40  // VRAM reading / writing

/*
** VDC CONTROL (R05)
*/
#define		VDC_CTRL_CC				0x01	// enable IRQ on sprite collision
#define		VDC_CTRL_OC				0x02	// enable IRQ on "over"
#define		VDC_CTRL_RC				0x04	// enable IRQ1 on raster detection
#define		VDC_CTRL_VC				0x08	// enable IRQ1 on v-retrace period

/*
** DMA CONTROL ($0F)
*/
#define		VDC_DMACTRL_CR			0x01	// request IRQ1 after VRAM->SATB DMA
#define		VDC_DMACTRL_OR			0x02	// request IRQ1 after VRAM->VRAM DMA


/*-----------------------------------------------------------------------------
	Define Function Prototypes
-----------------------------------------------------------------------------*/
void //Kitao更新。戻り値は不要にした。
VDC_Init();

void //Kitao追加
VDC_Reset();

Uint8
VDC_Read(Uint32 regNum);

void
VDC_Write(
	Uint32	regNum,
	Uint8	data);

void //Kitao更新。戻り値は不要にした。
VDC_AdvanceClock(
	Sint32	clock);

//Kitao更新
void
VDC_AdvanceLine(
	Uint32*		pScreenBuf,
	Sint32		drawFrame);

const Sint32
VDC_GetDisplayHeight();

//Kitao追加
Sint32
VDC_GetTvStartLine();

Sint32
VDC_GetScanLine();

//Kitao追加
BOOL
VDC_CheckVBlankStart();

//Kitao追加
void
VDC_SetScreenWidth(
	Sint32	screenW);

//Kitao更新
const Sint32
VDC_GetScreenWidth();

//Kitao追加。v2.14
Sint32
VDC_GetHDR();

BOOL
VDC_SaveState(
	FILE*	p);

BOOL
VDC_LoadState(
	FILE*	p);

//Kitao追加
void
VDC_SetRasterTiming(
	Sint32	n);

//Kitao追加
Sint32
VDC_GetRasterTimingType();

//Kitao追加
void
VDC_SetAutoRasterTiming(
	Sint32	n);

//Kitao追加
Sint32
VDC_GetAutoRasterTimingType();

//Kitao追加
void
VDC_SetOverClock(
	Sint32	n);

//Kitao追加
Sint32
VDC_GetOverClockType();

//Kitao追加
void
VDC_SetAutoOverClock(
	Sint32	n);

//Kitao追加
Sint32
VDC_GetAutoOverClock();

//Kitao追加
void
VDC_SetWaitPatch(
	Sint32	cycle);

//Kitao追加
void
VDC_SetForceRaster(
	BOOL	forceRaster);

void
VDC_SetForceVBlank(
	BOOL	forceVBlank);

//Kitao追加
void
VDC_SetPerformSpriteLimit(
	BOOL	bPerform);

//Kitao追加
void
VDC_SetAutoPerformSpriteLimit(
	BOOL	bPerform);

//Kitao追加
BOOL
VDC_GetPerformSpriteLimit();

//Kitao追加
void
VDC_SetLayer();

//Kitao追加
void
VDC_SetIRQ1CancelExecute();

//Kitao追加
Uint8
VDC_GetVdcStatus();

//Kitao追加
void
VDC_SetSuperGrafx(
	Sint32	superGrafx);

//Kitao追加
Sint32
VDC_GetSuperGrafx();

//Kitao追加
void
VDC_SetTvStartLine();

//Kitao追加
void
VDC_SetVblankLine();

//Kitao追加
void
VDC_SetVpcPriority();

//Kitao追加
void
VDC_SetTvStartPos();

//Kitao追加
BOOL
VDC_GetOverClockNow();

//Kitao追加
void
VDC_SetShinMegamiTensei(
	BOOL	shinMegamiTensei);

//Kitao追加
Sint32
VDC_GetShinMegamiTensei();

//Kitao追加
void
VDC_SetWorldStadium91(
	BOOL	worldStadium91);


//Kitao更新。v1.11からVCE.c をここへ統合した。
void
VDC_VceWrite(
	Uint32	regNum,
	Uint8	data);

Uint8
VDC_VceRead(Uint32 regNum);

//Kitao追加
Sint32
VDC_GetTvWidth();

//Kitao追加
Uint8
VDC_GetDCC();

//Kitao追加
Uint32
VDC_GetSpColorZero();


#endif		/* VDC_H_INCLUDED */
