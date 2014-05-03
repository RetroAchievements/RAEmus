/******************************************************************************
Ootake
・割り込み要求を受け付けたときに、すぐ割り込み要求をクリアするようにした。
・割り込み要求を受け付けたときに、割り込み禁止かどうかをチェックするようにして
  禁止だった場合も割り込み要求はそのままにしておく(次回以降割り込める)ようにし
  た。v0.64
・ラスタ割り込みタイミング(IRQ1)を最重視するため、割り込みの優先順位を逆順とし
  た。
・Dフラグが立っているときのADC演算で、うまく値が足されていなかった不具合を修正
  した。
・CSH,CSL命令の動きを実装した。（CSLはラスタ割り込み処理の時間待ちに使われるこ
  とがある）
・タイマーカウンタはここでは進めずにCPUとは無関係で進めるようにした。
・転送命令中にも、CPU以外の動作が並列動作できるようにした。
・CPUの消費サイクルを一部変更した。
・割り込み処理時の消費サイクル数を8(HuC6280のBRKと同数)に変更した。(実機未確認)
・CPUの消費サイクルを全命令1だけにするモード（ターボサイクルモード）を付けた。
  v1.61からオーバークロックメニューと統合し、速さの種類を増やした。
・PSG処理をシンプルにしたため_ClockUpCountを不要とした。
・ステートセーブ時に_ClockCountと_ClockElapsedもセーブするようにした。
・わかりにくくならない範囲内でサブルーチン処理を減らして高速化した。v0.93
・read(),write()関数をマクロ化することで高速化した。v1.07
・変数への代入処理をできるだけ減らすことで高速化した。v1.08
・ゼロページメモリへのアクセスが必ずメインメモリ($F8バンク)に限らない(Kiさんか
  らの情報)ことへ対応した。v1.29
・よく使われる命令群を先に記述することで高速化した。v1.30
・消費クロックで、転送命令のTIAとTAIのみを6サイクル(ネット上の資料でよく書かれ
  値ている)とした。前記２命令よりも処理がシンプルなTII,TDD,TINは5サイクル(VDC
  のアクセスウェイトを入れると6サイクル)とした。実機での測定はしていないが、ゲ
  ームの動きを見るとこの実装が実機に最も近い感じだ。v1.30
・割り込み処理の際に、IFの状態は命令実行前の状態(prevIF)を見るようにした。ジャ
  ックニクラウスゴルフ(Huカード版)で、画面左側が時々乱れていた問題が解消した。
  v1.61

Copyright(C)2006-2011 Kitao Nakamura.
	改造版・後継版を公開なさるときは必ずソースコードを添付してください。
	その際に事後でかまいませんので、ひとことお知らせいただけると幸いです。
	商的な利用は禁じます。
	あとは「GNU General Public License(一般公衆利用許諾契約書)」に準じます。

*******************************************************************************
	[CPU.c]

		Implements a HuC6280 Emulator.

	Copyright (C) 2004-2005 Ki

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
******************************************************************************/
#include "CPU.h"
#include "MainBoard.h"
#include "Printf.h"
#include "Debug.h"
#include "App.h"

Sint32 gCPU_ClockCount = 0; //v2.00追加。高速化のためカウンタをグローバル化。
Sint32 gCPU_Transfer;		//v2.31更新。高速化のためカウンタをグローバル化。

static Uint8		_A;
static Uint8		_X;
static Uint8		_Y;
static Uint8		_S;
static Uint8		_P;
static Uint16		_PC;

static Uint8		_CF;
static Uint8		_ZF;
static Uint8		_IF;
static Uint8		_PrevIF; //Kitao追加。命令開始前のIフラグを保管しておく。命令後の割り込み処理をするかどうかの判断にはこれ(命令実行前の状態)が使われる。v1.61
static Uint8		_DF;
static Uint8		_BF;
static Uint8		_TF;
static Uint8		_VF;
static Uint8		_NF;

static Uint32		_MPR[8];

static Sint32		_ClockElapsed; //v1.61からクロック消費用としては非使用。デバッグ時に現在の命令消費サイクルを示す変数として使用。
static BOOL			_bSpeedLow; //Kitao追加。LowSpeedモード（CSL命令後）のときはTRUE。

static Uint16		_TransferSrcAddr; //Kitao追加
static Uint16		_TransferDstAddr; //Kitao追加
static Uint16		_TransferLength; //Kitao追加
static BOOL			_TransferIncDec; //Kitao追加。TRUEならインクリメント

static BOOL			_bRDY;		// DMA要求 
static BOOL			_bNMI;
static BOOL			_bIRQ1;
static BOOL			_bIRQ2;
static BOOL			_bTIRQ;
static BOOL			_bPrevIRQ1; //Kitao追加。PrevIFと共にIRQの状態も、命令開始前の状態が使われるので、それを保管しておく。v1.61
static BOOL			_bPrevIRQ2; //
static BOOL			_bPrevTIRQ; //
static Uint8		_IntDisable; //Kitao追加
static Uint8		_PrevIntDisable; //Kitao追加。PrevIFと共に割り込みの禁止状態も、命令開始前の状態が使われるので、それを保管しておく。v1.61

static Sint32		_SelectVDC; //Kitao追加。スーパーグラフィックス用

static BOOL			_bDebug = FALSE; //Kitao追加。v1.07
static Uint32		_OpCode = 0; //Kitao追加。v1.07
static Uint16		_PrevPC = 0; //Kitao追加。v1.07
static Uint8		_PrevFlags = 0; //Kitao追加。v1.07


// 読み出し関数 
static Uint8 (*ReadMem)(Uint32 mpr, Uint32 addr); //Kitao更新。v1.47

// 書き込み関数 
static void (*WriteMem)(Uint32 mpr, Uint32 addr, Uint8 data); //Kitao更新。v1.47


static Sint32	_CycleTableBase[256] =
{
//Kitao更新
//  PLP(28),PLA(68)を4サイクル消費にした。ネクロスのOPデモが実機と同じタイミングになる。ダブルドラゴンIIのOPデモで必要。v1.61,v2.51更新
//  PHP(08)を4サイクル消費にした。ネクロスのOPデモが実機と同じタイミングになる。サザンアイズOPもより実機に近く。v2.07更新
//  サイクル変更した箇所は実機でも同様かは未確認です。
//	0 1 2  3 4 5 6 7 8 9 A B C D E F ←Lo↓Hi
	8,7,3, 4,6,4,6,7,4,2,2,2,7,5,7,6, //0
	2,7,7, 4,6,4,6,7,2,5,2,2,7,5,7,6, //1
	7,7,3, 4,4,4,6,7,4,2,2,2,5,5,7,6, //2
	2,7,7, 2,4,4,6,7,2,5,2,2,5,5,7,6, //3
	7,7,3, 4,7,4,6,7,3,2,2,2,4,5,7,6, //4  Kitao更新。コード44(BSR)の値を変更。ジャンプ時のサイクルと合わせると合計7に。
	2,7,7, 5,2,4,6,7,2,5,3,2,2,5,7,6, //5  Kitao更新。コード54(CSL)の値を変更。Kiさんからの確かな実験情報によりHigh→Lowのときは9サイクル(7.159090MHz換算)。Low→Lowの場合もあるのでここでは2サイクルの値にしておく。バイオレントソルジャーOK。
	7,7,2, 2,4,4,6,7,4,2,2,2,7,5,7,6, //6
	2,7,7,17,4,4,6,7,2,5,4,2,7,5,7,6, //7  Kitao追記。コード7A(PLY)を3にすると、ダブルドラゴン２のOPデモでやや口パクが合わない。v2.55
	4,7,2, 7,4,4,4,7,2,2,2,2,5,5,5,6, //8  Kitao更新。コード80(BRA)の値を変更。ジャンプ時のサイクルと合わせると合計4に。3だとボンバーマン'93のスタートデモで１フレーム乱れ。
	2,7,7, 8,4,4,4,7,2,5,2,2,5,5,5,6, //9
	2,7,2, 7,4,4,4,7,2,2,2,2,5,5,5,6, //A
	2,7,7, 8,4,4,4,7,2,5,2,2,5,5,5,6, //B
	2,7,2,17,4,4,6,7,2,2,2,2,5,5,7,6, //C
	2,7,7,17,2,4,6,7,2,5,3,2,2,5,7,6, //D  Kitao更新。コードD4(CSH)の値を変更。Kiさんからの確かな実験情報によりLow→Highのときは6サイクル(7.159090MHz換算)。High→Highの場合もあるのでここでは2サイクルの値にしておく。バイオレントソルジャーOK。
	2,7,2,17,4,4,6,7,2,2,2,2,5,5,7,6, //E
	2,7,7,17,2,4,6,7,2,5,3,2,2,5,7,6  //F  Kitao追記。コードFA(PLX)を4にすると、ロードランナー(パックインビデオ版)スタート時にもたつき。v2.51
};
static Sint32	_CycleTableSlow[256];
static Sint32	_CycleTableTurbo1[256];
static Sint32	_CycleTableTurbo2[256];
static Sint32*	_pCycleTable; //v1.61更新


//Kitao追加。v1.61
//低速動作モード(CLS命令)の設定をする。低速に切り替えるならTRUE，通常高速モードに戻すならFALSEで呼ぶ。
static inline void
SetSpeedLow(
	BOOL	bSpeedLow)
{
	_bSpeedLow = bSpeedLow;
	if (VDC_GetOverClockType() < 100) //ターボサイクルモードのときはそのまま。それ以外のモードなら
	{
		if (_bSpeedLow)
			_pCycleTable = &_CycleTableSlow[0]; //LowSpeedモード（CSL命令後）のときは４倍のサイクル数が掛かる
		else//通常高速モード時
			_pCycleTable = &_CycleTableBase[0];
	}
}

//Kitao追加。v1.61
void
CPU_SetTurboCycle(
	Sint32	n)
{
	switch (n)
	{
		case 100: _pCycleTable = &_CycleTableTurbo1[0]; break; //ターボ１倍
		case 200: _pCycleTable = &_CycleTableTurbo2[0]; break; //ターボ２倍
		case 300: _pCycleTable = &_CycleTableTurbo2[0]; break; //ターボ３倍。２倍と同じだが、これに加えてVDC.cでオーバークロックする。
		default: //0
			SetSpeedLow(_bSpeedLow);
	}
}


//Kitao追加。PrevIF(命令実行前のフラグ状態で実際の判断に使われる値)を更新する。v1.61
static inline void
refreshPrevIF()
{
	_PrevIF = _IF;
	_bPrevIRQ1 = _bIRQ1;
	_bPrevIRQ2 = _bIRQ2;
	_bPrevTIRQ = _bTIRQ;
	_PrevIntDisable = _IntDisable;
}


/*-----------------------------------------------------------------------------
** Implement memory read/write stages
**---------------------------------------------------------------------------*/
//Kitao更新。read()とwrite()関数を、高速化のためサブルーチンでなくマクロ化した。v1.07
//			 ※マクロ定義なのでREAD(),WRITE(),READZP(),WRITEZP()以外の関数(READINC()など)は、カッコ内や演算式内には記述できない。
//			 ※マクロ定義なので引数のaddrに重くなる関数や演算子は書かないようにする。
#define READ(addr)		ReadMem(_MPR[(addr) >> 13], ((addr) & 0x1FFF))
#define READINC(addr)	ReadMem(_MPR[(addr) >> 13], ((addr) & 0x1FFF)); addr++ //インクリメント用
#define READINC_X(addr)	ReadMem(_MPR[(addr) >> 13], ((addr) & 0x1FFF)) + _X; addr++ //+_X＆インクリメント用
#define READINC_Y(addr)	ReadMem(_MPR[(addr) >> 13], ((addr) & 0x1FFF)) + _Y; addr++ //+_Y＆インクリメント用
#define READDEC(addr)	ReadMem(_MPR[(addr) >> 13], ((addr) & 0x1FFF)); addr-- //デクリメント用。転送命令時に使用。
#define READZP(addr)	ReadMem(_MPR[1], addr) //ゼロページ用
#define WRITE(addr,data)	WriteMem(_MPR[(addr) >> 13], ((addr) & 0x1FFF), data)
#define WRITEINC(addr,data)	WriteMem(_MPR[(addr) >> 13], ((addr) & 0x1FFF), data); addr++ //インクリメント用
#define WRITEDEC(addr,data)	WriteMem(_MPR[(addr) >> 13], ((addr) & 0x1FFF), data); addr-- //デクリメント用
#define WRITEZP(addr,data)	WriteMem(_MPR[1], addr, data) //ゼロページ用


