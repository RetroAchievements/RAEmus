/******************************************************************************
Ootake
・Direct3D用にScreenD3D.cを作ったため、DirectDraw用はScreenDD.cに名前を変更し
  た。(名前順にファイルを並べたとき、修正などがしやすいように)
・Window表示にもDirectDrawを使うようにした。
・実機をTVにつなげたときの画面の明るさに近づけるため、ガンマを上げて画面を明る
  くし見やすくした。
・画面を拡大するときに、ドットのジャギーが目立たないよう、かつ画面がぼやけてし
  まわないように、独自の拡大処理(明るめのスキャンラインを入れる)で拡大するよう
  にした。
・TVモードを追加した。v0.91
・ディスプレイのリフレッシュレートが60Hzではなかった場合、自動的にVSyncをオフに
  するようにした。
・DirectDrawでVSync待ちを行う際にCPUの利用率が100%状態になるので、CPUパワーを抑
  える省電力モード(音質がやや落ちる)を付けた。
・ラインごとに解像度を変えるゲーム(龍虎の拳，あすか120%等)に対応した。v0.60
・上記の[ラインごとに解像度を変える処理]で重くなっていたので、ラインごとの解像
  度変更を行っていない通常のゲームでは、その処理を省くことで高速化した。v1.06
・スクリーン上にテキストメッセージの表示ができるようにした。v0.64
・DirectDrawのV-Syncで、GetScanLine()を使うようにした。環境によってサウンドにノ
  イズが入ることへの対策。v0.95。←オーバーレイを使った他のソフトを同時に起動し
  たときに逆に大きなノイズが定期的に載ってしまったので元に戻した。v0.97
・オフスクリーン描画用のメモリをシステムメモリに取ることも可能にした。ジャギー
  は目立つがクッキリした画質が好みの場合に利用。v1.31

Copyright(C)2006-2010 Kitao Nakamura.
	改造版・後継版を公開なさるときは必ずソースコードを添付してください。
	その際に事後でかまいませんので、ひとことお知らせいただけると幸いです。
	商的な利用は禁じます。
	あとは「GNU General Public License(一般公衆利用許諾契約書)」に準じます。

*******************************************************************************
	[SCREENDD.c]

	Implement ScreenInterface using Direct Draw.

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

#define DIRECTDRAW_VERSION	0x0900	//Kitao更新。ドライバ環境にもよるかもしれないが、DirectDraw8以降専用にしたほうが、割り込み処理にきちんと時間を割いてくれるのか音質が大きく上がった。
									//			 8より9のほうが全体は軽めだが8が最高音質だった。DirectX8のプログラマさん(もしくはnVidiaの旧世代ドライバ)は描画以外の部分もより安定して動作するように作ってくれていたのかも。
#define SOURCE_PITCH 		512		//Kitao追加

#include <ddraw.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include "ScreenDD.h"
#include "Screen.h"
#include "WinMain.h"
#include "Printf.h"
#include "App.h"
#include "MainBoard.h"

extern Uint32 _Gamma[8]; //Kitao追加。ガンマを計算した数値を入れておく。v1.14高速化。Uint32にしたほうが処理速かった。
extern Uint32 _GammaS80[8]; //Kitao追加。スキャンライン80%用
extern Uint32 _GammaS90[8]; //Kitao追加。スキャンライン90%用
extern Uint32 _MonoTableR[256]; //モノクロ変換用テーブル。高速化のため必要。v2.28
extern Uint32 _MonoTableG[256]; //
extern Uint32 _MonoTableB[256]; //

/*--------------------------------------------------------------------------*
 *                           Static variables                               *
 *--------------------------------------------------------------------------*/
static LPDIRECTDRAW				_pDD         = NULL;  // DirectDraw object
static LPDIRECTDRAWCLIPPER		_pDC         = NULL;  // Kitao追加。DirectDraw clipper object
static LPDIRECTDRAWSURFACE		_pDDSPrimary = NULL;  // DirectDraw primary surface
static LPDIRECTDRAWSURFACE		_pDDSBack    = NULL;  // DirectDraw offscreen surface for BackBuffer
static LPDIRECTDRAWSURFACE		_pDDSText    = NULL;  // Kitao追加。DirectDraw offscreen surface for Text
static LPDIRECTDRAWSURFACE		_pDDSFps     = NULL;  // Kitao追加。DirectDraw offscreen surface for FPS
static LPDIRECTDRAWSURFACE		_pDDS256     = NULL;  // Kitao追加。DirectDraw offscreen surface for 256WidthScreen

//Kitao追加。最初に設定した値を残しておいて高速化。
static	DDSURFACEDESC	_ddsdPrimary;
static	DDSURFACEDESC	_ddsdBack;
static	DDSURFACEDESC	_ddsd256;

static LONG		_SurfacePitch = 0;

static Sint32	_Width;  // width of display
static Sint32	_Height; // height of display
static Sint32	_Magnification; //Kitao追加
static Uint32	_Flags;
static HWND		_hWnd; //Kitao追加
static Uint16*	_pScreenPixels;
static BOOL		_bChangedFullScreen = FALSE; //Kitao追加

//Win9x用変数。Kitao追加
static DWORD	_PrevWidth;
static DWORD	_PrevHeight;
static DWORD	_PrevBitsPerPixel;
static DWORD	_PrevFrequency;

static DWORD	_LastTimeSyncTime; //Kitao追加。前回Syncしたときのタイマーカウント
static DWORD	_LastTimeSyncTime3; //3フレーム単位での、前回Syncしたときのタイマーカウント。v2.43
static DWORD	_Frame; //3フレーム単位を計るためでの、前回Syncしたときのタイマーカウント。v2.43
static Sint32	_SyncAjust; //Kitao追加。VSyncがオフのときに1/60秒間隔にできるだけ近づけるための変数。v1.67
static Sint32	_SyncAjust3; //v2.43追加
static BOOL		_bSyncTo60HzScreen = TRUE; //Kitao追加

static Sint32	_BitsPerPixel;
static Uint32	_Rmask;
static Uint32	_Gmask;
static Uint32	_Bmask;

static Uint32	_Rshift;
static Uint32	_Gshift;
//static Uint32	_Bshift;

//static BOOL	_bLocked = FALSE; //Kitao更新。プライマリをロックする必要がなくなったのでカット。v1.39

static LONG		_Pitch;
static LONG		_Pitch2; //Kitao追加。 //pitchの2倍の値を入れておく。速度アップ用。
static LONG		_Pitch3; //Kitao追加。 //pitchの3倍の値を入れておく。速度アップ用。
static LONG		_Pitch4; //Kitao追加。 //pitchの4倍の値を入れておく。速度アップ用。
static Uint32*	_pPixels; //v2.13更新。32bit単位で扱うようにした。(高速化)

static char*	_pMessageText = ""; //Kitao追加。テキストメッセージ表示用バッファ。ヌル（先頭が0）の場合、未表示。


/*-----------------------------------------------------------------------------
	[Deinit]
		Finished with all objects we use; release them
-----------------------------------------------------------------------------*/
void
SCREENDD_Deinit()
{
	if (_pDD != NULL)
	{
		if (_pDC != NULL) //Kitao追加
		{
			_pDDSPrimary->SetClipper(NULL);
			_pDC->Release();
			_pDC = NULL;
		}

		if (_pDDS256 != NULL) //Kitao追加
		{
			_pDDS256->Release();
			_pDDS256 = NULL;
		}

		if (_pDDSFps != NULL) //Kitao追加
		{
			_pDDSFps->Release();
			_pDDSFps = NULL;
		}

		if (_pDDSText != NULL) //Kitao追加
		{
			_pDDSText->Release();
			_pDDSText = NULL;
		}

		if (_pDDSBack != NULL)
		{
			_pDDSBack->Release();
			_pDDSBack = NULL;
		}

		if (_pDDSPrimary != NULL)
		{
			_pDDSPrimary->Release();
			_pDDSPrimary = NULL;
		}

		_pDD->Release();
		_pDD = NULL;
	}
}


static Uint32
get_shift(
	Uint32		mask)
{
	Uint32		i;

	for (i = 0; i < 32; i++)
		if (mask & (1 << i))
			return i;

	return 0;
}


/*-----------------------------------------------------------------------------
	[Init]
		スクリーンを初期化します。 
-----------------------------------------------------------------------------*/
BOOL
SCREENDD_Init(
	Sint32		width,
	Sint32		height,
	Sint32		magnification, //Kitao追加
	Sint32		bitsPerPixel,
	Uint32		flags) //Kitao追加
{
	HRESULT			hRet;
	DDSURFACEDESC	ddsd;
	HWND			hWnd = WINMAIN_GetHwnd();
	int 			a; //Kitao追加
	DEVMODE			dm; //Kitao追加

	hRet = DirectDrawCreate(NULL, &_pDD, NULL);
	if (hRet != DD_OK)
	{
		MessageBox(hWnd, "ERROR: DIRECTDRAW::DirectDrawCreate() failed.    ", "Ootake", MB_OK); //Kitao追加
		return FALSE;
	}

	// 協調レベルを設定する。 
	if (flags & SCREEN_FFULLSCREEN) //Kitao追加。フルスクリーンモードなら
		hRet = _pDD->SetCooperativeLevel(hWnd, DDSCL_ALLOWREBOOT | DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN);
	else//Windowモードなら
		hRet = _pDD->SetCooperativeLevel(hWnd, DDSCL_NORMAL);
	if (hRet != DD_OK)
	{
		MessageBox(hWnd, "ERROR: DIRECTDRAW::SetCooperativeLevel() failed.    ", "Ootake", MB_OK); //Kitao更新
		return FALSE;
	}

	// 画面の解像度を設定する。
	//  Kitao更新。ウィンドウスタイルの変更は画面の解像度を変えてからにした。
	//			   低解像度から高解像度に切り替えた場合に、他のアプリのウィンドウが縮んでしまうことがあった問題を解消。v0.95
	if (flags & SCREEN_FFULLSCREEN) //Kitao追加。フルスクリーンモードなら
	{
		if (_bChangedFullScreen == FALSE) //ウィンドウ(デスクトップ)→フルスクリーンへの切り替え時
		{
			//他のアプリのウィンドウの状態を保存しておく。v2.24
			SCREEN_SaveWindowPosition();
			//Windows98/Meの場合、CDS_FULLSCREENが効かなく、デスクトップに戻れないので、手動で元に戻すため変更前の設定を退避しておく。
			if (APP_GetWindows9x())
			{
				memset(&dm, 0, sizeof(DEVMODE));
				dm.dmSize = sizeof(DEVMODE);
				EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm);
				_PrevWidth = dm.dmPelsWidth;
				_PrevHeight = dm.dmPelsHeight;
				_PrevBitsPerPixel = dm.dmBitsPerPel;
				_PrevFrequency = dm.dmDisplayFrequency;
			}
		}
		memset(&dm, 0, sizeof(DEVMODE));
		dm.dmSize = sizeof(DEVMODE);
		dm.dmPelsWidth = width;
		dm.dmPelsHeight = height;
		dm.dmBitsPerPel = bitsPerPixel;
		dm.dmDisplayFrequency = 60;
		dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_DISPLAYFREQUENCY;
		if ((APP_GetResolutionAutoChange())&&(_bChangedFullScreen))
			ChangeDisplaySettings(&dm, 0); //設定を控えずに切替。一部の環境で切替時のもたつきを解消。v2.23。古いRADEONで切り替え時に画像の乱れ解消確認済み。v2.65
		else //通常
			ChangeDisplaySettings(&dm, CDS_FULLSCREEN);
		WINMAIN_SetFullScreenWindow(width, height);//v2.23追加。このタイミングで、ウィンドウスタイルの変更を行うようにした。パワーの無いマシンではDirectX初期化に時間が掛かるのでこのタイミングで画面を整えたほうが良さそう。
		_bChangedFullScreen = TRUE;
	}
	else //Kitao追加。ウィンドウモードなら
	{
		if (_bChangedFullScreen) //フルスクリーン→ウィンドウへの切り替え時
		{
			if (APP_GetWindows9x()) //Windows98/Meの場合、CDS_FULLSCREENが効かなく、自動でデスクトップを元の状態に戻せないので、手動で元に戻す。
			{
				memset(&dm, 0, sizeof(DEVMODE));
				dm.dmSize = sizeof(DEVMODE);
				dm.dmPelsWidth = _PrevWidth;
				dm.dmPelsHeight = _PrevHeight;
				dm.dmBitsPerPel = _PrevBitsPerPixel;
				dm.dmDisplayFrequency = _PrevFrequency;
				dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_DISPLAYFREQUENCY;
				ChangeDisplaySettings(&dm, 0);		
			}
			else //Windows XP/2000/Vista以降の場合	
				ChangeDisplaySettings(NULL, 0); //デスクトップを元に戻す
			SCREEN_LoadWindowPosition(); //他のアプリのウィンドウ状態を元に戻す。v2.24
			Sleep(1000); //環境によっては、デスクトップに戻るまでウェイトを入れないとDIRECTDRAWをCreate出来ないことがある。v1.61
		}
		WINMAIN_SetNormalWindow(width, height);//v2.23追加。このタイミングで、ウィンドウスタイルの変更を行うようにした。パワーの無いマシンではDirectX初期化に時間が掛かるのでこのタイミングで画面を整えたほうが良さそう。
		APP_WindowWithinScreen(); //ウィンドウが画面からはみ出していた場合、画面内に収まるように移動する。
		_bChangedFullScreen = FALSE;
	}

	// プライマリサーフェスを作成する
	ZeroMemory(&_ddsdPrimary, sizeof(_ddsdPrimary));
	_ddsdPrimary.dwSize = sizeof(_ddsdPrimary);
	_ddsdPrimary.dwFlags = DDSD_CAPS;
	_ddsdPrimary.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_VIDEOMEMORY;
	hRet = _pDD->CreateSurface(&_ddsdPrimary, &_pDDSPrimary, NULL);
	if (hRet != DD_OK)
	{
		MessageBox(hWnd, "ERROR: DIRECTDRAW::CreateSurface() failed.    ", "Ootake", MB_OK); //Kitao更新
		SCREENDD_Deinit();
		return FALSE;
	}

    // オフスクリーンサーフェスを作成する
	ZeroMemory(&_ddsdBack, sizeof(_ddsdBack));
	_ddsdBack.dwSize = sizeof(_ddsdBack);
	_ddsdBack.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
	if (APP_GetUseVideoCardMemory())
		_ddsdBack.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY;
	else
		_ddsdBack.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY; //システムメモリを利用(シャープ＆重い)。v1.31追加
	if (magnification == 1)
	 a = 512;
	else //2倍以上は全て横2倍。速度アップ＋こうすることで適度にバイリニアフィルタが掛かりいい感じになる
	 a = 512*2;
	_ddsdBack.dwWidth = a;
	_ddsdBack.dwHeight = 256*magnification; //オーバースキャン用に256まで用意。上側のオーバースキャン領域は241～256ライン目の領域を使用する。v1.43
	hRet = _pDD->CreateSurface(&_ddsdBack, &_pDDSBack, NULL);
	if (hRet != DD_OK)
	{
		MessageBox(hWnd, "ERROR: DIRECTDRAW::CreateSurface(Back) failed.    ", "Ootake", MB_OK); //Kitao更新
		SCREENDD_Deinit();
		return FALSE;
	}

    // Kitao追加。テキスト表示用オフスクリーンサーフェスを作成する
	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY; //テキストはボヤけて読みにくくならないようにシステムメモリ上に作成
	ddsd.dwWidth = 512*2;
	ddsd.dwHeight = 32;
	_pDD->CreateSurface(&ddsd, &_pDDSText, NULL);

    // Kitao追加。FPS表示用オフスクリーンサーフェスを作成する
	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY; //文字がボヤけて読みにくくならないようにシステムメモリ上に作成
	ddsd.dwWidth = 64*2;
	ddsd.dwHeight = 32;
	_pDD->CreateSurface(&ddsd, &_pDDSFps, NULL);

    // Kitao追加。横256ドット汎用表示用オフスクリーンサーフェスを作成する
	ZeroMemory(&_ddsd256, sizeof(_ddsd256));
	_ddsd256.dwSize = sizeof(_ddsd256);
	_ddsd256.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
	_ddsd256.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY; //文字がボヤけて読みにくくならないようにシステムメモリ上に作成
	_ddsd256.dwWidth = 256;
	_ddsd256.dwHeight = 224;
	_pDD->CreateSurface(&_ddsd256, &_pDDS256, NULL);

	if (!flags & SCREEN_FFULLSCREEN) //Kitao追加。ウィンドウモードなら
	{
		//クリッパーを用意 
		hRet = _pDD->CreateClipper(0, &_pDC, NULL);
		if (hRet != DD_OK)
		{
			MessageBox(hWnd, "ERROR: DIRECTDRAW::CreateClipper failed.    ", "Ootake", MB_OK); //Kitao更新
			SCREENDD_Deinit();
			return FALSE;
		}
		//クリッパーにウィンドウを関連付け
		hRet = _pDC->SetHWnd(0, hWnd);
		if (hRet != DD_OK)
		{
			MessageBox(hWnd, "ERROR: DIRECTDRAW::SetHWnd failed.    ", "Ootake", MB_OK); //Kitao更新
			SCREENDD_Deinit();
			return FALSE;
		}
		//サーフェスにクリッパーを関連付け
		hRet = _pDDSPrimary->SetClipper(_pDC);
		if (hRet != DD_OK)
		{
			MessageBox(hWnd, "ERROR: DIRECTDRAW::SetClipper failed.    ", "Ootake", MB_OK); //Kitao更新
			SCREENDD_Deinit();
			return FALSE;
		}
     }

	// Get pixel format 
	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_PIXELFORMAT | DDSD_CAPS;
	_pDDSPrimary->GetSurfaceDesc(&ddsd);

	_BitsPerPixel = ddsd.ddpfPixelFormat.dwRGBBitCount;
	_Rmask = ddsd.ddpfPixelFormat.dwRBitMask;
	_Gmask = ddsd.ddpfPixelFormat.dwGBitMask;
	_Bmask = ddsd.ddpfPixelFormat.dwBBitMask;

	// Rshift, Gshift, Bshift を求める。Kitao更新
	_Rshift = get_shift(_Rmask);
	if (_Rshift == 10) //RGB555のとき
		_Gshift = 5;
	else //RGB565のとき
		_Gshift = 6;

	// Save our screen resolution
	_Width = width;
	_Height = height;
	_Magnification = magnification;//Kitao追加
	_Flags = flags;
	_hWnd =	hWnd;

	//Kitao追加。ガンマ（明るさ調整）を計算済みのテーブルを用意。
	SCREEN_SetGamma(APP_GetScanLineType(), APP_GetScanLineDensity(), APP_GetTvMode());

	//Kitao追加。画面クリア用に使うため、オフスクリーン256をクリアしておく。
	SCREENDD_FillRect(0,0,256,224,0);

	_LastTimeSyncTime = timeGetTime(); //Kitao追加。前回Syncしたときのタイマーカウント
	_LastTimeSyncTime3 = _LastTimeSyncTime; //v2.43追加
	_SyncAjust  = 0; //VSyncがオフのときに1/60秒間隔にできるだけ近づけるための変数。v1.67
	_SyncAjust3 = 0; //v2.43追加
	_Frame = 0; //v2.43追加

	return TRUE;
}


