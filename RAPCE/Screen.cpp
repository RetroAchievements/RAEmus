/******************************************************************************
Ootake
・Direct3Dでの描画にも対応した。Vista対応。
・Window表示にもDirectDrawを使うようにした。
・早回し機能を付けた。

Copyright(C)2006-2009 Kitao Nakamura.
	改造版・後継版を公開なさるときは必ずソースコードを添付してください。
	その際に事後でかまいませんので、ひとことお知らせいただけると幸いです。
	商的な利用は禁じます。
	あとは「GNU General Public License(一般公衆利用許諾契約書)」に準じます。

*******************************************************************************
	[Screen.c]

	Implement ScreenInterface.

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
#include <math.h> //pow()関数で必要
#include "Screen.h"
#include "ScreenDD.h"
#include "ScreenD3D.h"
//#include "GDIScreen.h"
#include "TIMER.h"
#include "VDC.h"
#include "CDROM.h"
#include "APU.h"
#include "App.h"
#include "MainBoard.h"
#include "WinMain.h"
#include "Printf.h"

Uint32 _Gamma[8]; //Kitao追加。ガンマを計算した数値を入れておく。v1.14高速化。Uint32にしたほうが処理速かった。
Uint32 _GammaS80[8]; //Kitao追加。スキャンライン80%用
Uint32 _GammaS90[8]; //Kitao追加。スキャンライン90%用
Uint32 _MonoTableR[256]; //モノクロ変換用テーブル。高速化のため必要。v2.28
Uint32 _MonoTableG[256]; //
Uint32 _MonoTableB[256]; //

static Sint32	_Width;
static Sint32	_Height;
static Sint32	_Magnification;	//※ここ(Screen.cpp)での_Magnificationはスクリーンショット用表示処理のときは、それと一致しないことがあるので注意。v2.28記
static Sint32	_BitsPerPixel;  //※ここ(Screen.cpp)での_BitsPerPixelは、"DirectDrawフルスクリーンカラーの設定"の値であって、現在表示中のBitsPerPixelと一致しているとは限らないので注意。v2.28記
static Uint32	_Flags;

//Kitao追加。他アプリのウィンドウの状態を保存しておくための変数。v2.24
static HWND				_OtherAppWindowHWnd[512];
static WINDOWPLACEMENT	_OtherAppWindowPlacement[512];
static Sint32			_OtherAppWindowN;


//Kitao追加。ウィンドウ位置保存のためのコールバック。v2.24
static BOOL
CALLBACK EnumWindowsSaveProc(HWND hWnd, LPARAM lParam)
{
	if ((IsWindowVisible(hWnd))&&(hWnd != WINMAIN_GetHwnd()))
	{
		if (_OtherAppWindowN < 512)
		{
			_OtherAppWindowHWnd[_OtherAppWindowN] = hWnd;
			_OtherAppWindowPlacement[_OtherAppWindowN].length = sizeof(WINDOWPLACEMENT);
			GetWindowPlacement(hWnd, &_OtherAppWindowPlacement[_OtherAppWindowN]);
			_OtherAppWindowN++;
		}
	}
	return TRUE;
}

//Kitao追加。ウィンドウ位置保存。v2.24
void
SCREEN_SaveWindowPosition()
{
	_OtherAppWindowN = 0;
	EnumWindows(EnumWindowsSaveProc, NULL);
	//PRINTF("WindowN %d",_OtherAppWindowN); //test
}

//Kitao追加。ウィンドウ位置を戻すためのコールバック。v2.24
static BOOL
CALLBACK EnumWindowsLoadProc(HWND hWnd, LPARAM lParam)
{
	int		i;	

	for (i=0; i<_OtherAppWindowN; i++)
		if (_OtherAppWindowHWnd[i] == hWnd)
		{
			SetWindowPlacement(hWnd, &_OtherAppWindowPlacement[i]);
			break;
		}
	return TRUE;
}

//Kitao追加。ウィンドウ位置を戻す。v2.24
void
SCREEN_LoadWindowPosition()
{
	EnumWindows(EnumWindowsLoadProc, NULL);
}


/*-----------------------------------------------------------------------------
	[Init]
		スクリーンモードを初期化(変更)します。 Kitao更新。v2.28
-----------------------------------------------------------------------------*/
BOOL
SCREEN_Init(
	Sint32		width,
	Sint32		height,
	Sint32		magnification, //Kitao追加
	Uint32		bitsPerPixel,
	Uint32		flags)
{
	BOOL	ret;
	Uint32	i;

	_Width = width;
	_Height = height;
	_Magnification = magnification; //Kitao追加
	_BitsPerPixel = bitsPerPixel;
	_Flags = flags;

	switch (APP_GetDrawMethod())
	{
		case 1: //Direct3D
			SCREEND3D_Deinit();
			ret = SCREEND3D_Init(_Width, _Height, _Magnification, _Flags);
			break;
		case 2: //DirectDraw
			SCREENDD_Deinit();
			ret = SCREENDD_Init(_Width, _Height, _Magnification, _BitsPerPixel, _Flags);
			break;
		default:
			ret = FALSE;
			break;
	}

	//モノクロ変換用テーブルを作成。v2.28追加
	if ((APP_GetDrawMethod() == 2)&&(SCREENDD_GetBitsPerPixel() == 16))
	{	//16bitカラー（DirectDrawのみ）
		for (i=0; i<32; i++)
		{
			//R,G,Bの輝度を平均化してモノクロ化
			_MonoTableR[i] = (Uint32)((pow((i * 0.298912) / 32.0, 1.0/1.076900) * 32.0)); //※画面の暗さを抑えるためガンマも上げる。
			_MonoTableG[i] = (Uint32)((pow((i * 0.586611) / 32.0, 1.0/1.076900) * 32.0)); //※RGB足したときに値オーバーしないために小数点以下は切り捨て。
			_MonoTableB[i] = (Uint32)((pow((i * 0.114478) / 32.0, 1.0/1.076900) * 32.0)); //※16bitは切り捨てられたぶんの暗さが大きいのでそこも考慮してガンマを決定。
		}
	}
	else
	{	//32bitカラー
		for (i=0; i<256; i++)
		{
			//R,G,Bの輝度を平均化してモノクロ化
			_MonoTableR[i] = (Uint32)((pow((i * 0.298912) / 256.0, 1.0/1.0752080) * 256.0)); //※画面の暗さを抑えるためガンマも上げる。
			_MonoTableG[i] = (Uint32)((pow((i * 0.586611) / 256.0, 1.0/1.0752080) * 256.0)); //※RGB足したときに値オーバーしないために小数点以下は切り捨て。
			_MonoTableB[i] = (Uint32)((pow((i * 0.114478) / 256.0, 1.0/1.0752080) * 256.0)); //
		}
	}

	return ret;
}