/*-----------------------------------------------------------------------------
** ($ZP)
*/
//Kitao更新。ゼロページアクセスの際に、MPR1の値によって$F8(メインRAM)以外のバンクにもアクセスできるようにした。
//			 Kiさんから情報をいただきました。v1.29
static inline Uint16
fetchZpIndirect()
{
	Uint8	zpAddress;
	Uint16	low;

	zpAddress = ReadMem(_MPR[_PC >> 13], (_PC & 0x1FFF));
	_PC++;
	low = ReadMem(_MPR[1], zpAddress++);
	return ((READZP(zpAddress) << 8) | low);
}

/*-----------------------------------------------------------------------------
** ($ZP,X)
*/
//Kitao更新。v1.29
static inline Uint16
fetchZpIndexIndirect()
{
	Uint8	zpAddress;
	Uint16	low;

	zpAddress = ReadMem(_MPR[_PC >> 13], (_PC & 0x1FFF)) + _X;
	_PC++;
	low = ReadMem(_MPR[1], zpAddress++);
	return ((READZP(zpAddress) << 8) | low);
}

/*-----------------------------------------------------------------------------
** ($ZP),Y
*/
//Kitao更新。v1.29
static inline Uint16
fetchZpIndirectIndex()
{
	Uint8	zpAddress;
	Uint16	low;

	zpAddress = ReadMem(_MPR[_PC >> 13], (_PC & 0x1FFF));
	_PC++;
	low = ReadMem(_MPR[1], zpAddress++);
	return ((READZP(zpAddress) << 8) | low) + _Y;
}

/*-----------------------------------------------------------------------------
** $ABS
*/
//Kitao更新。v1.07
static inline Uint16
fetchAbs()
{
	Uint16	low;
	Uint16	high;

	low = ReadMem(_MPR[_PC >> 13], (_PC & 0x1FFF));
	_PC++;
	high = ReadMem(_MPR[_PC >> 13], (_PC & 0x1FFF));
	_PC++;
	return ((high << 8) | low);
}

/*-----------------------------------------------------------------------------
** $ABS,X
*/
//Kitao更新。v1.07
static inline Uint16
fetchAbsX()
{
	Uint16	low;
	Uint16	high;

	low = ReadMem(_MPR[_PC >> 13], (_PC & 0x1FFF));
	_PC++;
	high = ReadMem(_MPR[_PC >> 13], (_PC & 0x1FFF));
	_PC++;
	return ((high << 8) | low) + _X;
}

/*-----------------------------------------------------------------------------
** $ABS,Y
*/
//Kitao更新。v1.07
static inline Uint16
fetchAbsY()
{
	Uint16	low;
	Uint16	high;

	low = ReadMem(_MPR[_PC >> 13], (_PC & 0x1FFF));
	_PC++;
	high = ReadMem(_MPR[_PC >> 13], (_PC & 0x1FFF));
	_PC++;
	return ((high << 8) | low) + _Y;
}

/*-----------------------------------------------------------------------------
** ($ABS)
*/
//Kitao更新。v1.08
static inline Uint16
fetchAbsIndirect()
{
	Uint16	absAddr = fetchAbs();
	Uint16	low  = READINC(absAddr);
	return ((READ(absAddr) << 8) | low);
}

/*-----------------------------------------------------------------------------
** ($ABS,X)
*/
//Kitao更新。v1.08
static inline Uint16
fetchAbsIndirectX()
{
	Uint16	absAddr = fetchAbs() + _X;
	Uint16	low  = READINC(absAddr);
	return ((READ(absAddr) << 8) | low);
}


/*-----------------------------------------------------------------------------
** Implement stack operations
**---------------------------------------------------------------------------*/
static inline void
push(
	Uint8	reg8)
{
	WriteMem(_MPR[1], (0x100 | (_S--)), reg8);
}

static inline Uint8
pull()
{
	return ReadMem(_MPR[1], (0x100 | (++_S)));
}


/*-----------------------------------------------------------------------------
** Implement flag operations
** The idea is taken from nes6502.c by Matthew Conte.
**
** CF --- use CPU_CF (D0)
** ZF --- use 8-bit value
** IF --- use CPU_IF (D2)
** DF --- use CPU_DF (D3)
** BF --- use CPU_BF (D4)
** TF --- use CPU_TF (D5)
** VF --- use CPU_VF (D6)
** NF --- use 8-bit value
**---------------------------------------------------------------------------*/
static inline void
updateFlagZN(
	Uint8	val)
{
	_NF = _ZF = val;
}

static inline void
separateFlags(
	Uint8	p)
{
	_CF = p & CPU_CF;
	_ZF = ~(p >> 1) & 1;
	_IF = p & CPU_IF;
	_DF = p & CPU_DF;
	_BF = p & CPU_BF;
	_TF = p & CPU_TF;
	_VF = p;
	_NF = p;
}

static inline Uint8
gatherFlags()
{
	return _CF | ((_ZF==0)<<1) | _IF | _DF | _BF | _TF | (_VF&CPU_VF) | (_NF&CPU_NF);
}


/*-----------------------------------------------------------------------------
** Implement instruction execute stages
**---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
** BIT
*/
static inline void
bit(Uint8		val)
{
	_ZF = _A & val;
	_VF = _NF = val;
}

/*-----------------------------------------------------------------------------
** ADC
*/
static inline void
adc(Uint8		val)
{
	Uint32	lo;
	Uint32	hi;
	Uint32	sum;

	if (_DF)
	{
		//Kitao更新。値がうまく足されないことがあった不具合を修正した。パワーリーグ３のホームラン競争で発見。
		lo = (_A & 0x0F) + (val & 0x0F) + _CF;
		hi = (_A & 0xF0) + (val & 0xF0);
		if (lo > 0x09)
		{
			hi += 0x10;
			lo += 0x06;
		}
		_VF = (Uint8)((~(_A^val) & (_A^hi) & CPU_NF) >> 1);
		if (hi > 0x90)
			hi += 0x60;
		_CF = (Uint8)((hi & 0x100) >> 8);
		updateFlagZN(_A = (Uint8)((lo & 0x0F) + (hi & 0xF0))); //Kitao更新。ゼロフラグ＆ネガティブフラグを更新する。
		gCPU_ClockCount--;
	}
	else
	{
		sum = _A + val + _CF;
		_VF = (Uint8)((~(_A^val) & (_A^sum) & CPU_NF) >> 1);
		_CF = (Uint8)((sum & 0x100) >> 8);
		updateFlagZN(_A = (Uint8)sum);
	}
}

/*-----------------------------------------------------------------------------
** ADC with T-flag set
*/
static inline void
adc_t(Uint8		val)
{
	Uint8	M = READZP(_X);
	Uint32	lo;
	Uint32	hi;
	Uint32	sum;

	if (_DF)
	{
		//Kitao更新。値がうまく足されないことがあった不具合を修正した。パワーリーグ３のホームラン競争で発見。
		lo = (M & 0x0F) + (val & 0x0F) + _CF;
		hi = (M & 0xF0) + (val & 0xF0);
		if (lo > 0x09)
		{
			hi += 0x10;
			lo += 0x06;
		}
		_VF = (Uint8)((~(M^val) & (M^hi) & CPU_NF) >> 1);
		if (hi > 0x90)
			hi += 0x60;
		_CF = (Uint8)((hi & 0x100) >> 8);
		updateFlagZN(M = (Uint8)((lo & 0x0F) + (hi & 0xF0))); //Kitao更新。ゼロフラグ＆ネガティブフラグを更新する。
		gCPU_ClockCount -= 4; //v0.83更新。3サイクル消費(Kiさんから確かな実験情報)。-1はDフラグぶん
	}
	else
	{
		sum = M + val + _CF;
		_VF = (Uint8)((~(M^val) & (M^sum) & CPU_NF) >> 1);
		_CF = (Uint8)((sum & 0x100) >> 8);
		updateFlagZN(M = (Uint8)sum);
		gCPU_ClockCount -= 3; //v0.83更新。3サイクル消費(Kiさんから確かな実験情報)。
	}
	WRITEZP(_X, M);
}

/*-----------------------------------------------------------------------------
** SBC
*/
static inline void
sbc(Uint8		val)
{
	Uint32	flagC = (~_CF) & CPU_CF; // D0  v1.67更新Uint32に(おそらくわずかに高速化)。
	Uint32	temp = _A - val - flagC;
	Uint32	lo;
	Uint32	hi;

	if (_DF)
	{
		lo = (_A & 0x0F) - (val & 0x0F) - flagC;
		hi = (_A >> 4) - (val >> 4) - ((lo & 0x10) == 0x10);
		if (lo & 0x10)
			lo -= 6;
		if (hi & 0x10)
			hi -= 6;
		_CF = (Uint8)((~hi & 0x10) >> 4);
		_VF = (Uint8)(((_A ^ temp) & (_A ^ val) & CPU_NF) >> 1);
		updateFlagZN(_A = (Uint8)((lo & 0x0F) | (hi << 4)));
		gCPU_ClockCount--;
	}
	else
	{
		_CF = (Uint8)((~temp & 0x100) >> 8);
		_VF = (Uint8)(((_A ^ temp) & (_A ^ val) & CPU_NF) >> 1);
		updateFlagZN(_A = (Uint8)temp);
	}
}

/*-----------------------------------------------------------------------------
** AND
*/
static inline void
and(Uint8		val)
{
	_A &= val;
	updateFlagZN(_A);
}

/*-----------------------------------------------------------------------------
** AND with T-flag set
*/
static inline void
and_t(Uint8		val)
{
	Uint8 M = READZP(_X) & val;
	updateFlagZN(M);
	WRITEZP(_X, M);
	gCPU_ClockCount -= 3; //v0.83更新。3サイクル消費(Kiさんから確かな実験情報)。
}

/*-----------------------------------------------------------------------------
** ASL
*/
static inline Uint8
asl(Uint8		val)
{
	_CF = val >> 7; //Kitao更新。v1.07
	updateFlagZN(val = val << 1); //Kitao更新。v1.07
	return val;
}

/*-----------------------------------------------------------------------------
** LSR
*/
static inline Uint8
lsr(Uint8		val)
{
	_CF = val & CPU_CF;	// bit0
	updateFlagZN(val = val >> 1); //Kitao更新。v1.07
	return val;
}

/*-----------------------------------------------------------------------------
** ROL
*/
static inline Uint8
rol(Uint8		val)
{
	Uint8 oldFlagC = _CF; // bit0
	_CF = val >> 7; //Kitao更新。v1.07
	updateFlagZN(val = (val << 1) | oldFlagC);
	return val;
}

/*-----------------------------------------------------------------------------
** ROR
*/
static inline Uint8
ror(Uint8		val)
{
	Uint8 oldFlagC = _CF << 7;
	_CF = val & CPU_CF;
	updateFlagZN(val = (val >> 1) | oldFlagC);
	return val;
}

/*-----------------------------------------------------------------------------
** CMP
*/
static inline void
cmp(Uint8		val)
{
	Uint32 temp = _A - val;

	updateFlagZN((Uint8)temp);
	_CF = (Uint8)((~temp & 0x100) >> 8);
}

/*-----------------------------------------------------------------------------
** CPX
*/
static inline void
cpx(Uint8		val)
{
	Uint32 temp = _X - val;

	updateFlagZN((Uint8)temp);
	_CF = (Uint8)((~temp & 0x100) >> 8);
}

/*-----------------------------------------------------------------------------
** CPY
*/
static inline void
cpy(Uint8		val)
{
	Uint32 temp = _Y - val;

	updateFlagZN((Uint8)temp);
	_CF = (Uint8)((~temp & 0x100) >> 8);
}

/*-----------------------------------------------------------------------------
** EOR
*/
static inline void
eor(Uint8		val)
{
	_A ^= val;
	updateFlagZN(_A);
}

