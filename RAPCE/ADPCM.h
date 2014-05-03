/*-----------------------------------------------------------------------------
	[ADPCM.h]
		ＡＤＰＣＭを記述するのに必要な定義および
	関数のプロトタイプ宣言を行ないます。

	Copyright (C) 2004 Ki

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
**---------------------------------------------------------------------------*/
#ifndef ADPCM_H_INCLUDED
#define ADPCM_H_INCLUDED

#include <stdio.h>
#include "TypeDefs.h"

#define ADPCM_STATE_NORMAL			0
#define ADPCM_STATE_HALF_PLAYED		1
#define ADPCM_STATE_FULL_PLAYED		2
#define ADPCM_STATE_STOPPED			4


/*-----------------------------------------------------------------------------
	関数のプロトタイプ宣言を行ないます。
-----------------------------------------------------------------------------*/
BOOL
ADPCM_Init();

void
ADPCM_Deinit();

void
ADPCM_SetNotificationFunction(
	void	(*pfnNotification)(Uint32));

void
ADPCM_Reset();

void
ADPCM_SetAddrLo(Uint8	addrLo);

void
ADPCM_SetAddrHi(Uint8	addrHi);

void
ADPCM_SetReadAddr();

//Kitao更新
void
ADPCM_SetWriteAddr(
	Uint8	writeBuffer);

void
ADPCM_SetLength();

Uint8
ADPCM_ReadBuffer();

void
ADPCM_WriteBuffer(
	Uint8	data);

void
ADPCM_Play(
	BOOL	bPlay);

void
ADPCM_Repeat(
	BOOL	bRepeat);

//Kitao追加
void
ADPCM_Pause(
	BOOL	bPause);

void
ADPCM_Mix(
	Sint16*		pDst,				// 出力先バッファ 
	Sint32		nSample);			// 書き出すサンプル数 

//Kitao更新
void
ADPCM_SetFreq(
	Uint8		data);

BOOL
ADPCM_IsPlaying();

//Kitao追加。v1.40
BOOL
ADPCM_GetRepeat();

void
ADPCM_SetVolume(Uint32 volume);	/* 0 - 65535 */

void
ADPCM_FadeOut(Sint32 ms);

void
ADPCM_FadeIn(Sint32 ms);

void
ADPCM_AdvanceClock();

//Kitao更新
void
ADPCM_AdvanceFadeClock();

BOOL
ADPCM_SaveState(
	FILE*		p);

BOOL
ADPCM_LoadState(
	FILE*		p);

//Kitao追加
void
ADPCM_SetVolumeEffect(
	Uint32 volumeEffect);

//Kitao追加
void
ADPCM_ClearCDDAAjustCount();


#endif		/* ADPCM_H_INCLUDED */
