/******************************************************************************
Ootake
・VistaのAERO等に対応するため、Direct3Dでの描画を実装。現状、機能的には
  ScreenDD.c(DirectDraw描画)と同等。カラーは32ビットのみ対応とする。

Copyright(C)2006-2010 Kitao Nakamura.
	改造版・後継版を公開なさるときは必ずソースコードを添付してください。
	その際に事後でかまいませんので、ひとことお知らせいただけると幸いです。
	商的な利用は禁じます。
	あとは「GNU General Public License(一般公衆利用許諾契約書)」に準じます。

******************************************************************************/
#define _CRT_SECURE_NO_DEPRECATE

#define DIRECTINPUT_VERSION	0x0800
#define DIRECT3D_VERSION	0x0900

#define SOURCE_PITCH 		512

#include <D3D9.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include "ScreenD3D.h"
#include "Screen.h"
#include "WinMain.h"
#include "App.h"
#include "MainBoard.h"
#include "Printf.h"

//	##RA
#include "RA_Interface.h"
#include "Input.h"


extern Uint32 _Gamma[8]; //Kitao追加。ガンマを計算した数値を入れておく。v1.14高速化。Uint32にしたほうが処理速かった。
extern Uint32 _GammaS80[8]; //Kitao追加。スキャンライン80%用
extern Uint32 _GammaS90[8]; //Kitao追加。スキャンライン90%用
extern Uint32 _MonoTableR[256]; //モノクロ変換用テーブル。高速化のため必要。v2.28
extern Uint32 _MonoTableG[256]; //
extern Uint32 _MonoTableB[256]; //

//Kitao追加。Direct3D9Ex用定義。v2.21
static HINSTANCE _hD3D9Dll;
typedef HRESULT (WINAPI* FuncDirect3DCreate9Ex) (UINT, IDirect3D9Ex **);
static FuncDirect3DCreate9Ex _FuncDirect3DCreate9Ex;

static LPDIRECT3D9				_pD3D = NULL; //Direct3D object
static LPDIRECT3DDEVICE9		_pD3DDev = NULL; //デバイス
static IDirect3D9Ex*			_pD3DEx = NULL; //Direct3DEx object。v2.21追加。Win7/Vistaで描画遅延を少なくするために必要。
static IDirect3DDevice9Ex*		_pD3DDevEx = NULL; //同上
static LPDIRECT3DSURFACE9		_pD3DSBack = NULL; //バックバッファ(ここに転送すると実際に描画される)
static LPDIRECT3DSURFACE9		_pD3DSGame = NULL; //メイン画面用
static LPDIRECT3DSURFACE9		_pD3DSText = NULL; //テキストメッセージ用
static LPDIRECT3DSURFACE9		_pD3DSFps = NULL; //FPS表示用
static D3DPRESENT_PARAMETERS	_pD3DParam;

static BOOL		_bOldVideoChip = FALSE; //古いビデオチップ(ビデオカード)の場合TRUEに。v2.19

static LONG		_SurfacePitch = 0;

static Sint32	_Width;
static Sint32	_Height;
static Sint32	_Magnification;
static Uint32	_Flags;
static HWND		_hWnd;
static Uint16*	_pScreenPixels;
static BOOL		_bChangedFullScreen = FALSE;
static Sint32	_GameLeft;   //print_message()で表示位置決定のために、ゲーム画面転送先の座標を控えておく。
static Sint32	_GameRight;  //
static Sint32	_GameTop;    //
static Sint32	_GameBottom; //

//Win9x用変数
static DWORD	_PrevWidth;
static DWORD	_PrevHeight;
static DWORD	_PrevBitsPerPixel;
static DWORD	_PrevFrequency;

static DWORD	_LastTimeSyncTime; //前回Syncしたときのタイマーカウント
static DWORD	_LastTimeSyncTime3; //3フレーム単位での、前回Syncしたときのタイマーカウント。v2.43
static DWORD	_Frame; //3フレーム単位を計るためでの、前回Syncしたときのタイマーカウント。v2.43
static Sint32	_SyncAjust; //VSyncがオフのときに1/60秒間隔にできるだけ近づけるための変数。v1.67
static Sint32	_SyncAjust3; //v2.43追加
static BOOL		_bSyncTo60HzScreen = TRUE;

static LONG		_Pitch;
static LONG		_Pitch2; //pitchの2倍の値を入れておく。速度アップ用。
static LONG		_Pitch3; //pitchの3倍の値を入れておく。速度アップ用。
static LONG		_Pitch4; //pitchの4倍の値を入れておく。速度アップ用。
static Uint32*	_pPixels; //v2.13更新。32bit単位で扱うようにした。(高速化)

static char*	_pMessageText = ""; //テキストメッセージ表示用バッファ。ヌル（先頭が0）の場合、非表示。

void RenderAchievementOverlays( RECT& rc )
{	
	//_pD3DDev->StretchRect( _pD3DSGame, NULL, _pD3DSAchievementOverlay, NULL, D3DTEXF_NONE ); //文字はクッキリさせるためNONEを使う。
	//rval = _pD3DSAchievementOverlay->StretchRect( NULL, _pD3DSBack, NULL, DDBLT_WAIT | DDBLT_ASYNC, NULL);

	//_pD3DSBack->Lock();

	HDC hDC;
	if( _pD3DSBack->GetDC( &hDC ) != D3D_OK )
		return;
	//_pD3DDev->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &_pD3DSBack );
	//if( _pD3DSAchievementOverlay->GetDC( &hDC ) != D3D_OK )
	//	return;

	static int nOldTime = GetTickCount();

	int nDelta = GetTickCount() - nOldTime;
	nOldTime = GetTickCount();
	//nDelta = 18;	//	uncomment for fixed-frame advance

	RECT rcSize;
	SetRect( &rcSize, 0, 0, rc.right-rc.left, rc.bottom-rc.top );

	//INPUT_UpdateState( FALSE );

	INPUT_UpdateState(TRUE);
	Uint8 nPadInput = JOYPAD_Read( 0 );

	ControllerInput input;
	input.m_bUpPressed		= ( INPUT_IsPressed( 0, JOYPAD_BUTTON_UP )		|| INPUT_IsPressed( 1, JOYPAD_BUTTON_UP )		|| INPUT_IsPressed( 2, JOYPAD_BUTTON_UP ) );
	input.m_bDownPressed	= ( INPUT_IsPressed( 0, JOYPAD_BUTTON_DOWN )	|| INPUT_IsPressed( 1, JOYPAD_BUTTON_DOWN )		|| INPUT_IsPressed( 2, JOYPAD_BUTTON_DOWN ) );
	input.m_bLeftPressed	= ( INPUT_IsPressed( 0, JOYPAD_BUTTON_LEFT )	|| INPUT_IsPressed( 1, JOYPAD_BUTTON_LEFT )		|| INPUT_IsPressed( 2, JOYPAD_BUTTON_LEFT ) );
	input.m_bRightPressed	= ( INPUT_IsPressed( 0, JOYPAD_BUTTON_RIGHT )	|| INPUT_IsPressed( 1, JOYPAD_BUTTON_RIGHT )	|| INPUT_IsPressed( 2, JOYPAD_BUTTON_RIGHT ) );
	input.m_bCancelPressed	= ( INPUT_IsPressed( 0, JOYPAD_BUTTON_II )		|| INPUT_IsPressed( 1, JOYPAD_BUTTON_II )		|| INPUT_IsPressed( 2, JOYPAD_BUTTON_II ) );
	input.m_bConfirmPressed = ( INPUT_IsPressed( 0, JOYPAD_BUTTON_I )		|| INPUT_IsPressed( 1, JOYPAD_BUTTON_I )		|| INPUT_IsPressed( 2, JOYPAD_BUTTON_I ) );
	input.m_bQuitPressed	= ( INPUT_IsPressed( 0, JOYPAD_BUTTON_RUN )		|| INPUT_IsPressed( 1, JOYPAD_BUTTON_RUN )		|| INPUT_IsPressed( 2, JOYPAD_BUTTON_RUN ) );

	RA_UpdateRenderOverlay( hDC, &input, ((float)nDelta / 1000.0f), &rcSize, APP_GetFullScreen()==TRUE, MAINBOARD_GetPause()==TRUE );

	_pD3DSBack->ReleaseDC( hDC );

	//RECT rcSrc;
	//SetRect( &rcSrc, 0, 0, _Width, _Height );

	//_pD3DDev->StretchRect( _pD3DSAchievementOverlay, NULL, _pD3DSBack, &_rcDest, D3DTEXF_NONE );
}


//D3Dデバイスの開放処理
static void
d3dSurfaceRelease()
{
	if (_pD3DSFps != NULL)
	{	_pD3DSFps->Release();	_pD3DSFps = NULL;	}

	if (_pD3DSText != NULL)
	{	_pD3DSText->Release();	_pD3DSText = NULL;	}

	if (_pD3DSGame != NULL)
	{	_pD3DSGame->Release();	_pD3DSGame = NULL;	}

	if (_pD3DSBack != NULL)
	{	_pD3DSBack->Release();	_pD3DSBack = NULL;	}
}