/*-----------------------------------------------------------------------------
** EOR with T-flag set
*/
static inline void
eor_t(Uint8		val)
{
	Uint8 M = READZP(_X) ^ val;
	updateFlagZN(M);
	WRITEZP(_X, M);
	gCPU_ClockCount -= 3; //v0.83更新。3サイクル消費(Kiさんから確かな実験情報)。
}

/*-----------------------------------------------------------------------------
** ORA
*/
static inline void
ora(Uint8		val)
{
	_A |= val;
	updateFlagZN(_A);
}

/*-----------------------------------------------------------------------------
** ORA with T-flag set
*/
static inline void
ora_t(Uint8		val)
{
	Uint8 M = READZP(_X) | val;
	updateFlagZN(M);
	WRITEZP(_X, M);
	gCPU_ClockCount -= 3; //v0.83更新。3サイクル消費(Kiさんから確かな実験情報)。
}

/*-----------------------------------------------------------------------------
** LDA
*/
static inline void
lda(Uint8		val)
{
	_A = _NF = _ZF = val; //v1.47更新
}

/*-----------------------------------------------------------------------------
** LDX
*/
static inline void
ldx(Uint8		val)
{
	_X = _NF = _ZF = val; //v1.47更新
}

/*-----------------------------------------------------------------------------
** LDY
*/
static inline void
ldy(Uint8		val)
{
	_Y = _NF = _ZF = val; //v1.47更新
}

/*-----------------------------------------------------------------------------
** TAX
*/
static inline void
tax()
{
	_X = _A;
	updateFlagZN(_X);
}

/*-----------------------------------------------------------------------------
** TAY
*/
static inline void
tay()
{
	_Y = _A;
	updateFlagZN(_Y);
}

/*-----------------------------------------------------------------------------
** TXA
*/
static inline void
txa()
{
	_A = _X;
	updateFlagZN(_A);
}

/*-----------------------------------------------------------------------------
** TYA
*/
static inline void
tya()
{
	_A = _Y;
	updateFlagZN(_A);
}

/*-----------------------------------------------------------------------------
** TSX
*/
static inline void
tsx()
{
	_X = _S;
	updateFlagZN(_X);
}

/*-----------------------------------------------------------------------------
** BBRi (Branch on Bit Reset)
*/
static inline void
BBRi(const Uint8 bit)
{
	Uint8 addr8;
	Sint8 rel8;

	addr8 = READINC(_PC);
	rel8 = (Sint8)READINC(_PC);
	if ((READZP(addr8) & bit) == 0) //v1.29更新
	{
		gCPU_ClockCount--;
		gCPU_ClockCount--;
		_PC += (Sint16)rel8;
	}
}

/*-----------------------------------------------------------------------------
** BBSi (Branch on Bit Set)
*/
static inline void
BBSi(const Uint8 bit)
{
	Uint8 addr8;
	Sint8 rel8;

	addr8 = READINC(_PC);
	rel8 = (Sint8)READINC(_PC);
	if (READZP(addr8) & bit) //v1.29更新
	{
		gCPU_ClockCount--;
		gCPU_ClockCount--;
		_PC += (Sint16)rel8;
	}
}

/*-----------------------------------------------------------------------------
** TRB
*/
static inline Uint8
trb(Uint8		val)
{
	Uint8	M = ~_A & val;

	_VF = _NF = val;
	_ZF = M;
	return M;
}

/*-----------------------------------------------------------------------------
** TSB
*/
static inline Uint8
tsb(Uint8		val)
{
	Uint8	M = _A | val;

	_VF = _NF = val;
	_ZF = M;
	return M;
}

/*-----------------------------------------------------------------------------
** TST
*/
static inline void
tst(Uint8 imm, Uint8 M)
{
	_VF = _NF = M;
	_ZF = M & imm;
}

/*-----------------------------------------------------------------------------
** RMBi (Reset Memory Bit)
*/
static inline void
RMBi(Uint8 zp, Uint8 bit)
{
	WRITEZP(zp, READZP(zp) & (~bit));
}

/*-----------------------------------------------------------------------------
** SMBi (Set Memory Bit)
*/
static inline void
SMBi(Uint8 zp, Uint8 bit)
{
	WRITEZP(zp, READZP(zp) | bit);
}


/*-----------------------------------------------------------------------------
** Check for pending NMI / TIMER / IRQ1 / IRQ2
*/
//Kitao更新。ラスタ割り込みタイミング(IRQ1)を重視するため、割り込みの優先順位を逆順とした。
static inline void
fetchInterrupt()
{
	if (_PrevIF == 0) //v1.61更新。命令後の割り込み処理をするかどうかの判断は、命令実行前のIフラグの状態が使われる。
	{
		if ((_bPrevIRQ2)&&((_PrevIntDisable & INTCTRL_IRQ2) == 0)) //v1.61更新。PrevIF同様に、命令実行前の割り込み禁止状態が使われる。
		{
			INTCTRL_Cancel(INTCTRL_IRQ2); //Kitao追加

			push(_PC >> 8);
			push(_PC & 0xFF);

			_BF = 0;
			push(gatherFlags());

			_IF = CPU_IF;
			refreshPrevIF(); //v1.61追加
			_TF = _DF = 0;
			_PC  = READ(CPU_IRQ2VECTOR);
			_PC |= READ(CPU_IRQ2VECTOR + 1) << 8;
			gCPU_ClockCount -= 8; //Kitao更新。HuC6280のBRKに合わせて8サイクルとした。
			return; //Kitao追加
		}
		
		if ((_bPrevIRQ1)&&((_PrevIntDisable & INTCTRL_IRQ1) == 0))
		{
			INTCTRL_Cancel(INTCTRL_IRQ1); //Kitao追加

			push(_PC >> 8);
			push(_PC & 0xFF);

			_BF = 0;
			push(gatherFlags());

			_IF = CPU_IF;
			refreshPrevIF(); //v1.61追加
			_TF = _DF = 0;
			_PC  = READ(CPU_IRQ1VECTOR);
			_PC |= READ(CPU_IRQ1VECTOR + 1) << 8;
			gCPU_ClockCount -= 8; //Kitao更新。HuC6280のBRKに合わせて8サイクルとした。
			return; //Kitao追加
		}
		
		if ((_bPrevTIRQ)&&((_PrevIntDisable & INTCTRL_TIRQ) == 0))
		{
			INTCTRL_Cancel(INTCTRL_TIRQ); //Kitao追加。クロスワイバーで必要。

			push(_PC >> 8);
			push(_PC & 0xFF);

			_BF = 0;
			push(gatherFlags());

			_IF = CPU_IF;
			refreshPrevIF(); //v1.61追加
			_TF = _DF = 0;
			_PC  = READ(CPU_TIMERVECTOR);
			_PC |= READ(CPU_TIMERVECTOR + 1) << 8;
			gCPU_ClockCount -= 8; //Kitao更新。HuC6280のBRKに合わせて8サイクルとした。
			return; //Kitao追加
		}
	}

/*	if (_bNMI) //Kitao更新。現在エミュレータ上では未使用
	{
		push(_PC >> 8);
		push(_PC & 0xFF);

		// B フラグはクリアされた後にスタックへ退避される。
		_BF = 0;
		push(gatherFlags());

		_IF = CPU_IF;
		refreshPrevIF(); //v1.61追加
		_TF = _DF = 0;
		_PC  = READ(CPU_NMIVECTOR);
		_PC |= READ(CPU_NMIVECTOR + 1) << 8;
		gCPU_ClockCount -= 8; //Kitao更新。HuC6280のBRKに合わせて8サイクルとした。
	}
*/
}


/******************************************************************************
**							   ↓外部公開関数
******************************************************************************/


/*-----------------------------------------------------------------------------
** [CPU_SetReadFunction]
**	 バンクごとのリード関数を登録します。(必須)
**---------------------------------------------------------------------------*/
void
CPU_SetReadFunction(Uint8 (*RdFunc)(Uint32, Uint32)) //Kitao更新。マッピングレジスタとアドレスを個別に受け取るようにして高速化。v1.47
{
	ReadMem = RdFunc;
}

/*-----------------------------------------------------------------------------
** [CPU_SetWriteFunction]
**	 バンクごとのライト関数を登録します。(必須)
**---------------------------------------------------------------------------*/
void
CPU_SetWriteFunction(void (*WrFunc)(Uint32, Uint32, Uint8)) //Kitao更新。マッピングレジスタとアドレスを個別に受け取るようにして高速化。v1.47
{
	WriteMem = WrFunc;
}


/*-----------------------------------------------------------------------------
** [Reset]
** ＣＰＵコアをリセットします。消費クロック数を返します。
**---------------------------------------------------------------------------*/
Sint32
CPU_Reset()
{
	int		i;

	for (i=0; i<256; i++)
	{
		_CycleTableSlow[i] = _CycleTableBase[i] << 2; //LowSpeedモード（CSL命令後）のときは４倍のサイクル数が掛かる
		_CycleTableTurbo1[i] = 2; //Ootake独自のTurboモード1のときは全ての命令実行を2サイクルとする。
		_CycleTableTurbo2[i] = 1; //Ootake独自のTurboモード2のときは全ての命令実行を1サイクルとする(v1.60以前のターボサイクル)。
	}
	SetSpeedLow(FALSE); //Kitao追加
	gCPU_Transfer = 0; //Kitao追加

	//Kitao追加
	_bRDY = FALSE;
	_bIRQ1 = FALSE;
	_bIRQ2 = FALSE;
	_bTIRQ = FALSE;
	_bNMI = FALSE;
	_IntDisable = 0;

	//Kitao追加。念のために電源再投入時は全フラグを初期化
	_CF = 0;
	_ZF = 0;
	_IF = CPU_IF; //v1.61更新
	refreshPrevIF(); //v1.61追加。_bPrevIRQx,_PrevIntDisableも更新される。
	_DF = 0;
	_BF = 0;
	_TF = 0;
	_VF = 0;
	_NF = 0;

	ZeroMemory(_MPR, sizeof(_MPR)); //Kitao更新。念のために電源再投入時はマッピングレジスタも全て初期化。

	//Kitao追加。念のために電源再投入時は全フラグを初期化
	_A = 0;
	_X = 0;
	_Y = 0;
	_S = 0;
	_P = 0;
	_PC = READ(CPU_RESETVECTOR);
	_PC |= READ(CPU_RESETVECTOR + 1) << 8;

	//Kitao追加。スーパーグラフィックス用
	_SelectVDC = 0;

	gCPU_ClockCount = -6;
	_ClockElapsed = 0;

	return 6;
}


/*-----------------------------------------------------------------------------
** [CPU_ActivateRDY]
**	 ＤＭＡ要求線ＲＤＹをアクティブにします。
**---------------------------------------------------------------------------*/
void
CPU_ActivateRDY()
{
	_bRDY = TRUE;
}

/*-----------------------------------------------------------------------------
** [CPU_ActivateNMI]
**	 割込み入力線ＮＭＩをアクティブにします。
**---------------------------------------------------------------------------*/
void
CPU_ActivateNMI()
{
	_bNMI = TRUE;
}

/*-----------------------------------------------------------------------------
** [CPU_ActivateTIMER]
**	 タイマー割込み要求線ＴＩＭＥＲをアクティブにします。
**---------------------------------------------------------------------------*/
void
CPU_ActivateTIMER()
{
	_bTIRQ = TRUE;
}

/*-----------------------------------------------------------------------------
** [CPU_ActivateIRQ1]
**	 割込み入力線ＩＲＱ１をアクティブにします。
**---------------------------------------------------------------------------*/
void
CPU_ActivateIRQ1()
{
	_bIRQ1 = TRUE;
}

/*-----------------------------------------------------------------------------
** [CPU_ActivateIRQ2]
**	 割込み入力線ＩＲＱ２をアクティブにします。
**---------------------------------------------------------------------------*/
void
CPU_ActivateIRQ2()
{
	_bIRQ2 = TRUE;
}

/*-----------------------------------------------------------------------------
** [CPU_InactivateRDY]
**	 ＤＭＡ要求線ＲＤＹを無効にします。
**---------------------------------------------------------------------------*/
void
CPU_InactivateRDY()
{
	_bRDY = FALSE;
}

/*-----------------------------------------------------------------------------
** [CPU_InactivateNMI]
**	 割込み入力線ＮＭＩを無効にします。
**---------------------------------------------------------------------------*/
void
CPU_InactivateNMI()
{
	_bNMI = FALSE;
}

