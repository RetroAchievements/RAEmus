/******************************************************************************
Ootake
・スクリーン上にもメッセージの表示ができるようにした。v0.64

Copyright(C)2006-2009 Kitao Nakamura.
	改造版・後継版を公開なさるときは必ずソースコードを添付してください。
	その際に事後でかまいませんので、ひとことお知らせいただけると幸いです。
	商的な利用は禁じます。
	あとは「GNU General Public License(一般公衆利用許諾契約書)」に準じます。

*******************************************************************************
	[Printf.c]

		Implements a printf function which displays a text on the
	main window's title bar.

	Copyright (C) 2005 Ki

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
******************************************************************************/
#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include <stdarg.h>
#include "Printf.h"
#include "WinMain.h"
#include "Screen.h"
#include "App.h"
#include "MainBoard.h"


static char						_MessageBuffer[2048];
static Uint32					_MessageTime = 60*8; //表示時間(単位はフレーム)。Kitao追加。v1.58
static Uint32					_MessageTimeSaveLoad = 60*6; //ステートセーブとロード時の表示時間。他のメッセージより少し短めにする。v1.58
static BOOL						_bSaveLoadMessage = FALSE; //※"PRINTFをした後"に、ステートセーブとロード時の場合TRUEに設定する。v1.58
static Uint32					_FrameCount = 0; //メッセージ表示中は1以上になる。非表示中は0固定とする。v2.54更新


/*----------------------------------------------------------------------------
	[PRINTF]
		Displays a text message for several seconds.
----------------------------------------------------------------------------*/
void
PRINTF(
	const char*		pMessage, ...)
{
	va_list			ap;

	va_start(ap, pMessage);
	vsprintf(_MessageBuffer, pMessage, ap);
	va_end(ap);

	while (strchr(_MessageBuffer, '\n'))
		*strchr(_MessageBuffer, '\n') = '\0';

	//puts(_MessageBuffer);
	//Kitao追加。スクリーン上へのメッセージ表示も可能に。
	if (APP_GetFullScreen())
	{
		if (!APP_GetFullHideMessage())
			SCREEN_SetMessageText(_MessageBuffer);
	}
	else
	{
		if (APP_GetHideMessage())
			WINMAIN_SetCaption(_MessageBuffer);
		else
			SCREEN_SetMessageText(_MessageBuffer);
	}
	_FrameCount = 1;
	_bSaveLoadMessage = FALSE; //v1.58追加
}


void
PRINTF_Update()
{
	Uint32	mt;

	if (_FrameCount > 0) //メッセージ表示中なら。v2.54追加
	{
		if (_bSaveLoadMessage) //v1.58追加
			mt = _MessageTimeSaveLoad;
		else
			mt = _MessageTime;
		switch (MAINBOARD_GetFastForwarding()) //早回しが行われている場合、それに合わせてフレーム数を増やす。v2.38
		{
			case   10: mt = (Uint32)((double)mt * 1.10); break;
			case    5: mt = (Uint32)((double)mt * 1.20); break;
			case    3: mt = (Uint32)((double)mt * 1.33); break;
			case    2: mt = (Uint32)((double)mt * 1.50); break;
			case 1001: mt = (Uint32)((double)mt * 1.67); break;
			case 1000: mt = (Uint32)((double)mt * 1.83); break;
			case 2010: mt = (Uint32)((double)mt * 0.90); break;
			case 2004: mt = (Uint32)((double)mt * 0.75); break;
			case 2002: mt = (Uint32)((double)mt * 0.50); break;
		}
		if (++_FrameCount > mt) //v1.58追加。メッセージ表示時間を変更可能に。
		{
			if (MAINBOARD_GetPause())  //Kitao追加。PAUSE中はメッセージを消さない。
				--_FrameCount;
			else
			{	//メッセージ消去
				WINMAIN_SetCaption(NULL);
				SCREEN_SetMessageText(""); //Kitao追加
				_FrameCount = 0;
			}
		}
	}
}


BOOL
PRINTF_Init()
{
	SCREEN_SetMessageText(""); //Kitao追加
	_FrameCount = 0;
	return TRUE;
}


void
PRINTF_Deinit()
{
	SCREEN_SetMessageText(""); //Kitao追加
	_FrameCount = 0;
}


//Kitao追加。"PRINTFをした後"にここを呼ぶと、メッセージの表示時間を通常より短くできる。v1.58
void
PRINTF_SetSaveLoadMessage()
{
	_bSaveLoadMessage = TRUE;
}

