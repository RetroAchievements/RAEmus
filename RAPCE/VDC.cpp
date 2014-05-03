/******************************************************************************
Ootake
・スーパーグラフィックスに対応した。v0.89
・Vblank前後の時間が実機より少ないようなので、１フレームを263ラインと想定し、
  Vblank前後にCPUパワーを多く使えるようにした。(実機に近い処理速度になったはず)
・ラスタ割り込みのタイミングを実機に近づけた。また、GUIのCPUメニューから手動で
  の調整もできるようにした。
・RCRの値が更新されたときにまだ割り込み要求を行っていなかった場合、割り込みライ
  ン上かどうかをチェックしてすぐにラスタ割り込み要求を行うようにした。（１ライ
  ン毎に細かくラスタ割り込みするソフトでも、画面が崩れることなく確実にラスタ割
  り込みを発生させるため）
・できるだけ並列に近い動作をさせるために、ここでCPU,CD-ROM,APU,タイマー,マウス
  のクロックを進めるようにした。
・TVへの垂直表示開始位置がユニークなソフトにも対応した。
・TVへの水平表示開始位置がユニークなソフトにも対応した。v0.57。
・Vblank割り込みをするラインを"DISPLAY_STARTLINE + _ScreenH + 2"(たいていのソ
  フトは262)とした。これが263ラインを越えている場合は、263ライン目でVblankを起
  こすようにした。v1.40
・VCRが4より小さい場合、そのぶん早めにVblank割り込みを起こすようにした。v0.81
・VRAM-SATB間のDMA転送時のCPUクロック消費を256サイクル(DMA転送なのでほとんど消
  費しないと推定)とした。v1.14。
・VBlank割り込みを安定して伝えるため、VRAM-SATB間のDMA転送時のCPUクロック消費は
  VBlank開始から約１ライン後からおこなうようにした。v0.82。
・VRAM-SATB間のDMA転送時にVRAMライトが起こった場合は、CPUをストールさせるように
  した。v0.87。情報元：Kiさん
・VRAM-VRAM間のDMA転送開始処理にかかる時間をCPU換算で32クロック(全くの推定。こ
  の値だとネクタリスがスムーズに動く)とし、正確な時間(実機と同じかは未確認)経過
  後に完了の割り込みを発生させるようにした。v0.82。
・VRAMライト時にウェイトを入れるようにした。v0.82。
・MWRレジスタ(メモリ幅)を変更したときに、メモリマップをすぐには更新せず、Vblank
  期間で更新するようにした。（リンダキューブのプロローグ、SUPER桃鉄の優勝者発表
  時に画面が崩れる問題、その他画面切り替え時に一瞬画面が崩れる問題を解消）
・スプライト欠けを再現（スプライトオーバーがあった場合オーバーぶんのスプライト
  は表示しない）する設定を付けた。（パワーリーグ３の打席結果表示を実機同様にす
  るために必要）
・VRAMリード時に、アドレスレジスタ(AR)がVDC_VRRではないときもVRAMの内容を返す
  ようにした。(21エモンでの文字化けを解消)。v0.72。
・SAT(スプライト情報)のパターンアドレスの指定はbit0～10まで有効とした。(ぽっぷ
  るメイルのオープニングや、遊々人生のエンド画面等でスプライトのごみが出てしま
  っていたのを解消)。v0.70
・CPUにオーバークロック機能を付けた。
・VDCステータスは、Readされると必ずクリアされるようにした。
・BYRレジスタへの書き込みが行われた際に、まだそのラインの描画を終えていなかっ
  た場合はLineCounterをいち早くインクリメントするようにした。ダンジョンエクス
  プローラー、パックランド、エターナルシティ等で必要。v0.62更新
・どうしても速度タイミングが合わず問題が起こるゲームには、パッチとしてSATB転送
  時にウェイトを入れて動作させるようにした。（現バージョンでは使用の必要なし）
・ラスタ割り込みの要求が伝わっていなかった場合、オーバークロックしてでもそのラ
  イン描画中に伝えるようにした。（シュビビンマン２で稀に１フレーム乱れた画面が
  出る問題を回避）v0.57。←実機で確認したところ、実機でも１フレーム乱れるのが
  デフォルトだった(^^; ←天使の詩で描画が乱れたのでシュビビンマン２以外ではこ
  の処理をカットした。v0.61
・VBlankを起こす際にVCRの値が大きい場合、VBlankを起こすラインを遅らせるように
  した。v1.00。"雀神伝説"のステータス表示
・VRAM-VRAM間のDMA転送の際、DCRのbit4が立っていたら、描画期間中には転送せず、
  VBlank期間を待って転送するようにした。ラングリッサーで必要。v1.03
・スプライト処理部分とBG処理部分を、ソースの意味がわかりづらくならない範囲で高
  速化した。v1.08
・高速化のためVCE.c をここへ統合した。v1.11。
・スプライト表示処理で、((MWR & 0xC) == 0x4)のときはパターンインデックスが2bit
  パターンモードになることを実装した。Kiさんより資料情報をいただきました。"フ
  ァイティングラン"のスプライト表示が正常になった。v1.13。
・VCEのRead時に無効なポートが指定されていた場合、0でなく0xFFを返すようにした。
  "ミズバク大冒険"４面の岩起動の不具合が解消。"スタートリングオデッセイ２"も動
  作するようになった。v1.62
・VDCポートのアクセスウェイトは、バーストモード中はノンウェイトとした。多くの
  ゲームの速度,タイミングが実機に近づいた。(実機でこの実装が正しいかは未実験
  )。VCEポートのアクセスウェイトも同様かもしれないが、現状は未確定のため、エ
  ミュレート速度を優先して、バーストモード中でもウェイトを入れています。v2.07
・水平表示開始位置が特殊なケースに対応した。ファイナルブラスターのスタートデモ
  が実機同様になった。v2.14

Copyright(C)2006-2011 Kitao Nakamura.
	改造版・後継版を公開なさるときは必ずソースコードを添付してください。
	その際に事後でかまいませんので、ひとことお知らせいただけると幸いです。
	商的な利用は禁じます。
	あとは「GNU General Public License(一般公衆利用許諾契約書)」に準じます。

*******************************************************************************
	[VDC.c]
		ＶＤＣの実装を行ないます。

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
#include <string.h>
#include <malloc.h>
#include "VDC.h"
#include "MainBoard.h"
#include "App.h"
#include "Printf.h"

extern Sint32 gCPU_ClockCount;	//v2.00追加。高速化のためカウンタをグローバル化。
extern Sint32 gCPU_Transfer;	//v2.31追加。CPUが転送命令を実行中かどうか

/*
	Kitao更新。v0.81。１フレームをラスタ割り込み可能最大範囲の263ラインとした。ソフトの動作＆タイミングが実機に近くなった。

	１ライン分のＣＰＵサイクル数は、
		7159090 / 60 / 263 ~= 453.68124207858048162230671736375
		(119318.16666666666666666666666667 / 263)
*/
//define VDC_CYCLESPERLINE とMAX_SCANLINE はVDC.h に定義。

#define NUM_SPRITES			64

#define MAWR	_Regs[_VN][VDC_MAWR]
#define MARR	_Regs[_VN][VDC_MARR]
#define VWR		_Regs[_VN][VDC_VWR]
#define VRR		_Regs[_VN][VDC_VRR]
#define CR		_Regs[_VN][VDC_CR]
#define RCR		_Regs[_VN][VDC_RCR]
#define BXR		_Regs[_VN][VDC_BXR]
#define BYR		_Regs[_VN][VDC_BYR]
#define MWR		_Regs[_VN][VDC_MWR]
#define HSR		_Regs[_VN][VDC_HSR]
#define HDR		_Regs[_VN][VDC_HDR]
#define VPR		_Regs[_VN][VDC_VPR]
#define VDW		_Regs[_VN][VDC_VDW]
#define VCR		_Regs[_VN][VDC_VCR]
#define DCR		_Regs[_VN][VDC_DCR]
#define SOUR	_Regs[_VN][VDC_SOUR]
#define DESR	_Regs[_VN][VDC_DESR]
#define LENR	_Regs[_VN][VDC_LENR]
#define SATB	_Regs[_VN][VDC_SATB]

#define HSW		(HSR & 0xFF) //Kitao更新
#define VSW		(VPR & 0xFF) //Kitao更新
#define HDS		((HSR & 0x7F00) >> 8)
#define HDE		((HDR & 0x7F00) >> 8)
#define HDW		(HDR & 0x7F) //Kitao追加
#define VDS		(VPR >> 8)

#define DISPLAY_STARTLINE	((VPR & 0x1F)+1+VDS+2) //Kitao更新
#define DISPLAY_ENDLINE		(DISPLAY_STARTLINE+VDW+1)

//Kitao追加。v2.06
static BOOL		_bSpriteLayer;
static BOOL		_bSprite2Layer;
static BOOL		_bBGLayer;
static BOOL		_bBG2Layer;

//Kitao更新。テーブルを最初に宣言＆スーパーグラフィックス用にVDC２つぶんの変数を使用。
static const Uint8		_IncSizeTable[4] = {1, 32, 64, 128};
static const Uint32		_HeightTable[4] = {16, 32, 64, 64};
static const Uint8		_BGWidthTable[4] = {32, 64, 128, 128};
static Uint32 			_PatternLUT[256][2];

static Sint32		_SuperGrafx; //Kitao追加。1ならスーパーグラフィックスモード。通常は0。
static Sint32		_VN; //Kitao追加。アクセス中のVDCナンバー(0or1)。スパグラ用。通常のPCエンジンの場合常に0。
static Sint32		_VPC[8]; //Kitao追加。VPCの値保管用

static Uint16		_Regs[2][32];
static Uint32		_AR[2];

static Uint8		_VideoRam[2][65536]; //VRAM 64KB
static Uint16*		_pwVideoRam[2];
static Uint16		_ReadData[2]; //Kitao追加。実際のVRAMリードアクセスはVDCポートの"MARRライト時"におこなわれる(推定)。

static Sint32		_ScreenW[2];
static Sint32		_ScreenH[2];
static Uint32		_BGH[2];
static Uint32		_BGW[2];

static Uint16		_VdcAddrInc[2] = {1,1};
static Uint8		_VdcStatus[2] = {0,0};

static Uint32*		_pScreenBuf; //Kitao追加v1.11。ラインバッファを使わずにScreenBufに直接書き込むようにした。高速化。
static Uint32		_LineBufVDC2[512]; //Kitao追加。スーパーグラフィックス処理で使用。
static Uint8		_BgPutBuf[2][512]; //Kitao追加v1.11。0以外ならBG表示済み(透明以外)のドット。
static Uint8		_SpPutBuf[2][512]; //Kitao追加v1.11。1ならスプライト表示済み(透明以外)のドット。2ならBGに隠れたスプライトドットが存在(スパグラ用)。最上位ビットが1ならスプライトナンバー0のドットが表示済み(衝突判定で使用)。

static Sint32		_RasterCounter[2];
static BOOL			_bRasterRequested[2]; //Kitao追加。そのラインのラスタ割り込み要求を終えていたらTRUE
static Sint32		_LineCounter[2];
static Sint32		_LineCountMask[2];
static Sint32		_LineOffset[2]; //Kitao追加。描画中に「描画開始ライン」が変更された場合、このオフセットを利用して実機同様の描画を再現する。v1.30。※現在は１フレームごとにリセットされるのでステートセーブの必要は無いが、互換のためにセーブしている。
static Uint16		_PrevVSW[2]; //v2.09追加。スキャンライン先頭時のVSW。※１フレームごとに更新されるのでステートセーブの必要は無し

static BOOL			_bUpdateSATB[2];
static Uint32		_SpDmaCount[2];
static Sint32		_SpDmaStole[2];
static Uint32		_VramDmaCount[2]; //Kitao追加。v0.82
static BOOL			_bVramDmaExecute[2]; //Kitao追加。v1.02

static Uint32		_MaxSpPerLine[2]; //Kitao追加。v2.63
static BOOL			_bSpOver[2];
static BOOL			_bOverlap[2]; //Kitao追加
static BOOL			_bBurstMode[2] = {FALSE, FALSE};
static BOOL			_bVDCAccessWait[2] = {TRUE, TRUE}; //Kitao追加。v2.08
static BOOL			_bMWRchange[2]; //Kitao追加

static Uint8		_BgTileCache[2][2048][8*8];
static Uint8		_SpTileCache[2][512][16*16];

static Uint16		_SpRam[2][64*4];

static Sint32		_TvStartLine[2]; //Kitao追加
static Sint32		_TvStartLineAdjust[2]; //Kitao追加。ゲーム毎に手動でアジャスト用。現在未使用で将来用。v2.47
static Sint32		_VblankLine[2]; //Kitao追加
static Sint32		_VblankLineTop[2]; //Kitao追加。描画開始ライン時のVblankLineの値。v1.19追加。※１フレームごとに更新されるのでステートセーブの必要は無し
static BOOL			_bVDCleared[2]; //Kitao追加。VBlank割り込み要求をCPU側がスルーしたまま次フレームの描画開始に来た場合TRUEにし、同時にVBlank要求ステータスをクリアし、そのフレーム内で遅延VBlankを発生させる。v1.63
static Uint16		_prevCR[2]; //Kitao追加。１ライン前のCRの値。v1.21。※１フレームごとに更新されるのでステートセーブの必要は無し

static Sint32		_ScanLine;
static Sint32		_DisplayCounter;
static BOOL			_bRasterLate; //Kitao追加
static Sint32		_bForceRaster; //Kitao追加。ラスタ割り込みがCPUで処理されるまで待つ(CPUをオーバークロックする)ならTRUE。シュビビンマン２で使用。
static Sint32		_bForceVBlank; //Kitao追加。VBlank割り込みがCPUで処理されるまで待つ(CPUをオーバークロックする)ならTRUE。レディソードで使用。
static Sint32		_RasterTimingType = 2;//Kitao追加。ラスター割り込みをするタイミングを調整するための変数。1=早めにCPUにラスタタイミングを伝えるソフト。2=通常。3=遅めにCPUにラスタタイミングを伝えるソフト。
static Sint32		_RasterTimingCount; //Kitao追加
static Sint32		_AutoRasterTimingType; //Kitao追加。自動でラスタータイミングを設定しているゲームの場合その_RasterTimingTypeが入る。自動でない場合はノーマル(2)が入る。
static Sint32		_OverClockType = 0; //Kitao追加
static Sint32		_OverClockCycle = 0; //Kitao追加
static Sint32		_AutoOverClock = -1; //Kitao追加
static BOOL			_bOverClockNow = FALSE; //v1.61追加
static Sint32		_WaitPatch; //Kitao追加
static Sint32		_bPerformSpriteLimit; //Kitao追加。TRUEならスプライト欠けを再現（スプライトオーバーがあった場合オーバーぶんのスプライトは表示しない）。パワーリーグ３の打席結果表示を実機同様にするために必要。
static Sint32		_bAutoPerformSpriteLimit; //Kitao追加。自動でスプライト欠けを設定しているゲームの場合TRUE。
static Uint32		_CpuPastCount;//Kitao追加。v1.00。１ライン中のどこまでCPU処理を進めたか。(１ラインで初期化されるのでステートセーブの必要はなし)
static BOOL			_bIRQ1CancelExecute;//Kitao追加。v1.03。現在非使用。ステートロードの互換のために残してある。

static Sint32		_DrawFrame; //Kitao追加。※この変数は毎回初期化されるのでステートセーブの必要はない。

static Uint32		_ClockCounter; //現在非使用

static BOOL			_bShinMegamiTensei; //v2.20追加
static BOOL			_bWorldStadium91; //v2.64追加


/*
	Kitao更新。v1.11。高速化のためVCE.c の処理をここへ統合した。
	[DEV NOTE]
	Kitao更新。32ビットカラーにも対応するため、VCE.cではRGBの変換処理をせずに、RGB333のままデータを送り出すようにした。
	現在白黒モードは未実装（DDSCREENのほうに実装する）。
*/
static Uint8		_DCC;
static Uint16		_ColorTableAddr;
static Sint32		_TvStartPos; //Kitao追加。テレビ上の水平表示開始位置
static Sint32		_TvWidth; //Kitao追加。テレビ上の水平表示幅(解像度)
static Sint32		_TvMax; //Kitao追加。テレビ上の水平表示最終位置。v1.11追加

static Uint16		_Palette333[512]; //Kitao追記。0～255がBG用。256～511がスプライト用
static Uint32		_PaletteBG[256]; //Kitao更新。描画時に扱いやすいようにUint32で「0RGB」の32ビットで保持。
static Uint32		_PaletteSP[256]; //Kitao追加。コードの高速化のため、スプライト用とBG用で分けた。
static Uint16		_ColorLatch;

static Sint32		_VpcPriority[4]; //Kitao追加。スーパーグラフィックス用。描画の優先順位。４領域ぶん(おそらく[2][3]はどちらか１つしか使われず３領域となる)
static Sint32		_VpcBorder1; //Kitao追加。スーパーグラフィックス用。領域を横３つに分けるための境界縦線１
static Sint32		_VpcBorder2; //Kitao追加。スーパーグラフィックス用。領域を横３つに分けるための境界縦線２


/*-----------------------------------------------------------------------------
	[update_bg_tile_cache]
		ＢＧパターンをエンコードし保持します。
-----------------------------------------------------------------------------*/
//Kitao更新。fill_tile_cache()と統合して高速化した。v1.08
//			 タイルキャッシュが更新されているかどうかの判断はdrawBgLineのほうで行うようにして高速化。v1.65
static inline void
update_bg_tile_cache(
	Uint32	cg_addr) //Kitao更新。fill_tile_cache()と統合して高速化した。v1.08
{
	Uint32		D0, D1, D2, D3;
	Uint8*		pVRAM1;		// for D0 and D1
	Uint8*		pVRAM2;		// for D2 and D3
	Uint32*		pPixel;		// pixel 0-3 & 4-7

	//Kitao更新。使う変数を少なくしシンプル化して高速化。v2.00
	pPixel = (Uint32*)&_BgTileCache[_VN][cg_addr][0];
	pVRAM1 = &_VideoRam[_VN][(cg_addr << 5) +  0];
	pVRAM2 = &_VideoRam[_VN][(cg_addr << 5) + 16];
	//横8ピクセルを縦8回ぶんエンコードする。
	D0 = *pVRAM1++; D1 = *pVRAM1++; D2 = *pVRAM2++; D3 = *pVRAM2++;
	*pPixel++ = _PatternLUT[D0][0] | (_PatternLUT[D1][0] << 1) | (_PatternLUT[D2][0] << 2) | (_PatternLUT[D3][0] << 3); //まずpixel0-3を処理。v2.00更新
	*pPixel++ = _PatternLUT[D0][1] | (_PatternLUT[D1][1] << 1) | (_PatternLUT[D2][1] << 2) | (_PatternLUT[D3][1] << 3); //続いてpixel4-7を処理。
	D0 = *pVRAM1++; D1 = *pVRAM1++; D2 = *pVRAM2++; D3 = *pVRAM2++;
	*pPixel++ = _PatternLUT[D0][0] | (_PatternLUT[D1][0] << 1) | (_PatternLUT[D2][0] << 2) | (_PatternLUT[D3][0] << 3);
	*pPixel++ = _PatternLUT[D0][1] | (_PatternLUT[D1][1] << 1) | (_PatternLUT[D2][1] << 2) | (_PatternLUT[D3][1] << 3);
	D0 = *pVRAM1++; D1 = *pVRAM1++; D2 = *pVRAM2++; D3 = *pVRAM2++;
	*pPixel++ = _PatternLUT[D0][0] | (_PatternLUT[D1][0] << 1) | (_PatternLUT[D2][0] << 2) | (_PatternLUT[D3][0] << 3);
	*pPixel++ = _PatternLUT[D0][1] | (_PatternLUT[D1][1] << 1) | (_PatternLUT[D2][1] << 2) | (_PatternLUT[D3][1] << 3);
	D0 = *pVRAM1++; D1 = *pVRAM1++; D2 = *pVRAM2++; D3 = *pVRAM2++;
	*pPixel++ = _PatternLUT[D0][0] | (_PatternLUT[D1][0] << 1) | (_PatternLUT[D2][0] << 2) | (_PatternLUT[D3][0] << 3);
	*pPixel++ = _PatternLUT[D0][1] | (_PatternLUT[D1][1] << 1) | (_PatternLUT[D2][1] << 2) | (_PatternLUT[D3][1] << 3);
	D0 = *pVRAM1++; D1 = *pVRAM1++; D2 = *pVRAM2++; D3 = *pVRAM2++;
	*pPixel++ = _PatternLUT[D0][0] | (_PatternLUT[D1][0] << 1) | (_PatternLUT[D2][0] << 2) | (_PatternLUT[D3][0] << 3);
	*pPixel++ = _PatternLUT[D0][1] | (_PatternLUT[D1][1] << 1) | (_PatternLUT[D2][1] << 2) | (_PatternLUT[D3][1] << 3);
	D0 = *pVRAM1++; D1 = *pVRAM1++; D2 = *pVRAM2++; D3 = *pVRAM2++;
	*pPixel++ = _PatternLUT[D0][0] | (_PatternLUT[D1][0] << 1) | (_PatternLUT[D2][0] << 2) | (_PatternLUT[D3][0] << 3);
	*pPixel++ = _PatternLUT[D0][1] | (_PatternLUT[D1][1] << 1) | (_PatternLUT[D2][1] << 2) | (_PatternLUT[D3][1] << 3);
	D0 = *pVRAM1++; D1 = *pVRAM1++; D2 = *pVRAM2++; D3 = *pVRAM2++;
	*pPixel++ = _PatternLUT[D0][0] | (_PatternLUT[D1][0] << 1) | (_PatternLUT[D2][0] << 2) | (_PatternLUT[D3][0] << 3);
	*pPixel++ = _PatternLUT[D0][1] | (_PatternLUT[D1][1] << 1) | (_PatternLUT[D2][1] << 2) | (_PatternLUT[D3][1] << 3);
	D0 = *pVRAM1++; D1 = *pVRAM1++; D2 = *pVRAM2++; D3 = *pVRAM2++;
	*pPixel++ = _PatternLUT[D0][0] | (_PatternLUT[D1][0] << 1) | (_PatternLUT[D2][0] << 2) | (_PatternLUT[D3][0] << 3);
	*pPixel   = _PatternLUT[D0][1] | (_PatternLUT[D1][1] << 1) | (_PatternLUT[D2][1] << 2) | (_PatternLUT[D3][1] << 3);
}


/*-----------------------------------------------------------------------------
	[update_sp_tile_cache]
		ＳＰパターンをエンコードし保持します。
-----------------------------------------------------------------------------*/
//Kitao更新。fill_tile_cache()と統合して高速化した。v1.08
//			 タイルキャッシュが更新されているかどうかの判断はdraw_sp_lineのほうで行うようにして高速化。v1.65
static inline void
update_sp_tile_cache(
	Uint32	pc) //Kitao更新。fill_tile_cache()と統合して高速化した。v1.08
{
	int			i;
	Uint8*		pVRAM;
	Uint32*		pPixel; //pixel 0-3 & 4-7。v2.03更新。

	//Kitao更新。使う変数を少なくしシンプル化して高速化。v2.03
	pPixel = (Uint32*)&_SpTileCache[_VN][pc][0];
	pVRAM = &_VideoRam[_VN][pc << 7];

	//横16ピクセルを縦16回ぶんエンコードする。
	for (i=0; i<16; i++)
	{
		*pPixel++ =  _PatternLUT[pVRAM[1]][0] | (_PatternLUT[pVRAM[33]][0] << 1) | (_PatternLUT[pVRAM[65]][0] << 2) | (_PatternLUT[pVRAM[97]][0] << 3); //まずpixel0-3を処理。v2.03更新
		*pPixel++ =  _PatternLUT[pVRAM[1]][1] | (_PatternLUT[pVRAM[33]][1] << 1) | (_PatternLUT[pVRAM[65]][1] << 2) | (_PatternLUT[pVRAM[97]][1] << 3); //続いてpixel4-7を処理。
		*pPixel++ =  _PatternLUT[pVRAM[0]][0] | (_PatternLUT[pVRAM[32]][0] << 1) | (_PatternLUT[pVRAM[64]][0] << 2) | (_PatternLUT[pVRAM[96]][0] << 3); //まずpixel0-3を処理。v2.03更新
		*pPixel++ =  _PatternLUT[pVRAM[0]][1] | (_PatternLUT[pVRAM[32]][1] << 1) | (_PatternLUT[pVRAM[64]][1] << 2) | (_PatternLUT[pVRAM[96]][1] << 3); //続いてpixel4-7を処理。
		pVRAM++; pVRAM++;
	}
}


static inline void
invalidate_tile_cache()
{
	int		i;

	for (i = 0; i < 2048; i++)
		_BgTileCache[_VN][i][0] = 0x20;

	for (i = 0; i < 512; i++)
		_SpTileCache[_VN][i][0] = 0x20;
}


/*-----------------------------------------------------------------------------
	[write_vram]
		ビデオメモリにデータを書き込みます。
		(タイルキャッシュ対応) 
-----------------------------------------------------------------------------*/
static inline void
write_vram(
	Uint16	addr,
	Uint16	data)
{
	//PRINTF("addr, data = %X , %X", (int)addr, data); //Test用

	if ((addr < 0x8000)&&(_pwVideoRam[_VN][addr] != data))
	{
		// 「CG パターンが変更された」フラグおよび
		// 「SG パターンが変更された」フラグを立てる。
		// なお書き込まれたデータが CG/SG パターンとは限らないが、
		// それはかまわない。 
		_BgTileCache[_VN][addr >> 4][0] |= 0x20;
		_SpTileCache[_VN][addr >> 6][0] |= 0x20;
		
		_pwVideoRam[_VN][addr] = data;
	}
}


/*-----------------------------------------------------------------------------
	[update_satb]
		ＳＡＴＢを更新します。
-----------------------------------------------------------------------------*/
//v1.61更新。memcpyで高速化。
static inline void
update_satb(
	Uint16	addr)
{
	Uint32	i = sizeof(Uint16) * 256;

	if (addr > 0x7F00)
	{
		if (addr < 0x8000)
			i = sizeof(Uint16) * (0x8000 - addr);
		else
			return;
	}
	memcpy(_SpRam[_VN], &_pwVideoRam[_VN][addr], i);
}


