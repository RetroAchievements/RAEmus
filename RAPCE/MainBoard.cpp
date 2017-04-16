/******************************************************************************
Ootake
・スーパーグラフィックスに対応した。v0.89
・32ビットカラーに対応した。
・VCEコントロールレジスタのドットクロック値のとおりの解像度で表示するようにし
  た。シューティングゲームの縦画面モードなどに対応。
・横320の場合、実機では左右に(実機での)16ドットぶんずつの黒帯ができるのでそれ
  を再現した。横304（ぷよぷよ。Wiz5など）も同様に再現した。
・横表示解像度によりできるテレビ画面両端の未表示領域（黒帯）は、常にスプライト
  の透明色で埋めるようにした。（天外２ビジュアルシーンなどを実機同様に再現）
・VDC,VDEレジスタへのI/Oアクセスをノンウェイトとした。（VDCライト時だけウェイ
  トを入れるようにした）
・バックアップラムの書込み禁止時にReadはできるようにした。
・アーケードカードを未使用のソフトの場合、ステートセーブ時にアーケードカードの
  メモリ領域は保存しないようにした。
・ステートセーブ時にCD-DAが鳴っていた場合、ステートロード後にCD-DAの再生を行う
  ようにした。
・ステートセーブ時にステートセーブのバージョンを先頭に保存するようにした。（将
  来バージョンアップで仕様を変更した場合に、古いものを区別して読み込めるように
  するための備え）
・バックアップラムに変更がなかった場合（バックアップラム非対応ソフトも含む）
  は、バックアップラムファイルを更新（保存）しないようにした。
・縦画面風ストレッチモードを付けた。
・６ボタンパッド用ゲームを自動認識するようにした。
・ラスタ割り込みのタイミングを最適化するようにした。
・BigAudioBufferモードを付けた。このモードのときは音質重視のためフレームレート
  を下げる。v0.54更新。Most BigAudioBufferモードも追加。
・描画処理をストレッチ時と非ストレッチ時で分けずに、共用にした。
・ラインごとに解像度を変えるゲーム(あすか120%等)に対応した。v0.60
・イメージファイルの拡張子が.pce .hes .rom .bin 以外ならエラーにし、開かないよ
  うにした。v1.00
・CD-ROM用のバッファRAM(64KB+192KB。前半32KBは除いた)にアクセスするときは、ア
  クセスウェイトを入れるようにした。ライト時に、1ウェイトを入れることで、サザ
  ンアイズ，ダブルドラゴンII(画面乱れも解消)，スプリガンmk2のデモで音ずれを解
  消。実機でも同様かは未確認。v1.38。v2.09,v2.34更新
・セーブ用バッテリーバックアップRAMにアクセスしたときにウェイトを入れるように
  した。ダライアスプラスのハイスコア画面、レディーソードのロード時の画面、等の
  1フレームの画面乱れが実機と同様になった。v1.03。v1.61更新
・physAddrのアクセスI/O判定部分をより細かくした。"ラプラスの魔"の画面化けを解
  消。v1.13
・メモリマップのリードファンクション、ライトファンクションを高速化した。v1.15
・リードファンクションでI/Oアクセスの際に((physAddr & 0x1E00) == 0x1A00)だった
  場合は、0xFFを返すようにした。"イメージファイト"の最終面が正常に。v1.15
・特定の吸出し機（3Mbit未対応）で3Mが4MにオーバーダンプされてしまったROMイメー
  ジファイルを3Mに修正出力する機能を付けた。v1.35
・CPUの処理を1/240秒単位で区切ることで、PSG音の解像度を上げた。v1.39

Copyright(C)2006-2010 Kitao Nakamura.
	改造版・後継版を公開なさるときは必ずソースコードを添付してください。
	その際に事後でかまいませんので、ひとことお知らせいただけると幸いです。
	商的な利用は禁じます。
	あとは「GNU General Public License(一般公衆利用許諾契約書)」に準じます。

*******************************************************************************
	[MainBoard.c]
		メインボードを実装します。

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

#define DIRECTINPUT_VERSION	0x0800	//Kitao追加。環境にもよるかもしれないが、DirectInput5が軽い。7だとやや遅延あり。スペースハリアーがわかりやすい。

#include <stdio.h>
#include <malloc.h>
#include "MainBoard.h"
#include "Screen.h"
#include "Input.h"
#include "Printf.h"
#include "WinMain.h"
#include "App.h"
#include "CDInterface.h"
#include "AudioOut.h"
#include "PSG.h"
#include "TocDB.h"
#include "WriteMemory.h"


//	##RA
#include "RA_Interface.h"

static Uint32		_StateVersion; //Kitao追加
extern Sint32		gCPU_ClockCount; //v2.00追加。高速化のためカウンタをグローバル化。

/*-----------------------------------------------------------------------------
** メインボード内の記憶領域を宣言します。
**---------------------------------------------------------------------------*/
static Uint8*		_pMainRam; //Kitao更新。スーパーグラフィックス用に32KBで使用。
static Uint8*		_pOldMainRam; //8KB。過去バージョンのステートセーブを読み込むために残してある
static Uint32		_MainRamMask; //Kitao追加。通常は0x1FFF(8KB)。スパグラのときは0x7FFF(32KB)にする。
static Uint8*		_pBackupRam;
static Uint8*		_pRom;
static Uint32		_RomMask;
static Uint32		_RomSize; //Kitao追加
static Uint8*		_pRomMap[256];
static Uint8*		_pBufferRam; // 注意：これは本来 CD-ROM^2 システム内に存在する。 
static BOOL			_bContinuousWriteValue; //Kitao追加。定期的に１つのアドレスに１つの固定値を書き込み続ける(値を一定に保つ)処理を行う場合TRUE。デバッグ用。v2.39
static Uint32		_ContWriteMpr;			//			 書き込むマッピングアドレス
static Uint32		_ContWriteAddress;		//			 書き込むアドレス
static Uint8		_ContWriteData;			//			 書き込むデータ

static BOOL			_bSystemInit = FALSE;
static Uint32		_SystemTime; //Kitao追加。システムリセット後の経過時間。1/60秒単位。
static Uint32		_SystemTimeCheck; //Kitao追加
static Sint32		_SuperGrafx; //Kitao追加
static BOOL			_bSystemCard = FALSE; //Kitao追加。システムカード(純正v1.0～3.0，ゲームエクスプレスカード)のイメージファイルを起動中(CD-ROMシステム動作中)はTRUE。※ポピュラス(RAM付き特殊カード)起動中もTRUE。v2.40
static BOOL			_bArcadeCard = TRUE; //Kitao追加。両対応ソフトでアーケードカードをあえて使わないならFALSEにする。v1.11
static BOOL			_bBackupFull = FALSE; //Kitao追加。バックアップRAMの空き容量を強制的に0にするならTRUEにする。容量が足りない警告デモの表示で利用。v1.49
static BOOL			_bFourSplitScreen = FALSE; //Kitao追加。４分割画面モード中ならTRUEに。v2.27
static Sint32		_PrevFourSplitCpuSpeed; //Kitao追加。４分割画面モードへ入る前のCPU速度設定を退避用。v2.27
static BOOL			_bMonoColor = FALSE; //Kitao追加。白黒モードのときはTRUE。v2.27
static Sint32		_ForceMonoColor = 0; //Kitao追加。メニュー設定で"Screen->MonoColor"にチェックが入っているとき1。グリーンディスプレイモード=2。v2.28
static BOOL			_bKonamiStereo; //Kitao追加
static BOOL			_bGradiusII; //Kitao追加
static BOOL			_bAutoDisconnectMultiTap;//Kitao追加
static Sint32		_AutoRealStretched = -1; //Kitao追加。自動でストレッチさせるばあいは、0以上の値にする。その値は、変更前のStartStretchModeとする。v2.22，v2.64

static Sint32		_ScreenWidth; //Kitao追加
static Sint32		_ScreenHeight; //Kitao追加
static Sint32		_Magnification;
static BOOL			_bFullScreen; //Kitao追加
static Uint32		_FullScreenColor; //Kitao追加
static BOOL			_bStretched;
static BOOL			_bVStretched; //Kitao追加
static Uint32		_Flags; //Kitao更新

static BOOL			_bPause = TRUE; //Kitao更新

static Uint32		_ScreenBuf[512*256]; //512*256*sizeof(Uint32)。Kitao追加。32ビットカラー対応のためUint32へ。v2.00からフォーマットを変更。添え字の0～15,225～239をオーバースキャン領域用とした。
static Uint32		_ScreenBufOT[512*8]; //v2.00から_ScreenBufのフォーマットを変更ため、旧バージョンのステート読み込み時に変換を行う。そのときに使う変数。
static BOOL			_ScreenBufOk = FALSE; //Kitao追加。ScreenBufを設定できたらTRUE。現在非使用。
static Sint32		_prevTvW = 0; //Kitao追加
static Sint32		_ScanLine; //Kitao追加。現在描画中のライン。JoyPad.cからも利用
static Uint16		_TvW[272]; //Kitao追加。各ラインの解像度（ラスタ割り込みでラインごとに解像度を変えているソフトに対応）。現在256ラインぶんで足りるが旧バージョンのステートセーブを読み込むために272ぶん用意。
static Uint16		_TvWOT[8]; //v2.00から_ScreenBufのフォーマットを変更ため、旧バージョンのステート読み込み時に変換を行う。そのときに使う変数。
static Uint16		_WidestTvW; //Kitao追加。全ライン中で最大の解像度。 ※v1.28から非使用。過去バージョンのステートロードのために残しておく
static Uint32		_ScreenBufR[512*256]; //Kitao追加。ScreenBufを一時退避用
static Uint16		_TvWR[272]; //Kitao追加。TvWを一時退避用
static Sint32		_VDCWidthR; //Kitao追加。VDC_GetScreenWidth()を一時退避用
static BOOL			_bFourSplitScreenR; //Kitao追加。退避用
static BOOL			_bMonoColorR; //Kitao追加。退避用
static Sint32		_ForceMonoColorR; //Kitao追加。退避用
static BOOL			_bResolutionChange; //Kitao追加
static Sint32		_UpdateVsync = 1; //Kitao追加。通常は常に1にしておく。2の場合screenUpdate()でV-Sync待ちをしない。DirectDrawでのスクリーンショット時に使用。

//Kitao追加。オーバースキャン領域を表示するなら下記の変数にそのドット数を入れる。0なら表示しない。v1.43
static Sint32		_ShowOverscanTop	= 0;
static Sint32		_ShowOverscanBottom = 0;
static Sint32		_ShowOverscanLeft	= 0;
static Sint32		_ShowOverscanRight	= 0;

static Uint8		_Buffer;

static Sint32		_BigSoundBuffer = 0;//Kitao追加。1のときは曲観賞用サイズで大きくバッファを取る。2のときはさらに大きくサウンドバッファを取る。曲を高音質で鑑賞したい時用。
static Uint32		_BigSoundCounter = 0; //Kitao追加

static BOOL			_bAcUse = FALSE; //Kitao追加
static Uint8		_FirstBackupRam[0x2000]; //Kitao追加

static Sint32		_FastForwarding; //Kitao追加
static Sint32		_FastForwardingR; //Kitao追加
static BOOL			_bSoundAjust; //Kitao追加
static Sint32		_FastForwardingCount; //Kitao追加

//Kitao追加。処理落ち検査用
static Uint32		_PrevAdvanceFrameTime;
static Sint32		_ProcessingDelay[300]; //300=5秒間履歴を残す
static Uint32		_ProcessingDelayIndex;

/*
	All the Arcade Card I/O code is taken from Hu-Go!
*/
static Uint32		_AcShift;
static Uint32		_AcShiftBits;
static Uint8		_0x1AE5;
static Uint8		_AcRam[0x200000];

typedef struct
{
	Uint8		control;
	Uint32		base;
	Uint16		offset;
	Uint16		increment;
} ACIO;

static ACIO		_Ac[4];


static inline void
increment_acaddr(
	ACIO*		port)
{
	if (port->control & 1)		// CONFIRMED:  D0 enables base / offset increment
	{
		if (port->control & 0x10)	// CONFIRMED: D4 selects base / offset to be incremented
		{
			port->base += port->increment;
			port->base &= 0xFFFFFF;
		}
		else
		{
			port->offset += port->increment;
		}
	}
}


static Uint8
ac_read(
	Uint32		physAddr)
{
	ACIO	*port = &_Ac[(physAddr >> 4) & 3];
	Uint32	addr; //Kitao追加

	if ((physAddr & 0x1AE0) == 0x1AE0)
	{
		switch (physAddr & 0x1AEF)
		{
			case 0x1AE0:
				return (Uint8)_AcShift;
			case 0x1AE1:
				return (Uint8)(_AcShift >> 8);
			case 0x1AE2:
				return (Uint8)(_AcShift >> 16);
			case 0x1AE3:
				return (Uint8)(_AcShift >> 24);
			case 0x1AE4:
				return (Uint8)(_AcShiftBits);
			case 0x1AE5:
				return _0x1AE5;
			case 0x1AEE:
				return 0x10;
			case 0x1AEF:
				return 0x51;
		}
		return 0; //v1.62更新。ポート0xFまでは存在すると仮定し0を返すようにした。
	}

	switch (physAddr & 0xF)
	{
		case 0x0:
		case 0x1:
			if (port->control & 2)
			{
				if (port->control & 0x8) //Kitao追加。餓狼伝説２のゲーム中ADPCM音声が正常に。v2.60
					addr = port->base + port->offset + 0xFF0000;
				else
					addr = port->base + port->offset;
			}
			else
				addr = port->base;
			increment_acaddr(port);
			return _AcRam[addr & 0x1FFFFF]; //Kitao更新

		case 0x2:	return (Uint8)(port->base);
		case 0x3:	return (Uint8)(port->base >> 8);
		case 0x4:	return (Uint8)(port->base >> 16);
		case 0x5:	return (Uint8)(port->offset);
		case 0x6:	return (Uint8)(port->offset >> 8);
		case 0x7:	return (Uint8)(port->increment);
		case 0x8:	return (Uint8)(port->increment >> 8);
		case 0x9:	return port->control;
		case 0xA: //v1.62更新。ポート0xFまでは存在すると仮定し0を返すようにした。
		case 0xB:
		case 0xC:
		case 0xD:
		case 0xE:
		case 0xF:
			return 0;
	}
	return 0xFF;
}


static void
ac_write(
	Uint32		physAddr,
	Uint8		data)
{
	ACIO	*port = &_Ac[(physAddr >> 4) & 3];

	_bAcUse = TRUE;//Kitao追加。アーケードカードを利用した印。ステートセーブ時にアーケードカード関連もセーブする。

	if ((physAddr & 0x1AE0) == 0x1AE0)
	{
		switch (physAddr & 0xF)
		{
			case 0:
				_AcShift = (_AcShift & ~0xFF) | data;
				return;
			case 1:
				_AcShift = (_AcShift & ~0xFF00) | (data << 8);
				return;
			case 2:
				_AcShift = (_AcShift & ~0xFF0000) | (data << 16);
				return;
			case 3:
				_AcShift = (_AcShift & ~0xFF000000) | (data << 24);
				return;
			case 4:
				if ((_AcShiftBits = data & 0xF) != 0)
				{
					if (_AcShiftBits < 8)
						_AcShift <<= _AcShiftBits;
					else
						_AcShift >>= 16 - _AcShiftBits;
				}
				return;
			case 5:
				_0x1AE5 = data;
				return;
		}
	}
	else
	{
		switch (physAddr & 0xF)
		{
			case 0x0:
			case 0x1:
				if (port->control & 2)
				{
					if (port->control & 0x08) //v2.60追加
						_AcRam[(port->base + port->offset + 0xFF0000) & 0x1FFFFF] = data;
					else
						_AcRam[(port->base + port->offset) & 0x1FFFFF] = data;
				}
				else
					_AcRam[port->base & 0x1FFFFF] = data;
				increment_acaddr(port);
				return;
			case 0x2:
				port->base = (port->base & ~0xFF) | data;
				return;
			case 0x3:
				port->base = (port->base & ~0xFF00) | (data << 8);
				return;
			case 0x4:
				port->base = (port->base & ~0xFF0000) | (data << 16);
				return;
			case 0x5:
				port->offset = (port->offset & ~0xFF) | data;
				if ((port->control & 0x60) == 0x20) //Kitao追加。Thanks for anonymous information mail. 餓狼伝説２のCPUダメージ減らない問題が解消。v2.60
				{
					if (port->control & 0x08)
						port->base += port->offset + 0xFF0000;
					else
						port->base += port->offset;
					port->base &= 0xFFFFFF;
				}
				return;
			case 0x6:
				port->offset = (port->offset & ~0xFF00) | (data << 8);
				if ((port->control & 0x60) == 0x40)
				{
					if (port->control & 0x08)
						port->base += port->offset + 0xFF0000;
					else
						port->base += port->offset;
					port->base &= 0xFFFFFF;
				}
				return;
			case 0x7:
				port->increment = (port->increment & ~0xFF) | data;
				return;
			case 0x8:
				port->increment = (port->increment & ~0xFF00) | (data << 8);
				return;
			case 0x9:
				port->control = data & 0x7F;		// D7 is not used
				return;
			case 0xA:
				// value written is not used 
				if ((port->control & 0x60) == 0x60)
				{	//v2.60更新
					if (port->control & 0x08)
						port->base += port->offset + 0xFF0000;
					else
						port->base += port->offset;
					port->base &= 0xFFFFFF;
				}
				return;
		}
	}
}