/*-----------------------------------------------------------------------------
	[ChangeMode]
		スクリーンモードを変更します。 
-----------------------------------------------------------------------------*/
BOOL
SCREENDD_ChangeMode(
	Sint32		width,
	Sint32		height,
	Sint32		magnification, //Kitao追加
	Sint32		bitsPerPixel,
	Uint32		flags)
{
	SCREENDD_Deinit();
	return SCREENDD_Init(width, height, magnification, bitsPerPixel, flags);
}


/*-----------------------------------------------------------------------------
	[WaitVBlank]
		垂直帰線期間を待ちます。 
-----------------------------------------------------------------------------*/
//垂直帰線期間を待ちます。(VSync処理)
BOOL
SCREENDD_WaitVBlank(
	BOOL	bDraw) //bDraw…Direct3D用に追加したパラーメータ。TRUEに設定してもここでは常に描画は行われない。Kitao追加
{
	Sint32	t;

	//Kitao更新
	if (_bSyncTo60HzScreen)
	{
		t = timeGetTime() - _LastTimeSyncTime;
		if (t <= 16) //v1.09追加。17ms(1/60s=16.7ms)以上経過していた場合は待たない。(timeGetTime()の誤差があるのでt=17でもまだ16.1ms経過のこともあるが二度待ちしないことを優先）
		{
			//Kitao追加。他のアプリのために処理を一休み。※これをやらないとWaitForVerticalBlank()で待っている間にCPU占有率が100%になる。
			t = 16 - t - 11; //-11は処理落ちしないためのマージン。-11OK。-10ほぼOK。-9以下だとテンポの揺らぎからわずかに音が篭る気がする。16=１コマの時間約16ms。Sleep()はどうしても大きな誤差が出るときがあるようだ。
			if ((t >= 1)&&(t <= 16-11)) //タイマーカウンタのオーバーフローも考えて、この範囲のときだけSleepをおこなう。
				Sleep(t);
			//待つ
			_pDD->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, NULL);
		}
		_LastTimeSyncTime = timeGetTime(); //前回のSyncが終わったタイムとして記録しておく
		return TRUE;
	}
	else //_bSyncTo60HzScreen が FALSEの場合。Kitao更新。現状60fps専用としてシンプル化した。v1.67
	{
		t = timeGetTime() - _LastTimeSyncTime;
		if (t <= 16) //17ms(1/60s=16.7ms)以上経過していた場合は待たない。(timeGetTime()の誤差があるのでt=17でもまだ16.1ms経過のこともあるが二度待ちしないことを優先）
		{
			//待つ
			t = 16 - t; //16=１コマの時間約16ms
			if (_SyncAjust == 0)
				t--; //Sleepの誤差で処理落ちさせないためのマージン。ここで16.7ms以上経過してしまうと12フレーム(200.0ms)毎の帳尻合わせがうまくいかないため、これが必要。
			if ((t >= 1)&&(t <= 16)) //タイマーカウンタのオーバーフローも考えて、この範囲のときだけSleepをおこなう。
				Sleep(t);
			if (++_SyncAjust == 4)
				_SyncAjust = 0; //4回に1回だけSleepを短めにする。これで多くの環境で59.9Hz前後に近づいた(うちの環境)。
		}
		if ((_Frame % 12) == 11) //Sleepの誤差を減らすため、12フレーム(200.0ms)に１回、帳尻合わせを行う。v2.43
		{
			t = timeGetTime() - _LastTimeSyncTime3;
			if (t <= 200) //201ms以上経過していた場合は待たない
			{
				//待つ
				t = 200 - t; //200=12コマの時間200.0ms。
				if (_SyncAjust3 == 0)
					t--; //これで59.95Hzに近づいた(うちの環境)。
				if (t>16) t=16;
				if ((t >= 1)&&(t <= 16)) //タイマーカウンタのオーバーフローも考えて、この範囲のときだけSleepをおこなう。
					Sleep(t);
				if (++_SyncAjust3 == APP_GetNonVSyncTiming())
					_SyncAjust3 = 0; //10(APP_GetNonVSyncTiming())回に1回だけSleepを短めにする。これで多くの環境で59.94Hz前後に近づいた(うちの環境)。
			}
			_LastTimeSyncTime3 = timeGetTime(); //3フレームごとのSyncが終わったタイムとして記録しておく
		}
		_LastTimeSyncTime = timeGetTime(); //前回のSyncが終わったタイムとして記録しておく
		_Frame++;
		return TRUE;
	}
}


void*
SCREENDD_GetBuffer()
{
	return (void*)_pScreenPixels;
}


const Sint32
SCREENDD_GetBufferPitch()
{
	return _SurfacePitch/2;
}


//Kitao追加。文字表示、塗りつぶし等用のプライマリサーフェスへ転送する処理。現在、ソース解像度256x224固定。
static void
zoom_ddTensou256()
{
	Sint32		w = 256;
	Sint32		h = 224;
	Sint32		a,b;
	RECT		rc;
	RECT		rcDst;
	RECT		rcSrc;

	//転送先の位置調整
	if (_Flags & SCREEN_FFULLSCREEN)
		SetRect(&rcDst, 0, 0, _Width, _Height);
	else //ウィンドウモードのとき
	{
		GetWindowRect(_hWnd, &rc);
		a = rc.left + (rc.right - rc.left - _Width)/2;
		b = rc.bottom - (rc.right - rc.left - _Width)/2 - _Height; //枠の太さと表示領域の高さぶんを引く
		SetRect(&rcDst, a, b, a+_Width, b+_Height);
	}

	//転送元の位置調整
	SetRect(&rcSrc, 0, 0, w, h);

	//プライマリサーフェスへ転送する
	if (_pDDSPrimary->Blt(&rcDst, _pDDS256, &rcSrc, DDBLT_ASYNC, NULL) == DDERR_SURFACELOST)
		_pDDSPrimary->Restore();
}


//Kitao更新。ddsdの初期化(100バイト強のメモリアクセス)を省略して高速化した(v1.02更新)。v2.13更新。32bit単位で扱うようにした。
static BOOL
lock_offscreen_surface(
	LONG*				pPitch,
	Uint32**			ppPixels)
{
	HRESULT				hRet;
	int					trial = 10;

	while (trial--)
	{
		hRet = _pDDSBack->Lock(NULL, &_ddsdBack, DDLOCK_WAIT, NULL);
		if (hRet == DD_OK)
		{
			*pPitch = _ddsdBack.lPitch >> 2; //横１ラインのバイト数。32bit単位で扱うために4で割っておく。v2.13更新
			*ppPixels = (Uint32*)_ddsdBack.lpSurface;
			return TRUE;
		}
		else if (hRet == DDERR_SURFACELOST)
		{
			_pDDSBack->Restore();
		}
    }

	return FALSE;
}


//Kitao追加
static BOOL
lock_screen256_surface(
	LONG*				pPitch,
	Uint16**			ppPixels)
{
	HRESULT				hRet;
	int					trial = 10;

	while (trial--)
	{
		hRet = _pDDS256->Lock(NULL, &_ddsd256, DDLOCK_WAIT, NULL);
		if (hRet == DD_OK)
		{
			*pPitch = _ddsd256.lPitch;
			*ppPixels = (Uint16*)_ddsd256.lpSurface;
			return TRUE;
		}
		else if (hRet == DDERR_SURFACELOST)
		{
			_pDDS256->Restore();
		}
    }

	return FALSE;
}


/*-----------------------------------------------------------------------------
	[FillRect]
		バックバッファに指定の色の矩形を描きます。
-----------------------------------------------------------------------------*/
//Kitao変更。バックバッファに書き込む方式で書き直した。
//※現在、x,yは0固定。ソース解像度256x224固定。0フィル専用。32ビットカラー用。現在は0フィルなので16ビットモードでも使える。
void
SCREENDD_FillRect(
	Sint32		x,
	Sint32		y,
	Sint32		width,
	Sint32		height,
	Uint32		color)
{
	LONG		pitch;
	LONG		pitchHalf;
	Uint16*		pPixels;
	Uint16*		pPixels0;
	int			i,j;

	if (lock_screen256_surface(&pitch, &pPixels))
	{
		pitchHalf = pitch / 2;
		pPixels0 = pPixels;
		for (i = 0; i < 224; i++)
		{
			pPixels = pPixels0;
			if (_BitsPerPixel == 16)
			{
				for (j = 0; j < 256; j++)
					*pPixels++ = 0x00; //2byteぶん(１ドット)書き込まれる。
			}
			else
			{
				for (j = 0; j < 256; j++)
				{
					*pPixels++ = 0x00;
					*pPixels++ = 0x00; //4byteぶん(１ドット)書き込まれる。
				}
			}
			pPixels0 += pitchHalf;
		}
		
		_pDDS256->Unlock(NULL);
	}
}


//Kitao追加。画面全体をクリアする。v1.43
void
SCREENDD_Clear()
{
	//SCREENDD_FillRect()で作ったクリアイメージをプライマリサーフェスへ転送する。
	zoom_ddTensou256();
}


//Kitao追加。テキストメッセージを設定
void
SCREENDD_SetMessageText(
	char*	pText)
{
	_pMessageText = pText;
}


//Kitao追加。テキストメッセージの表示
static void
print_message(
	Sint32	executeCode) //v2.00更新
{
	HDC			dc;
	HFONT		prevFont;
	HFONT		hFont;
	LOGFONT		logFont;
	RECT		rcSrc;
	RECT		rcDst;
	RECT		rc;
	BOOL		bTvMode;
	int			a,fs;
	Sint32		hMagnification; //縦の表示倍率

	if (!APP_GetInit())
		return; //最初の初期化が全て完了していない場合は、表示処理を行わない。CD-ROMゲームをレジュームしたときにメッセージ欄がごちゃつくのを回避。v1.67

	//デバイスコンテキストを取得
	if (_pDDSText->GetDC(&dc) != DD_OK)
		return;

	//フォントサイズを決定
	if (_Magnification == 1)
		bTvMode = FALSE;
	else
		bTvMode = APP_GetTvMode();
	if ((bTvMode)||(_Magnification == 1))
		fs = 12; //フォントサイズ。v1.09更新
	else
		fs = 16;
	if (_Magnification <= 2)
	{
		if ((!bTvMode)&&((!APP_GetUseVideoCardMemory())||(APP_GetWindowsVista()))) //※x2のTVモード時はフォントが小さくてもOK。Vistaの場合ビデオメモリでもスムージングが掛からないので同様にフォントを大きくする。
		{	//システムメモリを使用している場合、256より大きな解像度で文字が崩れるのでフォントを大きくする必要がある。v1.53
			if (VDC_GetTvWidth() >= 512)
				fs = 32;
			else if (VDC_GetTvWidth() > 256)
				fs = 28;
		}
		else
		{	//x2以下の場合、横512時に文字が読みづらいので大きくする
			if ((!bTvMode)&&(VDC_GetTvWidth() >= 512))
				fs = 28;
		}
	}
	if ((_Magnification == 3)&&(!bTvMode)&&
		((!APP_GetUseVideoCardMemory())||(APP_GetWindowsVista()))&&(VDC_GetTvWidth() >= 512)) //x3(非TVモード)でシステムメモリ＆横512以上の場合。Vistaの場合ビデオメモリでもスムージングが掛からないので同様にフォントを大きくする。
			fs = 28;
	hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	GetObject(hFont, sizeof(LOGFONT), &logFont);
	logFont.lfHeight = fs;
	logFont.lfWidth = 0;
	hFont = CreateFontIndirect(&logFont);
	prevFont = (HFONT)SelectObject(dc, hFont);
	//文字列をバッファに描画
	rc.left		= 0;
	rc.right	= 0;
	rc.top		= 0;
	rc.bottom	=fs;
	SetBkColor(dc, RGB(48,96,48));
	SetTextColor(dc, RGB(255,255,255));
	DrawText(dc, _pMessageText, lstrlen(_pMessageText), &rc,
			 DT_CALCRECT | DT_SINGLELINE | DT_NOPREFIX); //表示シミュレーション。rcに表示したときのサイズが入る。
	DrawText(dc, _pMessageText, lstrlen(_pMessageText), &rc, DT_SINGLELINE | DT_NOPREFIX); //表示

	SelectObject(dc, prevFont);
	DeleteObject(hFont);
	_pDDSText->ReleaseDC(dc);
	
	//転送元位置＆サイズの調整
	rcSrc.left	= 0;
	rcSrc.top	= 0;
	rcSrc.right	= rc.right;
	a = VDC_GetTvWidth();
	if (APP_GetOverscanHideBlackLR())
		a += 16;
	else
		a += MAINBOARD_GetShowOverscanLeft()+MAINBOARD_GetShowOverscanRight();
	if (_Magnification > 1)
		a *= 2;
	if (rcSrc.right > a)
		rcSrc.right = a;
	rcSrc.bottom= fs;
	//転送先位置の調整＆表示した文字の範囲を見やすくするために拡大する。
	if (fs == 12)
	{
		if (APP_GetScanLineType() == 0) //ノンスキャンライン
		{
			if ((bTvMode)&&(_Magnification >= 2))
			{
				if (_Magnification == 4)
					hMagnification = 3;
				else
					hMagnification = 2;
			}
			else
				hMagnification = 1;
		}
		else //ノンスキャンライン以外
			hMagnification = _Magnification;
		rcDst.top = 4;
		rcDst.bottom = 4 + fs*hMagnification;
		if (_Magnification == 1)
			rcDst.left = 3;
		else
			rcDst.left = 2;
	}
	else
	{
		if (APP_GetScanLineType() == 0) //ノンスキャンライン
		{
			if (_Magnification == 4)
				hMagnification = 3;
			else
				hMagnification = 2;
		}
		else //ノンスキャンライン以外
			hMagnification = _Magnification;
		rcDst.top = 4;
		if ((_Magnification == 4)||
			((_Magnification == 3)&&(APP_GetScanLineType() != 0)))
				rcDst.bottom = rcDst.top + fs*2;
		else
			rcDst.bottom = rcDst.top + fs;
		rcDst.left = 3 -_Magnification/4;
	}
	if ((APP_GetFullStretched(TRUE))&&(_Flags & SCREEN_FFULLSCREEN)) //フルストレッチモードなら。v2.00追加
	{
		rcDst.top += 16*hMagnification;	   //上側オーバースキャン領域部分を足す
		rcDst.bottom += 16*hMagnification; //
	}
	else
	{
		rcDst.top += (16-MAINBOARD_GetShowOverscanTop())*hMagnification;	//上側オーバースキャン領域部分を足す
		rcDst.bottom += (16-MAINBOARD_GetShowOverscanTop())*hMagnification; //
	}
	if ((!APP_GetStretched())&&((_Flags & SCREEN_FFULLSCREEN) == 0)) //ウィンドウモードでストレッチをしない場合。v2.14更新
	{
		a = APP_GetNonstretchedWidth();
		//PRINTF("Test = %d",a);
		if ((a < 256)||
			((a > 256)&&(a < 336)))
		{
			if (_Magnification > 1)
				a = (a*2 / 2) - (rcSrc.right / 2);
			else
				a = (a / 2) - (rcSrc.right / 2);
			if (a > rcDst.left)
				rcDst.left = a; //左端を切らさないために中央に表示
		}
		else if (MAINBOARD_GetShowOverscanLeft() > 0) //左右のオーバースキャン領域を表示しているなら。v2.15追加
		{
			if (_Magnification == 1)
				rcDst.left += 8;
			else
				rcDst.left += 8*2;
		}
	}
	else if (executeCode == 5) //オーバースキャン領域の左右をカットする場合
	{
		if (_Magnification == 1)
			rcDst.left += 6;
		else
			rcDst.left += 6*2;
	}
	rcDst.right	= rcDst.left + rcSrc.right;
	if ((APP_GetOverscanHideBlackLR())&&(MAINBOARD_GetShowOverscanLeft() == 0)) //左右のオーバースキャン領域に黒帯を付けるなら
	{
		rcDst.right -= rcDst.left;
		rcDst.left = 0; //黒帯があるのでギリギリまで左に寄せても見栄えOK。
	}
	if ((APP_GetOverscanHideBlackTop())&&(MAINBOARD_GetShowOverscanTop() < 8)) //上側のオーバースキャン領域に黒帯を付けるなら
	{
		if ((8-MAINBOARD_GetShowOverscanTop())*hMagnification >= 4) //黒帯部分が4ドット以上あった場合
		{
			a = (16-MAINBOARD_GetShowOverscanTop())*hMagnification;
			rcDst.bottom -= (rcDst.top - a);
			rcDst.top =	a; //黒帯があるのでギリギリまで上に寄せても見栄えOK。
		}
	}
	if (MAINBOARD_GetFourSplitScreen()) //妖怪道中記,ワールドコート,はにいいんざすかい,パワードリフト,サイコチェイサーの４分割画面の場合。v2.27更新
	{	//左側の黒帯部分を考慮
		if ((_Magnification == 1)||(bTvMode))
		{
			rcDst.left  += 256/2;
			rcDst.right += 256/2;
		}
		else
		{
			rcDst.left  += 512/2;
			rcDst.right += 512/2;
		}
	}
	//転送
	if (_pDDSBack->Blt(&rcDst, _pDDSText, &rcSrc, DDBLT_ASYNC, NULL) == DDERR_SURFACELOST)
		_pDDSBack->Restore();
}


