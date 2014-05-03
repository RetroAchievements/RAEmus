/*-----------------------------------------------------------------------------
	[IntCtrl.h]
		割込みコントローラを記述するのに必要な定義および
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

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
**---------------------------------------------------------------------------*/
#ifndef INT_CTRL_H_INCLUDED
#define INT_CTRL_H_INCLUDED

#include <stdio.h>
#include "TypeDefs.h"


#define INTCTRL_NONE		0x00
#define INTCTRL_IRQ2		0x01
#define INTCTRL_IRQ1		0x02
#define INTCTRL_TIRQ		0x04


/*-----------------------------------------------------------------------------
** 関数のプロトタイプ宣言を行ないます．
**---------------------------------------------------------------------------*/
Sint32
INTCTRL_Init();

void
INTCTRL_Deinit();


Uint8
INTCTRL_Read(Uint32 regNum);

void
INTCTRL_Write(
	Uint32		regNum,
	Uint8		data);

Uint8
INTCTRL_GetPendingIrq();

void
INTCTRL_Request(Uint8 request);

void
INTCTRL_Cancel(Uint8 request);


BOOL
INTCTRL_SaveState(
	FILE*		p);

BOOL
INTCTRL_LoadState(
	FILE*		p);


#endif		/* INT_CTRL_H_INCLUDED */
