/*-----------------------------------------------------------------------------
	[PSG.h]
		ＰＳＧを記述するのに必要な定義および関数のプロトタイプ宣言を行ないます．

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
#ifndef PSG_H_INCLUDED
#define PSG_H_INCLUDED

#include <stdio.h>
#include "TypeDefs.h"

#define PSG_FRQ		3579545.0


/*-----------------------------------------------------------------------------
** 関数のプロトタイプ宣言を行ないます．
**---------------------------------------------------------------------------*/
Sint32
PSG_Init(
	Sint32		sampleRate);

void
PSG_Deinit();

void
PSG_Mix(
	Sint16*		pDst,				// 出力先バッファ 
	Sint32		nSample);			// 書き出すサンプル数 

void
PSG_SetSampleRate(
	Uint32		sampleRate);

void
PSGDEBUG_ShowRegs();

Uint8
PSG_Read(Uint32 regNum);

void
PSG_Write(
	Uint32		regNum,
	Uint8		data);

Sint32
PSG_AdvanceClock(Sint32 clock);

BOOL
PSG_SaveState(
	FILE*		p);

BOOL
PSG_LoadState(
	FILE*		p);

//Kitao追加。PSGのボリュームも個別に設定可能にした。
void
PSG_SetVolume(
	Uint32		volume);		// 0 - 65535

//Kitao追加。ボリュームミュート、ハーフなどをできるようにした。
void
PSG_SetVolumeEffect(
	Uint32 volumeEffect);

//Kitao追加
void
PSG_ResetVolumeReg();

//Kitao追加
void
PSG_SetMutePsgChannel(
	Sint32	num,
	BOOL	bMute);

//Kitao追加
BOOL
PSG_GetMutePsgChannel(
	Sint32	num);

//Kitao追加。v2.60
void
PSG_SetHoneyInTheSky(
	BOOL	bHoneyInTheSky);


#endif		/* PSG_H_INCLUDED */