//Kitao追加。FPSの表示
static void
print_fps(
	Sint32	executeCode) //v2.00更新
{
	HDC			dc;
	HFONT		prevFont;
	HFONT		hFont;
	LOGFONT		logFont;
	RECT		rcSrc;
	RECT		rcDst;
	RECT		rc;
	BOOL		bTvMode;
	int			fs;
	char		text[3+1]; //一応３桁まで用意
	int			a;
	Sint32		hMagnification; //縦の表示倍率

	//デバイスコンテキストを取得
	if (_pDDSFps->GetDC(&dc) != DD_OK)
		return;

	//フォントサイズを決定
	if (_Magnification == 1)
		bTvMode = FALSE;
	else
		bTvMode = APP_GetTvMode();
	if ((bTvMode)||(_Magnification == 1))
		fs = 12; //フォントサイズ。v1.09更新
	else
		fs = 16;
	if (_Magnification <= 2)
	{
		if ((!bTvMode)&&((!APP_GetUseVideoCardMemory())||(APP_GetWindowsVista()))) //※x2のTVモード時はフォントが小さくてもOK。Vistaの場合ビデオメモリでもスムージングが掛からないので同様にフォントを大きくする。
		{	//システムメモリを使用している場合、256より大きな解像度で文字が崩れるのでフォントを大きくする必要がある。v1.53
			if (VDC_GetTvWidth() >= 512)
				fs = 32;
			else if (VDC_GetTvWidth() > 256)
				fs = 28;
		}
		else
		{	//x2以下の場合、横512時に文字が読みづらいので大きくする
			if ((!bTvMode)&&(VDC_GetTvWidth() >= 512))
				fs = 28;
		}
	}
	if ((_Magnification == 3)&&(!bTvMode)&&
		((!APP_GetUseVideoCardMemory())||(APP_GetWindowsVista()))&&(VDC_GetTvWidth() >= 512)) //x3(非TVモード)でシステムメモリ＆横512以上の場合。Vistaの場合ビデオメモリでもスムージングが掛からないので同様にフォントを大きくする。
			fs = 28;
	hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	GetObject(hFont, sizeof(LOGFONT), &logFont);
	logFont.lfHeight = fs;
	logFont.lfWidth = 0;
	hFont = CreateFontIndirect(&logFont);
	prevFont = (HFONT)SelectObject(dc, hFont);
	//文字列をバッファに描画
	rc.left		= 0;
	rc.right	= 0;
	rc.top		= 0;
	rc.bottom	=fs;
	SetBkColor(dc, RGB(48,96,48));
	SetTextColor(dc, RGB(255,255,255));
	a = MAINBOARD_GetDisplayedFrames();
	if (a > 999) a = 999;
	sprintf(text, "%d", a);
	DrawText(dc, text, lstrlen(text), &rc,
			 DT_CALCRECT | DT_SINGLELINE | DT_NOPREFIX); //表示シミュレーション。rcに表示したときのサイズが入る。
	DrawText(dc, text, lstrlen(text), &rc, DT_SINGLELINE | DT_NOPREFIX); //表示
	
	SelectObject(dc, prevFont);
	DeleteObject(hFont);
	_pDDSFps->ReleaseDC(dc);
	
	//転送元位置＆サイズの調整
	rcSrc.left	= 0;
	rcSrc.top	= 0;
	rcSrc.right	= rc.right;
	rcSrc.bottom= fs;
	//転送先位置の調整＆表示した文字の範囲を見やすくするために拡大する。
	if (fs == 12)
	{	//"x1" or "TV mode"
		if (APP_GetScanLineType() == 0) //ノンスキャンライン
		{
			if ((bTvMode)&&(_Magnification >= 2))
			{
				if (_Magnification == 4)
					hMagnification = 3;
				else
					hMagnification = 2;
			}
			else
				hMagnification = 1;
		}
		else //ノンスキャンライン以外
			hMagnification = _Magnification;
		rcDst.bottom = 224*hMagnification - 3; //下側に表示
	    rcDst.top = rcDst.bottom - fs*hMagnification;
		if (_Magnification == 1)
			rcDst.left = 3;
		else
			rcDst.left = 2;
	}
	else
	{
		if (APP_GetScanLineType() == 0) //ノンスキャンライン
		{
			if (_Magnification == 4)
				hMagnification = 3;
			else
				hMagnification = 2;
		}
		else //ノンスキャンライン以外
			hMagnification = _Magnification;
		rcDst.bottom = 224*hMagnification - 3; //下側に表示
		if ((_Magnification == 4)||
			((_Magnification == 3)&&(APP_GetScanLineType() != 0)))
			    rcDst.top = rcDst.bottom - fs*2;
		else
		    rcDst.top = rcDst.bottom - fs;
		rcDst.left = 3 -_Magnification/4;
	}
	rcDst.top += (16+MAINBOARD_GetShowOverscanBottom())*hMagnification;	   //上下のオーバースキャン領域部分を足す。上は16ドットぶん必ず足す。
	rcDst.bottom += (16+MAINBOARD_GetShowOverscanBottom())*hMagnification; //
	if ((!APP_GetStretched())&&((_Flags & SCREEN_FFULLSCREEN) == 0)) //ウィンドウモードでストレッチをしない場合。v2.14更新
	{
		a = APP_GetNonstretchedWidth();
		//PRINTF("Test = %d",a);
		if ((a < 256)||
			((a > 256)&&(a < 336)))
		{
			if (_Magnification > 1)
				a = (a*2 / 2) - (rcSrc.right / 2);
			else
				a = (a / 2) - (rcSrc.right / 2);
			if (a > rcDst.left)
				rcDst.left = a; //左端を切らさないために中央に表示
		}
		else if (MAINBOARD_GetShowOverscanLeft() > 0) //左右のオーバースキャン領域を表示しているなら。v2.15追加
		{
			if (_Magnification == 1)
				rcDst.left += 8;
			else
				rcDst.left += 8*2;
		}
	}
	else if (executeCode == 5) //オーバースキャン領域の左右をカットする場合
	{
		if (_Magnification == 1)
			rcDst.left += 6;
		else
			rcDst.left += 6*2;
	}
	rcDst.right	= rcDst.left + rcSrc.right;
	if ((APP_GetOverscanHideBlackLR())&&(MAINBOARD_GetShowOverscanLeft() == 0)) //左右のオーバースキャン領域に黒帯を付けるなら
	{
		rcDst.right -= rcDst.left;
		rcDst.left = 0; //黒帯があるのでギリギリまで左に寄せても見栄えOK。
	}
	if ((APP_GetOverscanHideBlackBottom())&&(MAINBOARD_GetShowOverscanBottom() < 8)) //下側のオーバースキャン領域に黒帯を付けるなら
	{
		if ((8-MAINBOARD_GetShowOverscanBottom())*hMagnification >= 3) //黒帯部分が3ドット以上あった場合
		{
			a = rcDst.bottom - (16+224+MAINBOARD_GetShowOverscanBottom())*hMagnification;
			rcDst.bottom -= a;
			rcDst.top -= a; //黒帯があるのでギリギリまで下に寄せても見栄えOK。
		}
	}
	if (MAINBOARD_GetFourSplitScreen()) //妖怪道中記,ワールドコート,はにいいんざすかい,パワードリフト,サイコチェイサーの４分割画面の場合。v2.27更新
	{	//左側の黒帯部分を考慮
		if ((_Magnification == 1)||(bTvMode))
		{
			rcDst.left  += 256/2;
			rcDst.right += 256/2;
		}
		else
		{
			rcDst.left  += 512/2;
			rcDst.right += 512/2;
		}
	}
	//転送
	if (_pDDSBack->Blt(&rcDst, _pDDSFps, &rcSrc, DDBLT_ASYNC, NULL) == DDERR_SURFACELOST)
		_pDDSBack->Restore();
}