/*-----------------------------------------------------------------------------
	[Deinit]
		スクリーンの終了処理を行ないます。
-----------------------------------------------------------------------------*/
void
SCREEN_Deinit()
{
	switch (APP_GetDrawMethod())
	{
		case 1: //Direct3D
			SCREEND3D_Deinit();
			break;
		case 2: //DirectDraw
			SCREENDD_Deinit();
			break;
	}
}


/*-----------------------------------------------------------------------------
	[ToggleFullScreen]
		スクリーンをウインドウ／フルスクリーンに切り替えます．
-----------------------------------------------------------------------------*/
BOOL
SCREEN_ToggleFullScreen()
{
	if (_Flags & SCREEN_FFULLSCREEN)
		_Flags &= ~SCREEN_FFULLSCREEN;
	else
		_Flags |= SCREEN_FFULLSCREEN;
	switch (APP_GetDrawMethod())
	{
		case 1: //Direct3D
			SCREEND3D_Deinit();
			return SCREEND3D_Init(_Width, _Height, _Magnification, _Flags);
		case 2: //DirectDraw
			SCREENDD_Deinit();
			return SCREENDD_Init(_Width, _Height, _Magnification, _BitsPerPixel, _Flags);
		default:
			return FALSE;
	}
}


/*-----------------------------------------------------------------------------
	[WaitVBlank]
		垂直帰線期間を待ちます。 
-----------------------------------------------------------------------------*/
//Kitao更新
BOOL
SCREEN_WaitVBlank(
	BOOL	bDraw) //bDrawをTRUEにして呼ぶと描画も行う。FALSEの場合VSync待ちのみ。Direct3D利用時用。Kitao追加。
{
	switch (APP_GetDrawMethod())
	{
		case 1: //Direct3D
			return SCREEND3D_WaitVBlank(bDraw);
		case 2: //DirectDraw
			return SCREENDD_WaitVBlank(FALSE); //DirectDrawのときは常に描画は行われない。
		default:
			return FALSE;
	}
}