/*-----------------------------------------------------------------------------
	[do_vram_vram_dma]
		VRAM-VRAM 間のDMA転送を行ないます。

	[NOTE]
		１バイト転送するのに何ＣＰＵサイクルかかるのかは不明。
		VDC_STAT_DV は (DCR & 2) == 0 でもセットされるのかどうかは未確認。←Kitao更新。セットされない。グラディウスで確認。
-----------------------------------------------------------------------------*/
static void
do_vram_vram_dma()
{
	Uint16	srcInc;
	Uint16	dstInc;

	//PRINTF("LENR, DCR , CR = %d , %X , %X", (int)LENR,DCR,CR); //Test用

	//Kitao追加。DCRのbit4が立っていて、描画期間中に転送しようとした場合、転送をVBlank期間に行うようにした。
	if ((DCR & 0x10)&& //参考：DCRのbit4が1…ラングリッサー，謎のマスカレード，ヴォルフィード，BURAI。おそらく、描画期間中の転送を防ぐフラグ？
		(CR & 0xC0)&& //画面表示オンの場合のみ。これがないと謎のマスカレードで画面化け。画面表示オフのときはすぐ転送。
		(_DisplayCounter >= DISPLAY_STARTLINE)&&(_DisplayCounter < _VblankLineTop[_VN])) //描画期間中に転送しようとした
	{
		//ラングリッサーでスクロール時にステータス画面が乱れないために必要。
		_bVramDmaExecute[_VN] = TRUE;
	}
	else //通常
	{
		//Kitao追加。すぐに割り込み要求を実行するとネクタリスの４面以降のスクロールでもっさりするので数クロック経過後に割り込み要求。
		_VramDmaCount[_VN] = 24; //24クロック(全くの推定)後に、"DMA転送開始完了"の割り込み要求をすることにした。ネクタリスで最適。大きくすると音にノイズが載る＆武田信玄で問題。
		
		//ゲーム側はすぐに転送されていることを期待しているので、ここでVRAMを書き換える。
		srcInc = (DCR & 4) ? -1 : 1;
		dstInc = (DCR & 8) ? -1 : 1;
		do
		{
			write_vram(DESR, _pwVideoRam[_VN][SOUR]);
			SOUR += srcInc;
			DESR += dstInc;
		} while (LENR--);
	}
}


//Kitao追加。テレビ画面の水平表示開始位置を設定する。表示幅変数(_TvMax)も設定する。
static void
setTvStartPos()
{
	Sint32	tvWidth =_TvWidth;

	if (_VN != 0) return; //スーパーグラフィックスの場合、VDC0のときだけ設定するようにした。

	if (_TvWidth == 256)
	{
		_TvStartPos = (2 - HDS) * 8; //横256の場合ほとんどがHDS=2
		if (MAINBOARD_GetShowOverscanLeft() > 0) //左右のオーバースキャンエリアを表示する場合。v1.43追加
			tvWidth = 268;
	}
	else if (_TvWidth == 336)
	{
		_TvStartPos = (4 - HDS) * 8; //横352の場合、ほとんどがHDS=3で左右8ドットずつをカット。クイズ殿様の野望(HDS=2)は左16ドットをカット。天地を喰らう(HDS=3)は横368で左8ドット右24ドットをカットして336ドットとして出力。
		if (MAINBOARD_GetShowOverscanLeft() > 0) //左右のオーバースキャンエリアを表示する場合。v1.43追加
			tvWidth = 352;
	}
	else //(_TvWidth == 512)
		_TvStartPos = (11 - HDS) * 8; //横512(シャーロックホームズ)の場合HDS=11。横544モードの場合HDS=8(TVスポーツバスケの選手選択画面)。

	if ((MAINBOARD_GetShowOverscanLeft() > 0)&&(_TvWidth != 512)) //左右のオーバースキャンエリアを表示する場合。v1.43追加
	{
		if (_TvWidth == 256)
		{
			if (_ScreenW[0] > 256)
			{
				if ((HDS > 4)||(_ScreenW[0] >= 256+16)) //HDSが4以下でソース幅が272より小さい場合(264。F1パイロットなど)は、右側だけのソース増になるのでポジションはずらさずそのまま。
					_TvStartPos -= 8;
			}
		}
		else if (_TvWidth == 336)
		{
			if (_ScreenW[0] > 336)
			{
				if ((HDS > 4)||(_ScreenW[0] >= 336+16)) //HDSが4以下でソース幅が352より小さい場合(344など)は、右側だけのソース増になるのでポジションはずらさずそのまま。
					_TvStartPos -= 8;
			}
		}
	}
	else //左右のオーバースキャンエリアを表示しない場合か、横512の場合。v2.10更新
	{
		if (_ScreenW[0] < tvWidth)
			_TvStartPos -= (tvWidth - _ScreenW[0]) * 2;
	}

	if (_TvStartPos < 0) //VCEのドットクロックより先にScreenWidthが変更された場合に起こり得る。TVスポーツバスケで確認。
		_TvStartPos = 0;

	if (_ScreenW[0] > tvWidth) //V1.11追加
		_TvMax = tvWidth;
	else
		_TvMax = _ScreenW[0];

//Kitaoテスト用
//PRINTF("ScanLine=%d, HDS=%X, ScreenW=%d, StartPos=%d , Max=%d", _ScanLine,HDS,_ScreenW[0],_TvStartPos,_TvMax);
/*
if (WINMAIN_GetBreakTrap())
{
	char s[100];
	sprintf(s,"ScanLine=%d, HDS=%X, ScreenW=%d, StartPos=%d , Max=%d", _ScanLine,HDS,_ScreenW[0],_TvStartPos,_TvMax);
	int ok;
	ok = MessageBox(WINMAIN_GetHwnd(),
			s,
			"Test",
			MB_YESNO);
	if (ok != IDYES)
		WINMAIN_SetBreakTrap(FALSE);
}
*/
}


//Kitao追加。テレビ画面の水平表示開始位置を設定する。表示幅変数(_TvMax)も設定する。App.c, MainBoard.c からオーバースキャン表示切替時に利用。
void
VDC_SetTvStartPos()
{
	_VN = 0;
	setTvStartPos();
}


//Kitao追加。垂直表示開始ライン＆VBlankを発生させるラインを決める。表示開始ラインがユニークなソフトにも対応した。
void
VDC_SetTvStartLine()
{
	Sint32	vdw0;
	Sint32	prevStartLine = _TvStartLine[_VN];

	if (VPR & 0x80) //VSW(VPR&0xFF)が負の数なら
	{
		_TvStartLine[_VN] = VDS + (256 - VSW) + 18; //シャーロックホームズ、ヘビーユニットに合わせた
		
		if (VCR > VDS)
			if ((VCR & 0x80) == 0)
				_TvStartLine[_VN] += 10 - (256 - VSW); //シャーロックホームズ２、ヘビーユニット
	}
	else
	{
		_TvStartLine[_VN] = VDS + VSW + 10;
		
		if (VCR > VDS)
			if ((VCR & 0x80) == 0)
				_TvStartLine[_VN] += 10 - VSW; //沙羅曼蛇
		
		if ((VCR == 0x03)&&(VDW == 0xEF)&&(MWR == 0x00)) //スターモビールでオーバースキャンオフのときに一番下のラインが切れるのを回避。
			_TvStartLine[_VN]++;						 //  定規で計って確かめてないが実機でも切れてた気もする(時間あるときに確認)。どちらにしても見やすくなるのでこの実装にする。v1.61
		if ((VCR == 0xF6)&&(VDW == 0xE0)&&(MWR == 0x50))
			_TvStartLine[_VN]++; //悪魔城ドラキュラＸの最終対決や２面で上辺１ラインが乱れないように。実機でもＴＶによって１ラインは乱れていたかもしれない。v1.63
		if ((VCR == 0x04)&&(VDW == 0xE0)&&(MWR == 0x40))
			_TvStartLine[_VN]++; //サークIIIのフィールド画面で上辺１ラインが乱れないように。実機でもＴＶによって１ラインは乱れていたかもしれない。v2.35

		if (VDS+VDW < 255)
		{
			vdw0 = (VDW & 511) + 1;
			if (vdw0 < 240)
			{
				if (VCR & 0x80)
					_TvStartLine[_VN] -= (240 - vdw0) / 2; //悪魔城ドラキュラＸで小数点以下切り捨てがベスト
				else
					_TvStartLine[_VN] -= (240 - vdw0 +1) / 2; //「+1」はダライアスプラスのために四捨五入(ディスプレイによっては実機で最上辺切れてる)
				if (VSW & 4)
					_TvStartLine[_VN]++; //バザールでござーるで上辺１ラインが乱れないように。ぽっぷるメイルで入れるとずれるのでこの位置で処理。v2.11更新
			}
			else
				_TvStartLine[_VN] += (vdw0 - 240) / 2; //ぽっぷるメイルのために四捨五入しないで切り捨て
		}
		else
			if (VDS+VDW < 511)
			{
				if ((VCR & 0x80) == 0)
					_TvStartLine[_VN] += (256 - VDS - (VSW + 1)) - 239; //フラッシュハイダース、ラスタンサーガ２、トイショップボーイズ、レインボーアイランド、TVスポーツフットボール、ストライダー飛竜
				else
					_TvStartLine[_VN] += (256 - VDS) - 239; //あすか120%、天使の詩のカジノゲーム
			}
			else
				_TvStartLine[_VN]--; //パズルボーイ。VDS=13,VDW=511で特殊。コースクリア時のスタッフロールがぴったりに。
	}
	if (_TvStartLine[_VN] < 0)
		_TvStartLine[_VN] = (-_TvStartLine[_VN] +1) / 2; //ファイナルブラスターのスタートデモ画面で必要。v2.14追加

	//描画のオフセット値を設定する。ミズバク大冒険の雷アイテム画面揺れ、スプラッシュレイクで必要。v1.30更新
	_LineOffset[_VN] -= _TvStartLine[_VN] - prevStartLine;
	//PRINTF("%d , %d , %d, %d", _LineOffset[_VN],_TvStartLine[_VN],prevStartLine,_RasterCounter[_VN]); //test用

	//Vblank割り込みを発生させるラインを決める。
	VDC_SetVblankLine();
}


//Kitao追加。Vblank割り込みを発生させるラインを決める。
void
VDC_SetVblankLine()
{
	//Vblank割り込みを発生させるラインを決める。
	//クロスワイバーのOPデモループで261以上が必要。レインボーアイランドで263が必要。
	//イメージファイト２で260以上が必要。ソルジャーブレイド３面の橋部分で261以上が必要。
	//マジカルチェイスのタイトル画面で254-256辺りか260以上(VBlankTop)が必要。コズミックファンタジー２で260が必要(これ以上でも以下でも、買い物時に実機と違う揺れになる)。
	//風の伝説ザナドゥ２のプロローグ「風の伝説ザナドゥII」の文字が出る所で205-208(VBlankTop)辺りが必要。
	//パワーゴルフ２のクラブ選択時に261以上が必要。あすか120%の試合前デモで257か256が必要。
	//プライベートアイドルのスタートデモで上辺が乱れないために259辺りが必要。F1サーカス'92で262以上が必要。
	//ファージアスの邪皇帝の戦闘シーンで259以上が必要。雀神伝説のステータス表示で261あたりが必要。
	//フレイCDで251以上が必要。眠れぬ夜の小さなお話で252以下が必要。雀偵物語３のデモ(序盤ピアノのシーンの手前)で243が必要(244,242NG)。
	//BURAIのOPで255以上が必要(大きくても駄目)。モンビットのOPで260以上が必要。チェイスHQのタイトル画面で260以下が必要。
	//スプリガンmk2のステージ８デモ終盤で250が必要(249NG,251NG)。サーカスライドで音異常を起こさないために262以下が必要。
	_VblankLine[_VN] = DISPLAY_STARTLINE + _ScreenH[_VN] + 2;
	if (_VblankLine[_VN] > 263) _VblankLine[_VN] = 263; //ここでいったん263以下に整える。

	if ((MWR & 0x70) >= 0x50) //MWRが0x50以上のとき調整する。この条件がないと雀神伝説(MWR=0x40)のステータス画面で乱れ，アドベンチャーアイランド(MWR=0x10)の画面切り替え時に乱れ，マッドストーカー５面(MWR=0x10)で乱れ。v2.61
	{
		if (VCR & 0x80) //VCRが大きいとき
		{
			if (VDW <= 224) //この条件がないと雀偵物語３のデモ(１話のチョンボアップシーン)で乱れ。v2.36
			{
				if (VCR >= 240)
					_VblankLine[_VN] += (VCR-240); //+6以上でドラキュラX(MWR=0x50)の最終面の階段を上がるところでの乱れ＆極稀にフリーズする問題が解消。v1.61,v2.36更新
			}
			else if (VDW <= 232)
				_VblankLine[_VN] -= (232-VDW)/2; //-3で雀偵物語３(MWR=0x50,VDW=226)のデモ(序盤ピアノのシーンの手前)で画面揺れが解消。引きすぎるとダイナスティックヒーロー序盤(VDW=231)で画面化け。v2.40
			else
				_VblankLine[_VN] -= 2; //-2でコズミックファンタジー２買い物説明時の揺れが実機と同じに。v2.61
		}
		else if (VCR > 4) //VCRが標準より少し大きい時。v2.49更新
		{
			_VblankLine[_VN] += (VCR-4); //5以下を足すことで、マジカルチェイスのタイトル画面デモがループするときに音プチノイズが解消。v1.63,v2.36,v2.41,v2.49
										 //9以下を足すことで、TRAVELエプルのOPデモで１フレームの乱れが解消。5でBGMとのタイミングがベスト。
		}
		else if (VCR < 3) //VCRが小さい時。v2.65追加
		{
			_VblankLine[_VN] -= (3-VCR); //1以上を引くことで、フラッシュハイダースの試合決着時の画面ブラックアウトが解消。
		}
		if (_VblankLine[_VN] > 263) _VblankLine[_VN] = 263; //ここでいったん263以下に整える。
	}

	if ((CR & 0x100) == 0) //表示出力選択の下位ビットが立っていた場合は処理しない。この条件がないとレインボーアイランドのHurryUp時に乱れ。
	{
		if ((MWR & 0x60) == 0) //BGキャラマップが32x32(0x00),64x32(0x10)のとき調整する。この条件がないとBURAIのスタート,F1サーカス'92で決勝ゴール後(わずかな乱れは実機もある)，メタルエンジェルのタイトル画面,雀神伝説(MWR=0x40)のステータス画面で乱れ。v2.37,v2.41
		{
			if (VCR >= 4) //VCRが標準以上のとき。これがないとクロスワイバーNG。
			{
				_VblankLine[_VN]--; //-1以上でスト２’(MWR=0x10)のVEGA,RYUステージ開始時等の１フレーム乱れが解消。-1でスト２’(MWR=0x10)のエンディング(ノーコンティニュー)の処理落ち具合(曲の聞きやすさ)が最適。
									//-1でプライベートアイドル(MWR=0x10)のデモシーンで上辺の乱れが解消。(多く引くと駄目)。
									//-2か-1であすか120%(MWR=0x10)の試合前デモで揺れが解消。引きすぎると逆に乱れ。
									//-2以上引くと風ザナ２のOPデモで下辺乱れ。-2以上引くとソルジャーブレイド３面で乱れ。
									//-2以上引くとドラゴンスレイヤー英雄伝説の電源投入後デモで下辺が乱れ。v1.65更新
									//-3以上引くとフレイCDで乱れ。多く引きすぎると、お嬢様伝説ユナで乱れ。引きすぎるとニンジャウォリアーズの面クリ時揺れ。
			}
			if (_ScreenH[_VN] >= 240) //カードエンジェルスで引きすぎないために必要(引きすぎると最下辺が乱れ)。v1.67
			{
				_VblankLine[_VN]--; //-1以上でスト２’のVEGA,RYUステージ開始時等の１フレーム乱れが解消。エンディング(ノーコンティニュー)の処理落ち具合(曲の聞きやすさ)も変わる。
									//-1でプライベートアイドルのデモシーンで上辺の乱れが解消(逆に多く引いてもNG)。-1以上でパラソルスター面クリア時の揺れが解消。-1でお嬢様伝説ユナのOP乱れが解消。
									//-2以上引くとクロスワイバーがNG。-2以上引くとモンビットでNG。-2以上だとソルジャーブレイド３面で乱れ。
				if (VDS >= 0x10) //VDSが標準より大きい場合。この条件がないとBURAIのステータス表示キャンセル時に一瞬乱れ。v1.68,v2.37更新
					_VblankLine[_VN] -= 3; //-3であすか120%の試合前デモの１フレーム乱れが解消。v2.49
			}
			if (VSW == 0) //垂直同期パルス幅が0の場合
			{
				if (_ScreenH[_VN] < 240) //240以上のとき引くと、TVスポーツフットボールで下辺乱れ。v2.49
					if (VDS <= 27)
						_VblankLine[_VN] -= (27-VDS); //TVスポーツアイスホッケー(MWR=0x10)のアナウンサー乱れが軽減。大きく引きすぎるとTVスポーツバスケで下辺乱れ。v2.49更新
			}
		}
	}
	else //表示出力選択の下位ビットが立っていた場合
		_VblankLine[_VN] = 263; //スプラッシュレイクの面クリア時などで１フレームの乱れが解消。レインボーアイランド。v2.45
	//↑ここの処理は263以下に整えた後に行う。あすか120%OK。お嬢様伝説ユナOK。

	if (VDW & 0x100) //パズルボーイは特殊。コースクリア時に必要。v2.17
		_VblankLine[_VN] = 2; //2。4～9だとスタッフロール中に一時停止してしまう。実機では一時停止しないことを確認済み。

	if (_VblankLine[_VN] > 263)
		_VblankLine[_VN] = 263; //最大263。レインボーアイランド。v1.02更新

//Kitaoテスト用
//PRINTF("VbLine=%X, %d, %d, %d  [ %X , %X , %X , %X, %X, %X ] VBline=%d",_DCC,_TvStartLine[_VN],DISPLAY_STARTLINE,_ScreenH[_VN], VPR,VDW,VCR,MWR,DCR,CR, _VblankLine[_VN]);
//PRINTF("Kekka=%x, %x, %x, %x, %x, %x, %x, %x", _Regs[0][0],_Regs[0][1],_Regs[0][2],_Regs[0][3],_Regs[0][4],_Regs[0][5],_Regs[0][6],_Regs[0][7]);
//PRINTF("Kekka=%x, %x, %x, %x, %x, %x, %x, %x", _Regs[0][8],_Regs[0][9],_Regs[0][10],_Regs[0][11],_Regs[0][12],_Regs[0][13],_Regs[0][14],_Regs[0][15]);
//PRINTF("Kekka=%x, %x, %x, %x, %x, %x, %x, %x", _Regs[0][16],_Regs[0][17],_Regs[0][18],_Regs[0][19],_Regs[0][20],_Regs[0][21],_Regs[0][22],_Regs[0][23]);
//PRINTF("Kekka=%x, %x, %x, %x, %x, %x, %x, %x", _Regs[0][24],_Regs[0][25],_Regs[0][26],_Regs[0][27],_Regs[0][28],_Regs[0][29],_Regs[0][30],_Regs[0][31]);
/*
//if (WINMAIN_GetBreakTrap())
{
	char s[100];
	sprintf(s,"Kekka=%d , %d  [ %x , %x , %x ] VBline=%d   ",_TvStartLine[_VN],_ScreenH[_VN],VPR,VDW,VCR,_VblankLine[_VN]);
	int ok;
	ok = MessageBox(WINMAIN_GetHwnd(),
			s,
			"Test",
			MB_YESNO);
	if (ok != IDYES)
		WINMAIN_SetBreakTrap(FALSE);
}
*/
}


//Kitao追加。スーパーグラフィックスのVPC処理のエミュレート。プライオリティ情報を設定する。
void
VDC_SetVpcPriority()
{
	_VpcPriority[0] = (_VPC[0]     ) & 0xF; //Border1とBorder2よりも右側の領域用
	_VpcPriority[1] = (_VPC[0] >> 4) & 0xF; //Border1より右でBorder2より左側の領域用
	_VpcPriority[2] = (_VPC[1]     ) & 0xF; //Border1より左でBorder2より右側の領域用
	_VpcPriority[3] = (_VPC[1] >> 4) & 0xF; //Border1とBorder2よりも左側の領域用
	_VpcBorder1 = ((_VPC[3] & 3) << 8) + _VPC[2] - 128;
	_VpcBorder2 = ((_VPC[5] & 3) << 8) + _VPC[4] - 64;
}


