/*-----------------------------------------------------------------------------
	[APU.h]
		ＡＰＵを記述するのに必要な定義および
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
#ifndef APU_H_INCLUDED
#define APU_H_INCLUDED

#include <stdio.h>
#include "TypeDefs.h"


/*-----------------------------------------------------------------------------
** 関数のプロトタイプ宣言を行ないます．
**---------------------------------------------------------------------------*/
BOOL
APU_Init(
	Uint32		sampleRate,
	Uint32		bufSize);		// in samples

void
APU_Deinit();

//Kitao更新
BOOL
APU_Reset();

void
APU_Pause(
	BOOL		bPause);

BOOL
APU_SetSampleRate(
	Uint32		sampleRate);

BOOL
APU_SetBufferSize(
	Uint32		bufSize);

void
APU_SetVolume(
	Uint32		volume);

//Kitao追加
BOOL
APU_GetApuBusy();


void
APU_AdvanceClock();


BOOL
APU_SaveState(
	FILE*		p);

BOOL
APU_LoadState(
	FILE*		p);


#endif		/* APU_H_INCLUDED */