/*-----------------------------------------------------------------------------
** 読み出し関数(読み出しに関するI/Oマップ)と
** 書き込み関数(書き込みに関するI/Oマップ)を定義します。
**---------------------------------------------------------------------------*/
//Kitao更新。通常のHuカードゲーム用。(CD-ROMを未接続。case文の数が少ないとだいぶ高速化にもなる)
//			 addrは必ず0x1FFFでマスクしてからここ呼ぶ。
static Uint8
cpu_read_func_hucard(
	Uint32		mpr,
	Uint32		addr) //Kitao更新。マッピングレジスタとアドレスを個別に受け取るようにして高速化。v1.47
{
	//Kitao更新。ROMへのアクセスがいちばん頻繁におこなわれるので、最初に判定することで高速化。v1.15
	if (mpr <= 0x7F)
		return _pRom[((mpr << 13) | addr) & _RomMask];
	else
	{
		switch (mpr)
		{
 			//v1.15更新。頻繁に使われるものを先に置いて高速化
			case 0xFF:
				switch (addr & 0x1C00)
				{
					case 0x0000: // VDC
						return VDC_Read(addr);
					case 0x0400: // VCE
						return VDC_VceRead(addr); //v1.21更新。アドレスをそのまま渡して高速化。
					case 0x0800: // PSG
						return _Buffer; //PSG_Read(addr);
					case 0x0C00: // Timer
						_Buffer = (_Buffer & 0x80) | TIMER_Read();
						return _Buffer;
					case 0x1000: // PAD
						_Buffer = JOYPAD_Read(addr);
						return _Buffer;
					case 0x1400: // INT
						_Buffer = (_Buffer & 0xF8) | INTCTRL_Read(addr); //v1.21更新。アドレスをそのまま渡して高速化。
						return _Buffer;
					case 0x1800: // CD-ROM^2 (for BRAM)
						if ((addr & 0x1E00) == 0x1A00)
							return 0xFF; //Kitao追加。イメージファイトで必要。ついにイメージファイト最終面動くようになった。感動。v1.15。
						else
							return CDROM_Read(addr); //v1.21更新。アドレスをそのまま渡して高速化。
				}
				return 0xFF;
			case 0xF8:
			case 0xF9:
			case 0xFA:
			case 0xFB:
				return _pMainRam[((mpr << 13) | addr) & _MainRamMask];
			case 0xF7:
				gCPU_ClockCount -= 22; //Kitao追加。アクセスウェイト。22でレディーソードのロード時で画面,ダライアスプラスのハイスコア画面の乱れ具合が実機と同様に。
				if (addr <= 0x7FF) //Kitao追加。2KBだけ有効
					return _pBackupRam[addr]; //Kitao更新。リードの場合、書込み禁止の有無に関わらず読めるようにした。
			//case 0xFC:
			//case 0xFD:
			//case 0xFE:
			//	return 0xFF;	//caseの数を減らすことで高速化。v1.15
		}
		return 0xFF;
	}
}
static void
cpu_write_func_hucard(
	Uint32		mpr,
	Uint32		addr,
	Uint8		data) //Kitao更新。マッピングレジスタとアドレスを個別に受け取るようにして高速化。v1.47
{
	int		i;

	switch (mpr)
	{
 		//v1.15更新。頻繁に使われるものを先に置いて高速化
		case 0xFF:
			switch (addr & 0x1C00)
			{
				case 0x0000: // VDC
					VDC_Write(addr, data); //Kitao更新
					return;
				case 0x0400: // VCE
					VDC_VceWrite(addr, data); //v1.21更新。アドレスをそのまま渡して高速化。
					return;
				case 0x0800: // PSG
					_Buffer = data;
					PSG_Write(addr, data); //v1.21更新。アドレスをそのまま渡して高速化。
					return;
				case 0x0C00: // Timer
					_Buffer = data;
					TIMER_Write(addr, data); //v1.21更新。アドレスをそのまま渡して高速化。
					return;
				case 0x1000: // PAD
					_Buffer = data;
					JOYPAD_Write(addr, data);
					return;
				case 0x1400: // INT
					_Buffer = data;
					INTCTRL_Write(addr, data); //v1.21更新。アドレスをそのまま渡して高速化。
					return;
				case 0x1800: // CD-ROM^2 (for BRAM)
					if ((addr & 0x1E00) != 0x1A00) //Kitao追加。v1.15
						CDROM_Write(addr, data); //v1.21更新。アドレスをそのまま渡して高速化。
					return;
			}
			return;
		case 0xF8:
		case 0xF9:
		case 0xFA:
		case 0xFB:
			_pMainRam[((mpr << 13) | addr) & _MainRamMask] = data;
			return;
		case 0xF7:
			if (CDROM_IsBRAMEnabled())
			{
				gCPU_ClockCount -= 22; //Kitao追加。アクセスウェイト。22でレディーソードのセーブ時で画面の乱れ具合が実機と同様(オーバースキャン領域だけ若干乱れ)に。
				if (addr <= 0x7FF) //Kitao追加。2KBだけ有効
					_pBackupRam[addr] = data;
			}
			return;
	}
	//大容量ROMカード用処理
	if ((addr & 0x1FFC) == 0x1FF0) //Kitao追記。バンク切り替え(使用はスト２’だけ？)
		for (i = 0; i < 0x40; i++)
			_pRomMap[0x40+i] = &_pRom[(((addr & 3)+1) * 0x80000 + i*0x2000) & _RomMask];
}
//Kitao追加。ストリートファイター２’専用。RomMapの切り替えが必要な場合。スト２’以外は高速化のために、RomMapの切り替えを考慮しない。write_funcはスト２’も共用。
static Uint8
cpu_read_func_sf2(
	Uint32		mpr,
	Uint32		addr) //Kitao更新。マッピングレジスタとアドレスを個別に受け取るようにして高速化。v1.47
{
	//Kitao更新。ROMへのアクセスがいちばん頻繁におこなわれるので、最初に判定することで高速化。v1.15
	if (mpr <= 0x7F)
		return _pRomMap[mpr][addr];
	else
	{
		switch (mpr)
		{
 			//v1.15更新。頻繁に使われるものを先に置いて高速化
			case 0xFF:
				switch (addr & 0x1C00)
				{
					case 0x0000: // VDC
						return VDC_Read(addr);
					case 0x0400: // VCE
						return VDC_VceRead(addr); //v1.21更新。アドレスをそのまま渡して高速化。
					case 0x0800: // PSG
						return _Buffer; //PSG_Read(addr);
					case 0x0C00: // Timer
						_Buffer = (_Buffer & 0x80) | TIMER_Read();
						return _Buffer;
					case 0x1000: // PAD
						_Buffer = JOYPAD_Read(addr);
						return _Buffer;
					case 0x1400: // INT
						_Buffer = (_Buffer & 0xF8) | INTCTRL_Read(addr); //v1.21更新。アドレスをそのまま渡して高速化。
						return _Buffer;
				}
				return 0xFF;
			case 0xF8:
			case 0xF9:
			case 0xFA:
			case 0xFB:
				return _pMainRam[((mpr << 13) | addr) & _MainRamMask];
			//case 0xFC:
			//case 0xFD:
			//case 0xFE:
			//	return 0xFF;	//caseの数を減らすことで高速化。v1.15
		}
		return 0xFF;
	}
}

//Kitao追加。CD-ROMゲーム用。
static Uint8
cpu_read_func_cdrom2(
	Uint32		mpr,
	Uint32		addr) //Kitao更新。マッピングレジスタとアドレスを個別に受け取るようにして高速化。v1.47
{
	//Kitao更新。バッファRAMとROMへのアクセスは頻繁におこなわれるので、最初に判定することで高速化。v1.15。v1.62更新
	if ((mpr >= 0x68)&&(mpr <= 0x87)) //0x87-0x80=CD-ROM^2 buffer RAM (64K bytes) 0x7F-0x68=SUPER CD-ROM^2 buffer RAM (192K bytes)
		return _pBufferRam[((mpr << 13) | addr) & 0x3FFFF];
	else if (mpr <= 0x3F)
		return _pRom[((mpr << 13) | addr) & _RomMask];
	else
	{
		switch (mpr)
		{
	 		//v1.15更新。頻繁に使われるものを先に置いて高速化
			case 0xFF:
				switch (addr & 0x1C00)
				{
					case 0x0000: // VDC
						return VDC_Read(addr);
					case 0x0400: // VCE
						return VDC_VceRead(addr); //v1.21更新。アドレスをそのまま渡して高速化。
					case 0x0800: // PSG
						return _Buffer; //PSG_Read(addr);
					case 0x0C00: // Timer
						_Buffer = (_Buffer & 0x80) | TIMER_Read();
						return _Buffer;
					case 0x1000: // PAD
						_Buffer = JOYPAD_Read(addr);
						return _Buffer;
					case 0x1400: // INT
						_Buffer = (_Buffer & 0xF8) | INTCTRL_Read(addr); //v1.21更新。アドレスをそのまま渡して高速化。
						return _Buffer;
					case 0x1800: // CD-ROM^2
						if (((addr & 0x1E00) == 0x1A00)&&(_bArcadeCard)) // ARCADE CARD
							return ac_read(((mpr << 13) | addr));
						else
							return CDROM_Read(addr);
				}
				return 0xFF;

			case 0xF8:
			case 0xF9:
			case 0xFA:
			case 0xFB:
				return _pMainRam[((mpr << 13) | addr) & _MainRamMask];

			case 0x40: //Kitao更新。アーケードカード用RAM。ポピュラス(RAM内蔵カード)でも使われる。
			case 0x41:
			case 0x42:
			case 0x43:
				return ac_read(0x1A00 | ((mpr << 4) & 0xF0));

			case 0xF7: 
				gCPU_ClockCount -= 22; //Kitao追加。アクセスウェイト。スーパーダライアスのハイスコア画面の乱れが解消。
				if (addr <= 0x7FF) //Kitao追加。2KBだけ有効
					return _pBackupRam[addr]; //Kitao更新。リードの場合、書込み禁止の有無に関わらず読めるようにした。

			//case 0xFC:
			//case 0xFD:
			//case 0xFE:
			//	return 0xFF;	//caseの数を減らすことで高速化。v1.15
		}
		return 0xFF;
	}
}
static void
cpu_write_func_cdrom2(
	Uint32		mpr,
	Uint32		addr,
	Uint8		data) //Kitao更新。マッピングレジスタとアドレスを個別に受け取るようにして高速化。v1.47
{
	//Kitao更新。バッファRAMへのアクセスは頻繁におこなわれるので、最初に判定することで高速化。v1.15。v1.62更新
	if ((mpr <= 0x87)&&(mpr >= 0x68)) //0x87-0x80=CD-ROM^2 buffer RAM (64K bytes) 0x7F-0x68=SUPER CD-ROM^2 buffer RAM (192K bytes)
	{	//↑0x68以上かよりも、0x87以下かどうかを先に判定することで高速化(I/Oアクセス0xFFだった場合等)。
		if ((mpr & 0xFC) != 0x80) //CD-ROM^2 buffer RAM (64K bytes)の前半32KB(0x80-0x83)以外の場合。ここに入れるとサザンアイズでピッタリ来る。前半32KBだけノーウェイトにすることでソーサリアンOPがピッタリ来る。スナッチャーのオープニングOK。サザンアイズOK。v2.09,v2.34,v2.44更新
			gCPU_ClockCount--; //Kitao追加。1アクセスウェイト(実機未確認)。スターブレーカー(0x69時トルネの港で必要)，ダブルドラゴンII(0x86時に必要)，サザンアイズで1が必要。大きいとフラッシュハイダース，クイズ殿様の野望で画面揺れ。v1.62,v2.45更新
		_pBufferRam[((mpr << 13) | addr) & 0x3FFFF] = data;
		return;
	}
	else
	{
		switch (mpr)
		{
 			//v1.15更新。頻繁に使われるものを先に置いて高速化
	 		case 0xFF:
				switch (addr & 0x1C00)
				{
					case 0x0000: // VDC
						VDC_Write(addr, data); //Kitao更新
						return;
					case 0x0400: // VCE
						VDC_VceWrite(addr, data); //v1.21更新。アドレスをそのまま渡して高速化。
						return;
					case 0x0800: // PSG
						_Buffer = data;
						PSG_Write(addr, data); //v1.21更新。アドレスをそのまま渡して高速化。
						return;
					case 0x0C00: // Timer
						_Buffer = data;
						TIMER_Write(addr, data); //v1.21更新。アドレスをそのまま渡して高速化。
						return;
					case 0x1000: // PAD
						_Buffer = data;
						JOYPAD_Write(addr, data);
						return;
					case 0x1400: // INT
						_Buffer = data;
						INTCTRL_Write(addr, data); //v1.21更新。アドレスをそのまま渡して高速化。
						return;
					case 0x1800: // CD-ROM^2
						if ((addr & 0x1E00) == 0x1A00) // ARCADE CARD
							ac_write(((mpr << 13) | addr), data);
						else
							CDROM_Write(addr, data);
						return;
				}
				return;

			case 0xF8:
			case 0xF9:
			case 0xFA:
			case 0xFB:
				_pMainRam[((mpr << 13) | addr) & _MainRamMask] = data;
				return;

			case 0x40: //Kitao更新。アーケードカード用RAM。ポピュラス(RAM内蔵カード)でも使われる。
			case 0x41:
			case 0x42:
			case 0x43:
				ac_write(0x1A00 | ((mpr << 4) & 0xF0), data);
				return;

			case 0xF7:
				if (CDROM_IsBRAMEnabled())
				{
					gCPU_ClockCount -= 22; //Kitao追加。アクセスウェイト
					if (addr <= 0x7FF) //Kitao追加。2KBだけ有効
						_pBackupRam[addr] = data;
				}
				return;
		}
	}
}


//Kitao追加。ROM領域部分にデータライト処理(デバッグ用)。v2.39
void
MAINBOARD_WriteROM(
	Uint32		mpr,
	Uint32		addr,
	Uint8		data)
{
	_pRom[((mpr << 13) | addr) & _RomMask] = data;
}


//Kitao追加。データライト処理を定期的に更新(一定の値に保つ)し続けるかどうかを設定。v2.39
void
MAINBOARD_SetContinuousWriteValue(
	BOOL		bContinuous,
	Uint32		mpr,
	Uint32		addr,
	Uint8		data)
{
	_bContinuousWriteValue = bContinuous;
	_ContWriteMpr		= mpr;
	_ContWriteAddress	= addr;
	_ContWriteData		= data;
}