//Direct3D用スクリーンを安全に開放します。
void
SCREEND3D_Deinit()
{
	if (_pD3D != NULL)
	{
		d3dSurfaceRelease();
		if (APP_GetWindowsVista()) //Vista以降でDirect3D9Exを使った場合
		{
			if (_pD3DDevEx != NULL)
			{
				_pD3DDevEx->Release(); //Ex側で開放を行う
				_pD3DDevEx = NULL;
			}
			_pD3DEx->Release(); //Ex側で開放を行う
			_pD3DEx = NULL;
		}
		else //Vistaより前のWindowsの場合、普通に開放。
		{
			if (_pD3DDev != NULL)
				_pD3DDev->Release();
			_pD3D->Release();
		}
		_pD3DDev = NULL;
		_pD3D = NULL;
	}
	if (APP_GetWindowsVista())
		FreeLibrary(_hD3D9Dll); //DLLを開放
}

static HRESULT
d3dDevReset()
{
	int			a;
	HRESULT		hr;

	if (_pD3DDev == NULL) //未作成の場合
	{
		if (APP_GetWindowsVista())
		{	//v2.21追加
			hr = _pD3DEx->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, _hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING,
										 &_pD3DParam, NULL, &_pD3DDevEx);
			if (FAILED(hr))
				MessageBox(_hWnd, "ERROR: DIRECT3D::CreateDeviceEx() failed.    ", "Ootake", MB_OK);
			_pD3DDevEx->SetMaximumFrameLatency(0); //遅延を最小限に抑える。MSDNの記載によると0だとアダプタの既定値になるらしいが、1より0のほうが1フレーム遅延が少ない(GeForce)ので0に。Aeroオフ＋これでやっとXP並に。
			_pD3DDev = _pD3DDevEx; //型キャスト。あとは通常のDirect3D9に任せる。
		}
		else
			hr = _pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, _hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING,
									 &_pD3DParam, &_pD3DDev);
	}
	else //すでに作成済みの場合
	{
		d3dSurfaceRelease();
		hr = _pD3DDev->Reset(&_pD3DParam);
		if (FAILED(hr)) //リセットに失敗したら作り直す
		{
			_pD3DDev->Release();
			_pD3DDev = NULL;
			if (APP_GetWindowsVista())
			{	//v2.21追加
				hr = _pD3DEx->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, _hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING,
											 &_pD3DParam, NULL, &_pD3DDevEx);
				_pD3DDevEx->SetMaximumFrameLatency(0); //遅延を最小限に抑える。MSDNの記載によると0だとアダプタの既定値になるらしいが、1より0のほうが1フレーム遅延が少ない(GeForce)ので0に。Aeroオフ＋これでやっとXP並に。
				_pD3DDev = _pD3DDevEx; //型キャスト。あとは通常のDirect3D9に任せる。
			}
			else
				hr = _pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, _hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING,
										 &_pD3DParam, &_pD3DDev);
		}
	}
	if (FAILED(hr))
		return hr;

	//バックバッファーのアドレスを取得
	_pD3DDev->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &_pD3DSBack);

    //メイン(ゲーム)画面表示用オフスクリーンサーフェスを作成する
	if (_Magnification == 1)
		a = 512 + 8; //※+8は右端にクリアマージンを書き込み用。Direct3Dでは右端を黒でクリアしておかないとフィルタがそのバイトまで参照するので右端が乱れることがあるようだ。v2.06更新。+1で大丈夫と思うが念のため8の倍数にした。
	else //2倍以上は全て横2倍。速度アップ＋こうすることで適度にバイリニアフィルタが掛かりいい感じになる
		a = 512*2 + 8; //※+8は右端にクリアマージンを書き込み用。Direct3Dでは右端を黒でクリアしておかないとフィルタがそのバイトまで参照するので右端が乱れることがあるようだ。v2.06更新。+1で大丈夫と思うが念のため8の倍数にした。
	_pD3DDev->CreateOffscreenPlainSurface(a, 256*_Magnification, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &_pD3DSGame, NULL); //D3DPOOL_DEFAULTにしないとStretchRectが使えない。

    //テキスト表示用オフスクリーンサーフェスを作成する
	_pD3DDev->CreateOffscreenPlainSurface(512*2, 32, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &_pD3DSText, NULL);

    //FPS表示用オフスクリーンサーフェスを作成する
	_pD3DDev->CreateOffscreenPlainSurface(64*2, 32, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &_pD3DSFps, NULL);

	return hr;
}

//Direct3D用スクリーンを初期化します。
BOOL
SCREEND3D_Init(
	Sint32		width,
	Sint32		height,
	Sint32		magnification,
	Uint32		flags)
{
	DEVMODE					dm;
	D3DDISPLAYMODE			dmode;
	D3DADAPTER_IDENTIFIER9	di;
	HRESULT					hr;

	//パラメータを保管
	_Width = width;
	_Height = height;
	_Magnification = magnification;
	_Flags = flags;
	_hWnd =	WINMAIN_GetHwnd();

	if (APP_GetWindowsVista()) //Vista以降でDirect3D9Exが使える環境ならば
	{	//v2.21追加
		_hD3D9Dll = LoadLibrary("d3d9.dll"); //Vistaより前のWindowsでエラーを起こさないため、動的にDLLをロードして関数を検索し使用する。
		_FuncDirect3DCreate9Ex = (FuncDirect3DCreate9Ex)GetProcAddress(_hD3D9Dll, "Direct3DCreate9Ex");
		_FuncDirect3DCreate9Ex(D3D_SDK_VERSION, &_pD3DEx);
		_pD3D = _pD3DEx;
	}
	else //XP以前
		_pD3D = Direct3DCreate9(D3D_SDK_VERSION);

	if (_pD3D == NULL)
	{
		MessageBox(_hWnd, "ERROR: DIRECT3D::Direct3DCreate9() failed.    ", "Ootake", MB_OK);
		return FALSE;
	}

	//フルスクリーンの場合、 画面の解像度を設定し、ウィンドウを左上に配置＆画面全域の大きさにする。
	//	※Direct3D9のフルスクリーンモードは、メニューや他のダイアログ表示が正常に出来ないので使えない。手動でフルスクリーンに切り替えてからウィンドウモードを使う。
	//										 タイマーの進み方がおかしくなる不具合も、ウィンドウモードなら回避できる。
	if (flags & SCREEN_FFULLSCREEN) //フルスクリーンモードなら
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
		dm.dmBitsPerPel = 32;
		dm.dmDisplayFrequency = 60;
		dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_DISPLAYFREQUENCY;
		if ((APP_GetResolutionAutoChange())&&(_bChangedFullScreen))
			hr = ChangeDisplaySettings(&dm, 0); //設定を控えずに切替。一部の環境で切替時のもたつきを解消。v2.23。古いRADEONで切り替え時に画像の乱れ解消確認済み。v2.65
		else //通常
			ChangeDisplaySettings(&dm, CDS_FULLSCREEN);
		WINMAIN_SetFullScreenWindow(width, height);//v2.23追加。このタイミングで、ウィンドウスタイルの変更を行うようにした。パワーの無いマシンではDirectX初期化に時間が掛かるのでこのタイミングで画面を整えたほうが良さそう。
		_bChangedFullScreen = TRUE;
	}
	else //ウィンドウモードなら
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
			Sleep(1000); //デスクトップに戻る処理が完全に終わるまで念のためにウェイトを入れる。早いとDirecDrawのときのようにDirect3Dのデバイスの作成失敗しそうなので。
		}
		WINMAIN_SetNormalWindow(width, height);//v2.23追加。このタイミングで、ウィンドウスタイルの変更を行うようにした。パワーの無いマシンではDirectX初期化に時間が掛かるのでこのタイミングで画面を整えたほうが良さそう。
		APP_WindowWithinScreen(); //ウィンドウが画面からはみ出していた場合、画面内に収まるように移動する。
		_bChangedFullScreen = FALSE;
	}

	//ディスプレイモードを取得。WindowモードにてVSync可能かどうかの判断を行うために利用
	_pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &dmode);

	//ディスプレイアダプタ(ビデオチップ)の情報を取得。v2.19
	_pD3D->GetAdapterIdentifier(D3DADAPTER_DEFAULT, 0, &di);
	APP_StrToLower(di.Description);
	//PRINTF("VideoCard = %s", di.Description); //Test用
	if ((strstr(di.Description, "radeon ve") != NULL)||
		(strstr(di.Description, "radeon 2") != NULL)||
		(strstr(di.Description, "radeon 3") != NULL)||
		(strstr(di.Description, "radeon 7") != NULL)||
		(strstr(di.Description, "radeon 8") != NULL)||
		(strstr(di.Description, "radeon 90") != NULL)||
		(strstr(di.Description, "radeon 92") != NULL)||
		(strstr(di.Description, "wonder ve") != NULL)||
		(strstr(di.Description, "wonder 7") != NULL)||
		(strstr(di.Description, "wonder 90") != NULL)||
		(strstr(di.Description, "wonder 92") != NULL))
		{
			//PRINTF("Old! VideoCard = %s ", di.Description); //Test用
			_bOldVideoChip = TRUE; //古いビデオチップ
		}

	//Direct3Dデバイスを作成。メニューやダイアログ表示のため、フルスクリーン時もウィンドウデバイスとして作成する。
	ZeroMemory(&_pD3DParam, sizeof(_pD3DParam));
	_pD3DParam.hDeviceWindow = _hWnd;
	_pD3DParam.BackBufferWidth = width;
	_pD3DParam.BackBufferHeight = height;
	_pD3DParam.BackBufferFormat = D3DFMT_X8R8G8B8; //32ビットカラー専用
	_pD3DParam.BackBufferCount = 1;
	_pD3DParam.Windowed = TRUE; //Windowモード。参考：WinodwedがTRUEのときにリフレッシュレートを設定するとCreateエラー
	if ((APP_GetSyncTo60HzScreen())&&((dmode.RefreshRate == 60)||(dmode.RefreshRate == 59)||(dmode.RefreshRate == 61))||
		(APP_GetWindowsAutoVSyncBool())) //リフレッシュレートが60Hz(59と61も含む)かWindows自動V-Sync設定のときだけVSyncをOKに。
	{
		_pD3DParam.SwapEffect = D3DSWAPEFFECT_DISCARD; //COPYより軽い
		_pD3DParam.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
		if (APP_GetWindowsAutoVSyncBool()) //v2.65追加
			_pD3DParam.PresentationInterval = D3DPRESENT_INTERVAL_ONE; //D3DPRESENT_INTERVAL_ONEは、フルスクリーン時に、Windows側で自動V-Syncされるので表示遅延が発生する。RADEONはV-Syncお知らせタイミングがアバウトなようなので640x480フル時にはこれを使うしかなさそう。v2.65
		//D3DSWAPEFFECT_FLIPについて…D3DSWAPEFFECT_DISCARD(COPY)にして手動でVSyncを取るよりも、メモリ転送処理が省かれるためだいぶCPUへの負荷は軽いが
		//							  処理のタイミングが遅いときがあって処理落ち(＆ノイズ載り)がしやすいようなので、現状はDISCARD+IMMEDIATEで行く。
		//_pD3DParam.SwapEffect = D3DSWAPEFFECT_FLIP;
		//_pD3DParam.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	}
	else
	{
		_pD3DParam.SwapEffect = D3DSWAPEFFECT_DISCARD; //COPYより軽い
		_pD3DParam.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	}
	_pD3DParam.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
	hr = d3dDevReset(); //作成
	if (FAILED(hr))
	{
		MessageBox(_hWnd, "ERROR: DIRECT3D::CreateDevice() failed.    ", "Ootake", MB_OK);
		SCREEND3D_Deinit();
		return FALSE;
	}

	//ガンマ（明るさ調整）を計算済みのテーブルを用意。
	SCREEN_SetGamma(APP_GetScanLineType(), APP_GetScanLineDensity(), APP_GetTvMode());

	_LastTimeSyncTime = timeGetTime(); //前回Syncしたときのタイマーカウント
	_LastTimeSyncTime3 = _LastTimeSyncTime; //v2.43追加
	_SyncAjust  = 0; //VSyncがオフのときに1/60秒間隔にできるだけ近づけるための変数。v1.67
	_SyncAjust3 = 0; //v2.43追加
	_Frame = 0; //v2.43追加

	//画面をクリア
	SCREEND3D_Clear();

	return TRUE;
}


