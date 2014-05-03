/******************************************************************************
Ootake
・ROMの容量を返すようにした。
・吸出し機によってはヘッダが付いてしまうものがあるようなので、ヘッダ付きのイメ
  ージファイルにも対応した。

Copyright(C)2006 Kitao Nakamura.
	改造版・後継版を公開なさるときは必ずソースコードを添付してください。
	その際に事後でかまいませんので、ひとことお知らせいただけると幸いです。
	商的な利用は禁じます。
	あとは「GNU General Public License(一般公衆利用許諾契約書)」に準じます。

*******************************************************************************
	[Cartridge.c]
		カートリッジの読み込み等を行ないます．

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
#include "Cartridge.h"


static BOOL
read_384k(
	FILE*		p,
	Uint8*		pRom,
	Sint32		headerSize)//Kitao更新
{
	/* mask は 0xFFFFF になるので、$00000 - $FFFFF をすぺて埋める */
	if (fread(pRom, sizeof(Uint8), 262144, p) != 262144)
		return FALSE;

	fseek(p, headerSize, SEEK_SET);//Kitao更新

	if (fread(pRom + 262144, sizeof(Uint8), 393216, p) != 393216)
		return FALSE;

	memcpy(pRom+262144+393216, pRom, 0x100000-(262144+393216));

	return TRUE;
}


/*-----------------------------------------------------------------------------
** [LoadCartridge]
**	 カートリッジをファイルから読み込みます．
** カートリッジに必要な領域の確保も行ないます．
** 読み込んだカートリッジのマスク値を返します．(ゼロなら失敗)
**---------------------------------------------------------------------------*/
Uint32
CART_LoadCartridge(
	const char*		pGameName,
	Uint8**			ppRom,
	Uint32*			pRomSize) //Kitao更新。ROMの容量サイズも設定されるようにした。
{
	FILE*	pFile;
	Uint32	fileSize;
	Uint32	headerSize; //Kitao追加
	Uint32	mask;

	pFile = fopen(pGameName, "rb");
	
	if (pFile == NULL)
	{
		return 0;
	}

	fseek(pFile, 0, SEEK_END);
	fileSize = ftell(pFile);
	//Kitao追加。吸出し機によってはヘッダが付いてしまうものがあるようなのでヘッダがあった場合はカット。
	headerSize = fileSize % 1024;
	if (headerSize != 0)
	{
		fseek(pFile, headerSize, SEEK_SET);
		fileSize -= headerSize;
	}
	else
		fseek(pFile, 0, SEEK_SET);
	*pRomSize = fileSize;

	// ROM MASK の値を決定する。 
	// ROM のサイズが $6000 などの場合
	// (２のべき乗でない場合)も考慮する。 
	if (fileSize == 393216)
		mask = 1 << 20;
	else
	{
		mask = 0x2000;
		while (mask < fileSize)
			mask <<= 1;
	}

	// 実際には２のべき乗で確保する ($6000 の場合は $8000)
	*ppRom = (Uint8*)malloc(mask);
	if (*ppRom == NULL)
	{
		fclose(pFile);
		return 0;
	}
	memset(*ppRom, 0xFF, mask);
	--mask;	

	if (fileSize == 393216)
	{
		if (!read_384k(pFile, *ppRom, headerSize))
		{
			fclose(pFile);
			free(*ppRom);
			return 0;
		}
	}
	else if (fread(*ppRom, sizeof(Uint8), fileSize, pFile) != fileSize)
	{
		fclose(pFile);
		free(*ppRom);
		return 0;
	}

	fclose(pFile);

	return mask;
}


/*-----------------------------------------------------------------------------
** [FreeCartridge]
**	 カートリッジを破棄します．
** カートリッジ用に確保した領域を解放します．
**---------------------------------------------------------------------------*/
void
CART_FreeCartridge(
	Uint8*		pRom)
{
	free(pRom);
}