//Kitao追加。プライマリサーフェスへ転送する処理など。v2.00更新。上側のオーバースキャンライン領域もここで転送するようにした。
static void
zoom_ddTensou(
	Uint16*		pTvW,	//転送元の横ピクセル数。※[0]～[239]の224+16(下部のOverscan)ラインぶん。ここには[0](先頭)のアドレスを入れる。
	Sint32		h,		//転送元の縦ピクセル数。通常は224。下部のオーバースキャンをする場合そのぶん増やしてここを呼ぶ。
	Sint32		executeCode)  //実行コード。0…エンコードだけ行う。1…プライマリ画面へ転送も行う。2…スクリーンショット用画面へ転送を行う。
							  //		    3…左右に黒帯(オーバースキャン部)を配置しての転送(あとは1と同じ)。4…スクリーンショット後の画面復元転送を行う。
							  //v2.12更新   5…左右のオーバースキャン部をカットしての転送(あとは1と同じ)
{
	RECT		rcSrc;
	RECT		rcDst;
	RECT		rc;
	Sint32		a,b,c,d,s,s2,t,i,w2,h2;
	BOOL		bRasterChange = FALSE; //ラスタごとに異なる解像度が設定されていた場合TRUE
	Uint16*		pTvW2 = pTvW;
	Sint32		ot = 16-MAINBOARD_GetShowOverscanTop(); //転送元の開始Y座標。v2.00追加

	if (executeCode != 2) //スクリーンショット用へ転送以外のとき
	{
		//テキストメッセージの表示
		if (*_pMessageText != 0) //メッセージが設定されていれば
			print_message(executeCode);

		//FPSの表示。v1.50追加
		if (APP_GetShowFPS())
			print_fps(executeCode);
	}

	//転送元の横幅調整 
	s = (*pTvW2);
	for (i=0; i<h; i++)
	{
		if ((*pTvW2) != s)
		{
			bRasterChange = TRUE;
			if ((*pTvW) < s)
				s = (*pTvW); //s=一番小さい横幅。フルストレッチ時にごみが出ないように。
		}
		*pTvW2++;
	}

	//転送先の位置調整 
	if (_Flags & SCREEN_FFULLSCREEN)
	{
		if (APP_GetFullStretched(TRUE))
		{
			//アスペクト比を4:3に保つ
			if (_Width/4 >= _Height/3)
			{	//縦のほうが短いので縦を基準とする
				w2 = (Sint32)((double)(_Height / 3.0 * 4.0 + 0.5));
				a = (_Width - w2) / 2;
				if ((MAINBOARD_GetShowOverscanLeft() > 0)&&((*(pTvW+MAINBOARD_GetShowOverscanTop()+223)) != 512)&&(executeCode != 3)) //左右のオーバースキャン領域を表示するなら
					b = (_Height - (Sint32)((double)_Height * 0.96137 + 0.5)) / 2; //0.96137=224/((268-256)/4*3+224)
				else
					b = 0;
				c = _Width - a;
				d = _Height - b;
			}
			else
			{	//横のほうが短いので横を基準とする
				h2 = (Sint32)((double)(_Width / 4.0 * 3.0 + 0.5));
				a = 0;
				if ((MAINBOARD_GetShowOverscanLeft() > 0)&&((*(pTvW+MAINBOARD_GetShowOverscanTop()+223)) != 512)&&(executeCode != 3)) //左右のオーバースキャン領域を表示するなら
					b = (_Height - (Sint32)((double)h2 * 0.96137 + 0.5)) / 2; //0.96137=224/((268-256)/4*3+224)
				else
					b = (_Height - h2) / 2;
				c = _Width;
				d = _Height - b;
			}
			//フルストレッチ時は上下のオーバースキャン領域を表示できないため、オーバースキャン領域をカット。
			h -= MAINBOARD_GetShowOverscanTop() + MAINBOARD_GetShowOverscanBottom();
			ot = 16;
			bRasterChange = FALSE; //フルストレッチ時はラスタごとの解像度変更ができない。
		}
		else if (APP_GetStretched())
		{
			if ((MAINBOARD_GetShowOverscanLeft() > 0)&&(executeCode != 3)) //左右のオーバースキャン領域を表示するなら
				w2 = (Sint32)(313.24 * (double)_Magnification + 0.5);
			else
				w2 = 299 * _Magnification; //アスペクト比固定のため横は299(2倍時598)の表示範囲。
			if (APP_GetVStretched()) //縦画面モードなら
				w2 = (Sint32)((double)w2 * (256.0/336.0)); //実機同様に256:336の比で縮小する（横256以外のゲームも同様に可能）
			//アスペクト比をTV画面と同じ4:3に。縦の長さををPCE標準224の整数倍(２倍)にする
			a = (_Width - w2) / 2;
			b = (_Height - 224*_Magnification) / 2;
			c = a + w2;
			d = b + 224*_Magnification;
			if (MAINBOARD_GetShowOverscanTop() > 0) //上側のオーバースキャン領域を表示するなら
			{
				b -= MAINBOARD_GetShowOverscanTop() * _Magnification;
				if (b < 0) //画面からはみ出してしまう場合。1440x900,1600x900などであり得る。
				{	//アスペクト比を保つため、オーバースキャン部をカット。
					b = (_Height - 224*_Magnification) / 2;
					h -= MAINBOARD_GetShowOverscanTop();
					ot = 16;
				}
			}
			if (MAINBOARD_GetShowOverscanBottom() > 0) //下側のオーバースキャン領域を表示するなら
			{
				d += MAINBOARD_GetShowOverscanBottom() * _Magnification;
				if (d > _Height) //画面からはみ出してしまう場合。1440x900,1600x900などであり得る。
				{	//アスペクト比を保つため、オーバースキャン部をカット。
					d = b + 224*_Magnification;
					h -= MAINBOARD_GetShowOverscanBottom();
				}
			}
		}
		else //ストレッチをしない場合
		{
			w2 = (*pTvW)*_Magnification;
			if (w2 > _Width) w2 = _Width; //画面からはみ出してしまう場合は、ストレッチ(縮小)して対応。
			a = (_Width - w2) / 2;
			b = (_Height - 224*_Magnification) / 2;
			c = a + w2;
			d = b + 224*_Magnification;
			if (MAINBOARD_GetShowOverscanTop() > 0) //上側のオーバースキャン領域を表示するなら
			{
				b -= MAINBOARD_GetShowOverscanTop() * _Magnification;
				if (b < 0) b = 0; //画面からはみ出してしまう場合は、ストレッチ(縮小)して対応。1440x900,1600x900などであり得る。
			}
			if (MAINBOARD_GetShowOverscanBottom() > 0) //下側のオーバースキャン領域を表示するなら
			{
				d += MAINBOARD_GetShowOverscanBottom() * _Magnification;
				if (d > _Height) d = _Height; //画面からはみ出してしまう場合は、ストレッチ(縮小)して対応。1440x900,1600x900などであり得る。
			}
		}
	}
	else //ウィンドウモードのとき
	{
		GetWindowRect(_hWnd, &rc);
		a = rc.left + (rc.right - rc.left - _Width)/2;
		b = rc.bottom - (rc.right - rc.left - _Width)/2 - _Height; //枠の太さと表示領域の高さぶんを引く
		if (APP_GetOverscanHideBlackTop()) //上側のオーバースキャン領域に黒帯を入れて隠す設定なら
			b += (8 - MAINBOARD_GetShowOverscanTop()) * _Magnification;
		c = a + APP_GetGameWidth(_Magnification);
		d = b + (MAINBOARD_GetShowOverscanTop() + 224 + MAINBOARD_GetShowOverscanBottom()) * _Magnification;
		if (((MAINBOARD_GetShowOverscanLeft() > 0)&&((*(pTvW+MAINBOARD_GetShowOverscanTop()+223)) == 512))|| //512のときはオーバースキャン表示しない
			((MAINBOARD_GetShowOverscanLeft() == 0)&&(APP_GetOverscanHideBlackLR()))||
			(executeCode == 3))
		{	//左右に黒帯を配置
			if (APP_GetVStretched()) //縦画面モードなら
			{
				a += (Sint32)(5.34 * (double)_Magnification + 0.5);		//5.34=299/256*(256/336)*6dot
				c -= (Sint32)(5.34 * (double)_Magnification + 0.5);		//v2.00更新。gccだと-1してちょうどの値だったがVisualC++では-1しないのがピッタリくる。
			}
			else if (APP_GetStretched())
			{
				a += (Sint32)(7.01 * (double)_Magnification + 0.5);		//7.01=299/256*6dot
				c -= (Sint32)(7.01 * (double)_Magnification + 0.5);		//v2.00更新。gccだと-1してちょうどの値だったがVisualC++では-1しないのがピッタリくる。
			}
		}
	}

	//垂直帰線期間を待つ
	if (executeCode != 4) //スクリーンショット後の復元時以外の場合
		SCREENDD_WaitVBlank(FALSE);

	if (IsIconic(_hWnd)) //ウィンドウが最小化されているとき。hesを最小化して聴くときなどに処理を軽くする。
		if (executeCode != 2) //スクリーンショット用へ転送以外の場合
			return;

	//描画解像度の変更があった場合、ゴミが残らないように画面全体をクリアする。
	if (MAINBOARD_GetResolutionChange())
			zoom_ddTensou256(); //プライマリサーフェスへ転送する

	//プライマリサーフェスへ転送する
	if (!bRasterChange)
	{	//通常
		s2 = 1;
		if (_Magnification >= 2) //ｘ２以上の場合
		{
			if (APP_GetScanLineType() != 0)
			{
				if (!APP_GetTvMode())
					s2 = 2; //横は２倍ドット固定。（速度アップ＋拡大されたときにバイリニアフィルタがいい感じにかかる）
				h  *= _Magnification; //縦は倍率ぶんのソースを用意して転送。
				ot *= _Magnification; //
			}
			else
			{
				s2 = 2;
				if (_Magnification == 2)
				{
					h  *= _Magnification; //縦は倍率ぶんのソースを用意して転送。
					ot *= _Magnification; //
				}
				else
				{
					h  *= _Magnification-1; //3x,4xのときは、それぞれ2x,3xに拡大。（ジャギー軽減＆速度アップ）
					ot *= _Magnification-1; //
				}
			}
		}
		if (MAINBOARD_GetFourSplitScreen()) //妖怪道中記,ワールドコート,はにいいんざすかい,パワードリフト,サイコチェイサーの４分割画面の場合。v2.27更新
		{		
			SetRect(&rcSrc, s/4*s2, ot, (s/4+s/2)*s2, ot+h); //ソースの左右黒帯部分をカット
			//４分の１に縮小して４回転送
			c = a + (c-a)/2;
			d = b + (d-b)/2;
			w2 = c - a;
			h2 = d - b;
			for (i=1; i<=4; i++)
			{
				SetRect(&rcDst, a, b, c, d);
				if (_pDDSPrimary->Blt(&rcDst, _pDDSBack, &rcSrc, DDBLT_ASYNC, NULL) == DDERR_SURFACELOST)
					_pDDSPrimary->Restore();
				switch (i)
				{
					case 1:	a+=w2; c+=w2; break; //次は右上
					case 2:	b+=h2; d+=h2; break; //次は右下
					case 3:	a-=w2; c-=w2; break; //次は左下
				}
			}
		}
		else //通常
		{
			SetRect(&rcDst, a, b, c, d);
			if ((!APP_GetStretched())&&((_Flags & SCREEN_FFULLSCREEN) == 0)) //ウィンドウモードでストレッチをしない場合。v2.14更新
			{
				w2 = (c-a) / _Magnification;
				//PRINTF("%d, %d", s,w2);//test
				w2 = (s-w2)/2;
				SetRect(&rcSrc, w2*s2, ot, (s-w2)*s2, ot+h); //転送元を転送先と同じドット数に合わせる。
			}
			else if (executeCode == 5) //オーバースキャン領域の左右をカットする場合
				SetRect(&rcSrc, 6*s2, ot, (s-6)*s2, ot+h);
			else //通常
				SetRect(&rcSrc, 0, ot, s*s2, ot+h);
			if (_pDDSPrimary->Blt(&rcDst, _pDDSBack, &rcSrc, DDBLT_ASYNC, NULL) == DDERR_SURFACELOST)
				_pDDSPrimary->Restore();
		}
	}
	else
	{	//ラスタごとに異なる解像度が設定されていたとき。※Vistaだと超スロー(nVidia環境で確認)で使い物にならない。Direct3DならOK。
		//v1.28更新。１ラインずつ転送するようにした。
		s2 = 1;
		if (_Magnification >= 2) //ｘ２以上の場合
		{
			if (APP_GetScanLineType() != 0)
			{
				if (!APP_GetTvMode())
					s2 = 2; //ｘ３以上も横は２倍ドット固定。（速度アップ＋拡大されたときにバイリニアフィルタがいい感じにかかる）
				t = _Magnification; //縦は倍率ぶんのソースを用意して転送。
			}
			else
			{	//ノンスキャンライン時
				s2 = 2;
				if (_Magnification == 2)
					t = _Magnification; //縦は倍率ぶんのソースを用意して転送。
				else
					t = _Magnification-1; //3x,4xのときは、それぞれ2x,3xに拡大。（ジャギー軽減＆速度アップ）
			}
		}
		else
			t = 1;
		for (i=0; i<h; i++)
		{
			if (((*pTvW) == 512)&&(MAINBOARD_GetShowOverscanLeft() > 0)) //512のときはオーバースキャン表示しない
			{	//左右に黒帯を配置
				if (APP_GetVStretched()) //縦画面モードなら
					SetRect(&rcDst, a+(int)(5.34*(double)_Magnification+0.5), b+i*_Magnification,
									c-(int)(5.34*(double)_Magnification+0.5)-1, b+i*_Magnification+_Magnification);
				else if (APP_GetStretched())
					SetRect(&rcDst, a+(int)(7.01*(double)_Magnification+0.5), b+i*_Magnification,
									c-(int)(7.01*(double)_Magnification+0.5)-1, b+i*_Magnification+_Magnification);
				else
					SetRect(&rcDst, a, b+i*_Magnification, c, b+i*_Magnification+_Magnification);
			}
			else //通常
				SetRect(&rcDst, a, b+i*_Magnification, c, b+i*_Magnification+_Magnification);
			if (executeCode == 5) //オーバースキャン領域の左右をカットする場合
				SetRect(&rcSrc, 6*s2, (ot+i)*t, ((*pTvW)-6)*s2, (ot+i)*t+t);
			else //通常
				SetRect(&rcSrc, 0, (ot+i)*t, (*pTvW)*s2, (ot+i)*t+t);
			if (_pDDSPrimary->Blt(&rcDst, _pDDSBack, &rcSrc, DDBLT_ASYNC, NULL) == DDERR_SURFACELOST)
				_pDDSPrimary->Restore();
			pTvW++;
		}
	}
}


/*
	ハードウェアに頼るストレッチ描画モード
	VRAM → VRAM 転送
	(ストレッチされた場合バイリニア補完がかかる)
*/
//Kitao追加
#define	GB	(d & 0x000000FF)
#define	GG	(d & 0x0000FF00)>>8
#define	GR	(d & 0x00FF0000)>>16
//Kitao更新。zoom3x()，zoom2x()，zoom1x()等も用意。速度重視のためそれぞれ別ルーチンに。ラインごとに解像度を変えているゲームに対応。
static void
zoom4x_dd16(
	Uint32*		pSrc,			// 転送元のポインタ
	Sint32		srcY,			// 転送元のY座標
	Uint16*		pTvW,			// 各ラインの幅。Kitao追加
	Sint32		srcH)			// 転送元の高さ
{
	int			i,j; //Kitao追加
	Uint32*		pSrc0; //Kitao追加
	Uint32*		pDst; //Kitao追加
	Uint32*		pDst0; //Kitao追加
	Uint32		d; //Kitao追加
	Uint32		a; //Kitao追加

	// スキャンライン80%
	pSrc0 = pSrc;
	pDst0 = _pPixels + srcY*_Pitch4;
	for (i = 0; i < srcH; i++)
	{
		pSrc = pSrc0;
		pDst = pDst0;
		for (j = 0; j < *pTvW; j++)
		{
			d = *pSrc++;
			*pDst++ = ((_Gamma[GR] << _Rshift)|(_Gamma[GG] << _Gshift) | _Gamma[GB])|
					  (((_GammaS90[GR] << _Rshift)|(_GammaS90[GG] << _Gshift) | _GammaS90[GB]) << 16); //4byteぶん(２ドット)書き込まれる。横を２倍拡大(90%縦スキャンラインで)
		}
		pDst = pDst0 + _Pitch;
		pSrc = pSrc0;
		for (j = 0; j < *pTvW; j++) //２列目は90%スキャンラインで。v2.34更新
		{
			d = *pSrc++;
			a =(_GammaS90[GR] << _Rshift)|(_GammaS90[GG] << _Gshift) | _GammaS90[GB];
			*pDst++ = a | (a << 16); //横を２倍拡大(90%縦スキャンラインで)
		}
		pDst = pDst0 + _Pitch2;
		pSrc = pSrc0;
		for (j = 0; j < *pTvW; j++) //３列目も90%スキャンラインで。v2.34更新
		{
			d = *pSrc++;
			a =(_GammaS90[GR] << _Rshift)|(_GammaS90[GG] << _Gshift) | _GammaS90[GB];
			*pDst++ = a | (a << 16); //横を２倍拡大(90%縦スキャンラインで)
		}
		pDst = pDst0 + _Pitch3;
		pSrc = pSrc0;
		for (j = 0; j < *pTvW; j++)
		{
			d = *pSrc++;
			a =(_GammaS80[GR] << _Rshift)|(_GammaS80[GG] << _Gshift) | _GammaS80[GB];
			*pDst++ = a | (a << 16); //横を２倍拡大(80%縦スキャンラインで)
		}
		pDst0 += _Pitch4;
		pSrc0 += SOURCE_PITCH;
		pTvW++;
	}
}

//Kitao更新。32ビットカラーにも対応。
static void
zoom4x_dd32(
	Uint32*		pSrc,			// 転送元のポインタ
	Sint32		srcY,			// 転送元のY座標
	Uint16*		pTvW,			// 各ラインの幅。Kitao追加
	Sint32		srcH)			// 転送元の高さ
{
	int			i,j; //Kitao追加
	Uint32*		pSrc0; //Kitao追加
	Uint32*		pDst; //Kitao追加
	Uint32*		pDst0; //Kitao追加
	Uint32		d; //Kitao追加
	Uint32		a; //Kitao追加

	// スキャンライン80%
	pSrc0 = pSrc;
	pDst0 = _pPixels + srcY*_Pitch4;
	for (i = 0; i < srcH; i++)
	{
		pSrc = pSrc0;
		pDst = pDst0;
		for (j = 0; j < *pTvW; j++)
		{
			d = *pSrc++;
			*pDst++ = (_Gamma[GR] << 16)|(_Gamma[GG] << 8)| _Gamma[GB]; //4byteぶん(１ドット)書き込まれる。
			*pDst++ = (_GammaS90[GR] << 16)|(_GammaS90[GG] << 8)| _GammaS90[GB]; //4byteぶん(１ドット)書き込まれる。横を２倍拡大(90%縦スキャンラインで)
		}
		pDst = pDst0 + _Pitch;
		pSrc = pSrc0;
		for (j = 0; j < *pTvW; j++) //２列目は90%スキャンラインで。v2.34更新
		{
			d = *pSrc++;
			a = (_GammaS90[GR] << 16)|(_GammaS90[GG] << 8)| _GammaS90[GB];
			*pDst++ = a;
			*pDst++ = a; //横を２倍拡大(90%縦スキャンラインで)。v2.34更新
		}
		pDst = pDst0 + _Pitch2;
		pSrc = pSrc0;
		for (j = 0; j < *pTvW; j++) //３列目も90%スキャンラインで。v2.34更新
		{
			d = *pSrc++;
			a = (_GammaS90[GR] << 16)|(_GammaS90[GG] << 8)| _GammaS90[GB];
			*pDst++ = a;
			*pDst++ = a; //横を２倍拡大(90%縦スキャンラインで)。v2.34更新
		}
		pDst = pDst0 + _Pitch3;
		pSrc = pSrc0;
		for (j = 0; j < *pTvW; j++)
		{
			d = *pSrc++;
			a = (_GammaS80[GR] << 16)|(_GammaS80[GG] << 8)| _GammaS80[GB];
			*pDst++ = a;
			*pDst++ = a; //横を２倍拡大(80%縦スキャンラインで)
		}
		pDst0 += _Pitch4;
		pSrc0 += SOURCE_PITCH;
		pTvW++;
	}
}

//Kitao更新。16ビットカラーx３倍用
static void
zoom3x_dd16(
	Uint32*		pSrc,			// 転送元のポインタ
	Sint32		srcY,			// 転送元のY座標
	Uint16*		pTvW,			// 各ラインの幅。Kitao追加
	Sint32		srcH)			// 転送元の高さ
{
	int			i,j; //Kitao追加
	Uint32*		pSrc0; //Kitao追加
	Uint32*		pDst; //Kitao追加
	Uint32*		pDst0; //Kitao追加
	Uint32		d; //Kitao追加
	Uint32		a; //Kitao追加

	// スキャンライン80%
	pSrc0 = pSrc;
	pDst0 = _pPixels + srcY*_Pitch3;
	for (i = 0; i < srcH; i++)
	{
		pSrc = pSrc0;
		pDst = pDst0;
		for (j = 0; j < *pTvW; j++)
		{
			d = *pSrc++;
			*pDst++ = ((_Gamma[GR] << _Rshift)|(_Gamma[GG] << _Gshift) | _Gamma[GB])|
					  (((_GammaS90[GR] << _Rshift)|(_GammaS90[GG] << _Gshift) | _GammaS90[GB]) << 16); //横を２倍拡大(90%縦スキャンラインで)
		}
		pDst = pDst0 + _Pitch;
		pSrc = pSrc0;
		for (j = 0; j < *pTvW; j++) //２列目は90%スキャンラインにする。
		{
			d = *pSrc++;
			a = (_GammaS90[GR] << _Rshift)|(_GammaS90[GG] << _Gshift) | _GammaS90[GB];
			*pDst++ = a | (a << 16); //横を２倍拡大(90%縦スキャンラインで)
		}
		pDst = pDst0 + _Pitch2;
		pSrc = pSrc0;
		for (j = 0; j < *pTvW; j++)
		{
			d = *pSrc++;
			a = (_GammaS80[GR] << _Rshift)|(_GammaS80[GG] << _Gshift) | _GammaS80[GB];
			*pDst++ = a | (a << 16); //横を２倍拡大(80%縦スキャンラインで)
		}
		pDst0 += _Pitch3;
		pSrc0 += SOURCE_PITCH;
		pTvW++;
	}
}

