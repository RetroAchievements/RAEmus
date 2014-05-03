/*-----------------------------------------------------------------------------
	[JoyPad.h]
		パッドを記述するのに必要な定義および関数のプロトタイプ宣言を行ないます．

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
#ifndef JOY_PAD_H_INCLUDED
#define JOY_PAD_H_INCLUDED

#include <stdio.h>
#include "TypeDefs.h"


enum JoyPadID
{
	JOYPAD_BUTTON_I = 0,
	JOYPAD_BUTTON_II,
	JOYPAD_BUTTON_III,
	JOYPAD_BUTTON_IV,
	JOYPAD_BUTTON_V,
	JOYPAD_BUTTON_VI,
	JOYPAD_BUTTON_SELECT,
	JOYPAD_BUTTON_RUN,
	JOYPAD_BUTTON_UP,
	JOYPAD_BUTTON_RIGHT,
	JOYPAD_BUTTON_DOWN,
	JOYPAD_BUTTON_LEFT
};


/*-----------------------------------------------------------------------------
	関数のプロトタイプ宣言を行ないます．
-----------------------------------------------------------------------------*/
Sint32
JOYPAD_Init();

void
JOYPAD_Deinit();

//Kitao追加
void
JOYPAD_Reset();


Uint8
JOYPAD_Read(Uint32 regNum);

void
JOYPAD_Write(
	Uint32		regNum,
	Uint8		data);


BOOL
JOYPAD_SaveState(
	FILE*		p);

BOOL
JOYPAD_LoadState(
	FILE*		p);


void
JOYPAD_ConnectMultiTap(
	BOOL	bConnect);

void
JOYPAD_UseSixButton(
	BOOL	bSixButton);

//Kitao追加
void
JOYPAD_UseThreeButton(
	BOOL	bThreeButton);

void
JOYPAD_ConnectMouse(
	BOOL	bConnect);

void
JOYPAD_ConnectMB128(
	BOOL	bConnect);


//Kitao追加
BOOL
JOYPAD_GetConnectMultiTap();

//Kitao追加
BOOL
JOYPAD_GetConnectSixButton();

//Kitao追加
BOOL
JOYPAD_GetConnectThreeButton();


//Kitao追加
void
JOYPAD_SetSwapSelRun(
	BOOL	bSwapSelRun);

//Kitao追加
void
JOYPAD_SetSwapIandII(
	BOOL	bSwapIandII);

//Kitao追加
BOOL
JOYPAD_GetSwapSelRun();

//Kitao追加
BOOL
JOYPAD_GetSwapIandII();

//Kitao追加
void
JOYPAD_ClearPrevUpdateStateLine();

//Kitao追加
void	JOYPAD_SetRenshaSpeedButton1(Sint32	n);
void	JOYPAD_SetRenshaSpeedButton2(Sint32	n);
void	JOYPAD_SetRenshaSpeedButtonRun(Sint32	n);
void	JOYPAD_SetRenshaSpeedButton5(Sint32	n);
void	JOYPAD_SetRenshaSpeedButton6(Sint32	n);
void	JOYPAD_SetRenshaButton1(BOOL	bSet);
void	JOYPAD_SetRenshaButton2(BOOL	bSet);
void	JOYPAD_SetRenshaButtonRun(BOOL	bSet);
void	JOYPAD_SetRenshaButton5(BOOL	bSet);
void	JOYPAD_SetRenshaButton6(BOOL	bSet);
void	JOYPAD_SetRenshaSpeedMax(Sint32	n);
BOOL	JOYPAD_GetRenshaButton1();
BOOL	JOYPAD_GetRenshaButton2();
BOOL	JOYPAD_GetRenshaButtonRun();

//Kitao追加
void
JOYPAD_SetMakeHesNoSound(
	BOOL	bMakeHesNoSound);

//Kitao追加
BOOL
JOYPAD_GetMakeHesNoSound();


#endif		/* JOY_PAD_H_INCLUDED */
