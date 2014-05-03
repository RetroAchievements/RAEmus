/******************************************************************************
Ootake
Copyright(C)2006-2007 Kitao Nakamura.
	改造版・後継版を公開なさるときは必ずソースコードを添付してください。
	その際に事後でかまいませんので、ひとことお知らせいただけると幸いです。
	商的な利用は禁じます。
	あとは「GNU General Public License(一般公衆利用許諾契約書)」に準じます。

*******************************************************************************
	[AppEvent.c]

	Copyright (C) 2004 Ki

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
#include <string.h>
#include <windows.h>
#include "AppEvent.h"

/*
	複数同時に起こるイベントについては今のところ考慮しない。
	EXIT イベントは他の全てのイベントよりも優先する。
*/

static char					_GameFileName[MAX_PATH+1];

static Sint32				_CurrentEvent = APPEVENT_NONE;
static BOOL					_bNoEvent;

static Uint32				_LongArg;

static BOOL					_bInit = FALSE;


BOOL
APPEVENT_Init()
{
	_CurrentEvent = APPEVENT_NONE;

	_bInit = TRUE;
	_bNoEvent = TRUE;

	return TRUE;
}


BOOL
APPEVENT_Deinit()
{
	_bInit = FALSE;
	_CurrentEvent = APPEVENT_NONE;

	return TRUE;
}


BOOL
APPEVENT_Set(
	const Sint32	event,
	const void*		pParam)
{
	if (!_bInit)
		return FALSE;

	// EXIT イベントを受け取った場合は他のイベントを受け付けない。 
	if (_CurrentEvent == APPEVENT_EXIT)
		return FALSE;

	if (!_bNoEvent)
		return FALSE;

	_CurrentEvent = event;

	// パラメータを受け取る必要があるイベントだけ処理する 
	switch (_CurrentEvent)
	{
		case APPEVENT_OPENGAME:
			if (pParam == NULL) return FALSE;
			strcpy(_GameFileName, (char*)pParam);
			break;

		default:
			return FALSE;
	}

	_bNoEvent = FALSE;

	return TRUE;
}

const Sint32
APPEVENT_GetEventID()
{
	MSG			msg;

	if (!_bInit)
		return APPEVENT_NONE;

	// メッセージループ 
	if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
	{
		if (!GetMessage(&msg, NULL, 0, 0))
		{
			_CurrentEvent = APPEVENT_EXIT;
		}
		else
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return _CurrentEvent;
}


const void*
APPEVENT_GetParam(
	const Sint32	event)
{
	if (!_bInit)
		return NULL;

	if (_bNoEvent)
		return NULL;

	switch (event)
	{
		case APPEVENT_OPENGAME:
			return (const void*)_GameFileName;

		case APPEVENT_SHOWMENU:
			return (const void*)_LongArg;

		default:
			return NULL;
	}

	return NULL;
}


void
APPEVENT_Ack()
{
	if (_bInit)
	{
		// EXIT イベントは Deinit するまで消えない 
		if (_CurrentEvent != APPEVENT_EXIT)
		{
			_CurrentEvent = APPEVENT_NONE;
			_bNoEvent = TRUE;
		}
	}
}