//Kitao更新。32ビットカラーx３倍用
static void
zoom3x_dd32(
	Uint32*		pSrc,			// 転送元のポインタ
	Sint32		srcY,			// 転送元のY座標
	Uint16*		pTvW,			// 各ラインの幅。Kitao追加
	Sint32		srcH)			// 転送元の高さ
{
	int			i,j; //Kitao追加
	Uint32*		pSrc0; //Kitao追加
	Uint32*		pDst; //Kitao追加
	Uint32*		pDst0; //Kitao追加
	Uint32		d; //Kitao追加
	Uint32		a; //Kitao追加

	// スキャンライン80%
	pSrc0 = pSrc;
	pDst0 = _pPixels + srcY*_Pitch3;
	for (i = 0; i < srcH; i++)
	{
		pSrc = pSrc0;
		pDst = pDst0;
		for (j = 0; j < *pTvW; j++)
		{
			d = *pSrc++;
			*pDst++ = (_Gamma[GR] << 16)|(_Gamma[GG] << 8)| _Gamma[GB];
			*pDst++ = (_GammaS90[GR] << 16)|(_GammaS90[GG] << 8)| _GammaS90[GB]; //横を２倍拡大(90%縦スキャンラインで)
		}
		pDst = pDst0 + _Pitch;
		pSrc = pSrc0;
		for (j = 0; j < *pTvW; j++) //２列目は90%スキャンラインにする。
		{
			d = *pSrc++;
			a = (_GammaS90[GR] << 16)|(_GammaS90[GG] << 8)| _GammaS90[GB];
			*pDst++ = a;
			*pDst++ = a; //横を２倍拡大(90%縦スキャンラインで)
		}
		pDst = pDst0 + _Pitch2;
		pSrc = pSrc0;
		for (j = 0; j < *pTvW; j++)
		{
			d = *pSrc++;
			a = (_GammaS80[GR] << 16)|(_GammaS80[GG] << 8)| _GammaS80[GB];
			*pDst++ = a;
			*pDst++ = a; //横を２倍拡大
		}
		pDst0 += _Pitch3;
		pSrc0 += SOURCE_PITCH;
		pTvW++;
	}
}

//Kitao更新。x2の16ビットカラー用。
static void
zoom2x_dd16(
	Uint32*		pSrc,			// 転送元のポインタ
	Sint32		srcY,			// 転送元のY座標
	Uint16*		pTvW,			// 各ラインの幅。Kitao追加
	Sint32		srcH)			// 転送元の高さ
{
	int			i,j; //Kitao追加
	Uint32*		pSrc0; //Kitao追加
	Uint32*		pDst; //Kitao追加
	Uint32*		pDst0; //Kitao追加
	Uint32		d; //Kitao追加
	Uint32		a; //Kitao追加

	// スキャンライン80%
	pSrc0 = pSrc;
	pDst0 = _pPixels + srcY*_Pitch2;
	for (i = 0; i < srcH; i++)
	{
		pSrc = pSrc0;
		pDst = pDst0;
		for (j = 0; j < *pTvW; j++)
		{
			d = *pSrc++;
			*pDst++ = ((_Gamma[GR] << _Rshift)|(_Gamma[GG] << _Gshift) | _Gamma[GB])|
					  (((_GammaS90[GR] << _Rshift)|(_GammaS90[GG] << _Gshift) | _GammaS90[GB]) << 16); //横を２倍拡大
		}
		pDst = pDst0 + _Pitch;
		pSrc = pSrc0;
		for (j = 0; j < *pTvW; j++)
		{
			d = *pSrc++;
			a = (_GammaS80[GR] << _Rshift)|(_GammaS80[GG] << _Gshift) | _GammaS80[GB];
			*pDst++ = a | (a << 16); //横を２倍拡大
		}
		pDst0 += _Pitch2;
		pSrc0 += SOURCE_PITCH;
		pTvW++;
	}
}

//Kitao更新。x2の32ビットカラー用。
static void
zoom2x_dd32(
	Uint32*		pSrc,			// 転送元のポインタ
	Sint32		srcY,			// 転送元のY座標
	Uint16*		pTvW,			// 各ラインの幅。Kitao追加
	Sint32		srcH)			// 転送元の高さ
{
	int			i,j; //Kitao追加
	Uint32*		pSrc0; //Kitao追加
	Uint32*		pDst; //Kitao追加
	Uint32*		pDst0; //Kitao追加
	Uint32		d; //Kitao追加
	Uint32		a; //Kitao追加

	// スキャンライン80%
	pSrc0 = pSrc;
	pDst0 = _pPixels + srcY*_Pitch2;
	for (i = 0; i < srcH; i++)
	{
		pSrc = pSrc0;
		pDst = pDst0;
		for (j = 0; j < *pTvW; j++)
		{
			d = *pSrc++;
			*pDst++ = (_Gamma[GR] << 16)|(_Gamma[GG] << 8)| _Gamma[GB];
			*pDst++ = (_GammaS90[GR] << 16)|(_GammaS90[GG] << 8)| _GammaS90[GB]; //横を２倍拡大(90%縦スキャンラインで)
		}
		pDst = pDst0 + _Pitch;
		pSrc = pSrc0;
		for (j = 0; j < *pTvW; j++)
		{
			d = *pSrc++;
			a = (_GammaS80[GR] << 16)|(_GammaS80[GG] << 8)| _GammaS80[GB];
			*pDst++ = a;
			*pDst++ = a; //横を２倍拡大(80%縦スキャンラインで)
		}
		pDst0 += _Pitch2;
		pSrc0 += SOURCE_PITCH;
		pTvW++;
	}
}

//Kitao追加。横スキャンライン16ビットカラーx４倍用。速度重視のためそれぞれ別ルーチンに。
static void
zoom4xHS_dd16(
	Uint32*		pSrc,			// 転送元のポインタ
	Sint32		srcY,			// 転送元のY座標
	Uint16*		pTvW,			// 各ラインの幅。Kitao追加
	Sint32		srcH)			// 転送元の高さ
{
	int			i,j; //Kitao追加
	Uint32*		pSrc0; //Kitao追加
	Uint32*		pDst; //Kitao追加
	Uint32*		pDst0; //Kitao追加
	Uint32		d; //Kitao追加
	Uint32		a; //Kitao追加

	// スキャンライン80%
	pSrc0 = pSrc;
	pDst0 = _pPixels + srcY*_Pitch4;
	for (i = 0; i < srcH; i++)
	{
		pSrc = pSrc0;
		pDst = pDst0;
		for (j = 0; j < *pTvW; j++)
		{
			d = *pSrc++;
			a = (_Gamma[GR] << _Rshift)|(_Gamma[GG] << _Gshift) | _Gamma[GB];
			*pDst++ = a | (a << 16);
		}
		pDst = pDst0 + _Pitch;
		pSrc = pSrc0;
		for (j = 0; j < *pTvW; j++) //２列目は90%スキャンラインで。v2.35更新
		{
			d = *pSrc++;
			a = (_GammaS90[GR] << _Rshift)|(_GammaS90[GG] << _Gshift) | _GammaS90[GB];
			*pDst++ = a | (a  << 16);
		}
		pDst = pDst0 + _Pitch2;
		pSrc = pSrc0;
		for (j = 0; j < *pTvW; j++) //３列目も90%スキャンラインで。v2.35更新
		{
			d = *pSrc++;
			a = (_GammaS90[GR] << _Rshift)|(_GammaS90[GG] << _Gshift) | _GammaS90[GB];
			*pDst++ = a | (a << 16);
		}
		pDst = pDst0 + _Pitch3;
		pSrc = pSrc0;
		for (j = 0; j < *pTvW; j++)
		{
			d = *pSrc++;
			a = (_GammaS80[GR] << _Rshift)|(_GammaS80[GG] << _Gshift) | _GammaS80[GB];
			*pDst++ = a | (a << 16);
		}
		pDst0 += _Pitch4;
		pSrc0 += SOURCE_PITCH;
		pTvW++;
	}
}

//Kitao追加。横スキャンライン32ビットカラーx４倍用
static void
zoom4xHS_dd32(
	Uint32*		pSrc,			// 転送元のポインタ
	Sint32		srcY,			// 転送元のY座標
	Uint16*		pTvW,			// 各ラインの幅。Kitao追加
	Sint32		srcH)			// 転送元の高さ
{
	int			i,j; //Kitao追加
	Uint32*		pSrc0; //Kitao追加
	Uint32*		pDst; //Kitao追加
	Uint32*		pDst0; //Kitao追加
	Uint32		d; //Kitao追加
	Uint32		a; //Kitao追加

	// スキャンライン80%
	pSrc0 = pSrc;
	pDst0 = _pPixels + srcY*_Pitch4;
	for (i = 0; i < srcH; i++)
	{
		pSrc = pSrc0;
		pDst = pDst0;
		for (j = 0; j < *pTvW; j++)
		{
			d = *pSrc++;
			a = (_Gamma[GR] << 16)|(_Gamma[GG] << 8)| _Gamma[GB];
			*pDst++ = a;
			*pDst++ = a; //横を２倍拡大
		}
		pDst = pDst0 + _Pitch;
		pSrc = pSrc0;
		for (j = 0; j < *pTvW; j++) //２列目は90%スキャンラインで。v2.35更新
		{
			d = *pSrc++;
			a = (_GammaS90[GR] << 16)|(_GammaS90[GG] << 8)| _GammaS90[GB];
			*pDst++ = a;
			*pDst++ = a; //横を２倍拡大
		}
		pDst = pDst0 + _Pitch2;
		pSrc = pSrc0;
		for (j = 0; j < *pTvW; j++) //３列目も90%スキャンラインで。v2.35更新
		{
			d = *pSrc++;
			a = (_GammaS90[GR] << 16)|(_GammaS90[GG] << 8)| _GammaS90[GB];
			*pDst++ = a;
			*pDst++ = a; //横を２倍拡大
		}
		pDst = pDst0 + _Pitch3;
		pSrc = pSrc0;
		for (j = 0; j < *pTvW; j++)
		{
			d = *pSrc++;
			a = (_GammaS80[GR] << 16)|(_GammaS80[GG] << 8)| _GammaS80[GB];
			*pDst++ = a;
			*pDst++ = a; //横を２倍拡大
		}
		pDst0 += _Pitch4;
		pSrc0 += SOURCE_PITCH;
		pTvW++;
	}
}

//Kitao追加。横スキャンライン16ビットカラーx３倍用
static void
zoom3xHS_dd16(
	Uint32*		pSrc,			// 転送元のポインタ
	Sint32		srcY,			// 転送元のY座標
	Uint16*		pTvW,			// 各ラインの幅。Kitao追加
	Sint32		srcH)			// 転送元の高さ
{
	int			i,j; //Kitao追加
	Uint32*		pSrc0; //Kitao追加
	Uint32*		pDst; //Kitao追加
	Uint32*		pDst0; //Kitao追加
	Uint32		d; //Kitao追加
	Uint32		a; //Kitao追加

	// スキャンライン80%
	pSrc0 = pSrc;
	pDst0 = _pPixels + srcY*_Pitch3;
	for (i = 0; i < srcH; i++)
	{
		pSrc = pSrc0;
		pDst = pDst0;
		for (j = 0; j < *pTvW; j++)
		{
			d = *pSrc++;
			a = (_Gamma[GR] << _Rshift)|(_Gamma[GG] << _Gshift) | _Gamma[GB];
			*pDst++ = a | (a << 16);
		}
		pDst = pDst0 + _Pitch;
		pSrc = pSrc0;
		for (j = 0; j < *pTvW; j++) //２列目は90%スキャンラインにする。
		{
			d = *pSrc++;
			a = (_GammaS90[GR] << _Rshift)|(_GammaS90[GG] << _Gshift) | _GammaS90[GB];
			*pDst++ = a | (a << 16);
		}
		pDst = pDst0 + _Pitch2;
		pSrc = pSrc0;
		for (j = 0; j < *pTvW; j++)
		{
			d = *pSrc++;
			a = (_GammaS80[GR] << _Rshift)|(_GammaS80[GG] << _Gshift) | _GammaS80[GB];
			*pDst++ = a | (a << 16);
		}
		pDst0 += _Pitch3;
		pSrc0 += SOURCE_PITCH;
		pTvW++;
	}
}

//Kitao追加。横スキャンライン32ビットカラーx３倍用
static void
zoom3xHS_dd32(
	Uint32*		pSrc,			// 転送元のポインタ
	Sint32		srcY,			// 転送元のY座標
	Uint16*		pTvW,			// 各ラインの幅。Kitao追加
	Sint32		srcH)			// 転送元の高さ
{
	int			i,j; //Kitao追加
	Uint32*		pSrc0; //Kitao追加
	Uint32*		pDst; //Kitao追加
	Uint32*		pDst0; //Kitao追加
	Uint32		d; //Kitao追加
	Uint32		a; //Kitao追加

	// スキャンライン80%
	pSrc0 = pSrc;
	pDst0 = _pPixels + srcY*_Pitch3;
	for (i = 0; i < srcH; i++)
	{
		pSrc = pSrc0;
		pDst = pDst0;
		for (j = 0; j < *pTvW; j++)
		{
			d = *pSrc++;
			a = (_Gamma[GR] << 16)|(_Gamma[GG] << 8)| _Gamma[GB];
			*pDst++ = a;
			*pDst++ = a; //横を２倍拡大
		}
		pDst = pDst0 + _Pitch;
		pSrc = pSrc0;
		for (j = 0; j < *pTvW; j++) //２列目は90%スキャンラインにする。
		{
			d = *pSrc++;
			a = (_GammaS90[GR] << 16)|(_GammaS90[GG] << 8)| _GammaS90[GB];
			*pDst++ = a;
			*pDst++ = a; //横を２倍拡大
		}
		pDst = pDst0 + _Pitch2;
		pSrc = pSrc0;
		for (j = 0; j < *pTvW; j++)
		{
			d = *pSrc++;
			a = (_GammaS80[GR] << 16)|(_GammaS80[GG] << 8)| _GammaS80[GB];
			*pDst++ = a;
			*pDst++ = a; //横を２倍拡大
		}
		pDst0 += _Pitch3;
		pSrc0 += SOURCE_PITCH;
		pTvW++;
	}
}

//Kitao追加。x2の横スキャンライン16ビットカラー用。
static void
zoom2xHS_dd16(
	Uint32*		pSrc,			// 転送元のポインタ
	Sint32		srcY,			// 転送元のY座標
	Uint16*		pTvW,			// 各ラインの幅。Kitao追加
	Sint32		srcH)			// 転送元の高さ
{
	int			i,j; //Kitao追加
	Uint32*		pSrc0; //Kitao追加
	Uint32*		pDst; //Kitao追加
	Uint32*		pDst0; //Kitao追加
	Uint32		d; //Kitao追加
	Uint32		a; //Kitao追加

	// スキャンライン80%
	pSrc0 = pSrc;
	pDst0 = _pPixels + srcY*_Pitch2;
	for (i = 0; i < srcH; i++)
	{
		pSrc = pSrc0;
		pDst = pDst0;
		for (j = 0; j < *pTvW; j++)
		{
			d = *pSrc++;
			a = (_Gamma[GR] << _Rshift)|(_Gamma[GG] << _Gshift) | _Gamma[GB];
			*pDst++ = a | (a << 16);
		}
		pDst = pDst0 + _Pitch;
		pSrc = pSrc0;
		for (j = 0; j < *pTvW; j++)
		{
			d = *pSrc++;
			a = (_GammaS80[GR] << _Rshift)|(_GammaS80[GG] << _Gshift) | _GammaS80[GB];
			*pDst++ = a | (a << 16);
		}
		pDst0 += _Pitch2;
		pSrc0 += SOURCE_PITCH;
		pTvW++;
	}
}

//Kitao追加。x2の横スキャンライン32ビットカラー用。
static void
zoom2xHS_dd32(
	Uint32*		pSrc,			// 転送元のポインタ
	Sint32		srcY,			// 転送元のY座標
	Uint16*		pTvW,			// 各ラインの幅。Kitao追加
	Sint32		srcH)			// 転送元の高さ
{
	int			i,j; //Kitao追加
	Uint32*		pSrc0; //Kitao追加
	Uint32*		pDst; //Kitao追加
	Uint32*		pDst0; //Kitao追加
	Uint32		d; //Kitao追加
	Uint32		a; //Kitao追加

	// スキャンライン80%
	pSrc0 = pSrc;
	pDst0 = _pPixels + srcY*_Pitch2;
	for (i = 0; i < srcH; i++)
	{
		pSrc = pSrc0;
		pDst = pDst0;
		for (j = 0; j < *pTvW; j++)
		{
			d = *pSrc++;
			a = (_Gamma[GR] << 16)|(_Gamma[GG] << 8)| _Gamma[GB];
			*pDst++ = a;
			*pDst++ = a; //横を２倍拡大
		}
		pDst = pDst0 + _Pitch;
		pSrc = pSrc0;
		for (j = 0; j < *pTvW; j++)
		{
			d = *pSrc++;
			a = (_GammaS80[GR] << 16)|(_GammaS80[GG] << 8)| _GammaS80[GB];
			*pDst++ = a;
			*pDst++ = a; //横を２倍拡大
		}
		pDst0 += _Pitch2;
		pSrc0 += SOURCE_PITCH;
		pTvW++;
	}
}