//スクリーンモードを変更します。 
BOOL
SCREEND3D_ChangeMode(
	Sint32		width,
	Sint32		height,
	Sint32		magnification,
	Uint32		flags)
{
	SCREEND3D_Deinit();
	return SCREEND3D_Init(width, height, magnification, flags);
}


//垂直帰線期間を待ちます。(VSync処理)
BOOL
SCREEND3D_WaitVBlank(
	BOOL	bDraw) //bDrawをTRUEにして呼ぶと描画も行う。FALSEの場合VSync待ちのみ。
{
	Sint32				t;
	D3DRASTER_STATUS	rs;
	HRESULT				hr;
	RECT				rc;
	UINT				bt, bt2;
	UINT				cy, cy2;
	Sint32				top;
	Sint32				h2;

	//Vistaで"Aero"か"Vistaスタンダード(Home Basicにのみ存在)"のデスクトップ利用時は、Windows自体でV-Syncを自動で行うので、処理がぶつかって遅くならないようにOotake側はV-Syncに構わず、先に描画を済ませる。v2.02
	if (((APP_GetWindowsVista())&&(APP_DwmIsCompositionEnabled()))||
		(APP_GetWindowsAutoVSyncBool())) //v2.65追加。明示的にWindowsの自動V-Syncを使うモードを設置。表示遅延が起こるので非推奨。ブラウン管なら大丈夫かな。
	{
		//Waitする前に描画処理を済ませる。
		if (bDraw)
		{
			//描画処理
			hr = _pD3DDev->Present(NULL, NULL, NULL, NULL); //Direct3Dは、SwapEffectをVSync待ちするものに設定すると、Present実行時に自動でVSync待ちもしてくれる。※自動VSyncだと処理が重いときがあるので現状は手動で待ち。
			if (hr == D3DERR_DEVICELOST)
			{
				d3dDevReset();
				_pD3DDev->Present(NULL, NULL, NULL, NULL);
			}
			if (APP_GetWindowsAutoVSyncBool())
				return TRUE;
			bDraw = FALSE; //描画処理完了
		}
	}

	if (_bSyncTo60HzScreen)
	{
		t = timeGetTime() - _LastTimeSyncTime;
		if (t <= 16) //17ms(1/60s=16.7ms)以上経過していた場合は待たない。(timeGetTime()の誤差があるのでt=17でもまだ16.1ms経過のこともあるが二度待ちしないことを優先）
		{
			//他のアプリのために処理を一休み。※VBlank待ちループ中はCPU占有率が100%になるので、これをやると省電力。
			t = 16 - t - 11; //-11は処理落ちしないためのマージン。-11OK。-10ほぼOK。-9以下だとテンポの揺らぎからわずかに音が篭る気がする。16=１コマの時間約16ms。
							 //-11も引くと、よほど速いPCでないと意味がないが、Sleep()はどうしても大きな誤差が出るときがあるようなので、この設定で。
			if ((t >= 1)&&(t <= 16-11)) //タイマーカウンタのオーバーフローも考えて、この範囲のときだけSleepをおこなう。
				Sleep(t);
			//待つ。GetRasterStatusメソッドで垂直同期を取る。
			//		ウィンドウが最小化されている時はなぜかPresentによるVSyncが効かない(環境にもよるかもしれないがDirect3D9の仕様っぽい)ので、手動でVSync処理を行う。
			cy = (UINT)GetSystemMetrics(SM_CYSCREEN);
			cy2 = cy - (cy / 5); //縦1200のディスプレイの場合で-250ライン(1200/5)までチェック。1ラインだけの判定だとSyncを見逃すことがあるので、マージンぶん。-300だとLightPSG時に２回VSyncしてしまう。マイナスが少ないとSync逃すことがありテンポやや乱れ。
			if (_Flags & SCREEN_FFULLSCREEN)
			{	//フルスクリーンの時は0ラインでVSyncさせる。
				//画面上部の隙間(＝下部の隙間)を計算
				if (APP_GetFullStretched(TRUE))
				{
					//アスペクト比を4:3に保つ
					if (_Width/4 >= _Height/3)
					{	//縦のほうが短いので縦を基準とする
						if (MAINBOARD_GetShowOverscanLeft() > 0) //左右のオーバースキャン領域を表示するなら
							top = (_Height - (Sint32)((double)_Height * 0.96137 + 0.5)) / 2; //0.96137=224/((268-256)/4*3+224)
						else
							top = 0;
					}
					else
					{	//横のほうが短いので横を基準とする
						h2 = (Sint32)((double)(_Width / 4.0 * 3.0 + 0.5));
						if (MAINBOARD_GetShowOverscanLeft() > 0) //左右のオーバースキャン領域を表示するなら
							top = (_Height - (Sint32)((double)h2 * 0.96137 + 0.5)) / 2; //0.96137=224/((268-256)/4*3+224)
						else
							top = (_Height - h2) / 2;
					}
				}
				else if (APP_GetStretched())
				{
					//アスペクト比をTV画面と同じ4:3に。縦の長さををPCE標準224の整数倍(２倍)にする
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
					top = (_Height - 224*_Magnification) / 2;
					if (MAINBOARD_GetShowOverscanTop() > 0) //上側のオーバースキャン領域を表示するなら
					{
						top -= MAINBOARD_GetShowOverscanTop() * _Magnification;
						if (top < 0) top = 0; //画面からはみ出してしまう場合は、ストレッチ(縮小)して対応。1440x900,1600x900などであり得る。
					}
				}
				bt = cy - top;
			}
			else if (IsIconic(_hWnd))
			{	//ウィンドウが最小化されている時は最下256ラインでVSyncさせる。
				bt = cy2;
			}
			else
			{	//ウィンドウモードの場合、ウィンドウ表示領域最下ラインを越えた所でSyncさせる。こうしないと画面上部でのSyncが間に合わない。v2.54
				GetWindowRect(_hWnd, &rc);
				bt = (UINT)rc.bottom;
			}
			bt2 = bt + (cy - cy2);
			while (TRUE)
			{
				_pD3DDev->GetRasterStatus(0, &rs);
				if (bt <= cy2)
				{
					if ((rs.ScanLine >= cy2)||(rs.ScanLine == 0)||(rs.InVBlank)) //スキャンラインの判定もしたほうがSyncを逃さずテンポ安定。v2.54
						break; //VBlank期間に入ったらループを抜け出す。
				}
				else //ウィンドウが下のほうにある場合,もしくはフルスクリーンモード
				{
					if ((_Flags & SCREEN_FFULLSCREEN)&&(cy >= 480)&&(APP_GetVSyncAdjust() > 0)) //PC環境によっては、少し早めにV-Blankを起こさないと画面上部の書き換えに間に合わない場合がある。古いRADEON機の640x480の画面モードで、これでタイミングを微調整すれば綺麗なV-Syncが出来る。400x300,320x240は補正すると逆に駄目。v2.62追加。v2.65更新
					{
						if ((rs.InVBlank)||(rs.ScanLine == 0)||(rs.ScanLine >= bt-APP_GetVSyncAdjust())||(rs.ScanLine <= bt2-cy)) //v2.62。「bt-APP_GetVSyncAdjust()」で早め(APP_GetVSyncAdjust()ドット分早く)にV-Blankを起こすことで、古めの機種で画面上部が乱れる問題を回避。値が-14だと速い機種(GeForce)で下部が乱れ。
							break; //VBlank期間に入ったらループを抜け出す。
					}
					else //通常
					{
						if ((rs.InVBlank)||(rs.ScanLine == 0)||(rs.ScanLine >= bt)||(rs.ScanLine <= bt2-cy))
							break; //VBlank期間に入ったらループを抜け出す。
					}
				}
				t = timeGetTime() - _LastTimeSyncTime;
				if ((t >= 18)||(t < 0)) //17.1ms以上経過していたら。orカウンタがオーバーフロー
					break;
			}
			//PRINTF("Test %d %d %d", bt, bt2, cy2);
		}
		_LastTimeSyncTime = timeGetTime(); //前回のSyncが終わったタイムとして記録しておく
		if (bDraw)
		{
			//描画処理
			hr = _pD3DDev->Present(NULL, NULL, NULL, NULL); //Direct3Dは、SwapEffectをVSync待ちするものに設定すると、Present実行時に自動でVSync待ちもしてくれる。※自動VSyncだと処理が重いときがあるので現状は手動で待ち。
			if (hr == D3DERR_DEVICELOST)
			{
				d3dDevReset();
				_pD3DDev->Present(NULL, NULL, NULL, NULL);
			}
		}
		return TRUE;
	}
	else //_bSyncTo60HzScreen が FALSEの場合
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
		if (bDraw)
		{
			//描画処理
			hr = _pD3DDev->Present(NULL, NULL, NULL, NULL); //PresentしてもVSync待ちせず描画だけする設定になっているので、待たずに描画される。
			if (hr == D3DERR_DEVICELOST)
			{
				d3dDevReset();
				_pD3DDev->Present(NULL, NULL, NULL, NULL);
			}
		}
		return TRUE;
	}
}