/*-----------------------------------------------------------------------------
** [CPU_InactivateTIMER]
**	 タイマー割込み要求線ＴＩＭＥＲを無効にします。
**---------------------------------------------------------------------------*/
void
CPU_InactivateTIMER()
{
	_bTIRQ = FALSE;
}

/*-----------------------------------------------------------------------------
** [CPU_InactivateIRQ1]
**	 割込み入力線ＩＲＱ１を無効にします。
**---------------------------------------------------------------------------*/
void
CPU_InactivateIRQ1()
{
	_bIRQ1 = FALSE;
}

/*-----------------------------------------------------------------------------
** [CPU_InactivateIRQ2]
**	 割込み入力線ＩＲＱ２を無効にします。
**---------------------------------------------------------------------------*/
void
CPU_InactivateIRQ2()
{
	_bIRQ2 = FALSE;
}


/*-----------------------------------------------------------------------------
** [CPU_Setxx]
**	 レジスタの値を設定します。 
**---------------------------------------------------------------------------*/
void CPU_SetA(Uint8 A) { _A = A; }
void CPU_SetX(Uint8 X) { _X = X; }
void CPU_SetY(Uint8 Y) { _Y = Y; }
void CPU_SetS(Uint8 S) { _S = S; }
void CPU_SetP(Uint8 P) { _P = P; separateFlags(P); }
void CPU_SetPC(Uint16 PC) { _PC = PC; }
void CPU_SetMPR(Sint32 i, Uint32 mpr) { _MPR[i] = mpr; }


/*-----------------------------------------------------------------------------
** [CPU_Getxx]
**	 レジスタの値を取得します。 
**---------------------------------------------------------------------------*/
Uint8 CPU_GetA() { return _A; }
Uint8 CPU_GetX() { return _X; }
Uint8 CPU_GetY() { return _Y; }
Uint8 CPU_GetS() { return _S; }
Uint8 CPU_GetP() { _P = gatherFlags(); return _P; }
Uint16 CPU_GetPC() { return _PC; }
Uint32 CPU_GetMPR(Sint32 i) { return _MPR[i]; }


//Kitao追加
void
CPU_WriteMemory(
	Uint32	addr,
	Uint8	data)
{
	WRITE(addr, data);
}

//Kitao追加
void
CPU_WriteMemoryZero(
	Uint8	addr,
	Uint8	data)
{
	WRITEZP(addr, data);
}

//Kitao追加
void
CPU_WriteMemoryMpr(
	Uint32	mpr,
	Uint32	addr,
	Uint8	data,
	BOOL	bContinuous) //毎フレーム、値を更新し続けるならbContinuousをTRUEで呼ぶ。v2.39
{
	mpr  &= 0xFF;

	if (APP_GetCDGame())
	{
		if (mpr <= 0x3F)
			MAINBOARD_WriteROM(mpr, (addr & 0x1FFF), data); //ROM領域の場合
		else
			WriteMem(mpr, (addr & 0x1FFF), data);
	}
	else
	{
		if (mpr <= 0x7F)
			MAINBOARD_WriteROM(mpr, (addr & 0x1FFF), data); //ROM領域の場合
		else
			WriteMem(mpr, (addr & 0x1FFF), data);
	}
	
	MAINBOARD_SetContinuousWriteValue(bContinuous, mpr,addr,data); //※ここではaddrを丸め込む(&0x1FFF)前のそのままのアドレスを引き渡す
}


//Kitao追加。割り込み禁止の設定
void
CPU_SetIntDisable(
	Uint8	intDisable)
{
	_IntDisable = intDisable;
}


//Kitao追加。スーパーグラフィックス用。ST0,ST1,ST2命令がどちらのVDCをアクセス対象にするかを設定。
void
CPU_SelectVDC(
	Sint32	selectVDC)	//selectVDC…0か1
{
	_SelectVDC = selectVDC;
}


//Kitao追加。デバッグモードの設定。v1.07
void
CPU_SetDebug(
	BOOL	debug)
{
	_bDebug = debug;
}

//Kitao追加。現在実行中の命令コードを返す。デバッグ用。v1.07
Uint32
CPU_GetOpCode()
{
	return _OpCode;
}

//Kitao追加。デバッグ用。v1.07
Uint8
CPU_ReadCode(
	Uint16	pc)
{
	return READ(pc);
}

//Kitao追加。現在実行中の命令の_PCを返す。デバッグ用。v1.07
Uint16
CPU_GetPrevPC()
{
	return _PrevPC;
}

//Kitao追加。デバッグ用。v1.07
Uint8
CPU_GetPrevFlags()
{
	return _PrevFlags;
}

//Kitao追加。現在実行中の命令の消費クロック数を返す。VDCアクセス時にウェイトを入れるかどうかの判断で使用。※現在非使用
Sint32
CPU_GetClockElapsed()
{
	return _ClockElapsed;
}