/*-----------------------------------------------------------------------------
	[VDC_Write]
		ＶＤＣへの書き込み動作を記述します。
-----------------------------------------------------------------------------*/
void
VDC_Write(
	Uint32	regNum,
	Uint8	data)
{
	//Kitaoテスト用
	//if (regNum & 0xFC)
	//{
	//	PRINTF("VDC_Write test! = %X, %X",regNum,data);
	//	return;
	//}

	//Kitao追加。スーパーグラフィックス用
	if (_SuperGrafx)
	{
		_VN = (regNum & 0x10) >> 4; //アクセスするVDC(0 or 1)
		if (regNum & 0x08)
		{	//VPC Write;
			regNum &= 7;
			_VPC[regNum] = data;
			switch (regNum)
			{
				case 0:
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
					VDC_SetVpcPriority();
					return;
				case 6:
					CPU_SelectVDC(data & 1);
					return;
			}
			return;
		}
	}
	else
		_VN = 0;

	regNum &= 3; //v2.31更新

	//VDCポートアクセスのウェイト。ネクロスの要塞(DCC=0,MWR=0x50)のOPデモでアマゾンの攻撃のはみ出し具合が実機と同様に。ゲンジ通信あげだまのOP(MWR=0x50)でも必要。
	//  サイレントデバッガーズ(VCR=4,MWR=0x10)で画面化けを起こさないために必要。ボンバーマン'93スタートデモ(VCR=4,MWR=0x00)で１フレーム乱さないために必要。
	//  姐(VCR=0xEE,MWR=0x10)の通販画面でスクロール時に１フレーム乱さないために必要。 サイバードッジ(MWR=0x50)で画面が乱れないために必要。
	//  サーク１２(VCR=3,MWR=0x40)でステータス画面を開くときに乱れないために必要。
	//  オーダイン(VCR=4,MWR=0x50)のショップ買い物後に１フレーム乱れないために必要。パックランド(VCR=3,MWR=0x10)で面開始時(特に2面開始)に１フレーム乱れないために必要。
	//  ロックオン(VCR=0xC,MWR=0x50)でやられた画面で必要。妖怪道中記(DCC=0,VCR=3,MWR=0x30)の賭博に入る画面で必要。
	//  ぽっぷるメイル(VCR=4,MWR=0x10)のオープニングデモで乱れないために必要。
	//  3x3EYES(VCR=0xBA,0xF6,MWR=0x00,0x40)のオープニングデモで音ズレを起こさないために必要。
	//  特定の設定時に、VDCアクセス時のウェイトをカット(実機では未検証)。ワースタ'91等が実機同様の動きに。v2.63更新
	if (gCPU_Transfer == 0) //転送命令以外でのVDCアクセスの場合。v2.31更新
	{
		//転送命令以外でのVDCアクセスの場合、_bVDCAccessWait[_VN]に関わらずにアクセスウェイトを入れる。v2.10追加
		//  サイレントデバッガーズ，ネクロスの要塞，ゲンジ通信あげだま，姐，サーク１２(MWR=0x40,CR=0x8C)，ワースタ&'91(MWR=0x50)ではウェイトが必要。
		//  regNum==2のときに、シティハンター電源投入デモ(VCR=3,MWR=0x10,CR=0xCC)でウェイトを入れると下辺が乱れ。v2.31追加
		if (regNum == 3)
		{	//上位バイト書き込み時にまとめてウェイトを発生させることで、ゼロヨンチャンプでのレース画面乱れが解消(稀に揺れるのは実機も同様)。v2.40更新
			if (_bVDCAccessWait[_VN]) //これでウェイトをカットしないと、ゼビウス(VCR=4,MWR=0x00)でやられたときに右下乱れ＆稀にフリーズ。
			{
				gCPU_ClockCount--; //CPU 1 Clock Wait. サイレントデバッガーズ(MWR=0x10)，姐(MWR=0x10)でウェイトが必要。
				gCPU_ClockCount--; //CPU 1 Clock Wait. ボンバーマン'93(VCR=4,MWR=0x00)でこのもう1ウェイトが必要。
			}
			else 
			{
				//MWR=0x00以外の場合は必ず1ウェイト。MWR=0x00のときに入れると、ゼビウス(VCR=4,MWR=0x00)でやられたときに右下乱れ＆稀にフリーズ。
				if (MWR & 0x70)
					gCPU_ClockCount--; //CPU 1 Clock Wait. ぽっぷるメイル(MWR=0x10)でウェイトが必要。
									   //	MWR=0x10のときに2ウェイト入れると、シティハンター電源投入デモ(VCR=3,MWR=0x10,CR=0xCC)で下辺が乱れ。
									   //	MWR=0x50のときに2ウェイト入れると、高橋名人の新冒険島で、ダンスシーン前等のBGM鳴り始めにノイズが入る。v2.58
			}
		}
		else if (regNum == 0)
			gCPU_ClockCount--; //CPU 1 Clock Wait. F1サーカスのスターティンググリッド表示(MWR=0x50)でウェイトが必要。
	}
	else //転送命令からのVDCアクセスの場合
	{
		//VDC利用状況によりアクセスウェイトを入れる。3x3EYES，オーダイン，ボンバーマン'93，パックランド，ロックオンでウェイトが必要。
		if (_bVDCAccessWait[_VN]) //これでウェイトをカットしないと、ワンダーモモ(VCR=3,MWR=0x30)，ワールドスタジアム初代&'91(VCR=4,MWR=0x50)でファール再開時に乱れ，アルガノスでフリーズ。
			gCPU_ClockCount--; //CPU 1 Clock Wait
		//  ワールドスタジアム'91で盗塁で画面切り替え時に１フレーム乱れるのは実機も同様。
	}

	//v1.15更新。高速化。case文は、よく使われるものを先に置いたほうが速く処理できそう。
	switch (regNum)
	{
		case 3: // 上位バイトの書き込み
			_Regs[_VN][_AR[_VN]] = (_Regs[_VN][_AR[_VN]] & 0x00FF) | (data << 8); //Kitao更新

			// 標準的な書き込み以外に特別な処理が必要な場合は以下に記述する 
			switch (_AR[_VN])
			{
				case VDC_VWR:  // vram data write
					write_vram(MAWR, VWR);
					MAWR += _VdcAddrInc[_VN];
					if (_SpDmaCount[_VN] == 0) return;//v1.61。高速化のため0ならすぐ帰る。
					
					//Kitao追加。転送中にVRAMへの書き込みがあった場合、CPUがストールする。v0.87
					//	情報源：Kiさんのホームページ「SATB転送中にVRAMライトがあるとCPUが転送完了までストールする」
					if ((_SpDmaCount[_VN] == 4)|| //SATB転送中に、VRAMライトが行われた場合
						((_SpDmaCount[_VN] == 3)&&(_CpuPastCount < 88))) //88。約ライン5分の1までのタイミング時とした。90だとダンジョンエクスプローラのOPスタッフロールで１ライン乱れることあり。ダブルドラゴンIIは多いほど最適。4分の1だと奇々怪界のOPデモ(船お払い)で乱れ。クロスワイバーで80(5.5分の1)ぐらい以上の値が必要。大きいとチェイスHQスタート時、ZIPANGスタート時にやや乱れ。v2.63更新
					{
						if (_VN == 0) //スパグラの場合、二重にウェイトしてしまわないようにVDC0のときのみウェイトとする。
						{
							//PRINTF("%X, %d, %d , %d", VCR,_ScanLine,_SpDmaCount[0],_CpuPastCount); //test用
							if (VCR < 4) //VCRが小さいときは、全転送分ストールさせるようにした。クロスワイバー,ワンダーモモで必要。v2.08,v2.31更新
								_SpDmaStole[0] = VDC_CYCLESPERLINE-_CpuPastCount + 1408;//1408; //ライン終端までぶんストール＋1408=5.5クロックx256byte(約３～４ライン。推定)ぶんCPUをストール。クロスワイバーのデモ２ループ目以降で必要。
							else if (VCR & 0x80) //VCRが大きいときは、多めにストールさせるようにした。ダブルドラゴンIIのOPデモで必要。v2.44更新
								_SpDmaStole[0] = (VDC_CYCLESPERLINE*(_SpDmaCount[0]-2) - _CpuPastCount) /  2; //CPUをストール。1/2にした。これを入れることで、ダブルドラゴンIIのOPデモ(最後の「あのビルだな」の口パク)で丁度いい。"マクロス2036のタイトル画面"OK。v2.44更新
							else //通常
								_SpDmaStole[0] = (VDC_CYCLESPERLINE*(_SpDmaCount[0]-2) - _CpuPastCount) /128; //CPUをストール。1/128にした。1/80だとサイドアームSPのアレンジモード開始時にステータス揺れ。これを入れることで、ZIPANG面スタート時も実機に近い率で主人公周りが抜ける。v2.61更新
																											  //						    エターナルシティでアイテム入手時などで画面揺れ解消1/64NG。"ダンジョンエクスプローラーのOPストーリースクロール"で1/50NG。"ロードス島戦記２"のエンディングで1/8NG。スト２’のベガステージ開始時に大きいと乱れ。
																											  //						    奇々怪界のOPデモ(船お払い)で稀に乱れることがある(1/4だと極稀に)。入れすぎると"チェイスH.Q.のタイトルデモ画面"にやや乱れ(1/3で乱れ)。"とらべらーず！"のイベントシーンで音ズレすることがある。
							gCPU_ClockCount -= _SpDmaStole[0]; //CPU Clock Wait
						}
					}
					//if (_SpDmaCount[_VN] >= 2)
					//	PRINTF("%d , %d", _SpDmaCount[_VN],_CpuPastCount); //test用
					return;

				case VDC_MARR:  //Kitao追加。VRAMリードアドレスをセットしたと同時にVRAMリードアクセスがおこなわれる。
					if (MARR < 0x8000)
						_ReadData[_VN] = _pwVideoRam[_VN][MARR]; //リードアクセス
					else
						_ReadData[_VN] = 0;
					return;

				case VDC_CR:  // control
					_VdcAddrInc[_VN] = _IncSizeTable[(data >> 3) & 3];
					return;

				case VDC_RCR:  // raster detection
					RCR &= 0x3FF;
					//Kitao追加。ここですぐにラスタ比較を行なう。１ラインごとにラスタスクロールするソフトで必要なときがある。
					if (_bRasterRequested[_VN] == FALSE) //まだこのラインでラスタ割り込み要求が行われていない場合
						if ((((_RasterCounter[_VN]-64) & 0xFF) == RCR-64)&&(_RasterCounter[_VN] >= 64))
						{
							if (CR & VDC_CTRL_RC)
							{
								_VdcStatus[_VN] |= VDC_STAT_RR;
								INTCTRL_Request(INTCTRL_IRQ1);
							}
							_bRasterRequested[_VN] = TRUE;
						}
					return;

				case VDC_BXR:  // bg x scroll
					BXR &= 0x3FF;
					return;

				case VDC_BYR:  // bg y scroll
					BYR &= 0x1FF;
					_LineCounter[_VN] = BYR; //Kitao更新。シンプルにした。
					if (!_bRasterLate) //Kitao追加。ライン描画前にラスタ割り込み要求があった場合、_LineCounterをいち早くインクリメント処理する。※上位バイトのときも行う。フォゴットンワールドで必要。v1.03
						if (_RasterCounter[_VN] != 64) //割り込み開始ライン(_RasterCounter==64)上では必ずRasterLate状態になる。フォゴットンワールドのお店シーン,BURAIなど。
							++_LineCounter[_VN]; //ダンジョンエクスプローラー、パックランド、ブラッディウルフ、エターナルシティ等で必要。
					_LineCounter[_VN] &= _LineCountMask[_VN];
					return;

				case VDC_HSR:  //Kitao追加。HDS(HSRのbit8-14)が変更されるとTV上の水平表示開始位置が変わる。
					setTvStartPos();
					return;

				case VDC_VPR:  //Kitao追加。VSyncレジスタ(上位バイト=VDS)。VDSが変更されるとTV上の最上段描画ラインが変わる。
					VDC_SetTvStartLine();
					return;

				case VDC_VDW:  // vertical display width
					_ScreenH[_VN] = (VDW & 511) + 1;
					//Kitao追加。VDWが変更されるとTV上の最上段描画ラインが変わる。
					VDC_SetTvStartLine();
					return;

				case VDC_LENR:
					do_vram_vram_dma();
					return;

				case VDC_SATB: //SATBのアドレスを設定＆転送の予約を入れる。
					_bUpdateSATB[_VN] = TRUE;
					return;
			}
			return;

		case 2: // 下位バイトの書き込み
			_Regs[_VN][_AR[_VN]] = (_Regs[_VN][_AR[_VN]] & 0xFF00) | data; //Kitao更新。v2.50追記：ここで上位バイトをクリアした場合、妖怪道中記の１面中盤から画面が化ける。

			// 標準的な書き込み以外に特別な処理が必要な場合は以下に記述する 
			switch (_AR[_VN])
			{
				case VDC_VWR:  //Kitao追加。v1.61
					return; //何もしないですぐに戻る。頻繁に使われるここ(ブロック転送などで連続して使われる)をcaseの先頭に持ってくることで高速化になる。

				case VDC_BYR:  // BG Y scroll
					_LineCounter[_VN] = BYR; //Kitao更新。シンプルにした。
					if (!_bRasterLate) //Kitao追加。ライン描画前にラスタ割り込み要求があった場合、_LineCounterをいち早くインクリメント処理する。
						if (_RasterCounter[_VN] != 64) //割り込み開始ライン(_RasterCounter==64)上では必ずRasterLate状態になる。フォゴットンワールドのお店シーン,BURAIなど。
							++_LineCounter[_VN]; //ダンジョンエクスプローラー、パックランド、ブラッディウルフ、エターナルシティ等で必要。
					_LineCounter[_VN] &= _LineCountMask[_VN];
					return;

				case VDC_MWR:  // Memory width
					//Kitao更新。MWRを更新したときは、Vblank期間中にメモリマップの更新を行うようにした。
					//			 リンダキューブのプロローグ、SUPER桃鉄の優勝者発表時に画面が崩れる問題を解消。
					_bMWRchange[_VN] = TRUE; //Kitao追加
					VDC_SetTvStartLine(); //MWRが変更されるとTV上の最上段描画ラインとVBlankラインが変わる。
					return;

				case VDC_HDR:  // horizontal display
					_ScreenW[_VN] = ((data & 0x7F) + 1) * 8;
					setTvStartPos(); //Kitao追加。HDRが変更されるとTV上の水平表示開始位置が変わる。
					if ((_ScreenW[_VN] <= 85)&&(VCR <= 4)) //85(256/3。実機未確認)。TRAVELエプルのOP，スターブレーカーの「アレムの町」で一泊するとき(_TvMax=112)、(_VPRが大きめのとき)にはこの処理をしない。v2.49更新
						_ScreenW[_VN] = _TvWidth; //ファイナルブラスターのスタートデモで必要。※_TvMaxは小さなまま。v2.14
					return;

				case VDC_VPR:  //Kitao追加。VSyncレジスタ(下位バイト=VSW)。VSWが変更されるとTV上の最上段描画ラインとVBlankラインが変わる。
					VDC_SetTvStartLine();
					return;

				case VDC_VDW:  //Kitao追加。下位バイトだけ変更する場合がある。ドラゴンスレイヤー英雄伝説２。
					_ScreenH[_VN] = (VDW & 511) + 1;
					//VDWが変更されるとTV上の最上段描画ラインとVBlankラインが変わる。
					VDC_SetTvStartLine();
					return;

				case VDC_VCR:  //Kitao追加。VCR(垂直表示終了位置)が変更されるとTV上の最上段描画ラインとVBlankラインが変わる。
					VDC_SetTvStartLine();
					return;

				case VDC_SATB: //SATBのアドレスを設定＆転送の予約を入れる。v1.61下位バイト書き換え時にも追加。
					_bUpdateSATB[_VN] = TRUE;
					return;
			}
			return;

//		case 1: // AR に上位ビットはない。 
//			return;

		case 0: // VDC のレジスタ番号を指定する。
			_AR[_VN] = data & 0x1F;	// AR = Address Register 
			return;
	}
}


/*-----------------------------------------------------------------------------
	[VDC_Read]
		ＶＤＣからの読み出し動作を記述します。

	[note]
		CR の D11-D12 で決定されるＶＲＡＭアドレスインクリメント値は
		MAWR, MARR の両方に有効である。 (01.10.2004 確認済み)
-----------------------------------------------------------------------------*/
Uint8
VDC_Read(
	Uint32	regNum)
{
	Uint8	ret;

	//Kitao追加。スーパーグラフィックス用
	if (_SuperGrafx)
	{
		_VN = (regNum & 0x10) >> 4; //アクセスするVDC(0 or 1)
		if (regNum & 0x08)
			return (Uint8)_VPC[regNum & 7]; //VPC Read;
	}
	else
		_VN = 0;

	//Kitaoテスト用。スタートリングオデッセイ２の止まるところ(洞窟抜けてソルディア大陸)でレジスタNo.に大きな値連発。
	//if (regNum & 0xFC)
	//	PRINTF("regNum = %X, %X, %X, %X", regNum,_ReadData[_VN],_VdcStatus[_VN],_AR[_VN]); //test用

	//regNumのbit6(0x40)はYs4(大樹の大雨の場面)で立っていることがあり、それは有効。regNumの最上位(0x80)はドロップロックほらホラで立っていることがあり、それは有効。
	//regNumのbit5(0x20)は凄ノ王伝説で立っていることがあり、それは有効。bit4はスパグラで使われている。
	if ((regNum & 0xE0) == 0xE0) //bit5-7まで全てが立っていてる場合。bit4-7までにすると１フレーム乱れ。v1.64更新
	{
		//PRINTF("regNum = %X, %X, %X, %X", regNum,_ReadData[_VN],_VdcStatus[_VN],_AR[_VN]); //test用
		if (_AR[_VN] == VDC_VRR)
			return 0; //プリンセスメーカー２で0を返す必要がある。v2.24
		else
			return 0xFF; //フォーメーションサッカー'90のエンディングで0xFFを返すことが必要。0を返すと一瞬音がスローになる。
		//※フォーメーションサッカーがregNumが大きなまま読みに来ること自体が、実機と違う動きになってしまっている可能性がある。
		//  現状は対処療法で。v2.24記
	}

	//v1.15更新。高速化。case文は、よく使われるものを先に置いたほうが速く処理できそう。
	switch (regNum & 3)
	{
		case 3: // 上位バイトの読み出し。Kitao更新。_ARがVDC_VRRではないときも、MARRの内容を返すようにした。21エモンで必要
			ret = (Uint8)(_ReadData[_VN] >> 8);
			if (_AR[_VN] == VDC_VRR)
			{
				MARR += _VdcAddrInc[_VN];
				//Kitao追加。アドレスがインクリメントされたと同時にVRAMリードアクセスが起こる。
				if (MARR < 0x8000)
					_ReadData[_VN] = _pwVideoRam[_VN][MARR]; //リードアクセス
				else
					_ReadData[_VN] = 0;
			}
			return ret;

		case 2: // 下位バイトの読み出し。Kitao更新。_ARがVDC_VRRではないときも、MARRの内容を返すようにした。21エモンで必要
			return (Uint8)_ReadData[_VN];

		case 1: //v1.62更新
			//PRINTF("VDC_Read reg=1 !!"); //test。ミズバク大冒険１面スタート直後,スターブレーカーなどで使われている。
			return 0; //0xFFを返すとミズバク大冒険の橋の辺り判定が無くなってしまう。おそらくステータスの上位バイト扱いなので0としておく。
			
		case 0:
			ret = _VdcStatus[_VN];
			//Kitao更新。v0.96。スト２’で１フレームの乱れが起こらないために、ラスタ割り込みと他の割り込みが重なってしまうことを防ぐ。
			//if (((_VdcStatus[_VN] & VDC_STAT_RR) == 0)||(_VdcStatus[_VN] == VDC_STAT_RR)) //CPUの割り込みタイミングのほうが重要。現在不要でシンプルに。v1.61カット
			INTCTRL_Cancel(INTCTRL_IRQ1); //Kitao更新。v0.95。通常はシンプルにキャンセルされる。
			_VdcStatus[_VN] = 0; //Kitao更新。おそらくシンプルに、リードされるとクリアされる。
			return ret;
	}

	return 0xFF; //v1.62更新。VCEのRead同様に、ポートに対しての無効なレジスタ指定時には0xFFを返しておく。
}


/*-----------------------------------------------------------------------------
** [CreatePatternTables]
**	キャラクタパターン → ビットマップデータへの変換ＬＵＴを作成します。
**	CH0 ～ CH3 用の４つを作成します。
**
**	CH0 の１バイト(D0 - D7) を，８バイトに展開する。その展開の仕方は：
**
**	byte	00 01 02 03 04 05 06 07
**	data	D7 D6 D5 D4 D3 D2 D1 D0   (データビットは全て D0 へシフトしたもの)
**
**	CH1 の１バイト(D0 - D7) を，８バイトに展開する。その展開の仕方は：
**
**	byte	00 01 02 03 04 05 06 07
**	data	D7 D6 D5 D4 D3 D2 D1 D0   (データビットは全て D1 へシフトしたもの)
**
**	CH2 の１バイト(D0 - D7) を，８バイトに展開する。その展開の仕方は：
**
**	byte	00 01 02 03 04 05 06 07
**	data	D7 D6 D5 D4 D3 D2 D1 D0   (データビットは全て D2 へシフトしたもの)
**
**	CH3 の１バイト(D0 - D7) を，８バイトに展開する。その展開の仕方は：
**
**	byte	00 01 02 03 04 05 06 07
**	data	D7 D6 D5 D4 D3 D2 D1 D0   (データビットは全て D3 へシフトしたもの)
**
**	[2004.3]
**	CH1-CH3 は CH0 を左に 1～3 ビットシフトしただけなので、
**	CH0 のみを作成し、CH1～CH3 は CH0 をシフトして対応するように変更した。
**---------------------------------------------------------------------------*/
static void
createPatternLUT()
{
	Uint32	i;
	Uint32	D0, D1, D2, D3, D4, D5, D6, D7;

	for (i = 0; i < 256; i++)
	{
		// 全て D0 へシフトする。
		D7 = (i & 0x80) >> 7;
		D6 = (i & 0x40) >> 6;
		D5 = (i & 0x20) >> 5;
		D4 = (i & 0x10) >> 4;
		D3 = (i & 0x08) >> 3;
		D2 = (i & 0x04) >> 2;
		D1 = (i & 0x02) >> 1;
		D0 = (i & 0x01) >> 0;

		_PatternLUT[i][0] = D7 | (D6 << 8) | (D5 << 16) | (D4 << 24);
		_PatternLUT[i][1] = D3 | (D2 << 8) | (D1 << 16) | (D0 << 24);
	}
}


//Kitao追加
void
VDC_Reset()
{
	int		i;

	memset(_VideoRam, 0, sizeof(_VideoRam));
	memset(_SpRam, 0, sizeof(_SpRam)); //Kitao追加
	memset(_Regs, 0, sizeof(_Regs));
	memset(_AR, 0, sizeof(_AR));
	
	memset(_BgPutBuf, 0, sizeof(_BgPutBuf)); //Kitao追加
	memset(_SpPutBuf, 0, sizeof(_SpPutBuf)); //Kitao追加

	//Kitao追加。スパグラ用に２つぶんリセットする
	for (_VN=0; _VN<=1; _VN++)
	{
		_pwVideoRam[_VN] = (Uint16*)&_VideoRam[_VN][0];
		_ReadData[_VN] = 0; //Kitao追加
		
		//Kitao追加。VDC_Deinit()から必要なぶんをこちらへ持ってきた。
		_VdcStatus[_VN] = 0;
		_bUpdateSATB[_VN] = FALSE;
		_MaxSpPerLine[_VN] = 16; //Kitao追加。v2.63
		_bSpOver[_VN] = FALSE;
		_bOverlap[_VN] = FALSE; //Kitao追加
		_SpDmaCount[_VN] = 0;
		_SpDmaStole[_VN] = 0; //Kitao追加
		_VramDmaCount[_VN] = 0; //Kitao追加
		_bVramDmaExecute[_VN] = FALSE; //Kitao追加。v1.02
		_bBurstMode[_VN] = FALSE;
		_bVDCAccessWait[_VN] = TRUE; //v2.08追加
		_bMWRchange[_VN] = FALSE; //Kitao追加
		_bVDCleared[_VN] = FALSE; //v1.63追加
		
		_RasterCounter[_VN] = 0;
		_bRasterRequested[_VN] = FALSE; //Kitao追加
		_LineCounter[_VN] = 0;
		_LineCountMask[_VN] = 0xFF;
		_LineOffset[_VN] = 0; //Kitao追加。v1.30
		_PrevVSW[_VN] = 0; //2.10追加
		
		_BGH[_VN] = 0;
		_BGW[_VN] = 0;
		//
		
		_ScreenW[_VN] = 256;
		_ScreenH[_VN] = 240;
		_TvStartLine[_VN] = 27; //Kitao追加
		_TvStartLineAdjust[_VN] = 0; //Kitao追加
		_VblankLine[_VN] = 262;
		_VblankLineTop[_VN] = _VblankLine[_VN]; //Kitao追加
		_prevCR[_VN] = 0; //Kitao追加。v1.21
		
		VDW = 239;
		VCR = 4;
		VPR = 0x0F02;
		
		_VdcAddrInc[_VN] = 1; // この初期化は必要  FIXED 2004.09.11 
		
		invalidate_tile_cache();
	}

	_ScanLine = 0;
 	_DisplayCounter = 0;
	_bRasterLate = TRUE; //Kitao追加

	//起動背景色を白に
	for (i = 0x000; i < 0x200; i++)
		_Palette333[i] = 0x1FF;
	for (i = 0x000; i < 0x100; i++)
		_PaletteBG[i] = _PaletteSP[i] = 0x00070707; //Kitao更新

	_DCC = 0;
	_ColorTableAddr = 0;
	_ColorLatch = 0;
	_TvStartPos = 0; //Kitao追加
	_TvWidth = 256; //Kitao追加
	_TvMax = 256; //Kitao追加

	//Kitao追加。スパグラ用
	memset(_VPC, 0, sizeof(_VPC));
	memset(_VpcPriority, 0, sizeof(_VpcPriority));
	_VpcBorder1 = 0;
	_VpcBorder2 = 0;
}

/*-----------------------------------------------------------------------------
	[VDC_Init]
		VDCを初期化します。
-----------------------------------------------------------------------------*/
void
VDC_Init()
{
	createPatternLUT();

	VDC_SetAutoRasterTiming(2);//Kitao追加
	_AutoOverClock = -1;//Kitao追加
	VDC_SetAutoPerformSpriteLimit(APP_GetDefaultSpriteLimit()); //Kitao追加
	VDC_SetLayer(); //v2.06追加
	_WaitPatch = 0; //Kitao追加
	_bForceRaster = FALSE; //Kitao追加
	_bForceVBlank = FALSE; //Kitao追加
	_bShinMegamiTensei = FALSE; //真・女神転生。v2.20追加
	_bWorldStadium91 = FALSE; //ワールドスタジアム'91。v2.64追加

	VDC_Reset();//Kitao更新
}


