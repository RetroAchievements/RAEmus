/*-----------------------------------------------------------------------------
	[PadConfig.h]

		Defines things needed for implementing configuration window.

	Copyright (C) 2004-2005 Ki

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
-----------------------------------------------------------------------------*/
#ifndef PAD_CONFIG_H_INCLUDED
#define PAD_CONFIG_H_INCLUDED

#include <windows.h>
#include "TypeDefs.h"
#include "Input.h" //Kitao追加


typedef struct
{
	Uint32		i;
	Uint32		ii;
	Uint32		iii;
	Uint32		iv;
	Uint32		v;
	Uint32		vi;
	Uint32		select;
	Uint32		run;
	Uint32		up;
	Uint32		right;
	Uint32		down;
	Uint32		left;
} JoyPad;


//Kitao更新。複数人プレイのときもキーボードのボタンを使えるようにした。
BOOL
PADCONFIG_Init(
	HINSTANCE	hInstance,
	Sint32	 	mode,
	Sint32	 	padID,
	PCEPAD* 	pPad,
	Sint32*		setOk);

void
PADCONFIG_Deinit();


#endif /* PAD_CONFIG_H_INCLUDED */