void*
SCREEND3D_GetBuffer()
{
	return (void*)_pScreenPixels;
}


const Sint32
SCREEND3D_GetBufferPitch()
{
	return _SurfacePitch/2;
}


//メイン(ゲーム)画面用オフスクリーンをロックして、書き込み(描画)用アドレスを設定する。v2.13更新。32bit単位で扱うようにした。
static BOOL
lock_offscreen_surface(
	LONG*				pPitch,
	Uint32**			ppPixels)
{
	D3DLOCKED_RECT	pLockedRect;
	HRESULT			hr;

	hr = _pD3DSGame->LockRect(&pLockedRect, NULL, 0);
	if (hr == D3D_OK)
	{
		*pPitch = pLockedRect.Pitch >> 2; //横１ラインのバイト数。32bit単位で扱うために4で割っておく。v2.13更新
		*ppPixels = (Uint32*)pLockedRect.pBits;
		return TRUE;
	}
	else
		return FALSE;
}


//メイン(ゲーム)画面用オフスクリーンをクリアします。
//※現状、パラメータの設定は反映されません。x,yは0固定。ソース解像度はオフスクリーン全体に固定。colorは0フィル固定。
void
SCREEND3D_FillRect(
	Sint32		x,
	Sint32		y,
	Sint32		width,
	Sint32		height,
	Uint32		color)
{
	LONG		pitch;
	Uint32*		pPixels;
	Uint32*		pPixels0;
	int			i,j;

	if (lock_offscreen_surface(&pitch, &pPixels))
	{
		pPixels0 = pPixels;
		for (i = 0; i < _Height; i++)
		{
			pPixels = pPixels0;
			for (j = 0; j < _Width; j++)
				*pPixels++ = 0x00; //4byteぶん(１ドット)書き込まれる。
			pPixels0 += pitch;
		}
		_pD3DSGame->UnlockRect();
	}
}


//画面全体をクリアします。
void
SCREEND3D_Clear()
{
	HRESULT		hr;

	_pD3DDev->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0,0,0), 1.0, 0);
	hr = _pD3DDev->Present(NULL, NULL, NULL, NULL); //Presentを実行しないと反映されない。
	if (hr == D3DERR_DEVICELOST)
	{
		d3dDevReset();
		_pD3DDev->Present(NULL, NULL, NULL, NULL);
	}
}


//テキストメッセージを設定します。
void
SCREEND3D_SetMessageText(
	char*	pText)
{
	_pMessageText = pText;
}


//テキストメッセージの表示処理
static void
print_message()
{
	HDC			dc;
	HFONT		prevFont;
	HFONT		hFont;
	LOGFONT		logFont;
	RECT		rcSrc;
	RECT		rcDst;
	RECT		rc;
	int			a,fs;

	if (!APP_GetInit())
		return; //最初の初期化が全て完了していない場合は、表示処理を行わない。CD-ROMゲームをレジュームしたときにメッセージ欄がごちゃつくのを回避。v1.67

	//デバイスコンテキストを取得
	if (_pD3DSText->GetDC(&dc) != D3D_OK)
		return;

	//フォントサイズを決定
	switch (_Magnification)
	{
		case 1: fs = 12; break;
		case 2:	fs = 16; break;
		case 3: fs = 12; break; //v2.44更新
		default: //4
			fs = 17; break; //v2.44更新
	}
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
	_pD3DSText->ReleaseDC(dc);
	
	//転送元位置＆サイズの調整
	rcSrc.left	= 0;
	rcSrc.top	= 0;
	rcSrc.right	= rc.right;
	rcSrc.bottom= fs;
	//転送先位置の調整＆表示した文字の範囲を見やすくするために拡大する。
	if ((APP_GetOverscanHideBlackLR())&&(MAINBOARD_GetShowOverscanLeft() == 0)) //左右のオーバースキャン領域に黒帯を付けるなら
		rcDst.left = _GameLeft; //黒帯があるのでギリギリまで左に寄せても見栄えOK。
	else //通常
		rcDst.left = _GameLeft + 3;
	if ((APP_GetOverscanHideBlackTop())&&(MAINBOARD_GetShowOverscanTop() < 8)) //上側のオーバースキャン領域に黒帯を付けるなら
	{
		rcDst.top = _GameTop; //黒帯があるのでギリギリまで上に寄せても見栄えOK。
		a = (8-MAINBOARD_GetShowOverscanTop())*_Magnification;
		if (a < 4) //黒帯部分が4ドット未満なら
			rcDst.top += (4-a);
	}
	else //通常
		rcDst.top = _GameTop + 4;
	if (_Magnification >= 3)
	{
		rcDst.right	= rcDst.left + rcSrc.right * 2; //横幅２倍拡大
		rcDst.bottom = rcDst.top + fs * 2; //縦幅２倍拡大
	}
	else
	{
		rcDst.right	= rcDst.left + rcSrc.right;
		rcDst.bottom = rcDst.top + fs;
	}
	if (rcDst.right > _GameRight) //メッセージが長すぎてはみ出てしまう場合
	{
		rcSrc.right = (LONG)((double)rcSrc.right / ((double)rcDst.right / (double)_GameRight)); //ソースを小さくする
		rcDst.right = _GameRight;
	}
	//転送
	_pD3DDev->StretchRect(_pD3DSText, &rcSrc, _pD3DSBack, &rcDst, D3DTEXF_NONE); //文字はクッキリさせるためNONEを使う。
}


//FPSの表示処理
static void
print_fps()
{
	HDC			dc;
	HFONT		prevFont;
	HFONT		hFont;
	LOGFONT		logFont;
	RECT		rcSrc;
	RECT		rcDst;
	RECT		rc;
	int			fs;
	char		text[3+1]; //一応３桁まで用意
	int			a;

	//デバイスコンテキストを取得
	if (_pD3DSFps->GetDC(&dc) != D3D_OK)
		return;

	//フォントサイズを決定
	switch (_Magnification)
	{
		case 1: fs = 12; break;
		case 2:	fs = 16; break;
		case 3: fs = 12; break; //v2.44更新
		default: //4
			fs = 17; break; //v2.44更新
	}
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
	_pD3DSFps->ReleaseDC(dc);
	
	//転送元位置＆サイズの調整
	rcSrc.left	= 0;
	rcSrc.top	= 0;
	rcSrc.right	= rc.right;
	rcSrc.bottom= fs;
	//転送先位置の調整＆表示した文字の範囲を見やすくするために拡大する。
	if ((APP_GetOverscanHideBlackLR())&&(MAINBOARD_GetShowOverscanLeft() == 0)) //左右のオーバースキャン領域に黒帯を付けるなら
		rcDst.left = _GameLeft; //黒帯があるのでギリギリまで左に寄せても見栄えOK。
	else //通常
		rcDst.left = _GameLeft + 3;
	if ((APP_GetOverscanHideBlackBottom())&&(MAINBOARD_GetShowOverscanBottom() < 8)) //下側のオーバースキャン領域に黒帯を付けるなら
	{
		rcDst.bottom = _GameBottom; //黒帯があるのでギリギリまで下に寄せても見栄えOK。
		a = (8-MAINBOARD_GetShowOverscanBottom())*_Magnification;
		if (a < 3) //黒帯部分が3ドット未満なら
			rcDst.bottom -= (3-a);
	}
	else //通常
		rcDst.bottom = _GameBottom - 3; //下側に表示
	if (_Magnification >= 3)
	{
		rcDst.right	= rcDst.left + rcSrc.right * 2; //横幅２倍拡大
	    rcDst.top = rcDst.bottom - fs * 2; //縦幅２倍拡大
	}
	else
	{
		rcDst.right	= rcDst.left + rcSrc.right;
	    rcDst.top = rcDst.bottom - fs;
	}
	//転送
	_pD3DDev->StretchRect(_pD3DSFps, &rcSrc, _pD3DSBack, &rcDst, D3DTEXF_NONE); //文字はクッキリさせるためNONEを使う。
}