//Kitao更新。タイミングを実機に近づけるために、CSL命令の実装と割り込み関連も変更した。
//			 割り込みフラグを見る位置を変更。CPU_AdvanceClock()を小さくしinlineにして高速化。v1.61。
void  //v0.94記。環境にもよると思うが大きすぎるところはinlineしないほうがPCのキャッシュに収まって安定した速さを保てていた。
CPU_ExecuteOperation()
{
	Uint8	ureg8;
	Uint8	addr8;
	Sint8	rel8;
	Uint16	addr16;

	//Kitao追加。実機のタイミングに合わせるため、ブロック転送をVDC等と並列に動作されるようにした。
	switch (gCPU_Transfer) //gCPU_Transfer…1=TII,2=TDD,3=TIN,4=TIA,5=TAI  v1.14更新。gCPU_Transferの判定を１つにして高速化。
	{
	case 0: //転送命令中ではない通常の場合
		if (_bDebug)
		{	//v1.07追加。デバッグウィンドウ表示中なら
			while (DEBUG_GetPause()) //デバッグウィンドウで停止設定中なら解除されるまで待つ
				Sleep(1);
			if (DEBUG_GetPauseLong()) //一時的な解除の場合
				DEBUG_SetPause(TRUE); //１命令実行後またポーズ
			
			fetchInterrupt();
			refreshPrevIF();
			_PrevPC = _PC;
			_PrevFlags = gatherFlags();
		}
		else //通常
		{
			//Kitao更新。命令を実行する前に割り込み処理を行うようにした。v1.61更新
			//			 割り込み状況のチェックは、ひとつ前の命令開始時のIフラグ＆割り込み信号状態を参照する。
			//			 ジャックニクラウスゴルフ(Huカード版)でショット時画面乱れが解消。スト２’で稀に１フレーム乱れることが解消。
			//			 フォーメーションサッカー'90のエンディングで必要。Iフラグと割り込み禁止だけでなくIRQもPrevIRQで見ることがメタルエンジェルで必要。
			fetchInterrupt();
			refreshPrevIF(); //次回の割り込み処理用に_PrevIF,_PrevIRQx,_PrevIntDisableに現在の状況を保管。v1.61追加
		}

		_OpCode = READINC(_PC);
		gCPU_ClockCount -= _pCycleTable[_OpCode]; //v1.61更新。消費は直接gCPU_ClockCountを引くようにした。高速化。
		//gCPU_ClockCount -= (_ClockElapsed = _pCycleTable[_OpCode]); //高速化のため現在非使用。デバッグ用

		//Kitao更新。よく使われる命令を先に持ってきて高速化した。v1.30。ループ内で使われそうな命令を先に。v1.63更新。
		switch (_OpCode)
		{
			/*-------------------------------------------------------------------**
			** register data transfer instructions
			**-------------------------------------------------------------------*/
			/*---- LDA ----------------------------------------------------------*/
			case CPU_INST_LDA_IMM:
				ureg8 = READINC(_PC);
				lda(ureg8);
				break;

			case CPU_INST_LDA_ZP:
				addr8 = READINC(_PC);
				lda(READZP(addr8));
				break;

			case CPU_INST_LDA_ZP_X:
				addr8 = READINC_X(_PC);
				lda(READZP(addr8));
				break;

			case CPU_INST_LDA_IND:
				addr16 = fetchZpIndirect();
				lda(READ(addr16));
				break;

			case CPU_INST_LDA_IND_X:
				addr16 = fetchZpIndexIndirect();
				lda(READ(addr16));
				break;

			case CPU_INST_LDA_IND_Y:
				addr16 = fetchZpIndirectIndex();
				lda(READ(addr16));
				break;

			case CPU_INST_LDA_ABS:
				addr16 = fetchAbs();
				lda(READ(addr16));
				break;

			case CPU_INST_LDA_ABS_X:
				addr16 = fetchAbsX();
				lda(READ(addr16));
				break;

			case CPU_INST_LDA_ABS_Y:
				addr16 = fetchAbsY();
				lda(READ(addr16));
				break;

			/*---- LDX ----------------------------------------------------------*/
			case CPU_INST_LDX_IMM:
				ureg8 = READINC(_PC);
				ldx(ureg8);
				break;

			case CPU_INST_LDX_ZP:
				addr8 = READINC(_PC);
				ldx(READZP(addr8));
				break;

			case CPU_INST_LDX_ZP_Y:
				addr8 = READINC_Y(_PC);
				ldx(READZP(addr8));
				break;

			case CPU_INST_LDX_ABS:
				addr16 = fetchAbs();
				ldx(READ(addr16));
				break;

			case CPU_INST_LDX_ABS_Y:
				addr16 = fetchAbsY();
				ldx(READ(addr16));
				break;

			/*---- LDY ----------------------------------------------------------*/
			case CPU_INST_LDY_IMM:
				ureg8 = READINC(_PC);
				ldy(ureg8);
				break;

			case CPU_INST_LDY_ZP:
				addr8 = READINC(_PC);
				ldy(READZP(addr8));
				break;

			case CPU_INST_LDY_ZP_X:
				addr8 = READINC_X(_PC);
				ldy(READZP(addr8));
				break;

			case CPU_INST_LDY_ABS:
				addr16 = fetchAbs();
				ldy(READ(addr16));
				break;

			case CPU_INST_LDY_ABS_X:
				addr16 = fetchAbsX();
				ldy(READ(addr16));
				break;

			/*---- STA ----------------------------------------------------------*/
			case CPU_INST_STA_ZP:
				addr8 = READINC(_PC);
				WRITEZP(addr8, _A);
				break;

			case CPU_INST_STA_ZP_X:
				addr8 = READINC_X(_PC);
				WRITEZP(addr8, _A);
				break;

			case CPU_INST_STA_IND:
				addr16 = fetchZpIndirect();
				WRITE(addr16, _A);
				break;

			case CPU_INST_STA_IND_X:
				addr16 = fetchZpIndexIndirect();
				WRITE(addr16, _A);
				break;

			case CPU_INST_STA_IND_Y:
				addr16 = fetchZpIndirectIndex();
				WRITE(addr16, _A);
				break;

			case CPU_INST_STA_ABS:
				addr16 = fetchAbs();
				WRITE(addr16, _A);
				break;

			case CPU_INST_STA_ABS_X:
				addr16 = fetchAbsX();
				WRITE(addr16, _A);
				break;

			case CPU_INST_STA_ABS_Y:
				addr16 = fetchAbsY();
				WRITE(addr16, _A);
				break;

			/*---- STX ----------------------------------------------------------*/
			case CPU_INST_STX_ZP:
				addr8 = READINC(_PC);
				WRITEZP(addr8, _X);
				break;

			case CPU_INST_STX_ZP_Y:
				addr8 = READINC_Y(_PC);
				WRITEZP(addr8, _X);
				break;

			case CPU_INST_STX_ABS:
				addr16 = fetchAbs();
				WRITE(addr16, _X);
				break;

			/*---- STY ----------------------------------------------------------*/
			case CPU_INST_STY_ZP:
				addr8 = READINC(_PC);
				WRITEZP(addr8, _Y);
				break;

			case CPU_INST_STY_ZP_X:
				addr8 = READINC_X(_PC);
				WRITEZP(addr8, _Y);
				break;

			case CPU_INST_STY_ABS:
				addr16 = fetchAbs();
				WRITE(addr16, _Y);
				break;

			/*---- ST0 ----------------------------------------------------------*/
			case CPU_INST_ST0_IMM:
				ureg8 = READINC(_PC);
				if (_SelectVDC == 0) //Kitao更新。スーパーグラフィックス用
					WriteMem(0xFF, 0x0000, ureg8);
				else
					WriteMem(0xFF, 0x0010, ureg8);
				break;

			/*---- ST1 ----------------------------------------------------------*/
			case CPU_INST_ST1_IMM:
				ureg8 = READINC(_PC);
				if (_SelectVDC == 0) //Kitao更新。スーパーグラフィックス用
					WriteMem(0xFF, 0x0002, ureg8);
				else
					WriteMem(0xFF, 0x0012, ureg8);
				break;

			/*---- ST2 ----------------------------------------------------------*/
			case CPU_INST_ST2_IMM:
				ureg8 = READINC(_PC);
				if (_SelectVDC == 0) //Kitao更新。スーパーグラフィックス用
					WriteMem(0xFF, 0x0003, ureg8);
				else
					WriteMem(0xFF, 0x0013, ureg8);
				break;

			/*---- STZ ----------------------------------------------------------*/
			case CPU_INST_STZ_ZP:
				addr8 = READINC(_PC);
				WRITEZP(addr8, 0);
				break;

			case CPU_INST_STZ_ZP_X:
				addr8 = READINC_X(_PC);
				WRITEZP(addr8, 0);
				break;

			case CPU_INST_STZ_ABS:
				addr16 = fetchAbs();
				WRITE(addr16, 0);
				break;

			case CPU_INST_STZ_ABS_X:
				addr16 = fetchAbsX();
				WRITE(addr16, 0);
				break;

			/*-------------------------------------------------------------------**
			** flag instructions
			**-------------------------------------------------------------------*/
			/*---- CLC ----------------------------------------------------------*/
			case CPU_INST_CLC:
				_CF = 0;
				break;

			/*---- CLD ----------------------------------------------------------*/
			case CPU_INST_CLD:
				_DF = 0;
				break;

			/*---- CLI ----------------------------------------------------------*/
			case CPU_INST_CLI:
				//Kitao更新。Iフラグクリア直後もIRQ のフェッチを行なう。ネクロスの要塞のオープニングでフリーズしないために必要。
				//			 割り込み動作をシンプルにしたので、ジャッキーチェンも問題なく起動。
				_IF = 0;
				break;

			/*---- CLV ----------------------------------------------------------*/
			case CPU_INST_CLV:
				_VF = 0;
				break;

			/*---- SEC ----------------------------------------------------------*/
			case CPU_INST_SEC:
				_CF = CPU_CF;
				break;

			/*---- SED ----------------------------------------------------------*/
			case CPU_INST_SED:
				_DF = CPU_DF;
				break;

			/*---- SEI ----------------------------------------------------------*/
			case CPU_INST_SEI:
				//Kitao追記。参考：Iフラグセット前に割り込みフェッチを行うと、ファイヤープロレスリングで画面化けが起こる。
				_IF = CPU_IF;
				break;

			/*---- SET ----------------------------------------------------------*/
			case CPU_INST_SET:
				_TF = CPU_TF;
				return; // Tフラグをリセットせずに次の命令へ

			/*-------------------------------------------------------------------**
			** ALU instructions
			**-------------------------------------------------------------------*/
			/*---- ADC ----------------------------------------------------------*/
			case CPU_INST_ADC_IMM:
				ureg8 = READINC(_PC);
				if (_TF)
					adc_t(ureg8);
				else
					adc(ureg8);
				break;

			case CPU_INST_ADC_ZP:
				ureg8 = READINC(_PC);
				if (_TF)
					adc_t(READZP(ureg8));
				else
					adc(READZP(ureg8));
				break;

			case CPU_INST_ADC_ZP_X:
				ureg8 = READINC_X(_PC);
				if (_TF)
					adc_t(READZP(ureg8));
				else
					adc(READZP(ureg8));
				break;

			case CPU_INST_ADC_IND:
				addr16 = fetchZpIndirect();
				if (_TF)
					adc_t(READ(addr16)); //Kitao更新。ureg8への代入を省略して高速化。以下の命令も同様に更新。v1.08
				else
					adc(READ(addr16));
				break;

			case CPU_INST_ADC_IND_X:
				addr16 = fetchZpIndexIndirect();
				if (_TF)
					adc_t(READ(addr16));
				else
					adc(READ(addr16));
				break;

			case CPU_INST_ADC_IND_Y:
				addr16 = fetchZpIndirectIndex();
				if (_TF)
					adc_t(READ(addr16));
				else
					adc(READ(addr16));
				break;

			case CPU_INST_ADC_ABS:
				addr16 = fetchAbs();
				if (_TF)
					adc_t(READ(addr16));
				else
					adc(READ(addr16));
				break;

			case CPU_INST_ADC_ABS_X:
				addr16 = fetchAbsX();
				if (_TF)
					adc_t(READ(addr16));
				else
					adc(READ(addr16));
				break;

			case CPU_INST_ADC_ABS_Y:
				addr16 = fetchAbsY();
				if (_TF)
					adc_t(READ(addr16));
				else
					adc(READ(addr16));
				break;

			/*---- SBC ----------------------------------------------------------*/
			case CPU_INST_SBC_IMM:
				ureg8 = READINC(_PC);
				sbc(ureg8);
				break;

			case CPU_INST_SBC_ZP:
				ureg8 = READINC(_PC);
				sbc(READZP(ureg8));
				break;

			case CPU_INST_SBC_ZP_X:
				ureg8 = READINC_X(_PC);
				sbc(READZP(ureg8));
				break;

			case CPU_INST_SBC_IND:
				addr16 = fetchZpIndirect();
				sbc(READ(addr16));
				break;

			case CPU_INST_SBC_IND_X:
				addr16 = fetchZpIndexIndirect();
				sbc(READ(addr16));
				break;

			case CPU_INST_SBC_IND_Y:
				addr16 = fetchZpIndirectIndex();
				sbc(READ(addr16));
				break;

			case CPU_INST_SBC_ABS:
				addr16 = fetchAbs();
				sbc(READ(addr16));
				break;

			case CPU_INST_SBC_ABS_X:
				addr16 = fetchAbsX();
				sbc(READ(addr16));
				break;

			case CPU_INST_SBC_ABS_Y:
				addr16 = fetchAbsY();
				sbc(READ(addr16));
				break;

			/*---- AND ----------------------------------------------------------*/
			case CPU_INST_AND_IMM:
				ureg8 = READINC(_PC);
				if (_TF)
					and_t(ureg8);
				else
					and(ureg8);
				break;

			case CPU_INST_AND_ZP:
				ureg8 = READINC(_PC);
				if (_TF)
					and_t(READZP(ureg8));
				else
					and(READZP(ureg8));
				break;

			case CPU_INST_AND_ZP_X:
				ureg8 = READINC_X(_PC);
				if (_TF)
					and_t(READZP(ureg8));
				else
					and(READZP(ureg8));
				break;

			case CPU_INST_AND_IND:
				addr16 = fetchZpIndirect();
				if (_TF)
					and_t(READ(addr16));
				else
					and(READ(addr16));
				break;

			case CPU_INST_AND_IND_X:
				addr16 = fetchZpIndexIndirect();
				if (_TF)
					and_t(READ(addr16));
				else
					and(READ(addr16));
				break;

			case CPU_INST_AND_IND_Y:
				addr16 = fetchZpIndirectIndex();
				if (_TF)
					and_t(READ(addr16));
				else
					and(READ(addr16));
				break;

			case CPU_INST_AND_ABS:
				addr16 = fetchAbs();
				if (_TF)
					and_t(READ(addr16));
				else
					and(READ(addr16));
				break;

			case CPU_INST_AND_ABS_X:
				addr16 = fetchAbsX();
				if (_TF)
					and_t(READ(addr16));
				else
					and(READ(addr16));
				break;

			case CPU_INST_AND_ABS_Y:
				addr16 = fetchAbsY();
				if (_TF)
					and_t(READ(addr16));
				else
					and(READ(addr16));
				break;

			/*---- ORA ----------------------------------------------------------*/
			case CPU_INST_ORA_IMM:
				ureg8 = READINC(_PC);
				if (_TF)
					ora_t(ureg8);
				else
					ora(ureg8);
				break;

			case CPU_INST_ORA_ZP:
				ureg8 = READINC(_PC);
				if (_TF)
					ora_t(READZP(ureg8));
				else
					ora(READZP(ureg8));
				break;

			case CPU_INST_ORA_ZP_X:
				ureg8 = READINC_X(_PC);
				if (_TF)
					ora_t(READZP(ureg8));
				else
					ora(READZP(ureg8));
				break;

			case CPU_INST_ORA_IND:
				addr16 = fetchZpIndirect();
				if (_TF)
					ora_t(READ(addr16));
				else
					ora(READ(addr16));
				break;

			case CPU_INST_ORA_IND_X:
				addr16 = fetchZpIndexIndirect();
				if (_TF)
					ora_t(READ(addr16));
				else
					ora(READ(addr16));
				break;

			case CPU_INST_ORA_IND_Y:
				addr16 = fetchZpIndirectIndex();
				if (_TF)
					ora_t(READ(addr16));
				else
					ora(READ(addr16));
				break;

			case CPU_INST_ORA_ABS:
				addr16 = fetchAbs();
				if (_TF)
					ora_t(READ(addr16));
				else
					ora(READ(addr16));
				break;

			case CPU_INST_ORA_ABS_X:
				addr16 = fetchAbsX();
				if (_TF)
					ora_t(READ(addr16));
				else
					ora(READ(addr16));
				break;

			case CPU_INST_ORA_ABS_Y:
				addr16 = fetchAbsY();
				if (_TF)
					ora_t(READ(addr16));
				else
					ora(READ(addr16));
				break;

			/*---- EOR ----------------------------------------------------------*/
			case CPU_INST_EOR_IMM:
				ureg8 = READINC(_PC);
				if (_TF)
					eor_t(ureg8);
				else
					eor(ureg8);
				break;

			case CPU_INST_EOR_ZP:
				ureg8 = READINC(_PC);
				if (_TF)
					eor_t(READZP(ureg8));
				else
					eor(READZP(ureg8));
				break;

			case CPU_INST_EOR_ZP_X:
				ureg8 = READINC_X(_PC);
				if (_TF)
					eor_t(READZP(ureg8));
				else
					eor(READZP(ureg8));
				break;

			case CPU_INST_EOR_IND:
				addr16 = fetchZpIndirect();
				if (_TF)
					eor_t(READ(addr16));
				else
					eor(READ(addr16));
				break;

			case CPU_INST_EOR_IND_X:
				addr16 = fetchZpIndexIndirect();
				if (_TF)
					eor_t(READ(addr16));
				else
					eor(READ(addr16));
				break;

			case CPU_INST_EOR_IND_Y:
				addr16 = fetchZpIndirectIndex();
				if (_TF)
					eor_t(READ(addr16));
				else
					eor(READ(addr16));
				break;

			case CPU_INST_EOR_ABS:
				addr16 = fetchAbs();
				if (_TF)
					eor_t(READ(addr16));
				else
					eor(READ(addr16));
				break;

			case CPU_INST_EOR_ABS_X:
				addr16 = fetchAbsX();
				if (_TF)
					eor_t(READ(addr16));
				else
					eor(READ(addr16));
				break;

			case CPU_INST_EOR_ABS_Y:
				addr16 = fetchAbsY();
				if (_TF)
					eor_t(READ(addr16));
				else
					eor(READ(addr16));
				break;

			/*---- INC ----------------------------------------------------------*/
			case CPU_INST_INC_ACCUM:
				updateFlagZN(++_A);
				break;

			case CPU_INST_INC_ZP:
				addr8 = READINC(_PC);
				ureg8 = READZP(addr8);
				updateFlagZN(++ureg8);
				WRITEZP(addr8, ureg8);
				break;

			case CPU_INST_INC_ZP_X:
				addr8 = READINC_X(_PC);
				ureg8 = READZP(addr8);
				updateFlagZN(++ureg8);
				WRITEZP(addr8, ureg8);
				break;

			case CPU_INST_INC_ABS:
				addr16 = fetchAbs();
				ureg8 = READ(addr16);
				updateFlagZN(++ureg8);
				WRITE(addr16, ureg8);
				break;

			case CPU_INST_INC_ABS_X:
				addr16 = fetchAbsX();
				ureg8 = READ(addr16);
				updateFlagZN(++ureg8);
				WRITE(addr16, ureg8);
				break;

			/*---- INX ----------------------------------------------------------*/
			case CPU_INST_INX:
				updateFlagZN(++_X);
				break;

			/*---- INY ----------------------------------------------------------*/
			case CPU_INST_INY:
				updateFlagZN(++_Y);
				break;

			/*---- DEC ----------------------------------------------------------*/
			case CPU_INST_DEC_ACCUM:
				updateFlagZN(--_A);
				break;

			case CPU_INST_DEC_ZP:
				addr8 = READINC(_PC);
				ureg8 = READZP(addr8);
				updateFlagZN(--ureg8);
				WRITEZP(addr8, ureg8);
				break;

			case CPU_INST_DEC_ZP_X:
				addr8 = READINC_X(_PC);
				ureg8 = READZP(addr8);
				updateFlagZN(--ureg8);
				WRITEZP(addr8, ureg8);
				break;

			case CPU_INST_DEC_ABS:
				addr16 = fetchAbs();
				ureg8 = READ(addr16);
				updateFlagZN(--ureg8);
				WRITE(addr16, ureg8);
				break;

			case CPU_INST_DEC_ABS_X:
				addr16 = fetchAbsX();
				ureg8 = READ(addr16);
				updateFlagZN(--ureg8);
				WRITE(addr16, ureg8);
				break;

			/*---- DEX ----------------------------------------------------------*/
			case CPU_INST_DEX:
				updateFlagZN(--_X);
				break;
					
			/*---- DEY ----------------------------------------------------------*/
			case CPU_INST_DEY:
				updateFlagZN(--_Y);
				break;

			/*---- CMP ----------------------------------------------------------*/
			case CPU_INST_CMP_IMM:
				ureg8 = READINC(_PC);
				cmp(ureg8);
				break;

			case CPU_INST_CMP_ZP:
				ureg8 = READINC(_PC);
				cmp(READZP(ureg8));
				break;

			case CPU_INST_CMP_ZP_X:
				ureg8 = READINC_X(_PC);
				cmp(READZP(ureg8));
				break;

			case CPU_INST_CMP_IND:
				addr16 = fetchZpIndirect();
				cmp(READ(addr16));
				break;

			case CPU_INST_CMP_IND_X:
				addr16 = fetchZpIndexIndirect();
				cmp(READ(addr16));
				break;

			case CPU_INST_CMP_IND_Y:
				addr16 = fetchZpIndirectIndex();
				cmp(READ(addr16));
				break;

			case CPU_INST_CMP_ABS:
				addr16 = fetchAbs();
				cmp(READ(addr16));
				break;

			case CPU_INST_CMP_ABS_X:
				addr16 = fetchAbsX();
				cmp(READ(addr16));
				break;

			case CPU_INST_CMP_ABS_Y:
				addr16 = fetchAbsY();
				cmp(READ(addr16));
				break;

			/*---- CPX ----------------------------------------------------------*/
			case CPU_INST_CPX_IMM:
				ureg8 = READINC(_PC);
				cpx(ureg8);
				break;

			case CPU_INST_CPX_ZP:
				ureg8 = READINC(_PC);
				cpx(READZP(ureg8));
				break;
			
			case CPU_INST_CPX_ABS:
				addr16 = fetchAbs();
				cpx(READ(addr16));
				break;

			/*---- CPY ----------------------------------------------------------*/
			case CPU_INST_CPY_IMM:
				ureg8 = READINC(_PC);
				cpy(ureg8);
				break;

			case CPU_INST_CPY_ZP:
				ureg8 = READINC(_PC);
				cpy(READZP(ureg8));
				break;
			
			case CPU_INST_CPY_ABS:
				addr16 = fetchAbs();
				cpy(READ(addr16));
				break;

			/*---- ASL ----------------------------------------------------------*/
			case CPU_INST_ASL_ACCUM:
				_A = asl(_A);
				break;

			case CPU_INST_ASL_ZP:
				addr8 = READINC(_PC);
				WRITEZP(addr8, asl(READZP(addr8)));
				break;

			case CPU_INST_ASL_ZP_X:
				addr8 = READINC_X(_PC);
				WRITEZP(addr8, asl(READZP(addr8)));
				break;

			case CPU_INST_ASL_ABS:
				addr16 = fetchAbs();
				WRITE(addr16, asl(READ(addr16)));
				break;
			
			case CPU_INST_ASL_ABS_X:
				addr16 = fetchAbsX();
				WRITE(addr16, asl(READ(addr16)));
				break;

			/*---- LSR ----------------------------------------------------------*/
			case CPU_INST_LSR_ACCUM:
				_A = lsr(_A);
				break;

			case CPU_INST_LSR_ZP:
				addr8 = READINC(_PC);
				WRITEZP(addr8, lsr(READZP(addr8)));
				break;

			case CPU_INST_LSR_ZP_X:
				addr8 = READINC_X(_PC);
				WRITEZP(addr8, lsr(READZP(addr8)));
				break;

			case CPU_INST_LSR_ABS:
				addr16 = fetchAbs();
				WRITE(addr16, lsr(READ(addr16)));
				break;
			
			case CPU_INST_LSR_ABS_X:
				addr16 = fetchAbsX();
				WRITE(addr16, lsr(READ(addr16)));
				break;

			/*---- ROL ----------------------------------------------------------*/
			case CPU_INST_ROL_ACCUM:
				_A = rol(_A);
				break;

			case CPU_INST_ROL_ZP:
				addr8 = READINC(_PC);
				WRITEZP(addr8, rol(READZP(addr8)));
				break;

			case CPU_INST_ROL_ZP_X:
				addr8 = READINC_X(_PC);
				WRITEZP(addr8, rol(READZP(addr8)));
				break;

			case CPU_INST_ROL_ABS:
				addr16 = fetchAbs();
				WRITE(addr16, rol(READ(addr16)));
				break;
			
			case CPU_INST_ROL_ABS_X:
				addr16 = fetchAbsX();
				WRITE(addr16, rol(READ(addr16)));
				break;

			/*---- ROR ----------------------------------------------------------*/
			case CPU_INST_ROR_ACCUM:
				_A = ror(_A);
				break;

			case CPU_INST_ROR_ZP:
				addr8 = READINC(_PC);
				WRITEZP(addr8, ror(READZP(addr8)));
				break;

			case CPU_INST_ROR_ZP_X:
				addr8 = READINC_X(_PC);
				WRITEZP(addr8, ror(READZP(addr8)));
				break;

			case CPU_INST_ROR_ABS:
				addr16 = fetchAbs();
				WRITE(addr16, ror(READ(addr16)));
				break;

			case CPU_INST_ROR_ABS_X:
				addr16 = fetchAbsX();
				WRITE(addr16, ror(READ(addr16)));
				break;

			/*---- CLA ----------------------------------------------------------*/
			case CPU_INST_CLA:
				_A = 0;
				break;

			/*---- CLX ----------------------------------------------------------*/
			case CPU_INST_CLX:
				_X = 0;
				break;

			/*---- CLY ----------------------------------------------------------*/
			case CPU_INST_CLY:
				_Y = 0;
				break;

			/*---- TAX ----------------------------------------------------------*/
			case CPU_INST_TAX:
				tax();
				break;

			/*---- TAY ----------------------------------------------------------*/
			case CPU_INST_TAY:
				tay();
				break;

			/*---- TSX ----------------------------------------------------------*/
			case CPU_INST_TSX:
				tsx();
				break;

			/*---- TXA ----------------------------------------------------------*/
			case CPU_INST_TXA:
				txa();
				break;

			/*---- TXS ----------------------------------------------------------*/
			case CPU_INST_TXS:
				_S = _X;
				break;

			/*---- TYA ----------------------------------------------------------*/
			case CPU_INST_TYA:
				tya();
				break;

			/*---- SAX ----------------------------------------------------------*/
			case CPU_INST_SAX:
				ureg8 = _A;
				_A = _X;
				_X = ureg8;
				break;

			/*---- SAY ----------------------------------------------------------*/
			case CPU_INST_SAY:
				ureg8 = _A;
				_A = _Y;
				_Y = ureg8;
				break;

			/*---- SXY ----------------------------------------------------------*/
			case CPU_INST_SXY:
				ureg8 = _X;
				_X = _Y;
				_Y = ureg8;
				break;

			/*-------------------------------------------------------------------**
			** test instructions
			**-------------------------------------------------------------------*/
			/*---- BIT ----------------------------------------------------------*/
			case CPU_INST_BIT_IMM:
				ureg8 = READINC(_PC);
				bit(ureg8);
				break;

			case CPU_INST_BIT_ZP:
				addr8 = READINC(_PC);
				bit(READZP(addr8));
				break;

			case CPU_INST_BIT_ZP_X:
				addr8 = READINC_X(_PC);
				bit(READZP(addr8));
				break;

			case CPU_INST_BIT_ABS:
				addr16 = fetchAbs();
				bit(READ(addr16));
				break;

			case CPU_INST_BIT_ABS_X:
				addr16 = fetchAbsX();
				bit(READ(addr16));
				break;

			/*---- TRB ----------------------------------------------------------*/
			case CPU_INST_TRB_ZP:
				addr8 = READINC(_PC);
				WRITEZP(addr8, trb(READZP(addr8)));
				break;

			case CPU_INST_TRB_ABS:
				addr16 = fetchAbs();
				WRITE(addr16, trb(READ(addr16)));
				break;

			/*---- TSB ----------------------------------------------------------*/
			case CPU_INST_TSB_ZP:
				addr8 = READINC(_PC);
				WRITEZP(addr8, tsb(READZP(addr8)));
				break;

			case CPU_INST_TSB_ABS:
				addr16 = fetchAbs();
				WRITE(addr16, tsb(READ(addr16)));
				break;

			/*---- TST ----------------------------------------------------------*/
			case CPU_INST_TST_IMM_ZP:
				ureg8 = READINC(_PC);
				addr8 = READINC(_PC);
				tst(ureg8, READZP(addr8));
				break;

			case CPU_INST_TST_IMM_ZP_X:
				ureg8 = READINC(_PC);
				addr8 = READINC_X(_PC);
				tst(ureg8, READZP(addr8));
				break;

			case CPU_INST_TST_IMM_ABS:
				ureg8 = READINC(_PC);
				addr16 = fetchAbs();
				tst(ureg8, READ(addr16));
				break;

			case CPU_INST_TST_IMM_ABS_X:
				ureg8 = READINC(_PC);
				addr16 = fetchAbsX();
				tst(ureg8, READ(addr16));
				break;

			/*-------------------------------------------------------------------**
			** branch / jump instructions
			**-------------------------------------------------------------------*/
			/*---- BCC ----------------------------------------------------------*/
			case CPU_INST_BCC_REL:
				rel8 = (Sint8)READINC(_PC);
				if (_CF == 0)
				{
					gCPU_ClockCount--;
					gCPU_ClockCount--;
					_PC += (Sint16)rel8;
				}
				break;

			/*---- BNE ----------------------------------------------------------*/
			case CPU_INST_BNE_REL:
				rel8 = (Sint8)READINC(_PC);
				if (_ZF) //ZFは0の場合が真(ゼロ)
				{
					gCPU_ClockCount--;
					gCPU_ClockCount--;
					_PC += (Sint16)rel8;
				}
				break;

			/*---- BVC ----------------------------------------------------------*/
			case CPU_INST_BVC_REL:
				rel8 = (Sint8)READINC(_PC);
				if ((_VF & CPU_VF) == 0)
				{
					gCPU_ClockCount--;
					gCPU_ClockCount--;
					_PC += (Sint16)rel8;
				}
				break;

			/*---- BPL ----------------------------------------------------------*/
			case CPU_INST_BPL_REL:
				rel8 = (Sint8)READINC(_PC);
				if ((_NF & CPU_NF) == 0)
				{
					gCPU_ClockCount--;
					gCPU_ClockCount--;
					_PC += (Sint16)rel8;
				}
				break;

			/*---- BCS ----------------------------------------------------------*/
			case CPU_INST_BCS_REL:
				rel8 = (Sint8)READINC(_PC);
				if (_CF)
				{
					gCPU_ClockCount--;
					gCPU_ClockCount--;
					_PC += (Sint16)rel8;
				}
				break;

			/*---- BEQ ----------------------------------------------------------*/
			case CPU_INST_BEQ_REL:
				rel8 = (Sint8)READINC(_PC);
				if (_ZF == 0) //ZFは0の場合が真(ゼロ)
				{
					gCPU_ClockCount--;
					gCPU_ClockCount--;
					_PC += (Sint16)rel8;
				}
				break;

			/*---- BVS ----------------------------------------------------------*/
			case CPU_INST_BVS_REL:
				rel8 = (Sint8)READINC(_PC);
				if (_VF & CPU_VF)
				{
					gCPU_ClockCount--;
					gCPU_ClockCount--;
					_PC += (Sint16)rel8;
				}
				break;

			/*---- BMI ----------------------------------------------------------*/
			case CPU_INST_BMI_REL:
				rel8 = (Sint8)READINC(_PC);
				if (_NF & CPU_NF)
				{
					gCPU_ClockCount--;
					gCPU_ClockCount--;
					_PC += (Sint16)rel8;
				}
				break;

			/*---- BRA ----------------------------------------------------------*/
			case CPU_INST_BRA_REL:
				rel8 = (Sint8)READINC(_PC);
				_PC += (Sint16)rel8;
				break;

			/*---- JMP ----------------------------------------------------------*/
			case CPU_INST_JMP_ABS:
				_PC = fetchAbs();
				break;

			case CPU_INST_JMP_INDIR:
				_PC = fetchAbsIndirect();
				break;

			case CPU_INST_JMP_INDIRX:
				_PC = fetchAbsIndirectX();
				break;

			/*---- BBRi ---------------------------------------------------------*/
			case CPU_INST_BBR0_ZP_REL:
				BBRi(0x01);
				break;

			case CPU_INST_BBR1_ZP_REL:
				BBRi(0x02);
				break;

			case CPU_INST_BBR2_ZP_REL:
				BBRi(0x04);
				break;

			case CPU_INST_BBR3_ZP_REL:
				BBRi(0x08);
				break;

			case CPU_INST_BBR4_ZP_REL:
				BBRi(0x10);
				break;

			case CPU_INST_BBR5_ZP_REL:
				BBRi(0x20);
				break;

			case CPU_INST_BBR6_ZP_REL:
				BBRi(0x40);
				break;

			case CPU_INST_BBR7_ZP_REL:
				BBRi(0x80);
				break;

			/*---- BBSi ---------------------------------------------------------*/
			case CPU_INST_BBS0_ZP_REL:
				BBSi(0x01);
				break;

			case CPU_INST_BBS1_ZP_REL:
				BBSi(0x02);
				break;

			case CPU_INST_BBS2_ZP_REL:
				BBSi(0x04);
				break;

			case CPU_INST_BBS3_ZP_REL:
				BBSi(0x08);
				break;

			case CPU_INST_BBS4_ZP_REL:
				BBSi(0x10);
				break;

			case CPU_INST_BBS5_ZP_REL:
				BBSi(0x20);
				break;

			case CPU_INST_BBS6_ZP_REL:
				BBSi(0x40);
				break;

			case CPU_INST_BBS7_ZP_REL:
				BBSi(0x80);
				break;

			/*-------------------------------------------------------------------**
			** subroutine instructions
			**-------------------------------------------------------------------*/
			/*---- BSR ----------------------------------------------------------*/
			case CPU_INST_BSR_REL: //v1.59更新
				push(_PC >> 8);
				push(_PC & 0xFF);
				rel8 = (Sint8)READINC(_PC);
				_PC += (Sint16)rel8;
				break;

			/*---- JSR ----------------------------------------------------------*/
			case CPU_INST_JSR_ABS: //v1.59更新
				++_PC;
				push(_PC >> 8);
				push(_PC & 0xFF);
				--_PC;
				_PC = fetchAbs();
				break;

			/*---- PHA ----------------------------------------------------------*/
			case CPU_INST_PHA:
				push(_A);
				break;

			/*---- PHP ----------------------------------------------------------*/
			case CPU_INST_PHP:
				_TF = 0;
				push(gatherFlags());
				break;

			/*---- PHX ----------------------------------------------------------*/
			case CPU_INST_PHX:
				push(_X);
				break;

			/*---- PHY ----------------------------------------------------------*/
			case CPU_INST_PHY:
				push(_Y);
				break;

			/*---- PLA ----------------------------------------------------------*/
			case CPU_INST_PLA:
				_A = pull();
				updateFlagZN(_A);
				break;

			/*---- PLP ----------------------------------------------------------*/
			// PLP 命令実行直後は：
			// - T フラグをリセットしない 
			case CPU_INST_PLP:
				separateFlags(pull());
				_BF = CPU_BF; //v1.61追加。Thanks Aladar's information.
				refreshPrevIF(); //v1.61追加。おそらくRTIと同じ機構。
				return;

			/*---- PLX ----------------------------------------------------------*/
			case CPU_INST_PLX:
				_X = pull();
				updateFlagZN(_X);
				break;

			/*---- PLY ----------------------------------------------------------*/
			case CPU_INST_PLY:
				_Y = pull();
				updateFlagZN(_Y);
				break;

			/*---- RTI ----------------------------------------------------------*/
			// RTI 命令実行直後は
			// - T フラグをリセットしない 
			// Kitao追記 - ここでIフラグ(IF)が復帰したときにPrevIFもIFと同値になる。pullしたIFが0ならば、この後すぐ割り込み処理が起こる。
			//			   Kiさんからいただいた情報(CLI命令時には１命令遅れて割り込みが起こるが、RTIはすぐに割り込みが起こる)の動作に一致した。v1.61
			case CPU_INST_RTI:
				separateFlags(pull());
				_BF = CPU_BF; //v1.61追加。Thanks Aladar's information.
				refreshPrevIF(); //v1.61追加
				_PC = pull();
				_PC |= pull() << 8;
				return;

			/*---- RTS ----------------------------------------------------------*/
			case CPU_INST_RTS:
				_PC = pull();
				_PC |= pull() << 8;
				++_PC;
				break;

			/*-------------------------------------------------------------------**
			** block data transfer instructions
			**-------------------------------------------------------------------*/
			/*---- TII ----------------------------------------------------------*/
			case CPU_INST_TII:
				_TransferSrcAddr = fetchAbs();
				_TransferDstAddr = fetchAbs();
				_TransferLength	= fetchAbs();
				gCPU_Transfer = 1; //Kitao追加。1=TII命令転送開始の合図。
				return;

			/*---- TDD ----------------------------------------------------------*/
			case CPU_INST_TDD:
				_TransferSrcAddr = fetchAbs();
				_TransferDstAddr = fetchAbs();
				_TransferLength	= fetchAbs();
				gCPU_Transfer = 2; //Kitao追加。2=TDD命令転送開始の合図。
				return;

			/*---- TIN ----------------------------------------------------------*/
			case CPU_INST_TIN:
				_TransferSrcAddr = fetchAbs();
				_TransferDstAddr = fetchAbs();
				_TransferLength	= fetchAbs();
				gCPU_Transfer = 3; //Kitao追加。3=TIN命令転送開始の合図。
				return;

			/*---- TIA ----------------------------------------------------------*/
			case CPU_INST_TIA:
				_TransferSrcAddr = fetchAbs();
				_TransferDstAddr = fetchAbs();
				_TransferLength	= fetchAbs();
				_TransferIncDec = TRUE;
				gCPU_Transfer = 4; //Kitao追加。4=TIA命令転送開始の合図。
				return;

			/*---- TAI ----------------------------------------------------------*/
			case CPU_INST_TAI:
				_TransferSrcAddr = fetchAbs();
				_TransferDstAddr = fetchAbs();
				_TransferLength	= fetchAbs();
				_TransferIncDec = TRUE;
				gCPU_Transfer = 5; //Kitao追加。5=TAI命令転送開始の合図。
				return;

			/*-------------------------------------------------------------------**
			** mapper instructions
			**-------------------------------------------------------------------*/
			/*---- TAMi ---------------------------------------------------------*/
			case CPU_INST_TAM:
				ureg8 = READINC(_PC);
//if ((ureg8 != 0x01)&&(ureg8 != 0x02)&&(ureg8 != 0x04)&&(ureg8 != 0x08)&&(ureg8 != 0x10)&&(ureg8 != 0x20)&&(ureg8 != 0x40)&&(ureg8 != 0x80))
//PRINTF("TAM %d",ureg8);//test
				if (ureg8 & 0x01) _MPR[0] = _A; //Kitao更新。MPRは左13回シフトせず、Aレジスタの値のまま保持するようにした。高速化。v1.47
				if (ureg8 & 0x02) _MPR[1] = _A;
				if (ureg8 & 0x04) _MPR[2] = _A;
				if (ureg8 & 0x08) _MPR[3] = _A;
				if (ureg8 & 0x10) _MPR[4] = _A;
				if (ureg8 & 0x20) _MPR[5] = _A;
				if (ureg8 & 0x40) _MPR[6] = _A;
				if (ureg8 & 0x80) _MPR[7] = _A;
				//Kitao追記。ureg8の複数のbitが立っていた場合どうなるか。現状は全てをセットするようにした。動かないソフトの挙動と関係あるかも。要テスト。
				break;

			/*---- TMAi ---------------------------------------------------------*/
			case CPU_INST_TMA:
				ureg8 = READINC(_PC);
//if ((ureg8 != 0x01)&&(ureg8 != 0x02)&&(ureg8 != 0x04)&&(ureg8 != 0x08)&&(ureg8 != 0x10)&&(ureg8 != 0x20)&&(ureg8 != 0x40)&&(ureg8 != 0x80))
//PRINTF("TMA %d",ureg8);//test
				if (ureg8 & 0x80) { _A = (Uint8)_MPR[7]; break; }
				if (ureg8 & 0x40) { _A = (Uint8)_MPR[6]; break; }
				if (ureg8 & 0x20) { _A = (Uint8)_MPR[5]; break; }
				if (ureg8 & 0x10) { _A = (Uint8)_MPR[4]; break; }
				if (ureg8 & 0x08) { _A = (Uint8)_MPR[3]; break; }
				if (ureg8 & 0x04) { _A = (Uint8)_MPR[2]; break; }
				if (ureg8 & 0x02) { _A = (Uint8)_MPR[1]; break; }
				if (ureg8 & 0x01) { _A = (Uint8)_MPR[0]; break; }
				//Kitao追記。ureg8の複数のbitが立っていた場合どうなるか。v1.59から高いビット側が最終的に残りそうなので、高いビットを優先するようにした。
				break;

			/*-------------------------------------------------------------------**
			** control instructions
			**-------------------------------------------------------------------*/
			/*---- CSL ----------------------------------------------------------*/
			case CPU_INST_CSL: //Kitao追加。時間待ちなどで使われる場合がある。
				if (_bSpeedLow == FALSE) //v0.85追加。High→Lowになるときは実行中にクロックが変わるので消費クロックも変則的になる
				{
					gCPU_ClockCount -= 9;
					SetSpeedLow(TRUE);
				}
				break;

			/*---- CSH ----------------------------------------------------------*/
			case CPU_INST_CSH: //Kitao追加。時間待ちなどで使われる場合がある。
				if (_bSpeedLow) //v0.85追加。Low→Highになるときは実行中にクロックが変わるので消費クロックも変則的になる
				{
					gCPU_ClockCount -= 6;
					SetSpeedLow(FALSE);
				}
				break;

			/*---- BRK ----------------------------------------------------------*/
			case CPU_INST_BRK:
				_PC++;
				push(_PC >> 8);
				push(_PC & 0xFF);
				_TF = 0; //Kitao更新。Tフラグをクリアしてからpush。v1.07
				_BF = CPU_BF;
				push(gatherFlags());
				_IF = CPU_IF;
				refreshPrevIF(); //v1.61追加
				_DF = 0;
				_PC  = READ(CPU_IRQ2VECTOR);
				_PC |= READ(CPU_IRQ2VECTOR + 1) << 8;
				break;

			/*---- NOP ----------------------------------------------------------*/
			case CPU_INST_NOP:
				break;

			/*---- RMBi ---------------------------------------------------------*/
			case CPU_INST_RMB0_ZP:
				ureg8 = READINC(_PC);
				RMBi(ureg8, 0x01);
				break;

			case CPU_INST_RMB1_ZP:
				ureg8 = READINC(_PC);
				RMBi(ureg8, 0x02);
				break;

			case CPU_INST_RMB2_ZP:
				ureg8 = READINC(_PC);
				RMBi(ureg8, 0x04);
				break;

			case CPU_INST_RMB3_ZP:
				ureg8 = READINC(_PC);
				RMBi(ureg8, 0x08);
				break;

			case CPU_INST_RMB4_ZP:
				ureg8 = READINC(_PC);
				RMBi(ureg8, 0x10);
				break;

			case CPU_INST_RMB5_ZP:
				ureg8 = READINC(_PC);
				RMBi(ureg8, 0x20);
				break;

			case CPU_INST_RMB6_ZP:
				ureg8 = READINC(_PC);
				RMBi(ureg8, 0x40);
				break;

			case CPU_INST_RMB7_ZP:
				ureg8 = READINC(_PC);
				RMBi(ureg8, 0x80);
				break;

			/*---- SMBi ---------------------------------------------------------*/
			case CPU_INST_SMB0_ZP:
				ureg8 = READINC(_PC);
				SMBi(ureg8, 0x01);
				break;

			case CPU_INST_SMB1_ZP:
				ureg8 = READINC(_PC);
				SMBi(ureg8, 0x02);
				break;

			case CPU_INST_SMB2_ZP:
				ureg8 = READINC(_PC);
				SMBi(ureg8, 0x04);
				break;

			case CPU_INST_SMB3_ZP:
				ureg8 = READINC(_PC);
				SMBi(ureg8, 0x08);
				break;

			case CPU_INST_SMB4_ZP:
				ureg8 = READINC(_PC);
				SMBi(ureg8, 0x10);
				break;

			case CPU_INST_SMB5_ZP:
				ureg8 = READINC(_PC);
				SMBi(ureg8, 0x20);
				break;

			case CPU_INST_SMB6_ZP:
				ureg8 = READINC(_PC);
				SMBi(ureg8, 0x40);
				break;

			case CPU_INST_SMB7_ZP:
				ureg8 = READINC(_PC);
				SMBi(ureg8, 0x80);
				break;

			/*------------------------------------------------------------------**
			** invalid instructions
			**------------------------------------------------------------------*/
			default:
				//PRINTF("CPU ERROR::invalid instructions"); //Kitao追加。v1.07
				break;
		}
		_TF = 0;
		return;

	case 1: //TII
		gCPU_ClockCount -= 5; //ネクロスの要塞のOPデモOK。ゲンジ通信あげだまOK。ダブルドラゴンIIのOPデモOK。
		ureg8 = READINC(_TransferSrcAddr);
		WRITEINC(_TransferDstAddr, ureg8);
		if (--_TransferLength <= 0) //転送を全て終えたなら
		{
			gCPU_Transfer = 0;
			_TF = 0;
			refreshPrevIF(); //v1.62追加
		}
		return;
	case 2: //TDD
		gCPU_ClockCount -= 5;
		ureg8 = READDEC(_TransferSrcAddr);
		WRITEDEC(_TransferDstAddr, ureg8);
		if (--_TransferLength <= 0) //転送を全て終えたなら
		{
			gCPU_Transfer = 0;
			_TF = 0;
			refreshPrevIF(); //v1.62追加
		}
		return;
	case 3: //TIN
		gCPU_ClockCount -= 5;
		ureg8 = READINC(_TransferSrcAddr);
		WRITE(_TransferDstAddr, ureg8);
		if (--_TransferLength <= 0) //転送を全て終えたなら
		{
			gCPU_Transfer = 0;
			_TF = 0;
			refreshPrevIF(); //v1.62追加
		}
		return;
	case 4: //TIA
		ureg8 = READINC(_TransferSrcAddr);
		if (_TransferIncDec)
		{
			gCPU_ClockCount -= 5; //ここが6だとワンダーモモで乱れ
			WRITEINC(_TransferDstAddr, ureg8);
			_TransferIncDec = FALSE;
		}
		else
		{
			gCPU_ClockCount -= 6; ///ワースタ'91ミート時に6以上が必要。6でワンダーモモOK。ロードス島戦記IIのエンディングで6以上が必要(5でもVCEウェイトが入ればOK)。v1.61更新
								  //"Writeアドレス"のINCとDECの判断＋処理でTII等より遅い？(実機未測定)
			WRITEDEC(_TransferDstAddr, ureg8);
			_TransferIncDec = TRUE;
		}
		if (--_TransferLength <= 0) //転送を全て終えたなら
		{
			gCPU_Transfer = 0;
			_TF = 0;
			refreshPrevIF(); //v1.62追加。ここで更新しないとロックオンで弾が出ない。おそらく実機も同様の動きで、転送命令は２命令分以上になるためその時間で信号(フラグ)も更新される。
		}
		return;
	case 5: //TAI
		if (_TransferIncDec)
		{
			gCPU_ClockCount -= 5;
			_TransferIncDec = FALSE;
			ureg8 = READINC(_TransferSrcAddr);
		}
		else
		{
			gCPU_ClockCount -= 6; //6でサザンアイズのOPで音声がピッタリ。5,7だと妖怪道中記のギャンブル画面で乱れ。7以上だとSUPER桃鉄２のタイトル画面で上辺乱れ。v1.62更新
								  //"Readアドレス"のINCとDECの判断＋処理でTII等より遅い？(実機未測定)
			_TransferIncDec = TRUE;
			ureg8 = READDEC(_TransferSrcAddr);
		}
		WRITEINC(_TransferDstAddr, ureg8);
		if (--_TransferLength <= 0) //転送を全て終えたなら
		{
			gCPU_Transfer = 0;
			_TF = 0;
			refreshPrevIF(); //v1.62追加
		}
		return;
	}
}