/*-----------------------------------------------------------------------------
	[drawBgLine]
		ＢＧを１ライン描画します。

	[DEV NOTES]

	キャラクタサイズ： 8x8
	キャラクタジェネレータに定義できるキャラクタの最大数：４０９６
	ＢＡＴは１６ビット／キャラクタで定義する。
	D0	- D11 ： キャラクタコード
	D12 - D15 ： ＣＧカラー(パレットの上位４ビット)
	[注] キャラクタコードが１２ビットなので，最大キャラクタ数＝４０９６個

	VRAM 中の BAT 領域の大きさは，メモリ幅 MWR で決定されるものと思われる。
	例えば MWR で WxH=32x32 と設定された場合は，BAT は32x32x2 = 2048 バイト
	となる。つまり VRAM $0000 - $07FF は BAT 領域となる。パターンが定義
	される領域(ＣＧアドレス)は，BAT 領域の直後($800)から始まる。

	[スキャンラインベースにする]
	スキャンラインが０～２３９で移動するものとする(未確認)。
	タイルは 8x8 なので，スキャンラインによるタイルのＹ方向のオフセットは，

		tileOffsetY = scanline / 8;

	で求まる。よって，スキャンラインで指定されたラインの描画を行なうには，

		BATWord = (VRAM[tileOffsetY*BGW*2] | (VRAM[tileOffsetY*BGW*2+1] << 8))

	からの２バイトを読み出す。この下位１２ビット＊３２で，ＣＧアドレスを
	得る。

		CGAddr = (BATWord & 0xfff) * 32

	ピクセルの上位４ビット(カラーブロック)は，BATWord の上位４ビットで指定される。

		ColorBlock = (BATWord & 0xf000) >> 8

	下位４ビットには，パターンが入るので，ColorBlockは上位４ビットに保持しておく。
	ＣＧデータは，３２バイトから成り，タイルの１ラインを４バイトで定義する。
	つまり，１ピクセルを４ビットで構成する。その詳細は，

		CG[0] の８ビット: line 0 の D0	|  CG[10] の８ビット: line 0 の D2
		CG[1] の８ビット: line 0 の D1	|  CG[11] の８ビット: line 0 の D3
		CG[2] の８ビット: line 1 の D0	|  CG[12] の８ビット: line 1 の D2
		CG[3] の８ビット: line 1 の D1	|  CG[13] の８ビット: line 1 の D3
		CG[4] の８ビット: line 2 の D0	|  CG[14] の８ビット: line 2 の D2
		CG[5] の８ビット: line 2 の D1	|  CG[15] の８ビット: line 2 の D3
		CG[6] の８ビット: line 3 の D0	|  CG[16] の８ビット: line 3 の D2
		CG[7] の８ビット: line 3 の D1	|  CG[17] の８ビット: line 3 の D3
		CG[8] の８ビット: line 4 の D0	|  CG[18] の８ビット: line 4 の D2
		CG[9] の８ビット: line 4 の D1	|  CG[19] の８ビット: line 4 の D3
		CG[A] の８ビット: line 5 の D0	|  CG[1A] の８ビット: line 5 の D2
		CG[B] の８ビット: line 5 の D1	|  CG[1B] の８ビット: line 5 の D3
		CG[C] の８ビット: line 6 の D0	|  CG[1C] の８ビット: line 6 の D2
		CG[D] の８ビット: line 6 の D1	|  CG[1D] の８ビット: line 6 の D3
		CG[E] の８ビット: line 7 の D0	|  CG[1E] の８ビット: line 7 の D2
		CG[F] の８ビット: line 7 の D1	|  CG[1F] の８ビット: line 7 の D3

	となっている。line は８ピクセルで構成され，８ビットの D0 が右端のドット
	になる。

	02/05/2004: [高速化する]

	1. 8bpp 化できないか？

	パターンに４ビット、カラーブロックで４ビットだから８ビット。
	ＣＧカラーはタイルごとに設定可能。
	タイルはＢＡＴに４０９６個設定可能。
	タイルを４０９６個設定した場合、使用する VRAM の領域は、

		4096 * 2 = 8192 [bytes]

	CG は１タイルのパターンで３２バイト消費するため、

		(65536 - 8192) / 32 = 1792 [tiles] 

	1792 のユニークなパターンを定義できる。
	このパターンは１ピクセルあたり４ビットなので使用するパレット領域は１６．
	→できなさそう...

	2. 事前にできることはやってしまう

	BG の描画には次の３つの処理が伴なう。

		a. パターンとＣＧカラーからパレット指標を計算する(エンコード処理)
		b. パレットから GRB333 9 ビットカラーを取得する
		c. GRB333 → RGB555 / RGB565 に変換する

		a. エンコード処理の高速化
			タイルのパターンデータはフレーム毎に変化することが
			あまりない(書き込みよりも読み出しの方が多い)ため、
			VRAM にＣＧデータが書き込まれたときにエンコード処理を行ない、
			保持しておくと以降の読み出しが高速になる。
			ＣＧカラーはＢＡＴで独立に設定されるので、
			エンコード処理ではＣＧデータ(4bits/pixel)のみを処理の対象とする。

			実際にはＣＧは３２バイトから成るため、データが書き込まれるたびに
			エンコード処理を行なうと最大３２回もエンコード処理を行なうことになる(無駄)。
			そこで、このうち異なる値の書き込みが１バイトでもあれば
			「更新フラグ」を立てるようにし、描画直前に更新フラグが立っている
			ものだけエンコード処理を行なう。

			MWR で設定できる仮想スクリーンの最小サイズは 32 x 32。
			このときＣＧデータのためのＶＲＡＭ領域を最も大きく取れて、

				65536 - 32 x 32 x 2 = 1984 [patterns]

			1984 のパターンを定義できる。ＣＧデータ→ 8x8 ピクセルの
			エンコードなので、次のようなテーブルを用意する。

				Uint8 decodedpixel[1984][8*8];

			エンコード後のピクセルデータは４ビットなので、
			上位４ビットは常にゼロになっている。このビットを使わないのは
			もったいないので、8x8 ピクセル配列の最初の要素
			(decodedpixel[xxxx][0]) の最上位ビットには「更新フラグ」
			を割り当てることにする。なお、ここに立てる「更新フラグ」は、
			描画の直前にエンコード処理がなされてクリアされるので
			ＣＧカラーと干渉することはない。

		b. GRB333 取得の高速化
		c. GRB333 → RGB555 / RGB565 変換の高速化
			b と c は同時に処理してしまう。
			タイルデータを tile (0 <= tile <= 255),
			パレットを palette[] (9 bits),
			GRB333 → RGB555 / RGB565 変換テーブルを table (16-bits)
			とすると、実際に描画するピクセル pixel は、

				pixel = table[palette[tile]];

			で得られる。この table と palette の２重参照を１つに減らすことは可能。
			palette を GRB333 を返すテーブルから RGB555 / RGB565 を返すテーブル
			にするだけでよい。
-----------------------------------------------------------------------------*/
//Kitao更新v1.11。ここでVCEパレットを直接書くようにした。
static void
drawBgLine(
	Uint32	lineCounter)
{
	Uint32		vramAddr;
	Uint32		vramData;
	Uint32		offsetX;		// 画面内タイルのXオフセット 
	Uint32		offsetXmask;	// v1.65追加
	Uint32		tileY8;			// タイル内のＹ座標(0-7)*8。Kitao更新。高速化。v1.08。
	Uint32		paletteSelect;	// 32ビット型にしたほうがだいぶ速かった。v1.21
	Uint8*		pSrc;
	Sint32		width;
	Sint32		x = - (BXR & 7); //v1.11更新

	//HDWの値が小さすぎる場合、左側を黒帯で埋める。ファイナルブラスターで必要。v2.14
	if ((_TvMax <= 85)&&(VCR <= 4)) //85(256/3。実機未確認)。TRAVELエプルのOP，スターブレーカーの「アレムの町」で一泊するとき(_TvMax=112)、(_VPRが大きめのとき)にはこの処理をしない。v2.49更新
	{
		width = HDS - 2;
		while (width-- > 0)
		{
			_BgPutBuf[_VN][x] = 0;  _pScreenBuf[x] = 0;
			_BgPutBuf[_VN][++x] = 0;  _pScreenBuf[x] = 0;
			_BgPutBuf[_VN][++x] = 0;  _pScreenBuf[x] = 0;
			_BgPutBuf[_VN][++x] = 0;  _pScreenBuf[x] = 0;
			_BgPutBuf[_VN][++x] = 0;  _pScreenBuf[x] = 0;
			_BgPutBuf[_VN][++x] = 0;  _pScreenBuf[x] = 0;
			_BgPutBuf[_VN][++x] = 0;  _pScreenBuf[x] = 0;
			_BgPutBuf[_VN][++x] = 0;  _pScreenBuf[x] = 0;  x++;
		}
	}

	offsetXmask = _BGW[_VN] - 1; //v1.65追加。高速化用
	offsetX = ((_TvStartPos + BXR) >> 3) & offsetXmask;
	tileY8 = (lineCounter & 7) << 3;
	vramAddr = ((lineCounter>>3)&(_BGH[_VN]-1)) * _BGW[_VN]; //画面内タイルのＹオフセット*_BGW[_VN]。v1.08更新

	width = _TvMax >> 3;

	//v1.11追加。高速化のため最初の１ブロック（xが0以上かどうかの判定をする）は別処理。
	vramData = _pwVideoRam[_VN][vramAddr + offsetX]; //vramData = hi4color | pattern address
	paletteSelect = (vramData & 0xF000) >> 8;
	vramData &= 0x7FF;
	if (_BgTileCache[_VN][vramData][0] & 0x20) //「CGパターンが変更された」= bit5が立っていたら、タイルキャッシュを更新する。
		update_bg_tile_cache(vramData);
	pSrc = &_BgTileCache[_VN][vramData][tileY8];
	if (x >= 0)	{ _BgPutBuf[_VN][x] = *pSrc++;  _pScreenBuf[x] = _PaletteBG[paletteSelect | _BgPutBuf[_VN][x]]; }
	else { pSrc++; }
	if (++x >= 0) { _BgPutBuf[_VN][x] = *pSrc++;  _pScreenBuf[x] = _PaletteBG[paletteSelect | _BgPutBuf[_VN][x]]; }
	else { pSrc++; }
	if (++x >= 0) { _BgPutBuf[_VN][x] = *pSrc++;  _pScreenBuf[x] = _PaletteBG[paletteSelect | _BgPutBuf[_VN][x]]; }
	else { pSrc++; }
	if (++x >= 0) { _BgPutBuf[_VN][x] = *pSrc++;  _pScreenBuf[x] = _PaletteBG[paletteSelect | _BgPutBuf[_VN][x]]; }
	else { pSrc++; }
	if (++x >= 0) { _BgPutBuf[_VN][x] = *pSrc++;  _pScreenBuf[x] = _PaletteBG[paletteSelect | _BgPutBuf[_VN][x]]; }
	else { pSrc++; }
	if (++x >= 0) { _BgPutBuf[_VN][x] = *pSrc++;  _pScreenBuf[x] = _PaletteBG[paletteSelect | _BgPutBuf[_VN][x]]; }
	else { pSrc++; }
	if (++x >= 0) { _BgPutBuf[_VN][x] = *pSrc++;  _pScreenBuf[x] = _PaletteBG[paletteSelect | _BgPutBuf[_VN][x]]; }
	else { pSrc++; }
	if (++x >= 0) { _BgPutBuf[_VN][x] = *pSrc++;  _pScreenBuf[x] = _PaletteBG[paletteSelect | _BgPutBuf[_VN][x]]; }
	else { pSrc++; }
	x++;
	offsetX++;
	offsetX &= offsetXmask;
	width--;

	while (width-- > 0)
	{
		// vramData = hi4color | pattern address
		vramData = _pwVideoRam[_VN][vramAddr + offsetX];
		paletteSelect = (vramData & 0xF000) >> 8;
		vramData &= 0x7FF;
		if (_BgTileCache[_VN][vramData][0] & 0x20) //「ＣＧパターンが変更された」= bit5が立っていたら、タイルキャッシュを更新する。
			update_bg_tile_cache(vramData);
		pSrc = &_BgTileCache[_VN][vramData][tileY8];
		_BgPutBuf[_VN][x] = *pSrc++;  _pScreenBuf[x] = _PaletteBG[paletteSelect | _BgPutBuf[_VN][x]];
		_BgPutBuf[_VN][++x] = *pSrc++;  _pScreenBuf[x] = _PaletteBG[paletteSelect | _BgPutBuf[_VN][x]];
		_BgPutBuf[_VN][++x] = *pSrc++;  _pScreenBuf[x] = _PaletteBG[paletteSelect | _BgPutBuf[_VN][x]];
		_BgPutBuf[_VN][++x] = *pSrc++;  _pScreenBuf[x] = _PaletteBG[paletteSelect | _BgPutBuf[_VN][x]];
		_BgPutBuf[_VN][++x] = *pSrc++;  _pScreenBuf[x] = _PaletteBG[paletteSelect | _BgPutBuf[_VN][x]];
		_BgPutBuf[_VN][++x] = *pSrc++;  _pScreenBuf[x] = _PaletteBG[paletteSelect | _BgPutBuf[_VN][x]];
		_BgPutBuf[_VN][++x] = *pSrc++;  _pScreenBuf[x] = _PaletteBG[paletteSelect | _BgPutBuf[_VN][x]];
		_BgPutBuf[_VN][++x] = *pSrc++;  _pScreenBuf[x] = _PaletteBG[paletteSelect | _BgPutBuf[_VN][x]];  x++;
		offsetX++;
		offsetX &= offsetXmask;
	}

	//v1.11追加。高速化のため最後の１ブロック（xがScreenWより小さいかかどうかの判定をする）は別処理。
	if (BXR & 7)
	{
		vramData = _pwVideoRam[_VN][vramAddr + offsetX]; //vramData = hi4color | pattern address
		paletteSelect = (vramData & 0xF000) >> 8;
		vramData &= 0x7FF;
		if (_BgTileCache[_VN][vramData][0] & 0x20) //「ＣＧパターンが変更された」= bit5が立っていたら、タイルキャッシュを更新する。
			update_bg_tile_cache(vramData);
		pSrc = &_BgTileCache[_VN][vramData][tileY8];
		if (x < _TvMax) { _BgPutBuf[_VN][x] = *pSrc++;  _pScreenBuf[x] = _PaletteBG[paletteSelect | _BgPutBuf[_VN][x]]; }
		if (++x < _TvMax) { _BgPutBuf[_VN][x] = *pSrc++;  _pScreenBuf[x] = _PaletteBG[paletteSelect | _BgPutBuf[_VN][x]]; }
		if (++x < _TvMax) { _BgPutBuf[_VN][x] = *pSrc++;  _pScreenBuf[x] = _PaletteBG[paletteSelect | _BgPutBuf[_VN][x]]; }
		if (++x < _TvMax) { _BgPutBuf[_VN][x] = *pSrc++;  _pScreenBuf[x] = _PaletteBG[paletteSelect | _BgPutBuf[_VN][x]]; }
		if (++x < _TvMax) { _BgPutBuf[_VN][x] = *pSrc++;  _pScreenBuf[x] = _PaletteBG[paletteSelect | _BgPutBuf[_VN][x]]; }
		if (++x < _TvMax) { _BgPutBuf[_VN][x] = *pSrc++;  _pScreenBuf[x] = _PaletteBG[paletteSelect | _BgPutBuf[_VN][x]]; }
		if (++x < _TvMax) { _BgPutBuf[_VN][x] = *pSrc++;  _pScreenBuf[x] = _PaletteBG[paletteSelect | _BgPutBuf[_VN][x]]; }
		if (++x < _TvMax) { _BgPutBuf[_VN][x] = *pSrc++;  _pScreenBuf[x] = _PaletteBG[paletteSelect | _BgPutBuf[_VN][x]]; }
	}

	//HDWの値が小さすぎる場合、右側を黒帯で埋める。ファイナルブラスターで必要。v2.14
	if ((_TvMax <= 85)&&(VCR <= 4)) //85(256/3。実機未確認)。TRAVELエプルのOP，スターブレーカーの「アレムの町」で一泊するとき(_TvMax=112)、(_VPRが大きめのとき)にはこの処理をしない。v2.49更新
	{
		width = ((_TvWidth-_TvMax) >> 3) - (HDS-2);
		while (width-- > 0)
		{
			_BgPutBuf[_VN][x] = 0;  _pScreenBuf[x] = 0;
			_BgPutBuf[_VN][++x] = 0;  _pScreenBuf[x] = 0;
			_BgPutBuf[_VN][++x] = 0;  _pScreenBuf[x] = 0;
			_BgPutBuf[_VN][++x] = 0;  _pScreenBuf[x] = 0;
			_BgPutBuf[_VN][++x] = 0;  _pScreenBuf[x] = 0;
			_BgPutBuf[_VN][++x] = 0;  _pScreenBuf[x] = 0;
			_BgPutBuf[_VN][++x] = 0;  _pScreenBuf[x] = 0;
			_BgPutBuf[_VN][++x] = 0;  _pScreenBuf[x] = 0;  x++;
		}
	}

}


// スプライト表示サブ(スプライト優先用)
//   戻り値…スプライト同士のオーバーラップがあったらTRUE。
// Kitao更新。スプライト優先時とBG優先時、それぞれでサブを分けて高速化した。v1.08
//			  戻り値を廃止して高速化。v1.65
static inline void
draw_sp_line_normal_s(
	Uint32	spno,			// スプライトナンバー。v1.65追加
	Uint32	pc,				// パターン番号 
	Uint32	mode2bit, 		// 0x80(bit7)を立てて呼ぶと2bitパターンモード。bit0にはスプライトインデックスの最下位ビットを入れて呼ぶ。v1.65更新
	Uint32	paletteSelect,	// カラーパレットインデックスの上位４ビット 
	Sint32	x,				// スキャンライン上のＸ位置 
	Uint32	tileY)			// タイル上のＹ座標 
{
	int			a,i,w;
	Uint32		data;

	x -= _TvStartPos; //v1.11追加
	a = tileY << 4; //Kitao追加。高速化
	w = x + 15;
	if (w >= _TvMax)
		w = _TvMax - 1; //v1.65更新。高速化
	if ((_TvMax <= 85)&&(VCR <= 4)) //v2.14追加。v2.49更新
	{
		x += (HDS - 2) * 8;
		w += (HDS - 2) * 8;
	}
	if (x < 0)
	{
		a += -x;
		x = 0;
	} //v1.65更新。高速化

	for (i=x; i<=w; i++)
	{
		if (mode2bit) //2bitパターンモードの場合。v1.13追加。Kiさんより資料情報をいただきました。"ファイティングラン"で必要。
		{
			if (mode2bit & 1)
				data = _SpTileCache[_VN][pc][a++] >> 2;
			else
				data = _SpTileCache[_VN][pc][a++] & 0x3;
		}
		else //通常のモード
			data = _SpTileCache[_VN][pc][a++];
		if (data & 0xF) //有効なピクセルがあれば
		{
			if (!_SpPutBuf[_VN][i])
			{
				// スプライトがBGの手前に設定されていた場合は、 
				// より優先度の高いスプライトのピクセルが無いときだけ 
				// ピクセルを描くことができる。 
				_pScreenBuf[i] = _PaletteSP[paletteSelect | data]; //v1.11更新。ここでVCEパレットを直接書くようにした。
				if (spno == 0)
					_SpPutBuf[_VN][i] = 0x81; //v1.65追加。スプライトナンバー0のドットがあることを退避。スプライトの衝突判定は"スプライトナンバー0"と"その他のスプライト"との衝突を判定するようにした。v1.65
				else
					_SpPutBuf[_VN][i] = 1;
			}
			else //すでに若い番号のスプライトドットが表示されていた場合
			{
				if (_SpPutBuf[_VN][i] & 0x80) //v1.65追加。スプライトナンバー0のドットがあった場合、衝突とする。
					_bOverlap[_VN] = TRUE; //スプライトの衝突
			}
		}
	}
}

// スプライト表示サブ(BG優先用)
//   戻り値…スプライト同士のオーバーラップがあったらTRUE。
// Kitao追加。スプライト優先時とBG優先時、それぞれでサブを分けて高速化した。v1.08
//			  戻り値を廃止して高速化。v1.65
static inline void
draw_sp_line_normal_b(
	Uint32	spno,			// スプライトナンバー。v1.65追加
	Uint32	pc,				// パターン番号 
	Uint32	mode2bit, 		// 0x80を立てて呼ぶと2bitパターンモード。0x01にはスプライトインデックスの最下位ビットを入れて呼ぶ。v1.65更新
	Uint32	paletteSelect,	// カラーパレットインデックスの上位４ビット 
	Sint32	x,				// スキャンライン上のＸ位置 
	Uint32	tileY)			// タイル上のＹ座標 
{
	int			a,i,w;
	Uint32		data;

	x -= _TvStartPos; //v1.11追加
	a = tileY << 4; //Kitao追加。高速化
	w = x + 15;
	if (w >= _TvMax)
		w = _TvMax - 1; //v1.65更新。高速化
	if ((_TvMax <= 85)&&(VCR <= 4)) //v2.14追加。v2.49更新
	{
		x += (HDS - 2) * 8;
		w += (HDS - 2) * 8;
	}
	if (x < 0)
	{
		a += -x;
		x = 0;
	} //v1.65更新。高速化

	for (i=x; i<=w; i++)
	{
		if (mode2bit) //2bitパターンモードの場合。v1.13追加。Kiさんより資料情報をいただきました。"ファイティングラン"で必要。
		{
			if (mode2bit & 1)
				data = _SpTileCache[_VN][pc][a++] >> 2;
			else
				data = _SpTileCache[_VN][pc][a++] & 0x3;
		}
		else //通常のモード
			data = _SpTileCache[_VN][pc][a++];
		if ((data & 0xF) == 0) //有効なピクセルが無いときはスキップする 
			continue;
		// スプライトがBGの背後の場合は 
		// BGの指標を残してMSBだけ1にセットする。 
		if (_BgPutBuf[_VN][i]) //BGのピクセルがあった
		{
			//v1.65更新。BGに隠れている場合、スプライトの衝突判定は行わないようにした。(実機未確認)
			//if (_SpPutBuf[_VN][i] & 0x80) //v1.65更新。スプライトナンバー0のドットがあった場合、衝突とする。
			//	_bOverlap[_VN] = TRUE;
			//if (spno == 0)
			//	_SpPutBuf[_VN][i] = 0x82; //v1.65追加。スプライトナンバー0のドットがあることを退避。スプライトの衝突判定は"スプライトナンバー0"と"その他のスプライト"との衝突を判定するようにした。v1.65
			//else
			_SpPutBuf[_VN][i] = 2; //BGの背後にスプライトが来た場合2に設定。スパグラの処理で必要。v1.11更新
			
		}
		else
		{
			if (!_SpPutBuf[_VN][i])
			{
				// スプライトがBGの背後に設定されていても、 
				// そこにBGもしくは優先度のより高いスプライトのピクセルが無い場合は 
				// ピクセルを描くことができる。 
				_pScreenBuf[i] = _PaletteSP[paletteSelect | data]; //v1.11更新。ここでVCEパレットを直接書くようにした。
				if (spno == 0)
					_SpPutBuf[_VN][i] = 0x81; //v1.65追加。スプライトナンバー0のドットがあることを退避。スプライトの衝突判定は"スプライトナンバー0"と"その他のスプライト"との衝突を判定するようにした。v1.65
				else
					_SpPutBuf[_VN][i] = 1;
			}
			else //すでに若い番号のスプライトドットが表示されていた場合
			{
				if (_SpPutBuf[_VN][i] & 0x80) //v1.65追加。スプライトナンバー0のドットがあった場合、衝突とする。
					_bOverlap[_VN] = TRUE; //スプライトの衝突
			}
		}
	}
}

// スプライト表示サブ(左右反転。スプライト優先用)
//   戻り値…スプライト同士のオーバーラップがあったらTRUE。
// Kitao更新。スプライト優先時とBG優先時、それぞれでサブを分けて高速化した。v1.08
//			  戻り値を廃止して高速化。v1.65
static inline void
draw_sp_line_hflip_s(
	Uint32	spno,			// スプライトナンバー。v1.65追加
	Uint32	pc,				// パターン番号 
	Uint32	mode2bit, 		// 0x80を立てて呼ぶと2bitパターンモード。0x01にはスプライトインデックスの最下位ビットを入れて呼ぶ。v1.65更新
	Uint32	paletteSelect,	// カラーパレットインデックスの上位４ビット 
	Sint32	x,				// スキャンライン上のＸ位置 
	Uint32	tileY)			// タイル上のＹ座標 
{
	int			a,i,w;
	Uint32		data;

	x -= _TvStartPos; //v1.11追加
	a = (tileY << 4) + 15; //Kitao追加。高速化
	w = x + 15;
	if (w >= _TvMax)
		w = _TvMax - 1; //v1.65更新。高速化
	if ((_TvMax <= 85)&&(VCR <= 4)) //v2.14追加。v2.49更新
	{
		x += (HDS - 2) * 8;
		w += (HDS - 2) * 8;
	}
	if (x < 0)
	{
		a -= -x;
		x = 0;
	} //v1.65更新。高速化

	for (i=x; i<=w; i++)
	{
		if (mode2bit) //2bitパターンモードの場合。v1.13追加。Kiさんより資料情報をいただきました。"ファイティングラン"で必要。
		{
			if (mode2bit & 1)
				data = _SpTileCache[_VN][pc][a--] >> 2;
			else
				data = _SpTileCache[_VN][pc][a--] & 0x3;
		}
		else //通常のモード
			data = _SpTileCache[_VN][pc][a--];
		if ((data & 0xF) == 0) //有効なピクセルが無いときはスキップする 
			continue;
		if (!_SpPutBuf[_VN][i])
		{
			_pScreenBuf[i] = _PaletteSP[paletteSelect | data]; //v1.11更新。ここでVCEパレットを直接書くようにした。
			if (spno == 0)
				_SpPutBuf[_VN][i] = 0x81; //v1.65追加。スプライトナンバー0のドットがあることを退避。スプライトの衝突判定は"スプライトナンバー0"と"その他のスプライト"との衝突を判定するようにした。v1.65
			else
				_SpPutBuf[_VN][i] = 1;
		}
		else //すでに若い番号のスプライトドットが表示されていた場合
		{
			if (_SpPutBuf[_VN][i] & 0x80) //v1.65追加。スプライトナンバー0のドットがあった場合、衝突とする。
				_bOverlap[_VN] = TRUE; //スプライトの衝突
		}
	}
}

// スプライト表示サブ(左右反転。BG優先用)
//   戻り値…スプライト同士のオーバーラップがあったらTRUE。
// Kitao更新。スプライト優先時とBG優先時、それぞれでサブを分けて高速化した。v1.08
//			  戻り値を廃止して高速化。v1.65
static inline void
draw_sp_line_hflip_b(
	Uint32	spno,			// スプライトナンバー。v1.65追加
	Uint32	pc,				// パターン番号 
	Uint32	mode2bit, 		// 0x80を立てて呼ぶと2bitパターンモード。0x01にはスプライトインデックスの最下位ビットを入れて呼ぶ。v1.65更新
	Uint32	paletteSelect,	// カラーパレットインデックスの上位４ビット 
	Sint32	x,				// スキャンライン上のＸ位置 
	Uint32	tileY)			// タイル上のＹ座標 
{
	int			a,i,w;
	Uint32		data;

	x -= _TvStartPos; //v1.11追加
	a = (tileY << 4) + 15; //Kitao追加。高速化
	w = x + 15;
	if (w >= _TvMax)
		w = _TvMax - 1; //v1.65更新。高速化
	if ((_TvMax <= 85)&&(VCR <= 4)) //v2.14追加。v2.49更新
	{
		x += (HDS - 2) * 8;
		w += (HDS - 2) * 8;
	}
	if (x < 0)
	{
		a -= -x;
		x = 0;
	} //v1.65更新。高速化

	for (i=x; i<=w; i++)
	{
		if (mode2bit) //2bitパターンモードの場合。v1.13追加。Kiさんより資料情報をいただきました。"ファイティングラン"で必要。
		{
			if (mode2bit & 1)
				data = _SpTileCache[_VN][pc][a--] >> 2;
			else
				data = _SpTileCache[_VN][pc][a--] & 0x3;
		}
		else //通常のモード
			data = _SpTileCache[_VN][pc][a--];
		if ((data & 0xF) == 0) //有効なピクセルが無いときはスキップする 
			continue;
		if (_BgPutBuf[_VN][i]) //BGのピクセルがあった
		{
			//v1.65更新。BGに隠れている場合、スプライトの衝突判定は行わないようにした。(実機未確認)
			//if (_SpPutBuf[_VN][i] & 0x80) //v1.65更新。スプライトナンバー0のドットがあった場合、衝突とする。
			//	_bOverlap[_VN] = TRUE;
			//if (spno == 0)
			//	_SpPutBuf[_VN][i] = 0x82; //v1.65追加。スプライトナンバー0のドットがあることを退避。スプライトの衝突判定は"スプライトナンバー0"と"その他のスプライト"との衝突を判定するようにした。v1.65
			//else
			_SpPutBuf[_VN][i] = 2; //BGの背後にスプライトが来た場合2に設定。スパグラの処理で必要。v1.11更新
		}
		else
		{
			if (!_SpPutBuf[_VN][i])
			{
				// スプライトがBGの背後に設定されていても、 
				// そこにBGもしくは優先度のより高いスプライトのピクセルが無い場合は 
				// ピクセルを描くことができる。 
				_pScreenBuf[i] = _PaletteSP[paletteSelect | data]; //v1.11更新。ここでVCEパレットを直接書くようにした。
				if (spno == 0)
					_SpPutBuf[_VN][i] = 0x81; //v1.65追加。スプライトナンバー0のドットがあることを退避。スプライトの衝突判定は"スプライトナンバー0"と"その他のスプライト"との衝突を判定するようにした。v1.65
				else
					_SpPutBuf[_VN][i] = 1;
			}
			else //すでに若い番号のスプライトドットが表示されていた場合
			{
				if (_SpPutBuf[_VN][i] & 0x80) //v1.65追加。スプライトナンバー0のドットがあった場合、衝突とする。
					_bOverlap[_VN] = TRUE; //スプライトの衝突
			}
		}
	}
}


/*-----------------------------------------------------------------------------
	[draw_sp_line]

	width = 32 のときは index の D0 がゼロのアドレスからスタートする。


	+---+
	|	|
	+---+

	+---+
	|	|
	+---+
	|	|
	+---+

	+---+
	|	|
	+---+
	|	|
	+---+
	|	|
	+---+
	|	|
	+---+

	+---+---+
	|	|	|
	+---+---+

	+---+---+
	|	|	|
	+---+---+
	|	|	|
	+---+---+

	+---+---+
	|	|	|
	+---+---+
	|	|	|
	+---+---+
	|	|	|
	+---+---+
	|	|	|
	+---+---+

-----------------------------------------------------------------------------*/
//v1.65更新。戻り値を廃止＆衝突関連の中間変数をカットして高速化。
static void
draw_sp_line(
	Sint32	line)	// 現在のラスタカウンタ値
{
	Uint32		i;
	Sint32		x;
	Sint32		y;
	Sint32		width;
	Sint32		height;
	Uint32		tileY;
	Uint32		paletteSelect; //Kitao更新
	Uint32		nSpPerLine = 0;
	Uint32		index;
	Uint32		spFlags; //v1.65追加
	Uint32		mode2bit; //v1.13追加。Kiさんより資料情報をいただきました。"ファイティングラン"で必要。v1.65更新

	for (i = 0; i < NUM_SPRITES; i++)
	{
		y = _SpRam[_VN][(i<<2)+0] & 0x3FF;
		if (line >= y) //Kitao更新。高速化のためheight設定前に第１条件をチェック。v1.07
		{
			spFlags = _SpRam[_VN][(i<<2)+3]; //v1.65。高速化のためここで変数に入れておく。
			height = _HeightTable[(spFlags & 0x3000) >> 12];
			if (line < y + height)
			{
				x = _SpRam[_VN][(i<<2)+1] & 0x3FF; //Kitao更新。v1.08
				width = spFlags & 0x100; //Kitao更新。width==0なら横16サイズ。それ以外なら横32サイズ。高速化。v1.08
				index = (_SpRam[_VN][(i<<2)+2] & 0x7FF) >> 1; //Kitao更新。パターンアドレスの指定はbit0～10までを有効とした。bit10まで広げたことでぽっぷるメイルのオープニングや、遊々人生のエンド画面等でごみが出るのを解消。
				if ((MWR & 0xC) == 0x4) //2bitパターンモードの場合。v1.13追加。Kiさんより資料情報をいただきました。"ファイティングラン"で必要。
					mode2bit = 0x80 | (_SpRam[_VN][(i<<2)+2] & 1); //v1.13追加。v1.65更新
				else
					mode2bit = 0;

				tileY = line - y;

				// width, height, 反転フラグに応じて index 値を修正する 
				nSpPerLine++;
				if (width) //横32サイズなら
				{
					index = (index & ~1) | ((spFlags & 0x800) >> 11); //v1.08更新。(spFlags & 0x800)…左右反転フラグ
					nSpPerLine++; //Kitao追加。横32モードのときは横２枚分と見なされる
				}

				if (nSpPerLine > _MaxSpPerLine[_VN]) //Kitao更新。実機で横に表示できるスプライト数を越えていたら。v2.63更新。7MHzモード(横336）のときは横に14個まで。"R-TYPE CD"のちらつきの多さを再現。
				{
					_bSpOver[_VN] = TRUE;
					if (_bPerformSpriteLimit) //Kitao追加。スプライト欠けを再現するならここで描画終了（パワーリーグ３の打席結果表示などで必要）
					{
						if ((width)&&(nSpPerLine == _MaxSpPerLine[_VN]+1)) //Kitao追加。横32のスプライトが半分だけ欠ける場合
							width = 0; //横16ドット分だけ表示する。風の伝説ザナドゥII(プロローグからRUNボタンを押した画面でタイトル文字の右にゴミ)で必要。
						else
							break; //描画終了
					}
				}

				if (y == 0 || x == 0 || y >= 240+64 || x >= _ScreenW[_VN]+32)
					continue; //画面表示範囲外なら

				x -= 32;

				//v1.65更新。シンプル化＆わずかに高速化
				if (height == 32)
				{
					if (tileY >= 16)
						index = (index | 2) ^ ((spFlags & 0x8000) >> 14); //(spFlags & 0x8000)…上下反転フラグ
					else
						index = (index & ~2) | ((spFlags & 0x8000) >> 14);
				}
				else if (height == 64)
				{
					if (tileY >= 48) //v1.08更新。高速化
						index = ((index | 6) ^ ((spFlags & 0x8000) >> 13)) ^ ((spFlags & 0x8000) >> 14);
					else if (tileY >= 32)
						index = (((index | 4) & ~2) ^ ((spFlags & 0x8000) >> 13)) | ((spFlags & 0x8000) >> 14);
					else if (tileY >= 16)
						index = (((index & ~4) | 2) | ((spFlags & 0x8000) >> 13)) ^ ((spFlags & 0x8000) >> 14);
					else
						index = (index & ~6) | ((spFlags & 0x8000) >> 13) | ((spFlags & 0x8000) >> 14);
				}

				//index &= 0x1FF;
				if (index > 0x1FF) //遊々人生のエンディングで変な車が表示されないようにするために必要。
				{
					//PRINTF("%X",index); //test
					continue;
				}

				tileY &= 15;
				if (spFlags & 0x8000) //上下反転フラグが設定されていた場合。v1.08更新。高速化
					tileY = 15 - tileY;

				paletteSelect = (spFlags & 0xF) << 4;

				if (_SpTileCache[_VN][index][0] & 0x20)
					update_sp_tile_cache(index);
				if (spFlags & 0x800) //左右反転フラグが設定されていた場合。v1.08更新。高速化
				{
					if (spFlags & 0x80) //スプライト優先の場合。v1.08更新。高速化
						draw_sp_line_hflip_s(i, index, mode2bit, paletteSelect, x, tileY);
					else //BG優先の場合
						draw_sp_line_hflip_b(i, index, mode2bit, paletteSelect, x, tileY);
				}
				else
				{
					if (spFlags & 0x80) //スプライト優先の場合。v1.08更新。高速化
						draw_sp_line_normal_s(i, index, mode2bit, paletteSelect, x, tileY);
					else //BG優先の場合
						draw_sp_line_normal_b(i, index, mode2bit, paletteSelect, x, tileY);
				}

				//横32サイズの場合、右側半分も表示
				if ((width)&&(x+16 < _ScreenW[_VN]))
				{
					index ^= 1;
					if (_SpTileCache[_VN][index][0] & 0x20)
						update_sp_tile_cache(index);
					if (spFlags & 0x800) //HFLIPが設定されていた場合。v1.08更新。高速化
					{
						if (spFlags & 0x80) //スプライト優先の場合。v1.08更新。高速化
							draw_sp_line_hflip_s(i, index, mode2bit, paletteSelect, x+16, tileY);
						else //BG優先の場合
							draw_sp_line_hflip_b(i, index, mode2bit, paletteSelect, x+16, tileY);
					}
					else
					{
						if (spFlags & 0x80) //スプライト優先の場合。v1.08更新。高速化
							draw_sp_line_normal_s(i, index, mode2bit, paletteSelect, x+16, tileY);
						else //BG優先の場合
							draw_sp_line_normal_b(i, index, mode2bit, paletteSelect, x+16, tileY);
					}
				}
			}
		}
	}
}