//Kitao更新。ノンスキャンライン用。2x3x4xで16ビットカラー。速度重視のためスキャンライン用と処理を分けた。
static void
zoom2x3x4xNS_dd16(
	Sint32		bairitsu,		// 2xか3xか4xか。2xなら2。3xなら3。4xなら4。
	Uint32*		pSrc,			// 転送元のポインタ
	Sint32		srcY,			// 転送元のY座標
	Uint16*		pTvW,			// 各ラインの幅。Kitao追加
	Sint32		srcH)			// 転送元の高さ
{
	int			i,j,k; //Kitao追加
	Uint32*		pSrc0; //Kitao追加
	Uint32*		pDst; //Kitao追加
	Uint32*		pDst0; //Kitao追加
	Uint32		d; //Kitao追加
	Uint32		a; //Kitao追加
	Sint32		bairitsu2; //Kitao追加。縦のドットを何倍にデジタル拡大するか。2xか3xなら2。4xなら3。

	if (bairitsu == 2)
		bairitsu2 = 2;
	else
		bairitsu2 = bairitsu -1;

	// スキャンラインなし
	pSrc0 = pSrc;
	pDst0 = _pPixels + srcY*(_Pitch*bairitsu2);
	for (i = 0; i < srcH; i++)
	{
		for (k = 1; k <= bairitsu2; k++) //縦をbairitsu倍にデジタル拡大する
		{
			pSrc = pSrc0;
			pDst = pDst0;
			for (j = 0; j < *pTvW; j++)
			{
				d = *pSrc++;
				a = (_Gamma[GR] << _Rshift)|(_Gamma[GG] << _Gshift) | _Gamma[GB];
				*pDst++ = a | (a << 16); //横を２倍拡大(スキャンラインなし)
			}
			pDst0 += _Pitch;
		}
		pSrc0 += SOURCE_PITCH;
		pTvW++;
	}
}

//Kitao更新。ノンスキャンラインで2x3x4x。32ビットカラー用
static void
zoom2x3x4xNS_dd32(
	Sint32		bairitsu,		// 2xか3xか4xか。2xなら2。3xなら3。4xなら4。
	Uint32*		pSrc,			// 転送元のポインタ
	Sint32		srcY,			// 転送元のY座標
	Uint16*		pTvW,			// 各ラインの幅。Kitao追加
	Sint32		srcH)			// 転送元の高さ
{
	int			i,j,k; //Kitao追加
	Uint32*		pSrc0; //Kitao追加
	Uint32*		pDst; //Kitao追加
	Uint32*		pDst0; //Kitao追加
	Uint32		d; //Kitao追加
	Uint32		a; //Kitao追加
	Sint32		bairitsu2; //Kitao追加。縦のドットを何倍にデジタル拡大するか。2xか3xなら2。4xなら3。

	if (bairitsu == 2)
		bairitsu2 = 2;
	else
		bairitsu2 = bairitsu -1;

	// スキャンラインなし
	pSrc0 = pSrc;
	pDst0 = _pPixels + srcY*(_Pitch*bairitsu2);
	for (i = 0; i < srcH; i++)
	{
		for (k = 1; k <= bairitsu2; k++) //縦をbairitsu倍にデジタル拡大する
		{
			pSrc = pSrc0;
			pDst = pDst0;
			for (j = 0; j < *pTvW; j++)
			{
				d = *pSrc++;
				a = (_Gamma[GR] << 16)|(_Gamma[GG] << 8)| _Gamma[GB];
				*pDst++ = a;
				*pDst++ = a; //横を２倍拡大(スキャンラインなし)
			}
			pDst0 += _Pitch;
		}
		pSrc0 += SOURCE_PITCH;
		pTvW++;
	}
}

//Kitao追加。16ビットカラーx１倍用。スキャンラインなし
static void
zoom1xNS_dd16(
	Uint32*		pSrc,			// 転送元のポインタ
	Sint32		srcY,			// 転送元のY座標
	Uint16*		pTvW,			 //Kitao追加
	Sint32		srcH)			// 転送元の高さ
{
	int			i,j; //Kitao追加
	Uint32*		pSrc0; //Kitao追加
	Uint32*		pDst; //Kitao追加
	Uint32*		pDst0; //Kitao追加
	Uint32		d; //Kitao追加
	Uint32		a; //Kitao追加

	// １倍なのでスキャンラインはなし
	pSrc0 = pSrc;
	pDst0 = _pPixels + srcY*_Pitch;
	for (i = 0; i < srcH; i++)
	{
		pSrc = pSrc0;
		pDst = pDst0;
		for (j = 0; j < (*pTvW >> 1); j++) //ソースを横２ドットずつ書き込む。v2.13更新
		{
			d = *pSrc++;
			a = (_Gamma[GR] << _Rshift)|(_Gamma[GG] << _Gshift) | _Gamma[GB];
			d = *pSrc++;
			*pDst++ = a | (((_Gamma[GR] << _Rshift)|(_Gamma[GG] << _Gshift) | _Gamma[GB]) << 16); //ソース２ドットぶん書き込まれる
		}
		pDst0 += _Pitch;
		pSrc0 += SOURCE_PITCH;
		pTvW++;
	}
}

//Kitao追加。32ビットカラーx１倍用。スキャンラインなし
static void
zoom1xNS_dd32(
	Uint32*		pSrc,			// 転送元のポインタ
	Sint32		srcY,			// 転送元のY座標
	Uint16*		pTvW, 			//Kitao追加
	Sint32		srcH)			// 転送元の高さ
{
	int			i,j; //Kitao追加
	Uint32*		pSrc0; //Kitao追加
	Uint32*		pDst; //Kitao追加
	Uint32*		pDst0; //Kitao追加
	Uint32		d; //Kitao追加

	// １倍なのでスキャンラインは無し
	pSrc0 = pSrc;
	pDst0 = _pPixels + srcY*_Pitch;
	for (i = 0; i < srcH; i++)
	{
		pSrc = pSrc0;
		pDst = pDst0;
		for (j = 0; j < *pTvW; j++)
		{
			d = *pSrc++;
			*pDst++ = (_Gamma[GR] << 16)|(_Gamma[GG] << 8)| _Gamma[GB];
		}
		pDst0 += _Pitch;
		pSrc0 += SOURCE_PITCH;
		pTvW++;
	}
}

//Kitao追加。TV Mode の16ビットカラーｘ４用。速度重視のためそれぞれ別ルーチンに。
static void
zoom4xTV_dd16(
	Uint32*		pSrc,			// 転送元のポインタ
	Sint32		srcY,			// 転送元のY座標
	Uint16*		pTvW,			// 各ラインの幅。Kitao追加
	Sint32		srcH)			// 転送元の高さ
{
	int			i,j; //Kitao追加
	Uint32*		pSrc0; //Kitao追加
	Uint32*		pDst; //Kitao追加
	Uint32*		pDst0; //Kitao追加
	Uint32		d; //Kitao追加
	Uint32		a; //Kitao追加

	// スキャンライン80%
	pSrc0 = pSrc;
	pDst0 = _pPixels + srcY*_Pitch4;
	for (i = 0; i < srcH; i++)
	{
		pSrc = pSrc0;
		pDst = pDst0;
		for (j = 0; j < (*pTvW >> 1); j++) //ソースを横２ドットずつ書き込む。v2.13更新
		{
			d = *pSrc++;
			a = (_Gamma[GR] << _Rshift)|(_Gamma[GG] << _Gshift) | _Gamma[GB];
			d = *pSrc++;
			*pDst++ = a | (((_Gamma[GR] << _Rshift)|(_Gamma[GG] << _Gshift) | _Gamma[GB]) << 16); //ソース２ドットぶん書き込まれる
		}
		pDst = pDst0 + _Pitch;
		pSrc = pSrc0;
		for (j = 0; j < (*pTvW >> 1); j++) //２列目は90%スキャンラインで。v2.35更新
		{
			d = *pSrc++;
			a = (_GammaS90[GR] << _Rshift)|(_GammaS90[GG] << _Gshift) | _GammaS90[GB];
			d = *pSrc++;
			*pDst++ = a | (((_GammaS90[GR] << _Rshift)|(_GammaS90[GG] << _Gshift) | _GammaS90[GB]) << 16);
		}
		pDst = pDst0 + _Pitch2;
		pSrc = pSrc0;
		for (j = 0; j < (*pTvW >> 1); j++) //３列目も90%スキャンラインで。v2.35更新
		{
			d = *pSrc++;
			a = (_GammaS90[GR] << _Rshift)|(_GammaS90[GG] << _Gshift) | _GammaS90[GB];
			d = *pSrc++;
			*pDst++ = a | (((_GammaS90[GR] << _Rshift)|(_GammaS90[GG] << _Gshift) | _GammaS90[GB]) << 16);
		}
		pDst = pDst0 + _Pitch3;
		pSrc = pSrc0;
		for (j = 0; j < (*pTvW >> 1); j++)
		{
			d = *pSrc++;
			a = (_GammaS80[GR] << _Rshift)|(_GammaS80[GG] << _Gshift) | _GammaS80[GB];
			d = *pSrc++;
			*pDst++ = a | (((_GammaS80[GR] << _Rshift)|(_GammaS80[GG] << _Gshift) | _GammaS80[GB]) << 16);
		}
		pDst0 += _Pitch4;
		pSrc0 += SOURCE_PITCH;
		pTvW++;
	}
}

//Kitao追加。TV Mode の32ビットカラーｘ４用。
static void
zoom4xTV_dd32(
	Uint32*		pSrc,			// 転送元のポインタ
	Sint32		srcY,			// 転送元のY座標
	Uint16*		pTvW,			// 各ラインの幅。Kitao追加
	Sint32		srcH)			// 転送元の高さ
{
	int			i,j; //Kitao追加
	Uint32*		pSrc0; //Kitao追加
	Uint32*		pDst; //Kitao追加
	Uint32*		pDst0; //Kitao追加
	Uint32		d; //Kitao追加

	// スキャンライン80%
	pSrc0 = pSrc;
	pDst0 = _pPixels + srcY*_Pitch4;
	for (i = 0; i < srcH; i++)
	{
		pSrc = pSrc0;
		pDst = pDst0;
		for (j = 0; j < *pTvW; j++)
		{
			d = *pSrc++;
			*pDst++ = (_Gamma[GR] << 16)|(_Gamma[GG] << 8)| _Gamma[GB];
		}
		pDst = pDst0 + _Pitch;
		pSrc = pSrc0;
		for (j = 0; j < *pTvW; j++) //２列目は90%スキャンラインで。v2.35更新
		{
			d = *pSrc++;
			*pDst++ = (_GammaS90[GR] << 16)|(_GammaS90[GG] << 8)| _GammaS90[GB];
		}
		pDst = pDst0 + _Pitch2;
		pSrc = pSrc0;
		for (j = 0; j < *pTvW; j++) //３列目も90%スキャンラインで。v2.35更新
		{
			d = *pSrc++;
			*pDst++ = (_GammaS90[GR] << 16)|(_GammaS90[GG] << 8)| _GammaS90[GB];
		}
		pDst = pDst0 + _Pitch3;
		pSrc = pSrc0;
		for (j = 0; j < *pTvW; j++)
		{
			d = *pSrc++;
			*pDst++ = (_GammaS80[GR] << 16)|(_GammaS80[GG] << 8)| _GammaS80[GB];
		}
		pDst0 += _Pitch4;
		pSrc0 += SOURCE_PITCH;
		pTvW++;
	}
}

//Kitao追加。TV Mode の16ビットカラーｘ３用。
static void
zoom3xTV_dd16(
	Uint32*		pSrc,			// 転送元のポインタ
	Sint32		srcY,			// 転送元のY座標
	Uint16*		pTvW,			// 各ラインの幅。Kitao追加
	Sint32		srcH)			// 転送元の高さ
{
	int			i,j; //Kitao追加
	Uint32*		pSrc0; //Kitao追加
	Uint32*		pDst; //Kitao追加
	Uint32*		pDst0; //Kitao追加
	Uint32		d; //Kitao追加
	Uint32		a; //Kitao追加

	// スキャンライン80%
	pSrc0 = pSrc;
	pDst0 = _pPixels + srcY*_Pitch3;
	for (i = 0; i < srcH; i++)
	{
		pSrc = pSrc0;
		pDst = pDst0;
		for (j = 0; j < (*pTvW >> 1); j++) //ソースを横２ドットずつ書き込む。v2.13更新
		{
			d = *pSrc++;
			a = (_Gamma[GR] << _Rshift)|(_Gamma[GG] << _Gshift) | _Gamma[GB];
			d = *pSrc++;
			*pDst++ = a | (((_Gamma[GR] << _Rshift)|(_Gamma[GG] << _Gshift) | _Gamma[GB]) << 16); //ソース２ドットぶん書き込まれる
		}
		pDst = pDst0 + _Pitch;
		pSrc = pSrc0;
		for (j = 0; j < (*pTvW >> 1); j++) //２列目は90%スキャンラインにする。
		{
			d = *pSrc++;
			a = (_GammaS90[GR] << _Rshift)|(_GammaS90[GG] << _Gshift) | _GammaS90[GB];
			d = *pSrc++;
			*pDst++ = a | (((_GammaS90[GR] << _Rshift)|(_GammaS90[GG] << _Gshift) | _GammaS90[GB]) << 16);
		}
		pDst = pDst0 + _Pitch2;
		pSrc = pSrc0;
		for (j = 0; j < (*pTvW >> 1); j++)
		{
			d = *pSrc++;
			a = (_GammaS80[GR] << _Rshift)|(_GammaS80[GG] << _Gshift) | _GammaS80[GB];
			d = *pSrc++;
			*pDst++ = a | (((_GammaS80[GR] << _Rshift)|(_GammaS80[GG] << _Gshift) | _GammaS80[GB]) << 16);
		}
		pDst0 += _Pitch3;
		pSrc0 += SOURCE_PITCH;
		pTvW++;
	}
}

//Kitao追加。TV Mode の32ビットカラーｘ３用。
static void
zoom3xTV_dd32(
	Uint32*		pSrc,			// 転送元のポインタ
	Sint32		srcY,			// 転送元のY座標
	Uint16*		pTvW,			// 各ラインの幅。Kitao追加
	Sint32		srcH)			// 転送元の高さ
{
	int			i,j; //Kitao追加
	Uint32*		pSrc0; //Kitao追加
	Uint32*		pDst; //Kitao追加
	Uint32*		pDst0; //Kitao追加
	Uint32		d; //Kitao追加

	// スキャンライン80%
	pSrc0 = pSrc;
	pDst0 = _pPixels + srcY*_Pitch3;
	for (i = 0; i < srcH; i++)
	{
		pSrc = pSrc0;
		pDst = pDst0;
		for (j = 0; j < *pTvW; j++)
		{
			d = *pSrc++;
			*pDst++ = (_Gamma[GR] << 16)|(_Gamma[GG] << 8)| _Gamma[GB];
		}
		pDst = pDst0 + _Pitch;
		pSrc = pSrc0;
		for (j = 0; j < *pTvW; j++) //２列目は90%スキャンラインにする。
		{
			d = *pSrc++;
			*pDst++ = (_GammaS90[GR] << 16)|(_GammaS90[GG] << 8)| _GammaS90[GB];
		}
		pDst = pDst0 + _Pitch2;
		pSrc = pSrc0;
		for (j = 0; j < *pTvW; j++)
		{
			d = *pSrc++;
			*pDst++ = (_GammaS80[GR] << 16)|(_GammaS80[GG] << 8)| _GammaS80[GB];
		}
		pDst0 += _Pitch3;
		pSrc0 += SOURCE_PITCH;
		pTvW++;
	}
}

//Kitao更新。TV Mode の16ビットカラー ｘ２用。
static void
zoom2xTV_dd16(
	Uint32*		pSrc,			// 転送元のポインタ
	Sint32		srcY,			// 転送元のY座標
	Uint16*		pTvW,			// 各ラインの幅。Kitao追加
	Sint32		srcH)			// 転送元の高さ
{
	int			i,j; //Kitao追加
	Uint32*		pSrc0; //Kitao追加
	Uint32*		pDst; //Kitao追加
	Uint32*		pDst0; //Kitao追加
	Uint32		d; //Kitao追加
	Uint32		a; //Kitao追加

	// スキャンライン80%
	pSrc0 = pSrc;
	pDst0 = _pPixels + srcY*_Pitch2;
	for (i = 0; i < srcH; i++)
	{
		pSrc = pSrc0;
		pDst = pDst0;
		for (j = 0; j < (*pTvW >> 1); j++) //ソースを横２ドットずつ書き込む。v2.13更新
		{
			d = *pSrc++;
			a = (_Gamma[GR] << _Rshift)|(_Gamma[GG] << _Gshift) | _Gamma[GB];
			d = *pSrc++;
			*pDst++ = a | (((_Gamma[GR] << _Rshift)|(_Gamma[GG] << _Gshift) | _Gamma[GB]) << 16); //ソース２ドットぶん書き込まれる
		}
		pDst = pDst0 + _Pitch;
		pSrc = pSrc0;
		for (j = 0; j < (*pTvW >> 1); j++)
		{
			d = *pSrc++;
			a = (_GammaS80[GR] << _Rshift)|(_GammaS80[GG] << _Gshift) | _GammaS80[GB];
			d = *pSrc++;
			*pDst++ = a | (((_GammaS80[GR] << _Rshift)|(_GammaS80[GG] << _Gshift) | _GammaS80[GB]) << 16);
		}
		pDst0 += _Pitch2;
		pSrc0 += SOURCE_PITCH;
		pTvW++;
	}
}