// save variable
#define SAVE_V(V)	if (fwrite(&V, sizeof(V), 1, p) != 1)	return FALSE
#define LOAD_V(V)	if (fread(&V, sizeof(V), 1, p) != 1)	return FALSE
// save array
#define SAVE_A(A)	if (fwrite(A, sizeof(A), 1, p) != 1)	return FALSE
#define LOAD_A(A)	if (fread(A, sizeof(A), 1, p) != 1)		return FALSE
/*-----------------------------------------------------------------------------
	[SaveState]
		ＣＰＵの状態をファイルに保存します。 
-----------------------------------------------------------------------------*/
BOOL
CPU_SaveState(
	FILE*		p)
{
	BOOL	bTurboCycle = FALSE;

	if (p == NULL)
		return FALSE;

	SAVE_V(_A);
	SAVE_V(_X);
	SAVE_V(_Y);
	SAVE_V(_S);
	SAVE_V(_P);
	SAVE_V(_CF);
	SAVE_V(_ZF);
	SAVE_V(_IF);
	SAVE_V(_PrevIF); //v1.61追加
	SAVE_V(_DF);
	SAVE_V(_BF);
	SAVE_V(_TF);
	SAVE_V(_VF);
	SAVE_V(_NF);
	SAVE_V(_PC);
	SAVE_A(_MPR);

	SAVE_V(_bRDY);
	SAVE_V(_bIRQ1);
	SAVE_V(_bIRQ2);
	SAVE_V(_bNMI);
	SAVE_V(_bTIRQ);
	SAVE_V(_IntDisable); //v0.64追加
	SAVE_V(_bPrevIRQ1); //v1.61追加
	SAVE_V(_bPrevIRQ2); //v1.61追加
	SAVE_V(_bPrevTIRQ); //v1.61追加
	SAVE_V(_PrevIntDisable); //v1.61追加

	SAVE_V(gCPU_Transfer); //Kitao追加
	SAVE_V(_TransferSrcAddr); //Kitao追加
	SAVE_V(_TransferDstAddr); //Kitao追加
	SAVE_V(_TransferLength); //Kitao追加
	SAVE_V(_TransferIncDec); //Kitao追加

	SAVE_V(gCPU_ClockCount); //Kitao追加
	SAVE_V(_ClockElapsed); //Kitao追加
	SAVE_V(_bSpeedLow); //Kitao追加
	SAVE_V(bTurboCycle); //Kitao追加。v1.61から非使用に。

	SAVE_V(_SelectVDC); //v0.89追加

	return TRUE;
}