/*-----------------------------------------------------------------------------
	[drawLine]
		BGおよびスプライトを１ライン描画します。
-----------------------------------------------------------------------------*/
static void //Kitao更新。割り込み要求をここで行うようにしたので、戻り値は不要にした。スパグラに対応した。
drawLine(
	Sint32	scanLine)
{
	int		i;

	_VN=0;

	if (_DisplayCounter < DISPLAY_STARTLINE || _DisplayCounter >= DISPLAY_ENDLINE || _bBurstMode[0])
	{
		//Kitao更新。未表示領域（黒帯）はスプライトの透明色で埋められる。
		for (i=0; i<_TvMax; i++)
			_pScreenBuf[i] = _PaletteSP[0];
	}
	else
	{
		//通常は、現ラインのCRの値を反映させる。レインボーアイランドの場合(CRのbit8-9(表示出力選択)が立っている)、１ライン前の値を使う。実機で同様かは未確認。v1.61更新
		//										表示出力選択が立っていても、BGかスプライトのどちらかを表示する場合は、現ラインを使う。そうでないと、スーパーダライアスでゲーム画面最上ラインが非表示になってしまう＆トイレキッズでステータスの境界に自キャラが埋まる。v1.67,v2.39
		if (((CR & 0x100) == 0)||(CR & 0xC0)) //((CR & 0x100) == 0)の条件を外すと、スト２’のザンギエフ面で画面右側のほうにある鎖が１ライン下にハミ出す。
			_prevCR[0] = _Regs[0][VDC_CR];
		
		if ((_prevCR[0] & 0x80)&& //レインボーアイランドで、非表示フラグは１ライン前の値を反映させるようにした。水没シーンが実機と同様になった。v1.12
			(_bBGLayer)) //v2.06追加
				drawBgLine(_LineCounter[0]);
		else
		{
			//BGの透明色で埋められる。レインボーアイランドの水没シーン
			for (i=0; i<_TvMax; i++)
				_pScreenBuf[i] = _PaletteBG[0];
			memset(&_BgPutBuf[0][0], 0, _TvMax); //v1.11追加
			if ((_TvMax <= 85)&&(VCR <= 4)) //v2.14追加。v2.49更新
			{
				for (i=_TvMax; i<_TvWidth; i++)
					_pScreenBuf[i] = _PaletteBG[0];
				memset(&_BgPutBuf[0][0], _TvMax, _TvWidth-_TvMax);
			}
		}
		
		if ((_prevCR[0] & 0x40)&& //レインボーアイランドで、非表示フラグは１ライン前の値を反映させるようにした。水没シーンが実機と同様になった。v1.12
			(_bSpriteLayer)) //v2.06追加
		{
			if ((_TvMax <= 85)&&(VCR <= 4)) //v2.14追加。v2.49更新
				memset(&_SpPutBuf[0][0], 0, _TvWidth);
			else //通常
				memset(&_SpPutBuf[0][0], 0, _TvMax);
			draw_sp_line(_RasterCounter[0]);
		}
	}
	_prevCR[0] = _Regs[0][VDC_CR]; //１ライン前のCR値として保存。v1.21
}

//Kitao追加。スーパーグラフィックス用。v1.11更新
static void
drawLineSuperGrafx(
	Sint32	scanLine)
{
	int			i;
	Uint32*		pScreenBufR;
	Sint32		x;
	Sint32		n;

	//２つ目のVDC（VDC1。描画結果は_LineBufVDC2に格納）
	_VN=1;
	pScreenBufR = _pScreenBuf;
	_pScreenBuf = &_LineBufVDC2[0];
	if (_DisplayCounter < DISPLAY_STARTLINE || _DisplayCounter >= DISPLAY_ENDLINE || _bBurstMode[1])
	{
		//Kitao更新。未表示領域（黒帯）はスプライトの透明色で埋められる。
		for (i=0; i<_TvMax; i++)
			_pScreenBuf[i] = _PaletteSP[0];
		memset(&_BgPutBuf[1][0], 0, _TvMax);
		memset(&_SpPutBuf[1][0], 0, _TvMax); //スプライト色で埋められるが、スプライトを表示しているわけではないので0フィル。大魔界村の起動時のメモリチェック表示で確認。v1.12
	}
	else
	{
		if (((CR & 0x100) == 0)||(CR & 0x40)) //通常は、現ラインのCRの値を反映させる。
			_prevCR[1] = _Regs[1][VDC_CR];
		
		if ((_prevCR[1] & 0x80)&&
			(_bBG2Layer)) //v2.06追加
				drawBgLine(_LineCounter[1]);
		else
		{
			//BGの透明色で埋められる。
			for (i=0; i<_TvMax; i++)
				_pScreenBuf[i] = _PaletteBG[0];
			memset(&_BgPutBuf[1][0], 0, _TvMax);
		}
		
		memset(&_SpPutBuf[1][0], 0, _TvMax);
		if ((_prevCR[1] & 0x40)&&
			(_bSprite2Layer)) //v2.06追加
				draw_sp_line(_RasterCounter[1]);
	}
	_prevCR[1] = _Regs[1][VDC_CR];

	//１つ目のVDC（VDC0。描画結果は_ScreenBufに格納）
	_VN=0;
	_pScreenBuf = pScreenBufR;
	if (_DisplayCounter < DISPLAY_STARTLINE || _DisplayCounter >= DISPLAY_ENDLINE || _bBurstMode[0])
	{
		//Kitao更新。未表示領域（黒帯）はスプライトの透明色で埋められる。
		for (i=0; i<_TvMax; i++)
			_pScreenBuf[i] = _PaletteSP[0];
		memset(&_BgPutBuf[0][0], 0, _TvMax);
		memset(&_SpPutBuf[0][0], 0, _TvMax); //スプライト色で埋められるが、スプライトを表示しているわけではないので0フィル。大魔界村の起動時のメモリチェック表示で確認。v1.12
	}
	else
	{
		if (((CR & 0x100) == 0)||(CR & 0x40)) //通常は、現ラインのCRの値を反映させる。
			_prevCR[0] = _Regs[0][VDC_CR];
		
		if ((_prevCR[0] & 0x80)&&
			(_bBGLayer)) //v2.06追加
				drawBgLine(_LineCounter[0]);
		else
		{
			//BGの透明色で埋められる。
			for (i=0; i<_TvMax; i++)
				_pScreenBuf[i] = _PaletteBG[0];
			memset(&_BgPutBuf[0][0], 0, _TvMax);
		}
		
		memset(&_SpPutBuf[0][0], 0, _TvMax);
		if ((_prevCR[0] & 0x40)&&
			(_bSpriteLayer)) //v2.06追加
				draw_sp_line(_RasterCounter[0]);
	}
	_prevCR[0] = _Regs[0][VDC_CR];

	//ドットごとのVDC優先順位を参照し、最終描画結果を_pScreenBufに収める。
	//Border1<Border2の場合を想定。おそらく通常はこれ。Border1>Border2は使っているソフトがあったら実装するべし。
	x = 0;
	for (i=3; i>=0; i--)
	{
		switch (i)
		{
			case 3: //左端からBorder1までを描画
				n = _VpcBorder1 + 1;
				if (n > _TvMax)
					n = _TvMax;
				break;
			case 2: //現在非使用。Border1>Border2は使っているソフトがあったら実装する。
				continue;
			case 1: //Border1からBorder2までを描画
				n = _VpcBorder2 + 1;
				if (n > _TvMax)
					n = _TvMax;
				break;
			case 0: //Border2から右を描画。通常はこれだけを使うゲームが多い。
				n = _TvMax;
				break;
			default:
				n = 0; //コンパイルエラー防止のための初期化
				break;
		}
		switch (_VpcPriority[i])
		{
			//VDC0だけ表示（下位2ビットが01）
			case 0x1:
			case 0xD:
			case 0x5:
			case 0x9:
				// そのままの状態でOK
				x = n;
				break;
			
			//VDC1だけ表示（下位2ビットが10）
			case 0x2:
			case 0xE:
			case 0x6:
			case 0xA:
				while (x < n)
				{
					_pScreenBuf[x] = _LineBufVDC2[x];
					x++;
				}
				break;
			
			//VDC0が優先でVDC0,VDC1とも表示（下位2ビットが11または00）
			case 0x3: //VDC0が優先
			case 0xF: //0xF…上位2ビットが11のときも00のときと同様にした。
			case 0x0:
			case 0xC:
				while (x < n)
				{
					if (!_BgPutBuf[0][x] && !_SpPutBuf[0][x]) //VDC0が透明色なら
						if (_BgPutBuf[1][x] || _SpPutBuf[1][x]) //VDC1が透明色じゃなければ
							_pScreenBuf[x] = _LineBufVDC2[x];
					x++;
				}
				break;
			case 0x7: //VDC1のスプライトがVDC0のBGより前に出る(VDC0のスプライトよりは後ろ)
			case 0x4:
				while (x < n)
				{
					if (_SpPutBuf[1][x] & 1) //VDC1のドットがスプライト(_SpPutBuf[1][x]==1 or 0x81のとき。2のときは手前にBGが表示されている)なら
					{
						if (!(_SpPutBuf[0][x] & 1)) //VDC0のドットがスプライトじゃなければ。※スプライトがBGの裏に隠れていた場合も、おそらくVDC1のスプライトが優先される。
							_pScreenBuf[x] = _LineBufVDC2[x];
					}
					else if (_BgPutBuf[1][x]) //VDC1のドットがBGなら
					{
						if (!_BgPutBuf[0][x] && !_SpPutBuf[0][x]) //VDC0が透明色なら
							_pScreenBuf[x] = _LineBufVDC2[x];
					}
					x++;
				}
				break;
			case 0xB: //VDC0のスプライトがVDC1のBGに隠れる(VDC1のスプライトよりは前)
			case 0x8:
				while (x < n)
				{
					if (_SpPutBuf[1][x] & 1) //VDC1のドットがスプライト(_SpPutBuf[1][x]==1 or 0x81のとき。2のときは手前にBGが表示されている)なら
					{
						if (!_BgPutBuf[0][x] && !_SpPutBuf[0][x]) //VDC0が透明色なら
							_pScreenBuf[x] = _LineBufVDC2[x];
					}
					else if (_BgPutBuf[1][x]) //VDC1のドットがBGなら
					{
						if ((_SpPutBuf[0][x] & 1)|| //VDC0がスプライトなら裏にBGが隠れていてもVDC1のBGが優先される？
							(!_BgPutBuf[0][x])) //もしくはVDC0がBGじゃなければ
								_pScreenBuf[x] = _LineBufVDC2[x];
					}
					x++;
				}
				break;
		}
	}
}


/*-----------------------------------------------------------------------------
	[VDC_AdvanceClock]
		指定のクロック数だけ VDC の処理を進めます。

	_ScanLine --- TV のスキャンライン
	_DisplayCounter --- VDC 内のラインカウンタ
	_RasterCounter --- ラスタ比較用カウンタ
	_LineCounter --- ラインカウンタというよりは VDC の VRAM アドレス
-----------------------------------------------------------------------------*/
//※現在非使用
void
VDC_AdvanceClock(
	Sint32	clock)
{
	_ClockCounter += clock;

	while (_ClockCounter >= VDC_CYCLESPERLINE)
	{
		_ClockCounter -= VDC_CYCLESPERLINE;
		VDC_AdvanceLine(NULL, 1);
	}
}


//Kitao追加。VRAM-VRAM間のDMA転送中のときの処理。スパグラ以外のソフト用
static inline void
vramDmaCountCheck()
{
	if (_VramDmaCount[0] > 0)
		if (--_VramDmaCount[0] == 0) //転送開始処理が完了した
			if (_Regs[0][VDC_DCR] & 0x2)
			{
				//割り込み要求。転送完了時ではなく、初期転送処理が終わった(転送が始まった)段階ですぐに要求する。
				_VdcStatus[0] |= VDC_STAT_DV;
				INTCTRL_Request(INTCTRL_IRQ1);
			}
}

//Kitao追加。VRAM-VRAM間のDMA転送中のときの処理。スパグラ用
static inline void
vramDmaCountCheckSG()
{
	if (_VramDmaCount[0] > 0)
		if (--_VramDmaCount[0] == 0) //転送開始処理が完了した
			if (_Regs[0][VDC_DCR] & 0x2)
			{
				//割り込み要求。転送完了時ではなく、初期転送処理が終わった(転送が始まった)段階ですぐに要求する。
				_VdcStatus[0] |= VDC_STAT_DV;
				INTCTRL_Request(INTCTRL_IRQ1);
			}
	if (_VramDmaCount[1] > 0)
		if (--_VramDmaCount[1] == 0) //転送開始処理が完了した
			if (_Regs[1][VDC_DCR] & 0x2)
			{
				//割り込み要求。転送完了時ではなく、初期転送処理が終わった(転送が始まった)段階ですぐに要求する。
				_VdcStatus[1] |= VDC_STAT_DV;
				INTCTRL_Request(INTCTRL_IRQ1);
			}
}

//Kitao更新。CPUやその他I/Oを進める処理
static void //ここはinlineにせず、コンパクトにまとめたほうが速い(うちの環境)
cpuAdvance(
	Sint32	clock)
{
	int		i;
	Sint32	drawFrame = _DrawFrame;

	if (APP_CheckRecordingNow())
		drawFrame = 1; //レコード記録中or再生中の場合、ずれを起こさないため曲のテンポの調整は行わず通常と同じ動作にする。v2.15
	switch (drawFrame)
	{
		case 1: //通常
			if (_SuperGrafx == 1)
			{	//スパグラ
				for (i=1; i<=clock; i++) //clockのぶんだけCPUを進める。
				{
					vramDmaCountCheckSG(); //Kiato追加。
					MOUSE_AdvanceClock();//Kitao更新。入力機器はCPU処理の前におこなうようにした。１クロックずつ動かすとカーソルの動きも滑らかに。
					if (gCPU_ClockCount++ >= 0) CPU_ExecuteOperation(); //Kitao更新。CPUクロックを経過させて可能なら命令を実行する。v2.00
																		//１クロックずつ進めると並列動作の精度が高い。音質も上がる。
					CDROM_AdvanceClock();
					APU_AdvanceClock(); //Kitao更新。CDDA発声などのため、CDROM処理が終わってからAPU処理
					TIMER_AdvanceClock(); //Kitao追加。タイマーはCPUの動きに関係なくここで進めるようにした。
					_CpuPastCount++;//Kitao追加。v1.00。どこまでCPUを進めているかがわかるように。
				}
			}
			else //高速化のため、スパグラのときと処理を分けた。
			{	//通常
				for (i=1; i<=clock; i++) //clockのぶんだけCPUを進める。
				{
					vramDmaCountCheck(); //Kiato追加。
					MOUSE_AdvanceClock();//Kitao更新。入力機器はCPU処理の前におこなうようにした。１クロックずつ動かすとカーソルの動きも滑らかに。
					if (gCPU_ClockCount++ >= 0) CPU_ExecuteOperation(); //Kitao更新。CPUクロックを経過させて可能なら命令を実行する。v2.00
																		//１クロックずつ進めると並列動作の精度が高い。音質も上がる。
					CDROM_AdvanceClock();
					APU_AdvanceClock(); //Kitao更新。CDDA発声などのため、CDROM処理が終わってからAPU処理
					TIMER_AdvanceClock(); //Kitao追加。タイマーはCPUの動きに関係なくここで進めるようにした。
					_CpuPastCount++;//Kitao追加。v1.00。どこまでCPUを進めているかがわかるように。
				}
			}
			break;
		case 0: //早回しのスキップフレーム。BGMのテンポを合わせるために、APUとタイマーは進めない。
			if (_SuperGrafx == 1)
			{
				for (i=1; i<=clock; i++)
				{
					vramDmaCountCheckSG();
					MOUSE_AdvanceClock();
					if (gCPU_ClockCount++ >= 0) CPU_ExecuteOperation();
					CDROM_AdvanceClock();
					_CpuPastCount++;
				}
			}
			else //高速化のため、スパグラのときと処理を分けた。
			{
				for (i=1; i<=clock; i++)
				{
					vramDmaCountCheck();
					MOUSE_AdvanceClock();
					if (gCPU_ClockCount++ >= 0) CPU_ExecuteOperation();
					CDROM_AdvanceClock();
					_CpuPastCount++;
				}
			}
			break;
		case 2: //１倍未満の早回し(スローフレーム)用。BGMをスローにさせないために、APUとタイマーを通常の２倍進める。
			if (_SuperGrafx == 1)
			{
				for (i=1; i<=clock; i++) //clockのぶんだけCPUを進める。
				{
					vramDmaCountCheckSG();
					MOUSE_AdvanceClock();
					if (gCPU_ClockCount++ >= 0) CPU_ExecuteOperation();
					CDROM_AdvanceClock();
					APU_AdvanceClock();
					TIMER_AdvanceClock();
					APU_AdvanceClock();
					TIMER_AdvanceClock();
					_CpuPastCount++;
				}
			}
			else //高速化のため、スパグラのときと処理を分けた。
			{
				for (i=1; i<=clock; i++) //clockのぶんだけCPUを進める。
				{
					vramDmaCountCheck();
					MOUSE_AdvanceClock();
					if (gCPU_ClockCount++ >= 0) CPU_ExecuteOperation();
					CDROM_AdvanceClock();
					APU_AdvanceClock();
					TIMER_AdvanceClock();
					APU_AdvanceClock();
					TIMER_AdvanceClock();
					_CpuPastCount++;
				}
			}
			break;
	}
}