//Kitao追加。CD-ROMを認識＆ゲームごとの設定をおこなう。
static void
setGameSetting()
{
	char*	pGameFileNameBuf;

	_SystemTimeCheck = 0; //起動時に自動的な動作をさせる場合、これを設定する。自動動作をさせない場合(通常)は0。v1.15追加
	_bKonamiStereo = FALSE;
	_bGradiusII = FALSE;
	VDC_SetWorldStadium91(FALSE);
	PSG_SetHoneyInTheSky(FALSE);
	INPUT_ResetGameSetting();
	_bAutoDisconnectMultiTap = FALSE;
	JOYPAD_SetRenshaSpeedMax(0);
	if (_AutoRealStretched != -1)
	{
		APP_SetStartStretchMode(_AutoRealStretched); //元のストレッチモードへ戻す。v2.64
		_AutoRealStretched = -1;
	}

	//※前ゲームで自動で設定したOverClock設定，３ボタンパッド設定は、APP.c のrestore_setting()で、すでに元に戻し済み。

	//CD-ROMの初期化処理。CDゲームプレイ時は、各ゲームごとのパッチが当てられる。
	if (CDROM_Init() <= 0)
	{
		if ((APP_GetCDGame())&&(!APP_GetCueFile()))
			PRINTF("ERROR: CD-ROM drive is not found.");
	}

	pGameFileNameBuf = APP_GetGameFileNameBuf(); //Kitao追加。ゲームのファイル名

	//６ボタンパッド用ソフトを自動認識。
	//ラスタ割り込み処理のタイミングが合わないソフトの場合、ラスタ割り込みタイミングを調整する。その他特別な処理。
	if (_RomSize == 2621440) //ストリートファイター２’
	{
		if ((_pRom[0x10] == 0x02)&&
			(_pRom[0x11] == 0x04)&&
			(_pRom[0x12] == 0xA9)&&
			(_pRom[0x13] == 0x01)&&
			(_pRom[0x14] == 0x8D)&&
			(_pRom[0x15] == 0x03)) //ストリートファイター２’
			{
				JOYPAD_UseSixButton(TRUE);
				PRINTF("Connected 6-button pad.");
			}
	}
	if (_RomSize == 1048576) //8M
	{
		if ((_pRom[0x30] == 0x3F)&&
			(_pRom[0x31] == 0xA9)&&
			(_pRom[0x32] == 0x73)&&
			(_pRom[0x33] == 0x8D)&&
			(_pRom[0x34] == 0x81)&&
			(_pRom[0x35] == 0x3F)) //パロディウスだ！
			if (APP_GetAutoStereo()) //オートステレオ設定なら
			{	//起動時に１ボタンを押しっぱなしにして自動的にステレオ起動する
				_bKonamiStereo = TRUE;
				INPUT_SetKonamiStereo(TRUE);
				_SystemTimeCheck = 3*60; //１ボタンを離すフレーム。起動から３秒後
			}
		if ((_pRom[0x10] == 0x20)&&
			(_pRom[0x11] == 0x72)&&
			(_pRom[0x12] == 0xE2)&&
			(_pRom[0x13] == 0xA9)&&
			(_pRom[0x14] == 0x1F)&&
			(_pRom[0x15] == 0x85)) //問題作ストリップファイター２
			{
				JOYPAD_UseSixButton(TRUE);
				PRINTF("Connected 6-button pad.");
			}
	}
	if (_RomSize == 786432) //6M
	{
		if ((_pRom[0x10] == 0x88)&&
			(_pRom[0x11] == 0xF0)&&
			(_pRom[0x12] == 0x03)&&
			(_pRom[0x13] == 0x4C)&&
			(_pRom[0x14] == 0x95)&&
			(_pRom[0x15] == 0xFB)) //パワーリーグ５
				VDC_SetAutoPerformSpriteLimit(TRUE); //打席結果表示の際にスプライト欠けを再現する必要がある
		if ((_pRom[0x10] == 0x88)&&
			(_pRom[0x11] == 0xF0)&&
			(_pRom[0x12] == 0x03)&&
			(_pRom[0x13] == 0x4C)&&
			(_pRom[0x14] == 0x48)&&
			(_pRom[0x15] == 0xFB)) //パワーリーグ'93
				VDC_SetAutoPerformSpriteLimit(TRUE); //打席結果表示の際にスプライト欠けを再現する必要がある
	}
	if (_RomSize == 524288) //4M
	{
		if (((_pRom[0x10] == 0x73)&&
			 (_pRom[0x11] == 0x00)&&
			 (_pRom[0x12] == 0x20)&&
			 (_pRom[0x13] == 0x01)&&
			 (_pRom[0x14] == 0x20)&&
			 (_pRom[0x15] == 0xFF))|| //シュビビンマン２
			(strstr(pGameFileNameBuf,"ShockMan (U)") != NULL))
			{
				VDC_SetForceRaster(TRUE); //序盤＆やられたときに１フレームの乱れが出るのを防ぐ。※実機でも出るが綺麗になるので実施。
				VDC_SetForceVBlank(TRUE); //３面スタート時(潜水艦面クリア後)に１フレームの乱れが出るのを防ぐ。※実機でどうかは未確認。時間のあるときに確認。
				JOYPAD_SetRenshaSpeedMax(2); //連射速度を最大Middleまでに抑制する。
			}
		if ((_pRom[0x20] == 0xA9)&&
			(_pRom[0x21] == 0x01)&&
			(_pRom[0x22] == 0x85)&&
			(_pRom[0x23] == 0x00)&&
			(_pRom[0x24] == 0xA2)&&
			(_pRom[0x25] == 0xFF)) //バルンバ
			{
				if (!JOYPAD_GetConnectThreeButton()) //２ボタンパッドを使用している場合
				{
					JOYPAD_UseThreeButton(TRUE);
					PRINTF("Connected 3-button pad.");
				}
				//if (!JOYPAD_GetConnectThreeButton()) //２ボタンパッドを使用している場合
				//{
				//	JOYPAD_SetSwapSelRun(TRUE); //セレクトボタンとランボタンをスワップする。
				//	PRINTF("Swapped Select&Run Buttons.");
				//}
			}
		if (((_pRom[0x30] == 0xA9)&&
			 (_pRom[0x31] == 0x0A)&&
			 (_pRom[0x32] == 0x99)&&
			 (_pRom[0x33] == 0xE0)&&
			 (_pRom[0x34] == 0x30)&&
			 (_pRom[0x35] == 0xA9))|| //レジェント・オブ・ヒーロートンマ
			(strstr(pGameFileNameBuf,"Legend of Hero Tonma (U)") != NULL))
				JOYPAD_SetRenshaSpeedMax(2); //連射速度を最大Middleまでに抑制する。
		if (((_pRom[0x30] == 0x9A)&&
			 (_pRom[0x31] == 0xA9)&&
			 (_pRom[0x32] == 0x01)&&
			 (_pRom[0x33] == 0x53)&&
			 (_pRom[0x34] == 0x40)&&
			 (_pRom[0x35] == 0xA9))|| //エアロブラスターズ
			(strstr(pGameFileNameBuf,"Aero Blasters (U)") != NULL))
				JOYPAD_SetRenshaSpeedMax(2); //連射速度を最大Middleまでに抑制する。
		if ((_pRom[0x30] == 0x03)&&
			(_pRom[0x31] == 0x14)&&
			(_pRom[0x32] == 0x03)&&
			(_pRom[0x33] == 0x05)&&
			(_pRom[0x34] == 0x13)&&
			(_pRom[0x35] == 0x08)) //はなたーかだか
				JOYPAD_SetRenshaSpeedMax(2); //連射速度を最大Middleまでに抑制する。
		if (((_pRom[0x30] == 0xF7)&&
			 (_pRom[0x31] == 0x20)&&
			 (_pRom[0x32] == 0x42)&&
			 (_pRom[0x33] == 0xEF)&&
			 (_pRom[0x34] == 0x64)&&
			 (_pRom[0x35] == 0x24))|| //スプラッターハウス
			((_pRom[0x30] == 0x65)&&
			 (_pRom[0x31] == 0x20)&&
			 (_pRom[0x32] == 0x2E)&&
			 (_pRom[0x33] == 0xF4)&&
			 (_pRom[0x34] == 0x20)&&
			 (_pRom[0x35] == 0x32))) //スプラッターハウス海外版
			{
				VDC_SetAutoPerformSpriteLimit(TRUE); //エンディングスタッフロールでスプライト欠けを再現する必要がある
				VDC_SetAutoRasterTiming(3); //ステージ５中盤(手の出てくるところ)で左下わずかにちらつくのを解消。実機でもちらついていたかもしれないが快適にプレイ。
				JOYPAD_SetRenshaSpeedMax(3); //連射速度を最大Lowまでに抑制する。
			}
		if (((_pRom[0x30] == 0xE4)&&
			 (_pRom[0x31] == 0x20)&&
			 (_pRom[0x32] == 0x99)&&
			 (_pRom[0x33] == 0xE4)&&
			 (_pRom[0x34] == 0xA2)&&
			 (_pRom[0x35] == 0x14))||
			(strstr(pGameFileNameBuf,"Bloody Wolf (U)") != NULL)) //ブラッディウルフ
				VDC_SetAutoPerformSpriteLimit(TRUE); //ボス登場時にスプライト欠けを再現する必要がある
		if ((_pRom[0x30] == 0x02)&&
			(_pRom[0x31] == 0xE1)&&
			(_pRom[0x32] == 0x20)&&
			(_pRom[0x33] == 0xED)&&
			(_pRom[0x34] == 0xFA)&&
			(_pRom[0x35] == 0x20)) //忍者龍剣伝
				VDC_SetAutoPerformSpriteLimit(TRUE); //ビジュアルシーンでスプライト欠けを再現する必要がある
		if (((_pRom[0x30] == 0xA9)&&
			 (_pRom[0x31] == 0x0C)&&
			 (_pRom[0x32] == 0x53)&&
			 (_pRom[0x33] == 0x08)&&
			 (_pRom[0x34] == 0x1A)&&
			 (_pRom[0x35] == 0x53))||
			(strstr(pGameFileNameBuf,"New Adventure Island (U)") != NULL)) //高橋名人の新冒険島
				VDC_SetAutoPerformSpriteLimit(TRUE); //STAGE2-3の海面で敵キャラが隠れるためにスプライト欠けを再現する必要がある。v2.57
		if ((_pRom[0x10] == 0x88)&&
			(_pRom[0x11] == 0xF0)&&
			(_pRom[0x12] == 0x03)&&
			(_pRom[0x13] == 0x4C)&&
			(_pRom[0x14] == 0x33)&&
			(_pRom[0x15] == 0xFB)) //パワーリーグ４
				VDC_SetAutoPerformSpriteLimit(TRUE); //打席結果表示の際にスプライト欠けを再現する必要がある
		if ((_pRom[0x30] == 0xFF)&&
			(_pRom[0x31] == 0x9A)&&
			(_pRom[0x32] == 0x20)&&
			(_pRom[0x33] == 0xBE)&&
			(_pRom[0x34] == 0xE0)&&
			(_pRom[0x35] == 0x4C)) //桃太郎活劇
				VDC_SetAutoPerformSpriteLimit(TRUE); //うらしまの村で、水の中に落ちたときにスプライト欠けを再現する必要がある
		if ((_pRom[0x30] == 0xE3)&&
			(_pRom[0x31] == 0x8D)&&
			(_pRom[0x32] == 0x00)&&
			(_pRom[0x33] == 0x00)&&
			(_pRom[0x34] == 0xE8)&&
			(_pRom[0x35] == 0xBD)) //熱血高校サッカー編
				VDC_SetAutoPerformSpriteLimit(TRUE); //スプライト欠けを再現
		if ((_pRom[0x30] == 0x3F)&&
			(_pRom[0x31] == 0xA9)&&
			(_pRom[0x32] == 0x05)&&
			(_pRom[0x33] == 0x85)&&
			(_pRom[0x34] == 0x5E)&&
			(_pRom[0x35] == 0x8D)) //ＰＣ電人
				VDC_SetAutoPerformSpriteLimit(TRUE); //スプライト欠けを再現
		if ((_pRom[0x30] == 0x00)&&
			(_pRom[0x31] == 0x73)&&
			(_pRom[0x32] == 0x00)&&
			(_pRom[0x33] == 0x20)&&
			(_pRom[0x34] == 0x01)&&
			(_pRom[0x35] == 0x20)) //魔動王グランゾート
				VDC_SetAutoPerformSpriteLimit(TRUE); //スプライト欠けを再現。スタート時の主人公フェードイン時に必要。
		if ((_pRom[0x30] == 0x0A)&&
			(_pRom[0x31] == 0xB9)&&
			(_pRom[0x32] == 0x8C)&&
			(_pRom[0x33] == 0xE0)&&
			(_pRom[0x34] == 0xD1)&&
			(_pRom[0x35] == 0x00)) //レーシング魂
				VDC_SetAutoPerformSpriteLimit(TRUE); //スプライト欠けを再現
		if ((_pRom[0x10] == 0xF8)&&
			(_pRom[0x11] == 0x53)&&
			(_pRom[0x12] == 0x02)&&
			(_pRom[0x13] == 0x64)&&
			(_pRom[0x14] == 0x7D)&&
			(_pRom[0x15] == 0xA9)) //S.C.I.
				VDC_SetAutoPerformSpriteLimit(TRUE); //スプライト欠けを再現。面クリア時にスプライトのゴミが残ることがあるため必要
		if ((_pRom[0x30] == 0xB3)&&
			(_pRom[0x31] == 0xE3)&&
			(_pRom[0x32] == 0xB3)&&
			(_pRom[0x33] == 0xE3)&&
			(_pRom[0x34] == 0xB3)&&
			(_pRom[0x35] == 0xE3)) //ちびまる子ちゃん
				VDC_SetAutoPerformSpriteLimit(TRUE); //スプライト欠けを再現
/*		if ((_pRom[0x00] == 0x5A)&&
			(_pRom[0x01] == 0x45)&&
			(_pRom[0x02] == 0x52)&&
			(_pRom[0x03] == 0x4F)&&
			(_pRom[0x04] == 0x34)&&
			(_pRom[0x05] == 0x20)) //ゼロヨンチャンプ
				VDC_SetAutoRasterTiming(1); //※実機も稀に乱れあり。v2.31ゼロヨンチャンプは実機に近い動作を優先のほうが良さそうなのでカット。
*/
		if ((_pRom[0x30] == 0x2E)&&
			(_pRom[0x31] == 0x86)&&
			(_pRom[0x32] == 0x33)&&
			(_pRom[0x33] == 0x20)&&
			(_pRom[0x34] == 0x8C)&&
			(_pRom[0x35] == 0xED)) //ダイハード
				VDC_SetAutoRasterTiming(1); //※実機も乱れあり
		if ((_pRom[0xA0] == 0x53)&&
			(_pRom[0xA1] == 0x02)&&
			(_pRom[0xA2] == 0xF3)&&
			(_pRom[0xA3] == 0x16)&&
			(_pRom[0xA4] == 0xE2)&&
			(_pRom[0xA5] == 0x00)) //カダッシュ海外版
				VDC_SetAutoRasterTiming(1);
		if ((_pRom[0x10] == 0x53)&&
			(_pRom[0x11] == 0x10)&&
			(_pRom[0x12] == 0x62)&&
			(_pRom[0x13] == 0x53)&&
			(_pRom[0x14] == 0x80)&&
			(_pRom[0x15] == 0x4C)) //アウトラン
				VDC_SetAutoRasterTiming(11);
		if (strstr(pGameFileNameBuf,"Order of the Griffon (U)") != NULL)
		{	//Thanks for this report, Tom.
			_AutoRealStretched = APP_GetStartStretchMode(); //自動変更前のストレッチ設定を保存。v2.64
			if (_AutoRealStretched != 1)
				APP_SetStartStretchMode(1); //v2.64。//設定ファイルには保存せず、一時的にリアルストレッチへ変更。v2.64
			VDC_SetAutoRasterTiming(4);
		}
	}
	if (_RomSize == 393216) //3M
	{
		if ((_pRom[0x10] == 0x88)&&
			(_pRom[0x11] == 0x05)&&
			(_pRom[0x12] == 0x89)&&
			(_pRom[0x13] == 0xF0)&&
			(_pRom[0x14] == 0x03)&&
			(_pRom[0x15] == 0x4C)) //パワーリーグ３
				VDC_SetAutoPerformSpriteLimit(TRUE); //打席結果表示の際にスプライト欠けを再現する必要がある
		if (((_pRom[0x30] == 0x02)&&
			 (_pRom[0x31] == 0x14)&&
			 (_pRom[0x32] == 0xD4)&&
			 (_pRom[0x33] == 0x78)&&
			 (_pRom[0x34] == 0xA2)&&
			 (_pRom[0x35] == 0xFF))|| //チェイスＨ.Ｑ.
			(strstr(pGameFileNameBuf,"Taito Chase H.Q. (U)") != NULL))
				VDC_SetAutoPerformSpriteLimit(TRUE); //スタート時にゴミが出ないようにスプライト欠けを再現する必要がある
		if ((_pRom[0x30] == 0x20)&&
			(_pRom[0x31] == 0x5E)&&
			(_pRom[0x32] == 0x40)&&
			(_pRom[0x33] == 0x80)&&
			(_pRom[0x34] == 0x03)&&
			(_pRom[0x35] == 0x20)) //カットビ宅配くん
				VDC_SetAutoPerformSpriteLimit(TRUE); //ステータス表示部の下にキャラが隠れるように、スプライト欠けを再現する必要がある。
		if (((_pRom[0x30] == 0xCD)&&
			 (_pRom[0x31] == 0xE0)&&
			 (_pRom[0x32] == 0x4C)&&
			 (_pRom[0x33] == 0xFA)&&
			 (_pRom[0x34] == 0xE3)&&
			 (_pRom[0x35] == 0x48))|| //パワーゴルフ
			(strstr(pGameFileNameBuf,"Power Golf (U)") != NULL))
				VDC_SetAutoPerformSpriteLimit(TRUE); //電源投入デモのスコア表示時に、スプライト欠けを再現する必要がある。
		if (((_pRom[0x10] == 0x2D)&&
			 (_pRom[0x11] == 0x43)&&
			 (_pRom[0x12] == 0x10)&&
			 (_pRom[0x13] == 0x48)&&
			 (_pRom[0x14] == 0xA9)&&
			 (_pRom[0x15] == 0x10))|| //虎への道
			(strstr(pGameFileNameBuf,"Tiger Road (U)") != NULL))
			{
				VDC_SetAutoPerformSpriteLimit(TRUE); //ステータス表示部の下に敵キャラが隠れるように、スプライト欠けを再現する必要がある。
				VDC_SetAutoRasterTiming(4);
			}
		if ((_pRom[0x30] == 0x3F)&&
			(_pRom[0x31] == 0x85)&&
			(_pRom[0x32] == 0x10)&&
			(_pRom[0x33] == 0x8D)&&
			(_pRom[0x34] == 0x02)&&
			(_pRom[0x35] == 0x00)) //バーニングエンジェル
				VDC_SetAutoPerformSpriteLimit(TRUE); //ステータス表示部の下にキャラが隠れるように、スプライト欠けを再現する必要がある。
		if ((_pRom[0xA0] == 0x09)&&
			(_pRom[0xA1] == 0xE2)&&
			(_pRom[0xA2] == 0x05)&&
			(_pRom[0xA3] == 0x20)&&
			(_pRom[0xA4] == 0xFB)&&
			(_pRom[0xA5] == 0x1F)) //カダッシュ
				VDC_SetAutoRasterTiming(1);
		if ((_pRom[0x30] == 0x02)&&
			(_pRom[0x31] == 0x3C)&&
			(_pRom[0x32] == 0x8D)&&
			(_pRom[0x33] == 0x03)&&
			(_pRom[0x34] == 0x3C)&&
			(_pRom[0x35] == 0x8D)) //バリバリ伝説
				VDC_SetAutoRasterTiming(1);
		if (((_pRom[0x30] == 0x08)&&
			 (_pRom[0x31] == 0x1A)&&
			 (_pRom[0x32] == 0x53)&&
			 (_pRom[0x33] == 0x10)&&
			 (_pRom[0x34] == 0x1A)&&
			 (_pRom[0x35] == 0x53))|| //デビルクラッシュ
			(strstr(pGameFileNameBuf,"Devil's Crush (U)") != NULL))
				VDC_SetAutoRasterTiming(3);
	}
	if (_RomSize == 262144) //2M
	{
		if (((_pRom[0x30] == 0x00)&&
			 (_pRom[0x31] == 0x02)&&
			 (_pRom[0x32] == 0x73)&&
			 (_pRom[0x33] == 0x3F)&&
			 (_pRom[0x34] == 0xE3)&&
			 (_pRom[0x35] == 0xD0))|| //グラディウス
			((_pRom[0x30] == 0xEE)&&
			 (_pRom[0x31] == 0x10)&&
			 (_pRom[0x32] == 0x21)&&
			 (_pRom[0x33] == 0xA9)&&
			 (_pRom[0x34] == 0xFF)&&
			 (_pRom[0x35] == 0x85)))  //沙羅曼蛇
			if (APP_GetAutoStereo()) //オートステレオ設定なら
			{	//起動時に１ボタンを押しっぱなしにして自動的にステレオ起動する
				_bKonamiStereo = TRUE;
				INPUT_SetKonamiStereo(TRUE);
				_SystemTimeCheck = 3*60; //１ボタンを離すフレーム。起動から３秒後
			}
		if ((_pRom[0x40] == 0x31)&&
			(_pRom[0x41] == 0x32)&&
			(_pRom[0x42] == 0x2D)&&
			(_pRom[0x43] == 0x31)&&
			(_pRom[0x44] == 0x37)&&
			(_pRom[0x45] == 0x20)) //ワールドスタジアム'91
				VDC_SetWorldStadium91(TRUE); //盗塁の画面切り替え時に１フレーム乱れる現象（実機でも起こる）を解消。v2.64
		if ((_pRom[0x30] == 0xE0)&&
			(_pRom[0x31] == 0x20)&&
			(_pRom[0x32] == 0xCA)&&
			(_pRom[0x33] == 0x9A)&&
			(_pRom[0x34] == 0x20)&&
			(_pRom[0x35] == 0x52)) //はにいいんざすかい
				PSG_SetHoneyInTheSky(TRUE); //ポーズ時に実機との微妙なタイミング違いによるプチノイズが出てしまうのを抑制。v2.60更新
		if ((_pRom[0x10] == 0x9D)&&
			(_pRom[0x11] == 0x26)&&
			(_pRom[0x12] == 0x33)&&
			(_pRom[0x13] == 0xA9)&&
			(_pRom[0x14] == 0x01)&&
			(_pRom[0x15] == 0x9D)) //妖怪道中記
			{
				JOYPAD_ConnectMultiTap(FALSE);//マルチタップを無効に。
				_bAutoDisconnectMultiTap = TRUE;//過去バージョンのステートセーブを読み込んだ場合も無効をキープし続けるために必要。
				JOYPAD_SetRenshaSpeedMax(2); //連射速度を最大Middleまでに抑制する。
				VDC_SetAutoPerformSpriteLimit(TRUE); //スコア表示欄に敵が被ったときにスプライト欠けを再現する必要がある
			}
		if (((_pRom[0x30] == 0x1A)&&
			 (_pRom[0x31] == 0x78)&&
			 (_pRom[0x32] == 0xA2)&&
			 (_pRom[0x33] == 0xFF)&&
			 (_pRom[0x34] == 0x9A)&&
			 (_pRom[0x35] == 0x20))|| //ドラゴンスピリット
			(strstr(pGameFileNameBuf,"Dragon Spirit (U)") != NULL))
				JOYPAD_SetRenshaSpeedMax(2); //連射速度を最大Middleまでに抑制する。
		if (((_pRom[0x30] == 0x8D)&&
			 (_pRom[0x31] == 0x04)&&
			 (_pRom[0x32] == 0x04)&&
			 (_pRom[0x33] == 0xA9)&&
			 (_pRom[0x34] == 0x00)&&
			 (_pRom[0x35] == 0x8D))|| //パラノイア
			(strstr(pGameFileNameBuf,"Psychosis (U)") != NULL))
				JOYPAD_SetRenshaSpeedMax(2); //連射速度を最大Middleまでに抑制する。
		if (((_pRom[0x30] == 0x85)&&
			 (_pRom[0x31] == 0x17)&&
			 (_pRom[0x32] == 0x86)&&
			 (_pRom[0x33] == 0x16)&&
			 (_pRom[0x34] == 0x60)&&
			 (_pRom[0x35] == 0x68))|| //アドベンチャーアイランド
			(strstr(pGameFileNameBuf,"Dragon's Curse (U)") != NULL))
				VDC_SetAutoPerformSpriteLimit(TRUE); //マウスマン時のボスキャラでスプライト欠けを再現する必要がある
		if (((_pRom[0x60] == 0xA2)&&
			 (_pRom[0x61] == 0xFF)&&
			 (_pRom[0x62] == 0x9A)&&
			 (_pRom[0x63] == 0x20)&&
			 (_pRom[0x64] == 0x1B)&&
			 (_pRom[0x65] == 0xE1))|| //パックランド
			(strstr(pGameFileNameBuf,"Pac-Land (U)") != NULL))
				VDC_SetForceRaster(TRUE); //面セレクト直後に１フレームの乱れが出るのを防ぐ。※実機でも出るが綺麗になるので実施。
		if ((_pRom[0x10] == 0xC2)&&
			(_pRom[0x11] == 0x80)&&
			(_pRom[0x12] == 0x02)&&
			(_pRom[0x13] == 0xA0)&&
			(_pRom[0x14] == 0x01)&&
			(_pRom[0x15] == 0xA2)) //ソンソン２
				VDC_SetAutoRasterTiming(1);
		if ((_pRom[0x10] == 0xFF)&&
			(_pRom[0x11] == 0x9A)&&
			(_pRom[0x12] == 0x20)&&
			(_pRom[0x13] == 0x2D)&&
			(_pRom[0x14] == 0xEC)&&
			(_pRom[0x15] == 0xA6)) //麻雀ウォーズ
				VDC_SetAutoRasterTiming(3);
	}
}

unsigned char RAMByteReader( unsigned int nOffs )
{
	return MAINBOARD_GetpMainRam()[ nOffs ];
}

void RAMByteWriter( unsigned int nOffs, unsigned int nVal )
{
	MAINBOARD_GetpMainRam()[ nOffs ] = nVal;
}

//Kitao追加。CDROM.c から呼ばれる。v1.21
void
MAINBOARD_SetGradiusII()
{
	_bGradiusII = TRUE;
	_SystemTimeCheck = _SystemTime + 1*60; //コントローラ２の下キーを離すフレーム。現在のフレームから１秒後
}