void*
SCREEN_GetBuffer()
{
	switch (APP_GetDrawMethod())
	{
		case 1: //Direct3D
			return SCREEND3D_GetBuffer();
		case 2: //DirectDraw
			return SCREENDD_GetBuffer();
		default:
			return NULL;
	}
}


const Sint32
SCREEN_GetBufferPitch()
{
	switch (APP_GetDrawMethod())
	{
		case 1: //Direct3D
			return SCREEND3D_GetBufferPitch();
		case 2: //DirectDraw
			return SCREENDD_GetBufferPitch();
		default:
			return 0;
	}
}


/*-----------------------------------------------------------------------------
	[FillRect]
		バックバッファに指定の色の矩形を描きます。
	呼ぶ前に SCREEN_Lock() しましょう。
-----------------------------------------------------------------------------*/
void
SCREEN_FillRect(
	Sint32		x,
	Sint32		y,
	Sint32		width,
	Sint32		height,
	Uint32		color)
{
	switch (APP_GetDrawMethod())
	{
		case 1: //Direct3D
			SCREEND3D_FillRect(x, y, width, height, color);
			break;
		case 2: //DirectDraw
			SCREENDD_FillRect(x, y, width, height, color);
			break;
	}
}


//Kitao追加。スクリーン全体をクリアする。v1.43
void
SCREEN_Clear()
{
	switch (APP_GetDrawMethod())
	{
		case 1: //Direct3D
			SCREEND3D_Clear();
			break;
		case 2: //DirectDraw
			SCREENDD_Clear();
			break;
	}
}


/*-----------------------------------------------------------------------------
	[Blt]
		pSrc からバックバッファへ画像を書き込みます。拡大／縮小、
	呼ぶ前に SCREEN_Lock() しましょう。
-----------------------------------------------------------------------------*/
//Kitao更新。ラインごとに解像度を変えているゲーム(龍虎の拳，あすか120%など)に対応。
void
SCREEN_Blt(
	Uint32*		pSrc,
	Sint32		srcX,
	Sint32		srcY,
	Uint16*		pSrcW,	//Kitao更新。転送元の横ピクセル数。※srcHラインの数ぶん
	Sint32		srcH,	//Kitao更新。dstWとdstH はカットした。(ここで固定せず、様々な大きさでのペーストに対応するため)
	Sint32		executeCode)  //Kitao追加。実行コード。0…エンコードだけ行う。1…プライマリ画面へ転送も行う。
							  //					   3…左右に黒帯(オーバースキャン部)を配置しての転送(あとは1と同じ)
							  //					   5…左右のオーバースキャン部をカットしての転送(あとは1と同じ)
{
	switch (APP_GetDrawMethod())
	{
		case 1: //Direct3D
			SCREEND3D_Blt(pSrc, srcX, srcY, pSrcW, srcH, executeCode);
			break;
		case 2: //DirectDraw
			SCREENDD_Blt(pSrc, srcX, srcY, pSrcW, srcH, executeCode);
			break;
	}
}


//Kitao追加。VSync(垂直帰線待ち)を行うかどうかを設定。現在のディスプレイ表示環境でVSyncが行えるかどうかのチェックも行う。
void
SCREEN_SetSyncTo60HzScreen(
	BOOL	bSyncTo60HzScreen)
{
	switch (APP_GetDrawMethod())
	{
		case 1: //Direct3D
			SCREEND3D_SetSyncTo60HzScreen(bSyncTo60HzScreen);
			break;
		case 2: //DirectDraw
			SCREENDD_SetSyncTo60HzScreen(bSyncTo60HzScreen);
			break;
	}
}