/*-----------------------------------------------------------------------------
	[VDC_AdvanceLine]
		１スキャンライン分だけ VDC の処理を進めます。

	_ScanLine --- TV のスキャンライン
	_DisplayCounter --- VDC 内のラインカウンタ
	_RasterCounter --- ラスタ比較用カウンタ
	_LineCounter --- ラインカウンタというよりは VDC の VRAM アドレス
-----------------------------------------------------------------------------*/
//Kitao更新。できるだけシンプルな実装を目指してやってみることにした。
//			 ラスタ割り込み部分の乱れを解消。スト２’等でたまに一瞬画面が乱れる現象も解消した。
//	_DisplayCounterは1から始まるため先にインクリメントして使う。ここ以外にもdrawLine()などからも参照される。
//	_RasterCounterは64から始まる。
//	_LineCounterはBYRから始まる。CPU処理スレッドでも更新される。
//	_ScanLineは0から始まる。ここでしか使われない。
void
VDC_AdvanceLine(
	Uint32*		pScreenBuf,
	Sint32		drawFrame)
{
	int		i;
	int		a;
	Uint16	srcInc;
	Uint16	dstInc;
	Sint32	rasterTimingCount;
	BOOL	bmwr0x40;

	_pScreenBuf = pScreenBuf;
	_DrawFrame = drawFrame;
	_CpuPastCount = 0;

	// 座標をインクリメント
	_DisplayCounter++;

	for (_VN=0; _VN<=_SuperGrafx; _VN++) //スーパーグラフィックスモードならVDC２つぶんを動かす
	{
		//VRAM-SATB間のDMA転送中の場合の処理
		if (_SpDmaCount[_VN] > 0)
		{
			if (--_SpDmaCount[_VN] == 1) //Kitao更新。VBlank開始から約３ライン弱経過。SATBの転送が完了。
			{
			//完了の割り込み要求
				if (DCR & 0x1) //DCR の D1 がセットされているときは完了割り込みを要求する
				{
					_VdcStatus[_VN] |= VDC_STAT_DS; //Kitao追記。(DCR & 0x1)が真のときだけステータスをセットする。"ドロップロックほらホラ"で必要。
					INTCTRL_Request(INTCTRL_IRQ1);
				}
			}
			/* //DMA転送なのでノンウェイトとした。v1.30
			else if (_SpDmaCount[_VN] == 0)
			{
				if (_VN == 0) //スパグラの場合、二重にウェイトしてしまわないようにVDC0のときのみウェイトとする。
				{
					//Kitao更新。SATB転送で消費するCPUクロックを引く。DMA転送なのでほとんど消費しないと仮定。
					gCPU_ClockCount -= 17; //初期処理ぶん17(全くの推定)。
										   //ウェイトが大きすぎると、ZIPANGスタート時にやや乱れ(40でNG)。プライベートアイドルのOPで１フレーム乱れ＆ゲンジ通信あげだまのデモが合わない。
					if (_WaitPatch != 0) //標準実装でどうしてもうまく動かないソフト用。現在非使用。
						gCPU_ClockCount -= _WaitPatch; //CPU Clock Wait
				}
			}
			*/
		}

	 	if (_DisplayCounter == DISPLAY_STARTLINE) //Kitao更新。表示開始ラインピッタリに来たとき
	 	{
			_LineCounter[_VN] = BYR;
			_RasterCounter[_VN] = 64;
			_prevCR[_VN] = CR; //v1.21追加。レインボーアイランドで必要
			
			//v1.63追加。期間中にVBlank割り込みをCPU側が受け取らなかった場合、描画中はVBlank割り込みステータスをクリアし次のフレーム描画後に遅延してVBlankを起こすようにした。
			//			 天使の詩２のビジュアルシーンが正常に。DE・JAで描画中画面が乱れるのが軽減。ヴァリス１など読み込みのタイミングの問題で黒画面フリーズしやすいソフトもおそらく安定するはず。実機が同じ仕組みの動きかは未確認です。
			if (_VdcStatus[_VN] & VDC_STAT_VD)
				if ((VDW == 0xFF)||(VDW < 224)) //VDWが0xFF（天使の詩２，ヴァリス１）か小さいときのみ。この条件がないとファイナルラップツインで乱れ，レディソードの画面の乱れ具合が実機と異なる，コズミックファンタジー４が動かない，IQパニックの宿屋等でフリーズ，ファージアスの邪皇帝のスタートデモで乱れ，ダブルドラゴン２のOPデモで乱れ，チェイスHQでリセット後再スタートでフリーズ。v2.56
				{
					_VdcStatus[_VN] &= ~VDC_STAT_VD;
					_bVDCleared[_VN] = TRUE;
				}
			
			//Kitao追加。この時点でのVBlankLineを保管。この時点での値を利用する。龍虎の拳で画面乱れを解消。v1.19
			_VblankLineTop[_VN] = _VblankLine[_VN];
			//CR,DCRレジスタの状況によって、VBlankラインを調整する。v1.67更新
			if	((_DCC & 4)&& //DCCが0のときは調整しない。ネクロスの要塞のOPデモでナイトの攻撃時に１フレーム乱れないために必要。アマゾンの攻撃時に乱れるのは実機も同様。v1.64
				((CR & 0x100) == 0)) //表示出力選択の下位ビットが立っていた場合は処理しない。この条件がないとレインボーアイランドのHurry!!時に乱れ，スプラッシュレイクで乱れ。
			{
				if (_ScreenH[_VN] >= 240) //240未満のとき調整すると、ドラキュラX最終面,プリンスオブペルシャで乱れ。マジカルチェイスのタイトル画面デモがループする時に音プチノイズが出る。フラッシュハイダース,カードエンジェルスで最下辺乱れ。v1.67
				{
					switch (MWR & 0x70) //マップサイズごとに分けないとダブルドラゴン２(MWR=0x70)で乱れ。
					{
						//縦が64キャラマップの場合(横128除く)
						case 0x50: //マップサイズが64x64の場合。このケースが一番多いので高速化のため先に記述
						case 0x40: //マップサイズが32x64の場合。
							//BGとスプライトの表示具合によって、遅めか早めにVBlankを起こす。
							if ((CR & 0xC0) != 0xC0) //BGかスプライトどちらか非表示の場合
							{
								if (MWR & 0x10) //MWR=0x50の時。MWR=0x50の時に2以上引くと、ゲンジ通信あげだま，メタルエンジェル，F1サーカス'92でNG。v2.61更新
								{
									if (VCR < 4) //VCRが小さい時。お嬢様伝説ユナOPデモ(VCR=0xF6)で引いてしまうとスロー。ゲンジ通信あげだまOPデモ(VCR=4)で引くと合わない。v2.63更新
										_VblankLineTop[_VN]--; //-1以上でレーシング魂(BG非表示)で画面揺れが解消。v2.41更新
								}
								else //MWR=0x40の時
									_VblankLineTop[_VN]--; //-1でサーク１２(MWR=0x40,VCR=3)でステータス表示時の上部乱れが実機と同じに。-1で雀神伝説のステータス表示(MWR=0x240,VCR=0xF6)の乱れが解消。v2.63
							}
							if (DCR & 0x10) //VRAM->SATBへ自動転送処理を行う場合、遅めにVBlankを起こす。v2.52
								if (VCR >= 4) //VCRが標準以上。
									if (MWR & 0x10) //MWR=0x50時のみ。v2.60追加。MWR=0x40のときに遅らせると、メタルエンジェル２のＯＰデモが表示されない。
										if ((CR & 0x02) == 0) //スプライトの表示異常割り込みを行わないときのみ。サーカスライドで蜂が針を落としたときに時々起こる音の乱れが解消（軽減）。v2.64追加
											_VblankLineTop[_VN]++; //+1でゲンジ通信あげだまOPデモOK(MWR=0x50)。メタルエンジェルのタイトル画面乱れ解消(MWR=0x50)。F1サーカス'92で決勝ゴール後乱れ解消(MWR=0x50)。カダッシュで乱れ解消(MWR=0x50)。ワールドコートで音の安定。v2.58
								break;
						//縦が32キャラマップの場合(横128除く)
						case 0x10: //マップサイズが64x32の場合。
						case 0x00: //マップサイズが32x32の場合。
							if (DCR & 0x01) //VRAM->SATBへの転送終了後の割り込み処理を行う場合、遅めにVBlankを起こす。
								_VblankLineTop[_VN]++; //+1でソルジャーブレイド３面での乱れが解消。大きく足しすぎると"クイズ殿様の野望"で画面揺れ。v2.04,v2.36
							if (DCR & 0x10) //VRAM->SATBへ自動転送処理を行う場合、早めにVBlankを起こす。v2.49
								if (VCR >= 4) //VCRが標準以上の場合。これがないとクロスワイバーのOPデモループ(MWR=0x10)でタイミング合わず。
									_VblankLineTop[_VN]--; //-1。-1であすか120%(MWR=0x10)の試合前デモの１フレーム乱れが解消。-2以上引くとファージアスの邪皇帝戦闘シーン(MWR=0x10,VCR=0xF6)で乱れ。v2.50
							break;
					}
				}
				else if (_ScreenH[_VN] >= 224) //224未満のとき調整すると風の伝説ザナドゥ２のOPで下辺乱れ。v2.43
				{
					switch (MWR & 0x70) //マップサイズごとに分けないとダブルドラゴン２(MWR=0x70)で乱れ。
					{
						//縦が64キャラマップの場合(横128除く)
						case 0x50: //マップサイズが64x64の場合。
						case 0x40: //マップサイズが32x64の場合。
							//BGとスプライトの表示具合によって早めにVBlankを起こす。
							if ((CR & 0xC0) == 0x80) //BGだけ表示し、スプライトは表示しない場合、早めにVBlankを起こす。BG非表示時にも引いてしまうと、ドラキュラXの最終面階段上り下りで乱れ。v2.41更新
								_VblankLineTop[_VN] -= 3; //-3で雀探物語３(MWR=0x50)の乱れが解消。-2以上でフラッシュハイダース(MWR=0x50)で対戦決着後１フレームブラックアウトが解消(多く引きすぎてもオーバーレイ時乱れ)。v2.41更新
														  //-2以上で原由子の眠れぬ夜(MWR=0x50)のカーレースの時など(MWR=0x50,ScreenH=224)で１ラインちらつきが解消。
							break;
					}
				}
			}
			if ((_VblankLineTop[_VN] < 1)||(_VblankLineTop[_VN] > 263))
				_VblankLineTop[_VN] = 263;
			//Kitaoテスト用
			//PRINTF("VBtest=%X, %d, %d, %d  [ %X , %X , %X , %X, %X , %X ] VBlineTop=%d",_DCC,_TvStartLine[_VN],DISPLAY_STARTLINE,_ScreenH[_VN], VPR,VDW,VCR,MWR,DCR,CR, _VblankLineTop[_VN]);
		}
		else
		{
			_LineCounter[_VN]++;
			_RasterCounter[_VN]++;
		}
		_LineCounter[_VN] &= _LineCountMask[_VN];
		
		//前回のラスタ割り込みが処理されていなかった場合、ここで再び処理を促す。Ｆ１サーカス'92の決勝後の順位発表で画面を乱さないために必要。v2.39
		if (_VdcStatus[_VN] & VDC_STAT_RR)
			INTCTRL_Request(INTCTRL_IRQ1);

		//ラスタ比較を行なう
		if ((_RasterCounter[_VN] == RCR)&&(_RasterCounter[_VN] >= 64))
		{
			if (CR & VDC_CTRL_RC)
			{
				_VdcStatus[_VN] |= VDC_STAT_RR; //Kitao追記。割り込みオンのときのみステータスはセット。グラディウスで確認。
				INTCTRL_Request(INTCTRL_IRQ1);
			}
			_bRasterRequested[_VN] = TRUE;
		}
		else
			_bRasterRequested[_VN] = FALSE;
	}

	//Kitao追加。ラスタ比較直後にCPUを動かすようにした。こうすることで実機に近いラスタ割り込みの受け取りが出来る。
	_bRasterLate = FALSE;
	rasterTimingCount = _RasterTimingCount;
	if (_RasterTimingType == 2) //RasterTiming設定がNormal(88)の場合
	{	//v1.41追加。割込み処理などを行うかどうかによって、ラスタ割り込みが発生するタイミングが変わる。
		//タイミングの値は推定値。他の要因も絡んで来るので確かな所は不明なのでエミュレータとしてはソフトの動作が全く同じになれば現状はよしとする。v1.61
		if ((_DCC & 4) == 0) //このときは特殊
		{
			if (_Regs[0][VDC_MWR] > 0xFF) //特殊
				rasterTimingCount = 76; //76。パチ夫君十番勝負で76以下が必要。ストライダー飛龍OK。
			else if ((_Regs[0][VDC_MWR] & 0x60) == 0x00) //32x32か64x32キャラのBGマップサイズの場合
			{
				if (_Regs[0][VDC_VCR] >= 4) //VCRが標準(4)以上の場合
					rasterTimingCount = 86; //86。スペースハリアーで86以下必要(大きいとポーズ時上辺ちらつき)。サイバークロスで大きめの値が必要。
				else //VCRの値が小さい場合。v1.61
					rasterTimingCount = 69; //69。熱血高校ドッチボールで小さめの値が必要(小さすぎても駄目。実機でもやや乱れあり)。
			}
			else //BGマップサイズが大きい場合
			{
				if (_TvWidth == 256) //横256モード(5MHz)のとき
					rasterTimingCount = 88; //88。モンスタープロレスで83以上が必要。マニアックプロレスで大きな値はNG。
				else //横336モード(7MHz)以上のとき
					rasterTimingCount = 70; //70。スライムワールドで70以下が必要。
			}
		}
		else if (_Regs[0][VDC_VDW] & 0x100) //特殊
			rasterTimingCount = 84; //84。パズルボーイのコースクリア時に84以下が必要。
		else if (_Regs[0][VDC_VPR] & 0x01) //特殊。ストライダー飛龍(90NG)もこれだが(_DCC==0)を優先させる。v2.17
			rasterTimingCount = 106; //106。ミズバク大冒険の３面ボス(ノコギリ炎汽車みたいなやつ)で106が最適。
		else if (_Regs[0][VDC_DCR] & 0x01) //VRAM->SATB間の転送終了時の割り込みを行う(DCR&0x1)場合、特殊とする。
		{
			if ((_Regs[0][VDC_MWR] & 0x60) == 0x00) //32x32か64x32キャラのBGマップサイズの場合
			{
				if (_Regs[0][VDC_VCR] & 0x80) //VCRの値が大きい場合
					rasterTimingCount = 200; //200。姐(あねさん)の通販画面で揺れないために大きな値が必要(190NG)。
				else
					rasterTimingCount = 90; //90。ソルジャーブレイド(３面)で90以下が必要(大きすぎると橋の上辺がちらつく)。スーパー桃太郎電鉄２のタイトル画面で89以上が必要。
			}
			else //BGマップサイズが大きい場合
				rasterTimingCount = 58; //58。スーパーバレーボール(VCR=4,MWR=0x30)で58以下が必要。ヒット・ジ・アイス(MWR=0x50)で64以下ぐらいが必要。
		}
		else if (_Regs[0][VDC_DCR] & 0x02) //VRAM->VRAM間の転送終了時の割り込みを行う(DCR&0x2)場合、特殊とする。
			rasterTimingCount = 20; //20。マジカルチェイス(MWR=0x50)のお店で小さめの値が必要。
		else if ((_Regs[0][VDC_CR] & 0x42) == 0x42) //スプライト異常の割込みをする場合。スプライト非表示時(0x40が0。超兄貴)は除く
			rasterTimingCount = 90; //90。スターパロジャーのタイトル画面で90以上が必要。チェイスH.Q.で91以下が必要。最後の忍道。
		else if (_TvWidth == 256) //横256モード(5MHz)のとき
		{
			if (_Regs[0][VDC_DCR] == 0x00) //VRAM転送関連の割込み処理などを行わないなら。
			{	//割込み発生よりも描画処理のほうが早く終わる可能性が高くなる。
				if ((_Regs[0][VDC_MWR] & 0x60) == 0x60) //おそらくマップサイズごとにタイミングが異なる
				{	//BGマップサイズが128x64の場合(MWR=0x60,0x70)
					rasterTimingCount = 85; //85。パワーゲイトで85以下が必要。
				}
				else if (_Regs[0][VDC_MWR] & 0x40) //おそらくマップサイズごとにタイミングが異なる
				{	//BGマップサイズが32x64,64x64の場合(MWR=0x40,0x50)
					if (_Regs[0][VDC_VCR] >= 4) //VCRが標準(4)以上の場合
					{
						if ((_Regs[0][VDC_VCR] & 0x80) == 0) //VCRの値が大きくない場合
							rasterTimingCount = 88; //88。21エモンで86以上が必要。ダンジョンエクスプローラーOK(大きめのほうが安定っぽい)。
						else
							rasterTimingCount = 72; //72。リンダキューブ戦闘で72以下が必要。ワールドヒーローズ２(緑のお面の相手)で72辺りが最適。スプリガンmk2の8面開始前デモで78以下が必要。龍虎の拳(横256時)OK。
					}
					else //VCRの値が小さい場合。v2.17更新
						rasterTimingCount = 70; //70。ジャッキーチェン(特に１面後半)で70以下必要。
				}
				else if (_Regs[0][VDC_MWR] & 0x30) //おそらくマップサイズごとにタイミングが異なる
				{	//BGマップサイズが64x32,128x32の場合(MWR=0x10,0x20,0x30)
					if (_Regs[0][VDC_VCR] >= 4) //VCRが標準(4)以上の場合
					{
						if ((_Regs[0][VDC_VCR] & 0x80) == 0) //VCRの値が大きくない場合
							rasterTimingCount = 61; //61。天外魔境ZIRIAのOPデモで61以下が必要。スト２’で小さな値が必要(大きくてもNG)。ファイナルソルジャーで70以下が必要。アドベンチャーアイランドOK。
						else //VCRの値が特殊な場合。CDゲームに多く、ラスタのタイミングも違うようだ。
							rasterTimingCount = 38; //38。天外魔境２の戦闘シーンで背景が揺れないために38以下ぐらいが必要(小さすぎても駄目)。v2.62
					}
					else //VCRの値が小さい場合。v2.31追加
						rasterTimingCount = 144; //144。ラビオレプススペシャル(VCR=3)で144以上が最適(ステータス表示の下に完全に敵がもぐる)。ロードランナー(パックインビデオ版)で大きすぎると良くない。
				}
				else
				{	//BGマップサイズが32x32の場合(MWR=0x00)
					rasterTimingCount = 80; //80。真女神転生パラメータ振り分けで80辺りが最適(大きすぎても小さすぎても駄目)。
				}
			}
			else
			{	//VRAM転送関連の割込み処理などを行うとき
				switch (_Regs[0][VDC_MWR] & 0x70) //おそらくマップサイズごとにタイミングが異なる
				{
					case 0x50: //64x64キャラのBGマップサイズ。主にCDゲームでこのケースが一番多いので高速化のため先に記述
						if (_Regs[0][VDC_VCR] & 0x80) //VCRの値が大きい場合
							rasterTimingCount = 86; //86。//らんま1/2とらわれの花嫁で86以下が必要。
						else
						{
							if (_Regs[0][VDC_CR] & 0x40) //スプライト表示時
								rasterTimingCount = 88; //88。ゼロヨンチャンプで88以上が必要(稀に揺れるが実機も同様)。Ｆ１サーカスのスターティンググリッドで88以下必要。ダンジョンエクスプローラーで83以上が必要。
							else //スプライト非表示時は早めに判定を行う
								rasterTimingCount = 87; //87。エターナルシティ87以下が必要。サンダーブレードで88以下が必要。ダンジョンエクスプローラーで83以上が必要。サイドアームSPのアレンジゲームOK。v2.62
						}
						break;
					case 0x10: //64x32キャラのBGマップサイズ。このケースも多い。
						if (_Regs[0][VDC_VCR] & 0x80) //VCRの値が大きい場合
							rasterTimingCount = 78; //78。ダウンタウン熱血物語のゲームモード選択画面(RUN押した直後の下辺)で78以下必要。
						else
						{
							if (_Regs[0][VDC_VCR] < 4) //VCRの値が小さい場合
								rasterTimingCount = 81; //81。クロスワイバーで81以下が必要。パックランドで81以上が必要。
							else
								rasterTimingCount = 85; //85。ドンドコドンの面クリアスクロール時に85以上必要(84以下だと乱れなく実機と相違)。パワースポーツのNEWSで85以下が必要。ZIPANGタイトル画面で84以上が必要。シュビビンマン２で小さめの値が必要。
														//    サイレントデバッガーズで小さめの値がいい。スプラッターハウスNG(大きいとステージ５中盤(手の出てくるところ)で左下わずかに乱れる。実機でも同じかもしれない。時間のあるときに確認)。
						}
						break;
					case 0x00: //32x32キャラのBGマップサイズ。
						rasterTimingCount = 65; //65。ダウンタウン熱血行進曲大運動会で65以下が最適。オペレーションウルフのスタートデモ(パラシュート落下&フェードアウト時)で66以下が必要。がんばれゴルフボーイズのタイトル画面で65辺りが最適（小さすぎると中央の雲が乱れ。小さくても大きくても乱れる)。
						break;
					case 0x20: //128x32キャラのBGマップサイズ。
					case 0x30: //128x32キャラのBGマップサイズ。
						if (_Regs[0][VDC_VCR] & 0x80) //VCRの値が大きい場合
							rasterTimingCount = 58; //58。聖夜物語の戦闘シーンで58が最適。銀河婦警伝説サファイアOK。
						else
							rasterTimingCount = 83; //83。ワンダーモモで83以下が必要(小さくても駄目)。ちびまるこちゃんOK(大きめの値必要)。ファイナルラップツインOK。
						break;
					case 0x60: //128x64キャラのBGマップサイズ。
					case 0x70: //128x64キャラのBGマップサイズ。
						rasterTimingCount = 70; //70。Ｆ１パイロット(VCR=3)で70辺りが必要(タイヤの上が乱れない辺りが最適。実機でもかなりラスタが乱れる)。飛翔騎兵カイザードで81以下が必要。熱血高校サッカー編OK。
						break;
					case 0x40: //32x64キャラのBGマップサイズ。
						if ((_Regs[0][VDC_VCR] < 4)&&(_VblankLineTop[0] >= 262)) //VCRの値が小さい場合＆VBlankLineTopが262以上の場合(これがないとサーク１２で乱れ)。
							rasterTimingCount = 108; //108。ドラゴンセイバーのゲームオーバー時に108以下必要,３面で112以上が理想。v2.58
						else
							rasterTimingCount = 28; //28。メタルエンジェルで28以下必要(小さすぎても駄目)。アフターバーナー２で56以下必要。サーク１２で小さめの値が必要。
						break;
				}
			}
		}
		else if (_TvWidth == 336) //横336モード(7MHz)のとき
		{	//割込み発生が描画処理の前に起こる可能性が高くなる。
			if (_Regs[0][VDC_DCR] == 0x00) //VRAM転送関連の割込み処理などを行わないなら、通常よりも、先に描画が終わる可能性が高くなる。
			{
				if (_Regs[0][VDC_MWR] & 0x40) //縦64キャラのBGマップサイズの場合
				{
					if (_Regs[0][VDC_VCR] >= 240)
						rasterTimingCount = 87; //87。龍虎の拳(横336時)で87以下が必要。
					else
						rasterTimingCount = 110; //110。プリンスオブペルシャのLV8画面反転のときに110以上(109だと画面切替わり時に稀に乱れ)が必要。ドラゴンスレイヤー英雄伝説で132以下が必要。
				}
				else //縦32キャラのBGマップサイズの場合
					rasterTimingCount = 82; //82。R-TYPE1(VCR=3,MWR=0x10)で82以下が必要。大魔界村３面で大きな値は駄目82OK。
			}
			else
			{	//VRAM転送関連の割込み処理などを行うとき
				if (_Regs[0][VDC_MWR] & 0x40) //縦64キャラのBGマップサイズの場合
				{
					if (_Regs[0][VDC_VCR] & 0x80) //VCRが大きい場合
					{
						if (_Regs[0][VDC_VCR] > 224)
							rasterTimingCount = 88; //88。ぷよぷよＣＤタイトル画面で104以下が必要。
						else //VCRが大きいが224以下の場合
							rasterTimingCount = 134; //134。斬(マップ画面でIIボタン連打)で134以上が必要。
					}
					else
						rasterTimingCount = 88; //88。サイドアーム(Hu)＆サイドアームSPのアーケードモードで86以上が必要。ホラーストーリーで大きいと駄目。v2.19
				}
				else //縦32キャラのBGマップサイズの場合
				{
					if (_Regs[0][VDC_VCR] & 0x80) //VCRが大きい場合
						rasterTimingCount = 130; //130。ぷよぷよＣＤハイスコア画面(VCR=0xEE,MWR=0x2A)で130以上が必要(バックアップRAMのウェイトとも絡むので注意)。BURAIで111以上が必要。
					else
						rasterTimingCount = 88; //88。青いブリンクで106以下が必要。
				}
			}
		}
		else if (_TvWidth == 512) //横512モード以上(10MHz)のとき
		{	//割込み発生が描画処理の前に起こる可能性が高くなる。
			if (_Regs[0][VDC_MWR] & 0x40) //縦64キャラのBGマップサイズの場合
				rasterTimingCount = 132; //132。とりあえず5MHz通常の1.5倍
			else //縦32キャラのBGマップサイズの場合
				rasterTimingCount = 88; //88。あすか120%(校舎画面)で95以下ぐらいが必要。v2.41
		}
	}
	if (_OverClockType >= 100) //ターボサイクルモードの場合
		if ((_RasterTimingType != 1)&&(_RasterTimingType != 4)&&(_RasterTimingType != 14)) //EARLY以外なら
		{
			if (_OverClockType == 100)
				rasterTimingCount = (rasterTimingCount +1) /2; //+2は四捨五入のため。
			else //200,300
				rasterTimingCount = (rasterTimingCount +2) /5; //+2は四捨五入のため。ぽっぷるメイルで/5が必要(/4だとOPデモで上辺が稀に乱れ)。
		}
	//Kitaoテスト用
	//_VN=0; PRINTF("RTtest=%d (%d) %X, %d, %d, %d  [ %X , %X , %X , %X, %X , %X ] %X VB=%d",rasterTimingCount,_TvWidth,_DCC,_TvStartLine[_VN],DISPLAY_STARTLINE,_ScreenH[_VN], VPR,VDW,VCR,MWR,DCR,CR, _VdcStatus[_VN], _VblankLineTop[_VN]);
	//PRINTF("Kekka=%x, %x, %x, %x, %x, %x, %x, %x", _Regs[0][0],_Regs[0][1],_Regs[0][2],_Regs[0][3],_Regs[0][4],_Regs[0][5],_Regs[0][6],_Regs[0][7]);
	//PRINTF("Kekka=%x, %x, %x, %x, %x, %x, %x, %x", _Regs[0][8],_Regs[0][9],_Regs[0][10],_Regs[0][11],_Regs[0][12],_Regs[0][13],_Regs[0][14],_Regs[0][15]);
	//PRINTF("Kekka=%x, %x, %x, %x, %x, %x, %x, %x", _Regs[0][16],_Regs[0][17],_Regs[0][18],_Regs[0][19],_Regs[0][20],_Regs[0][21],_Regs[0][22],_Regs[0][23]);
	//PRINTF("Kekka=%x, %x, %x, %x, %x, %x, %x, %x", _Regs[0][24],_Regs[0][25],_Regs[0][26],_Regs[0][27],_Regs[0][28],_Regs[0][29],_Regs[0][30],_Regs[0][31]);

	cpuAdvance(rasterTimingCount); //rasterTimingCountのパワーぶんだけCPUを進める。ラスタ割り込み後、すぐに書き換えを行うゲームの場合、ここで_LineCounter等が書き換えられる。

	//Kitao追記。まず、rasterTimingCountのパワーぶんだけCPUを進めた。
	//			 現在のラインをすぐに書き換えたいゲームの場合は、この時点でCPUによって_LineCounter等が書き換えられている。

	if (_ScanLine == 0) //スキャンラインが先頭
	{
		//_VN=0; PRINTF("WaitTest=%d (%d) %X, %d, %d, %d  [ %X , %X , %X , %X, %X , %X ] %X VB=%d",rasterTimingCount,_TvWidth,_DCC,_TvStartLine[_VN],DISPLAY_STARTLINE,_ScreenH[_VN], VPR,VDW,VCR,MWR,DCR,CR, _VdcStatus[_VN], _VblankLineTop[_VN]);
		for (_VN=0; _VN<=_SuperGrafx; _VN++) //スーパーグラフィックスモードならVDC２つぶんを動かす
		{
			//スキャンライン先頭時のVSWを保管しておく。v2.09追加
			_PrevVSW[_VN] = VSW;
			
			//バーストモード判定。Kitao更新。スキャンラインが0ラインのときに判定するようにした。このラインで非表示状態なら残りのラインもすべて非表示(スプライト透明カラーで埋まる)。お嬢様伝説ユナ２、サイバークロス。
			//								 ラスタ割り込み判定が終わったあとに、バーストモード判定をすることで、レインボーアイランドのHurryUpでの画面乱れを安定して解消。v1.19
			_bBurstMode[_VN] = ((CR & 0xC0) == 0);

			//特定の設定時に、VDCアクセス時のウェイトをカット。(実機でも同様かは未確認)。高速化のためここで判定しておく。v2.17,v2.31,v2.38,v2.41更新
			if ((((DCR & 0x12) == 0x10)&&(CR & 0x80))|| //SATBへの自動転送を行うとき(＆終了時割り込みオフ時(オン時にノーウェイトだと、マジカルチェイス(DCR=0x12,VCR=6,MWR=0x50)DEMO中にノイズ)＆かつ、＆BG表示時の場合は、ノーウェイト動作にする。ワールドスタジアム，アルガノス，あげだまで必要。v2.63更新
				(MWR & 0x20)|| //BGマップの横サイズが128のときはウェイトをカット。ワンダーモモ(MWR=0x30)で必要。
				((MWR & 0x70) == 0x40)|| //BGマップサイズが32x64のときはウェイトをカット。メタルエンジェル自己紹介後(MWR=0x40)でノーウェイトが必要。v2.63
				(((MWR & 0x70) == 0x50)&&(VCR < 4))) //BGマップサイズが64x64で、VCRが小さいときはウェイトをカット。高橋名人の新冒険島のダンスシーン(MWR=0x50)開始時にノイズが出ないためにノーウェイトが必要。v2.63
					_bVDCAccessWait[_VN] = FALSE; //アルガノス(DCR=0x10,MWR=0x50,VCR=4,CR=0xCC)の対戦モードでノーウェイトが必要。v2.41更新
												  //シティハンター(MWR=0x10,VCR=3)でノーウェイトが必要。ウェイトを入れると、電源投入直後の画面で下辺がたまに乱れ。
												  //ワールドスタジアム(DCR=0x10,MWR=0x50,VCR=3,CR=0xC8)のミート時にノーウェイトが必要。ゲンジ通信あげだまのデモでノーウェイトが必要。v2.63
			else
				_bVDCAccessWait[_VN] = TRUE; //ボンバーマン'93(DCR=0,MWR=0x00,VCR=4)，サイレントデバッガーズのスタート後の銃を撃つシーン(DCR=0x10,MWR=0x10,VCR=4,CR=0x00?)でウェイトが必要。
											 //パックランド(MWR=0x10,VCR=3,CR=0)で面開始時に１フレーム乱れないために必要。
											 //スターブレイカーの街や洞窟を出た際にフリーズしないために、ウェイトが必要。メタルエンジェル(DCR=0x10,MWR=0x50,VCR=4,CR=0x08?)のスタートデモ後にフリーズしないためにウェイトが必要。v2.63更新
			if ((_DCC & 4) == 0) //DCCのbit2がクリアのときは必ずウェイトを入れる。これがないと、妖怪道中記(MWR=0x30,VCR=3)の賭博で乱れ。
				_bVDCAccessWait[_VN] = TRUE;
			if (_bWorldStadium91)
				_bVDCAccessWait[_VN] = FALSE; //盗塁の画面切り替え時に１フレーム乱れる現象（実機でも起こるが、スポーツゲームなのでより快適動作を優先）を解消。v2.64
		}
	}

	//１ライン描画
	if (pScreenBuf != NULL)
	{
		if (_SuperGrafx == 0)
			drawLine(_ScanLine);
		else
			drawLineSuperGrafx(_ScanLine); //Kitao追加。スーパーグラフィック用。
		//drawLine(),drawLineSuperGrafx()で、_bOverlap[_VN]と_bSpOver[_VN]が設定される。v1.65更新
		for (_VN=0; _VN<=_SuperGrafx; _VN++) //スーパーグラフィックスモードならVDC２つぶんを動かす
		{
			//Kitao更新。スプライトの衝突割り込み要求はここで行うようにした。参考：v0.74でVBlank期間で行うようにしたら超兄貴で不具合。ここが良好。
			if (_bOverlap[_VN])
			{
				_bOverlap[_VN] = FALSE;
				if (CR & VDC_CTRL_CC)
				{
					_VdcStatus[_VN] |= VDC_STAT_CR;
					INTCTRL_Request(INTCTRL_IRQ1);
				}
			}
			//Kitao更新。スプライトオーバーの割り込み要求はここで行うようにした。
			if (_bSpOver[_VN])
			{
				_bSpOver[_VN] = FALSE;
				if (CR & VDC_CTRL_OC)
				{
					_VdcStatus[_VN] |= VDC_STAT_OR;
					INTCTRL_Request(INTCTRL_IRQ1);
				}
			}
		}
	}
	_bRasterLate = TRUE; //ここから後でBYRの書き換えが起こった場合は、そのときに_LineCounterをインクリメントしない。

	//Vblank処理。Kitao追記。Vblank処理はここ（１ライン描画直後でCPUパワー使用前）が良いようだ。パワーテニス、ソルジャーブレイド、スーパーダライアス等で確認。
	//			  vblank直後に多くのCPUパワーが必要（vblank割り込み要求を確実にCPUへ伝えるため）。パワーテニスのタイトル画面で確認。
	_ScanLine++;
	if ((_ScanLine == _VblankLineTop[0])||(_ScanLine == _VblankLineTop[_SuperGrafx])) //VBlankラインに来た
	{
		MOUSE_UpdateDelta(); //Kitao追加。ここでマウスの座標をアップデートする。マウス座標が参照される(ことが多い)直前になる、この位置が良さそう。v1.23
		
		//v0.81追加。VBlank割り込み前に１ラインの途中までCPUパワーを使う(VBlank割り込み要求を遅らせる)。実機のタイミングに近くなった。
		//  TvWidth=336未満 ゲンジ通信あげだまのスタートデモ(MWR=0x50。40.0/100OK)。
		//  プライベートアイドルのスタート直後のデモ(MWR=0x10。38.6NG。39.0NG。46.2OK。小さめの値が必要。小さすぎても大きすぎても駄目。VblankLineTopの値によって変化する)。
		//  サイドアームスペシャルのBEFORE版でスタート時にステータス表示が揺ないために、大きめの値が必要（MWR=0x50。38.6時々NG。38.8OK）。
		//  スプリガンmk2の8面開始前デモで隊長のアップが乱れないために大きめの値が必要。（MWR=0x50。38.6,40.0OK)。
		//  ダンジョンエクスプローラーのストーリー紹介終了時にスクロール欄が１ライン一瞬乱れる(大きめの値が理想。SATB時のCPUストールの具合によって変化する。"SATB…/16"時に38.2NG)。
		//  エターナルシティで大きめの値で、アイテム入手時などで揺れが解消。39.0NG
		//  パワースポーツのニュース直前の9TV画面で揺れないために大きめの値が必要（MWR=0x10。SATB時のCPUストールの具合によって変化する。38.0,40.0OK）。
		//  ロードス島戦記２のエンディング(MWR=0x50。40.0OK。SATB時のCPUストールの具合によっても変化する)。
		//  サイレントデバッガーズのスタートで拳銃を撃つシーンで乱れる(小さめの値が必要。39.0/100OK。VBlineの値にもよる)。
		//  モトローダー(MWR=0x10)で大きめの値がよさそう。大きいと稀に画面揺れがあった(確実にここの値が関係しているかは不明)。
		//	ソルジャーブレイドのOPデモ(MWR=0x10。66.0/100ぐらい以下が必要。出来るだけ小さめの値のほうが曲のテンポ安定。SATBの自動転送は無し)。
		//  メタルエンジェルのスタート後自己紹介後に止まらないために(MWR=0x40。36.0/100OK。小さいとマネージャー紹介後のフェードアウトがNG。大き目のほうがいいみたい。rasterTimingCountの値によって最適値が変わる)。
		//	大きい値だと、パラソルスターでステージクリア時に稀に画面が揺れる(MWR=0x10。61.2NG。VCR=3)。
		//  ワールドヒーローズ２(MWR=0x50。57.0/100以下が必要)。聖夜物語の戦闘シーン(MWR=0x30。61.2NG。小さな値が必要。ScreenH=224。BG横128)。
		//  ゼロヨンチャンプのレースシーンで大き目のほうがいい。
		//  TvWidth=336以上 Mr.ヘリの大冒険(MWR=0x50。40/100以下が必要)。
		//  あすか120%のスタート直後のデモ(40/100以下辺りが必要。小さくても大きくても駄目。VblankLineTopの値によって変化する)。
		//  レインボーアイランド(27.2/100以上が必要)。
		//  ※rasterTimingCountの値やSATB転送のウェイト値によって、ここの最適値も変化する。大きめの値のほうがラスタ割り込みの漏れが少なくなるので、どちらかというと理想。
		if (_Regs[0][VDC_MWR] & 0x40) //BGの縦マップサイズが64(MWRが0x40以上)の場合。v2.61
			a = (int)(VDC_CYCLESPERLINE*40.0/100) - rasterTimingCount; // 40.0/100。メタルエンジェル，サイドアームスペシャルOK。Mrヘリの大冒険OK。エターナルシティOK。v2.62更新
																	   //           スプリガンmk2の8面デモ，ダンジョンエクスプローラー，ロードス島戦記２のエンディング。ワールドヒーローズ２。ゲンジ通信あげだまのスタートデモ。
		else
			a = (int)(VDC_CYCLESPERLINE*38.4/100) - rasterTimingCount; // 38.4/100。プライベートアイドルOK。あすか120%OK。v2.62更新
																	   //           パワースポーツ，ワールドヒーローズ２。聖夜物語。ソルジャーブレイド。パラソルスター。
		if (a < 0)
			a = 0;
		else
			cpuAdvance(a);

		//真・女神転生のオートマップ画面崩れ対策。この位置でオーバークロックする。※実機でも乱れるが、快適にプレイできるよう実施。v2.20
		if (_bShinMegamiTensei)
		{
			if (APP_GetAutoShinMegamiTensei())
				for (i=1; i<=3350; i++) //3350でOK。3360以上だと若干他の場面(COMPメニューを開くタイミングなど)で速すぎる気がする。3300だとシンジュクや後半の大きなMAPでNG。他の場面で影響が出ないよう、なるべく小さい値にする。フェードアウトや人や店のメッセージ表示のときに実機の速度のように「ある程度の間」があるほうが雰囲気が出ていい。
					if (gCPU_ClockCount++ >= 0) CPU_ExecuteOperation();
		}
		
		for (_VN=0; _VN<=_SuperGrafx; _VN++) //スーパーグラフィックスモードならVDC２つぶんを動かす
		{
			if (_ScanLine == _VblankLineTop[_VN])
			{
				//Kitao追加。MWR（メモリ幅）レジスタが変更されていた場合、ここでメモリマップを更新する。
				if (_bMWRchange[_VN])
				{
					_bMWRchange[_VN] = FALSE;
					
					bmwr0x40 = ((MWR & 0x40) != 0);
					_BGH[_VN] = (bmwr0x40) ? 64 : 32;
					_BGW[_VN] = _BGWidthTable[(MWR >> 4) & 3];
					_LineCountMask[_VN] = (bmwr0x40) ? 0x1FF : 0xFF;
					_LineCounter[_VN] &= _LineCountMask[_VN];
					invalidate_tile_cache();

					if (MWR & 0x08)
						_MaxSpPerLine[_VN] = 14; //スプライト横並び14個制限。v2.63追加
					else
						_MaxSpPerLine[_VN] = 16; //スプライト横並び16個制限
				}
				
				//Kitao追加。VRAM-VRAM間のDMA転送コマンドが来ていた場合、ここで実行する。ここで行うことがラングリッサーで必要。
				if (_bVramDmaExecute[_VN])
				{
					_bVramDmaExecute[_VN] = FALSE;
					
					srcInc = (DCR & 4) ? -1 : 1;
					dstInc = (DCR & 8) ? -1 : 1;
					do
					{
						write_vram(DESR, _pwVideoRam[_VN][SOUR]);
						SOUR += srcInc;
						DESR += dstInc;
					} while (LENR--);
					
					if (DCR & 0x2)
					{
						_VdcStatus[_VN] |= VDC_STAT_DV;
						INTCTRL_Request(INTCTRL_IRQ1);
					}
				}
				
				//Vblank割り込み要求
				if ((CR & VDC_CTRL_VC)||(_bVDCleared[_VN]))
				{
					_bVDCleared[_VN] = FALSE;
					_VdcStatus[_VN] |= VDC_STAT_VD; //割り込みを発生させるときのみセット。Kitao追記：SUPER桃鉄１で必要。
					INTCTRL_Request(INTCTRL_IRQ1);
				}
				
				//DMA from VRAM to SATB (実機では転送中に CPU が完全に停止しない)。
				if (_bUpdateSATB[_VN] || (DCR & 0x10))
				{
					_bUpdateSATB[_VN] = FALSE;
					update_satb(SATB); //ここですぐ更新することが、天外魔境ZIRIAで必要。
					_SpDmaCount[_VN] = 4; //4。参考：5以上だとレミングスのステータス表示に問題。1だとサイバークロスのデモが動かず。大きいとスーパーバレーで画面揺れ。小さいとチェイスH.Q.で画面がフラッシュする。
					_SpDmaStole[_VN] = 0;
				}
			}
		}
	
		//VBlank後に残りのCPUパワーを使う。
		if (_OverClockCycle < 0) gCPU_ClockCount += _OverClockCycle; //ダウンクロックする場合。このタイミングで入れると動作への影響が少ない。v1.61追加。主にデバッグ用。
		cpuAdvance(VDC_CYCLESPERLINE - (rasterTimingCount + a));
	}
	else //通常
	{
		//Kitao追加。残りのCPUパワーを描画後に使う。
		if (_OverClockCycle < 0) gCPU_ClockCount += _OverClockCycle; //ダウンクロックする場合。このタイミングで入れると動作への影響が少ない。v1.61追加。主にデバッグ用。
		if (_ScanLine <= 179) //ぴったりと動作クロックを合わせるため、179ラインまでは1クロック多く進める。
			cpuAdvance(VDC_CYCLESPERLINE -rasterTimingCount +1); //小数点端数(453.0 != 453.68)のぶんも進める
		else //180ライン以降
			cpuAdvance(VDC_CYCLESPERLINE -rasterTimingCount);
	}

	//Kitao追加。オーバークロック機能を追加。
	if (_OverClockCycle > 0)
	{	//ADPCMのアクセスタイミングや、曲のテンポなどへの影響を避けるため、CDROM,APU,TIMERは進めない。
		_bOverClockNow = TRUE;
		if (_SuperGrafx == 1)
		{
			for (i=1; i<=_OverClockCycle; i++)
			{
				vramDmaCountCheckSG();
				MOUSE_AdvanceClock(); //マウスは動きが良くなるので進める
				if (gCPU_ClockCount++ >= 0) CPU_ExecuteOperation();
			}
		}
		else //高速化のため、スパグラのときと処理を分けた。
		{
			for (i=1; i<=_OverClockCycle; i++)
			{
				vramDmaCountCheck();
				MOUSE_AdvanceClock(); //マウスは動きが良くなるので進める
				if (gCPU_ClockCount++ >= 0) CPU_ExecuteOperation();
			}
		}
		_bOverClockNow = FALSE;
	}

	//Kitao追加。シュビビンマン２で１フレーム画面が崩れる部分（※実機でも起こる）を解消。
	//			 ラスタ割り込みの要求がまだ伝わっていなかった場合。オーバークロックして意地でも伝える。
	//			 実機でも起こるのでそのままでもと思ったが、せっかく綺麗にできるので実装。
	//			 実機と違う動きになるので、他のソフト（天使の詩等）では逆に弊害が出ることがある。
	//           パックランドで面セレクト直後１フレーム画面が崩れる部分（※実機でも起こる）を解消。
	//           真・女神転生のイベントシーンや戦闘シーンなどで時々１フレーム画面が崩れる部分（※実機でも起こる）を解消。
	//			 実機で味になっているような現象はそのままにしておきたいが、「明らかに崩れないほうがいい場面」に限っては快適な動きにする方向で行きたい。v2.08記
	if (_bForceRaster)
	{
		for (_VN=0; _VN<=_SuperGrafx; _VN++) //スーパーグラフィックスモードならVDC２つぶんを動かす
		{
			i = 0;
			while (_VdcStatus[_VN] & VDC_STAT_RR)
			{
				if (gCPU_ClockCount++ >= 0) CPU_ExecuteOperation();
				if (++i > VDC_CYCLESPERLINE*MAX_SCANLINE*2) //最大約２フレームぶんオーバークロック（これより少ないとシュビビンマン２でやや乱れ）。それまで待って駄目ならスルー。
					break;
			}
		}
	}

	if (_ScanLine == MAX_SCANLINE)
	{
		//Kitao追加。パックランドで面セレクト直後１フレーム画面が崩れる部分（※実機でも起こる）を解消。_bForceRasterを使うようにしたので現在非使用。
		//           真・女神転生のイベントシーンや戦闘シーンなどで時々１フレーム画面が崩れる部分（※実機でも起こる）を解消。v2.24
		//			 シュビビンマン２でステージ２(潜水艦)クリア→次の面スタート時に１フレーム画面が崩れる部分（※確認してないがたぶん実機でも起こる。時間のあるときに確認）を解消。v2.40
		if (_bForceVBlank)
		{
			for (_VN=0; _VN<=_SuperGrafx; _VN++) //スーパーグラフィックスモードならVDC２つぶんを動かす
			{
				i = 0;
				while (_VdcStatus[_VN] & VDC_STAT_VD)
				{
					if (gCPU_ClockCount++ >= 0) CPU_ExecuteOperation();
					if (++i > VDC_CYCLESPERLINE*MAX_SCANLINE*2) //最大約２フレームぶんオーバークロック。それまで待って駄目ならスルー。
						break;
				}
			}
		}
		//次のフレームへの準備
		_ScanLine = 0;
		_DisplayCounter = 0;
		//VBlank期間に垂直表示開始位置が変更されていた場合、次フレームの垂直表示開始位置を補正する。v2.09更新
		for (_VN=0; _VN<=_SuperGrafx; _VN++) //スーパーグラフィックスモードならVDC２つぶんを動かす
		{
			if	(_DCC & 4) //DCCが0のときは調整しない。妖怪道中記。v2.49
				_TvStartLine[_VN] += _LineOffset[_VN]; //ミズバク大冒険の雷アイテム画面揺れ再現に必要。スプラッシュレイクのステージ開始時に必要。v2.45更新
			_LineOffset[_VN] = 0; //オフセットをリセットする。
		}
	}
}