/*-----------------------------------------------------------------------------
	[LoadState]
		ＣＰＵの状態をファイルから読み込みます。 
-----------------------------------------------------------------------------*/
BOOL
CPU_LoadState(
	FILE*		p)
{
	int		i;
	BOOL	bTurboCycle;

	if (p == NULL)
		return FALSE;

	LOAD_V(_A);
	LOAD_V(_X);
	LOAD_V(_Y);
	LOAD_V(_S);
	LOAD_V(_P);
	LOAD_V(_CF);
	LOAD_V(_ZF);
	LOAD_V(_IF);
	if (MAINBOARD_GetStateVersion() >= 34) //v1.61beta以降のセーブファイルなら
	{
		LOAD_V(_PrevIF);
	}
	else //旧バージョンのステートを読んだ場合
		_PrevIF = _IF;
	LOAD_V(_DF);
	LOAD_V(_BF);
	LOAD_V(_TF);
	LOAD_V(_VF);
	LOAD_V(_NF);
	LOAD_V(_PC);
	LOAD_A(_MPR);
	for (i=0; i<8; i++)
		if (_MPR[i] > 0xFF)
			_MPR[i] >>= 13; //v1.47より前のセーブデータだった場合、MPRがシフトされているのを戻す。

	LOAD_V(_bRDY);
	LOAD_V(_bIRQ1);
	LOAD_V(_bIRQ2);
	LOAD_V(_bNMI);
	LOAD_V(_bTIRQ);
	if (MAINBOARD_GetStateVersion() >= 8) //Kitao追加。v0.64以降のセーブファイルなら
		LOAD_V(_IntDisable); //旧バージョンステートの場合もIntCtrl.c から_IntDisableが設定される。
	if (MAINBOARD_GetStateVersion() >= 35) //v1.61以降のセーブファイルなら
	{
		LOAD_V(_bPrevIRQ1);
		LOAD_V(_bPrevIRQ2);
		LOAD_V(_bPrevTIRQ);
	}
	else //旧バージョンのステートを読んだ場合
	{
		_bPrevIRQ1 = _bIRQ1;
		_bPrevIRQ2 = _bIRQ2;
		_bPrevTIRQ = _bTIRQ;
	}
	if (MAINBOARD_GetStateVersion() >= 34) //v1.61beta以降のセーブファイルなら
	{
		LOAD_V(_PrevIntDisable);
	}
	else //旧バージョンのステートを読んだ場合
		_PrevIntDisable = _IntDisable;

	LOAD_V(gCPU_Transfer); //Kitao追加
	LOAD_V(_TransferSrcAddr); //Kitao追加
	LOAD_V(_TransferDstAddr); //Kitao追加
	LOAD_V(_TransferLength); //Kitao追加
	LOAD_V(_TransferIncDec); //Kitao追加

	LOAD_V(gCPU_ClockCount); //Kitao追加
	LOAD_V(_ClockElapsed); //Kitao追加
	if (MAINBOARD_GetStateVersion() < 34) //v1.61betaより前のセーブファイルなら
		gCPU_ClockCount -= _ClockElapsed;//v1.61からデバッグ以外非使用に。旧バージョンステートの場合、ここで溜まっていた分を引く。
	LOAD_V(_bSpeedLow); //Kitao追加
	LOAD_V(bTurboCycle); //Kitao追加。v1.61から非使用に。
	CPU_SetTurboCycle(0); //ターボサイクルをオフにし、現在の速度状況(_bSpeedLow)に合わせて消費サイクルをセット。v1.61追加

	if (MAINBOARD_GetStateVersion() >= 17) //Kitao追加。v0.89以降のセーブファイルなら
	{
		LOAD_V(_SelectVDC);
	}
	else
		_SelectVDC = 0;

	return TRUE;
}

#undef SAVE_V
#undef SAVE_A
#undef LOAD_V
#undef LOAD_A