//Kitao追加。早回し用のカウンタを初期化する
static void
resetFastForwardingCount()
{
	_FastForwardingCount = 1;
}


//Kitao追加。早回し機能のための処理。早回し中はフレームをスキップする。
//fastForwarding。0…通常。
//				  2…1.5倍速モード。3…1.33倍速モード。4…1.25倍速モード。5以降(1+(1/n))倍速モード。
//				  102…2倍速モード。103…3倍速モード。104…4倍速モード。105以降(n-100)倍速モード。
//				  1000…1.83倍速モード(2倍と違いフレームがバラけるのでフレームスキップの影響が少ないため、特別に採用)。
//				  1001…1.67倍速モード(2倍と違いフレームがバラけるのでフレームスキップの影響が少ないため、特別に採用)。
//				  2002…0.5倍速モード。2004…0.75倍速モード。2010…0.90倍速モード。
static inline int
fastForwardingCheck()
{
	//妖怪道中記,ワールドコート,はにいいんざすかい,パワードリフト,サイコチェイサーの４分割画面。v2.27更新
	//※現状は、１フレーム内のCPUの速度が実機より速い。ただし単順にCPUパワーを0.5倍に設定するとタイミング等の問題で実機より遅くなる。
	//  余裕ができたら、その辺りも実機に近づける。
	//※シャーロックホームズ,シャドーオブザビースト,TVスポーツバスケ,あすか120%は、横512だが倍速にしない。
	if ((VDC_GetTvWidth() == 512)&&(APP_GetNonstretchedWidth() <= 256)) //横512ドットモードで、ソースが横256以下の場合におそらく、４分割画面でVSyncが倍速になる。
	{
		if (!_bFourSplitScreen)
		{
			_PrevFourSplitCpuSpeed = VDC_GetOverClockType();
			//VDC_SetOverClock(-2); //１フレームぶんのCPUパワーを半分にする。これで実機と同じ速度になる。※実機より遅くなるので現状はカット
			_bFourSplitScreen = TRUE;
		}
		if (_FastForwarding == 2002) //0.5倍速モードの場合
			return 1; //通常の速度にする
		else if (_FastForwarding == 2004) //0.75倍速モードの場合
		{	//1.50倍速にする
			if (_FastForwardingCount-- <= 2)
			{
				if (_FastForwardingCount == 0)
					_FastForwardingCount = 2 + 1; //3回に2回だけ描画(＆ウェイト)する
				return 1;
			}
		}
		else if (_FastForwarding == 2010) //0.90倍速モードの場合
		{	//1.83倍速にする
			if ((_FastForwardingCount-- % 2) == 1) //奇数(1,3,5,7,9,11)のときだけ描画
			{
				if (_FastForwardingCount == 0)
					_FastForwardingCount = 11; //11回に6回だけ描画(＆ウェイト)する
				return 1;
			}
		}
		else
		{
			if ((_FastForwardingCount-- % 2) == 1) //奇数(1)のときだけ描画
			{
				if (_FastForwardingCount == 0)
					_FastForwardingCount = 2; //2回に1回だけ描画(＆ウェイト)する。
				return 1;
			}
		}
		return 0;
	}
	else if (_bFourSplitScreen)
	{
		//VDC_SetOverClock(_PrevFourSplitCpuSpeed); //CPU速度を元に戻す。※実機より遅くなるので現状はカット
		_bFourSplitScreen = FALSE;
	}	

	if (_FastForwarding == 0)
		return 1;
	else if (_FastForwarding == 1000) //1.83倍速モードの場合
	{
		if ((_FastForwardingCount-- % 2) == 1) //奇数(1,3,5,7,9,11)のときだけ描画
		{
			if (_FastForwardingCount == 0)
				_FastForwardingCount = 11; //11回に6回だけ描画(＆ウェイト)する
			return 1;
		}
	}
	else if (_FastForwarding == 1001) //1.67倍速モードの場合
	{
		if ((_FastForwardingCount-- % 2) == 1) //奇数(1,3,5)のときだけ描画
		{
			if (_FastForwardingCount == 0)
				_FastForwardingCount = 5; //5回に3回だけ描画(＆ウェイト)する
			return 1;
		}
	}
	else if (_FastForwarding <= 100)
	{
		if (_FastForwardingCount-- <= _FastForwarding)
		{
			if (_FastForwardingCount == 0)
				_FastForwardingCount = _FastForwarding + 1; //(_FastForwarding+1)回に(_FastForwarding)回だけ描画(＆ウェイト)する
			return 1;
		}
	}
	else if (_FastForwarding == 2002) //0.5倍速モードの場合
		return 2; //２回VBlank待ちを行う
	else if (_FastForwarding >= 2003) //0.67倍速以下のモードの場合
	{
		if (_FastForwardingCount-- <= _FastForwarding-2000-2)
		{
			if (_FastForwardingCount == 0)
				_FastForwardingCount = _FastForwarding-2000-1; //(_FastForwarding-1)回に(_FastForwarding-2)回は、１回だけのVBlank待ち。
			return 1;
		}
		return 2; //２回VBlank待ちを行う
	}
	else //(_FastForwarding > 100) ２倍速以上
	{
		if (--_FastForwardingCount == 0)
		{
			_FastForwardingCount = (_FastForwarding-100); //(_FastForwardingCount1)回に１回だけ描画(＆ウェイト)する
			return 1;
		}
	}
	return 0;
}

//Kitao追加。早回しモードのための変数を設定する
void
MAINBOARD_SetFastForwarding(
	Sint32	fastForwarding,
	BOOL	bSoundAjust,
	BOOL	bReset)
{
	_FastForwarding = fastForwarding;
	_bSoundAjust = bSoundAjust;
	if (bReset)
	{
		_FastForwardingR = fastForwarding; //_FastForwardingは早回しボタンの状況によって値が変化するので、初期値を退避しておく。
		resetFastForwardingCount(); //早回し処理用のカウンタをリセット
	}
}

//Kitao追加。現在実施されている早回し状況を返す。早回しボタンが押されていないときは早回し無しの値を返す。v2.38更新
Sint32
MAINBOARD_GetFastForwarding()
{
	return _FastForwarding;
}

//Kitao追加。早回しボタンの状況にかかわらない値(メニュー上の設定値)を返す。v2.38更新
Sint32
MAINBOARD_GetFastForwardingR()
{
	return _FastForwardingR; //_FastForwardingは早回しボタンの状況によって値が変化するので、退避しておいた初期値のほうを返す。
}


/*-----------------------------------------------------------------------------
** [Init]
**   ハードウェアの実行準備を整えます。
**---------------------------------------------------------------------------*/
BOOL
MAINBOARD_Init(
	const char*		pGameName)
{
	int		i, j;
	char	buf[512];
	char	buf2[5];
	Uint8	tgbad1[6] = {0x02, 0x04, 0x72, 0xA2, 0xC2, 0x04}; //Kitao追加
	Uint8	tgbad2[4] = {0x95, 0xFF, 0xCA, 0x80}; //Kitao追加
	Uint8	tgbad3[6] = {0x32, 0xCF, 0x07, 0x32, 0x3F, 0x17}; //Kitao追加
	BOOL	bBad; //Kitao追加
	BOOL	bStreetFighter2 = FALSE; //Kitao追加
	BOOL	bPopulous = FALSE; //ポピュラス(RAM内蔵カード)起動の場合TRUE。v2.44
	char	fn[MAX_PATH+1];
	char*	pOtherSys1OpenName; //v2.07追加
	BOOL	bCDGameChanged = FALSE; //v2.40追加

	_bSystemCard = FALSE; //Kitao追加

	// at least pGameName needs to be valid
	if (pGameName == NULL)
		return FALSE;

	// Kitao追加。拡張子が.pce .hes .rom .bin 以外ならエラーにする。v1.00
	//（他のエミュレータのセーブファイル等を誤って開いた場合にレジュームファイルが壊れてしまうのを防ぐために必要）
	strcpy(fn, pGameName); //pGameNameは書き換えないようにするためfnを使う
	if ((strstr(APP_StrToLower(APP_ExtractFileExt(fn)), "pce") == NULL)&&
		(strstr(APP_StrToLower(APP_ExtractFileExt(fn)), "hes") == NULL)&&
		(strstr(APP_StrToLower(APP_ExtractFileExt(fn)), "rom") == NULL)&&
		(strstr(APP_StrToLower(APP_ExtractFileExt(fn)), "bin") == NULL))
	{
		sprintf(buf, "Couldn't Open \"%s\".    ", APP_GetGameFilePathName());
		MessageBox(WINMAIN_GetHwnd(), buf, "Ootake", MB_OK);
		return FALSE;
	}

	// カートリッジを読み込む
	if ((_RomMask = CART_LoadCartridge(pGameName, &_pRom, &_RomSize)) == 0) //Kitao追加。ROMの容量も_RomSizeに設定されるようにした。
	{
		sprintf(buf, "Couldn't Open \"%s\".    ", APP_GetGameFilePathName());
		MessageBox(WINMAIN_GetHwnd(), buf, "Ootake", MB_OK);
		return FALSE;
	}

	//Kitao追加。v.1.11。TG16のターボチップを日本向けの吸出し機で吸い出したままのBad-imageかどうかをチェック。- Mr.Tom2007, thanks for your cooperation.
	bBad = FALSE;
	for (i=0; i<0x2000; i++)
	{
		for (j=0; j<6; j++)
			if (_pRom[i+j] != tgbad1[j])
				break;
			else if (j == 5)
				bBad = TRUE;
	}
	for (i=0; i<0x06; i++)
	{
		for (j=0; j<4; j++)
			if (_pRom[i+j] != tgbad2[j])
				break;
			else if (j == 3)
				bBad = TRUE;
		for (j=0; j<6; j++)
			if (_pRom[i+j] != tgbad3[j])
				break;
			else if (j == 5)
				bBad = TRUE;
	}
	if (bBad)
	{
		sprintf(buf, "\"%s\"\n"
					 " This is a \"BAD(data is shuffled)\" ROM-image file.\n"
					 " Operating it in this state contradicts the reason.\n\n"
					 " It is possible to convert it into a \"just proper\" image file now.    \n"
					 " It can be used with [a lot of other Free TG16/PCE] emulators, too.    \n"
					 " The conversion is started.\n\n"
					 " In Japanese language\n"
					 " Huカード規格で吸い出されてしまったROMイメージです。\n"
					 " 適切なTG16規格に変換したファイルをinstallフォルダへ出力します。", APP_GetGameFileNameBuf());
		if (MessageBox(WINMAIN_GetHwnd(), buf, "Ootake", MB_YESNO) == IDYES)
			MAINBOARD_TG16BitConvert();
		return FALSE;
	}

	//Kitao追加。スーパーグラフィックス用ソフトなら、スーパーグラフィックスモードに自動で切り替える。
	_SuperGrafx = 0;
	if (_RomSize == 1048576) //8M
	{
		if ((_pRom[0x30] == 0x22)&&
			(_pRom[0x31] == 0x20)&&
			(_pRom[0x32] == 0x7F)&&
			(_pRom[0x33] == 0xE5)&&
			(_pRom[0x34] == 0x20)&&
			(_pRom[0x35] == 0xA8)) //大魔界村
				_SuperGrafx = 1;
		if ((_pRom[0x30] == 0x62)&&
			(_pRom[0x31] == 0x8D)&&
			(_pRom[0x32] == 0x12)&&
			(_pRom[0x33] == 0x00)&&
			(_pRom[0x34] == 0x8D)&&
			(_pRom[0x35] == 0x13)) //１９４１
				_SuperGrafx = 1;
		if ((_pRom[0x30] == 0x24)&&
			(_pRom[0x31] == 0x8D)&&
			(_pRom[0x32] == 0x00)&&
			(_pRom[0x33] == 0x00)&&
			(_pRom[0x34] == 0xA9)&&
			(_pRom[0x35] == 0x0E)) //オルディネス
				_SuperGrafx = 1;
	}
	if (_RomSize == 786432) //6M
	{
/*		if ((_pRom[0x00] == 0x44)&&
			(_pRom[0x01] == 0x41)&&
			(_pRom[0x02] == 0x52)&&
			(_pRom[0x03] == 0x49)&&
			(_pRom[0x04] == 0x55)&&
			(_pRom[0x05] == 0x53)) //ダライアスプラス(両対応) 現状はノーマルモードでもちらつかない＆処理が速いのでノーマルモードで。
				_SuperGrafx = 1;
*/
	}
	if (_RomSize == 524288) //4M
	{
		if ((_pRom[0x30] == 0x9C)&&
			(_pRom[0x31] == 0x13)&&
			(_pRom[0x32] == 0x00)&&
			(_pRom[0x33] == 0xF3)&&
			(_pRom[0x34] == 0xA5)&&
			(_pRom[0x35] == 0xE0)) //バトルエース
				_SuperGrafx = 1;
		if ((_pRom[0x30] == 0x00)&&
			(_pRom[0x31] == 0x73)&&
			(_pRom[0x32] == 0x00)&&
			(_pRom[0x33] == 0x20)&&
			(_pRom[0x34] == 0x01)&&
			(_pRom[0x35] == 0x20)) //魔動王グランゾート
				_SuperGrafx = 1;
	}
	MAINBOARD_SetSuperGrafx(_SuperGrafx); //アドレスマスク＆VDCの変数＆Windowのキャプションも更新される。

	//Kitao追加
	if (_RomSize == 2621440) //20M
	{
		if ((_pRom[0x10] == 0x02)&&
			(_pRom[0x11] == 0x04)&&
			(_pRom[0x12] == 0xA9)&&
			(_pRom[0x13] == 0x01)&&
			(_pRom[0x14] == 0x8D)&&
			(_pRom[0x15] == 0x03)) //ストリートファイター２’
				bStreetFighter2 = TRUE;
	}

	//Kitao追加。システムカード用処理。
	//			 吸出し機によってはRAM部分も吸い出す可能性があるので、容量は絞らないようにした。アーケードカードのオーバー吸出しもOK。v2.43更新
	if ((_pRom[0x00] == 0x4C)&&
		(_pRom[0x01] == 0xE7)&&
		(_pRom[0x02] == 0xE0)&&
		(_pRom[0x03] == 0x4C)&&
		(_pRom[0x0E] == 0xEE)&&
		(_pRom[0x0F] == 0x4C))	//v1.00
			_bSystemCard = TRUE;
	if ((_pRom[0x00] == 0x4C)&&
		(_pRom[0x01] == 0xF3)&&
		(_pRom[0x02] == 0xE0)&&
		(_pRom[0x03] == 0x4C)&&
		(_pRom[0x0E] == 0xED)&&
		(_pRom[0x0F] == 0x4C))	//v2.10,v3.00
			_bSystemCard = TRUE;
	if ((_pRom[0x00] == 0x4C)&&
		(_pRom[0x01] == 0xB3)&&
		(_pRom[0x02] == 0xEA)&&
		(_pRom[0x03] == 0x4C)&&
		(_pRom[0x0E] == 0xE9)&&
		(_pRom[0x0F] == 0x4C))	//ゲームエクスプレスカード
			_bSystemCard = TRUE;
	//Kitao追加。ポピュラス(RAMメモリ内蔵カード)用処理
	if ((_pRom[0x30] == 0xAE)&&
		(_pRom[0x31] == 0xE4)&&
		(_pRom[0x32] == 0x9C)&&
		(_pRom[0x33] == 0x02)&&
		(_pRom[0x34] == 0x14)&&
		(_pRom[0x35] == 0x58)) //ポピュラス
			bPopulous = TRUE; //内蔵RAMへのアクセスのため、CD-ROMシステムと同じI/Oメモリマップで動かす。

	//Kitao追加。特定の吸出し機（3Mbit未対応）により3Mが4MにオーバーダンプされてしまったROMイメージをチェック
	if (_RomSize == 524288) //4M
	{
		bBad = TRUE;
		for (i=0x40000; i<0x60000; i++) //0x40000～と0x60000～のデータが全て一致した場合、オーバーダンプと見なす。
			if (_pRom[i] != _pRom[i+0x20000])
			{
				bBad = FALSE;
				break;
			}
		if (bBad)
		{
			sprintf(buf, "\"%s\"\n"
						 " This is a \"BAD(over dumped)\" ROM-image file.\n"
						 " Operating it in this state contradicts the reason.\n\n"
						 " It is possible to convert it into a \"just proper\" image file now.    \n"
						 " It can be used with [a lot of other Free TG16/PCE] emulators, too.    \n"
						 " The conversion is started.\n\n"
						 " In Japanese language\n"
						 " オーバーダンプされてしまったROMイメージです。\n"
						 " 適切なサイズ（3Mbit）に変換したファイルをinstallフォルダへ出力します。", APP_GetGameFileNameBuf());
			if (MessageBox(WINMAIN_GetHwnd(), buf, "Ootake", MB_YESNO) == IDYES)
				MAINBOARD_OverDumpedConvert();
			return FALSE;
		}
	}

	// メモリを確保する
	_pMainRam    = (Uint8*)malloc(0x8000); //Kitao更新。スーパーグラフィックスに対応。32KB
	_pOldMainRam = (Uint8*)malloc(0x2000); //過去バージョンのステートファイルを読み込み用
	_pBackupRam = (Uint8*)malloc(0x2000);
	_pBufferRam = (Uint8*)malloc(0x40000);
	memset(_pMainRam, 0, 0x8000); //Kitao更新。スーパーグラフィックスに対応。32KB
	memset(_pBufferRam, 0, 0x40000);
	memset(_AcRam, 0, 0x200000); //Kitao追加。ACカードバッファメモリをクリア
	//Kitao追加。バックアップラムを初期化する
	memset(_pBackupRam, 0, 0x2000);
	_pBackupRam[0] = 0x48;
	_pBackupRam[1] = 0x55;
	_pBackupRam[2] = 0x42;
	_pBackupRam[3] = 0x4D;
	_pBackupRam[4] = 0x00;
	_pBackupRam[5] = 0x88; //v1.37更新。"ネオネクタリス"と"マクロス愛のラブソング"でセーブ可能になった。
	_pBackupRam[6] = 0x10;
	_pBackupRam[7] = 0x80;
	if (_bBackupFull) //バックアップRAMの空き容量を0にするモード中の場合
	{
		_pBackupRam[ 7] = 0xFF;
		_pBackupRam[16] = 0xDE; //ゲッツェンディーナーで必要
	}
	//Kitao追加。バックアップラムの最初の状態を退避しておく。終了時に更新された形跡があったときだけ、バックアップラムを保存するようにした。
	//           こうすることで、間違えてステートロードしてしまった時でも、すぐ終了すれば元のバックアップラムファイルは残ってくれる。＆バックアップの必要のないゲームはファイルが不要になる。
	memcpy(&_FirstBackupRam, _pBackupRam, 0x2000);
	//Kitao追加。メモリベース128を初期化する
	MB128_Init();

	_bAcUse = FALSE; //Kitao追加。アーケードカードのメモリを使用した形跡があればTRUEになる。
	_bContinuousWriteValue = FALSE; //Kitao追加。定期的に１つのアドレスに１つの固定値を書き込み続ける(値を一定に保つ)処理を行う場合TRUE。デバッグ用。v2.39
	WRITEMEM_ClearCode(); //Kitao追加。ゲームが変わったらWriteMemoryフォームのコードEDITのデフォルト値をクリアしておく。v2.39

	//４分割＆白黒画面関連の変数を初期化。v2.28
	if (_bFourSplitScreen)
	{
		//VDC_SetOverClock(_PrevFourSplitCpuSpeed); //CPU速度を元に戻す。※実機より遅くなるので現状はカット
		_bFourSplitScreen = FALSE;
	}	
	_bMonoColor = FALSE;
	_ForceMonoColor = APP_GetMonoColorMode();

	// RomMapを設定する
	for (i = 0; i < sizeof(_pRomMap)/sizeof(_pRomMap[0]); i++)
		_pRomMap[i] = _pRom + ((i * 0x2000) & _RomMask);

	// CPUのリード／ライト関数とゼロページメモリを登録する
	if (_bSystemCard || bPopulous || APP_GetCDGame() || APP_GetHesFile()) //Kitao更新。システムカード(現在動かないがゲームエクスプレスカードも含む)を起動した場合、または明示的にCD-ROMプレイの場合。hesファイル時もADPCMを考慮しCDのメモリマップにしておく。
	{	// CD-ROM用のI/Oメモリマップ
		CPU_SetReadFunction(cpu_read_func_cdrom2);
		CPU_SetWriteFunction(cpu_write_func_cdrom2);
		if (!APP_GetCDGame() && !bPopulous && !APP_GetHesFile()) //CDゲーム起動じゃなかった場合(ポピュラス，hesファイルは除く)
		{
			APP_SetCDGame(); //Openメニューからシステムカードを指定した場合もCDゲームモードにする。v2.40追加
			bCDGameChanged = TRUE;
		}
	}
	else
	{	// I/Oから CD-ROM^2 システムを切り離す。
		if (bStreetFighter2) //Kitao追加。スト２とそれ以外に処理を分けて高速化。
			CPU_SetReadFunction(cpu_read_func_sf2);
		else
			CPU_SetReadFunction(cpu_read_func_hucard);
		CPU_SetWriteFunction(cpu_write_func_hucard);
	}

	// 割り込みコントローラを初期化する
	INTCTRL_Init();

	// タイマーを初期化する 
	TIMER_Init();

	// 入力を初期化する 
	JOYPAD_Init();

	// VDCを初期化する
	VDC_Init(); //Kitao更新。ラスタタイミングをデフォルトに戻す。
				//自動で不具合対策のためのオーバークロックをしていた場合は、元の速度に戻す。
				//スプライト欠け再現をデフォルトに戻す。
				//ウェイトパッチをデフォルト(無効)に戻す。
				//強制ラスタ割り込み設定をデフォルト(無効)に戻す。
				//強制VBlank割り込み設定をデフォルト(無効)に戻す。
				//真女神転生用パッチを無効に戻す。

	// APUをデフォルトで初期化する
	if (!APU_Init(44100, 2048))
	{
		PRINTF("(NOTICE: MAINBOARD_Init: APU_Init failed.)");
	}

	//Kitao追加。CD-ROMを認識＆ゲームごとの設定をおこなう。
	setGameSetting();

	//Openメニューからシステムカードを指定してCDゲームモードにした場合、切り替えに必要な処理をする。v2.40追加
	if (bCDGameChanged)
		APP_SetGameFileNameBuf(); //ゲーム名を_GameFileNameBufに設定しておく（ステートセーブ時等で利用）

	//獣王記の問題(スーパーシステムカードでプレイすると途中で止まることがある)対策。v2.07
	//  実機でもスーパーシステムカード(v3.0)だと２面でドラゴンに変身出来ず止まる問題があるので旧システムカードを使用する。
	if ((APP_GetCDGame())&&(strcmp(TOCDB_GetGameTitle(),"Juuouki (J)") == 0))
	{
		pOtherSys1OpenName = APP_ChangeToOtherSysCard1();
		switch (CDROM_GetCDInstall())
		{
			case 1: //データトラックインストール済み
				strcpy(buf2, "Icd:"); //※buf2=最大4文字
				break;
			case 2: //データ＆音楽トラックフルインストール済み
				if (CDIF_GetBadInstalled())
					strcpy(buf2, "Bcd:"); //古いOotakeでリッピング失敗のイメージファイルの場合。v2.31追加
				else
					strcpy(buf2, "Fcd:");
				break;
			default:
				strcpy(buf2, "CD:");
				break;
		}
		if (pOtherSys1OpenName != NULL)
		{
			//イメージファイルを読み込み直す
			_RomMask = CART_LoadCartridge(pOtherSys1OpenName, &_pRom, &_RomSize);
			sprintf(buf, "\"Other SystemCard 1\" Set OK.  %s Juuouki (J)", buf2);
		}
		else //イメージファイルを読み込めなかった場合
			sprintf(buf, "Use \"SystemCard v2.1(or less)\".  %s Juuouki (J)", buf2);
		PRINTF(buf);
	}

	//CPUをリセット
	CPU_Reset();

	//Kitao追加。早回しモード用のカウンタをリセット
	resetFastForwardingCount();

	//Kitao追加。処理落ち検査用の変数を初期化
	_PrevAdvanceFrameTime = -1;
	for (i=0; i<300 ;i++)
		_ProcessingDelay[i] = 0;
	_ProcessingDelayIndex = 0;

	_bSystemInit = TRUE;
	_SystemTime = 0; //Kitao追加。v1.15

	_prevTvW = 0; //Kitao追加。v1.42

	RA_ClearMemoryBanks();
	RA_InstallMemoryBank(0,	RAMByteReader, RAMByteWriter, 0x8000);
	RA_OnLoadNewRom( MAINBOARD_GetpMainROM(), MAINBOARD_GetpMainROMSize());

	//Kitao追加。v1.11。メインウィンドウが最小化されていた場合(TG16変換のときにもこうなる)、元に戻す。
	//					それ以外の場合でも、ウィンドウを前面に出す効果。
	ShowWindow(WINMAIN_GetHwnd(), SW_SHOWNORMAL);

	return TRUE;
}


