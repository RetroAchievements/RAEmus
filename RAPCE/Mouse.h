/*-----------------------------------------------------------------------------
	[Mouse.h]
		マウスの実装に必要な定義や宣言を行います。

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
#ifndef MOUSE_H_INCLUDED
#define MOUSE_H_INCLUDED

#include <stdio.h>
#include "TypeDefs.h"

void
MOUSE_RButtonDown(
	BOOL		bDown);

void
MOUSE_LButtonDown(
	BOOL		bDown);


void
MOUSE_SelButtonDown(
	BOOL		bDown);

void
MOUSE_RunButtonDown(
	BOOL		bDown);


void
MOUSE_Reset();


Uint8
MOUSE_ReadDelta();

Uint8
MOUSE_ReadButtons();

//Kitao追加
void
MOUSE_SetMouseWheelFlg();

//Kitao追加
Uint32
MOUSE_GetMouseWheelFlg();

void
MOUSE_Connect(
	BOOL		bConnect);

BOOL
MOUSE_IsConnected();

void
MOUSE_AdvanceClock();

void
MOUSE_UpdateDelta();

BOOL
MOUSE_SaveState(
	FILE*		p);


BOOL
MOUSE_LoadState(
	FILE*		p);


#endif /* MOUSE_H_INCLUDED */
