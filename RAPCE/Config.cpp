/******************************************************************************
Ootake
Copyright(C)2006-2009 Kitao Nakamura.
	改造版・後継版を公開なさるときは必ずソースコードを添付してください。
	その際に事後でかまいませんので、ひとことお知らせいただけると幸いです。
	商的な利用は禁じます。
	あとは「GNU General Public License(一般公衆利用許諾契約書)」に準じます。

*******************************************************************************
	[Config.c]

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
#include <malloc.h>
#include "Config.h"

#define		CONFIG_MAXITEM		128 //Kitao更新
#define		CONFIG_MAXLEN		256

typedef struct
{
	char		name[CONFIG_MAXLEN];
	char		value[CONFIG_MAXLEN];
} ConfigItem;

static ConfigItem		_Item[CONFIG_MAXITEM];
static Sint32			_nItem = 0;


/*-----------------------------------------------------------------------------
	[Init]
		コンフィグを初期化します。
-----------------------------------------------------------------------------*/
BOOL
CONFIG_Init()
{
	// メモリアロケート型にするときはここでメモリを確保する 
	return TRUE;
}


/*-----------------------------------------------------------------------------
	[Load]
		ファイルから設定項目を読み込みます。
-----------------------------------------------------------------------------*/
BOOL
CONFIG_Load(
	const char*		pPathName)
{
	FILE*		in;
	Sint32		nItem;

	if ((in = fopen(pPathName, "rb")) == NULL)	return FALSE;

	if (fread(&nItem, sizeof(nItem), 1, in) != 1)
	{
		fclose(in);
		return TRUE;
	}

	fread(_Item, sizeof(ConfigItem), CONFIG_MAXITEM, in);
	fclose(in);

	_nItem = nItem;

	return TRUE;
}


/*-----------------------------------------------------------------------------
	[Save]
		ファイルに設定項目を書き込みます。
-----------------------------------------------------------------------------*/
BOOL
CONFIG_Save(
	const char*		pPathName)
{
	FILE*		out;
	if ((out = fopen(pPathName, "wb")) == NULL)	return FALSE;

	if (fwrite(&_nItem, sizeof(_nItem), 1, out) != 1)
	{
		fclose(out);
		return FALSE;
	}

	if (fwrite(_Item, sizeof(ConfigItem), _nItem, out) != _nItem)
	{
		fclose(out);
		return FALSE;
	}

	fclose(out);

	return TRUE;
}


/*-----------------------------------------------------------------------------
	[Deinit]
		コンフィグを終了します。
-----------------------------------------------------------------------------*/
BOOL
CONFIG_Deinit()
{
	_nItem = 0;

	// メモリアロケート型にするときはここでメモリを解放する 
	return TRUE;
}


/*-----------------------------------------------------------------------------
	[Set]
	  pName で指定されるコンフィグを設定します。
	設定項目はユニークな名前と値と値のサイズ(バイト数)で登録します。
	項目名 pName が既に存在した場合は、値が上書きされます。
	文字列を登録する場合は strlen(文字列)+1 を valueSize に指定します。
-----------------------------------------------------------------------------*/
BOOL
CONFIG_Set(
	const char*		pName,
	const void*		pValue,
	const Sint32	valueSize)
{
	int				i;

	// pName が既存かどうかを調べる。 
	for (i = 0; i < _nItem; i++)
	{
		if (strcmp(_Item[i].name, pName) == 0)
		{
			// 既存ならその値を変更する。 
			if (valueSize > CONFIG_MAXLEN)
				return FALSE;

			memcpy(_Item[i].value, pValue, valueSize);
			return TRUE;
		}
	}

	// まだない設定項目なら新規に追加する。 
	if (_nItem >= CONFIG_MAXITEM)
		return FALSE;

	if (strlen(pName) + 1 > CONFIG_MAXLEN)
		return FALSE;

	if (valueSize > CONFIG_MAXLEN)
		return FALSE;

	strcpy(_Item[_nItem].name, pName);
	memcpy(_Item[_nItem].value, pValue, valueSize);
	++_nItem;

	return TRUE;
}


/*-----------------------------------------------------------------------------
	[Get]
	  pName で指定された設定項目の値を返します。
-----------------------------------------------------------------------------*/
BOOL
CONFIG_Get(
	const char*		pName,
	      void*		pValue,
	const Sint32	valueSize)
{
	int				i;

	for (i = 0; i < _nItem; i++)
	{
		if (strcmp(_Item[i].name, pName) == 0)
		{
			// 設定項目がみつかったらその値をコピーする。 
			memcpy(pValue, _Item[i].value, valueSize);
			return TRUE;
		}
	}

	return FALSE;
}