/*-----------------------------------------------------------------------------
	[Reset]
-----------------------------------------------------------------------------*/
void
MAINBOARD_Reset()
{
	_bAcUse = FALSE;//Kitao追加
	if (_bFourSplitScreen)
	{
		//VDC_SetOverClock(_PrevFourSplitCpuSpeed); //CPU速度を元に戻す。※実機より遅くなるので現状はカット
		_bFourSplitScreen = FALSE;
	}	
	_bMonoColor = FALSE;
	_ForceMonoColor = APP_GetMonoColorMode(); //v2.28追加
	VDC_Reset();//Kitao追加
	JOYPAD_Reset();//Kitao追加
	APU_Reset();//Kitao更新。フラッシュハイダースのパッチ当ての関係でCDROMより先にリセットする
	CDROM_Deinit();
	memset(_pMainRam, 0, 0x8000); //Kitao追加。メインメモリをクリア
	memset(_pBufferRam, 0, 0x40000); //Kitao追加。CD-ROMバッファメモリをクリア
	memset(_AcRam, 0, 0x200000); //Kitao追加。ACカードバッファメモリをクリア
	_bContinuousWriteValue = FALSE; //Kitao追加。定期的に１つのアドレスに１つの固定値を書き込み続ける(値を一定に保つ)処理を行う場合TRUE。デバッグ用。v2.39
	setGameSetting();//Kitao追加。CD-ROMを認識＆ゲームごとの設定をおこなう。
	CPU_Reset();
	resetFastForwardingCount(); //Kitao追加
	_SystemTime = 0; //Kitao追加。v1.15
	_prevTvW = 0; //Kitao追加。v1.42
}


/*-----------------------------------------------------------------------------
** [MAINBOARD_Deinit]
**   ハードウェアを破棄します。
**---------------------------------------------------------------------------*/
void
MAINBOARD_Deinit()
{
	if (!_bSystemInit)
		return;

	APU_Deinit();

	// メモリを解放する 
	free(_pMainRam);
	free(_pOldMainRam);
	free(_pBackupRam);
	free(_pBufferRam);

	// カートリッジを解放する 
	CART_FreeCartridge(_pRom);

	JOYPAD_Deinit();

	CDROM_Deinit();
	INTCTRL_Deinit();
	TIMER_Deinit();

	_bSystemInit = FALSE;
}


//Kitao追加。正常にゲームが読み込めてシステムが初期化されているかどうかを返す
BOOL
MAINBOARD_GetSystemInit()
{
	return _bSystemInit;
}


/*-----------------------------------------------------------------------------
	[SaveBRAM]
		BRAM を指定のファイルに保存します。
-----------------------------------------------------------------------------*/
BOOL
MAINBOARD_SaveBRAM(
	const char*		pathName)
{
	FILE*	bram;

	if (_bBackupFull) return FALSE; //バックアップRAMの空き容量を0にするモード中の場合は、保存しない。

	//Kitao追加。バックアップラムが更新された形跡があったときだけ、バックアップラムを保存するようにした。
	//           こうすることで、間違えてステートロードしてしまった時でも、すぐ終了すれば元のバックアップラムファイルは残ってくれる。＆バックアップの必要のないゲームはファイルが不要になる。
	if (memcmp(_pBackupRam, &_FirstBackupRam, 0x2000) != 0) //更新されていれば
	{
		bram = fopen(pathName, "wb");
		if (bram != NULL && _pBackupRam != NULL)
		{
			fwrite(_pBackupRam, sizeof(Uint8), 0x2000, bram);
			fclose(bram);
			//Kitao追加。更新直後のバックアップラムの状態を退避しておく。終了時(またはステートセーブ時)に前回から更新された形跡があったときだけ、バックアップラムを保存。
			memcpy(&_FirstBackupRam, _pBackupRam, 0x2000);
			return TRUE;
		}
	}

	return FALSE;
}


/*-----------------------------------------------------------------------------
	[LoadBRAM]
		BRAM を指定のファイルから読み込みます。
-----------------------------------------------------------------------------*/
BOOL
MAINBOARD_LoadBRAM(
	const char*		pathName)
{
	FILE*	bram;

	if (_bBackupFull) return FALSE; //バックアップRAMの空き容量を0にするモード中の場合は、読み込みしない。

	bram = fopen(pathName, "rb");
	if (bram != NULL && _pBackupRam != NULL)
	{
		fread(_pBackupRam, sizeof(Uint8), 0x2000, bram);
		fclose(bram);
		//Kitao追加。バックアップラムの最初の状態を退避しておく。終了時に更新された形跡があったときだけ、バックアップラムを保存するようにした。
		memcpy(&_FirstBackupRam, _pBackupRam, 0x2000);
		return TRUE;
	}

	return FALSE;
}


/*-----------------------------------------------------------------------------
	[MAINBOARD_AdvanceFrame]
		１フレーム分のエミュレーションを実行します。

	[DEV NOTE]

	IRQ について

		・IRQ は VDC, TIMER, CD-ROM^2 システムから発行される。
		・VDC からの IRQ は全て IRQ1 が使用される。
		・TIMER からの IRQ は TIRQ が使用される。
		・CD-ROM^2 からの IRQ は IRQ2 が使用される。

		・周辺装置が IRQ を要求したときは、MAINBOARD_AdvanceFrame の処理内で
		  その通知を受け取り、これを Interrupt Controller (IntCtrl)
		  に渡す。

		・IntCtrl は、通知された IRQ をＣＰＵが acknowledge するまで
		  記憶しておき、CLI が実行されたら即 IRQ を発動できるような
		  状態にする。

-----------------------------------------------------------------------------*/
void //Kitao更新。戻り値は不要とした。
MAINBOARD_AdvanceFrame() //Kitao更新。ストレッチのときも非ストレッチのときもここを使うようにした。
{
	Sint32		line = 0;
	Sint32		srcX = 0;
	Sint32		srcW;
	Sint32		tvW; //Kitao追加
	Uint32		bkColor; //Kitao追加
	RECT		rc,rc2; //Kitao追加
	int			x,w; //Kitao追加
	int 		i; //Kitao追加
	Sint32 		t; //Kitao追加
	Sint32		ly; //Kitao追加
	Sint32		startLine = VDC_GetTvStartLine(); //Kitao追加。様々な垂直表示開始位置にも対応。トイレキッズで確認。
	Sint32		drawFrame; //Kitao追加
	BOOL		bDraw; //Kitao追加
	Sint32		encodeLine; //v1.28追加。エンコードを終えたライン数を退避用。
	Sint32		encodeLineH; //v1.39追加。エンコードするラインの高さを計算用。
	Sint32		psgQuality; //v2.29追加

	if (!_bSystemInit || _bPause) //初期化前かポーズしているとき
	{
		Sleep(16); //Kitao追加。他のアプリにCPUを引き渡す。DirectXでV-Sync使用時はこれがないとCPU使用率100%に。
		_PrevAdvanceFrameTime = -1; //-1…次のフレームを処理落ちにカウントしない。
		return;
	}

	//Kitao追加。パソコン環境のパワーが足りずに処理落ちした場合、「処理落ちしたフレーム数」に加算。v1.21
	t = timeGetTime()-_PrevAdvanceFrameTime;
	if ((t >= 20)&&(_PrevAdvanceFrameTime != -1)) //前回から20ms以上(1/60s=16.7ms。誤差があるので20msで判定)掛かっていたら処理落ち
		_ProcessingDelay[_ProcessingDelayIndex] = (t - 3) / 17; //何フレームぶん遅れたかを記録
	else
		_ProcessingDelay[_ProcessingDelayIndex] = 0;
	if (++_ProcessingDelayIndex >= 300) //5秒(300フレーム)ぶんの処理落ち履歴を取り続ける。
		_ProcessingDelayIndex = 0;
	_PrevAdvanceFrameTime = timeGetTime();

	//Kitao追加。リセット後何フレームが経過したかを調べて、ゲーム毎に「起動直後の自動動作」をできるようにした。v1.15
	if (_SystemTime != 0xFFFFFFFF)
		if (++_SystemTime == _SystemTimeCheck)
		{
			if (_bKonamiStereo) //コナミの「グラディウス」「沙羅曼蛇」「パロディウスだ！」で自動的にステレオにする。
				INPUT_SetKonamiStereo(FALSE); //押しっぱなしにしていた１ボタンを離す
			if (_bGradiusII) //「グラディウスII」で自動的にレーザー・スプレッドボムをちらつかない設定＆ステレオ設定にする。
			{
				INPUT_SetGradiusII(FALSE); //押しっぱなしにしていた２コントローラ下ボタンを離す
				CPU_WriteMemoryMpr(0xF8, 0x005F, 0x01, FALSE); //ステレオモードに設定。v2.34
				CPU_WriteMemoryMpr(0xF8, 0x1F84, 0x00, FALSE); //メニュー表示上もステレオに。
			}
		}

	//Kitao追加。倍速モードの場合、表示をスキップするフレームかどうかを決定する。
	drawFrame = fastForwardingCheck(); //drawFrame 0…スキップ，1…描画(通常)，2…描画＆１フレームウェイト。４分割画面モードかどうか(_bFourSplitScreen)も判定される。

	//白黒画面モードかどうかをチェックする。v2.28
	if (_ForceMonoColor != 0)
		_bMonoColor = TRUE;
	else
		_bMonoColor = (VDC_GetDCC() & 0x80);

	//Kitao更新。VCEのドットクロックのとおりの解像度で表示するようにした。シューティングゲームの縦画面モードなどに対応。
	//			 横352の場合、実際のTVでは左右各8ドットぶんずつは見えず、実質的な表示解像度は336となる。
	//			 また横320の場合、実機では左右に(実機での)16ドットぶんずつの黒帯ができるようなのでそれを再現。※横304（ぷよぷよ。Wiz5など）も同様。
	tvW = VDC_GetTvWidth();

	//Kitao追加。横544モードのときは左右16ドットずつカットして中央512ドットをTV画面上に表示する。TVスポーツバスケの選手選択画面で確認。
	//			 横368モードのときは左8ドット・右24ドットをカットして中央336ドットをTV画面上に表示する。天地を喰らうで確認。
	//			 横352モードのときは左右8ドットずつカットして中央336ドットをTV画面上に表示する。アイレム系ゲームやシミュレーション系で多数。
	//			 横264モードのときは左から256ドットぶんをTV画面上に表示する。F1パイロットで確認。
	srcW = APP_GetNonstretchedWidth();

	//Kitao追加。ディスプレイのオート解像度変更モードの場合の処理。v1.35
	if (tvW != _prevTvW)
	{
		if (_bFullScreen)
			if (APP_GetResolutionAutoChange())
			{
				if (tvW == 256)
					APP_AutoChangeScreenMode(0);
				else if (tvW == 336)
					APP_AutoChangeScreenMode(1);
				else
					APP_AutoChangeScreenMode(5);
			}
	}

	//Kitao追加。ノンストレッチモードのときに解像度の変更があった場合の処理
	_bResolutionChange = FALSE;
	if (!_bStretched) //Kitao追加。ノンストレッチなら
	{
		if (_bFullScreen)
		{
			if (tvW != _prevTvW) //もし描画解像度が前回描画時と異なるなら、ゴミが残らないように画面全体をクリアする。
				_bResolutionChange = TRUE;
		}
		else //ノンストレッチウィンドウモードなら
		{
			if (_ScreenWidth != APP_GetNonstretchedWidth()*_Magnification) //もしウィンドウサイズが描画ソースと異なるなら、ウィンドウのサイズを変更する。
			{
				GetWindowRect(WINMAIN_GetHwnd(), &rc);//変更前のウィンドウ位置を記憶しておく
				_ScreenWidth = APP_GetNonstretchedWidth()*_Magnification; //v2.14更新
				WINMAIN_SetNormalWindow(_ScreenWidth, _ScreenHeight);//ウィンドウサイズ変更
				//ウィンドウを変更前の中央位置と合わせる
				GetWindowRect(WINMAIN_GetHwnd(), &rc2);//変更後のウィンドウ位置
				x = rc.left + (rc.right-rc.left)/2 - (rc2.right-rc2.left)/2;
				if (x < 0)
					x = 0;
				if (x+(rc2.right-rc2.left) > GetSystemMetrics(SM_CXSCREEN))
					x = GetSystemMetrics(SM_CXSCREEN) - (rc2.right-rc2.left);
				w = rc2.right-rc2.left;
				if (w > GetSystemMetrics(SM_CXSCREEN)) //大きすぎて左端に表示しても画面に収まらない場合は、収まらない部分をカットして対応。
				{
					w = GetSystemMetrics(SM_CXSCREEN);
					x = 0;
				}
				MoveWindow(WINMAIN_GetHwnd(), x, rc2.top, w, rc2.bottom-rc2.top, TRUE);
				
				MAINBOARD_ChangeScreenMode(_ScreenWidth, _ScreenHeight, _Magnification,
										   _bFullScreen, _FullScreenColor);
				APP_SetScreenWH(_ScreenWidth, _ScreenHeight); //App.cの変数も更新
			}
		}
	}
	else //ストレッチなら
	{
		if ((_ShowOverscanLeft > 0)||(APP_GetOverscanHideBlackLR())|| //左右のオーバースキャン領域を表示するなら
			(APP_GetOverscanHideBlackTop())||(APP_GetOverscanHideBlackBottom())) //上下に黒帯表示する場合
		{
			if (tvW != _prevTvW) //もし描画解像度が前回描画時と異なるなら、ゴミが残らないように画面全体をクリアする。
				_bResolutionChange = TRUE;
		}
	}
	_prevTvW = tvW;

	encodeLine = 16-_ShowOverscanTop; //v1.28追加。エンコードを終えたライン数を退避用。オーバースキャン領域を除く通常の開始ラインは[16]の位置から書き込みスタート。v2.00更新
	for (_ScanLine=0; _ScanLine<=MAX_SCANLINE-1; _ScanLine++) //Kitao更新。実記並のパフォーマンスを出すには、vblank前後にもっとCPUパワーが必要なため、１フレームを263ラインぶんと仮定した。
	{
		//定期的に値を書き込む場合の処理。デバッグ用。v2.39
		if (_bContinuousWriteValue)
			CPU_WriteMemoryMpr(_ContWriteMpr, _ContWriteAddress, _ContWriteData, TRUE);

		// １ライン分の画像を更新する 
		//		本プログラムでは画面のオフセット値を19に設定している 
		//		→ ライン#19-#258 の 240ラインを画面表示領域とする 
		//		→ 実際は上下８ラインはＴＶ画面から見えないので ライン#27-#250 の 224ラインを画面表示領域とする
		// ※Kitao更新。ソフトによっては#27が一番上ではないこともあるので、VDC.cで、VPRとVDWの値によって、描画開始ラインを決めるようにした。
		bDraw = FALSE;
		if ((drawFrame != 0)&& //Kitao追加。倍速モード時でスキップするフレームのときは、エンコードを省略して高速化。
			(_BigSoundCounter == 0)) //Kitao追加。ビッグサウンドバッファモードのときは6回に1度だけ画面描画をおこなう。
		{
			if	(((_ScanLine >= startLine)&&(_ScanLine < startLine+224))|| //通常描画範囲
				 ((_ShowOverscanTop > 0)&& //上側のオーバースキャン領域を描画。v1.43追加
				  ((_ScanLine >= startLine-_ShowOverscanTop)&&(_ScanLine < startLine)))||
				 ((_ShowOverscanBottom > 0)&& //下側のオーバースキャン領域を描画。v1.43追加
				  ((_ScanLine >= startLine+224)&&(_ScanLine < startLine+224+_ShowOverscanBottom))))
			{
				line = 16 + _ScanLine - startLine;
				bDraw = TRUE;
			}
		}
		if (bDraw)
		{
			if (line>255) ly=255;
			ly = line << 9; //*512
			//Kitao追加。ラスタ割り込みでラインごとに解像度を設定しているゲームのための処理
			_TvW[line] = (Uint16)VDC_GetTvWidth(); //各ラインの解像度
			srcW = VDC_GetScreenWidth(); //ソースの幅を更新。龍虎の拳で必要
			if (_ShowOverscanLeft > 0) //左右のオーバースキャンエリアを表示する場合。v1.43追加
			{
				switch (_TvW[line])
				{
					case 256:
						_TvW[line] = 268;
						if ((srcW > 256)&&(srcW < 272)) srcW = 256; //272に満たない場合は左側をオーバースキャン表示させないために256に設定。右側だけのオーバースキャンとなる。Ｆ１パイロット
						break;
					case 336:
						_TvW[line] = 352; //現状は左右8ドットに固定。
						if ((srcW > 336)&&(srcW < 352)) srcW = 336; //352に満たない場合は左側をオーバースキャン表示させないために336に設定。右側だけのオーバースキャンとなる。
						break;
				}
			}
			if (srcW < _TvW[line])
			{
				srcX = (_TvW[line] - srcW) / 2;
				//Kitao更新。左右の帯カラーは黒に決まらず「スプライト透明色」で埋めるようにした。天外２のデモなどで実機同様の動作になった。
				bkColor = VDC_GetSpColorZero();
				for (i=ly; i<ly+srcX; i++)
					_ScreenBuf[i] = bkColor;
				for (i=ly+srcX+srcW; i<ly+srcX+srcW+(_TvW[line]-(srcX+srcW)); i++)
					_ScreenBuf[i] = bkColor;
			}
			else
				srcX = 0; //闇の血族で必要。v1.42
			
			//Kitao更新。CPU動作も"VDC.c"のほうで進めるようにした。実機同様の動作にするには、「ラスタ割り込み処理の直後」にCPUの動作が入るのが理想。
			//  V1.11更新。VCEの処理もVDC.c で行うようにして高速化した。2次バッファ&エンコードをせずに、直接ScreenBufに書き込むことで大きく高速化できた。
			VDC_AdvanceLine(&_ScreenBuf[ly+srcX], drawFrame);
		}
		else
			VDC_AdvanceLine(NULL, drawFrame); //v1.11更新。表示範囲外の場合、表示処理を省いて高速化する。
		
		// おおよそ1/240秒単位でPSGのレジスタ更新を行い(早めに更新されてしまわないように待ち)、音色を実機に近づける。v1.39
		psgQuality = APP_GetPsgQuality();
		if (drawFrame == 0) //倍速モード時でスキップするフレームのときは、エンコードを省略。
			psgQuality = 0;
		else if (_bMonoColor) //白黒モード時は転送処理が少し重くなるのでエンコードの間隔を1段広げる。v2.29
			psgQuality >>= 1;
		if (psgQuality == 4)
		{
			if ((_ScanLine == MAX_SCANLINE/4)||
				(_ScanLine == MAX_SCANLINE/2)||
				(_ScanLine == MAX_SCANLINE/2+MAX_SCANLINE/4+1))
			{
				//画面の途中までをエンコードしておく
				if (_BigSoundCounter == 0)
				{
					encodeLineH = (16+_ScanLine-startLine) - encodeLine;
					if (encodeLineH < 0)
						encodeLineH = 0;
					SCREEN_Blt(_ScreenBuf, 0, encodeLine, _TvW, encodeLineH, 0);
					encodeLine += encodeLineH;
				}				
				//PSGの分解能を上げるために、ここで約1/240秒が経過するのを待つ。コントローラ入力の分解能も上がる。
				t = timeGetTime() - SCREEN_GetLastTimeSyncTime();
				if (_ScanLine == MAX_SCANLINE/2)
					t -= 4;
				if (_ScanLine == MAX_SCANLINE/2+MAX_SCANLINE/4+1)
					t -= 8-1; //-1は処理落ち防止のため（最終描画転送処理があるぶん）。
				//if (t <= 2) PRINTF("%d", t); //test
				if (t <= 3) //3ms以上(2.1ms以上)経過していた場合は待たない。理論上は4msまで待つようにしたい所(1/240s=4.17ms)だが、VRAM転送＆描画処理を後半にやらねばならないので、ここを2ms以下で処理できたPCだけ処理を行う。
				{
					t = 4-t; //4…１コマの時間約16msの1/4
					if ((t >= 1)&&(t <= 4)) //タイマーカウンタのオーバーフローも考えて、この範囲のときだけSleepをおこなう。
						Sleep(t);
				}
			}
		}
		else if (psgQuality == 2)
		{
			if (_ScanLine == MAX_SCANLINE/2)
			{
				//画面の途中までをエンコードしておく
				if (_BigSoundCounter == 0)
				{
					encodeLineH = (16+_ScanLine-startLine) - encodeLine;
					if (encodeLineH < 0)
						encodeLineH = 0;
					SCREEN_Blt(_ScreenBuf, 0, encodeLine, _TvW, encodeLineH, 0);
					encodeLine += encodeLineH;
				}
				
				//PSGの分解能を上げるために、ここで約1/120秒が経過するのを待つ。コントローラ入力の分解能も上がる。
				t = timeGetTime() - SCREEN_GetLastTimeSyncTime();
				if (t <= 6) //6ms以上(5.1ms以上)経過していた場合は待たない。
				{
					t = 8-t-1; //8…１コマの時間約16msの1/2。-1は処理落ち防止のため（最終描画転送処理があるぶん）。
					if ((t >= 1)&&(t <= 8)) //タイマーカウンタのオーバーフローも考えて、この範囲のときだけSleepをおこなう。
						Sleep(t);
				}
			}
		}
	}
	JOYPAD_ClearPrevUpdateStateLine(); //Kitao追加。Windowsパッドの状態を、前回更新した位置をリセット。

	if ((drawFrame != 0)&& //Kitao追加。倍速モード時でスキップするフレームのときは、エンコードを省略して高速化。
		(_BigSoundCounter == 0)) //Kitao追加。ビッグサウンドバッファモードのときは6回に1度だけ画面描画をおこなう。
	{
		// 最後(最下部)のエンコードを行い、画面表示を更新する。
		SCREEN_Blt(_ScreenBuf, 0, encodeLine, _TvW, 240-encodeLine+_ShowOverscanBottom, 1); //Kitao更新。VCEの解像度(tvW)で表示するようにした。
		_ScreenBufOk = TRUE; //Kitao追加。バッファにデータが入った印。現在非使用。
	}
	
	if (drawFrame == 2) //Kitao追加。倍速モード時でSlowのときここでウェイトフレームを入れる
	{
		for (i=1; i<=7159090/60; i++) //音が途切れないように１フレームぶんCD-ROMとAPUを進める
		{
			CDROM_AdvanceClock();
			APU_AdvanceClock(); //CDDA発声などのため、CDROM処理が終わってからAPU処理
		}
		SCREEN_WaitVBlank(FALSE); //垂直帰線期間を待つ。描画は行わない。
	}
	
	if (_BigSoundBuffer != 0)
	{
		if ((_BigSoundCounter != 0)&&(drawFrame != 0))
			SCREEN_WaitVBlank(FALSE); //垂直帰線期間を待つ。描画は行わない。
		if (++_BigSoundCounter == 6)
			_BigSoundCounter = 0;
	}

	_bResolutionChange = FALSE;

}

