/*-----------------------------------------------------------------------------
	[CDROM.h]
		CD-ROMドライブを記述するのに必要な定義および
	関数のプロトタイプ宣言を行ないます．

	Copyright (C) 2004 Ki

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
**---------------------------------------------------------------------------*/
#ifndef CD_ROM_H_INCLUDED
#define CD_ROM_H_INCLUDED

#include "TypeDefs.h"

#define	CDROM_IRQ2		1


/*-----------------------------------------------------------------------------
** 関数のプロトタイプ宣言を行ないます．
**---------------------------------------------------------------------------*/
Sint32
CDROM_Init();

void
CDROM_Deinit();

BOOL
CDROM_Reset();

Uint8
CDROM_Read(
	Uint32 regNum);

void
CDROM_Write(
	Uint32	regNum,
	Uint8	data);

BOOL
CDROM_IsCDROMEnabled();

BOOL
CDROM_IsBRAMEnabled();

void
CDROM_AdvanceClock();

void
CDROM_AdvanceFadeClock();

BOOL
CDROM_SaveState(
	FILE*	p);

BOOL
CDROM_LoadState(
	FILE*	p);

//Kitao追加
void
CDROM_Stop(
	BOOL	bClearParameter);

//Kitao追加
BOOL
CDROM_CheckPaused();

//Kitao追加
BOOL
CDROM_CheckPlaying();

//Kitao追加
BOOL
CDROM_CheckInterrupt();

//Kitao追加
void
CDROM_Mix(
	Sint16*		pDst,
	Sint32		nSample);

//Kitao追加
void
CDROM_SetCdVolume(
	Sint32		volume);

//Kitao追加
void
CDROM_SetFastCD(
	BOOL fastCD);

//Kitao追加
BOOL
CDROM_GetFastCD();

//Kitao追加
void
CDROM_SetFastSeek(
	BOOL fastSeek);

//Kitao追加
BOOL
CDROM_GetFastSeek();

//Kitao追加
void
CDROM_SetVolumeEffect(
	Uint32 effectVolume);

//Kitao追加
void
CDROM_SetCDDAReadBufferSize();

//Kitao追加
void
CDROM_ResetCDAccessCount(
	BOOL	bReset);

//Kitao追加
void
CDROM_LoadPlayAudioTrack();

//Kitao追加
Sint32
CDROM_GetCDDAReadBufferSize();

//Kitao追加
Sint32
CDROM_GetReadByteCount();

//Kitao追加
Uint8
CDROM_Read1801();

//Kitao追加
void
CDROM_NoResetCDChange(
	int	dn);

//Kitao追加
void
CDROM_SetInstallFileName(
	int		track,
	char*	pFileName);

//Kitao追加
void
CDROM_SetInstallWavFileName(
	int		track,
	char*	pFileName);

//Kitao追加
void
CDROM_SetInstallCueFileName(
	char*	pFileName);

//Kitao追加
BOOL
CDROM_CDInstall(
	HWND	hWnd,
	BOOL	bFull);

//Kitao追加
void
CDROM_CDUninstall();

//Kitao追加
Sint32
CDROM_CheckCDInstall();

//Kitao追加
void
CDROM_SetInstallRequiredHDD(
	BOOL	bFull);

//Kitao追加
Sint32
CDROM_GetInstallRequiredHDD();

//Kitao追加
void
CDROM_SetPort(
	Uint32		n,
	Uint8		data);

//Kitao追加
Uint8
CDROM_GetPort(
	Uint32		n);

//Kitao追加
Sint32
CDROM_GetCDInstall();

//Kitao追加
void
CDROM_ClearCDDAAjustCount();

//Kitao追加
void
CDROM_UpdateCDInstall();


#endif		/* CD_ROM_H_INCLUDED */