//Kitao追加。VSync(垂直帰線待ち)を行っているかどうかを得る（現在のディスプレイ表示環境でVSyncが行えるかどうかのチェックを反映した値）。
BOOL
SCREEN_GetSyncTo60HzScreen()
{
	BOOL	bSyncTo60HzScreen;

	switch (APP_GetDrawMethod())
	{
		case 1: //Direct3D
			bSyncTo60HzScreen = SCREEND3D_GetSyncTo60HzScreen();
			break;
		case 2: //DirectDraw
			bSyncTo60HzScreen = SCREENDD_GetSyncTo60HzScreen();
			break;
	}

	return bSyncTo60HzScreen;
}


//Kitao追加。テキストメッセージを設定
void
SCREEN_SetMessageText(
	char*	pText)
{
	switch (APP_GetDrawMethod())
	{
		case 1: //Direct3D
			SCREEND3D_SetMessageText(pText);
			break;
		case 2: //DirectDraw
			SCREENDD_SetMessageText(pText);
			break;
	}
}


//Kitao追加。ガンマ（明るさ調整）を計算済みのテーブルを用意。v2.28更新。Direct3DとDirectDrawで共用にした。
void
SCREEN_SetGamma(
	Sint32	scanLineType,
	Sint32	scanLineDensity, //スキャンラインの濃度(%)
	BOOL	bTvMode)
{
	Sint32	magnification;
	Sint32	bitsPerPixel;
	int 	a,i;
	double	d = APP_GetGammaValue(); //縦横スキャンライン時の基本ガンマ値
	Sint32	b = APP_GetBrightValue(); //ブライトネス

	//基本設定ではなく、現在実際に表示されているMagnificationとBitsPerPixelを取得する。v2.28
	switch (APP_GetDrawMethod())
	{
		case 1: //Direct3D
			magnification = SCREEND3D_GetMagnification();
			bitsPerPixel = 32; //32bit固定
			break;
		case 2: //DirectDraw
			magnification = SCREENDD_GetMagnification();
			bitsPerPixel = SCREENDD_GetBitsPerPixel();
			break;
		default:
			magnification = 2;
			bitsPerPixel = 32;
			break;
	}

	if (bTvMode)
	{
		if ((scanLineType != 0)&&(magnification >= 2)) //TV Mode でx2,x3,x4のとき
			scanLineType = 4;
	}
	else
	{
		if (scanLineType == 4) //スタートTV Mode が設定してあって、TvModeが一時的にFALSEの状態なら
			scanLineType = 1; //スペシャル(縦横)スキャンラインで描画
	}

	if ((scanLineType != 0)&&(magnification >= 2)) //スキャンラインの場合、スキャンラインで暗くなるぶん、ガンマを明るめに。
	{
		switch (scanLineType)
		{
			case 1: //縦横スキャンライン
				break;
			case 2: //横だけスキャンライン
			case 3: //縦だけスキャンライン(未実装)
			case 4: //TV Mode
				d = (1-15/800)*d; // (1-15/800)*1.305 縦横時と比べて明るいぶんを引く
				if (APP_GetOptimizeGamma())
					d = d * (1+(80-(double)scanLineDensity)*0.005); // ダウンした明るさ分を上げる。v2.35更新
				break;
			default:
				d = (1-95/800)*d;
				break;
		}
	}
	else //ノンスキャンラインもしくはx1の場合
		d = (1-95/800)*d; // (1-95/800)*1.305 縦横時と比べて明るいぶんを引く

	if ((scanLineType>=2)&&(scanLineType<=4)) //横スキャンラインの場合。v2.35更新
	{
		if (bitsPerPixel == 16)
		{
			for (i=0; i<=7 ; i++)
			{
				if (i == 0)
					a = 0; //黒は真っ黒に
				else
					a = (i << 2) + b; //+1。白が白く見える範囲で小さめにしたほうが目にきつくない。
				_Gamma[i] = (Uint32)((pow((double)a / 32.0, 1.0/d) * 32.0) +0.5);
				_GammaS80[i] = (Uint32)((pow((double)a*((double)scanLineDensity / 100) / 32.0, 1.0/d) * 32.0) +0.5); //スキャンライン本線用
				_GammaS90[i] = (Uint32)((pow((double)a*((double)(scanLineDensity+(100-scanLineDensity)/2) / 100) / 32.0, 1.0/d) * 32.0) +0.5); //スキャンラインとドットの境界用
			}
		}
		else //32ビットカラーの場合
		{
			for (i=0; i<=7 ; i++)
			{
				if (i == 0)
					a = 0; //黒は真っ黒に
				else
					a = (i << 5) + b; //+1。白が白く見える範囲で小さめにしたほうが目にきつくない。
				_Gamma[i] = (Uint32)((pow((double)a / 256.0, 1.0/d) * 256.0) +0.5);
				_GammaS80[i] = (Uint32)((pow((double)a*((double)scanLineDensity / 100) / 256.0, 1.0/d) * 256.0) +0.5); //スキャンライン本線用
				_GammaS90[i] = (Uint32)((pow((double)a*((double)(scanLineDensity+(100-scanLineDensity)/2) / 100) / 256.0, 1.0/d) * 256.0) +0.5); //スキャンラインとドットの境界用
			}
		}
	}
	else //横スキャンライン以外の場合。横スキャンライン80%を基準とする。v2.35更新
	{
		if (bitsPerPixel == 16)
		{
			for (i=0; i<=7 ; i++)
			{
				a = i*4 + (Sint32)((double)b/9 * ((double)i/7) + 0.5); //黒は真っ黒に。白は白く見える範囲で小さめにしたほうが目にきつくない。v2.35更新
				_Gamma[i] = (Uint32)((pow((double)a / 32.0, 1.0/d) * 32.0) +0.5);
				_GammaS80[i] = (Uint32)((pow((double)a*0.80 / 32.0, 1.0/d) * 32.0) +0.5); //スキャンライン80%用（スキャンライン本線）
				_GammaS90[i] = (Uint32)((pow((double)a*0.90 / 32.0, 1.0/d) * 32.0) +0.5); //スキャンライン90%用（スキャンラインとドットの境界用）
			}
		}
		else //32ビットカラーの場合
		{
			for (i=0; i<=7 ; i++)
			{
				a = i*32 + (Sint32)((double)b * ((double)i/7) + 0.5); //黒は真っ黒に。白は白く見える範囲で小さめにしたほうが目にきつくない。v2.35更新
				_Gamma[i] = (Uint32)((pow((double)a / 256.0, 1.0/d) * 256.0) +0.5);
				_GammaS80[i] = (Uint32)((pow((double)a*0.80 / 256.0, 1.0/d) * 256.0) +0.5); //スキャンライン80%用（スキャンライン本線）
				_GammaS90[i] = (Uint32)((pow((double)a*0.90 / 256.0, 1.0/d) * 256.0) +0.5); //スキャンライン90%用（スキャンラインとドットの境界用）
			}
		}
	}
}