//描画用バックサーフェスへ転送する処理など。
static void
zoom_d3dTensou(
	Uint16*		pTvW,	//転送元の横ピクセル数。※[0]～[255]の16+224+16(下部のOverscan)ラインぶん。ここには表示開始(例.オーバースキャン表示しない場合は[16]になる)のアドレスを入れる。
	Sint32		h,		//転送元の縦ピクセル数。通常は224。上下のオーバースキャンをする場合そのぶん増やしてここを呼ぶ。
	Sint32		executeCode)
{
	RECT		rcSrc;
	RECT		rcDst;
	Sint32		a,b,c,d,s,i,w2,h2;
	BOOL		bRasterChange = FALSE; //ラスタごとに異なる解像度が設定されていた場合TRUE
	Uint16*		pTvW2 = pTvW;
	Sint32		ot = 16-MAINBOARD_GetShowOverscanTop(); //転送元の開始Y座標
	Sint32		wMag; //横方向のソース倍率
	Sint32		hMag; //縦方向のソース倍率

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
			//フルストレッチ時は上下のオーバースキャン領域を表示できないため、上下のオーバースキャン領域をカット。
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
		//Direct3DのStretchRectではWindowの位置や枠の太さぶんなども調整する必要がない。常に(0,0)が描画の左上になる。
		a = 0;
		b = 0;
		if (APP_GetOverscanHideBlackTop()) //上側のオーバースキャン領域に黒帯を入れて隠す設定なら
			b += (8 - MAINBOARD_GetShowOverscanTop()) * _Magnification;
		c = APP_GetGameWidth(_Magnification);
		d = b + (MAINBOARD_GetShowOverscanTop() + 224 + MAINBOARD_GetShowOverscanBottom()) * _Magnification;
		if (((MAINBOARD_GetShowOverscanLeft() > 0)&&((*(pTvW+MAINBOARD_GetShowOverscanTop()+223)) == 512))|| //512のときはオーバースキャン表示しない
			((MAINBOARD_GetShowOverscanLeft() == 0)&&(APP_GetOverscanHideBlackLR()))||
			(executeCode == 3))
		{	//左右に黒帯を配置
			if (APP_GetVStretched()) //縦画面モードなら
			{
				a += (Sint32)(5.34 * (double)_Magnification + 0.5);		//5.34=299/256*(256/336)*6dot
				c -= (Sint32)(5.34 * (double)_Magnification + 0.5);
			}
			else if (APP_GetStretched())
			{
				a += (Sint32)(7.01 * (double)_Magnification + 0.5);		//7.01=299/256*6dot
				c -= (Sint32)(7.01 * (double)_Magnification + 0.5);
			}
		}
	}

	if (IsIconic(_hWnd)) //ウィンドウが最小化されている時。hesを最小化して聴くときなどに処理を軽くする。
		if (executeCode != 2) //スクリーンショット用へ転送以外の場合
		{
			//垂直帰線期間を待つ
			SCREEND3D_WaitVBlank(FALSE); //描画処理は行わない
			return;
		}

	//描画解像度の変更があった場合、ゴミが残らないように画面全体をクリアする。
	if (MAINBOARD_GetResolutionChange())
		_pD3DDev->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0,0,0), 1.0, 0);

	//ソース範囲を決定し、デバイスのバックサーフェス(DirectDrawでのプライマリサーフェスと同様)へ転送する。
	if (_Magnification >= 2) //x2以上の場合
	{
		if (APP_GetScanLineType() != 0)
		{
			if (APP_GetTvMode())
				wMag = 1;
			else
				wMag = 2; //x2以上も横は２倍ドット固定。（速度アップ＋拡大されたときにバイリニアフィルタがいい感じにかかる）
			hMag = _Magnification; //縦は倍率ぶんのソースを用意して転送。
		}
		else
		{
			wMag = 2;
			if (_Magnification == 2)
				hMag = _Magnification; //縦は倍率ぶんのソースを用意して転送。
			else
				hMag = _Magnification-1; //3x,4xのときは、それぞれ2x,3xに拡大。（ジャギー軽減＆速度アップ）
		}
	}
	else //x1の場合
		wMag = hMag = 1;
	_GameLeft   = a; //print_message()で表示位置決定のために、ゲーム画面転送先の座標を控えておく。
	_GameRight  = c; //
	_GameTop    = b; //
	_GameBottom = d; //
	if (!bRasterChange)
	{	//通常
		if (MAINBOARD_GetFourSplitScreen()) //妖怪道中記,ワールドコート,はにいいんざすかい,パワードリフト,サイコチェイサーの４分割画面の場合。v2.27更新
		{		
			SetRect(&rcSrc, s/4*wMag, ot*hMag, (s/4+s/2)*wMag, (ot+h)*hMag); //ソースの左右黒帯部分をカット
			//４分の１に縮小して４回転送
			c = a + (c-a)/2;
			d = b + (d-b)/2;
			w2 = c - a;
			h2 = d - b;
			for (i=1; i<=4; i++)
			{
				SetRect(&rcDst, a, b, c, d);
				switch (APP_GetD3DEffect())
				{
					case 1:
						if ((APP_GetTvMode())&&((_Magnification == 2)||(_Magnification == 4)))
							_pD3DDev->StretchRect(_pD3DSGame, &rcSrc, _pD3DSBack, &rcDst, D3DTEXF_LINEAR); //TVモードでx2,x4のPOINTだと1/4縮小の影響で画面が暗くなるのでLINEARにする。
						else
							_pD3DDev->StretchRect(_pD3DSGame, &rcSrc, _pD3DSBack, &rcDst, D3DTEXF_POINT); //だいぶドットがクッキリ
						break;
					case 2: _pD3DDev->StretchRect(_pD3DSGame, &rcSrc, _pD3DSBack, &rcDst, D3DTEXF_LINEAR); //ちょうどいい按配
						break;
					default: //0
						_pD3DDev->StretchRect(_pD3DSGame, &rcSrc, _pD3DSBack, &rcDst, D3DTEXF_NONE); //かなりドットがクッキリ
						break;
				}
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
				w2 = c / _Magnification;
				//PRINTF("%d, %d", s,w2);//test
				w2 = (s-w2)/2;
				SetRect(&rcSrc, w2*wMag, ot*hMag, (s-w2)*wMag, (ot+h)*hMag); //転送元を転送先と同じドット数に合わせる。
			}
			else if (executeCode == 5) //オーバースキャン領域の左右をカットする場合
				SetRect(&rcSrc, 6*wMag, ot*hMag, (s-6)*wMag, (ot+h)*hMag); //このとき黒帯部分のソースは6ドットx倍率。
			else //通常
				SetRect(&rcSrc, 0, ot*hMag, s*wMag, (ot+h)*hMag);

			switch (APP_GetD3DEffect())
			{
				case 1: _pD3DDev->StretchRect(_pD3DSGame, &rcSrc, _pD3DSBack, &rcDst, D3DTEXF_POINT); //だいぶドットがクッキリ
					break;
				case 2: _pD3DDev->StretchRect(_pD3DSGame, &rcSrc, _pD3DSBack, &rcDst, D3DTEXF_LINEAR); //ちょうどいい按配
					break;
				default: //0
					_pD3DDev->StretchRect(_pD3DSGame, &rcSrc, _pD3DSBack, &rcDst, D3DTEXF_NONE); //かなりドットがクッキリ
					break;
			}
		}
	}
	else
	{	//ラスタごとに異なる解像度が設定されていたとき。v1.28更新。１ラインずつ転送するようにした。v2.00更新。Direct3DならVistaでも高速。
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
				SetRect(&rcSrc, 6*wMag, (ot+i)*hMag, ((*pTvW)-6)*wMag, (ot+i)*hMag+hMag);
			else //通常
				SetRect(&rcSrc, 0, (ot+i)*hMag, (*pTvW)*wMag, (ot+i)*hMag+hMag);
			switch (APP_GetD3DEffect())
			{
				case 1: _pD3DDev->StretchRect(_pD3DSGame, &rcSrc, _pD3DSBack, &rcDst, D3DTEXF_POINT); //だいぶドットがクッキリ
					break;
				case 2: _pD3DDev->StretchRect(_pD3DSGame, &rcSrc, _pD3DSBack, &rcDst, D3DTEXF_LINEAR); //ちょうどいい按配
					break;
				default: //0
					_pD3DDev->StretchRect(_pD3DSGame, &rcSrc, _pD3DSBack, &rcDst, D3DTEXF_NONE); //かなりドットがクッキリ
					break;
			}
			pTvW++;
		}
	}

	if (executeCode == 2) return;//スクリーンショット用へ転送の場合ここで終了

	//テキストメッセージの表示。Direct3D9のStretchRectはプレーンサーフェス同士の等倍転送が出来ないので、
	//							DirectDrawのときと違い、テキストメッセージもバックバッファーへ直接転送するようにした。
	if (*_pMessageText != 0) //メッセージが設定されていれば
		print_message();

	//FPSの表示。v1.50追加
	if (APP_GetShowFPS())
		print_fps();

	RenderAchievementOverlays( rcDst );

	//垂直帰線期間を待つ＆実際の描画処理
	SCREEND3D_WaitVBlank(TRUE);
}