//Kitao追加
Sint32
MAINBOARD_GetScanLine()
{
	return _ScanLine;
}


//Kitao追加。画面を再描画する。
//ステートロード時は、ロードしたshowOverscan変数を入れて呼ぶ。（余分な画面クリアを避けるために使用）
void
screenUpdate(
	Sint32	showOverscanTop,
	Sint32	showOverscanBottom,
	Sint32	showOverscanLeft,
	Sint32	showOverscanRight)
{
	int		x,y;
	RECT	rc;
	Sint32	showFPS;

	if (!_bSystemInit) //WinMain.c から起動時に呼び出されたのをスルーするために必要
		return;

	if ((_bFullScreen)|| //フルスクリーン表示の場合、メニュー表示などではみだしてしまった部分が残らないようにクリアする。
		(showOverscanTop < _ShowOverscanTop)||(showOverscanBottom < _ShowOverscanBottom)||
		(showOverscanLeft < _ShowOverscanLeft)||(showOverscanLeft < _ShowOverscanLeft)) //描画するソースのほうが小さい場合
			SCREEN_Clear();

	//上下のオーバースキャン表示のサイズが合わない場合、黒帯となる部分をクリアする必要がある。
	if (showOverscanTop < _ShowOverscanTop) //ソースが小さい場合
	{	//隙間部分をクリア
		showOverscanTop = _ShowOverscanTop;
		for (y=16-_ShowOverscanTop; y<16-showOverscanTop; y++)
			for (x=0; x<512; x++)
				_ScreenBuf[y*512+x] = 0;
	}
	if (showOverscanBottom < _ShowOverscanBottom) //ソースが小さい場合
	{	//データが不定な最下部分をクリア
		for (y=16+224+showOverscanBottom; y<16+224+_ShowOverscanBottom; y++)
			for (x=0; x<512; x++)
				_ScreenBuf[y*512+x] = 0;
	}

	showFPS = APP_GetShowFPS(); //FPS表示設定を退避
	APP_SetShowFPS(FALSE); //FPS表示を解除
	if (showOverscanLeft == _ShowOverscanLeft)
		SCREEN_Blt(_ScreenBuf, 0, (16-_ShowOverscanTop), _TvW, 240-(16-_ShowOverscanTop)+_ShowOverscanBottom, _UpdateVsync); //Kitao更新。VCEの解像度(tvW)で表示するようにした。
	else if (showOverscanLeft < _ShowOverscanLeft) //描画するソースのほうが小さい場合
		SCREEN_Blt(_ScreenBuf, 0, (16-_ShowOverscanTop), _TvW, 240-(16-_ShowOverscanTop)+_ShowOverscanBottom, 3); //アスペクト比を合わせるため、左右に黒帯を配置しての転送。
	else //描画するソースのほうが大きい場合
		SCREEN_Blt(_ScreenBuf, 0, (16-_ShowOverscanTop), _TvW, 240-(16-_ShowOverscanTop)+_ShowOverscanBottom, 5); //アスペクト比を合わせるため、左右のオーバースキャン部をカットしての転送。
	APP_SetShowFPS(showFPS); //FPS表示設定を元に戻す

	if (APP_GetFullScreen()) //フルスクリーン表示の場合
	{
		if (!APP_GetRunning())
			APP_MenuShow(TRUE); //フルスクリーンの場合手動でメニュー表示する必要がある
	}
	else //ウィンドウ表示の場合
	{
		if (_bPause)
		{
			UpdateWindow(WINMAIN_GetHwnd()); //先にメニュー表示等を更新し、乱れを防ぐ。
			//InvalidateRect(WINMAIN_GetHwnd(), NULL, TRUE );
			//GetWindowRect(WINMAIN_GetHwnd(), &rc);
			//SetWindowPos(WINMAIN_GetHwnd(), HWND_NOTOPMOST, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_FRAMECHANGED); //常に手前に表示を解除。
		}
	}
}


//Kitao追加。画面を再描画する。APP.c，ScreenD3D.c，ScreenDD.c などから利用。
void
MAINBOARD_ScreenUpdate(
	BOOL	bVsync) //通常はbVsyncをTRUEにして呼ぶ。特にDirect3DはVsync待ちを行わないと描画も行われないので注意。
{
	if (!bVsync) _UpdateVsync=2;
	screenUpdate(_ShowOverscanTop,_ShowOverscanBottom,_ShowOverscanLeft,_ShowOverscanRight);
	_UpdateVsync=1; //常に1に戻しておく。
}


//Kitao追加。画面のエンコード処理のみ行う。Screen.c から利用。v2.43。現在非使用。
void
MAINBOARD_ScreenEncord()
{
	SCREEN_Blt(_ScreenBuf, 0, (16-_ShowOverscanTop), _TvW, 240-(16-_ShowOverscanTop)+_ShowOverscanBottom, 0);
}

//Kitao追加。エンコード等の処理は省き、画面の転送更新だけ行う。Screen.c から利用。v2.43。
void
MAINBOARD_ScreenUpdateFast()
{
	SCREEN_Blt(_ScreenBuf, 0, 240+_ShowOverscanBottom, _TvW, 0, 1); //V-Syncも行う。
}


//Kitao追加。スクリーンショット用に画面を再描画する。
void
MAINBOARD_DrawScreenshot()
{
	SCREEN_Blt(_ScreenBuf, 0, (16-_ShowOverscanTop), _TvW, 240-(16-_ShowOverscanTop)+_ShowOverscanBottom, 2);
}


//Kitao追加。画面(＆バッファ)をクリアする
void
MAINBOARD_ScreenClear()
{
	memset(_ScreenBuf, 0, sizeof(_ScreenBuf));
	SCREEN_SetMessageText("");
	screenUpdate(_ShowOverscanTop,_ShowOverscanBottom,_ShowOverscanLeft,_ShowOverscanRight);
}


//Kitao追加。ビッグサウンドバッファモードかどうかを設定する。ビッグサウンドバッファモードのときは描画を1/6に省いてサウンドへCPUパワーを回す。
void
MAINBOARD_SetBigSoundBuffer(
	Sint32 n)
{
	_BigSoundBuffer = n;
	_BigSoundCounter = 0;
}


//Kitao追加。現在の画面表示倍率を返す
Sint32
MAINBOARD_GetMagnification()
{
	return _Magnification;
}


//Kitao追加。エミュレータがポーズ中かどうかを返す
BOOL
MAINBOARD_GetPause()
{
	return _bPause;
}


//Kitao追加
static void
pause_sub(
	BOOL	bPause)
{
	APU_Pause(bPause); //ポーズ時は、音はノイズが出ないようにいち早く止めておく
	_bPause = bPause;
	_PrevAdvanceFrameTime = -1; //-1…次のフレームを処理落ちにカウントしない。
}

//Kitao追加
static void
pause_sub2(
	BOOL	bPause)
{
	char	buf[100];
	RECT	rc;

	if (!APP_GetFullScreen()) //Kitao追加。ウィンドウ表示の場合、PAUSE中は最前面を解除し、見出し部分にPAUSEメッセージ表示をする。
	{
		GetWindowRect(WINMAIN_GetHwnd(), &rc);
		strcpy(buf, APP_GetAppName());
		strcat(buf, " - Pause.");
		if (bPause)
		{
			SetWindowPos(WINMAIN_GetHwnd(), HWND_NOTOPMOST, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_FRAMECHANGED); //常に手前に表示を解除。
			SetWindowPos(WINMAIN_GetHwnd(), HWND_TOPMOST, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_FRAMECHANGED); //常に手前に表示に戻す。タスクバーなどに重なっていた場合のごみをクリアするために必要。
			SetWindowPos(WINMAIN_GetHwnd(), HWND_NOTOPMOST, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_FRAMECHANGED); //再び、常に手前に表示を解除。
			WINMAIN_SetCaption(buf);
		}
		else
		{
			if (APP_GetWindowTopMost())
				SetWindowPos(WINMAIN_GetHwnd(), HWND_TOPMOST, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_FRAMECHANGED); //常に手前に表示に戻す。
			if (strcmp(WINMAIN_GetCaption(), buf) == 0)
				WINMAIN_SetCaption(NULL);
		}
	}
}

//Kitao追加。ウィンドウ表示の場合、見出し部分にPAUSEメッセージ表示をするようにした。
void
MAINBOARD_Pause(
	BOOL	bPause)
{
	if (!_bSystemInit) //WinMain.c から起動時に呼び出されたのをスルーするために必要
		return;

	RA_SetPaused( bPause==TRUE );

	if (bPause)
		pause_sub(TRUE);
	pause_sub2(bPause);
	if (!bPause)
		pause_sub(FALSE);
}

//Kitao追加。PAUSEメッセージ非表示用
void
MAINBOARD_PauseNoMessage(
	BOOL	bPause)
{
	RA_SetPaused( bPause==TRUE );
	pause_sub(bPause);
}


/*-----------------------------------------------------------------------------
	[ChangeScreenMode]
		スクリーンの設定を変更します。
-----------------------------------------------------------------------------*/
//Kitao更新。v0.91
//  scanLineType,bStretched(bVStretched)とbSync60HzScreenは、
//  変更されてもWindowsの画面設定を変える必要がないのでここでのパラメータからは外した。
BOOL
MAINBOARD_ChangeScreenMode(
	Sint32		screenWidth,
	Sint32		screenHeight,
	Sint32		magnification,
	BOOL		bFullScreen,
	Uint32		fullScreenColor) //Kitao更新。fullScreenColor…フルスクリーン時のbitsPerPixel
{
	_ScreenWidth = screenWidth; //Kitao追加
	_ScreenHeight = screenHeight; //Kitao追加
	_Magnification = magnification;
	_bFullScreen = bFullScreen; //Kitao追加。ウィンドウサイズ変更のため（ノンストレッチのとき変更されることがある）にウィンドウモードかどうかを調べるときに必要。
	_FullScreenColor = fullScreenColor; //Kitao追加

	_Flags = SCREEN_FDEFAULT;
	if (bFullScreen)
		_Flags |= SCREEN_FFULLSCREEN;

	return SCREEN_Init(screenWidth, screenHeight, magnification, fullScreenColor, _Flags);
}


/*-----------------------------------------------------------------------------
	[ChangeSoundMode]
		サウンドの設定を変更します。
-----------------------------------------------------------------------------*/
BOOL
MAINBOARD_ChangeSoundMode(
	Uint32		bufferSize,
	Uint32		sampleRate,
	Uint32		masterVolume,
	BOOL		bReverb)
{
	APU_Deinit();

	if (!APU_Init(sampleRate, bufferSize))
	{
		PRINTF("MAINBOARD_ChangeSoundMode:  failed changing sound mode.");
		return FALSE;
	}

	APU_SetVolume(masterVolume);

	return TRUE;
}