//Kitao更新。TV Mode の32ビットカラー ｘ２用。
static void
zoom2xTV_dd32(
	Uint32*		pSrc,			// 転送元のポインタ
	Sint32		srcY,			// 転送元のY座標
	Uint16*		pTvW,			// 各ラインの幅。Kitao追加
	Sint32		srcH)			// 転送元の高さ
{
	int			i,j; //Kitao追加
	Uint32*		pSrc0; //Kitao追加
	Uint32*		pDst; //Kitao追加
	Uint32*		pDst0; //Kitao追加
	Uint32		d; //Kitao追加

	// スキャンライン80%
	pSrc0 = pSrc;
	pDst0 = _pPixels + srcY*_Pitch2;
	for (i = 0; i < srcH; i++)
	{
		pSrc = pSrc0;
		pDst = pDst0;
		for (j = 0; j < *pTvW; j++)
		{
			d = *pSrc++;
			*pDst++ = (_Gamma[GR] << 16)|(_Gamma[GG] << 8)| _Gamma[GB];
		}
		pDst = pDst0 + _Pitch;
		pSrc = pSrc0;
		for (j = 0; j < *pTvW; j++)
		{
			d = *pSrc++;
			*pDst++ = (_GammaS80[GR] << 16)|(_GammaS80[GG] << 8)| _GammaS80[GB];
		}
		pDst0 += _Pitch2;
		pSrc0 += SOURCE_PITCH;
		pTvW++;
	}
}

//白黒モード 32ビットカラー。メモリ(&ソースコード)効率も優先。各画面モードで共通処理は共通して行う。v2.29
#define	DOGREEN	if (bGreen) {b=a+(a>>3)+(a>>4); if (b>0xFF) b=0xFF; a=a>>2;} else {b=a;}
static void
monoColor_32(
	Sint32		bairitsu,		// ウィンドウ表示倍率
	Sint32		scanLineType,	// スキャンラインのタイプ
	Uint32*		pSrc,			// 転送元のポインタ
	Sint32		srcY,			// 転送元のY座標
	Uint16*		pTvW,			// 各ラインの幅
	Sint32		srcH,			// 転送元の高さ
	Sint32		bGreen)			// グリーンディスプレイ
{
	int			i,j,k;
	Uint32*		pSrc0;
	Uint32*		pDst;
	Uint32*		pDst0;
	Uint32		d;
	Uint32		a;
	Uint32		b;
	int			sl;

	if (bairitsu == 1)
		scanLineType = 0;
	if (((scanLineType == 2)||(scanLineType == 0))&&(bairitsu >= 2))
		sl = 2; //横スキャンラインかノンスキャンラインの場合
	else
		sl = 1;
	if (scanLineType == 0) //ノンスキャンライン時
	{
		if (bairitsu >= 3)
			bairitsu--;
	}

	// スキャンライン80%
	pSrc0 = pSrc;
	pDst0 = _pPixels + srcY*_Pitch*bairitsu;
	for (i=0; i<srcH; i++)
	{
		pSrc = pSrc0;
		pDst = pDst0;
		for (j=0; j<*pTvW; j++)
		{
			d = *pSrc++;
			a = _MonoTableR[_Gamma[GR]] + _MonoTableG[_Gamma[GG]] + _MonoTableB[_Gamma[GB]]; DOGREEN //R,G,Bの輝度を平均化。DOGREEN定義…bGreenがTRUEならグリーンディスプレイ化
			for (k=0; k<sl; k++) //横スキャンラインの場合、同じドットで２倍拡大。
				*pDst++ = (a << 16)|(b << 8)| a; //モノクロ化。１ドットぶん書き込まれる。
			if (scanLineType == 1) //縦横スキャンラインの場合
			{
				a = _MonoTableR[_GammaS90[GR]] + _MonoTableG[_GammaS90[GG]] + _MonoTableB[_GammaS90[GB]]; DOGREEN //R,G,Bの輝度を平均化
				*pDst++ = (a << 16)|(b << 8)| a; //１ドットぶん書き込まれる。横を２倍拡大(90%縦スキャンラインで)
			}
		}
		*pDst = 0; //Direct3Dでは右端に１ドットぶんクリア(黒)マージンを書いておかないと右端が乱れる。
		pDst = pDst0 + _Pitch;
		pSrc = pSrc0;
		switch (bairitsu)
		{
			case 4: //x4倍
				for (j=0; j<*pTvW; j++) //２列目は90%スキャンラインで。v2.35更新
				{
					d = *pSrc++;
					a = _MonoTableR[_GammaS90[GR]] + _MonoTableG[_GammaS90[GG]] + _MonoTableB[_GammaS90[GB]]; DOGREEN //R,G,Bの輝度を平均化
					for (k=0; k<sl; k++) //横スキャンラインの場合、同じドットで２倍拡大。
						*pDst++ = (a << 16)|(b << 8)| a; //モノクロ化。１ドットぶん書き込まれる。
					if (scanLineType == 1) //縦横スキャンラインの場合
						*pDst++ = (a << 16)|(b << 8)| a; //モノクロ化。１ドットぶん書き込まれる。横を２倍拡大(90%縦スキャンラインで)
				}
				*pDst = 0;
				pDst = pDst0 + _Pitch2;
				pSrc = pSrc0;
				for (j=0; j <*pTvW; j++) //３列目も90%スキャンラインで。v2.35更新
				{
					d = *pSrc++;
					a = _MonoTableR[_GammaS90[GR]] + _MonoTableG[_GammaS90[GG]] + _MonoTableB[_GammaS90[GB]]; DOGREEN //R,G,Bの輝度を平均化
					for (k=0; k<sl; k++) //横スキャンラインの場合、同じドットで２倍拡大。
						*pDst++ = (a << 16)|(b << 8)| a; //モノクロ化。１ドットぶん書き込まれる。
					if (scanLineType == 1) //縦横スキャンラインの場合
						*pDst++ = (a << 16)|(b << 8)| a; //モノクロ化。１ドットぶん書き込まれる。横を２倍拡大(90%縦スキャンラインで)
				}
				*pDst = 0;
				pDst = pDst0 + _Pitch3;
				break;
			case 3: //x3倍
				for (j=0; j<*pTvW; j++) //２列目は90%スキャンラインで
				{
					d = *pSrc++;
					a = _MonoTableR[_GammaS90[GR]] + _MonoTableG[_GammaS90[GG]] + _MonoTableB[_GammaS90[GB]]; DOGREEN //R,G,Bの輝度を平均化
					for (k=0; k<sl; k++) //横スキャンラインの場合、同じドットで２倍拡大。
						*pDst++ = (a << 16)|(b << 8)| a; //モノクロ化。１ドットぶん書き込まれる。
					if (scanLineType == 1) //縦横スキャンラインの場合
						*pDst++ = (a << 16)|(b << 8)| a; //モノクロ化。１ドットぶん書き込まれる。横を２倍拡大(90%縦スキャンラインで)
				}
				*pDst = 0;
				pDst = pDst0 + _Pitch2;
				break;
			//case 2: //x2倍
			//	x2倍のときは処理なし。
		}
		if (bairitsu >= 2)
		{
			pSrc = pSrc0;
			for (j=0; j<*pTvW; j++)
			{
				d = *pSrc++;
				a = _MonoTableR[_GammaS80[GR]] + _MonoTableG[_GammaS80[GG]] + _MonoTableB[_GammaS80[GB]]; DOGREEN //R,G,Bの輝度を平均化
				for (k=0; k<sl; k++) //横スキャンラインの場合、同じドットで２倍拡大。
					*pDst++ = (a << 16)|(b << 8)| a; //モノクロ化。１ドットぶん書き込まれる。
				if (scanLineType == 1) //縦横スキャンラインの場合
					*pDst++ = (a << 16)|(b << 8)| a; //モノクロ化。１ドットぶん書き込まれる。横を２倍拡大(80%縦スキャンラインで)
			}
			*pDst = 0;
		}
		pDst0 += _Pitch * bairitsu;
		pSrc0 += SOURCE_PITCH;
		pTvW++;
	}
}
#undef DOGREEN

//白黒モード 16ビットカラー。メモリ(&ソースコード)効率も優先。各画面モードで共通処理は共通して行う。v2.29
#define	DOGREEN		if (bGreen) {b=a+(a>>3)+(a>>4); if (b>0x1F) b=0x1F; a=a>>2;} else {b=a;}
#define	DOGREEN2	if (bGreen) {b2=a2+(a2>>3)+(a2>>4); if (b2>0x1F) b2=0x1F; a2=a2>>2;} else {b2=a2;}
static void
monoColor_16(
	Sint32		bairitsu,		// ウィンドウ表示倍率
	Sint32		scanLineType,	// スキャンラインのタイプ
	Uint32*		pSrc,			// 転送元のポインタ
	Sint32		srcY,			// 転送元のY座標
	Uint16*		pTvW,			// 各ラインの幅
	Sint32		srcH,			// 転送元の高さ
	Sint32		bGreen)			// グリーンディスプレイ
{
	int			i,j;
	Uint32*		pSrc0;
	Uint32*		pDst;
	Uint32*		pDst0;
	Uint32		d;
	Uint32		a,a2;
	Uint32		b,b2;
	int			sl;

	if (bairitsu == 1)
		scanLineType = 0;
	if (((scanLineType == 2)||(scanLineType == 0))&&(bairitsu >= 2))
		sl = 2; //横スキャンラインかノンスキャンラインの場合
	else
		sl = 1;
	if (scanLineType == 0) //ノンスキャンライン時
	{
		if (bairitsu >= 3)
			bairitsu--;
	}

	// スキャンライン80%
	pSrc0 = pSrc;
	pDst0 = _pPixels + srcY*_Pitch*bairitsu;
	for (i=0; i<srcH; i++)
	{
		pSrc = pSrc0;
		pDst = pDst0;
		for (j=0; j<*pTvW; j++)
		{
			d = *pSrc++;
			a = _MonoTableR[_Gamma[GR]] + _MonoTableG[_Gamma[GG]] + _MonoTableB[_Gamma[GB]]; DOGREEN //R,G,Bの輝度を平均化。DOGREEN定義…bGreenがTRUEならグリーンディスプレイ化
			if (sl == 2) //横スキャンラインの場合、同じドットで２倍拡大。
				*pDst++ = ((a << _Rshift) | (b << _Gshift) | a) | (((a << _Rshift) | (b << _Gshift) | a) << 16); //モノクロ化。２ドットぶん書き込まれる。
			else if (scanLineType == 1) //縦横スキャンラインの場合
			{
				a2 = _MonoTableR[_GammaS90[GR]] + _MonoTableG[_GammaS90[GG]] + _MonoTableB[_GammaS90[GB]]; DOGREEN2 //R,G,Bの輝度を平均化
				*pDst++ = ((a << _Rshift) | (b << _Gshift) | a) | (((a2 << _Rshift) | (b2 << _Gshift) | a2) << 16); //モノクロ化。２ドットぶん書き込まれる。横を２倍拡大(90%縦スキャンラインで)
			}
			else
			{
				d = *pSrc++;
				a2 = _MonoTableR[_Gamma[GR]] + _MonoTableG[_Gamma[GG]] + _MonoTableB[_Gamma[GB]]; DOGREEN2 //R,G,Bの輝度を平均化。DOGREEN2定義…bGreenがTRUEならグリーンディスプレイ化
				*pDst++ = ((a << _Rshift) | (b << _Gshift) | a) | (((a2 << _Rshift) | (b2 << _Gshift) | a2) << 16); //モノクロ化。２ドットぶん書き込まれる。
				j++; //ソース２ドットぶんを一度に処理完了
			}
		}
		*pDst = 0; //Direct3Dでは右端に１ドットぶんクリア(黒)マージンを書いておかないと右端が乱れる。
		pDst = pDst0 + _Pitch;
		pSrc = pSrc0;
		switch (bairitsu)
		{
			case 4: //x4倍
				for (j=0; j<*pTvW; j++) //２列目は90%スキャンラインで。v2.35更新
				{
					d = *pSrc++;
					a = _MonoTableR[_GammaS90[GR]] + _MonoTableG[_GammaS90[GG]] + _MonoTableB[_GammaS90[GB]]; DOGREEN //R,G,Bの輝度を平均化
					if (sl == 2) //横スキャンラインの場合、同じドットで２倍拡大。
						*pDst++ = ((a << _Rshift) | (b << _Gshift) | a) | (((a << _Rshift) | (b << _Gshift) | a) << 16); //モノクロ化。２ドットぶん書き込まれる。
					else if (scanLineType == 1) //縦横スキャンラインの場合
						*pDst++ = ((a << _Rshift) | (b << _Gshift) | a) | (((a << _Rshift) | (b << _Gshift) | a) << 16); //モノクロ化。２ドットぶん書き込まれる。横を２倍拡大(90%縦スキャンラインで)
					else
					{
						d = *pSrc++;
						a2 = _MonoTableR[_GammaS90[GR]] + _MonoTableG[_GammaS90[GG]] + _MonoTableB[_GammaS90[GB]]; DOGREEN2 //R,G,Bの輝度を平均化。DOGREEN2定義…bGreenがTRUEならグリーンディスプレイ化
						*pDst++ = ((a << _Rshift) | (b << _Gshift) | a) | (((a2 << _Rshift) | (b2 << _Gshift) | a2) << 16); //モノクロ化。２ドットぶん書き込まれる。
						j++; //ソース２ドットぶんを一度に処理完了
					}
				}
				*pDst = 0;
				pDst = pDst0 + _Pitch2;
				pSrc = pSrc0;
				for (j=0; j <*pTvW; j++) //３列目も90%スキャンラインで。v2.35更新
				{
					d = *pSrc++;
					a = _MonoTableR[_GammaS90[GR]] + _MonoTableG[_GammaS90[GG]] + _MonoTableB[_GammaS90[GB]]; DOGREEN //R,G,Bの輝度を平均化
					if ((sl == 2)||(scanLineType == 1)) //横スキャンラインか縦横スキャンラインの場合、同じドットで２倍拡大。
						*pDst++ = ((a << _Rshift) | (b << _Gshift) | a) | (((a << _Rshift) | (b << _Gshift) | a) << 16); //モノクロ化。２ドットぶん書き込まれる。
					else
					{
						d = *pSrc++;
						a2 = _MonoTableR[_GammaS90[GR]] + _MonoTableG[_GammaS90[GG]] + _MonoTableB[_GammaS90[GB]]; DOGREEN2 //R,G,Bの輝度を平均化。DOGREEN2定義…bGreenがTRUEならグリーンディスプレイ化
						*pDst++ = ((a << _Rshift) | (b << _Gshift) | a) | (((a2 << _Rshift) | (b2 << _Gshift) | a2) << 16); //モノクロ化。２ドットぶん書き込まれる。
						j++; //ソース２ドットぶんを一度に処理完了
					}
				}
				*pDst = 0;
				pDst = pDst0 + _Pitch3;
				break;
			case 3: //x3倍
				for (j=0; j<*pTvW; j++) //２列目は90%スキャンラインで
				{
					d = *pSrc++;
					a = _MonoTableR[_GammaS90[GR]] + _MonoTableG[_GammaS90[GG]] + _MonoTableB[_GammaS90[GB]]; DOGREEN //R,G,Bの輝度を平均化
					if ((sl == 2)||(scanLineType == 1)) //横スキャンラインか縦横スキャンラインの場合、同じドットで２倍拡大。
						*pDst++ = ((a << _Rshift) | (b << _Gshift) | a) | (((a << _Rshift) | (b << _Gshift) | a) << 16); //モノクロ化。２ドットぶん書き込まれる。
					else
					{
						d = *pSrc++;
						a2 = _MonoTableR[_GammaS90[GR]] + _MonoTableG[_GammaS90[GG]] + _MonoTableB[_GammaS90[GB]]; DOGREEN2 //R,G,Bの輝度を平均化。DOGREEN2定義…bGreenがTRUEならグリーンディスプレイ化
						*pDst++ = ((a << _Rshift) | (b << _Gshift) | a) | (((a2 << _Rshift) | (b2 << _Gshift) | a2) << 16); //モノクロ化。２ドットぶん書き込まれる。
						j++; //ソース２ドットぶんを一度に処理完了
					}
				}
				*pDst = 0;
				pDst = pDst0 + _Pitch2;
				break;
			//case 2: //x2倍
			//	x2倍のときは処理なし。
		}
		if (bairitsu >= 2)
		{
			pSrc = pSrc0;
			for (j=0; j<*pTvW; j++)
			{
				d = *pSrc++;
				a = _MonoTableR[_GammaS80[GR]] + _MonoTableG[_GammaS80[GG]] + _MonoTableB[_GammaS80[GB]]; DOGREEN //R,G,Bの輝度を平均化
				if ((sl == 2)||(scanLineType == 1)) //横スキャンラインか縦横スキャンラインの場合、同じドットで２倍拡大。
					*pDst++ = ((a << _Rshift) | (b << _Gshift) | a) | (((a << _Rshift) | (b << _Gshift) | a) << 16); //モノクロ化。２ドットぶん書き込まれる。
				else
				{
					d = *pSrc++;
					a2 = _MonoTableR[_GammaS80[GR]] + _MonoTableG[_GammaS80[GG]] + _MonoTableB[_GammaS80[GB]]; DOGREEN2 //R,G,Bの輝度を平均化。DOGREEN2定義…bGreenがTRUEならグリーンディスプレイ化
					*pDst++ = ((a << _Rshift) | (b << _Gshift) | a) | (((a2 << _Rshift) | (b2 << _Gshift) | a2) << 16); //モノクロ化。２ドットぶん書き込まれる。
					j++; //ソース２ドットぶんを一度に処理完了
				}
			}
			*pDst = 0;
		}
		pDst0 += _Pitch * bairitsu;
		pSrc0 += SOURCE_PITCH;
		pTvW++;
	}
}
#undef DOGREEN16
#undef GR
#undef GG
#undef GB