//描画。メモリ書き込み処理。
#define	GB	(d & 0x000000FF)
#define	GG	(d & 0x0000FF00)>>8
#define	GR	(d & 0x00FF0000)>>16
//zoom3x()，zoom2x()，zoom1x()等も用意。速度重視のためそれぞれ別ルーチンに。ラインごとに解像度を変えているゲームに対応。
//32ビットカラーx４倍用
static void
zoom4x_dd32(
	Uint32*		pSrc,			// 転送元のポインタ
	Sint32		srcY,			// 転送元のY座標
	Uint16*		pTvW,			// 各ラインの幅
	Sint32		srcH)			// 転送元の高さ
{
	int			i,j;
	Uint32*		pSrc0;
	Uint32*		pDst;
	Uint32*		pDst0;
	Uint32		d;

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
			*pDst++ = (_Gamma[GR] << 16)|(_Gamma[GG] << 8)| _Gamma[GB]; //１ドットぶん書き込まれる。
			*pDst++ = (_GammaS90[GR] << 16)|(_GammaS90[GG] << 8)| _GammaS90[GB]; //１ドットぶん書き込まれる。横を２倍拡大(90%縦スキャンラインで)
		}
		*pDst = 0; //Direct3Dでは右端に１ドットぶんクリア(黒)マージンを書いておかないと右端が乱れる。
		pDst = pDst0 + _Pitch;
		pSrc = pSrc0;
		for (j = 0; j < *pTvW; j++) //２列目は90スキャンラインで。v2.34更新。中央部分はグラデーションを付けずに平坦にしたほうが違和感がない。
		{
			d = *pSrc++;
			*pDst++ = (_GammaS90[GR] << 16)|(_GammaS90[GG] << 8)| _GammaS90[GB]; //v2.34更新
			*pDst++ = (_GammaS90[GR] << 16)|(_GammaS90[GG] << 8)| _GammaS90[GB]; //横を２倍拡大(90%縦スキャンラインで)
		}
		*pDst = 0;
		pDst = pDst0 + _Pitch2;
		pSrc = pSrc0;
		for (j = 0; j < *pTvW; j++) //３列目も90%スキャンラインで。v2.34更新。中央部分はグラデーションを付けずに平坦にしたほうが違和感がない。
		{
			d = *pSrc++;
			*pDst++ = (_GammaS90[GR] << 16)|(_GammaS90[GG] << 8)| _GammaS90[GB]; //v2.34更新
			*pDst++ = (_GammaS90[GR] << 16)|(_GammaS90[GG] << 8)| _GammaS90[GB];  //横を２倍拡大(90%縦スキャンラインで)。v2.34更新
		}
		*pDst = 0;
		pDst = pDst0 + _Pitch3;
		pSrc = pSrc0;
		for (j = 0; j < *pTvW; j++)
		{
			d = *pSrc++;
			*pDst++ = (_GammaS80[GR] << 16)|(_GammaS80[GG] << 8)| _GammaS80[GB];
			*pDst++ = (_GammaS80[GR] << 16)|(_GammaS80[GG] << 8)| _GammaS80[GB]; //横を２倍拡大(80%縦スキャンラインで)
		}
		*pDst = 0;
		pDst0 += _Pitch4;
		pSrc0 += SOURCE_PITCH;
		pTvW++;
	}
}

//32ビットカラーx３倍用
static void
zoom3x_dd32(
	Uint32*		pSrc,			// 転送元のポインタ
	Sint32		srcY,			// 転送元のY座標
	Uint16*		pTvW,			// 各ラインの幅
	Sint32		srcH)			// 転送元の高さ
{
	int			i,j;
	Uint32*		pSrc0;
	Uint32*		pDst;	//v2.13更新。32bit単位にして高速化。
	Uint32*		pDst0;
	Uint32		d;

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
			*pDst++ = (_Gamma[GR] << 16)|(_Gamma[GG] << 8)| _Gamma[GB]; //4byteぶん(１ドット)書き込まれる。
			*pDst++ = (_GammaS90[GR] << 16)|(_GammaS90[GG] << 8)| _GammaS90[GB]; //4byteぶん(１ドット)書き込まれる。横を２倍拡大(90%縦スキャンラインで)
		}
		*pDst = 0; //Direct3Dでは右端に１ドットぶんクリア(黒)マージンを書いておかないと右端が乱れる。
		pDst = pDst0 + _Pitch;
		pSrc = pSrc0;
		for (j = 0; j < *pTvW; j++) //２列目は90%スキャンラインにする。
		{
			d = *pSrc++;
			*pDst++ = (_GammaS90[GR] << 16)|(_GammaS90[GG] << 8)| _GammaS90[GB]; //4byteぶん(１ドット)書き込まれる。
			*pDst++ = (_GammaS90[GR] << 16)|(_GammaS90[GG] << 8)| _GammaS90[GB]; //4byteぶん(１ドット)書き込まれる。横を２倍拡大
		}
		*pDst = 0;
		pDst = pDst0 + _Pitch2;
		pSrc = pSrc0;
		for (j = 0; j < *pTvW; j++)
		{
			d = *pSrc++;
			*pDst++ = (_GammaS80[GR] << 16)|(_GammaS80[GG] << 8)| _GammaS80[GB]; //4byteぶん(１ドット)書き込まれる。
			*pDst++ = (_GammaS80[GR] << 16)|(_GammaS80[GG] << 8)| _GammaS80[GB]; //4byteぶん(１ドット)書き込まれる。横を２倍拡大
		}
		*pDst = 0;
		pDst0 += _Pitch3;
		pSrc0 += SOURCE_PITCH;
		pTvW++;
	}
}

//32ビットカラーx２倍用。
static void
zoom2x_dd32(
	Uint32*		pSrc,			// 転送元のポインタ
	Sint32		srcY,			// 転送元のY座標
	Uint16*		pTvW,			// 各ラインの幅
	Sint32		srcH)			// 転送元の高さ
{
	int			i,j;
	Uint32*		pSrc0;
	Uint32*		pDst;
	Uint32*		pDst0;
	Uint32		d;

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
		*pDst = 0; //Direct3Dでは右端に１ドットぶんクリア(黒)マージンを書いておかないと右端が乱れる。
		pDst = pDst0 + _Pitch;
		pSrc = pSrc0;
		for (j = 0; j < *pTvW; j++)
		{
			d = *pSrc++;
			*pDst++ = (_GammaS80[GR] << 16)|(_GammaS80[GG] << 8)| _GammaS80[GB];
			*pDst++ = (_GammaS80[GR] << 16)|(_GammaS80[GG] << 8)| _GammaS80[GB]; //横を２倍拡大(80%縦スキャンラインで)
		}
		*pDst = 0;
		pDst0 += _Pitch2;
		pSrc0 += SOURCE_PITCH;
		pTvW++;
	}
}

//横スキャンライン32ビットカラーx４倍用
static void
zoom4xHS_dd32(
	Uint32*		pSrc,			// 転送元のポインタ
	Sint32		srcY,			// 転送元のY座標
	Uint16*		pTvW,			// 各ラインの幅
	Sint32		srcH)			// 転送元の高さ
{
	int			i,j;
	Uint32*		pSrc0;
	Uint32*		pDst;
	Uint32*		pDst0;
	Uint32		d;

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
			*pDst++ = (_Gamma[GR] << 16)|(_Gamma[GG] << 8)| _Gamma[GB];
		}
		*pDst = 0; //Direct3Dでは右端に１ドットぶんクリア(黒)マージンを書いておかないと右端が乱れる。
		pDst = pDst0 + _Pitch;
		pSrc = pSrc0;
		for (j = 0; j < *pTvW; j++) //２列目は90%スキャンラインで。v2.35更新。中央部分はグラデーションを付けずに平坦にしたほうが違和感がない。
		{
			d = *pSrc++;
			*pDst++ = (_GammaS90[GR] << 16)|(_GammaS90[GG] << 8)| _GammaS90[GB];
			*pDst++ = (_GammaS90[GR] << 16)|(_GammaS90[GG] << 8)| _GammaS90[GB];
		}
		*pDst = 0;
		pDst = pDst0 + _Pitch2;
		pSrc = pSrc0;
		for (j = 0; j < *pTvW; j++) //３列目も90%スキャンラインで。v2.35更新。中央部分はグラデーションを付けずに平坦にしたほうが違和感がない。
		{
			d = *pSrc++;
			*pDst++ = (_GammaS90[GR] << 16)|(_GammaS90[GG] << 8)| _GammaS90[GB];
			*pDst++ = (_GammaS90[GR] << 16)|(_GammaS90[GG] << 8)| _GammaS90[GB];
		}
		*pDst = 0;
		pDst = pDst0 + _Pitch3;
		pSrc = pSrc0;
		for (j = 0; j < *pTvW; j++)
		{
			d = *pSrc++;
			*pDst++ = (_GammaS80[GR] << 16)|(_GammaS80[GG] << 8)| _GammaS80[GB];
			*pDst++ = (_GammaS80[GR] << 16)|(_GammaS80[GG] << 8)| _GammaS80[GB];
		}
		*pDst = 0;
		pDst0 += _Pitch4;
		pSrc0 += SOURCE_PITCH;
		pTvW++;
	}
}