/*-----------------------------------------------------------------------------
	[ChangeMemoryValue]
		メモリの内容を変更します。
-----------------------------------------------------------------------------*/
void
MAINBOARD_ChangeMemoryValue(
	Sint32		ramType,
	Uint32		addr,
	Uint8		data)
{
	if (_pMainRam == NULL)
		return;

	if (_pBufferRam == NULL)
		return;

	switch (ramType)
	{
		case MAINBOARD_MAINRAM:
			_pMainRam[addr & _MainRamMask] = data;
			break;
		case MAINBOARD_BUFFERRAM:
			_pBufferRam[addr & 0x3FFFF] = data;
			break;
		case MAINBOARD_ARCADERAM:
			_AcRam[addr & 0x1FFFFF] = data;
			break;
	}
}


//Kitao追加。スーパーグラフィックスモードに設定または解除する。
void
MAINBOARD_SetSuperGrafx(
	Sint32	superGrafx)
{
	char	buf[100]; //Kitao追加

	_SuperGrafx = superGrafx;

	//スーパーグラフィックスのときはMainRamアクセス時のアドレスマスクを変更する。
	if (_SuperGrafx == 1)
	{
		strcpy(buf, "Super ");
		strcat(buf, MAINCAPTION);
		_MainRamMask = 0x7FFF; //32KB用
	}
	else
	{
		strcpy(buf, MAINCAPTION);
		_MainRamMask = 0x1FFF; //8KB用
	}
	APP_SetAppName(buf);
	WINMAIN_SetCaption(buf);

	//VDC.c の変数も更新
	VDC_SetSuperGrafx(_SuperGrafx);
}

//Kitao追加。スーパーグラフィックスモードかどうかを返す。
Sint32
MAINBOARD_GetSuperGrafx()
{
	return _SuperGrafx;
}


//Kitao追加。v1.11。アーケードカード使用モードの設定。両対応ソフトでアーケードカードをあえて使わないならFALSEに設定する。
void
MAINBOARD_SetArcadeCard(
	BOOL	arcadeCard)
{
	_bArcadeCard = arcadeCard;
}

//Kitao追加。v1.11。アーケードカードをあえて使わないモードならFALSEを返す。
BOOL
MAINBOARD_GetArcadeCard()
{
	return _bArcadeCard;
}


//Kitao追加。v1.49
void
MAINBOARD_SetBackupFull(
	BOOL	backupFull)
{
	_bBackupFull = backupFull;
}

//Kitao追加。v1.49
BOOL
MAINBOARD_GetBackupFull()
{
	return _bBackupFull;
}


//Kitao追加。ROMメモリを日本のPCエンジン向けにBIT変換してファイルに保存
void
MAINBOARD_TG16BitConvert()
{
	Uint32	addr;
	Sint32	offset;
	Uint8	src;
	Uint8	dst;
	char	fileName[MAX_PATH+1];
	FILE*	fp;

	//BIT変換
	if (_RomSize == 393216) //3Mの場合
		offset = 0x40000;
	else
		offset = 0;
	for (addr=offset; addr<offset+_RomSize; addr++)
	{
		dst = 0x00;
		src = _pRom[addr];
		if (src & 0x01) dst |= 0x80;
		if (src & 0x02) dst |= 0x40;
		if (src & 0x04) dst |= 0x20;
		if (src & 0x08) dst |= 0x10;
		if (src & 0x10) dst |= 0x08;
		if (src & 0x20) dst |= 0x04;
		if (src & 0x40) dst |= 0x02;
		if (src & 0x80) dst |= 0x01;
		_pRom[addr] = dst;
	}

	//installフォルダ内に保存
	strcpy(fileName, APP_GetAppPath());
	strcat(fileName, "install");
	CreateDirectory(fileName, NULL);//installディレクトリがない場合作る
	strcat(fileName, "\\");
	strcat(fileName, APP_GetGameFileNameBuf());
	strcat(fileName, ".pce");
	
	fp = fopen(fileName, "wb");
	if (fp != NULL)
	{
		fwrite(_pRom+offset, _RomSize, 1, fp);
		fclose(fp);
		sprintf(fileName, "Conversion was completed.\n\n"
						  "Correct \"%s.pce\" was saved into the \"intall\" folder.    \n"
						  "The \"install\" folder is opened.    ", APP_GetGameFileNameBuf());
		MessageBox(WINMAIN_GetHwnd(), fileName, "Ootake", MB_OK);
		APP_OpenInstallFolder();
		CloseWindow(WINMAIN_GetHwnd()); //メインウィンドウを最小化
	}
	
	memset(_pRom, 0, _RomSize); //このあとで中途半端に起動してしまうことを防ぎ、真っ白画面にする。
}


//Kitao追加。特定の吸出し機（3Mbit未対応）で3Mが4MにオーバーダンプされてしまったROMイメージを3Mに修正する。
void
MAINBOARD_OverDumpedConvert()
{
	char	fileName[MAX_PATH+1];
	FILE*	fp;

	//installフォルダ内に保存
	strcpy(fileName, APP_GetAppPath());
	strcat(fileName, "install");
	CreateDirectory(fileName, NULL);//installディレクトリがない場合作る
	strcat(fileName, "\\");
	strcat(fileName, APP_GetGameFileNameBuf());
	strcat(fileName, ".pce");
	
	fp = fopen(fileName, "wb");
	if (fp != NULL)
	{
		fwrite(_pRom, 393216, 1, fp); //3Mサイズで保存。
		fclose(fp);
		sprintf(fileName, "Conversion was completed.\n\n"
						  "Correct \"%s.pce\" was saved into the \"intall\" folder.    \n"
						  "The \"install\" folder is opened.    ", APP_GetGameFileNameBuf());
		MessageBox(WINMAIN_GetHwnd(), fileName, "Ootake", MB_OK);
		APP_OpenInstallFolder();
		CloseWindow(WINMAIN_GetHwnd()); //メインウィンドウを最小化
	}
	
	memset(_pRom, 0, _RomSize); //このあとで中途半端に起動してしまうことを防ぎ、真っ白画面にする。
}


//Kitao追加。ストレッチ関連の変数を更新する。(常にAPP.c のストレッチ変数と同じ内容にする。速度アップのためにこの構造に。)
void
MAINBOARD_SetStretched(
	BOOL	bStretched,
	BOOL	bVStretched)
{
	_bStretched = bStretched;
	_bVStretched = bVStretched;
}


//Kitao追加。リセットからの経過時間(単位はフレーム)を１フレーム経過させる。APP.c から使用。v1.15
void
MAINBOARD_IncSystemTime()
{
	if (_SystemTime != 0xFFFFFFFF)
		_SystemTime++;
}


//Kitao追加。処理落ちテスト用。App.c から利用。v1.21
int
MAINBOARD_GetProcessingDelay()
{
	int		i;
	int		n = 0;

	//この30秒間で処理落ちしたフレーム数を集計して返す。
	for (i=0; i<300; i++)
		if (_ProcessingDelay[i] != 0)
			n++;
	return n;
}

//Kitao追加。時間内に表示できたフレーム数を返す。DDSCREEN.c から利用。v1.50
int
MAINBOARD_GetDisplayedFrames()
{
	int		i,j,k;
	int		n = 0;

	//この60コマ間で処理落ちしたフレーム数を集計
	i = _ProcessingDelayIndex;
	k = 60; //最大60フレームぶんチェック。処理落ちがあった場合そのぶんチェックフレーム数を減らす。
	j = 0;
	while (j < k)
	{
		if (--i < 0) i = 299;
		if (_ProcessingDelay[i] != 0)
		{
			n += _ProcessingDelay[i];
			k -= _ProcessingDelay[i];
		}
		j++;
	}
	//表示できたフレーム数を計算
	n = 60 - n;
	if (n < 1) n = 1;
	return n;
}


//Kitao追加。表示中のゲーム画面の横解像度の退避変数をリセットする。
void
MAINBOARD_ResetPrevTvW()
{
	_prevTvW = 0;
}


//Kitao追加。フルスクリーン時に描画解像度の変更があったかどうかを返す。
BOOL
MAINBOARD_GetResolutionChange()
{
	return _bResolutionChange;
}

//Kitao追加。スクリーンショット時に使用
void
MAINBOARD_SetResolutionChange(
	BOOL	bResolutionChange)
{
	_bResolutionChange = bResolutionChange;
}


//Kitao追加。４分割画面モード中かどうかを返す。v2.27
BOOL
MAINBOARD_GetFourSplitScreen()
{
	return _bFourSplitScreen;
}

//Kitao追加。v2.27
BOOL
MAINBOARD_GetMonoColor()
{
	return _bMonoColor;
}

//Kitao追加。v2.28
Sint32
MAINBOARD_GetForceMonoColor()
{
	return _ForceMonoColor;
}

//Kitao追加。v2.28
void
MAINBOARD_SetForceMonoColor(
	Sint32	forceMonoColor)
{
	_ForceMonoColor = forceMonoColor;
}


//Kitao追加
Sint32
MAINBOARD_GetShowOverscanTop()
{
		return _ShowOverscanTop;
}

//Kitao追加
Sint32
MAINBOARD_GetShowOverscanBottom()
{
	return _ShowOverscanBottom;
}

//Kitao追加
Sint32
MAINBOARD_GetShowOverscanLeft()
{
	return _ShowOverscanLeft;
}

//Kitao追加
Sint32
MAINBOARD_GetShowOverscanRight()
{
	return _ShowOverscanRight;
}

//Kitao追加
BOOL
MAINBOARD_GetShowOverscan()
{
	if ((_ShowOverscanTop > 0)||(_ShowOverscanBottom > 0)||(_ShowOverscanLeft > 0)||(_ShowOverscanRight > 0))
		return TRUE;
	else
		return FALSE;
}

//Kitao追加
void
MAINBOARD_SetShowOverscanTop(
	Sint32	showOverscanTop)
{
	_ShowOverscanTop = showOverscanTop;
}

//Kitao追加
void
MAINBOARD_SetShowOverscanBottom(
	Sint32	showOverscanBottom)
{
	_ShowOverscanBottom = showOverscanBottom;
}

//Kitao追加
void
MAINBOARD_SetShowOverscanLeft(
	Sint32	showOverscanLeft)
{
	_ShowOverscanLeft = showOverscanLeft;
}

//Kitao追加
void
MAINBOARD_SetShowOverscanRight(
	Sint32	showOverscanRight)
{
	_ShowOverscanRight = showOverscanRight;
}


//Kitao追加。読み込み中のステートセーブファイルのバージョンを返す。
Uint32
MAINBOARD_GetStateVersion()
{
	return _StateVersion;
}


//Kitao追加。Debug.cppから使用。v2.34
Uint8*
MAINBOARD_GetpMainRam()
{
	return _pMainRam;
}


//Kitao追加。v2.40
BOOL
MAINBOARD_GetSystemCard()
{
	return _bSystemCard;
}


//Kitao追加。v2.63
Uint32
MAINBOARD_GetSystemTime()
{
	return _SystemTime;
}

//Kitao追加。v2.63
void
MAINBOARD_ResetSystemTime()
{
	_SystemTime = 0;
}

//	##RA
Uint8*
MAINBOARD_GetpMainROM()
{
	return _pRom;
}

//	##RA
Uint32
MAINBOARD_GetpMainROMSize()
{
	return _RomSize;
}

//上側のオーバースキャンライン領域がScreenBuf[512*240],_TvW[240]以降に入っているので、現バージョンの仕様に合わせる。v2.00追加
static void
shiftScreenBuf(
	Sint32	showOverscanTop,
	Sint32	showOverscanBottom)
{
	int			i,j;

	//上側のオーバースキャンライン領域が[512*240]以降に入っているので、現バージョンの仕様に合わせる。
	for (i=0; i<8; i++)
		for (j=0; j<512; j++)
			_ScreenBufOT[512*i + j] = _ScreenBuf[512*(240+i) + j]; //上側のオーバースキャンライン領域を退避
	for (i=16+223+showOverscanBottom; i>=16; i--)
		for (j=0; j<512; j++)
			_ScreenBuf[512*i + j] = _ScreenBuf[512*(i-16) + j]; //通常表示領域と下側のオーバースキャンライン領域を転送
	for (i=16-showOverscanTop; i<16; i++)
		for (j=0; j<512; j++)
			_ScreenBuf[512*i + j] = _ScreenBufOT[512*(i-(16-showOverscanTop)) + j]; //上側のオーバースキャンライン領域を転送

	//上側のオーバースキャンライン領域が[240]以降に入っているので、現バージョンの仕様に合わせる。
	for (i=0; i<8; i++)
		_TvWOT[i] = _TvW[240+i]; //上側のオーバースキャンライン領域を退避
	for (i=16+223+showOverscanBottom; i>=16; i--)
		_TvW[i] = _TvW[i-16]; //通常表示領域と下側のオーバースキャンライン領域を転送
	for (i=16-showOverscanTop; i<16; i++)
		_TvW[i] = _TvWOT[i-(16-showOverscanTop)]; //上側のオーバースキャンライン領域を転送
}

// save variable
#define SAVE_V(V)	if (fwrite(&V, sizeof(V), 1, p) != 1)	return FALSE
#define LOAD_V(V)	if (fread(&V, sizeof(V), 1, p) != 1)	return FALSE
// save array
#define SAVE_P(P, N)	if (fwrite(P, N, 1, p) != 1)	return FALSE
#define LOAD_P(P, N)	if (fread(P, N, 1, p) != 1)		return FALSE
//Kitao追加。スクリーンバッファだけ読み込んでステートセーブ時の画面を表示。
//			 (元のスクリーンバッファは後で戻せるように退避しておく)
BOOL
MAINBOARD_LoadScreenBuf(
	Sint32	num,
	FILE*	p)
{
	Uint32		stateVersion;
	BOOL		bStretched; //Dummy読み込み用
	BOOL		bVStretched; //Dummy読み込み用
	BOOL		bTvMode; //Dummy読み込み用
	BOOL		screenBufOk; //Dummy読み込み用
	Sint32		VDCWidth;
	Sint32		showOverscanTop		= 0;
	Sint32		showOverscanBottom	= 0;
	Sint32		showOverscanLeft	= 0;
	Sint32		showOverscanRight	= 0;
	Sint32		prevFourSplitCpuSpeed;
	char		bufS[11+11+1];
	char		bufL[11+11+1];

	memcpy(&_ScreenBufR, &_ScreenBuf, sizeof(_ScreenBuf)); //元のスクリーンバッファを退避
	memcpy(&_TvWR, &_TvW, sizeof(_TvW)); //退避
	_VDCWidthR = VDC_GetScreenWidth(); //退避
	_bFourSplitScreenR = _bFourSplitScreen; //退避
	_bMonoColorR = _bMonoColor; //退避
	_ForceMonoColorR = _ForceMonoColor; //退避
	
	LOAD_V(stateVersion); //Kitao追加。ステートセーブにバージョンを付けておくことで、将来仕様を変更したときも、古いものは古いものとして読み込むことが出来るようになる。
	if (stateVersion < 20) //Kitao追加。v0.94より古ければ
	{	//古いバージョンのステートセーブではスクリーンショット読み込み不可能。画面をクリアしておく。
		memset(_ScreenBuf, 0, sizeof(_ScreenBuf));
		SCREEN_SetMessageText("Old Version State");
		screenUpdate(_ShowOverscanTop,_ShowOverscanBottom,_ShowOverscanLeft,_ShowOverscanRight); //スクリーン更新
		return FALSE;
	}
	else
	{
		LOAD_V(bStretched); //ここではDummy。将来はこれも反映させられれば
		LOAD_V(bVStretched); //ここではDummy。将来はこれも反映させられれば
		LOAD_V(bTvMode); //ここではDummy
		if (stateVersion >= 32) //v1.43以降のセーブファイルなら
		{
			LOAD_V(showOverscanTop);
			LOAD_V(showOverscanBottom);
			LOAD_V(showOverscanLeft);
			LOAD_V(showOverscanRight);
		}
		LOAD_P(_ScreenBuf, sizeof(_ScreenBuf)); //ロード＆ポーズ時にすぐ画面表示が出来るようにするために必要。スクリーンショットとしても使える。
		LOAD_V(screenBufOk);
		LOAD_P(_TvW, sizeof(_TvW));
		if (stateVersion < 40) //Kitao追加。v2.00より古ければ
			shiftScreenBuf(showOverscanTop,showOverscanBottom);//上側のオーバースキャンライン領域がScreenBuf[512*240],_TvW[240]以降に入っているので、現バージョンの仕様に合わせる。
		LOAD_V(_WidestTvW); //現在非使用のため、ダミー読み込み。
		LOAD_V(VDCWidth);
		VDC_SetScreenWidth(VDCWidth);
		if (stateVersion >= 42) //v2.27以降のセーブファイルなら
		{
			LOAD_V(_bFourSplitScreen);
			LOAD_V(prevFourSplitCpuSpeed); //ここではダミー読み込み
			LOAD_V(_bMonoColor);
		}
		else
		{
			_bFourSplitScreen = FALSE;
			_bMonoColor = 0;
		}
		if (stateVersion >= 43) //v2.28以降のセーブファイルなら
		{
			LOAD_V(_ForceMonoColor);
		}
		else
			_ForceMonoColor = 0;
		if (num == 0) //インプット録画時
			strcpy(bufS, "Former File");
		else
			APP_CheckStateTime(-abs(num), bufS, bufL);
		SCREEN_SetMessageText(bufS);
		screenUpdate(showOverscanTop,showOverscanBottom,showOverscanLeft,showOverscanRight); //スクリーン更新
		return TRUE;
	}
}

//Kitao追加。MAINBOARD_LoadScreenBuf()で退避した元のスクリーンバッファに戻す。
void
MAINBOARD_RestoreScreenBuf()
{
	memcpy(&_ScreenBuf, &_ScreenBufR, sizeof(_ScreenBufR)); //元のスクリーンバッファに復元
	memcpy(&_TvW, &_TvWR, sizeof(_TvWR));
	VDC_SetScreenWidth(_VDCWidthR);
	_bFourSplitScreen = _bFourSplitScreenR;
	_bMonoColor = _bMonoColorR;
	_ForceMonoColor = _ForceMonoColorR;
	screenUpdate(_ShowOverscanTop,_ShowOverscanBottom,_ShowOverscanLeft,_ShowOverscanRight); //スクリーン更新
}

