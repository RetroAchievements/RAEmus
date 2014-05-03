/******************************************************************************
Ootake
・割り込み禁止指令が来た場合、割り込み要求をキャンセルせずに、割り込み発生時に
  割り込みをさせないだけにして、ステータスはセットされたままにするようにした。

Copyright(C)2006 Kitao Nakamura.
	改造版・後継版を公開なさるときは必ずソースコードを添付してください。
	その際に事後でかまいませんので、ひとことお知らせいただけると幸いです。
	商的な利用は禁じます。
	あとは「GNU General Public License(一般公衆利用許諾契約書)」に準じます。

*******************************************************************************
	[IntCtrl.c]
		割込みコントローラを実装します．

	Implements the interrupt controller.

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
#include <stdio.h>
#include "IntCtrl.h"
#include "CPU.h"
#include "Printf.h"


static Uint8	_IntDisable;
static Uint8	_IntStatus;


/*
	[DEV NOTE]

	ＣＰＵが IntCtrl の $1403 に書き込み動作を行なった瞬間に
	TIRQ のリクエストは取り下げられる。

	ＣＰＵが $FF:0000 (VDC) から読み出しを行なった瞬間に
	IRQ1 のリクエストは取り下げられる。

	上の２つの動作をＣＰＵが行なわず、
	IntCtrl の disable フラグがゼロで、
	かつ ＣＰＵ のＩフラグがゼロの場合は
	割り込みが発生し続ける。
	（ただし割り込みが発生するとＩフラグは１にセットされる）

	ＣＰＵが $1402 (割り込み禁止レジスタ) に書き込みを行なったとき、
	IntCtrl は次の３つの動作を行なわなければならない：

	１．書き込まれたデータを保存する(下位３ビット)
	２．その書き込みによって新たに禁止されたビットを調べ、 IRQ を取り下げる
	３．その書き込みによって新たに許可されたビットを調べ、
		周辺機器からの割り込み要求状況に応じて CPU に IRQ を要求する

	[Kitao更新]
	・IRQ1のリクエストは、VDCのステータスを読み出したときだけではなく
	  割り込みが行われたと同時にリクエストが取り下げられる（つまりリクエスト
	  はいつも１回だけ有効）動作にした。これでCLI命令直後の割り込みを許可した
	  場合も、ジャッキーチェンの起動がうまくいくようになった。
	・割り込み禁止指令が来た場合、割り込み要求をキャンセルせずに、割り込み発生
	  時に割り込みをさせないだけにして、ステータスはセットされたままにするよう
	  にした。
*/


/*-----------------------------------------------------------------------------
** [INTCTRL_Request]
**   割込み要求ステータスをセットします．
**---------------------------------------------------------------------------*/
//Kitao更新。割り込み禁止時にもステータスをセットされるようにした。
void
INTCTRL_Request(
	Uint8		request)
{
	if (request & INTCTRL_IRQ2)
	{
		_IntStatus |= INTCTRL_IRQ2;
		CPU_ActivateIRQ2();
	}

	if (request & INTCTRL_IRQ1)
	{
		_IntStatus |= INTCTRL_IRQ1;
		CPU_ActivateIRQ1();
	}

	if (request & INTCTRL_TIRQ)
	{
		_IntStatus |= INTCTRL_TIRQ;
		CPU_ActivateTIMER();
	}
}


/*-----------------------------------------------------------------------------
** [INTCTRL_Cancel]
**   割込み要求ステータスをリセットします．
**---------------------------------------------------------------------------*/
void
INTCTRL_Cancel(
	Uint8		request)
{
	if (request & INTCTRL_IRQ2)
	{
		_IntStatus &= ~INTCTRL_IRQ2;
		CPU_InactivateIRQ2();
	}

	if (request & INTCTRL_IRQ1)
	{
		_IntStatus &= ~INTCTRL_IRQ1;
		CPU_InactivateIRQ1();
	}

	if (request & INTCTRL_TIRQ)
	{
		_IntStatus &= ~INTCTRL_TIRQ;
		CPU_InactivateTIMER();
	}
}


/*-----------------------------------------------------------------------------
** [INTCTRL_Init]
**   割込みコントローラを初期化します．
**---------------------------------------------------------------------------*/
Sint32
INTCTRL_Init()
{
	_IntDisable = 0;
	CPU_SetIntDisable(_IntDisable); //Kitao追加
	_IntStatus = 0;

	return 0;
}


/*-----------------------------------------------------------------------------
** [INTCTRL_Deinit]
**   割込みコントローラの終了処理を行ないます．
**---------------------------------------------------------------------------*/
void
INTCTRL_Deinit()
{
	_IntDisable = 0;
	CPU_SetIntDisable(_IntDisable); //Kitao追加
	_IntStatus = 0;
}


/*-----------------------------------------------------------------------------
** [INTCTRL_Read]
**   割込みコントローラからの読み出し動作です．
**---------------------------------------------------------------------------*/
Uint8
INTCTRL_Read(Uint32 regNum)
{
	switch (regNum & 3) //Kitao更新
	{
		case 2:
			return _IntDisable;

		case 3:
			return _IntStatus;
	}
	//(regNum & 3)が0か1の場合
	return 0; //MainBoard.c から呼ばれた場合、実際にはBufferの値とミックスされた値が読み出される。v1.62記
}


/*-----------------------------------------------------------------------------
** [INTCTRL_Write]
**   割込みコントローラへの書き込み動作です．
**---------------------------------------------------------------------------*/
//Kitao更新
void
INTCTRL_Write(
	Uint32		regNum,
	Uint8		data)
{
	//PRINTF("IntCtrl Write = %X,%X", regNum,data); //Test用

	switch (regNum & 3) //Kitao更新
	{
		case 2:
			_IntDisable = data & 7; //Kitao更新
			CPU_SetIntDisable(_IntDisable); //Kitao追加
			return;

		case 3:
			//PRINTF("IntCtrl Write = %X,%X", regNum,data); //Test用
			INTCTRL_Cancel(INTCTRL_TIRQ);
			return;
	}
}


// save variable
#define SAVE_V(V)	if (fwrite(&V, sizeof(V), 1, p) != 1)	return FALSE
#define LOAD_V(V)	if (fread(&V, sizeof(V), 1, p) != 1)	return FALSE
/*-----------------------------------------------------------------------------
	[SaveState]
		状態をファイルに保存します。 
-----------------------------------------------------------------------------*/
BOOL
INTCTRL_SaveState(
	FILE*		p)
{
	if (p == NULL)
		return FALSE;

	SAVE_V(_IntDisable);
	SAVE_V(_IntStatus);

	return TRUE;
}


/*-----------------------------------------------------------------------------
	[LoadState]
		状態をファイルから読み込みます。 
-----------------------------------------------------------------------------*/
BOOL
INTCTRL_LoadState(
	FILE*		p)
{
	if (p == NULL)
		return FALSE;

	LOAD_V(_IntDisable);
	CPU_SetIntDisable(_IntDisable); //Kitao追加
	LOAD_V(_IntStatus);

	return TRUE;
}

#undef SAVE_V
#undef LOAD_V