//横スキャンライン32ビットカラーx３倍用
static void
zoom3xHS_dd32(
	Uint32*		pSrc,			// 転送元のポインタ
	Sint32		srcY,			// 転送元のY座標
	Uint16*		pTvW,			// 各ラインの幅
	Sint32		srcH)			// 転送元の高さ
{
	int			i,j;
	Uint32*		pSrc0;
	Uint32*		pDst;
	Uint32*		pDst0;
	Uint32		d;

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
			*pDst++ = (_Gamma[GR] << 16)|(_Gamma[GG] << 8)| _Gamma[GB];
		}
		*pDst = 0; //Direct3Dでは右端に１ドットぶんクリア(黒)マージンを書いておかないと右端が乱れる。
		pDst = pDst0 + _Pitch;
		pSrc = pSrc0;
		for (j = 0; j < *pTvW; j++) //２列目は90%スキャンラインにする。
		{
			d = *pSrc++;
			*pDst++ = (_GammaS90[GR] << 16)|(_GammaS90[GG] << 8)| _GammaS90[GB];
			*pDst++ = (_GammaS90[GR] << 16)|(_GammaS90[GG] << 8)| _GammaS90[GB];
		}
		*pDst = 0;
		pDst = pDst0 + _Pitch2;
		pSrc = pSrc0;
		for (j = 0; j < *pTvW; j++)
		{
			d = *pSrc++;
			*pDst++ = (_GammaS80[GR] << 16)|(_GammaS80[GG] << 8)| _GammaS80[GB];
			*pDst++ = (_GammaS80[GR] << 16)|(_GammaS80[GG] << 8)| _GammaS80[GB];
		}
		*pDst = 0;
		pDst0 += _Pitch3;
		pSrc0 += SOURCE_PITCH;
		pTvW++;
	}
}

//横スキャンライン32ビットカラーx２倍用。
static void
zoom2xHS_dd32(
	Uint32*		pSrc,			// 転送元のポインタ
	Sint32		srcY,			// 転送元のY座標
	Uint16*		pTvW,			// 各ラインの幅
	Sint32		srcH)			// 転送元の高さ
{
	int			i,j;
	Uint32*		pSrc0;
	Uint32*		pDst;
	Uint32*		pDst0;
	Uint32		d;

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
			*pDst++ = (_Gamma[GR] << 16)|(_Gamma[GG] << 8)| _Gamma[GB];
		}
		*pDst = 0; //Direct3Dでは右端に１ドットぶんクリア(黒)マージンを書いておかないと右端が乱れる。
		pDst = pDst0 + _Pitch;
		pSrc = pSrc0;
		for (j = 0; j < *pTvW; j++)
		{
			d = *pSrc++;
			*pDst++ = (_GammaS80[GR] << 16)|(_GammaS80[GG] << 8)| _GammaS80[GB];
			*pDst++ = (_GammaS80[GR] << 16)|(_GammaS80[GG] << 8)| _GammaS80[GB];
		}
		*pDst = 0;
		pDst0 += _Pitch2;
		pSrc0 += SOURCE_PITCH;
		pTvW++;
	}
}

//ノンスキャンラインで２倍３倍４倍用。32ビットカラー。
static void
zoom2x3x4xNS_dd32(
	Sint32		bairitsu,		// 2xか3xか4xか。2xなら2。3xなら3。4xなら4。
	Uint32*		pSrc,			// 転送元のポインタ
	Sint32		srcY,			// 転送元のY座標
	Uint16*		pTvW,			// 各ラインの幅
	Sint32		srcH)			// 転送元の高さ
{
	int			i,j,k;
	Uint32*		pSrc0;
	Uint32*		pDst;
	Uint32*		pDst0;
	Uint32		d;
	Sint32		bairitsu2; //縦のドットを何倍にデジタル拡大するか。x2かx3なら2。x4なら3。

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
				*pDst++ = (_Gamma[GR] << 16)|(_Gamma[GG] << 8)| _Gamma[GB];
				*pDst++ = (_Gamma[GR] << 16)|(_Gamma[GG] << 8)| _Gamma[GB]; //横を２倍拡大(スキャンラインなし)
			}
			*pDst = 0; //Direct3Dでは右端に１ドットぶんクリア(黒)マージンを書いておかないと右端が乱れる。
			pDst0 += _Pitch;
		}
		pSrc0 += SOURCE_PITCH;
		pTvW++;
	}
}

//32ビットカラーx１倍用。スキャンラインなし。
static void
zoom1xNS_dd32(
	Uint32*		pSrc,			// 転送元のポインタ
	Sint32		srcY,			// 転送元のY座標
	Uint16*		pTvW, 			//
	Sint32		srcH)			// 転送元の高さ
{
	int			i,j;
	Uint32*		pSrc0;
	Uint32*		pDst;
	Uint32*		pDst0;
	Uint32		d;

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
		*pDst = 0; //Direct3Dでは右端に１ドットぶんクリア(黒)マージンを書いておかないと右端が乱れる。
		pDst0 += _Pitch;
		pSrc0 += SOURCE_PITCH;
		pTvW++;
	}
}

//TV Mode の32ビットカラーｘ４倍用。
static void
zoom4xTV_dd32(
	Uint32*		pSrc,			// 転送元のポインタ
	Sint32		srcY,			// 転送元のY座標
	Uint16*		pTvW,			// 各ラインの幅
	Sint32		srcH)			// 転送元の高さ
{
	int			i,j;
	Uint32*		pSrc0;
	Uint32*		pDst;
	Uint32*		pDst0;
	Uint32		d;

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
		*pDst = 0; //Direct3Dでは右端に１ドットぶんクリア(黒)マージンを書いておかないと右端が乱れる。
		pDst = pDst0 + _Pitch;
		pSrc = pSrc0;
		for (j = 0; j < *pTvW; j++) //２列目は90%スキャンラインで。v2.35更新。中央部分はグラデーションを付けずに平坦にしたほうが違和感がない。
		{
			d = *pSrc++;
			*pDst++ = (_GammaS90[GR] << 16)|(_GammaS90[GG] << 8)| _GammaS90[GB];
		}
		*pDst = 0;
		pDst = pDst0 + _Pitch2;
		pSrc = pSrc0;
		for (j = 0; j < *pTvW; j++) //３列目も90%スキャンラインで。v2.35更新。中央部分はグラデーションを付けずに平坦にしたほうが違和感がない。
		{
			d = *pSrc++;
			*pDst++ = (_GammaS90[GR] << 16)|(_GammaS90[GG] << 8)| _GammaS90[GB];
		}
		*pDst = 0;
		pDst = pDst0 + _Pitch3;
		pSrc = pSrc0;
		for (j = 0; j < *pTvW; j++)
		{
			d = *pSrc++;
			*pDst++ = (_GammaS80[GR] << 16)|(_GammaS80[GG] << 8)| _GammaS80[GB];
		}
		*pDst = 0;
		pDst0 += _Pitch4;
		pSrc0 += SOURCE_PITCH;
		pTvW++;
	}
}

//TV Mode の32ビットカラーｘ３倍用。
static void
zoom3xTV_dd32(
	Uint32*		pSrc,			// 転送元のポインタ
	Sint32		srcY,			// 転送元のY座標
	Uint16*		pTvW,			// 各ラインの幅
	Sint32		srcH)			// 転送元の高さ
{
	int			i,j;
	Uint32*		pSrc0;
	Uint32*		pDst;
	Uint32*		pDst0;
	Uint32		d;

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
		*pDst = 0; //Direct3Dでは右端に１ドットぶんクリア(黒)マージンを書いておかないと右端が乱れる。
		pDst = pDst0 + _Pitch;
		pSrc = pSrc0;
		for (j = 0; j < *pTvW; j++) //２列目は90%スキャンラインにする。
		{
			d = *pSrc++;
			*pDst++ = (_GammaS90[GR] << 16)|(_GammaS90[GG] << 8)| _GammaS90[GB];
		}
		*pDst = 0;
		pDst = pDst0 + _Pitch2;
		pSrc = pSrc0;
		for (j = 0; j < *pTvW; j++)
		{
			d = *pSrc++;
			*pDst++ = (_GammaS80[GR] << 16)|(_GammaS80[GG] << 8)| _GammaS80[GB];
		}
		*pDst = 0;
		pDst0 += _Pitch3;
		pSrc0 += SOURCE_PITCH;
		pTvW++;
	}
}