//Kitao追加。前回のVBlank待ちが終わった時刻を返す。
DWORD
SCREEN_GetLastTimeSyncTime()
{
	switch (APP_GetDrawMethod())
	{
		case 1: //Direct3D
			return SCREEND3D_GetLastTimeSyncTime();
		case 2: //DirectDraw
			return SCREENDD_GetLastTimeSyncTime();
		default:
			return 0;
	}
}


//Kitao追加。スクリーンショットのBitmapを書き込む。v2.12
void
SCREEN_WriteScreenshot(
	FILE*	fp)
{
	switch (APP_GetDrawMethod())
	{
		case 1: //Direct3D
			SCREEND3D_WriteScreenshot(fp);
			break;
		case 2: //DirectDraw
			SCREENDD_WriteScreenshot(fp);
			break;
	}
}


//Kitao追加。描画倍率を設定する。v2.36
void
SCREEN_SetMagnification(
	Sint32	magnification)
{
	switch (APP_GetDrawMethod())
	{
		case 1: //Direct3D
			SCREEND3D_SetMagnification(magnification);
			break;
		case 2: //DirectDraw
			SCREENDD_SetMagnification(magnification);
			break;
	}
}


//Kitao追加。実際に「描画処理で使用している」描画倍率を得る。v2.36
Sint32
SCREEN_GetMagnification()
{
	switch (APP_GetDrawMethod())
	{
		case 1: //Direct3D
			return SCREEND3D_GetMagnification();
		case 2: //DirectDraw
			return SCREENDD_GetMagnification();
	}
	return 0;
}


