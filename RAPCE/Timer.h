/*-----------------------------------------------------------------------------
	[Timer.h]
		タイマーを記述するのに必要な定義および関数のプロトタイプ宣言を行ないます．

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
#ifndef TIMER_H_INCLUDED
#define TIMER_H_INCLUDED

#include <stdio.h>
#include "TypeDefs.h"

#define TIMER_IRQ			0x01

/*-----------------------------------------------------------------------------
** 関数のプロトタイプ宣言を行ないます．
**---------------------------------------------------------------------------*/

Sint32
TIMER_Init();

void
TIMER_Deinit();

Uint8
TIMER_Read();

void
TIMER_Write(
	Uint32		regNum,
	Uint8		data);

void
TIMER_AdvanceClock();

BOOL
TIMER_SaveState(
	FILE*		p);

BOOL
TIMER_LoadState(
	FILE*		p);

#endif		/* TIMER_H_INCLUDED */