//TV Mode の32ビットカラー ｘ２倍用。
static void
zoom2xTV_dd32(
	Uint32*		pSrc,			// 転送元のポインタ
	Sint32		srcY,			// 転送元のY座標
	Uint16*		pTvW,			// 各ラインの幅
	Sint32		srcH)			// 転送元の高さ
{
	int			i,j;
	Uint32*		pSrc0;
	Uint32*		pDst;
	Uint32*		pDst0;
	Uint32		d;

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
		*pDst = 0; //Direct3Dでは右端に１ドットぶんクリア(黒)マージンを書いておかないと右端が乱れる。
		pDst = pDst0 + _Pitch;
		pSrc = pSrc0;
		for (j = 0; j < *pTvW; j++)
		{
			d = *pSrc++;
			*pDst++ = (_GammaS80[GR] << 16)|(_GammaS80[GG] << 8)| _GammaS80[GB];
		}
		*pDst = 0;
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
#undef GR
#undef GG
#undef GB


//pSrcの描画データをパソコン表示向けにエンコードし、バックバッファへ画像を書き込みます。
//		ラインごとに解像度を変えているゲーム（龍虎の拳，あすか120%など）にも対応。
void
SCREEND3D_Blt(
	Uint32*		pSrc,
	Sint32		srcX,
	Sint32		srcY,
	Uint16*		pTvW,	//転送元の横ピクセル数。※srcHラインの数ぶん
	Sint32		srcH,
	Sint32		executeCode)  //Kitao追加。実行コード。0…エンコードだけ行う。1…プライマリ画面へ転送も行う。2…スクリーンショット用画面へ転送を行う。
								  //					   3…左右に黒帯(オーバースキャン部)を配置しての転送(あとは1と同じ)
								  //					   5…左右のオーバースキャン部をカットしての転送(あとは1と同じ)
{
	Uint32*			pSrc32;
	Uint16*			pTvW2;
	Sint32			scanLineType;

	if (pSrc==NULL) return;

	//v1.28更新。ライン途中からのエンコードに対応。
	pSrc32 = pSrc + srcY * 512 + srcX;
	pTvW2 = pTvW + srcY;

	if (srcH > 0) //v2.43更新
	{ 
		if (lock_offscreen_surface(&_Pitch, &_pPixels)) //バックサーフェスをロック
		{
			_Pitch2 = _Pitch * 2;
			_Pitch3 = _Pitch * 3;
			_Pitch4 = _Pitch * 4;
		
			//速度重視のためそれぞれの画面モードで、別々のルーチンを使う。
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
				monoColor_32(_Magnification, scanLineType, pSrc32, srcY, pTvW2, srcH, (MAINBOARD_GetForceMonoColor() == 2)); //32bitカラー
			}
			else
			{	//通常
				switch (scanLineType)
				{
					case 0: //ノンスキャンライン
						switch (_Magnification)
						{
							case 1:	zoom1xNS_dd32(pSrc32, srcY, pTvW2, srcH); //32bitカラー
									break;
							case 2:
							case 3:
							case 4:
								zoom2x3x4xNS_dd32(_Magnification, pSrc32, srcY, pTvW2, srcH); //32bitカラー
								break;
						}
						break;
					case 1: //縦横スキャンライン
						switch (_Magnification)
						{
							case 1:	zoom1xNS_dd32(pSrc32, srcY, pTvW2, srcH); //32bitカラー。※ノンスキャンラインと同じ。
									break;
							case 2:	zoom2x_dd32(pSrc32, srcY, pTvW2, srcH); //32bitカラー
									break;
							case 3:	zoom3x_dd32(pSrc32, srcY, pTvW2, srcH); //32bitカラー
									break;
							case 4:	zoom4x_dd32(pSrc32, srcY, pTvW2, srcH); //32bitカラー
									break;
						}
						break;
					case 2: //横のみスキャンライン
						switch (_Magnification)
						{
							case 1:	zoom1xNS_dd32(pSrc32, srcY, pTvW2, srcH); //32bitカラー。※ノンスキャンラインと同じ。
									break;
							case 2:	zoom2xHS_dd32(pSrc32, srcY, pTvW2, srcH); //32bitカラー
									break;
							case 3:	zoom3xHS_dd32(pSrc32, srcY, pTvW2, srcH); //32bitカラー
									break;
							case 4:	zoom4xHS_dd32(pSrc32, srcY, pTvW2, srcH); //32bitカラー
									break;
						}
						break;
					//case 3: //縦のみスキャンライン（将来実装）。5以降にもエフェクトを実装できる。
					case 4: //TV Mode
						switch (_Magnification)
						{
							case 1:	zoom1xNS_dd32(pSrc32, srcY, pTvW2, srcH); //32bitカラー。※ノンスキャンラインと同じ。
									break;
							case 2:	zoom2xTV_dd32(pSrc32, srcY, pTvW2, srcH); //32bitカラー
									break;
							case 3:	zoom3xTV_dd32(pSrc32, srcY, pTvW2, srcH); //32bitカラー
									break;
							case 4:	zoom4xTV_dd32(pSrc32, srcY, pTvW2, srcH); //32bitカラー
									break;
						}
						break;
				}
			}

			//ロックを解除
			_pD3DSGame->UnlockRect();
		}
	}

	//プライマリ画面への転送
	if (executeCode > 0)
		zoom_d3dTensou(pTvW+(16-MAINBOARD_GetShowOverscanTop()),
					   srcY+srcH-(16-MAINBOARD_GetShowOverscanTop()),
					   executeCode);
}


//VSync(垂直帰線待ち)を行うかどうかを設定。VSyncをする設定でもSyncできない環境の場合は自動で一時的にオフにする。
void
SCREEND3D_SetSyncTo60HzScreen(
	BOOL	bSyncTo60HzScreen)
{
	BOOL		bPrevSyncTo60HzScreen = _bSyncTo60HzScreen;
	DEVMODE		dm;

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
	//設定が変更された場合、Direct3Dオブジェクトを初期化し直す。
	if (_bSyncTo60HzScreen != bPrevSyncTo60HzScreen)
		if (_pD3D != NULL) //初期化が終了していれば。初期化前に呼ばれた場合(App.cから呼ばれる)は変数の更新のみ。
			SCREEND3D_ChangeMode(_Width, _Height, _Magnification, _Flags);
}

//VSync(垂直帰線待ち)を行っているかどうかを返す。VSyncをする設定でもSyncできない環境の場合はFALSEが返る。v2.43
BOOL
SCREEND3D_GetSyncTo60HzScreen()
{
	return _bSyncTo60HzScreen;
}


//Windowsの自動V-Sync機能周りの変数を更新する。v2.65
void
SCREEND3D_UpdateWindowsAutoVSync()
{
	SCREEND3D_ChangeMode(_Width, _Height, _Magnification, _Flags); //Direct3Dオブジェクトを初期化し直す。
}


//前回のVBlank待ちが終わった時刻を返す。v1.28
DWORD
SCREEND3D_GetLastTimeSyncTime()
{
	return _LastTimeSyncTime;
}


//スクリーンショットをファイルに書き込む。v2.12
void
SCREEND3D_WriteScreenshot(
	FILE*	fp)
{
	int				i,j;
	D3DLOCKED_RECT	pLockedRect;
	HRESULT			hr;
	LONG			pitch;
	Uint32*			pPixels0;
	Uint32*			pPixels;
	Uint32			BGRR; //Bule,Green,Red,Reserved
	Sint32			width;
	Sint32			height;
	Sint32			left;
	Sint32			top;
	Sint32			w2,h2;
	Sint32			wp;

	//バッファに再描画する。
	if (((APP_GetOverscanHideBlackLR())&&(MAINBOARD_GetShowOverscanLeft() == 0))|| //左右のオーバースキャン領域に黒帯表示していた場合
		((APP_GetOverscanHideBlackTop())&&(MAINBOARD_GetShowOverscanTop() < 8))) //上側のオーバースキャン領域に黒帯表示していた場合
			MAINBOARD_SetResolutionChange(TRUE); //描画時にゴミが残らないように画面全体をクリアしてから描画する。
	MAINBOARD_DrawScreenshot();
	//※_pD3DDev->Presentでの表示更新は行わない。
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
		left = 0;
		top = 0;
	}

	//キャプチャするメモリをロック
	hr = _pD3DSBack->LockRect(&pLockedRect, NULL, D3DLOCK_READONLY); //READONLYで高速化。v2.28
	if (hr != D3D_OK) return;

	pitch = pLockedRect.Pitch / 4; //32bit単位で処理するためピッチは1/4に。
	pPixels0 = (Uint32*)pLockedRect.pBits + pitch*(top+height-1) + left; //最下段からファイルに出力して行く(BMPの仕様)。

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
	_pD3DSBack->UnlockRect();
}


//Kitao追加。スクリーンショット時のために一時的に描画倍率を変更する。Screen.cppから使用。v2.13
void
SCREEND3D_SetMagnification(
	Sint32	magnification)
{
	_Magnification = magnification;
}

//Kitao追加。v2.13
Sint32
SCREEND3D_GetMagnification()
{
	return _Magnification;
}

//Kitao追加。v2.19
BOOL
SCREEND3D_GetOldVideoChip()
{
	return _bOldVideoChip;
}