//Kitao追加。描画時のドット拡大率を設定する。
void
SCREEN_SetPixelMagnification(
	Sint32*		wMag,
	Sint32*		hMag)
{
	Sint32	magnification;

	//基本設定ではなく、現在実際に表示されているMagnificationを取得する。v2.28
	switch (APP_GetDrawMethod())
	{
		case 1: //Direct3D
			magnification = SCREEND3D_GetMagnification();
			break;
		case 2: //DirectDraw
			magnification = SCREENDD_GetMagnification();
			break;
		default:
			magnification = 2;
			break;
	}

	if (magnification >= 2) //x2以上の場合
	{
		if (APP_GetScanLineType() != 0)
		{
			if (APP_GetTvMode())
				*wMag = 1;
			else
				*wMag = 2; //x2以上も横は２倍ドット固定。（速度アップ＋拡大されたときにバイリニアフィルタがいい感じにかかる）
			*hMag = magnification; //縦は倍率ぶんのソースを用意して転送。
		}
		else
		{
			*wMag = 2;
			if (magnification == 2)
				*hMag = magnification; //縦は倍率ぶんのソースを用意して転送。
			else
				*hMag = magnification-1; //3x,4xのときは、それぞれ2x,3xに拡大。（ジャギー軽減＆速度アップ）
		}
	}
	else //x1の場合
		*wMag = *hMag = 1;
}


//Kitao追加。ディスプレイの垂直走査周波数を測定して返す。v2.43
Sint32
SCREEN_GetVerticalScanFrequency()
{
	Sint32	vsf = 0;
	Sint32	a;
	DWORD	t, t2;
	BOOL	bHideMessage;

	if (APP_GetFullScreen())
	{
		bHideMessage = APP_GetFullHideMessage(); //退避
		APP_SetFullHideMessage(FALSE); //メッセージを必ず画面内に表示。
	}
	else
	{
		bHideMessage = APP_GetHideMessage(); //退避
		APP_SetHideMessage(FALSE); //表示速度が間に合うように、メッセージを必ず画面内に表示。
	}
	PRINTF("Checking Now... Please wait for 60 seconds.");
	MAINBOARD_ScreenUpdateFast();

	SetCursor(LoadCursor(NULL, IDC_WAIT)); //カーソルを砂時計に。

	SCREEN_WaitVBlank(FALSE); //"前回VBlankが終わった時刻"を更新するために必要。
	t = timeGetTime();
	t2 = t + 60000;
	while (t2 < t) //終了予定時刻のタイマーカウンタがオーバーフローしていた場合、開始予定時刻のタイマーカウンタが0に戻るまで待つ。
	{
		SCREEN_WaitVBlank(FALSE); //"前回VBlankが終わった時刻"を更新するために必要。
		t = timeGetTime();
		t2 = t + 60000;
	}
	
	while (timeGetTime() <= t2)
	{
		vsf++;
		Sleep(12); //12。連続V-Sync処理による計測不良を防ぐためウェイトが必要(とくにV-Sync設定オフ時に必要)。11だと計測不良(早送り現象)があった。あまり大きくSleepさうると遅いマシンで逆に処理落ちによる計測不良。
		SCREEN_WaitVBlank(FALSE);
	}

	vsf -= 6; //誤差があるのか、-6でちょうど実際の周波数表記(ディスプレイのinfoボタン)と同じになった。全ての環境でこれが当てはまらないと意味がないので要確認。
	vsf = vsf * 10 / 6;
	//四捨五入
	a = vsf % 10; //一の位を退避
	vsf /= 10; //一の位をカット
	if (a < 2)
		vsf *= 10;
	else if (a < 7)
		vsf = vsf * 10 + 5;
	else
		vsf = vsf * 10 + 10;

	//メッセージ表示設定を元に戻す。
	if (APP_GetFullScreen())
		APP_SetFullHideMessage(bHideMessage);
	else
		APP_SetHideMessage(bHideMessage);

	SetCursor(LoadCursor(NULL, IDC_ARROW)); //カーソルを元に戻す

	return vsf;
}