/*-----------------------------------------------------------------------------
	[SaveState]
		状態をファイルに保存します。 
-----------------------------------------------------------------------------*/
BOOL
MAINBOARD_SaveState(
	FILE*		p)
{
	BOOL		ret;
	int			i;
	Uint32		offset;
	BOOL		bTvMode = APP_GetTvMode();
	Sint32		VDCWidth;
	BOOL		bUseVideoSpeedUpButton = APP_GetUseVideoSpeedUpButton();
	BOOL		bF1NoReset = APP_GetF1NoReset();
	Sint32		showOverscanTop;
	Sint32		showOverscanBottom;
	Sint32		showOverscanLeft;
	Sint32		showOverscanRight;

	if (p == NULL)
		return FALSE;

	AOUT_Play(FALSE); //Kitao追加。すぐ音を止める。AOUTスレッドがループに陥ってしまわないためにも必要。

	//Kitao追加。CDIFのスレッドで処理中(CDアクセス等)の場合、処理が終わるまで待つ。
	CDROM_ResetCDAccessCount(TRUE); //CDIFのスレッドがループに陥ってしまわないために必要。
	CDIF_WaitDeviceBusy();
	CDROM_ResetCDAccessCount(FALSE);

	_StateVersion = SAVE_STATE_VERSION; //Kitao追加
	SAVE_V(_StateVersion); //Kitao追加。ステートセーブにバージョンを付けておくことで、将来仕様を変更したときも、古いものは古いものとして読み込むことが出来るようになる。

	//Kitao追加
	SAVE_V(_bStretched);
	SAVE_V(_bVStretched);
	SAVE_V(bTvMode);

	//メニューで設定されているオーバースキャン設定で保存する。※自動でオーバースキャン表示された状態（640x480のフルスクリーン＆フルストレッチ＆横スキャンライン時等）は保存しない。v2.64
	showOverscanTop    = APP_GetShowOverscanTop();
	showOverscanBottom = APP_GetShowOverscanBottom();
	showOverscanLeft   = APP_GetShowOverscanLeft();
	showOverscanRight  = APP_GetShowOverscanRight();
	SAVE_V(showOverscanTop); //v1.43追加
	SAVE_V(showOverscanBottom);
	SAVE_V(showOverscanLeft);
	SAVE_V(showOverscanRight);

	SAVE_P(_ScreenBuf, sizeof(_ScreenBuf)); //v0.51追加。ロード＆ポーズ時にすぐ画面表示が出来るようにするために必要。スクリーンショットとしても使える。
	SAVE_V(_ScreenBufOk);
	SAVE_P(_TvW, sizeof(_TvW));
	SAVE_V(_WidestTvW);
	VDCWidth = VDC_GetScreenWidth();
	SAVE_V(VDCWidth); //v0.94追加
	SAVE_V(_bFourSplitScreen); //v2.27追加
	SAVE_V(_PrevFourSplitCpuSpeed); //v2.27追加
	SAVE_V(_bMonoColor); //v2.27追加
	SAVE_V(_ForceMonoColor); //v2.28追加

	SAVE_V(_SystemTime); //v1.15追加。ゲーム起動からの経過時間(単位はフレーム)

	SAVE_V(_SuperGrafx); //v0.89追加
	SAVE_P(_pMainRam, 0x8000); //v0.89更新。スーパーグラフィックス対応。32KB
	SAVE_P(_pBackupRam, 0x2000);
	SAVE_P(_pBufferRam, 0x40000);

	// ポインタ→オフセット変換 
	for (i = 0; i < 256; i++)
	{
		offset = _pRomMap[i] - _pRom;
		SAVE_V(offset);
	}

	SAVE_V(_bSystemInit);
	SAVE_V(_Buffer);
	SAVE_V(_AcShift);
	SAVE_V(_AcShiftBits);
	SAVE_V(_0x1AE5);

	//v1.02更新。CDROMのステートを読み込んだときにVDCの設定を行う場合があるのでVDCのステートを先に読み込めるようにした。
	//           ※v1.03からCDROMのステートを読み込んだときにVDCの設定を行わないようにしたのでVDCとCDROMの順序は自由に。
	ret =  CPU_SaveState(p);
	ret &= TIMER_SaveState(p);
	ret &= INTCTRL_SaveState(p);
	ret &= JOYPAD_SaveState(p);
	ret &= VDC_SaveState(p);
	ret &= APU_SaveState(p);
	ret &= CDROM_SaveState(p);

	//Kitao更新。アーケードカードを使用しているゲームの場合だけ、アーケードカード関連についてもセーブするようにした。それ以外の場合はファイル容量節約。
	SAVE_V(_bArcadeCard); //v1.11追加
	SAVE_V(_bAcUse);
	if (_bAcUse)
	{
		SAVE_P(&_AcRam[0], 0x200000);
		SAVE_P(&_Ac[0], sizeof(_Ac));
	}

	//v0.95追加
	SAVE_V(_FastForwardingR);
	SAVE_V(_bSoundAjust);
	SAVE_V(bUseVideoSpeedUpButton);

	//v1.00追加
	SAVE_V(bF1NoReset);

	//v1.15追加
	SAVE_V(_SystemTimeCheck);
	SAVE_V(_bKonamiStereo); //これは再設定するので現在非使用になった。v1.12

	return ret;
}

/*-----------------------------------------------------------------------------
	[LoadState]
		状態をファイルから読み込みます。 
-----------------------------------------------------------------------------*/
BOOL
MAINBOARD_LoadState(
	FILE*		p)
{
	int			i;
	BOOL		ret;
	Uint32		offset;
	BOOL		bStretched; //Dummy読み込み用
	BOOL		bTvMode;
	Sint32		VDCWidth; //Dummy読み込み用
	BOOL		bUseVideoSpeedUpButton;
	BOOL		bF1NoReset;
	BOOL		bKonamiStereo;
	BOOL		bGradiusII;
	Sint32		fastForwardingR;
	BOOL		bSoundAjust;
	Sint32		showOverscanTop		= 0;
	Sint32		showOverscanBottom	= 0;
	Sint32		showOverscanLeft	= 0;
	Sint32		showOverscanRight	= 0;

	if (p == NULL)
		return FALSE;

	AOUT_Play(FALSE); //Kitao追加。すぐ音を止める。AOUTスレッドがループに陥ってしまわないために必要。
	CDROM_Stop(FALSE); //Kitao追加。ロード前にCDDAを再生していた場合、STOP処理をする。

	//Kitao追加。CDIFのスレッドで処理中(CDアクセス等)の場合、処理が終わるまで待つ。
	CDROM_ResetCDAccessCount(TRUE); //CDIFのスレッドがループに陥ってしまわないために必要。
	CDIF_WaitDeviceBusy();
	CDROM_ResetCDAccessCount(FALSE);
	CDROM_Stop(FALSE); //Kitao追加。タイミングによって止まっていない可能性もあるのでSTOP処理。v2.05

	LOAD_V(_StateVersion); //Kitao追加。ステートセーブにバージョンを付けておくことで、将来仕様を変更したときも、古いものは古いものとして読み込むことが出来るようになる。
	if (_StateVersion > SAVE_STATE_VERSION) //もし旧バージョンのOotakeで未対応の新ver.ステートセーブデータを読み込んだ場合
	{
		PRINTF("Couldn't Load \"Over Version\" state data.");
		return FALSE;
	}

	if (_StateVersion >= 20) //Kitao追加。v0.94以降なら
	{
		LOAD_V(bStretched); //Kitao追加。v0.71からストレッチ設定は縦のみ反映させるようにした。※次回起動時にこの設定が残ってしまうのを避けるため。
		LOAD_V(_bVStretched); //Kitao追加
		if ((APP_GetFullStretched(FALSE))||(!APP_GetStretched()))
			_bVStretched = FALSE; //v1.24追加
		LOAD_V(bTvMode);
		//if (bTvMode != APP_GetTvMode()) //現在のモードと異なる場合。※v1.53からTvModeは反映しないようにした
		//	APP_SetTvMode(bTvMode);
		APP_SetStretched(_bStretched, _bVStretched, APP_GetFullStretched(FALSE)); //Kitao追加
		if (_StateVersion >= 32) //Kitao追加。v1.43以降のセーブファイルなら
		{
			LOAD_V(showOverscanTop);
			LOAD_V(showOverscanBottom);
			LOAD_V(showOverscanLeft);
			LOAD_V(showOverscanRight);
		}
		LOAD_P(_ScreenBuf, sizeof(_ScreenBuf)); //ロード＆ポーズ時にすぐ画面表示が出来るようにするために必要。スクリーンショットとしても使える。
		LOAD_V(_ScreenBufOk);
		LOAD_P(_TvW, sizeof(_TvW));
		if (_StateVersion < 40) //Kitao追加。v2.00より古ければ
			shiftScreenBuf(showOverscanTop,showOverscanBottom);//上側のオーバースキャンライン領域がScreenBuf[512*240],_TvW[240]以降に入っているので、現バージョンの仕様に合わせる。
		LOAD_V(_WidestTvW);
		LOAD_V(VDCWidth); //0.94追加。スクリーンバッファ復元時に使用
		if (_StateVersion >= 42) //v2.27以降のセーブファイルなら
		{
			LOAD_V(_bFourSplitScreen);
			LOAD_V(_PrevFourSplitCpuSpeed);
			LOAD_V(_bMonoColor);
		}
		else
		{
			_bFourSplitScreen = FALSE;
			_PrevFourSplitCpuSpeed = 0;
			_bMonoColor = 0;
		}
		if (_StateVersion >= 43) //v2.28以降のセーブファイルなら
		{
			LOAD_V(_ForceMonoColor); //強制モノカラー状態でセーブしてあった場合、強制モノカラー状態に切り替える。※その後F1リセットや、ゲームを変えた場合には元に戻す。
		}
		else
			_ForceMonoColor = 0;
	}
	if (_StateVersion >= 27) //Kitao追加。v1.15以降のセーブファイルなら
	{
		{//Test用
//			long so = ftell(p);
//			char s[100];
//			sprintf(s,"v%d SystemTime Offset = %X", _StateVersion, so);
//			MessageBox(WINMAIN_GetHwnd(), s, "Test", MB_OK);
		}
		LOAD_V(_SystemTime); //ゲーム起動からの経過時間(単位はフレーム)
	}
	else
		_SystemTime = 0xFFFFFFFF;
	if (_SystemTime == 0xFFFFFFFF)
		if (APP_GetLoadingResumeFile()) //「古いバージョンでスタートしたレジュームファイル」をロードした場合、経過時間を0にリセット。v2.63
			_SystemTime = 0;

	if (_StateVersion >= 17) //Kitao追加。v0.89以降なら
	{
		LOAD_V(_SuperGrafx);
		LOAD_P(_pMainRam, 0x8000);
	}
	else
		LOAD_P(_pMainRam, 0x2000);
	_bBackupFull = FALSE;
	LOAD_P(_pBackupRam, 0x2000);
	if ((_pBackupRam[4]==0x00)&&(_pBackupRam[5]==0xA0)&&(_pBackupRam[6]==0x10)&&(_pBackupRam[7]==0x80))
	{	//v1.37追加。旧バージョンのバックアップラム初期値だった場合、正しい値に修正する。ネオネクタリスとマクロスラヴで必要。
		for (i=0x0008; i<0x2000; i++)
			if (_pBackupRam[i] != 0x00) break;
		if (i == 0x2000) //0x0008以降が全て0x00だった場合、初期値。
			_pBackupRam[5] = 0x88; //正しい初期値に変更
	}
	//Kitao追加。バックアップラムの最初の状態を退避しておく。終了時に更新された形跡があったときだけ、バックアップラムを保存するようにした。
	//           こうすることで、間違えてステートロードしてしまった時でも、すぐ終了すれば元のバックアップラムファイルは残ってくれる。
	memcpy(&_FirstBackupRam, _pBackupRam, 0x2000);
	LOAD_P(_pBufferRam, 0x40000);

	// オフセット→ポインタ変換 
	for (i=0; i<256; i++)
	{
		LOAD_V(offset);
		_pRomMap[i] = _pRom + offset;
	}

	LOAD_V(_bSystemInit);
	LOAD_V(_Buffer);
	LOAD_V(_AcShift);
	LOAD_V(_AcShiftBits);
	LOAD_V(_0x1AE5);

	if (_StateVersion < 20) //Kitao追加。v0.94より古ければ
	{
		LOAD_V(bStretched); //Kitao追加。v0.71からストレッチ設定は縦のみ反映させるようにした。※次回起動時にこの設定が残ってしまうのを避けるため。
		LOAD_V(_bVStretched); //Kitao追加
		if ((APP_GetFullStretched(FALSE))||(!APP_GetStretched()))
			_bVStretched = FALSE; //v1.24追加
		if (_StateVersion >= 18) //Kitao追加。v0.91以降なら
		{
			LOAD_V(bTvMode);
			//if (bTvMode != APP_GetTvMode()) //現在のモードと異なる場合。※v1.53からTvModeは反映しないようにした
			//	APP_SetTvMode(bTvMode);
		}
		APP_SetStretched(_bStretched, _bVStretched, APP_GetFullStretched(FALSE)); //Kitao追加
		if (_StateVersion >= 2) //Kitao追加。v0.51以降なら
		{
			LOAD_P(_ScreenBuf, sizeof(_ScreenBuf)); //ロード＆ポーズ時にすぐ画面表示が出来るようにするために必要。スクリーンショットとしても使える。
			LOAD_V(_ScreenBufOk);
		}
		else
			memset(_ScreenBuf, 0, sizeof(_ScreenBuf)); //古いバージョンのステートセーブではスクリーンショット読み込み不可能。画面をクリアしておく。
		if (_StateVersion >= 5) //Kitao追加。v0.60以降なら
		{
			LOAD_P(_TvW, sizeof(_TvW));
			LOAD_V(_WidestTvW);
		}
		else
		{
			for (i=0; i<256; i++)
				_TvW[i] = 256;
			_WidestTvW = 256;
		}
		shiftScreenBuf(showOverscanTop,showOverscanBottom);//上側のオーバースキャンライン領域がScreenBuf[512*240],_TvW[240]以降に入っているので、現バージョンの仕様に合わせる。
	}

	ret =  CPU_LoadState(p);
	ret &= TIMER_LoadState(p);
	ret &= INTCTRL_LoadState(p);
	ret &= JOYPAD_LoadState(p);
	if (_StateVersion >= 23) //Kitao追加。v1.02以降なら
	{	//CDROMのステートを読み込んだときにVDCの設定を行う場合があるのでVDCのステートを先に読み込むようにした。
		ret &= VDC_LoadState(p);
		ret &= APU_LoadState(p);
		ret &= CDROM_LoadState(p);
	}
	else
	{
		ret &= APU_LoadState(p);
		ret &= CDROM_LoadState(p);
		ret &= VDC_LoadState(p);
	}

	//スーパーグラフィックスモード切替
	MAINBOARD_SetSuperGrafx(_SuperGrafx); //アドレスマスク＆VDCの変数＆Windowのキャプションも更新される。

	//Kitao更新。アーケードカードを使用しているゲームの場合だけ、アーケードカード関連についてもロードするようにした。
	if (_StateVersion >= 26) //Kitao追加。v1.11以降のセーブファイルなら
	{
		LOAD_V(_bArcadeCard); //v1.11追加
	}
	else
		_bArcadeCard = TRUE;
	LOAD_V(_bAcUse);
	if (_bAcUse)
	{
		LOAD_P(&_AcRam[0], 0x200000);
		LOAD_P(&_Ac[0], sizeof(_Ac));
	}

	if (_StateVersion >= 21) //Kitao追加。v0.95以降のセーブファイルなら
	{
		LOAD_V(fastForwardingR);
		LOAD_V(bSoundAjust);
		LOAD_V(bUseVideoSpeedUpButton);
		if (APP_GetLoadStateSpeedSetting()) //速度変更設定を反映する設定の場合。反映しない設定(デフォルト)の場合は、現状の早送り設定を引き継ぐ。誤ってHome,Endキーを押して知らずに速度変更してしまうケースがあるのでv2.36からデフォルトでは反映しないようにした。
		{
			_FastForwardingR = fastForwardingR;
			_bSoundAjust = bSoundAjust;
			APP_SetUseVideoSpeedUpButton(bUseVideoSpeedUpButton);
		}
	}
	MAINBOARD_SetFastForwarding(_FastForwardingR, _bSoundAjust, TRUE); //Kitao追加。早回し設定を更新。

	if (_StateVersion >= 22) //Kitao追加。v1.00以降のセーブファイルなら
	{
		LOAD_V(bF1NoReset);
		APP_SetF1NoReset(bF1NoReset);
	}

	if (_StateVersion >= 27) //Kitao追加。v1.15以降のセーブファイルなら
	{
		LOAD_V(_SystemTimeCheck);
		if (_SystemTime == 0) //「古いバージョンでスタートしたレジュームファイル」をロードした場合。v2.63
			_SystemTimeCheck = 0;
		LOAD_V(bKonamiStereo); //これは再設定するので現在非使用。ダミー読み込み
	}
	else
		_SystemTimeCheck = 0;

	bKonamiStereo = FALSE;
	if ((_bKonamiStereo)&&(APP_GetAutoStereo()))
		if (_SystemTime < _SystemTimeCheck) //チェックポイント未到達の場合
			bKonamiStereo = TRUE;
	INPUT_SetKonamiStereo(bKonamiStereo);
	bGradiusII = FALSE;
	if ((_bGradiusII)&&(APP_GetAutoGradiusII()))
		if (_SystemTime < _SystemTimeCheck) //チェックポイント未到達の場合
			bGradiusII = TRUE;
	INPUT_SetGradiusII(bGradiusII);

	//Kitao追加。ゲームごとの自動設定があればそれを優先する
	if (_bAutoDisconnectMultiTap)
		JOYPAD_ConnectMultiTap(FALSE);

	//オーバースキャン領域表示関連。v1.43追加
	if ((((APP_GetFullMagnification() == 0)&&((_ScreenWidth != 640)||(_ScreenHeight != 480)))||(APP_GetFullMagnification() == 2))&&
		(_bFullScreen)&&(APP_GetFullStretched(FALSE))&&((APP_GetScanLineType() == 2)||(APP_GetTvMode())))
	{	//640x480のフルスクリーン＆フルストレッチ＆スキャンライン表示の場合、画質維持のためフルストレッチ(低倍率拡大)せず、オーバースキャンを表示することで画面を一杯にする。v2.64追加
		showOverscanTop		= 8;
		showOverscanBottom	= 8;
		showOverscanLeft	= 8;
		showOverscanRight	= 8;
	}
	else //通常
	{
		//メニュー表示側にも反映させる。v2.65更新
		APP_SetShowOverscanTop(showOverscanTop);
		APP_SetShowOverscanBottom(showOverscanBottom);
		APP_SetShowOverscanLeft(showOverscanLeft);
		APP_SetShowOverscanRight(showOverscanRight);
		if (showOverscanTop > 0)
			APP_SetShowOverscanHeight(showOverscanTop);
		else if (showOverscanBottom > 0)
			APP_SetShowOverscanHeight(showOverscanBottom);
	}
	if ((showOverscanTop != _ShowOverscanTop)||(showOverscanBottom != _ShowOverscanBottom)||
		(showOverscanLeft != _ShowOverscanLeft)||(showOverscanRight != _ShowOverscanRight)) //ロードする前と表示領域が異なっていた場合。変わっていない場合は速度アップのため何もしない。
			if (!_bFullScreen)
			{
				_ShowOverscanTop = showOverscanTop;
				_ShowOverscanBottom = showOverscanBottom;
				_ShowOverscanLeft = showOverscanLeft;
				_ShowOverscanRight = showOverscanRight;
				APP_UpdateScreenMode(TRUE); //ウィンドウの大きさを変える必要があるため画面モードを再設定する。
				SCREEN_Clear();
			}
	//Kitao追加v0.51。セーブ時のスクリーンショットをすぐに表示
	VDC_SetTvStartPos();//水平表示位置を設定し直す。v1.61
	screenUpdate(showOverscanTop,showOverscanBottom,showOverscanLeft,showOverscanRight);
	_ShowOverscanTop = showOverscanTop; //screenUpdate()で無駄な画面クリアを行わないようにするために、ここで_ShowOverscan変数を更新する。
	_ShowOverscanBottom = showOverscanBottom;
	_ShowOverscanLeft = showOverscanLeft;
	_ShowOverscanRight = showOverscanRight;

	//Kitao追加。CD-DAを鳴らす
	CDROM_LoadPlayAudioTrack();

	return TRUE;
}

#undef SAVE_V
#undef LOAD_V
#undef SAVE_P
#undef LOAD_P