//Kitao追加。ラスタ割り込み判定直後～描画までに使うCPUパワーを設定する。
//			 記：エミュレーションの処理速度制限のため、実機のようにライン描画途中で"１ドット描画ごと"に割り込みを
//				 発生させることはできないため、このRasterTimingの設定が必要。
//				 そのため、デフォルト設定だと画面が揺れたりずれてしまうソフトがいくつか出てしまうが、ソフトごとの設定が
//				 決まれば、実機よりもちらつかない綺麗なラインスクロールができる利点がある。
void
VDC_SetRasterTiming(
	Sint32	n)
//nの値→ 1=早めに「ラスタ割り込み要求」を知らせるのが最適なソフト。2=中ぐらい（通常）に「ラスタ割り込み要求」を知らせるのが最適なソフト。3=遅めに「ラスタ割り込み要求」を知らせるのが最適なソフト。
//v1.59でレジスタの設定によってタイミングが変わる(推定)のを再現。LATEなどなどはほぼ不必要に。EARLYだけいくつかのソフトで現状必要(何かサインはあるはず)。
{
	//Normalに設定するときで、"メニュー表示がNormalの位置のまま_RasterTimingTypeを自動変更するゲームの場合"はその設定値に戻す。
	if (n == 2)
		if (_AutoRasterTimingType >= 11) //メニュー表示がNormalの位置のまま_RasterTimingTypeを自動変更するゲーム
			n = _AutoRasterTimingType;

	_RasterTimingType = n;

	switch (_RasterTimingType)
	{
		case 1: //EARLY
			_RasterTimingCount = 200; //200
			break;
		case 2: //MIDDLE(Normal)
			//カダッシュで大きめの値が必要。エターナルシティ。Ｆ１サーカスのスターティンググリッド。サイドアームSPで大き目の値が必要。ダンジョンエクスプローラーOK(大き目のほうが良し)。大きいとチェイスＨＱで乱れ。
			_RasterTimingCount = 88; //88。手持ちのソフトでは他のゲームもほとんどがMIDDLEで問題ないようだ。
			break;
		case 3: //LATE
			_RasterTimingCount = 67; //67。スプラッターハウス。デビルクラッシュ(69以下必要。実機では乱れてる)。マジクール。レギオン。
			break;
		case 4: //MORE EARLY
			_RasterTimingCount = 350; //350。虎への道(210ぐらい以上が必要)。"オーダーザグリフォン(U)"もこれが最適とのこと(海外の方からの情報)。
			break;
		case 5: //MORE LATE
			_RasterTimingCount = 33; //33。スーパーダライアスIIのハイスコア表示。
			break;
		case 11: //LittleEARLY アウトラン(88～95必要。95最適)。
			_RasterTimingCount = 95; //95
			break;
		case 12: //サイドアームSP用だったが現在非使用。サイドアームSPのアレンジ版は「_RasterTimingType=2(Normal)」で割り込みの時間を考慮するとうまく動く。
			_RasterTimingType = 2;
			_RasterTimingCount = 88;
			break;
		case 13: //LittleLATE ワンダーモモ(現在非使用)。フォゴットンワールド(現在非使用)。ステートセーブしていた場合を考えて残しておく。
			_RasterTimingType = 2;
			_RasterTimingCount = 88; //元は84,79,80
			break;
		case 14: //MostEARLY 龍虎の拳専用(現在非使用)。ステートセーブしていた場合を考えて残しておく。
			_RasterTimingType = 2;
			_RasterTimingCount = 88; //元は80,420
			break;
		case 15: //ソルジャーブレード用だったが現在非使用。ステートセーブしていた場合を考えて残しておく。
			//チェイスH.Q.(以前はMORE LATE)でも使用。「過去は個別設定→現在はノーマル設定」に変えた場合、過去バージョンのステートをロードしたときのためにここを使って必ずノーマルに戻るようにする。
			_RasterTimingType = 2;
			_RasterTimingCount = 88; //旧305。ソルジャーブレード(３面序盤で305以上必要。※逆に大きすぎてもOPデモの最後でやや乱れ)。
			break;
	}
}

//Kitao追加
Sint32
VDC_GetRasterTimingType()
{
	return _RasterTimingType;
}

//Kitao追加。問題のあるゲームは起動時に自動でラスタタイミングを設定する。MAINBOARD.c と CDROM.c から利用。
void
VDC_SetAutoRasterTiming(
	Sint32	n)
{
	_AutoRasterTimingType = n;
	VDC_SetRasterTiming(n);
}

//Kitao追加
Sint32
VDC_GetAutoRasterTimingType()
{
	return _AutoRasterTimingType;
}


//Kitao追加。CPUをオーバークロックして高速化する。
void
VDC_SetOverClock(
	Sint32	n)
//n=0ならノーマル
{
	_OverClockType = n;
	_OverClockCycle = 0;
	CPU_SetTurboCycle(0);
	switch (_OverClockType)
	{
		case -2: _OverClockCycle = -(Sint32)(VDC_CYCLESPERLINE / 2.0 +0.5); break;	//0.50倍
		case -1: _OverClockCycle = -(Sint32)(VDC_CYCLESPERLINE / 4.0 +0.5); break;	//0.75倍
		case  1: _OverClockCycle = (Sint32)(VDC_CYCLESPERLINE / 4.0 +0.5); break;	//1.25倍
		case  2: _OverClockCycle = (Sint32)(VDC_CYCLESPERLINE / 3.0 +0.5); break;	//1.33倍
		case  3: _OverClockCycle = (Sint32)(VDC_CYCLESPERLINE / 2.0 +0.5); break;	//1.5倍
		case  4: _OverClockCycle = VDC_CYCLESPERLINE; break;				//2.0倍
		case  5: _OverClockCycle = (Sint32)(VDC_CYCLESPERLINE*1.5); break;			//2.5倍
		case  6: _OverClockCycle = (Sint32)(VDC_CYCLESPERLINE*2.0); break;			//3.0倍
		//100,200,300はターボサイクルモード(消費サイクルを減らす)。CPU.cにて速さを高める。
		case 100: CPU_SetTurboCycle(100); break; //ターボ１倍
		case 200: CPU_SetTurboCycle(200); break; //ターボ２倍
		case 300: CPU_SetTurboCycle(300); _OverClockCycle = (Sint32)(VDC_CYCLESPERLINE / 2.0 +0.5);	break; //ターボ３倍(ターボ２倍x1.5倍)
	}
}

//Kitao追加。使用中のオーバークロックタイプを得る。cpu.c から使用
Sint32
VDC_GetOverClockType()
{
	return _OverClockType;
}

//Kitao追加。ゲームのタイミング不具合を解消するために、自動でオーバークロック設定にする。※ぽっぷるメイル,IQパニックで使用。
void
VDC_SetAutoOverClock(
	Sint32	n)
{
	_AutoOverClock = n;
	if ((_OverClockType < _AutoOverClock)|| //※すでにオーバークロックしていた場合はそのまま
		(_AutoOverClock == 0)) //スピードダウンのとき
			VDC_SetOverClock(_AutoOverClock);
}

//Kitao追加
Sint32
VDC_GetAutoOverClock()
{
	return _AutoOverClock;
}


//Kitao追加。速度タイミングが合わず問題が起こるゲームにウェイトを入れる。（SATB転送時にウェイトを入れる）
void
VDC_SetWaitPatch(
	Sint32	cycle)
{
	_WaitPatch = cycle; //１フレームでウェイトするサイクル数
}


//Kitao追加。強制的にラスタ割り込みがCPUに伝わるまで待つかどうかを設定する。（CPUに伝わるまでオーバークロックする）
void
VDC_SetForceRaster(
	BOOL	forceRaster)
{
	_bForceRaster = forceRaster;
}


//Kitao追加。強制的にVBlank割り込みがCPUに伝わるまで待つかどうかを設定する。（CPUに伝わるまでオーバークロックする）
void
VDC_SetForceVBlank(
	BOOL	forceVBlank)
{
	_bForceVBlank = forceVBlank;
}


//Kitao追加。スプライト欠けを再現するかどうかを設定する。
void
VDC_SetPerformSpriteLimit(
	BOOL	bPerform)
{
	_bPerformSpriteLimit = bPerform;
}

//Kitao追加。スプライト全表示で問題のあるゲームは、起動時に自動でスプライト欠け再現の設定にする。MAINBOARD.c と CDROM.c から利用。
void
VDC_SetAutoPerformSpriteLimit(
	BOOL	bPerform)
{
	_bAutoPerformSpriteLimit = bPerform;
	_bPerformSpriteLimit = bPerform;
}

//Kitao追加
BOOL
VDC_GetPerformSpriteLimit()
{
	return _bPerformSpriteLimit;
}


//Kitao追加。スプライト,BGを表示・非表示切替する機能(開発向け)。v2.06
void
VDC_SetLayer()
{
	_bSpriteLayer = APP_GetSpriteLayer();
	_bSprite2Layer = APP_GetSprite2Layer();
	_bBGLayer  = APP_GetBGLayer();
	_bBG2Layer = APP_GetBG2Layer();
}


Sint32
VDC_GetScanLine()
{
	return _ScanLine;
}


//Kitao追加。_ScanLineがVBlank期間開始の次のラインならTRUEを返す。CDROM.c から利用。※現在非使用
BOOL
VDC_CheckVBlankStart()
{
	return (_ScanLine == _VblankLineTop[0] +1);
}


//Kitao追加
void
VDC_SetScreenWidth(
	Sint32	screenW)
{
	_ScreenW[0] = screenW;
}

//Kitao更新
const Sint32
VDC_GetScreenWidth()
{
	return _ScreenW[0];
}


const Sint32
VDC_GetDisplayHeight()
{
	return _ScreenH[0];
}


//Kitao追加。テレビ画面上の「一番上のライン」の座標を返す。
Sint32
VDC_GetTvStartLine()
{
	return _TvStartLine[0] + _TvStartLineAdjust[0]; //v2.47更新
}


//Kitao追加
Uint8
VDC_GetVdcStatus()
{
	return _VdcStatus[0];
}


//Kitao追加
void
VDC_SetSuperGrafx(
	Sint32	superGrafx)
{
	_SuperGrafx = superGrafx;
}

//Kitao追加
Sint32
VDC_GetSuperGrafx()
{
	return _SuperGrafx;
}

//Kitao追加。オーバークロック分のCPU処理中はTRUE。オーバークロックしていないか、または通常分パワーを処理中ならFALSEを返す。
BOOL
VDC_GetOverClockNow()
{
	return _bOverClockNow;
}


/*-----------------------------------------------------------------------------
Kitao更新。v1.11。高速化のためVCE.c をここへ統合した。
-----------------------------------------------------------------------------*/

//Kitao追加。TV画面上での表示幅を設定する
static inline void
set_TvWidth()
{
	if (_DCC & 2)
		_TvWidth = 512;
	else if (_DCC & 1)
		_TvWidth = 336;
	else
		_TvWidth = 256;
	_VN = 0;
	setTvStartPos(); //水平表示開始位置を更新
}

//Kitao追加。v2.50
static inline void
set_Palette()
{
	Uint32	c;

	_Palette333[_ColorTableAddr] = _ColorLatch; //Kitao更新。「0RGB」で32ビット変数へ格納するようにした。v1.61若干高速化。
	c = ((_ColorLatch & 0x0038)<<13)|((_ColorLatch & 0x01C0)<<2)|(_ColorLatch & 0x0007);
	if (_ColorTableAddr < 0x0100) //BG用パレット
	{
		if (_ColorTableAddr == 0x0000) //BG用パレット透明色
		{
			_PaletteBG[0x00] = _PaletteBG[0x10] = _PaletteBG[0x20] = _PaletteBG[0x30] =
			_PaletteBG[0x40] = _PaletteBG[0x50] = _PaletteBG[0x60] = _PaletteBG[0x70] =
			_PaletteBG[0x80] = _PaletteBG[0x90] = _PaletteBG[0xA0] = _PaletteBG[0xB0] =
			_PaletteBG[0xC0] = _PaletteBG[0xD0] = _PaletteBG[0xE0] = _PaletteBG[0xF0] = c;
		}
		else if (_ColorTableAddr & 0xF) //透明色以外なら
			_PaletteBG[_ColorTableAddr] = c;
	}
	else //スプライト用パレット
	{
		if (_ColorTableAddr == 0x0100) //スプライト用パレット透明色
		{
			_PaletteSP[0x00] = _PaletteSP[0x10] = _PaletteSP[0x20] = _PaletteSP[0x30] =
			_PaletteSP[0x40] = _PaletteSP[0x50] = _PaletteSP[0x60] = _PaletteSP[0x70] =
			_PaletteSP[0x80] = _PaletteSP[0x90] = _PaletteSP[0xA0] = _PaletteSP[0xB0] =
			_PaletteSP[0xC0] = _PaletteSP[0xD0] = _PaletteSP[0xE0] = _PaletteSP[0xF0] = c;
		}
		else if (_ColorTableAddr & 0xF) //透明色以外なら
			_PaletteSP[_ColorTableAddr & 0xFF] = c;
	}
}