/*-----------------------------------------------------------------------------
	[Blt]
		pSrc からバックバッファへ画像を書き込みます。拡大／縮小、
-----------------------------------------------------------------------------*/
//Kitao更新。必ずハードウェアアクセラレーションを使用する。
//			 ラインごとに解像度を変えているゲーム（龍虎の拳，あすか120%など）にも対応。
void
SCREENDD_Blt(
	Uint32*		pSrc,
	Sint32		srcX,
	Sint32		srcY,
	Uint16*		pTvW,	//Kitao更新。転送元の横ピクセル数。※srcHラインの数ぶん
	Sint32		srcH,
	Sint32		executeCode)  //Kitao追加。実行コード。0…エンコードだけ行う。1…プライマリ画面へ転送も行う。2…スクリーンショット用画面へ転送を行う。
							  //					   3…左右に黒帯(オーバースキャン部)を配置しての転送(あとは1と同じ)。4…スクリーンショット後の画面復元転送を行う。
							  //v2.12更新			   5…左右のオーバースキャン部をカットしての転送(あとは1と同じ)
{
	Uint32*		pSrc32;
	Uint16*		pTvW2;
	Sint32		scanLineType;

	if (pSrc==NULL) return;

	//v1.28更新。ライン途中からのエンコードに対応。
	pSrc32 = (Uint32*)pSrc + srcY * 512 + srcX;
	pTvW2 = pTvW + srcY;

	//Kitao更新。
	if (srcH > 0) //v2.43更新
	{
		if (lock_offscreen_surface(&_Pitch, &_pPixels)) //バックサーフェスをロック
		{
			_Pitch2 = _Pitch * 2;
			_Pitch3 = _Pitch * 3;
			_Pitch4 = _Pitch * 4;

			//Kitao更新。速度重視のためそれぞれの画面モードで、別々のルーチンを使う。
			scanLineType = APP_GetScanLineType();
			if (APP_GetTvMode())
			{
				if ((scanLineType != 0)&&(_Magnification >= 2)) //TV Mode でx2,x3,x4のとき
					scanLineType = 4;
				else // x1のときはノンスキャンラインと同じ
					scanLineType = 0;
			}
			else
			{
				if (scanLineType == 4) //スタートTV Mode が設定してあって、TvModeが一時的にFALSEの状態なら
					scanLineType = 1; //スペシャル(縦横)スキャンラインで描画
			}
			if (MAINBOARD_GetMonoColor()) //白黒モードなら。はにいいんざすかい,パワードリフト等の裏技
			{	//白黒モードの場合。v2.28,2.29
				if (_BitsPerPixel == 16)
					monoColor_16(_Magnification, scanLineType, pSrc32, srcY, pTvW2, srcH, (MAINBOARD_GetForceMonoColor() == 2)); //16bitカラー
				else
					monoColor_32(_Magnification, scanLineType, pSrc32, srcY, pTvW2, srcH, (MAINBOARD_GetForceMonoColor() == 2)); //32bitカラー
			}
			else
			{	//通常
				switch (scanLineType)
				{
					case 0: //ノンスキャンライン
						switch (_Magnification)
						{
							case 1:
								if (_BitsPerPixel == 16)
									zoom1xNS_dd16(pSrc32, srcY, pTvW2, srcH); //16bitカラー
								else
									zoom1xNS_dd32(pSrc32, srcY, pTvW2, srcH); //32bitカラー
								break;
							case 2:
							case 3:
							case 4:
								if (_BitsPerPixel == 16)
									zoom2x3x4xNS_dd16(_Magnification, pSrc32, srcY, pTvW2, srcH); //16bitカラー
								else
									zoom2x3x4xNS_dd32(_Magnification, pSrc32, srcY, pTvW2, srcH); //32bitカラー
								break;
						}
						break;
					case 1: //縦横スキャンライン
						switch (_Magnification)
						{
							case 1:
								if (_BitsPerPixel == 16)
									zoom1xNS_dd16(pSrc32, srcY, pTvW2, srcH); //16bitカラー
								else
									zoom1xNS_dd32(pSrc32, srcY, pTvW2, srcH); //32bitカラー
								break;
							case 2:
								if (_BitsPerPixel == 16)
									zoom2x_dd16(pSrc32, srcY, pTvW2, srcH); //16bitカラー
								else
									zoom2x_dd32(pSrc32, srcY, pTvW2, srcH); //32bitカラー
								break;
							case 3:
								if (_BitsPerPixel == 16)
									zoom3x_dd16(pSrc32, srcY, pTvW2, srcH); //16bitカラー
								else
									zoom3x_dd32(pSrc32, srcY, pTvW2, srcH); //32bitカラー
								break;
							case 4:
								if (_BitsPerPixel == 16)
									zoom4x_dd16(pSrc32, srcY, pTvW2, srcH); //16bitカラー
								else
									zoom4x_dd32(pSrc32, srcY, pTvW2, srcH); //32bitカラー
								break;
						}
						break;
					case 2: //横のみスキャンライン
						switch (_Magnification)
						{
							case 1:
								if (_BitsPerPixel == 16)
									zoom1xNS_dd16(pSrc32, srcY, pTvW2, srcH); //16bitカラー
								else
									zoom1xNS_dd32(pSrc32, srcY, pTvW2, srcH); //32bitカラー
								break;
							case 2:
								if (_BitsPerPixel == 16)
									zoom2xHS_dd16(pSrc32, srcY, pTvW2, srcH); //16bitカラー
								else
									zoom2xHS_dd32(pSrc32, srcY, pTvW2, srcH); //32bitカラー
								break;
							case 3:
								if (_BitsPerPixel == 16)
									zoom3xHS_dd16(pSrc32, srcY, pTvW2, srcH); //16bitカラー
								else
									zoom3xHS_dd32(pSrc32, srcY, pTvW2, srcH); //32bitカラー
								break;
							case 4:
								if (_BitsPerPixel == 16)
									zoom4xHS_dd16(pSrc32, srcY, pTvW2, srcH); //16bitカラー
								else
									zoom4xHS_dd32(pSrc32, srcY, pTvW2, srcH); //32bitカラー
								break;
						}
						break;
					//case 3: //縦のみスキャンライン（将来実装）。5以降にもエフェクトを実装できる。
					case 4: //TV Mode
						switch (_Magnification)
						{
							case 1: //ノンスキャンラインと同じ
								if (_BitsPerPixel == 16)
									zoom1xNS_dd16(pSrc32, srcY, pTvW2, srcH); //16bitカラー
								else
									zoom1xNS_dd32(pSrc32, srcY, pTvW2, srcH); //32bitカラー
								break;
							case 2:
								if (_BitsPerPixel == 16)
									zoom2xTV_dd16(pSrc32, srcY, pTvW2, srcH); //16bitカラー
								else
									zoom2xTV_dd32(pSrc32, srcY, pTvW2, srcH); //32bitカラー
								break;
							case 3:
								if (_BitsPerPixel == 16)
									zoom3xTV_dd16(pSrc32, srcY, pTvW2, srcH); //16bitカラー
								else
									zoom3xTV_dd32(pSrc32, srcY, pTvW2, srcH); //32bitカラー
								break;
							case 4:
								if (_BitsPerPixel == 16)
									zoom4xTV_dd16(pSrc32, srcY, pTvW2, srcH); //16bitカラー
								else
									zoom4xTV_dd32(pSrc32, srcY, pTvW2, srcH); //32bitカラー
								break;
						}
						break;
				}
			}
	
			//ロックを解除
			_pDDSBack->Unlock(NULL);
		}
	}		

	//プライマリ画面への転送
	if (executeCode > 0)
		zoom_ddTensou(pTvW+(16-MAINBOARD_GetShowOverscanTop()),
					  srcY+srcH-(16-MAINBOARD_GetShowOverscanTop()),
					  executeCode);
}


//Kitao追加。VSync(垂直帰線待ち)を行うかどうかを設定
void
SCREENDD_SetSyncTo60HzScreen(
	BOOL	bSyncTo60HzScreen)
{
	DEVMODE		dm; //Kitao追加

	//ディスプレイの現在の表示周波数を求める。V-Syncを行えるかどうか判断する。
	memset(&dm, 0, sizeof(DEVMODE));
	dm.dmSize = sizeof(DEVMODE);
	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm);
	if ((dm.dmDisplayFrequency == 60)||(dm.dmDisplayFrequency == 59)||(dm.dmDisplayFrequency == 61)) //リフレッシュレートが60Hz(59と61も含む)か強制設定のとき。ForceVSyncはv2.65から廃止。
	{
		_bSyncTo60HzScreen = APP_GetSyncTo60HzScreen();
		APP_EnableVSyncMenu(TRUE); //メニュー更新
	}
	else
	{
		_bSyncTo60HzScreen = FALSE;
		APP_EnableVSyncMenu(FALSE); //メニュー更新
	}
}

//VSync(垂直帰線待ち)を行っているかどうかを返す。VSyncをする設定でもSyncできない環境の場合はFALSEが返る。v2.43
BOOL
SCREENDD_GetSyncTo60HzScreen()
{
	return _bSyncTo60HzScreen;
}


//Kitao追加。前回のVBlank待ちが終わった時刻を返す。v1.28
DWORD
SCREENDD_GetLastTimeSyncTime()
{
	return _LastTimeSyncTime;
}


//Kitao追加。スクリーンショットをファイルに書き込む。v2.12
void
SCREENDD_WriteScreenshot(
	FILE*	fp)
{
	int				i,j;
	HRESULT			hr;
	int				trial = 10;
	LONG			pitch;
	Uint32*			pPixels0;
	Uint32*			pPixels;
	Uint32			BGRR; //Bule,Green,Red,Reserved
	Sint32			width;
	Sint32			height;
	Sint32			left;
	Sint32			top;
	Sint32			w2,h2;
	RECT			rc;
	Sint32			wp;

	//バッファに再描画する。
	if (((APP_GetOverscanHideBlackLR())&&(MAINBOARD_GetShowOverscanLeft() == 0))|| //左右のオーバースキャン領域に黒帯表示していた場合
		((APP_GetOverscanHideBlackTop())&&(MAINBOARD_GetShowOverscanTop() < 8))) //上側のオーバースキャン領域に黒帯表示していた場合
			MAINBOARD_SetResolutionChange(TRUE); //描画時にゴミが残らないように画面全体をクリアしてから描画する。
	MAINBOARD_DrawScreenshot();
	MAINBOARD_SetResolutionChange(FALSE); //元に戻す

	//サイズを計算
	width = APP_GetGameWidth(_Magnification);
	height = APP_GetGameHeight(_Magnification);
	wp = 4 - (width*3 % 4); //１ラインを4byte単位に整えるための値
	if (wp==4) wp=0;

	//左上の位置を計算
	if (_Flags & SCREEN_FFULLSCREEN)
	{
		if (APP_GetFullStretched(TRUE))
		{
			//アスペクト比を4:3に保つ
			if (_Width/4 >= _Height/3)
			{	//縦のほうが短いので縦を基準とする
				w2 = (Sint32)((double)(_Height / 3.0 * 4.0 + 0.5));
				left = (_Width - w2) / 2;
				if (MAINBOARD_GetShowOverscanLeft() > 0) //左右のオーバースキャン領域を表示するなら
					top = (_Height - (Sint32)((double)_Height * 0.96137 + 0.5)) / 2; //0.96137=224/((268-256)/4*3+224)
				else
					top = 0;
			}
			else
			{	//横のほうが短いので横を基準とする
				h2 = (Sint32)((double)(_Width / 4.0 * 3.0 + 0.5));
				left = 0;
				if (MAINBOARD_GetShowOverscanLeft() > 0) //左右のオーバースキャン領域を表示するなら
					top = (_Height - (Sint32)((double)h2 * 0.96137 + 0.5)) / 2; //0.96137=224/((268-256)/4*3+224)
				else
					top = (_Height - h2) / 2;
			}
		}
		else if (APP_GetStretched())
		{
			if (MAINBOARD_GetShowOverscanLeft() > 0) //左右のオーバースキャン領域を表示するなら
				w2 = (Sint32)(313.24 * (double)_Magnification + 0.5);
			else
				w2 = 299 * _Magnification;
			if (APP_GetVStretched()) //縦画面モードなら
				w2 = (Sint32)((double)w2 * (256.0/336.0));
			//アスペクト比をTV画面と同じ4:3に。縦の長さををPCE標準224の整数倍(２倍)にする
			left = (_Width - w2) / 2;
			top = (_Height - 224*_Magnification) / 2;
			if (MAINBOARD_GetShowOverscanTop() > 0) //上側のオーバースキャン領域を表示するなら
			{
				top -= MAINBOARD_GetShowOverscanTop() * _Magnification;
				if (top < 0) //画面からはみ出してしまう場合。1440x900,1600x900などであり得る。
					top = (_Height - 224*_Magnification) / 2; //アスペクト比を保つため、オーバースキャン部をカット。
			}
		}
		else //ノンストレッチの場合
		{
			w2 = APP_GetNonstretchedWidth() * _Magnification;
			if (w2 > _Width) w2 = _Width; //画面からはみ出してしまう場合は、ストレッチ(縮小)して対応。
			left = (_Width - w2) / 2;
			top = (_Height - 224*_Magnification) / 2;
			if (MAINBOARD_GetShowOverscanTop() > 0) //上側のオーバースキャン領域を表示するなら
			{
				top -= MAINBOARD_GetShowOverscanTop() * _Magnification;
				if (top < 0) top = 0; //画面からはみ出してしまう場合は、ストレッチ(縮小)して対応。1440x900,1600x900などであり得る。
			}
		}
	}
	else
	{
		GetWindowRect(_hWnd, &rc);
		left = rc.left + (rc.right - rc.left - _Width)/2;
		top = rc.bottom - (rc.right - rc.left - _Width)/2 - _Height; //枠の太さと表示領域の高さぶんを引く
	}

	//キャプチャするメモリをロック
	while (trial--)
	{
		hr = _pDDSPrimary->Lock(NULL, &_ddsdPrimary, DDLOCK_WAIT | DDLOCK_READONLY, NULL); //READONLYで高速化。v2.28
		if (hr == DD_OK)
			break;
    }
	if (hr != DD_OK) return;

	pitch = _ddsdPrimary.lPitch / 4; //32bit単位で処理するためピッチは1/4に。
	pPixels0 = (Uint32*)_ddsdPrimary.lpSurface + pitch*(top+height-1) + left; //最下段からファイルに出力して行く(BMPの仕様)。

	for (i = 0; i < height; i++)
	{
		pPixels = pPixels0;
		for (j = 0; j < width; j++)
		{
			BGRR = (*pPixels++);
			fwrite(&BGRR, 3, 1, fp); //BGRだけ書き込む
		}
		if (wp > 0)
		{	//１ラインを4byte単位に整える
			BGRR = 0;
			fwrite(&BGRR, wp, 1, fp);
		}
		pPixels0 -= pitch;
	}
	
	//ロックを解除
	_pDDSPrimary->Unlock(NULL);
}


//Kitao追加。スクリーンショット時のために一時的に描画倍率を変更する。Screen.cppから使用。v2.13
void
SCREENDD_SetMagnification(
	Sint32	magnification)
{
	_Magnification = magnification;
}

//Kitao追加。v2.13
Sint32
SCREENDD_GetMagnification()
{
	return _Magnification;
}

//Kitao追加。v2.28
Sint32
SCREENDD_GetBitsPerPixel()
{
	return _BitsPerPixel;
}
