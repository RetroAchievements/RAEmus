/*-----------------------------------------------------------------------------
	[MB128.h]
		Memory Base 128 互換メディアの実装に必要な定義や宣言を行ないます。

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
#ifndef MB128_H_INCLUDED
#define MB128_H_INCLUDED

#include <stdio.h>
#include "TypeDefs.h"


#define MB128_FILENAME		"MB128.dat"


void
MB128_Init();


//Kitao追加
BOOL
MB128_LoadFile();


//Kitao更新
BOOL
MB128_SaveFile();


void
MB128_Connect(
	BOOL		bConnect);

BOOL
MB128_IsConnected();


BOOL
MB128_IsActive();


Uint8
MB128_Read();


void
MB128_Write(
	Uint8		data);


/*-----------------------------------------------------------------------------
	[SaveState]
		状態をファイルに保存します。 
-----------------------------------------------------------------------------*/
BOOL
MB128_SaveState(
	FILE*		p);


/*-----------------------------------------------------------------------------
	[LoadState]
		状態をファイルから読み込みます。 
-----------------------------------------------------------------------------*/
BOOL
MB128_LoadState(
	FILE*		p);


#endif /* MB128_H_INCLUDED */