/*-----------------------------------------------------------------------------
	[VDC_VceWrite]
		VCEへの書き込み動作を記述します。
-----------------------------------------------------------------------------*/
/*
	[write_palette]

	convert GRB333 --> RGB555 / RGB565
	and write to the palette.

	basic logic:

	Uint8 G = (color & 0x1c0) >> 6;
	Uint8 R = (color & 0x038) >> 3;
	Uint8 B = (color & 0x007) >> 0;

	Rshift = 10;
	Gshift = 5;
	Bshift = 0;

	_Palette[addr]	= R << (Rshift+2);
	_Palette[addr] |= G << (Gshift+2);
	_Palette[addr] |= B << (Bshift+2);
*/
void
VDC_VceWrite(
	Uint32	regNum,
	Uint8	data)
{
	//CPU 1 Clock Wait  VCEポートアクセスのウェイト。
	//		妖怪道中記のギャンブル画面で１フレームの乱れが解消。
	//		ネクロスの要塞(MWR=0x50)のOPデモでナイトの攻撃をはみ出さないために必要。
	//		バーストモードの最中は、ウェイトが入らないかもしれない。現状はPCへの負荷を優先して、常にウェイトが入ることとする。v2.07記
	if (gCPU_Transfer != 0) //転送命令からのVDCアクセスの場合。これがないと、雀偵物語２のＯＰデモで乱れ。v2.62更新
		gCPU_ClockCount--; //CPU 1 Clock Wait

	//v1.15更新。高速化。case文は、よく使われるものを先に置いたほうが速く処理できそう。
	switch (regNum & 7)
	{
		case 5:	// カラーテーブルデータライト Ｈ
			if (data & 1) //v1.10更新
				_ColorLatch |= 0x100;
			else
				_ColorLatch &= 0xFF;
			set_Palette(); //Kitao更新。v2.50
			_ColorTableAddr++;
			_ColorTableAddr &= 0x1FF;
			return;

		case 4:	// カラーテーブルデータライト Ｌ
			_ColorLatch = data; //Kitao更新。上位バイトはクリア。ワールドサーキット。v2.50
			set_Palette(); //Kitao更新。Lowバイト側書き換え時も、画面結果に反映される。ワールドサーキット。v2.50
			return;

		case 3:	// カラーテーブルアドレス Ｈ
			_ColorTableAddr = (_ColorTableAddr & 0xFF) | ((data & 1) << 8);
			return;

		case 2:	// カラーテーブルアドレス Ｌ
			_ColorTableAddr = (_ColorTableAddr & 0x100) | data; //v2.50記：上位バイトをクリアするとドルアーガの塔でキャラの色化け
			return;

//		case 1: // コントロールレジスタ Ｈ
//			return;

		case 0: // コントロールレジスタ Ｌ
			// (ドットクロックを指定する。bit7が1のときは白黒モード）
			_DCC = data & 0x87;
			set_TvWidth(); //Kitao追加
			VDC_SetTvStartLine(); //Kitao追加。垂直表示開始位置を更新
			return;
	}
}

/*-----------------------------------------------------------------------------
	[VDC_VceRead]
		VCEからの読み出し動作を記述します．
-----------------------------------------------------------------------------*/
/*
	[read_palette]

	convert RGB555 / RGB565 --> GRB333

	basic logic:

	Uint8 R = (_Palette[addr] & 0x7c00) >> 10;
	Uint8 G = (_Palette[addr] & 0x03e0) >> 5;
	Uint8 B = (_Palette[addr] & 0x001f) >> 0;

	G >>= 2; R >>=	2; B >>= 2;

	return (G << 6) | (R << 3) | B;

	 0RRR RRGG GGGB BBBB
   & 0111 0011 1001 1100  (0x739c)
   = 0RRR 00GG G00B BB00

	 0000 000G GGRR RBBB
*/
Uint8
VDC_VceRead(
	Uint32	regNum)
{
	Uint8	ret;

	switch (regNum & 7) //Kitao更新
	{
		case 5:	/* カラーテーブルデータリード Ｈ */
			ret = (Uint8)(_Palette333[_ColorTableAddr++] >> 8) | 0xFE;
			_ColorTableAddr &= 0x1FF;
			return ret;
		case 4:	/* カラーテーブルデータリード Ｌ */
			return (Uint8)_Palette333[_ColorTableAddr];
	}

	//PRINTF("VceRead %X", regNum);
	return 0xFF; //Kitao更新。0でなく0xFFを返すことで、ミズバク大冒険の４面の不具合(岩の軌道が異なる)がついに解消した。v1.62
}


//Kitao追加。テレビ画面の横表示解像度（ドットクロックで決まる）を返す。
Sint32
VDC_GetTvWidth()
{
	return _TvWidth;
}

//Kitao追加
Uint8
VDC_GetDCC()
{
	return _DCC;
}


//Kitao追加。スプライトの透明色を32ビットカラーで返す。
Uint32
VDC_GetSpColorZero()
{
	return _PaletteSP[0]; //スプライトの透明色
}


//Kitao追加。v2.20
void
VDC_SetShinMegamiTensei(
	BOOL	shinMegamiTensei)
{
	_bShinMegamiTensei = shinMegamiTensei;
}

//Kitao追加。v2.20
Sint32
VDC_GetShinMegamiTensei()
{
	return	_bShinMegamiTensei;
}

//Kitao追加。v2.64
void
VDC_SetWorldStadium91(
	BOOL	worldStadium91)
{
	_bWorldStadium91 = worldStadium91;
}


// save variable
#define SAVE_V(V)	if (fwrite(&V, sizeof(V), 1, p) != 1)	return FALSE
#define LOAD_V(V)	if (fread(&V, sizeof(V), 1, p) != 1)	return FALSE
// save array
#define SAVE_A(A)	if (fwrite(A, sizeof(A), 1, p) != 1)	return FALSE
#define LOAD_A(A)	if (fread(A, sizeof(A), 1, p) != 1)		return FALSE

//Kitao追加。v0.89より古いステートセーブファイル(スーパーグラフィックス実装前)をロード
BOOL
OldLoadState(
	FILE*		p)
{
	Uint16		Regs[32];
	Uint32		AR;

	Uint8		VideoRam[65536]; //VRAM 64KB
	Uint16		ReadData; //Kitao追加。実際のVRAMリードアクセスはVDCポートの"MARRライト時"におこなわれる(推定)。

	Sint32		ScreenW;
	Sint32		ScreenH;
	Uint32		BGH;
	Uint32		BGW;

	Uint16		VdcAddrInc;
	Uint8		VdcStatus;

	Sint32		RasterCounter;
	BOOL		bRasterRequested; //Kitao追加。そのラインのラスタ割り込み要求を終えていたらTRUE
	Sint32		LineCounter;
	Sint32		LineCountMask;

	BOOL		bUpdateSATB;
	Uint32		SpDmaCount;
	Sint32		SpDmaStole;
	Uint32		VramDmaCount; //v0.82

	BOOL		bSpOver;
	BOOL		bOverlap;
	BOOL		bBurstMode;
	BOOL		bMWRchange;

	Uint16		SpRam[64*4];

	Sint32		TvStartLine;
	Sint32		VblankLine;

	Sint32		rasterTimingType;
	BOOL		bPerformSpriteLimit;
	Sint32		waitPatch;
	Sint32		popfulMail;
	Sint32		overClockType;

	LOAD_A(Regs);
	LOAD_A(VideoRam);
	LOAD_A(SpRam);

	LOAD_V(AR);
	if (MAINBOARD_GetStateVersion() >= 10) //Kitao追加。v0.72以降のセーブファイルなら
	{
		LOAD_V(ReadData); //Kitao追加。v0.72
	}
	else
		ReadData = 0;

	LOAD_V(ScreenW);
	LOAD_V(ScreenH);
	LOAD_V(BGH);
	LOAD_V(BGW);

	LOAD_V(VdcAddrInc);
	LOAD_V(VdcStatus);

	LOAD_V(_ClockCounter); //Kitao追加
	LOAD_V(_ScanLine);
	LOAD_V(RasterCounter);
	LOAD_V(bRasterRequested); //Kitao追加
	if (MAINBOARD_GetStateVersion() >= 7) //Kitao追加。v0.62以降のセーブファイルなら
	{
		LOAD_V(_bRasterLate);
	}
	else
		_bRasterLate = TRUE;
	LOAD_V(_DisplayCounter);
	LOAD_V(LineCounter);
	LOAD_V(LineCountMask);

	LOAD_V(bUpdateSATB);
	LOAD_V(bSpOver);
	if (MAINBOARD_GetStateVersion() >= 11) //Kitao追加。v0.74以降のセーブファイルなら
	{
		LOAD_V(bOverlap);
	}
	else
		bOverlap = FALSE;
	LOAD_V(SpDmaCount);
	if (MAINBOARD_GetStateVersion() >= 16) //Kitao追加。v0.87以降のセーブファイルなら
		LOAD_V(SpDmaStole); //旧バージョンの場合、現在とは違う利用値なので非使用。ダミー読み込み。
	if (MAINBOARD_GetStateVersion() >= 15) //Kitao追加。v0.82以降のセーブファイルなら
	{
		LOAD_V(VramDmaCount);
	}
	else
		VramDmaCount = 0;
	LOAD_V(bBurstMode);
	LOAD_V(bMWRchange); //Kitao追加

	LOAD_V(VblankLine); //Kitao追加
	LOAD_V(TvStartLine); //Kitao追加
	LOAD_V(rasterTimingType); //Kitao追加
	if (_AutoRasterTimingType != 2) //※_RasterTimingTypeを自動変更してある場合は読み込まず、そのまま_RasterTimingTypeを固定する。
		VDC_SetRasterTiming(_AutoRasterTimingType); //自動変更してあるゲームの場合
	else
		VDC_SetRasterTiming(rasterTimingType); //自動変更していなかった場合
	LOAD_V(overClockType); //Kitao追加
	if ((overClockType < _AutoOverClock)||(_AutoOverClock == 0)) //ゲームごとの自動設定があればそれを優先する。すでにオーバークロックしていた場合はそのまま。ノーマル強制のゲームは強制で(現在未使用)。
		VDC_SetOverClock(_AutoOverClock);
	else
	{
		if (APP_GetLoadStateSpeedSetting()) //速度変更設定を反映する設定の場合。反映しない設定(デフォルト)の場合は、現状のオーバークロック設定を引き継ぐ。誤ってDeleteキーを押して知らずに速度変更してしまうケースがあるのでv2.36からデフォルトでは反映しないようにした。
			VDC_SetOverClock(overClockType);
	}
	LOAD_V(bPerformSpriteLimit); //Kitao追加
	if (!_bAutoPerformSpriteLimit) //※_bPerformSpriteLimitを自動変更してある場合は読み込まず、そのまま_bPerformSpriteLimitを固定する。
		VDC_SetPerformSpriteLimit(bPerformSpriteLimit); //自動変更していなかった場合
	LOAD_V(waitPatch); //Kitao追加。現在ダミー。_WaitPatchはゲーム固有の初期値のままにする。

	if (MAINBOARD_GetStateVersion() >= 8) //Kitao追加。v0.64以降のセーブファイルなら
		LOAD_V(popfulMail); //v0.83から非使用

	//VDC１つ目の変数に当てはめる
	memcpy(_Regs, Regs, sizeof(Regs));
	_AR[0] = AR;
	memcpy(_VideoRam, VideoRam, sizeof(VideoRam));
	_ReadData[0] = ReadData;
	_ScreenW[0] = ScreenW;
	_ScreenH[0] = ScreenH;
	_BGH[0] = BGH;
	_BGW[0] = BGW;
	_VdcAddrInc[0] = VdcAddrInc;
	_VdcStatus[0] = VdcStatus;
	_RasterCounter[0] = RasterCounter;
	_bRasterRequested[0] = bRasterRequested;
	_LineCounter[0] = LineCounter;
	_LineCountMask[0] = LineCountMask;
	_LineOffset[0] = 0; //v1.30追加
	_bUpdateSATB[0] = bUpdateSATB;
	_SpDmaCount[0] = SpDmaCount;
	_SpDmaStole[0] = 0;
	_VramDmaCount[0] = VramDmaCount;
	_bVramDmaExecute[0] = FALSE;
	_bSpOver[0] = bSpOver;
	_bOverlap[0] = bOverlap;
	_bBurstMode[0] = bBurstMode;
	_bVDCAccessWait[0] = TRUE; //v2.08追加
	_bMWRchange[0] = bMWRchange;
	memcpy(_SpRam, SpRam, sizeof(SpRam));
	_TvStartLine[0] = TvStartLine;
	_VblankLine[0] = VblankLine;

	_VN=0; //Kitao追加
	if (VDW == 0) //v0.95以前の不具合対策。画面の初期化をしていないソフト(hesファイルなど)で0のまま保存してしまっていたので、初期値(239)を入れる。
	{
		VDW = 239;
		VCR = 4;
		VPR = 0x0F02;
	}
	invalidate_tile_cache(); // remake all the bg and sp tiles
	if (MWR & 0x08)
		_MaxSpPerLine[_VN] = 14; //スプライト横並び14個制限。v2.63追加
	else
		_MaxSpPerLine[_VN] = 16; //スプライト横並び16個制限

	_bVDCleared[0] = FALSE; //v1.63追加

	//VCE.c をここへ統合。v1.11
	LOAD_V(_DCC);
	LOAD_V(_ColorTableAddr);
	if (MAINBOARD_GetStateVersion() >= 3) //Kitao追加。v0.57以降のセーブファイルなら
	{
		LOAD_V(_TvStartPos); //次のset_TvWidth()で再設定されるため、現在非必要。ダミー読み込み。v1.11
		LOAD_V(_TvWidth); //次のset_TvWidth()で再設定されるため、現在非必要。ダミー読み込み。。v1.11
	}
	set_TvWidth(); //Kitao追加。_TvWidth,_TvStartPos,_TvMaxを最新の状態に更新。
	_VN=0;
	VDC_SetTvStartLine();
	_TvStartLineAdjust[_VN] = 0; //v2.47追加
	VDC_SetVblankLine(); //v2.48追加。必要
	_VblankLineTop[_VN] = _VblankLine[_VN]; //v1.19追加
	LOAD_A(_Palette333);
	LOAD_A(_PaletteBG); //v1.10更新
	LOAD_A(_PaletteSP); //v1.10更新
	LOAD_V(_ColorLatch);

	return TRUE;
}

/*-----------------------------------------------------------------------------
	[SaveState]
		状態をファイルに保存します。 
-----------------------------------------------------------------------------*/
BOOL
VDC_SaveState(
	FILE*		p)
{
	Sint32	overClockType = _OverClockType;

	if (p == NULL)
		return FALSE;

	SAVE_A(_Regs);
	SAVE_A(_VideoRam);
	SAVE_A(_SpRam);

	SAVE_V(_AR);
	SAVE_V(_ReadData); //Kitao追加。v0.72

	SAVE_V(_ScreenW);
	SAVE_V(_ScreenH);
	SAVE_V(_BGH);
	SAVE_V(_BGW);

	SAVE_V(_VdcAddrInc);
	SAVE_V(_VdcStatus);

	SAVE_V(_ClockCounter); //Kitao追加
	SAVE_V(_ScanLine);
	SAVE_V(_RasterCounter);
	SAVE_V(_bRasterRequested); //Kitao追加
	SAVE_V(_bRasterLate); //Kitao追加。v0.62
	SAVE_V(_DisplayCounter);
	SAVE_V(_LineCounter);
	SAVE_V(_LineCountMask);
	SAVE_V(_LineOffset); //Kitao追加。v1.30
	SAVE_V(_TvStartLine); //Kitao追加。v2.47
	SAVE_V(_TvStartLineAdjust); //Kitao追加。v2.47

	SAVE_V(_bUpdateSATB);
	SAVE_V(_bSpOver);
	SAVE_V(_bOverlap); //Kitao追加。v0.74
	SAVE_V(_SpDmaCount);
	SAVE_V(_SpDmaStole);
	SAVE_V(_VramDmaCount); //Kitao追加。v0.82
	SAVE_V(_bVramDmaExecute); //Kitao追加。v1.02
	SAVE_V(_bBurstMode);
	SAVE_V(_bVDCAccessWait); //Kitao追加。v2.08
	SAVE_V(_bMWRchange); //Kitao追加

	SAVE_V(_RasterTimingType); //Kitao追加
	SAVE_V(_AutoRasterTimingType); //v1.65追加
	if (_AutoOverClock != -1) //自動でオーバークロックしているゲームの場合。v2.20追加
		if (_OverClockType == _AutoOverClock)
			overClockType = 0; //自動と同じ状態なら、ノーマル速度としてセーブしておく。将来バージョンで自動オーバークロックしない設定にしても問題がないように。
	SAVE_V(overClockType); //Kitao追加
	SAVE_V(_bPerformSpriteLimit); //Kitao追加
	SAVE_V(_WaitPatch); //Kitao追加
	SAVE_V(_bIRQ1CancelExecute);//Kitao追加。v1.03。現在非使用
	SAVE_V(_bVDCleared); //v1.63追加

	SAVE_A(_VPC); //Kitao追加。v.0.89

	//VCE.c をここへ統合。v1.11
	SAVE_V(_DCC);
	SAVE_V(_ColorTableAddr);

	SAVE_A(_Palette333);
	SAVE_A(_PaletteBG); //v1.10更新
	SAVE_A(_PaletteSP); //v1.10更新

	SAVE_V(_ColorLatch);

	SAVE_A(_VpcPriority); //v0.89追加
	SAVE_V(_VpcBorder1); //v0.89追加
	SAVE_V(_VpcBorder2); //v0.89追加

	return TRUE;
}

/*-----------------------------------------------------------------------------
	[LoadState]
		状態をファイルから読み込みます。 
-----------------------------------------------------------------------------*/
BOOL
VDC_LoadState(
	FILE*		p)
{
	Sint32	rasterTimingType;
	Sint32	autoRasterTimingType;
	BOOL	bPerformSpriteLimit;
	Sint32	waitPatch;
	Sint32	popfulMail;
	Sint32	overClockType;

	if (p == NULL)
		return FALSE;

	if (MAINBOARD_GetStateVersion() < 17) //Kitao追加。v0.89より古いセーブファイルなら
	{
		return OldLoadState(p);
	}

	LOAD_A(_Regs);
	LOAD_A(_VideoRam);
	LOAD_A(_SpRam);

	LOAD_V(_AR);
	LOAD_V(_ReadData); //Kitao追加。v0.72

	LOAD_V(_ScreenW);
	LOAD_V(_ScreenH);
	LOAD_V(_BGH);
	LOAD_V(_BGW);

	LOAD_V(_VdcAddrInc);
	LOAD_V(_VdcStatus);

	LOAD_V(_ClockCounter); //Kitao追加
	LOAD_V(_ScanLine);
	LOAD_V(_RasterCounter);
	LOAD_V(_bRasterRequested); //Kitao追加
	LOAD_V(_bRasterLate); //Kitao追加。v0.62
	LOAD_V(_DisplayCounter);
	LOAD_V(_LineCounter);
	LOAD_V(_LineCountMask);
	if (MAINBOARD_GetStateVersion() >= 28) //Kitao追加。v1.30以降のセーブファイルなら
		LOAD_V(_LineOffset); //v2.09からフレーム終わりで必ず0にリセットされるのでダミー読み込み。
	_LineOffset[0] = 0;
	_LineOffset[1] = 0;
	if (MAINBOARD_GetStateVersion() >= 45) //Kitao追加。v2.47以降のセーブファイルなら
	{
		LOAD_V(_TvStartLine);
		LOAD_V(_TvStartLineAdjust);
	}
	else
	{
		if (MAINBOARD_GetStateVersion() == 44) //Kitao追加。v2.47betaのセーブファイルなら
		{
			LOAD_V(_TvStartLine); //ダミー読み込み
		}
		for (_VN=0; _VN<=_SuperGrafx; _VN++)
		{
			VDC_SetTvStartLine();
			_TvStartLineAdjust[_VN] = 0; //v2.47追加
		}
	}

	LOAD_V(_bUpdateSATB);
	LOAD_V(_bSpOver);
	LOAD_V(_bOverlap);
	LOAD_V(_SpDmaCount);
	if (MAINBOARD_GetStateVersion() < 22) //Kitao追加。v1.00より前のセーブファイルなら
		LOAD_V(_SpDmaStole);//ダミー。旧バージョンの_SpDmaStoleは違う用途で使っていたので破棄。
	_SpDmaStole[0] = 0;
	_SpDmaStole[1] = 0;
	if (MAINBOARD_GetStateVersion() >= 24) //Kitao追加。v1.03以降のセーブファイルなら
		LOAD_V(_SpDmaStole);
	LOAD_V(_VramDmaCount);
	if (MAINBOARD_GetStateVersion() >= 23) //Kitao追加。v1.02以降のセーブファイルなら
	{
		LOAD_V(_bVramDmaExecute);
	}
	else
	{
		_bVramDmaExecute[0] = FALSE;
		_bVramDmaExecute[1] = FALSE;
	}
	LOAD_V(_bBurstMode);
	if (MAINBOARD_GetStateVersion() >= 41) //v2.08以降のセーブファイルなら
	{
		LOAD_V(_bVDCAccessWait);
	}
	else
	{
		_bVDCAccessWait[0] = TRUE;
		_bVDCAccessWait[1] = TRUE;
	}
	LOAD_V(_bMWRchange); //Kitao追加

	if (MAINBOARD_GetStateVersion() < 26) //Kitao追加。v1.11より前のセーブファイルなら
	{
		LOAD_V(_VblankLine);
		LOAD_V(_TvStartLine);
		for (_VN=0; _VN<=_SuperGrafx; _VN++)
		{
			VDC_SetTvStartLine(); //設定しなおす
			_TvStartLineAdjust[_VN] = 0; //v2.47追加
		}
	}
	LOAD_V(rasterTimingType); //Kitao追加
	if (_AutoRasterTimingType != 2) //※_RasterTimingTypeを自動変更してある場合は読み込まず、そのまま_RasterTimingTypeを固定する。
		VDC_SetRasterTiming(_AutoRasterTimingType); //自動変更してあるゲームの場合
	else
		VDC_SetRasterTiming(rasterTimingType); //自動変更していなかった場合
	if (MAINBOARD_GetStateVersion() >= 37) //Kitao追加。v1.65以降のセーブファイルなら
	{
		LOAD_V(autoRasterTimingType);
		if ((autoRasterTimingType != 2)&&(_AutoRasterTimingType == 2)) //旧バージョンでは_AutoRasterTimingTypeを設定していたゲームが、現バージョンではノーマルになった場合。
			VDC_SetRasterTiming(2); //自動でノーマルに戻す
	}
	LOAD_V(overClockType); //Kitao追加
	if ((overClockType < _AutoOverClock)||(_AutoOverClock == 0)) //ゲームごとの自動設定があればそれを優先する。すでにオーバークロックしていた場合はそのまま。ノーマル強制のゲームは強制で(現在未使用)。
		VDC_SetOverClock(_AutoOverClock);
	else
	{
		if (APP_GetLoadStateSpeedSetting()) //速度変更設定を反映する設定の場合。反映しない設定(デフォルト)の場合は、現状のオーバークロック設定を引き継ぐ。誤ってDeleteキーを押して知らずに速度変更してしまうケースがあるのでv2.36からデフォルトでは反映しないようにした。
			VDC_SetOverClock(overClockType);
	}
	LOAD_V(bPerformSpriteLimit); //Kitao追加
	if (!_bAutoPerformSpriteLimit) //※_bPerformSpriteLimitを自動変更してある場合は読み込まず、そのまま_bPerformSpriteLimitを固定する。
		VDC_SetPerformSpriteLimit(bPerformSpriteLimit); //自動変更していなかった場合
	LOAD_V(waitPatch); //Kitao追加。現在ダミー。_WaitPatchはゲーム固有の初期値のままにする。
	if (MAINBOARD_GetStateVersion() >= 24) //Kitao追加。v1.03以降のセーブファイルなら
	{
		LOAD_V(_bIRQ1CancelExecute);//Kitao追加。v1.03。現在非使用。
	}
	else
		_bIRQ1CancelExecute = FALSE;
	if (MAINBOARD_GetStateVersion() >= 36) //Kitao追加。v1.63以降のセーブファイルなら
	{
		LOAD_V(_bVDCleared);
	}
	else
	{
		_bVDCleared[0] = FALSE;
		_bVDCleared[1] = FALSE;
	}

	if (MAINBOARD_GetStateVersion() < 26) //Kitao追加。v1.11より前のセーブファイルなら
		LOAD_V(popfulMail); //ダミー読み込み。

	LOAD_A(_VPC); //Kitao追加。v.0.89

	// remake all the bg and sp tiles
	for (_VN=0; _VN<=_SuperGrafx; _VN++) //Kitao更新
	{
		if (VDW == 0) //v0.95以前の不具合対策。画面の初期化をしていないソフト(hesファイルなど)で0のまま保存してしまっていたので、初期値(239)を入れる。
		{
			VDW = 239;
			VCR = 4;
			VPR = 0x0F02;
		}
		invalidate_tile_cache();
		if (MWR & 0x08)
			_MaxSpPerLine[_VN] = 14; //スプライト横並び14個制限。v2.63追加
		else
			_MaxSpPerLine[_VN] = 16; //スプライト横並び16個制限
	}

	//VCE.c をここへ統合。v1.11
	LOAD_V(_DCC);
	LOAD_V(_ColorTableAddr);
	if ((MAINBOARD_GetStateVersion() >= 3)&&(MAINBOARD_GetStateVersion() <= 25)) //Kitao追加。v0.57～v1.10のセーブファイルなら
	{
		LOAD_V(_TvStartPos); //次のset_TvWidth()で再設定されるため、現在非必要。ダミー読み込み。v1.11
		LOAD_V(_TvWidth); //次のset_TvWidth()で再設定されるため、現在非必要。ダミー読み込み。v1.11
	}
	set_TvWidth(); //Kitao追加。_TvWidth,_TvStartPos,_TvMaxを最新の状態に更新。
	VDC_SetVblankLine(); //v2.48追加。必要
	for (_VN=0; _VN<=_SuperGrafx; _VN++)  //Kitao追加。TvStartLine(垂直表示開始位置)&VBlankLineを更新(スパグラの場合VDC２つとも更新)
		_VblankLineTop[_VN] = _VblankLine[_VN]; //v1.19追加
	LOAD_A(_Palette333);
	LOAD_A(_PaletteBG); //v1.10更新
	LOAD_A(_PaletteSP); //v1.10更新
	LOAD_V(_ColorLatch);

	if (MAINBOARD_GetStateVersion() >= 17) //Kitao追加。v0.89以降のセーブファイルなら
	{
		LOAD_A(_VpcPriority);
		VDC_SetVpcPriority(); //v0.91より前の_VpcProprityは仕様が違うのでここで改めて更新する。
		LOAD_V(_VpcBorder1);
		LOAD_V(_VpcBorder2);
	}

	return TRUE;
}

#undef SAVE_V
#undef SAVE_A
#undef LOAD_V
#undef LOAD_A
