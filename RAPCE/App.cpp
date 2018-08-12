/******************************************************************************
Ootake
・使用するCDドライブを選択できるようにした。
・バックアップメモリの保存をゲームごとにおこなうようにした。
・CD-ROMのゲームを起動したときは、ステートセーブ＆バックアップメモリセーブ時に
  ゲーム名をファイル名として使うようにした。
・レジューム機能をつけた。v0.90
・早回しモードを付けた。v0.92
・縦画面モードを付けた。（全てのゲームで縦長モード表示が可能）
・DirectX使用時にデフォルトではパソコンのCPUパワーを100%使用するので、パワーを
  セーブしてCPU利用率を下げる省電力モード（音質がやや落ちる）を付けた。
・現状では、描画をDirectX（ハードウェアアクセラレーション）専用とした。
・ウィンドウモードでもVSyncを有効にした。
・フルスクリーンモードでも32ビットカラーに対応した。
・サウンドの再生サンプルレートを44.1KHzに固定した(CD-DA再生時の速度アップのた
  め)。
・PSGのボリュームを個別に設定できるようにした。
・各サウンドのボリュームを音割れしない最大限のところまで引き上げて音質を向上さ
  せた。
・サウンドのミュート機能、簡易ボリューム調整機能を付けた。
・他のアプリへの影響を考慮して、Windowsのボリュームミキサーは不使用にした。
・音質を重視した(音は大きく遅れる)ビッグサウンドバッファモードを付けた。
・パッドの入力更新をここでは行わず、JoyPad.cで入力要求があるたびにそのつど、入
  力状態を更新するようにし、入力反応性を実機に近づけた。
・２ボタンパッドのボタン設定と、６ボタンパッドのボタン設定を、それぞれ別々に保
  存するようにした。
・PCEパッド1～5の操作をキーボードにも自由に割り当てられるようにした。
・起動時に、前回使用したときの位置にウィンドウを移動させるようにした。
・スクリーンセーバーの起動を禁止するようにした。
・CD-ROMのアクセスを実機並の遅いスピードにする設定を付けた。
・ラスタ割り込みのタイミングを最適化する設定を付けた。
・スプライト欠けを再現する設定を付けた。
・マニュアルを見るメニューを付けた。
・複数のステートセーブができるようにした。v0.51
・hesファイルの読み込みにも対応した。v0.52
・フルスクリーンでスタートするか、ウィンドウでスタートするかを選択するメニュー
  を付けた。v0.55
・CD-ROMアクセス中でも安定してステートセーブできるようにした。v0.61
・システムメッセージをスクリーン上にも表示できるようにした。v0.64
・ZIP圧縮したイメージファイルを読み込めるようにした。v0.65
・CD-ROMを入れ替えて別のゲームを始められるメニューを追加した。v0.73
・リセットせずにCD-ROMを入れ替えられるメニューを追加した。「ＣＤバトル 光の勇
  者たち」や内蔵音楽ＣＤプレイヤーなどで必要。v1.00
・Windows Vistaでの動作に対応した。v0.95。
・オーディオ処理時に、クリティカルセクションは必要ない(書き込みが同時に行われ
  るわけではない)ようなので、省略し高速化した。v1.09
・DirectX9に対応し、Direct3Dでの描画にも対応した。v2.00
・スクリーンショット機能を付けた。v2.12

Copyright(C)2006-2012 Kitao Nakamura.
    Attach the source code when you open the remodeling version and the
    succession version to the public. and, please contact me by E-mail.
    Business use is prohibited.
	Additionally, it applies to "GNU General Public License". 
	改造版・後継版を公開なさるときは必ずソースコードを添付してください。
	その際に事後でかまいませんので、ひとことお知らせいただけると幸いです。
	商的な利用は禁じます。
	あとは「GNU General Public License(一般公衆利用許諾契約書)」に準じます。

*******************************************************************************
	[App.c]
		アプリケーションの動作を記述します。
		Implement the application's behaviour.

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
#pragma warning(disable : 4995) //dshow.h使用時の警告を非表示。v2.52
#define _WIN32_DCOM //v2.23追加

#define DIRECTINPUT_VERSION	0x0800	//Kitao追加。環境にもよるかもしれないが、DirectInput5が軽い。7だとやや遅延あり。スペースハリアーがわかりやすい。

//Kitao更新。v2.00から開発環境を"Visual C++ 2008"に変更しました。
//※Vista以降でOotakeをProgram Filesフォルダで利用したときに、設定＆セーブファイルがリダイレクトされることを前提にコーディングしてあります。
//  ですので、"Visual C++ 2008"で、構成プロパティの"リンカ→マニフェストファイル→ユーザーアカウント制御(UAC)を有効にする"を「いいえ」に
//  設定してコンパイルしてください。（添付のOotake.vcprojを使うとすでに設定済み）。
//※dinput.libは、古いもの(17.3KBサイズ。DirectX 9.0 SDK Update - (October 2004)以前)を使ってください。
//  反応速度が最も速いです。このlibの違いで、例えばスーパースターソルジャー等でプレイ感とスコアに顕著な差が出ます。
//  dinput.libのダウンロード先など、詳しくはReadme.txtを見てください。

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <shlobj.h>
#include <shlwapi.h> //PathFileExists()で必要
#include <dshow.h> //リンカの入力にstrmiids.libの追加も必要。v2.52
//#include <unistd.h>
#include "UNZIP32.H"
#include "App.h"
#include "MainBoard.h"
#include "Screen.h"
#include "ScreenD3D.h"
#include "Resolution.h"
#include "Config.h"
#include "WinMain.h"
#include "AppEvent.h"
#include "Menu.h"
#include "Printf.h"
#include "PadConfig.h"
#include "AudioConfig.h"
#include "CDInterface.h"
#include "TocDB.h"
#include "Startup.h"
#include "AudioOut.h"
#include "CDInstall.h"
#include "Debug.h"
#include "WriteMemory.h"
#include "RecentRom.h"


//	##RA
#include "RA_Interface.h"
#include "RA_Implementation.h"


//Kitao追加。UNZIP32.DLL用定義
typedef WORD (WINAPI* UNZIPGETVERSION)(void);
typedef HARC (WINAPI* UNZIPOPENARCHIVE)(const HWND, LPCSTR, const DWORD);
typedef int (WINAPI* UNZIPCLOSEARCHIVE)(HARC);
typedef int (WINAPI* UNZIPFINDFIRST)(HARC, LPCSTR, LPINDIVIDUALINFO);
typedef int (WINAPI* UNZIP)(const HWND, LPCSTR, LPSTR, const DWORD);

//Kitao追加。dwmapi.dll用定義。v2.21
static HINSTANCE _hDwmapiDll;
typedef HRESULT (WINAPI* FuncDwmIsCompositionEnabled) (BOOL*);
static FuncDwmIsCompositionEnabled _FuncDwmIsCompositionEnabled;
typedef HRESULT (WINAPI* FuncDwmEnableComposition) (UINT);
static FuncDwmEnableComposition _FuncDwmEnableComposition;


static char		_AppName[100] = MAINCAPTION; //Kitao追加。Windowに表示するアプリ名
static char		_SoftVersion[5] = "2.65"; //Kitao追加。バージョン情報。5バイト(4文字)固定。

static BOOL		_bInit = FALSE;//Kitao追加。初期設定が完了したらTRUE
static BOOL		_bWindows9x = FALSE; //Kitao追加。OSが98/Me/95ならTRUE。v1.36
static BOOL		_bWindowsVista = FALSE; //Kitao追加。OSがVista以上ならTRUE。Win7もTRUE。セーブファイルがリダイレクトする動作の対応に必要。
static BOOL		_bWindows7 = FALSE; //Kitao追加。OSがWin7以上ならTRUE。バッファ量を最適化するために使用。v2.20
static char		_VirtualStorePath[MAX_PATH+1] = ""; //Kitao追加。Vista/Win7でセーブファイルがリダイレクトする動作の対応に必要。
static char		_DesktopPath[MAX_PATH+1] = ""; //Kitao追加。v1.29
static HMIXER	_hMix = (HMIXER)INVALID_HANDLE_VALUE; //ボリュームミキサーのハンドル。v1.61
static MIXERCONTROLDETAILS			_MixerControlD; //v2.16更新
static MIXERCONTROLDETAILS_UNSIGNED	_MixerControlDU;//v2.16更新
static DWORD	_dwWaveControlID; //WAVEボリュームのコントロールID。v1.61
static DWORD	_dwWaveMaxVolume; //WAVEボリュームの最大値。v1.61
static DWORD	_dwDefaultWindowsVolumeValue;//Kitao追加。起動時のWindowsのボリュームコントロール(WAVE)を退避しておく変数。Ootake終了時、この値に戻す。
static Sint32	_PrevMouseX = -1;//Kitao追加
static Sint32	_PrevMouseY = -1;//Kitao追加
static Sint32	_MouseStopCount = 0;//Kitao追加
static char		_OpenFilePathName[MAX_PATH+1];
static char		_GameFilePathName[MAX_PATH+1] = "";
static char		_AppPath[MAX_PATH+1];
static char		_ZipTempFilePathName[MAX_PATH+1];//Kitao追加
static char		_GameFileNameBuf[MAX_PATH+1];//Kitao追加
static BOOL		_bCDGame = FALSE;//Kitao追加。CDのゲームを起動したならTRUEになる（ステートセーブで個別のファイル名が付く）
static BOOL		_bCueFile = FALSE;//Kitao追加。フルインストールしたCUEファイルから起動している場合TRUEに。v2.24
static char		_CueFilePathName[MAX_PATH+1];//Kitao追加。CUEファイルのファイル名。v2.24
static Sint32	_OtherSysCard = 0;//Kitao追加。デフォルト以外のシステムカードを使用中は1～2の値に。v1.49
static BOOL		_bF1NoReset = FALSE;//Kitao追加。F1キーでリセットせずにＣＤ情報だけ更新する場合TRUE。"ＣＤバトル 光の勇者たち" で使用。
static BOOL		_bHesFile = FALSE;//Kitao追加
static BOOL		_bAutoBigSoundBuffer = FALSE;//Kitao追加
static BOOL		_bResumeMode;//Kitao追加
static BOOL		_bLoadingResumeFile = FALSE;//v1.61追加
static BOOL		_bAutoResumeMode = FALSE;//Kitao追加。ゲーム起動時に自動でレジューム機能をオンにするならTRUE。v1.54
static Sint32	_OverClockTypeR;//Kitao追加
static Sint32	_RenshaSpeedSelR;//Kitao追加
static BOOL		_bUseThreeButtonR;//Kitao追加
static BOOL		_bArcadeCardR;//Kitao追加
static BOOL		_bUseVideoSpeedUpButton;//Kitao追加。VideoSpeedUp時に、専用のボタンを押している間だけスピードアップするならTRUE。常にスピードアップするならFALSE。
static BOOL		_bConfigReturnToDefault = FALSE;//Kitao追加。v1.03。設定をデフォルトに戻すときに使用。
static BOOL		_bSpriteLayer  = TRUE; //v2.06追加。スプライトを全非表示にするならFALSE。(開発向け)
static BOOL		_bSprite2Layer = TRUE; //v2.06追加。スーパーグラフィックスの2つ目VDCのスプライトを全非表示にするならFALSE。(開発向け)
static BOOL		_bBGLayer      = TRUE; //v2.06追加。BGを非表示にするならFALSE。(開発向け)
static BOOL		_bBG2Layer     = TRUE; //v2.06追加。スーパーグラフィックスの2枚目のBGを非表示にするならFALSE。(開発向け)
static BOOL		_bInvalidateCdInstall = FALSE; //v2.34追加。CDインストールしたファイルを利用せずに実CD-ROMで利用する場合TRUE。(動作テスト向け)

// [APP Device] (default values)
static Sint32	_CurrentCdDrive			= 0;//Kitao追加。使用するCDドライブナンバー。※0なら１台目。1なら２台目。
static BOOL		_bCDSpeedDown			= FALSE;//Kitao追加。PCのCD-ROMドライブのREADスピードを遅くするならTRUE。スターパロジャーなど読み込みが厳しいゲームで必要。静音化にもなる。ドライブによっては機能しないかもしれない。v2.50 ※テスト不足のため現在未使用。
static Sint32	_MySetOverClockType		= 100;//Kitao追加。[Delete]キーを押したときに設定できるCPUオーバークロックスピード。v1.61
static Sint32	_VideoSpeedUpButton		= -1;//Kitao追加。VideoSpeedUp時に使うボタン。
static Sint32	_FastForwarding			= 0;//Kitao追加
static BOOL		_bSoundAjust			= TRUE;//Kitao追加
static BOOL		_bLoadStateSpeedSetting	= FALSE;//Kitao追加。VideoSpeedUpとCpuSpeedUpの設定をステートロード時に反映するならTRUE。v2.36からデフォルトでは反映しないようにした。
static char		_CurrentOpenPath[MAX_PATH+1] = "";//Kitao追加。ROMイメージ等を開くときのカレントパス。v1.29
static char		_CurrentSavePath[MAX_PATH+1] = "";//Kitao追加。WAVサウンドなどを保存するときのカレントパス。v1.29
static char		_OtherSysCardPath1[MAX_PATH+1] = "";//Kitao追加。別のシステムカードのファイルパス(ファイル名も含む)。v1.49
static char		_OtherSysCardPath2[MAX_PATH+1] = "";//Kitao追加。別のシステムカードのファイルパス(ファイル名も含む)。v1.49
static Sint32	_SaveStateButton		= DIK_S;//Kitao追加
static Sint32	_LoadStateButton		= DIK_L;//Kitao追加
static Sint32	_ScreenshotButton		= DIK_SYSRQ;//Kitao追加。PrintScreenキー
static Sint32	_PauseButton			= -1;//Kitao追加。ポーズを行うためのボタン。初期設定では誤動作を避けるため未設定にしておく。v2.49
static Sint32	_FunctionButton			= -1;//Kitao追加。他のボタンと組み合わせてショートカット動作を行うためのボタン。初期設定では誤動作を避けるため未設定にしておく。v2.38
static Sint32	_FuncSaveStateButton	= -1;//Kitao追加。ファンクションボタンとこのボタンを組み合わせてセーブステートを行える。v2.38
static Sint32	_FuncLoadStateButton	= -1;//Kitao追加。ファンクションボタンとこのボタンを組み合わせてロードステートを行える。v2.38
static BOOL		_bFuncVolume			= TRUE;//Kitao追加。ファンクションボタン＋十字キーで音量変更を可能にするならTRUE。v2.38
static BOOL		_bFuncTurboButton		= TRUE;//Kitao追加。ファンクションボタン＋I,IIボタンで連射切替を可能にするならTRUE。v2.38
static BOOL		_bFuncScreenshot		= TRUE;//Kitao追加。ファンクションボタン＋SELECTボタンでスクリーンショットを可能にするならTRUE。v2.38
static BOOL		_bFuncRecent			= TRUE;//Kitao追加。ファンクションボタン＋RUNボタンで最近開いたROMの読み込みを可能にするならTRUE。v2.48
static BOOL		_bFuncVSpeedUp			= TRUE;//Kitao追加。ファンクションボタン＋早回しボタンで早回し切替を可能にするならTRUE。v2.38
static BOOL		_bKeyRepeatSaveState	= FALSE;//Kitao追加
static BOOL		_bKeyRepeatLoadState	= FALSE;//Kitao追加
static BOOL		_bKeyRepeatScreenshot	= FALSE;//Kitao追加
static BOOL		_bKeyRepeatPause		= FALSE;//Kitao追加
static BOOL		_bKeyRepeatVolumeChangeUD = FALSE;//Kitao追加
static BOOL		_bKeyRepeatVolumeChangeLR = FALSE;//Kitao追加
static BOOL		_bKeyRepeatTurboSelect1	= FALSE;//Kitao追加
static BOOL		_bKeyRepeatTurboSelect2	= FALSE;//Kitao追加
static BOOL		_bKeyRepeatFuncScreenshot = FALSE;//Kitao追加
static BOOL		_bKeyRepeatSpeedChange	= FALSE;//Kitao追加
static BOOL		_bKeyRepeatFuncSave		= FALSE;//Kitao追加
static BOOL		_bKeyRepeatFuncLoad		= FALSE;//Kitao追加
static BOOL		_bKeyRepeatFuncRecent	= FALSE;//Kitao追加
static DWORD	_PriorityClass			= NORMAL_PRIORITY_CLASS;//Kitao追加。Ootake実行の優先度。hesやBGMを聞きながらネットサーフィンなどするときにHIGHじゃないと音割れが起こる。(特にDirect3D描画使用時)

// [APP DISPLAY] (default values)
static Sint32	_ScreenWidth			= 299 * 3;//Kitao更新。v2.63からデフォルトを*3にした（以前は*2）。
static Sint32	_ScreenHeight			= 224 * 3;//		   v2.63からデフォルトを*3にした（以前は*2）。
static Sint32	_Magnification			= 3; //Kitao更新。v2.63からデフォルトを3にした（以前は2）。
static Sint32	_FullMagnification		= 3; //Kitao追加。フルスクリーン時には別に設定できるようにした。0(カスタマイズ専用),1(デフォ300x240),5(デフォ400x300)は、カスタマイズ解像度として扱う。v2.63からデフォルトを3にした（以前は2）。
static Sint32	_ScreenshotMagnification = 2; //Kitao追加。スクリーンショット時の表示倍率(1～4)。0の場合ノンストレッチ(x1)。
static Sint32	_CustomWidth1			= 640; //Kitao追加
static Sint32	_CustomHeight1			= 480; //Kitao追加
static Sint32	_CustomWidth2			= 320; //Kitao追加
static Sint32	_CustomHeight2			= 240; //Kitao追加
static Sint32	_CustomWidth3			= 400; //Kitao追加
static Sint32	_CustomHeight3			= 300; //Kitao追加
static BOOL		_bResolutionAutoChange	= FALSE; //Kitao追加。v1.35
static BOOL		_bFullScreen			= FALSE;
static double 	_GammaValue				= 1.340; //Kitao追加。v1.41更新
static Sint32	_BrightValue			= 1; //Kitao追加。v1.41
static Sint32	_ScanLineType			= 0; //Kitao追加。0…ノンスキャンライン，1…縦横スキャンライン，2…横のみスキャンライン，3…縦のみスキャンライン(未実装)，4…TVモード。5以降も拡張可能。v2.63からデフォルトを1に。
static Sint32	_ScreenshotScanLineType	= 2; //Kitao追加。スクリーンショット時のスキャンラインタイプ。-1の場合、プレイ中と同じ(_ScanLineTypeの値を使用)。デフォルトは静止画で綺麗な2(＆Sizeはx2)。
static Sint32	_ScanLineDensity		= 70; //Kitao追加。v1.09。v2.35からデフォルトは30%にした。v2.64からデフォルトを70%にした。
static BOOL		_bOptimizeGamma			= TRUE; //Kitao追加。スキャンラインの濃度に応じてガンマを最適化するならTRUE。v2.35
static BOOL		_bTvMode				= FALSE; //Kitao追加。TVのビデオ出力ような雰囲気を出すならTRUE。
static Sint32	_MonoColorMode			= 0; //Kitao追加。モノクロ(白黒)表示にするなら1。グリーンディスプレイなら2。v2.28
static Sint32	_ShowOverscanTop		= 0; //Kitao追加。上側のオーバースキャン領域を表示するドット数。Screenメニューを手動で設定した状態が入る。v.1.43
static Sint32	_ShowOverscanBottom		= 0; //下側のオーバースキャン領域を表示するドット数。
static Sint32	_ShowOverscanLeft		= 0; //左側のオーバースキャン領域を表示するドット数。
static Sint32	_ShowOverscanRight		= 0; //右側のオーバースキャン領域を表示するドット数。
static Sint32	_ShowOverscanHeight		= 8; //上下のオーバースキャン領域をオンにしたときのドット数。
static Sint32	_StartShowOverscanTop	= 0; //起動時用。"Setting->Screen"メニューでの設定状態が入る。
static Sint32	_StartShowOverscanBottom= 0; //
static Sint32	_StartShowOverscanLeft	= 0; //
static Sint32	_StartShowOverscanRight	= 0; //オフのときはTop,Bottom.Left.Rightそれぞれ全てが0。
static Sint32	_StartShowOverscanHeight= 8; //
static BOOL		_bOverscanHideBlackBelt	= TRUE; //ウィンドウモードでオーバースキャン領域を隠す際に、ウィンドウは広げたままで黒帯でマスクする場合TRUE。v1.45追加
static BOOL		_bStretched				= TRUE;
static BOOL		_bFullStretched			= FALSE;//Kitao追加。画面一杯まで引き伸ばすならTRUE。
static BOOL		_bVStretched			= FALSE;//Kitao追加。縦画面モードにするならTRUE。縦画面モードは次回起動時には元に戻すため、Configファイルには記録しない。
static Sint32	_StartStretchMode		= 1; //0=ノンストレッチ，1=リアルストレッチ，2=フルストレッチ，3=縦ストレッチ(※現状は縦ストレッチスタートはせず、起動時はリアルストレッチに設定)。v2.64追加

static BOOL		_bSyncTo60HzScreen		= TRUE;//Kitao変更。ウィンドウモードでも使うので_bSyncTo60HzFullScreenから_bSyncTo60HzScreenへ。
static Sint32	_VSyncAdjust			= 0;//Direct3D利用時のV-Syncタイミング補正値。1なら1ライン走査線ぶん、早いタイミングでV-Sync割り込みを発生させる。古めのPCでタイミングが合わないときにこれを使うと改善できる場合がある。うちのRADEON機(XP)は13で640x480が最適になった（※640x480より荒い解像度では補正しない）。14だとGeForce機(Win7)でNG。通常は0。v2.65
static Sint32	_NonVSyncTiming			= 10;//10。4-17OK。約59.94Hzに出来るだけ近く。非V-Sync時の動作タイミング。PC環境によって速さが微妙に異なるのでその調整用。1が一番速いタイミング(タイミング処理が遅いPC用),大きな値は遅いタイミング(タイミング処理が速いPC用)。v2.43
static BOOL		_bForceVSync			= FALSE;//Kitao追加。リフレッシュレートが59～61以外の場合でも、VSyncを有効にする場合はTRUEに。古い環境でリフレッシュレートの値が0を返す場合などで必要。v2.41。_WindowsAutoVSyncを利用してもらうことにしてv2.65から非使用とする。
static Sint32	_WindowsAutoVSync		= 0;//1か2の場合、Windowsの自動V-Sync機能でV-Syncを行う。2の場合フルスクリーン時のみ。この自動V-Syncを使うと表示の遅延(おそらく1フレーム)が発生するので、PC環境によってやむ終えない場合のみ利用。v2.65追加
static Sint32	_DrawMethod				= 0;//Kitao追加。描画方法を設定する。1ならDirect3D。2ならDirectDraw。
static Sint32	_D3DEffectRGB			= 2;//Kitao追加。RGBモード(TVモード以外)時に使う「Direct3Dの描画(転送)エフェクト」を設定する。2ならLINEAR(スムージング)。1ならPOINT(かなりシャープ)。0ならNONE(何もしないのでシャープ)。
static Sint32	_D3DEffectTV			= 1;//Kitao追加。TVモード時に使う「Direct3Dの描画(転送)エフェクト」を設定する。値は同上。TVモード時のデフォルトは1(POINT)。RGBモード時のデフォルトは1(LINEAR)。が個人的には最適。
static BOOL		_bUseVideoCardMemory	= TRUE;//Kitao追加。TRUEならビデオカードのメモリーを使う。FALSEならシステムのメモリを使う(ジャギーが出るがクッキリ＆速度やや低下)。
static BOOL		_bUseSystemMemoryOnlyWindow = FALSE; //Kitao追加。v1.53。TRUEならウィンドウモード時だけシステムメモリを使う。
static Uint32	_FullScreenColor		= 16;//Kitao追加。フルスクリーン時のカラービット数
static BOOL		_bStartFullScreen		= FALSE;//Kitao追加。ゲーム起動時に自動でフルスクリーンにするかどうか。
static BOOL		_bWindowTopMost			= FALSE;//Kitao追加。Windowモード時に、Ootakeのウィンドウを最前面に表示するかどうか。v1.34追加
static BOOL		_bWindowActivate		= TRUE;
static BOOL		_bInactivePause			= FALSE;
static Sint32	_StartFastCD			= 1;
static BOOL		_bFastCD				= TRUE; //Kitao追加
static Sint32	_StartFastSeek			= 1;//Kitao追加。ゲーム起動時の"Fast Seek"設定。0…OFF, 1…ON, 2…前回終了時の設定。v1.08からON(1)をデフォルトにした。
static BOOL		_bFastSeek				= TRUE; //Kitao追加。v1.08からTRUEをデフォルトにした。
static Sint32	_StartSpriteLimit		= 0;//Kitao追加。ゲーム起動時の"Perform SpriteLimit"設定。0…OFF, 1…ON, 2…前回終了時の設定
static BOOL		_bSpriteLimit			= FALSE; //Kitao追加
static BOOL		_bHideMenu				= FALSE;//Kitao追加。エミュレータ動作時にメニューを隠すならTRUE
static BOOL		_bHideMessage			= FALSE;//Kitao追加。メッセージ表示をオフにするならTRUE
static BOOL		_bFullHideMessage		= FALSE;//Kitao追加。メッセージ表示をオフにするならTRUE。フルスクリーン用
static char		_ScreenshotFilePathName[MAX_PATH+1] = ""; //Kitao追加。v2.12
static BOOL		_bScreenshotDialog		= FALSE; //Kitao追加。v2.13
static BOOL		_bDisableWindowsAero	= TRUE; //Kitao追加。WindowsAero(Win7/Vistaで透明化などができるデスクトップ)を無効にするならTRUE。無効にしないと表示遅延(2-3フレーム?)がある。v2.21

// [APP SOUND] (default values)
static Sint32	_SoundType				= 1;//Kitao追加。1…通常(ソフトバッファ＆ミキシング。環境にもよるかもしれないがベストな音質)。2…ハードバッファ＆ソフトミキシング(それなりにいい音が鳴る。環境によってはこちらのほうがノイズが載らず安定する)。
static Sint32	_SoundBufferSize		= 1664;//Kitao更新。バッファが小さすぎると音のつながりが悪くなりノイズが混じって音質自体が下がりがち。バッファが大きすぎると、音が遅れる＆ADPCMの再生が間に合わなず音が切られて大きく飛ぶ。
static Sint32	_SoundSampleRate		= 44100;//固定にした。
static Uint32	_SoundApuMasterVolume	= 65535;//Kitao変更。65535で音割れしない最大限の音質を目指す。65535以外の場合、設定画面で％に合わせるためには656の倍数で。/* 0 - 65535 */
static Uint32	_SoundPsgVolume			= 65535;//Kitao追加。同上。
static Uint32	_SoundAdpcmVolume		= 65535;//Kitao変更。同上。
static Uint32	_SoundCdVolume		 	= 65535;//Kitao追加。同上。この変数はミキサーのボリュームとしてではなく、Ootake内部のCDボリューム値として使用。
static Sint32	_WindowsVolume			= -1;//Kitao追加。Windowsのボリュームコントロール(WAVE)を設定する。値は1(Min)～120(Max。※101～120はHuカードゲーム時のみ有効。CDゲーム時は100と同じ音量に)。-1ならWindowsデフォルトのままの音量。ボリュームコントロールを直接変更すると、音質をほぼ落とさずにボリューム調整ができる(PC2Eのときと同じ方式)。v1.61
static Sint32	_PrevWindowsVolume		= -2;//Kitao追加。レジュームする前の音量値を退避用。退避していないときは-2とする。
static BOOL		_bWindowsVolumeChanged	= FALSE;//Kitao追加。一度でも音量を変更したらTRUE。これがTRUEのとき、Ootake終了時に音量を元に戻す。v2.16
static Uint32	_VolumeEffect			= 1;//Kitao追加。ボリュームをこの値で割ってから出力する。ミュートは0。"4分の3"は3。
static Sint32	_VolumeStep				= 4;//ボリューム変更時のステップ数。v2.45
static Sint32	_BigSoundBuffer			= 0;//Kitao追加。1のときは曲観賞用サイズで大きくバッファを取る。2のときはさらに大きくサウンドバッファを取る。曲を高音質で鑑賞したい時用。
#define BigSoundBufferRate 4 //Kitao追加。ビッグサウンドバッファモード時のバッファ倍率（通常設定のバッファにこの倍率が掛けられる）
#define MostBigSoundBufferRate 8 //Kitao追加。モーストビッグサウンドバッファモード時のバッファ倍率（通常設定のバッファにこの倍率が掛けられる）
static Sint32	_PsgQuality				= 4;//Kitao追加。PSGの再現性を高めるなら4か2。処理速度優先なら1。(1/60)*(1/_HighQualitySound)秒間に１回、実機のタイミングに合わせる。回数が多いほど再現性は高まる(現在4と2と1のみ実装)。v1.39
static BOOL		_bAutoStereo = TRUE;//Kitao追加。グラディウス，沙羅曼蛇,パロディウスだ！を起動したときに、自動でステレオモードにするならTRUE。v1.15
static char		_WavFilePathName[MAX_PATH+1] = ""; //Kitao追加。v1.29
static BOOL		_bOutputWavNext = FALSE; //Kitao追加。v1.29
static BOOL		_bOutputWavAddNoSound = TRUE; //Kitao追加。v1.29
static Sint32	_CddaAdjust = 5995; //V-Sync時にディスプレイリフレッシュレートの周波数の微妙な違い(59.3Hz,59.4Hz,59.5Hz,59.6Hz,59.7Hz,59.8Hz,59.9Hz,60.0Hz,60.1Hz,60.2Hz等)で、長いCD-DA再生のビジュアルシーンで音ズレを防ぐための微調整値。v2.32
								    //フルHD(1920x1080)のワイドディスプレイに多い「59.95Hz」をデフォルト設定とする。59.75Hzを基準(補正なし)とし、59.7Hzでは-1200(画面が20秒間に1フレーム遅い)。59.8Hzでは+1200(画面が20秒間に1フレーム早い)。ゲームの動作を見ながら合わせた。
static Sint32	_CddaAdjustFull[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; //フルスクリーン時の各解像度ごとの_CddaAdjust。ウィンドウとは別で設定を保存。添え字は_FullMagnification(0～6。将来の予備として[10]まで確保しておいた)。v2.32
static Sint32	_CddaAdjustFineTune; //_CddaAdjust(_CddaAdjustFull)の微調整用。設定を行う際の小数点第２位の値。0か5。v2.32
static Sint32	_CddaDelayFrame = 2; //パソコンの液晶ディスプレイ利用時に起こってしまう表示遅延を、何フレームぶん考慮するか。デフォルトは2フレームぶんCD-DAの再生タイミングを遅らせる。v2.33

// [APP INPUT] (default values)
//Kitao更新。キーボードにも対応できるように_JoyPad[]を廃止し、_PcePad2[],_PcePad6[]を新設。２ボタンパッド時と６ボタンパッド時で別々のボタン設定を保存可能にした。
static PCEPAD	_PcePad2[6]; //２ボタン使用時の、各PCEパッド[1]～[5]のボタン設定([0]は非使用)
static PCEPAD	_PcePad3[6]; //３ボタン使用時の、各PCEパッド[1]～[5]のボタン設定([0]は非使用)。v0.79追加
static PCEPAD	_PcePad6[6]; //６ボタン使用時の、各PCEパッド[1]～[5]のボタン設定([0]は非使用)
static Sint32	_RenshaSpeedSel = 1; //起動時の連射スピード(0=強制Off, 1=High, 2=Middle, 3=Low)。v0.62追加
static BOOL		_bUseThreeButton = FALSE; //３ボタンパッド設定を使うならTRUE。２ボタンパッド設定を使うならFALSE。
static BOOL		_bAutoGradiusII = TRUE;//Kitao追加。グラディウスIIを起動したときに、自動でレーザー・スプレッドボムがちらつかない設定＆ステレオ設定にするならTRUE。v1.21。v2.34
static BOOL		_bAutoShinMegamiTensei = TRUE;//Kitao追加。実機でも起こる「時々起こる描画関連の細かいバグ」をCPU速度を上げることで解消するならTRUE。v2.20
static BOOL		_bJoypadBackground = TRUE; //v1.41追加
static BOOL		_bKeyboardBackground = FALSE; //v1.41追加
static Sint32	_InputRecordMode = 2; //プレイレコードのモード(1=１プレイヤーのみ(連射専用ボタン非対応)，2=連射専用ボタン対応＆２プレイヤーパッド対応)。現状は2固定とする。将来5人プレイ記録にも対応したい。v2.15追加
static Sint32	_InputRecordingNumber = -1; //プレイレコード記録中の場合、ここに記録ナンバー(#0～#10)が入る。v2.15追加
static Sint32	_InputPlayRecordNumber = -1; //プレイレコード再生中の場合、ここに記録ナンバー(#0～#10)が入る。v2.15追加
static BOOL		_bCheckSimultaneouslyPush = TRUE; //十字キーの左右（または上下）同時押しを抑制する場合TRUE。v2.59追加

static FILE*	_fpRecording = NULL;
static FILE*	_fpPlayRecord = NULL;

static HANDLE	_Menu;
static HANDLE	_FileMenu;
static HANDLE	_CaptureMenu;//Kitao追加
static HANDLE	_SaveAnotherMenu;//Kitao追加
static HANDLE	_SaveAnother2Menu;//Kitao追加
static HANDLE	_LoadAnotherMenu;//Kitao追加
static HANDLE	_LoadAnother2Menu;//Kitao追加
static HANDLE	_RecordingAnotherMenu;//Kitao追加
static HANDLE	_PlayRecordAnotherMenu;//Kitao追加
static HANDLE	_MoveRecordMenu;//Kitao追加
static HANDLE	_AviAttentionMenu;//Kitao追加
static HANDLE	_SaveButtonMenu;//Kitao追加
static HANDLE	_SShotButtonMenu;//Kitao追加
static HANDLE	_SShotSettingMenu;//Kitao追加
static HANDLE	_WavAnotherMenu;//Kitao追加
static HANDLE	_WavSettingMenu;//Kitao追加
static HANDLE	_RecentMenu;//Kitao追加
static HANDLE	_DriveMenu;//Kitao追加
static HANDLE	_CDChangeNRMenu;//Kitao追加
static HANDLE	_SettingMenu;//Kitao更新
static HANDLE	_SizeMenu;//Kitao追加
static HANDLE	_AudioSubMenu;//Kitao追加
static HANDLE	_SpeedMenu;//Kitao追加
static HANDLE	_VolumeMenu;//Kitao追加
static HANDLE	_InfoMenu;//Kitao追加
static HANDLE	_ScreenMenu;
static HANDLE	_CpuMenu;
static HANDLE	_InputMenu;
static HANDLE	_AudioMenu;
static HANDLE	_CDROMMenu;//Kitao追加
static HANDLE	_SetCpuMenu;//Kitao追加
static HANDLE	_SetInputMenu;//Kitao追加
static HANDLE	_ToolMenu;//Kitao追加
static HANDLE	_TurboButtonMenu;//Kitao追加
static HANDLE	_FuncButtonMenu;//Kitao追加
//static HANDLE	_SampleRateMenu; Kitaoカット
static HANDLE	_DefaultMenu;//Kitao追加
static HANDLE	_ScanlineMenu;//Kitao追加
static HANDLE	_DensityMenu;//Kitao追加
static HANDLE	_GammaMenu;//Kitao追加
static HANDLE	_BrightnessMenu;//Kitao追加
static HANDLE	_ResolutionMenu;//Kitao追加
static HANDLE	_Direct3DMenu;//Kitao追加
static HANDLE	_DirectDrawMenu;//Kitao追加
static HANDLE	_OverscanMenu;//Kitao追加
static HANDLE	_StartOverscanMenu;//Kitao追加
static HANDLE	_OtherSyscardMenu;//Kitao追加
static HANDLE	_TempVolMenu;//Kitao追加
static HANDLE	_VolAttentionMenu;//Kitao追加
static HANDLE	_VolDetailMenu;//Kitao追加
static HANDLE	_VolStepMenu;//Kitao追加
static HANDLE	_MuteMenu;//Kitao追加
static HANDLE	_CpuSpeedMenu;//Kitao追加
static HANDLE	_WriteAttentionMenu;//Kitao追加

static BOOL		_bScreenMenuFull;//Kitao追加。フルスクリーン用のメニューを表示しているときはTRUE。

static BOOL		_bRunning = TRUE;
static BOOL		_bPauseNoRelease = FALSE; //Kitao追加
static BOOL		_bShowFPS = FALSE; //Kitao追加。v1.50

static char		_RecentRom[21][MAX_PATH+1] = {" ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ",
											        " ", " ", " ", " ", " ", " ", " ", " ", " ", " "};//Kitao追加。ROMイメージファイルの履歴20個ぶん。[1]～[20]（[0]は非使用）
static char		_RecentRomTmp[MAX_PATH+1]; //v2.48追加


//Kitao追加。英数大文字を小文字に変換する関数。
static char*
strToLower(
	char*	pStr)
{
	char*	p = pStr;

	while (*p)
	{
		*p = tolower(*p);
		p++;
	}
	return pStr;
}

//Kitao追加。英数大文字を小文字に変換する関数。外部から使う用
char*
APP_StrToLower(
	char*	pStr)
{
	return strToLower(pStr);
}

//Kitao追加。ディレクトリパスを取り除いてファイル名だけを返す関数。MinGWでstrrchr関数だと「ソ」等の日本語文字を\の区切りと間違ってしまうため独自の処理で'\'をサーチ。
static char*
extractFileName(
	char*	p)
{
	int		a,b;
	Uint8	c;
	BOOL	ok = FALSE;

	a = (int)strlen(p);
	while (--a > 0)
	{
		c = *(p+a);
		if (c == '\\')
		{
			b = a;
			while (--b >= 0)
			{
				c = *(p+b); //手前のバイトの文字を調べる
				if (!(((c>0x80)&&(c<0xA0))||
					  ((c>0xDF)&&(c<0xFD)))) //「ソ」「表」などの日本語内の「\」ではなければ
				{
					a++;
					ok = TRUE;
					break;
				}
				else // c が全角のLowByte部分である可能性もあるのでそれをチェック。v0.78
				{
					if (--b >= 0)
					{
						c = *(p+b); //手前のバイトの文字を調べる
						if (!(((c>0x80)&&(c<0xA0))||
							  ((c>0xDF)&&(c<0xFD)))) // c が全角のHighByteではなければ
							break;
						// c が全角のHighByteならまだ半角の「\」かどうかを決定できず。さらに前のバイトのチェックを続ける。
					}
				}
			}
			if (ok)
				break;
		}
	}
	return (p+a);
}

//Kitao追加。拡張子だけを返す関数。
static char*
extractFileExt(
	char*	p)
{
	int		a;

	a = (int)strlen(p);
	while (--a > 0)
	{
		if (*(p+a) == '.')
		{
			a++;
			break;
		}
	}
	return (p+a);
}

//Kitao追加。拡張子だけを返す関数。外部から使う用
char*
APP_ExtractFileExt(
	char*	p)
{
	return extractFileExt(p);
}

//Kitao追加。ゲームのファイル名（拡張子はカット）をpGameFileNameBufに設定する
static void
SetGameFileNameBuf(
	BOOL	bCDGame,
	BOOL	bHesFile,
	char*	pGameFilePathName,
	char*	pGameFileNameBuf,
	BOOL	bSetCueMark)
{
	char	buf[MAX_PATH+1];

	strcpy(pGameFileNameBuf, "");

	if (bCDGame)
	{//CDのゲームだった場合、TOCゲーム名を入れる
		strcpy(pGameFileNameBuf, "(CD)");
		strcat(pGameFileNameBuf, TOCDB_GetGameTitle());
	}
	else //Huカードのゲームの場合
	{
		strcat(pGameFileNameBuf, extractFileName(pGameFilePathName));
		strcpy(buf, pGameFilePathName); //pGameFilePathNameは書き換えないようにするためbufを使う
		if ((bHesFile)||
			(strstr(strToLower(extractFileExt(buf)), "hes"))) //bHesFileがFALSEでも、拡張子がhesなら。
		{	//HESファイルの場合
			strcpy(buf, pGameFileNameBuf);
			if (bSetCueMark)
				strcpy(pGameFileNameBuf, "[hes] "); //ROMオープン履歴メニュー表示時に使用。v2.24
			else
				strcpy(pGameFileNameBuf, "(hes)");
			strcat(pGameFileNameBuf, buf);
		}
		else if (bSetCueMark) //bSetCueMarkをTRUEにして呼んだ時。ROMオープン履歴メニュー表示時のみ使用。[CD][Hu]マークを付ける。v2.24
		{
			if (strstr(strToLower(extractFileExt(buf)), "cue")) //拡張子がcueなら。
			{	//CUEファイルの場合
				strcpy(buf, pGameFileNameBuf);
				strcpy(pGameFileNameBuf, "[CD] ");
				strcat(pGameFileNameBuf, buf);
			}
			else if (strcmp(pGameFilePathName, " ") != 0)
			{	//Huカードの場合
				strcpy(buf, pGameFileNameBuf);
				strcpy(pGameFileNameBuf, "[Hu] ");
				strcat(pGameFileNameBuf, buf);
			}
		}
		//拡張子をカット
		if (strrchr(pGameFileNameBuf, '.') != NULL)
			*(strrchr(pGameFileNameBuf, '.')) = 0;
	}
}


//Kitao追加。ステートセーブ用のファイル名を設定
static void
SetSaveStateFileName(
	const Sint32	number, //number=ステートセーブの番号。番号なしは0。
	char*			pBuf)
{
	char	c[2+1] = "";

	strcpy(pBuf, _AppPath);
	strcat(pBuf, "save");
	CreateDirectory(pBuf, NULL);//saveディレクトリがない場合作る
	strcat(pBuf, "\\");
	strcat(pBuf, _GameFileNameBuf);//Kitao更新
	strcat(pBuf, "_state");
	if (number > 0) //番号付き保存の場合
	{
		sprintf(c, "%d", (int)number);
		strcat(pBuf, c);
	}
	strcat(pBuf, ".dat");
}


//Kitao追加。プレイレコード保存用のファイル名を設定
static void
SetRecordFileName(
	const Sint32	number, //number=ステートセーブの番号。番号なしは0。
	char*			pBuf)
{
	char	c[2+1] = "";

	strcpy(pBuf, _AppPath);
	strcat(pBuf, "record");
	CreateDirectory(pBuf, NULL);//recordディレクトリがない場合作る
	strcat(pBuf, "\\");
	strcat(pBuf, _GameFileNameBuf);//Kitao更新
	strcat(pBuf, "_record");
	if (number > 0) //番号付き保存の場合
	{
		sprintf(c, "%d", (int)number);
		strcat(pBuf, c);
	}
	strcat(pBuf, ".dat");
}


//Kitao追加。v2.32
static Sint32
getCddaAdjust()
{
	if (_bFullScreen)
		return _CddaAdjustFull[_FullMagnification];
	else
		return _CddaAdjust;
}

//1.61追加
static void
overClockMenuUpdate()
{
	switch (VDC_GetOverClockType())
	{
		case    0: MENU_CheckRadioItem(_CpuSpeedMenu, WM_SPEED_P0, WM_SPEED_M2, WM_SPEED_P0);
				   MENU_ChangeItemText(_SpeedMenu, WM_SPEED_CPU, "CPU Speed  [Normal]"); break;
		case  300: MENU_CheckRadioItem(_CpuSpeedMenu, WM_SPEED_P0, WM_SPEED_M2, WM_SPEED_T3);
				   MENU_ChangeItemText(_SpeedMenu, WM_SPEED_CPU, "CPU Speed  [TURBOx3]"); break;
		case  200: MENU_CheckRadioItem(_CpuSpeedMenu, WM_SPEED_P0, WM_SPEED_M2, WM_SPEED_T2);
				   MENU_ChangeItemText(_SpeedMenu, WM_SPEED_CPU, "CPU Speed  [TURBOx2]"); break;
		case  100: MENU_CheckRadioItem(_CpuSpeedMenu, WM_SPEED_P0, WM_SPEED_M2, WM_SPEED_T1);
				   MENU_ChangeItemText(_SpeedMenu, WM_SPEED_CPU, "CPU Speed  [TURBOx1]"); break;
		case    6: MENU_CheckRadioItem(_CpuSpeedMenu, WM_SPEED_P0, WM_SPEED_M2, WM_SPEED_P6);
				   MENU_ChangeItemText(_SpeedMenu, WM_SPEED_CPU, "CPU Speed  [x3.00]"); break;
		case    5: MENU_CheckRadioItem(_CpuSpeedMenu, WM_SPEED_P0, WM_SPEED_M2, WM_SPEED_P5);
				   MENU_ChangeItemText(_SpeedMenu, WM_SPEED_CPU, "CPU Speed  [x2.50]"); break;
		case    4: MENU_CheckRadioItem(_CpuSpeedMenu, WM_SPEED_P0, WM_SPEED_M2, WM_SPEED_P4);
				   MENU_ChangeItemText(_SpeedMenu, WM_SPEED_CPU, "CPU Speed  [x2.00]"); break;
		case    3: MENU_CheckRadioItem(_CpuSpeedMenu, WM_SPEED_P0, WM_SPEED_M2, WM_SPEED_P3);
				   MENU_ChangeItemText(_SpeedMenu, WM_SPEED_CPU, "CPU Speed  [x1.50]"); break;
		case    2: MENU_CheckRadioItem(_CpuSpeedMenu, WM_SPEED_P0, WM_SPEED_M2, WM_SPEED_P2);
				   MENU_ChangeItemText(_SpeedMenu, WM_SPEED_CPU, "CPU Speed  [x1.33]"); break;
		case    1: MENU_CheckRadioItem(_CpuSpeedMenu, WM_SPEED_P0, WM_SPEED_M2, WM_SPEED_P1);
				   MENU_ChangeItemText(_SpeedMenu, WM_SPEED_CPU, "CPU Speed  [x1.25]"); break;
		case   -1: MENU_CheckRadioItem(_CpuSpeedMenu, WM_SPEED_P0, WM_SPEED_M2, WM_SPEED_M1);
				   MENU_ChangeItemText(_SpeedMenu, WM_SPEED_CPU, "CPU Speed  [x0.75]"); break;
		case   -2: MENU_CheckRadioItem(_CpuSpeedMenu, WM_SPEED_P0, WM_SPEED_M2, WM_SPEED_M2);
				   MENU_ChangeItemText(_SpeedMenu, WM_SPEED_CPU, "CPU Speed  [x0.50]"); break;
	}
}

//Kitao追加。v2.29
void
updateVolumeMenu()
{
	int		a;
	char	buf[32];

	switch (_WindowsVolume/10)
	{
		case 12: MENU_CheckRadioItem(_VolumeMenu, WM_VOLUME_DEFAULT, WM_VOLUME_1, WM_VOLUME_12); break;
		case 11: MENU_CheckRadioItem(_VolumeMenu, WM_VOLUME_DEFAULT, WM_VOLUME_1, WM_VOLUME_11); break;
		case 10: MENU_CheckRadioItem(_VolumeMenu, WM_VOLUME_DEFAULT, WM_VOLUME_1, WM_VOLUME_10); break;
		case  9: MENU_CheckRadioItem(_VolumeMenu, WM_VOLUME_DEFAULT, WM_VOLUME_1, WM_VOLUME_9); break;
		case  8: MENU_CheckRadioItem(_VolumeMenu, WM_VOLUME_DEFAULT, WM_VOLUME_1, WM_VOLUME_8); break;
		case  7: MENU_CheckRadioItem(_VolumeMenu, WM_VOLUME_DEFAULT, WM_VOLUME_1, WM_VOLUME_7); break;
		case  6: MENU_CheckRadioItem(_VolumeMenu, WM_VOLUME_DEFAULT, WM_VOLUME_1, WM_VOLUME_6); break;
		case  5: MENU_CheckRadioItem(_VolumeMenu, WM_VOLUME_DEFAULT, WM_VOLUME_1, WM_VOLUME_5); break;
		case  4: MENU_CheckRadioItem(_VolumeMenu, WM_VOLUME_DEFAULT, WM_VOLUME_1, WM_VOLUME_4); break;
		case  3: MENU_CheckRadioItem(_VolumeMenu, WM_VOLUME_DEFAULT, WM_VOLUME_1, WM_VOLUME_3); break;
		case  2: MENU_CheckRadioItem(_VolumeMenu, WM_VOLUME_DEFAULT, WM_VOLUME_1, WM_VOLUME_2); break;
		case  1: MENU_CheckRadioItem(_VolumeMenu, WM_VOLUME_DEFAULT, WM_VOLUME_1, WM_VOLUME_1); break;
		case  0:
			//if (_WindowsVolume == 0) //MUTE用。現在未実装
			//else
			if (_WindowsVolume == -1)
				MENU_CheckRadioItem(_VolumeMenu, WM_VOLUME_DEFAULT, WM_VOLUME_1, WM_VOLUME_DEFAULT);
			else
				MENU_CheckRadioItem(_VolumeMenu, WM_VOLUME_DEFAULT, WM_VOLUME_1, WM_VOLUME_1); //1～9の場合もVolume1にチェック
			break;
	}
	if (_WindowsVolume == -1)
	{
		mixerGetControlDetails((HMIXEROBJ)_hMix, &_MixerControlD, MIXER_OBJECTF_HMIXER|MIXER_GETCONTROLDETAILSF_VALUE);
		_dwDefaultWindowsVolumeValue = _MixerControlDU.dwValue; //最新のWindowsボリューム値に更新。v2.16
		a = (int)((double)_dwDefaultWindowsVolumeValue/(double)_dwWaveMaxVolume * 100 +0.5);
	}
	else
		a = _WindowsVolume;
	sprintf(buf, "Detail Volume  [%d]", a);
	MENU_ChangeItemText(_VolumeMenu, WM_VOLUME_DETAIL, buf);
}

//ゲーム起動からの経過時間(単位はフレーム)を読み込んでメニュー表示用テキストを作成。v2.63追加
//※numを負の数で呼んだ場合、ロードサムネイルへ時間表示を入れる処理用(MainBoard.cppからAPP_CheckStateTime()で利用)。
static void
checkStateTime(
	const Sint32	num0,
	char*			pBufS,
	char*			pBufL)
{
	int		a;
	char	buf2[MAX_PATH+1+9];
	char	bufT[11+1];
	FILE*	fp;
	Uint32	stateVersion;
	Uint32	systemTime;
	int		t1, t2, t3;
	Sint32	num = num0;

	if (num0 < 0)
	{
		strcpy(pBufS, "Former File");
		strcpy(pBufL, "");
		num = abs(num0);
	}
	else if (num0 == 0)
	{
		strcpy(pBufS, "&Save State");
		strcpy(pBufL, "&Load State");
	}
	else
	{
		sprintf(pBufS, "Save State #%d", num);
		sprintf(pBufL, "Load State #%d", num);
	}
	SetSaveStateFileName(num, buf2); //ステートセーブのファイル名を設定
	if ((fp = fopen(buf2, "rb")) != NULL) //セーブファイルがあれば
	{	
		fread(&stateVersion, sizeof(stateVersion), 1, fp);
		if (stateVersion > SAVE_STATE_VERSION) //もし旧バージョンのOotakeで未対応の新ver.ステートセーブデータを読み込んだ場合
			strcat(pBufL, " x");
		else if (stateVersion >= 27) //v1.15以降のセーブファイルなら
		{
			a = 0x8025A; //経過時間データが入っている位置
			if (stateVersion < 32) //v1.43より前のセーブファイルなら
				a -= 16;
			if (stateVersion < 42) //v2.27より前のセーブファイルなら
				a -= 12;
			if (stateVersion < 43) //v2.28より前のセーブファイルなら
				a -= 4;
			fseek(fp, a ,SEEK_SET);
			fread(&systemTime, sizeof(systemTime), 1, fp);
			if (systemTime == 0xFFFFFFFF)
			{
				strcat(pBufS, " <--:--:-->");
				strcat(pBufL, " <--:--:-->");
			}
			else
			{
				//60.00HzなどのディスプレイでV-Sync動作時でも、「59.94フレームでゲーム時間は１秒経過」とし、実機と同時間な表示にする。
				systemTime *= 100;
				t1 = systemTime / int(59.94 * 59.94 * 59.94 * 100.0);
				systemTime -= t1 * int(59.94 * 59.94 * 59.94 * 100.0);
				if (t1>99) t1=99;
				t2 = systemTime / int(59.94 * 59.94 * 100.0);
				systemTime -= t2 * int(59.94 * 59.94 * 100.0);
				t3 = systemTime / int(59.94 * 100.0);
				sprintf(bufT, " <%02d:%02d:%02d>", t1, t2, t3);
				strcat(pBufS, bufT);
				strcat(pBufL, bufT);
			}
		}
		else //v1.15より前のセーブファイルなら
		{
			strcat(pBufS, " <--:--:-->");
			strcat(pBufL, " <--:--:-->");
		}
		fclose(fp);
	}
	if (num0 == 0)
	{
		if (_SaveStateButton == DIK_S) //キー設定がデフォルトの場合
			strcat(pBufS, "\tS");
		if (_LoadStateButton == DIK_L) //キー設定がデフォルトの場合
			strcat(pBufL, "\tL");
	}
}

//Kitao追加。メニューの選択状態をアップデートする。ステートロード時などでも使用。
static void
updateMenu()
{
	int		a;
	char	buf[MAX_PATH+1];
	char	buf2[MAX_PATH+1+9];
	char	bufS[14+11+1];
	char	bufL[14+11+1];
	char	bufC[33+1+1];
	FILE*	fp;

	//Kitao追加
	checkStateTime(0, bufS, bufL);
	MENU_ChangeItemText(_FileMenu, WM_SAVE_STATE, bufS);
	MENU_ChangeItemText(_FileMenu, WM_LOAD_STATE, bufL);
	if (_ScreenshotButton == DIK_SYSRQ) //キー設定がデフォルトの場合
		MENU_ChangeItemText(_FileMenu, WM_OUTPUT_SCREENSHOT, "Output Screens&hot\tPrtScr");
	else
		MENU_ChangeItemText(_FileMenu, WM_OUTPUT_SCREENSHOT, "Output Screens&hot");

	//Kitao追加
	if (_bCDGame)
	{
		if (_bF1NoReset) //F1キーでリセットせずにＣＤを入れ替える設定の場合。"ＣＤバトル 光の勇者たち" で利用。
		{
			MENU_ChangeItemText(_FileMenu, WM_CD_CHANGE, "&CD Change");
			MENU_ChangeItemText(_DriveMenu, WM_CD_CHANGE, "CD Change");
		}
		else
		{
			if (_bCueFile)
			{
				MENU_ChangeItemText(_FileMenu, WM_CD_CHANGE, "&CD Change\tCtrl+F1");
				MENU_ChangeItemText(_DriveMenu, WM_CD_CHANGE, "CD Change\tCtrl+F1");
			}
			else
			{
				MENU_ChangeItemText(_FileMenu, WM_CD_CHANGE, "&CD Change\tF1");
				MENU_ChangeItemText(_DriveMenu, WM_CD_CHANGE, "CD Change\tF1");
			}
		}
	}
	else
	{
		MENU_ChangeItemText(_FileMenu, WM_CD_CHANGE, "&CD Play\tCtrl+F1");
		MENU_ChangeItemText(_DriveMenu, WM_CD_CHANGE, "CD Play\tCtrl+F1");
	}

	//Kitao追加
	MENU_CheckItem(_FileMenu, WM_SET_RESUME, _bResumeMode);
	MENU_CheckItem(_FileMenu, WM_AUTO_RESUME, _bAutoResumeMode);

	//Kitao追加
	checkStateTime( 1, bufS, bufL); MENU_ChangeItemText(_SaveAnotherMenu, WM_SAVE_STATE_1, bufS);  MENU_ChangeItemText(_LoadAnotherMenu, WM_LOAD_STATE_1, bufL);
	checkStateTime( 2, bufS, bufL); MENU_ChangeItemText(_SaveAnotherMenu, WM_SAVE_STATE_2, bufS);  MENU_ChangeItemText(_LoadAnotherMenu, WM_LOAD_STATE_2, bufL);
	checkStateTime( 3, bufS, bufL); MENU_ChangeItemText(_SaveAnotherMenu, WM_SAVE_STATE_3, bufS);  MENU_ChangeItemText(_LoadAnotherMenu, WM_LOAD_STATE_3, bufL);
	checkStateTime( 4, bufS, bufL); MENU_ChangeItemText(_SaveAnotherMenu, WM_SAVE_STATE_4, bufS);  MENU_ChangeItemText(_LoadAnotherMenu, WM_LOAD_STATE_4, bufL);
	checkStateTime( 5, bufS, bufL); MENU_ChangeItemText(_SaveAnotherMenu, WM_SAVE_STATE_5, bufS);  MENU_ChangeItemText(_LoadAnotherMenu, WM_LOAD_STATE_5, bufL);
	checkStateTime( 6, bufS, bufL); MENU_ChangeItemText(_SaveAnotherMenu, WM_SAVE_STATE_6, bufS);  MENU_ChangeItemText(_LoadAnotherMenu, WM_LOAD_STATE_6, bufL);
	checkStateTime( 7, bufS, bufL); MENU_ChangeItemText(_SaveAnotherMenu, WM_SAVE_STATE_7, bufS);  MENU_ChangeItemText(_LoadAnotherMenu, WM_LOAD_STATE_7, bufL);
	checkStateTime( 8, bufS, bufL); MENU_ChangeItemText(_SaveAnotherMenu, WM_SAVE_STATE_8, bufS);  MENU_ChangeItemText(_LoadAnotherMenu, WM_LOAD_STATE_8, bufL);
	checkStateTime( 9, bufS, bufL); MENU_ChangeItemText(_SaveAnotherMenu, WM_SAVE_STATE_9, bufS);  MENU_ChangeItemText(_LoadAnotherMenu, WM_LOAD_STATE_9, bufL);
	checkStateTime(10, bufS, bufL); MENU_ChangeItemText(_SaveAnotherMenu, WM_SAVE_STATE_10, bufS); MENU_ChangeItemText(_LoadAnotherMenu, WM_LOAD_STATE_10, bufL);
	checkStateTime(11, bufS, bufL); MENU_ChangeItemText(_SaveAnotherMenu, WM_SAVE_STATE_11, bufS); MENU_ChangeItemText(_LoadAnotherMenu, WM_LOAD_STATE_11, bufL);
	checkStateTime(12, bufS, bufL); MENU_ChangeItemText(_SaveAnotherMenu, WM_SAVE_STATE_12, bufS); MENU_ChangeItemText(_LoadAnotherMenu, WM_LOAD_STATE_12, bufL);
	checkStateTime(13, bufS, bufL); MENU_ChangeItemText(_SaveAnotherMenu, WM_SAVE_STATE_13, bufS); MENU_ChangeItemText(_LoadAnotherMenu, WM_LOAD_STATE_13, bufL);
	checkStateTime(14, bufS, bufL); MENU_ChangeItemText(_SaveAnotherMenu, WM_SAVE_STATE_14, bufS); MENU_ChangeItemText(_LoadAnotherMenu, WM_LOAD_STATE_14, bufL);
	checkStateTime(15, bufS, bufL); MENU_ChangeItemText(_SaveAnotherMenu, WM_SAVE_STATE_15, bufS); MENU_ChangeItemText(_LoadAnotherMenu, WM_LOAD_STATE_15, bufL);
	checkStateTime(16, bufS, bufL); MENU_ChangeItemText(_SaveAnotherMenu, WM_SAVE_STATE_16, bufS); MENU_ChangeItemText(_LoadAnotherMenu, WM_LOAD_STATE_16, bufL);
	checkStateTime(17, bufS, bufL); MENU_ChangeItemText(_SaveAnotherMenu, WM_SAVE_STATE_17, bufS); MENU_ChangeItemText(_LoadAnotherMenu, WM_LOAD_STATE_17, bufL);
	checkStateTime(18, bufS, bufL); MENU_ChangeItemText(_SaveAnotherMenu, WM_SAVE_STATE_18, bufS); MENU_ChangeItemText(_LoadAnotherMenu, WM_LOAD_STATE_18, bufL);
	checkStateTime(19, bufS, bufL); MENU_ChangeItemText(_SaveAnotherMenu, WM_SAVE_STATE_19, bufS); MENU_ChangeItemText(_LoadAnotherMenu, WM_LOAD_STATE_19, bufL);
	checkStateTime(20, bufS, bufL); MENU_ChangeItemText(_SaveAnotherMenu, WM_SAVE_STATE_20, bufS); MENU_ChangeItemText(_LoadAnotherMenu, WM_LOAD_STATE_20, bufL);
	checkStateTime(21, bufS, bufL); MENU_ChangeItemText(_SaveAnother2Menu, WM_SAVE_STATE_21, bufS); MENU_ChangeItemText(_LoadAnother2Menu, WM_LOAD_STATE_21, bufL);
	checkStateTime(22, bufS, bufL); MENU_ChangeItemText(_SaveAnother2Menu, WM_SAVE_STATE_22, bufS); MENU_ChangeItemText(_LoadAnother2Menu, WM_LOAD_STATE_22, bufL);
	checkStateTime(23, bufS, bufL); MENU_ChangeItemText(_SaveAnother2Menu, WM_SAVE_STATE_23, bufS); MENU_ChangeItemText(_LoadAnother2Menu, WM_LOAD_STATE_23, bufL);
	checkStateTime(24, bufS, bufL); MENU_ChangeItemText(_SaveAnother2Menu, WM_SAVE_STATE_24, bufS); MENU_ChangeItemText(_LoadAnother2Menu, WM_LOAD_STATE_24, bufL);
	checkStateTime(25, bufS, bufL); MENU_ChangeItemText(_SaveAnother2Menu, WM_SAVE_STATE_25, bufS); MENU_ChangeItemText(_LoadAnother2Menu, WM_LOAD_STATE_25, bufL);
	checkStateTime(26, bufS, bufL); MENU_ChangeItemText(_SaveAnother2Menu, WM_SAVE_STATE_26, bufS); MENU_ChangeItemText(_LoadAnother2Menu, WM_LOAD_STATE_26, bufL);
	checkStateTime(27, bufS, bufL); MENU_ChangeItemText(_SaveAnother2Menu, WM_SAVE_STATE_27, bufS); MENU_ChangeItemText(_LoadAnother2Menu, WM_LOAD_STATE_27, bufL);
	checkStateTime(28, bufS, bufL); MENU_ChangeItemText(_SaveAnother2Menu, WM_SAVE_STATE_28, bufS); MENU_ChangeItemText(_LoadAnother2Menu, WM_LOAD_STATE_28, bufL);
	checkStateTime(29, bufS, bufL); MENU_ChangeItemText(_SaveAnother2Menu, WM_SAVE_STATE_29, bufS); MENU_ChangeItemText(_LoadAnother2Menu, WM_LOAD_STATE_29, bufL);
	checkStateTime(30, bufS, bufL); MENU_ChangeItemText(_SaveAnother2Menu, WM_SAVE_STATE_30, bufS); MENU_ChangeItemText(_LoadAnother2Menu, WM_LOAD_STATE_30, bufL);
	checkStateTime(31, bufS, bufL); MENU_ChangeItemText(_SaveAnother2Menu, WM_SAVE_STATE_31, bufS); MENU_ChangeItemText(_LoadAnother2Menu, WM_LOAD_STATE_31, bufL);
	checkStateTime(32, bufS, bufL); MENU_ChangeItemText(_SaveAnother2Menu, WM_SAVE_STATE_32, bufS); MENU_ChangeItemText(_LoadAnother2Menu, WM_LOAD_STATE_32, bufL);
	checkStateTime(33, bufS, bufL); MENU_ChangeItemText(_SaveAnother2Menu, WM_SAVE_STATE_33, bufS); MENU_ChangeItemText(_LoadAnother2Menu, WM_LOAD_STATE_33, bufL);
	checkStateTime(34, bufS, bufL); MENU_ChangeItemText(_SaveAnother2Menu, WM_SAVE_STATE_34, bufS); MENU_ChangeItemText(_LoadAnother2Menu, WM_LOAD_STATE_34, bufL);
	checkStateTime(35, bufS, bufL); MENU_ChangeItemText(_SaveAnother2Menu, WM_SAVE_STATE_35, bufS); MENU_ChangeItemText(_LoadAnother2Menu, WM_LOAD_STATE_35, bufL);
	checkStateTime(36, bufS, bufL); MENU_ChangeItemText(_SaveAnother2Menu, WM_SAVE_STATE_36, bufS); MENU_ChangeItemText(_LoadAnother2Menu, WM_LOAD_STATE_36, bufL);
	checkStateTime(37, bufS, bufL); MENU_ChangeItemText(_SaveAnother2Menu, WM_SAVE_STATE_37, bufS); MENU_ChangeItemText(_LoadAnother2Menu, WM_LOAD_STATE_37, bufL);
	checkStateTime(38, bufS, bufL); MENU_ChangeItemText(_SaveAnother2Menu, WM_SAVE_STATE_38, bufS); MENU_ChangeItemText(_LoadAnother2Menu, WM_LOAD_STATE_38, bufL);
	checkStateTime(39, bufS, bufL); MENU_ChangeItemText(_SaveAnother2Menu, WM_SAVE_STATE_39, bufS); MENU_ChangeItemText(_LoadAnother2Menu, WM_LOAD_STATE_39, bufL);
	checkStateTime(40, bufS, bufL); MENU_ChangeItemText(_SaveAnother2Menu, WM_SAVE_STATE_40, bufS); MENU_ChangeItemText(_LoadAnother2Menu, WM_LOAD_STATE_40, bufL);
	checkStateTime(41, bufS, bufL); MENU_ChangeItemText(_SaveAnother2Menu, WM_SAVE_STATE_41, bufS); MENU_ChangeItemText(_LoadAnother2Menu, WM_LOAD_STATE_41, bufL);
	checkStateTime(42, bufS, bufL); MENU_ChangeItemText(_SaveAnother2Menu, WM_SAVE_STATE_42, bufS); MENU_ChangeItemText(_LoadAnother2Menu, WM_LOAD_STATE_42, bufL);
	checkStateTime(43, bufS, bufL); MENU_ChangeItemText(_SaveAnother2Menu, WM_SAVE_STATE_43, bufS); MENU_ChangeItemText(_LoadAnother2Menu, WM_LOAD_STATE_43, bufL);
	checkStateTime(44, bufS, bufL); MENU_ChangeItemText(_SaveAnother2Menu, WM_SAVE_STATE_44, bufS); MENU_ChangeItemText(_LoadAnother2Menu, WM_LOAD_STATE_44, bufL);
	checkStateTime(45, bufS, bufL); MENU_ChangeItemText(_SaveAnother2Menu, WM_SAVE_STATE_45, bufS); MENU_ChangeItemText(_LoadAnother2Menu, WM_LOAD_STATE_45, bufL);
	checkStateTime(46, bufS, bufL); MENU_ChangeItemText(_SaveAnother2Menu, WM_SAVE_STATE_46, bufS); MENU_ChangeItemText(_LoadAnother2Menu, WM_LOAD_STATE_46, bufL);
	checkStateTime(47, bufS, bufL); MENU_ChangeItemText(_SaveAnother2Menu, WM_SAVE_STATE_47, bufS); MENU_ChangeItemText(_LoadAnother2Menu, WM_LOAD_STATE_47, bufL);
	checkStateTime(48, bufS, bufL); MENU_ChangeItemText(_SaveAnother2Menu, WM_SAVE_STATE_48, bufS); MENU_ChangeItemText(_LoadAnother2Menu, WM_LOAD_STATE_48, bufL);
	checkStateTime(49, bufS, bufL); MENU_ChangeItemText(_SaveAnother2Menu, WM_SAVE_STATE_49, bufS); MENU_ChangeItemText(_LoadAnother2Menu, WM_LOAD_STATE_49, bufL);
	checkStateTime(50, bufS, bufL); MENU_ChangeItemText(_SaveAnother2Menu, WM_SAVE_STATE_50, bufS); MENU_ChangeItemText(_LoadAnother2Menu, WM_LOAD_STATE_50, bufL);

	//Kitao追加。v2.15
	strcpy(bufS, "Start Recording #1");
	strcpy(bufL, "Play Record #1");
	strcpy(bufC, "Move \"Default Record Data\" to #1");
	SetRecordFileName(1, buf2); //ステートセーブのファイル名を設定
	if ((fp = fopen(buf2, "r")) != NULL) //セーブファイルがあれば
	{
		fclose(fp);
		strcat(bufS, "*");
		strcat(bufL, "*");
		strcat(bufC, "*");
	}
	MENU_ChangeItemText(_RecordingAnotherMenu, WM_RECORDING_1, bufS);
	MENU_ChangeItemText(_PlayRecordAnotherMenu, WM_PLAYRECORD_1, bufL);
	MENU_ChangeItemText(_MoveRecordMenu, WM_MOVERECORD_1, bufC);
	strcpy(bufS,"Start Recording #2");  strcpy(bufL,"Play Record #2");  strcpy(bufC, "Move \"Default Record Data\" to #2");
	SetRecordFileName( 2,buf2); if ((fp = fopen(buf2, "r")) != NULL) {fclose(fp);strcat(bufS,"*");strcat(bufL,"*");strcat(bufC,"*");}
	MENU_ChangeItemText(_RecordingAnotherMenu,WM_RECORDING_2 ,bufS); MENU_ChangeItemText(_PlayRecordAnotherMenu,WM_PLAYRECORD_2 ,bufL);  MENU_ChangeItemText(_MoveRecordMenu,WM_MOVERECORD_2 ,bufC);
	strcpy(bufS,"Start Recording #3");  strcpy(bufL,"Play Record #3");  strcpy(bufC, "Move \"Default Record Data\" to #3");
	SetRecordFileName( 3,buf2); if ((fp = fopen(buf2, "r")) != NULL) {fclose(fp);strcat(bufS,"*");strcat(bufL,"*");strcat(bufC,"*");}
	MENU_ChangeItemText(_RecordingAnotherMenu,WM_RECORDING_3 ,bufS); MENU_ChangeItemText(_PlayRecordAnotherMenu,WM_PLAYRECORD_3 ,bufL);  MENU_ChangeItemText(_MoveRecordMenu,WM_MOVERECORD_3 ,bufC);
	strcpy(bufS,"Start Recording #4");  strcpy(bufL,"Play Record #4");  strcpy(bufC, "Move \"Default Record Data\" to #4");
	SetRecordFileName( 4,buf2); if ((fp = fopen(buf2, "r")) != NULL) {fclose(fp);strcat(bufS,"*");strcat(bufL,"*");strcat(bufC,"*");}
	MENU_ChangeItemText(_RecordingAnotherMenu,WM_RECORDING_4 ,bufS); MENU_ChangeItemText(_PlayRecordAnotherMenu,WM_PLAYRECORD_4 ,bufL);  MENU_ChangeItemText(_MoveRecordMenu,WM_MOVERECORD_4 ,bufC);
	strcpy(bufS,"Start Recording #5");  strcpy(bufL,"Play Record #5");  strcpy(bufC, "Move \"Default Record Data\" to #5");
	SetRecordFileName( 5,buf2); if ((fp = fopen(buf2, "r")) != NULL) {fclose(fp);strcat(bufS,"*");strcat(bufL,"*");strcat(bufC,"*");}
	MENU_ChangeItemText(_RecordingAnotherMenu,WM_RECORDING_5 ,bufS); MENU_ChangeItemText(_PlayRecordAnotherMenu,WM_PLAYRECORD_5 ,bufL);  MENU_ChangeItemText(_MoveRecordMenu,WM_MOVERECORD_5 ,bufC);
	strcpy(bufS,"Start Recording #6");  strcpy(bufL,"Play Record #6");  strcpy(bufC, "Move \"Default Record Data\" to #6");
	SetRecordFileName( 6,buf2); if ((fp = fopen(buf2, "r")) != NULL) {fclose(fp);strcat(bufS,"*");strcat(bufL,"*");strcat(bufC,"*");}
	MENU_ChangeItemText(_RecordingAnotherMenu,WM_RECORDING_6 ,bufS); MENU_ChangeItemText(_PlayRecordAnotherMenu,WM_PLAYRECORD_6 ,bufL);  MENU_ChangeItemText(_MoveRecordMenu,WM_MOVERECORD_6 ,bufC);
	strcpy(bufS,"Start Recording #7");  strcpy(bufL,"Play Record #7");  strcpy(bufC, "Move \"Default Record Data\" to #7");
	SetRecordFileName( 7,buf2); if ((fp = fopen(buf2, "r")) != NULL) {fclose(fp);strcat(bufS,"*");strcat(bufL,"*");strcat(bufC,"*");}
	MENU_ChangeItemText(_RecordingAnotherMenu,WM_RECORDING_7 ,bufS); MENU_ChangeItemText(_PlayRecordAnotherMenu,WM_PLAYRECORD_7 ,bufL);  MENU_ChangeItemText(_MoveRecordMenu,WM_MOVERECORD_7 ,bufC);
	strcpy(bufS,"Start Recording #8");  strcpy(bufL,"Play Record #8");  strcpy(bufC, "Move \"Default Record Data\" to #8");
	SetRecordFileName( 8,buf2); if ((fp = fopen(buf2, "r")) != NULL) {fclose(fp);strcat(bufS,"*");strcat(bufL,"*");strcat(bufC,"*");}
	MENU_ChangeItemText(_RecordingAnotherMenu,WM_RECORDING_8 ,bufS); MENU_ChangeItemText(_PlayRecordAnotherMenu,WM_PLAYRECORD_8 ,bufL);  MENU_ChangeItemText(_MoveRecordMenu,WM_MOVERECORD_8 ,bufC);
	strcpy(bufS,"Start Recording #9");  strcpy(bufL,"Play Record #9");  strcpy(bufC, "Move \"Default Record Data\" to #9");
	SetRecordFileName( 9,buf2); if ((fp = fopen(buf2, "r")) != NULL) {fclose(fp);strcat(bufS,"*");strcat(bufL,"*");strcat(bufC,"*");}
	MENU_ChangeItemText(_RecordingAnotherMenu,WM_RECORDING_9 ,bufS); MENU_ChangeItemText(_PlayRecordAnotherMenu,WM_PLAYRECORD_9 ,bufL);  MENU_ChangeItemText(_MoveRecordMenu,WM_MOVERECORD_9 ,bufC);
	strcpy(bufS,"Start Recording #10"); strcpy(bufL,"Play Record #10"); strcpy(bufC, "Move \"Default Record Data\" to #10");
	SetRecordFileName(10,buf2); if ((fp = fopen(buf2, "r")) != NULL) {fclose(fp);strcat(bufS,"*");strcat(bufL,"*");strcat(bufC,"*");}
	MENU_ChangeItemText(_RecordingAnotherMenu,WM_RECORDING_10,bufS); MENU_ChangeItemText(_PlayRecordAnotherMenu,WM_PLAYRECORD_10,bufL);  MENU_ChangeItemText(_MoveRecordMenu,WM_MOVERECORD_10,bufC);

	//Kitao追加。v2.12，v2.13
	switch (_ScreenshotMagnification)
	{
		case    1: MENU_CheckRadioItem(_SShotSettingMenu, WM_SCREENSHOT_X1, WM_SCREENSHOT_XN, WM_SCREENSHOT_X1); break;
		case    2: MENU_CheckRadioItem(_SShotSettingMenu, WM_SCREENSHOT_X1, WM_SCREENSHOT_XN, WM_SCREENSHOT_X2); break;
		case    3: MENU_CheckRadioItem(_SShotSettingMenu, WM_SCREENSHOT_X1, WM_SCREENSHOT_XN, WM_SCREENSHOT_X3); break;
		case    4: MENU_CheckRadioItem(_SShotSettingMenu, WM_SCREENSHOT_X1, WM_SCREENSHOT_XN, WM_SCREENSHOT_X4); break;
		default: //0
			MENU_CheckRadioItem(_SShotSettingMenu, WM_SCREENSHOT_X1, WM_SCREENSHOT_XN, WM_SCREENSHOT_XN); break;
	}
	switch (_ScreenshotScanLineType)
	{
		case 0: MENU_CheckRadioItem(_SShotSettingMenu, WM_SSHOT_SAMEPLAYING, WM_SSHOT_NONSCANLINED, WM_SSHOT_NONSCANLINED); break;
		case 1: MENU_CheckRadioItem(_SShotSettingMenu, WM_SSHOT_SAMEPLAYING, WM_SSHOT_NONSCANLINED, WM_SSHOT_SCANLINED); break;
		case 2: MENU_CheckRadioItem(_SShotSettingMenu, WM_SSHOT_SAMEPLAYING, WM_SSHOT_NONSCANLINED, WM_SSHOT_HRSCANLINED); break;
		//case 3: 縦スキャンライン。現在未実装。
		case 4: MENU_CheckRadioItem(_SShotSettingMenu, WM_SSHOT_SAMEPLAYING, WM_SSHOT_NONSCANLINED, WM_SSHOT_TVMODE); break;
		default: //-1
			MENU_CheckRadioItem(_SShotSettingMenu, WM_SSHOT_SAMEPLAYING, WM_SSHOT_NONSCANLINED, WM_SSHOT_SAMEPLAYING); break;
	}
	if (_bScreenshotDialog)
		MENU_CheckRadioItem(_SShotSettingMenu, WM_SSHOT_SAVEFOLDER, WM_SSHOT_SAVEDIALOG, WM_SSHOT_SAVEDIALOG);
	else
		MENU_CheckRadioItem(_SShotSettingMenu, WM_SSHOT_SAVEFOLDER, WM_SSHOT_SAVEDIALOG, WM_SSHOT_SAVEFOLDER);

	//Kitao追加。v1.29
	if (_bOutputWavNext)
		MENU_CheckRadioItem(_WavSettingMenu, WM_OUTPUT_WAVS1, WM_OUTPUT_WAVS2, WM_OUTPUT_WAVS2);
	else
		MENU_CheckRadioItem(_WavSettingMenu, WM_OUTPUT_WAVS1, WM_OUTPUT_WAVS2, WM_OUTPUT_WAVS1);
	MENU_CheckItem(_WavSettingMenu, WM_OUTPUT_WAVNT, JOYPAD_GetMakeHesNoSound());
	MENU_CheckItem(_WavSettingMenu, WM_OUTPUT_WAVBE, _bOutputWavAddNoSound);

	//Kitao追加
	SetGameFileNameBuf(FALSE,FALSE, _RecentRom[1], buf, TRUE);
	if (strcmp(_RecentRom[1], " ") == 0)
		strcpy(buf2, ">>             ");//履歴がなかった場合、見栄えを良くするためにスペースを追加。
	else
	{	//ショートカットキーで選択してしまわないように先頭にスペースを入れる。
		//例えばRキーを２回押すとR-TYPEが選択決定されてしまうため、それを防ぐのに必要。
		strcpy(buf2, ">> ");
		strcat(buf2, buf);
		strcat(buf2, "   ");
	}
	MENU_ChangeItemText(_RecentMenu, WM_RECENT_1, buf2);
	SetGameFileNameBuf(FALSE,FALSE, _RecentRom[2], buf, TRUE); strcpy(buf2, " ::  "); strcat(buf2, buf); strcat(buf2, "    "); MENU_ChangeItemText(_RecentMenu, WM_RECENT_2, buf2);
	SetGameFileNameBuf(FALSE,FALSE, _RecentRom[3], buf, TRUE); strcpy(buf2, " ::  "); strcat(buf2, buf); strcat(buf2, "    "); MENU_ChangeItemText(_RecentMenu, WM_RECENT_3, buf2);
	SetGameFileNameBuf(FALSE,FALSE, _RecentRom[4], buf, TRUE); strcpy(buf2, " ::  "); strcat(buf2, buf); strcat(buf2, "    "); MENU_ChangeItemText(_RecentMenu, WM_RECENT_4, buf2);
	SetGameFileNameBuf(FALSE,FALSE, _RecentRom[5], buf, TRUE); strcpy(buf2, " ::  "); strcat(buf2, buf); strcat(buf2, "    "); MENU_ChangeItemText(_RecentMenu, WM_RECENT_5, buf2);
	SetGameFileNameBuf(FALSE,FALSE, _RecentRom[6], buf, TRUE); strcpy(buf2, " ::  "); strcat(buf2, buf); strcat(buf2, "    "); MENU_ChangeItemText(_RecentMenu, WM_RECENT_6, buf2);
	SetGameFileNameBuf(FALSE,FALSE, _RecentRom[7], buf, TRUE); strcpy(buf2, " ::  "); strcat(buf2, buf); strcat(buf2, "    "); MENU_ChangeItemText(_RecentMenu, WM_RECENT_7, buf2);
	SetGameFileNameBuf(FALSE,FALSE, _RecentRom[8], buf, TRUE); strcpy(buf2, " ::  "); strcat(buf2, buf); strcat(buf2, "    "); MENU_ChangeItemText(_RecentMenu, WM_RECENT_8, buf2);
	SetGameFileNameBuf(FALSE,FALSE, _RecentRom[9], buf, TRUE); strcpy(buf2, " ::  "); strcat(buf2, buf); strcat(buf2, "    "); MENU_ChangeItemText(_RecentMenu, WM_RECENT_9, buf2);
	SetGameFileNameBuf(FALSE,FALSE, _RecentRom[10], buf, TRUE); strcpy(buf2, " ::  "); strcat(buf2, buf); strcat(buf2, "    "); MENU_ChangeItemText(_RecentMenu, WM_RECENT_10, buf2);
	SetGameFileNameBuf(FALSE,FALSE, _RecentRom[11], buf, TRUE); strcpy(buf2, " ::  "); strcat(buf2, buf); strcat(buf2, "    "); MENU_ChangeItemText(_RecentMenu, WM_RECENT_11, buf2);
	SetGameFileNameBuf(FALSE,FALSE, _RecentRom[12], buf, TRUE); strcpy(buf2, " ::  "); strcat(buf2, buf); strcat(buf2, "    "); MENU_ChangeItemText(_RecentMenu, WM_RECENT_12, buf2);
	SetGameFileNameBuf(FALSE,FALSE, _RecentRom[13], buf, TRUE); strcpy(buf2, " ::  "); strcat(buf2, buf); strcat(buf2, "    "); MENU_ChangeItemText(_RecentMenu, WM_RECENT_13, buf2);
	SetGameFileNameBuf(FALSE,FALSE, _RecentRom[14], buf, TRUE); strcpy(buf2, " ::  "); strcat(buf2, buf); strcat(buf2, "    "); MENU_ChangeItemText(_RecentMenu, WM_RECENT_14, buf2);
	SetGameFileNameBuf(FALSE,FALSE, _RecentRom[15], buf, TRUE); strcpy(buf2, " ::  "); strcat(buf2, buf); strcat(buf2, "    "); MENU_ChangeItemText(_RecentMenu, WM_RECENT_15, buf2);
	SetGameFileNameBuf(FALSE,FALSE, _RecentRom[16], buf, TRUE); strcpy(buf2, " ::  "); strcat(buf2, buf); strcat(buf2, "    "); MENU_ChangeItemText(_RecentMenu, WM_RECENT_16, buf2);
	SetGameFileNameBuf(FALSE,FALSE, _RecentRom[17], buf, TRUE); strcpy(buf2, " ::  "); strcat(buf2, buf); strcat(buf2, "    "); MENU_ChangeItemText(_RecentMenu, WM_RECENT_17, buf2);
	SetGameFileNameBuf(FALSE,FALSE, _RecentRom[18], buf, TRUE); strcpy(buf2, " ::  "); strcat(buf2, buf); strcat(buf2, "    "); MENU_ChangeItemText(_RecentMenu, WM_RECENT_18, buf2);
	SetGameFileNameBuf(FALSE,FALSE, _RecentRom[19], buf, TRUE); strcpy(buf2, " ::  "); strcat(buf2, buf); strcat(buf2, "    "); MENU_ChangeItemText(_RecentMenu, WM_RECENT_19, buf2);
	SetGameFileNameBuf(FALSE,FALSE, _RecentRom[20], buf, TRUE); strcpy(buf2, " ::  "); strcat(buf2, buf); strcat(buf2, "    "); MENU_ChangeItemText(_RecentMenu, WM_RECENT_20, buf2);

	//Kitao追加
	switch (MAINBOARD_GetFastForwardingR())
	{
		case    0: MENU_CheckRadioItem(_SpeedMenu, WM_SPEED_V0, WM_SPEED_V9, WM_SPEED_V0); break;
		case   10: MENU_CheckRadioItem(_SpeedMenu, WM_SPEED_V0, WM_SPEED_V9, WM_SPEED_V1); break;
		case    5: MENU_CheckRadioItem(_SpeedMenu, WM_SPEED_V0, WM_SPEED_V9, WM_SPEED_V2); break;
		case    3: MENU_CheckRadioItem(_SpeedMenu, WM_SPEED_V0, WM_SPEED_V9, WM_SPEED_V3); break;
		case    2: MENU_CheckRadioItem(_SpeedMenu, WM_SPEED_V0, WM_SPEED_V9, WM_SPEED_V4); break;
		case 1001: MENU_CheckRadioItem(_SpeedMenu, WM_SPEED_V0, WM_SPEED_V9, WM_SPEED_V5); break;
		case 1000: MENU_CheckRadioItem(_SpeedMenu, WM_SPEED_V0, WM_SPEED_V9, WM_SPEED_V6); break;
		case 2010: MENU_CheckRadioItem(_SpeedMenu, WM_SPEED_V0, WM_SPEED_V9, WM_SPEED_V7); break;
		case 2004: MENU_CheckRadioItem(_SpeedMenu, WM_SPEED_V0, WM_SPEED_V9, WM_SPEED_V8); break;
		case 2002: MENU_CheckRadioItem(_SpeedMenu, WM_SPEED_V0, WM_SPEED_V9, WM_SPEED_V9); break;
	}
	if (_bUseVideoSpeedUpButton)
		MENU_CheckRadioItem(_SpeedMenu, WM_SPEED_VAL, WM_SPEED_VUSE, WM_SPEED_VUSE);
	else
		MENU_CheckRadioItem(_SpeedMenu, WM_SPEED_VAL, WM_SPEED_VUSE, WM_SPEED_VAL);
	if (_MySetOverClockType == -2)
	{
		MENU_ChangeItemText(_CpuSpeedMenu, WM_SPEED_M2, "CPU Speed x0.50   \tDelete");
		MENU_ChangeItemText(_CpuSpeedMenu, WM_SPEED_M1, "CPU Speed x0.75   \tCtrl+Del");
	}
	else
	{
		MENU_ChangeItemText(_CpuSpeedMenu, WM_SPEED_M2, "CPU Speed x0.50   \tCtrl+Del");
		if (_MySetOverClockType == -1) MENU_ChangeItemText(_CpuSpeedMenu, WM_SPEED_M1, "CPU Speed x0.75   \tDelete");
		else MENU_ChangeItemText(_CpuSpeedMenu, WM_SPEED_M1, "CPU Speed x0.75");
	}
	if (_MySetOverClockType == 1) MENU_ChangeItemText(_CpuSpeedMenu, WM_SPEED_P1, "CPU Speed x1.25   \tDelete");
	else MENU_ChangeItemText(_CpuSpeedMenu, WM_SPEED_P1, "CPU Speed x1.25");
	if (_MySetOverClockType == 2) MENU_ChangeItemText(_CpuSpeedMenu, WM_SPEED_P2, "CPU Speed x1.33   \tDelete");
	else MENU_ChangeItemText(_CpuSpeedMenu, WM_SPEED_P2, "CPU Speed x1.33");
	if (_MySetOverClockType == 3) MENU_ChangeItemText(_CpuSpeedMenu, WM_SPEED_P3, "CPU Speed x1.50   \tDelete");
	else MENU_ChangeItemText(_CpuSpeedMenu, WM_SPEED_P3, "CPU Speed x1.50");
	if (_MySetOverClockType == 4) MENU_ChangeItemText(_CpuSpeedMenu, WM_SPEED_P4, "CPU Speed x2.00   \tDelete");
	else MENU_ChangeItemText(_CpuSpeedMenu, WM_SPEED_P4, "CPU Speed x2.00");
	if (_MySetOverClockType == 5) MENU_ChangeItemText(_CpuSpeedMenu, WM_SPEED_P5, "CPU Speed x2.50   \tDelete");
	else MENU_ChangeItemText(_CpuSpeedMenu, WM_SPEED_P5, "CPU Speed x2.50");
	if (_MySetOverClockType == 6) MENU_ChangeItemText(_CpuSpeedMenu, WM_SPEED_P6, "CPU Speed x3.00   \tDelete");
	else MENU_ChangeItemText(_CpuSpeedMenu, WM_SPEED_P6, "CPU Speed x3.00");
	if (_MySetOverClockType == 100) MENU_ChangeItemText(_CpuSpeedMenu, WM_SPEED_T1, "CPU Speed TURBOx1   \tDelete");
	else MENU_ChangeItemText(_CpuSpeedMenu, WM_SPEED_T1, "CPU Speed TURBOx1");
	if (_MySetOverClockType == 300)
	{
		MENU_ChangeItemText(_CpuSpeedMenu, WM_SPEED_T2, "CPU Speed TURBOx2   \tShift+Del");
		MENU_ChangeItemText(_CpuSpeedMenu, WM_SPEED_T3, "CPU Speed TURBOx3   \tDelete");
	}
	else
	{
		if (_MySetOverClockType == 200) MENU_ChangeItemText(_CpuSpeedMenu, WM_SPEED_T2, "CPU Speed TURBOx2   \tDelete");
		else MENU_ChangeItemText(_CpuSpeedMenu, WM_SPEED_T2, "CPU Speed TURBOx2");
		MENU_ChangeItemText(_CpuSpeedMenu, WM_SPEED_T3, "CPU Speed TURBOx3   \tShift+Del");
	}
	MENU_ChangeItemText(_CpuSpeedMenu, WM_SPEED_P0, "Normal CPU Speed   \tBS");
	overClockMenuUpdate();
	if (_bLoadStateSpeedSetting) //v2.36追加
		MENU_CheckRadioItem(_SpeedMenu, WM_SPEED_UNLOAD, WM_SPEED_LOAD, WM_SPEED_LOAD);
	else
		MENU_CheckRadioItem(_SpeedMenu, WM_SPEED_UNLOAD, WM_SPEED_LOAD, WM_SPEED_UNLOAD);
	MENU_CheckItem(_SpeedMenu, WM_SPEED_FASTCD, CDROM_GetFastCD());
	MENU_CheckItem(_SpeedMenu, WM_SPEED_FASTSEEK, CDROM_GetFastSeek());
	MENU_EnableItem(_SpeedMenu, WM_SPEED_FASTSEEK, CDROM_GetFastCD());

	switch (_ScanLineType) //Kitao追加
	{
		case 0: MENU_CheckRadioItem(_ScreenMenu, WM_SCREEN_SPSCANLINED, WM_SCREEN_NONSCANLINED, WM_SCREEN_NONSCANLINED);
				MENU_CheckRadioItem(_ScanlineMenu, WM_SCREEN_SPSCANLINED, WM_SCREEN_NONSCANLINED, WM_SCREEN_NONSCANLINED); break;
		case 1: MENU_CheckRadioItem(_ScreenMenu, WM_SCREEN_SPSCANLINED, WM_SCREEN_NONSCANLINED, WM_SCREEN_SPSCANLINED);
				MENU_CheckRadioItem(_ScanlineMenu, WM_SCREEN_SPSCANLINED, WM_SCREEN_NONSCANLINED, WM_SCREEN_SPSCANLINED); break;
		case 2: MENU_CheckRadioItem(_ScreenMenu, WM_SCREEN_SPSCANLINED, WM_SCREEN_NONSCANLINED, WM_SCREEN_HRSCANLINED);
				MENU_CheckRadioItem(_ScanlineMenu, WM_SCREEN_SPSCANLINED, WM_SCREEN_NONSCANLINED, WM_SCREEN_HRSCANLINED); break;
		//case 3: 縦スキャンライン。現在未実装。
		case 4: MENU_CheckRadioItem(_ScreenMenu, WM_SCREEN_SPSCANLINED, WM_SCREEN_NONSCANLINED, WM_SCREEN_STARTTV);
				MENU_CheckRadioItem(_ScanlineMenu, WM_SCREEN_SPSCANLINED, WM_SCREEN_NONSCANLINED, WM_SCREEN_STARTTV); break;
	}
	if (_ScanLineType == 4)
		MENU_EnableItem(_SizeMenu, WM_SCREEN_SCANDENSITY, _bTvMode); //スペシャルスキャンライン時は、スキャンライン濃度の変更をできないようにする。v2.35
	else
		MENU_EnableItem(_SizeMenu, WM_SCREEN_SCANDENSITY, (_ScanLineType >= 2)||(_bTvMode)); //ノンスキャンラインorスペシャルスキャンライン時は、スキャンライン濃度の変更をできないようにする。v2.35
	switch (_DrawMethod) //v2.00追加
	{
		case 1: MENU_CheckRadioItem(_ScreenMenu, WM_SCREEN_DIRECT3D, WM_SCREEN_DIRECTDRAW, WM_SCREEN_DIRECT3D); break;
		case 2: MENU_CheckRadioItem(_ScreenMenu, WM_SCREEN_DIRECT3D, WM_SCREEN_DIRECTDRAW, WM_SCREEN_DIRECTDRAW); break;
	}
	switch (_D3DEffectTV) //v2.00追加
	{
		case 1: MENU_CheckRadioItem(_ScreenMenu, WM_SCREEN_USE3DT_LINEAR, WM_SCREEN_USE3DT_NONE, WM_SCREEN_USE3DT_POINT); break;
		case 2: MENU_CheckRadioItem(_ScreenMenu, WM_SCREEN_USE3DT_LINEAR, WM_SCREEN_USE3DT_NONE, WM_SCREEN_USE3DT_LINEAR); break;
		default: //0
			MENU_CheckRadioItem(_ScreenMenu, WM_SCREEN_USE3DT_LINEAR, WM_SCREEN_USE3DT_NONE, WM_SCREEN_USE3DT_NONE); break;
	}
	switch (_D3DEffectRGB) //v2.00追加
	{
		case 1: MENU_CheckRadioItem(_ScreenMenu, WM_SCREEN_USE3DR_LINEAR, WM_SCREEN_USE3DR_NONE, WM_SCREEN_USE3DR_POINT); break;
		case 2: MENU_CheckRadioItem(_ScreenMenu, WM_SCREEN_USE3DR_LINEAR, WM_SCREEN_USE3DR_NONE, WM_SCREEN_USE3DR_LINEAR); break;
		default: //0
			MENU_CheckRadioItem(_ScreenMenu, WM_SCREEN_USE3DR_LINEAR, WM_SCREEN_USE3DR_NONE, WM_SCREEN_USE3DR_NONE); break;
	}
	if (_bUseSystemMemoryOnlyWindow) //v1.53更新
		MENU_CheckRadioItem(_ScreenMenu, WM_SCREEN_USE_VIDEOMEM, WM_SCREEN_USE_SYSTEMMEMW, WM_SCREEN_USE_SYSTEMMEMW);
	else if (_bUseVideoCardMemory) //Kitao追加。v1.31
		MENU_CheckRadioItem(_ScreenMenu, WM_SCREEN_USE_VIDEOMEM, WM_SCREEN_USE_SYSTEMMEMW, WM_SCREEN_USE_VIDEOMEM);
	else
		MENU_CheckRadioItem(_ScreenMenu, WM_SCREEN_USE_VIDEOMEM, WM_SCREEN_USE_SYSTEMMEMW, WM_SCREEN_USE_SYSTEMMEM);
	if (_FullScreenColor == 16) //Kitao追加
		MENU_CheckRadioItem(_ScreenMenu, WM_SCREEN_FULL16BITCOLOR, WM_SCREEN_FULL32BITCOLOR, WM_SCREEN_FULL16BITCOLOR);
	else
		MENU_CheckRadioItem(_ScreenMenu, WM_SCREEN_FULL16BITCOLOR, WM_SCREEN_FULL32BITCOLOR, WM_SCREEN_FULL32BITCOLOR);
	if (_bStartFullScreen) //Kitao追加
		MENU_CheckRadioItem(_ScreenMenu, WM_SCREEN_STARTWINDOW, WM_SCREEN_STARTFULL, WM_SCREEN_STARTFULL);
	else
		MENU_CheckRadioItem(_ScreenMenu, WM_SCREEN_STARTWINDOW, WM_SCREEN_STARTFULL, WM_SCREEN_STARTWINDOW);
	if (_bWindowActivate) //Kitao追加
		MENU_CheckRadioItem(_ScreenMenu, WM_SCREEN_ACTIVATE, WM_SCREEN_NONACTIVATE, WM_SCREEN_ACTIVATE);
	else
		MENU_CheckRadioItem(_ScreenMenu, WM_SCREEN_ACTIVATE, WM_SCREEN_NONACTIVATE, WM_SCREEN_NONACTIVATE);
	MENU_CheckItem(_ScreenMenu, WM_SCREEN_UNPAUSE, !_bInactivePause); //Kitao追加
	if (_WindowsAutoVSync > 0) //Kitao追加。v2.65更新
	{
		if (_WindowsAutoVSync == 1)
			MENU_CheckRadioItem(_ScreenMenu, WM_SCREEN_SYNC_VBLANK, WM_SCREEN_SYNC_NON, WM_SCREEN_SYNC_WINDOWS);
		else //==2
			MENU_CheckRadioItem(_ScreenMenu, WM_SCREEN_SYNC_VBLANK, WM_SCREEN_SYNC_NON, WM_SCREEN_SYNC_WINDOWSF);
	}
	else if (_bSyncTo60HzScreen)
		MENU_CheckRadioItem(_ScreenMenu, WM_SCREEN_SYNC_VBLANK, WM_SCREEN_SYNC_NON, WM_SCREEN_SYNC_VBLANK);
	else
		MENU_CheckRadioItem(_ScreenMenu, WM_SCREEN_SYNC_VBLANK, WM_SCREEN_SYNC_NON, WM_SCREEN_SYNC_NON);
	if (_VSyncAdjust > 0) //v2.65追加
		MENU_CheckItem(_ScreenMenu, WM_SCREEN_SYNC_ADJUST, TRUE);
	else
		MENU_CheckItem(_ScreenMenu, WM_SCREEN_SYNC_ADJUST, FALSE);

	//Kitao追加
	if (_bFullScreen)
	{
		MENU_RemoveItem(_SizeMenu, WM_SCREEN_CS);
		if ((_CustomWidth1 != 640)||(_CustomHeight1 != 480))
		{
			sprintf(buf, "%dx%d", (int)_CustomWidth1, (int)_CustomHeight1);
			MENU_InsertItem(_SizeMenu, NULL, buf, WM_SCREEN_CS, WM_SCREEN_X1);
		}
		sprintf(buf, "%dx%d", (int)_CustomWidth2, (int)_CustomHeight2);
		MENU_ChangeItemText(_SizeMenu, WM_SCREEN_X1, buf);
		MENU_ChangeItemText(_SizeMenu, WM_SCREEN_X2, "640x480");
		MENU_ChangeItemText(_SizeMenu, WM_SCREEN_X3, "1024x768");
		MENU_ChangeItemText(_SizeMenu, WM_SCREEN_X4, "1280x1024");
		sprintf(buf, "%dx%d", (int)_CustomWidth3, (int)_CustomHeight3);
		if (_bScreenMenuFull == FALSE)
		{
			MENU_InsertItem(_SizeMenu, NULL, buf, WM_SCREEN_F1, WM_SCREEN_X2);
			MENU_InsertItem(_SizeMenu, NULL, "800x600", WM_SCREEN_F2, WM_SCREEN_X3);
			_bScreenMenuFull = TRUE;
		}
		else
			MENU_ChangeItemText(_SizeMenu, WM_SCREEN_F1, buf);
	}
	else
	{
		MENU_RemoveItem(_SizeMenu, WM_SCREEN_CS);
		MENU_ChangeItemText(_SizeMenu, WM_SCREEN_X1, "x1");
		MENU_ChangeItemText(_SizeMenu, WM_SCREEN_X2, "x2");
		MENU_ChangeItemText(_SizeMenu, WM_SCREEN_X3, "x3");
		MENU_ChangeItemText(_SizeMenu, WM_SCREEN_X4, "x4");
		MENU_RemoveItem(_SizeMenu, WM_SCREEN_F1);
		MENU_RemoveItem(_SizeMenu, WM_SCREEN_F2);
		_bScreenMenuFull = FALSE;
	}
	MENU_CheckItem(_SizeMenu, WM_SCREEN_FULLSCREEN, _bFullScreen);
	if (_bFullScreen)
	{
		switch (_FullMagnification)
		{
			case 0: 
				if ((_CustomWidth1 != 640)||(_CustomHeight1 != 480))
					MENU_CheckRadioItem(_SizeMenu, WM_SCREEN_CS, WM_SCREEN_F2, WM_SCREEN_CS); //v1.35追加
				else
					MENU_CheckRadioItem(_SizeMenu, WM_SCREEN_CS, WM_SCREEN_F2, WM_SCREEN_X2); //640x480の場合項目を追加していないのでデフォの640x480にチェックを入れる。v.2.22
				break;
			case 1: MENU_CheckRadioItem(_SizeMenu, WM_SCREEN_CS, WM_SCREEN_F2, WM_SCREEN_X1); break;
			case 2: MENU_CheckRadioItem(_SizeMenu, WM_SCREEN_CS, WM_SCREEN_F2, WM_SCREEN_X2); break;
			case 3: MENU_CheckRadioItem(_SizeMenu, WM_SCREEN_CS, WM_SCREEN_F2, WM_SCREEN_X3); break;
			case 4: MENU_CheckRadioItem(_SizeMenu, WM_SCREEN_CS, WM_SCREEN_F2, WM_SCREEN_X4); break;
			case 5: MENU_CheckRadioItem(_SizeMenu, WM_SCREEN_CS, WM_SCREEN_F2, WM_SCREEN_F1); break;
			case 6: MENU_CheckRadioItem(_SizeMenu, WM_SCREEN_CS, WM_SCREEN_F2, WM_SCREEN_F2); break;
		}
	}
	else
		MENU_CheckRadioItem(_SizeMenu, WM_SCREEN_X1, WM_SCREEN_X4, WM_SCREEN_X1+_Magnification-1);
	if (_bDisableWindowsAero) //v2.21追加
		MENU_CheckRadioItem(_SizeMenu, WM_SCREEN_DISABLEAERO, WM_SCREEN_USEAERO, WM_SCREEN_DISABLEAERO);
	else
		MENU_CheckRadioItem(_SizeMenu, WM_SCREEN_DISABLEAERO, WM_SCREEN_USEAERO, WM_SCREEN_USEAERO);
	MENU_EnableItem(_SizeMenu, WM_SCREEN_DISABLEAERO, !_bFullScreen); //フルスクリーン時は変更できないようにする
	MENU_EnableItem(_SizeMenu, WM_SCREEN_USEAERO, !_bFullScreen); //
	switch (_ScanLineType) //v2.35追加
	{
		case 0: MENU_ChangeItemText(_SizeMenu, WM_SCREEN_SCANLINE, "Mode  [Non-Scanlined]"); break;
		case 1: MENU_ChangeItemText(_SizeMenu, WM_SCREEN_SCANLINE, "Mode  [Special Scanlined]"); break;
		case 2: MENU_ChangeItemText(_SizeMenu, WM_SCREEN_SCANLINE, "Mode  [Horizontal Scanlined]"); break;
		//case 3: 縦スキャンライン。現在未実装。
		case 4: MENU_ChangeItemText(_SizeMenu, WM_SCREEN_SCANLINE, "Mode  [TV Mode]"); break;
	}
	if (_bOptimizeGamma)
		sprintf(buf, "Scanline Density  [%d%%]", (int)_ScanLineDensity);
	else
		sprintf(buf, "Scanline Density  [%d%% NoOpt]", (int)_ScanLineDensity);
	MENU_ChangeItemText(_SizeMenu, WM_SCREEN_SCANDENSITY, buf);
	switch (_ScanLineDensity) //v1.09追加
	{
		case  0: MENU_CheckRadioItem(_DensityMenu, WM_SCREEN_SCANDENSITY0, WM_SCREEN_SCANDENSITY80, WM_SCREEN_SCANDENSITY0); break;
		case 10: MENU_CheckRadioItem(_DensityMenu, WM_SCREEN_SCANDENSITY0, WM_SCREEN_SCANDENSITY80, WM_SCREEN_SCANDENSITY10); break;
		case 20: MENU_CheckRadioItem(_DensityMenu, WM_SCREEN_SCANDENSITY0, WM_SCREEN_SCANDENSITY80, WM_SCREEN_SCANDENSITY20); break;
		case 30: MENU_CheckRadioItem(_DensityMenu, WM_SCREEN_SCANDENSITY0, WM_SCREEN_SCANDENSITY80, WM_SCREEN_SCANDENSITY30); break;
		case 40: MENU_CheckRadioItem(_DensityMenu, WM_SCREEN_SCANDENSITY0, WM_SCREEN_SCANDENSITY80, WM_SCREEN_SCANDENSITY40); break;
		case 50: MENU_CheckRadioItem(_DensityMenu, WM_SCREEN_SCANDENSITY0, WM_SCREEN_SCANDENSITY80, WM_SCREEN_SCANDENSITY50); break;
		case 60: MENU_CheckRadioItem(_DensityMenu, WM_SCREEN_SCANDENSITY0, WM_SCREEN_SCANDENSITY80, WM_SCREEN_SCANDENSITY60); break;
		case 70: MENU_CheckRadioItem(_DensityMenu, WM_SCREEN_SCANDENSITY0, WM_SCREEN_SCANDENSITY80, WM_SCREEN_SCANDENSITY70); break;
		case 80: MENU_CheckRadioItem(_DensityMenu, WM_SCREEN_SCANDENSITY0, WM_SCREEN_SCANDENSITY80, WM_SCREEN_SCANDENSITY80); break;
	}
	MENU_CheckItem(_DensityMenu, WM_SCREEN_SCANGAMMA, _bOptimizeGamma); //v2.35追加
	if (_GammaValue < 1.000)
		sprintf(buf, "Gamma  [0.%02d]", (int)(_GammaValue*100+0.5));
	else
		sprintf(buf, "Gamma  [1.%02d]", (int)(_GammaValue*100+0.5)-100);
	MENU_ChangeItemText(_SizeMenu, WM_SCREEN_GAMMA, buf);
	if (_GammaValue == 1.340) MENU_CheckRadioItem(_SizeMenu, WM_SCREEN_GAMMA1, WM_SCREEN_GAMMA7, WM_SCREEN_GAMMA5);
	else if (_GammaValue == 0.940) MENU_CheckRadioItem(_SizeMenu, WM_SCREEN_GAMMA1, WM_SCREEN_GAMMA7, WM_SCREEN_GAMMA1);
	else if (_GammaValue == 1.040) MENU_CheckRadioItem(_SizeMenu, WM_SCREEN_GAMMA1, WM_SCREEN_GAMMA7, WM_SCREEN_GAMMA2);
	else if (_GammaValue == 1.140) MENU_CheckRadioItem(_SizeMenu, WM_SCREEN_GAMMA1, WM_SCREEN_GAMMA7, WM_SCREEN_GAMMA3);
	else if (_GammaValue == 1.240) MENU_CheckRadioItem(_SizeMenu, WM_SCREEN_GAMMA1, WM_SCREEN_GAMMA7, WM_SCREEN_GAMMA4);
	else if (_GammaValue == 1.390) MENU_CheckRadioItem(_SizeMenu, WM_SCREEN_GAMMA1, WM_SCREEN_GAMMA7, WM_SCREEN_GAMMA6);
	else if (_GammaValue == 1.440) MENU_CheckRadioItem(_SizeMenu, WM_SCREEN_GAMMA1, WM_SCREEN_GAMMA7, WM_SCREEN_GAMMA7);
	if (_BrightValue >= 2)
		sprintf(buf, "Brightness  [+%d]", (int)_BrightValue-1);
	else
		sprintf(buf, "Brightness  [%d]", (int)_BrightValue-1);
	MENU_ChangeItemText(_SizeMenu, WM_SCREEN_BRIGHT, buf);
	switch (_BrightValue)
	{
		case  0: MENU_CheckRadioItem(_SizeMenu, WM_SCREEN_BRIGHT1, WM_SCREEN_BRIGHT9, WM_SCREEN_BRIGHT1); break;
		case  1: MENU_CheckRadioItem(_SizeMenu, WM_SCREEN_BRIGHT1, WM_SCREEN_BRIGHT9, WM_SCREEN_BRIGHT2); break;
		case  2: MENU_CheckRadioItem(_SizeMenu, WM_SCREEN_BRIGHT1, WM_SCREEN_BRIGHT9, WM_SCREEN_BRIGHT3); break;
		case  3: MENU_CheckRadioItem(_SizeMenu, WM_SCREEN_BRIGHT1, WM_SCREEN_BRIGHT9, WM_SCREEN_BRIGHT4); break;
		case  4: MENU_CheckRadioItem(_SizeMenu, WM_SCREEN_BRIGHT1, WM_SCREEN_BRIGHT9, WM_SCREEN_BRIGHT5); break;
		case  6: MENU_CheckRadioItem(_SizeMenu, WM_SCREEN_BRIGHT1, WM_SCREEN_BRIGHT9, WM_SCREEN_BRIGHT6); break;
		case 11: MENU_CheckRadioItem(_SizeMenu, WM_SCREEN_BRIGHT1, WM_SCREEN_BRIGHT9, WM_SCREEN_BRIGHT7); break;
		case 21: MENU_CheckRadioItem(_SizeMenu, WM_SCREEN_BRIGHT1, WM_SCREEN_BRIGHT9, WM_SCREEN_BRIGHT8); break;
		case 31: MENU_CheckRadioItem(_SizeMenu, WM_SCREEN_BRIGHT1, WM_SCREEN_BRIGHT9, WM_SCREEN_BRIGHT9); break;
	}
	MENU_CheckItem(_ResolutionMenu, WM_SCREEN_FULLSCREENCSA, _bResolutionAutoChange); //v1.35追加
	if (_bStretched)
	{
		if (_bFullStretched)
			MENU_CheckRadioItem(_SizeMenu, WM_SCREEN_NONSTRETCHED, WM_SCREEN_VERTICAL, WM_SCREEN_FULLSTRETCHED);
		else if (_bVStretched)
			MENU_CheckRadioItem(_SizeMenu, WM_SCREEN_NONSTRETCHED, WM_SCREEN_VERTICAL, WM_SCREEN_VERTICAL);
		else
			MENU_CheckRadioItem(_SizeMenu, WM_SCREEN_NONSTRETCHED, WM_SCREEN_VERTICAL, WM_SCREEN_STRETCHED);
	}
	else
		MENU_CheckRadioItem(_SizeMenu, WM_SCREEN_NONSTRETCHED, WM_SCREEN_VERTICAL, WM_SCREEN_NONSTRETCHED);
	MENU_CheckItem(_SizeMenu, WM_SCREEN_MONOCOLOR, (MAINBOARD_GetForceMonoColor() != 0)); //ステートロード(orレジューム)で一時的に白黒モードにすることに対応したので、_MonoColorModeではなくMAINBOARD_GetForceMonoColor()を使う。v2.28
	MENU_CheckItem(_SizeMenu, WM_SCREEN_TV, _bTvMode);
	
	MENU_CheckItem(_SizeMenu, WM_SCREEN_SHOWOVERSCAN, MAINBOARD_GetShowOverscan()); //v1.43追加
	if ((_ShowOverscanTop > 0)&&(_ShowOverscanBottom > 0))
		MENU_CheckRadioItem(_SizeMenu, WM_SCREEN_OVERTB, WM_SCREEN_OVERNONETB, WM_SCREEN_OVERTB);
	else if (_ShowOverscanTop > 0)
		MENU_CheckRadioItem(_SizeMenu, WM_SCREEN_OVERTB, WM_SCREEN_OVERNONETB, WM_SCREEN_OVERTOP);
	else if (_ShowOverscanBottom > 0)
		MENU_CheckRadioItem(_SizeMenu, WM_SCREEN_OVERTB, WM_SCREEN_OVERNONETB, WM_SCREEN_OVERBOTTOM);
	else
		MENU_CheckRadioItem(_SizeMenu, WM_SCREEN_OVERTB, WM_SCREEN_OVERNONETB, WM_SCREEN_OVERNONETB);
	switch (_ShowOverscanHeight)
	{
		case 7: MENU_CheckRadioItem(_SizeMenu, WM_SCREEN_OVERHEIGHT8, WM_SCREEN_OVERHEIGHT1, WM_SCREEN_OVERHEIGHT7); break;
		case 6: MENU_CheckRadioItem(_SizeMenu, WM_SCREEN_OVERHEIGHT8, WM_SCREEN_OVERHEIGHT1, WM_SCREEN_OVERHEIGHT6); break;
		case 4: MENU_CheckRadioItem(_SizeMenu, WM_SCREEN_OVERHEIGHT8, WM_SCREEN_OVERHEIGHT1, WM_SCREEN_OVERHEIGHT4); break;
		case 2: MENU_CheckRadioItem(_SizeMenu, WM_SCREEN_OVERHEIGHT8, WM_SCREEN_OVERHEIGHT1, WM_SCREEN_OVERHEIGHT2); break;
		case 1: MENU_CheckRadioItem(_SizeMenu, WM_SCREEN_OVERHEIGHT8, WM_SCREEN_OVERHEIGHT1, WM_SCREEN_OVERHEIGHT1); break;
		default: //8
			MENU_CheckRadioItem(_SizeMenu, WM_SCREEN_OVERHEIGHT8, WM_SCREEN_OVERHEIGHT1, WM_SCREEN_OVERHEIGHT8); break;
	}
	if (_ShowOverscanLeft > 0)
		MENU_CheckRadioItem(_SizeMenu, WM_SCREEN_OVERLR, WM_SCREEN_OVERNONELR, WM_SCREEN_OVERLR);
	else
		MENU_CheckRadioItem(_SizeMenu, WM_SCREEN_OVERLR, WM_SCREEN_OVERNONELR, WM_SCREEN_OVERNONELR);
	
	if ((_StartShowOverscanTop > 0)&&(_StartShowOverscanBottom > 0))
		MENU_CheckRadioItem(_ScreenMenu, WM_SCREEN_SOVERTB, WM_SCREEN_SOVERNONETB, WM_SCREEN_SOVERTB);
	else if (_StartShowOverscanTop > 0)
		MENU_CheckRadioItem(_ScreenMenu, WM_SCREEN_SOVERTB, WM_SCREEN_SOVERNONETB, WM_SCREEN_SOVERTOP);
	else if (_StartShowOverscanBottom > 0)
		MENU_CheckRadioItem(_ScreenMenu, WM_SCREEN_SOVERTB, WM_SCREEN_SOVERNONETB, WM_SCREEN_SOVERBOTTOM);
	else
		MENU_CheckRadioItem(_ScreenMenu, WM_SCREEN_SOVERTB, WM_SCREEN_SOVERNONETB, WM_SCREEN_SOVERNONETB);
	switch (_StartShowOverscanHeight)
	{
		case 7: MENU_CheckRadioItem(_ScreenMenu, WM_SCREEN_SOVERHEIGHT8, WM_SCREEN_SOVERHEIGHT1, WM_SCREEN_SOVERHEIGHT7); break;
		case 6: MENU_CheckRadioItem(_ScreenMenu, WM_SCREEN_SOVERHEIGHT8, WM_SCREEN_SOVERHEIGHT1, WM_SCREEN_SOVERHEIGHT6); break;
		case 4: MENU_CheckRadioItem(_ScreenMenu, WM_SCREEN_SOVERHEIGHT8, WM_SCREEN_SOVERHEIGHT1, WM_SCREEN_SOVERHEIGHT4); break;
		case 2: MENU_CheckRadioItem(_ScreenMenu, WM_SCREEN_SOVERHEIGHT8, WM_SCREEN_SOVERHEIGHT1, WM_SCREEN_SOVERHEIGHT2); break;
		case 1: MENU_CheckRadioItem(_ScreenMenu, WM_SCREEN_SOVERHEIGHT8, WM_SCREEN_SOVERHEIGHT1, WM_SCREEN_SOVERHEIGHT1); break;
		default: //8
			MENU_CheckRadioItem(_ScreenMenu, WM_SCREEN_SOVERHEIGHT8, WM_SCREEN_SOVERHEIGHT1, WM_SCREEN_SOVERHEIGHT8); break;
	}
	if (_StartShowOverscanLeft > 0)
		MENU_CheckRadioItem(_ScreenMenu, WM_SCREEN_SOVERLR, WM_SCREEN_SOVERNONELR, WM_SCREEN_SOVERLR);
	else
		MENU_CheckRadioItem(_ScreenMenu, WM_SCREEN_SOVERLR, WM_SCREEN_SOVERNONELR, WM_SCREEN_SOVERNONELR);
	MENU_CheckItem(_ScreenMenu, WM_SCREEN_OVERBLACK, _bOverscanHideBlackBelt); //v1.45追加
	
	MENU_CheckItem(_SizeMenu, WM_SCREEN_TOPMOST, _bWindowTopMost);

/*	switch (_SoundSampleRate)
	{
		case 11025: MENU_CheckRadioItem(_SampleRateMenu, WM_AUDIO_SR11025, WM_AUDIO_SR96000, WM_AUDIO_SR11025); break;
		case 22050: MENU_CheckRadioItem(_SampleRateMenu, WM_AUDIO_SR11025, WM_AUDIO_SR96000, WM_AUDIO_SR22050); break;
		case 32000: MENU_CheckRadioItem(_SampleRateMenu, WM_AUDIO_SR11025, WM_AUDIO_SR96000, WM_AUDIO_SR32000); break;
		case 44100: MENU_CheckRadioItem(_SampleRateMenu, WM_AUDIO_SR11025, WM_AUDIO_SR96000, WM_AUDIO_SR44100); break;
		case 48000: MENU_CheckRadioItem(_SampleRateMenu, WM_AUDIO_SR11025, WM_AUDIO_SR96000, WM_AUDIO_SR48000); break;
		case 64000: MENU_CheckRadioItem(_SampleRateMenu, WM_AUDIO_SR11025, WM_AUDIO_SR96000, WM_AUDIO_SR64000); break;
		case 88200: MENU_CheckRadioItem(_SampleRateMenu, WM_AUDIO_SR11025, WM_AUDIO_SR96000, WM_AUDIO_SR88200); break;
		case 96000: MENU_CheckRadioItem(_SampleRateMenu, WM_AUDIO_SR11025, WM_AUDIO_SR96000, WM_AUDIO_SR96000); break;
	}
*/	
	if (_bAutoStereo) //Kitao追加
		MENU_CheckRadioItem(_AudioMenu, WM_AUDIO_STEREO, WM_AUDIO_MONO, WM_AUDIO_STEREO);
	else
		MENU_CheckRadioItem(_AudioMenu, WM_AUDIO_STEREO, WM_AUDIO_MONO, WM_AUDIO_MONO);
	switch (_SoundType) //Kitao追加
	{
		case 2: MENU_CheckRadioItem(_AudioMenu, WM_AUDIO_INI1, WM_AUDIO_INI2, WM_AUDIO_INI2); break;
		default: //=1
				MENU_CheckRadioItem(_AudioMenu, WM_AUDIO_INI1, WM_AUDIO_INI2, WM_AUDIO_INI1); break;
	}
	switch (_SoundBufferSize)
	{
		case 1024: MENU_CheckRadioItem(_AudioMenu, WM_AUDIO_SB1024, WM_AUDIO_SB3072, WM_AUDIO_SB1024); break;
		case 1152: MENU_CheckRadioItem(_AudioMenu, WM_AUDIO_SB1024, WM_AUDIO_SB3072, WM_AUDIO_SB1152); break;
		case 1280: MENU_CheckRadioItem(_AudioMenu, WM_AUDIO_SB1024, WM_AUDIO_SB3072, WM_AUDIO_SB1280); break;
		case 1408: MENU_CheckRadioItem(_AudioMenu, WM_AUDIO_SB1024, WM_AUDIO_SB3072, WM_AUDIO_SB1408); break;
		case 1536: MENU_CheckRadioItem(_AudioMenu, WM_AUDIO_SB1024, WM_AUDIO_SB3072, WM_AUDIO_SB1536); break;
		case 1664: MENU_CheckRadioItem(_AudioMenu, WM_AUDIO_SB1024, WM_AUDIO_SB3072, WM_AUDIO_SB1664); break;
		case 1792: MENU_CheckRadioItem(_AudioMenu, WM_AUDIO_SB1024, WM_AUDIO_SB3072, WM_AUDIO_SB1792); break;
		case 2048: MENU_CheckRadioItem(_AudioMenu, WM_AUDIO_SB1024, WM_AUDIO_SB3072, WM_AUDIO_SB2048); break;
		case 2176: MENU_CheckRadioItem(_AudioMenu, WM_AUDIO_SB1024, WM_AUDIO_SB3072, WM_AUDIO_SB2176); break;
		case 2304: MENU_CheckRadioItem(_AudioMenu, WM_AUDIO_SB1024, WM_AUDIO_SB3072, WM_AUDIO_SB2304); break;
		case 2560: MENU_CheckRadioItem(_AudioMenu, WM_AUDIO_SB1024, WM_AUDIO_SB3072, WM_AUDIO_SB2560); break;
		case 3072: MENU_CheckRadioItem(_AudioMenu, WM_AUDIO_SB1024, WM_AUDIO_SB3072, WM_AUDIO_SB3072); break;
	}
	//Kitao追加
	strcpy(buf, "Mute PSG  [");
	if (PSG_GetMutePsgChannel(0) && PSG_GetMutePsgChannel(1) && PSG_GetMutePsgChannel(2) && PSG_GetMutePsgChannel(3) && PSG_GetMutePsgChannel(4) && PSG_GetMutePsgChannel(5))
		strcat(buf, "All");
	else
	{
		a = 0;
		if (PSG_GetMutePsgChannel(0)) {strcat(buf,"1"); a++;}
		if (PSG_GetMutePsgChannel(1)) {if (a>0) strcat(buf,","); strcat(buf,"2"); a++;}
		if (PSG_GetMutePsgChannel(2)) {if (a>0) strcat(buf,","); strcat(buf,"3"); a++;}
		if (PSG_GetMutePsgChannel(3)) {if (a>0) strcat(buf,","); strcat(buf,"4"); a++;}
		if (PSG_GetMutePsgChannel(4)) {if (a>0) strcat(buf,","); strcat(buf,"5"); a++;}
		if (PSG_GetMutePsgChannel(5)) {if (a>0) strcat(buf,","); strcat(buf,"6"); a++;}
		if (a == 0) strcat(buf,"Unmute");
	}
	strcat(buf, "]");
	MENU_ChangeItemText(_VolumeMenu, WM_VOLUME_MUTEPSG, buf);
	MENU_CheckItem(_VolumeMenu, WM_VOLUME_MUTE1, PSG_GetMutePsgChannel(0));
	MENU_CheckItem(_VolumeMenu, WM_VOLUME_MUTE2, PSG_GetMutePsgChannel(1));
	MENU_CheckItem(_VolumeMenu, WM_VOLUME_MUTE3, PSG_GetMutePsgChannel(2));
	MENU_CheckItem(_VolumeMenu, WM_VOLUME_MUTE4, PSG_GetMutePsgChannel(3));
	MENU_CheckItem(_VolumeMenu, WM_VOLUME_MUTE5, PSG_GetMutePsgChannel(4));
	MENU_CheckItem(_VolumeMenu, WM_VOLUME_MUTE6, PSG_GetMutePsgChannel(5));

	//Kitao追加
	switch (_VolumeStep)
	{
		case 10: MENU_CheckRadioItem(_VolStepMenu, WM_VOLUME_STEP10, WM_VOLUME_STEP1, WM_VOLUME_STEP10); break;
		case  8: MENU_CheckRadioItem(_VolStepMenu, WM_VOLUME_STEP10, WM_VOLUME_STEP1, WM_VOLUME_STEP8); break;
		case  6: MENU_CheckRadioItem(_VolStepMenu, WM_VOLUME_STEP10, WM_VOLUME_STEP1, WM_VOLUME_STEP6); break;
		case  5: MENU_CheckRadioItem(_VolStepMenu, WM_VOLUME_STEP10, WM_VOLUME_STEP1, WM_VOLUME_STEP5); break;
		case  4: MENU_CheckRadioItem(_VolStepMenu, WM_VOLUME_STEP10, WM_VOLUME_STEP1, WM_VOLUME_STEP4); break;
		case  3: MENU_CheckRadioItem(_VolStepMenu, WM_VOLUME_STEP10, WM_VOLUME_STEP1, WM_VOLUME_STEP3); break;
		case  2: MENU_CheckRadioItem(_VolStepMenu, WM_VOLUME_STEP10, WM_VOLUME_STEP1, WM_VOLUME_STEP2); break;
		case  1: MENU_CheckRadioItem(_VolStepMenu, WM_VOLUME_STEP10, WM_VOLUME_STEP1, WM_VOLUME_STEP1); break;
	}

	//Kitao追加
	switch (getCddaAdjust() / 10) //getCddaAdjust()で現在の画面モードでの_CddaAdjust設定を得る。
	{
		case 593: MENU_CheckRadioItem(_AudioSubMenu, WM_AUDIO_CDDA593, WM_AUDIO_CDDA602, WM_AUDIO_CDDA593); break;
		case 594: MENU_CheckRadioItem(_AudioSubMenu, WM_AUDIO_CDDA593, WM_AUDIO_CDDA602, WM_AUDIO_CDDA594); break;
		case 595: MENU_CheckRadioItem(_AudioSubMenu, WM_AUDIO_CDDA593, WM_AUDIO_CDDA602, WM_AUDIO_CDDA595); break;
		case 596: MENU_CheckRadioItem(_AudioSubMenu, WM_AUDIO_CDDA593, WM_AUDIO_CDDA602, WM_AUDIO_CDDA596); break;
		case 597: MENU_CheckRadioItem(_AudioSubMenu, WM_AUDIO_CDDA593, WM_AUDIO_CDDA602, WM_AUDIO_CDDA597); break;
		case 598: MENU_CheckRadioItem(_AudioSubMenu, WM_AUDIO_CDDA593, WM_AUDIO_CDDA602, WM_AUDIO_CDDA598); break;
		case 599: MENU_CheckRadioItem(_AudioSubMenu, WM_AUDIO_CDDA593, WM_AUDIO_CDDA602, WM_AUDIO_CDDA599); break;
		case 600: MENU_CheckRadioItem(_AudioSubMenu, WM_AUDIO_CDDA593, WM_AUDIO_CDDA602, WM_AUDIO_CDDA600); break;
		case 601: MENU_CheckRadioItem(_AudioSubMenu, WM_AUDIO_CDDA593, WM_AUDIO_CDDA602, WM_AUDIO_CDDA601); break;
		case 602: MENU_CheckRadioItem(_AudioSubMenu, WM_AUDIO_CDDA593, WM_AUDIO_CDDA602, WM_AUDIO_CDDA602); break;
	}
	if ((getCddaAdjust() % 10) == 5)
		MENU_CheckRadioItem(_AudioSubMenu, WM_AUDIO_CDDAP000, WM_AUDIO_CDDAP005, WM_AUDIO_CDDAP005);
	else
		MENU_CheckRadioItem(_AudioSubMenu, WM_AUDIO_CDDAP000, WM_AUDIO_CDDAP005, WM_AUDIO_CDDAP000);
	MENU_CheckItem(_AudioSubMenu, WM_AUDIO_SYNC_VBLANK, _bSyncTo60HzScreen); //Setting->ScreenメニューのV-Sync設定と共通設定。v2.32追加
	switch (_CddaDelayFrame) //v2.33追加
	{
		case 0: MENU_CheckRadioItem(_AudioSubMenu, WM_AUDIO_DELAYFRAME0, WM_AUDIO_DELAYFRAME4, WM_AUDIO_DELAYFRAME0); break;
		case 1: MENU_CheckRadioItem(_AudioSubMenu, WM_AUDIO_DELAYFRAME0, WM_AUDIO_DELAYFRAME4, WM_AUDIO_DELAYFRAME1); break;
		case 2: MENU_CheckRadioItem(_AudioSubMenu, WM_AUDIO_DELAYFRAME0, WM_AUDIO_DELAYFRAME4, WM_AUDIO_DELAYFRAME2); break;
		case 3: MENU_CheckRadioItem(_AudioSubMenu, WM_AUDIO_DELAYFRAME0, WM_AUDIO_DELAYFRAME4, WM_AUDIO_DELAYFRAME3); break;
		case 4: MENU_CheckRadioItem(_AudioSubMenu, WM_AUDIO_DELAYFRAME0, WM_AUDIO_DELAYFRAME4, WM_AUDIO_DELAYFRAME4); break;
	}

	switch (_BigSoundBuffer)
	{
		case 0: MENU_CheckRadioItem(_AudioSubMenu, WM_AUDIO_NORMALBUFFER, WM_AUDIO_MOSTBUFFER, WM_AUDIO_NORMALBUFFER); break;
		case 1: MENU_CheckRadioItem(_AudioSubMenu, WM_AUDIO_NORMALBUFFER, WM_AUDIO_MOSTBUFFER, WM_AUDIO_BIGBUFFER); break;
		case 2: MENU_CheckRadioItem(_AudioSubMenu, WM_AUDIO_NORMALBUFFER, WM_AUDIO_MOSTBUFFER, WM_AUDIO_MOSTBUFFER); break;
	}
	switch (_PsgQuality)
	{
		case 1: MENU_CheckRadioItem(_AudioSubMenu, WM_AUDIO_HQPSG1, WM_AUDIO_HQPSG3, WM_AUDIO_HQPSG3); break;
		case 2: MENU_CheckRadioItem(_AudioSubMenu, WM_AUDIO_HQPSG1, WM_AUDIO_HQPSG3, WM_AUDIO_HQPSG2); break;
		case 4: MENU_CheckRadioItem(_AudioSubMenu, WM_AUDIO_HQPSG1, WM_AUDIO_HQPSG3, WM_AUDIO_HQPSG1); break;
	}

	//Kitao追加。使用するCDドライブを設定するメニュー
	a = CDIF_GetNumDevices();
	if (a > 10)
		a = 10;//最大10台
	if (_CurrentCdDrive > a-1)
		_CurrentCdDrive = 0;
	switch (_CurrentCdDrive)
	{
		case 0: MENU_CheckRadioItem(_DriveMenu, WM_DEVICE_CD0, WM_DEVICE_CD9, WM_DEVICE_CD0); break;
		case 1: MENU_CheckRadioItem(_DriveMenu, WM_DEVICE_CD0, WM_DEVICE_CD9, WM_DEVICE_CD1); break;
		case 2: MENU_CheckRadioItem(_DriveMenu, WM_DEVICE_CD0, WM_DEVICE_CD9, WM_DEVICE_CD2); break;
		case 3: MENU_CheckRadioItem(_DriveMenu, WM_DEVICE_CD0, WM_DEVICE_CD9, WM_DEVICE_CD3); break;
		case 4: MENU_CheckRadioItem(_DriveMenu, WM_DEVICE_CD0, WM_DEVICE_CD9, WM_DEVICE_CD4); break;
		case 5: MENU_CheckRadioItem(_DriveMenu, WM_DEVICE_CD0, WM_DEVICE_CD9, WM_DEVICE_CD5); break;
		case 6: MENU_CheckRadioItem(_DriveMenu, WM_DEVICE_CD0, WM_DEVICE_CD9, WM_DEVICE_CD6); break;
		case 7: MENU_CheckRadioItem(_DriveMenu, WM_DEVICE_CD0, WM_DEVICE_CD9, WM_DEVICE_CD7); break;
		case 8: MENU_CheckRadioItem(_DriveMenu, WM_DEVICE_CD0, WM_DEVICE_CD9, WM_DEVICE_CD8); break;
		case 9: MENU_CheckRadioItem(_DriveMenu, WM_DEVICE_CD0, WM_DEVICE_CD9, WM_DEVICE_CD9); break;
	}
	//Kitao追加。CD Change(NoReset)メニューのカレントドライブ欄を更新
	sprintf(buf, "&Current( %c: )", (int)(CDIF_GetDriveLetters(CDIF_GetDeviceInUse())));
	if (_bF1NoReset) //F1キーでリセットせずにＣＤを入れ替える設定の場合。"ＣＤバトル 光の勇者たち" で利用。
		strcat(buf, "\t   F1");
	MENU_ChangeItemText(_CDChangeNRMenu, WM_CHANGE_CDC, buf);
	MENU_CheckItem(_DriveMenu, WM_F1_NORESET, _bF1NoReset);
	//Kitao追加。v2.07
	MENU_CheckItem(_DriveMenu, WM_CD_JUUOUKI, TRUE); //必ずTRUE
	//Kitao追加。v1.11。v1.49
	MENU_CheckItem(_DriveMenu, WM_CD_OSYSCARD1, (_OtherSysCard == 1));
	MENU_CheckItem(_DriveMenu, WM_CD_OSYSCARD2, (_OtherSysCard == 2));
	MENU_CheckItem(_DriveMenu, WM_CD_ARCADECARD, !MAINBOARD_GetArcadeCard());
	MENU_CheckItem(_DriveMenu, WM_CD_BACKUPFULL, MAINBOARD_GetBackupFull());

	//Kitao追加
	switch (_StartFastCD)
	{
		case 0: MENU_CheckRadioItem(_CDROMMenu, WM_STARTFASTCD_ON, WM_STARTFASTCD_PRE, WM_STARTFASTCD_OFF); break;
		case 1: MENU_CheckRadioItem(_CDROMMenu, WM_STARTFASTCD_ON, WM_STARTFASTCD_PRE, WM_STARTFASTCD_ON); break;
		case 2: MENU_CheckRadioItem(_CDROMMenu, WM_STARTFASTCD_ON, WM_STARTFASTCD_PRE, WM_STARTFASTCD_PRE); break;
	}

	//Kitao追加
	switch (_StartFastSeek)
	{
		case 0: MENU_CheckRadioItem(_CDROMMenu, WM_STARTFASTSEEK_ON, WM_STARTFASTSEEK_PRE, WM_STARTFASTSEEK_OFF); break;
		case 1: MENU_CheckRadioItem(_CDROMMenu, WM_STARTFASTSEEK_ON, WM_STARTFASTSEEK_PRE, WM_STARTFASTSEEK_ON); break;
		case 2: MENU_CheckRadioItem(_CDROMMenu, WM_STARTFASTSEEK_ON, WM_STARTFASTSEEK_PRE, WM_STARTFASTSEEK_PRE); break;
	}

	MENU_CheckItem(_SetCpuMenu, WM_AUTO_GRADIUS2, _bAutoGradiusII); //Kitao追加
	MENU_CheckItem(_SetCpuMenu, WM_AUTO_MEGATEN, _bAutoShinMegamiTensei); //Kitao追加

	MENU_CheckItem(_SetInputMenu, WM_INPUT_CHECKPAD_LR, _bCheckSimultaneouslyPush); //Kitao追加

	//Kitao追加
	switch (_StartSpriteLimit)
	{
		case 0: MENU_CheckRadioItem(_SetCpuMenu, WM_STARTSPRITE_OFF, WM_STARTSPRITE_PRE, WM_STARTSPRITE_OFF); break;
		case 1: MENU_CheckRadioItem(_SetCpuMenu, WM_STARTSPRITE_OFF, WM_STARTSPRITE_PRE, WM_STARTSPRITE_ON); break;
		case 2: MENU_CheckRadioItem(_SetCpuMenu, WM_STARTSPRITE_OFF, WM_STARTSPRITE_PRE, WM_STARTSPRITE_PRE); break;
	}

	//Kitao追加。v1.61
	switch (_VolumeEffect)
	{
		case 0: MENU_ChangeItemText(_VolumeMenu, WM_VOLUME_TEMP, "&Temporary Volume  [Mute]"); break;
		case 1: MENU_ChangeItemText(_VolumeMenu, WM_VOLUME_TEMP, "&Temporary Volume  [Norm]"); break;
		case 2: MENU_ChangeItemText(_VolumeMenu, WM_VOLUME_TEMP, "&Temporary Volume  [Half]"); break;
		case 3: MENU_ChangeItemText(_VolumeMenu, WM_VOLUME_TEMP, "&Temporary Volume  [3Qua]"); break;
		case 4: MENU_ChangeItemText(_VolumeMenu, WM_VOLUME_TEMP, "&Temporary Volume  [Quar]"); break;
	}
	updateVolumeMenu(); //v2.29更新

	//Kitao追加
	switch (_RenshaSpeedSel)
	{
		case 0: MENU_CheckRadioItem(_InputMenu, WM_INPUT_TURBO_HIGH, WM_INPUT_TURBO_OFF, WM_INPUT_TURBO_OFF); break;
		case 1: MENU_CheckRadioItem(_InputMenu, WM_INPUT_TURBO_HIGH, WM_INPUT_TURBO_OFF, WM_INPUT_TURBO_HIGH); break;
		case 2: MENU_CheckRadioItem(_InputMenu, WM_INPUT_TURBO_HIGH, WM_INPUT_TURBO_OFF, WM_INPUT_TURBO_MIDDLE); break;
		case 3: MENU_CheckRadioItem(_InputMenu, WM_INPUT_TURBO_HIGH, WM_INPUT_TURBO_OFF, WM_INPUT_TURBO_LOW); break;
	}
	MENU_CheckItem(_InputMenu, WM_INPUT_TURBO_1, JOYPAD_GetRenshaButton1());
	MENU_CheckItem(_InputMenu, WM_INPUT_TURBO_2, JOYPAD_GetRenshaButton2());
	MENU_CheckItem(_InputMenu, WM_INPUT_TURBO_RUN, JOYPAD_GetRenshaButtonRun());

	if (MOUSE_IsConnected())
		MENU_CheckRadioItem(_InputMenu, WM_INPUT_TWO_BUTTON_PAD, WM_INPUT_MOUSE, WM_INPUT_MOUSE);
	else if (JOYPAD_GetConnectSixButton())
		MENU_CheckRadioItem(_InputMenu, WM_INPUT_TWO_BUTTON_PAD, WM_INPUT_MOUSE, WM_INPUT_SIX_BUTTON_PAD);
	else if (JOYPAD_GetConnectThreeButton())
		MENU_CheckRadioItem(_InputMenu, WM_INPUT_TWO_BUTTON_PAD, WM_INPUT_MOUSE, WM_INPUT_THR_BUTTON_PAD);
	else
		MENU_CheckRadioItem(_InputMenu, WM_INPUT_TWO_BUTTON_PAD, WM_INPUT_MOUSE, WM_INPUT_TWO_BUTTON_PAD);
	MENU_CheckItem(_InputMenu, WM_INPUT_MB128, MB128_IsConnected());
	MENU_CheckItem(_InputMenu, WM_INPUT_MULTI_TAP, JOYPAD_GetConnectMultiTap());
	MENU_CheckItem(_InputMenu, WM_INPUT_SWAP_SELRUN, JOYPAD_GetSwapSelRun()); //Kitao追加
	MENU_CheckItem(_InputMenu, WM_INPUT_SWAP_IANDII, JOYPAD_GetSwapIandII()); //Kitao追加
	MENU_CheckItem(_InputMenu, WM_INPUT_CONFIGURE_JOYBG, _bJoypadBackground); //Kitao追加
	MENU_CheckItem(_InputMenu, WM_INPUT_CONFIGURE_KEYBG, _bKeyboardBackground); //Kitao追加

	//Kitao追加。v2.38
	MENU_CheckItem(_FuncButtonMenu, WM_INPUT_FB_CURSOR, _bFuncVolume);
	MENU_CheckItem(_FuncButtonMenu, WM_INPUT_FB_IandII, _bFuncTurboButton);
	MENU_CheckItem(_FuncButtonMenu, WM_INPUT_FB_SEL, _bFuncScreenshot);
	MENU_CheckItem(_FuncButtonMenu, WM_INPUT_FB_RUN, _bFuncRecent); //v2.48追加
	MENU_CheckItem(_FuncButtonMenu, WM_INPUT_FB_VSPEEDUP, _bFuncVSpeedUp);

	//Kitao追加
	MENU_CheckItem(_SettingMenu, WM_MENU_HIDEMENU, _bHideMenu);
	MENU_EnableItem(_SettingMenu, WM_MENU_HIDEMENU, _bFullScreen == FALSE);
	if (_bFullScreen)
		MENU_CheckItem(_SettingMenu, WM_MENU_HIDEMESSAGE, _bFullHideMessage);
	else
		MENU_CheckItem(_SettingMenu, WM_MENU_HIDEMESSAGE, _bHideMessage);

	//Kitao追加
	if (_PriorityClass == HIGH_PRIORITY_CLASS)
		MENU_CheckRadioItem(_SettingMenu, WM_PRIORITY_HIGH, WM_PRIORITY_NORMAL, WM_PRIORITY_HIGH);
	else //Low(Normal)
		MENU_CheckRadioItem(_SettingMenu, WM_PRIORITY_HIGH, WM_PRIORITY_NORMAL, WM_PRIORITY_NORMAL);

	//Kitao追加
	switch (VDC_GetRasterTimingType())
	{
		case 1: MENU_CheckRadioItem(_CpuMenu, WM_RASTERTIMING_MEARLY, WM_RASTERTIMING_MLATE, WM_RASTERTIMING_EARLY); break;
		case 3: MENU_CheckRadioItem(_CpuMenu, WM_RASTERTIMING_MEARLY, WM_RASTERTIMING_MLATE, WM_RASTERTIMING_LATE); break;
		case 4: MENU_CheckRadioItem(_CpuMenu, WM_RASTERTIMING_MEARLY, WM_RASTERTIMING_MLATE, WM_RASTERTIMING_MEARLY); break;
		case 5: MENU_CheckRadioItem(_CpuMenu, WM_RASTERTIMING_MEARLY, WM_RASTERTIMING_MLATE, WM_RASTERTIMING_MLATE); break;
		default:
			MENU_CheckRadioItem(_CpuMenu, WM_RASTERTIMING_MEARLY, WM_RASTERTIMING_MLATE, WM_RASTERTIMING_MIDDLE); break;
	}

	//Kitao追加
	MENU_CheckItem(_CpuMenu, WM_INVALIDATE_CDINST, _bInvalidateCdInstall);
	MENU_CheckItem(_CpuMenu, WM_SUPERGRAFX, (MAINBOARD_GetSuperGrafx() == 1));
	MENU_CheckItem(_CpuMenu, WM_SPRITEOVER, VDC_GetPerformSpriteLimit());

	//Kitao追加
	MENU_CheckItem(_CpuMenu, WM_LAYER_SPRITE, _bSpriteLayer);
	MENU_CheckItem(_CpuMenu, WM_LAYER_SPRITE2, _bSprite2Layer);
	MENU_CheckItem(_CpuMenu, WM_LAYER_BG, _bBGLayer);
	MENU_CheckItem(_CpuMenu, WM_LAYER_BG2, _bBG2Layer);

	RebuildMenu();
}


static BOOL
init_menu()
{
	int		a,i;//Kitao追加
	char	dn[16];//Kitao追加

	_Menu			= MENU_Init();
	_FileMenu		= MENU_CreateSubMenu();
	_CaptureMenu	= MENU_CreateSubMenu();//Kitao追加
	_SaveAnotherMenu= MENU_CreateSubMenu();//Kitao追加
	_SaveAnother2Menu = MENU_CreateSubMenu();//Kitao追加
	_LoadAnotherMenu= MENU_CreateSubMenu();//Kitao追加
	_LoadAnother2Menu = MENU_CreateSubMenu();//Kitao追加
	_RecordingAnotherMenu= MENU_CreateSubMenu();//Kitao追加
	_PlayRecordAnotherMenu= MENU_CreateSubMenu();//Kitao追加
	_MoveRecordMenu = MENU_CreateSubMenu();//Kitao追加
	_AviAttentionMenu = MENU_CreateSubMenu();//Kitao追加
	_SaveButtonMenu	= MENU_CreateSubMenu();//Kitao追加
	_SShotButtonMenu= MENU_CreateSubMenu();//Kitao追加
	_SShotSettingMenu = MENU_CreateSubMenu();//Kitao追加
	_WavAnotherMenu	= MENU_CreateSubMenu();//Kitao追加
	_WavSettingMenu	= MENU_CreateSubMenu();//Kitao追加
	_RecentMenu		= MENU_CreateSubMenu();//Kitao追加
	_DriveMenu		= MENU_CreateSubMenu();//Kitao追加
	_CDChangeNRMenu = MENU_CreateSubMenu();//Kitao追加
	_SettingMenu	= MENU_CreateSubMenu();//Kitao更新
	_SizeMenu		= MENU_CreateSubMenu();//Kitao追加
	_AudioSubMenu	= MENU_CreateSubMenu();//Kitao追加
	_SpeedMenu		= MENU_CreateSubMenu();//Kitao追加
	_VolumeMenu		= MENU_CreateSubMenu();//Kitao追加
	_InfoMenu		= MENU_CreateSubMenu();//Kitao追加
	_CpuMenu		= MENU_CreateSubMenu();
	_ScreenMenu		= MENU_CreateSubMenu();
	_InputMenu		= MENU_CreateSubMenu();
	_AudioMenu		= MENU_CreateSubMenu();
	_CDROMMenu		= MENU_CreateSubMenu();//Kitao追加
	_SetCpuMenu		= MENU_CreateSubMenu();//Kitao追加
	_SetInputMenu	= MENU_CreateSubMenu();//Kitao追加
	_ToolMenu		= MENU_CreateSubMenu();//Kitao追加
	_TurboButtonMenu= MENU_CreateSubMenu();//Kitao追加
	_FuncButtonMenu= MENU_CreateSubMenu();//Kitao追加
//	_SampleRateMenu	= MENU_CreateSubMenu(); Kitaoカット
	_ScanlineMenu	= MENU_CreateSubMenu();//Kitao追加
	_DensityMenu	= MENU_CreateSubMenu();//Kitao追加
	_GammaMenu		= MENU_CreateSubMenu();//Kitao追加
	_BrightnessMenu	= MENU_CreateSubMenu();//Kitao追加
	_ResolutionMenu	= MENU_CreateSubMenu();//Kitao追加
	_Direct3DMenu	= MENU_CreateSubMenu();//Kitao追加
	_DirectDrawMenu	= MENU_CreateSubMenu();//Kitao追加
	_OverscanMenu	= MENU_CreateSubMenu();//Kitao追加
	_StartOverscanMenu = MENU_CreateSubMenu();//Kitao追加
	_OtherSyscardMenu = MENU_CreateSubMenu();//Kitao追加
	_TempVolMenu	= MENU_CreateSubMenu();//Kitao追加
	_VolAttentionMenu = MENU_CreateSubMenu();//Kitao追加
	_VolDetailMenu	= MENU_CreateSubMenu();//Kitao追加
	_VolStepMenu	= MENU_CreateSubMenu();//Kitao追加
	_MuteMenu		= MENU_CreateSubMenu();//Kitao追加
	_DefaultMenu	= MENU_CreateSubMenu();//Kitao追加
	_CpuSpeedMenu	= MENU_CreateSubMenu();//Kitao追加
	_WriteAttentionMenu = MENU_CreateSubMenu();//Kitao追加

	// add sub menu
	MENU_AddItem(_Menu, _InfoMenu, 		"I&nfo", 0);//Kitao追加
	MENU_AddItem(_Menu, _InputMenu, 	"&Input", 0);//Kitao更新
	MENU_AddItem(_Menu, _SpeedMenu, 	"Spee&d", 0);//Kitao追加
	MENU_AddItem(_Menu, _VolumeMenu, 	"&Vol.", 0);//Kitao追加
	MENU_AddItem(_Menu, _AudioSubMenu, 	"&Audio", 0);//Kitao追加
	MENU_AddItem(_Menu, _SizeMenu, 		"&Screen", 0);//Kitao追加
	MENU_AddItem(_Menu, _SettingMenu, 	"S&etting", 0);
	MENU_AddItem(_Menu, _CpuMenu, 		"C&PU", 0);
	MENU_AddItem(_Menu, _DriveMenu,		"&CD-ROM", 0);//Kitao追加
	MENU_AddItem(_Menu, _RecentMenu, 	"&Recent", 0);//Kitao追加
	MENU_AddItem(_Menu, _CaptureMenu, 	"Cap&ture", 0);//Kitao追加。v2.52
	MENU_AddItem(_Menu, _FileMenu, 		"&File", 0);

	// add sub^2 menu
	//Kitao追加
	MENU_AddItem(_SettingMenu, _DefaultMenu, "All Default", 0);
	MENU_AddItem(_SettingMenu, NULL, NULL, 0); //separator 
	MENU_AddItem(_SettingMenu, NULL, "Priority Low", WM_PRIORITY_NORMAL);
	MENU_AddItem(_SettingMenu, NULL, "Priority High (Default)", WM_PRIORITY_HIGH);
	MENU_AddItem(_SettingMenu, NULL, NULL, 0); //separator 
	MENU_AddItem(_SettingMenu, NULL, "Hide Message", WM_MENU_HIDEMESSAGE);
	MENU_AddItem(_SettingMenu, NULL, "Hide Menu", WM_MENU_HIDEMENU);
	MENU_AddItem(_SettingMenu, NULL, NULL, 0); //separator 
	MENU_AddItem(_SettingMenu, _ToolMenu,	  "&Tool", 0);//Kitao追加
	MENU_AddItem(_SettingMenu, _SetInputMenu, "&Input", 0);//Kitao追加
	MENU_AddItem(_SettingMenu, _SetCpuMenu,	  "CP&U", 0);//Kitao追加
	MENU_AddItem(_SettingMenu, _CDROMMenu,    "&CD-ROM", 0);//Kitao追加
	MENU_AddItem(_SettingMenu, _AudioMenu,    "&Audio", 0);
	MENU_AddItem(_SettingMenu, _ScreenMenu,   "&Screen", 0);
	
	//Kitao追加。システムカードのイメージファイルの場所を設定するメニュー
	MENU_AddItem(_DriveMenu, NULL, "Help", WM_CD_HELP);
	MENU_AddItem(_DriveMenu, NULL, NULL, 0); //separator 
	MENU_AddItem(_DriveMenu, NULL, "Play with Backup-RAM Full", WM_CD_BACKUPFULL); //v1.49追加
	MENU_AddItem(_DriveMenu, NULL, "Play with Non ARCADE Card", WM_CD_ARCADECARD); //v1.49追加
	MENU_AddItem(_DriveMenu, NULL, "Play with Other SystemCard 2", WM_CD_OSYSCARD2); //v1.49追加
	MENU_AddItem(_DriveMenu, NULL, "Play with Other SystemCard 1", WM_CD_OSYSCARD1); //v1.49追加
	MENU_AddItem(_DriveMenu, NULL, NULL, 0); //separator 
	MENU_AddItem(_DriveMenu, NULL, "\"Juuouki (J)\" Use \"Other SysCard 1\"", WM_CD_JUUOUKI); //v2.07追加
	MENU_AddItem(_DriveMenu, _OtherSyscardMenu, "Set Other SystemCard image", 0); //v1.49追加
	MENU_AddItem(_DriveMenu, NULL, "Set SystemCard image...", WM_CD_SETSYSCARD);
	MENU_AddItem(_DriveMenu, NULL, NULL, 0); //separator 
	MENU_AddItem(_DriveMenu, NULL, "Open \"install\" Folder", WM_CD_OPENINSTALL);
	MENU_AddItem(_DriveMenu, NULL, "CD Uninstall", WM_CD_UNINSTALL);
	MENU_AddItem(_DriveMenu, NULL, "CD FullInstall (Audio also)", WM_CD_FULLINSTALL); //v2.24追加
	MENU_AddItem(_DriveMenu, NULL, "CD Install (CD speed-up)", WM_CD_INSTALL);
	MENU_AddItem(_DriveMenu, NULL, NULL, 0); //separator 
	MENU_AddItem(_DriveMenu, NULL, "F1 for \"CD Change(NoReset)\"  ", WM_F1_NORESET); //v1.00追加
	MENU_AddItem(_DriveMenu, _CDChangeNRMenu, "&CD Change(NoReset)", 0); //v1.00追加。「ＣＤバトル」でディスクの入れ替えが必要。
	MENU_AddItem(_DriveMenu, NULL, "CD Change\tF1", WM_CD_CHANGE); //Kitao追加
	MENU_AddItem(_DriveMenu, NULL, NULL, 0); //separator 
	//Kitao追加。使用するCDドライブを設定するメニュー
	a = CDIF_GetNumDevices();
	if (a > 10)
		a = 10;//最大10台
	for (i = a-1; i >= 0 ; i--)
	{
		sprintf(dn, "%c:", (int)(CDIF_GetDriveLetters(i)));
		switch (i)
		{
			case 9: MENU_AddItem(_DriveMenu, NULL, dn, WM_DEVICE_CD9); break;
			case 8: MENU_AddItem(_DriveMenu, NULL, dn, WM_DEVICE_CD8); break;
			case 7: MENU_AddItem(_DriveMenu, NULL, dn, WM_DEVICE_CD7); break;
			case 6: MENU_AddItem(_DriveMenu, NULL, dn, WM_DEVICE_CD6); break;
			case 5: MENU_AddItem(_DriveMenu, NULL, dn, WM_DEVICE_CD5); break;
			case 4: MENU_AddItem(_DriveMenu, NULL, dn, WM_DEVICE_CD4); break;
			case 3: MENU_AddItem(_DriveMenu, NULL, dn, WM_DEVICE_CD3); break;
			case 2: MENU_AddItem(_DriveMenu, NULL, dn, WM_DEVICE_CD2); break;
			case 1: MENU_AddItem(_DriveMenu, NULL, dn, WM_DEVICE_CD1); break;
			case 0: MENU_AddItem(_DriveMenu, NULL, dn, WM_DEVICE_CD0); break;
		}
	}
	MENU_AddItem(_DriveMenu, NULL, NULL, 0); //separator 
	MENU_AddItem(_DriveMenu, NULL, "&Play FullInstalled CD-Game...\tP", WM_CD_PLAYINSTALL); //v2.24追加
	
	//Kitao追加
	MENU_AddItem(_SpeedMenu, NULL, "Help", WM_SPEED_HELP);
	MENU_AddItem(_SpeedMenu, NULL, NULL, 0); //separator 
	MENU_AddItem(_SpeedMenu, NULL, "Fast Seek (Default)", WM_SPEED_FASTSEEK);
	MENU_AddItem(_SpeedMenu, NULL, "Fast CD (Default)", WM_SPEED_FASTCD);
	MENU_AddItem(_SpeedMenu, NULL, NULL, 0); //separator 
	MENU_AddItem(_SpeedMenu, NULL, "Load Speed at LoadState", WM_SPEED_LOAD);
	MENU_AddItem(_SpeedMenu, NULL, "Unload Speed at LoadState", WM_SPEED_UNLOAD);
	MENU_AddItem(_SpeedMenu, NULL, NULL, 0); //separator 
	MENU_AddItem(_SpeedMenu, _CpuSpeedMenu, "CPU Speed  [Normal]", WM_SPEED_CPU);
	MENU_AddItem(_SpeedMenu, NULL, NULL, 0); //separator 
	MENU_AddItem(_SpeedMenu, NULL, "Configure VSpeedUp Button...", WM_SPEED_VSET);
	MENU_AddItem(_SpeedMenu, NULL, NULL, 0); //separator 
	MENU_AddItem(_SpeedMenu, NULL, "Use VSpeedUp Button\t Ins", WM_SPEED_VUSE);
	MENU_AddItem(_SpeedMenu, NULL, "Always VideoSpeedUp\t Ins", WM_SPEED_VAL);
	MENU_AddItem(_SpeedMenu, NULL, NULL, 0); //separator 
	MENU_AddItem(_SpeedMenu, NULL, "Video Speed x1.10\tEnd", WM_SPEED_V1);
	MENU_AddItem(_SpeedMenu, NULL, "Video Speed x1.20",	WM_SPEED_V2);
	MENU_AddItem(_SpeedMenu, NULL, "Video Speed x1.33",	WM_SPEED_V3);
	MENU_AddItem(_SpeedMenu, NULL, "Video Speed x1.50",	WM_SPEED_V4);
	MENU_AddItem(_SpeedMenu, NULL, "Video Speed x1.67",	WM_SPEED_V5);
	MENU_AddItem(_SpeedMenu, NULL, "Video Speed x1.83\tHome", WM_SPEED_V6);
	MENU_AddItem(_SpeedMenu, NULL, "Video Speed x0.50", WM_SPEED_V9);
	MENU_AddItem(_SpeedMenu, NULL, "Video Speed x0.75", WM_SPEED_V8);
	MENU_AddItem(_SpeedMenu, NULL, "Video Speed x0.90", WM_SPEED_V7);
	MENU_AddItem(_SpeedMenu, NULL, "Normal Video Speed\tBS", WM_SPEED_V0);
	
	//Kitao追加
	MENU_AddItem(_SizeMenu, NULL, "Window TopMost", WM_SCREEN_TOPMOST);
	MENU_AddItem(_SizeMenu, NULL, NULL, 0);	// separator 
	MENU_AddItem(_SizeMenu, _OverscanMenu, "Overscan Customize", 0);
	MENU_AddItem(_SizeMenu, NULL, "Show Overscan Area\tF11", WM_SCREEN_SHOWOVERSCAN);
	MENU_AddItem(_SizeMenu, NULL, NULL, 0); //separator 
	MENU_AddItem(_SizeMenu, NULL, "&Vertical Stretched", WM_SCREEN_VERTICAL);
	MENU_AddItem(_SizeMenu, NULL, "&Full Stretched", WM_SCREEN_FULLSTRETCHED);
	MENU_AddItem(_SizeMenu, NULL, "&Real Stretched (Default)", WM_SCREEN_STRETCHED);
	MENU_AddItem(_SizeMenu, NULL, "&Non-Stretched", WM_SCREEN_NONSTRETCHED);
	MENU_AddItem(_SizeMenu, NULL, NULL, 0); //separator 
	MENU_AddItem(_SizeMenu, _ResolutionMenu, "FullScreen Customize", 0);
	MENU_AddItem(_SizeMenu, NULL, "FullScreen 640x480", WM_SCREEN_FULLSCREEN640);
	MENU_AddItem(_SizeMenu, NULL, "FullScreen\tF12", WM_SCREEN_FULLSCREEN);
	MENU_AddItem(_SizeMenu, NULL, NULL, 0); //separator 
	MENU_AddItem(_SizeMenu, NULL, "Minimize && Pause\tF9", WM_SCREEN_MINIMIZE);
	MENU_AddItem(_SizeMenu, NULL, NULL, 0); //separator 
	MENU_AddItem(_SizeMenu, NULL, "MonoColor Mode", WM_SCREEN_MONOCOLOR);//v2.28追加
	MENU_AddItem(_SizeMenu, _BrightnessMenu, "Brightness  [0]", WM_SCREEN_BRIGHT);
	MENU_AddItem(_SizeMenu, _GammaMenu, "Gamma  [1.34]", WM_SCREEN_GAMMA);
	MENU_AddItem(_SizeMenu, _DensityMenu, "Scanline Density  [70%]", WM_SCREEN_SCANDENSITY);
	MENU_AddItem(_SizeMenu, NULL, "TV Mode\tCtrl+F11", WM_SCREEN_TV);
	MENU_AddItem(_SizeMenu, _ScanlineMenu, "Mode  [Special Scanlined]", WM_SCREEN_SCANLINE);
	MENU_AddItem(_SizeMenu, NULL, NULL, 0); //separator 
	MENU_AddItem(_SizeMenu, NULL, "Use Aero (Time-Lag exists)\tCtrl+F12", WM_SCREEN_USEAERO);
	MENU_AddItem(_SizeMenu, NULL, "Disable Aero (Recommend)\tCtrl+F12", WM_SCREEN_DISABLEAERO);
	MENU_AddItem(_SizeMenu, NULL, NULL, 0); //separator 
	MENU_AddItem(_SizeMenu, NULL, "x4", WM_SCREEN_X4);
	MENU_AddItem(_SizeMenu, NULL, "x3", WM_SCREEN_X3);
	MENU_AddItem(_SizeMenu, NULL, "x2", WM_SCREEN_X2);
	MENU_AddItem(_SizeMenu, NULL, "x1", WM_SCREEN_X1);
	
	//Kitao追加
	MENU_AddItem(_AudioSubMenu, NULL, "Light PSG (Fast)", WM_AUDIO_HQPSG3);
	MENU_AddItem(_AudioSubMenu, NULL, "A Little Light PSG", WM_AUDIO_HQPSG2);
	MENU_AddItem(_AudioSubMenu, NULL, "High Quality PSG (Default)", WM_AUDIO_HQPSG1);
	MENU_AddItem(_AudioSubMenu, NULL, NULL, 0); //separator 
	MENU_AddItem(_AudioSubMenu, NULL, "Most BigAudioBuffer", WM_AUDIO_MOSTBUFFER);
	MENU_AddItem(_AudioSubMenu, NULL, "BigAudioBuffer", WM_AUDIO_BIGBUFFER);
	MENU_AddItem(_AudioSubMenu, NULL, "NormalAudioBuffer", WM_AUDIO_NORMALBUFFER);
	MENU_AddItem(_AudioSubMenu, NULL, NULL, 0); //separator 
	MENU_AddItem(_AudioSubMenu, NULL, "CD-DA Delay Frame 4", WM_AUDIO_DELAYFRAME4);
	MENU_AddItem(_AudioSubMenu, NULL, "CD-DA Delay Frame 3", WM_AUDIO_DELAYFRAME3);
	MENU_AddItem(_AudioSubMenu, NULL, "CD-DA Delay Frame 2 (Default)", WM_AUDIO_DELAYFRAME2);
	MENU_AddItem(_AudioSubMenu, NULL, "CD-DA Delay Frame 1", WM_AUDIO_DELAYFRAME1);
	MENU_AddItem(_AudioSubMenu, NULL, "CD-DA Delay Frame 0 (for CRT)", WM_AUDIO_DELAYFRAME0);
	MENU_AddItem(_AudioSubMenu, NULL, NULL, 0); //separator 
	MENU_AddItem(_AudioSubMenu, NULL, "V-Sync ( 59-61Hz )", WM_AUDIO_SYNC_VBLANK);
	MENU_AddItem(_AudioSubMenu, NULL, NULL, 0); //separator 
	MENU_AddItem(_AudioSubMenu, NULL, "Adjust CD-DA Plus 0.05Hz (Default)", WM_AUDIO_CDDAP005);
	MENU_AddItem(_AudioSubMenu, NULL, "Adjust CD-DA Plus 0.00Hz", WM_AUDIO_CDDAP000);
	MENU_AddItem(_AudioSubMenu, NULL, NULL, 0); //separator 
	MENU_AddItem(_AudioSubMenu, NULL, "Adjust CD-DA V59.3Hz (Audio Slow)", WM_AUDIO_CDDA593);
	MENU_AddItem(_AudioSubMenu, NULL, "Adjust CD-DA V59.4Hz", WM_AUDIO_CDDA594);
	MENU_AddItem(_AudioSubMenu, NULL, "Adjust CD-DA V59.5Hz", WM_AUDIO_CDDA595);
	MENU_AddItem(_AudioSubMenu, NULL, "Adjust CD-DA V59.6Hz", WM_AUDIO_CDDA596);
	MENU_AddItem(_AudioSubMenu, NULL, "Adjust CD-DA V59.7Hz", WM_AUDIO_CDDA597);
	MENU_AddItem(_AudioSubMenu, NULL, "Adjust CD-DA V59.8Hz", WM_AUDIO_CDDA598);
	MENU_AddItem(_AudioSubMenu, NULL, "Adjust CD-DA V59.9Hz (Default)", WM_AUDIO_CDDA599);
	MENU_AddItem(_AudioSubMenu, NULL, "Adjust CD-DA V60.0Hz", WM_AUDIO_CDDA600);
	MENU_AddItem(_AudioSubMenu, NULL, "Adjust CD-DA V60.1Hz", WM_AUDIO_CDDA601);
	MENU_AddItem(_AudioSubMenu, NULL, "Adjust CD-DA V60.2Hz (Audio Fast)", WM_AUDIO_CDDA602);
	MENU_AddItem(_AudioSubMenu, NULL, NULL, 0); //separator 
	MENU_AddItem(_AudioSubMenu, NULL, "Adjust CD-DA Auto Set", WM_AUDIO_CDDAAUTO);

	//Kitao追加
	MENU_AddItem(_VolumeMenu, NULL, "Balance Default", WM_AUDIO_DEFAULTVOLUME);
	MENU_AddItem(_VolumeMenu, NULL, "Balance Setting...", WM_AUDIO_SETVOLUME);
	MENU_AddItem(_VolumeMenu, NULL, NULL, 0); //separator 
	MENU_AddItem(_VolumeMenu, _MuteMenu, "Mute PSG  [Unmute]", WM_VOLUME_MUTEPSG);
	MENU_AddItem(_VolumeMenu, NULL, NULL, 0); //separator 
	MENU_AddItem(_VolumeMenu, _VolAttentionMenu, "Attention", 0);
	MENU_AddItem(_VolumeMenu, _VolDetailMenu, "Detail Volume  [100]", WM_VOLUME_DETAIL);
	MENU_AddItem(_VolumeMenu, _VolStepMenu, "Step Setting", WM_VOLUME_STEP);
	MENU_AddItem(_VolumeMenu, NULL, "Volume ::10 (Min)\tPageDown", WM_VOLUME_1);
	MENU_AddItem(_VolumeMenu, NULL, "Volume ::::20", WM_VOLUME_2);
	MENU_AddItem(_VolumeMenu, NULL, "Volume ::::::30", WM_VOLUME_3);
	MENU_AddItem(_VolumeMenu, NULL, "Volume ::::::::40", WM_VOLUME_4);
	MENU_AddItem(_VolumeMenu, NULL, "Volume ::::::::::50", WM_VOLUME_5);
	MENU_AddItem(_VolumeMenu, NULL, "Volume ::::::::::::60", WM_VOLUME_6);
	MENU_AddItem(_VolumeMenu, NULL, "Volume ::::::::::::::70", WM_VOLUME_7);
	MENU_AddItem(_VolumeMenu, NULL, "Volume ::::::::::::::::80", WM_VOLUME_8);
	MENU_AddItem(_VolumeMenu, NULL, "Volume ::::::::::::::::::90", WM_VOLUME_9);
	MENU_AddItem(_VolumeMenu, NULL, "Volume ::::::::::::::::::::100 (CD-Game Max)", WM_VOLUME_10);
	MENU_AddItem(_VolumeMenu, NULL, "Volume ::::::::::::::::::::::110", WM_VOLUME_11);
	MENU_AddItem(_VolumeMenu, NULL, "Volume ::::::::::::::::::::::::120 (Hu-Card Max)\tPageUp", WM_VOLUME_12);
	MENU_AddItem(_VolumeMenu, NULL, "Use Windows Default", WM_VOLUME_DEFAULT);
	MENU_AddItem(_VolumeMenu, NULL, NULL, 0); //separator 
	MENU_AddItem(_VolumeMenu, _TempVolMenu, "&Temporary Volume  [Norm]", WM_VOLUME_TEMP);

	MENU_AddItem(_InfoMenu, NULL, "&Version", WM_INFO_VERSION);//Kitao追加
	MENU_AddItem(_InfoMenu, NULL, NULL, 0); //separator 
	MENU_AddItem(_InfoMenu, NULL, "&Homepage", WM_INFO_HOMEPAGE);//Kitao追加
	MENU_AddItem(_InfoMenu, NULL, NULL, 0); //separator 
	MENU_AddItem(_InfoMenu, NULL, "&Readme.txt", WM_INFO_README);//Kitao追加
	MENU_AddItem(_InfoMenu, NULL, "Manual-&Japanese", WM_INFO_MANUJAPANESE);//Kitao追加
	MENU_AddItem(_InfoMenu, NULL, "Manual-&English", WM_INFO_MANUENGLISH);//Kitao追加
	MENU_AddItem(_InfoMenu, NULL, NULL, 0); //separator 
	MENU_AddItem(_InfoMenu, NULL, "&PC Performance Test", WM_INFO_TESTDELAY);//Kitao追加
	MENU_AddItem(_InfoMenu, NULL, "Show &FPS", WM_INFO_SHOWFPS);//Kitao追加。v1.50

	// add sub^3 menu
//	MENU_AddItem(_AudioMenu, _SampleRateMenu, "&Sample Rate", 0); Kitaoカット

	//Kitao追加。リセットせずにCDを入れ替え、または使用するCDドライブを変更するメニュー。v1.00
	for (i = a-1; i >= 0 ; i--)
	{
		sprintf(dn, "%c:", (int)(CDIF_GetDriveLetters(i)));
		switch (i)
		{
			case 9: MENU_AddItem(_CDChangeNRMenu, NULL, dn, WM_CHANGE_CD9); break;
			case 8: MENU_AddItem(_CDChangeNRMenu, NULL, dn, WM_CHANGE_CD8); break;
			case 7: MENU_AddItem(_CDChangeNRMenu, NULL, dn, WM_CHANGE_CD7); break;
			case 6: MENU_AddItem(_CDChangeNRMenu, NULL, dn, WM_CHANGE_CD6); break;
			case 5: MENU_AddItem(_CDChangeNRMenu, NULL, dn, WM_CHANGE_CD5); break;
			case 4: MENU_AddItem(_CDChangeNRMenu, NULL, dn, WM_CHANGE_CD4); break;
			case 3: MENU_AddItem(_CDChangeNRMenu, NULL, dn, WM_CHANGE_CD3); break;
			case 2: MENU_AddItem(_CDChangeNRMenu, NULL, dn, WM_CHANGE_CD2); break;
			case 1: MENU_AddItem(_CDChangeNRMenu, NULL, dn, WM_CHANGE_CD1); break;
			case 0: MENU_AddItem(_CDChangeNRMenu, NULL, dn, WM_CHANGE_CD0); break;
		}
	}
	MENU_AddItem(_CDChangeNRMenu, NULL, NULL, 0); //separator
	sprintf(dn, "&Current( %c: )", (int)(CDIF_GetDriveLetters(CDIF_GetDeviceInUse())));
	MENU_AddItem(_CDChangeNRMenu, NULL, dn, WM_CHANGE_CDC);

	//Kitao追加。別のシステムカードをセットするメニュー。v1.49
	MENU_AddItem(_OtherSyscardMenu, NULL, "Set Other SystemCard image 2...", WM_CD_SETSYSCARD2);
	MENU_AddItem(_OtherSyscardMenu, NULL, "Set Other SystemCard image 1...", WM_CD_SETSYSCARD1);

	//Kitao追加。Ootake起動時の"Fast Seek"設定
	MENU_AddItem(_CDROMMenu, NULL, "Start Previous", WM_STARTFASTSEEK_PRE);
	MENU_AddItem(_CDROMMenu, NULL, "Start NormalSeek", WM_STARTFASTSEEK_OFF);
	MENU_AddItem(_CDROMMenu, NULL, "Start FastSeek (Default)", WM_STARTFASTSEEK_ON);
	MENU_AddItem(_CDROMMenu, NULL, NULL, 0); //separator 
	//Kitao追加。Ootake起動時の"Fast CD"設定
	MENU_AddItem(_CDROMMenu, NULL, "Start Previous", WM_STARTFASTCD_PRE);
	MENU_AddItem(_CDROMMenu, NULL, "Start Real(Slow) CD", WM_STARTFASTCD_OFF);
	MENU_AddItem(_CDROMMenu, NULL, "Start Fast CD (Default)", WM_STARTFASTCD_ON);

 	/* 88200[Hz] and 96000[Hz] are not supported by the APU */
/*	MENU_AddItem(_SampleRateMenu, NULL, "96000 [Hz]", WM_AUDIO_SR96000);
	MENU_AddItem(_SampleRateMenu, NULL, "88200 [Hz]", WM_AUDIO_SR88200);
	MENU_AddItem(_SampleRateMenu, NULL, "64000 [Hz]", WM_AUDIO_SR64000);
	MENU_AddItem(_SampleRateMenu, NULL, "48000 [Hz]", WM_AUDIO_SR48000);
	MENU_AddItem(_SampleRateMenu, NULL, "44100 [Hz]", WM_AUDIO_SR44100);
	MENU_AddItem(_SampleRateMenu, NULL, "32000 [Hz]", WM_AUDIO_SR32000);
	MENU_AddItem(_SampleRateMenu, NULL, "22050 [Hz]", WM_AUDIO_SR22050);
	MENU_AddItem(_SampleRateMenu, NULL, "11025 [Hz]", WM_AUDIO_SR11025); */ //Kitaoカット。CD-DA再生の速度アップのため44100固定に。

	//Kitao追加。サウンドボード(DirectSound)初期化時の設定を変更する
	MENU_AddItem(_AudioMenu, NULL, "Hard Buffer && Soft Mixing", WM_AUDIO_INI2);
	MENU_AddItem(_AudioMenu, NULL, "Soft Buffer && Soft Mixing (Default)", WM_AUDIO_INI1);
	MENU_AddItem(_AudioMenu, NULL, NULL, 0); //separator 

	//Kitao追加。サウンドボード(DirectSound)初期化時の設定を変更する
	MENU_AddItem(_AudioMenu, NULL, "Mono (Gradius,Salamander,Parodius)", WM_AUDIO_MONO);
	MENU_AddItem(_AudioMenu, NULL, "Auto Stereo (Default)", WM_AUDIO_STEREO);
	MENU_AddItem(_AudioMenu, NULL, NULL, 0); //separator 

	MENU_AddItem(_AudioMenu, NULL, "Buffer3072", WM_AUDIO_SB3072);
	MENU_AddItem(_AudioMenu, NULL, "Buffer2560", WM_AUDIO_SB2560);
	MENU_AddItem(_AudioMenu, NULL, "Buffer2304", WM_AUDIO_SB2304);
	MENU_AddItem(_AudioMenu, NULL, "Buffer2176", WM_AUDIO_SB2176);
	MENU_AddItem(_AudioMenu, NULL, "Buffer2048", WM_AUDIO_SB2048);
	MENU_AddItem(_AudioMenu, NULL, "Buffer1792", WM_AUDIO_SB1792);
	MENU_AddItem(_AudioMenu, NULL, "Buffer1664 (Default)", WM_AUDIO_SB1664);
	MENU_AddItem(_AudioMenu, NULL, "Buffer1536", WM_AUDIO_SB1536);
	MENU_AddItem(_AudioMenu, NULL, "Buffer1408", WM_AUDIO_SB1408);
	MENU_AddItem(_AudioMenu, NULL, "Buffer1280", WM_AUDIO_SB1280);
	MENU_AddItem(_AudioMenu, NULL, "Buffer1152", WM_AUDIO_SB1152);
	MENU_AddItem(_AudioMenu, NULL, "Buffer1024", WM_AUDIO_SB1024);

	//Kitao追加。Ootake起動時の"Perform SpriteLimit"設定
	MENU_AddItem(_SetCpuMenu, NULL, "Start Previous", WM_STARTSPRITE_PRE);
	MENU_AddItem(_SetCpuMenu, NULL, "Start Perform SpriteLimit", WM_STARTSPRITE_ON);
	MENU_AddItem(_SetCpuMenu, NULL, "Start Draw AllSprites (Default)", WM_STARTSPRITE_OFF);
	MENU_AddItem(_SetCpuMenu, NULL, NULL, 0);	// separator 
	MENU_AddItem(_SetCpuMenu, NULL, "Auto Reduce Bug ( Shin Megami Tensei )", WM_AUTO_MEGATEN);
	MENU_AddItem(_SetCpuMenu, NULL, "Auto Improve Graphics && Sound ( Gradius II )", WM_AUTO_GRADIUS2);

	//ジョイパッドの左右（または上下）同時押しを許可するかどうかの設定。v2.59追加
	MENU_AddItem(_SetInputMenu, NULL, "Check Pad LR(or UD) Simultaneously Push (Default)", WM_INPUT_CHECKPAD_LR);

//	MENU_AddItem(_CpuMenu, NULL, "AdvanceFrame 100", WM_TRACE_100_FRAME);
//	MENU_AddItem(_CpuMenu, NULL, "AdvanceFrame 10", WM_TRACE_10_FRAME);
//	MENU_AddItem(_CpuMenu, NULL, NULL, 0);	// separator 
	MENU_AddItem(_CpuMenu, NULL, "Perform SpriteLimit", WM_SPRITEOVER);//Kitao追加
	MENU_AddItem(_CpuMenu, NULL, NULL, 0);	// separator 
	MENU_AddItem(_CpuMenu, NULL, "Super Grafx Mode", WM_SUPERGRAFX);//Kitao追加
	MENU_AddItem(_CpuMenu, NULL, NULL, 0);	// separator 
	MENU_AddItem(_CpuMenu, NULL, "BG2 ( for SG ) Layer", WM_LAYER_BG2);//Kitao追加
	MENU_AddItem(_CpuMenu, NULL, "Sprite2 ( for SG ) Layer", WM_LAYER_SPRITE2);//Kitao追加
	MENU_AddItem(_CpuMenu, NULL, "BG Layer", WM_LAYER_BG);//Kitao追加
	MENU_AddItem(_CpuMenu, NULL, "Sprite Layer", WM_LAYER_SPRITE);//Kitao追加
	MENU_AddItem(_CpuMenu, NULL, NULL, 0);	// separator 
	MENU_AddItem(_CpuMenu, NULL, "Invalidate \"CD Install\"", WM_INVALIDATE_CDINST);//Kitao追加
	MENU_AddItem(_CpuMenu, NULL, NULL, 0);	// separator 
	MENU_AddItem(_CpuMenu, NULL, "MoreLate RasterTiming", WM_RASTERTIMING_MLATE);//Kitao追加
	MENU_AddItem(_CpuMenu, NULL, "Late RasterTiming", WM_RASTERTIMING_LATE);//Kitao追加
	MENU_AddItem(_CpuMenu, NULL, "Normal RasterTiming\tBS", WM_RASTERTIMING_MIDDLE);//Kitao追加
	MENU_AddItem(_CpuMenu, NULL, "Early RasterTiming", WM_RASTERTIMING_EARLY);//Kitao追加
	MENU_AddItem(_CpuMenu, NULL, "MoreEarly RasterTiming", WM_RASTERTIMING_MEARLY);//Kitao追加
	MENU_AddItem(_CpuMenu, NULL, NULL, 0);	// separator 
	MENU_AddItem(_CpuMenu, NULL, "Configure Pause Button...", WM_PAUSE_BUTTON);//v2.49
	MENU_AddItem(_CpuMenu, NULL, "Advance One Frame\tF2", WM_TRACE_1_FRAME);
	MENU_AddItem(_CpuMenu, NULL, "Reset\tF1", WM_RESET_EMULATOR);
	MENU_AddItem(_CpuMenu, NULL, "Run\tEsc", WM_RUN_EMULATOR);//Kitao更新
	MENU_AddItem(_CpuMenu, NULL, NULL, 0);	// separator 
	MENU_AddItem(_CpuMenu, _WriteAttentionMenu, "Attention", 0);//Kitao追加
	MENU_AddItem(_CpuMenu, NULL, "Write Memory...", WM_WRITE_MEMORY);//Kitao追加

	//Kitao追加
	MENU_AddItem(_WriteAttentionMenu, NULL, "Read Attention of this \"Write Memory\" Menu", WM_ABOUT_WRITEMEM);//Kitao追加

	//Kitao更新
	MENU_AddItem(_ScreenMenu, NULL, "V-Sync Timing Adjust (for Old PC)", WM_SCREEN_SYNC_ADJUST);//v2.65追加
	MENU_AddItem(_ScreenMenu, NULL, NULL, 0);	// separator 
	MENU_AddItem(_ScreenMenu, NULL, "Non-V-Sync", WM_SCREEN_SYNC_NON);//v2.65追加
	MENU_AddItem(_ScreenMenu, NULL, "Windows Auto V-Sync only FullScreen", WM_SCREEN_SYNC_WINDOWSF);//v2.65追加
	MENU_AddItem(_ScreenMenu, NULL, "Windows Auto V-Sync (Time-Lag exists)", WM_SCREEN_SYNC_WINDOWS);//v2.65追加
	MENU_AddItem(_ScreenMenu, NULL, "V-Sync ( 59-61Hz ) (No Time-Lag)", WM_SCREEN_SYNC_VBLANK);
	MENU_AddItem(_ScreenMenu, NULL, NULL, 0);	// separator 
	MENU_AddItem(_ScreenMenu, NULL, "Window Activated Non-automatically", WM_SCREEN_NONACTIVATE);//Kitao追加
	MENU_AddItem(_ScreenMenu, NULL, "Window Always Activated (Default)", WM_SCREEN_ACTIVATE);//Kitao追加
	MENU_AddItem(_ScreenMenu, NULL, NULL, 0);	// separator 
	MENU_AddItem(_ScreenMenu, NULL, "Un-Pause Window Inactive (Default)", WM_SCREEN_UNPAUSE);//Kitao追加
	MENU_AddItem(_ScreenMenu, NULL, NULL, 0);	// separator 
	MENU_AddItem(_ScreenMenu, NULL, "Use Black Belt when Hide Overscan", WM_SCREEN_OVERBLACK);//Kitao追加
	MENU_AddItem(_ScreenMenu, _StartOverscanMenu, "Start Overscan Customize", 0);//Kitao追加
	MENU_AddItem(_ScreenMenu, NULL, NULL, 0);	// separator 
	MENU_AddItem(_ScreenMenu, NULL, "Start FullScreen mode", WM_SCREEN_STARTFULL);//Kitao追加
	MENU_AddItem(_ScreenMenu, NULL, "Start Window mode", WM_SCREEN_STARTWINDOW);//Kitao追加
	MENU_AddItem(_ScreenMenu, NULL, NULL, 0);	// separator 
	MENU_AddItem(_ScreenMenu, _DirectDrawMenu, "Setting DirectDraw", WM_SCREEN_SETDIRECTDRAW);//Kitao追加
	MENU_AddItem(_ScreenMenu, _Direct3DMenu, "Setting Direct3D", WM_SCREEN_SETDIRECT3D);//Kitao追加
	MENU_AddItem(_ScreenMenu, NULL, NULL, 0);	// separator 
	MENU_AddItem(_ScreenMenu, NULL, "DirectDraw (Old)", WM_SCREEN_DIRECTDRAW);//Kitao追加
	MENU_AddItem(_ScreenMenu, NULL, "Direct3D (Default)", WM_SCREEN_DIRECT3D);//Kitao追加
	MENU_AddItem(_ScreenMenu, NULL, NULL, 0);	// separator 
	MENU_AddItem(_ScreenMenu, NULL, "Non-Scanlined (Fast)", WM_SCREEN_NONSCANLINED);
	MENU_AddItem(_ScreenMenu, NULL, "Horizontal Scanlined", WM_SCREEN_HRSCANLINED);
	MENU_AddItem(_ScreenMenu, NULL, "TV Mode (Fast)", WM_SCREEN_STARTTV);//Kitao追加
	MENU_AddItem(_ScreenMenu, NULL, "Special Scanlined (Sharp)", WM_SCREEN_SPSCANLINED);//Kitao追加

	//Kitao追加
	MENU_AddItem(_ScanlineMenu, NULL, "Non-Scanlined (Fast)", WM_SCREEN_NONSCANLINED);
	MENU_AddItem(_ScanlineMenu, NULL, "Horizontal Scanlined", WM_SCREEN_HRSCANLINED);
	MENU_AddItem(_ScanlineMenu, NULL, "TV Mode (Fast)", WM_SCREEN_STARTTV);//Kitao追加
	MENU_AddItem(_ScanlineMenu, NULL, "Special Scanlined (Sharp)", WM_SCREEN_SPSCANLINED);//Kitao追加
	MENU_AddItem(_DensityMenu, NULL, "Optimize Gamma (Default)", WM_SCREEN_SCANGAMMA);//Kitao追加
	MENU_AddItem(_DensityMenu, NULL, NULL, 0);	// separator 
	MENU_AddItem(_DensityMenu, NULL, "Scanline Density 80%", WM_SCREEN_SCANDENSITY80);//Kitao追加
	MENU_AddItem(_DensityMenu, NULL, "Scanline Density 70% (Default)", WM_SCREEN_SCANDENSITY70);//Kitao追加
	MENU_AddItem(_DensityMenu, NULL, "Scanline Density 60%", WM_SCREEN_SCANDENSITY60);//Kitao追加
	MENU_AddItem(_DensityMenu, NULL, "Scanline Density 50%", WM_SCREEN_SCANDENSITY50);//Kitao追加
	MENU_AddItem(_DensityMenu, NULL, "Scanline Density 40%", WM_SCREEN_SCANDENSITY40);//Kitao追加
	MENU_AddItem(_DensityMenu, NULL, "Scanline Density 30%", WM_SCREEN_SCANDENSITY30);//Kitao追加
	MENU_AddItem(_DensityMenu, NULL, "Scanline Density 20%", WM_SCREEN_SCANDENSITY20);//Kitao追加
	MENU_AddItem(_DensityMenu, NULL, "Scanline Density 10%", WM_SCREEN_SCANDENSITY10);//Kitao追加
	MENU_AddItem(_DensityMenu, NULL, "Scanline Density 0%", WM_SCREEN_SCANDENSITY0);//Kitao追加
	MENU_AddItem(_GammaMenu, NULL, "Gamma 1.44", WM_SCREEN_GAMMA7);
	MENU_AddItem(_GammaMenu, NULL, "Gamma 1.39", WM_SCREEN_GAMMA6);
	MENU_AddItem(_GammaMenu, NULL, "Gamma 1.34 (Default)", WM_SCREEN_GAMMA5);
	MENU_AddItem(_GammaMenu, NULL, "Gamma 1.24", WM_SCREEN_GAMMA4);
	MENU_AddItem(_GammaMenu, NULL, "Gamma 1.14", WM_SCREEN_GAMMA3);
	MENU_AddItem(_GammaMenu, NULL, "Gamma 1.04", WM_SCREEN_GAMMA2);
	MENU_AddItem(_GammaMenu, NULL, "Gamma 0.94", WM_SCREEN_GAMMA1);
	MENU_AddItem(_BrightnessMenu, NULL, "Brightness +30", WM_SCREEN_BRIGHT9);
	MENU_AddItem(_BrightnessMenu, NULL, "Brightness +20", WM_SCREEN_BRIGHT8);
	MENU_AddItem(_BrightnessMenu, NULL, "Brightness +10", WM_SCREEN_BRIGHT7);
	MENU_AddItem(_BrightnessMenu, NULL, "Brightness +5", WM_SCREEN_BRIGHT6);
	MENU_AddItem(_BrightnessMenu, NULL, "Brightness +3", WM_SCREEN_BRIGHT5);
	MENU_AddItem(_BrightnessMenu, NULL, "Brightness +2", WM_SCREEN_BRIGHT4);
	MENU_AddItem(_BrightnessMenu, NULL, "Brightness +1", WM_SCREEN_BRIGHT3);
	MENU_AddItem(_BrightnessMenu, NULL, "Brightness  0 (Default)", WM_SCREEN_BRIGHT2);
	MENU_AddItem(_BrightnessMenu, NULL, "Brightness -1", WM_SCREEN_BRIGHT1);

	//Kitao追加
	MENU_AddItem(_ResolutionMenu, NULL, "FullScreen Resolution Auto Change", WM_SCREEN_FULLSCREENCSA);
	MENU_AddItem(_ResolutionMenu, NULL, NULL, 0);	// separator
	MENU_AddItem(_ResolutionMenu, NULL, "Set FullScreen Resolution <3>...", WM_SCREEN_FULLSCREENCS3);
	MENU_AddItem(_ResolutionMenu, NULL, "Set FullScreen Resolution <2>...", WM_SCREEN_FULLSCREENCS2);
	MENU_AddItem(_ResolutionMenu, NULL, "Set FullScreen Resolution <1>...", WM_SCREEN_FULLSCREENCS1);

	//Kitao追加
	MENU_AddItem(_OverscanMenu, NULL, "Overscan Start Setting", WM_SCREEN_OVERSTART);
	MENU_AddItem(_OverscanMenu, NULL, NULL, 0);	// separator
	MENU_AddItem(_OverscanMenu, NULL, "Show Overscan None", WM_SCREEN_OVERNONELR);
	MENU_AddItem(_OverscanMenu, NULL, "Show Overscan Left&&Right", WM_SCREEN_OVERLR);
	MENU_AddItem(_OverscanMenu, NULL, NULL, 0);	// separator
	MENU_AddItem(_OverscanMenu, NULL, "Overscan Height 1dot", WM_SCREEN_OVERHEIGHT1);
	MENU_AddItem(_OverscanMenu, NULL, "Overscan Height 2dot", WM_SCREEN_OVERHEIGHT2);
	MENU_AddItem(_OverscanMenu, NULL, "Overscan Height 4dot", WM_SCREEN_OVERHEIGHT4);
	MENU_AddItem(_OverscanMenu, NULL, "Overscan Height 6dot", WM_SCREEN_OVERHEIGHT6);
	MENU_AddItem(_OverscanMenu, NULL, "Overscan Height 7dot", WM_SCREEN_OVERHEIGHT7);
	MENU_AddItem(_OverscanMenu, NULL, "Overscan Height 8dot", WM_SCREEN_OVERHEIGHT8);
	MENU_AddItem(_OverscanMenu, NULL, NULL, 0);	// separator
	MENU_AddItem(_OverscanMenu, NULL, "Show Overscan None", WM_SCREEN_OVERNONETB);
	MENU_AddItem(_OverscanMenu, NULL, "Show Overscan Bottom", WM_SCREEN_OVERBOTTOM);
	MENU_AddItem(_OverscanMenu, NULL, "Show Overscan Top", WM_SCREEN_OVERTOP);
	MENU_AddItem(_OverscanMenu, NULL, "Show Overscan Top&&Bottom", WM_SCREEN_OVERTB);

	//Kitao追加
	MENU_AddItem(_StartOverscanMenu, NULL, "Show Overscan None", WM_SCREEN_SOVERNONELR);
	MENU_AddItem(_StartOverscanMenu, NULL, "Show Overscan Left&&Right", WM_SCREEN_SOVERLR);
	MENU_AddItem(_StartOverscanMenu, NULL, NULL, 0);	// separator
	MENU_AddItem(_StartOverscanMenu, NULL, "Overscan Height 1dot", WM_SCREEN_SOVERHEIGHT1);
	MENU_AddItem(_StartOverscanMenu, NULL, "Overscan Height 2dot", WM_SCREEN_SOVERHEIGHT2);
	MENU_AddItem(_StartOverscanMenu, NULL, "Overscan Height 4dot", WM_SCREEN_SOVERHEIGHT4);
	MENU_AddItem(_StartOverscanMenu, NULL, "Overscan Height 6dot", WM_SCREEN_SOVERHEIGHT6);
	MENU_AddItem(_StartOverscanMenu, NULL, "Overscan Height 7dot", WM_SCREEN_SOVERHEIGHT7);
	MENU_AddItem(_StartOverscanMenu, NULL, "Overscan Height 8dot", WM_SCREEN_SOVERHEIGHT8);
	MENU_AddItem(_StartOverscanMenu, NULL, NULL, 0);	// separator
	MENU_AddItem(_StartOverscanMenu, NULL, "Show Overscan None", WM_SCREEN_SOVERNONETB);
	MENU_AddItem(_StartOverscanMenu, NULL, "Show Overscan Bottom", WM_SCREEN_SOVERBOTTOM);
	MENU_AddItem(_StartOverscanMenu, NULL, "Show Overscan Top", WM_SCREEN_SOVERTOP);
	MENU_AddItem(_StartOverscanMenu, NULL, "Show Overscan Top&&Bottom", WM_SCREEN_SOVERTB);

	//Kitao追加
	MENU_AddItem(_Direct3DMenu, NULL, "TV<->RGB switch is [Ctrl]+[F11] key", WM_SCREEN_USE3D_HELP);
	MENU_AddItem(_Direct3DMenu, NULL, NULL, 0);	// separator
	MENU_AddItem(_Direct3DMenu, NULL, "RGB Use NONE (Most Sharp)", WM_SCREEN_USE3DR_NONE);
	MENU_AddItem(_Direct3DMenu, NULL, "RGB Use POINT Effect (Sharp)", WM_SCREEN_USE3DR_POINT);
	MENU_AddItem(_Direct3DMenu, NULL, "RGB Use LINEAR Effect (Default)", WM_SCREEN_USE3DR_LINEAR);
	MENU_AddItem(_Direct3DMenu, NULL, NULL, 0);	// separator
	MENU_AddItem(_Direct3DMenu, NULL, "TV Use NONE (Most Sharp)", WM_SCREEN_USE3DT_NONE);
	MENU_AddItem(_Direct3DMenu, NULL, "TV Use POINT Effect (Default)", WM_SCREEN_USE3DT_POINT);
	MENU_AddItem(_Direct3DMenu, NULL, "TV Use LINEAR Effect (Smooth)", WM_SCREEN_USE3DT_LINEAR);

	//Kitao追加
	MENU_AddItem(_DirectDrawMenu, NULL, "FullScreen 32bit Color", WM_SCREEN_FULL32BITCOLOR);
	MENU_AddItem(_DirectDrawMenu, NULL, "FullScreen 16bit Color (Fast)", WM_SCREEN_FULL16BITCOLOR);
	MENU_AddItem(_DirectDrawMenu, NULL, NULL, 0);	// separator 
	MENU_AddItem(_DirectDrawMenu, NULL, "Use System Memory Only Window", WM_SCREEN_USE_SYSTEMMEMW);
	MENU_AddItem(_DirectDrawMenu, NULL, "Use System Memory (Sharp&&Slow)", WM_SCREEN_USE_SYSTEMMEM);
	MENU_AddItem(_DirectDrawMenu, NULL, "Use VideoCard Memory (Default)", WM_SCREEN_USE_VIDEOMEM);

	//Kitao追加
	MENU_AddItem(_TempVolMenu, NULL, "&Mute\tF8", WM_VOLUME_MUTE);
	MENU_AddItem(_TempVolMenu, NULL, "&Quarter", WM_VOLUME_QUARTER);
	MENU_AddItem(_TempVolMenu, NULL, "&Half", WM_VOLUME_HALF);
	MENU_AddItem(_TempVolMenu, NULL, "&3Quarters", WM_VOLUME_3QUARTERS);
	MENU_AddItem(_TempVolMenu, NULL, "&Normal", WM_VOLUME_NORMAL);
	MENU_AddItem(_VolAttentionMenu, NULL, "Start \"Windows Volume Control\" (for XP/9x)", WM_VOLUME_CONTROL);
	MENU_AddItem(_VolAttentionMenu, NULL, NULL, 0);	// separator
	MENU_AddItem(_VolAttentionMenu, NULL, "Read Attention of this Volume Menu", WM_VOLUME_ATTENTION);
	MENU_AddItem(_VolStepMenu, NULL, "1", WM_VOLUME_STEP1);
	MENU_AddItem(_VolStepMenu, NULL, "2", WM_VOLUME_STEP2);
	MENU_AddItem(_VolStepMenu, NULL, "3", WM_VOLUME_STEP3);
	MENU_AddItem(_VolStepMenu, NULL, "4 (Default)", WM_VOLUME_STEP4);
	MENU_AddItem(_VolStepMenu, NULL, "5", WM_VOLUME_STEP5);
	MENU_AddItem(_VolStepMenu, NULL, "6" , WM_VOLUME_STEP6);
	MENU_AddItem(_VolStepMenu, NULL, "8" , WM_VOLUME_STEP8);
	MENU_AddItem(_VolStepMenu, NULL, "10", WM_VOLUME_STEP10);
	MENU_AddItem(_VolDetailMenu, NULL, "Detail Volume Down\t  Ctrl+PageDown", WM_VOLUME_DETAILDN);
	MENU_AddItem(_VolDetailMenu, NULL, "Detail Volume Up\t  Ctrl+PageUp", WM_VOLUME_DETAILUP);
	MENU_AddItem(_MuteMenu, NULL, "Unmute PSG Channel All", WM_VOLUME_MUTEU);
	MENU_AddItem(_MuteMenu, NULL, "Mute PSG Channel All", WM_VOLUME_MUTEA);
	MENU_AddItem(_MuteMenu, NULL, NULL, 0);	// separator
	MENU_AddItem(_MuteMenu, NULL, "Mute PSG Channel 6", WM_VOLUME_MUTE6);
	MENU_AddItem(_MuteMenu, NULL, "Mute PSG Channel 5", WM_VOLUME_MUTE5);
	MENU_AddItem(_MuteMenu, NULL, "Mute PSG Channel 4", WM_VOLUME_MUTE4);
	MENU_AddItem(_MuteMenu, NULL, "Mute PSG Channel 3", WM_VOLUME_MUTE3);
	MENU_AddItem(_MuteMenu, NULL, "Mute PSG Channel 2", WM_VOLUME_MUTE2);
	MENU_AddItem(_MuteMenu, NULL, "Mute PSG Channel 1", WM_VOLUME_MUTE1);

	//1.61追加
	MENU_AddItem(_CpuSpeedMenu, NULL, "Set Present Speed to [Delete]key",	WM_SPEED_CSET);
	MENU_AddItem(_CpuSpeedMenu, NULL, NULL, 0);	// separator
	MENU_AddItem(_CpuSpeedMenu, NULL, "CPU Speed x0.50   \tCtrl+Del", WM_SPEED_M2);
	MENU_AddItem(_CpuSpeedMenu, NULL, "CPU Speed x0.75", WM_SPEED_M1);
	MENU_AddItem(_CpuSpeedMenu, NULL, "CPU Speed x1.25", WM_SPEED_P1);
	MENU_AddItem(_CpuSpeedMenu, NULL, "CPU Speed x1.33", WM_SPEED_P2);
	MENU_AddItem(_CpuSpeedMenu, NULL, "CPU Speed x1.50", WM_SPEED_P3);
	MENU_AddItem(_CpuSpeedMenu, NULL, "CPU Speed x2.00", WM_SPEED_P4);
	MENU_AddItem(_CpuSpeedMenu, NULL, "CPU Speed x2.50", WM_SPEED_P5);
	MENU_AddItem(_CpuSpeedMenu, NULL, "CPU Speed x3.00", WM_SPEED_P6);
	MENU_AddItem(_CpuSpeedMenu, NULL, "CPU Speed TURBOx1", WM_SPEED_T1);
	MENU_AddItem(_CpuSpeedMenu, NULL, "CPU Speed TURBOx2", WM_SPEED_T2);
	MENU_AddItem(_CpuSpeedMenu, NULL, "CPU Speed TURBOx3   \tShift+Del", WM_SPEED_T3);
	MENU_AddItem(_CpuSpeedMenu, NULL, "Normal CPU Speed   \tBS", WM_SPEED_P0);

	MENU_AddItem(_InputMenu, NULL, "Keyboard Background Ok", WM_INPUT_CONFIGURE_KEYBG); //v1.41追加
	MENU_AddItem(_InputMenu, NULL, "Joypad Background Ok", WM_INPUT_CONFIGURE_JOYBG); //v1.41追加
	MENU_AddItem(_InputMenu, NULL, NULL, 0);	// separator
	MENU_AddItem(_InputMenu, _FuncButtonMenu, "Configure Function Button", 0); //v2.38追加
	MENU_AddItem(_InputMenu, NULL, NULL, 0);	// separator
	MENU_AddItem(_InputMenu, NULL, "Initialize Pad #1-#5", WM_INPUT_CONFIGURE_INIT); //Kitao追加
	MENU_AddItem(_InputMenu, _TurboButtonMenu, "Configure TurboButton", 0); //Kitao追加
	MENU_AddItem(_InputMenu, NULL, "Configure Pad #5...", WM_INPUT_CONFIGURE_PAD5);
	MENU_AddItem(_InputMenu, NULL, "Configure Pad #4...", WM_INPUT_CONFIGURE_PAD4);
	MENU_AddItem(_InputMenu, NULL, "Configure Pad #3...", WM_INPUT_CONFIGURE_PAD3);
	MENU_AddItem(_InputMenu, NULL, "Configure Pad #2...", WM_INPUT_CONFIGURE_PAD2);
	MENU_AddItem(_InputMenu, NULL, "Configure Pad #1...", WM_INPUT_CONFIGURE_PAD1);
	MENU_AddItem(_InputMenu, NULL, NULL, 0);	// separator 
	MENU_AddItem(_InputMenu, NULL, "Swap  I && II", WM_INPUT_SWAP_IANDII); //Kitao追加。レミングスをやりやすいように。
	MENU_AddItem(_InputMenu, NULL, "Swap SEL&&RUN\t8", WM_INPUT_SWAP_SELRUN); //Kitao追加。バルンバやフォゴットンをやりやすいように。
	MENU_AddItem(_InputMenu, NULL, NULL, 0);	// separator 
	MENU_AddItem(_InputMenu, NULL, "MB128", WM_INPUT_MB128);
	MENU_AddItem(_InputMenu, NULL, "Multi-Tap", WM_INPUT_MULTI_TAP);
	MENU_AddItem(_InputMenu, NULL, NULL, 0);	// separator 
	MENU_AddItem(_InputMenu, NULL, "Mouse", WM_INPUT_MOUSE);
	MENU_AddItem(_InputMenu, NULL, "6-Button Pad", WM_INPUT_SIX_BUTTON_PAD);
	MENU_AddItem(_InputMenu, NULL, "3-Button Pad", WM_INPUT_THR_BUTTON_PAD);
	MENU_AddItem(_InputMenu, NULL, "2-Button Pad", WM_INPUT_TWO_BUTTON_PAD);
	MENU_AddItem(_InputMenu, NULL, NULL, 0);	// separator 
	MENU_AddItem(_InputMenu, NULL, "TurboSpeed Off\t7", WM_INPUT_TURBO_OFF);
	MENU_AddItem(_InputMenu, NULL, "TurboSpeed Low\t6", WM_INPUT_TURBO_LOW);
	MENU_AddItem(_InputMenu, NULL, "TurboSpeed Middle\t5", WM_INPUT_TURBO_MIDDLE);
	MENU_AddItem(_InputMenu, NULL, "TurboSpeed High\t4", WM_INPUT_TURBO_HIGH);
	MENU_AddItem(_InputMenu, NULL, NULL, 0);	// separator 
	MENU_AddItem(_InputMenu, NULL, "TurboButton RUN\t3", WM_INPUT_TURBO_RUN);
	MENU_AddItem(_InputMenu, NULL, "TurboButton II\t2", WM_INPUT_TURBO_2);
	MENU_AddItem(_InputMenu, NULL, "TurboButton I\t1", WM_INPUT_TURBO_1);

	//Kitao追加
	MENU_AddItem(_TurboButtonMenu, NULL, "TurboButton Pad #5...", WM_INPUT_CONFIGURE_TB5);
	MENU_AddItem(_TurboButtonMenu, NULL, "TurboButton Pad #4...", WM_INPUT_CONFIGURE_TB4);
	MENU_AddItem(_TurboButtonMenu, NULL, "TurboButton Pad #3...", WM_INPUT_CONFIGURE_TB3);
	MENU_AddItem(_TurboButtonMenu, NULL, "TurboButton Pad #2...", WM_INPUT_CONFIGURE_TB2);
	MENU_AddItem(_TurboButtonMenu, NULL, "TurboButton Pad #1...", WM_INPUT_CONFIGURE_TB1);

	//Kitao追加。v2.38
	MENU_AddItem(_FuncButtonMenu, NULL, "Configure Function Save&&Load Buttons...", WM_INPUT_FB_SAVELOAD);
	MENU_AddItem(_FuncButtonMenu, NULL, "FuncButton + [VSpeedUp] for VSpeedUp", WM_INPUT_FB_VSPEEDUP);
	MENU_AddItem(_FuncButtonMenu, NULL, "FuncButton + [RUN] for RecentGame", WM_INPUT_FB_RUN);
	MENU_AddItem(_FuncButtonMenu, NULL, "FuncButton + [SEL] for Screenshot", WM_INPUT_FB_SEL);
	MENU_AddItem(_FuncButtonMenu, NULL, "FuncButton + [ I ][ II ] for TurboButton", WM_INPUT_FB_IandII);
	MENU_AddItem(_FuncButtonMenu, NULL, "FuncButton + [U][D][L][R] for Volume", WM_INPUT_FB_CURSOR);
	MENU_AddItem(_FuncButtonMenu, NULL, NULL, 0);	// separator 
	MENU_AddItem(_FuncButtonMenu, NULL, "Configure Function Button...", WM_INPUT_FUNCTION);

	//Kitao追加
	MENU_AddItem(_RecentMenu, NULL, "&Play FullInstalled CD-Game...\tP", WM_CD_PLAYINSTALL); //v2.24追加
	MENU_AddItem(_RecentMenu, NULL, "&Open...\tO", WM_OPEN_FILE);
	MENU_AddItem(_RecentMenu, NULL, NULL, 0);//separator
	MENU_AddItem(_RecentMenu, NULL, " ",WM_RECENT_20);
	MENU_AddItem(_RecentMenu, NULL, " ",WM_RECENT_19);
	MENU_AddItem(_RecentMenu, NULL, " ",WM_RECENT_18);
	MENU_AddItem(_RecentMenu, NULL, " ",WM_RECENT_17);
	MENU_AddItem(_RecentMenu, NULL, " ",WM_RECENT_16);
	MENU_AddItem(_RecentMenu, NULL, " ",WM_RECENT_15);
	MENU_AddItem(_RecentMenu, NULL, " ",WM_RECENT_14);
	MENU_AddItem(_RecentMenu, NULL, " ",WM_RECENT_13);
	MENU_AddItem(_RecentMenu, NULL, " ",WM_RECENT_12);
	MENU_AddItem(_RecentMenu, NULL, " ",WM_RECENT_11);
	MENU_AddItem(_RecentMenu, NULL, NULL, 0);//separator
	MENU_AddItem(_RecentMenu, NULL, " ",WM_RECENT_10);
	MENU_AddItem(_RecentMenu, NULL, " ", WM_RECENT_9);
	MENU_AddItem(_RecentMenu, NULL, " ", WM_RECENT_8);
	MENU_AddItem(_RecentMenu, NULL, " ", WM_RECENT_7);
	MENU_AddItem(_RecentMenu, NULL, " ", WM_RECENT_6);
	MENU_AddItem(_RecentMenu, NULL, " ", WM_RECENT_5);
	MENU_AddItem(_RecentMenu, NULL, " ", WM_RECENT_4);
	MENU_AddItem(_RecentMenu, NULL, " ", WM_RECENT_3);
	MENU_AddItem(_RecentMenu, NULL, " ", WM_RECENT_2);
	MENU_AddItem(_RecentMenu, NULL, "          ", WM_RECENT_1);

	//Kitao追加。v1.29
	MENU_AddItem(_CaptureMenu, _WavSettingMenu, "Setting of Output WAV", 0);
	MENU_AddItem(_WavSettingMenu, NULL, "Add \"no sound part\" to begin and end (Default)", WM_OUTPUT_WAVBE);
	MENU_AddItem(_WavSettingMenu, NULL, NULL, 0);	// separator 
	MENU_AddItem(_WavSettingMenu, NULL, "Add \"no sound part\" when going to Next Tune (hes)", WM_OUTPUT_WAVNT);
	MENU_AddItem(_WavSettingMenu, NULL, NULL, 0);	// separator 
	MENU_AddItem(_WavSettingMenu, NULL, "Start at the Next Tune", WM_OUTPUT_WAVS2);
	MENU_AddItem(_WavSettingMenu, NULL, "Output WAV Start Normal", WM_OUTPUT_WAVS1);
	MENU_AddItem(_CaptureMenu, _WavAnotherMenu, "Output WAV another", 0);
	MENU_AddItem(_WavAnotherMenu, NULL, "* \"Output only with few channels\" is more High Quality.", WM_OUTPUT_WAV0);
	MENU_AddItem(_WavAnotherMenu, NULL, "Output WAV File (CDDA)", WM_OUTPUT_WAV3);
	MENU_AddItem(_WavAnotherMenu, NULL, "Output WAV File (ADPCM)", WM_OUTPUT_WAV2);
	MENU_AddItem(_WavAnotherMenu, NULL, "Output WAV File (PSG+ADPCM)", WM_OUTPUT_WAV12);
	MENU_AddItem(_WavAnotherMenu, NULL, "Output WAV File (PSG+ADPCM+CDDA)", WM_OUTPUT_WAV123);
	MENU_AddItem(_CaptureMenu, NULL, "Output &WAV File (PSG)", WM_OUTPUT_WAV1);
	MENU_AddItem(_CaptureMenu, NULL, NULL, 0);	// separator 

	//Kitao追加。v2.12
	MENU_AddItem(_CaptureMenu, NULL, "Open \"screenshot\" Folder", WM_FOLDER_SCREENSHOT);
	MENU_AddItem(_CaptureMenu, _SShotSettingMenu, "Setting of Screenshot", 0);
	MENU_AddItem(_CaptureMenu, NULL, "Output Screens&hot\tPrtScr", WM_OUTPUT_SCREENSHOT);
	MENU_AddItem(_CaptureMenu, NULL, NULL, 0);	// separator 

	//Kitao追加。v2.52
/*	MENU_AddItem(_CaptureMenu, _AviAttentionMenu, "Attention", 0);
	MENU_AddItem(_AviAttentionMenu, NULL, "Read Attention of this \"Output AVI File\" Menu", WM_ABOUT_AVI);//Kitao追加
	MENU_AddItem(_CaptureMenu, NULL, "&Output AVI File", WM_OUTPUT_AVI);
	MENU_AddItem(_CaptureMenu, NULL, NULL, 0);	// separator 
*/
	MENU_AddItem(_CaptureMenu, NULL, "Open \"record\" Folder", WM_FOLDER_GAMEPLAY);//Kitao追加
	MENU_AddItem(_CaptureMenu, _PlayRecordAnotherMenu, "Play Record another", 0);//Kitao追加。v2.15
	MENU_AddItem(_CaptureMenu, NULL, "&Play Record\tF6", WM_PLAYRECORD_GAMEPLAY);
	MENU_AddItem(_CaptureMenu, _MoveRecordMenu, "Move \"Default Record Data\"", 0);//Kitao追加。v2.15
	MENU_AddItem(_CaptureMenu, _RecordingAnotherMenu, "Start Recording another", 0);//Kitao追加。v2.15
	MENU_AddItem(_CaptureMenu, NULL, "Start &Recording\tF5", WM_RECORDING_GAMEPLAY);//Kitao更新

	MENU_AddItem(_FileMenu, NULL, "E&xit\tAlt+F4", WM_EXIT);//Kitao更新
	MENU_AddItem(_FileMenu, NULL, NULL, 0);	// separator 

	MENU_AddItem(_FileMenu, NULL, "Open \"mb128\" Folder", WM_FOLDER_MB128);//Kitao追加
	MENU_AddItem(_FileMenu, NULL, "Open \"bram\" Folder", WM_FOLDER_BRAM);//Kitao追加
	MENU_AddItem(_FileMenu, NULL, NULL, 0);	// separator 

	MENU_AddItem(_FileMenu, NULL, "Open \"save\" Folder", WM_FOLDER_STATE);//Kitao追加
	MENU_AddItem(_FileMenu, _SaveButtonMenu, "Configure Save&&Load Button", 0); //Kitao追加
	MENU_AddItem(_FileMenu, _LoadAnother2Menu, "Load State another II", 0);//Kitao追加。v2.11
	MENU_AddItem(_FileMenu, _LoadAnotherMenu, "Load State another", 0);//Kitao追加
	MENU_AddItem(_FileMenu, NULL, "Load State && Pause", WM_LOAD_STATE_P);//Kitao更新
	MENU_AddItem(_FileMenu, NULL, "&Load State", WM_LOAD_STATE);
	MENU_AddItem(_FileMenu, _SaveAnother2Menu, "Save State another II", 0);//Kitao追加。v2.11
	MENU_AddItem(_FileMenu, _SaveAnotherMenu, "Save State another", 0);//Kitao追加
	MENU_AddItem(_FileMenu, NULL, "&Save State", WM_SAVE_STATE);
	MENU_AddItem(_FileMenu, NULL, NULL, 0);	// separator

	//Kitao追加
	MENU_AddItem(_LoadAnotherMenu, NULL, "Load State #20",WM_LOAD_STATE_20);
	MENU_AddItem(_LoadAnotherMenu, NULL, "Load State #19",WM_LOAD_STATE_19);
	MENU_AddItem(_LoadAnotherMenu, NULL, "Load State #18",WM_LOAD_STATE_18);
	MENU_AddItem(_LoadAnotherMenu, NULL, "Load State #17",WM_LOAD_STATE_17);
	MENU_AddItem(_LoadAnotherMenu, NULL, "Load State #16",WM_LOAD_STATE_16);
	MENU_AddItem(_LoadAnotherMenu, NULL, "Load State #15",WM_LOAD_STATE_15);
	MENU_AddItem(_LoadAnotherMenu, NULL, "Load State #14",WM_LOAD_STATE_14);
	MENU_AddItem(_LoadAnotherMenu, NULL, "Load State #13",WM_LOAD_STATE_13);
	MENU_AddItem(_LoadAnotherMenu, NULL, "Load State #12",WM_LOAD_STATE_12);
	MENU_AddItem(_LoadAnotherMenu, NULL, "Load State #11",WM_LOAD_STATE_11);
	MENU_AddItem(_LoadAnotherMenu, NULL, NULL, 0);//separator
	MENU_AddItem(_LoadAnotherMenu, NULL, "Load State #10",WM_LOAD_STATE_10);
	MENU_AddItem(_LoadAnotherMenu, NULL, "Load State #9", WM_LOAD_STATE_9);
	MENU_AddItem(_LoadAnotherMenu, NULL, "Load State #8", WM_LOAD_STATE_8);
	MENU_AddItem(_LoadAnotherMenu, NULL, "Load State #7", WM_LOAD_STATE_7);
	MENU_AddItem(_LoadAnotherMenu, NULL, "Load State #6", WM_LOAD_STATE_6);
	MENU_AddItem(_LoadAnotherMenu, NULL, "Load State #5", WM_LOAD_STATE_5);
	MENU_AddItem(_LoadAnotherMenu, NULL, "Load State #4", WM_LOAD_STATE_4);
	MENU_AddItem(_LoadAnotherMenu, NULL, "Load State #3", WM_LOAD_STATE_3);
	MENU_AddItem(_LoadAnotherMenu, NULL, "Load State #2", WM_LOAD_STATE_2);
	MENU_AddItem(_LoadAnotherMenu, NULL, "Load State #1", WM_LOAD_STATE_1);
	//Kitao追加
	MENU_AddItem(_SaveAnotherMenu, NULL, "Save State #20",WM_SAVE_STATE_20);
	MENU_AddItem(_SaveAnotherMenu, NULL, "Save State #19",WM_SAVE_STATE_19);
	MENU_AddItem(_SaveAnotherMenu, NULL, "Save State #18",WM_SAVE_STATE_18);
	MENU_AddItem(_SaveAnotherMenu, NULL, "Save State #17",WM_SAVE_STATE_17);
	MENU_AddItem(_SaveAnotherMenu, NULL, "Save State #16",WM_SAVE_STATE_16);
	MENU_AddItem(_SaveAnotherMenu, NULL, "Save State #15",WM_SAVE_STATE_15);
	MENU_AddItem(_SaveAnotherMenu, NULL, "Save State #14",WM_SAVE_STATE_14);
	MENU_AddItem(_SaveAnotherMenu, NULL, "Save State #13",WM_SAVE_STATE_13);
	MENU_AddItem(_SaveAnotherMenu, NULL, "Save State #12",WM_SAVE_STATE_12);
	MENU_AddItem(_SaveAnotherMenu, NULL, "Save State #11",WM_SAVE_STATE_11);
	MENU_AddItem(_SaveAnotherMenu, NULL, NULL, 0);//separator
	MENU_AddItem(_SaveAnotherMenu, NULL, "Save State #10",WM_SAVE_STATE_10);
	MENU_AddItem(_SaveAnotherMenu, NULL, "Save State #9", WM_SAVE_STATE_9);
	MENU_AddItem(_SaveAnotherMenu, NULL, "Save State #8", WM_SAVE_STATE_8);
	MENU_AddItem(_SaveAnotherMenu, NULL, "Save State #7", WM_SAVE_STATE_7);
	MENU_AddItem(_SaveAnotherMenu, NULL, "Save State #6", WM_SAVE_STATE_6);
	MENU_AddItem(_SaveAnotherMenu, NULL, "Save State #5", WM_SAVE_STATE_5);
	MENU_AddItem(_SaveAnotherMenu, NULL, "Save State #4", WM_SAVE_STATE_4);
	MENU_AddItem(_SaveAnotherMenu, NULL, "Save State #3", WM_SAVE_STATE_3);
	MENU_AddItem(_SaveAnotherMenu, NULL, "Save State #2", WM_SAVE_STATE_2);
	MENU_AddItem(_SaveAnotherMenu, NULL, "Save State #1", WM_SAVE_STATE_1);

	//Kitao追加。v2.11
	MENU_AddItem(_LoadAnother2Menu, NULL, "Load State #50",WM_LOAD_STATE_50);
	MENU_AddItem(_LoadAnother2Menu, NULL, "Load State #49",WM_LOAD_STATE_49);
	MENU_AddItem(_LoadAnother2Menu, NULL, "Load State #48",WM_LOAD_STATE_48);
	MENU_AddItem(_LoadAnother2Menu, NULL, "Load State #47",WM_LOAD_STATE_47);
	MENU_AddItem(_LoadAnother2Menu, NULL, "Load State #46",WM_LOAD_STATE_46);
	MENU_AddItem(_LoadAnother2Menu, NULL, "Load State #45",WM_LOAD_STATE_45);
	MENU_AddItem(_LoadAnother2Menu, NULL, "Load State #44",WM_LOAD_STATE_44);
	MENU_AddItem(_LoadAnother2Menu, NULL, "Load State #43",WM_LOAD_STATE_43);
	MENU_AddItem(_LoadAnother2Menu, NULL, "Load State #42",WM_LOAD_STATE_42);
	MENU_AddItem(_LoadAnother2Menu, NULL, "Load State #41",WM_LOAD_STATE_41);
	MENU_AddItem(_LoadAnother2Menu, NULL, NULL, 0);//separator
	MENU_AddItem(_LoadAnother2Menu, NULL, "Load State #40",WM_LOAD_STATE_40);
	MENU_AddItem(_LoadAnother2Menu, NULL, "Load State #39",WM_LOAD_STATE_39);
	MENU_AddItem(_LoadAnother2Menu, NULL, "Load State #38",WM_LOAD_STATE_38);
	MENU_AddItem(_LoadAnother2Menu, NULL, "Load State #37",WM_LOAD_STATE_37);
	MENU_AddItem(_LoadAnother2Menu, NULL, "Load State #36",WM_LOAD_STATE_36);
	MENU_AddItem(_LoadAnother2Menu, NULL, "Load State #35",WM_LOAD_STATE_35);
	MENU_AddItem(_LoadAnother2Menu, NULL, "Load State #34",WM_LOAD_STATE_34);
	MENU_AddItem(_LoadAnother2Menu, NULL, "Load State #33",WM_LOAD_STATE_33);
	MENU_AddItem(_LoadAnother2Menu, NULL, "Load State #32",WM_LOAD_STATE_32);
	MENU_AddItem(_LoadAnother2Menu, NULL, "Load State #31",WM_LOAD_STATE_31);
	MENU_AddItem(_LoadAnother2Menu, NULL, NULL, 0);//separator
	MENU_AddItem(_LoadAnother2Menu, NULL, "Load State #30",WM_LOAD_STATE_30);
	MENU_AddItem(_LoadAnother2Menu, NULL, "Load State #29",WM_LOAD_STATE_29);
	MENU_AddItem(_LoadAnother2Menu, NULL, "Load State #28",WM_LOAD_STATE_28);
	MENU_AddItem(_LoadAnother2Menu, NULL, "Load State #27",WM_LOAD_STATE_27);
	MENU_AddItem(_LoadAnother2Menu, NULL, "Load State #26",WM_LOAD_STATE_26);
	MENU_AddItem(_LoadAnother2Menu, NULL, "Load State #25",WM_LOAD_STATE_25);
	MENU_AddItem(_LoadAnother2Menu, NULL, "Load State #24",WM_LOAD_STATE_24);
	MENU_AddItem(_LoadAnother2Menu, NULL, "Load State #23",WM_LOAD_STATE_23);
	MENU_AddItem(_LoadAnother2Menu, NULL, "Load State #22",WM_LOAD_STATE_22);
	MENU_AddItem(_LoadAnother2Menu, NULL, "Load State #21",WM_LOAD_STATE_21);
	//Kitao追加。v2.11
	MENU_AddItem(_SaveAnother2Menu, NULL, "Save State #50",WM_SAVE_STATE_50);
	MENU_AddItem(_SaveAnother2Menu, NULL, "Save State #49",WM_SAVE_STATE_49);
	MENU_AddItem(_SaveAnother2Menu, NULL, "Save State #48",WM_SAVE_STATE_48);
	MENU_AddItem(_SaveAnother2Menu, NULL, "Save State #47",WM_SAVE_STATE_47);
	MENU_AddItem(_SaveAnother2Menu, NULL, "Save State #46",WM_SAVE_STATE_46);
	MENU_AddItem(_SaveAnother2Menu, NULL, "Save State #45",WM_SAVE_STATE_45);
	MENU_AddItem(_SaveAnother2Menu, NULL, "Save State #44",WM_SAVE_STATE_44);
	MENU_AddItem(_SaveAnother2Menu, NULL, "Save State #43",WM_SAVE_STATE_43);
	MENU_AddItem(_SaveAnother2Menu, NULL, "Save State #42",WM_SAVE_STATE_42);
	MENU_AddItem(_SaveAnother2Menu, NULL, "Save State #41",WM_SAVE_STATE_41);
	MENU_AddItem(_SaveAnother2Menu, NULL, NULL, 0);//separator
	MENU_AddItem(_SaveAnother2Menu, NULL, "Save State #40",WM_SAVE_STATE_40);
	MENU_AddItem(_SaveAnother2Menu, NULL, "Save State #39",WM_SAVE_STATE_39);
	MENU_AddItem(_SaveAnother2Menu, NULL, "Save State #38",WM_SAVE_STATE_38);
	MENU_AddItem(_SaveAnother2Menu, NULL, "Save State #37",WM_SAVE_STATE_37);
	MENU_AddItem(_SaveAnother2Menu, NULL, "Save State #36",WM_SAVE_STATE_36);
	MENU_AddItem(_SaveAnother2Menu, NULL, "Save State #35",WM_SAVE_STATE_35);
	MENU_AddItem(_SaveAnother2Menu, NULL, "Save State #34",WM_SAVE_STATE_34);
	MENU_AddItem(_SaveAnother2Menu, NULL, "Save State #33",WM_SAVE_STATE_33);
	MENU_AddItem(_SaveAnother2Menu, NULL, "Save State #32",WM_SAVE_STATE_32);
	MENU_AddItem(_SaveAnother2Menu, NULL, "Save State #31",WM_SAVE_STATE_31);
	MENU_AddItem(_SaveAnother2Menu, NULL, NULL, 0);//separator
	MENU_AddItem(_SaveAnother2Menu, NULL, "Save State #30",WM_SAVE_STATE_30);
	MENU_AddItem(_SaveAnother2Menu, NULL, "Save State #29",WM_SAVE_STATE_29);
	MENU_AddItem(_SaveAnother2Menu, NULL, "Save State #28",WM_SAVE_STATE_28);
	MENU_AddItem(_SaveAnother2Menu, NULL, "Save State #27",WM_SAVE_STATE_27);
	MENU_AddItem(_SaveAnother2Menu, NULL, "Save State #26",WM_SAVE_STATE_26);
	MENU_AddItem(_SaveAnother2Menu, NULL, "Save State #25",WM_SAVE_STATE_25);
	MENU_AddItem(_SaveAnother2Menu, NULL, "Save State #24",WM_SAVE_STATE_24);
	MENU_AddItem(_SaveAnother2Menu, NULL, "Save State #23",WM_SAVE_STATE_23);
	MENU_AddItem(_SaveAnother2Menu, NULL, "Save State #22",WM_SAVE_STATE_22);
	MENU_AddItem(_SaveAnother2Menu, NULL, "Save State #21",WM_SAVE_STATE_21);

	//Kitao追加。v2.15
	MENU_AddItem(_RecordingAnotherMenu, NULL, "Attention at CD-ROM Games", WM_RECORDING_HELP);
	MENU_AddItem(_RecordingAnotherMenu, NULL, NULL, 0);//separator
	MENU_AddItem(_RecordingAnotherMenu, NULL, "Start Recording #10",WM_RECORDING_10);
	MENU_AddItem(_RecordingAnotherMenu, NULL, "Start Recording #9", WM_RECORDING_9);
	MENU_AddItem(_RecordingAnotherMenu, NULL, "Start Recording #8", WM_RECORDING_8);
	MENU_AddItem(_RecordingAnotherMenu, NULL, "Start Recording #7", WM_RECORDING_7);
	MENU_AddItem(_RecordingAnotherMenu, NULL, "Start Recording #6", WM_RECORDING_6);
	MENU_AddItem(_RecordingAnotherMenu, NULL, "Start Recording #5", WM_RECORDING_5);
	MENU_AddItem(_RecordingAnotherMenu, NULL, "Start Recording #4", WM_RECORDING_4);
	MENU_AddItem(_RecordingAnotherMenu, NULL, "Start Recording #3", WM_RECORDING_3);
	MENU_AddItem(_RecordingAnotherMenu, NULL, "Start Recording #2", WM_RECORDING_2);
	MENU_AddItem(_RecordingAnotherMenu, NULL, "Start Recording #1", WM_RECORDING_1);
	//Kitao追加。v2.15
	MENU_AddItem(_PlayRecordAnotherMenu, NULL, "Attention at CD-ROM Games", WM_RECORDING_HELP);
	MENU_AddItem(_PlayRecordAnotherMenu, NULL, NULL, 0);//separator
	MENU_AddItem(_PlayRecordAnotherMenu, NULL, "Play Record #10",WM_PLAYRECORD_10);
	MENU_AddItem(_PlayRecordAnotherMenu, NULL, "Play Record #9", WM_PLAYRECORD_9);
	MENU_AddItem(_PlayRecordAnotherMenu, NULL, "Play Record #8", WM_PLAYRECORD_8);
	MENU_AddItem(_PlayRecordAnotherMenu, NULL, "Play Record #7", WM_PLAYRECORD_7);
	MENU_AddItem(_PlayRecordAnotherMenu, NULL, "Play Record #6", WM_PLAYRECORD_6);
	MENU_AddItem(_PlayRecordAnotherMenu, NULL, "Play Record #5", WM_PLAYRECORD_5);
	MENU_AddItem(_PlayRecordAnotherMenu, NULL, "Play Record #4", WM_PLAYRECORD_4);
	MENU_AddItem(_PlayRecordAnotherMenu, NULL, "Play Record #3", WM_PLAYRECORD_3);
	MENU_AddItem(_PlayRecordAnotherMenu, NULL, "Play Record #2", WM_PLAYRECORD_2);
	MENU_AddItem(_PlayRecordAnotherMenu, NULL, "Play Record #1", WM_PLAYRECORD_1);
	//Kitao追加。v2.15
	MENU_AddItem(_MoveRecordMenu, NULL, "Move \"Default Record Data\" to #10",WM_MOVERECORD_10);
	MENU_AddItem(_MoveRecordMenu, NULL, "Move \"Default Record Data\" to #9", WM_MOVERECORD_9);
	MENU_AddItem(_MoveRecordMenu, NULL, "Move \"Default Record Data\" to #8", WM_MOVERECORD_8);
	MENU_AddItem(_MoveRecordMenu, NULL, "Move \"Default Record Data\" to #7", WM_MOVERECORD_7);
	MENU_AddItem(_MoveRecordMenu, NULL, "Move \"Default Record Data\" to #6", WM_MOVERECORD_6);
	MENU_AddItem(_MoveRecordMenu, NULL, "Move \"Default Record Data\" to #5", WM_MOVERECORD_5);
	MENU_AddItem(_MoveRecordMenu, NULL, "Move \"Default Record Data\" to #4", WM_MOVERECORD_4);
	MENU_AddItem(_MoveRecordMenu, NULL, "Move \"Default Record Data\" to #3", WM_MOVERECORD_3);
	MENU_AddItem(_MoveRecordMenu, NULL, "Move \"Default Record Data\" to #2", WM_MOVERECORD_2);
	MENU_AddItem(_MoveRecordMenu, NULL, "Move \"Default Record Data\" to #1", WM_MOVERECORD_1);

	//Kitao追加
	MENU_AddItem(_SaveButtonMenu, NULL, "Return to default ([S]&&[L] key)", WM_SAVE_DEFAULT);
	MENU_AddItem(_SaveButtonMenu, NULL, "Configure \"Load State\" Button...", WM_LOAD_BUTTON);
	MENU_AddItem(_SaveButtonMenu, NULL, "Configure \"Save State\" Button...", WM_SAVE_BUTTON);

	//Kitao追加
	MENU_AddItem(_SShotSettingMenu, NULL, "Show Save Dialog", WM_SSHOT_SAVEDIALOG);
	MENU_AddItem(_SShotSettingMenu, NULL, "Save to \"screenshot\" Folder", WM_SSHOT_SAVEFOLDER);
	MENU_AddItem(_SShotSettingMenu, NULL, NULL, 0);	// separator
	MENU_AddItem(_SShotSettingMenu, _SShotButtonMenu, "Configure Screenshot Button", 0);
	MENU_AddItem(_SShotSettingMenu, NULL, NULL, 0);	// separator
	MENU_AddItem(_SShotSettingMenu, NULL, "Non-Scanlined", WM_SSHOT_NONSCANLINED);
	MENU_AddItem(_SShotSettingMenu, NULL, "Horizontal Scanlined (Default)", WM_SSHOT_HRSCANLINED);
	MENU_AddItem(_SShotSettingMenu, NULL, "TV Mode", WM_SSHOT_TVMODE);
	MENU_AddItem(_SShotSettingMenu, NULL, "Special Scanlined", WM_SSHOT_SCANLINED);
	MENU_AddItem(_SShotSettingMenu, NULL, "Same as Playing Mode", WM_SSHOT_SAMEPLAYING);
	MENU_AddItem(_SShotSettingMenu, NULL, NULL, 0);	// separator
	MENU_AddItem(_SShotSettingMenu, NULL, "Readme About Size", WM_SCREENSHOT_SIZE);
	MENU_AddItem(_SShotSettingMenu, NULL, "Size Non-Stretched (x1)", WM_SCREENSHOT_XN);
	MENU_AddItem(_SShotSettingMenu, NULL, "Size x4", WM_SCREENSHOT_X4);
	MENU_AddItem(_SShotSettingMenu, NULL, "Size x3", WM_SCREENSHOT_X3);
	MENU_AddItem(_SShotSettingMenu, NULL, "Size x2 (Default)", WM_SCREENSHOT_X2);
	MENU_AddItem(_SShotSettingMenu, NULL, "Size x1", WM_SCREENSHOT_X1);

	//Kitao追加
	MENU_AddItem(_SShotButtonMenu, NULL, "Return to default ([PrintScreen] key)", WM_SCREENSHOT_DEFAULT);
	MENU_AddItem(_SShotButtonMenu, NULL, "Configure Screenshot Button...", WM_SCREENSHOT_BUTTON);

	MENU_AddItem(_FileMenu, NULL, "Auto Check \"Set Resume\"", WM_AUTO_RESUME); //Kitao追加
	MENU_AddItem(_FileMenu, NULL, "Set Resume", WM_SET_RESUME);//Kitao追加
	MENU_AddItem(_FileMenu, NULL, NULL, 0);	// separator

	MENU_AddItem(_FileMenu, NULL, "&CD Change\tF1", WM_CD_CHANGE); //Kitao追加
	MENU_AddItem(_FileMenu, NULL, "&Open...\tO", WM_OPEN_FILE);

	MENU_AddItem(_ToolMenu, NULL, "TG16 ROM-image Bit Convert   ", WM_BIT_CONVERT); //Kitao追加
	MENU_AddItem(_ToolMenu, NULL, "Show Debug Window\tF4", WM_SHOW_DEBUG); //Kitao追加

	MENU_AddItem(_DefaultMenu, NULL, "Return to default All Settings", WM_ALL_DEFAULT); //Kitao追加

	//メニューの選択状態を初期化
	MENU_CheckRadioItem(_VolumeMenu, WM_VOLUME_NORMAL, WM_VOLUME_MUTE, WM_VOLUME_NORMAL);
//	MENU_CheckRadioItem(_SampleRateMenu, WM_AUDIO_SR11025, WM_AUDIO_SR48000, WM_AUDIO_SR44100);
	_bScreenMenuFull = FALSE;
	updateMenu();

	return TRUE;
}


static void
deinit_menu()
{
	MENU_Deinit(_Menu);
}


//Kitao追加。ZIP用テンポラリファイルを削除処理
static void
delete_zipTempfile()
{
	char	tempFilePathName[MAX_PATH+1];
	FILE*	fp;

	strcpy(tempFilePathName, _AppPath);
	strcat(tempFilePathName, "temp\\TEMP.pce");

	if (strcmp(_ZipTempFilePathName, tempFilePathName) == 0) //_ZipTempFilePathNameが壊れていないかをチェック。これで不慮の事故があった場合でも確実に間違ったファイルを消すことはなくなる。
		if ((fp = fopen(_ZipTempFilePathName, "r")) != NULL)
		{
			fclose(fp);
			SetFileAttributes(_ZipTempFilePathName, FILE_ATTRIBUTE_NORMAL); //読み込み専用ファイルにしてあった場合に必要
			remove(_ZipTempFilePathName);
		}
}

//Kitao追加。ROMイメージのファイル名とゲーム名を設定する。ZIPファイルだった場合は解凍処理も行う。
static BOOL
set_openFilePathName(
	const char*		fileName,	//開くファイルのパスと名前
	char*			openFilePathName,	//開くファイルのパスと名前。ZIPファイル選択時には必ず"\temp\TEMP.pce"が入る。
	char*			gameFilePathName)	//ZIPファイルを選択した場合、解凍する中身ではなく"書庫ファイル自身"のファイル名が入る。ZIP以外の場合、openFilePathNameと全く同じ内容が入る。
{
	//ZIP解凍用
	HINSTANCE			lib;
	UNZIPGETVERSION		UnZipGetVersion;
	UNZIPOPENARCHIVE	UnZipOpenArchive;
	UNZIPCLOSEARCHIVE	UnZipCloseArchive;
	UNZIPFINDFIRST		UnZipFindFirst;
	UNZIP				UnZip;
	char				cmd[256 + MAX_PATH * 2];
	char				buf[256];
	HARC				hArc;
	INDIVIDUALINFO		info;
	char				tempPath[MAX_PATH+1];
	char				srcFileName[MAX_PATH+1];
	char				extrFileName[MAX_PATH+1];
	char*				p;

	strcpy(srcFileName, fileName); //fileNameは書き換えないようにするためsrcFileNameにコピー
	if (strstr(strToLower(extractFileExt(srcFileName)), "zip")) //.zipの場合
	{
		//ZIP書庫を解凍
		lib = LoadLibrary("UNZIP32.DLL");
		if (lib == NULL)
		{
			if (MessageBox( WINMAIN_GetHwnd(),
							"\"UNZIP32.DLL\" is necessary.  Is the homepage of download opened ?    ",
							"Ootake", MB_YESNO) == IDYES)
					ShellExecute(NULL, "open", "http://www.csdinc.co.jp/archiver/lib/unzip32.html", NULL, NULL, SW_SHOW); //UNZIP32.DLLのホームページを開く
			return FALSE;
		}
		UnZipGetVersion = (UNZIPGETVERSION)GetProcAddress(lib, "UnZipGetVersion");
		if (UnZipGetVersion == NULL) { FreeLibrary(lib); return FALSE; }
		if (UnZipGetVersion() < 541) //UNZIP32.DLLのバージョンチェック。正規表現の解除ができる5.41以降が必要。
		{
			FreeLibrary(lib);
			if (MessageBox( WINMAIN_GetHwnd(),
							"\"Please use new version \"UNZIP32.DLL\".  Is the homepage of download opened ?    ",
							"Ootake", MB_YESNO) == IDYES)
				ShellExecute(NULL, "open", "http://www.csdinc.co.jp/archiver/lib/unzip32.html", NULL, NULL, SW_SHOW); //UNZIP32.DLLのホームページを開く
			return FALSE;
		}
		UnZipOpenArchive = (UNZIPOPENARCHIVE)GetProcAddress(lib, "UnZipOpenArchive");
		if (UnZipOpenArchive == NULL) { FreeLibrary(lib); return FALSE; }
		UnZipCloseArchive = (UNZIPCLOSEARCHIVE)GetProcAddress(lib, "UnZipCloseArchive");
		if (UnZipCloseArchive == NULL) { FreeLibrary(lib); return FALSE; }
		UnZipFindFirst = (UNZIPFINDFIRST)GetProcAddress(lib, "UnZipFindFirst");
		if (UnZipFindFirst == NULL) { FreeLibrary(lib); return FALSE; }
		UnZip = (UNZIP)GetProcAddress(lib, "UnZip");
		if (UnZip == NULL) { FreeLibrary(lib); return FALSE; }
			
		delete_zipTempfile();
		hArc = UnZipOpenArchive(WINMAIN_GetHwnd(), fileName, M_NOT_INQUIRE_WRITE | M_BAR_WINDOW_OFF);
		if (UnZipFindFirst(hArc, "*.pce *.hes", &info) != 0)
		{
			MessageBox(WINMAIN_GetHwnd(), "ROM image file was not found.    ", "Ootake", MB_OK);
			UnZipCloseArchive(hArc); FreeLibrary(lib); return FALSE;
		}
		UnZipCloseArchive(hArc);
		
		strcpy(tempPath, _AppPath);
		strcat(tempPath, "temp\\");
		sprintf(cmd, "-x --i -j -o -qq -qr0 \"%s\" \"%s\" \"%s\"", fileName, tempPath, info.szFileName); //"-qr0"(正規表現の解除)をしないと"[]"をファイル名に使っていたときに解凍できない。
		if (UnZip(WINMAIN_GetHwnd(), cmd, buf, 256) != 0)
		{
			MessageBox(WINMAIN_GetHwnd(), "The decompression cannot have been done.    ", "Ootake", MB_OK);
			FreeLibrary(lib); return FALSE;
		}
		strcpy(srcFileName, tempPath);
		p = strrchr(info.szFileName, '/');
		if (p == NULL)
			strcpy(extrFileName, info.szFileName);
		else
			strcpy(extrFileName, p+1);
		strcat(srcFileName, extrFileName);
		//リネーム
		SetFileAttributes(srcFileName, FILE_ATTRIBUTE_NORMAL);
		rename(srcFileName, _ZipTempFilePathName);
		FreeLibrary(lib);
		
		strcpy(openFilePathName, _ZipTempFilePathName);
		strcpy(gameFilePathName, fileName);
		if (strstr(strToLower(extractFileExt(extrFileName)), "hes"))
			_bHesFile = TRUE;
		else
			_bHesFile = FALSE;
	}
	else //.pce .hes の場合
	{
		strcpy(openFilePathName, fileName);
		strcpy(gameFilePathName, fileName);
		if (strstr(strToLower(extractFileExt(srcFileName)), "hes"))
			_bHesFile = TRUE;
		else
			_bHesFile = FALSE;
	}
	return TRUE;
}

//Kitao追加。ダイアログで使う「オープン用フォルダ」と「保存用フォルダ」を保管。
//			 v1.49から「別のシステムカードのファイルパス(２つぶん)」も同じファイルに保管。
static void
SaveDialogFolder()
{
	char	fileName[MAX_PATH+1];
	char	buf[MAX_PATH+2];
	char	kaigyou[2] = {0x0A, 0x00};
	FILE*	fp;

	strcpy(fileName, _AppPath);
	strcat(fileName, "Folder.dat");
	if ((fp = fopen(fileName, "w")) != NULL)
	{
		strcpy(buf, _CurrentOpenPath);
		strcat(buf, kaigyou); //LFコードを付加
		fputs(buf, fp);
		strcpy(buf, _CurrentSavePath);
		strcat(buf, kaigyou); //LFコードを付加
		fputs(buf, fp);
		strcpy(buf, _OtherSysCardPath1);
		strcat(buf, kaigyou); //LFコードを付加
		fputs(buf, fp);
		strcpy(buf, _OtherSysCardPath2);
		strcat(buf, kaigyou); //LFコードを付加
		fputs(buf, fp);
		fclose(fp);
	}
}

/*----------------------------------------------------------------------------
	ファイルダイアログを表示し、開くファイルの絶対パスを取得する。
----------------------------------------------------------------------------*/
//Kitao更新。システムカードのイメージファイルを指定するときにも使用。
static BOOL
file_dialog(
	Sint32		type,	//Kitao追加。type=1なら通常ROMイメージ。2ならシステムカードイメージ。3,4なら"別のシステムカード"イメージ。
	HWND		hWnd,
	char*		openFilePathName,	//開くファイルのパスと名前。ZIPファイル選択時には必ず"\temp\TEMP.pce"が入る。
	char*		gameFilePathName)	//ZIPファイルを選択した場合、解凍する中身ではなく"書庫ファイル自身"のファイル名が入る。ZIP以外の場合、openFilePathNameと全く同じ内容が入る。
{
	OPENFILENAME	ofn;
	char			fileName[MAX_PATH+1];
	char			buf[MAX_PATH+1];
	BOOL			ret;

	if (_CurrentOpenPath[0] == 0)
		strcpy(_CurrentOpenPath, _AppPath);
	if (!PathFileExists(_CurrentOpenPath)) //ディレクトリが存在していない場合
		strcpy(_CurrentOpenPath, _AppPath);
	strcpy(fileName, _CurrentOpenPath);
	strcat(fileName, "(Select)"); //ここが空欄だとWin2000以降ではディレクトリ指定のダイアログオープンが出来ない。v1.29

	/* OPENFILENAME 構造体 ofn パラメータを設定する */
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize 	  = sizeof(OPENFILENAME);
	ofn.hwndOwner		  = hWnd;
	ofn.hInstance		  = (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE);
	ofn.lpstrFilter		  = "\"PC Engine\" ROM image (*.zip *.pce *.hes)\0*.zip;*.pce;*.hes\0ZIP files (*.zip)\0*.zip\0PCE files (*.pce)\0*.pce\0HES files (*.hes)\0*.zip\0All files (*.*)\0*.*\0"; //Kitao更新。zip,hesファイルの読み込みにも対応した。
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter	  = 0;
	ofn.nFilterIndex	  = 0;
	ofn.lpstrInitialDir	  = _CurrentOpenPath; //v1.29追加
	ofn.lpstrFile		  = fileName;
	ofn.nMaxFile		  = sizeof(fileName) - 1;
	ofn.lpstrFileTitle	  = NULL;
	ofn.nMaxFileTitle	  = 0;
	switch (type) //Kitao追加
	{
		case 1:
			ofn.lpstrTitle = "Open ROM image"; //Kitao更新
			break;
		case 2:
			ofn.lpstrTitle = "Set SystemCard image"; //Kitao更新
			break;
		case 3:
			ofn.lpstrTitle = "Set Other SystemCard image 1"; //v1.49追加
			break;
		case 4:
			ofn.lpstrTitle = "Set Other SystemCard image 2"; //v1.49追加
			break;
	}
	ofn.nFileOffset 	  = 0;
	ofn.nFileExtension	  = 0;
	ofn.lpstrDefExt 	  = (LPSTR)NULL;
	ofn.lCustData		  = 0;
	ofn.lpfnHook 		  = (LPOFNHOOKPROC)NULL;
	ofn.lpTemplateName	  = (LPSTR)NULL;
	ofn.Flags			  = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	//CoUninitialize(); //一時的にCOMを開放。v2.23追加。COINIT_MULTITHREADEDでCOMを初期化すると、"ファイルの種類"の選択ができなくなってしまう問題を回避。
	ret = GetOpenFileName(&ofn);
	//CoInitializeEx(NULL, COINIT_MULTITHREADED); //COMを再初期化。v2.23

	if (ret == FALSE)
		return FALSE;

	strcpy(buf, fileName);
	buf[strlen(fileName)-strlen(extractFileName(fileName))] = 0; //ファイル名をカットしてパスだけにする
	strcpy(_CurrentOpenPath, buf);
	if (type == 3) //v1.49追加
		strcpy(_OtherSysCardPath1, fileName);
	if (type == 4) //v1.49追加
		strcpy(_OtherSysCardPath2, fileName);
	SaveDialogFolder();

	ret = set_openFilePathName(fileName, openFilePathName, gameFilePathName);
	return ret;
}


//v2.24追加
static LPITEMIDLIST
GetPIDL(char*	lpszPath)
{
	ULONG			chEatan;
    ULONG			dwAttributes;
    OLECHAR			olePath[MAX_PATH];
	LPSHELLFOLDER	pDesktopFolder;
	LPITEMIDLIST	pidl = NULL;

	if (SUCCEEDED(SHGetDesktopFolder(&pDesktopFolder)))
	{
        MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, lpszPath, -1, olePath, MAX_PATH);
        if (FAILED(pDesktopFolder->ParseDisplayName(NULL, NULL, olePath, &chEatan, &pidl, &dwAttributes)))
            pidl = NULL;
        pDesktopFolder->Release();
    }
	return pidl;
}

//v2.24追加
static int CALLBACK
BrowseCallbackProc(HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	RECT	rcW;
	RECT	rcC;
	LONG	frameWidth;

	if (uMsg == BFFM_INITIALIZED)
	{
		GetWindowRect(WINMAIN_GetHwnd(), &rcW);
		GetClientRect(WINMAIN_GetHwnd(), &rcC);
		frameWidth = ((rcW.right-rcW.left)-(rcC.right-rcC.left)) / 2;
		SetWindowPos(hWnd, HWND_TOP, rcW.left+frameWidth,
									 rcW.top+((rcW.bottom-rcW.top)-(rcC.bottom-rcC.top)-frameWidth), 0, 0, SWP_NOSIZE); //ダイアログの位置を設定。大きさは前回開いた大きさを引き継ぎ、変化させない。
	}
	return 0;
}

//Kitao追加。フォルダ選択のダイアログを表示する。v2.24
static BOOL
folder_dialog(
	HWND		hWnd,
	char*		fileFolderName)	//選択したフォルダのパスを返す。
{
	BROWSEINFO		bi;
	LPITEMIDLIST	pidl;
	char			root[MAX_PATH+1];
	char			folder[MAX_PATH+1];

	strcpy(root, _AppPath);
	strcat(root, "install");
	CreateDirectory(root, NULL);//installディレクトリがない場合作る

	ZeroMemory(&bi, sizeof(BROWSEINFO));
	bi.hwndOwner		=	hWnd;
	bi.pidlRoot			=	GetPIDL(root);
	bi.pszDisplayName	=	folder;
	bi.lpszTitle		=	"[Ootake] Select \"FullInstalled Game Folder\".";
	bi.ulFlags			=	BIF_NEWDIALOGSTYLE | BIF_NONEWFOLDERBUTTON | BIF_RETURNONLYFSDIRS; 
	bi.lpfn				=	&BrowseCallbackProc;
	bi.lParam			=	NULL;
    bi.iImage			=	0;

	//CoUninitialize(); //一時的にCOMを開放。
	//CoInitializeEx(NULL, COINIT_APARTMENTTHREADED); //COMをアパートメントで初期化
	pidl = SHBrowseForFolder(&bi);
	//CoUninitialize();
	//CoInitializeEx(NULL, COINIT_MULTITHREADED); //COMを再初期化

	if (!pidl)
		return FALSE;
	SHGetPathFromIDList(pidl, folder);
	CoTaskMemFree(pidl);
	strcpy(fileFolderName, folder);
	return TRUE;
}


//Kitao追加。正常に開けた場合、ROMイメージ履歴に追加。v1.00
static void
AddRecentRom()
{
	int		i, j;
	char	fileName[MAX_PATH+1];
	char	gameFilePathName[MAX_PATH+1+2];
	char	kaigyou[2] = {0x0A, 0x00};
	FILE*	fp;

	if ((_bCDGame)&&(_bCueFile)) //CUEファイル起動中の場合
		strcpy(gameFilePathName, _CueFilePathName); //v2.24追加
	else //通常
		strcpy(gameFilePathName, _GameFilePathName);

	if (strcmp(_RecentRom[1], gameFilePathName) == 0)
		return; //前回と同じファイルを開いた場合何もせずにリターン

	for (i=2; i<=20; i++)
		if (strcmp(_RecentRom[i], gameFilePathName) == 0) //既に履歴に入っていた場合
		{
			for (j=i-1; j>=1; j--)
				strcpy(_RecentRom[j+1], _RecentRom[j]);
			break;
		}
	if (i==21) //履歴に入っていなかった場合、1～19を2～20へ移す(元の20は消滅)
		for (j=19; j>=1; j--)
			strcpy(_RecentRom[j+1], _RecentRom[j]);
	strcpy(_RecentRom[1], gameFilePathName);

	strcpy(fileName, _AppPath);
	strcat(fileName, "RecentRom.dat");
	if ((fp = fopen(fileName, "w")) != NULL)
	{
		for (i=1; i<=20; i++)
		{
			strcpy(gameFilePathName, _RecentRom[i]);
			strcat(gameFilePathName, kaigyou); //LFコードを付加
			fputs(gameFilePathName, fp);
		}
		fclose(fp);
	}

	//メニュー表示を更新
	updateMenu();
}


//Kitao追加。"Windows Aero"の有効・無効を切り替える。v2.21
static void
setWindowsAero(
	BOOL	bWindowsAero)
{
	if (_bWindowsVista) //Vista以降のみ処理を行う
		if (_DrawMethod == 1) //Direct3D利用時のみ切り替えが有効。DirectDraw時は元々オフ固定。
			_FuncDwmEnableComposition(bWindowsAero);
}


//Kitao追加。ウィンドウが画面からはみ出していた場合、画面内に収まるように移動する。
void
APP_WindowWithinScreen()
{
	RECT		rc;//Kitao追加
	int			cx,cy;//Kitao追加
	
	GetWindowRect(WINMAIN_GetHwnd(), &rc);
	cx = GetSystemMetrics(SM_CXSCREEN);
	cy = GetSystemMetrics(SM_CYSCREEN);
	if ((rc.right > cx)&&(rc.bottom > cy)) //右にも下にもはみ出してしまった場合
		MoveWindow(WINMAIN_GetHwnd(), cx - (rc.right-rc.left), cy - (rc.bottom-rc.top),
					rc.right-rc.left, rc.bottom-rc.top, TRUE);//ウィンドウの位置を変える
	else
	{
		if (rc.right > cx) //右にだけはみ出してしまった場合
			MoveWindow(WINMAIN_GetHwnd(), cx - (rc.right-rc.left), rc.top,
						rc.right-rc.left, rc.bottom-rc.top, TRUE);//ウィンドウの位置を変える
		else if (rc.bottom > cy) //下にだけはみ出してしまった場合
			MoveWindow(WINMAIN_GetHwnd(), rc.left, cy - (rc.bottom-rc.top),
						rc.right-rc.left, rc.bottom-rc.top, TRUE);//ウィンドウの位置を変える
	}
}


//Kitao追加。画面を再描画する。
static void
screenUpdate()
{
	MAINBOARD_ScreenUpdate(TRUE);
}


static BOOL
change_screen_mode(
	Sint32		magnification, //Kitao更新。5,6の場合、フルスクリーンの特殊解像度。5…400x300, 6…800x600。0,1,5はカスタマイズ解像度としても使用。
	BOOL		bFullScreen,
	Uint32		fullScreenColor, //Kitao追加。fullScreenColor…フルスクリーン時のカラービット数。16or32
	BOOL		bResolutionAutoChange) //フルスクリーンの自動解像度切替モードでの切替の場合TRUEにして呼ぶ。自動解像度切替モードでも最初の1回(デスクトップ→フルスクリーン切替)はFALSEで呼ぶ。v2.21追加
{
	Sint32		width = 640; //widthとheightは引数として使わなくしたので、こちらで宣言。
	Sint32		height = 480; //widthとheightは引数として使わなくしたので、こちらで宣言。
	Sint32		fullMagnification = magnification; //フルスクリーン時に設定を保管用
	Sint32		a;
	Sint32		tvW; //Kitao追加
	Sint32		srcW; //Kitao追加

	//640x480のフルスクリーン＆フルストレッチ＆スキャンライン表示の場合、画質維持のためフルストレッチ(低倍率拡大)せず、オーバースキャンを表示することで画面を一杯にする。v2.64追加
	if ((((fullMagnification == 0)&&((_CustomWidth1 != 640)||(_CustomHeight1 != 480)))||(fullMagnification == 2))&&
		(bFullScreen)&&(_bFullStretched)&&((_ScanLineType == 2)||(_bTvMode)))
	{
		MAINBOARD_SetShowOverscanTop(8);
		MAINBOARD_SetShowOverscanBottom(8);
		MAINBOARD_SetShowOverscanLeft(8);
		MAINBOARD_SetShowOverscanRight(8);
	}
	else
	{
		MAINBOARD_SetShowOverscanTop(_ShowOverscanTop);
		MAINBOARD_SetShowOverscanBottom(_ShowOverscanBottom);
		MAINBOARD_SetShowOverscanLeft(_ShowOverscanLeft);
		MAINBOARD_SetShowOverscanRight(_ShowOverscanRight);
	}

	//Kitao更新。ストレッチでウィンドウモードのときはwidthを299*nに設定。
	if (bFullScreen)
	{
		switch (magnification)
		{
			case 0:		width = _CustomWidth1;  height = _CustomHeight1;
						if (_bStretched) //ストレッチモード時は、それに合わせて、倍率を大きくしすぎないようにする。v1.57
						{
							if ((MAINBOARD_GetShowOverscanLeft() > 0)||((_bOverscanHideBlackBelt)&&(_StartShowOverscanLeft > 0)))
								magnification = (Sint32)((double)width / 313.24); //左右のオーバースキャンエリアを表示する場合
							else
								magnification = width / 299; //アスペクト比固定のため横は299になる
						}
						else
							magnification = width / 256;
						if ((MAINBOARD_GetShowOverscanTop()+MAINBOARD_GetShowOverscanBottom() > 0)||((_bOverscanHideBlackBelt)&&(_StartShowOverscanTop+_StartShowOverscanBottom > 0)))
							a = height / 240; //上下のオーバースキャンエリアを表示する場合
						else
							a = height / 224;
						if (a < magnification)
							magnification = a; //縦か横か狭いほうに合わせる
						break;
			case 1:		width = _CustomWidth2;  height = _CustomHeight2;
						if (_bStretched) //ストレッチモード時は、それに合わせて、倍率を大きくしすぎないようにする。v1.57
						{
							if ((MAINBOARD_GetShowOverscanLeft() > 0)||((_bOverscanHideBlackBelt)&&(_StartShowOverscanLeft > 0)))
								magnification = (Sint32)((double)width / 313.24); //左右のオーバースキャンエリアを表示する場合
							else
								magnification = width / 299; //アスペクト比固定のため横は299になる
						}
						else
							magnification = width / 336;
						if ((MAINBOARD_GetShowOverscanTop()+MAINBOARD_GetShowOverscanBottom() > 0)||((_bOverscanHideBlackBelt)&&(_StartShowOverscanTop+_StartShowOverscanBottom > 0)))
							a = height / 240; //上下のオーバースキャンエリアを表示する場合
						else
							a = height / 224;
						if (a < magnification)
							magnification = a; //縦か横か狭いほうに合わせる
						break;
			case 2:		width =  640; height = 480; break;
			case 3:		width = 1024; height = 768; break;
			case 4:		width = 1280; height =1024; break;
			case 5:		width = _CustomWidth3;  height = _CustomHeight3;
						if (_bStretched) //ストレッチモード時は、それに合わせて、倍率を大きくしすぎないようにする。v1.57
						{
							if ((MAINBOARD_GetShowOverscanLeft() > 0)||((_bOverscanHideBlackBelt)&&(_StartShowOverscanLeft > 0)))
								magnification = (Sint32)((double)width / 313.24); //左右のオーバースキャンエリアを表示する場合
							else
								magnification = width / 299; //アスペクト比固定のため横は299になる
						}
						else
							magnification = width / 512; //カスタマイズ解像度３は、横512時の想定用で使うので倍率が高くならないようにする。
						if ((MAINBOARD_GetShowOverscanTop()+MAINBOARD_GetShowOverscanBottom() > 0)||((_bOverscanHideBlackBelt)&&(_StartShowOverscanTop+_StartShowOverscanBottom > 0)))
							a = height / 240; //上下のオーバースキャンエリアを表示する場合
						else
							a = height / 224;
						if (a < magnification)
							magnification = a; //縦か横か狭いほうに合わせる
						break;
			case 6:		width =  800; height = 600;
						magnification = 2;
						break;
			default:	width =  640; height = 480; //バージョンダウンしたとき(将来バージョンで値が7以降に拡張されている可能性)のために必要
						magnification = 2;
						break;
		}
		if (magnification < 1)
			magnification = 1;
		if (magnification > 4)
			magnification = 4;
	}
	else //ウィンドウモード時
	{
		if (_bStretched)
		{
			if ((MAINBOARD_GetShowOverscanLeft() > 0)||((_bOverscanHideBlackBelt)&&(_StartShowOverscanLeft > 0)))
				width = (Sint32)(313.24 * (double)magnification + 0.5); //左右のオーバースキャンエリアを表示する場合。v1.43追加
			else
				width = 299 * magnification; //アスペクト比固定のため横は299(2倍時598)のウィンドウに。
			if (_bVStretched) //縦画面モード
				width = (Sint32)((double)width * (256.0/336.0)); //実機同様に256:336の比で縮小する（横256以外のゲームも同様に可能）
			if (_bOverscanHideBlackBelt)
			{	//オーバースキャン領域を黒帯で隠す設定の場合
				height = 224;
				if (_StartShowOverscanTop > 0)
					height += 8;
				else
					height += MAINBOARD_GetShowOverscanTop();
				if (_StartShowOverscanBottom > 0)
					height += 8;
				else
					height += MAINBOARD_GetShowOverscanBottom();
				height *= magnification;
			}
			else //通常
				height = (MAINBOARD_GetShowOverscanTop()+224+MAINBOARD_GetShowOverscanBottom()) * magnification;
		}
		else
		{//ノンストレッチモード
			tvW = VDC_GetTvWidth();
			srcW = VDC_GetScreenWidth();
			if (MAINBOARD_GetShowOverscanLeft() > 0) //左右のオーバースキャンエリアを表示する場合。v1.43追加
			{
				switch (tvW)
				{
					case 256:
						tvW = 268;
						break;
					case 336:
						tvW = 352; //現状は左右8ドットに固定。
						break;
				}
			}
			if (srcW > tvW)
				srcW = tvW;
			width  = srcW * magnification; //ソフトで使われる解像度と同じ比率でウィンドウを用意。
			height = (MAINBOARD_GetShowOverscanTop()+224+MAINBOARD_GetShowOverscanBottom()) * magnification;
		}
	}

	// 引数長すぎ ※Kitao更新。v0.90までもうひとつ増やしてしまいました(^^;がv0.91にて整頓完了！
	if (MAINBOARD_ChangeScreenMode(width, height, magnification, bFullScreen, fullScreenColor))
	{
		_ScreenWidth			= width;//Kitao追加
		_ScreenHeight			= height;//Kitao追加
		if (bFullScreen) //Kitao追加
			_FullMagnification	= fullMagnification;
		else
			_Magnification		= magnification;
		_bFullScreen			= bFullScreen;
		_FullScreenColor		= fullScreenColor;
	}
	else //解像度が合わず、スクリーンの設定に失敗した場合
	{	//基本的な項目だけデフォルトに戻す。DrawMethodやFullScreenColor等は、勝手に変えると逆に訳がわからなくなりそうなので自動では変えないこととする。
		PRINTF("Change screen mode failed; recovering the default screen mode.");
		if (bFullScreen)
			_FullMagnification		= 2;//Kitao追加。どの環境でも必ず表示可能な640x480に設定する。
		else
		{
			_Magnification			= 2;//Kitao変更
			_ScreenWidth			= 299 * 2;//Kitao変更
			_ScreenHeight			= 224 * 2;
		}
		_bFullScreen			= FALSE;
		WINMAIN_SetNormalWindow(_ScreenWidth, _ScreenHeight);
		if (!MAINBOARD_ChangeScreenMode(_ScreenWidth, _ScreenHeight, _Magnification,
										_bFullScreen, _FullScreenColor))
		{
			// それでもダメだったらあきらめる。 
			PRINTF("Error; can't set the default screen mode.");
			return FALSE;
		}
	}
	//Kitao更新。画面変更に成功したときも失敗したときもここでコンフィグを更新
	CONFIG_Set("[APP DISPLAY] Screen Width", &_ScreenWidth, sizeof(_ScreenWidth));
	CONFIG_Set("[APP DISPLAY] Screen Height", &_ScreenHeight, sizeof(_ScreenHeight));
	CONFIG_Set("[APP DISPLAY] FullMagnification", &_FullMagnification, sizeof(_FullMagnification));
	CONFIG_Set("[APP DISPLAY] Magnification", &_Magnification, sizeof(_Magnification));

	updateMenu();

	if (_WindowsAutoVSync == 2) //v2.65追加
	{
		if (bFullScreen)
			_bSyncTo60HzScreen = FALSE;
		else
			_bSyncTo60HzScreen = TRUE;
	}
	SCREEN_SetSyncTo60HzScreen(_bSyncTo60HzScreen); //v1.35追加
	MAINBOARD_ResetPrevTvW(); //自動解像度変更をおこなうため＆表示範囲外になった部分のゴミをクリアするために必要。v1.35

	return TRUE;
}


//Kitao追加。更新された設定でスクリーンモードを変更する
static BOOL
updateScreenMode(
	BOOL	bWindowCenter)	//bWindowCenter…ウィンドウを中央位置に合わせる場合TRUEにして呼ぶ
{
	BOOL	ret;
	RECT	rc,rc2;
	int		x;

	//_CddaAdjust(CD-DAテンポ微調整)関連を再設定
	if ((getCddaAdjust() % 10) == 5)
		_CddaAdjustFineTune = 5;
	else
		_CddaAdjustFineTune = 0;
	CDROM_ClearCDDAAjustCount(); //CddaAjustが切替わったので必ずカウンタをクリア。v2.32
	ADPCM_ClearCDDAAjustCount(); //

	if (_bFullScreen)
		return change_screen_mode(_FullMagnification, _bFullScreen, _FullScreenColor, FALSE);
	else
	{
		//ウィンドウモードの場合、ウィンドウを変更前の中央位置と合わせる
		GetWindowRect(WINMAIN_GetHwnd(), &rc);//変更前のウィンドウ位置を記憶しておく
		ret = change_screen_mode(_Magnification, _bFullScreen, _FullScreenColor, FALSE);
		if (bWindowCenter)
		{
			GetWindowRect(WINMAIN_GetHwnd(), &rc2);//変更後のウィンドウ位置
			x = rc.left + (rc.right-rc.left)/2 - (rc2.right-rc2.left)/2;
			if (x < 0)
				x = 0;
			if (x+(rc2.right-rc2.left) > GetSystemMetrics(SM_CXSCREEN))
				x = GetSystemMetrics(SM_CXSCREEN) - (rc2.right-rc2.left);
			MoveWindow(WINMAIN_GetHwnd(), x, rc2.top,
						rc2.right-rc2.left, rc2.bottom-rc2.top, TRUE);
		}
		return ret;
	}
}


/*----------------------------------------------------------------------------
	[toggle_fullscreen] v2.23更新。他からも呼べるようにした
----------------------------------------------------------------------------*/
void
APP_ToggleFullscreen()
{
	int 	kidouX; //Kitao追加
	int 	kidouY; //Kitao追加
	RECT	rc;
	BOOL	ret;
	BOOL	bPauseNoReleaseR = _bPauseNoRelease; //v2.48追加。RecentRomフォームからここを呼ぶ場合、_bPauseNoReleaseをFALSEに戻さないために必要。

	_bPauseNoRelease = TRUE; //Kitao追加。スクリーンモード切替中はアクティブになってもポーズを解除しないようにする。

	MAINBOARD_Pause(TRUE); //音を乱さないためにポーズ
	WINMAIN_ShowCursor(TRUE); //フル→ウィンドウ時にカーソルが消えてしまわないように

	if (!_bFullScreen) //Kitao追加。ウィンドウ→フルの場合、ウィンドウ位置を記憶しておく。
		APP_SaveWindowPosition();
	_bFullScreen ^= TRUE;

	//Kitao追加
	if (_bFullScreen) //ウィンドウ→フルスクリーン
	{
		APP_SetForegroundWindowOotake(); //スクリーンチェンジ前にOotakeを確実にアクティブにしておく。Vistaで必要。
		ret = updateScreenMode(FALSE);
		if (_bDisableWindowsAero)
			setWindowsAero(FALSE); //フルスクリーンの場合、Aeroはオフ固定で。v2.21追加
	}
	else //フルスクリーン→ウィンドウ
	{
		if (!_bDisableWindowsAero)
			setWindowsAero(TRUE); //Aero使用ならAeroをオンに戻す。v2.21追加
		APP_SetForegroundWindowOotake(); //スクリーンチェンジ前にOotakeを確実にアクティブにしておく。Vistaで必要。
		updateScreenMode(FALSE); //Kitao追加
		//フル→ウィンドウの場合、ウィンドウ位置を読み出して復元する。
		CONFIG_Get("[APP DISPLAY] Window Left", &kidouX, sizeof(kidouX));
		CONFIG_Get("[APP DISPLAY] Window Top", &kidouY, sizeof(kidouY));
		GetWindowRect(WINMAIN_GetHwnd(), &rc);
		MoveWindow(WINMAIN_GetHwnd(), kidouX, kidouY,
					rc.right-rc.left, rc.bottom-rc.top, TRUE);
	}

	_bPauseNoRelease = bPauseNoReleaseR; //Kitao追加
}


/*----------------------------------------------------------------------------
	[run_emulator]
----------------------------------------------------------------------------*/
static void
run_emulator(
	BOOL	bRun)
{
	//Kitao追加。フルスクリーンでメニューを表示していた場合、画面にゴミが残らないようクリアしておく
	if ((_bFullScreen)&&(!_bRunning)&&(bRun))
		SCREEN_Clear();

	_bRunning = bRun;

	if (_bRunning)
	{
		if ((_bHideMenu)||(_bFullScreen)) //Kitao追加。メニューを隠す場合
			MENU_Show(NULL);
		else
			MENU_Show(_Menu);

		if (_bFullScreen)
			WINMAIN_SetFullScreenWindow(_ScreenWidth, _ScreenHeight);
		else
			WINMAIN_SetNormalWindow(_ScreenWidth, _ScreenHeight);

		if (MOUSE_IsConnected() || _bFullScreen)
			WINMAIN_ShowCursor(FALSE);

		if (_bFullScreen)
			UpdateWindow(WINMAIN_GetHwnd()); //フルスクリーンではすぐにメニュー表示等を更新し、乱れを防ぐ。

		MAINBOARD_Pause(FALSE);
	}
	else
	{
		MAINBOARD_Pause(TRUE);
		//WAV出力をしていた場合、出力を完了させる。v1.29
		if (APP_OutputWavEnd())
		{	//「次の曲」を録音モードで、録音(音発生)前だった場合、すぐに録音を開始して続けるなら。
			run_emulator(TRUE);
			return;
		}
		updateMenu();//v2.16追加。Windowsのボリューム値などを最新に反映。
		MENU_Show(_Menu);
		WINMAIN_ShowCursor(TRUE);

		if (_bFullScreen)
			WINMAIN_SetFullScreenWindow(_ScreenWidth, _ScreenHeight);
		else
			WINMAIN_SetNormalWindow(_ScreenWidth, _ScreenHeight);

		//Kitao追加
		if (!_bFullScreen)
		{
			//画面からウィンドウがはみ出していた場合画面内に戻す。※メニュー操作時に描画が乱れてしまうのを防ぐため
			APP_WindowWithinScreen();
			//ウィンドウモードのときは、メニューが出たときに画面の下端が消えてしまうため、画面を再描画する。
			screenUpdate();
		}
		else
			UpdateWindow(WINMAIN_GetHwnd()); //フルスクリーンではすぐにメニュー表示等を更新し、乱れを防ぐ。
	}
}


//Kitao追加。v1.29
static void
OutputWavStart(
	Sint32	mode)
{
	unsigned char header[44] = {'R','I','F','F', 0,0,0,0, 'W','A','V','E', 'f','m','t',' ',
							    16,0,0,0, 1,0, 2,0, 0x44,0xAC,0,0, 0x10,0xB1,0x02,0, 4,0,16,0, //リニアPCM 16bitステレオ44100Hz
							    'd','a','t','a', 0,0,0,0};
	int		i;
	FILE*	fp;
	char	noSound[4] = {0,0,0,0};
	char	buf[256+MAX_PATH+1];
	char	buf2[8+1];
	OPENFILENAME	ofn;
	BOOL			ret;

	if (_CurrentSavePath[0] == 0)
		strcpy(_CurrentSavePath, _DesktopPath);
	if (!PathFileExists(_CurrentSavePath)) //ディレクトリが存在していない場合
		strcpy(_CurrentSavePath, _DesktopPath);
	strcpy(_WavFilePathName, _CurrentSavePath);
	strcat(_WavFilePathName, _GameFileNameBuf);
	strcat(_WavFilePathName, " - ");
	for (i=1; i<=9999; i++)
	{
		strcpy(buf, _WavFilePathName);
		sprintf(buf2, "%d.wav", i);
		strcat(_WavFilePathName, buf2);
		if (!PathFileExists(_WavFilePathName)) //同名ファイルがなければ
		{
			strcpy(_WavFilePathName, buf);
			sprintf(buf2, "%d", i);
			strcat(_WavFilePathName, buf2);
			break;
		}
		else
			strcpy(_WavFilePathName, buf);
	}

	/* OPENFILENAME 構造体 ofn パラメータを設定する */
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize 	  = sizeof(OPENFILENAME);
	ofn.hwndOwner		  = WINMAIN_GetHwnd();
	ofn.hInstance		  = (HINSTANCE)GetWindowLong(WINMAIN_GetHwnd(), GWL_HINSTANCE);
	ofn.lpstrFilter		  = "WAV file (*.wav)\0*.wav\0All files (*.*)\0*.*\0";
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter	  = 0;
	ofn.nFilterIndex	  = 0;
	ofn.lpstrInitialDir	  = _CurrentSavePath;
	ofn.lpstrFile		  = _WavFilePathName;
	ofn.nMaxFile		  = sizeof(_WavFilePathName) - 1 - 4; //-4は拡張子追加分
	ofn.lpstrFileTitle	  = NULL;
	ofn.nMaxFileTitle	  = 0;
	ofn.lpstrTitle		  = "Save WAV file";
	ofn.nFileOffset 	  = 0;
	ofn.nFileExtension	  = 0;
	ofn.lpstrDefExt 	  = (LPSTR)NULL;
	ofn.lCustData		  = 0;
	ofn.lpfnHook 		  = (LPOFNHOOKPROC)NULL;
	ofn.lpTemplateName	  = (LPSTR)NULL;
	ofn.Flags			  = OFN_PATHMUSTEXIST;

	//CoUninitialize(); //一時的にCOMを開放。v2.23追加。COINIT_MULTITHREADEDでCOMを初期化すると、"ファイルの種類"の選択ができなくなってしまう問題を回避。
	ret = GetSaveFileName(&ofn);
	//CoInitializeEx(NULL, COINIT_MULTITHREADED); //COMを再初期化。v2.23

	if (ret == FALSE)
	{
		strcpy(_WavFilePathName, "");
		return;
	}
	strcpy(buf, _WavFilePathName);
	buf[strlen(_WavFilePathName)-strlen(extractFileName(_WavFilePathName))] = 0; //ファイル名をカットしてパスだけにする
	strcpy(_CurrentSavePath, buf);
	SaveDialogFolder();

	strcpy(buf, _WavFilePathName); //_WavFilePathName自体は大文字を小文字に変換しないようにするためbufを使う。
	if (strstr(strToLower(extractFileExt(buf)), "wav") == NULL) //WAVファイルじゃなかった場合。拡張子がなかった場合も含む。
		strcat(_WavFilePathName, ".wav");

	//同じファイル名のファイルが既にあった場合は上書きするかを問う
	fp = fopen(_WavFilePathName, "rb");
	if (fp != NULL)
	{
		fclose(fp);
		sprintf(buf, "There is already the \"%s\".    \nOverwrite Ok?", _WavFilePathName);
		if (MessageBox(WINMAIN_GetHwnd(), buf, "Ootake", MB_YESNO) != IDYES)
		{
			strcpy(_WavFilePathName, "");
			return;
		}
	}

	fp = fopen(_WavFilePathName, "wb");
	if (fp != NULL)
	{
		if (_bOutputWavNext)
			PRINTF("Ready Output (Next Tune) \"%s\".", APP_GetWavFileName());
		else
			PRINTF("Ready Output \"%s\".", APP_GetWavFileName());
		fwrite(header, 44, 1, fp); //ヘッダーを出力
		if (_bOutputWavAddNoSound)
		{	//頭に0.5秒の無音を作る
			for (i=0; i<44100*0.5; i++)
				fwrite(noSound, 4, 1, fp);
			AOUT_SetOutputWavFileSize((DWORD)(4*44100*0.5)); //0.5秒
		}
		else
			AOUT_SetOutputWavFileSize(0);
		AOUT_SetFpOutputWAV(fp, mode); //WAV出力開始の合図
	}
	else
	{
		sprintf(buf, "Couldn't Write \"%s\".    ", _WavFilePathName);
		MessageBox(WINMAIN_GetHwnd(), buf, "Ootake Error", MB_OK);
		strcpy(_WavFilePathName, "");
	}
}

//Kitao追加
#define OutputWavFadeOutSecond 8 //フェードアウト完了までの秒数
BOOL
APP_OutputWavEnd()
//戻り値…録音を続けるならTRUEを返す。
{
	FILE*	fp;
	FILE*	fp2;
	char	tempFilePathName[MAX_PATH+1];
	char	buf[4096];
	Sint16	sbuf[2]; //ステレオ2ch
	char	noSound[4] = {0,0,0,0};
	DWORD	i;
	DWORD	size;
	DWORD	b;
	DWORD	a;
	double	r;
	Sint32	outputWavWaitFinish = AOUT_GetOutputWavWaitFinish();

	if (_WavFilePathName[0] != 0)
	{
		if ((_bOutputWavNext)&&(outputWavWaitFinish == 3)) //「次の曲」録音モードで、現在の曲が鳴っているときに(録音ゼロで)終了した場合。
		{
			WINMAIN_ShowCursor(TRUE);
			if (MessageBox( WINMAIN_GetHwnd(),
							"Now \"Output WAV File Start at the Next Tune\" mode.    \n"
							"Please push \"Yes\" if you start recording at once.\n"
							"(If \"No\" is pushed, the recording is aborted.)",
							"Ootake", MB_YESNO) == IDYES)
				return TRUE; //録音を続ける
		}
		
		fp = AOUT_GetFpOutputWAV();
		AOUT_SetFpOutputWAV(NULL, 0); //録音終了
		fclose(fp);
		if (outputWavWaitFinish < 5) //無音のみの録音だった場合
		{
			remove(_WavFilePathName); //ファイルを削除
			PRINTF("Abort Output \"%s\".", APP_GetWavFileName());
			strcpy(_WavFilePathName, "");
			return FALSE;
		}
		
		SetCursor(LoadCursor(NULL, IDC_WAIT));
		WINMAIN_ShowCursor(TRUE);
		
		//末尾の無音部分をカット
		fp = fopen(_WavFilePathName, "rb");
		strcpy(tempFilePathName, _AppPath);
		strcat(tempFilePathName, "temp\\TEMP.wav"); //テンポラリファイル
		fp2 = fopen(tempFilePathName, "w+b");
		size = 44 + AOUT_GetOutputWavFileSize();
		for (i=size-4; i>=44; i-=4)
		{
			fseek(fp, i, SEEK_SET);
			fread(buf, 4, 1, fp);
			if ((buf[0]==0)&&(buf[1]==0)&&(buf[2]==0)&&(buf[3]==0))
				size -= 4; //無音だった場合カット
			else
				break;
		}
		for (i=0; i<size; i+=4096)
		{
			fseek(fp, i, SEEK_SET);
			b = (DWORD)fread(buf, 1, 4096, fp);
			fwrite(buf, 1, b, fp2);
		}
		fclose(fp);
		
		//フェードアウトをするかどうかダイアログで問う。するなら末尾を８秒ぶんフェードアウト処理
		if (size >= 44+4*44100*OutputWavFadeOutSecond)
		{
			if (MessageBox( WINMAIN_GetHwnd(),
							"\"Output WAV File\" ended.\n Is the \"Fade Out\" processing executed ?    ",
							"Ootake", MB_YESNO) == IDYES)
			{
				for (i=size-4*44100*OutputWavFadeOutSecond; i<size; i+=4) //末尾"OutputWavFadeOutSecond"秒前の位置から書き換え
				{
					fseek(fp2, i, SEEK_SET);
					fread(sbuf, 2, 2, fp2);
					a = 4*44100*OutputWavFadeOutSecond - (i - (size-4*44100*OutputWavFadeOutSecond)) - 1;
					r = (double)a / (4*44100*OutputWavFadeOutSecond); //ボリューム減少率
					sbuf[0] = (Sint16)((double)sbuf[0] * r);
					sbuf[1] = (Sint16)((double)sbuf[1] * r);
					fseek(fp2, i, SEEK_SET);
					fwrite(sbuf, 2, 2, fp2);
				}
			}
		}
		
		if (_bOutputWavAddNoSound)
		{	//末尾に２秒の無音データを追加
			for (i=0; i<44100*2; i++)
				fwrite(noSound, 4, 1, fp2);
			size += 4*44100*2; //２秒
		}
		fseek(fp2, 4, SEEK_SET);
		size -= 8;
		fwrite(&size, 4, 1, fp2); //ファイルサイズ(-8)を書き込み
		size -= 36;
		fseek(fp2, 40, SEEK_SET);
		fwrite(&size, 4, 1, fp2); //波形データサイズを書き込み
		fclose(fp2);
		
		//テンポラリファイルを本ファイルへコピー
		CopyFile(tempFilePathName, _WavFilePathName, FALSE);
		remove(tempFilePathName);
		
		SetCursor(LoadCursor(NULL, IDC_ARROW));
		PRINTF("Complete Output \"%s\".", APP_GetWavFileName());
		strcpy(_WavFilePathName, "");
	}
	return FALSE;
}


//Kitao追加。v2.52
static void
OutputAviStart()
{
	//(開発中)
}


//Kitao追加。ノンストレッチ時のゲーム画面表示領域の横幅を返す。v2.12
Sint32
APP_GetNonstretchedWidth()
{
	Sint32	tvW;
	Sint32	srcW;

	tvW = VDC_GetTvWidth();
	srcW = VDC_GetScreenWidth();
	if (MAINBOARD_GetShowOverscanLeft() > 0) //左右のオーバースキャンエリアを表示する場合。v1.43追加
	{
		switch (tvW)
		{
			case 256:
				tvW = 268;
				break;
			case 336:
				tvW = 352; //現状は左右8ドットに固定。
				break;
		}
	}
	if (srcW > tvW)
		srcW = tvW;
	
	return srcW;
}

//Kitao追加。ゲーム画面表示領域の横幅を返す。v2.12
Sint32
APP_GetGameWidth(
	Sint32	magnification)
{
	Sint32	width;
	Sint32	w2;
	Sint32	left;

	if (_bFullScreen && APP_GetFullStretched(TRUE)) //640x480フルストレッチスキャンライン時の動作のために_bFullStretchedを直に参照せず、APP_GetFullStretched()で参照。v2.64
	{
		//アスペクト比を4:3に保つ
		if (_ScreenWidth/4 >= _ScreenHeight/3)
		{	//縦のほうが短いので縦を基準とする
			w2 = (Sint32)((double)(_ScreenHeight / 3.0 * 4.0 + 0.5));
			left = (_ScreenWidth - w2) / 2;
		}
		else
		{	//横のほうが短いので横を基準とする
			left = 0;
		}
		width = _ScreenWidth - left*2;
	}
	else if (_bStretched)
	{
		if ((APP_GetOverscanHideBlackLR())|| //左右のオーバースキャン領域に黒帯を付けているなら
			(MAINBOARD_GetShowOverscanLeft() > 0)) //または左右のオーバースキャン領域を表示しているなら
			width = (Sint32)(313.24 * (double)magnification + 0.5);
		else
			width = 299 * magnification;
		if (_bVStretched) //縦画面モードなら
			width = (Sint32)((double)width * (256.0/336.0)); //実機同様に256:336の比で縮小する
	}
	else //ノンストレッチモード
		width  = APP_GetNonstretchedWidth() * magnification; //ソフトで使われる解像度と同じ比率でウィンドウを用意。

	return width;
}

//Kitao追加。ゲーム画面表示領域の縦幅を返す。v2.12
Sint32
APP_GetGameHeight(
	Sint32	magnification)
{
	Sint32	height;
	Sint32	h2;
	Sint32	top;

	if (_bFullScreen && APP_GetFullStretched(TRUE)) //640x480フルストレッチスキャンライン時の動作のために_bFullStretchedを直に参照せず、APP_GetFullStretched()で参照。v2.64
	{
		//アスペクト比を4:3に保つ
		if (_ScreenWidth/4 >= _ScreenHeight/3)
		{	//縦のほうが短いので縦を基準とする
			if (MAINBOARD_GetShowOverscanLeft() > 0) //左右のオーバースキャン領域を表示するなら
				top = (_ScreenHeight - (Sint32)((double)_ScreenHeight * 0.96137 + 0.5)) / 2; //0.96137=224/((268-256)/4*3+224)
			else
				top = 0;
		}
		else
		{	//横のほうが短いので横を基準とする
			h2 = (Sint32)((double)(_ScreenWidth / 4.0 * 3.0 + 0.5));
			if (MAINBOARD_GetShowOverscanLeft() > 0) //左右のオーバースキャン領域を表示するなら
				top = (_ScreenHeight - (Sint32)((double)h2 * 0.96137 + 0.5)) / 2; //0.96137=224/((268-256)/4*3+224)
			else
				top = (_ScreenHeight - h2) / 2;
		}
		height = _ScreenHeight - top*2;
	}
	else
	{
		height = 224 * magnification;
		if (APP_GetOverscanHideBlackTop()) //上側のオーバースキャン領域に黒帯を付けるなら
			height += 8 * magnification;
		else
			height += (MAINBOARD_GetShowOverscanTop()) * magnification;
		if (APP_GetOverscanHideBlackBottom()) //下側のオーバースキャン領域に黒帯を付けるなら
			height += 8 * magnification;
		else
			height += (MAINBOARD_GetShowOverscanBottom()) * magnification;
	}

	return height;
}

//Kitao追加。スクリーンショット機能の処理。v2.12,v2.36更新
static void
OutputScreenshot()
{
	int			i;
	Uint32		fsize;
	Sint32		width;
	Sint32		height;
	FILE*		fp;
	unsigned char header[54] = {'B','M', 0,0,0,0, 0,0,0,0, 54,0,0,0, //BMPファイルヘッダ
								40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0, 0,0,0,0, 0,0,0,0, //情報ヘッダ(for Windows)
								0xC4,0x0E,0,0, 0xC4,0x0E,0,0, 0,0,0,0, 0,0,0,0};
	char			buf[256+MAX_PATH+1];
	char			buf2[8+1];
	OPENFILENAME	ofn;
	char			title[21+4+1];
	SYSTEMTIME		st;
	Sint32			wp;
	Sint32			magnification;
	BOOL			bToggle = FALSE;
	BOOL			ret;
	Sint32			prevMagnification	= SCREEN_GetMagnification();
	BOOL			bPrevStretched		= _bStretched;
	BOOL			bPrevVStretched		= _bVStretched;
	BOOL			bPrevFullStretched	= _bFullStretched;
	Sint32			prevScanLineType	= _ScanLineType;
	BOOL			prevTvMode			= _bTvMode;
	BOOL			bOk;


	if (_bScreenshotDialog)
	{
		if (_CurrentSavePath[0] == 0)
			strcpy(_CurrentSavePath, _DesktopPath);
		if (!PathFileExists(_CurrentSavePath)) //ディレクトリが存在していない場合
			strcpy(_CurrentSavePath, _DesktopPath);
		strcpy(_ScreenshotFilePathName, _CurrentSavePath);
	}
	else
	{
		strcpy(_ScreenshotFilePathName, _AppPath);
		strcat(_ScreenshotFilePathName, "screenshot");
		CreateDirectory(_ScreenshotFilePathName, NULL);//screenshotディレクトリがない場合作る
		strcat(_ScreenshotFilePathName, "\\");
	}
	strcat(_ScreenshotFilePathName, _GameFileNameBuf);
	strcat(_ScreenshotFilePathName, " - ");
	GetLocalTime(&st);
	sprintf(buf, "%02d%02d%02d_%02d%02d", (st.wYear % 100),st.wMonth,st.wDay,st.wHour,st.wMinute);
	strcat(_ScreenshotFilePathName, buf);
	strcpy(buf, _ScreenshotFilePathName);
	strcat(buf, ".bmp");
	if (PathFileExists(buf)) //同名ファイルがすでにあったなら
	{
		strcat(_ScreenshotFilePathName, "_");
		for (i=1; i<=9999; i++)
		{
			strcpy(buf, _ScreenshotFilePathName);
			sprintf(buf2, "%d.bmp", i);
			strcat(_ScreenshotFilePathName, buf2);
			if (!PathFileExists(_ScreenshotFilePathName)) //同名ファイルがなければ
			{
				strcpy(_ScreenshotFilePathName, buf);
				sprintf(buf2, "%d", i);
				strcat(_ScreenshotFilePathName, buf2);
				break;
			}
			else
				strcpy(_ScreenshotFilePathName, buf);
		}
	}

	//拡大率を設定。処理速度優先のため、現状は、最大で「現在の表示ウィンドウの大きさまで」をキャプチャ可能とする。
	if (_ScreenshotMagnification <= prevMagnification)
		magnification = _ScreenshotMagnification;
	else
		magnification = prevMagnification;
	if (_ScreenshotMagnification == 0) //ノンストレッチ(x1)設定の場合
		magnification = 1;

	_bPauseNoRelease = TRUE; //音の乱れを防ぐため、アクティブになってもポーズを解除しないようにする。

	if (_bScreenshotDialog)
	{
		//セーブダイアログを出す前にカーソルを表示(ショートカットキーを押してここが呼び出されたときのために必要)。
		MAINBOARD_Pause(TRUE); //Pause表示
		WINMAIN_ShowCursor(TRUE);

		/* OPENFILENAME 構造体 ofn パラメータを設定する */
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize 	  = sizeof(OPENFILENAME);
		ofn.hwndOwner		  = WINMAIN_GetHwnd();
		ofn.hInstance		  = (HINSTANCE)GetWindowLong(WINMAIN_GetHwnd(), GWL_HINSTANCE);
		ofn.lpstrFilter		  = "BMP file (*.bmp)\0*.bmp\0All files (*.*)\0*.*\0";
		ofn.lpstrCustomFilter = NULL;
		ofn.nMaxCustFilter	  = 0;
		ofn.nFilterIndex	  = 0;
		ofn.lpstrInitialDir	  = _CurrentSavePath;
		ofn.lpstrFile		  = _ScreenshotFilePathName;
		ofn.nMaxFile		  = sizeof(_ScreenshotFilePathName) - 1 - 4; //-4は拡張子追加分
		ofn.lpstrFileTitle	  = NULL;
		ofn.nMaxFileTitle	  = 0;
		strcpy(title, "Save BMP file [ Size ");
		switch (magnification)
		{
			case 1: strcat(title, "x1 ]"); break;
			case 2: strcat(title, "x2 ]"); break;
			case 3: strcat(title, "x3 ]"); break;
			case 4: strcat(title, "x4 ]"); break;
		}
		ofn.lpstrTitle		  = title;
		ofn.nFileOffset 	  = 0;
		ofn.nFileExtension	  = 0;
		ofn.lpstrDefExt 	  = (LPSTR)NULL;
		ofn.lCustData		  = 0;
		ofn.lpfnHook 		  = (LPOFNHOOKPROC)NULL;
		ofn.lpTemplateName	  = (LPSTR)NULL;
		ofn.Flags			  = OFN_PATHMUSTEXIST;

		//CoUninitialize(); //一時的にCOMを開放。v2.23追加。COINIT_MULTITHREADEDでCOMを初期化すると、"ファイルの種類"の選択ができなくなってしまう問題を回避。
		ret = GetSaveFileName(&ofn);
		//CoInitializeEx(NULL, COINIT_MULTITHREADED); //COMを再初期化。v2.23

		if (ret == FALSE)
		{
			strcpy(_ScreenshotFilePathName, "");
			screenUpdate();
			_bPauseNoRelease = FALSE;
			return;
		}
		strcpy(buf, _ScreenshotFilePathName);
		buf[strlen(_ScreenshotFilePathName)-strlen(extractFileName(_ScreenshotFilePathName))] = 0; //ファイル名をカットしてパスだけにする
		strcpy(_CurrentSavePath, buf);
		SaveDialogFolder();
	}

	strcpy(buf, _ScreenshotFilePathName); //_ScreenshotFilePathName自体は大文字を小文字に変換しないようにするためbufを使う。
	if (strstr(strToLower(extractFileExt(buf)), "bmp") == NULL) //BMPファイルじゃなかった場合。拡張子がなかった場合も含む。
		strcat(_ScreenshotFilePathName, ".bmp");

	//同じファイル名のファイルが既にあった場合は上書きするかを問う
	bOk = TRUE;
	fp = fopen(_ScreenshotFilePathName, "rb");
	if (fp != NULL)
	{
		fclose(fp);
		sprintf(buf, "There is already the \"%s\".    \nOverwrite Ok?", _ScreenshotFilePathName);
		if (MessageBox(WINMAIN_GetHwnd(), buf, "Ootake", MB_YESNO) != IDYES)
			bOk = FALSE;
	}

	//出力処理
	if (bOk)
	{
		if ((_DrawMethod == 2)&&(_bFullScreen)&&(_FullScreenColor == 16)) //DirectDrawで16bitカラーを使っていた場合、ウィンドウモードに切り替える。
		{
			APP_ToggleFullscreen();
			run_emulator(FALSE);//メニュー表示などを乱さないように更新
			bToggle = TRUE;
		}

		//ノンストレッチ(x1)設定の場合
		if (_ScreenshotMagnification == 0)
		{
			_bStretched = FALSE;
			_bVStretched = FALSE;
			_bFullStretched = FALSE;
			if (APP_GetGameWidth(magnification) > _ScreenWidth) //ウィンドウからはみ出してしまう場合（x1でプレイ中に横336以上のゲームの場合）
			{	//強制的にストレッチのx1にする。(そうしないとメモリ範囲オーバーでOotakeごと落ちる)
				_bStretched = TRUE;
				_bVStretched = FALSE;
				_bFullStretched = FALSE;
			}
		}
		//描画倍率を設定する。ガンマ設定で必要なのでこの位置で。
		SCREEN_SetMagnification(magnification);
		//描画モードとガンマの再設定
		if ((_ScreenshotScanLineType != -1)||(_ScreenshotMagnification == 0))
		{
			if (_ScreenshotScanLineType != -1)
				APP_SetScanLineType(_ScreenshotScanLineType, FALSE);
			if (_ScreenshotScanLineType == 4)
				APP_SetTvMode(TRUE, FALSE);  //ガンマも更新される
			else
				APP_SetTvMode(FALSE, FALSE); //ガンマも更新される
		}

		//ファイルへ出力
		fp = fopen(_ScreenshotFilePathName, "wb");
		if (fp != NULL)
		{
			SetCursor(LoadCursor(NULL, IDC_WAIT));
			//width
			width = APP_GetGameWidth(magnification);
			header[18] = (byte)width;
			header[19] = (byte)(width/0x100);
			//height
			height = APP_GetGameHeight(magnification);
			header[22] = (byte)height;
			header[23] = (byte)(height/0x100);
			//ファイルサイズ
			wp = 4 - (width*3 % 4); //１ラインを4byte単位に整えるための値
			if (wp==4) wp=0;
			fsize = 54 + (width*3+wp)*height;
			header[2] = (byte)fsize;
			header[3] = (byte)(fsize/0x100);
			header[4] = (byte)(fsize/0x10000);
			header[5] = (byte)(fsize/0x1000000);
			fwrite(header, 54, 1, fp); //ヘッダーを出力
			SCREEN_WriteScreenshot(fp);
			fclose(fp);
			//出力完了したメッセージの表示
			if (_ScreenshotMagnification == 0)
				strcpy(buf, "Non-Stretched");
			else
				switch (magnification)
				{
					case 2: strcpy(buf, "x2"); break;
					case 3: strcpy(buf, "x3"); break;
					case 4: strcpy(buf, "x4"); break;
					default: //x1
						strcpy(buf, "x1"); break;
				}
			PRINTF("Screenshot Saved. ( Size %s )", buf);
			PRINTF_SetSaveLoadMessage(); //PRINTFメッセージの表示時間を少し短くする
			SetCursor(LoadCursor(NULL, IDC_ARROW));
		}
		else
		{
			sprintf(buf, "Couldn't Write \"%s\".    ", _ScreenshotFilePathName);
			MessageBox(WINMAIN_GetHwnd(), buf, "Ootake Error", MB_OK);
			strcpy(_ScreenshotFilePathName, "");
		}

		//描画倍率を元に戻す
		SCREEN_SetMagnification(prevMagnification);
		//ストレッチを元に戻す
		_bStretched = bPrevStretched;
		_bVStretched = bPrevVStretched;
		_bFullStretched = bPrevFullStretched;
		//画面モードを元に戻す
		APP_SetScanLineType(prevScanLineType, FALSE);
		APP_SetTvMode(prevTvMode, FALSE);
		//表示をプレイ画面に戻す
		if (((APP_GetOverscanHideBlackLR())&&(MAINBOARD_GetShowOverscanLeft() == 0))|| //左右のオーバースキャン領域に黒帯表示していた場合
			((APP_GetOverscanHideBlackTop())&&(MAINBOARD_GetShowOverscanTop() < 8))) //上側のオーバースキャン領域に黒帯表示していた場合
				MAINBOARD_SetResolutionChange(TRUE); //描画時にゴミが残らないように画面全体をクリアしてから描画する。
		switch (_DrawMethod)
		{
			case 1: //Direct3D
				MAINBOARD_ScreenUpdate(TRUE); //このとき_pD3DDev->Presentでの表示更新は行っていないため、キャプチャ中に表示が乱れることがない。
				break;
			case 2: //DirectDraw
				MAINBOARD_ScreenUpdate(FALSE); //V-Syncは行わない。こうして戻しておけば、キャプチャ中に表示が乱れづらい。
				break;
		}
		MAINBOARD_SetResolutionChange(FALSE); //元に戻す

		if (bToggle) //フルスクリーンモードに切り替えていた場合は元に戻す
		{
			Sleep(1000); //急に戻すとディスプレイに負担が掛かる場合があるのでウェイト。
			APP_ToggleFullscreen();
			run_emulator(FALSE);//メニュー表示などを乱さないように更新
		}
	}

	screenUpdate(); //ダイアログ表示などで黒くなった部分を再描画。メッセージもすぐ表示される。
	_bPauseNoRelease = FALSE;
}


/*----------------------------------------------------------------------------
	[save_config]
----------------------------------------------------------------------------*/
static BOOL
save_config(
	const char*		pAppPath)
{
	char	filePathName[MAX_PATH+1];

	sprintf(filePathName, "%s" APP_CONFIG_FILENAME, pAppPath);
	return CONFIG_Save(filePathName);
}


/*----------------------------------------------------------------------------
	[load_config]
----------------------------------------------------------------------------*/
static BOOL
load_config(
	const char*		pAppPath)
{
	char	filePathName[MAX_PATH+1];

	sprintf(filePathName, "%s" APP_CONFIG_FILENAME, pAppPath);
	return CONFIG_Load(filePathName);
}


/*----------------------------------------------------------------------------
	[save_bram]
----------------------------------------------------------------------------*/
static void
save_bram()
{
	char	filePathName[MAX_PATH+1];

	//Kitao更新。バックアップラムもゲームごとに別ファイルで保存するようにした。
	strcpy(filePathName, _AppPath);
	strcat(filePathName, "bram");//Kitao追加。bramフォルダに保存するようにした
	CreateDirectory(filePathName, NULL);//bramディレクトリがない場合作る
	strcat(filePathName, "\\");//Kitao追加
	strcat(filePathName, _GameFileNameBuf);//Kitao更新
	strcat(filePathName, "_bram.dat");
	MAINBOARD_SaveBRAM(filePathName);

	//Kitao追加。MB128もここで保存するようにした。
	MB128_SaveFile();
}


/*----------------------------------------------------------------------------
	[load_bram]
----------------------------------------------------------------------------*/
static void
load_bram()
{
	char	filePathName[MAX_PATH+1];

	//Kitao更新。バックアップラムもゲームごとに別ファイルで保存するようにした。
	strcpy(filePathName, _AppPath);
	strcat(filePathName, "bram\\");//Kitao追加。bramフォルダに保存するようにした
	strcat(filePathName, _GameFileNameBuf);//Kitao更新
	strcat(filePathName, "_bram.dat");
	MAINBOARD_LoadBRAM(filePathName);

	//Kitao追加。MB128もここで読み込みするようにした。
	MB128_LoadFile();
}


/*----------------------------------------------------------------------------
	[end_recording]
----------------------------------------------------------------------------*/
static void
end_recording()
{
	/* 記録中だった場合は終了する */
	if (_fpRecording != NULL)
	{
		if (_InputRecordingNumber == 0)
			PRINTF("End Recording.");
		else
			PRINTF("End Recording #%d.", _InputRecordingNumber);
		INPUT_Record(FALSE);
		INPUT_WriteBuffer(_fpRecording);
		fclose(_fpRecording);
		_fpRecording = NULL;
		MENU_ChangeItemText(_FileMenu, WM_RECORDING_GAMEPLAY, "Start &Recording\tF5");//Kitao更新
		_InputRecordingNumber = -1;
	}
}


/*----------------------------------------------------------------------------
	[end_playrecord]
----------------------------------------------------------------------------*/
static void
end_playrecord()
{
	/* 再生中だった場合は終了する */
	if (_fpPlayRecord != NULL)
	{
		if (_InputPlayRecordNumber == 0)
			PRINTF("End Play Record.");
		else
			PRINTF("End Play Record #%d.", _InputPlayRecordNumber);
		INPUT_PlayRecord(FALSE);
		fclose(_fpPlayRecord);
		_fpPlayRecord = NULL;
		MENU_ChangeItemText(_FileMenu, WM_PLAYRECORD_GAMEPLAY, "&Play Record\tF6");
		_InputPlayRecordNumber = -1;
	}
}


//Kitao追加。レジュームファイルを保存。v0.90
static void
save_resume()
{
	char	buf[MAX_PATH+1];
	FILE*	fp;

	if (_bResumeMode == FALSE)
		return;

	strcpy(buf, _AppPath);
	strcat(buf, "save");//saveフォルダに保存
	CreateDirectory(buf, NULL);//saveディレクトリがない場合作る
	strcat(buf, "\\");
	strcat(buf, _GameFileNameBuf);
	strcat(buf, "_resume.dat");

	fp = fopen(buf, "wb");
	if (fp != NULL)
	{
		MAINBOARD_SaveState(fp);
		fclose(fp);
	}
}


//Kitao追加。レジュームファイルがあるかどうかチェックし、あればレジュームファイルを読み込んで再開させる。
static void
load_resume()
{
	char	buf[MAX_PATH+1];
	char	buf2[256];
	FILE*	fp;

	MAINBOARD_PauseNoMessage(TRUE); //Kitao追加。再開時の音の乱れを防ぐため、音を停止しておく。

	strcpy(buf, _AppPath);
	strcat(buf, "save\\");//saveフォルダ
	strcat(buf, _GameFileNameBuf);
	strcat(buf, "_resume.dat");

	fp = fopen(buf, "rb");
	if ((fp != NULL)&&(!MAINBOARD_GetBackupFull())) //レジュームファイルがあった場合
	{
		_bLoadingResumeFile = TRUE; //レジュームファイルのステートを読み出し中であることを示す。APP_ResumeWindowsVolume()で利用。
		_bResumeMode = MAINBOARD_LoadState(fp); //読み込み成功ならTRUE。
		_bLoadingResumeFile = FALSE; //通常はFALSEの状態に。
		if (_PrevWindowsVolume != -2) //音量がレジュームファイル読み込みにより変更された場合
			sprintf(buf2, "Vol.[%d] Resume Loaded.", (int)_WindowsVolume);
		else
			sprintf(buf2, "Resume Loaded.");
		if (_bCDGame)
		{
			switch (CDROM_GetCDInstall())
			{
				case 1: //データトラックインストール済み
					sprintf(buf2, "%s Icd: %s", buf2,TOCDB_GetGameTitle());
					break;
				case 2: //データ＆音楽トラックフルインストール済み
					if (CDIF_GetBadInstalled())
						sprintf(buf2, "%s Bcd: %s", buf2,TOCDB_GetGameTitle()); //古いOotakeでリッピング失敗のイメージファイルの場合。v2.31追加
					else
						sprintf(buf2, "%s Fcd: %s", buf2,TOCDB_GetGameTitle());
					break;
				default:
					sprintf(buf2, "%s CD: %s", buf2,TOCDB_GetGameTitle());
					break;
			}
		}
		PRINTF("%s", buf2);
		if (!_bCDGame)
			PRINTF_SetSaveLoadMessage(); //PRINTFメッセージの表示時間を少し短くする
		fclose(fp);
	}
	else
		_bResumeMode = FALSE;
	if (_bAutoResumeMode)
		_bResumeMode = TRUE;
}


//Kitao追加。レジュームモードの場合、レジュームが解除されることを確認する処理。
static BOOL
non_resume_ok()
{
	BOOL	ret;

	if (_bResumeMode == FALSE) //レジュームモードでなければ確認OKですぐ帰る。
		return TRUE;

	MAINBOARD_Pause(TRUE);
	WINMAIN_ShowCursor(TRUE);
	screenUpdate(); //ダイアログ表示などで黒くなった部分を再描画。
	rewind(stdin); //キーバッファのクリア。resumeモード時に確認ダイアログが出るので、そこで誤入力を防ぐため必要。v2.26
	if (MessageBox(WINMAIN_GetHwnd(),"\"Resume Mode\" is temporarily invalidated. OK?    ",
					 "Ootake", MB_YESNO) == IDYES)
		ret = TRUE;
	else
		ret = FALSE;
	return ret;
}

//Kitao追加。レジュームを一時停止する処理
static void
non_resume_temporarily()
{
	char	buf2[256];

	if (_bResumeMode == FALSE)
		return;

	_bResumeMode = FALSE;
	updateMenu();

	if (_fpPlayRecord == NULL)
	{
		if (_bCDGame)
		{
			sprintf(buf2, "Non-Resume Mode.");
			switch (CDROM_GetCDInstall())
			{
				case 1: //データトラックインストール済み
					sprintf(buf2, "%s Icd: %s", buf2,TOCDB_GetGameTitle());
					break;
				case 2: //データ＆音楽トラックフルインストール済み
					if (CDIF_GetBadInstalled())
						sprintf(buf2, "%s Bcd: %s", buf2,TOCDB_GetGameTitle()); //古いOotakeでリッピング失敗のイメージファイルの場合。v2.31追加
					else
						sprintf(buf2, "%s Fcd: %s", buf2,TOCDB_GetGameTitle());
					break;
				default:
					sprintf(buf2, "%s CD: %s", buf2,TOCDB_GetGameTitle());
					break;
			}
			PRINTF("%s", buf2);
		}
		else
			PRINTF("Non-Resume Mode.");
		PRINTF_SetSaveLoadMessage(); //PRINTFメッセージの表示時間を少し短くする
	}
}


/*----------------------------------------------------------------------------
	[save_state]
----------------------------------------------------------------------------*/
//Kitao更新。番号付きで保存できるようにした。v0.51
static BOOL
save_state(
	Sint32	number)
{
	char	buf[MAX_PATH+1];
	FILE*	fp;
	char	c[2+1] = "";
	char	ms[256];

	SetSaveStateFileName(number, buf); //Kitao追加。ステートセーブ用のファイル名を設定。saveフォルダに保存するようにした。

	if (number > 0) //Kitao追加。番号付き保存の場合
	{
		sprintf(c, "%d", (int)number);
		//v0.94追加。すでに同じ番号のセーブファイルがあった場合は、上書きしていいかどうかを確認する。
		if ((fp = fopen(buf, "rb")) != NULL)
		{
			MAINBOARD_LoadScreenBuf(number, fp); //ステートのスクリーンショットを表示する
			fclose(fp);
			sprintf(ms, "There is already the \"State #%d File\".    \nOverwrite Ok?", (int)number);
			if (MessageBox(WINMAIN_GetHwnd(), ms, "Ootake", MB_YESNO) != IDYES)
			{
				SCREEN_SetMessageText(""); //"Former File"メッセージを消す。HideMessageモードのときのために必要
				PRINTF("Canceled SaveState.");
				MAINBOARD_RestoreScreenBuf(); //スクリーンバッファを復元する。PRINTFメッセージをすぐに表示するためにも必要。
				return FALSE;
			}
			else
			{
				SCREEN_SetMessageText(""); //"Former File"メッセージを消す。HideMessageモードのときのために必要
				MAINBOARD_RestoreScreenBuf(); //スクリーンバッファを復元する。
			}
		}
	}

	fp = fopen(buf, "wb");
	if (fp != NULL)
	{
		if (number > 0) //Kitao追加。番号付き保存の場合
			PRINTF("State Saved #%s.", c);
		else
			PRINTF("State Saved.");
		PRINTF_SetSaveLoadMessage(); //PRINTFメッセージの表示時間を少し短くする。v1.58追加
		MAINBOARD_SaveState(fp);
		fclose(fp);
		//Kitao追加。ステートセーブと同時にレジュームファイルも更新しておくようにした。
		save_resume();
		//Kitao追加。ステートセーブと同時にバックアップラム(＆MB128も)のファイルも更新しておくようにした。
		save_bram(); //MB128も保存される
		//Kitao追加。メニュー表示を更新
		updateMenu();
		return TRUE;
	}

	if (number > 0) //Kitao追加。番号付き保存の場合
		PRINTF("Couldn't Save #%s\"\\save\\%s_state%s.dat\".", c, _GameFileNameBuf, c);
	else
		PRINTF("Couldn't Save \"\\save\\%s_state.dat\".", _GameFileNameBuf);
	screenUpdate(); //Kitao追加v0.64。PRINTFメッセージをすぐに表示するため必要
	return FALSE;
}


/*----------------------------------------------------------------------------
	[load_state]
----------------------------------------------------------------------------*/
//Kitao更新。番号付きで読込できるようにした。v0.51
static BOOL
load_state(
	Sint32	number)
{
	char	buf[MAX_PATH+1];
	FILE*	fp;
	char	c[2+1] = "";

	//レジュームモードの場合、レジュームが解除されることを確認する。
	if (non_resume_ok() == FALSE)
		return FALSE;

	/* 記録中・再生中だった場合は終了する */
	end_recording();
	end_playrecord();

	SetSaveStateFileName(number, buf); //Kitao追加。ステートセーブ用のファイル名を設定。saveフォルダに保存するようにした。
	if (number > 0) //Kitao追加。番号付き保存の場合
		sprintf(c, "%d", (int)number);

	fp = fopen(buf, "rb");
	if (fp != NULL)
	{
		if (number > 0) //Kitao追加。番号付き保存の場合
			PRINTF("State Loaded #%s.", c);
		else
			PRINTF("State Loaded.");
		PRINTF_SetSaveLoadMessage(); //PRINTFメッセージの表示時間を少し短くする。v1.58追加
		//Kitao追加。ステートロードする前までの内容でBRAMファイルを更新。
		save_bram();
		//Kitao追加。レジュームモードだった場合、一時的に解除する。
		save_resume(); //レジューム一時停止前に、ここまでの状態をレジュームファイルに保存。
		non_resume_temporarily();
		MAINBOARD_LoadState(fp);
		fclose(fp);
		//Kitao追加。メニュー表示を更新
		updateMenu();
		return TRUE;
	}

	if (number > 0) //Kitao追加。番号付き保存の場合
		PRINTF("Couldn't Load #%s\"\\save\\%s_state%s.dat\".", c, _GameFileNameBuf, c);
	else
		PRINTF("Couldn't Load \"\\save\\%s_state.dat\".", _GameFileNameBuf);
	screenUpdate(); //Kitao追加v0.64。PRINTFメッセージをすぐに表示するため必要
	return FALSE;
}


/*----------------------------------------------------------------------------
	[recording_gameplay]
----------------------------------------------------------------------------*/
static BOOL
recording_gameplay(
	Sint32	number)
{
	char	buf[MAX_PATH+1];
	FILE*	fp;
	char	c[2+1] = "";
	char	ms[256];

	if (_fpRecording == NULL)
	{	//記録開始
		end_playrecord(); //再生中だった場合は終了する

		SetRecordFileName(number, buf);
		sprintf(c, "%d", (int)number);

		//Kitao追加。すでに同じ番号のセーブファイルがあった場合は、上書きしていいかどうかを確認する。
		if ((fp = fopen(buf, "rb")) != NULL)
		{
			MAINBOARD_LoadScreenBuf(0, fp); //ステートのスクリーンショットを表示する
			fclose(fp);
			if (number > 0)
				sprintf(ms, "There is already the \"Record #%d File\".    \nOverwrite Ok?", (int)number);
			else
				sprintf(ms, "There is already the \"Record File\".    \nOverwrite Ok?");
			if (MessageBox(WINMAIN_GetHwnd(), ms, "Ootake", MB_YESNO) != IDYES)
			{
				SCREEN_SetMessageText(""); //"Former File"メッセージを消す。HideMessageモードのときのために必要
				PRINTF("Canceled Recording.");
				MAINBOARD_RestoreScreenBuf(); //スクリーンバッファを復元する。PRINTFメッセージをすぐに表示するためにも必要。
				return FALSE;
			}
			else
			{
				SCREEN_SetMessageText(""); //"Former File"メッセージを消す。HideMessageモードのときのために必要
				MAINBOARD_RestoreScreenBuf(); //スクリーンバッファを復元する。
			}
		}

		_fpRecording = fopen(buf, "wb");
		if (_fpRecording != NULL)
		{
			_InputRecordingNumber = number;
			if (number > 0) //Kitao追加。番号付き保存の場合
				PRINTF("Start Recording #%s.", c);
			else
				PRINTF("Start Recording.");
			PRINTF_SetSaveLoadMessage(); //PRINTFメッセージの表示時間を少し短くする
			MAINBOARD_SaveState(_fpRecording);
			INPUT_Record(TRUE);
			MENU_ChangeItemText(_FileMenu, WM_RECORDING_GAMEPLAY, "End Recording\tF5");//Kitao更新
			return TRUE;
		}
	}
	else
	{	//記録終了
		end_recording();
		return TRUE;
	}

	if (number > 0) //Kitao追加。番号付き保存の場合
		PRINTF("Couldn't Recording #%s\"\\save\\%s_record%s.dat\".", c, _GameFileNameBuf, c);
	else
		PRINTF("Couldn't Recording \"\\record\\%s_record.dat\".", _GameFileNameBuf);
	return FALSE;
}


/*----------------------------------------------------------------------------
	[playrecord_gameplay]
----------------------------------------------------------------------------*/
static BOOL
playrecord_gameplay(
	Sint32	number)
{
	char	buf[MAX_PATH+1];
	char	c[2+1] = "";

	if (_fpRecording != NULL) //記録中だった場合、無効とする。v2.15追加
	{
		PRINTF("No. It is Recording Now. ( [F5]=Stop )");
		return FALSE;
	}

	if (_fpPlayRecord != NULL) //再生中だった場合
	{
		end_playrecord();
		if (number == 0) //F6キーか"End PlayRecord"メニューの場合、再生を止めて帰る。
			return TRUE;
	}

	//レジュームモードの場合、レジュームが解除されることを確認する。
	if (non_resume_ok() == FALSE)
		return FALSE;

	SetRecordFileName(number, buf);
	if (number > 0) //Kitao追加。番号付き保存の場合
		sprintf(c, "%d", (int)number);

	_fpPlayRecord = fopen(buf, "rb");
	if (_fpPlayRecord != NULL)
	{
		_InputPlayRecordNumber = number;
		if (number > 0) //Kitao追加。番号付き保存の場合
			PRINTF("Start Play Record #%s.", c);
		else
			PRINTF("Start Play Record.");
		PRINTF_SetSaveLoadMessage(); //PRINTFメッセージの表示時間を少し短くする
		//Kitao追加。ステートロードする前までの内容でBRAMファイルを更新。
		save_bram();
		//Kitao追加。レジュームモードだった場合、一時的に解除する。
		save_resume(); //レジューム一時停止前に、ここまでの状態をレジュームファイルに保存。
		non_resume_temporarily();
		MAINBOARD_LoadState(_fpPlayRecord);
		INPUT_ReadBuffer(_fpPlayRecord);
		INPUT_PlayRecord(TRUE);
		MENU_ChangeItemText(_FileMenu, WM_PLAYRECORD_GAMEPLAY, "End &Play Record\tF6");
		updateMenu(); //メニューを更新
		return TRUE;
	}

	if (number > 0) //Kitao追加。番号付き保存の場合
		PRINTF("Couldn't Play Record #%s\"\\record\\%s_record%s.dat\".", c, _GameFileNameBuf, c);
	else
		PRINTF("Couldn't Play Record \"\\record\\%s_record.dat\".", _GameFileNameBuf);
	return FALSE;
}


//Kitao追加。v2.15
static BOOL
moverecord_gameplay(
	Sint32	number) //必ず番号付き(1以上)で呼び出す。
{
	char	buf0[MAX_PATH+1];
	char	buf[MAX_PATH+1];
	FILE*	fp;
	char	c[2+1] = "";
	char	ms[256];

	end_recording(); //記録中だった場合は終了する
	end_playrecord(); //再生中だった場合は終了する

	SetRecordFileName(0, buf0);
	SetRecordFileName(number, buf);
	sprintf(c, "%d", (int)number);

	//すでに同じ番号のセーブファイルがあった場合は、上書きしていいかどうかを確認する。
	if ((fp = fopen(buf, "rb")) != NULL)
	{
		MAINBOARD_LoadScreenBuf(0, fp); //ステートのスクリーンショットを表示する
		fclose(fp);
		sprintf(ms, "There is already the \"Record #%d File\".    \nOverwrite Ok?", (int)number);
		if (MessageBox(WINMAIN_GetHwnd(), ms, "Ootake", MB_YESNO) != IDYES)
		{
			SCREEN_SetMessageText(""); //"Former File"メッセージを消す。HideMessageモードのときのために必要
			PRINTF("Canceled Move.");
			MAINBOARD_RestoreScreenBuf(); //スクリーンバッファを復元する。PRINTFメッセージをすぐに表示するためにも必要。
			return FALSE;
		}
		else
		{
			remove(buf);
			SCREEN_SetMessageText(""); //"Former File"メッセージを消す。HideMessageモードのときのために必要
			MAINBOARD_RestoreScreenBuf(); //スクリーンバッファを復元する。
		}
	}

	if (CopyFile(buf0, buf, FALSE) != 0)
	{
		remove(buf0);
		PRINTF("Moved \"Default Record Data\" to #%s.", c);
		screenUpdate(); //メッセージをすぐ表示
		updateMenu(); //メニューを更新
		return TRUE;
	}

	PRINTF("Couldn't Copy Record #%s \"\\record\\%s_record%s.dat\".", c, _GameFileNameBuf, c);
	return FALSE;
}


/*----------------------------------------------------------------------------
	[load_memory_values]
----------------------------------------------------------------------------*/
static BOOL
load_memory_values(
	const char*		pAppPath,
	const char*		pGameFilePathName)
{
	FILE*	p;
	char	buf[MAX_PATH+1];
	char	linebuf[256];
	char	value[256];
	char*	c;
	Uint32	addr;
	Uint32	data;

	strcpy(buf, _AppPath);
	if (strrchr(pGameFilePathName, '\\') != NULL)
		strcat(buf, strrchr(pGameFilePathName, '\\')+1);
	strcat(buf, "_PM.TXT");

	p = fopen(buf, "rt");
	if (p == NULL)
	{
		PRINTF("Failed loading partial memory data from file \"%s\".", buf);
		return FALSE;
	}
		
	while (fgets(linebuf, sizeof(linebuf), p) != NULL)
	{
		if (strstr(linebuf, "MAINRAM") == NULL && 
			strstr(linebuf, "BUFFERRAM") == NULL &&
			strstr(linebuf, "ARCADERAM") == NULL) continue;

		c = strchr(linebuf, ' ');
		if (c == NULL)	continue;
		while (*c == ' ') ++c;

		sprintf(value, "0x%s", c);
		sscanf(value, "%lx %lx", &addr, &data);

		if (strstr(linebuf, "MAINRAM"))
		{
			MAINBOARD_ChangeMemoryValue(MAINBOARD_MAINRAM, addr, (Uint8)data);
			PRINTF("APP: $%02lX is written to offset $%04lX of MAIN RAM.", data, addr);
		}
		else if (strstr(linebuf, "BUFFERRAM"))
		{
			MAINBOARD_ChangeMemoryValue(MAINBOARD_BUFFERRAM, addr, (Uint8)data);
			PRINTF("APP: $%02lX is written to offset $%05lX of BUFFER RAM.", data, addr);
		}
		else if (strstr(linebuf, "ARCADERAM"))
		{
			MAINBOARD_ChangeMemoryValue(MAINBOARD_BUFFERRAM, addr, (Uint8)data);
			PRINTF("APP: $%02lX is written to offset $%06lX of ARCADE RAM.", data, addr);
		}
	}

	fclose(p);
	return TRUE;
}


//Kitao追加。サンプルレートを変更するときはここを呼ぶ。
//44100Hz固定にしたため、現在未使用。
static void
changeAudioSampleRate(
	Sint32	sampleRate)
{
	if (APU_SetSampleRate(sampleRate))
	{
		_SoundSampleRate = sampleRate;
		updateMenu(); //メニュー表示を更新
		CONFIG_Set("[APP SOUND] Sound Sample Rate", &_SoundSampleRate, sizeof(_SoundSampleRate));
		
		PRINTF("APU_SetSampleRate(%ld)", _SoundSampleRate);
	}
}


//Kitao追加。バッファサイズを変更するときはここを呼ぶ。
static BOOL
changeAudioBufferSize(
	Sint32	bufSize)
{
	_SoundBufferSize = bufSize;
	updateMenu(); //メニュー表示を更新
	CONFIG_Set("[APP SOUND] Sound Buffer Size", &_SoundBufferSize, sizeof(_SoundBufferSize));

	if (_BigSoundBuffer == 1)
		bufSize *= BigSoundBufferRate;
	else if (_BigSoundBuffer == 2)
		bufSize *= MostBigSoundBufferRate;

	if (APU_SetBufferSize(bufSize))
	{	//成功
		MAINBOARD_SetBigSoundBuffer(_BigSoundBuffer);//Kitao追加。MainBoard.cにもモード変更を伝える
		PRINTF("Set Audio Buffer (%ld)", bufSize); //Kitao更新
		return TRUE;
	}
	else//失敗
		return FALSE;
}


//Kitao追加。サウンドの初期化設定を変更するときはここを呼ぶ。
static BOOL
changeSoundInitializeType(
	Sint32	soundType)
{
	Sint32 oldSoundType = _SoundType;

	//ハード環境によっては、変更で強制終了の可能性もあるので、セーブデータを更新しておく。
	end_playrecord(); end_recording(); save_resume(); save_bram();

	APP_SetSoundType(soundType);

	if (changeAudioBufferSize(_SoundBufferSize)) //バッファサイズのときの処理を再利用。メニュー表示も更新される。
	{	//成功
		return TRUE;
	}
	else//失敗
	{
		PRINTF("ERROR: Sound Initialize Type (%ld)", soundType);
		APP_SetSoundType(oldSoundType);
		updateMenu(); //メニュー表示を更新
		return FALSE;
	}
}


//Kitao追加。サウンドや入力の設定を反映させる処理をサブルーチン化。MAINBORD_Init()をしたあとは必ず必要。
static void
set_userSetting()
{
	int		i;
	Sint32	bufSize = _SoundBufferSize;
	Sint32	prevBigSoundBuffer = _BigSoundBuffer;

	//スプライト,BG表示をデフォルトに戻す。v2.06追加
	_bSpriteLayer = TRUE;
	_bSprite2Layer = TRUE;
	_bBGLayer     = TRUE;
	_bBG2Layer    = TRUE;
	VDC_SetLayer();

	//前回、自動的にビッグサウンドバッファモードにしていた場合(HESファイル)は元に戻す。
	if (_bAutoBigSoundBuffer)
		_BigSoundBuffer = 0;

	//HESファイルを開いた場合は自動的にビッグサウンドバッファモードにする。v0.82
	if ((_bHesFile)&&(_BigSoundBuffer == 0))
	{
		if (prevBigSoundBuffer == 2) //前回Mostビッグ
		{
			PRINTF("Set Most BigAudioBuffer Mode.");
			_BigSoundBuffer = 2;
		}
		else
		{
			PRINTF("Set BigAudioBuffer Mode.");
			_BigSoundBuffer = 1;
		}
		_bAutoBigSoundBuffer = TRUE;
	}
	else
		_bAutoBigSoundBuffer = FALSE;

	// サウンドを設定のモードに変更する 
	if (_BigSoundBuffer == 1)
		bufSize *= BigSoundBufferRate;
	else if (_BigSoundBuffer == 2)
		bufSize *= MostBigSoundBufferRate;
	if (!MAINBOARD_ChangeSoundMode(bufSize, _SoundSampleRate, _SoundApuMasterVolume, FALSE))
	{
		// 失敗したらデフォルトに戻す。 ※//Kitao変更。デフォルトの値を変更
		_SoundType			= 1;//Kitao追加
		_SoundBufferSize	= 1664;
		_BigSoundBuffer		= 0;//Kitao追加
		_SoundSampleRate	= 44100;

		if (!MAINBOARD_ChangeSoundMode(_SoundBufferSize, _SoundSampleRate, _SoundApuMasterVolume, FALSE))
		{
			//PRINTF("APP_Init: error setting sound mode from configuration file %s.", APP_CONFIG_FILENAME);
			//PRINTF("APP_Init: there will be no sound during emulation.");
			PRINTF("APP_Init: Sound Board Initialize Error.");
		}
		else
		{
			CONFIG_Set("[APP SOUND] Sound Initialize Type", &_SoundType, sizeof(_SoundType));//Kitao追加
			CONFIG_Set("[APP SOUND] Sound Buffer Size", &_SoundBufferSize, sizeof(_SoundBufferSize));
			CONFIG_Set("[APP SOUND] Sound Sample Rate", &_SoundSampleRate, sizeof(_SoundSampleRate));
			CONFIG_Set("[APP SOUND] APU Master Volume", &_SoundApuMasterVolume, sizeof(_SoundApuMasterVolume));
			CONFIG_Set("[APP SOUND] PSG Volume", &_SoundPsgVolume, sizeof(_SoundPsgVolume));//Kitao追加。PSGも個別に音量設定できるようにした。
			CONFIG_Set("[APP SOUND] ADPCM Volume", &_SoundAdpcmVolume, sizeof(_SoundAdpcmVolume));
			CONFIG_Set("[APP SOUND] CD Volume", &_SoundCdVolume, sizeof(_SoundCdVolume));
		}
	}

	CONFIG_Set("[APP SOUND] Sound Sample Rate", &_SoundSampleRate, sizeof(_SoundSampleRate));
	CONFIG_Set("[APP SOUND] Sound Buffer Size", &_SoundBufferSize, sizeof(_SoundBufferSize));

	MAINBOARD_SetBigSoundBuffer(_BigSoundBuffer);//Kitao追加。MainBoard.cにもモード変更を伝える

	PSG_SetVolumeEffect(_VolumeEffect);//Kitao追加
	ADPCM_SetVolumeEffect(_VolumeEffect);//Kitao追加
	CDROM_SetVolumeEffect(_VolumeEffect);//Kitao追加
	for (i=0; i<6; i++)
		PSG_SetMutePsgChannel(i, FALSE);//Kitao追加

	//HESファイルを開いた場合、曲間無音作成を有効にする。HES以外を開いた場合は解除。v1.29追加
	JOYPAD_SetMakeHesNoSound(_bHesFile);

	// 入力を設定どおりにする 
	APP_SetInputConfiguration();
}


//Kitao追加。画像表示開始前に１秒強CPUを動かす。実機同様に、起動時の画面乱れを見せないようにした。v0.90更新。
static void
first_run_cpu()
{
	int n;
	int	scanLine;

	if (_bUseVideoSpeedUpButton)
		MAINBOARD_SetFastForwarding(0, _bSoundAjust, FALSE); //早回しボタンを離した状態にして起動

	run_emulator(TRUE); //APU等をスタートさせる。メニュー表示の更新等も行われる。

	SCREEN_Clear(); //画面クリア。メニュー表示更新後にしないとフルスクリーン時にメニューに被った部分にゴミが出る。v2.00

	if (_bResumeMode)
		return; //レジュームモードのときはここではCPUを動かさない

	if (_bCDGame)
	{	//CDゲーム(システムカード)のときは、素早く表示。
		for (n=1; n<=2; n++) //2フレームぶん
		{
			MAINBOARD_IncSystemTime();
			for (scanLine=0; scanLine<=MAX_SCANLINE-1; scanLine++)
				VDC_AdvanceLine(NULL, 1);
			SCREEN_WaitVBlank(FALSE); //垂直帰線期間を待つ。描画は行わない。
		}
	}
	else
	{	//Huカードの場合、実機同様にして画面の乱れを見せない。
		for (n=1; n<=69; n++) //69フレームぶん。シュビビンマン２起動時にメサイヤのロゴは表示されないのが実機と同じ動作。あっぱれゲートボール、Ｆ１サーカス等で合わせた。
		{
			MAINBOARD_IncSystemTime();
			for (scanLine=0; scanLine<=MAX_SCANLINE-1; scanLine++)
				VDC_AdvanceLine(NULL, 1);
			SCREEN_WaitVBlank(FALSE); //垂直帰線期間を待つ。描画は行わない。
		}
	}
}


//Kitao追加。2ボタンパッドの設定を初期化する。
static void
initPadConfiguration2()
{
	int 	i;

	for (i=1; i<=5; i++)
	{
		_PcePad2[i].buttonU		= 200+i*100; //上キーの設定。アスキーコード(1～255)。Windowsジョイスティック１(300～399)。同２(400～499)。以下５本まで同様。
		_PcePad2[i].buttonR		= 201+i*100; //右キーの設定
		_PcePad2[i].buttonD		= 202+i*100; //下キーの設定
		_PcePad2[i].buttonL		= 203+i*100; //左キーの設定
		_PcePad2[i].buttonSel	= 250+i*100; //Selectボタンの設定。350=ジョイパッド１のボタン1
		_PcePad2[i].buttonRun	= 251+i*100; //Runボタンの設定
		_PcePad2[i].button1		= 252+i*100; //Iボタンの設定
		_PcePad2[i].button2		= 253+i*100; //IIボタンの設定
		_PcePad2[i].button3		= -1;
		_PcePad2[i].button4		= -1;
		_PcePad2[i].button5		= -1;
		_PcePad2[i].button6		= -1;
	}
}

//Kitao追加。3ボタンパッドの設定を初期化する。
static void
initPadConfiguration3()
{
	int 	i;

	for (i=1; i<=5; i++)
	{
		_PcePad3[i].buttonU		= 200+i*100; //上キーの設定。アスキーコード(1～255)。Windowsジョイスティック１(300～399)。同２(400～499)。以下５本まで同様。
		_PcePad3[i].buttonR		= 201+i*100; //右キーの設定
		_PcePad3[i].buttonD		= 202+i*100; //下キーの設定
		_PcePad3[i].buttonL		= 203+i*100; //左キーの設定
		_PcePad3[i].buttonSel	= 254+i*100; //Selectボタンの設定。350=ジョイパッド１のボタン1
		_PcePad3[i].buttonRun	= 255+i*100; //Runボタンの設定
		_PcePad3[i].button1		= 251+i*100; //Iボタンの設定
		_PcePad3[i].button2		= 252+i*100; //IIボタンの設定
		_PcePad3[i].button3		= 253+i*100; //III[Run]ボタンの設定
		_PcePad3[i].button4		= 250+i*100; //IV[Select]ボタンの設定
		_PcePad3[i].button5		= -1;
		_PcePad3[i].button6		= -1;
	}
}

//Kitao追加。6ボタンパッドの設定を初期化する。
static void
initPadConfiguration6()
{
	int 	i;

	for (i=1; i<=5; i++)
	{
		_PcePad6[i].buttonU		= 200+i*100; //上キーの設定。アスキーコード(1～255)。Windowsジョイスティック１(300～399)。同２(400～499)。以下５本まで同様。
		_PcePad6[i].buttonR		= 201+i*100; //右キーの設定
		_PcePad6[i].buttonD		= 202+i*100; //下キーの設定
		_PcePad6[i].buttonL		= 203+i*100; //左キーの設定
		_PcePad6[i].buttonSel	= 254+i*100; //Selectボタンの設定
		_PcePad6[i].buttonRun	= 255+i*100; //Runボタンの設定
		_PcePad6[i].button1		= 251+i*100; //Iボタンの設定
		_PcePad6[i].button2		= 252+i*100; //IIボタンの設定
		_PcePad6[i].button3		= 253+i*100; //IIIボタンの設定
		_PcePad6[i].button4		= 256+i*100; //IVボタンの設定
		_PcePad6[i].button5		= 257+i*100; //Vボタンの設定
		_PcePad6[i].button6		= 250+i*100; //VIボタンの設定
	}
}


//Kitao追加。１つ目の2ボタンパッドの設定をキーボードで初期化する。
static void
initPad1ConfigurationKB2()
{
	_PcePad2[1].buttonU		= DIK_UP; //上キーの設定。アスキーコード(1～255)。Windowsジョイスティック１(300～399)。同２(400～499)。以下５本まで同様。
	_PcePad2[1].buttonR		= DIK_RIGHT; //右キーの設定
	_PcePad2[1].buttonD		= DIK_DOWN; //下キーの設定
	_PcePad2[1].buttonL		= DIK_LEFT; //左キーの設定
	_PcePad2[1].buttonSel	= DIK_TAB; //Selectボタンの設定
	_PcePad2[1].buttonRun	= DIK_RETURN; //Runボタンの設定
	_PcePad2[1].button1		= DIK_N; //Iボタンの設定
	_PcePad2[1].button2		= DIK_SPACE; //IIボタンの設定
}

//Kitao追加。１つ目の3ボタンパッドの設定をキーボードで初期化する。
static void
initPad1ConfigurationKB3()
{
	_PcePad3[1].buttonU		= DIK_UP; //上キーの設定。アスキーコード(1～255)。Windowsジョイスティック１(300～399)。同２(400～499)。以下５本まで同様。
	_PcePad3[1].buttonR		= DIK_RIGHT; //右キーの設定
	_PcePad3[1].buttonD		= DIK_DOWN; //下キーの設定
	_PcePad3[1].buttonL		= DIK_LEFT; //左キーの設定
	_PcePad3[1].buttonSel	= DIK_TAB; //Selectボタンの設定
	_PcePad3[1].buttonRun	= DIK_RETURN; //Runボタンの設定
	_PcePad3[1].button1		= DIK_N; //Iボタンの設定
	_PcePad3[1].button2		= DIK_SPACE; //IIボタンの設定
	_PcePad3[1].button3		= DIK_B; //III[Run]ボタンの設定
	_PcePad3[1].button4		= DIK_V; //IV[Select]ボタンの設定
}

//Kitao追加。１つ目の6ボタンパッドの設定をキーボードで初期化する。
static void
initPad1ConfigurationKB6()
{
	_PcePad6[1].buttonU		= DIK_UP; //上キーの設定。アスキーコード(1～255)。Windowsジョイスティック１(300～399)。同２(400～499)。以下５本まで同様。
	_PcePad6[1].buttonR		= DIK_RIGHT; //右キーの設定
	_PcePad6[1].buttonD		= DIK_DOWN; //下キーの設定
	_PcePad6[1].buttonL		= DIK_LEFT; //左キーの設定
	_PcePad6[1].buttonSel	= DIK_TAB; //Selectボタンの設定
	_PcePad6[1].buttonRun	= DIK_RETURN; //Runボタンの設定
	_PcePad6[1].button1		= DIK_N; //Iボタンの設定
	_PcePad6[1].button2		= DIK_SPACE; //IIボタンの設定
	_PcePad6[1].button3		= DIK_B; //IIIボタンの設定
	_PcePad6[1].button4		= DIK_G; //IVボタンの設定
	_PcePad6[1].button5		= DIK_H; //Vボタンの設定
	_PcePad6[1].button6		= DIK_J; //VIボタンの設定
}


//Kitao追加。volumeは1～120（※101～120は100と同じWindowsボリュームを使用しソフトで音量を上げる）。-1ならWindowsのデフォルトボリューム値を使う。v2.62更新。
static void
setWaveVolume(
	DWORD	volume)
{
	DWORD	v;

	if (volume == -1)
		v = _dwDefaultWindowsVolumeValue;
	else if (volume > 100) //v2.62追加。Huカードゲーム時(PSGオンリー)のみ、ボリューム120まで上げられるようにした。Windowsボリュームは最大100で101以上の場合、ソフト的に上げる。
		v = _dwWaveMaxVolume;  //ハード音量は最大値(ボリューム100と同じ)に。
	else
	{
		v = (DWORD)(_dwWaveMaxVolume * (double)_WindowsVolume/100.0 +0.5);
		if (v>_dwWaveMaxVolume) v=_dwWaveMaxVolume;
	}
	PSG_SetVolume(_SoundPsgVolume); //PSGのソフト音量調整。

	if (_hMix != INVALID_HANDLE_VALUE)
	{
		_MixerControlDU.dwValue = v;
		mixerSetControlDetails((HMIXEROBJ)_hMix, &_MixerControlD, MIXER_OBJECTF_HMIXER|MIXER_SETCONTROLDETAILSF_VALUE);
	}
}

//Kitao追加
static void
setWindowsVolume(
	int		volume)
{
	int		i;
	char	buf[100];
	char	buf2[3+1];

	_WindowsVolume = volume;
	if (_WindowsVolume == -1)
	{
		i = (int)((double)_dwDefaultWindowsVolumeValue/(double)_dwWaveMaxVolume * 100.0 +0.5);
		PRINTF("Use Windows Default Volume. [%d]", i);
	}
	else
	{
		strcpy(buf, "");
		//if (_WindowsVolume == 0)
		//else
		if (_WindowsVolume < 10)
			strcat(buf, ")");
		else
			for (i=0; i<_WindowsVolume/10; i++)
				strcat(buf, ")");
		sprintf(buf2, "%d", (int)_WindowsVolume);
		PRINTF("Volume %s [%s]", buf, buf2);
		_bWindowsVolumeChanged = TRUE;
	}
	PRINTF_SetSaveLoadMessage(); //PRINTFメッセージの表示時間を少し短くする。
	screenUpdate(); //メッセージをすぐに表示。特にジョイパッドのファンクションボタンで呼び出されたときに必要。v2.45
	setWaveVolume(_WindowsVolume);
	if (_WindowsVolume == -1) //※レジュームされた音量が極端だった場合を考えて、DefaultVolumeに合わせた時(volume==-1)のみレジュームの音量情報を削除。手動微調整してもレジューム前の音量に戻るほうが使いやすそうなので。
		_PrevWindowsVolume = -2; //レジューム時に退避していた音量情報を削除。手動で音量を変更したので次にゲームを変えたときも、今設定した音量を継続させる。

	updateVolumeMenu();
}

//Kitao追加
static void
setVolumeStep(
	int		num)
{
	_VolumeStep = num;
	CONFIG_Set("[APP SOUND] Windows Volume Step", &_VolumeStep, sizeof(_VolumeStep));

	PRINTF("Set Volume Step [%d].", _VolumeStep);
	switch (_VolumeStep)
	{
		case 10: MENU_CheckRadioItem(_VolStepMenu, WM_VOLUME_STEP10, WM_VOLUME_STEP1, WM_VOLUME_STEP10); break;
		case  8: MENU_CheckRadioItem(_VolStepMenu, WM_VOLUME_STEP10, WM_VOLUME_STEP1, WM_VOLUME_STEP8); break;
		case  6: MENU_CheckRadioItem(_VolStepMenu, WM_VOLUME_STEP10, WM_VOLUME_STEP1, WM_VOLUME_STEP6); break;
		case  5: MENU_CheckRadioItem(_VolStepMenu, WM_VOLUME_STEP10, WM_VOLUME_STEP1, WM_VOLUME_STEP5); break;
		case  4: MENU_CheckRadioItem(_VolStepMenu, WM_VOLUME_STEP10, WM_VOLUME_STEP1, WM_VOLUME_STEP4); break;
		case  3: MENU_CheckRadioItem(_VolStepMenu, WM_VOLUME_STEP10, WM_VOLUME_STEP1, WM_VOLUME_STEP3); break;
		case  2: MENU_CheckRadioItem(_VolStepMenu, WM_VOLUME_STEP10, WM_VOLUME_STEP1, WM_VOLUME_STEP2); break;
		case  1: MENU_CheckRadioItem(_VolStepMenu, WM_VOLUME_STEP10, WM_VOLUME_STEP1, WM_VOLUME_STEP1); break;
	}
}


//Kitao追加。CD初期起動処理のサブ。v2.24
static BOOL
cdIniOpenSub()
{
	char	srcFilePathName[MAX_PATH+1] = "";
	BOOL	bOk;

	strcpy(_OpenFilePathName, _AppPath);
	strcat(_OpenFilePathName, "SYSCARD.pce");
	strcpy(_GameFilePathName, _OpenFilePathName);
	strcpy(_GameFileNameBuf, ""); //_GameFileNameBufは、MAINBOARD_Initで使われるのでクリアしておく必要がある。
			
	bOk = MAINBOARD_Init(_OpenFilePathName);
	if (bOk)
		if (!MAINBOARD_GetSystemCard()) //SYSCARD.pceがシステムカードのイメージではなかった場合。v2.40
		{
			MAINBOARD_Deinit();
			_bInit = FALSE; //PRINTFによるメッセージ表示を初期化が完了するまで行わないようにするために必要。
			bOk = FALSE;
		}
	if (!bOk) //SYSCARD.pceがなかった場合か、SYSCARD.pceがシステムカードのイメージではなかった場合。v2.40
	{
		//システムカードのイメージファイルを指定してもらう
		MessageBox(WINMAIN_GetHwnd(), "To play CD-ROM games,\nplease select the \"SystemCard  ROM-image\".    ", "Ootake", MB_OK);
		while (TRUE)
		{
			if (file_dialog(2, WINMAIN_GetHwnd(), srcFilePathName, _GameFilePathName))
			{
				SetFileAttributes(_OpenFilePathName, FILE_ATTRIBUTE_NORMAL); //読み込み専用ファイルにしてあった場合解除
				if (CopyFile(srcFilePathName, _OpenFilePathName, FALSE)) //実行フォルダ内へコピー
				{
					SetFileAttributes(_OpenFilePathName, FILE_ATTRIBUTE_NORMAL); //読み込み専用ファイルにしてあった場合解除
					PRINTF("SystemCard ROM-image Set.");
				}
			}
			else
				return FALSE;
			//もう一度試みる
			if (MAINBOARD_Init(_OpenFilePathName))
				break; //正常に開けた
		}
	}
	//ゲーム名を_GameFileNameBufに設定しておく（ステートセーブ時等で利用）
	SetGameFileNameBuf(_bCDGame, _bHesFile, _GameFilePathName, _GameFileNameBuf, FALSE);
	return TRUE;
}

//Kitao追加。v2.64
static void
initShowOverscan()
{
	MAINBOARD_SetShowOverscanTop(_StartShowOverscanTop); //オーバースキャン領域表示を起動時設定に戻す。v1.43
	MAINBOARD_SetShowOverscanBottom(_StartShowOverscanBottom);
	MAINBOARD_SetShowOverscanLeft(_StartShowOverscanLeft);
	MAINBOARD_SetShowOverscanRight(_StartShowOverscanRight);
	if (MAINBOARD_GetShowOverscan()) //スタート時のオーバースキャン表示がオンなら、その設定を"Screen"メニューにも反映させる。
	{
		_ShowOverscanTop	= _StartShowOverscanTop;
		_ShowOverscanBottom	= _StartShowOverscanBottom;
		_ShowOverscanLeft	= _StartShowOverscanLeft;
		_ShowOverscanRight	= _StartShowOverscanRight;
	}
	_ShowOverscanHeight	= _StartShowOverscanHeight;
}

/*-----------------------------------------------------------------------------
	[Init]
		アプリケーションを初期化します。
-----------------------------------------------------------------------------*/
BOOL
APP_Init(
	int			argc,
	char**		argv)
{
	int					i;
	HWND				hWnd = WINMAIN_GetHwnd(); //Kitao追加
	UINT				id;
	MIXERCAPS			mc;
	MIXERLINE			ml;
	MIXERLINECONTROLS	mlc;
	MIXERCONTROL		mctrl[4];
	RECT			 	rc; //Kitao追加
	int 				kidouX; //Kitao追加
	int 				kidouY; //Kitao追加
	char				srcFilePathName[MAX_PATH+1] = ""; //Kitao追加
	char				fileName[MAX_PATH+1]; //Kitao追加
	OSVERSIONINFO		osInfo; //Kitao追加
	LPITEMIDLIST		pidl; //Kitao追加
	Sint32				_CDorHu = 0; //Kitao追加
	MSG					msg;
	FILE*				fp;
	char				softVersion[5] = "0.00"; //Kitao追加。バージョン情報。5バイト(4文字)固定。

#if defined(__GNUC__)
	puts("          Compiled with GCC version " __VERSION__);
#endif

	CHAR sStartDir[1024];
	GetCurrentDirectory( 1024, sStartDir );

	//Kitao追加。デスクトップフォルダの位置を取得（WAV等を保存時のデフォルトフォルダ）
	if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOPDIRECTORY, &pidl)))
	{
		SHGetPathFromIDList(pidl, _DesktopPath);
		CoTaskMemFree(pidl);
		strcat(_DesktopPath, "\\");
		*_DesktopPath = toupper(*_DesktopPath); //ドライブ名を確実に大文字にする。v2.27追加
	}
	// アプリケーションのディレクトリを保存しておく 
	strcpy(_AppPath, argv[0]);
	if (strrchr(_AppPath, '\\') != NULL)
		*(strrchr(_AppPath, '\\')+1) = '\0';
	*_AppPath = toupper(*_AppPath); //ドライブ名を確実に大文字にする。v2.24追加
	//Kitao追加。ZIPファイル解凍時のテンポラリファイル名を設定
	strcpy(_ZipTempFilePathName, _AppPath);
	strcat(_ZipTempFilePathName, "temp");
	CreateDirectory(_ZipTempFilePathName, NULL);//tempディレクトリがない場合作る
	strcat(_ZipTempFilePathName, "\\TEMP.pce");

	//Kitao追加。Vista,Win9x対策
	osInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osInfo);
	if (osInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) //Windows98/Me/95なら
		_bWindows9x = TRUE;
	if (osInfo.dwMajorVersion >= 6) //WindowsVista以上なら
	{
		_bWindowsVista = TRUE;
		if (((osInfo.dwMajorVersion == 6)&&(osInfo.dwMinorVersion >= 1))|| //Windows7(v6.1)以上なら。v2.20
			(osInfo.dwMajorVersion >= 7)) //とりあえず次のOS(v7?)もWin7と同じ扱いにしておく
				_bWindows7 = TRUE;

		//Vista以降の場合、dwmapi.dllをロードする。デスクトップウィンドウマネージャ(AERO処理に必要なもの)を無効にするために必要。v2.21
		_hDwmapiDll = LoadLibrary("dwmapi.dll");
		_FuncDwmIsCompositionEnabled = (FuncDwmIsCompositionEnabled)GetProcAddress(_hDwmapiDll, "DwmIsCompositionEnabled");
		_FuncDwmEnableComposition = (FuncDwmEnableComposition)GetProcAddress(_hDwmapiDll, "DwmEnableComposition");

		//Vista(Win7も)では"Program Files"以下に書き込まれたファイルは、自動でリダイレクトされて各ユーザーのフォルダに書き込まれるので、
		//その書き込み先のフォルダを調べて、_VirtualStorePathに入れておく。
		if ((*_AppPath == *_DesktopPath)&& //Windowsのシステムドライブ(DesktopPathのドライブ)内にOotakeがインストールされている場合のみ。v2.27追加
			((strstr(_AppPath,"\\Program Files\\") != NULL)||(strstr(_AppPath,"\\Program Files (x86)\\") != NULL))) //"Program Files"にインストールした場合のみ。自前のフォルダへインストールした場合はリダイレクトされない。
		{
			if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_LOCAL_APPDATA, &pidl)))
			{
				SHGetPathFromIDList(pidl, _VirtualStorePath);
				CoTaskMemFree(pidl);
				strcat(_VirtualStorePath, "\\VirtualStore\\");
				strcat(_VirtualStorePath, _AppPath + 3); //64bit版では"Program Files (x86)"になるので、それにも対応。v2.20
			}
		}
	}

	SetThreadExecutionState(ES_SYSTEM_REQUIRED | ES_CONTINUOUS); //Kitao追加。スタンバイを一時的無効に。環境によってジョイパッド操作でスタンバイ抑止が出来ない場合に必要。v2.22

	//サウンドカードのボリュームミキサー情報を取得。v1.61
	id = mixerGetNumDevs();
	if (id >= 1)
	{
		id = 0;
		ZeroMemory(&_MixerControlD, sizeof(_MixerControlD));
		ZeroMemory(&_MixerControlDU, sizeof(_MixerControlDU));
		ZeroMemory(&mc, sizeof(mc));
		ZeroMemory(&ml, sizeof(ml));
		ZeroMemory(&mlc, sizeof(mlc));
		ZeroMemory(&mctrl, sizeof(mctrl));
		if (mixerOpen(&_hMix, id, (DWORD)hWnd, 0, MIXER_OBJECTF_MIXER|CALLBACK_WINDOW) == MMSYSERR_NOERROR) //_hMixを設定
			if (mixerGetDevCaps(id, &mc, sizeof(mc)) == MMSYSERR_NOERROR)
			{
				ml.cbStruct = sizeof(MIXERLINE);
				ml.dwComponentType = MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT;
				mixerGetLineInfo((HMIXEROBJ)_hMix, &ml, MIXER_OBJECTF_HMIXER|MIXER_GETLINEINFOF_COMPONENTTYPE); //指定コンポーネントのミキサーライン情報を得る
				for (i=0; i<sizeof(mctrl)/sizeof(mctrl[0]); i++)
					mctrl[i].cbStruct = sizeof(MIXERCONTROL);
				mlc.cbStruct		= sizeof(MIXERLINECONTROLS);
				mlc.dwLineID		= ml.dwLineID;
				mlc.dwControlType	= MIXERCONTROL_CONTROLTYPE_VOLUME;
				mlc.cControls		= ml.cControls;
				mlc.cbmxctrl		= sizeof(MIXERCONTROL);
				mlc.pamxctrl		= (LPMIXERCONTROL)&mctrl[0];
				mixerGetLineControls((HMIXEROBJ)_hMix, &mlc, MIXER_OBJECTF_HMIXER|MIXER_GETLINECONTROLSF_ONEBYTYPE); //ミキサーラインのコントロールを得る
				for (i=0; i<(int)ml.cControls; i++)
					if (mctrl[i].dwControlType == MIXERCONTROL_CONTROLTYPE_VOLUME)
					{
						_dwWaveControlID = mctrl[i].dwControlID; //WAVEのコントロールIDを設定完了。
						_dwWaveMaxVolume = mctrl[i].Bounds.lMaximum; //WAVEの最大値を設定完了。
						break;
					}
				_MixerControlD.cbStruct = sizeof(MIXERCONTROLDETAILS);
				_MixerControlD.dwControlID = _dwWaveControlID;
				_MixerControlD.cChannels = 1;
				_MixerControlD.hwndOwner = NULL;
				_MixerControlD.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
				_MixerControlD.paDetails = (LPMIXERCONTROLDETAILS_UNSIGNED)&_MixerControlDU;
				mixerGetControlDetails((HMIXEROBJ)_hMix, &_MixerControlD, MIXER_OBJECTF_HMIXER|MIXER_GETCONTROLDETAILSF_VALUE);
				_dwDefaultWindowsVolumeValue = _MixerControlDU.dwValue; //起動時のWAVEボリュームの値を退避。
			}
	}

//Kitao更新。著作権については起動時に別フォーム（Startup.cpp）にて表示するようにしました。
//           法律よりも一番大切な「なぜ守らなくてはいけないのか」についてを中心に書かせていただきました。
//			 最も大切なのは気持ちだと思います。
//
//           現在、PCエンジンの実機（特にCD-ROM機）はもう寿命で故障が続出しています。
//           一部の悪意を持った人のせいで、多くの方にハッピーが生まれる可能性のあるPC2Eをこのままお蔵入り
//           させてしまうのは本当にもったいないと思い、僭越ながら後継版の公開に踏み切らせていただきました。
//           この場をお借りして、PC2Eの作者であるKiさんに大きな感謝を表します。
/*
#ifndef DEBUGBUILD
	// NEVER REMOVE THIS LINE OR THE PROJECT IS DEAD!!
	if (MessageBox(	hWnd,
					"Before using this program, please accept the following terms:\r\n"
					"\r\n"
					"1. This program must not be distributed with any copyrighted ROM images.\r\n"
					"\r\n"
					"2. This program must not be used with illegally obtained ROM images.\r\n"
					"   Use this program only with ROM images dumped by yourself.\r\n"
					"\r\n"
					"If you accept the terms above, click on the \"Yes\" button.\r\n"
					"If you do not, click on the \"No\" button.\r\n"
					"Do you accept the terms above?",
					"THIS PROGRAM IS NOT FOR PIRACY!!",
					MB_YESNO) != IDYES)	return FALSE;
#endif
*/

	//PRINF初期化
	PRINTF_Init();

	//コンフィグを初期化する。 Kitao更新。カレントCDドライブ設定の読み込み等が必要なため、ゲームファイル選択前にコンフィグを読み込むようにした。
	CONFIG_Init();

	//Kitao追加。PCエンジンパッド1～5を、Windowsジョイスティック1～5を使う設定で初期化する。
	initPadConfiguration2();
	initPadConfiguration3();
	initPadConfiguration6();

	//設定ファイルを読み込む。 
	if (!load_config(_AppPath))
	{	// 設定ファイルが読み込めなかった場合はデフォルトを設定する。 
		// ディスプレイ関連 
		CONFIG_Set("[APP DISPLAY] Screen Width", &_ScreenWidth, sizeof(_ScreenWidth));
		CONFIG_Set("[APP DISPLAY] Screen Height", &_ScreenHeight, sizeof(_ScreenHeight));
		CONFIG_Set("[APP DISPLAY] Full Screen Mode", &_bStartFullScreen, sizeof(_bStartFullScreen)); //Kitao更新。v0.55。起動時にフルスクリーンにするかどうかに変更。
		CONFIG_Set("[APP DISPLAY] Window TopMost", &_bWindowTopMost, sizeof(_bWindowTopMost)); //Kitao追加。v1.34
		CONFIG_Set("[APP DISPLAY] Window Activate", &_bWindowActivate, sizeof(_bWindowActivate)); //Kitao追加。v0.97
		CONFIG_Set("[APP DISPLAY] Window Pause", &_bInactivePause, sizeof(_bInactivePause)); //Kitao追加。v2.26
		CONFIG_Set("[APP DISPLAY] Gamma Value", &_GammaValue, sizeof(_GammaValue));//Kitao追加。v1.09
		CONFIG_Set("[APP DISPLAY] Brightness Value", &_BrightValue, sizeof(_BrightValue));//Kitao追加。v1.41
		CONFIG_Set("[APP DISPLAY] Scanline Type", &_ScanLineType, sizeof(_ScanLineType));//Kitao追加
		CONFIG_Set("[APP DISPLAY] Screenshot Scanline", &_ScreenshotScanLineType, sizeof(_ScreenshotScanLineType));//Kitao追加。v2.13
		CONFIG_Set("[APP DISPLAY] Scanline Density", &_ScanLineDensity, sizeof(_ScanLineDensity));//Kitao追加。v1.09
		CONFIG_Set("[APP DISPLAY] Optimize Gamma", &_bOptimizeGamma, sizeof(_bOptimizeGamma));//Kitao追加。v2.35
		CONFIG_Set("[APP DISPLAY] Stretched Mode", &_bStretched, sizeof(_bStretched));
		CONFIG_Set("[APP DISPLAY] FullStretched Mode", &_bFullStretched, sizeof(_bFullStretched));//Kitao追加。v1.24
		CONFIG_Set("[APP DISPLAY] Magnification", &_Magnification, sizeof(_Magnification));
		CONFIG_Set("[APP DISPLAY] FullMagnification", &_FullMagnification, sizeof(_FullMagnification));//Kitao追加。フルスクリーン時の_Magnification。
		CONFIG_Set("[APP DISPLAY] ScreenshotMagnification", &_ScreenshotMagnification, sizeof(_ScreenshotMagnification));//Kitao追加。v2.12
		CONFIG_Set("[APP DISPLAY] Full CustomWidth1", &_CustomWidth1, sizeof(_CustomWidth1));//Kitao追加。v1.35
		CONFIG_Set("[APP DISPLAY] Full CustomHeight1", &_CustomHeight1, sizeof(_CustomHeight1));//Kitao追加。v1.35
		CONFIG_Set("[APP DISPLAY] Full CustomWidth2", &_CustomWidth2, sizeof(_CustomWidth2));//Kitao追加。v1.35
		CONFIG_Set("[APP DISPLAY] Full CustomHeight2", &_CustomHeight2, sizeof(_CustomHeight2));//Kitao追加。v1.35
		CONFIG_Set("[APP DISPLAY] Full CustomWidth3", &_CustomWidth3, sizeof(_CustomWidth3));//Kitao追加。v1.35
		CONFIG_Set("[APP DISPLAY] Full CustomHeight3", &_CustomHeight3, sizeof(_CustomHeight3));//Kitao追加。v1.35
		CONFIG_Set("[APP DISPLAY] Resolution AutoChange", &_bResolutionAutoChange, sizeof(_bResolutionAutoChange));//Kitao追加。v1.35
		CONFIG_Set("[APP DISPLAY] Sync To 60Hz Fullscreen", &_bSyncTo60HzScreen, sizeof(_bSyncTo60HzScreen));//Kitao更新。保存名に"Fullscreen"が入っているがウィンドウ時にも使用。
		CONFIG_Set("[APP DISPLAY] VSync Adjust", &_VSyncAdjust, sizeof(_VSyncAdjust));//Kitao追加。v2.65
		CONFIG_Set("[APP DISPLAY] Force VSync", &_bForceVSync, sizeof(_bForceVSync));//Kitao追加。v2.41
		CONFIG_Set("[APP DISPLAY] Windows Auto VSync", &_WindowsAutoVSync, sizeof(_WindowsAutoVSync));//Kitao追加。v2.65
		CONFIG_Set("[APP DISPLAY] Draw Method", &_DrawMethod, sizeof(_DrawMethod));//Kitao追加。Direct3D用に追加
		CONFIG_Set("[APP DISPLAY] Direct3D Effect TV", &_D3DEffectTV, sizeof(_D3DEffectTV));//Kitao追加。Direct3D用に追加
		CONFIG_Set("[APP DISPLAY] Direct3D Effect RGB", &_D3DEffectRGB, sizeof(_D3DEffectRGB));//Kitao追加。Direct3D用に追加
		CONFIG_Set("[APP DISPLAY] Use VideoCard Memory", &_bUseVideoCardMemory, sizeof(_bUseVideoCardMemory));//Kitao追加。v1.31
		CONFIG_Set("[APP DISPLAY] Use System Memory Only Window", &_bUseSystemMemoryOnlyWindow, sizeof(_bUseSystemMemoryOnlyWindow));//Kitao追加。v1.53
		CONFIG_Set("[APP DISPLAY] FullScreen Color", &_FullScreenColor, sizeof(_FullScreenColor));//Kitao追加
		CONFIG_Set("[APP DISPLAY] Overscan Top", &_StartShowOverscanTop, sizeof(_StartShowOverscanTop));//Kitao追加。v1.43
		CONFIG_Set("[APP DISPLAY] Overscan Bottom", &_StartShowOverscanBottom, sizeof(_StartShowOverscanBottom));//Kitao追加。v1.43
		CONFIG_Set("[APP DISPLAY] Overscan Left", &_StartShowOverscanLeft, sizeof(_StartShowOverscanLeft));//Kitao追加。v1.43
		CONFIG_Set("[APP DISPLAY] Overscan Right", &_StartShowOverscanRight, sizeof(_StartShowOverscanRight));//Kitao追加。v1.43
		CONFIG_Set("[APP DISPLAY] Overscan Height", &_StartShowOverscanHeight, sizeof(_StartShowOverscanHeight));//Kitao追加。v1.43
		CONFIG_Set("[APP DISPLAY] Overscan HideBlack", &_bOverscanHideBlackBelt, sizeof(_bOverscanHideBlackBelt));//Kitao追加。v1.45
		CONFIG_Set("[APP DISPLAY] Hide Menu", &_bHideMenu, sizeof(_bHideMenu)); //Kitao追加。v0.62
		CONFIG_Set("[APP DISPLAY] Hide Message", &_bHideMessage, sizeof(_bHideMessage)); //Kitao追加。v0.64
		CONFIG_Set("[APP DISPLAY] Full Hide Message", &_bFullHideMessage, sizeof(_bFullHideMessage)); //Kitao追加。v0.64
		CONFIG_Set("[APP DISPLAY] Screenshot Dialog", &_bScreenshotDialog, sizeof(_bScreenshotDialog));//Kitao追加。v2.13
		CONFIG_Set("[APP DISPLAY] Disable Windows Aero", &_bDisableWindowsAero, sizeof(_bDisableWindowsAero));//Kitao追加。v2.21

		// サウンド関連 
		CONFIG_Set("[APP SOUND] Sound Initialize Type", &_SoundType, sizeof(_SoundType));//Kitao追加
		CONFIG_Set("[APP SOUND] Sound Buffer Size", &_SoundBufferSize, sizeof(_SoundBufferSize));
		CONFIG_Set("[APP SOUND] Sound Sample Rate", &_SoundSampleRate, sizeof(_SoundSampleRate));
		CONFIG_Set("[APP SOUND] Windows Volume Control", &_WindowsVolume, sizeof(_WindowsVolume));//Kitao追加。v1.61
		CONFIG_Set("[APP SOUND] Windows Volume Step", &_VolumeStep, sizeof(_VolumeStep));//Kitao追加。v2.45
		CONFIG_Set("[APP SOUND] APU Master Volume", &_SoundApuMasterVolume, sizeof(_SoundApuMasterVolume));
		CONFIG_Set("[APP SOUND] PSG Volume", &_SoundPsgVolume, sizeof(_SoundPsgVolume));//Kitao追加。PSGも個別に音量設定できるようにした。
		CONFIG_Set("[APP SOUND] ADPCM Volume", &_SoundAdpcmVolume, sizeof(_SoundAdpcmVolume));
		CONFIG_Set("[APP SOUND] CD Volume", &_SoundCdVolume, sizeof(_SoundCdVolume));
		CONFIG_Set("[APP SOUND] PSG Quality", &_PsgQuality, sizeof(_PsgQuality));//Kitao追加。v1.39
		CONFIG_Set("[APP SOUND] Auto Stereo", &_bAutoStereo, sizeof(_bAutoStereo));//Kitao追加。v1.15
		CONFIG_Set("[APP SOUND] WAV Next", &_bOutputWavNext, sizeof(_bOutputWavNext));//Kitao追加。v1.29
		CONFIG_Set("[APP SOUND] WAV Add No Sound", &_bOutputWavAddNoSound, sizeof(_bOutputWavAddNoSound));//Kitao追加。v1.29
		CONFIG_Set("[APP SOUND] CDDA Adjust Window", &_CddaAdjust, sizeof(_CddaAdjust));//Kitao追加。v2.32
		CONFIG_Set("[APP SOUND] CDDA Adjust FullScreen", _CddaAdjustFull, sizeof(_CddaAdjustFull));//Kitao追加。v2.32
		CONFIG_Set("[APP SOUND] CDDA Delay Frame", &_CddaDelayFrame, sizeof(_CddaDelayFrame));//Kitao追加。v2.33

		// 入力関連 Kitao更新。キーボード対応の_PcePadへ変更。
		CONFIG_Set("[APP INPUT] JoyPad Setting for 2Button", _PcePad2, sizeof(_PcePad2));
		CONFIG_Set("[APP INPUT] JoyPad Setting for 3Button", _PcePad3, sizeof(_PcePad3)); //v0.79追加
		CONFIG_Set("[APP INPUT] JoyPad Setting for 6Button", _PcePad6, sizeof(_PcePad6));
		CONFIG_Set("[APP INPUT] JoyPad TurboButton Speed", &_RenshaSpeedSel, sizeof(_RenshaSpeedSel)); //v0.62追加
		CONFIG_Set("[APP INPUT] JoyPad Use 3Button", &_bUseThreeButton, sizeof(_bUseThreeButton)); //v0.79追加
		CONFIG_Set("[APP INPUT] Auto Gradius II", &_bAutoGradiusII, sizeof(_bAutoGradiusII)); //v1.21追加
		CONFIG_Set("[APP INPUT] Auto ShinMegamiTensei", &_bAutoShinMegamiTensei, sizeof(_bAutoShinMegamiTensei)); //v2.20追加
		CONFIG_Set("[APP INPUT] Joypad Background", &_bJoypadBackground, sizeof(_bJoypadBackground)); //v1.41追加
		CONFIG_Set("[APP INPUT] Keyboard Background", &_bKeyboardBackground, sizeof(_bKeyboardBackground)); //v1.41追加
		CONFIG_Set("[APP INPUT] InputRecord Mode", &_InputRecordMode, sizeof(_InputRecordMode)); //v2.15追加

		//Kitao追加。ウインドウの座標を前に起動した座標と同じにする。
		GetWindowRect(hWnd, &rc);
		kidouX = rc.left;
		kidouY = rc.top;
		CONFIG_Set("[APP DISPLAY] Window Left", &kidouX, sizeof(kidouX));
		CONFIG_Set("[APP DISPLAY] Window Top", &kidouY, sizeof(kidouY));

		//Kitao追加。デバイス関連
		CONFIG_Set("[APP DEVICE] Current CD Drive", &_CurrentCdDrive, sizeof(_CurrentCdDrive));
		CONFIG_Set("[APP DEVICE] Start FastCD", &_StartFastCD, sizeof(_StartFastCD));
		CONFIG_Set("[APP DEVICE] FastCD", &_bFastCD, sizeof(_bFastCD));
		CONFIG_Set("[APP DEVICE] Start FastSeek", &_StartFastSeek, sizeof(_StartFastSeek)); //v0.82追加
		CONFIG_Set("[APP DEVICE] FastSeek", &_bFastSeek, sizeof(_bFastSeek)); //v0.82追加
		CONFIG_Set("[APP DEVICE] MySet OverClock Type", &_MySetOverClockType, sizeof(_MySetOverClockType)); //v1.61追加
		CONFIG_Set("[APP DEVICE] VideoSpeedUp Button", &_VideoSpeedUpButton, sizeof(_VideoSpeedUpButton)); //v0.92追加
		CONFIG_Set("[APP DEVICE] VideoSpeedUp Mode", &_FastForwarding, sizeof(_FastForwarding)); //v0.92追加
		CONFIG_Set("[APP DEVICE] VideoSpeedUp SoundAjust", &_bSoundAjust, sizeof(_bSoundAjust)); //v0.92追加
		CONFIG_Set("[APP DEVICE] LoadState SpeedSetting", &_bLoadStateSpeedSetting, sizeof(_bLoadStateSpeedSetting)); //v2.36追加
		CONFIG_Set("[APP DEVICE] SaveState Button", &_SaveStateButton, sizeof(_SaveStateButton)); //v1.56追加
		CONFIG_Set("[APP DEVICE] LoadState Button", &_LoadStateButton, sizeof(_LoadStateButton)); //v1.56追加
		CONFIG_Set("[APP DEVICE] Screenshot Button", &_ScreenshotButton, sizeof(_ScreenshotButton)); //v2.12追加
		CONFIG_Set("[APP DEVICE] Pause Button", &_PauseButton, sizeof(_PauseButton)); //v2.49追加
		CONFIG_Set("[APP DEVICE] Function Button", &_FunctionButton, sizeof(_FunctionButton)); //v2.38追加
		CONFIG_Set("[APP DEVICE] Function Volume", &_bFuncVolume, sizeof(_bFuncVolume)); //v2.38追加
		CONFIG_Set("[APP DEVICE] Function TurboButton", &_bFuncTurboButton, sizeof(_bFuncTurboButton)); //v2.38追加
		CONFIG_Set("[APP DEVICE] Function Screenshot", &_bFuncScreenshot, sizeof(_bFuncScreenshot)); //v2.38追加
		CONFIG_Set("[APP DEVICE] Function RecentGame", &_bFuncRecent, sizeof(_bFuncRecent)); //v2.48追加
		CONFIG_Set("[APP DEVICE] Function VSpeedUp", &_bFuncVSpeedUp, sizeof(_bFuncVSpeedUp)); //v2.38追加
		CONFIG_Set("[APP DEVICE] Function SaveButton", &_FuncSaveStateButton, sizeof(_FuncSaveStateButton)); //v2.38追加
		CONFIG_Set("[APP DEVICE] Function LoadButton", &_FuncLoadStateButton, sizeof(_FuncLoadStateButton)); //v2.38追加
		CONFIG_Set("[APP DEVICE] Windows PriorityClass", &_PriorityClass, sizeof(_PriorityClass)); //v2.04追加

		//Kitao追加。CPUなどの動作関連
		CONFIG_Set("[APP CPU] Start SpriteLimit", &_StartSpriteLimit, sizeof(_StartSpriteLimit));
		CONFIG_Set("[APP CPU] SpriteLimit", &_bSpriteLimit, sizeof(_bSpriteLimit));
		CONFIG_Set("[APP SAVE] AutoResume", &_bAutoResumeMode, sizeof(_bAutoResumeMode));
	}
	else //設定ファイルが読み込めた場合
	{
		// 読み込んだ設定を復元する。 
		CONFIG_Get("[APP SYSTEM] Soft Version", &softVersion, sizeof(softVersion));//Kitao追加。設定データを保存したときのバージョン

		// ビデオ関連
		CONFIG_Get("[APP DISPLAY] Screen Width", &_ScreenWidth, sizeof(_ScreenWidth));
		CONFIG_Get("[APP DISPLAY] Screen Height", &_ScreenHeight, sizeof(_ScreenHeight));
		CONFIG_Get("[APP DISPLAY] Full Screen Mode", &_bStartFullScreen, sizeof(_bStartFullScreen)); //Kitao更新。v0.55
		CONFIG_Get("[APP DISPLAY] Window TopMost", &_bWindowTopMost, sizeof(_bWindowTopMost)); //Kitao追加。v1.34
		CONFIG_Get("[APP DISPLAY] Window Activate", &_bWindowActivate, sizeof(_bWindowActivate)); //Kitao追加。v0.97
		CONFIG_Get("[APP DISPLAY] Window Pause", &_bInactivePause, sizeof(_bInactivePause)); //Kitao追加。v2.26
		CONFIG_Get("[APP DISPLAY] Gamma Value", &_GammaValue, sizeof(_GammaValue));//Kitao追加。v1.09
		if (strcmp(softVersion, "1.41") < 0) //v1.41より前の設定の場合
		{
			if (_GammaValue == 1.185) _GammaValue = 1.140;
			if (_GammaValue == 1.285) _GammaValue = 1.240;
			if (_GammaValue == 1.385) _GammaValue = 1.340;
			if (_GammaValue == 1.485) _GammaValue = 1.390;
			if (_GammaValue == 1.585) _GammaValue = 1.440;
		}
		CONFIG_Get("[APP DISPLAY] Brightness Value", &_BrightValue, sizeof(_BrightValue));//Kitao追加。v1.41
		CONFIG_Get("[APP DISPLAY] Scanline Type", &_ScanLineType, sizeof(_ScanLineType));//Kitao追加
		if (strcmp(softVersion, "2.63") < 0)
			if (_ScanLineType == 4)
			{
				_ScanLineType = 1; //v2.63からデフォルトを1(縦横スキャンライン)にした。
				CONFIG_Set("[APP DISPLAY] Scanline Type", &_ScanLineType, sizeof(_ScanLineType));
			}
		CONFIG_Get("[APP DISPLAY] Screenshot Scanline", &_ScreenshotScanLineType, sizeof(_ScreenshotScanLineType));//Kitao追加。v2.13
		CONFIG_Get("[APP DISPLAY] Scanline Density", &_ScanLineDensity, sizeof(_ScanLineDensity));//Kitao追加。v1.09
		if (strcmp(softVersion, "2.35") < 0)
			if (_ScanLineDensity == 20)
			{
				_ScanLineDensity = 30; //v2.35からデフォルトを30%にした。
				CONFIG_Set("[APP DISPLAY] Scanline Density", &_ScanLineDensity, sizeof(_ScanLineDensity));
			}
		if (strcmp(softVersion, "2.64") < 0)
			if (_ScanLineDensity == 30)
			{
				_ScanLineDensity = 70; //v2.64からデフォルトを70%にした。
				CONFIG_Set("[APP DISPLAY] Scanline Density", &_ScanLineDensity, sizeof(_ScanLineDensity));
			}
		CONFIG_Get("[APP DISPLAY] Optimize Gamma", &_bOptimizeGamma, sizeof(_bOptimizeGamma));//Kitao追加。v2.35
		CONFIG_Get("[APP DISPLAY] Stretched Mode", &_bStretched, sizeof(_bStretched));
		//_bStretched = TRUE; //v0.62更新。起動時は常にリアルストレッチモードで起動することにした。v0.69更新。前回の設定で起動するようにした。
		CONFIG_Get("[APP DISPLAY] FullStretched Mode", &_bFullStretched, sizeof(_bFullStretched));//Kitao追加。v1.24
		if (!_bStretched)
			_StartStretchMode = 0;
		else if (_bFullStretched)
			_StartStretchMode = 2;
		else
			_StartStretchMode = 1;
		CONFIG_Get("[APP DISPLAY] Magnification", &_Magnification, sizeof(_Magnification));
		if (strcmp(softVersion, "2.63") < 0)
			if (_Magnification == 2)
			{
				_Magnification = 3; //v2.63からx3をデフォルトに。
				CONFIG_Set("[APP DISPLAY] Magnification", &_Magnification, sizeof(_Magnification));
			}
		CONFIG_Get("[APP DISPLAY] FullMagnification", &_FullMagnification, sizeof(_FullMagnification));//Kitao追加。フルスクリーン時の_Magnification。
		if (strcmp(softVersion, "1.09") < 0)
			if (_FullMagnification == 6)
				_FullMagnification = 2; //v1.09から640x480をデフォルトに戻した。v2.63からは1024x768をデフォルトに。
		if (strcmp(softVersion, "2.63") < 0)
			if (_FullMagnification == 2)
			{
				_FullMagnification = 3; //v2.63から1024x768をデフォルトに。
				CONFIG_Set("[APP DISPLAY] FullMagnification", &_FullMagnification, sizeof(_FullMagnification));
			}
		CONFIG_Get("[APP DISPLAY] ScreenshotMagnification", &_ScreenshotMagnification, sizeof(_ScreenshotMagnification));//Kitao追加。v2.12
		CONFIG_Get("[APP DISPLAY] Full CustomWidth1", &_CustomWidth1, sizeof(_CustomWidth1));//Kitao追加。v1.35
		CONFIG_Get("[APP DISPLAY] Full CustomHeight1", &_CustomHeight1, sizeof(_CustomHeight1));//Kitao追加。v1.35
		CONFIG_Get("[APP DISPLAY] Full CustomWidth2", &_CustomWidth2, sizeof(_CustomWidth2));//Kitao追加。v1.35
		CONFIG_Get("[APP DISPLAY] Full CustomHeight2", &_CustomHeight2, sizeof(_CustomHeight2));//Kitao追加。v1.35
		CONFIG_Get("[APP DISPLAY] Full CustomWidth3", &_CustomWidth3, sizeof(_CustomWidth3));//Kitao追加。v1.35
		CONFIG_Get("[APP DISPLAY] Full CustomHeight3", &_CustomHeight3, sizeof(_CustomHeight3));//Kitao追加。v1.35
		CONFIG_Get("[APP DISPLAY] Resolution AutoChange", &_bResolutionAutoChange, sizeof(_bResolutionAutoChange));//Kitao追加。v1.35
		CONFIG_Get("[APP DISPLAY] Sync To 60Hz Fullscreen", &_bSyncTo60HzScreen, sizeof(_bSyncTo60HzScreen));
		CONFIG_Get("[APP DISPLAY] VSync Adjust", &_VSyncAdjust, sizeof(_VSyncAdjust));//Kitao追加。v2.65
		CONFIG_Get("[APP DISPLAY] Force VSync", &_bForceVSync, sizeof(_bForceVSync));//Kitao追加。v2.41
		CONFIG_Get("[APP DISPLAY] Windows Auto VSync", &_WindowsAutoVSync, sizeof(_WindowsAutoVSync));//Kitao追加。v2.65
		if (strcmp(softVersion, "2.65") < 0) //v2.65より前のバージョン
		{
			if (_bForceVSync)
			{
				_WindowsAutoVSync = 1; //ForceVSyncモードで使っていた場合、WindowsAutoVSyncモードへ切り替える。
				CONFIG_Set("[APP DISPLAY] Windows Auto VSync", &_WindowsAutoVSync, sizeof(_WindowsAutoVSync));
			}
		}
		_bForceVSync = FALSE; //v2.65からForceVSyncを廃止
		CONFIG_Get("[APP DISPLAY] Draw Method", &_DrawMethod, sizeof(_DrawMethod));//Kitao追加。Direct3D用に追加
		CONFIG_Get("[APP DISPLAY] Direct3D Effect TV", &_D3DEffectTV, sizeof(_D3DEffectTV));//Kitao追加。Direct3D用に追加
		CONFIG_Get("[APP DISPLAY] Direct3D Effect RGB", &_D3DEffectRGB, sizeof(_D3DEffectRGB));//Kitao追加。Direct3D用に追加
		CONFIG_Get("[APP DISPLAY] Use VideoCard Memory", &_bUseVideoCardMemory, sizeof(_bUseVideoCardMemory));//Kitao追加。v1.31
		CONFIG_Get("[APP DISPLAY] Use System Memory Only Window", &_bUseSystemMemoryOnlyWindow, sizeof(_bUseSystemMemoryOnlyWindow));//Kitao追加。v1.53
		CONFIG_Get("[APP DISPLAY] FullScreen Color", &_FullScreenColor, sizeof(_FullScreenColor));//Kitao追加
		CONFIG_Get("[APP DISPLAY] Overscan Top", &_StartShowOverscanTop, sizeof(_StartShowOverscanTop));//Kitao追加。v1.43
		CONFIG_Get("[APP DISPLAY] Overscan Bottom", &_StartShowOverscanBottom, sizeof(_StartShowOverscanBottom));//Kitao追加。v1.43
		CONFIG_Get("[APP DISPLAY] Overscan Left", &_StartShowOverscanLeft, sizeof(_StartShowOverscanLeft));//Kitao追加。v1.43
		CONFIG_Get("[APP DISPLAY] Overscan Right", &_StartShowOverscanRight, sizeof(_StartShowOverscanRight));//Kitao追加。v1.43
		CONFIG_Get("[APP DISPLAY] Overscan Height", &_StartShowOverscanHeight, sizeof(_StartShowOverscanHeight));//Kitao追加。v1.43
		CONFIG_Get("[APP DISPLAY] Overscan HideBlack", &_bOverscanHideBlackBelt, sizeof(_bOverscanHideBlackBelt));//Kitao追加。v1.45
		CONFIG_Get("[APP DISPLAY] Hide Menu", &_bHideMenu, sizeof(_bHideMenu)); //Kitao追加。v0.62
		CONFIG_Get("[APP DISPLAY] Hide Message", &_bHideMessage, sizeof(_bHideMessage)); //Kitao追加。v0.64
		CONFIG_Get("[APP DISPLAY] Full Hide Message", &_bFullHideMessage, sizeof(_bFullHideMessage)); //Kitao追加。v0.64
		CONFIG_Get("[APP DISPLAY] Screenshot Dialog", &_bScreenshotDialog, sizeof(_bScreenshotDialog));//Kitao追加。v2.13
		CONFIG_Get("[APP DISPLAY] Disable Windows Aero", &_bDisableWindowsAero, sizeof(_bDisableWindowsAero));//Kitao追加。v2.21

		// サウンド関連 
		CONFIG_Get("[APP SOUND] Sound Initialize Type", &_SoundType, sizeof(_SoundType));//Kitao追加
		if (_SoundType == 3) //v1.30以前で3に設定されていた場合
		{
			_SoundType = 2;
			CONFIG_Set("[APP SOUND] Sound Initialize Type", &_SoundType, sizeof(_SoundType));
		}
		if (strcmp(softVersion, "2.44") < 0)
			if ((_bWindowsVista)&&(_SoundType == 2))
			{
				_SoundType = 1;//v2.44より前のバージョンをVistaかWin7で動かしていた場合、デフォルトを2にしていたのでv2.44以降では1に再設定する。
				CONFIG_Set("[APP SOUND] Sound Initialize Type", &_SoundType, sizeof(_SoundType));
			}
		CONFIG_Get("[APP SOUND] Sound Buffer Size", &_SoundBufferSize, sizeof(_SoundBufferSize));
		if (strcmp(softVersion, "2.36") < 0)
		{
			if (_SoundBufferSize > 1664)
				_SoundBufferSize = 1664;//v2.36より前のバージョンをWin7/Vistaで動かしていた場合、デフォルトバッファが大きかったのでv2.36以降では1664に再設定する。XPも環境によってはデフォルトを大きくしていた方もいるかもしれないので1664に再設定しておく。
			CONFIG_Set("[APP SOUND] Sound Buffer Size", &_SoundBufferSize, sizeof(_SoundBufferSize));
		}
		CONFIG_Get("[APP SOUND] Sound Sample Rate", &_SoundSampleRate, sizeof(_SoundSampleRate));
		CONFIG_Get("[APP SOUND] Windows Volume Control", &_WindowsVolume, sizeof(_WindowsVolume));//Kitao追加。v1.61
		CONFIG_Get("[APP SOUND] Windows Volume Step", &_VolumeStep, sizeof(_VolumeStep));//Kitao追加。v2.45
		CONFIG_Get("[APP SOUND] APU Master Volume", &_SoundApuMasterVolume, sizeof(_SoundApuMasterVolume));
		CONFIG_Get("[APP SOUND] PSG Volume", &_SoundPsgVolume, sizeof(_SoundPsgVolume));//Kitao追加。PSGも個別に音量設定できるようにした。
		CONFIG_Get("[APP SOUND] ADPCM Volume", &_SoundAdpcmVolume, sizeof(_SoundAdpcmVolume));
		CONFIG_Get("[APP SOUND] CD Volume", &_SoundCdVolume, sizeof(_SoundCdVolume));
		CONFIG_Get("[APP SOUND] PSG Quality", &_PsgQuality, sizeof(_PsgQuality));//Kitao追加。v1.39
		CONFIG_Get("[APP SOUND] Auto Stereo", &_bAutoStereo, sizeof(_bAutoStereo));//Kitao追加。v1.15
		CONFIG_Get("[APP SOUND] WAV Next", &_bOutputWavNext, sizeof(_bOutputWavNext));//Kitao追加。v1.29
		CONFIG_Get("[APP SOUND] WAV Add No Sound", &_bOutputWavAddNoSound, sizeof(_bOutputWavAddNoSound));//Kitao追加。v1.29
		CONFIG_Get("[APP SOUND] CDDA Adjust Window", &_CddaAdjust, sizeof(_CddaAdjust));//Kitao追加。v2.32
		CONFIG_Get("[APP SOUND] CDDA Adjust FullScreen", _CddaAdjustFull, sizeof(_CddaAdjustFull));//Kitao追加。v2.32
		CONFIG_Get("[APP SOUND] CDDA Delay Frame", &_CddaDelayFrame, sizeof(_CddaDelayFrame));//Kitao追加。v2.33

		// 入力関連 Kitao更新。キーボード対応の_PcePadへ変更。
		CONFIG_Get("[APP INPUT] JoyPad Setting for 2Button", _PcePad2, sizeof(_PcePad2));
		CONFIG_Get("[APP INPUT] JoyPad Setting for 3Button", _PcePad3, sizeof(_PcePad3)); //v0.79追加
		CONFIG_Get("[APP INPUT] JoyPad Setting for 6Button", _PcePad6, sizeof(_PcePad6));
		CONFIG_Get("[APP INPUT] JoyPad TurboButton Speed", &_RenshaSpeedSel, sizeof(_RenshaSpeedSel));
		CONFIG_Get("[APP INPUT] JoyPad Use 3Button", &_bUseThreeButton, sizeof(_bUseThreeButton)); //v0.79追加
		JOYPAD_UseThreeButton(_bUseThreeButton);
		CONFIG_Get("[APP INPUT] Auto Gradius II", &_bAutoGradiusII, sizeof(_bAutoGradiusII)); //v1.21追加
		CONFIG_Get("[APP INPUT] Auto ShinMegamiTensei", &_bAutoShinMegamiTensei, sizeof(_bAutoShinMegamiTensei)); //v2.20追加
		CONFIG_Get("[APP INPUT] Joypad Background", &_bJoypadBackground, sizeof(_bJoypadBackground)); //v1.41追加
		CONFIG_Get("[APP INPUT] Keyboard Background", &_bKeyboardBackground, sizeof(_bKeyboardBackground)); //v1.41追加
		CONFIG_Get("[APP INPUT] InputRecord Mode", &_InputRecordMode, sizeof(_InputRecordMode)); //v2.15追加

		//Kitao追加。ウインドウの座標を前に起動した座標と同じにするために必要
		CONFIG_Get("[APP DISPLAY] Window Left", &kidouX, sizeof(kidouX));
		CONFIG_Get("[APP DISPLAY] Window Top", &kidouY, sizeof(kidouY));

		//Kitao追加。デバイス関連
		CONFIG_Get("[APP DEVICE] Current CD Drive", &_CurrentCdDrive, sizeof(_CurrentCdDrive));
		CONFIG_Get("[APP DEVICE] Start FastCD", &_StartFastCD, sizeof(_StartFastCD));
		CONFIG_Get("[APP DEVICE] FastCD", &_bFastCD, sizeof(_bFastCD));
		if (_StartFastCD == 0)
			_bFastCD = FALSE;
		else if (_StartFastCD == 1)
			_bFastCD = TRUE;
		CONFIG_Get("[APP DEVICE] Start FastSeek", &_StartFastSeek, sizeof(_StartFastSeek)); //v0.82追加
		if (strcmp(softVersion, "1.08") < 0)
			if (_StartFastSeek == 0)
			{
				_StartFastSeek = 1; //v1.08からデフォルトでONに。
				CONFIG_Set("[APP DEVICE] Start FastSeek", &_StartFastSeek, sizeof(_StartFastSeek)); //次に起動したときのためにすぐ更新が必要
			}
		CONFIG_Get("[APP DEVICE] FastSeek", &_bFastSeek, sizeof(_bFastSeek)); //v0.82追加
		if (_StartFastSeek == 0)
			_bFastSeek = FALSE;
		else if (_StartFastSeek == 1)
			_bFastSeek = TRUE;
		CONFIG_Get("[APP DEVICE] MySet OverClock Type", &_MySetOverClockType, sizeof(_MySetOverClockType)); //v1.61追加
		CONFIG_Get("[APP DEVICE] VideoSpeedUp Button", &_VideoSpeedUpButton, sizeof(_VideoSpeedUpButton)); //v0.92追加
		CONFIG_Get("[APP DEVICE] VideoSpeedUp Mode", &_FastForwarding, sizeof(_FastForwarding)); //v0.92追加
		if (_FastForwarding >= 2000) //早回しがSlow倍率モードだった場合、起動時はノーマル速度に戻す。
		{
			_FastForwarding = 0;
			CONFIG_Set("[APP DEVICE] VideoSpeedUp Mode", &_FastForwarding, sizeof(_FastForwarding));
		}
		CONFIG_Get("[APP DEVICE] VideoSpeedUp SoundAjust", &_bSoundAjust, sizeof(_bSoundAjust)); //v0.92追加
		CONFIG_Get("[APP DEVICE] LoadState SpeedSetting", &_bLoadStateSpeedSetting, sizeof(_bLoadStateSpeedSetting)); //v2.36追加
		CONFIG_Get("[APP DEVICE] SaveState Button", &_SaveStateButton, sizeof(_SaveStateButton)); //v1.56追加
		CONFIG_Get("[APP DEVICE] LoadState Button", &_LoadStateButton, sizeof(_LoadStateButton)); //v1.56追加
		CONFIG_Get("[APP DEVICE] Screenshot Button", &_ScreenshotButton, sizeof(_ScreenshotButton)); //v2.12追加
		CONFIG_Get("[APP DEVICE] Pause Button", &_PauseButton, sizeof(_PauseButton)); //v2.49追加
		CONFIG_Get("[APP DEVICE] Function Button", &_FunctionButton, sizeof(_FunctionButton)); //v2.38追加
		CONFIG_Get("[APP DEVICE] Function Volume", &_bFuncVolume, sizeof(_bFuncVolume)); //v2.38追加
		CONFIG_Get("[APP DEVICE] Function TurboButton", &_bFuncTurboButton, sizeof(_bFuncTurboButton)); //v2.38追加
		CONFIG_Get("[APP DEVICE] Function Screenshot", &_bFuncScreenshot, sizeof(_bFuncScreenshot)); //v2.38追加
		CONFIG_Get("[APP DEVICE] Function RecentGame", &_bFuncRecent, sizeof(_bFuncRecent)); //v2.48追加
		CONFIG_Get("[APP DEVICE] Function VSpeedUp", &_bFuncVSpeedUp, sizeof(_bFuncVSpeedUp)); //v2.38追加
		CONFIG_Get("[APP DEVICE] Function SaveButton", &_FuncSaveStateButton, sizeof(_FuncSaveStateButton)); //v2.38追加
		CONFIG_Get("[APP DEVICE] Function LoadButton", &_FuncLoadStateButton, sizeof(_FuncLoadStateButton)); //v2.38追加
		CONFIG_Get("[APP DEVICE] Windows PriorityClass", &_PriorityClass, sizeof(_PriorityClass)); //v2.04追加

		//Kitao追加。CPUなどの動作関連
		CONFIG_Get("[APP CPU] Start SpriteLimit", &_StartSpriteLimit, sizeof(_StartSpriteLimit));
		CONFIG_Get("[APP CPU] SpriteLimit", &_bSpriteLimit, sizeof(_bSpriteLimit));
		CONFIG_Get("[APP SAVE] AutoResume", &_bAutoResumeMode, sizeof(_bAutoResumeMode));
	}
	if (_DrawMethod == 0) //初めてのv2.00起動の場合
	{	//Direct3Dで起動
		_DrawMethod = 1; //Direct3D
		CONFIG_Set("[APP DISPLAY] Draw Method", &_DrawMethod, sizeof(_DrawMethod));
		_ScanLineType = 0;
		CONFIG_Set("[APP DISPLAY] Scanline Type", &_ScanLineType, sizeof(_ScanLineType));
		_ScanLineDensity = 70;
		CONFIG_Set("[APP DISPLAY] Scanline Density", &_ScanLineDensity, sizeof(_ScanLineDensity));
		_bStretched = TRUE;
		CONFIG_Set("[APP DISPLAY] Stretched Mode", &_bStretched, sizeof(_bStretched));
		_bFullStretched = FALSE;
		CONFIG_Set("[APP DISPLAY] FullStretched Mode", &_bFullStretched, sizeof(_bFullStretched));
		_StartStretchMode = 1;
	}

	//Windowsアプリのプライオリティを設定。v2.04
	WINMAIN_SetPriority(_PriorityClass);

	SetCurrentDirectory( sStartDir );
	RA_Init( hWnd, RA_PCE, RAPCE_VERSION );
	RA_InitShared();

	RA_UpdateAppTitle( "" );

	RebuildMenu();

	RA_AttemptLogin(true);

	RebuildMenu();

	//早回しモードを初期化
	_bUseVideoSpeedUpButton = TRUE;
	MAINBOARD_SetFastForwarding(_FastForwarding, _bSoundAjust, TRUE);

	//ダイアログ用のフォルダー設定ファイルがあれば読み込む
	strcpy(fileName, _AppPath);
	strcat(fileName, "Folder.dat");
	if ((fp = fopen(fileName, "r")) != NULL) //フォルダー設定ファイルがあったら
	{
		fgets(_CurrentOpenPath, MAX_PATH+1, fp);
		if (strlen(_CurrentOpenPath) >= 1)
			*(_CurrentOpenPath + strlen(_CurrentOpenPath) - 1) = 0; //末尾の改行コードをカット
		fgets(_CurrentSavePath, MAX_PATH+1, fp);
		if (strlen(_CurrentSavePath) >= 1)
			*(_CurrentSavePath + strlen(_CurrentSavePath) - 1) = 0; //末尾の改行コードをカット
		fgets(_OtherSysCardPath1, MAX_PATH+1, fp);
		if (strlen(_OtherSysCardPath1) >= 1)
			*(_OtherSysCardPath1 + strlen(_OtherSysCardPath1) - 1) = 0; //末尾の改行コードをカット
		fgets(_OtherSysCardPath2, MAX_PATH+1, fp);
		if (strlen(_OtherSysCardPath2) >= 1)
			*(_OtherSysCardPath2 + strlen(_OtherSysCardPath2) - 1) = 0; //末尾の改行コードをカット
		fclose(fp);
	}

	//ROMイメージの履歴ファイルがあれば読み込む
	strcpy(fileName, _AppPath);
	strcat(fileName, "RecentRom.dat");
	if ((fp = fopen(fileName, "r")) != NULL) //履歴ファイルがあったら
	{
		for (i=1; i<=20; i++)
		{
			if (fgets(_RecentRom[i], MAX_PATH+1, fp))
			{
				if (strlen(_RecentRom[i]) >= 1)
					*(_RecentRom[i] + strlen(_RecentRom[i]) - 1) = 0; //末尾の改行コードをカット
			}
			else //読み込みエラー（ユーザーが編集した場合などで起こり得る）
				strcpy(_RecentRom[i], " ");
		}
		fclose(fp);
	}

	if (argc > 2)
	{
		MessageBox(hWnd,"Error: too many arguments.    ", "Ootake", MB_OK);
		//PRINTF("ex.)");
		//PRINTF("	Ootake hucard.rom");
		//PRINTF("	Ootake \"filename with spaces.rom\"");
		return FALSE;
	}

	if (argc < 2 ) // No game specified.
	{
		//Kitao追加。CDゲームを遊ぶのか、Huカードを遊ぶのかを尋ねる。
		STARTUP_Init(WINMAIN_GetHInstance(), &_CDorHu); //Startupウィンドウを表示
		//メッセージループ
		while (_CDorHu == 0) //ダイアログが結果を返すまでループ
		{ 
			GetMessage(&msg ,NULL, 0, 0);
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (_CDorHu == -1)
			return FALSE; //バッテン印などでダイアログを閉じた場合
		if (_CDorHu == 1)
		{//CD-ROM
			_bCDGame = TRUE;
			if (cdIniOpenSub() == FALSE) //CD起動処理
				return FALSE;
		}
		else if (_CDorHu == 2)
		{//フルインストールCDのCUEファイルから起動。v2.24追加
			while (TRUE)
			{
				if (!folder_dialog(hWnd, fileName))
					return FALSE;
				strcpy(srcFilePathName, fileName);
				strcat(fileName, "\\");
				strcat(fileName, extractFileName(srcFilePathName));
				strcat(fileName, ".cue");
				strToLower(srcFilePathName);
				if (strcmp(extractFileName(srcFilePathName),"install") != 0) //ルート(install)フォルダが選択された場合を除く
					if ((fp = fopen(fileName, "r")) != NULL) //cueファイルが存在すれば
					{
						fclose(fp);
						_bCDGame = TRUE;
						strcpy(_CueFilePathName, fileName);
						_bCueFile = TRUE;
						if (cdIniOpenSub() == FALSE) //CD起動処理
							return FALSE;
						else
							break; //正常に開けた
					}
			}
			//正常に開けた場合、ROMイメージ履歴に追加。
			AddRecentRom();
		}
		else if (_CDorHu == 3)
		{//Hu-Card
			while (TRUE)
			{
				if (!file_dialog(1, hWnd, _OpenFilePathName, _GameFilePathName))
					return FALSE;
				//拡張子をカットしたゲームファイル名を_GameFileNameBufに設定しておく（ステートセーブ時等で利用）
				SetGameFileNameBuf(_bCDGame, _bHesFile, _GameFilePathName, _GameFileNameBuf, FALSE);
				if (MAINBOARD_Init(_OpenFilePathName))
					break; //正常に開けた
			}
			//正常に開けた場合、ROMイメージ履歴に追加。
			AddRecentRom();
		}
		else //(_CDorHu == 4)
		{//リプレイHu-CardまたはCUE
			if (strcmp(_RecentRom[1], " ") == 0) //履歴がなかった場合
			{
				if (!file_dialog(1, hWnd, _OpenFilePathName, _GameFilePathName))
					return FALSE;
			}
			else
			{
				strcpy(fileName, _RecentRom[1]); //_RecentRom[1]は書き換えないようにするためfileNameにコピー
				if (strstr(strToLower(extractFileExt(fileName)), "cue")) //CUEファイルの場合、CDゲームとして起動。v2.24追加
				{
					_bCDGame = TRUE;
					strcpy(_CueFilePathName, _RecentRom[1]);
					_bCueFile = TRUE;
				}
				else //通常
				{
					if (!set_openFilePathName(_RecentRom[1], _OpenFilePathName, _GameFilePathName))
					{
						MessageBox(hWnd,"File not found.    ", "Ootake", MB_OK);
						return FALSE;
					}
				}
			}
			if (_bCueFile)
			{
				if (cdIniOpenSub() == FALSE) //CD起動処理
					return FALSE;
			}
			else
			{
				SetGameFileNameBuf(_bCDGame, _bHesFile, _GameFilePathName, _GameFileNameBuf, FALSE); //拡張子をカットしたゲームファイル名を_GameFileNameBufに設定しておく（ステートセーブ時等で利用）
				if (!MAINBOARD_Init(_OpenFilePathName))
				{
					MessageBox(hWnd,"MainBoard Init Error.    ", "Ootake", MB_OK);
					return FALSE;
				}
			}
			//正常に開けた場合、ROMイメージ履歴に追加。
			AddRecentRom(); //履歴がなかった場合用。履歴があった場合は何もせずに戻ってくる。
		}
	}
	else if (argc == 2)
	{
		strcpy(fileName, argv[1]); //argv[1]はstrToLower()で書き換えないようにするためfileNameにコピー
		strToLower(fileName);//全て小文字に
		if (strcmp(fileName,"/cd") == 0)
		{	//[/cd]オプションが付いていた場合CD起動。v2.33
			_bCDGame = TRUE;
			if (cdIniOpenSub() == FALSE) //CD起動処理
				return FALSE;
		}
		else
		{
			if (!set_openFilePathName(argv[1], _OpenFilePathName, _GameFilePathName))
			{
				MessageBox(hWnd,"File not found.    ", "Ootake", MB_OK);
				return FALSE;
			}
			if (strstr(extractFileExt(fileName), "cue")) //CUEファイルの場合、CDゲームとして起動。v2.33追加
			{
				_bCDGame = TRUE;
				strcpy(_CueFilePathName, argv[1]);
				_bCueFile = TRUE;
				if (cdIniOpenSub() == FALSE) //CD起動処理
					return FALSE;
				//正常に開けた場合、ROMイメージ履歴に追加。
				if (strcmp(TOCDB_GetGameTitle(),"UnknownTitle") != 0)
					AddRecentRom();
			}
			else
			{
				SetGameFileNameBuf(_bCDGame, _bHesFile, _GameFilePathName, _GameFileNameBuf, FALSE); //拡張子をカットしたゲームファイル名を_GameFileNameBufに設定しておく（ステートセーブ時等で利用）
				if (!MAINBOARD_Init(_OpenFilePathName))
				{
					MessageBox(hWnd,"MainBoard Init Error.    ", "Ootake", MB_OK);
					return FALSE;
				}
				//正常に開けた場合、ROMイメージ履歴に追加。
				AddRecentRom();
			}
		}
	}



	//Kitao追加。ゲーム開始前のCPU速度設定＆パッド設定を退避しておく。違うゲームを開くときに設定を元に戻す(誤動作を避けるため)。
	//※手動作で設定が変更されたときに、この退避変数を書き換える。レジュームやステートロードによって自動で速度やパッド設定が変わったときは書き換えない。
	_OverClockTypeR = 0;
	_RenshaSpeedSelR = _RenshaSpeedSel;
	_bUseThreeButtonR = _bUseThreeButton;
	_bArcadeCardR = TRUE;

	//Kitao更新。サウンド・入力などの設定を反映させる
	if (strcmp(softVersion, "0.00") == 0) //初めての起動の場合
	{
		if (INPUT_GetNumJoystick() == 0) //ジョイスティックが１本もつながっていなければ、PCエンジンパッド１はキーボードを使う設定で初期化する。
		{
			initPad1ConfigurationKB2();
			initPad1ConfigurationKB3();
			initPad1ConfigurationKB6();
			CONFIG_Set("[APP INPUT] JoyPad Setting for 2Button", _PcePad2, sizeof(_PcePad2));
			CONFIG_Set("[APP INPUT] JoyPad Setting for 3Button", _PcePad3, sizeof(_PcePad3));
			CONFIG_Set("[APP INPUT] JoyPad Setting for 6Button", _PcePad6, sizeof(_PcePad6));
		}
	}
	CDROM_SetCDDAReadBufferSize();
	set_userSetting();

	//Kitao追加。スクリーン用変数を設定
	_bFullScreen =_bStartFullScreen;
	SCREEN_SetSyncTo60HzScreen(_bSyncTo60HzScreen);
	if (_ScanLineType == 4)
		_bTvMode = TRUE; //v2.00。初回起動時にもTVモードが反映されるようにここへ移動。v1.69以前は初回起動時にTVモードが有効になっていなかった。
	MAINBOARD_SetStretched(_bStretched, _bVStretched);
	initShowOverscan();

	//Kitao追加。_CddaAdjust(CD-DAテンポ微調整)関連を設定
	for (i=0; i<10; i++)
		if (_CddaAdjustFull[i] == 0)
			_CddaAdjustFull[i] = 5995; //59.95Hzをデフォルトとする。v2.43更新（フルHDサイズのワイドディスプレイで59.94Hzが多く、今後も主流で一番多いと思われる）
	if ((getCddaAdjust() % 10) == 5)
		_CddaAdjustFineTune = 5;
	else
		_CddaAdjustFineTune = 0;

	//メニューを初期化
	init_menu();
	if ((!_bHideMenu)&&(!_bFullScreen)) //Kitao追加
		MENU_Show(_Menu);

	//Kitao追加。ウインドウの座標を前に起動した座標と同じにする。
	GetWindowRect(hWnd, &rc);
	if (kidouX < 0) kidouX = 0;
	if (kidouY < 0) kidouY = 0;
	if (kidouX > GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left))
		kidouX = GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left);
	if (kidouY > GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top))
		kidouY = GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top);
	MoveWindow(hWnd, kidouX, kidouY, rc.right - rc.left, rc.bottom - rc.top, TRUE);

	//スクリーンを初期化し、設定のモードに変更する。v1.67更新
	if (!updateScreenMode(FALSE)) //Kitao更新
	{
		sprintf(fileName, "APP_Init: Error. Please delete file \"%s\" and restart.    ", APP_CONFIG_FILENAME);
		MessageBox(hWnd, fileName, "Ootake", MB_OK);
		return FALSE;
	}
	if (SCREEND3D_GetOldVideoChip()) //Kitao追加。古いビデオチップ(ビデオカード対策)。v2.19
	{	//今も結構現役で使われているRADEONの古い型の場合、Direct3Dのエフェクトに非対応で画面が出ないので、自動で対応。
		if (strcmp(softVersion, "2.19") < 0) //v2.19より下か、初起動(0.00)の場合
		{
			_D3DEffectTV = 0; //エフェクトをNONEに。
			_D3DEffectRGB = 0; //エフェクトをNONEに。
			CONFIG_Set("[APP DISPLAY] Direct3D Effect TV", &_D3DEffectTV, sizeof(_D3DEffectTV));
			CONFIG_Set("[APP DISPLAY] Direct3D Effect RGB", &_D3DEffectRGB, sizeof(_D3DEffectRGB));
		}
		if (strcmp(softVersion, "2.65") < 0) //v2.65より下か、初起動(0.00)の場合
		{
			_VSyncAdjust = 13;
			CONFIG_Set("[APP DISPLAY] VSync Adjust", &_VSyncAdjust, sizeof(_VSyncAdjust));
		}
	} 
	UpdateWindow(hWnd); //Kitao追加。すぐにメニュー等へアクセスできるように。
	if (_bDisableWindowsAero)
	{
		setWindowsAero(FALSE); //Aeroを無効に。v2.21追加
		UpdateWindow(hWnd);
	}

	//バッテリーバックアップラムを読み込む。
	load_bram();

	//Kitao追加。レジュームファイルがあればレジュームする。※スクリーン初期化後におこなわないと固まる
	load_resume(); //レジュームファイルがなく、オートレジュームがオンの場合、レジュームがオンにされる。
	updateMenu(); //レジュームでロードされた設定を反映させるため、メニュー表示を更新。

	//イベント処理系を初期化 
	APPEVENT_Init();

	//Windowsミキサーの音量を設定。他の初期化やROM読み込みが正常に終了したここで行う(Ootake終了時に確実に元の音量に戻すため)。_WindowsVolumeが-1のときはWindowsミキサーを変更しない。
	if (_WindowsVolume != -1)
	{
		setWaveVolume(_WindowsVolume);
		_bWindowsVolumeChanged = TRUE;
	}

	//CD-ROMのReadスピードをx4に制限する。v2.50 ※テスト不足のため現在未使用。
	//if (_bCDSpeedDown)
	//	CDIF_SetSpeed(4);

	_bInit = TRUE; //Kitao追加

#ifdef DEBUGBUILD
	run_emulator(FALSE);
#else
	first_run_cpu(); //Kitao追加。画像表示前に約１秒ぶんCPUを動かす(実機同様に起動時の画面乱れを見せないようにする)
#endif

	//##RA
	RA_HandleHTTPResults();
	MAINBOARD_Init( _OpenFilePathName );

	return TRUE;
}


static void
advance_frames(
	Uint32				nFrame)
{
	MAINBOARD_Pause(FALSE);
	while (nFrame--)
		MAINBOARD_AdvanceFrame();
	MAINBOARD_Pause(TRUE);
	_bRunning = FALSE; //Kitao追加
}


//Kitao更新。複数人プレイのときもキーボードのボタンを使えるようにした。
static void
configure_pad(
	Uint32		padID) //Kitao更新。joyID→padID(1～5)へ。PCエンジンのパッドナンバーで管理するようにした。
{
	PCEPAD		pcePad; //ボタン設定用バッファ
	Sint32		setOk = 0;
	MSG			msg;
	BOOL		bToggle = FALSE;

	//PadConfigダイアログを表示
	if (_bFullScreen)
	{
		APP_ToggleFullscreen();
		run_emulator(FALSE);//メニュー表示などを乱さないように更新
		bToggle = TRUE;
	}
	PADCONFIG_Init(WINMAIN_GetHInstance(), 0, padID, &pcePad, &setOk);
	//メッセージループ
	while (setOk == 0) //ダイアログが結果を返すまでループ
	{ 
		GetMessage(&msg ,NULL, 0, 0);
		
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	if (bToggle) //フルスクリーンモードだった場合は元に戻す
	{
		APP_ToggleFullscreen();
		run_emulator(FALSE);//メニュー表示などを乱さないように更新
	}
	if (setOk == 1) //設定が成功した場合
	{
		if (JOYPAD_GetConnectSixButton())
		{
			_PcePad6[padID] = pcePad;
			CONFIG_Set("[APP INPUT] JoyPad Setting for 6Button", _PcePad6, sizeof(_PcePad6));
			PRINTF("6-Button Pad Configure OK.");
		}
		else if (JOYPAD_GetConnectThreeButton())
		{
			_PcePad3[padID] = pcePad;
			CONFIG_Set("[APP INPUT] JoyPad Setting for 3Button", _PcePad3, sizeof(_PcePad3));
			PRINTF("3-Button Pad Configure OK.");
		}
		else
		{
			_PcePad2[padID] = pcePad;
			CONFIG_Set("[APP INPUT] JoyPad Setting for 2Button", _PcePad2, sizeof(_PcePad2));
			PRINTF("2-Button Pad Configure OK.");
		}
		
		//設定を反映させる
		INPUT_Deinit();
		INPUT_Init();
		APP_SetInputConfiguration();
		
		run_emulator(TRUE); //設定完了後すぐに動き出すように。
	}
	else //設定をキャンセルした場合
	{
		PRINTF("Canceled JoyPad Configure.");
		screenUpdate();//すぐにメッセージを表示
	}
}


//Kitao追加。連射専用のボタンを設定。v0.91。
static void
configure_turbo_button(
	Uint32		padID) //padID(1～5)
{
	PCEPAD	pcePad; //ボタン設定用バッファ
	Sint32	setOk = 0;
	MSG		msg;
	BOOL	bToggle = FALSE;

	if (JOYPAD_GetConnectSixButton()) //６ボタンパッドにはターボボタンの設定はできない
	{
		MessageBox(WINMAIN_GetHwnd(),
		"When \"6-Button Pad\" is connected, The configure of TurboButton cannot be done.    ",
		"Ootake", MB_OK);
		return;
	}

	if (JOYPAD_GetConnectThreeButton())
		pcePad = _PcePad3[padID];
	else
		pcePad = _PcePad2[padID];

	//PadConfigダイアログを表示
	if (_bFullScreen)
	{
		APP_ToggleFullscreen();
		run_emulator(FALSE);//メニュー表示などを乱さないように更新
		bToggle = TRUE;
	}
	PADCONFIG_Init(WINMAIN_GetHInstance(), 1, padID, &pcePad, &setOk);
	//メッセージループ
	while (setOk == 0) //ダイアログが結果を返すまでループ
	{ 
		GetMessage(&msg ,NULL, 0, 0);
		
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	if (bToggle) //フルスクリーンモードだった場合は元に戻す
	{
		APP_ToggleFullscreen();
		run_emulator(FALSE);//メニュー表示などを乱さないように更新
	}
	if (setOk == 1) //設定が成功した場合
	{
		if (JOYPAD_GetConnectThreeButton())
		{
			_PcePad3[padID] = pcePad;
			CONFIG_Set("[APP INPUT] JoyPad Setting for 3Button", _PcePad3, sizeof(_PcePad3));
			PRINTF("3-Button Pad TurboButton Configure OK.");
		}
		else
		{
			_PcePad2[padID] = pcePad;
			CONFIG_Set("[APP INPUT] JoyPad Setting for 2Button", _PcePad2, sizeof(_PcePad2));
			PRINTF("2-Button Pad TurboButton Configure OK.");
		}
		
		//設定を反映させる
		INPUT_Deinit();
		INPUT_Init();
		APP_SetInputConfiguration();
		
		run_emulator(TRUE); //設定完了後すぐに動き出すように。
	}
	else //設定をキャンセルした場合
	{
		PRINTF("Canceled TurboButton Configure.");
		screenUpdate();//すぐにメッセージを表示
	}
}


//Kitao追加。ボリュームミュート、ハーフなどをできるようにした。
static void
SetVolumeEffect(
	int	num)
{
	MENU_CheckRadioItem(_VolumeMenu, WM_VOLUME_NORMAL, WM_VOLUME_MUTE, WM_VOLUME_NORMAL + num);

	switch (num)
	{
		case 0://NORMAL
			_VolumeEffect = 1;
			break;
		case 1://3QUARTER(3/4)
			_VolumeEffect = 3;
			break;
		case 2://HALF
			_VolumeEffect = 2;
			break;
		case 3://QUARTER
			_VolumeEffect = 4;
			break;
		case 4://MUTE
			_VolumeEffect = 0;
			break;
	}
	PSG_SetVolumeEffect(_VolumeEffect);
	ADPCM_SetVolumeEffect(_VolumeEffect);
	CDROM_SetVolumeEffect(_VolumeEffect);

	updateMenu();
}


//Kitao追加。特別なセッティングを元に戻す。ゲームが替わる際に使用。
static void
return_special_setting()
{
	BOOL	bUpdate;
	Sint32	startStretchMode;

	if (((_StartShowOverscanTop != MAINBOARD_GetShowOverscanTop())||
		 (_StartShowOverscanBottom != MAINBOARD_GetShowOverscanBottom())||
		 (_StartShowOverscanLeft != MAINBOARD_GetShowOverscanLeft())||
		 (_StartShowOverscanRight != MAINBOARD_GetShowOverscanRight()))&&
		(!_bFullScreen)) //ウィンドウモードでオーバースキャン表示が変更されていた場合
			bUpdate = TRUE; //ウィンドウの大きさを変える必要がある
	else
			bUpdate = FALSE;
	//640x480のフルスクリーン＆フルストレッチ＆スキャンライン表示の場合、画質維持のためフルストレッチ(低倍率拡大)せず、オーバースキャンを表示することで画面を一杯にする。v2.64追加
	if ((((_FullMagnification == 0)&&((_CustomWidth1 != 640)||(_CustomHeight1 != 480)))||(_FullMagnification == 2))&&
		(_bFullScreen)&&(_StartStretchMode == 2)&&((_ScanLineType == 2)||(_ScanLineType == 4)))
	{
		MAINBOARD_SetShowOverscanTop(8);
		MAINBOARD_SetShowOverscanBottom(8);
		MAINBOARD_SetShowOverscanLeft(8);
		MAINBOARD_SetShowOverscanRight(8);
	}
	else		
		initShowOverscan(); //オーバースキャン領域表示を起動時設定に戻す。v1.43
	if (!_bStretched)
		startStretchMode = 0;
	else if (_bFullStretched)
		startStretchMode = 2;
	else if (_bVStretched)
		startStretchMode = 3;
	else
		startStretchMode = 1;
	if (_StartStretchMode != startStretchMode) //v2.64追加
	{
		switch (_StartStretchMode)
		{
			case 0: //ノンストレッチ
				APP_SetStretched(FALSE, FALSE, FALSE);
				break;
			case 2: //フルストレッチ
				APP_SetStretched(TRUE, FALSE, TRUE);
				break;
			default: //1=リアルストレッチ
				APP_SetStretched(TRUE, FALSE, FALSE);
				break;
		}
		bUpdate = FALSE;
	}
	if (bUpdate)
		updateScreenMode(TRUE); //ウィンドウの大きさを変える必要があるため画面モードを再設定する。
	if (_ScanLineType == 4) //Settingメニューでのスタート"TV Mode"の場合はTV Modeに戻す。
	{
		if (!_bTvMode)
			APP_SetTvMode(TRUE, TRUE);
	}
	else //スタート"TV Mode"以外はTV Modeを解除
	{
		if (_bTvMode)
			APP_SetTvMode(FALSE, TRUE);
	}

	//前のゲームを遊んでいたときに、音量がレジュームファイル読み込みにより変更されていた場合、音量を元に戻す。
	if (_PrevWindowsVolume != -2)
	{
		_WindowsVolume = _PrevWindowsVolume;
		setWaveVolume(_WindowsVolume);
		_PrevWindowsVolume = -2;//復元完了
	}
}


//Kitao追加。ゲーム開始前(もしくは手動変更後)のCPU速度＆アーケードカード使用設定に戻す。
static void
restore_setting(
	BOOL	bOtherGame) //bOtherGame=他のゲームに変えた場合(HuカードのF1キーリセット以外の場合)、TRUEにして呼ぶ。
{
	if (bOtherGame) //ゲームが変わる場合
	{	//v1.15更新。前のゲームで速度アップしたことを忘れて、ビジュアルシーンなどで音ずれが起こることがあるので、ゲーム起動ごとにノーマルに戻すようにした。
		_OverClockTypeR = 0;
		_bUseVideoSpeedUpButton = TRUE; //早回しがAlwaysモードのときはUseButtonモードに戻す。
		if (_FastForwarding >= 2000) //早回しがSlow倍率モードのときはノーマル速度に戻す。
		{
			_FastForwarding = 0;
			CONFIG_Set("[APP DEVICE] VideoSpeedUp Mode", &_FastForwarding, sizeof(_FastForwarding));
		}
		//獣王記を起動していた場合、デフォルトのシステムカード使用に戻す。v2.07
		if (strcmp(TOCDB_GetGameTitle(),"Juuouki (J)") == 0) //獣王記→Huカード→その他CDゲームの時も元に戻すため_bCDGameがTRUEかどうかはチェックしない。
			_OtherSysCard = 0;
	}
	VDC_SetOverClock(_OverClockTypeR); //ステートロードで変わっていた場合元に戻る
	MAINBOARD_SetFastForwarding(_FastForwarding, _bSoundAjust, TRUE); //ステートロードで変わっていた場合元に戻る
	MAINBOARD_SetFastForwarding(0, _bSoundAjust, FALSE); //早回しボタンを離した状態にして起動
	MAINBOARD_SetArcadeCard(_bArcadeCardR); //アーケードカードの使用設定を元に戻す。(ステートロードで切り替わっていた場合に有効)
}


//Kitao追加。ゲーム開始前(もしくは手動変更後)のパッド設定に戻す。
static void
restore_pad()
{
	//連射速度設定と３ボタンパッド使用を通常使用の状態に戻す。
	_RenshaSpeedSel = _RenshaSpeedSelR;
	JOYPAD_SetRenshaSpeedButton1(_RenshaSpeedSel);
	JOYPAD_SetRenshaSpeedButton2(_RenshaSpeedSel);
	JOYPAD_SetRenshaSpeedButtonRun(_RenshaSpeedSel);
	JOYPAD_SetRenshaSpeedButton5(_RenshaSpeedSel);
	JOYPAD_SetRenshaSpeedButton6(_RenshaSpeedSel);
	_bUseThreeButton = _bUseThreeButtonR;
	JOYPAD_UseThreeButton(_bUseThreeButton);

	//パッドの基本設定を初期化する。
	JOYPAD_UseSixButton(FALSE);
	JOYPAD_ConnectMultiTap(TRUE);
	JOYPAD_SetSwapSelRun(FALSE);
	JOYPAD_SetSwapIandII(FALSE);
	JOYPAD_ConnectMouse(FALSE);
	JOYPAD_ConnectMB128(TRUE);
	JOYPAD_SetRenshaButton1(FALSE);
	JOYPAD_SetRenshaButton2(FALSE);
	JOYPAD_SetRenshaButtonRun(FALSE);
	JOYPAD_SetRenshaButton5(TRUE);
	JOYPAD_SetRenshaButton6(TRUE);
}


//Kitao追加。リセット処理
static void
app_reset_sub(
	BOOL	bResume) //同ゲームリセットでのレジュームを有効にして起動する場合（CUEファイル起動時で履歴メニューの一番上(同ゲーム)を選択してリセット時）はTRUEで呼ぶ。通常はFALSEで呼ぶ。
{
	char	prevGameFileNameBuf[MAX_PATH+1];

	set_userSetting();//サウンド、入力関係を設定どうりにする。

	if (_bCDGame)
	{//CDゲームを遊ぶ場合、TOCゲーム名を更新
		strcpy(prevGameFileNameBuf, _GameFileNameBuf);
		*_GameFileNameBuf = 0;
		strcat(_GameFileNameBuf, "(CD)");
		strcat(_GameFileNameBuf, TOCDB_GetGameTitle());
		//新しいゲーム名のBRAMを読み込み
		load_bram();
	}

	//HuカードゲームのときorCDを入れ替えずにリセットした場合、レジュームモードは一時無効にする。
	if ((bResume == FALSE)&&
		((_bCDGame == FALSE)||(strcmp(_GameFileNameBuf, prevGameFileNameBuf) == 0))) //HuカードかCDを入れ替えずにリセットした
	{
		if (_bResumeMode) //レジュームモードでリセットした
		{
			if (_bCDGame) //CDゲームのとき。(Huカードでリセットしたときはすでにメッセージを表示済み)
			{
				_bInit = TRUE; //再初期化完了にしてCDのタイトル＆注意メッセージを表示する。v1.67
				screenUpdate(); //メッセージ表示＆ダイアログ表示で黒くなった部分があるので再描画。
				MessageBox(WINMAIN_GetHwnd(),"\"Resume Mode\" is temporarily invalidated.    ", "Ootake", MB_YESNO); //リセット時にYESボタン連打で行けるようにMB_OKではなくMB_YESNOを使う
			}
			non_resume_temporarily(); //メニュー表示も更新される
		}
		else
			updateMenu(); //速度設定や、自動的に６ボタンパッドをつなげたソフトのためにメニュー表示を更新。
	}
	else
	{
		//ゲームが変わるので、縦ストレッチモードや"TV Mode"にしてあった場合は解除する。
		return_special_setting();
		//レジュームファイルがあれば読み込む
		if ((_OtherSysCard == 0)|| //安全のため、リセット(CD入れ替え)時は、デフォルトシステムカードのみレジュームを有効にする。
			((_OtherSysCard == 1)&&(_bCDGame)&&(strcmp(TOCDB_GetGameTitle(),"Juuouki (J)") == 0))) //獣王記の場合はレジューム有効に。v2.07
				load_resume();
		else
			_bResumeMode = FALSE;
		updateMenu(); //レジューム設定や速度設定、自動的に６ボタンパッドをつなげたソフトのためにメニュー表示を更新。
	}

	_bInit = TRUE; //再初期化完了。v1.67
	//マウス利用時にSetCursorPos()関数が使われると、ダイアログの残骸が表示されてしまうことがある(おそらくWindowsXPの仕様)ため、これが必要。v2.10追加
	UpdateWindow(WINMAIN_GetHwnd());
	//画像表示前に１フレームぶんCPUを動かす(起動時の画面乱れを見せないようにする)
	first_run_cpu();
}


//Kitao追加。Hu→CDに切り替え、またはCD入れ替え時の処理。正常にCDチェンジできた場合TRUEを返す。v1.67更新
static BOOL
app_cd_change(
	BOOL	bResume) //同ゲームリセットでのレジュームを有効にして起動する場合（CUEファイル起動時で履歴メニューの一番上(同ゲーム)を選択してリセット時）はTRUEで呼ぶ。通常はFALSEで呼ぶ。
{
	char	srcFilePathName[MAX_PATH+1] = "";

	end_playrecord();
	end_recording();
	save_resume();
	save_bram();

	//ゲーム開始前(もしくは手動変更後)のCPU速度＆パッド設定に戻す。
	restore_setting(TRUE);
	restore_pad();

	_bCDGame = TRUE;
	_bHesFile = FALSE;
	switch (_OtherSysCard)
	{
		case 0:
			strcpy(_OpenFilePathName, _AppPath);
			strcat(_OpenFilePathName, "SYSCARD.pce");
			break;
		case 1:
			strcpy(_OpenFilePathName, _OtherSysCardPath1);
			break;
		case 2:
			strcpy(_OpenFilePathName, _OtherSysCardPath2);
			break;
	}
	strcpy(_GameFilePathName, _OpenFilePathName);

	MAINBOARD_Deinit();
	_bInit = FALSE; //PRINTFによるメッセージ表示を初期化が完了するまで行わないようにするために必要。v1.67

	if (!MAINBOARD_Init(_OpenFilePathName))
	{
		//システムカードのイメージファイルを指定してもらう
		if (file_dialog(2, WINMAIN_GetHwnd(), srcFilePathName, _GameFilePathName))
		{
			SetFileAttributes(_OpenFilePathName, FILE_ATTRIBUTE_NORMAL); //読み込み専用ファイルにしてあった場合解除
			if (CopyFile(srcFilePathName, _OpenFilePathName, FALSE)) //実行フォルダ内へコピー
				SetFileAttributes(_OpenFilePathName, FILE_ATTRIBUTE_NORMAL); //読み込み専用ファイルにしてあった場合解除
		}
		//もう一度試みる
		if (!MAINBOARD_Init(_OpenFilePathName))
			return FALSE; //失敗
	}
	updateMenu(); //CD Change(NoReset)メニューのカレントドライブ表示を更新

	app_reset_sub(bResume);

	return TRUE;
}


//Kitao追加。リセット時の処理
static void
app_reset(
	Sint32	superGrafx)
{
	//リセットしたことがわかりやすいようにメッセージを消しておく
	APP_ReturnCaption();

	//画面をクリア
	MAINBOARD_ScreenClear();

	/* 記録・再生を終了させる */
	end_playrecord();
	end_recording();

	save_resume(); //レジューム一時停止前に、ここまでの状態をレジュームファイルに保存。

	//BRAMを保存
	save_bram();

	//ゲーム開始前(もしくは手動変更後)のCPU速度に戻す。
	restore_setting(FALSE);
	//CDゲームを遊んでいた場合、新しいゲームに入れ替えることを想定して、入力機器を初期化する。
	if ((_bCDGame)&&(!_bCueFile))
		restore_pad();

	//リセット処理
	_bInit = FALSE; //PRINTFによるメッセージ表示を初期化が完了するまで行わないようにするために必要。v1.67
	MAINBOARD_Reset();

	//スーパーグラフィックスモードを決定
	MAINBOARD_SetSuperGrafx(superGrafx);

	//設定更新や、CDゲームを遊んでいた場合の処理（レジュームも）などをおこなう。
	app_reset_sub(FALSE); //FALSE=同ゲームをリセットしたときにはレジュームを行わないようにする
}


//Kitao追加。ゲーム再オープン時の処理。正常にオープンできた場合TRUEを返す
static BOOL
app_game_reopen(
	BOOL	bResume) //レジュームを無効にして起動する場合（SYSTEMカード起動時）はFALSEで呼ぶ。
{
	BOOL	ret;
	char	fileName[MAX_PATH+1]; //ファイル名格納用

	//リオープンしたことがわかりやすいようにメッセージを消しておく
	APP_ReturnCaption();

	//ゲーム開始前(もしくは手動変更後)のCPU速度＆パッド設定に戻す。
	restore_setting(TRUE);
	restore_pad();

	//Kitao追加。拡張子をカットしたゲームファイル名を_GameFileNameBufに設定しておく（ステートセーブ時等で利用）
	SetGameFileNameBuf(_bCDGame, _bHesFile, _GameFilePathName, _GameFileNameBuf, FALSE);

	MAINBOARD_Deinit();
	_bInit = FALSE; //PRINTFによるメッセージ表示を初期化が完了するまで行わないようにするために必要。v1.67

	if (MAINBOARD_Init(_OpenFilePathName) == FALSE)
	{
		_bResumeMode = FALSE; //オープンに失敗した場合、確実にレジュームを無効にする（拡張子が違うセーブファイルなどを誤って開いた場合にレジュームファイルを壊さないために必要）
		if (bResume) //通常ゲームの起動の場合
		{
			//Huカードゲームを開き直してもらう
			MAINBOARD_Pause(TRUE); //ショートカットキーを押してここが呼び出されたときのために必要。
			WINMAIN_ShowCursor(TRUE); //ショートカットキーを押してここが呼び出されたときのために必要。
			MAINBOARD_ScreenClear();//画面をクリア。ゲームを変えるときはこうするとわかりやすい＆他のウィンドウが重なって画面が消えたあとにリセットした場合でも綺麗に再開できる。
			do {
				strcpy(fileName, _GameFilePathName); //Kitao追加
				if (file_dialog(1, WINMAIN_GetHwnd(), _OpenFilePathName, fileName)) //Kitao更新
				{
					_bCDGame = FALSE;//Kitao追加
					_bCueFile = FALSE;//v2.24追加
					strcpy(_GameFilePathName, fileName);//Kitao追加
					SetGameFileNameBuf(_bCDGame, _bHesFile, _GameFilePathName, _GameFileNameBuf, FALSE);
					ret = MAINBOARD_Init(_OpenFilePathName);
					if (ret)
						AddRecentRom();//正常に開けた場合、ROMイメージ履歴に追加。
				}
				else //ファイルダイアログをキャンセルした場合
					return FALSE; //失敗
			} while (ret == FALSE);
		}
		else
			return FALSE; //失敗
	}
	set_userSetting();//サウンド、入力関係を設定どうりにする。

	//新しいゲームのBRAMを読み込み
	load_bram();

	//ゲームが変わるので、縦ストレッチモードや"TV Mode"にしてあった場合は解除する。
	return_special_setting();

	//レジュームファイルがあれば読み込む
	if (bResume)
		load_resume();
	else
		_bResumeMode = FALSE;
	if (_bAutoResumeMode)
		_bResumeMode = TRUE;

	//レジューム設定や速度設定、自動的に６ボタンパッドをつなげたソフトのためにメニュー表示を更新。
	updateMenu();

	_bInit = TRUE; //再初期化完了。v1.67
	//マウス利用時にSetCursorPos()関数が使われると、ダイアログの残骸が表示されてしまうことがある(おそらくWindowsXPの仕様)ため、これが必要。v2.10追加
	UpdateWindow(WINMAIN_GetHwnd());
	//画像表示前に１フレームぶんCPUを動かす(起動時の画面乱れを見せないようにする)
	first_run_cpu();

	return TRUE;
}


//Kitao追加。CDROM.c から使用。メニューの更新も行う。ここは自動で頻繁に切り替えることも想定し、updateMenuを使わず高速に書き換えが済むようにする。
//			 現在、ぽっぷるメイルでのみ使用。
void
APP_SetAutoOverClock(
	Sint32	n)
{
	VDC_SetAutoOverClock(n);
	overClockMenuUpdate();
}


//Kitao追加。WinMain.c(ショートカットキー) から使用
void
APP_SetSpeedNormal()
{
	//Video速度をノーマルに
	_bUseVideoSpeedUpButton = TRUE; //早回しがAlwaysモードのときはUseButtonに戻す。
	if (MAINBOARD_GetFastForwardingR() >= 2000) //Slow倍率モードのときだけ、ノーマル速度に戻す。
	{
		_FastForwarding = 0;
		CONFIG_Set("[APP DEVICE] VideoSpeedUp Mode", &_FastForwarding, sizeof(_FastForwarding));
		MAINBOARD_SetFastForwarding(_FastForwarding, _bSoundAjust, TRUE);
	}

	//CPU速度をノーマルに
	if (VDC_GetAutoOverClock() != -1)
		VDC_SetAutoOverClock(VDC_GetAutoOverClock()); //自動でオーバークロックしてあるゲームの場合、その速度をデフォルトとする。
	else	
		VDC_SetOverClock(0); //通常のゲーム。ノーマルに戻す。
	_OverClockTypeR = 0;

	//CPUラスタタイミングをデフォルトに。v1.61
	if (VDC_GetAutoRasterTimingType() != 2)
		VDC_SetRasterTiming(VDC_GetAutoRasterTimingType()); //タイミングを自動変更してあるゲームの場合、そのタイミングをデフォルトとする。
	else
		VDC_SetRasterTiming(2); //通常のゲーム。ノーマルに戻す。

	PRINTF("Set Normal Speed&Timing.");
	updateMenu(); //メニュー更新
	if (!_bRunning) //フルスクリーン時に画面を(１フレーム一瞬だけ真っ黒に)乱さないようにするため、ショートカットキー(WinMain.cから)で呼び出された場合はrun_emulator()をコールしない。
		run_emulator(TRUE);
}


//Kitao追加
static void
set_rensha_speed(
	Sint32	n)
{
	char	buf[32];

	JOYPAD_SetRenshaSpeedButton1(n);
	JOYPAD_SetRenshaSpeedButton2(n);
	JOYPAD_SetRenshaSpeedButtonRun(n);
	JOYPAD_SetRenshaSpeedButton5(n);
	JOYPAD_SetRenshaSpeedButton6(n);
	_RenshaSpeedSel = n;
	CONFIG_Set("[APP INPUT] JoyPad TurboButton Speed", &_RenshaSpeedSel, sizeof(_RenshaSpeedSel));
	_RenshaSpeedSelR = _RenshaSpeedSel; //手動で変更した設定を退避
	
	if ((!JOYPAD_GetRenshaButton1())&&(!JOYPAD_GetRenshaButton2())&&(!JOYPAD_GetRenshaButtonRun()))
		strcpy(buf, " none");
	else
	{
		strcpy(buf, "");
		if (JOYPAD_GetRenshaButton1())
			strcat(buf, " I=on");
		if (JOYPAD_GetRenshaButton2())
			strcat(buf, " II=on");
		if (JOYPAD_GetRenshaButtonRun())
			strcat(buf, " RUN=on");
	}
	
	switch (n)
	{
		case 0:
			PRINTF("Set AutoFire Speed \"Off\". (%s )", buf);
			break;
		case 1:
			PRINTF("Set AutoFire Speed \"High\". (%s )", buf);
			break;
		case 2:
			PRINTF("Set AutoFire Speed \"Middle\". (%s )", buf);
			break;
		case 3:
			PRINTF("Set AutoFire Speed \"Low\". (%s )", buf);
			break;
	}
}

//Kitao追加
static void
set_shortcut_key_button(
	Sint32	mode)
//modeの値 2…早回し用ボタンの設定。3…ステートセーブ用ボタンの設定。4…ステートロード用ボタンの設定。5…スクリーンショット用ボタンの設定。
//		   6…ファンクション用ボタンの設定。7…ファンクションボタン時用のセーブ＆ロード用ボタンの設定。8…ポーズ用ボタンの設定。
{
	PCEPAD	pcePad; //ボタン設定用バッファ
	Sint32	setOk = 0;
	MSG		msg;
	BOOL	bToggle = FALSE;

	//PadConfigダイアログを表示
	if (_bFullScreen)
	{
		APP_ToggleFullscreen();
		run_emulator(FALSE);//メニュー表示などを乱さないように更新
		bToggle = TRUE;
	}
	PADCONFIG_Init(WINMAIN_GetHInstance(), mode, 1, &pcePad, &setOk);
	//メッセージループ
	while (setOk == 0) //ダイアログが結果を返すまでループ
	{ 
		GetMessage(&msg ,NULL, 0, 0);
		
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	if (bToggle) //フルスクリーンモードだった場合は元に戻す
	{
		APP_ToggleFullscreen();
		run_emulator(FALSE);//メニュー表示などを乱さないように更新
	}
	if (setOk == 1) //設定が成功した場合
	{
		switch (mode)
		{
			case 2: //早回し用ボタンの設定
				_VideoSpeedUpButton = pcePad.button1;
				CONFIG_Set("[APP DEVICE] VideoSpeedUp Button", &_VideoSpeedUpButton, sizeof(_VideoSpeedUpButton));
				PRINTF("VideoSpeedUp Button Set OK.");
				break;
			case 3: //ステートセーブ用ボタンの設定
				_SaveStateButton = pcePad.button1;
				if (_SaveStateButton == -1) _SaveStateButton = DIK_S;
				CONFIG_Set("[APP DEVICE] SaveState Button", &_SaveStateButton, sizeof(_SaveStateButton));
				PRINTF("SaveState Button Set OK.");
				break;
			case 4: //ステートロード用ボタンの設定
				_LoadStateButton = pcePad.button1;
				if (_LoadStateButton == -1) _LoadStateButton = DIK_L;
				CONFIG_Set("[APP DEVICE] LoadState Button", &_LoadStateButton, sizeof(_LoadStateButton));
				PRINTF("LoadState Button Set OK.");
				break;
			case 5: //スクリーンショット用ボタンの設定
				_ScreenshotButton = pcePad.button1;
				if (_ScreenshotButton == -1) _ScreenshotButton = DIK_SYSRQ;
				CONFIG_Set("[APP DEVICE] Screenshot Button", &_ScreenshotButton, sizeof(_ScreenshotButton));
				PRINTF("Screenshot Button Set OK.");
				break;
			case 6: //ファンクション用ボタンの設定
				_FunctionButton = pcePad.button1;
				CONFIG_Set("[APP DEVICE] Function Button", &_FunctionButton, sizeof(_FunctionButton));
				PRINTF("Function Button Set OK.");
				break;
			case 7: //ファンクションボタン時のステートセーブ＆ロード用ボタンの設定
				_FuncSaveStateButton = pcePad.button1;
				_FuncLoadStateButton = pcePad.button2;
				if ((_SaveStateButton == _FuncSaveStateButton)&&(_FuncSaveStateButton != -1))
					_SaveStateButton = DIK_S; //Fileメニューでのセーブボタン設定と被った場合、Fileメニューでのセーブボタンは初期化しておく。
				if ((_LoadStateButton == _FuncLoadStateButton)&&(_FuncLoadStateButton != -1))
					_LoadStateButton = DIK_L; //Fileメニューでのロードボタン設定と被った場合、Fileメニューでのロードボタンは初期化しておく。
				CONFIG_Set("[APP DEVICE] Function SaveButton", &_FuncSaveStateButton, sizeof(_FuncSaveStateButton));
				CONFIG_Set("[APP DEVICE] Function LoadButton", &_FuncLoadStateButton, sizeof(_FuncLoadStateButton));
				PRINTF("Function Save&LoadState Buttons Set OK.");
				break;
			case 8: //ポーズ用ボタンの設定
				_PauseButton = pcePad.button1;
				CONFIG_Set("[APP DEVICE] Pause Button", &_PauseButton, sizeof(_PauseButton));
				PRINTF("Pause Button Set OK.");
				break;
		}
	}
	run_emulator(TRUE);
}

//Kitao追加
static void
always_video_speed_up()
{
	char	buf[64];

	MENU_CheckRadioItem(_SpeedMenu, WM_SPEED_VAL, WM_SPEED_VUSE, WM_SPEED_VAL);
	_bUseVideoSpeedUpButton = FALSE;
	strcpy(buf, "Always VideoSpeedUp. ( ");
	switch (MAINBOARD_GetFastForwardingR())
	{
		case    0: strcat(buf, "x1.00"); break;
		case   10: strcat(buf, "x1.10"); break;
		case    5: strcat(buf, "x1.20"); break;
		case    3: strcat(buf, "x1.33"); break;
		case    2: strcat(buf, "x1.50"); break;
		case 1001: strcat(buf, "x1.67"); break;
		case 1000: strcat(buf, "x1.83"); break;
		case 2010: strcat(buf, "x0.90"); break;
		case 2004: strcat(buf, "x0.75"); break;
		case 2002: strcat(buf, "x0.50"); break;
	}
	strcat(buf, " )");
	PRINTF(buf);
	PRINTF_SetSaveLoadMessage(); //PRINTFメッセージの表示時間を少し短くする
	MAINBOARD_SetFastForwarding(MAINBOARD_GetFastForwardingR(), _bSoundAjust, TRUE);
	if (!_bRunning) //フルスクリーン時に画面を乱さないようにするため、ショートカットキー(WinMain.cから)で呼び出された場合はrun_emulator()をコールしない。
		run_emulator(TRUE);
}

//Kitao追加
static void
use_video_speed_up_button()
{
	char	buf[64];

	MENU_CheckRadioItem(_SpeedMenu, WM_SPEED_VAL, WM_SPEED_VUSE, WM_SPEED_VUSE);
	_bUseVideoSpeedUpButton = TRUE;
	strcpy(buf, "Use VideoSpeedUp Button. ( ");
	switch (MAINBOARD_GetFastForwardingR())
	{
		case    0:
		case 2010: //Slowのときに早回しボタンを押した場合、通常速度に戻す。
		case 2004: //"ボタンを押して一時的にスローにする機能"は、安易に使えすぎてアクション・シューティングゲームの楽しみを奪ってしまいそうなので搭載しないことにした。
		case 2002: //（RUNボタンのポーズ連射は、実機ジョイスティックやパッドにも同機能が付いていて懐かしさを味わえるため実装。）
			strcat(buf, "x1.00"); break;
		case   10: strcat(buf, "x1.10"); break;
		case    5: strcat(buf, "x1.20"); break;
		case    3: strcat(buf, "x1.33"); break;
		case    2: strcat(buf, "x1.50"); break;
		case 1001: strcat(buf, "x1.67"); break;
		case 1000: strcat(buf, "x1.83"); break;
	}
	strcat(buf, " )");
	PRINTF(buf);
	PRINTF_SetSaveLoadMessage(); //PRINTFメッセージの表示時間を少し短くする
	MAINBOARD_SetFastForwarding(MAINBOARD_GetFastForwardingR(), _bSoundAjust, TRUE);
	if (!_bRunning) //フルスクリーン時に画面を乱さないようにするため、ショートカットキー(WinMain.cから)で呼び出された場合はrun_emulator()をコールしない。
		run_emulator(TRUE);
}

//Kitao追加。v1.61更新
void
setVideoSpeedUpSub(
	Sint32	fastForwarding)
{
	_FastForwarding = fastForwarding;
	CONFIG_Set("[APP DEVICE] VideoSpeedUp Mode", &_FastForwarding, sizeof(_FastForwarding));
	MAINBOARD_SetFastForwarding(_FastForwarding, _bSoundAjust, TRUE);

	switch (_FastForwarding)
	{
		case 0:
			MENU_CheckRadioItem(_SpeedMenu, WM_SPEED_V0, WM_SPEED_V9, WM_SPEED_V0);
			PRINTF("Set Normal Video Speed."); break;
		case 2002:
			MENU_CheckRadioItem(_SpeedMenu, WM_SPEED_V0, WM_SPEED_V9, WM_SPEED_V9);
			PRINTF("Set VideoSpeed x0.50. (always)"); break;
		case 2004:
			MENU_CheckRadioItem(_SpeedMenu, WM_SPEED_V0, WM_SPEED_V9, WM_SPEED_V8);
			PRINTF("Set VideoSpeed x0.75. (always)"); break;
		case 2010:
			MENU_CheckRadioItem(_SpeedMenu, WM_SPEED_V0, WM_SPEED_V9, WM_SPEED_V7);
			PRINTF("Set VideoSpeed x0.90. (always)"); break;
		case 10:
			MENU_CheckRadioItem(_SpeedMenu, WM_SPEED_V0, WM_SPEED_V9, WM_SPEED_V1);
			if (_bUseVideoSpeedUpButton) PRINTF("Set VideoSpeed x1.10. (use button)");
			else PRINTF("Set VideoSpeed x1.10. (always)"); break;
		case 5:
			MENU_CheckRadioItem(_SpeedMenu, WM_SPEED_V0, WM_SPEED_V9, WM_SPEED_V2);
			if (_bUseVideoSpeedUpButton) PRINTF("Set VideoSpeed x1.20. (use button)");
			else PRINTF("Set VideoSpeed x1.20. (always)"); break;
		case 3:
			MENU_CheckRadioItem(_SpeedMenu, WM_SPEED_V0, WM_SPEED_V9, WM_SPEED_V3);
			if (_bUseVideoSpeedUpButton) PRINTF("Set VideoSpeed x1.33. (use button)");
			else PRINTF("Set VideoSpeed x1.33. (always)"); break;
		case 2:
			MENU_CheckRadioItem(_SpeedMenu, WM_SPEED_V0, WM_SPEED_V9, WM_SPEED_V4);
			if (_bUseVideoSpeedUpButton) PRINTF("Set VideoSpeed x1.50. (use button)");
			else PRINTF("Set VideoSpeed x1.50. (always)"); break;
		case 1001:
			MENU_CheckRadioItem(_SpeedMenu, WM_SPEED_V0, WM_SPEED_V9, WM_SPEED_V5);
			if (_bUseVideoSpeedUpButton) PRINTF("Set VideoSpeed x1.67. (use button)");
			else PRINTF("Set VideoSpeed x1.67. (always)"); break;
		case 1000:
			MENU_CheckRadioItem(_SpeedMenu, WM_SPEED_V0, WM_SPEED_V9, WM_SPEED_V6);
			if (_bUseVideoSpeedUpButton) PRINTF("Set VideoSpeed x1.83. (use button)");
			else PRINTF("Set VideoSpeed x1.83. (always)"); break;
	}
}


//Kitao追加
static BOOL
recent_sub(
	int		n)
{
	char	fileName[MAX_PATH+1];

	if (strcmp(_RecentRom[n], " ") != 0) //履歴あった場合
	{
		MAINBOARD_ScreenClear();//画面をクリア
		end_playrecord(); end_recording(); save_resume(); save_bram();

		strcpy(fileName, _RecentRom[n]); //_RecentRom[1]は書き換えないようにするためfileNameにコピー
		if (strstr(strToLower(extractFileExt(fileName)), "cue")) //CUEファイルの場合、CDゲームとして起動。v2.24追加
		{
			strcpy(_CueFilePathName, _RecentRom[n]);
			_bCueFile = TRUE; //CUEファイル有効
			if (app_cd_change(TRUE)) //_bCDGame等の変数設定やrun_emulator()も行われる
				AddRecentRom();//正常に開けた場合、ROMイメージ履歴リストの一番上に持ってくる。
			else
				return FALSE; //システムカードのイメージ読み込み失敗のため、強制終了。
		}
		else //通常
		{
			_bCDGame = FALSE;
			_bCueFile = FALSE;//v2.24追加
			set_openFilePathName(_RecentRom[n], _OpenFilePathName, _GameFilePathName);
			SetGameFileNameBuf(_bCDGame, _bHesFile, _GameFilePathName, _GameFileNameBuf, FALSE);
			if (app_game_reopen(TRUE))
				AddRecentRom();//正常に開けた場合、ROMイメージ履歴リストの一番上に持ってくる。
			else
				return FALSE; //開けないファイルを開こうとして失敗した
		}
	}
	else
		run_emulator(TRUE);

	return TRUE;
}

//Kitao追加
static void
cdchange_noreset_sub(
	int		n)
{
	BOOL	bCueFile;

	MAINBOARD_PauseNoMessage(TRUE);
	end_playrecord();	end_recording(); save_resume(); save_bram();

	bCueFile = _bCueFile; //退避
	_bCueFile = FALSE;
	CDROM_NoResetCDChange(n); //CD-ROMの情報を再設定
	_bCueFile = bCueFile; //元に戻す

	run_emulator(TRUE);
}

//Kitao追加。オーバースキャン領域表示切替のサブ
void
setShowOverscanArea(
	BOOL	bShowOverscan)
{
	char	buf[7+1];

	MAINBOARD_PauseNoMessage(TRUE); //F11キーで切り替えたときに、音を乱さないためにポーズ。

	if (bShowOverscan)
	{
		MAINBOARD_SetShowOverscanTop(_ShowOverscanTop);
		MAINBOARD_SetShowOverscanBottom(_ShowOverscanBottom);
		MAINBOARD_SetShowOverscanLeft(_ShowOverscanLeft);
		MAINBOARD_SetShowOverscanRight(_ShowOverscanRight);
		strcpy(buf, "");
		if ((_ShowOverscanTop == 0)&&(_ShowOverscanBottom == 0)&&(_ShowOverscanLeft == 0)&&(_ShowOverscanRight == 0))
			strcat(buf, "none ");
		else
		{
			if (_ShowOverscanTop > 0)
				strcat(buf, "T ");
			if (_ShowOverscanBottom > 0)
				strcat(buf, "B ");
			if (_ShowOverscanLeft > 0)
				strcat(buf, "LR ");
		}
		PRINTF("Show Overscan Area. ( %s%d )", buf,_ShowOverscanHeight);
	}
	else
	{
		_ShowOverscanTop = 0;
		_ShowOverscanBottom = 0;
		_ShowOverscanLeft = 0;
		_ShowOverscanRight = 0;
		MAINBOARD_SetShowOverscanTop(0);
		MAINBOARD_SetShowOverscanBottom(0);
		MAINBOARD_SetShowOverscanLeft(0);
		MAINBOARD_SetShowOverscanRight(0);
		PRINTF("Hide Overscan Area.");
	}
	VDC_SetTvStartPos(); //TvStartPosを更新する
	if (!_bFullScreen)
	{
		updateScreenMode(TRUE); //ウィンドウモードのときだけ、ウィンドウの大きさを変える必要があるため画面モードを再設定する。メニューもアップデートされる。
		if ((_DrawMethod == 2)&&(bShowOverscan)&&(MAINBOARD_GetFourSplitScreen())) //DirectDrawで４分割画面表示中の場合。v2.27追加
			SCREEN_Clear(); //表示範囲外になった部分のゴミがあればクリアする。余分にこれをやるとちらつくので、必要時以外はしないで済むようにすること。
	}
	else
	{
		updateMenu();
		if ((_bFullStretched)||(!bShowOverscan)) //フルストレッチか、オフにする場合のみ。v2.27更新
			SCREEN_Clear(); //表示範囲外になった部分のゴミがあればクリアする。余分にこれをやるとちらつくので、必要時以外はしないで済むようにすること。
	}
}

//Kitao追加。スタート時のオーバースキャン領域表示切替のサブ
void
setStartShowOverscanArea()
{
	_ShowOverscanTop	= _StartShowOverscanTop;
	_ShowOverscanBottom	= _StartShowOverscanBottom;
	_ShowOverscanLeft	= _StartShowOverscanLeft;
	_ShowOverscanRight	= _StartShowOverscanRight;
	_ShowOverscanHeight	= _StartShowOverscanHeight;
	setShowOverscanArea(TRUE);
}

//Kitao追加。レジューム機能オンオフ切替のサブ
void
toggleResume()
{
	char		fn[MAX_PATH+1]; //Kitao追加。ファイルネーム格納用
	FILE*		fp; //Kitao追加
	BOOL		ok; //Kitao追加

	strcpy(fn, _AppPath);
	strcat(fn, "save\\");//saveフォルダ
	strcat(fn, _GameFileNameBuf);
	strcat(fn, "_resume.dat");//fn=レジュームファイル名
	if (_bResumeMode)
	{
		//レジュームを解除していいかどうかの確認
		screenUpdate(); //メニューやダイアログ表示などで黒くなった部分を再描画。
		if (MessageBox(WINMAIN_GetHwnd(),
			"\"Resume Mode\" released Ok?    ",
			"Ootake", MB_YESNO) == IDYES)
		{
			if ((fp = fopen(fn, "r")) != NULL) //レジュームファイルが存在すれば
			{
				fclose(fp);
				//レジュームファイルを残すかどうかの確認
				screenUpdate(); //メニューやダイアログ表示などで黒くなった部分を再描画。
				if (MessageBox(WINMAIN_GetHwnd(),
					"\"Resume File\" reserved?    ",
					"Ootake", MB_YESNO) == IDNO)
				{	//レジュームファイルを削除
					SetFileAttributes(fn, FILE_ATTRIBUTE_NORMAL); //読み込み専用ファイルにしてあった場合に必要
					remove(fn);
				}
				else //レジュームファイルを削除しない場合は、ここでレジュームファイルを更新しておく。
					save_resume();
			}
			MENU_CheckItem(_FileMenu, WM_SET_RESUME, FALSE);
			_bResumeMode = FALSE;
			PRINTF("Non-Resume Mode.");
		}
	}
	else
	{
		//すでにレジュームファイルがあった場合、古いレジュームファイルは上書きされる旨を確認。
		ok = TRUE;
		if ((fp = fopen(fn, "r")) != NULL)
		{
			fclose(fp);
			screenUpdate(); //メニューやダイアログ表示などで黒くなった部分を再描画。
			if (MessageBox(WINMAIN_GetHwnd(),
				"There is already the \"Resume File\".    \nOverwrite Ok?",
				"Ootake", MB_YESNO) != IDYES)
					ok = FALSE;
		}
		if (ok)
		{
			MENU_CheckItem(_FileMenu, WM_SET_RESUME, TRUE);
			_bResumeMode = TRUE;
			PRINTF("Set Resume Mode.");
			if (MAINBOARD_GetSystemTime() == 0xFFFFFFFF) //「古いバージョンでスタートしたステート」をロードして経過時間が不明な場合、ここで経過時間を0にリセット。v2.63
				MAINBOARD_ResetSystemTime();
		}
	}
}


//Kitao追加。v1.61更新
void
setOverscanSub(
	Sint32	height)
{
	_ShowOverscanHeight = height;
	if (_ShowOverscanTop	> 0) _ShowOverscanTop = _ShowOverscanHeight;
	if (_ShowOverscanBottom	> 0) _ShowOverscanBottom = _ShowOverscanHeight;
	setShowOverscanArea(TRUE);
}

//Kitao追加。v1.61更新
void
setStartOverscanSub(
	Sint32	height)
{
	_StartShowOverscanHeight = height;
	if (_StartShowOverscanTop	 > 0) _StartShowOverscanTop = _StartShowOverscanHeight;
	if (_StartShowOverscanBottom > 0) _StartShowOverscanBottom = _StartShowOverscanHeight;
	setStartShowOverscanArea();
	CONFIG_Set("[APP DISPLAY] Overscan Height", &_StartShowOverscanHeight, sizeof(_StartShowOverscanHeight));
	CONFIG_Set("[APP DISPLAY] Overscan Top", &_StartShowOverscanTop, sizeof(_StartShowOverscanTop));
	CONFIG_Set("[APP DISPLAY] Overscan Bottom", &_StartShowOverscanBottom, sizeof(_StartShowOverscanBottom));
}


//Kitao追加。v2.32
void
setCddaAdjust(
	Sint32	cddaAdjust)
{
	char	buf[16];
	char	buf2[16];

	cddaAdjust = cddaAdjust * 10 + _CddaAdjustFineTune;

	CDROM_ClearCDDAAjustCount(); //再設定時には必ずカウンタをクリア
	ADPCM_ClearCDDAAjustCount(); //

	if (_bFullScreen)
	{
		_CddaAdjustFull[_FullMagnification] = cddaAdjust;
		CONFIG_Set("[APP SOUND] CDDA Adjust FullScreen", _CddaAdjustFull, sizeof(_CddaAdjustFull));
		sprintf(buf2, "(%dx%d)", _ScreenWidth,_ScreenHeight);
	}
	else
	{
		_CddaAdjust = cddaAdjust;
		CONFIG_Set("[APP SOUND] CDDA Adjust Window", &_CddaAdjust, sizeof(_CddaAdjust));
		strcpy(buf2, "(Window)");
	}

	sprintf(buf, "%d", cddaAdjust);
	if ((cddaAdjust % 10) == 0)
	{
		buf[4] = 0;
		buf[3] = buf[2];
		buf[2] = '.';
	}
	else
	{
		buf[5] = 0;
		buf[4] = buf[3];
		buf[3] = buf[2];
		buf[2] = '.';
	}

	PRINTF("Adjust CD-DA V%sHz %s.", buf,buf2);
	updateMenu();
}


//Kitao追加。v2.33
static void
setCddaDelayFrame(
	Sint32	cddaDelayFrame)
{
	_CddaDelayFrame = cddaDelayFrame;
	CONFIG_Set("[APP SOUND] CDDA Delay Frame", &_CddaDelayFrame, sizeof(_CddaDelayFrame));
	PRINTF("CD-DA Delay Frame [ %d ].", _CddaDelayFrame);
	updateMenu();
}


//Kitao追加。v2.38
static void
ToggleInputTurbo1()
{
	char	buf[16];

	strcpy(buf, "");
	if (JOYPAD_GetRenshaButton1())
	{
		JOYPAD_SetRenshaButton1(FALSE);
		if (!JOYPAD_GetConnectSixButton())
			JOYPAD_SetRenshaButton5(TRUE);
		//if (!JOYPAD_GetRenshaButton2())
		//	strcat(buf, "[ II ]");
		//if (!JOYPAD_GetRenshaButtonRun())
		//	strcat(buf, "[RUN]");
		PRINTF("Reset AutoFire Button [ I ]%s .", buf);
	}
	else
	{
		JOYPAD_SetRenshaButton1(TRUE);
		JOYPAD_SetRenshaButton5(FALSE);
		if (JOYPAD_GetRenshaButton2())
			strcat(buf, "[ II ]");
		if (JOYPAD_GetRenshaButtonRun())
			strcat(buf, "[RUN]");
		switch (_RenshaSpeedSel)
		{
			case 0: PRINTF("Set AutoFire Button [ I ]%s . ( Speed Off )",buf); break;
			case 1: PRINTF("Set AutoFire Button [ I ]%s . ( Speed High )",buf); break;
			case 2: PRINTF("Set AutoFire Button [ I ]%s . ( Speed Middle )",buf); break;
			case 3: PRINTF("Set AutoFire Button [ I ]%s . ( Speed Low )",buf); break;
		}
	}
	MENU_CheckItem(_InputMenu, WM_INPUT_TURBO_1, JOYPAD_GetRenshaButton1());
}

//Kitao追加。v2.38
static void
ToggleInputTurbo2()
{
	char	buf[16];

	strcpy(buf, "");
	if (JOYPAD_GetRenshaButton2())
	{
		JOYPAD_SetRenshaButton2(FALSE);
		if (!JOYPAD_GetConnectSixButton())
			JOYPAD_SetRenshaButton6(TRUE);
		//if (!JOYPAD_GetRenshaButton1())
		//	strcat(buf, "[ I ]");
		//if (!JOYPAD_GetRenshaButtonRun())
		//	strcat(buf, "[RUN]");
		PRINTF("Reset AutoFire Button [ II ]%s .", buf);
	}
	else
	{
		JOYPAD_SetRenshaButton2(TRUE);
		JOYPAD_SetRenshaButton6(FALSE);
		if (JOYPAD_GetRenshaButton1())
			strcat(buf, "[ I ]");
		if (JOYPAD_GetRenshaButtonRun())
			strcat(buf, "[RUN]");
		switch (_RenshaSpeedSel)
		{
			case 0: PRINTF("Set AutoFire Button [ II ]%s . ( Speed Off )",buf); break;
			case 1: PRINTF("Set AutoFire Button [ II ]%s . ( Speed High )",buf); break;
			case 2: PRINTF("Set AutoFire Button [ II ]%s . ( Speed Middle )",buf); break;
			case 3: PRINTF("Set AutoFire Button [ II ]%s . ( Speed Low )",buf); break;
		}
	}
	MENU_CheckItem(_InputMenu, WM_INPUT_TURBO_2, JOYPAD_GetRenshaButton2());
}

//Kitao追加。v2.38
static void
ToggleInputTurboRun()
{
	char	buf[16];

	strcpy(buf, "");
	if (JOYPAD_GetRenshaButtonRun())
	{
		JOYPAD_SetRenshaButtonRun(FALSE);
		//if (!JOYPAD_GetRenshaButton1())
		//	strcat(buf, "[ I ]");
		//if (!JOYPAD_GetRenshaButton2())
		//	strcat(buf, "[ II ]");
		PRINTF("Reset AutoFire Button [RUN] .", buf);
	}
	else
	{
		JOYPAD_SetRenshaButtonRun(TRUE);
		if (JOYPAD_GetRenshaButton1())
			strcat(buf, "[ I ]");
		if (JOYPAD_GetRenshaButton2())
			strcat(buf, "[ II ]");
		switch (_RenshaSpeedSel)
		{
			case 0: PRINTF("Set AutoFire Button [RUN]%s . ( Speed Off )",buf); break;
			case 1: PRINTF("Set AutoFire Button [RUN]%s . ( Speed High )",buf); break;
			case 2: PRINTF("Set AutoFire Button [RUN]%s . ( Speed Middle )",buf); break;
			case 3: PRINTF("Set AutoFire Button [RUN]%s . ( Speed Low )",buf); break;
		}
	}
	MENU_CheckItem(_InputMenu, WM_INPUT_TURBO_RUN, JOYPAD_GetRenshaButtonRun());
}


//Kitao追加。v2.38
static void
VolumeUp()
{
	Sint32	wv;

	if (_WindowsVolume == -1)
		_WindowsVolume = (Sint32)((double)_dwDefaultWindowsVolumeValue/(double)_dwWaveMaxVolume * 100 +0.99999); //小数点第一位切り上げ
	wv = _WindowsVolume + _VolumeStep;
	wv /= _VolumeStep; //_VolumeStep(デフォルト=4)未満の端数を切り捨て
	wv *= _VolumeStep; //_VolumeStep(デフォルト=4)の倍数に整え完了
	if ((_WindowsVolume<100)&&(wv>100)) wv=100; //v2.62追加。100を越える時はいったん100にする。
	if (wv>120) wv=120;
	_WindowsVolume = wv;
	setWindowsVolume(_WindowsVolume);
	rewind(stdin); //キーバッファのクリア。処理が遅い場合があるので、連続して音量キーを押したままにするとバッファが溜まってしまうため必要。v2.20
}

//Kitao追加。v2.38
static void
VolumeDown()
{
	Sint32	wv;

	if (_WindowsVolume == -1)
		_WindowsVolume = (Sint32)((double)_dwDefaultWindowsVolumeValue/(double)_dwWaveMaxVolume * 100); //小数点第一位切り捨て
	wv = _WindowsVolume - _VolumeStep;
	wv = (wv + (_VolumeStep - 1)) / _VolumeStep; //_VolumeStep(デフォルト=4)未満の端数を切り上げ
	wv *= _VolumeStep; //_VolumeStep(デフォルト=4)の倍数に整え完了
	if ((_WindowsVolume>100)&&(wv<100)) wv=100; //v2.62追加。100をまたぐ時はいったん100にする。
	if (wv<1) wv=1;
	_WindowsVolume = wv;
	setWindowsVolume(_WindowsVolume);
	rewind(stdin); //キーバッファのクリア。処理が遅い場合があるので、連続して音量キーを押したままにするとバッファが溜まってしまうため必要。v2.20
}

//Kitao追加。v2.38
static void
VolumeDetailUp()
{
	if (_WindowsVolume == -1)
		_WindowsVolume = (Sint32)((double)_dwDefaultWindowsVolumeValue/(double)_dwWaveMaxVolume * 100 +0.99999); //小数点第一位切り上げ
	_WindowsVolume++;
	if (_WindowsVolume>120) _WindowsVolume=120;
	setWindowsVolume(_WindowsVolume);
	rewind(stdin); //キーバッファのクリア。処理が遅い場合があるので、連続して音量キーを押したままにするとバッファが溜まってしまうため必要。v2.20
}

//Kitao追加。v2.38
static void
VolumeDetailDown()
{
	if (_WindowsVolume == -1)
		_WindowsVolume = (Sint32)((double)_dwDefaultWindowsVolumeValue/(double)_dwWaveMaxVolume * 100); //小数点第一位切り捨て
	_WindowsVolume --;
	if (_WindowsVolume<1) _WindowsVolume=1;
	setWindowsVolume(_WindowsVolume);
	rewind(stdin); //キーバッファのクリア。処理が遅い場合があるので、連続して音量キーを押したままにするとバッファが溜まってしまうため必要。v2.20
}


//Kitao追加。v2.48
static BOOL
FileDialogSub()
{
	char	fn[MAX_PATH+1];

	MAINBOARD_ScreenClear();//画面をクリア。ゲームを変えるときはこうするとわかりやすい＆他のウィンドウが重なって画面が消えたあとにリセットした場合でも綺麗に再開できる。
	strcpy(fn, _GameFilePathName); //Kitao追加。元のゲームの_GameFilePathNameをいったん退避
	if (file_dialog(1, WINMAIN_GetHwnd(), _OpenFilePathName, fn)) //Kitao更新
	{
		end_playrecord();
		end_recording();
		save_resume();//Kitao追加
		save_bram();
		
		_bCDGame = FALSE;//Kitao追加
		_bCueFile = FALSE;//v2.24追加
		strcpy(_GameFilePathName, fn);//Kitao追加
		if (app_game_reopen(TRUE))//Kitao更新
			AddRecentRom();//正常に開けた場合、ROMイメージ履歴に追加。
		else
			return FALSE; //開けないファイルを開こうとして失敗したため、強制終了。
	}
	else
		run_emulator(TRUE);
	return TRUE;
}

//Kitao追加。v2.48
static BOOL
FolderDialogSub()
{
	char	fn[MAX_PATH+1];
	char	srcFilePathName[MAX_PATH+1];
	FILE*	fp;
	BOOL	bOk;

	MAINBOARD_ScreenClear();//画面をクリア。ゲームを変えるときはこうするとわかりやすい＆他のウィンドウが重なって画面が消えたあとにリセットした場合でも綺麗に再開できる。

	bOk = folder_dialog(WINMAIN_GetHwnd(), fn);
	strcpy(srcFilePathName, fn);
	strcat(fn, "\\");
	strcat(fn, extractFileName(srcFilePathName));
	strcat(fn, ".cue");
	strToLower(srcFilePathName);
	if ((bOk)&&(strcmp(extractFileName(srcFilePathName),"install") != 0)) //ルート(install)フォルダが選択された場合を除く
	{
		if ((fp = fopen(fn, "r")) != NULL) //cueファイルが存在すれば
		{
			fclose(fp);
			strcpy(_CueFilePathName, fn);
			_bCueFile = TRUE; //CUEファイル有効
			if (!app_cd_change(TRUE)) //run_emulator()も行われる。TRUE=同じゲームを開いたときにレジュームを無効にしない
				return FALSE; //システムカードのイメージ読み込み失敗のため、強制終了。
			AddRecentRom(); //CUEファイルを履歴に追加する
		}
		else
			PRINTF("\"%s\" is not found. Do \"FullInstall\".", extractFileName(fn));
	}
	run_emulator(TRUE);
	return TRUE;
}


/*----------------------------------------------------------------------------
	[handle_event]
		アプリケーションに対して発生したイベントに対する処理を行ないます。
----------------------------------------------------------------------------*/
static BOOL
handle_event(
	Sint32		hdEvent)
{
	int			i;
	BOOL		bRunNormalState;
	char		buf[2048];  //Kitao追加。文字列演算用
	char		buf2[2048]; //
	char		buf3[2048]; //
	char		fn[MAX_PATH+1]; //Kitao追加。ファイルネーム格納用
	char		srcFilePathName[MAX_PATH+1]; //Kitao追加
	char		dstFilePathName[MAX_PATH+1]; //Kitao追加
	Sint32		resolutionWidth; //Kitao追加
	Sint32		resolutionHeight; //Kitao追加
	MSG			msg;
	RECT		rc;
	STARTUPINFO			StartupInfo;
	PROCESS_INFORMATION	ProcessInfo;
	FILE*		fp;
	BOOL		bOk;
	Sint32		vsf;

#ifdef DEBUGBUILD
	bRunNormalState = FALSE;
#else
	bRunNormalState = TRUE;
#endif

	/* イベント処理 */
	switch (hdEvent)
	{
		case APPEVENT_REDRAWSCREEN: //Kitao更新
			if ((_bFullScreen)||(!MAINBOARD_GetPause()))
				screenUpdate(); //描画更新
			break;

		case APPEVENT_SHOWMENU:
			//run_emulator(FALSE);
			break;

		case APPEVENT_HIDEMENU:
			run_emulator(TRUE);//unused?
			break;

		case APPEVENT_RUN:
			run_emulator(_bRunning ^ TRUE);
			break;

//		case APPEVENT_PAUSE:
//			run_emulator(FALSE);
//			break;

		case APPEVENT_ADVANCEFRAME_1:	advance_frames(1);			break;
//		case APPEVENT_ADVANCEFRAME_10:	advance_frames(10);			break;
//		case APPEVENT_ADVANCEFRAME_100:	advance_frames(100);		break;

		//Kitao追加。v2.49
		case APPEVENT_PAUSE_BUTTON: //ポーズ用のボタンを設定
			set_shortcut_key_button(8);
			break;

		//Kitao追加。v2.39
		case APPEVENT_WRITE_MEMORY:
			APP_ShowWriteMemoryForm(WINMAIN_GetHwnd(), WINMAIN_GetHInstance());
			screenUpdate(); //ダイアログ表示などで黒くなった部分を再描画。
			run_emulator(bRunNormalState);
			break;
		case APPEVENT_ABOUT_WRITEMEM:
			screenUpdate(); //メニューやダイアログ表示などで黒くなった部分を再描画。
			MessageBox(	WINMAIN_GetHwnd(),
						"This \"Write Memory\" menu can be used to execute a so-called cheat    \n"
						"command. For instance, BGM can be appreciated without being    \n"
						"obstructed to the sound effect (attack sound and jump sound, etc.)    \n"
						"by using the time limitation release and the invincibility command    \n"
						"etc. (Within the range of the personal hobby)    \n"
						"On the other hand, a cheat command when using it by \"Purpose to    \n"
						"clear the game easy\" is not recommended. Because, it becomes a    \n"
						"game balance that the game production person doesn't intend, and the    \n"
						"interest of the game is surely lost. Moreover, it becomes impossible    \n"
						"to talk about the work with \"person who enjoyed it by normal play\".    \n"
						"Therefore, I think that playing only by normal-play(non-cheat-play)    \n"
						"will be consequentially happy (It can deeply enjoy playing the game).    \n"
						"\"GAME OVER\" should be welcome. Even if it cannot be cleared, the    \n"
						"game is happy. I think that it is a most wonderful point of games (    \n"
						"especially, in the retro games). Personally, if you can use this    \n"
						"menu in the purpose of the research and development, I am glad.    \n\n"
						"In Japanese language\n"
						"この\"Write Memory\"メニューは、いわゆるチートコマンドの実行にも利用   \n"
						"できます。無敵コマンドや時間制限解除などを利用して、効果音(攻撃音や   \n"
						"ジャンプ音など)を出さずにBGM鑑賞をしたりなどで、個人的な趣味の範囲で   \n"
						"有効に使ってもらえるかと思います。\n"
						"一方で、\"ゲームを楽ちんにクリアする目的\"で使った場合のチートコマン   \n"
						"ドは、ゲーム制作者の方が意図しないバランスになってしまって、確実にゲ   \n"
						"ームがつまらなくなりますし、\"通常のプレイで楽しんだ人\"とその作品に   \n"
						"ついて語ったりすることもできなくなります。ですので、通常プレイだけで   \n"
						"遊ぶほうが結果的に幸せな（ゲームを深く楽しめる）場合が多いのではない   \n"
						"かと考えています。\n"
						"ゲームオーバー上等。クリアできなくても面白いのがゲーム本来の良さ(特に   \n"
						"レトロゲーム)だと思います。個人的には、\"Write Memory\"メニューは、研   \n"
						"究・開発の目的において使ってもらえると嬉しいです。\n",
						"About \"Write Memory\" Menu", MB_OK);
			run_emulator(bRunNormalState);
			break;

		case APPEVENT_FILEDIALOG:
			MAINBOARD_Pause(TRUE); //Kitao追記：ショートカットキーを押してここが呼び出されたときのために必要。
			WINMAIN_ShowCursor(TRUE); //Kitao追記：ショートカットキーを押してここが呼び出されたときのために必要。
			if (!FileDialogSub()) //ファイルダイアログを表示
					return FALSE; //開けないファイルを開こうとして失敗したため、強制終了。
			break;

		case APPEVENT_OPENGAME: //ウィンドウにファイルをドロップされたとき。Kitao更新
			MAINBOARD_PauseNoMessage(TRUE);
			MAINBOARD_ScreenClear();//画面をクリア。ゲームを変えるときはこうするとわかりやすい＆他のウィンドウが重なって画面が消えたあとにリセットした場合でも綺麗に再開できる。
			strcpy(fn, _GameFilePathName); //Kitao追加。元のゲームの_GameFilePathNameをいったん退避
			strcpy(srcFilePathName, (char*)APPEVENT_GetParam(hdEvent));
			if (set_openFilePathName(srcFilePathName, _OpenFilePathName, fn))
			{
				end_playrecord();
				end_recording();
				save_resume();//Kitao追加
				save_bram();
				
				_bCDGame = FALSE;//Kitao追加
				_bCueFile = FALSE;//v2.24追加
				strcpy(_GameFilePathName, fn);//Kitao追加
				if (app_game_reopen(TRUE))//Kitao更新
					AddRecentRom();//正常に開けた場合、ROMイメージ履歴に追加。
				else
					return FALSE; //開けないファイルを開こうとして失敗したため、強制終了。
			}
			else
				run_emulator(bRunNormalState);
			break;

		//Kitao追加。CD-ROMを別のCD-ROMに入れ替え、リセットする。もしくは、Huカードを遊んでいたときに、CD-ROMゲームに切り替える。
		case APPEVENT_CDCHANGE:
			MAINBOARD_Pause(TRUE);
			WINMAIN_ShowCursor(TRUE);
			MAINBOARD_ScreenClear();//画面をクリア。ゲームを変えるときはこうするとわかりやすい＆他のウィンドウが重なって画面が消えたあとにリセットした場合でも綺麗に再開できる。
			if (_bCDGame)
				strcpy(buf, "Replace");
			else
				strcpy(buf, "Put");
			strcat(buf, " CD-ROM, and click the \"Yes\" button.    \n( cancel = \"No\" button )");
			if (MessageBox(WINMAIN_GetHwnd(), buf, "CD Change & Reset", MB_YESNO) == IDYES)
			{
				_bInvalidateCdInstall = FALSE; //CDインストール機能を有効に。v2.34追加
				_bCueFile = FALSE; //v2.24追加
				if (!app_cd_change(FALSE)) //run_emulator()も行われる。FALSE=CDを入れ替えずにF1キーでリセットした場合に、レジュームを無効にする。
					return FALSE; //システムカードのイメージ読み込み失敗のため、強制終了。
			}
			else
				run_emulator(bRunNormalState);
			break;

		//Kitao追加。リセットはしないでCD-ROMを別のCD-ROMに入れ替える。
		//			 「ＣＤバトル 光の勇者たち」や、内蔵の音楽CDプレイヤーで使う。もし同人ソフトなどで２枚組みのゲームがあればそれにも対応できる。
		case APPEVENT_CHANGE_CDC: //カレント(前回と同じ)ドライブ。初設定のときは初期ドライブ。
			cdchange_noreset_sub(CDIF_GetDeviceInUse()); break;
		case APPEVENT_CHANGE_CD0:
			cdchange_noreset_sub(0); break;
		case APPEVENT_CHANGE_CD1:
			cdchange_noreset_sub(1); break;
		case APPEVENT_CHANGE_CD2:
			cdchange_noreset_sub(2); break;
		case APPEVENT_CHANGE_CD3:
			cdchange_noreset_sub(3); break;
		case APPEVENT_CHANGE_CD4:
			cdchange_noreset_sub(4); break;
		case APPEVENT_CHANGE_CD5:
			cdchange_noreset_sub(5); break;
		case APPEVENT_CHANGE_CD6:
			cdchange_noreset_sub(6); break;
		case APPEVENT_CHANGE_CD7:
			cdchange_noreset_sub(7); break;
		case APPEVENT_CHANGE_CD8:
			cdchange_noreset_sub(8); break;
		case APPEVENT_CHANGE_CD9:
			cdchange_noreset_sub(9); break;

		//Kitao追加。F1キーをリセットせずにCD-ROMを入れ替える設定にする。v1.00
		case APPEVENT_F1_NORESET:
			if (_bF1NoReset)
			{
				_bF1NoReset = FALSE;
				PRINTF("Set F1 key for \"CD Change(Reset)\".");
			}
			else
			{
				_bF1NoReset = TRUE;
				PRINTF("Set F1 key for \"CD Change(NoReset)\".");
			}
			updateMenu();
			run_emulator(bRunNormalState);
			break;

		case APPEVENT_RESET:
			if (_bResumeMode == FALSE) //レジュームモードではない場合、リセットしていいかの確認をする
			{
				MAINBOARD_Pause(TRUE);
				WINMAIN_ShowCursor(TRUE);
				screenUpdate(); //ダイアログ表示などで黒くなった部分を再描画。
				if (MessageBox(WINMAIN_GetHwnd(),"Game Reset OK?   ",
								 "Ootake", MB_YESNO) == IDYES)
					bOk = TRUE;
				else
					bOk = FALSE;
			}
			else
				bOk = non_resume_ok(); //レジュームモードの場合、レジュームが解除されることを確認する。
			if (bOk)
				app_reset(MAINBOARD_GetSuperGrafx()); //run_emulator()も行われる
			else
				run_emulator(bRunNormalState);
			break;

		//Kitao追加。開いたイメージファイルの履歴メニュー
		case APPEVENT_RECENT_1:
			if ((_bCDGame)&&(!_bCueFile))
			{
				if (recent_sub(1) == FALSE) return FALSE; //履歴からファイルを開く。失敗した場合は強制終了。
			}
			else //HuカードゲームかCUEファイル起動の場合、「前回開いたファイル＝現在遊んでいるゲーム」。
			{
				//今ノンレジューム状態で、レジュームファイルが存在するなら（レジュームをステートロードなどで解除していた場合）、レジューム復帰する。
				if ((_bResumeMode == FALSE)&&(!MAINBOARD_GetBackupFull())) //※バックアップRAMフルメニューを使用中のときはレジューム復帰しない
				{
					strcpy(fn, _AppPath);
					strcat(fn, "save\\");//saveフォルダ
					strcat(fn, _GameFileNameBuf);
					strcat(fn, "_resume.dat");
					if ((fp = fopen(fn, "r")) != NULL) //レジュームファイルがあれば
					{
						fclose(fp);
						if (recent_sub(1) == FALSE) return FALSE; //ファイルを開き直してレジューム再開する。失敗した場合は強制終了。
					}
				}
				run_emulator(bRunNormalState);
			}
			break;
		case APPEVENT_RECENT_2:
			if (recent_sub(2) == FALSE) return FALSE; break; //履歴からファイルを開く。失敗した場合は強制終了。
		case APPEVENT_RECENT_3:
			if (recent_sub(3) == FALSE) return FALSE; break;
		case APPEVENT_RECENT_4:
			if (recent_sub(4) == FALSE) return FALSE; break;
		case APPEVENT_RECENT_5:
			if (recent_sub(5) == FALSE) return FALSE; break;
		case APPEVENT_RECENT_6:
			if (recent_sub(6) == FALSE) return FALSE; break;
		case APPEVENT_RECENT_7:
			if (recent_sub(7) == FALSE) return FALSE; break;
		case APPEVENT_RECENT_8:
			if (recent_sub(8) == FALSE) return FALSE; break;
		case APPEVENT_RECENT_9:
			if (recent_sub(9) == FALSE) return FALSE; break;
		case APPEVENT_RECENT_10:
			if (recent_sub(10) == FALSE) return FALSE; break;
		case APPEVENT_RECENT_11:
			if (recent_sub(11) == FALSE) return FALSE; break;
		case APPEVENT_RECENT_12:
			if (recent_sub(12) == FALSE) return FALSE; break;
		case APPEVENT_RECENT_13:
			if (recent_sub(13) == FALSE) return FALSE; break;
		case APPEVENT_RECENT_14:
			if (recent_sub(14) == FALSE) return FALSE; break;
		case APPEVENT_RECENT_15:
			if (recent_sub(15) == FALSE) return FALSE; break;
		case APPEVENT_RECENT_16:
			if (recent_sub(16) == FALSE) return FALSE; break;
		case APPEVENT_RECENT_17:
			if (recent_sub(17) == FALSE) return FALSE; break;
		case APPEVENT_RECENT_18:
			if (recent_sub(18) == FALSE) return FALSE; break;
		case APPEVENT_RECENT_19:
			if (recent_sub(19) == FALSE) return FALSE; break;
		case APPEVENT_RECENT_20:
			if (recent_sub(20) == FALSE) return FALSE; break;

		//Kitao追加。レジューム機能。
		case APPEVENT_RESUME:
			MAINBOARD_Pause(TRUE);
			WINMAIN_ShowCursor(TRUE);
			toggleResume();
			run_emulator(bRunNormalState);
			break;

		//Kitao追加。v1.54
		case APPEVENT_AUTO_RESUME:
			if (_bAutoResumeMode)
			{
				MENU_CheckItem(_FileMenu, WM_AUTO_RESUME, FALSE);
				_bAutoResumeMode = FALSE;
				PRINTF("Non-Auto Resume Check Mode.");
			}
			else
			{
				MENU_CheckItem(_FileMenu, WM_AUTO_RESUME, TRUE);
				_bAutoResumeMode = TRUE;
				PRINTF("Set Auto Resume Check Mode.");
			}
			CONFIG_Set("[APP SAVE] AutoResume", &_bAutoResumeMode, sizeof(_bAutoResumeMode));
			if (!_bResumeMode)
				toggleResume();
			run_emulator(bRunNormalState);
			break;

		case APPEVENT_SAVESTATE:
			MAINBOARD_PauseNoMessage(TRUE); //Kitao追加。再開時の音の乱れを防ぐため、音を停止しておく。
			save_state(0);
			run_emulator(bRunNormalState);
			break;
		//Kitao追加。番号付きセーブ
		case APPEVENT_SAVESTATE_1:
		case APPEVENT_SAVESTATE_2:
		case APPEVENT_SAVESTATE_3:
		case APPEVENT_SAVESTATE_4:
		case APPEVENT_SAVESTATE_5:
		case APPEVENT_SAVESTATE_6:
		case APPEVENT_SAVESTATE_7:
		case APPEVENT_SAVESTATE_8:
		case APPEVENT_SAVESTATE_9:
		case APPEVENT_SAVESTATE_10:
		case APPEVENT_SAVESTATE_11:
		case APPEVENT_SAVESTATE_12:
		case APPEVENT_SAVESTATE_13:
		case APPEVENT_SAVESTATE_14:
		case APPEVENT_SAVESTATE_15:
		case APPEVENT_SAVESTATE_16:
		case APPEVENT_SAVESTATE_17:
		case APPEVENT_SAVESTATE_18:
		case APPEVENT_SAVESTATE_19:
		case APPEVENT_SAVESTATE_20:
		case APPEVENT_SAVESTATE_21:
		case APPEVENT_SAVESTATE_22:
		case APPEVENT_SAVESTATE_23:
		case APPEVENT_SAVESTATE_24:
		case APPEVENT_SAVESTATE_25:
		case APPEVENT_SAVESTATE_26:
		case APPEVENT_SAVESTATE_27:
		case APPEVENT_SAVESTATE_28:
		case APPEVENT_SAVESTATE_29:
		case APPEVENT_SAVESTATE_30:
		case APPEVENT_SAVESTATE_31:
		case APPEVENT_SAVESTATE_32:
		case APPEVENT_SAVESTATE_33:
		case APPEVENT_SAVESTATE_34:
		case APPEVENT_SAVESTATE_35:
		case APPEVENT_SAVESTATE_36:
		case APPEVENT_SAVESTATE_37:
		case APPEVENT_SAVESTATE_38:
		case APPEVENT_SAVESTATE_39:
		case APPEVENT_SAVESTATE_40:
		case APPEVENT_SAVESTATE_41:
		case APPEVENT_SAVESTATE_42:
		case APPEVENT_SAVESTATE_43:
		case APPEVENT_SAVESTATE_44:
		case APPEVENT_SAVESTATE_45:
		case APPEVENT_SAVESTATE_46:
		case APPEVENT_SAVESTATE_47:
		case APPEVENT_SAVESTATE_48:
		case APPEVENT_SAVESTATE_49:
		case APPEVENT_SAVESTATE_50:
			MAINBOARD_Pause(TRUE);
			if (save_state(hdEvent - APPEVENT_SAVESTATE_1 + 1)) run_emulator(bRunNormalState);
			break;

		case APPEVENT_LOADSTATE:
			MAINBOARD_PauseNoMessage(TRUE); //Kitao追加。再開時の音の乱れを防ぐため、音を停止しておく。
			load_state(0);
			run_emulator(bRunNormalState); //すぐにエミュレートを再開
			break;
		case APPEVENT_LOADSTATE_P: //Kitao追加。ロード後、ポーズしたままにするメニュー。デフォルトのステートセーブを、番号付きステートセーブにコピーしたいときに使える。
			MAINBOARD_Pause(TRUE);
			load_state(0);
			MAINBOARD_Pause(TRUE); //ステートロード時に見出しがリセットされるので、"Pause"表示のために必要。
			break;
		//Kitao追加。番号付きセーブ
		case APPEVENT_LOADSTATE_1:
		case APPEVENT_LOADSTATE_2:
		case APPEVENT_LOADSTATE_3:
		case APPEVENT_LOADSTATE_4:
		case APPEVENT_LOADSTATE_5:
		case APPEVENT_LOADSTATE_6:
		case APPEVENT_LOADSTATE_7:
		case APPEVENT_LOADSTATE_8:
		case APPEVENT_LOADSTATE_9:
		case APPEVENT_LOADSTATE_10:
		case APPEVENT_LOADSTATE_11:
		case APPEVENT_LOADSTATE_12:
		case APPEVENT_LOADSTATE_13:
		case APPEVENT_LOADSTATE_14:
		case APPEVENT_LOADSTATE_15:
		case APPEVENT_LOADSTATE_16:
		case APPEVENT_LOADSTATE_17:
		case APPEVENT_LOADSTATE_18:
		case APPEVENT_LOADSTATE_19:
		case APPEVENT_LOADSTATE_20:
		case APPEVENT_LOADSTATE_21:
		case APPEVENT_LOADSTATE_22:
		case APPEVENT_LOADSTATE_23:
		case APPEVENT_LOADSTATE_24:
		case APPEVENT_LOADSTATE_25:
		case APPEVENT_LOADSTATE_26:
		case APPEVENT_LOADSTATE_27:
		case APPEVENT_LOADSTATE_28:
		case APPEVENT_LOADSTATE_29:
		case APPEVENT_LOADSTATE_30:
		case APPEVENT_LOADSTATE_31:
		case APPEVENT_LOADSTATE_32:
		case APPEVENT_LOADSTATE_33:
		case APPEVENT_LOADSTATE_34:
		case APPEVENT_LOADSTATE_35:
		case APPEVENT_LOADSTATE_36:
		case APPEVENT_LOADSTATE_37:
		case APPEVENT_LOADSTATE_38:
		case APPEVENT_LOADSTATE_39:
		case APPEVENT_LOADSTATE_40:
		case APPEVENT_LOADSTATE_41:
		case APPEVENT_LOADSTATE_42:
		case APPEVENT_LOADSTATE_43:
		case APPEVENT_LOADSTATE_44:
		case APPEVENT_LOADSTATE_45:
		case APPEVENT_LOADSTATE_46:
		case APPEVENT_LOADSTATE_47:
		case APPEVENT_LOADSTATE_48:
		case APPEVENT_LOADSTATE_49:
		case APPEVENT_LOADSTATE_50:
			MAINBOARD_Pause(TRUE);
			load_state(hdEvent - APPEVENT_LOADSTATE_1 + 1);
			MAINBOARD_Pause(TRUE); //ステートロード時に見出しがリセットされるので、"Pause"表示のために必要。
			break;

		case APPEVENT_SAVE_BUTTON: //Kitao追加。ステートセーブ用のボタンを設定
			set_shortcut_key_button(3);
			updateMenu();//メニュー表示を更新(ショートカットキーの表示)
			break;
		case APPEVENT_LOAD_BUTTON: //Kitao追加。ステートロード用のボタンを設定
			set_shortcut_key_button(4);
			updateMenu();//メニュー表示を更新(ショートカットキーの表示)
			break;
		case APPEVENT_SAVE_DEFAULT: //Kitao追加。ステートセーブ＆ロード用のボタンをデフォルトに戻す。
			MAINBOARD_Pause(TRUE);
			WINMAIN_ShowCursor(TRUE);
			screenUpdate(); //メニューやダイアログ表示などで黒くなった部分を再描画。
			if (MessageBox(	WINMAIN_GetHwnd(),
							"Do you return \"Save&LoadState Button\" to the default([S]&[L]key)?    ",
							"Ootake", MB_YESNO) == IDYES)
			{
				_SaveStateButton = DIK_S;
				CONFIG_Set("[APP DEVICE] SaveState Button", &_SaveStateButton, sizeof(_SaveStateButton));
				_LoadStateButton = DIK_L;
				CONFIG_Set("[APP DEVICE] LoadState Button", &_LoadStateButton, sizeof(_LoadStateButton));
				updateMenu();//メニュー表示を更新(ショートカットキーの表示)
				PRINTF("Returned Save&LoadState Button.");
				run_emulator(bRunNormalState);
			}
			break;

		case APPEVENT_FOLDER_STATE: //Kitao追加。saveフォルダをマイコンピュータで開く
			{
				updateMenu();//メニュー表示を更新(番号付きセーブのファイルの有無を更新しておく)
				APP_OpenSaveFolder();
			}
			break;

		case APPEVENT_FOLDER_BRAM: //Kitao追加。bramフォルダをマイコンピュータで開く
			{
				save_bram(); //bramファイルを最新の状態に更新する。※bramフォルダがない場合作られる。
				strcpy(fn, _AppPath);
				strcat(fn, "bram");
				if (_VirtualStorePath[0] != 0) //v0.95追加。v2.20更新。Vista/7で"Program Files"にインストールした場合、セーブファイルがリダイレクトされているのでリダイレクト先のフォルダを開くようにする。
				{
					strcpy(fn, _VirtualStorePath);
					strcat(fn, "bram");
				}
				ShellExecute(NULL, "open", fn, NULL, NULL, SW_SHOW);
			}
			break;

		case APPEVENT_FOLDER_MB128: //Kitao追加。mb128フォルダをマイコンピュータで開く
			{
				save_bram(); //bramとmb128ファイルを最新の状態に更新する。※mb128フォルダがない場合作られる。
				strcpy(fn, _AppPath);
				strcat(fn, "mb128");
				if (_VirtualStorePath[0] != 0) //v0.95追加。v2.20更新。Vista/7で"Program Files"にインストールした場合、セーブファイルがリダイレクトされているのでリダイレクト先のフォルダを開くようにする。
				{
					strcpy(fn, _VirtualStorePath);
					strcat(fn, "mb128");
				}
				ShellExecute(NULL, "open", fn, NULL, NULL, SW_SHOW);
			}
			break;

		case APPEVENT_PARTIALMEMORYLOAD:
			load_memory_values(_AppPath, _GameFilePathName);
			break;

		case APPEVENT_RECORDING:
			MAINBOARD_Pause(TRUE); //Kitao追加。再開時の音の乱れを防ぐため、音を停止しておく。＆CD-ROMのアクセスが終了するまで待つ。
			recording_gameplay(0);
			run_emulator(TRUE);
			break;
		//Kitao追加。番号付き保存。v2.15
		case APPEVENT_RECORDING_1:  MAINBOARD_Pause(TRUE); recording_gameplay( 1); run_emulator(TRUE); break;
		case APPEVENT_RECORDING_2:  MAINBOARD_Pause(TRUE); recording_gameplay( 2); run_emulator(TRUE); break;
		case APPEVENT_RECORDING_3:  MAINBOARD_Pause(TRUE); recording_gameplay( 3); run_emulator(TRUE); break;
		case APPEVENT_RECORDING_4:  MAINBOARD_Pause(TRUE); recording_gameplay( 4); run_emulator(TRUE); break;
		case APPEVENT_RECORDING_5:  MAINBOARD_Pause(TRUE); recording_gameplay( 5); run_emulator(TRUE); break;
		case APPEVENT_RECORDING_6:  MAINBOARD_Pause(TRUE); recording_gameplay( 6); run_emulator(TRUE); break;
		case APPEVENT_RECORDING_7:  MAINBOARD_Pause(TRUE); recording_gameplay( 7); run_emulator(TRUE); break;
		case APPEVENT_RECORDING_8:  MAINBOARD_Pause(TRUE); recording_gameplay( 8); run_emulator(TRUE); break;
		case APPEVENT_RECORDING_9:  MAINBOARD_Pause(TRUE); recording_gameplay( 9); run_emulator(TRUE); break;
		case APPEVENT_RECORDING_10: MAINBOARD_Pause(TRUE); recording_gameplay(10); run_emulator(TRUE); break;
		//CD-ROMゲームで利用するときの注意を表示する。v2.15追加
		case APPEVENT_RECORDING_HELP:
			screenUpdate(); //メニューやダイアログ表示などで黒くなった部分を再描画。
			MessageBox(	WINMAIN_GetHwnd(),
						"In CD-ROM Game, when there is an access to CD-ROM while   \n"
						"recording, the gap is caused at playing the record. Please use it   \n"
						"within the range where the CD access doesn't happen. I want to   \n"
						"improve it in the future.\n\n"
						"In Japanese language\n"
						"CD-ROMゲームで、レコーディング中にCD-ROMへのアクセスがあった   \n"
						"場合、再生時にズレが生じます。現状は、CD-ROMゲームでは、CD   \n"
						"アクセスが起こらない範囲で利用してください。将来的に改善できれば   \n"
						"と思っています。",
						"About Recoding", MB_OK);
			break;

		case APPEVENT_PLAYRECORD:
			MAINBOARD_Pause(TRUE); //Kitao追加。再開時の音の乱れを防ぐため、音を停止しておく。
			playrecord_gameplay(0);
			run_emulator(TRUE);
			break;
		//Kitao追加。番号付き再生。v2.15
		case APPEVENT_PLAYRECORD_1:  MAINBOARD_Pause(TRUE); playrecord_gameplay( 1); run_emulator(TRUE); break;
		case APPEVENT_PLAYRECORD_2:  MAINBOARD_Pause(TRUE); playrecord_gameplay( 2); run_emulator(TRUE); break;
		case APPEVENT_PLAYRECORD_3:  MAINBOARD_Pause(TRUE); playrecord_gameplay( 3); run_emulator(TRUE); break;
		case APPEVENT_PLAYRECORD_4:  MAINBOARD_Pause(TRUE); playrecord_gameplay( 4); run_emulator(TRUE); break;
		case APPEVENT_PLAYRECORD_5:  MAINBOARD_Pause(TRUE); playrecord_gameplay( 5); run_emulator(TRUE); break;
		case APPEVENT_PLAYRECORD_6:  MAINBOARD_Pause(TRUE); playrecord_gameplay( 6); run_emulator(TRUE); break;
		case APPEVENT_PLAYRECORD_7:  MAINBOARD_Pause(TRUE); playrecord_gameplay( 7); run_emulator(TRUE); break;
		case APPEVENT_PLAYRECORD_8:  MAINBOARD_Pause(TRUE); playrecord_gameplay( 8); run_emulator(TRUE); break;
		case APPEVENT_PLAYRECORD_9:  MAINBOARD_Pause(TRUE); playrecord_gameplay( 9); run_emulator(TRUE); break;
		case APPEVENT_PLAYRECORD_10: MAINBOARD_Pause(TRUE); playrecord_gameplay(10); run_emulator(TRUE); break;

		//Kitao追加。プレイレコードを番号付きへムーブ。v2.15
		case APPEVENT_MOVERECORD_1:  MAINBOARD_Pause(TRUE); moverecord_gameplay( 1); break;
		case APPEVENT_MOVERECORD_2:  MAINBOARD_Pause(TRUE); moverecord_gameplay( 2); break;
		case APPEVENT_MOVERECORD_3:  MAINBOARD_Pause(TRUE); moverecord_gameplay( 3); break;
		case APPEVENT_MOVERECORD_4:  MAINBOARD_Pause(TRUE); moverecord_gameplay( 4); break;
		case APPEVENT_MOVERECORD_5:  MAINBOARD_Pause(TRUE); moverecord_gameplay( 5); break;
		case APPEVENT_MOVERECORD_6:  MAINBOARD_Pause(TRUE); moverecord_gameplay( 6); break;
		case APPEVENT_MOVERECORD_7:  MAINBOARD_Pause(TRUE); moverecord_gameplay( 7); break;
		case APPEVENT_MOVERECORD_8:  MAINBOARD_Pause(TRUE); moverecord_gameplay( 8); break;
		case APPEVENT_MOVERECORD_9:  MAINBOARD_Pause(TRUE); moverecord_gameplay( 9); break;
		case APPEVENT_MOVERECORD_10: MAINBOARD_Pause(TRUE); moverecord_gameplay(10); break;

		case APPEVENT_FOLDER_GAMEPLAY: //Kitao追加。recordフォルダをマイコンピュータで開く
			{
				strcpy(fn, _AppPath);
				strcat(fn, "record");
				CreateDirectory(fn, NULL);//recordディレクトリがない場合作る
				if (_VirtualStorePath[0] != 0) //v0.95追加。v2.20更新。Vista/7で"Program Files"にインストールした場合、セーブファイルがリダイレクトされているのでリダイレクト先のフォルダを開くようにする。
				{
					strcpy(fn, _VirtualStorePath);
					strcat(fn, "record");
				}
				ShellExecute(NULL, "open", fn, NULL, NULL, SW_SHOW);
			}
			break;

		case APPEVENT_SCREEN_FULLSCREEN: //Kitao更新
			APP_ToggleFullscreen();
			Sleep(1000); //他のアプリのウィンドウ復元などでCPU処理が取られて音が乱れるので１秒待ってから再開させる。
			run_emulator(bRunNormalState);
			break;
		case APPEVENT_SCREEN_FULLSCREEN640: //Kitao追加。お使いのPC環境で表示できない解像度を選んでしまった場合のためのメニュー
			_FullMagnification = 2; //どの環境でも必ず表示可能な640x480に設定する。
			_bResolutionAutoChange = FALSE; //オートチェンジを解除
			CONFIG_Set("[APP DISPLAY] Resolution AutoChange", &_bResolutionAutoChange, sizeof(_bResolutionAutoChange));
			if (_bFullScreen)
				change_screen_mode(2, _bFullScreen, _FullScreenColor, FALSE); //_FullMagnificationの設定もiniに保存される。メニューも更新される。
			else
			{
				APP_ToggleFullscreen(); //_FullMagnificationの設定もiniに保存される。メニューも更新される。
				Sleep(1000); //他のアプリのウィンドウ復元などでCPU処理が取られて音が乱れるので１秒待ってから再開させる。
			}
			run_emulator(bRunNormalState);
			break;
		case APPEVENT_SCREEN_FULLSCREENCS1: //Kitao追加。任意の解像度を選択。
			if (_bFullScreen)
			{
				APP_ToggleFullscreen();
				GetWindowRect(WINMAIN_GetHwnd(), &rc);
				SetWindowPos(WINMAIN_GetHwnd(), HWND_NOTOPMOST, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_FRAMECHANGED);
			}
			resolutionWidth = 0;
			resolutionHeight = 0;
			RESOLUTION_Init(WINMAIN_GetHInstance(), 1, &resolutionWidth, &resolutionHeight); //Resolutionウィンドウを表示
			//メッセージループ
			while (resolutionHeight == 0) //ダイアログが結果を返すまでループ
			{ 
				GetMessage(&msg ,NULL, 0, 0);
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			//Okボタンが押されていた場合、解像度設定を変更する。
			if (resolutionHeight == 1) //デフォルトに戻す場合
			{
				_CustomWidth1 = 640;
				_CustomHeight1 = 480;
			}
			else if (resolutionHeight > 0)
			{
				_CustomWidth1 = resolutionWidth;
				_CustomHeight1 = resolutionHeight;
				_FullMagnification = 0;
				CONFIG_Set("[APP DISPLAY] FullMagnification", &_FullMagnification, sizeof(_FullMagnification));
				APP_ToggleFullscreen();
				Sleep(1000); //他のアプリのウィンドウ復元などでCPU処理が取られて音が乱れるので１秒待ってから再開させる。
			}
			updateMenu(); //メニュー表示を更新
			CONFIG_Set("[APP DISPLAY] Full CustomWidth1", &_CustomWidth1, sizeof(_CustomWidth1));
			CONFIG_Set("[APP DISPLAY] Full CustomHeight1", &_CustomHeight1, sizeof(_CustomHeight1));
			if (_bFullScreen)
				run_emulator(bRunNormalState);
			break;
		case APPEVENT_SCREEN_FULLSCREENCS2: //Kitao追加。任意の解像度を選択。
			if (_bFullScreen)
			{
				APP_ToggleFullscreen();
				GetWindowRect(WINMAIN_GetHwnd(), &rc);
				SetWindowPos(WINMAIN_GetHwnd(), HWND_NOTOPMOST, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_FRAMECHANGED);
			}
			resolutionWidth = 0;
			resolutionHeight = 0;
			RESOLUTION_Init(WINMAIN_GetHInstance(), 2, &resolutionWidth, &resolutionHeight); //Resolutionウィンドウを表示
			//メッセージループ
			while (resolutionHeight == 0) //ダイアログが結果を返すまでループ
			{ 
				GetMessage(&msg ,NULL, 0, 0);
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			//Okボタンが押されていた場合、解像度設定を変更する。
			if (resolutionHeight == 1) //デフォルトに戻す場合
			{
				_CustomWidth2 = 320;
				_CustomHeight2 = 240;
			}
			else if (resolutionHeight > 0)
			{
				_CustomWidth2 = resolutionWidth;
				_CustomHeight2 = resolutionHeight;
				_FullMagnification = 1;
				CONFIG_Set("[APP DISPLAY] FullMagnification", &_FullMagnification, sizeof(_FullMagnification));
				APP_ToggleFullscreen();
				Sleep(1000); //他のアプリのウィンドウ復元などでCPU処理が取られて音が乱れるので１秒待ってから再開させる。
			}
			updateMenu(); //メニュー表示を更新
			CONFIG_Set("[APP DISPLAY] Full CustomWidth2", &_CustomWidth2, sizeof(_CustomWidth2));
			CONFIG_Set("[APP DISPLAY] Full CustomHeight2", &_CustomHeight2, sizeof(_CustomHeight2));
			if (_bFullScreen)
				run_emulator(bRunNormalState);
			break;
		case APPEVENT_SCREEN_FULLSCREENCS3: //Kitao追加。任意の解像度を選択。
			if (_bFullScreen)
			{
				APP_ToggleFullscreen();
				GetWindowRect(WINMAIN_GetHwnd(), &rc);
				SetWindowPos(WINMAIN_GetHwnd(), HWND_NOTOPMOST, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_FRAMECHANGED);
			}
			resolutionWidth = 0;
			resolutionHeight = 0;
			RESOLUTION_Init(WINMAIN_GetHInstance(), 3, &resolutionWidth, &resolutionHeight); //Resolutionウィンドウを表示
			//メッセージループ
			while (resolutionHeight == 0) //ダイアログが結果を返すまでループ
			{ 
				GetMessage(&msg ,NULL, 0, 0);
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			//Okボタンが押されていた場合、解像度設定を変更する。
			if (resolutionHeight == 1) //デフォルトに戻す場合
			{
				_CustomWidth3 = 400;
				_CustomHeight3 = 300;
			}
			else if (resolutionHeight > 0)
			{
				_CustomWidth3 = resolutionWidth;
				_CustomHeight3 = resolutionHeight;
				_FullMagnification = 5;
				CONFIG_Set("[APP DISPLAY] FullMagnification", &_FullMagnification, sizeof(_FullMagnification));
				APP_ToggleFullscreen();
				Sleep(1000); //他のアプリのウィンドウ復元などでCPU処理が取られて音が乱れるので１秒待ってから再開させる。
			}
			updateMenu(); //メニュー表示を更新
			CONFIG_Set("[APP DISPLAY] Full CustomWidth3", &_CustomWidth3, sizeof(_CustomWidth3));
			CONFIG_Set("[APP DISPLAY] Full CustomHeight3", &_CustomHeight3, sizeof(_CustomHeight3));
			if (_bFullScreen)
				run_emulator(bRunNormalState);
			break;
		//Kitao追加。ゲームの解像度に合わせてディスプレイの解像度を変更できる設定を追加。
		//			 15kHzモニターなど多彩な解像度(縦横比)を設定可能なディスプレイ(＆ビデオカード)向け。v1.35
		case APPEVENT_SCREEN_FULLSCREENCSA: 
			if (_bResolutionAutoChange)
			{
				_bResolutionAutoChange = FALSE;
				PRINTF("Reset Resolution Auto Change. ( for FullScreen )");
			}
			else
			{
				_bResolutionAutoChange = TRUE;
				MessageBox(	WINMAIN_GetHwnd(),
							"Resolution Auto Change mode Ok.\n\n"
							"FullScreen Resolution <1> = for 256x224 game   \n"
							"FullScreen Resolution <2> = for 336x224 game   \n"
							"FullScreen Resolution <3> = for 512x224 game   ",
							"Ootake", MB_OK);
				if (_bFullScreen)
					updateScreenMode(FALSE);
				PRINTF("Set Resolution Auto Change. ( for FullScreen )");
			}
			MENU_CheckItem(_ResolutionMenu, WM_SCREEN_FULLSCREENCSA, _bResolutionAutoChange);
			CONFIG_Set("[APP DISPLAY] Resolution AutoChange", &_bResolutionAutoChange, sizeof(_bResolutionAutoChange));
			run_emulator(bRunNormalState);
			break;

		case APPEVENT_SCREEN_CS:	change_screen_mode(0, _bFullScreen, _FullScreenColor, FALSE); //Kitao追加。カスタマイズ解像度１
									run_emulator(bRunNormalState); break;
		case APPEVENT_SCREEN_X1:	change_screen_mode(1, _bFullScreen, _FullScreenColor, FALSE); //Kitao更新。カスタマイズ解像度２としても使われる。
									run_emulator(bRunNormalState); break;
		case APPEVENT_SCREEN_X2:	change_screen_mode(2, _bFullScreen, _FullScreenColor, FALSE);
									run_emulator(bRunNormalState); break;
		case APPEVENT_SCREEN_X3:	change_screen_mode(3, _bFullScreen, _FullScreenColor, FALSE);
									run_emulator(bRunNormalState); break;
		case APPEVENT_SCREEN_X4:	change_screen_mode(4, _bFullScreen, _FullScreenColor, FALSE);
									run_emulator(bRunNormalState); break;
		//Kitao追加。フルスクリーン特殊解像度用
		case APPEVENT_SCREEN_F1:	change_screen_mode(5, _bFullScreen, _FullScreenColor, FALSE); //Kitao更新。カスタマイズ解像度３としても使われる。
									run_emulator(bRunNormalState); break;
		case APPEVENT_SCREEN_F2:	change_screen_mode(6, _bFullScreen, _FullScreenColor, FALSE);
									run_emulator(bRunNormalState); break;

		//Kitao追加。"Windows Aero"(Win7/Vista)の無効・有効を切り替える。v2.21
		case APPEVENT_SCREEN_DISABLEAERO:
			if (_bFullScreen)
				PRINTF("Always Disable Aero at FullScreen."); //Ctrl+F12キーで切り替えたときのメッセージ
			else
			{
				PRINTF("Disable Aero ( No Time-Lag ).");
				MENU_CheckRadioItem(_SizeMenu, WM_SCREEN_DISABLEAERO, WM_SCREEN_USEAERO, WM_SCREEN_DISABLEAERO);
				_bDisableWindowsAero = TRUE;
				CONFIG_Set("[APP DISPLAY] Disable Windows Aero", &_bDisableWindowsAero, sizeof(_bDisableWindowsAero));
				setWindowsAero(FALSE); //Aeroを無効に。
			}
			run_emulator(bRunNormalState);
			break;
		case APPEVENT_SCREEN_USEAERO:
			if (_bFullScreen)
				PRINTF("Always Disable Aero at FullScreen."); //Ctrl+F12キーで切り替えたときのメッセージ
			else
			{
				PRINTF("Use Aero.");
				MENU_CheckRadioItem(_SizeMenu, WM_SCREEN_DISABLEAERO, WM_SCREEN_USEAERO, WM_SCREEN_USEAERO);
				_bDisableWindowsAero = FALSE;
				CONFIG_Set("[APP DISPLAY] Disable Windows Aero", &_bDisableWindowsAero, sizeof(_bDisableWindowsAero));
				setWindowsAero(TRUE); //Aeroを有効に。
			}
			run_emulator(bRunNormalState);
			break;

		//Kitao追加。スクリーンのガンマ値を変更する。
		case APPEVENT_SCREEN_GAMMA1:	_GammaValue = 0.940; updateMenu(); CONFIG_Set("[APP DISPLAY] Gamma Value", &_GammaValue, sizeof(_GammaValue));
										SCREEN_SetGamma(APP_GetScanLineType(), _ScanLineDensity, _bTvMode); run_emulator(bRunNormalState); break;
		case APPEVENT_SCREEN_GAMMA2:	_GammaValue = 1.040; updateMenu(); CONFIG_Set("[APP DISPLAY] Gamma Value", &_GammaValue, sizeof(_GammaValue));
										SCREEN_SetGamma(APP_GetScanLineType(), _ScanLineDensity, _bTvMode); run_emulator(bRunNormalState); break;
		case APPEVENT_SCREEN_GAMMA3:	_GammaValue = 1.140; updateMenu(); CONFIG_Set("[APP DISPLAY] Gamma Value", &_GammaValue, sizeof(_GammaValue));
										SCREEN_SetGamma(APP_GetScanLineType(), _ScanLineDensity, _bTvMode); run_emulator(bRunNormalState); break;
		case APPEVENT_SCREEN_GAMMA4:	_GammaValue = 1.240; updateMenu(); CONFIG_Set("[APP DISPLAY] Gamma Value", &_GammaValue, sizeof(_GammaValue));
										SCREEN_SetGamma(APP_GetScanLineType(), _ScanLineDensity, _bTvMode); run_emulator(bRunNormalState); break;
		case APPEVENT_SCREEN_GAMMA5:	_GammaValue = 1.340; updateMenu(); CONFIG_Set("[APP DISPLAY] Gamma Value", &_GammaValue, sizeof(_GammaValue));
										SCREEN_SetGamma(APP_GetScanLineType(), _ScanLineDensity, _bTvMode); run_emulator(bRunNormalState); break;
		case APPEVENT_SCREEN_GAMMA6:	_GammaValue = 1.390; updateMenu(); CONFIG_Set("[APP DISPLAY] Gamma Value", &_GammaValue, sizeof(_GammaValue));
										SCREEN_SetGamma(APP_GetScanLineType(), _ScanLineDensity, _bTvMode); run_emulator(bRunNormalState); break;
		case APPEVENT_SCREEN_GAMMA7:	_GammaValue = 1.440; updateMenu(); CONFIG_Set("[APP DISPLAY] Gamma Value", &_GammaValue, sizeof(_GammaValue));
										SCREEN_SetGamma(APP_GetScanLineType(), _ScanLineDensity, _bTvMode); run_emulator(bRunNormalState); break;

		//Kitao追加。スクリーンの明るさを微調整する。
		case APPEVENT_SCREEN_BRIGHT1:	_BrightValue = 0; updateMenu(); CONFIG_Set("[APP DISPLAY] Brightness Value", &_BrightValue, sizeof(_BrightValue));
										SCREEN_SetGamma(APP_GetScanLineType(), _ScanLineDensity, _bTvMode); run_emulator(bRunNormalState); break;
		case APPEVENT_SCREEN_BRIGHT2:	_BrightValue = 1; updateMenu(); CONFIG_Set("[APP DISPLAY] Brightness Value", &_BrightValue, sizeof(_BrightValue));
										SCREEN_SetGamma(APP_GetScanLineType(), _ScanLineDensity, _bTvMode); run_emulator(bRunNormalState); break;
		case APPEVENT_SCREEN_BRIGHT3:	_BrightValue = 2; updateMenu(); CONFIG_Set("[APP DISPLAY] Brightness Value", &_BrightValue, sizeof(_BrightValue));
										SCREEN_SetGamma(APP_GetScanLineType(), _ScanLineDensity, _bTvMode); run_emulator(bRunNormalState); break;
		case APPEVENT_SCREEN_BRIGHT4:	_BrightValue = 3; updateMenu(); CONFIG_Set("[APP DISPLAY] Brightness Value", &_BrightValue, sizeof(_BrightValue));
										SCREEN_SetGamma(APP_GetScanLineType(), _ScanLineDensity, _bTvMode); run_emulator(bRunNormalState); break;
		case APPEVENT_SCREEN_BRIGHT5:	_BrightValue = 4; updateMenu(); CONFIG_Set("[APP DISPLAY] Brightness Value", &_BrightValue, sizeof(_BrightValue));
										SCREEN_SetGamma(APP_GetScanLineType(), _ScanLineDensity, _bTvMode); run_emulator(bRunNormalState); break;
		case APPEVENT_SCREEN_BRIGHT6:	_BrightValue = 6; updateMenu(); CONFIG_Set("[APP DISPLAY] Brightness Value", &_BrightValue, sizeof(_BrightValue));
										SCREEN_SetGamma(APP_GetScanLineType(), _ScanLineDensity, _bTvMode); run_emulator(bRunNormalState); break;
		case APPEVENT_SCREEN_BRIGHT7:	_BrightValue = 11; updateMenu(); CONFIG_Set("[APP DISPLAY] Brightness Value", &_BrightValue, sizeof(_BrightValue));
										SCREEN_SetGamma(APP_GetScanLineType(), _ScanLineDensity, _bTvMode); run_emulator(bRunNormalState); break;
		case APPEVENT_SCREEN_BRIGHT8:	_BrightValue = 21; updateMenu(); CONFIG_Set("[APP DISPLAY] Brightness Value", &_BrightValue, sizeof(_BrightValue));
										SCREEN_SetGamma(APP_GetScanLineType(), _ScanLineDensity, _bTvMode); run_emulator(bRunNormalState); break;
		case APPEVENT_SCREEN_BRIGHT9:	_BrightValue = 31; updateMenu(); CONFIG_Set("[APP DISPLAY] Brightness Value", &_BrightValue, sizeof(_BrightValue));
										SCREEN_SetGamma(APP_GetScanLineType(), _ScanLineDensity, _bTvMode); run_emulator(bRunNormalState); break;

		//Kitao追加。最小化してポーズする。(ショートカットは"F9"キー)
		case APPEVENT_SCREEN_MINIMIZE:
			if (_bFullScreen) //フルスクリーンの時はウィンドウに表示に切り替える
				APP_ToggleFullscreen();
			run_emulator(TRUE); //メニューを出していた場合消す＆復帰時にすぐスタートするためにrunしておく
			MAINBOARD_Pause(TRUE);
			WINMAIN_ShowCursor(TRUE);
 			CloseWindow(WINMAIN_GetHwnd()); //最小化
			break;

		case APPEVENT_INPUT_TURBO_1: //Kitao追加
			ToggleInputTurbo1();
			run_emulator(bRunNormalState);
			break;
		case APPEVENT_INPUT_TURBO_2: //Kitao追加
			ToggleInputTurbo2();
			run_emulator(bRunNormalState);
			break;
		case APPEVENT_INPUT_TURBO_RUN: //Kitao追加
			ToggleInputTurboRun();
			run_emulator(bRunNormalState);
			break;

		case APPEVENT_INPUT_TURBO_HIGH: //Kitao追加
			MENU_CheckRadioItem(_InputMenu, WM_INPUT_TURBO_HIGH, WM_INPUT_TURBO_OFF, WM_INPUT_TURBO_HIGH);
			set_rensha_speed(1);
			run_emulator(bRunNormalState);
			break;
		case APPEVENT_INPUT_TURBO_MIDDLE: //Kitao追加
			MENU_CheckRadioItem(_InputMenu, WM_INPUT_TURBO_HIGH, WM_INPUT_TURBO_OFF, WM_INPUT_TURBO_MIDDLE);
			set_rensha_speed(2);
			run_emulator(bRunNormalState);
			break;
		case APPEVENT_INPUT_TURBO_LOW: //Kitao追加
			MENU_CheckRadioItem(_InputMenu, WM_INPUT_TURBO_HIGH, WM_INPUT_TURBO_OFF, WM_INPUT_TURBO_LOW);
			set_rensha_speed(3);
			run_emulator(bRunNormalState);
			break;
		case APPEVENT_INPUT_TURBO_OFF: //Kitao追加
			MENU_CheckRadioItem(_InputMenu, WM_INPUT_TURBO_HIGH, WM_INPUT_TURBO_OFF, WM_INPUT_TURBO_OFF);
			set_rensha_speed(0);
			run_emulator(bRunNormalState);
			break;

		//Kitao更新。６ボタンパッド
		case APPEVENT_INPUT_SIX_BUTTON_PAD:
			MENU_CheckItem(_InputMenu, WM_INPUT_SWAP_SELRUN, FALSE);
			MENU_CheckItem(_InputMenu, WM_INPUT_SWAP_IANDII, FALSE);
			MENU_CheckRadioItem(_InputMenu, WM_INPUT_TWO_BUTTON_PAD, WM_INPUT_MOUSE, WM_INPUT_SIX_BUTTON_PAD);
			JOYPAD_SetSwapSelRun(FALSE);
			JOYPAD_SetSwapIandII(FALSE);
			JOYPAD_ConnectMouse(FALSE);
			JOYPAD_UseSixButton(TRUE);
			APP_SetInputConfiguration(); //Kitao追加。ボタン設定を反映させる
			PRINTF("Connected 6-button pad.");
			run_emulator(bRunNormalState);
			break;

		//Kitao追加。３ボタンパッド
		case APPEVENT_INPUT_THR_BUTTON_PAD:
			MENU_CheckItem(_InputMenu, WM_INPUT_SWAP_SELRUN, FALSE);
			MENU_CheckItem(_InputMenu, WM_INPUT_SWAP_IANDII, FALSE);
			MENU_CheckRadioItem(_InputMenu, WM_INPUT_TWO_BUTTON_PAD, WM_INPUT_MOUSE, WM_INPUT_THR_BUTTON_PAD);
			JOYPAD_SetSwapSelRun(FALSE);
			JOYPAD_SetSwapIandII(FALSE);
			JOYPAD_ConnectMouse(FALSE);
			JOYPAD_UseSixButton(FALSE);
			JOYPAD_UseThreeButton(TRUE);
			_bUseThreeButton = TRUE;
			CONFIG_Set("[APP INPUT] JoyPad Use 3Button", &_bUseThreeButton, sizeof(_bUseThreeButton));
			_bUseThreeButtonR = _bUseThreeButton; //手動で変更した設定を退避
			APP_SetInputConfiguration(); //ボタン設定を反映させる
			PRINTF("Connected 3-button pad.");
			run_emulator(bRunNormalState);
			break;

		//Kitao更新。２ボタン(標準)パッド
		case APPEVENT_INPUT_TWO_BUTTON_PAD:
			MENU_CheckItem(_InputMenu, WM_INPUT_SWAP_SELRUN, FALSE);
			MENU_CheckItem(_InputMenu, WM_INPUT_SWAP_IANDII, FALSE);
			MENU_CheckRadioItem(_InputMenu, WM_INPUT_TWO_BUTTON_PAD, WM_INPUT_MOUSE, WM_INPUT_TWO_BUTTON_PAD);
			JOYPAD_SetSwapSelRun(FALSE);
			JOYPAD_SetSwapIandII(FALSE);
			JOYPAD_ConnectMouse(FALSE);
			JOYPAD_UseSixButton(FALSE);
			JOYPAD_UseThreeButton(FALSE);
			_bUseThreeButton = FALSE;
			CONFIG_Set("[APP INPUT] JoyPad Use 3Button", &_bUseThreeButton, sizeof(_bUseThreeButton));
			_bUseThreeButtonR = _bUseThreeButton; //手動で変更した設定を退避
			APP_SetInputConfiguration(); //ボタン設定を反映させる
			PRINTF("Connected 2-button pad.");
			run_emulator(bRunNormalState);
			break;

		case APPEVENT_INPUT_MOUSE:
			MENU_CheckItem(_InputMenu, WM_INPUT_MULTI_TAP, FALSE);
			MENU_CheckRadioItem(_InputMenu, WM_INPUT_TWO_BUTTON_PAD, WM_INPUT_MOUSE, WM_INPUT_MOUSE);
			JOYPAD_ConnectMultiTap(FALSE);
			JOYPAD_ConnectMouse(TRUE);
			PRINTF("Connected Mouse.");
			run_emulator(bRunNormalState);
			break;

		case APPEVENT_INPUT_MULTI_TAP:
			if (JOYPAD_GetConnectMultiTap())
			{
				MENU_CheckItem(_InputMenu, WM_INPUT_MULTI_TAP, FALSE);
				JOYPAD_ConnectMultiTap(FALSE);
				PRINTF("Disconnected Multi-tap.");
			}
			else
			{
				MENU_CheckItem(_InputMenu, WM_INPUT_MULTI_TAP, TRUE);
				JOYPAD_ConnectMultiTap(TRUE);
				PRINTF("Connected Multi-tap.");
			}
			run_emulator(bRunNormalState);
			break;

		case APPEVENT_INPUT_MB128:
			if (MB128_IsConnected())
			{
				MENU_CheckItem(_InputMenu, WM_INPUT_MB128, FALSE);
				JOYPAD_ConnectMB128(FALSE);
				PRINTF("Disconnected MB128.");
			}
			else
			{
				MENU_CheckItem(_InputMenu, WM_INPUT_MB128, TRUE);
				JOYPAD_ConnectMB128(TRUE);
				PRINTF("Connected MB128.");
			}
			run_emulator(bRunNormalState);
			break;

		case APPEVENT_INPUT_SWAP_SELRUN: //Kitao追加
			if (JOYPAD_GetSwapSelRun())
			{
				MENU_CheckItem(_InputMenu, WM_INPUT_SWAP_SELRUN, FALSE);
				JOYPAD_SetSwapSelRun(FALSE);
				PRINTF("Normal Select&Run Buttons.");
			}
			else
			{
				MENU_CheckItem(_InputMenu, WM_INPUT_SWAP_SELRUN, TRUE);
				JOYPAD_SetSwapSelRun(TRUE);
				PRINTF("Swapped Select&Run Buttons.");
			}
			run_emulator(bRunNormalState);
			break;

		case APPEVENT_INPUT_SWAP_IANDII: //Kitao追加
			if (JOYPAD_GetSwapIandII())
			{
				MENU_CheckItem(_InputMenu, WM_INPUT_SWAP_IANDII, FALSE);
				JOYPAD_SetSwapIandII(FALSE);
				PRINTF("Normal I & II Buttons.");
			}
			else
			{
				MENU_CheckItem(_InputMenu, WM_INPUT_SWAP_IANDII, TRUE);
				JOYPAD_SetSwapIandII(TRUE);
				PRINTF("Swapped I & II Buttons.");
			}
			run_emulator(bRunNormalState);
			break;

		case APPEVENT_INPUT_CHECKPAD_LR: //Kitao追加。v2.59
			if (_bCheckSimultaneouslyPush)
			{
				_bCheckSimultaneouslyPush = FALSE;
				PRINTF("Non-Check Pad LR(or UD) Simulataneously Push.");
			}
			else
			{
				_bCheckSimultaneouslyPush = TRUE;
				PRINTF("Check Pad LR(or UD) Simulataneously Push.");
			}
			MENU_CheckItem(_SetInputMenu, WM_INPUT_CHECKPAD_LR, _bCheckSimultaneouslyPush);
			run_emulator(bRunNormalState);
			break;

		case APPEVENT_INPUT_CONFIGURE_JOYBG: //Kitao追加。v1.41
			if (_bJoypadBackground)
			{
				MENU_CheckItem(_InputMenu, WM_INPUT_CONFIGURE_JOYBG, FALSE);
				_bJoypadBackground = FALSE;
				PRINTF("Joypad Invalid at Background.");
			}
			else
			{
				MENU_CheckItem(_InputMenu, WM_INPUT_CONFIGURE_JOYBG, TRUE);
				_bJoypadBackground = TRUE;
				PRINTF("Joypad Effective at Background.");
			}
			CONFIG_Set("[APP INPUT] Joypad Background", &_bJoypadBackground, sizeof(_bJoypadBackground));
			INPUT_Deinit();
			INPUT_Init();
			APP_SetInputConfiguration();
			run_emulator(bRunNormalState);
			break;

		case APPEVENT_INPUT_CONFIGURE_KEYBG: //Kitao追加。v1.41
			if (_bKeyboardBackground)
			{
				MENU_CheckItem(_InputMenu, WM_INPUT_CONFIGURE_KEYBG, FALSE);
				_bKeyboardBackground = FALSE;
				PRINTF("Keyboard Invalid at Background.");
			}
			else
			{
				MENU_CheckItem(_InputMenu, WM_INPUT_CONFIGURE_KEYBG, TRUE);
				_bKeyboardBackground = TRUE;
				PRINTF("Keyboard Effective at Background.");
			}
			CONFIG_Set("[APP INPUT] Keyboard Background", &_bKeyboardBackground, sizeof(_bKeyboardBackground));
			INPUT_Deinit();
			INPUT_Init();
			APP_SetInputConfiguration();
			run_emulator(bRunNormalState);
			break;

		//ファンクションボタンを設定する。v2.38追加
		case APPEVENT_INPUT_FUNCTION:
			set_shortcut_key_button(6);
			break;
		case APPEVENT_INPUT_FB_CURSOR:
			if (_bFuncVolume)
			{
				_bFuncVolume = FALSE;
				PRINTF("Disable FuncButton + [U][D][L][R] for Volume.");
			}
			else
			{
				_bFuncVolume = TRUE;
				PRINTF("Enable FuncButton + [U][D][L][R] for Volume.");
			}
			MENU_CheckItem(_FuncButtonMenu, WM_INPUT_FB_CURSOR, _bFuncVolume);
			CONFIG_Set("[APP DEVICE] Function Volume", &_bFuncVolume, sizeof(_bFuncVolume));
			run_emulator(bRunNormalState);
			break;
		case APPEVENT_INPUT_FB_IandII:
			if (_bFuncTurboButton)
			{
				_bFuncTurboButton = FALSE;
				PRINTF("Disable FuncButton + [ I ][ II ] for TurboButton.");
			}
			else
			{
				_bFuncTurboButton = TRUE;
				PRINTF("Enable FuncButton + [ I ][ II ] for TurboButton.");
			}
			MENU_CheckItem(_FuncButtonMenu, WM_INPUT_FB_IandII, _bFuncTurboButton);
			CONFIG_Set("[APP DEVICE] Function TurboButton", &_bFuncTurboButton, sizeof(_bFuncTurboButton)); //v2.38追加
			run_emulator(bRunNormalState);
			break;
		case APPEVENT_INPUT_FB_SEL:
			if (_bFuncScreenshot)
			{
				_bFuncScreenshot = FALSE;
				PRINTF("Disable FuncButton + [SELECT] for Screenshot.");
			}
			else
			{
				_bFuncScreenshot = TRUE;
				PRINTF("Enable FuncButton + [SELECT] for Screenshot.");
			}
			MENU_CheckItem(_FuncButtonMenu, WM_INPUT_FB_SEL, _bFuncScreenshot);
			CONFIG_Set("[APP DEVICE] Function Screenshot", &_bFuncScreenshot, sizeof(_bFuncScreenshot)); //v2.38追加
			run_emulator(bRunNormalState);
			break;
		case APPEVENT_INPUT_FB_RUN:
			if (_bFuncRecent)
			{
				_bFuncRecent = FALSE;
				PRINTF("Disable FuncButton + [RUN] for RecentGame.");
			}
			else
			{
				_bFuncRecent = TRUE;
				PRINTF("Enable FuncButton + [RUN] for RecentGame.");
			}
			MENU_CheckItem(_FuncButtonMenu, WM_INPUT_FB_RUN, _bFuncRecent);
			CONFIG_Set("[APP DEVICE] Function RecentGame", &_bFuncRecent, sizeof(_bFuncRecent)); //v2.48追加
			run_emulator(bRunNormalState);
			break;
		case APPEVENT_INPUT_FB_VSPEEDUP:
			if (_bFuncVSpeedUp)
			{
				_bFuncVSpeedUp = FALSE;
				PRINTF("Disable FuncButton + [VSpeedUp] for VSpeedUp.");
			}
			else
			{
				_bFuncVSpeedUp = TRUE;
				PRINTF("Enable FuncButton + [VSpeedUp] for VSpeedUp.");
			}
			MENU_CheckItem(_FuncButtonMenu, WM_INPUT_FB_VSPEEDUP, _bFuncVSpeedUp);
			CONFIG_Set("[APP DEVICE] Function VSpeedUp", &_bFuncVSpeedUp, sizeof(_bFuncVSpeedUp)); //v2.38追加
			run_emulator(bRunNormalState);
			break;
		case APPEVENT_INPUT_FB_SAVELOAD:
			set_shortcut_key_button(7); //ファンクションボタン時のセーブ＆ロード用ボタンを設定
			break;

		case APPEVENT_SCREEN_STRETCHED:
			MENU_CheckRadioItem(_SizeMenu, WM_SCREEN_NONSTRETCHED, WM_SCREEN_VERTICAL, WM_SCREEN_STRETCHED);
			APP_SetStretched(TRUE, FALSE, FALSE); //Kitao更新
			CONFIG_Set("[APP DISPLAY] Stretched Mode", &_bStretched, sizeof(_bStretched));
			CONFIG_Set("[APP DISPLAY] FullStretched Mode", &_bFullStretched, sizeof(_bFullStretched));
			_StartStretchMode = 1; //v2.64
			APP_ReturnCaption(); //フルストレッチからすぐに元に戻したときのために必要
			run_emulator(bRunNormalState);
			break;
		case APPEVENT_SCREEN_FULLSTRETCHED: //Kitao追加
			MENU_CheckRadioItem(_SizeMenu, WM_SCREEN_NONSTRETCHED, WM_SCREEN_VERTICAL, WM_SCREEN_FULLSTRETCHED);
			APP_SetStretched(TRUE, FALSE, TRUE);
			CONFIG_Set("[APP DISPLAY] Stretched Mode", &_bStretched, sizeof(_bStretched));
			CONFIG_Set("[APP DISPLAY] FullStretched Mode", &_bFullStretched, sizeof(_bFullStretched));
			_StartStretchMode = 2; //v2.64
			if (!_bFullScreen)
				PRINTF("FullStretched Mode. Effective at FullScreen.");
			else
				APP_ReturnCaption();
			run_emulator(bRunNormalState);
			break;
		case APPEVENT_SCREEN_VERTICAL: //Kitao追加。縦画面モード（シューティングゲームの裏技やオプションであるやつを再現）
			MENU_CheckRadioItem(_SizeMenu, WM_SCREEN_NONSTRETCHED, WM_SCREEN_VERTICAL, WM_SCREEN_VERTICAL);
			APP_SetStretched(TRUE, TRUE, FALSE);
			CONFIG_Set("[APP DISPLAY] FullStretched Mode", &_bFullStretched, sizeof(_bFullStretched));
			APP_ReturnCaption(); //フルストレッチからすぐに元に戻したときのために必要
			run_emulator(bRunNormalState);
			break;
		case APPEVENT_SCREEN_NONSTRETCHED: //Kitao追加
			MENU_CheckRadioItem(_SizeMenu, WM_SCREEN_NONSTRETCHED, WM_SCREEN_VERTICAL, WM_SCREEN_NONSTRETCHED);
			APP_SetStretched(FALSE, FALSE, FALSE);
			CONFIG_Set("[APP DISPLAY] Stretched Mode", &_bStretched, sizeof(_bStretched));
			CONFIG_Set("[APP DISPLAY] FullStretched Mode", &_bFullStretched, sizeof(_bFullStretched));
			_StartStretchMode = 0; //v2.64
			APP_ReturnCaption(); //フルストレッチからすぐに元に戻したときのために必要
			run_emulator(bRunNormalState);
			break;

		//Kitao追加。TV Mode
		case APPEVENT_SCREEN_TV:
			if (_bTvMode)
			{
				APP_SetTvMode(FALSE, TRUE);
				PRINTF("RGB Mode.");
			}
			else
			{
				APP_SetTvMode(TRUE, TRUE);
				PRINTF("TV Mode.");
			}
			run_emulator(bRunNormalState);
			break;

		//Kitao追加。MonoColor Mode。v2.28
		case APPEVENT_SCREEN_MONOCOLOR:
			if (GetKeyState(VK_CONTROL) < 0) //Ctrlキーを押しながらの場合
				_MonoColorMode = 2; //グリーンディスプレイモード
			else //通常
			{
				if (MAINBOARD_GetForceMonoColor() != 0) //ステートロードでモノカラーモードに設定されていた場合も考慮するためMAINBOARD_GetForceMonoColor()を参照。
					_MonoColorMode = 0;
				else
					_MonoColorMode = 1; //白黒ディスプレイモード
			}
			MENU_CheckItem(_SizeMenu, WM_SCREEN_MONOCOLOR, (_MonoColorMode != 0));
			MAINBOARD_SetForceMonoColor(_MonoColorMode);
			run_emulator(bRunNormalState);
			break;

		//Kitao追加。オーバースキャン領域表示関連の設定。v1.43
		case APPEVENT_SCREEN_SHOWOVERSCAN:
			if (MAINBOARD_GetShowOverscan())
				setShowOverscanArea(FALSE);
			else
			{
				if ((_ShowOverscanTop == 0)&&(_ShowOverscanBottom == 0)&&(_ShowOverscanLeft == 0)&&(_ShowOverscanRight == 0))
				{	//オンにしたときに、カスタマイズで全てがNoneだった場合は、全てをオンにする。
					_ShowOverscanTop	= _ShowOverscanHeight;
					_ShowOverscanBottom	= _ShowOverscanHeight;
					_ShowOverscanLeft	= _ShowOverscanHeight;
					_ShowOverscanRight	= _ShowOverscanHeight;
				}
				setShowOverscanArea(TRUE);
			}
			run_emulator(bRunNormalState);
			break;
		//Kitao追加。v1.43
		case APPEVENT_SCREEN_OVERTB:
			_ShowOverscanTop = _ShowOverscanHeight;
			_ShowOverscanBottom = _ShowOverscanHeight;
			setShowOverscanArea(TRUE);
			run_emulator(bRunNormalState);
			break;
		//Kitao追加。v1.43
		case APPEVENT_SCREEN_OVERTOP:
			_ShowOverscanTop = _ShowOverscanHeight;
			_ShowOverscanBottom = 0;
			setShowOverscanArea(TRUE);
			run_emulator(bRunNormalState);
			break;
		//Kitao追加。v1.43
		case APPEVENT_SCREEN_OVERBOTTOM:
			_ShowOverscanTop = 0;
			_ShowOverscanBottom = _ShowOverscanHeight;
			setShowOverscanArea(TRUE);
			run_emulator(bRunNormalState);
			break;
		//Kitao追加。v1.43
		case APPEVENT_SCREEN_OVERNONETB:
			_ShowOverscanTop = 0;
			_ShowOverscanBottom = 0;
			setShowOverscanArea(TRUE);
			run_emulator(bRunNormalState);
			break;
		//Kitao追加。v1.43
		case APPEVENT_SCREEN_OVERHEIGHT8: setOverscanSub(8); run_emulator(bRunNormalState); break;
		case APPEVENT_SCREEN_OVERHEIGHT7: setOverscanSub(7); run_emulator(bRunNormalState); break;
		case APPEVENT_SCREEN_OVERHEIGHT6: setOverscanSub(6); run_emulator(bRunNormalState); break;
		case APPEVENT_SCREEN_OVERHEIGHT4: setOverscanSub(4); run_emulator(bRunNormalState); break;
		case APPEVENT_SCREEN_OVERHEIGHT2: setOverscanSub(2); run_emulator(bRunNormalState); break;//v1.61追加
		case APPEVENT_SCREEN_OVERHEIGHT1: setOverscanSub(1); run_emulator(bRunNormalState); break;//v1.61追加
		//Kitao追加。v1.43
		case APPEVENT_SCREEN_OVERLR:
			_ShowOverscanLeft	= 8; //現状は、左右は8ドット固定。
			_ShowOverscanRight	= 8;
			setShowOverscanArea(TRUE);
			run_emulator(bRunNormalState);
			break;
		//Kitao追加。v1.43
		case APPEVENT_SCREEN_OVERNONELR:
			_ShowOverscanLeft	= 0;
			_ShowOverscanRight	= 0;
			setShowOverscanArea(TRUE);
			run_emulator(bRunNormalState);
			break;
		//Kitao追加。v1.43
		case APPEVENT_SCREEN_OVERSTART:
			MAINBOARD_Pause(TRUE);
			WINMAIN_ShowCursor(TRUE);
			screenUpdate(); //メニューやダイアログ表示などで黒くなった部分を再描画。
			MessageBox( WINMAIN_GetHwnd(),
						"Please operate \"Setting -> Screen\" menu.    \nAnd, you can set it for the start setting.",
						"Ootake", MB_OK);
			screenUpdate(); //メニューやダイアログ表示などで黒くなった部分を再描画。
			break;
		//Kitao追加。ウィンドウモードでオーバースキャン領域を隠す際に、ウィンドウを広げたまま黒帯表示する設定。v1.45
		case APPEVENT_SCREEN_OVERBLACK:
			if (_bOverscanHideBlackBelt)
			{
				_bOverscanHideBlackBelt = FALSE;
				PRINTF("Non-Use Black Belt when Hide Overscan at window mode.");
			}
			else
			{
				_bOverscanHideBlackBelt = TRUE;
				PRINTF("Use Black Belt when Hide Overscan at window mode.");
			}
			if (!_bFullScreen)
				updateScreenMode(TRUE); //メニューもアップデートされる。
			else
				MENU_CheckItem(_SizeMenu, WM_SCREEN_OVERBLACK, _bOverscanHideBlackBelt);
			CONFIG_Set("[APP DISPLAY] Overscan HideBlack", &_bOverscanHideBlackBelt, sizeof(_bOverscanHideBlackBelt));
			run_emulator(bRunNormalState);
			break;

		//Kitao追加。スタート時のオーバースキャン領域表示関連の設定。v1.43
		case APPEVENT_SCREEN_SOVERTB:
			_StartShowOverscanTop = _StartShowOverscanHeight;
			_StartShowOverscanBottom = _StartShowOverscanHeight;
			setStartShowOverscanArea();
			CONFIG_Set("[APP DISPLAY] Overscan Top", &_StartShowOverscanTop, sizeof(_StartShowOverscanTop));
			CONFIG_Set("[APP DISPLAY] Overscan Bottom", &_StartShowOverscanBottom, sizeof(_StartShowOverscanBottom));
			run_emulator(bRunNormalState);
			break;
		//Kitao追加。v1.43
		case APPEVENT_SCREEN_SOVERTOP:
			_StartShowOverscanTop = _StartShowOverscanHeight;
			_StartShowOverscanBottom = 0;
			setStartShowOverscanArea();
			CONFIG_Set("[APP DISPLAY] Overscan Top", &_StartShowOverscanTop, sizeof(_StartShowOverscanTop));
			CONFIG_Set("[APP DISPLAY] Overscan Bottom", &_StartShowOverscanBottom, sizeof(_StartShowOverscanBottom));
			run_emulator(bRunNormalState);
			break;
		//Kitao追加。v1.43
		case APPEVENT_SCREEN_SOVERBOTTOM:
			_StartShowOverscanTop = 0;
			_StartShowOverscanBottom = _StartShowOverscanHeight;
			setStartShowOverscanArea();
			CONFIG_Set("[APP DISPLAY] Overscan Top", &_StartShowOverscanTop, sizeof(_StartShowOverscanTop));
			CONFIG_Set("[APP DISPLAY] Overscan Bottom", &_StartShowOverscanBottom, sizeof(_StartShowOverscanBottom));
			run_emulator(bRunNormalState);
			break;
		//Kitao追加。v1.43
		case APPEVENT_SCREEN_SOVERNONETB:
			_StartShowOverscanTop = 0;
			_StartShowOverscanBottom = 0;
			setStartShowOverscanArea();
			CONFIG_Set("[APP DISPLAY] Overscan Top", &_StartShowOverscanTop, sizeof(_StartShowOverscanTop));
			CONFIG_Set("[APP DISPLAY] Overscan Bottom", &_StartShowOverscanBottom, sizeof(_StartShowOverscanBottom));
			run_emulator(bRunNormalState);
			break;
		//Kitao追加。v1.43
		case APPEVENT_SCREEN_SOVERHEIGHT8: setStartOverscanSub(8); run_emulator(bRunNormalState); break;
		case APPEVENT_SCREEN_SOVERHEIGHT7: setStartOverscanSub(7); run_emulator(bRunNormalState); break;
		case APPEVENT_SCREEN_SOVERHEIGHT6: setStartOverscanSub(6); run_emulator(bRunNormalState); break;
		case APPEVENT_SCREEN_SOVERHEIGHT4: setStartOverscanSub(4); run_emulator(bRunNormalState); break;
		case APPEVENT_SCREEN_SOVERHEIGHT2: setStartOverscanSub(2); run_emulator(bRunNormalState); break; //v1.61追加
		case APPEVENT_SCREEN_SOVERHEIGHT1: setStartOverscanSub(1); run_emulator(bRunNormalState); break; //v1.61追加
		//Kitao追加。v1.43
		case APPEVENT_SCREEN_SOVERLR:
			_StartShowOverscanLeft	= 8; //現状は、左右は8ドット固定。
			_StartShowOverscanRight	= 8;
			setStartShowOverscanArea();
			CONFIG_Set("[APP DISPLAY] Overscan Left", &_StartShowOverscanLeft, sizeof(_StartShowOverscanLeft));
			CONFIG_Set("[APP DISPLAY] Overscan Right", &_StartShowOverscanRight, sizeof(_StartShowOverscanRight));
			run_emulator(bRunNormalState);
			break;
		//Kitao追加。v1.43
		case APPEVENT_SCREEN_SOVERNONELR:
			_StartShowOverscanLeft	= 0;
			_StartShowOverscanRight	= 0;
			setStartShowOverscanArea();
			CONFIG_Set("[APP DISPLAY] Overscan Left", &_StartShowOverscanLeft, sizeof(_StartShowOverscanLeft));
			CONFIG_Set("[APP DISPLAY] Overscan Right", &_StartShowOverscanRight, sizeof(_StartShowOverscanRight));
			run_emulator(bRunNormalState);
			break;

		case APPEVENT_SCREEN_TOPMOST: //Kitao追加
			if (_bWindowTopMost)
			{
				_bWindowTopMost = FALSE;
				PRINTF("The Window is a normal position.");
			}
			else
			{
				_bWindowTopMost = TRUE;
				PRINTF("The Window is TopMost.");
			}
			MENU_CheckItem(_SizeMenu, WM_SCREEN_TOPMOST, _bWindowTopMost);
			CONFIG_Set("[APP DISPLAY] Window TopMost", &_bWindowTopMost, sizeof(_bWindowTopMost));
			run_emulator(bRunNormalState);
			break;

		//Kitao追加
		case APPEVENT_SCREEN_NONSCANLINED:
			_ScanLineType = 0;
			APP_SetTvMode(FALSE, TRUE); //ガンマも更新される
			CONFIG_Set("[APP DISPLAY] Scanline Type", &_ScanLineType, sizeof(_ScanLineType));
			updateMenu();
			PRINTF("RGB Mode.");
			run_emulator(bRunNormalState);
			break;
		case APPEVENT_SCREEN_SPSCANLINED:
			_ScanLineType = 1;
			APP_SetTvMode(FALSE, TRUE); //ガンマも更新される
			CONFIG_Set("[APP DISPLAY] Scanline Type", &_ScanLineType, sizeof(_ScanLineType));
			updateMenu();
			PRINTF("RGB Mode.");
			run_emulator(bRunNormalState);
			break;
		case APPEVENT_SCREEN_HRSCANLINED:
			_ScanLineType = 2;
			APP_SetTvMode(FALSE, TRUE); //ガンマも更新される
			CONFIG_Set("[APP DISPLAY] Scanline Type", &_ScanLineType, sizeof(_ScanLineType));
			updateMenu();
			PRINTF("RGB Mode.");
			run_emulator(bRunNormalState);
			break;
		case APPEVENT_SCREEN_STARTTV:
			_ScanLineType = 4;
			APP_SetTvMode(TRUE, TRUE); //ガンマも更新される
			CONFIG_Set("[APP DISPLAY] Scanline Type", &_ScanLineType, sizeof(_ScanLineType));
			updateMenu();
			PRINTF("TV Mode.");
			run_emulator(bRunNormalState);
			break;

		//Kitao追加。スキャンラインの濃度を変更する。
		case APPEVENT_SCREEN_SCANDENSITY0:	MENU_CheckRadioItem(_ScreenMenu, WM_SCREEN_SCANDENSITY0, WM_SCREEN_SCANDENSITY80, WM_SCREEN_SCANDENSITY0);
											_ScanLineDensity = 0; CONFIG_Set("[APP DISPLAY] Scanline Density", &_ScanLineDensity, sizeof(_ScanLineDensity));
											SCREEN_SetGamma(APP_GetScanLineType(), _ScanLineDensity, _bTvMode); run_emulator(bRunNormalState); break;
		case APPEVENT_SCREEN_SCANDENSITY10:	MENU_CheckRadioItem(_ScreenMenu, WM_SCREEN_SCANDENSITY0, WM_SCREEN_SCANDENSITY80, WM_SCREEN_SCANDENSITY10);
											_ScanLineDensity = 10; CONFIG_Set("[APP DISPLAY] Scanline Density", &_ScanLineDensity, sizeof(_ScanLineDensity));
											SCREEN_SetGamma(APP_GetScanLineType(), _ScanLineDensity, _bTvMode); run_emulator(bRunNormalState); break;
		case APPEVENT_SCREEN_SCANDENSITY20:	MENU_CheckRadioItem(_ScreenMenu, WM_SCREEN_SCANDENSITY0, WM_SCREEN_SCANDENSITY80, WM_SCREEN_SCANDENSITY20);
											_ScanLineDensity = 20; CONFIG_Set("[APP DISPLAY] Scanline Density", &_ScanLineDensity, sizeof(_ScanLineDensity));
											SCREEN_SetGamma(APP_GetScanLineType(), _ScanLineDensity, _bTvMode); run_emulator(bRunNormalState); break;
		case APPEVENT_SCREEN_SCANDENSITY30:	MENU_CheckRadioItem(_ScreenMenu, WM_SCREEN_SCANDENSITY0, WM_SCREEN_SCANDENSITY80, WM_SCREEN_SCANDENSITY30);
											_ScanLineDensity = 30; CONFIG_Set("[APP DISPLAY] Scanline Density", &_ScanLineDensity, sizeof(_ScanLineDensity));
											SCREEN_SetGamma(APP_GetScanLineType(), _ScanLineDensity, _bTvMode); run_emulator(bRunNormalState); break;
		case APPEVENT_SCREEN_SCANDENSITY40:	MENU_CheckRadioItem(_ScreenMenu, WM_SCREEN_SCANDENSITY0, WM_SCREEN_SCANDENSITY80, WM_SCREEN_SCANDENSITY40);
											_ScanLineDensity = 40; CONFIG_Set("[APP DISPLAY] Scanline Density", &_ScanLineDensity, sizeof(_ScanLineDensity));
											SCREEN_SetGamma(APP_GetScanLineType(), _ScanLineDensity, _bTvMode); run_emulator(bRunNormalState); break;
		case APPEVENT_SCREEN_SCANDENSITY50:	MENU_CheckRadioItem(_ScreenMenu, WM_SCREEN_SCANDENSITY0, WM_SCREEN_SCANDENSITY80, WM_SCREEN_SCANDENSITY50);
											_ScanLineDensity = 50; CONFIG_Set("[APP DISPLAY] Scanline Density", &_ScanLineDensity, sizeof(_ScanLineDensity));
											SCREEN_SetGamma(APP_GetScanLineType(), _ScanLineDensity, _bTvMode); run_emulator(bRunNormalState); break;
		case APPEVENT_SCREEN_SCANDENSITY60:	MENU_CheckRadioItem(_ScreenMenu, WM_SCREEN_SCANDENSITY0, WM_SCREEN_SCANDENSITY80, WM_SCREEN_SCANDENSITY60);
											_ScanLineDensity = 60; CONFIG_Set("[APP DISPLAY] Scanline Density", &_ScanLineDensity, sizeof(_ScanLineDensity));
											SCREEN_SetGamma(APP_GetScanLineType(), _ScanLineDensity, _bTvMode); run_emulator(bRunNormalState); break;
		case APPEVENT_SCREEN_SCANDENSITY70:	MENU_CheckRadioItem(_ScreenMenu, WM_SCREEN_SCANDENSITY0, WM_SCREEN_SCANDENSITY80, WM_SCREEN_SCANDENSITY70);
											_ScanLineDensity = 70; CONFIG_Set("[APP DISPLAY] Scanline Density", &_ScanLineDensity, sizeof(_ScanLineDensity));
											SCREEN_SetGamma(APP_GetScanLineType(), _ScanLineDensity, _bTvMode); run_emulator(bRunNormalState); break;
		case APPEVENT_SCREEN_SCANDENSITY80:	MENU_CheckRadioItem(_ScreenMenu, WM_SCREEN_SCANDENSITY0, WM_SCREEN_SCANDENSITY80, WM_SCREEN_SCANDENSITY80);
											_ScanLineDensity = 80; CONFIG_Set("[APP DISPLAY] Scanline Density", &_ScanLineDensity, sizeof(_ScanLineDensity));
											SCREEN_SetGamma(APP_GetScanLineType(), _ScanLineDensity, _bTvMode); run_emulator(bRunNormalState); break;

		//Kitao追加。スキャンラインの濃度によってガンマ値を最適化するかどうかの設定（デフォルト…暗部のキャラクターが見づらくならないように最適化する）。v2.35
		case APPEVENT_SCREEN_SCANGAMMA:
			_bOptimizeGamma ^= TRUE;
			MENU_CheckItem(_DensityMenu, WM_SCREEN_SCANGAMMA, _bOptimizeGamma);
			CONFIG_Set("[APP DISPLAY] Optimize Gamma", &_bOptimizeGamma, sizeof(_bOptimizeGamma));
			SCREEN_SetGamma(APP_GetScanLineType(), _ScanLineDensity, _bTvMode);
			run_emulator(bRunNormalState);
			break;

		case APPEVENT_AUDIO_SYNC_VBLANK: //Kitao追加。AudioメニューからのV-Sync設定。v2.65
			_WindowsAutoVSync = 0;
			_bSyncTo60HzScreen ^= TRUE;
			if (_bSyncTo60HzScreen)
				MENU_CheckRadioItem(_ScreenMenu, WM_SCREEN_SYNC_VBLANK, WM_SCREEN_SYNC_NON, WM_SCREEN_SYNC_VBLANK);
			else
				MENU_CheckRadioItem(_ScreenMenu, WM_SCREEN_SYNC_VBLANK, WM_SCREEN_SYNC_NON, WM_SCREEN_SYNC_NON);
			MENU_CheckItem(_AudioSubMenu, WM_AUDIO_SYNC_VBLANK, _bSyncTo60HzScreen);
			SCREEN_SetSyncTo60HzScreen(_bSyncTo60HzScreen);
			CONFIG_Set("[APP DISPLAY] Windows Auto VSync", &_WindowsAutoVSync, sizeof(_WindowsAutoVSync));//Kitao追加。v2.65
			CONFIG_Set("[APP DISPLAY] Sync To 60Hz Fullscreen", &_bSyncTo60HzScreen, sizeof(_bSyncTo60HzScreen)); //Kitao追加
			sprintf(buf, "%d", getCddaAdjust());
			if ((getCddaAdjust() % 10) == 0)
			{
				buf[4] = 0;
				buf[3] = buf[2];
				buf[2] = '.';
			}
			else
			{
				buf[5] = 0;
				buf[4] = buf[3];
				buf[3] = buf[2];
				buf[2] = '.';
			}
			if (_bSyncTo60HzScreen)
				PRINTF("Set V-Sync Mode. ( CD-DA V%sHz )", buf);
			else
				PRINTF("Non-V-Sync Mode. ( CD-DA V%sHz )", buf);
			run_emulator(bRunNormalState);
			break;

		case APPEVENT_SCREEN_SYNC_VBLANK:
			_WindowsAutoVSync = 0;
			_bSyncTo60HzScreen = TRUE;
			MENU_CheckRadioItem(_ScreenMenu, WM_SCREEN_SYNC_VBLANK, WM_SCREEN_SYNC_NON, WM_SCREEN_SYNC_VBLANK);
			MENU_CheckItem(_AudioSubMenu, WM_AUDIO_SYNC_VBLANK, _bSyncTo60HzScreen); //v2.32追加
			SCREEN_SetSyncTo60HzScreen(_bSyncTo60HzScreen); //Kitao更新。画面モードは変更せずに高速に切り替わるようにした。
			CONFIG_Set("[APP DISPLAY] Windows Auto VSync", &_WindowsAutoVSync, sizeof(_WindowsAutoVSync));//Kitao追加。v2.65
			CONFIG_Set("[APP DISPLAY] Sync To 60Hz Fullscreen", &_bSyncTo60HzScreen, sizeof(_bSyncTo60HzScreen)); //Kitao追加
			sprintf(buf, "%d", getCddaAdjust());
			if ((getCddaAdjust() % 10) == 0)
			{
				buf[4] = 0;
				buf[3] = buf[2];
				buf[2] = '.';
			}
			else
			{
				buf[5] = 0;
				buf[4] = buf[3];
				buf[3] = buf[2];
				buf[2] = '.';
			}
			PRINTF("Set V-Sync Mode. ( CD-DA V%sHz )", buf);
			run_emulator(bRunNormalState);
			break;

		/*
		case APPEVENT_SCREEN_SYNC_FORCE: //Kitao追加。v2.41。v2.65から廃止
			_bForceVSync ^= TRUE;
			MENU_CheckItem(_ScreenMenu, WM_SCREEN_SYNC_FORCE, _bForceVSync);
			SCREEN_SetSyncTo60HzScreen(_bSyncTo60HzScreen); //V-Sync設定を反映
			CONFIG_Set("[APP DISPLAY] Force VSync", &_bForceVSync, sizeof(_bForceVSync));
			if (_bForceVSync)
				PRINTF("Set Force V-Sync Mode.");
			else
				PRINTF("Reset Force V-Sync Mode.");
			run_emulator(bRunNormalState);
			break;
		*/

		case APPEVENT_SCREEN_SYNC_WINDOWS: //Kitao追加。v2.65
			_WindowsAutoVSync = 1;
			_bSyncTo60HzScreen = FALSE;
			MENU_CheckRadioItem(_ScreenMenu, WM_SCREEN_SYNC_VBLANK, WM_SCREEN_SYNC_NON, WM_SCREEN_SYNC_WINDOWS);
			MENU_CheckItem(_AudioSubMenu, WM_AUDIO_SYNC_VBLANK, _bSyncTo60HzScreen);
			if (_DrawMethod == 1) //Direct3D利用時のみ
				SCREEND3D_UpdateWindowsAutoVSync(); //V-Sync設定を反映(Direct3D初期化)
			CONFIG_Set("[APP DISPLAY] Windows Auto VSync", &_WindowsAutoVSync, sizeof(_WindowsAutoVSync));
			CONFIG_Set("[APP DISPLAY] Sync To 60Hz Fullscreen", &_bSyncTo60HzScreen, sizeof(_bSyncTo60HzScreen));
			PRINTF("Set Windows Auto V-Sync Mode. (Always)");
			run_emulator(bRunNormalState);
			break;

		case APPEVENT_SCREEN_SYNC_WINDOWSF: //Kitao追加。v2.65
			_WindowsAutoVSync = 2; //フルスクリーン時のみオートV-Sync。
			MENU_CheckRadioItem(_ScreenMenu, WM_SCREEN_SYNC_VBLANK, WM_SCREEN_SYNC_NON, WM_SCREEN_SYNC_WINDOWSF);
			if (_bFullScreen)
			{
				_bSyncTo60HzScreen = FALSE;
				if (_DrawMethod == 1) //Direct3D利用時のみ
					SCREEND3D_UpdateWindowsAutoVSync(); //V-Sync設定を反映(Direct3D初期化)
			}
			else
			{
				_bSyncTo60HzScreen = TRUE;
				SCREEN_SetSyncTo60HzScreen(_bSyncTo60HzScreen);
			}
			MENU_CheckItem(_AudioSubMenu, WM_AUDIO_SYNC_VBLANK, _bSyncTo60HzScreen);
			CONFIG_Set("[APP DISPLAY] Windows Auto VSync", &_WindowsAutoVSync, sizeof(_WindowsAutoVSync));
			CONFIG_Set("[APP DISPLAY] Sync To 60Hz Fullscreen", &_bSyncTo60HzScreen, sizeof(_bSyncTo60HzScreen));
			PRINTF("Set Windows Auto V-Sync Mode. (FullScreen)");
			run_emulator(bRunNormalState);
			break;

		case APPEVENT_SCREEN_SYNC_NON: //Kitao追加。v2.65
			_WindowsAutoVSync = 0;
			_bSyncTo60HzScreen = FALSE;
			MENU_CheckRadioItem(_ScreenMenu, WM_SCREEN_SYNC_VBLANK, WM_SCREEN_SYNC_NON, WM_SCREEN_SYNC_NON);
			MENU_CheckItem(_AudioSubMenu, WM_AUDIO_SYNC_VBLANK, _bSyncTo60HzScreen);
			if (_DrawMethod == 1) //Direct3D利用時のみ
				SCREEND3D_UpdateWindowsAutoVSync(); //V-Sync設定を反映(Direct3D初期化)
			CONFIG_Set("[APP DISPLAY] Windows Auto VSync", &_WindowsAutoVSync, sizeof(_WindowsAutoVSync));//Kitao追加。v2.65
			CONFIG_Set("[APP DISPLAY] Sync To 60Hz Fullscreen", &_bSyncTo60HzScreen, sizeof(_bSyncTo60HzScreen)); //Kitao追加
			sprintf(buf, "%d", getCddaAdjust());
			if ((getCddaAdjust() % 10) == 0)
			{
				buf[4] = 0;
				buf[3] = buf[2];
				buf[2] = '.';
			}
			else
			{
				buf[5] = 0;
				buf[4] = buf[3];
				buf[3] = buf[2];
				buf[2] = '.';
			}
			PRINTF("Non-V-Sync Mode. ( CD-DA V%sHz )", buf);
			run_emulator(bRunNormalState);
			break;

		case APPEVENT_SCREEN_SYNC_ADJUST: //Kitao追加。v2.65
			if (_VSyncAdjust == 0)
			{
				_VSyncAdjust = 13;
				MENU_CheckItem(_ScreenMenu, WM_SCREEN_SYNC_ADJUST, TRUE);
				if (!_bFullScreen)
					PRINTF("Set V-Sync Adjust. Effective at FullScreen.");
				else
					PRINTF("Set V-Sync Adjust.");
			}
			else
			{
				_VSyncAdjust = 0;
				MENU_CheckItem(_ScreenMenu, WM_SCREEN_SYNC_ADJUST, FALSE);
				PRINTF("Reset V-Sync Adjust.");
			}
			CONFIG_Set("[APP DISPLAY] VSync Adjust", &_VSyncAdjust, sizeof(_VSyncAdjust));
			run_emulator(bRunNormalState);
			break;

		case APPEVENT_SCREEN_DIRECT3D: //Kitao追加。v2.00
			if (_DrawMethod != 1)
			{
				if (_bFullScreen)
					APP_ToggleFullscreen(); //Direct3Dに切り替える前に、フルスクリーンモードを解除。
				SCREEN_Deinit();
			}
			_DrawMethod = 1;
			if (!updateScreenMode(FALSE))
			{	//失敗
				_DrawMethod = 2;
				return FALSE; //Ootakeを終了
			}
			CONFIG_Set("[APP DISPLAY] Draw Method", &_DrawMethod, sizeof(_DrawMethod));
			PRINTF("Use Direct3D.");
			run_emulator(bRunNormalState);
			break;

		case APPEVENT_SCREEN_DIRECTDRAW: //Kitao追加。v2.00
			if (_DrawMethod != 2)
			{
				if (_bFullScreen)
					APP_ToggleFullscreen(); //DirectDrawに切り替える前に、フルスクリーンモードを解除。
				SCREEN_Deinit();
			}
			_DrawMethod = 2;
			if (!updateScreenMode(FALSE))
			{	//失敗
				_DrawMethod = 1;
				return FALSE; //Ootakeを終了
			}
			CONFIG_Set("[APP DISPLAY] Draw Method", &_DrawMethod, sizeof(_DrawMethod));
			PRINTF("Use DirectDraw.");
			run_emulator(bRunNormalState);
			break;

		case APPEVENT_SCREEN_USE3DT_LINEAR: //Kitao追加。v2.00
			_D3DEffectTV = 2;
			CONFIG_Set("[APP DISPLAY] Direct3D Effect TV", &_D3DEffectTV, sizeof(_D3DEffectTV));
			updateMenu();
			if (_DrawMethod == 1)
			{
				APP_SetTvMode(TRUE, TRUE);
				PRINTF("TV Use Direct3D LINEAR Effect (Smooth).");
			}
			//App.cの変数を設定するだけで次のフレーム描画から反映されるので、updateScreenMode()を呼んで画面を再設定する必要はない。
			run_emulator(bRunNormalState);
			break;
		case APPEVENT_SCREEN_USE3DT_POINT: //Kitao追加。v2.00
			_D3DEffectTV = 1;
			CONFIG_Set("[APP DISPLAY] Direct3D Effect TV", &_D3DEffectTV, sizeof(_D3DEffectTV));
			updateMenu();
			if (_DrawMethod == 1)
			{
				APP_SetTvMode(TRUE, TRUE);
				PRINTF("TV Use Direct3D POINT Effect (Sharp).");
			}
			run_emulator(bRunNormalState);
			break;
		case APPEVENT_SCREEN_USE3DT_NONE: //Kitao追加。v2.00
			_D3DEffectTV = 0;
			CONFIG_Set("[APP DISPLAY] Direct3D Effect TV", &_D3DEffectTV, sizeof(_D3DEffectTV));
			updateMenu();
			if (_DrawMethod == 1)
			{
				APP_SetTvMode(TRUE, TRUE);
				PRINTF("TV Use Direct3D NONE Effect (Most Sharp).");
			}
			run_emulator(bRunNormalState);
			break;

		case APPEVENT_SCREEN_USE3DR_LINEAR: //Kitao追加。v2.00
			_D3DEffectRGB = 2;
			CONFIG_Set("[APP DISPLAY] Direct3D Effect RGB", &_D3DEffectRGB, sizeof(_D3DEffectRGB));
			updateMenu();
			if (_DrawMethod == 1)
			{
				APP_SetTvMode(FALSE, TRUE);
				PRINTF("RGB Use Direct3D LINEAR Effect (Smooth).");
			}
			//App.cの変数を設定するだけで次のフレーム描画から反映されるので、updateScreenMode()を呼んで画面を再設定する必要はない。
			run_emulator(bRunNormalState);
			break;
		case APPEVENT_SCREEN_USE3DR_POINT: //Kitao追加。v2.00
			_D3DEffectRGB = 1;
			CONFIG_Set("[APP DISPLAY] Direct3D Effect RGB", &_D3DEffectRGB, sizeof(_D3DEffectRGB));
			updateMenu();
			if (_DrawMethod == 1)
			{
				APP_SetTvMode(FALSE, TRUE);
				PRINTF("RGB Use Direct3D POINT Effect (Sharp).");
			}
			run_emulator(bRunNormalState);
			break;
		case APPEVENT_SCREEN_USE3DR_NONE: //Kitao追加。v2.00
			_D3DEffectRGB = 0;
			CONFIG_Set("[APP DISPLAY] Direct3D Effect RGB", &_D3DEffectRGB, sizeof(_D3DEffectRGB));
			updateMenu();
			if (_DrawMethod == 1)
			{
				APP_SetTvMode(FALSE, TRUE);
				PRINTF("RGB Use Direct3D NONE Effect (Most Sharp).");
			}
			run_emulator(bRunNormalState);
			break;

		case APPEVENT_SCREEN_USE3D_HELP: //Kitao追加。v2.00
			screenUpdate(); //メニューやダイアログ表示などで黒くなった部分を再描画。
			MessageBox(	WINMAIN_GetHwnd(),
						"TV<->RGB switch is [Ctrl]+[F11] key.\n\n"
						"At the most top menu of \"Setting->Screen\" menu, it can be set, too.   \n"
						"In that case, it is reflected when Ootake starts next time.\n",
						"Ootake", MB_OK);
			break;

		case APPEVENT_SCREEN_USE_VIDEOMEM: //Kitao追加
			_bUseVideoCardMemory = TRUE;
			_bUseSystemMemoryOnlyWindow = FALSE;
			CONFIG_Set("[APP DISPLAY] Use VideoCard Memory", &_bUseVideoCardMemory, sizeof(_bUseVideoCardMemory));
			CONFIG_Set("[APP DISPLAY] Use System Memory Only Window", &_bUseSystemMemoryOnlyWindow, sizeof(_bUseSystemMemoryOnlyWindow));//Kitao追加
			updateMenu();
			if (_DrawMethod == 2)
			{
				updateScreenMode(FALSE);
				PRINTF("Use VideoCard Memory Mode.");
			}
			run_emulator(bRunNormalState);
			break;
		case APPEVENT_SCREEN_USE_SYSTEMMEM: //Kitao追加
			_bUseVideoCardMemory = FALSE;
			_bUseSystemMemoryOnlyWindow = FALSE;
			CONFIG_Set("[APP DISPLAY] Use VideoCard Memory", &_bUseVideoCardMemory, sizeof(_bUseVideoCardMemory));
			CONFIG_Set("[APP DISPLAY] Use System Memory Only Window", &_bUseSystemMemoryOnlyWindow, sizeof(_bUseSystemMemoryOnlyWindow));//Kitao追加
			updateMenu();
			if (_DrawMethod == 2)
			{
				updateScreenMode(FALSE);
				PRINTF("Use System Memory Mode.");
			}
			run_emulator(bRunNormalState);
			break;
		case APPEVENT_SCREEN_USE_SYSTEMMEMW: //Kitao追加。v1.53
			_bUseVideoCardMemory = FALSE;
			_bUseSystemMemoryOnlyWindow = TRUE;
			CONFIG_Set("[APP DISPLAY] Use VideoCard Memory", &_bUseVideoCardMemory, sizeof(_bUseVideoCardMemory));
			CONFIG_Set("[APP DISPLAY] Use System Memory Only Window", &_bUseSystemMemoryOnlyWindow, sizeof(_bUseSystemMemoryOnlyWindow));//Kitao追加
			updateMenu();
			if (_DrawMethod == 2)
			{
				updateScreenMode(FALSE);
				PRINTF("Use System Memory Mode at Only Window-mode.");
			}
			run_emulator(bRunNormalState);
			break;

		case APPEVENT_SCREEN_FULL16BITCOLOR: //Kitao追加
			_FullScreenColor = 16;
			CONFIG_Set("[APP DISPLAY] FullScreen Color", &_FullScreenColor, sizeof(_FullScreenColor));
			updateMenu();
			if (_DrawMethod == 2)
			{
				if (_bFullScreen)
					updateScreenMode(FALSE);
				PRINTF("Use FullScreen 16bit Color.");
			}
			run_emulator(bRunNormalState);
			break;
		case APPEVENT_SCREEN_FULL32BITCOLOR: //Kitao追加
			_FullScreenColor = 32;
			CONFIG_Set("[APP DISPLAY] FullScreen Color", &_FullScreenColor, sizeof(_FullScreenColor));
			updateMenu();
			if (_DrawMethod == 2)
			{
				if (_bFullScreen)
					updateScreenMode(FALSE);
				PRINTF("Use FullScreen 32bit Color.");
			}
			run_emulator(bRunNormalState);
			break;

		case APPEVENT_SCREEN_STARTWINDOW: //Kitao追加
			MENU_CheckRadioItem(_ScreenMenu, WM_SCREEN_STARTWINDOW, WM_SCREEN_STARTFULL, WM_SCREEN_STARTWINDOW);
			_bStartFullScreen = FALSE;
			CONFIG_Set("[APP DISPLAY] Full Screen Mode", &_bStartFullScreen, sizeof(_bStartFullScreen));
			if (_bFullScreen)
				APP_ToggleFullscreen();
			PRINTF("It starts in the Window mode.");
			screenUpdate(); //PRINTFメッセージをすぐに表示する
			Sleep(1000); //他のアプリのウィンドウ復元などでCPU処理が取られて音が乱れるので１秒待ってから再開させる。
			run_emulator(bRunNormalState);
			break;

		case APPEVENT_SCREEN_STARTFULL: //Kitao追加
			MENU_CheckRadioItem(_ScreenMenu, WM_SCREEN_STARTWINDOW, WM_SCREEN_STARTFULL, WM_SCREEN_STARTFULL);
			_bStartFullScreen = TRUE;
			CONFIG_Set("[APP DISPLAY] Full Screen Mode", &_bStartFullScreen, sizeof(_bStartFullScreen));
			if (!_bFullScreen)
				APP_ToggleFullscreen();
			PRINTF("It starts in the FullScreen mode.");
			screenUpdate(); //PRINTFメッセージをすぐに表示する
			Sleep(1000); //他のアプリのウィンドウ復元などでCPU処理が取られて音が乱れるので１秒待ってから再開させる。
			run_emulator(bRunNormalState);
			break;

		case APPEVENT_SCREEN_ACTIVATE: //Kitao追加
			MENU_CheckRadioItem(_ScreenMenu, WM_SCREEN_ACTIVATE, WM_SCREEN_NONACTIVATE, WM_SCREEN_ACTIVATE);
			_bWindowActivate = TRUE;
			CONFIG_Set("[APP DISPLAY] Window Activate", &_bWindowActivate, sizeof(_bWindowActivate));
			PRINTF("Window is always automatically activated.");
			run_emulator(bRunNormalState);
			break;

		case APPEVENT_SCREEN_NONACTIVATE: //Kitao追加
			MENU_CheckRadioItem(_ScreenMenu, WM_SCREEN_ACTIVATE, WM_SCREEN_NONACTIVATE, WM_SCREEN_NONACTIVATE);
			_bWindowActivate = FALSE;
			CONFIG_Set("[APP DISPLAY] Window Activate", &_bWindowActivate, sizeof(_bWindowActivate));
			PRINTF("Window is not automatically activated.");
			run_emulator(bRunNormalState);
			break;

		case APPEVENT_SCREEN_UNPAUSE: //Kitao追加。v2.26
			if (_bInactivePause)
			{
				_bInactivePause = FALSE;
				PRINTF("Pause isn't done when window becomes non-active.");
			}
			else
			{
				_bInactivePause = TRUE;
				PRINTF("Pause is done when window becomes non-active.");
			}
			MENU_CheckItem(_ScreenMenu, WM_SCREEN_UNPAUSE, !_bInactivePause);
			CONFIG_Set("[APP DISPLAY] Window Pause", &_bInactivePause, sizeof(_bInactivePause));
			run_emulator(bRunNormalState);
			break;

		case APPEVENT_MENU_HIDEMENU: //Kitao追加
			if (_bHideMenu)
			{
				_bHideMenu = FALSE;
				PRINTF("The Menu is always displayed.");
			}
			else
			{
				_bHideMenu = TRUE;
				PRINTF("The Menu is automatically hidden.");
			}
			MENU_CheckItem(_SettingMenu, WM_MENU_HIDEMENU, _bHideMenu);
			CONFIG_Set("[APP DISPLAY] Hide Menu", &_bHideMenu, sizeof(_bHideMenu));
			run_emulator(bRunNormalState);
			break;

		case APPEVENT_MENU_HIDEMESSAGE: //Kitao追加
			PRINTF(_AppName); //キャプションをノーマルに戻す。
			SCREEN_SetMessageText(""); //表示中のメッセージを消す
			if (_bFullScreen)
			{
				if (_bFullHideMessage)
				{
					_bFullHideMessage = FALSE;
					PRINTF("The message is displayed here.");
				}
				else
					_bFullHideMessage = TRUE;
				MENU_CheckItem(_SettingMenu, WM_MENU_HIDEMESSAGE, _bFullHideMessage);
				CONFIG_Set("[APP DISPLAY] Full Hide Message", &_bFullHideMessage, sizeof(_bFullHideMessage));
			}
			else
			{
				if (_bHideMessage)
					_bHideMessage = FALSE;
				else
					_bHideMessage = TRUE;
				PRINTF("The message is displayed here.");
				MENU_CheckItem(_SettingMenu, WM_MENU_HIDEMESSAGE, _bHideMessage);
				CONFIG_Set("[APP DISPLAY] Hide Message", &_bHideMessage, sizeof(_bHideMessage));
			}
			run_emulator(bRunNormalState);
			break;

		case APPEVENT_PRIORITY_HIGH: //Kitao追加。v2.04
			_PriorityClass = HIGH_PRIORITY_CLASS;
			MENU_CheckRadioItem(_SettingMenu, WM_PRIORITY_HIGH, WM_PRIORITY_NORMAL, WM_PRIORITY_HIGH);
			CONFIG_Set("[APP DEVICE] Windows PriorityClass", &_PriorityClass, sizeof(_PriorityClass));
			WINMAIN_SetPriority(_PriorityClass);
			PRINTF("High Priority Mode. (HQ Sound)");
			run_emulator(bRunNormalState);
			break;

		case APPEVENT_PRIORITY_NORMAL: //Kitao追加。v2.04
			_PriorityClass = ABOVE_NORMAL_PRIORITY_CLASS; //Vistaで最低ABOVE_NORMALは必要。
			MENU_CheckRadioItem(_SettingMenu, WM_PRIORITY_HIGH, WM_PRIORITY_NORMAL, WM_PRIORITY_NORMAL);
			CONFIG_Set("[APP DEVICE] Windows PriorityClass", &_PriorityClass, sizeof(_PriorityClass));
			WINMAIN_SetPriority(_PriorityClass);
			PRINTF("Low Priority Mode. (Cooperation)");
			run_emulator(bRunNormalState);
			break;

		case APPEVENT_INPUT_CONFIGURE_PAD1:	configure_pad(1);	break;
		case APPEVENT_INPUT_CONFIGURE_PAD2:	configure_pad(2);	break;
		case APPEVENT_INPUT_CONFIGURE_PAD3:	configure_pad(3);	break;
		case APPEVENT_INPUT_CONFIGURE_PAD4:	configure_pad(4);	break;
		case APPEVENT_INPUT_CONFIGURE_PAD5:	configure_pad(5);	break;
		//Kitao追加
		case APPEVENT_INPUT_CONFIGURE_TB1:	configure_turbo_button(1);	break;
		case APPEVENT_INPUT_CONFIGURE_TB2:	configure_turbo_button(2);	break;
		case APPEVENT_INPUT_CONFIGURE_TB3:	configure_turbo_button(3);	break;
		case APPEVENT_INPUT_CONFIGURE_TB4:	configure_turbo_button(4);	break;
		case APPEVENT_INPUT_CONFIGURE_TB5:	configure_turbo_button(5);	break;
		//Kitao追加
		case APPEVENT_INPUT_CONFIGURE_INIT: //デフォルトのジョイパッド設定に戻す。
			MAINBOARD_Pause(TRUE);
			WINMAIN_ShowCursor(TRUE);
			strcpy(buf, "Do you return \"");
			if (JOYPAD_GetConnectSixButton())
				strcat(buf, "6");
			else if (JOYPAD_GetConnectThreeButton())
				strcat(buf, "3");
			else
				strcat(buf, "2");
			strcat(buf, "-button Pad Configuration\" to the default setting?    ");
			screenUpdate(); //メニューやダイアログ表示などで黒くなった部分を再描画。
			if (MessageBox(WINMAIN_GetHwnd(), buf, "JoyPad Initialize", MB_YESNO) == IDYES)
			{
				if (JOYPAD_GetConnectSixButton())
				{
					initPadConfiguration6();
					if (INPUT_GetNumJoystick() == 0) //ジョイスティックが１本もつながっていなければ、PCエンジンパッド１はキーボードを使う設定で初期化する。
						initPad1ConfigurationKB6();
					CONFIG_Set("[APP INPUT] JoyPad Setting for 6Button", _PcePad6, sizeof(_PcePad6));
					PRINTF("6-Button Pad Initialize OK.\n");
				}
				else if (JOYPAD_GetConnectThreeButton())
				{
					initPadConfiguration3();
					if (INPUT_GetNumJoystick() == 0)
						initPad1ConfigurationKB3();
					CONFIG_Set("[APP INPUT] JoyPad Setting for 3Button", _PcePad3, sizeof(_PcePad3)); //v0.79追加
					PRINTF("3-Button Pad Initialize OK.\n");
				}
				else
				{
					initPadConfiguration2();
					if (INPUT_GetNumJoystick() == 0)
						initPad1ConfigurationKB2();
					CONFIG_Set("[APP INPUT] JoyPad Setting for 2Button", _PcePad2, sizeof(_PcePad2));
					PRINTF("2-Button Pad Initialize OK.\n");
				}
				
				//設定を反映させる
				INPUT_Deinit();
				INPUT_Init();
				APP_SetInputConfiguration();
				
				run_emulator(bRunNormalState);
			}
			break;

		case APPEVENT_AUDIO_SR96000: changeAudioSampleRate(96000); run_emulator(bRunNormalState); break;
		case APPEVENT_AUDIO_SR88200: changeAudioSampleRate(88200); run_emulator(bRunNormalState); break;
		case APPEVENT_AUDIO_SR64000: changeAudioSampleRate(64000); run_emulator(bRunNormalState); break;
		case APPEVENT_AUDIO_SR48000: changeAudioSampleRate(48000); run_emulator(bRunNormalState); break;
		case APPEVENT_AUDIO_SR44100: changeAudioSampleRate(44100); run_emulator(bRunNormalState); break;
		case APPEVENT_AUDIO_SR32000: changeAudioSampleRate(32000); run_emulator(bRunNormalState); break;
		case APPEVENT_AUDIO_SR22050: changeAudioSampleRate(22050); run_emulator(bRunNormalState); break;
		case APPEVENT_AUDIO_SR11025: changeAudioSampleRate(11025); run_emulator(bRunNormalState); break;

		case APPEVENT_AUDIO_SB3072: changeAudioBufferSize(3072); run_emulator(bRunNormalState); break;
		case APPEVENT_AUDIO_SB2560: changeAudioBufferSize(2560); run_emulator(bRunNormalState); break;
		case APPEVENT_AUDIO_SB2304: changeAudioBufferSize(2304); run_emulator(bRunNormalState); break;
		case APPEVENT_AUDIO_SB2176: changeAudioBufferSize(2176); run_emulator(bRunNormalState); break;
		case APPEVENT_AUDIO_SB2048: changeAudioBufferSize(2048); run_emulator(bRunNormalState); break;
		case APPEVENT_AUDIO_SB1792: changeAudioBufferSize(1792); run_emulator(bRunNormalState); break;
		case APPEVENT_AUDIO_SB1664: changeAudioBufferSize(1664); run_emulator(bRunNormalState); break;
		case APPEVENT_AUDIO_SB1536: changeAudioBufferSize(1536); run_emulator(bRunNormalState); break;
		case APPEVENT_AUDIO_SB1408: changeAudioBufferSize(1408); run_emulator(bRunNormalState); break;
		case APPEVENT_AUDIO_SB1280: changeAudioBufferSize(1280); run_emulator(bRunNormalState); break;
		case APPEVENT_AUDIO_SB1152: changeAudioBufferSize(1152); run_emulator(bRunNormalState); break;
		case APPEVENT_AUDIO_SB1024: changeAudioBufferSize(1024); run_emulator(bRunNormalState); break;

		//Kitao追加v1.15
		case APPEVENT_AUDIO_STEREO:
			MENU_CheckRadioItem(_AudioMenu, WM_AUDIO_STEREO, WM_AUDIO_MONO, WM_AUDIO_STEREO);
			_bAutoStereo = TRUE;
			CONFIG_Set("[APP SOUND] Auto Stereo", &_bAutoStereo, sizeof(_bAutoStereo));
			PRINTF("Set Auto Stereo (Gradius,Salamander,Parodius).");
			run_emulator(bRunNormalState);
			break;
		case APPEVENT_AUDIO_MONO:
			MENU_CheckRadioItem(_AudioMenu, WM_AUDIO_STEREO, WM_AUDIO_MONO, WM_AUDIO_MONO);
			_bAutoStereo = FALSE;
			CONFIG_Set("[APP SOUND] Auto Stereo", &_bAutoStereo, sizeof(_bAutoStereo));
			PRINTF("Set Mono Audio (Gradius,Salamander,Parodius).");
			run_emulator(bRunNormalState);
			break;

		//Kitao追加。サウンドボードの初期化方法を変更する。v1.03
		case APPEVENT_AUDIO_INI1:
			if (changeSoundInitializeType(1))
			{
				PRINTF("Soft Buffer & Soft Mixing Audio.");
				run_emulator(bRunNormalState);
			}
			break;
		case APPEVENT_AUDIO_INI2:
			if (changeSoundInitializeType(2))
			{
				PRINTF("Hard Buffer & Soft Mixing Audio.");
				run_emulator(bRunNormalState);
			}
			break;

		case APPEVENT_AUDIO_SETVOLUME:
			AUDIOCONFIG_Deinit();
			AUDIOCONFIG_Init(WINMAIN_GetHInstance(),
							 &_SoundApuMasterVolume,
							 &_SoundPsgVolume,
							 &_SoundAdpcmVolume,
							 &_SoundCdVolume);
			if (!MOUSE_IsConnected()) //v0.80追加。マウスをつないでいた場合はポーズしたまま。それ以外ならポーズを解除。
				run_emulator(bRunNormalState);
			break;
		//Kitao追加
		case APPEVENT_AUDIO_DEFAULTVOLUME: //デフォルトのボリュームバランス値に戻す。
			MAINBOARD_Pause(TRUE);
			WINMAIN_ShowCursor(TRUE);
			screenUpdate(); //メニューやダイアログ表示などで黒くなった部分を再描画。
			if (MessageBox(	WINMAIN_GetHwnd(),
							"Do you return \"Volume Balance\" to the default setting?    ",
							"Ootake", MB_YESNO) == IDYES)
			{
				AUDIOCONFIG_Deinit(); //音量バランスウィンドウが出ていた場合閉じる
				_SoundApuMasterVolume	= 65535;
				_SoundPsgVolume			= 65535;
				_SoundAdpcmVolume		= 65535;
				_SoundCdVolume 			= 65535;
				APU_SetVolume(_SoundApuMasterVolume);
				PSG_SetVolume(_SoundPsgVolume);
				ADPCM_SetVolume(_SoundAdpcmVolume);
				CDROM_SetCdVolume(_SoundCdVolume);
				screenUpdate(); //メニューやダイアログ表示などで黒くなった部分を再描画。
				MessageBox(	WINMAIN_GetHwnd(),
							"Ok. \"Volume Balance\" was returned to the default setting.    \n\nMASTER = 100   PSG = 100\nADPCM = 100   CDDA = 100",
							"Ootake", MB_OK);
				run_emulator(bRunNormalState);
			}
			break;
		//Kitao追加
		case APPEVENT_VOLUME_NORMAL:
			SetVolumeEffect(0);
			run_emulator(bRunNormalState);
			break;
		//Kitao追加
		case APPEVENT_VOLUME_3QUARTERS:
			SetVolumeEffect(1);
			run_emulator(bRunNormalState);
			break;
		//Kitao追加
		case APPEVENT_VOLUME_HALF:
			SetVolumeEffect(2);
			run_emulator(bRunNormalState);
			break;
		//Kitao追加
		case APPEVENT_VOLUME_QUARTER:
			SetVolumeEffect(3);
			run_emulator(bRunNormalState);
			break;
		//Kitao追加
		case APPEVENT_VOLUME_MUTE:
			SetVolumeEffect(4);
			run_emulator(bRunNormalState);
			break;

		//Kitao追加。音量を10段階で選択設定。レジューム設定にするとゲーム毎に音量が設定できる。v1.61
		case APPEVENT_VOLUME_DEFAULT: setWindowsVolume(-1); run_emulator(bRunNormalState); break;
		case APPEVENT_VOLUME_12: setWindowsVolume(120); run_emulator(bRunNormalState); break;
		case APPEVENT_VOLUME_11: setWindowsVolume(110);  run_emulator(bRunNormalState); break;
		case APPEVENT_VOLUME_10: setWindowsVolume(100); run_emulator(bRunNormalState); break;
		case APPEVENT_VOLUME_9:  setWindowsVolume(90);  run_emulator(bRunNormalState); break;
		case APPEVENT_VOLUME_8:	 setWindowsVolume(80);  run_emulator(bRunNormalState); break;
		case APPEVENT_VOLUME_7:	 setWindowsVolume(70);  run_emulator(bRunNormalState); break;
		case APPEVENT_VOLUME_6:	 setWindowsVolume(60);  run_emulator(bRunNormalState); break;
		case APPEVENT_VOLUME_5:	 setWindowsVolume(50);  run_emulator(bRunNormalState); break;
		case APPEVENT_VOLUME_4:	 setWindowsVolume(40);  run_emulator(bRunNormalState); break;
		case APPEVENT_VOLUME_3:	 setWindowsVolume(30);  run_emulator(bRunNormalState); break;
		case APPEVENT_VOLUME_2:	 setWindowsVolume(20);  run_emulator(bRunNormalState); break;
		case APPEVENT_VOLUME_1:	 setWindowsVolume(10);  run_emulator(bRunNormalState); break;
		case APPEVENT_VOLUME_STEP10: setVolumeStep(10); run_emulator(bRunNormalState); break; //v2.45。音量ステップ設定追加。
		case APPEVENT_VOLUME_STEP8: setVolumeStep(8); run_emulator(bRunNormalState); break;
		case APPEVENT_VOLUME_STEP6: setVolumeStep(6); run_emulator(bRunNormalState); break;
		case APPEVENT_VOLUME_STEP5: setVolumeStep(5); run_emulator(bRunNormalState); break;
		case APPEVENT_VOLUME_STEP4: setVolumeStep(4); run_emulator(bRunNormalState); break;
		case APPEVENT_VOLUME_STEP3: setVolumeStep(3); run_emulator(bRunNormalState); break;
		case APPEVENT_VOLUME_STEP2: setVolumeStep(2); run_emulator(bRunNormalState); break;
		case APPEVENT_VOLUME_STEP1: setVolumeStep(1); run_emulator(bRunNormalState); break;
		case APPEVENT_VOLUME_UP:
			VolumeUp();
			run_emulator(bRunNormalState);
			break;
		case APPEVENT_VOLUME_DOWN:
			VolumeDown();
			run_emulator(bRunNormalState);
			break;
		case APPEVENT_VOLUME_DETAILUP:
			VolumeDetailUp();
			run_emulator(bRunNormalState);
			break;
		case APPEVENT_VOLUME_DETAILDN:
			VolumeDetailDown();
			run_emulator(bRunNormalState);
			break;
		case APPEVENT_VOLUME_ATTENTION:
			screenUpdate(); //メニューやダイアログ表示などで黒くなった部分を再描画。
			MessageBox(	WINMAIN_GetHwnd(),
						"With Windows XP/2000/9x, when the volume is adjusted by this menu,    \n"
						"the volume of other Windows applications (that uses volume of WAVE    \n"
						"like Movie software etc.) changes, too. This method is a specification    \n"
						"because it doesn't lower tone quality. It automatically returns to    \n"
						"former volume when \"Ootake\" is closed.    \n"
						"Moreover, it is also possible to adjust the volume quickly with    \n"
						"[PageUp][PageDown] key.\n\n"
						"In Japanese language\n"
						"Windows XP/2000/9xでの利用の場合、このメニューで音量を調節すると、   \n"
						"同時に動かしている他のWindowsソフト（動画再生ソフトなど、WAVEの   \n"
						"音量を利用しているソフト）の音量も変化します。これは「音質を下げ   \n"
						"ないため」の仕様です。Ootakeを終了時に、自動的に元の音量に戻ります。   \n"
						"なお、キーボードの[PageUp][PageDown]キーで素早く音量調節することも   \n"
						"可能です。\n",
						"About this Volume(10-100) Menu", MB_OK);
			break;
		case APPEVENT_VOLUME_CONTROL: //Windowsのボリュームコントロールを立ち上げる。万が一のトラブルなどでWAVE音量がおかしくなった場合、再設定しやすいように。
			screenUpdate(); //メニューやダイアログ表示などで黒くなった部分を再描画。
			strcpy(fn, "sndvol32.exe");
			GetStartupInfo(&StartupInfo);
			StartupInfo.cb = sizeof(StartupInfo);
			StartupInfo.dwFlags = STARTF_USESHOWWINDOW;
			StartupInfo.wShowWindow = SW_SHOWNORMAL;
			if (CreateProcess(NULL, fn, NULL, NULL, FALSE, 0, NULL, NULL, &StartupInfo, &ProcessInfo))
			{
				Sleep(1000);//MessageBoxを実行する前にボリュームコントロールが立ち上がり完了するように１秒待つ。
				if (MessageBox(WINMAIN_GetHwnd(), "\"Windows Volume Control\" is safely started.    \n Is \"Ootake\" closed?", "Ootake", MB_YESNO) == IDYES)
					return FALSE; //Ootakeを終了
			}
			else
			{	//失敗した場合
				MessageBox(	WINMAIN_GetHwnd(),
							"Error. \"sndvol32.exe\" was not found.    \n"
							"(In Windows Vista/7, it doesn't exist originally.)    \n",
							"Ootake", MB_OK);
			}
			break;

		//Kitao追加。PSGの各チャンネルをミュート。v1.30
		case APPEVENT_VOLUME_MUTE1:
			if (PSG_GetMutePsgChannel(0))
				PSG_SetMutePsgChannel(0, FALSE);
			else
				PSG_SetMutePsgChannel(0, TRUE);
			updateMenu();
			APP_ReturnCaption();
			for (i=0; i<6; i++)
				if (!PSG_GetMutePsgChannel(i)) break;
			if (i < 6) //１チャンネルでもミュートが外れていれば。全ミュートならrunしない。
				run_emulator(bRunNormalState);
			break;
		case APPEVENT_VOLUME_MUTE2:
			if (PSG_GetMutePsgChannel(1))
				PSG_SetMutePsgChannel(1, FALSE);
			else
				PSG_SetMutePsgChannel(1, TRUE);
			updateMenu();
			APP_ReturnCaption();
			for (i=0; i<6; i++)
				if (!PSG_GetMutePsgChannel(i)) break;
			if (i < 6) //１チャンネルでもミュートが外れていれば。全ミュートならrunしない。
				run_emulator(bRunNormalState);
			break;
		case APPEVENT_VOLUME_MUTE3:
			if (PSG_GetMutePsgChannel(2))
				PSG_SetMutePsgChannel(2, FALSE);
			else
				PSG_SetMutePsgChannel(2, TRUE);
			updateMenu();
			APP_ReturnCaption();
			for (i=0; i<6; i++)
				if (!PSG_GetMutePsgChannel(i)) break;
			if (i < 6) //１チャンネルでもミュートが外れていれば。全ミュートならrunしない。
				run_emulator(bRunNormalState);
			break;
		case APPEVENT_VOLUME_MUTE4:
			if (PSG_GetMutePsgChannel(3))
				PSG_SetMutePsgChannel(3, FALSE);
			else
				PSG_SetMutePsgChannel(3, TRUE);
			updateMenu();
			APP_ReturnCaption();
			for (i=0; i<6; i++)
				if (!PSG_GetMutePsgChannel(i)) break;
			if (i < 6) //１チャンネルでもミュートが外れていれば。全ミュートならrunしない。
				run_emulator(bRunNormalState);
			break;
		case APPEVENT_VOLUME_MUTE5:
			if (PSG_GetMutePsgChannel(4))
				PSG_SetMutePsgChannel(4, FALSE);
			else
				PSG_SetMutePsgChannel(4, TRUE);
			updateMenu();
			APP_ReturnCaption();
			for (i=0; i<6; i++)
				if (!PSG_GetMutePsgChannel(i)) break;
			if (i < 6) //１チャンネルでもミュートが外れていれば。全ミュートならrunしない。
				run_emulator(bRunNormalState);
			break;
		case APPEVENT_VOLUME_MUTE6:
			if (PSG_GetMutePsgChannel(5))
				PSG_SetMutePsgChannel(5, FALSE);
			else
				PSG_SetMutePsgChannel(5, TRUE);
			updateMenu();
			APP_ReturnCaption();
			for (i=0; i<6; i++)
				if (!PSG_GetMutePsgChannel(i)) break;
			if (i < 6) //１チャンネルでもミュートが外れていれば。全ミュートならrunしない。
				run_emulator(bRunNormalState);
			break;
		case APPEVENT_VOLUME_MUTEA:
			for (i=0; i<6; i++)
				PSG_SetMutePsgChannel(i, TRUE);
			updateMenu();
			APP_ReturnCaption(); //Unmuteした後に、すぐ実行した場合に紛らわしくないように。
			//この後、たいがいMUTEを外す操作をするので、ここではrun_emulatorしない。
			break;
		case APPEVENT_VOLUME_MUTEU:
			for (i=0; i<6; i++)
				PSG_SetMutePsgChannel(i, FALSE);
			updateMenu();
			PRINTF("Unmute PSG Channel All.");
			run_emulator(bRunNormalState);
			break;

		//Kitao追加。CD-DAの再生テンポを微調整(人間の耳では言われないと気が付かないぐらいの微調整(こだわれば音質的には0(59.7Hz)がベスト)。あらゆるディスプレイ環境での画面と同期のために必要)。v2.32
		case APPEVENT_AUDIO_CDDAAUTO: //AUTOで調整。v2.43追加
			screenUpdate(); //メニューやダイアログ表示などで黒くなった部分を再描画。
			if (MessageBox( WINMAIN_GetHwnd(),
							"The timing value of the CD sound (CD-DA and ADPCM) is set    \n"
							"automatically to match the sound to the picture.\n"
							"The set completion is about 60 seconds. Please close other heavy    \n"
							"applications for an accurate measurement now. This automatic    \n"
							"setting starts when \"Yes\" is pushed.\n"
							"\n"
							"In Japanese language\n"
							"お使いのパソコン環境において、絵とCD音源（CD-DA,ADPCM）のズレ   \n"
							"が起こらないように、タイミング値を自動設定します。\n"
							"設定完了まで約60秒間掛かります。その間、Ootakeを処理落ちさせない   \n"
							"ために、他の重いアプリケーションは閉じてください。\"はい\"を押すと、自動   \n"
							"設定を開始します。",
							"Ootake", MB_YESNO) == IDYES)
			{
				//いったんメニューを隠す(メニューを隠す設定の場合)
				if ((_bHideMenu)||(_bFullScreen))
					if ((_DrawMethod != 2)||(!_bFullScreen)) //※DirectDrawのフルスクリーン時は、このタイミングでメニューを隠すと、次のダイアログ表示時にフォーカスを失ってしまうので隠さない。
					{
						run_emulator(TRUE);
						MAINBOARD_Pause(TRUE);
						WINMAIN_ShowCursor(TRUE);
					}
				vsf = SCREEN_GetVerticalScanFrequency();
				if (_bFullScreen)
					sprintf(fn, "%dx%d", _ScreenWidth,_ScreenHeight);
				else
					strcpy(fn, "Window");
				if ((vsf >= 5930)&&(vsf <=6025))
				{
					_CddaAdjustFineTune = vsf % 10;
					setCddaAdjust(vsf / 10);
					screenUpdate();
					if (SCREEN_GetSyncTo60HzScreen()) //V-Syncが行われている場合
					{
						sprintf(buf2,"The setting was completed. The frequency in \"%s\" mode of    \n"
									 "your display is [%d.%02dHz]. If you newly buy a display and a video    \n"
									 "card of your PC, execute this menu again.\n", fn, vsf/100, vsf%100);
						sprintf(buf3,"設定が完了しました。お使いのディスプレイの\"%sモード\"での周波   \n"
									 "数は【%d.%02dHz】です。パソコンのディスプレイやビデオカードを買い換えた   \n"
									 "場合は、再度このメニューを実行し、再設定してください。\n", fn, vsf/100, vsf%100);
					}
					else
					{
						sprintf(buf2,"The setting was completed. The frequency with Non-V-Sync mode    \n"
									 "is [%d.%02dHz corresponding]. If you newly buy a display of your PC,    \n"
									 "execute this menu again.\n", vsf/100, vsf%100);
						sprintf(buf3,"設定が完了しました。非V-Sync動作で、周波数は【%d.%02dHz相当】です。   \n"
									 "パソコンのディスプレイを買い換えた場合は、再度このメニューを実行し、   \n"
									 "再設定してください。\n", vsf/100, vsf%100);
					}
					sprintf(buf,"%s"
								"And, set it respectively in the window mode and the full-screen    \n"
								"mode. (The timing value changes by the display resolution. The    \n"
								"setting is individually preserved.)    \n"
								"\n"
								"In Japanese language\n"
								"%s"
								"また、ウィンドウモードとフルスクリーンモードでは、解像度によって最適   \n"
								"なタイミング値が異なりますので、それぞれ個別に設定してください。（設   \n"
								"定も個別に保存されます）", buf2, buf3);
					MessageBox( WINMAIN_GetHwnd(), buf, "Ootake", MB_OK);
				}
				else
				{
					PRINTF("V = %d.%02dHz", vsf/100, vsf%100);
					screenUpdate();
					if (SCREEN_GetSyncTo60HzScreen()) //V-Syncが行われている場合
					{
						sprintf(buf,"The frequency in \"%s\" mode of your display is [%d.%02dHz].\n"
									"Your display cannot do V-Sync at Ootake. (It might be possible in    \n"
									"the full-screen mode. Please try.)\n"
									"Or, it was not possible to measure it correctly. Please try again. If    \n"
									"this message is displayed again, remove the check on    \n"
									"\"Audio->V-Sync (59-61Hz)\" menu. And try again.    \n"
									"\n"
									"In Japanese language\n"
									"お使いのディスプレイの\"%sモード\"での周波数は【%d.%02dHz】です。\n"
									"このディスプレイは、OotakeのV-Syncに対応していません。（フルスクリー   \n"
									"ンモードでは対応している可能性がありますので試してみてください）\n"
									"もしくは、正しく計測ができなかった可能性があります。もう一度計測して   \n"
									"も、このメッセージが出た場合、\"Audio->V-Sync (59-61Hz)\"メニューの   \n"
									"チェックを外してから計測してみてください。", fn, vsf/100, vsf%100, fn, vsf/100, vsf%100);
					}
					else
					{
						sprintf(buf,"As a result of the measurement, the frequency with Non-V-Sync    \n"
									"mode is [%d.%02dHz corresponding].\n"
									"The power of your personal computer might be insufficient. Or, it    \n"
									"was not possible to measure it correctly.\n"
									"\n"
									"In Japanese language\n"
									"計測の結果、非V-Sync動作で、周波数は【%d.%02dHz相当】です。\n"
									"パソコンのパワーが足りないか、もしくは、正しく計測ができなかった   \n"
									"可能性があります。", vsf/100, vsf%100, vsf/100, vsf%100);
					}
					MessageBox( WINMAIN_GetHwnd(), buf, "Ootake", MB_OK);
				}
			}
			run_emulator(bRunNormalState);
			break;
		case APPEVENT_AUDIO_CDDA593: setCddaAdjust(593); run_emulator(bRunNormalState); break;
		case APPEVENT_AUDIO_CDDA594: setCddaAdjust(594); run_emulator(bRunNormalState); break;
		case APPEVENT_AUDIO_CDDA595: setCddaAdjust(595); run_emulator(bRunNormalState); break;
		case APPEVENT_AUDIO_CDDA596: setCddaAdjust(596); run_emulator(bRunNormalState); break;
		case APPEVENT_AUDIO_CDDA597: setCddaAdjust(597); run_emulator(bRunNormalState); break;
		case APPEVENT_AUDIO_CDDA598: setCddaAdjust(598); run_emulator(bRunNormalState); break;
		case APPEVENT_AUDIO_CDDA599: setCddaAdjust(599); run_emulator(bRunNormalState); break;
		case APPEVENT_AUDIO_CDDA600: setCddaAdjust(600); run_emulator(bRunNormalState); break;
		case APPEVENT_AUDIO_CDDA601: setCddaAdjust(601); run_emulator(bRunNormalState); break;
		case APPEVENT_AUDIO_CDDA602: setCddaAdjust(602); run_emulator(bRunNormalState); break;
		//case APPEVENT_AUDIO_CDDA603: setCddaAdjust(603); run_emulator(bRunNormalState); break; //※ここまで補正してしまうと音のキー変化が目立つので、多少の遅れより60.2Hzを利用したほうがいいのでカット。

		//Kitao追加。v2.32
		case APPEVENT_AUDIO_CDDAP000:
			_CddaAdjustFineTune = 0;
			setCddaAdjust(getCddaAdjust() / 10); //getCddaAdjust()で現在の画面モードでの_CddaAdjustを得る
			run_emulator(bRunNormalState);
			break;
		case APPEVENT_AUDIO_CDDAP005:
			_CddaAdjustFineTune = 5;
			setCddaAdjust(getCddaAdjust() / 10); //getCddaAdjust()で現在の画面モードでの_CddaAdjustを得る
			run_emulator(bRunNormalState);
			break;

		//Kitao追加。CD-DA再生開始時に、どのぐらい表示遅延を考慮するかを設定する。v2.33
		case APPEVENT_AUDIO_DELAYFRAME0: setCddaDelayFrame(0); run_emulator(bRunNormalState); break;
		case APPEVENT_AUDIO_DELAYFRAME1: setCddaDelayFrame(1); run_emulator(bRunNormalState); break;
		case APPEVENT_AUDIO_DELAYFRAME2: setCddaDelayFrame(2); run_emulator(bRunNormalState); break;
		case APPEVENT_AUDIO_DELAYFRAME3: setCddaDelayFrame(3); run_emulator(bRunNormalState); break;
		case APPEVENT_AUDIO_DELAYFRAME4: setCddaDelayFrame(4); run_emulator(bRunNormalState); break;

		//Kitao追加
		case APPEVENT_AUDIO_NORMALBUFFER:
			_BigSoundBuffer = 0;
			changeAudioBufferSize(_SoundBufferSize);
			run_emulator(bRunNormalState);
			break;
		//Kitao追加。音楽を高音質でじっくり聴きたいときのためのモード。次回起動時にはノーマルに戻る。
		case APPEVENT_AUDIO_BIGBUFFER:
			_BigSoundBuffer = 1;
			changeAudioBufferSize(_SoundBufferSize);
			run_emulator(bRunNormalState);
			break;
		//Kitao追加。大きく音が遅れるが、音楽を高音質でじっくり聴きたいときのためのモード。次回起動時にはノーマルに戻る。
		case APPEVENT_AUDIO_MOSTBUFFER:
			_BigSoundBuffer = 2;
			changeAudioBufferSize(_SoundBufferSize);
			run_emulator(bRunNormalState);
			break;

		//Kitao追加。PSGの再現性の設定。再現性を高めると処理はそのぶん重くなる。
		case APPEVENT_AUDIO_HQPSG1:
			MENU_CheckRadioItem(_AudioSubMenu, WM_AUDIO_HQPSG1, WM_AUDIO_HQPSG3, WM_AUDIO_HQPSG1);
			APP_SetPsgQuality(4);
			PRINTF("Set High Quality PSG Sound.");
			run_emulator(bRunNormalState);
			break;
		case APPEVENT_AUDIO_HQPSG2:
			MENU_CheckRadioItem(_AudioSubMenu, WM_AUDIO_HQPSG1, WM_AUDIO_HQPSG3, WM_AUDIO_HQPSG2);
			APP_SetPsgQuality(2);
			PRINTF("Set A Little Light PSG Sound.");
			run_emulator(bRunNormalState);
			break;
		case APPEVENT_AUDIO_HQPSG3:
			MENU_CheckRadioItem(_AudioSubMenu, WM_AUDIO_HQPSG1, WM_AUDIO_HQPSG3, WM_AUDIO_HQPSG3);
			APP_SetPsgQuality(1);
			PRINTF("Set Light PSG Sound.");
			run_emulator(bRunNormalState);
			break;

		//Kitao追加。v1.50
		case APPEVENT_INFO_SHOWFPS:
			if (_bShowFPS)
			{
				_bShowFPS = FALSE;
				APP_ReturnCaption(); //FPS表示してすぐに消したときのために必要。
			}
			else
			{
				_bShowFPS = TRUE;
				PRINTF("Show FPS (left-bottom).");
			}
			MENU_CheckItem(_InfoMenu, WM_INFO_SHOWFPS, _bShowFPS);
			run_emulator(bRunNormalState);
			break;
		//Kitao追加。v1.21
		case APPEVENT_INFO_TESTDELAY:
			sprintf(buf, "\"Processing Delay\" test result.  \"処理落ち\"テスト結果。\n\n");
			if (MAINBOARD_GetProcessingDelay() <= 3)
				strcat(buf,"<<< Good");
			else
				strcat(buf,"<<< Bad");
			sprintf(buf, "%s Condition [ %d ] ! >>>\n\n"
						 " There was \"Processing Delay\" of\n"
						 " [ %d ] frame(s) in these 5 seconds.\n"
						 "   ^ Ootake operates comfortably if this is zero[ 0 ] or extremely near ( 1,2 or 3 ).      \n"
						 "     If this value is large, trouble might happen in CD-Game.\n"
						 "     ここの値（フレーム遅れ）が大きい場合、ＣＤゲームで不具合が起こる可能性があります。      \n\n", buf, MAINBOARD_GetProcessingDelay(), MAINBOARD_GetProcessingDelay());
			strcat(buf, "The \"FullScreen\" mode (F12 key) is fast in some measure.    \n"
						"If you test again, make the emulator run for 5 seconds.    \n"
						"再テストするときは、エミュレーターを５秒間走らせてから行ってください。    ");
			sprintf(fn, "Ootake - [ %d ] frame(s) delay on your personal computer.", MAINBOARD_GetProcessingDelay());
			MessageBox(	WINMAIN_GetHwnd(), buf, fn, MB_OK);
			run_emulator(bRunNormalState); //続けて再テストするときのためにエミュレータを走らせる
			break;
		//Kitao追加
		case APPEVENT_INFO_MANUENGLISH:
			strcpy(fn, _AppPath);
			strcat(fn, "Manual-E.txt");
			ShellExecute(NULL, "open", fn, NULL, NULL, SW_SHOW);
			break;
		//Kitao追加
		case APPEVENT_INFO_MANUJAPANESE:
			strcpy(fn, _AppPath);
			strcat(fn, "Manual-J.txt");
			ShellExecute(NULL, "open", fn, NULL, NULL, SW_SHOW);
			break;
		//Kitao追加
		case APPEVENT_INFO_README:
			strcpy(fn, _AppPath);
			strcat(fn, "Readme.txt");
			ShellExecute(NULL, "open", fn, NULL, NULL, SW_SHOW);
			break;

		//Kitao追加
		case APPEVENT_INFO_HOMEPAGE:
			strcpy(fn, _AppPath);
			strcat(fn, "Homepage.html");
			ShellExecute(NULL, "open", fn, NULL, NULL, SW_SHOW);
			break;

		//Kitao追加
		case APPEVENT_INFO_VERSION:
			MAINBOARD_Pause(TRUE);
			WINMAIN_ShowCursor(TRUE);
			screenUpdate(); //メニューやダイアログ表示などで黒くなった部分を再描画。
			sprintf(buf, "\"Ootake\" Version %s\n\nCopyright(C)2006-2012 Kitao Nakamura.   \n"
						 "Copyright(C)2004-2005 Ki.(PC2E code)\n\nThis program is not guaranteed.\n"
						 "Please use it by the self-responsibility.\n"
						 "Business use is prohibited.\n\n"
						 "In Japanese language\n"
						 "このプログラムは無保証です。\n"
						 "各自の責任にてご利用ください。\n"
						 "商的な利用は禁じます。", _SoftVersion);
			MessageBox(WINMAIN_GetHwnd(), buf, "About \"Ootake\"", MB_OK);
			run_emulator(bRunNormalState);
			break;

		//Kitao追加。CD-ROMのデータをあらかじめ読み込んでおき、アクセス動作を高速化させる。v1.00。シャーロックホームズ・イース４などで必要。
		case APPEVENT_CD_INSTALL:
			MAINBOARD_Pause(TRUE);
			WINMAIN_ShowCursor(TRUE);
			screenUpdate(); //メニューやダイアログ表示などで黒くなった部分を再描画。
			if (_bCDGame)
			{
				strcpy(fn, _AppPath);
				strcat(fn, "install");
				CreateDirectory(fn, NULL);//installディレクトリがない場合作る
				strcat(fn, "\\");
				strcat(fn, TOCDB_GetGameTitle());
				CreateDirectory(fn, NULL);//インストールするゲームのディレクトリがない場合作る
				
				CDINSTALL_Init(WINMAIN_GetHInstance(), FALSE);
				if (CDINSTALL_GetResult() == 1)
				{
					PRINTF("Install Completed. ( CD Access High-Speed )");
					screenUpdate(); //メニューやダイアログ表示などで黒くなった部分を再描画。PRINTFもすぐ表示。
					run_emulator(bRunNormalState);
				}
				else //-1
				{
					PRINTF("Abort \"CD Install\".");
					screenUpdate(); //メニューやダイアログ表示などで黒くなった部分を再描画。PRINTFもすぐ表示。
				}
			}
			else //Huカードを遊んでいる場合
				MessageBox(WINMAIN_GetHwnd(), "The game of \"Hu-CARD\" is played now.\n"
											  "When the \"CD-ROM game\" is played, it is possible to install it.    ", "Ootake", MB_OK);
			break;

		case APPEVENT_CD_FULLINSTALL: //v2.24追加。音楽トラックも全てインストール。
			MAINBOARD_Pause(TRUE);
			WINMAIN_ShowCursor(TRUE);
			screenUpdate(); //メニューやダイアログ表示などで黒くなった部分を再描画。
			if (_bCDGame)
			{
				strcpy(fn, _AppPath);
				strcat(fn, "install");
				CreateDirectory(fn, NULL);//installディレクトリがない場合作る
				strcat(fn, "\\");
				strcat(fn, TOCDB_GetGameTitle());
				CreateDirectory(fn, NULL);//インストールするゲームのディレクトリがない場合作る
				
				CDINSTALL_Init(WINMAIN_GetHInstance(), TRUE); //TRUE…フルインストール
				if (CDINSTALL_GetResult() == 1)
				{
					if (_bCueFile) //すでにフルインストール済みでcue起動だった場合
					{
						if (CDIF_GetBadInstalled())
							PRINTF("Maybe Bad Installed. Please FullInstall again.");
						else
							PRINTF("FullInstall Completed. Fcd: %s", TOCDB_GetGameTitle());
					}
					else //通常
					{
						CDROM_SetInstallCueFileName(_CueFilePathName);
						_bCueFile = TRUE; //CUEファイルから起動した状態に切り替える
						AddRecentRom(); //CUEファイルを履歴に追加する
						CDROM_UpdateCDInstall(); //CDインストールの状況を更新する。v2.35
						PRINTF("FullInstall Completed. Fcd: %s", TOCDB_GetGameTitle());
					}
					screenUpdate(); //メニューやダイアログ表示などで黒くなった部分を再描画。PRINTFもすぐ表示。
					run_emulator(bRunNormalState);
				}
				else //-1
				{
					PRINTF("Abort \"CD FullInstall\".");
					screenUpdate(); //メニューやダイアログ表示などで黒くなった部分を再描画。PRINTFもすぐ表示。
				}
			}
			else //Huカードを遊んでいる場合
				MessageBox(WINMAIN_GetHwnd(), "The game of \"Hu-CARD\" is played now.\n"
											  "When the \"CD-ROM game\" is played, it is possible to install it.    ", "Ootake", MB_OK);
			break;

		case APPEVENT_CD_UNINSTALL: //Kitao追加。CDアンインストール
			MAINBOARD_Pause(TRUE);
			WINMAIN_ShowCursor(TRUE);
			screenUpdate(); //メニューやダイアログ表示などで黒くなった部分を再描画。
			if (_bCDGame)
			{
				strcpy(fn, _AppPath);
				strcat(fn, "install\\");
				strcat(fn, TOCDB_GetGameTitle());
				if (PathFileExists(fn))
				{	//そのゲームのインストールディレクトリがあれば、アンインストールする。
					sprintf(buf, "Uninstall \"%s\" Ok?    ", TOCDB_GetGameTitle());
					if (MessageBox(WINMAIN_GetHwnd(), buf, "Ootake", MB_YESNO) == IDYES)
					{
						if (MessageBox(WINMAIN_GetHwnd(), "\"Image File\" reserved?    \nイメージファイルを残しますか？    \n", "Ootake", MB_YESNO) == IDNO)
						{
							//アンインストール処理
							CDROM_CDUninstall();
							RemoveDirectory(fn);
							sprintf(buf, "\"%s\" Uninstall Completed.    ", TOCDB_GetGameTitle());
							MessageBox(WINMAIN_GetHwnd(), buf, "Ootake", MB_OK);
							//cueファイルから起動中にアンインストールした場合、リセットする。
							if (_bCueFile)
							{
								_bCueFile = FALSE;
								if (!app_cd_change(FALSE)) //run_emulator()も行われる。TRUE=同じゲームを開いたときにレジュームを無効にしない
									return FALSE; //システムカードのイメージ読み込み失敗のため、強制終了。
							}
							else
							{
								CDROM_UpdateCDInstall(); //CDインストールの状況を更新する。v2.35
								PRINTF("\"%s\" Uninstall Completed.", TOCDB_GetGameTitle());
							}
						}
						else
							PRINTF("Abort \"CD Uninstall\".");
					}
					else
						PRINTF("Abort \"CD Uninstall\".");
				}
				else //そのゲームのインストールディレクトリがない場合
				{
					sprintf(buf, "\"%s\" is not installed.    ",TOCDB_GetGameTitle());
					MessageBox(WINMAIN_GetHwnd(), buf, "Ootake", MB_OK);
				}
				screenUpdate(); //メニューやダイアログ表示などで黒くなった部分を再描画。PRINTFもすぐ表示。
				run_emulator(bRunNormalState);
			}
			else //Huカードを遊んでいる場合
				MessageBox(WINMAIN_GetHwnd(), "The game of \"Hu-CARD\" is played now.\n"
											  "When the \"CD-ROM game\" is played, it is possible to install it.    ", "Ootake", MB_OK);
			break;

		case APPEVENT_CD_OPENINSTALL: //Kitao追加。installフォルダをマイコンピュータで開く
			APP_OpenInstallFolder();
			break;

		//Kitao追加。フルインストールしたCDイメージから起動する。v2.24
		case APPEVENT_CD_PLAYINSTALL:
			/*			
			//CUEマウントtest用
			CDROM_SetInstallCueFileName(_CueFilePathName);
			_bCueFile = TRUE;
			CDIF_SelectDevice(0);
			run_emulator(bRunNormalState);
			break;
			*/
			MAINBOARD_Pause(TRUE); //ショートカットキーを押してここが呼び出されたときのために必要。
			WINMAIN_ShowCursor(TRUE); //ショートカットキーを押してここが呼び出されたときのために必要。
			if (!FolderDialogSub()) //フォルダーダイアログを表示
					return FALSE; //システムカードのイメージ読み込み失敗のため、強制終了。
			break;

		//Kitao追加。システムカードのイメージファイルを指定する
		case APPEVENT_CD_SETSYSCARD:
			MAINBOARD_Pause(TRUE);
			WINMAIN_ShowCursor(TRUE);
			MAINBOARD_ScreenClear();//画面をクリア。ゲームを変えるときはこうするとわかりやすい＆他のウィンドウが重なって画面が消えたあとにリセットした場合でも綺麗に再開できる。
			
			strcpy(srcFilePathName, "");
			strcpy(dstFilePathName, "");
			strcpy(fn, _GameFilePathName); //元のゲームの_GameFilePathNameをいったん退避
			if (file_dialog(2, WINMAIN_GetHwnd(), srcFilePathName, fn))
			{
				strcpy(dstFilePathName, _AppPath);
				strcat(dstFilePathName, "SYSCARD.pce");
				SetFileAttributes(dstFilePathName, FILE_ATTRIBUTE_NORMAL); //読み込み専用ファイルにしてあった場合解除
				if (CopyFile(srcFilePathName, dstFilePathName, FALSE)) //実行フォルダ内へコピー
				{
					SetFileAttributes(dstFilePathName, FILE_ATTRIBUTE_NORMAL); //読み込み専用ファイルにしてあった場合解除
					//現在開いているゲームの記録・再生を終了＆バックアップラムを保存
					end_playrecord();
					end_recording();
					save_resume();
					save_bram();
					//新たにシステムカードを起動する。
					_bCDGame = TRUE;
					_OtherSysCard = 0;
					_bHesFile = FALSE;
					strcpy(_OpenFilePathName, dstFilePathName);
					strcpy(_GameFilePathName, fn);//Kitao追加
					if (app_game_reopen(FALSE) == FALSE) //レジュームを無効にして起動する
						return FALSE; //開けないファイルを開こうとして失敗したため、強制終了。
				}
			}
			break;

		//Kitao追加。デフォルトとは別のシステムカードのイメージファイルを指定する。v1.49
		case APPEVENT_CD_SETSYSCARD1:
			MAINBOARD_Pause(TRUE);
			WINMAIN_ShowCursor(TRUE);
			MAINBOARD_ScreenClear();//画面をクリア。ゲームを変えるときはこうするとわかりやすい＆他のウィンドウが重なって画面が消えたあとにリセットした場合でも綺麗に再開できる。
			
			strcpy(fn, _GameFilePathName); //元のゲームの_GameFilePathNameをいったん退避
			if (file_dialog(3, WINMAIN_GetHwnd(), _OpenFilePathName, fn)) //Kitao更新
			{
				//現在開いているゲームの記録・再生を終了＆バックアップラムを保存
				end_playrecord();
				end_recording();
				save_resume();
				save_bram();
				//新たにシステムカードを起動する。
				_bCDGame = TRUE;
				_OtherSysCard = 1;
				_bHesFile = FALSE;
				strcpy(_GameFilePathName, fn);//Kitao追加
				if (app_game_reopen(FALSE) == FALSE) //レジュームを無効にして起動する
					return FALSE; //開けないファイルを開こうとして失敗したため、強制終了。
			}
			break;
		case APPEVENT_CD_SETSYSCARD2:
			MAINBOARD_Pause(TRUE);
			WINMAIN_ShowCursor(TRUE);
			MAINBOARD_ScreenClear();//画面をクリア。ゲームを変えるときはこうするとわかりやすい＆他のウィンドウが重なって画面が消えたあとにリセットした場合でも綺麗に再開できる。
			
			strcpy(fn, _GameFilePathName); //元のゲームの_GameFilePathNameをいったん退避
			if (file_dialog(4, WINMAIN_GetHwnd(), _OpenFilePathName, fn)) //Kitao更新
			{
				//現在開いているゲームの記録・再生を終了＆バックアップラムを保存
				end_playrecord();
				end_recording();
				save_resume();
				save_bram();
				//新たにシステムカードを起動する。
				_bCDGame = TRUE;
				_OtherSysCard = 2;
				_bHesFile = FALSE;
				strcpy(_GameFilePathName, fn);//Kitao追加
				if (app_game_reopen(FALSE) == FALSE) //レジュームを無効にして起動する
					return FALSE; //開けないファイルを開こうとして失敗したため、強制終了。
			}
			break;

		//Kitao追加。v2.07
		case APPEVENT_CD_JUUOUKI:
			screenUpdate(); //メニューやダイアログ表示などで黒くなった部分を再描画。
			MessageBox(	WINMAIN_GetHwnd(),
						"\"Juuouki (J)\" doesn't operate in \"SUPER SYSTEM CARD (v3.00)\".    \n\n"
						"Therefore, at \"CD-ROM->Set Other SystemCard image 1\" menu,\n"
						"set the ROM-image of \"CD-ROM2 SYSTEM CARD v2.1(or less)\".    \n\n"
						"Then, the game can be played without the stopping problem.",
						"Ootake", MB_OK);
			break;

		//Kitao追加。v1.49
		case APPEVENT_CD_HELP:
			screenUpdate(); //メニューやダイアログ表示などで黒くなった部分を再描画。
			MessageBox(	WINMAIN_GetHwnd(),
						"First of all, at \"CD-ROM->Set SystemCard image\" menu,\n"
						"set the ROM-image of \"SUPER CD-ROM2 SYSTEM CARD Ver.3.00\".    \n\n"
						"Then, the game of CD-ROM can be played.",
						"Ootake", MB_OK);
			break;

		//Kitao追加
		case APPEVENT_DEVICE_CD0:
			MAINBOARD_Pause(TRUE); WINMAIN_ShowCursor(TRUE);
			MAINBOARD_ScreenClear();//画面をクリア
			sprintf(buf, "After the CD drive is changed,  \"Ootake\" is reset.   \nIs the CD drive changed to [ %c : ] ?", (int)(CDIF_GetDriveLetters(0)));
			if (MessageBox(WINMAIN_GetHwnd(), buf, "CD Drive Change & Reset", MB_YESNO) == IDYES)
			{
				MENU_CheckRadioItem(_CDROMMenu, WM_DEVICE_CD0, WM_DEVICE_CD9, WM_DEVICE_CD0);
				_CurrentCdDrive = 0;
				CONFIG_Set("[APP DEVICE] Current CD Drive", &_CurrentCdDrive, sizeof(_CurrentCdDrive));
				_bCueFile = FALSE; //v2.26追加
				if (!app_cd_change(FALSE)) //run_emulator()も行われる
					return FALSE; //システムカードのイメージ読み込み失敗のため、強制終了。
			}
			else
				run_emulator(bRunNormalState);
			break;
		case APPEVENT_DEVICE_CD1:
			MAINBOARD_Pause(TRUE); WINMAIN_ShowCursor(TRUE);
			MAINBOARD_ScreenClear();//画面をクリア
			sprintf(buf, "After the CD drive is changed,  \"Ootake\" is reset.   \nIs the CD drive changed to [ %c : ] ?", (int)(CDIF_GetDriveLetters(1)));
			if (MessageBox(WINMAIN_GetHwnd(), buf, "CD Drive Change & Reset", MB_YESNO) == IDYES)
			{
				MENU_CheckRadioItem(_CDROMMenu, WM_DEVICE_CD0, WM_DEVICE_CD9, WM_DEVICE_CD1);
				_CurrentCdDrive = 1;
				CONFIG_Set("[APP DEVICE] Current CD Drive", &_CurrentCdDrive, sizeof(_CurrentCdDrive));
				_bCueFile = FALSE; //v2.26追加
				if (!app_cd_change(FALSE)) //run_emulator()も行われる
					return FALSE; //システムカードのイメージ読み込み失敗のため、強制終了。
			}
			else
				run_emulator(bRunNormalState);
			break;
		case APPEVENT_DEVICE_CD2:
			MAINBOARD_Pause(TRUE); WINMAIN_ShowCursor(TRUE);
			MAINBOARD_ScreenClear();//画面をクリア
			sprintf(buf, "After the CD drive is changed,  \"Ootake\" is reset.   \nIs the CD drive changed to [ %c : ] ?", (int)(CDIF_GetDriveLetters(2)));
			if (MessageBox(WINMAIN_GetHwnd(), buf, "CD Drive Change & Reset", MB_YESNO) == IDYES)
			{
				MENU_CheckRadioItem(_CDROMMenu, WM_DEVICE_CD0, WM_DEVICE_CD9, WM_DEVICE_CD2);
				_CurrentCdDrive = 2;
				CONFIG_Set("[APP DEVICE] Current CD Drive", &_CurrentCdDrive, sizeof(_CurrentCdDrive));
				_bCueFile = FALSE; //v2.26追加
				if (!app_cd_change(FALSE)) //run_emulator()も行われる
					return FALSE; //システムカードのイメージ読み込み失敗のため、強制終了。
			}
			else
				run_emulator(bRunNormalState);
			break;
		case APPEVENT_DEVICE_CD3:
			MAINBOARD_Pause(TRUE); WINMAIN_ShowCursor(TRUE);
			MAINBOARD_ScreenClear();//画面をクリア
			sprintf(buf, "After the CD drive is changed,  \"Ootake\" is reset.   \nIs the CD drive changed to [ %c : ] ?", (int)(CDIF_GetDriveLetters(3)));
			if (MessageBox(WINMAIN_GetHwnd(), buf, "CD Drive Change & Reset", MB_YESNO) == IDYES)
			{
				MENU_CheckRadioItem(_CDROMMenu, WM_DEVICE_CD0, WM_DEVICE_CD9, WM_DEVICE_CD3);
				_CurrentCdDrive = 3;
				CONFIG_Set("[APP DEVICE] Current CD Drive", &_CurrentCdDrive, sizeof(_CurrentCdDrive));
				_bCueFile = FALSE; //v2.26追加
				if (!app_cd_change(FALSE)) //run_emulator()も行われる
					return FALSE; //システムカードのイメージ読み込み失敗のため、強制終了。
			}
			else
				run_emulator(bRunNormalState);
			break;
		case APPEVENT_DEVICE_CD4:
			MAINBOARD_Pause(TRUE); WINMAIN_ShowCursor(TRUE);
			MAINBOARD_ScreenClear();//画面をクリア
			sprintf(buf, "After the CD drive is changed,  \"Ootake\" is reset.   \nIs the CD drive changed to [ %c : ] ?", (int)(CDIF_GetDriveLetters(4)));
			if (MessageBox(WINMAIN_GetHwnd(), buf, "CD Drive Change & Reset", MB_YESNO) == IDYES)
			{
				MENU_CheckRadioItem(_CDROMMenu, WM_DEVICE_CD0, WM_DEVICE_CD9, WM_DEVICE_CD4);
				_CurrentCdDrive = 4;
				CONFIG_Set("[APP DEVICE] Current CD Drive", &_CurrentCdDrive, sizeof(_CurrentCdDrive));
				_bCueFile = FALSE; //v2.26追加
				if (!app_cd_change(FALSE)) //run_emulator()も行われる
					return FALSE; //システムカードのイメージ読み込み失敗のため、強制終了。
			}
			else
				run_emulator(bRunNormalState);
			break;
		case APPEVENT_DEVICE_CD5:
			MAINBOARD_Pause(TRUE); WINMAIN_ShowCursor(TRUE);
			MAINBOARD_ScreenClear();//画面をクリア
			sprintf(buf, "After the CD drive is changed,  \"Ootake\" is reset.   \nIs the CD drive changed to [ %c : ] ?", (int)(CDIF_GetDriveLetters(5)));
			if (MessageBox(WINMAIN_GetHwnd(), buf, "CD Drive Change & Reset", MB_YESNO) == IDYES)
			{
				MENU_CheckRadioItem(_CDROMMenu, WM_DEVICE_CD0, WM_DEVICE_CD9, WM_DEVICE_CD5);
				_CurrentCdDrive = 5;
				CONFIG_Set("[APP DEVICE] Current CD Drive", &_CurrentCdDrive, sizeof(_CurrentCdDrive));
				_bCueFile = FALSE; //v2.26追加
				if (!app_cd_change(FALSE)) //run_emulator()も行われる
					return FALSE; //システムカードのイメージ読み込み失敗のため、強制終了。
			}
			else
				run_emulator(bRunNormalState);
			break;
		case APPEVENT_DEVICE_CD6:
			MAINBOARD_Pause(TRUE); WINMAIN_ShowCursor(TRUE);
			MAINBOARD_ScreenClear();//画面をクリア
			sprintf(buf, "After the CD drive is changed,  \"Ootake\" is reset.   \nIs the CD drive changed to [ %c : ] ?", (int)(CDIF_GetDriveLetters(6)));
			if (MessageBox(WINMAIN_GetHwnd(), buf, "CD Drive Change & Reset", MB_YESNO) == IDYES)
			{
				MENU_CheckRadioItem(_CDROMMenu, WM_DEVICE_CD0, WM_DEVICE_CD9, WM_DEVICE_CD6);
				_CurrentCdDrive = 6;
				CONFIG_Set("[APP DEVICE] Current CD Drive", &_CurrentCdDrive, sizeof(_CurrentCdDrive));
				_bCueFile = FALSE; //v2.26追加
				if (!app_cd_change(FALSE)) //run_emulator()も行われる
					return FALSE; //システムカードのイメージ読み込み失敗のため、強制終了。
			}
			else
				run_emulator(bRunNormalState);
			break;
		case APPEVENT_DEVICE_CD7:
			MAINBOARD_Pause(TRUE); WINMAIN_ShowCursor(TRUE);
			MAINBOARD_ScreenClear();//画面をクリア
			sprintf(buf, "After the CD drive is changed,  \"Ootake\" is reset.   \nIs the CD drive changed to [ %c : ] ?", (int)(CDIF_GetDriveLetters(7)));
			if (MessageBox(WINMAIN_GetHwnd(), buf, "CD Drive Change & Reset", MB_YESNO) == IDYES)
			{
				MENU_CheckRadioItem(_CDROMMenu, WM_DEVICE_CD0, WM_DEVICE_CD9, WM_DEVICE_CD7);
				_CurrentCdDrive = 7;
				CONFIG_Set("[APP DEVICE] Current CD Drive", &_CurrentCdDrive, sizeof(_CurrentCdDrive));
				_bCueFile = FALSE; //v2.26追加
				if (!app_cd_change(FALSE)) //run_emulator()も行われる
					return FALSE; //システムカードのイメージ読み込み失敗のため、強制終了。
			}
			else
				run_emulator(bRunNormalState);
			break;
		case APPEVENT_DEVICE_CD8:
			MAINBOARD_Pause(TRUE); WINMAIN_ShowCursor(TRUE);
			MAINBOARD_ScreenClear();//画面をクリア
			sprintf(buf, "After the CD drive is changed,  \"Ootake\" is reset.   \nIs the CD drive changed to [ %c : ] ?", (int)(CDIF_GetDriveLetters(8)));
			if (MessageBox(WINMAIN_GetHwnd(), buf, "CD Drive Change & Reset", MB_YESNO) == IDYES)
			{
				MENU_CheckRadioItem(_CDROMMenu, WM_DEVICE_CD0, WM_DEVICE_CD9, WM_DEVICE_CD8);
				_CurrentCdDrive = 8;
				CONFIG_Set("[APP DEVICE] Current CD Drive", &_CurrentCdDrive, sizeof(_CurrentCdDrive));
				_bCueFile = FALSE; //v2.26追加
				if (!app_cd_change(FALSE)) //run_emulator()も行われる
					return FALSE; //システムカードのイメージ読み込み失敗のため、強制終了。
			}
			else
				run_emulator(bRunNormalState);
			break;
		case APPEVENT_DEVICE_CD9:
			MAINBOARD_Pause(TRUE); WINMAIN_ShowCursor(TRUE);
			MAINBOARD_ScreenClear();//画面をクリア
			sprintf(buf, "After the CD drive is changed,  \"Ootake\" is reset.   \nIs the CD drive changed to [ %c : ] ?", (int)(CDIF_GetDriveLetters(9)));
			if (MessageBox(WINMAIN_GetHwnd(), buf, "CD Drive Change & Reset", MB_YESNO) == IDYES)
			{
				MENU_CheckRadioItem(_CDROMMenu, WM_DEVICE_CD0, WM_DEVICE_CD9, WM_DEVICE_CD9);
				_CurrentCdDrive = 9;
				CONFIG_Set("[APP DEVICE] Current CD Drive", &_CurrentCdDrive, sizeof(_CurrentCdDrive));
				_bCueFile = FALSE; //v2.26追加
				if (!app_cd_change(FALSE)) //run_emulator()も行われる
					return FALSE; //システムカードのイメージ読み込み失敗のため、強制終了。
			}
			else
				run_emulator(bRunNormalState);
			break;

		//Kitao追加。起動時の"Fast CD"を設定する。
		case APPEVENT_STARTFASTCD_ON:
			MENU_CheckRadioItem(_CDROMMenu, WM_STARTFASTCD_ON, WM_STARTFASTCD_PRE, WM_STARTFASTCD_ON);
			_StartFastCD = 1;
			CONFIG_Set("[APP DEVICE] Start FastCD", &_StartFastCD, sizeof(_StartFastCD));
			MENU_CheckItem(_SpeedMenu, WM_SPEED_FASTCD, TRUE);
			PRINTF("Set FastCD Speed.");
			CDROM_SetFastCD(TRUE);
			_bFastCD = TRUE;
			CONFIG_Set("[APP DEVICE] FastCD", &_bFastCD, sizeof(_bFastCD));
			run_emulator(bRunNormalState);
			break;
		case APPEVENT_STARTFASTCD_OFF:
			MENU_CheckRadioItem(_CDROMMenu, WM_STARTFASTCD_ON, WM_STARTFASTCD_PRE, WM_STARTFASTCD_OFF);
			_StartFastCD = 0;
			CONFIG_Set("[APP DEVICE] Start FastCD", &_StartFastCD, sizeof(_StartFastCD));
			MENU_CheckItem(_SpeedMenu, WM_SPEED_FASTCD, FALSE);
			PRINTF("Set RealCD Speed.");
			CDROM_SetFastCD(FALSE);
			_bFastCD = FALSE;
			CONFIG_Set("[APP DEVICE] FastCD", &_bFastCD, sizeof(_bFastCD));
			run_emulator(bRunNormalState);
			break;
		case APPEVENT_STARTFASTCD_PRE:
			MENU_CheckRadioItem(_CDROMMenu, WM_STARTFASTCD_ON, WM_STARTFASTCD_PRE, WM_STARTFASTCD_PRE);
			_StartFastCD = 2;
			CONFIG_Set("[APP DEVICE] Start FastCD", &_StartFastCD, sizeof(_StartFastCD));
			CONFIG_Set("[APP DEVICE] FastCD", &_bFastCD, sizeof(_bFastCD));
			PRINTF("Memorize FastCD setting.");
			run_emulator(bRunNormalState);
			break;

		//Kitao追加。起動時の"Fast Seek"を設定する。
		case APPEVENT_STARTFASTSEEK_ON:
			MENU_CheckRadioItem(_CDROMMenu, WM_STARTFASTSEEK_ON, WM_STARTFASTSEEK_PRE, WM_STARTFASTSEEK_ON);
			_StartFastSeek = 1;
			CONFIG_Set("[APP DEVICE] Start FastSeek", &_StartFastSeek, sizeof(_StartFastSeek));
			MENU_CheckItem(_SpeedMenu, WM_SPEED_FASTSEEK, TRUE);
			PRINTF("Set FastSeek Speed.");
			CDROM_SetFastSeek(TRUE);
			_bFastSeek = TRUE;
			CONFIG_Set("[APP DEVICE] FastSeek", &_bFastSeek, sizeof(_bFastSeek));
			run_emulator(bRunNormalState);
			break;
		case APPEVENT_STARTFASTSEEK_OFF:
			MENU_CheckRadioItem(_CDROMMenu, WM_STARTFASTSEEK_ON, WM_STARTFASTSEEK_PRE, WM_STARTFASTSEEK_OFF);
			_StartFastSeek = 0;
			CONFIG_Set("[APP DEVICE] Start FastSeek", &_StartFastSeek, sizeof(_StartFastSeek));
			MENU_CheckItem(_SpeedMenu, WM_SPEED_FASTSEEK, FALSE);
			PRINTF("Set NormalSeek Speed.");
			CDROM_SetFastSeek(FALSE);
			_bFastSeek = FALSE;
			CONFIG_Set("[APP DEVICE] FastSeek", &_bFastSeek, sizeof(_bFastSeek));
			run_emulator(bRunNormalState);
			break;
		case APPEVENT_STARTFASTSEEK_PRE:
			MENU_CheckRadioItem(_CDROMMenu, WM_STARTFASTSEEK_ON, WM_STARTFASTSEEK_PRE, WM_STARTFASTSEEK_PRE);
			_StartFastSeek = 2;
			CONFIG_Set("[APP DEVICE] Start FastSeek", &_StartFastSeek, sizeof(_StartFastSeek));
			CONFIG_Set("[APP DEVICE] FastSeek", &_bFastSeek, sizeof(_bFastSeek));
			PRINTF("Memorize FastSeek setting.");
			run_emulator(bRunNormalState);
			break;

		//Kitao追加。グラディウス起動時に、自動で設定(レーザーちらつきなし)を行うかどうかを決める。
		case APPEVENT_AUTO_GRADIUS2:
			if (MessageBox(WINMAIN_GetHwnd(), "This setting becomes effective from the Next Start(or [F1]key Reset). Ok?    ", "Ootake - \"Gradius II \" Setting", MB_YESNO) == IDYES)
			{
				if (_bAutoGradiusII)
				{
					PRINTF("Reset Auto Improve Graphics & Sound ( Gradius II ).");
					_bAutoGradiusII = FALSE;
				}
				else
				{
					PRINTF("Set Auto Improve Graphics & Sound ( Gradius II ).");
					_bAutoGradiusII = TRUE;
				}
				CONFIG_Set("[APP INPUT] Auto Gradius II", &_bAutoGradiusII, sizeof(_bAutoGradiusII));
				MENU_CheckItem(_SetCpuMenu, WM_AUTO_GRADIUS2, _bAutoGradiusII);
				run_emulator(bRunNormalState);
			}
			break;

		//Kitao追加。真女神転生起動時に、自動でCPU速度を上げる設定(実機でも起こる描画関連のバグを解消)を行うかどうかを決める。v2.20
		case APPEVENT_AUTO_MEGATEN:
			if (_bAutoShinMegamiTensei)
			{
				PRINTF("Reset Auto Reduce Bug ( Shin Megami Tensei ).");
				if (VDC_GetShinMegamiTensei()) //真女神転生をプレイ中の場合、すぐに反映させる。
				{
					VDC_SetForceRaster(FALSE);
					VDC_SetShinMegamiTensei(FALSE);
				}
				_bAutoShinMegamiTensei = FALSE;
			}
			else
			{
				PRINTF("Set Auto Reduce Bug ( Shin Megami Tensei ).");
				if (VDC_GetShinMegamiTensei()) //真女神転生をプレイ中の場合、すぐに反映させる。
				{
					VDC_SetForceRaster(TRUE);
					VDC_SetShinMegamiTensei(TRUE);
				}
				_bAutoShinMegamiTensei = TRUE;
			}
			CONFIG_Set("[APP INPUT] Auto ShinMegamiTensei", &_bAutoShinMegamiTensei, sizeof(_bAutoShinMegamiTensei));
			MENU_CheckItem(_SetCpuMenu, WM_AUTO_MEGATEN, _bAutoShinMegamiTensei);
			run_emulator(bRunNormalState);
			break;

		//Kitao追加。起動時の"Perform SpriteLimit"を設定する。
		case APPEVENT_STARTSPRITE_OFF:
			MENU_CheckRadioItem(_SetCpuMenu, WM_STARTSPRITE_OFF, WM_STARTSPRITE_PRE, WM_STARTSPRITE_OFF);
			_StartSpriteLimit = 0;
			CONFIG_Set("[APP CPU] Start SpriteLimit", &_StartSpriteLimit, sizeof(_StartSpriteLimit));
			MENU_CheckItem(_CpuMenu, WM_SPRITEOVER, FALSE);
			PRINTF("Set Draw AllSprites.");
			VDC_SetPerformSpriteLimit(FALSE);
			_bSpriteLimit = FALSE;
			CONFIG_Set("[APP CPU] SpriteLimit", &_bSpriteLimit, sizeof(_bSpriteLimit));
			run_emulator(bRunNormalState);
			break;
		case APPEVENT_STARTSPRITE_ON:
			MENU_CheckRadioItem(_SetCpuMenu, WM_STARTSPRITE_OFF, WM_STARTSPRITE_PRE, WM_STARTSPRITE_ON);
			_StartSpriteLimit = 1;
			CONFIG_Set("[APP CPU] Start SpriteLimit", &_StartSpriteLimit, sizeof(_StartSpriteLimit));
			MENU_CheckItem(_CpuMenu, WM_SPRITEOVER, TRUE);
			PRINTF("Set Perform SpriteLimit.");
			VDC_SetPerformSpriteLimit(TRUE);
			_bSpriteLimit = TRUE;
			CONFIG_Set("[APP CPU] SpriteLimit", &_bSpriteLimit, sizeof(_bSpriteLimit));
			run_emulator(bRunNormalState);
			break;
		case APPEVENT_STARTSPRITE_PRE:
			MENU_CheckRadioItem(_SetCpuMenu, WM_STARTSPRITE_OFF, WM_STARTSPRITE_PRE, WM_STARTSPRITE_PRE);
			_StartSpriteLimit = 2;
			CONFIG_Set("[APP CPU] Start SpriteLimit", &_StartSpriteLimit, sizeof(_StartSpriteLimit));
			CONFIG_Set("[APP CPU] SpriteLimit", &_bSpriteLimit, sizeof(_bSpriteLimit));
			PRINTF("Memorize SpriteLimit setting.");
			run_emulator(bRunNormalState);
			break;

		//Kitao追加。早回し(VIDEO描画速度を加速)する機能を付けた。v0.92
		//			 2.0倍以上の速度はフレームが完全に間引かれてしまいゲームにならなくなってしまう可能性があるので
		//			 自分としては現状は1.83倍までにしておく。
		case APPEVENT_SPEED_V0: setVideoSpeedUpSub(   0); run_emulator(bRunNormalState); break; //1.0倍(ノーマル)
		case APPEVENT_SPEED_V9: setVideoSpeedUpSub(2002); run_emulator(bRunNormalState); break; //0.5倍(スロー)
		case APPEVENT_SPEED_V8: setVideoSpeedUpSub(2004); run_emulator(bRunNormalState); break; //0.75倍
		case APPEVENT_SPEED_V7: setVideoSpeedUpSub(2010); run_emulator(bRunNormalState); break; //0.90倍
		case APPEVENT_SPEED_V1: setVideoSpeedUpSub(  10); run_emulator(bRunNormalState); break; //1.10倍
		case APPEVENT_SPEED_V2: setVideoSpeedUpSub(   5); run_emulator(bRunNormalState); break; //1.20倍
		case APPEVENT_SPEED_V3: setVideoSpeedUpSub(   3); run_emulator(bRunNormalState); break; //1.33倍
		case APPEVENT_SPEED_V4: setVideoSpeedUpSub(   2); run_emulator(bRunNormalState); break; //1.50倍
		case APPEVENT_SPEED_V5: setVideoSpeedUpSub(1001); run_emulator(bRunNormalState); break; //1.67倍
		case APPEVENT_SPEED_V6: setVideoSpeedUpSub(1000); run_emulator(bRunNormalState); break; //1.83倍
		case APPEVENT_SPEED_VAL: always_video_speed_up(); break;
		case APPEVENT_SPEED_VUSE: use_video_speed_up_button(); break;
		case APPEVENT_SPEED_VSET: set_shortcut_key_button(2); break;
		//v1.61追加。[Home][End]キーでVideo速度の操作を可能に。1.0～1.83倍までを切り替え可能。
		//			 現状、1.0倍未満のスローダウンにする操作は、ショートカットキーには含まれない仕様としています。
		//			 ショートカットキーでむやみに難場面をスローで切り抜くプレイは、逆に作品の楽しみを奪ってしまうことが多いので
		//			 どうしても老人になって反射神経が衰えた方向けに、メニューからスローを選択する形で搭載しています。
		case APPEVENT_SPEED_VUP:
			if (_bUseVideoSpeedUpButton)
				_FastForwarding = 0;
			else
				_FastForwarding = MAINBOARD_GetFastForwardingR();
			if (_FastForwarding >= 0) //ノーマルになる以外の場合、AlwaysSpeedUpにする。
			{
				MENU_CheckRadioItem(_SpeedMenu, WM_SPEED_VAL, WM_SPEED_VUSE, WM_SPEED_VAL);
				_bUseVideoSpeedUpButton = FALSE;
			}
			else //ノーマルになる場合、UseVideoSpeedUpButtonにする。
			{
				MENU_CheckRadioItem(_SpeedMenu, WM_SPEED_VAL, WM_SPEED_VUSE, WM_SPEED_VUSE);
				_bUseVideoSpeedUpButton = TRUE;
			}
			switch (_FastForwarding)
			{
				case    0: setVideoSpeedUpSub(  10); break;
				case   10: setVideoSpeedUpSub(   5); break;
				case    5: setVideoSpeedUpSub(   3); break;
				case    3: setVideoSpeedUpSub(   2); break;
				case    2: setVideoSpeedUpSub(1001); break;
				case 1001: setVideoSpeedUpSub(1000); break;
				case 1000: setVideoSpeedUpSub(1000); break; //すでに最高速のときもメッセージ表示のため再設定。
				default: //スローだった場合
					setVideoSpeedUpSub(0); //ノーマルに戻す
			}
			run_emulator(bRunNormalState);
			break;
		case APPEVENT_SPEED_VDOWN:
			if (_bUseVideoSpeedUpButton)
				_FastForwarding = 0;
			else
				_FastForwarding = MAINBOARD_GetFastForwardingR();
			if ((MAINBOARD_GetFastForwardingR() > 0)&&(MAINBOARD_GetFastForwardingR() != 10)) //ノーマルになる以外の場合、AlwaysSpeedUpにする。
			{
				MENU_CheckRadioItem(_SpeedMenu, WM_SPEED_VAL, WM_SPEED_VUSE, WM_SPEED_VAL);
				_bUseVideoSpeedUpButton = FALSE;
			}
			else //ノーマルになる場合、UseVideoSpeedUpButtonにする。
			{
				MENU_CheckRadioItem(_SpeedMenu, WM_SPEED_VAL, WM_SPEED_VUSE, WM_SPEED_VUSE);
				_bUseVideoSpeedUpButton = TRUE;
			}
			switch (_FastForwarding)
			{
				case    0: setVideoSpeedUpSub(   0); break; //すでに最低速のときもメッセージ表示のため再設定。
				case   10: setVideoSpeedUpSub(   0); break;
				case    5: setVideoSpeedUpSub(  10); break;
				case    3: setVideoSpeedUpSub(   5); break;
				case    2: setVideoSpeedUpSub(   3); break;
				case 1001: setVideoSpeedUpSub(   2); break;
				case 1000: setVideoSpeedUpSub(1001); break;
				default: //スローだった場合
					setVideoSpeedUpSub(0); //ノーマルに戻す
			}
			run_emulator(bRunNormalState);
			break;

		//Kitao追加。CPU速度を設定する機能を付けた。v1.61更新
		case APPEVENT_SPEED_P0:
			VDC_SetOverClock(0); _OverClockTypeR = 0; updateMenu();	PRINTF("Set Normal CPU Speed.");
			run_emulator(bRunNormalState); break;
		case APPEVENT_SPEED_T3:
			VDC_SetOverClock(300); _OverClockTypeR = 300; updateMenu(); PRINTF("Set CpuSpeed TURBOx3.");
			run_emulator(bRunNormalState); break;
		case APPEVENT_SPEED_T2:
			VDC_SetOverClock(200); _OverClockTypeR = 200; updateMenu(); PRINTF("Set CpuSpeed TURBOx2.");
			run_emulator(bRunNormalState); break;
		case APPEVENT_SPEED_T1:
			VDC_SetOverClock(100); _OverClockTypeR = 100; updateMenu(); PRINTF("Set CpuSpeed TURBOx1.");
			run_emulator(bRunNormalState); break;
		case APPEVENT_SPEED_P6:
			VDC_SetOverClock(6); _OverClockTypeR = 6; updateMenu(); PRINTF("Set CpuSpeed x3.00.");
			run_emulator(bRunNormalState); break;
		case APPEVENT_SPEED_P5:
			VDC_SetOverClock(5); _OverClockTypeR = 5; updateMenu(); PRINTF("Set CpuSpeed x2.50.");
			run_emulator(bRunNormalState); break;
		case APPEVENT_SPEED_P4:
			VDC_SetOverClock(4); _OverClockTypeR = 4; updateMenu(); PRINTF("Set CpuSpeed x2.00.");
			run_emulator(bRunNormalState); break;
		case APPEVENT_SPEED_P3:
			VDC_SetOverClock(3); _OverClockTypeR = 3; updateMenu(); PRINTF("Set CpuSpeed x1.50.");
			run_emulator(bRunNormalState); break;
		case APPEVENT_SPEED_P2:
			VDC_SetOverClock(2); _OverClockTypeR = 2; updateMenu(); PRINTF("Set CpuSpeed x1.33.");
			run_emulator(bRunNormalState); break;
		case APPEVENT_SPEED_P1:
			VDC_SetOverClock(1); _OverClockTypeR = 1; updateMenu(); PRINTF("Set CpuSpeed x1.25.");
			run_emulator(bRunNormalState); break;
		case APPEVENT_SPEED_M1:
			VDC_SetOverClock(-1); _OverClockTypeR = -1; updateMenu(); PRINTF("Set CpuSpeed x0.75.");
			run_emulator(bRunNormalState); break;
		case APPEVENT_SPEED_M2:
			VDC_SetOverClock(-2); _OverClockTypeR = -2; updateMenu(); PRINTF("Set CpuSpeed x0.50.");
			run_emulator(bRunNormalState); break;
		case APPEVENT_SPEED_CSET:
			if (VDC_GetOverClockType() == 0)
			{
				MessageBox(	WINMAIN_GetHwnd(),
							"First of all, please select a favorite speed.\n"
							"And, execute this menu. Then, it is possible to change with [Delete]key.   \n",
							"Ootake", MB_OK);
				break;
			}
			_MySetOverClockType = VDC_GetOverClockType();
			CONFIG_Set("[APP DEVICE] MySet OverClock Type", &_MySetOverClockType, sizeof(_MySetOverClockType));
			switch (_MySetOverClockType)
			{
				case 300:	strcpy(buf,"TURBOx3");	break;
				case 200:	strcpy(buf,"TURBOx2");	break;
				case 100:	strcpy(buf,"TURBOx1");	break;
				case   6:	strcpy(buf,"x3.00");	break;
				case   5:	strcpy(buf,"x2.50");	break;
				case   4:	strcpy(buf,"x2.00");	break;
				case   3:	strcpy(buf,"x1.50");	break;
				case   2:	strcpy(buf,"x1.33");	break;
				case   1:	strcpy(buf,"x1.25");	break;
				case  -1:	strcpy(buf,"x0.75");	break;
				case  -2:	strcpy(buf,"x0.50");	break;
				default:	strcpy(buf,"none");		break;
			}
			PRINTF("Set CpuSpeed with [Delete]key. (%s)", buf);
			updateMenu();
			run_emulator(bRunNormalState);
			break;

		//Kitao追加。v2.36
		case APPEVENT_SPEED_UNLOAD:
			_bLoadStateSpeedSetting = FALSE;
			CONFIG_Set("[APP DEVICE] LoadState SpeedSetting", &_bLoadStateSpeedSetting, sizeof(_bLoadStateSpeedSetting));
			_bUseVideoSpeedUpButton = TRUE; //早回しがAlwaysモードのときはUseButtonモードに戻す。
			VDC_SetOverClock(_OverClockTypeR); //ステートロードで変わっていた場合元に戻る
			MAINBOARD_SetFastForwarding(_FastForwarding, _bSoundAjust, TRUE); //ステートロードで変わっていた場合元に戻る
			updateMenu();
			PRINTF("Unload Speed-Setting at LoadState.");
			run_emulator(bRunNormalState);
			break;
		case APPEVENT_SPEED_LOAD:
			_bLoadStateSpeedSetting = TRUE;
			CONFIG_Set("[APP DEVICE] LoadState SpeedSetting", &_bLoadStateSpeedSetting, sizeof(_bLoadStateSpeedSetting));
			MENU_CheckRadioItem(_SpeedMenu, WM_SPEED_UNLOAD, WM_SPEED_LOAD, WM_SPEED_LOAD);
			PRINTF("Load Speed-Setting at LoadState.");
			run_emulator(bRunNormalState);
			break;

		//Kitao追加
		case APPEVENT_SPEED_HELP:
			screenUpdate(); //メニューやダイアログ表示などで黒くなった部分を再描画。
			MessageBox(	WINMAIN_GetHwnd(),
						"In \"Simulation Game\" or \"Slow Motion happens Game\", CPU\n"
						"Speed-Up is effective.\n"
						"- Problem that screen falls into disorder might happen by Speed-Up.   \n"
						"- With a slow personal computer, the frame drops when the speed   \n"
						"  is increased too much.\n\n"
						"Keyborad - [Insert]Toggle Always VideoSpeedUp\n"
						"               [Home][End]Select VideoSpeed\n"
						"               [Delete]Change CPUSpeed (+[Shift][Ctrl]Select)\n"
						"               [BackSpace]Return to NomalSpeed",
						"Ootake", MB_OK);
			//screenUpdate(); //フルスクリーン時にメッセージ読んだ後にダイアログの黒い跡が残るのをこれで消せるかと思ったが駄目だったので省略。v2.00
			break;

		//Kitao追加。CD-ROMのアクセスで、実記並にウェイトを入れられる機能を付けた。
		case APPEVENT_SPEED_FASTCD:
			if (CDROM_GetFastCD())
			{
				CDROM_SetFastCD(FALSE);
				PRINTF("Set RealCD Speed.");
				_bFastCD = FALSE;
			}
			else
			{
				CDROM_SetFastCD(TRUE);
				PRINTF("Set FastCD Speed.");
				_bFastCD = TRUE;
			}
			MENU_CheckItem(_SpeedMenu, WM_SPEED_FASTCD, _bFastCD);
			MENU_EnableItem(_SpeedMenu, WM_SPEED_FASTSEEK, _bFastCD);
			CONFIG_Set("[APP DEVICE] FastCD", &_bFastCD, sizeof(_bFastCD));
			run_emulator(bRunNormalState);
			break;

		//Kitao追加。CD-ROMのシークのタイミングを高速化する機能を付けた。v0.82。
		case APPEVENT_SPEED_FASTSEEK:
			if (CDROM_GetFastSeek())
			{
				CDROM_SetFastSeek(FALSE);
				PRINTF("Set NormalSeek Speed.");
				_bFastSeek = FALSE;
			}
			else
			{
				CDROM_SetFastSeek(TRUE);
				PRINTF("Set FastSeek Speed.");
				_bFastSeek = TRUE;
			}
			MENU_CheckItem(_SpeedMenu, WM_SPEED_FASTSEEK, _bFastSeek);
			CONFIG_Set("[APP DEVICE] FastSeek", &_bFastSeek, sizeof(_bFastSeek));
			run_emulator(bRunNormalState);
			break;

		//Kitao追加。ラスタ割り込みのタイミングを設定する機能を付けた。
		case APPEVENT_RASTERTIMING_MEARLY:
			MENU_CheckRadioItem(_CpuMenu, WM_RASTERTIMING_MEARLY, WM_RASTERTIMING_MLATE, WM_RASTERTIMING_MEARLY);
			VDC_SetRasterTiming(4);
			PRINTF("Set MoreEarly RasterTiming.");
			run_emulator(bRunNormalState);
			break;
		case APPEVENT_RASTERTIMING_EARLY:
			MENU_CheckRadioItem(_CpuMenu, WM_RASTERTIMING_MEARLY, WM_RASTERTIMING_MLATE, WM_RASTERTIMING_EARLY);
			VDC_SetRasterTiming(1);
			PRINTF("Set Early RasterTiming.");
			run_emulator(bRunNormalState);
			break;
		case APPEVENT_RASTERTIMING_MIDDLE:
			MENU_CheckRadioItem(_CpuMenu, WM_RASTERTIMING_MEARLY, WM_RASTERTIMING_MLATE, WM_RASTERTIMING_MIDDLE);
			VDC_SetRasterTiming(2);
			PRINTF("Set Normal RasterTiming.");
			run_emulator(bRunNormalState);
			break;
		case APPEVENT_RASTERTIMING_LATE:
			MENU_CheckRadioItem(_CpuMenu, WM_RASTERTIMING_MEARLY, WM_RASTERTIMING_MLATE, WM_RASTERTIMING_LATE);
			VDC_SetRasterTiming(3);
			PRINTF("Set Late RasterTiming.");
			run_emulator(bRunNormalState);
			break;
		case APPEVENT_RASTERTIMING_MLATE:
			MENU_CheckRadioItem(_CpuMenu, WM_RASTERTIMING_MEARLY, WM_RASTERTIMING_MLATE, WM_RASTERTIMING_MLATE);
			VDC_SetRasterTiming(5);
			PRINTF("Set MoreLate RasterTiming.");
			run_emulator(bRunNormalState);
			break;

		//Kitao追加。CDインストールを一時的に無効にし、実CD-ROMで動作させる。v2.34
		case APPEVENT_INVALIDATE_CDINST:
			MAINBOARD_PauseNoMessage(TRUE);
			screenUpdate(); //メニューやダイアログ表示などで黒くなった部分を再描画。
			if (_bInvalidateCdInstall)
			{
				_bInvalidateCdInstall = FALSE;
				MENU_CheckItem(_CpuMenu, WM_INVALIDATE_CDINST, _bInvalidateCdInstall);
				PRINTF("\"CD Install\" is Effective.");
				run_emulator(bRunNormalState);
			}
			else
			{
				if (MessageBox(	WINMAIN_GetHwnd(),
								"This is compulsorily operated with \"Real CD-ROM\".   \n"
								"\"CD Installed\" file is not used. (for test)   \n"
								"- It returns when reset ([F1] key) or other games are started.   \n"
								"Push [Yes] after setting real CD-ROM.   \n\n"
								"In Japanese language\n"
								"\"CD Install\"したファイルを使わずに、実CD-ROMで動作させます。   \n"
								"※リセット([F1]キー)や他のゲームを起動すると、元に戻ります。   \n"
								"実CD-ROMをセット後に、[はい]を押してください。   \n",
								"Ootake", MB_YESNO) == IDYES)
				{
					//CDリセット
					_bInvalidateCdInstall = TRUE;
					_bCueFile = FALSE;
					if (!app_cd_change(FALSE)) //メニュー表示の更新とrun_emulator()も行われる。FALSE=CDを入れ替えずにF1キーでリセットした場合に、レジュームを無効にする。
						return FALSE; //システムカードのイメージ読み込み失敗のため、強制終了。
				}
			}
			break;

		//Kitao追加。スーパーグラフィックスモード切替
		case APPEVENT_SUPERGRAFX:
			MAINBOARD_PauseNoMessage(TRUE);
			screenUpdate(); //メニューやダイアログ表示などで黒くなった部分を再描画。
			if (_bResumeMode) //レジュームモードなら
				sprintf(buf, "After the \"Super Grafx Mode\" is changed,  \"Ootake\" is reset.   \nAnd \"Resume Mode\" is temporarily invalidated. OK?    ");
			else
				sprintf(buf, "After the \"Super Grafx Mode\" is changed,  \"Ootake\" is reset. Ok?    ");
			if (MessageBox(WINMAIN_GetHwnd(), buf, "Ootake", MB_YESNO) == IDYES)
			{
				if (MAINBOARD_GetSuperGrafx() == 1)
				{
					MENU_CheckItem(_CpuMenu, WM_SUPERGRAFX, FALSE);
					PRINTF("Non-SuperGrafx Mode.");
					//スパグラモードを切り替えてリセットする
					app_reset(0); //run_emulator()も行われる
				}
				else
				{
					MENU_CheckItem(_CpuMenu, WM_SUPERGRAFX, TRUE);
					PRINTF("Set SuperGrafx Mode.");
					//スパグラモードを切り替えてリセットする
					app_reset(1); //run_emulator()も行われる
				}
			}
			else
				run_emulator(bRunNormalState);
			break;

		//Kitao追加。スプライト欠けを再現する機能を付けた。
		case APPEVENT_SPRITEOVER:
			if (VDC_GetPerformSpriteLimit())
			{
				VDC_SetPerformSpriteLimit(FALSE);
				PRINTF("Set Draw AllSprites.");
				_bSpriteLimit = FALSE;
			}
			else
			{
				VDC_SetPerformSpriteLimit(TRUE);
				PRINTF("Set Perform SpriteLimit.");
				_bSpriteLimit = TRUE;
			}
			MENU_CheckItem(_CpuMenu, WM_SPRITEOVER, _bSpriteLimit);
			CONFIG_Set("[APP CPU] SpriteLimit", &_bSpriteLimit, sizeof(_bSpriteLimit));
			run_emulator(bRunNormalState);
			break;

		//Kitao追加。スプライトレイヤーを全非表示する。v2.06
		case APPEVENT_LAYER_SPRITE:
			_bSpriteLayer ^= TRUE;
			VDC_SetLayer();
			MENU_CheckItem(_CpuMenu, WM_LAYER_SPRITE, _bSpriteLayer);
			run_emulator(bRunNormalState);
			break;

		//Kitao追加。スパグラ用2つ目のスプライトレイヤーを全非表示する。v2.06
		case APPEVENT_LAYER_SPRITE2:
			_bSprite2Layer ^= TRUE;
			VDC_SetLayer();
			MENU_CheckItem(_CpuMenu, WM_LAYER_SPRITE2, _bSprite2Layer);
			run_emulator(bRunNormalState);
			break;

		//Kitao追加。BGレイヤーを全非表示する。v2.06
		case APPEVENT_LAYER_BG:
			_bBGLayer ^= TRUE;
			VDC_SetLayer();
			MENU_CheckItem(_CpuMenu, WM_LAYER_BG, _bBGLayer);
			run_emulator(bRunNormalState);
			break;

		//Kitao追加。スパグラ用2枚目のBGレイヤーを全非表示する。v2.06
		case APPEVENT_LAYER_BG2:
			_bBG2Layer ^= TRUE;
			VDC_SetLayer();
			MENU_CheckItem(_CpuMenu, WM_LAYER_BG2, _bBG2Layer);
			run_emulator(bRunNormalState);
			break;

		//Kitao追加。別のシステムカードに切替。v1.49
		case APPEVENT_CD_OSYSCARD1:
			MAINBOARD_PauseNoMessage(TRUE);
			MAINBOARD_ScreenClear();//画面をクリア。ゲームを変えるときはこうするとわかりやすい＆他のウィンドウが重なって画面が消えたあとにリセットした場合でも綺麗に再開できる。
			if (_bCDGame)
			{
				if (_bResumeMode) //レジュームモードなら
					sprintf(buf, "After the \"Other SystemCard Mode\" is changed,  \"Ootake\" is reset.   \nAnd \"Resume Mode\" is temporarily invalidated. OK?    ");
				else
					sprintf(buf, "After the \"Other SystemCard Mode\" is changed,  \"Ootake\" is reset. Ok?    ");
				if (MessageBox(WINMAIN_GetHwnd(), buf, "Ootake", MB_YESNO) == IDYES)
				{
					if (_OtherSysCard == 1)
					{
						_OtherSysCard = 0;
						strcpy(fn, _AppPath);
						strcat(fn, "SYSCARD.pce");
						set_openFilePathName(fn, _OpenFilePathName, _GameFilePathName);
					}
					else
					{
						_OtherSysCard = 1;
						set_openFilePathName(_OtherSysCardPath1, _OpenFilePathName, _GameFilePathName); //zipファイルだった場合にも対応
					}
					if ((fp = fopen(_OpenFilePathName, "r")) != NULL) //システムカードのイメージファイルが存在すれば
						fclose(fp);
					else //システムカードのイメージファイルが存在しなければ
					{
						//システムカードのイメージファイルを指定してもらう
						if (!file_dialog(3, WINMAIN_GetHwnd(), _OpenFilePathName, _GameFilePathName))
							return FALSE; //失敗した場合、強制終了。
					}
					//システムカードを切り替えてリセットする
					if (app_game_reopen(FALSE) == FALSE) //レジュームを無効にして起動する。run_emulator()も行われる。メニュー表示の更新も行われる。
						return FALSE; //開けないファイルを開こうとして失敗したため、強制終了。
				}
				else
					run_emulator(bRunNormalState);
			}
			else
			{
				MessageBox(WINMAIN_GetHwnd(), "This menu is effective only in the \"CD-ROM\" games.    ", "Ootake", MB_OK);
				run_emulator(bRunNormalState);
			}
			break;
		case APPEVENT_CD_OSYSCARD2:
			MAINBOARD_PauseNoMessage(TRUE);
			MAINBOARD_ScreenClear();//画面をクリア。ゲームを変えるときはこうするとわかりやすい＆他のウィンドウが重なって画面が消えたあとにリセットした場合でも綺麗に再開できる。
			if (_bCDGame)
			{
				if (_bResumeMode) //レジュームモードなら
					sprintf(buf, "After the \"Other SystemCard Mode\" is changed,  \"Ootake\" is reset.   \nAnd \"Resume Mode\" is temporarily invalidated. OK?    ");
				else
					sprintf(buf, "After the \"Other SystemCard Mode\" is changed,  \"Ootake\" is reset. Ok?    ");
				if (MessageBox(WINMAIN_GetHwnd(), buf, "Ootake", MB_YESNO) == IDYES)
				{
					if (_OtherSysCard == 2)
					{
						_OtherSysCard = 0;
						strcpy(fn, _AppPath);
						strcat(fn, "SYSCARD.pce");
						set_openFilePathName(fn, _OpenFilePathName, _GameFilePathName);
					}
					else
					{
						_OtherSysCard = 2;
						set_openFilePathName(_OtherSysCardPath2, _OpenFilePathName, _GameFilePathName); //zipファイルだった場合にも対応
					}
					if ((fp = fopen(_OpenFilePathName, "r")) != NULL) //システムカードのイメージファイルが存在すれば
						fclose(fp);
					else //システムカードのイメージファイルが存在しなければ
					{
						//システムカードのイメージファイルを指定してもらう
						if (!file_dialog(4, WINMAIN_GetHwnd(), _OpenFilePathName, _GameFilePathName))
							return FALSE; //失敗した場合、強制終了。
					}
					//システムカードを切り替えてリセットする
					if (app_game_reopen(FALSE) == FALSE) //レジュームを無効にして起動する。run_emulator()も行われる。メニュー表示の更新も行われる。
						return FALSE; //開けないファイルを開こうとして失敗したため、強制終了。
				}
				else
					run_emulator(bRunNormalState);
			}
			else
			{
				MessageBox(WINMAIN_GetHwnd(), "This menu is effective only in the \"CD-ROM\" games.    ", "Ootake", MB_OK);
				run_emulator(bRunNormalState);
			}
			break;

		//Kitao追加。アーケードカード使用モード切替
		case APPEVENT_CD_ARCADECARD:
			MAINBOARD_PauseNoMessage(TRUE);
			screenUpdate(); //メニューやダイアログ表示などで黒くなった部分を再描画。
			if (_bCDGame)
			{
				if (_bResumeMode) //レジュームモードなら
					sprintf(buf, "After the \"ARCADE Card Mode\" is changed,  \"Ootake\" is reset.   \nAnd \"Resume Mode\" is temporarily invalidated. OK?    ");
				else
					sprintf(buf, "After the \"ARCADE Card Mode\" is changed,  \"Ootake\" is reset. Ok?    ");
				if (MessageBox(WINMAIN_GetHwnd(), buf, "Ootake", MB_YESNO) == IDYES)
				{
					if (MAINBOARD_GetArcadeCard())
					{
						MAINBOARD_SetArcadeCard(FALSE);
						_bArcadeCardR = FALSE; //手動で切り替えた設定。以後Ootakeを終了するまで、違うゲームに切り替えた場合もこれがデフォルトになる。
						//アーケードカードのモードを切り替えてリセットする
						app_reset(MAINBOARD_GetSuperGrafx()); //run_emulator()も行われる。メニュー表示の更新も行われる。
						PRINTF("Non-ArcadeCard Mode.");
					}
					else
					{
						MAINBOARD_SetArcadeCard(TRUE);
						_bArcadeCardR = TRUE; //手動で切り替えた設定。以後Ootakeを終了するまで、違うゲームに切り替えた場合もこれがデフォルトになる。
						//アーケードカードのモードを切り替えてリセットする
						app_reset(MAINBOARD_GetSuperGrafx()); //run_emulator()も行われる。メニュー表示の更新も行われる。
						PRINTF("Set ArcadeCard Mode.");
					}
				}
				else
					run_emulator(bRunNormalState);
			}
			else
			{
				MessageBox(WINMAIN_GetHwnd(), "This menu is effective only in the \"CD-ROM\" games.    ", "Ootake", MB_OK);
				run_emulator(bRunNormalState);
			}
			break;

		//Kitao追加。バックアップメモリの空き容量を0に切替。v1.49
		case APPEVENT_CD_BACKUPFULL:
			MAINBOARD_PauseNoMessage(TRUE);
			MAINBOARD_ScreenClear();//画面をクリア。ゲームを変えるときはこうするとわかりやすい＆他のウィンドウが重なって画面が消えたあとにリセットした場合でも綺麗に再開できる。
			if (_bResumeMode) //レジュームモードなら
				sprintf(buf, "After the \"Backup-RAM Full Mode\" is changed,  \"Ootake\" is reset.   \nAnd \"Resume Mode\" is temporarily invalidated. OK?    ");
			else
				sprintf(buf, "After the \"Backup-RAM Full Mode\" is changed,  \"Ootake\" is reset. Ok?    ");
			if (MessageBox(WINMAIN_GetHwnd(), buf, "Ootake", MB_YESNO) == IDYES)
			{
				MAINBOARD_ScreenClear();//画面をクリア
				end_playrecord();
				end_recording();
				save_resume();
				save_bram();
				
				if (MAINBOARD_GetBackupFull())
				{
					MAINBOARD_SetBackupFull(FALSE);
					if (app_game_reopen(FALSE) == FALSE) //レジュームを無効にして起動する。run_emulator()も行われる。メニュー表示の更新も行われる。
						return FALSE; //開けないファイルを開こうとして失敗したため、強制終了。
					PRINTF("Set Nomal Backup-RAM Mode.");
				}
				else
				{
					MAINBOARD_SetBackupFull(TRUE);
					if (app_game_reopen(FALSE) == FALSE) //レジュームを無効にして起動する。run_emulator()も行われる。メニュー表示の更新も行われる。
						return FALSE; //開けないファイルを開こうとして失敗したため、強制終了。
					PRINTF("Set Backup-RAM Full Mode.");
				}
			}
			else
				run_emulator(bRunNormalState);
			break;

		//Kitao追加。ターボグラフィックス16用のHuカードを、日本のPCエンジン向けの吸出し機で吸出した場合、
		//			 データが化けているので正して出力する機能。
		case APPEVENT_BIT_CONVERT:
			MAINBOARD_Pause(TRUE); //ショートカットキーを押してここが呼び出されたときのために必要。
			WINMAIN_ShowCursor(TRUE); //ショートカットキーを押してここが呼び出されたときのために必要。
			MAINBOARD_ScreenClear();//画面をクリア
			strcpy(fn, _GameFilePathName);
			if (file_dialog(1, WINMAIN_GetHwnd(), _OpenFilePathName, fn))
			{
				end_playrecord();
				end_recording();
				save_resume();
				save_bram();
				
				_bCDGame = FALSE;
				_bCueFile = FALSE;//v2.24追加
				strcpy(_GameFilePathName, fn);
				if (app_game_reopen(TRUE))
				{	//ROMメモリをBIT変換してファイルに保存（"install"フォルダ内に保存される）
					MAINBOARD_TG16BitConvert();
					run_emulator(FALSE);
				}
				else
				{
					return FALSE; //開けないファイルを開こうとして失敗したため、強制終了。
				}
			}
			else
				run_emulator(bRunNormalState);
			break;

		//Kitao追加。デバッグ用のウィンドウを表示する。v1.07
		case APPEVENT_SHOW_DEBUG:
			if (_bFullScreen) //フルスクリーンの時はウィンドウに表示に切り替える
				APP_ToggleFullscreen();
			run_emulator(TRUE); //デバッグウィンドウの大きさを設定(メインと同じ高さに)するために、先にメニューを隠す。＆エミュレート再開。
			DEBUG_Init(WINMAIN_GetHInstance()); //デバッグウィンドウを表示
			break;

		//Kitao追加。全ての設定をデフォルトに戻すメニュー
		case APPEVENT_ALL_DEFAULT:
			if (MessageBox(WINMAIN_GetHwnd(), "All the settings are returned to default. Ok?    ", "Ootake", MB_YESNO) == IDYES)
				if (MessageBox(WINMAIN_GetHwnd(), "To initialize the setting, \"Ootake\" is closed.    ", "Ootake", MB_YESNO) == IDYES)
				{
					_bConfigReturnToDefault = TRUE;
					return APP_QUIT; //Ootakeを終了
				}
			break;

		//Kitao追加。スクリーンショットを保存。v2.12
		case APPEVENT_OUTPUT_SCREENSHOT:
			MAINBOARD_PauseNoMessage(TRUE); //ショートカットキーを押してここが呼び出されたときのために必要。
			OutputScreenshot();
			//※メニューから実行した場合、エミュレートをポーズしたままにしておく。違うサイズで同場面を撮りたい時用に。
			break;
		case APPEVENT_FOLDER_SCREENSHOT: //screenshotフォルダをマイコンピュータで開く
			{
				strcpy(fn, _AppPath);
				strcat(fn, "screenshot");
				CreateDirectory(fn, NULL);//recordディレクトリがない場合作る
				if (_VirtualStorePath[0] != 0) //v0.95追加。v2.20更新。Vista/7で"Program Files"にインストールした場合、セーブファイルがリダイレクトされているのでリダイレクト先のフォルダを開くようにする。
				{
					strcpy(fn, _VirtualStorePath);
					strcat(fn, "screenshot");
				}
				ShellExecute(NULL, "open", fn, NULL, NULL, SW_SHOW);
			}
			break;
		case APPEVENT_SCREENSHOT_BUTTON: //スクリーンショット用のボタンを設定
			set_shortcut_key_button(5);
			updateMenu();//メニュー表示を更新(ショートカットキーの表示)
			break;
		case APPEVENT_SCREENSHOT_DEFAULT: //スクリーンショット用のボタンをデフォルトに戻す。
			MAINBOARD_Pause(TRUE);
			WINMAIN_ShowCursor(TRUE);
			screenUpdate(); //メニューやダイアログ表示などで黒くなった部分を再描画。
			if (MessageBox(	WINMAIN_GetHwnd(),
							"Do you return \"Screenshot Button\" to the default([PrintScreen]key)?    ",
							"Ootake", MB_YESNO) == IDYES)
			{
				_ScreenshotButton = DIK_SYSRQ;
				CONFIG_Set("[APP DEVICE] Screenshot Button", &_ScreenshotButton, sizeof(_ScreenshotButton));
				updateMenu();//メニュー表示を更新(ショートカットキーの表示)
				PRINTF("Returned Screenshot Button.");
				run_emulator(bRunNormalState);
			}
			break;
		case APPEVENT_SCREENSHOT_X1:
			MENU_CheckRadioItem(_SShotSettingMenu, WM_SCREENSHOT_X1, WM_SCREENSHOT_XN, WM_SCREENSHOT_X1);
			PRINTF("Set Screenshot Size x1."); screenUpdate();//メッセージをすぐに表示
			_ScreenshotMagnification = 1;			
			CONFIG_Set("[APP DISPLAY] ScreenshotMagnification", &_ScreenshotMagnification, sizeof(_ScreenshotMagnification));			
			break;
		case APPEVENT_SCREENSHOT_X2:
			MENU_CheckRadioItem(_SShotSettingMenu, WM_SCREENSHOT_X1, WM_SCREENSHOT_XN, WM_SCREENSHOT_X2);
			PRINTF("Set Screenshot Size x2."); screenUpdate();//メッセージをすぐに表示
			_ScreenshotMagnification = 2;			
			CONFIG_Set("[APP DISPLAY] ScreenshotMagnification", &_ScreenshotMagnification, sizeof(_ScreenshotMagnification));			
			break;
		case APPEVENT_SCREENSHOT_X3:
			MENU_CheckRadioItem(_SShotSettingMenu, WM_SCREENSHOT_X1, WM_SCREENSHOT_XN, WM_SCREENSHOT_X3);
			PRINTF("Set Screenshot Size x3."); screenUpdate();//メッセージをすぐに表示
			_ScreenshotMagnification = 3;			
			CONFIG_Set("[APP DISPLAY] ScreenshotMagnification", &_ScreenshotMagnification, sizeof(_ScreenshotMagnification));			
			break;
		case APPEVENT_SCREENSHOT_X4:
			MENU_CheckRadioItem(_SShotSettingMenu, WM_SCREENSHOT_X1, WM_SCREENSHOT_XN, WM_SCREENSHOT_X4);
			PRINTF("Set Screenshot Size x4."); screenUpdate();//メッセージをすぐに表示
			_ScreenshotMagnification = 4;			
			CONFIG_Set("[APP DISPLAY] ScreenshotMagnification", &_ScreenshotMagnification, sizeof(_ScreenshotMagnification));			
			break;
		case APPEVENT_SCREENSHOT_XN:
			MENU_CheckRadioItem(_SShotSettingMenu, WM_SCREENSHOT_X1, WM_SCREENSHOT_XN, WM_SCREENSHOT_XN);
			PRINTF("Set Screenshot Size Non-Stretched(x1)."); screenUpdate();//メッセージをすぐに表示
			_ScreenshotMagnification = 0;			
			CONFIG_Set("[APP DISPLAY] ScreenshotMagnification", &_ScreenshotMagnification, sizeof(_ScreenshotMagnification));			
			break;
		case APPEVENT_SCREENSHOT_SIZE:
			screenUpdate(); //メニューやダイアログ表示などで黒くなった部分を再描画。
			MessageBox(	WINMAIN_GetHwnd(),
						"If this size setting is set more large than \"Screen playing present    \n"
						"game\", it is saved with the size of \"Screen playing present game\".    \n"
						"Please enlarge the play screen by the \"Screen\" menu when you    \n"
						"want to save it with large size. (This is a specification for the    \n"
						"speed valuing.)\n\n"
						"In Japanese language\n"
						"このスクリーンショットのサイズ設定が、\"現在プレイ中の画面\"よりも大き   \n"
						"く設定されていた場合、\"現在プレイ中の画面\"の大きさで保存されます。   \n"
						"大きく保存する場合は、Screenメニューでプレイ画面も大きくしてくださ   \n"
						"い。(速度重視のための仕様です)",
						"About Screenshot Size", MB_OK);
			break;
		case APPEVENT_SSHOT_SAMEPLAYING:
			MENU_CheckRadioItem(_SShotSettingMenu, WM_SSHOT_SAMEPLAYING, WM_SSHOT_NONSCANLINED, WM_SSHOT_SAMEPLAYING);
			PRINTF("Set Screenshot Same as Playing."); screenUpdate();//メッセージをすぐに表示
			_ScreenshotScanLineType = -1;			
			CONFIG_Set("[APP DISPLAY] Screenshot Scanline", &_ScreenshotScanLineType, sizeof(_ScreenshotScanLineType));
			break;
		case APPEVENT_SSHOT_SCANLINED:
			MENU_CheckRadioItem(_SShotSettingMenu, WM_SSHOT_SAMEPLAYING, WM_SSHOT_NONSCANLINED, WM_SSHOT_SCANLINED);
			PRINTF("Set Screenshot Special Scanlined."); screenUpdate();
			_ScreenshotScanLineType = 1;			
			CONFIG_Set("[APP DISPLAY] Screenshot Scanline", &_ScreenshotScanLineType, sizeof(_ScreenshotScanLineType));
			break;
		case APPEVENT_SSHOT_TVMODE:
			MENU_CheckRadioItem(_SShotSettingMenu, WM_SSHOT_SAMEPLAYING, WM_SSHOT_NONSCANLINED, WM_SSHOT_TVMODE);
			PRINTF("Set Screenshot TV Mode."); screenUpdate();
			_ScreenshotScanLineType = 4;			
			CONFIG_Set("[APP DISPLAY] Screenshot Scanline", &_ScreenshotScanLineType, sizeof(_ScreenshotScanLineType));
			break;
		case APPEVENT_SSHOT_HRSCANLINED:
			MENU_CheckRadioItem(_SShotSettingMenu, WM_SSHOT_SAMEPLAYING, WM_SSHOT_NONSCANLINED, WM_SSHOT_HRSCANLINED);
			PRINTF("Set Screenshot Horizontal Scanlined."); screenUpdate();
			_ScreenshotScanLineType = 2;			
			CONFIG_Set("[APP DISPLAY] Screenshot Scanline", &_ScreenshotScanLineType, sizeof(_ScreenshotScanLineType));
			break;
		case APPEVENT_SSHOT_NONSCANLINED:
			MENU_CheckRadioItem(_SShotSettingMenu, WM_SSHOT_SAMEPLAYING, WM_SSHOT_NONSCANLINED, WM_SSHOT_NONSCANLINED);
			PRINTF("Set Screenshot Non-Scanlined."); screenUpdate();
			_ScreenshotScanLineType = 0;			
			CONFIG_Set("[APP DISPLAY] Screenshot Scanline", &_ScreenshotScanLineType, sizeof(_ScreenshotScanLineType));
			break;
		case APPEVENT_SSHOT_SAVEFOLDER:
			MENU_CheckRadioItem(_SShotSettingMenu, WM_SSHOT_SAVEFOLDER, WM_SSHOT_SAVEDIALOG, WM_SSHOT_SAVEFOLDER);
			PRINTF("Screenshot saves to \"screenshot\" Folder."); screenUpdate();
			_bScreenshotDialog = FALSE;			
			CONFIG_Set("[APP DISPLAY] Screenshot Dialog", &_bScreenshotDialog, sizeof(_bScreenshotDialog));
			break;
		case APPEVENT_SSHOT_SAVEDIALOG:
			MENU_CheckRadioItem(_SShotSettingMenu, WM_SSHOT_SAVEFOLDER, WM_SSHOT_SAVEDIALOG, WM_SSHOT_SAVEDIALOG);
			PRINTF("Show Screenshot Save Dialog."); screenUpdate();
			_bScreenshotDialog = TRUE;			
			CONFIG_Set("[APP DISPLAY] Screenshot Dialog", &_bScreenshotDialog, sizeof(_bScreenshotDialog));
			break;

		//Kitao追加。サウンドをWAV形式ファイルで出力する。v1.29
		case APPEVENT_OUTPUT_WAV1:
			MAINBOARD_Pause(TRUE); //ショートカットキーを押してここが呼び出されたときのために必要。
			WINMAIN_ShowCursor(TRUE); //ショートカットキーを押してここが呼び出されたときのために必要。
			OutputWavStart(1);
			run_emulator(bRunNormalState);
			break;
		case APPEVENT_OUTPUT_WAV123:
			MAINBOARD_Pause(TRUE);
			WINMAIN_ShowCursor(TRUE);
			OutputWavStart(123);
			run_emulator(bRunNormalState);
			break;
		case APPEVENT_OUTPUT_WAV12:
			MAINBOARD_Pause(TRUE);
			WINMAIN_ShowCursor(TRUE);
			OutputWavStart(12);
			run_emulator(bRunNormalState);
			break;
		case APPEVENT_OUTPUT_WAV2:
			MAINBOARD_Pause(TRUE);
			WINMAIN_ShowCursor(TRUE);
			OutputWavStart(2);
			run_emulator(bRunNormalState);
			break;
		case APPEVENT_OUTPUT_WAV3:
			MAINBOARD_Pause(TRUE);
			WINMAIN_ShowCursor(TRUE);
			OutputWavStart(3);
			run_emulator(bRunNormalState);
			break;
		case APPEVENT_OUTPUT_WAV0:
			screenUpdate(); //メニューやダイアログ表示などで黒くなった部分を再描画。
			MessageBox( WINMAIN_GetHwnd(),
						"\"Output only with few channels (Example. Only PSG)\" is more High Quality.    ",
						"Ootake Hint", MB_OK);
			break;
		case APPEVENT_OUTPUT_WAVS1:
			MENU_CheckRadioItem(_WavSettingMenu, WM_OUTPUT_WAVS1, WM_OUTPUT_WAVS2, WM_OUTPUT_WAVS1);
			_bOutputWavNext = FALSE;
			CONFIG_Set("[APP SOUND] WAV Next", &_bOutputWavNext, sizeof(_bOutputWavNext));
			PRINTF("Set \"Output WAV Start\" Normal.");
			screenUpdate();
			break;
		case APPEVENT_OUTPUT_WAVS2:
			MENU_CheckRadioItem(_WavSettingMenu, WM_OUTPUT_WAVS1, WM_OUTPUT_WAVS2, WM_OUTPUT_WAVS2);
			_bOutputWavNext = TRUE;
			CONFIG_Set("[APP SOUND] WAV Next", &_bOutputWavNext, sizeof(_bOutputWavNext));
			PRINTF("Set \"Output WAV Start\" at the Next Tune.");
			screenUpdate();
			break;
		case APPEVENT_OUTPUT_WAVNT:
			if (JOYPAD_GetMakeHesNoSound())
			{
				JOYPAD_SetMakeHesNoSound(FALSE);
				PRINTF("Reset Output WAV Add \"no sound part\" when going to Next Tune.");
			}
			else
			{
				JOYPAD_SetMakeHesNoSound(TRUE);
				PRINTF("Set Output WAV Add \"no sound part\" when going to Next Tune.");
			}
			MENU_CheckItem(_WavSettingMenu, WM_OUTPUT_WAVNT, JOYPAD_GetMakeHesNoSound());
			run_emulator(bRunNormalState);
			break;
		case APPEVENT_OUTPUT_WAVBE:
			if (_bOutputWavAddNoSound)
			{
				_bOutputWavAddNoSound = FALSE;
				PRINTF("Reset Output WAV Add \"no sound part\" to begin and end.");
			}
			else
			{
				_bOutputWavAddNoSound = TRUE;
				PRINTF("Set Output WAV Add \"no sound part\" to begin and end.");
			}
			MENU_CheckItem(_WavSettingMenu, WM_OUTPUT_WAVBE, _bOutputWavAddNoSound);
			CONFIG_Set("[APP SOUND] WAV Add No Sound", &_bOutputWavAddNoSound, sizeof(_bOutputWavAddNoSound));
			screenUpdate();
			break;

		//Kitao追加。プレイ動画をAVI出力(搭載予定)
		case APPEVENT_OUTPUT_AVI:
			MAINBOARD_Pause(TRUE); //ショートカットキーを押してここが呼び出されたときのために必要。
			WINMAIN_ShowCursor(TRUE); //ショートカットキーを押してここが呼び出されたときのために必要。
			OutputAviStart();
			run_emulator(bRunNormalState);
			break;
		case APPEVENT_ABOUT_AVI:
			break;
	}

	APPEVENT_Ack();
	return TRUE; //Kitao追加。正常終了。
}


//Kitao追加。Ootakeのウィンドウを確実に手前に表示してアクティブにする。
void
APP_SetForegroundWindowOotake()
{
	int		foregroundID, targetID;
	DWORD	sp_time;

	foregroundID = GetWindowThreadProcessId(GetForegroundWindow(), NULL);
	targetID = GetWindowThreadProcessId(WINMAIN_GetHwnd(), NULL);
	AttachThreadInput(targetID, foregroundID, TRUE);
	SystemParametersInfo(SPI_GETFOREGROUNDLOCKTIMEOUT, 0, &sp_time, 0);
	SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, (LPVOID)0, 0);
	SetForegroundWindow(WINMAIN_GetHwnd());
	SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, &sp_time, 0);
	AttachThreadInput(targetID, foregroundID, FALSE);
}


//Kitao追加。ファンクションボタンを押しならRUNボタンの処理。最近読み込んだROMを選択。v2.48
void
FuncRecentRom()
{
	Sint32	selectRom = 0;
	MSG		msg;
	char	fn[MAX_PATH+1]; //Kitao追加。ファイルネーム格納用
	FILE*	fp;
	BOOL	bJoypadBackground;
	BOOL	bWindowTopMostR;

	_bPauseNoRelease = TRUE; //RecentRomウィンドウ破棄時に自動でポーズが解除されてしまわないために必要。

	MAINBOARD_Pause(TRUE);
	WINMAIN_ShowCursor(FALSE);

	bJoypadBackground = _bJoypadBackground;
	if (bJoypadBackground == FALSE)
		APP_SetJoypadBackgroundTemp(TRUE); //バックグラウンドからのジョイパッド入力が無効だった場合でも、一時的に有効とする。

	bWindowTopMostR = _bWindowTopMost;
	APP_SetWindowTopMostTemp(FALSE); //DirectDrawでフルスクリーン時に必要。Direct3DはなくてもOK。
	RECENT_Init(WINMAIN_GetHInstance(), &selectRom);
	while (selectRom == 0) //ダイアログが結果を返すまでループ
	{ 
		GetMessage(&msg ,NULL, 0, 0);
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	APP_SetWindowTopMostTemp(bWindowTopMostR);

	if (selectRom == 1) //一番上（CDプレイ以外の場合、現在遊んでいるゲーム）を選択した場合
	{
		if ((_bCDGame)&&(!_bCueFile))
		{
			if (recent_sub(1) == FALSE) //履歴からファイルを開く
				APPEVENT_Set(APPEVENT_EXIT, NULL); //失敗した場合は強制終了。
		}
		else //HuカードゲームかCUEファイル起動の場合、「前回開いたファイル＝現在遊んでいるゲーム」。
		{
			//今ノンレジューム状態で、レジュームファイルが存在するなら（レジュームをステートロードなどで解除していた場合）、レジューム復帰する。
			if ((_bResumeMode == FALSE)&&(!MAINBOARD_GetBackupFull())) //※バックアップRAMフルメニューを使用中のときはレジューム復帰しない
			{
				strcpy(fn, _AppPath);
				strcat(fn, "save\\");//saveフォルダ
				strcat(fn, _GameFileNameBuf);
				strcat(fn, "_resume.dat");
				if ((fp = fopen(fn, "r")) != NULL) //レジュームファイルがあれば
				{
					fclose(fp);
					if (recent_sub(1) == FALSE) //履歴からファイルを開く
						APPEVENT_Set(APPEVENT_EXIT, NULL); //失敗した場合は強制終了。
				}
			}
			run_emulator(TRUE);
		}
	}
	else if (selectRom >= 2) //ROMを選択した場合
	{
		if (recent_sub(selectRom) == FALSE) //履歴からファイルを開く
			APPEVENT_Set(APPEVENT_EXIT, NULL); //失敗した場合は強制終了。
	}
	else if (selectRom == -2) //Open Rom
	{
		WINMAIN_ShowCursor(TRUE);
		if (!FileDialogSub()) //ファイルダイアログを表示
			APPEVENT_Set(APPEVENT_EXIT, NULL); //開けないファイルを開こうとして失敗したため、強制終了。
	}
	else if (selectRom == -3) //Open Cue
	{
		WINMAIN_ShowCursor(TRUE);
		if (!FolderDialogSub()) //フォルダーダイアログを表示
			APPEVENT_Set(APPEVENT_EXIT, NULL); //システムカードのイメージ読み込み失敗のため、強制終了。
	}
	else //(selectRom == -1)
		run_emulator(TRUE);

	if (bJoypadBackground == FALSE)
		APP_SetJoypadBackgroundTemp(FALSE); //元に戻す
	WINMAIN_ShowCursor(FALSE);
	_bPauseNoRelease = FALSE;
}


/*-----------------------------------------------------------------------------
	[ProcessEvents]
		アプリケーションに対する入力やメッセージなどを処理します。 
-----------------------------------------------------------------------------*/
Sint32
APP_ProcessEvents()
{
	Sint32	event;
	Sint32	fastForwarding;
	BOOL	ok;
	POINT	p;
	BOOL	ret;

	//イベント処理
	if ((event = APPEVENT_GetEventID()) == APPEVENT_EXIT)
		return APP_QUIT;
	else
	{	//Kitao更新
		ret = handle_event(event);
		if (ret == FALSE)
		{
			APPEVENT_Ack();
			return APP_QUIT;
		}
	}

	//Kitao更新。ショートカットキー用に入力状態を更新。（将来バージョンのショートカットキーコンフィグ用）
	//	PCEパッドの入力状態更新は、反応性向上のためJoyPad.cで入力を細かく(そのつど)更新するようにした。
	INPUT_UpdateState(FALSE); //FALSE=記録・再生はしない

	//Kitao追加。Ootakeのウィンドウが非アクティブ状態のときに、プレイヤー１のジョイパッドが操作されたら自動でアクティブに戻す。
	if ((_bWindowActivate)&&
		(GetForegroundWindow() != WINMAIN_GetHwnd())&&
		(IsIconic(WINMAIN_GetHwnd()) == FALSE)&& //最小化されているときはアクティブにしない
		(MAINBOARD_GetPause() == FALSE)) //ポーズされているときはアクティブにしない。v1.06
	{
		ok = FALSE;
		//キーボード操作(_PcePadの値が300未満(0～255))には反応させない。キーボードは他のアプリ操作優先にしておく。
		if (JOYPAD_GetConnectSixButton())
		{
			if ((_PcePad6[1].buttonU   >= 300)&&(INPUT_CheckButtonState(_PcePad6[1].buttonU)))   ok = TRUE;
			if ((_PcePad6[1].buttonR   >= 300)&&(INPUT_CheckButtonState(_PcePad6[1].buttonR)))   ok = TRUE;
			if ((_PcePad6[1].buttonD   >= 300)&&(INPUT_CheckButtonState(_PcePad6[1].buttonD)))   ok = TRUE;
			if ((_PcePad6[1].buttonL   >= 300)&&(INPUT_CheckButtonState(_PcePad6[1].buttonL)))   ok = TRUE;
			if ((_PcePad6[1].buttonSel >= 300)&&(INPUT_CheckButtonState(_PcePad6[1].buttonSel))) ok = TRUE;
			if ((_PcePad6[1].buttonRun >= 300)&&(INPUT_CheckButtonState(_PcePad6[1].buttonRun))) ok = TRUE;
			if ((_PcePad6[1].button1   >= 300)&&(INPUT_CheckButtonState(_PcePad6[1].button1)))   ok = TRUE;
			if ((_PcePad6[1].button2   >= 300)&&(INPUT_CheckButtonState(_PcePad6[1].button2)))   ok = TRUE;
			if ((_PcePad6[1].button3   >= 300)&&(INPUT_CheckButtonState(_PcePad6[1].button3)))   ok = TRUE;
			if ((_PcePad6[1].button4   >= 300)&&(INPUT_CheckButtonState(_PcePad6[1].button4)))   ok = TRUE;
			if ((_PcePad6[1].button5   >= 300)&&(INPUT_CheckButtonState(_PcePad6[1].button5)))   ok = TRUE;
			if ((_PcePad6[1].button6   >= 300)&&(INPUT_CheckButtonState(_PcePad6[1].button6)))   ok = TRUE;
		}
		else if (JOYPAD_GetConnectThreeButton())
		{
			if ((_PcePad3[1].buttonU   >= 300)&&(INPUT_CheckButtonState(_PcePad3[1].buttonU)))   ok = TRUE;
			if ((_PcePad3[1].buttonR   >= 300)&&(INPUT_CheckButtonState(_PcePad3[1].buttonR)))   ok = TRUE;
			if ((_PcePad3[1].buttonD   >= 300)&&(INPUT_CheckButtonState(_PcePad3[1].buttonD)))   ok = TRUE;
			if ((_PcePad3[1].buttonL   >= 300)&&(INPUT_CheckButtonState(_PcePad3[1].buttonL)))   ok = TRUE;
			if ((_PcePad3[1].buttonSel >= 300)&&(INPUT_CheckButtonState(_PcePad3[1].buttonSel))) ok = TRUE;
			if ((_PcePad3[1].buttonRun >= 300)&&(INPUT_CheckButtonState(_PcePad3[1].buttonRun))) ok = TRUE;
			if ((_PcePad3[1].button1   >= 300)&&(INPUT_CheckButtonState(_PcePad3[1].button1)))   ok = TRUE;
			if ((_PcePad3[1].button2   >= 300)&&(INPUT_CheckButtonState(_PcePad3[1].button2)))   ok = TRUE;
			if ((_PcePad3[1].button3   >= 300)&&(INPUT_CheckButtonState(_PcePad3[1].button3)))   ok = TRUE;
			if ((_PcePad3[1].button4   >= 300)&&(INPUT_CheckButtonState(_PcePad3[1].button4)))   ok = TRUE;
			if ((_PcePad3[1].button5   >= 300)&&(INPUT_CheckButtonState(_PcePad3[1].button5)))   ok = TRUE; //連射専用ボタン
			if ((_PcePad3[1].button6   >= 300)&&(INPUT_CheckButtonState(_PcePad3[1].button6)))   ok = TRUE; //連射専用ボタン
		}
		else //２ボタンパッド時
		{
			if ((_PcePad2[1].buttonU   >= 300)&&(INPUT_CheckButtonState(_PcePad2[1].buttonU)))   ok = TRUE;
			if ((_PcePad2[1].buttonR   >= 300)&&(INPUT_CheckButtonState(_PcePad2[1].buttonR)))   ok = TRUE;
			if ((_PcePad2[1].buttonD   >= 300)&&(INPUT_CheckButtonState(_PcePad2[1].buttonD)))   ok = TRUE;
			if ((_PcePad2[1].buttonL   >= 300)&&(INPUT_CheckButtonState(_PcePad2[1].buttonL)))   ok = TRUE;
			if ((_PcePad2[1].buttonSel >= 300)&&(INPUT_CheckButtonState(_PcePad2[1].buttonSel))) ok = TRUE;
			if ((_PcePad2[1].buttonRun >= 300)&&(INPUT_CheckButtonState(_PcePad2[1].buttonRun))) ok = TRUE;
			if ((_PcePad2[1].button1   >= 300)&&(INPUT_CheckButtonState(_PcePad2[1].button1)))   ok = TRUE;
			if ((_PcePad2[1].button2   >= 300)&&(INPUT_CheckButtonState(_PcePad2[1].button2)))   ok = TRUE;
			if ((_PcePad2[1].button5   >= 300)&&(INPUT_CheckButtonState(_PcePad2[1].button5)))   ok = TRUE; //連射専用ボタン
			if ((_PcePad2[1].button6   >= 300)&&(INPUT_CheckButtonState(_PcePad2[1].button6)))   ok = TRUE; //連射専用ボタン
		}
		if ((_VideoSpeedUpButton >= 300)&&(INPUT_CheckButtonState(_VideoSpeedUpButton))) ok = TRUE; //早回し用ボタン
		
		if (ok)
			APP_SetForegroundWindowOotake(); //ウィンドウをフォアグラウンドに持ってきてアクティブにする
	}

	//Kitao追加。早回しモードの処理。AlWaysモードか、早回し機能(VideoSpeedUp)のボタンを押しているときだけ早回しモードにする。
	fastForwarding = MAINBOARD_GetFastForwardingR(); //event中にFastForwardingが変化することがあるので必ずこの位置で。
	if (fastForwarding != 0)
	{
		if (_bUseVideoSpeedUpButton)
		{
			if (fastForwarding < 2000)
			{
				if (INPUT_CheckButtonState(_VideoSpeedUpButton))
					MAINBOARD_SetFastForwarding(fastForwarding, _bSoundAjust, FALSE);
				else
					MAINBOARD_SetFastForwarding(0, _bSoundAjust, FALSE); //押していなかったら早回しモードを解除
			}
			else //Slow倍率の場合
			{
				if (INPUT_CheckButtonState(_VideoSpeedUpButton))
					MAINBOARD_SetFastForwarding(0, _bSoundAjust, FALSE); //Slowのときは押されていたら通常速度に
				else
					MAINBOARD_SetFastForwarding(fastForwarding, _bSoundAjust, FALSE);
			}
		}
		else //Alwaysモードの場合
		{
			if (INPUT_CheckButtonState(_VideoSpeedUpButton))
				MAINBOARD_SetFastForwarding(0, _bSoundAjust, FALSE); //Alwaysモードのときは押されていたら通常速度に
			else
				MAINBOARD_SetFastForwarding(fastForwarding, _bSoundAjust, FALSE);
		}
	}

	MAINBOARD_AdvanceFrame();

	//##RA
	RA_DoAchievementsFrame();
	RA_HandleHTTPResults();

	PRINTF_Update();

	//Kitao追加
	if ((!_bFullScreen)&&(!MOUSE_IsConnected())&&(!MAINBOARD_GetPause())) //ウィンドウモード時＆マウス非接続のとき
	{
		GetCursorPos(&p);
		if ((p.x != _PrevMouseX)||(p.y != _PrevMouseY)) //マウスカーソルが動いていた場合
		{
			WINMAIN_ShowCursor(TRUE); //マウスカーソルを表示。マウスカーソル隠し用カウンタもリセットされる。
			_PrevMouseX = p.x;
			_PrevMouseY = p.y;
		}
		else
		{	//マウスカーソルが止まっている場合
			if (_MouseStopCount > 0)
				if (--_MouseStopCount == 0)
					WINMAIN_ShowCursor(FALSE); //マウスカーソルを隠す
		}
	}

	if( MAINBOARD_GetPause() == TRUE )	//	Push overlay when paused
	{
		screenUpdate();
		//InvalidateRect( WINMAIN_GetHwnd(), NULL, FALSE );
		//Sleep( 16 );
	}

	//ショートカットキー入力処理。v1.56追加
	ok = (GetForegroundWindow() == WINMAIN_GetHwnd()); //ウィンドウがアクティブならTRUE
	if (_bKeyRepeatSaveState) //ステートセーブ
	{	//前回押して、まだ押しっぱなしの場合
		if (!INPUT_CheckButtonState(_SaveStateButton))
			_bKeyRepeatSaveState = FALSE;
	}
	else
	{
		if ((ok)||(_SaveStateButton >= 300)) //ウィンドウがアクティブか、またはジョイパッドのボタンに設定しあった場合のみ、入力を受付。(キーボードで非アクティブのときは受け付けない)
			if (INPUT_CheckButtonState(_SaveStateButton))
			{
				MAINBOARD_PauseNoMessage(TRUE);
				save_state(0);
				run_emulator(TRUE);
				_bKeyRepeatSaveState = TRUE;//キーリピートを防ぐために必要
			}
	}
	if (_bKeyRepeatLoadState) //ステートロード
	{	//前回押して、まだ押しっぱなしの場合
		if (!INPUT_CheckButtonState(_LoadStateButton))
			_bKeyRepeatLoadState = FALSE;
	}
	else
	{
		if ((ok)||(_LoadStateButton >= 300))
			if (INPUT_CheckButtonState(_LoadStateButton))
			{
				MAINBOARD_PauseNoMessage(TRUE);
				load_state(0);
				run_emulator(TRUE);
				_bKeyRepeatLoadState = TRUE;//キーリピートを防ぐために必要
			}
	}
	if (_bKeyRepeatScreenshot) //スクリーンショット
	{	//前回押して、まだ押しっぱなしの場合
		if (!INPUT_CheckButtonState(_ScreenshotButton))
			_bKeyRepeatScreenshot = FALSE;
	}
	else
	{
		if ((ok)||(_ScreenshotButton >= 300))
			if (INPUT_CheckButtonState(_ScreenshotButton))
			{
				MAINBOARD_PauseNoMessage(TRUE);
				OutputScreenshot();
				run_emulator(TRUE);
				_bKeyRepeatScreenshot = TRUE;//キーリピートを防ぐために必要
			}
	}
	if (_bKeyRepeatPause) //ポーズ
	{	//前回押して、まだ押しっぱなしの場合
		if (!INPUT_CheckButtonState(_PauseButton))
			_bKeyRepeatPause = FALSE;
	}
	else
	{
		if ((ok)||(_PauseButton >= 300))
			if (INPUT_CheckButtonState(_PauseButton))
			{
				run_emulator(_bRunning ^ TRUE);
				_bKeyRepeatPause = TRUE;//キーリピートを防ぐために必要
			}
	}
	if (_bFuncVolume) //ファンクションボタンを押しながら十字キーの上下左右を押した場合、音量を調節する。v2.38追加
	{
		//ファンクションボタンを押しながら十字キーの上下を押した場合、音量を大まかに調節する。
		if (_bKeyRepeatVolumeChangeUD)
		{	//前回押して、まだ押しっぱなしの場合
			if (JOYPAD_GetConnectSixButton())
			{
				if ((!INPUT_CheckButtonState(_PcePad6[1].buttonU))&&(!INPUT_CheckButtonState(_PcePad6[1].buttonD)))
					_bKeyRepeatVolumeChangeUD = FALSE;
			}
			else if (JOYPAD_GetConnectThreeButton())
			{
				if ((!INPUT_CheckButtonState(_PcePad3[1].buttonU))&&(!INPUT_CheckButtonState(_PcePad3[1].buttonD)))
					_bKeyRepeatVolumeChangeUD = FALSE;
			}
			else //２ボタンパッド時
			{
				if ((!INPUT_CheckButtonState(_PcePad2[1].buttonU))&&(!INPUT_CheckButtonState(_PcePad2[1].buttonD)))
					_bKeyRepeatVolumeChangeUD = FALSE;
			}
		}
		else
		{
			if ((ok)||(_FunctionButton >= 300))
				if (INPUT_CheckButtonState(_FunctionButton))
				{
					if (JOYPAD_GetConnectSixButton())
					{
						if (INPUT_CheckButtonState(_PcePad6[1].buttonU)&&(_FunctionButton != _PcePad6[1].buttonU))
						{
							VolumeUp();
							_bKeyRepeatVolumeChangeUD = TRUE;//キーリピートを防ぐために必要
						}
						if (INPUT_CheckButtonState(_PcePad6[1].buttonD)&&(_FunctionButton != _PcePad6[1].buttonD))
						{
							VolumeDown();
							_bKeyRepeatVolumeChangeUD = TRUE;//キーリピートを防ぐために必要
						}
					}
					else if (JOYPAD_GetConnectThreeButton())
					{
						if (INPUT_CheckButtonState(_PcePad3[1].buttonU)&&(_FunctionButton != _PcePad3[1].buttonU))
						{
							VolumeUp();
							_bKeyRepeatVolumeChangeUD = TRUE;//キーリピートを防ぐために必要
						}
						if (INPUT_CheckButtonState(_PcePad3[1].buttonD)&&(_FunctionButton != _PcePad3[1].buttonD))
						{
							VolumeDown();
							_bKeyRepeatVolumeChangeUD = TRUE;//キーリピートを防ぐために必要
						}
					}
					else //２ボタンパッド時
					{
						if (INPUT_CheckButtonState(_PcePad2[1].buttonU)&&(_FunctionButton != _PcePad2[1].buttonU))
						{
							VolumeUp();
							_bKeyRepeatVolumeChangeUD = TRUE;//キーリピートを防ぐために必要
						}
						if (INPUT_CheckButtonState(_PcePad2[1].buttonD)&&(_FunctionButton != _PcePad2[1].buttonD))
						{
							VolumeDown();
							_bKeyRepeatVolumeChangeUD = TRUE;//キーリピートを防ぐために必要
						}
					}
				}
		}
		//ファンクションボタンを押しながら十字キーの左右を押した場合、音量を微調整する。
		if (_bKeyRepeatVolumeChangeLR)
		{	//前回押して、まだ押しっぱなしの場合
			if (JOYPAD_GetConnectSixButton())
			{
				if ((!INPUT_CheckButtonState(_PcePad6[1].buttonL))&&(!INPUT_CheckButtonState(_PcePad6[1].buttonR)))
					_bKeyRepeatVolumeChangeLR = FALSE;
			}
			else if (JOYPAD_GetConnectThreeButton())
			{
				if ((!INPUT_CheckButtonState(_PcePad3[1].buttonL))&&(!INPUT_CheckButtonState(_PcePad3[1].buttonR)))
					_bKeyRepeatVolumeChangeLR = FALSE;
			}
			else //２ボタンパッド時
			{
				if ((!INPUT_CheckButtonState(_PcePad2[1].buttonL))&&(!INPUT_CheckButtonState(_PcePad2[1].buttonR)))
					_bKeyRepeatVolumeChangeLR = FALSE;
			}
		}
		else
		{
			if ((ok)||(_FunctionButton >= 300))
				if (INPUT_CheckButtonState(_FunctionButton))
				{
					if (JOYPAD_GetConnectSixButton())
					{
						if (INPUT_CheckButtonState(_PcePad6[1].buttonL)&&(_FunctionButton != _PcePad6[1].buttonL))
						{
							VolumeDetailDown();
							_bKeyRepeatVolumeChangeLR = TRUE;//キーリピートを防ぐために必要
						}
						if (INPUT_CheckButtonState(_PcePad6[1].buttonR)&&(_FunctionButton != _PcePad6[1].buttonR))
						{
							VolumeDetailUp();
							_bKeyRepeatVolumeChangeLR = TRUE;//キーリピートを防ぐために必要
						}
					}
					else if (JOYPAD_GetConnectThreeButton())
					{
						if (INPUT_CheckButtonState(_PcePad3[1].buttonL)&&(_FunctionButton != _PcePad3[1].buttonL))
						{
							VolumeDetailDown();
							_bKeyRepeatVolumeChangeLR = TRUE;//キーリピートを防ぐために必要
						}
						if (INPUT_CheckButtonState(_PcePad3[1].buttonR)&&(_FunctionButton != _PcePad3[1].buttonR))
						{
							VolumeDetailUp();
							_bKeyRepeatVolumeChangeLR = TRUE;//キーリピートを防ぐために必要
						}
					}
					else //２ボタンパッド時
					{
						if (INPUT_CheckButtonState(_PcePad2[1].buttonL)&&(_FunctionButton != _PcePad2[1].buttonL))
						{
							VolumeDetailDown();
							_bKeyRepeatVolumeChangeLR = TRUE;//キーリピートを防ぐために必要
						}
						if (INPUT_CheckButtonState(_PcePad2[1].buttonR)&&(_FunctionButton != _PcePad2[1].buttonR))
						{
							VolumeDetailUp();
							_bKeyRepeatVolumeChangeLR = TRUE;//キーリピートを防ぐために必要
						}
					}
				}
		}
	}
	if (_bFuncTurboButton) //ファンクションボタンを押しながらI,IIボタンを押した場合、連射モードを切り替える。v2.38追加
	{
		//ファンクションボタンを押しながらIボタン
		if (_bKeyRepeatTurboSelect1)
		{	//前回押して、まだ押しっぱなしの場合
			if (JOYPAD_GetConnectSixButton())
			{
				if (!INPUT_CheckButtonState(_PcePad6[1].button1))
					_bKeyRepeatTurboSelect1 = FALSE;
			}
			else if (JOYPAD_GetConnectThreeButton())
			{
				if (!INPUT_CheckButtonState(_PcePad3[1].button1))
					_bKeyRepeatTurboSelect1 = FALSE;
			}
			else //２ボタンパッド時
			{
				if (!INPUT_CheckButtonState(_PcePad2[1].button1))
					_bKeyRepeatTurboSelect1 = FALSE;
			}
		}
		else
		{
			if ((ok)||(_FunctionButton >= 300))
			{
				if (JOYPAD_GetConnectSixButton())
				{
					if ((INPUT_CheckButtonState(_FunctionButton))&&(INPUT_CheckButtonState(_PcePad6[1].button1))&&(_FunctionButton != _PcePad6[1].button1))
					{
						if (JOYPAD_GetSwapIandII()) //SWAP設定を考慮
							ToggleInputTurbo2();
						else
							ToggleInputTurbo1();
						_bKeyRepeatTurboSelect1 = TRUE;//キーリピートを防ぐために必要
					}
				}
				else if (JOYPAD_GetConnectThreeButton())
				{
					if ((INPUT_CheckButtonState(_FunctionButton))&&(INPUT_CheckButtonState(_PcePad3[1].button1))&&(_FunctionButton != _PcePad3[1].button1))
					{
						if (JOYPAD_GetSwapIandII()) //SWAP設定を考慮
							ToggleInputTurbo2();
						else
							ToggleInputTurbo1();
						_bKeyRepeatTurboSelect1 = TRUE;//キーリピートを防ぐために必要
					}
				}
				else //２ボタンパッド時
				{
					if ((INPUT_CheckButtonState(_FunctionButton))&&(INPUT_CheckButtonState(_PcePad2[1].button1))&&(_FunctionButton != _PcePad2[1].button1))
					{
						if (JOYPAD_GetSwapIandII()) //SWAP設定を考慮
							ToggleInputTurbo2();
						else
							ToggleInputTurbo1();
						_bKeyRepeatTurboSelect1 = TRUE;//キーリピートを防ぐために必要
					}
				}
			}
		}
		//ファンクションボタンボタンを押しながらIIボタン
		if (_bKeyRepeatTurboSelect2)
		{	//前回押して、まだ押しっぱなしの場合
			if (JOYPAD_GetConnectSixButton())
			{
				if (!INPUT_CheckButtonState(_PcePad6[1].button2))
					_bKeyRepeatTurboSelect2 = FALSE;
			}
			else if (JOYPAD_GetConnectThreeButton())
			{
				if (!INPUT_CheckButtonState(_PcePad3[1].button2))
					_bKeyRepeatTurboSelect2 = FALSE;
			}
			else //２ボタンパッド時
			{
				if (!INPUT_CheckButtonState(_PcePad2[1].button2))
					_bKeyRepeatTurboSelect2 = FALSE;
			}
		}
		else
		{
			if ((ok)||(_FunctionButton >= 300))
			{
				if (JOYPAD_GetConnectSixButton())
				{
					if ((INPUT_CheckButtonState(_FunctionButton))&&(INPUT_CheckButtonState(_PcePad6[1].button2))&&(_FunctionButton != _PcePad6[1].button2))
					{
						if (JOYPAD_GetSwapIandII()) //SWAP設定を考慮
							ToggleInputTurbo1();
						else
							ToggleInputTurbo2();
						_bKeyRepeatTurboSelect2 = TRUE;//キーリピートを防ぐために必要
					}
				}
				else if (JOYPAD_GetConnectThreeButton())
				{
					if ((INPUT_CheckButtonState(_FunctionButton))&&(INPUT_CheckButtonState(_PcePad3[1].button2))&&(_FunctionButton != _PcePad3[1].button2))
					{
						if (JOYPAD_GetSwapIandII()) //SWAP設定を考慮
							ToggleInputTurbo1();
						else
							ToggleInputTurbo2();
						_bKeyRepeatTurboSelect2 = TRUE;//キーリピートを防ぐために必要
					}
				}
				else //２ボタンパッド時
				{
					if ((INPUT_CheckButtonState(_FunctionButton))&&(INPUT_CheckButtonState(_PcePad2[1].button2))&&(_FunctionButton != _PcePad2[1].button2))
					{
						if (JOYPAD_GetSwapIandII()) //SWAP設定を考慮
							ToggleInputTurbo1();
						else
							ToggleInputTurbo2();
						_bKeyRepeatTurboSelect2 = TRUE;//キーリピートを防ぐために必要
					}
				}
			}
		}
	}
	if (_bFuncScreenshot) //ファンクションボタンボタンを押しながらSELECTボタンを押した場合、スクリーンショットを撮る。v2.38追加
	{
		if (_bKeyRepeatFuncScreenshot)
		{	//前回押して、まだ押しっぱなしの場合
			if (JOYPAD_GetConnectSixButton())
			{
				if (!INPUT_CheckButtonState(_PcePad6[1].buttonSel))
					_bKeyRepeatFuncScreenshot = FALSE;
			}
			else if (JOYPAD_GetConnectThreeButton())
			{
				if (!INPUT_CheckButtonState(_PcePad3[1].buttonSel))
					_bKeyRepeatFuncScreenshot = FALSE;
			}
			else //２ボタンパッド時
			{
				if (!INPUT_CheckButtonState(_PcePad2[1].buttonSel))
					_bKeyRepeatFuncScreenshot = FALSE;
			}
		}
		else if (!_bKeyRepeatScreenshot) //通常のスクリーンショットボタン処理との被りを回避。
		{
			if ((ok)||(_FunctionButton >= 300))
			{
				if (JOYPAD_GetConnectSixButton())
				{
					if ((INPUT_CheckButtonState(_FunctionButton))&&(INPUT_CheckButtonState(_PcePad6[1].buttonSel))&&(_FunctionButton != _PcePad6[1].buttonSel)) //※SWAP設定は考慮しない
					{
						MAINBOARD_PauseNoMessage(TRUE);
						OutputScreenshot();
						run_emulator(TRUE);
						_bKeyRepeatFuncScreenshot = TRUE;//キーリピートを防ぐために必要
					}
				}
				else if (JOYPAD_GetConnectThreeButton())
				{
					if ((INPUT_CheckButtonState(_FunctionButton))&&(INPUT_CheckButtonState(_PcePad3[1].buttonSel))&&(_FunctionButton != _PcePad3[1].buttonSel)) //※SWAP設定は考慮しない
					{
						MAINBOARD_PauseNoMessage(TRUE);
						OutputScreenshot();
						run_emulator(TRUE);
						_bKeyRepeatFuncScreenshot = TRUE;//キーリピートを防ぐために必要
					}
				}
				else //２ボタンパッド時
				{
					if ((INPUT_CheckButtonState(_FunctionButton))&&(INPUT_CheckButtonState(_PcePad2[1].buttonSel))&&(_FunctionButton != _PcePad2[1].buttonSel)) //※SWAP設定は考慮しない
					{
						MAINBOARD_PauseNoMessage(TRUE);
						OutputScreenshot();
						run_emulator(TRUE);
						_bKeyRepeatFuncScreenshot = TRUE;//キーリピートを防ぐために必要
					}
				}
			}
		}
	}
	if (_bFuncRecent) //ファンクションボタンを押しながらRUNボタンを押した場合、リーセントROMのロードを行える。v2.48追加
	{
		if (_bKeyRepeatFuncRecent)
		{	//前回押して、まだ押しっぱなしの場合
			if (JOYPAD_GetConnectSixButton())
			{
				if (!INPUT_CheckButtonState(_PcePad6[1].buttonRun))
					_bKeyRepeatFuncRecent = FALSE;
			}
			else if (JOYPAD_GetConnectThreeButton())
			{
				if (!INPUT_CheckButtonState(_PcePad3[1].buttonRun))
					_bKeyRepeatFuncRecent = FALSE;
			}
			else //２ボタンパッド時
			{
				if (!INPUT_CheckButtonState(_PcePad2[1].buttonRun))
					_bKeyRepeatFuncRecent = FALSE;
			}
		}
		else
		{
			if ((ok)||(_FunctionButton >= 300))
			{
				if (JOYPAD_GetConnectSixButton())
				{
					if ((INPUT_CheckButtonState(_FunctionButton))&&(INPUT_CheckButtonState(_PcePad6[1].buttonRun))&&(_FunctionButton != _PcePad6[1].buttonRun)) //※SWAP設定は考慮しない
					{
						FuncRecentRom();
						_bKeyRepeatFuncRecent = TRUE;//キーリピートを防ぐために必要
					}
				}
				else if (JOYPAD_GetConnectThreeButton())
				{
					if ((INPUT_CheckButtonState(_FunctionButton))&&(INPUT_CheckButtonState(_PcePad3[1].buttonRun))&&(_FunctionButton != _PcePad3[1].buttonRun)) //※SWAP設定は考慮しない
					{
						FuncRecentRom();
						_bKeyRepeatFuncRecent = TRUE;//キーリピートを防ぐために必要
					}
				}
				else //２ボタンパッド時
				{
					if ((INPUT_CheckButtonState(_FunctionButton))&&(INPUT_CheckButtonState(_PcePad2[1].buttonRun))&&(_FunctionButton != _PcePad2[1].buttonRun)) //※SWAP設定は考慮しない
					{
						FuncRecentRom();
						_bKeyRepeatFuncRecent = TRUE;//キーリピートを防ぐために必要
					}
				}
			}
		}
	}
	if (_bFuncVSpeedUp) //ファンクションボタンを押しながら早回し機能(VideoSpeedUp)のボタンを押した場合、早回しボタンの使用モードをを切り替える。v2.38追加
	{
		if (_bKeyRepeatSpeedChange)
		{	//前回押して、まだ押しっぱなしの場合
			if (!INPUT_CheckButtonState(_VideoSpeedUpButton))
				_bKeyRepeatSpeedChange = FALSE;
		}
		else
		{
			if ((ok)||(_FunctionButton >= 300))
			{
				if ((INPUT_CheckButtonState(_FunctionButton))&&(INPUT_CheckButtonState(_VideoSpeedUpButton))&&(_FunctionButton != _VideoSpeedUpButton))
				{
					APP_ToggleUseVideoSpeedUpButton();
					_bKeyRepeatSpeedChange = TRUE;//キーリピートを防ぐために必要
				}
			}
		}
	}
	if (_bKeyRepeatFuncSave) //ファンクションボタンを押しながらファンクション用セーブボタンを押した場合、ステートセーブを行える。v2.38追加
	{	//前回押して、まだ押しっぱなしの場合
		if (!INPUT_CheckButtonState(_FuncSaveStateButton))
			_bKeyRepeatFuncSave = FALSE;
	}
	else if (!_bKeyRepeatSaveState) //通常のステートセーブボタン(デフォルト[S]キー)との処理の被りを回避。
	{
		if ((ok)||(_FunctionButton >= 300))
		{
			if ((INPUT_CheckButtonState(_FunctionButton))&&(INPUT_CheckButtonState(_FuncSaveStateButton))&&(_FunctionButton != _FuncSaveStateButton))
			{
				MAINBOARD_PauseNoMessage(TRUE);
				save_state(0);
				run_emulator(TRUE);
				_bKeyRepeatFuncSave = TRUE;//キーリピートを防ぐために必要
			}
		}
	}
	if (_bKeyRepeatFuncLoad) //ファンクションボタンを押しながらファンクション用ロードボタンを押した場合、ステートロードを行える。v2.38追加
	{	//前回押して、まだ押しっぱなしの場合
		if (!INPUT_CheckButtonState(_FuncLoadStateButton))
			_bKeyRepeatFuncLoad = FALSE;
	}
	else if (!_bKeyRepeatLoadState) //通常のステートロードボタン(デフォルト[L]キー)との処理の被りを回避。
	{
		if ((ok)||(_FunctionButton >= 300))
		{
			if ((INPUT_CheckButtonState(_FunctionButton))&&(INPUT_CheckButtonState(_FuncLoadStateButton))&&(_FunctionButton != _FuncLoadStateButton))
			{
				MAINBOARD_PauseNoMessage(TRUE);
				load_state(0);
				run_emulator(TRUE);
				_bKeyRepeatFuncLoad = TRUE;//キーリピートを防ぐために必要
			}
		}
	}

	return APP_IDLE;
}


/*-----------------------------------------------------------------------------
	[APP_Deinit]
		アプリケーションの終了処理をおこないます。
-----------------------------------------------------------------------------*/
BOOL
APP_Deinit()
{
	char	fileName[MAX_PATH+1];

	if (MAINBOARD_GetSystemInit()) //Kitao追加。正常にシステムが起動できていた場合
	{
		MAINBOARD_Pause(TRUE); //Kitao追加。レジューム再開時の音の乱れを防ぐため、音を停止しておく。
		
		/* 記録・再生を終了させる */
		APP_OutputWavEnd(); //Kitao追加
		end_playrecord();
		end_recording();
		
		//Kitao追加。レジュームモードの場合、ステートセーブする。
		save_resume();
		
		CDROM_Stop(FALSE); //CDDAをストップ
		//if (_bCDSpeedDown)
		//	CDIF_SetSpeed(0); //CD-ROMのReadスピードを元に戻す。v2.50 ※テスト不足のため現在未使用。
		
		save_bram();
		
		MAINBOARD_Deinit();
	}

	AUDIOCONFIG_Deinit();
	DEBUG_Deinit();
	
	if (_PrevWindowsVolume != -2) //前のゲームを遊んでいたときに、音量がレジュームファイル読み込みにより変更されていた場合、音量設定を元に戻す。
		_WindowsVolume = _PrevWindowsVolume;
	CONFIG_Set("[APP SOUND] Windows Volume Control", &_WindowsVolume, sizeof(_WindowsVolume)); //v1.61追加
	CONFIG_Set("[APP SOUND] APU Master Volume", &_SoundApuMasterVolume, sizeof(_SoundApuMasterVolume));
	CONFIG_Set("[APP SOUND] PSG Volume", &_SoundPsgVolume, sizeof(_SoundPsgVolume));//Kitao追加。PSGも個別に音量設定できるようにした。
	CONFIG_Set("[APP SOUND] ADPCM Volume", &_SoundAdpcmVolume, sizeof(_SoundAdpcmVolume));
	CONFIG_Set("[APP SOUND] CD Volume", &_SoundCdVolume, sizeof(_SoundCdVolume));
	CONFIG_Set("[APP SYSTEM] Soft Version", &_SoftVersion, sizeof(_SoftVersion));//Kitao追加。設定データを保存したときのバージョンを記録しておく。

	/* 設定を保存する。 */
	if (save_config(_AppPath))
		PRINTF("configurations saved to \"" APP_CONFIG_FILENAME "\".");

	CONFIG_Deinit();

	delete_zipTempfile(); //Kitao追加。ZIP用テンポラリファイルを削除

	//WAVE音量を元に戻し、ボリュームミキサーを閉じる。
	if (_hMix)
	{
		if (_bWindowsVolumeChanged) //Windowsのデフォルトボリュームを使用し、一度もボリュームを変更していない場合は音量調整しない。v2.16更新
			setWaveVolume(-1);
		mixerClose(_hMix);
	}

	PRINTF_Deinit();
	SCREEN_Deinit();

	if (_bConfigReturnToDefault) //Kitao追加。設定をデフォルトに戻す場合。v1.03。
	{
		strcpy(fileName, _AppPath);
		strcat(fileName, "RecentRom.dat");
		remove(fileName); //RecentRomファイルを削除
		strcpy(fileName, _AppPath);
		strcat(fileName, "Folder.dat");
		remove(fileName); //Folderファイルを削除
		strcpy(fileName, _AppPath);
		strcat(fileName, "Ootake.dat");
		remove(fileName); //設定ファイルを削除
	}

	deinit_menu();

	/* イベント処理系の破棄はいちばん最後 */
	APPEVENT_Deinit();

	SetThreadExecutionState(ES_CONTINUOUS); //Kitao追加。スタンバイの無効を解除。v2.22

	//v2.21追加
	if (_bWindowsVista)
		FreeLibrary(_hDwmapiDll);

	RA_Shutdown();

	return TRUE;
}


//Kitao追加
void
APP_RunEmulator(
	BOOL		bRun)
{
	if (!_bInit) //Kitao追加。起動直後にWinMain.c から呼ばれた場合はスルー。
		return;
	run_emulator(bRun);
}


//Kitao追加
BOOL
APP_GetCDGame()
{
	return _bCDGame;
}

//Kitao追加。HuカードモードからCDゲームモードへ設定を切り替える。v2.40
void
APP_SetCDGame()
{
	_bCDGame = TRUE;
	strcpy(_GameFileNameBuf, ""); //_GameFileNameBufは、MAINBOARD_Initで使われるのでクリアしておく必要がある。
}

//Kitao追加。ゲーム名を_GameFileNameBufに設定しておく（ステートセーブ時等で利用）。v2.40
void
APP_SetGameFileNameBuf()
{
	SetGameFileNameBuf(_bCDGame, _bHesFile, _GameFilePathName, _GameFileNameBuf, FALSE);
}


//Kitao追加。ウィンドウの位置を保存。WinMain.cから使用。
void
APP_SaveWindowPosition()
{
	RECT	rc;
	RECT	rc2;
	double	w;

	if (_bFullScreen) return;//フルスクリーンのときは記憶しない
	if (IsIconic(WINMAIN_GetHwnd())) return;//アイコン化状態のときは記憶しない

	GetWindowRect(WINMAIN_GetHwnd(), &rc);
	if (!_bStretched) //ノンストレッチモードにしていた場合は、横の幅を256ドットゲーム時の状態に戻してからウィンドウ位置を保存する。
	{
		GetClientRect(WINMAIN_GetHwnd(), &rc2);
		rc.left += (rc2.right-rc2.left - 256*_Magnification) / 2;
	}
	if (_bVStretched) //縦画面モードにしていた場合は、通常モードの座標に戻してからウィンドウ位置を保存する。
	{
		w = (double)_ScreenWidth * (336.0/256.0);//256:336にしたものを336:256にして元に戻す
		rc.left = (int)(((double)rc.left+(double)_ScreenWidth/2.0) - w/2.0 + 0.5);
	}
	CONFIG_Set("[APP DISPLAY] Window Left", &rc.left, sizeof(rc.left));
	CONFIG_Set("[APP DISPLAY] Window Top", &rc.top, sizeof(rc.top));
}


//Kitao追加。V-SyncメニューをEnableまたは灰色に。ScreenDD.cから使用
void
APP_EnableVSyncMenu(
	BOOL screen60hz)
{
	MENU_EnableItem(_ScreenMenu, WM_SCREEN_SYNC_VBLANK, screen60hz);
	MENU_EnableItem(_AudioSubMenu, WM_AUDIO_SYNC_VBLANK, screen60hz);
}


//Kitao追加。エミュレータが動作中かどうか（メニューを表示中かどうか）を返す
BOOL
APP_GetRunning()
{
	return _bRunning; //※メニュー表示中なら_bRunningはFALSE。
}


//Kitao追加。アプリケーション(Ootake)のディレクトリを返す。
char*
APP_GetAppPath()
{
	return _AppPath;
}


//Kitao追加。使用するCDドライブのナンバーを返す。※0なら１台目。1なら２台目。
Sint32
APP_GetCurrentCdDrive()
{
	return _CurrentCdDrive;
}


//Kitao追加。PSGのボリュームを返す。
Uint32
APP_GetPsgVolume()
{
	return _SoundPsgVolume;
}

//Kitao追加。ADPCMのボリュームを返す。
Uint32
APP_GetAdpcmVolume()
{
	return _SoundAdpcmVolume;
}

//Kitao追加。CDのボリュームを返す。
Uint32
APP_GetCdVolume()
{
	return _SoundCdVolume;
}

//Kitao追加。起動時の連射スピード設定を返す。
Sint32
APP_GetRenshaSpeedSel()
{
	return _RenshaSpeedSel;
}

//Kitao追加。起動時の連射スピードを一時的に設定する。
void
APP_SetRenshaSpeedSel(
	Sint32	renshaSpeedSel)
{
	_RenshaSpeedSel = renshaSpeedSel;
}

//Kitao追加。プレイレコードを停止する
void
APP_EndRecording()
{
	end_recording();
}

//Kitao追加。プレイバックを停止する
void
APP_EndPlayRecord()
{
	end_playrecord();
}

//Kitao追加。現在フルスクリーン表示かどうかを返す。
BOOL
APP_GetFullScreen()
{
	return _bFullScreen;
}

//Kitao追加。ウィンドウ表示時に、メッセージを表示するかどうかを返す。
BOOL
APP_GetHideMessage()
{
	return _bHideMessage;
}

//Kitao追加。フルスクリーン表示時に、メッセージを表示するかどうかを返す。
BOOL
APP_GetFullHideMessage()
{
	return _bFullHideMessage;
}

//Kitao追加
void
APP_SetHideMessage(
	BOOL	bHideMessage)
{
	_bHideMessage = bHideMessage;
}

//Kitao追加
void
APP_SetFullHideMessage(
	BOOL	bFullHideMessage)
{
	_bFullHideMessage = bFullHideMessage;
}

//Kitao追加
BOOL
APP_GetDefaultFastCD()
{
	BOOL	bFastCD = TRUE;

	switch (_StartFastCD)
	{
		case 0:
			bFastCD = FALSE;
			break;
		case 2:
			bFastCD = _bFastCD;
			break;
	}

	return bFastCD;
}

//Kitao追加
BOOL
APP_GetDefaultFastSeek()
{
	BOOL	bFastSeek = TRUE;

	switch (_StartFastSeek)
	{
		case 0:
			bFastSeek = FALSE;
			break;
		case 2:
			bFastSeek = _bFastSeek;
			break;
	}

	return bFastSeek;
}

//Kitao追加
BOOL
APP_GetDefaultSpriteLimit()
{
	BOOL	bSpriteLimit = TRUE;

	switch (_StartSpriteLimit)
	{
		case 0:
			bSpriteLimit = FALSE;
			break;
		case 2:
			bSpriteLimit = _bSpriteLimit;
			break;
	}

	return bSpriteLimit;
}

//Kitao更新。ステートロード時にJoyPad.c からも使用する。
void
APP_SetInputConfiguration()
{
	int i;
	
	if (JOYPAD_GetConnectSixButton())
	{
		for (i=1; i<=5; i++)
			INPUT_ConnectButton(i, &_PcePad6[i]);
	}
	else if (JOYPAD_GetConnectThreeButton())
	{
		for (i=1; i<=5; i++)
			INPUT_ConnectButton(i, &_PcePad3[i]);
	}
	else
	{
		for (i=1; i<=5; i++)
			INPUT_ConnectButton(i, &_PcePad2[i]);
	}
}

//Kitao追加
char*
APP_GetGameFileNameBuf()
{
	return _GameFileNameBuf;
}

//Kitao追加
char*
APP_GetGameFilePathName()
{
	return _GameFilePathName;
}

//Kitao追加
void
APP_SetAppName(
	char*	pAppName)
{
	strcpy(_AppName, pAppName);
}

//Kitao追加
char*
APP_GetAppName()
{
	return _AppName;
}


//Kitao追加。更新された設定でスクリーンモードを変更する
BOOL
APP_UpdateScreenMode(
	BOOL	bWindowCenter)	//bWindowCenter…ウィンドウを中央位置に合わせる場合TRUEにして呼ぶ
{
	return updateScreenMode(bWindowCenter);
}


//v2.64追加
static void
check640FullScan()
{
	//640x480のフルスクリーン＆フルストレッチ＆スキャンライン表示の場合、画質維持のためフルストレッチ(低倍率拡大)せず、オーバースキャンを表示することで画面を一杯にする。
	if ((((_FullMagnification == 0)&&((_CustomWidth1 != 640)||(_CustomHeight1 != 480)))||(_FullMagnification == 2))&&
		(_bFullScreen)&&(_bFullStretched)&&((_ScanLineType == 2)||(_bTvMode)))
	{
		MAINBOARD_SetShowOverscanTop(8);
		MAINBOARD_SetShowOverscanBottom(8);
		MAINBOARD_SetShowOverscanLeft(8);
		MAINBOARD_SetShowOverscanRight(8);
	}
	else
	{
		MAINBOARD_SetShowOverscanTop(_ShowOverscanTop);
		MAINBOARD_SetShowOverscanBottom(_ShowOverscanBottom);
		MAINBOARD_SetShowOverscanLeft(_ShowOverscanLeft);
		MAINBOARD_SetShowOverscanRight(_ShowOverscanRight);
	}
}

//Kitao追加。_bStretched,_bVStretched,_bFullStretchedを設定する。MainBorad.c からステートロード時にも使用。CDROM.c からも使用。
void
APP_SetStretched(
	BOOL	bStretched,
	BOOL	bVStretched,
	BOOL	bFullStretched)
{
	BOOL	bPrevStretched = _bStretched;
	BOOL	bPrevVStretched = _bVStretched;
	BOOL	bPrevFullStretched = _bFullStretched;

	_bStretched = bStretched;
	_bVStretched = bVStretched;
	_bFullStretched = bFullStretched;

	if	((_bStretched != bPrevStretched)||(_bVStretched != bPrevVStretched)||(_bFullStretched != bPrevFullStretched)) //設定が前回と異なる場合のみ、画面を再設定する。
	{
		updateMenu(); //メニュー表示を更新
		MAINBOARD_SetStretched(_bStretched, _bVStretched); //ストレッチ用の変数を更新
		if (!_bFullScreen)
			updateScreenMode(TRUE); //ウィンドウモードのときだけ、ウィンドウの大きさを変える必要があるため画面モードを再設定する。
		else
		{
			check640FullScan(); //v2.64追加
			MAINBOARD_ResetPrevTvW(); //表示範囲外になった部分のゴミをクリアするために必要。
		}
	}
	SCREEN_SetGamma(APP_GetScanLineType(), _ScanLineDensity, _bTvMode); //ガンマを更新。フルストレッチ→他のストレッチ(TV-mode)のときに必要。
}

//Kitao追加
BOOL
APP_GetStretched()
{
	return _bStretched;
}

//Kitao追加
BOOL
APP_GetFullStretched(
	BOOL	bFilter) //bFilter=TRUEなら、640x480のフルスクリーンでスキャンラインの場合、フルストレッチを常にFALSEで返す。
{
	if (!bFilter)
		return _bFullStretched;

	if ((((_FullMagnification == 0)&&((_CustomWidth1 != 640)||(_CustomHeight1 != 480)))||(_FullMagnification == 2))&&
		(_bFullScreen)&&((_ScanLineType == 2)||(_bTvMode))) //640x480のフルスクリーンでスキャンラインの場合、画質維持(低倍率拡大をNG)のためフルストレッチをせずオーバースキャン表示にて対応。v2.64
		return FALSE;
	else
		return _bFullStretched;
}

//Kitao追加
BOOL
APP_GetVStretched()
{
	return _bVStretched;
}

//v2.64追加
void
APP_SetStartStretchMode(
	Sint32	startStretchMode)
{
	_StartStretchMode = startStretchMode;
	switch (_StartStretchMode)
	{
		case 0:
			APP_SetStretched(FALSE, FALSE, FALSE);
			break;
		case 2:
			APP_SetStretched(TRUE, FALSE, TRUE);
			break;
		default: //1
			APP_SetStretched(TRUE, FALSE, FALSE);
			break;
	}
}

//v2.64追加
Sint32
APP_GetStartStretchMode()
{
	return _StartStretchMode;
}

//Kitao追加
BOOL
APP_GetSyncTo60HzScreen()
{
	return _bSyncTo60HzScreen;
}

//Kitao追加。Startup.cpp から使用。
void
APP_SetSyncTo60HzScreen(
	BOOL	bSyncTo60HzScreen)
{
	_bSyncTo60HzScreen = bSyncTo60HzScreen;
	CONFIG_Set("[APP DISPLAY] Sync To 60Hz Fullscreen", &_bSyncTo60HzScreen, sizeof(_bSyncTo60HzScreen));
}

//Kitao追加。v2.41
BOOL
APP_GetForceVSync()
{
	return _bForceVSync;
}

//Kitao追加
Sint32
APP_GetScanLineType()
{
	return _ScanLineType;
}

//Kitao追加。Startup.cpp，Screen.cppから使用。
void
APP_SetScanLineType(
	Sint32	scanLineType,
	BOOL	configWrite)
{
	_ScanLineType = scanLineType;
	if (configWrite)
		CONFIG_Set("[APP DISPLAY] Scanline Type", &_ScanLineType, sizeof(_ScanLineType));
}

//Kitao追加
Sint32
APP_GetScanLineDensity()
{
	return _ScanLineDensity;
}

//Kitao追加。v2.35
BOOL
APP_GetOptimizeGamma()
{
	return _bOptimizeGamma;
}

//Kitao追加
double
APP_GetGammaValue()
{
	return _GammaValue;
}

//Kitao追加
Sint32
APP_GetBrightValue()
{
	return _BrightValue;
}

//Kitao追加
void
APP_SetTvMode(
	BOOL	bTvMode,
	BOOL	screenUpdate)
{
	BOOL	bStretched;

	_bTvMode = bTvMode;
	SCREEN_SetGamma(APP_GetScanLineType(), _ScanLineDensity, _bTvMode); //ガンマも更新する
	check640FullScan(); //v2.64追加

	if (screenUpdate)
	{
		MENU_CheckItem(_SizeMenu, WM_SCREEN_TV, _bTvMode);
		if (_bFullScreen)
			SCREEN_Clear(); //フルスクリーンの場合、画面にゴミが残らないようクリアしておく
		if (_bTvMode)
		{
			if (!_bStretched) //TV Modeでノンストレッチだった場合、ストレッチモードに変更する。
				APP_SetStretched(TRUE, _bVStretched, _bFullStretched);
		}
		else
		{
			//TV Modeを解除した場合、ストレッチモードを設定ファイルから読み出して元に戻す。
			CONFIG_Get("[APP DISPLAY] Stretched Mode", &bStretched, sizeof(bStretched));
			APP_SetStretched(bStretched, _bVStretched, _bFullStretched); //bStretchedが_bStretchedと違えば画面が更新される
		}
	}
}

//Kitao追加
BOOL
APP_GetTvMode()
{
	return _bTvMode;
}

//Kitao追加。v2.28
Sint32
APP_GetMonoColorMode()
{
	return _MonoColorMode;
}

//Kitao追加
void
APP_SetUseVideoSpeedUpButton(
	BOOL	bUseVideoSpeedUpButton)
{
	_bUseVideoSpeedUpButton = bUseVideoSpeedUpButton;
}

//Kitao追加
BOOL
APP_GetUseVideoSpeedUpButton()
{
	return _bUseVideoSpeedUpButton;
}

//Kitao追加
void
APP_ToggleUseVideoSpeedUpButton()
{
	if (_bUseVideoSpeedUpButton)
		always_video_speed_up();
	else
		use_video_speed_up_button();
}

//Kitao追加
void
APP_MenuShow(
	BOOL	bShow)
{
	if (bShow)
		MENU_Show(_Menu);
	else
		MENU_Show(NULL);
}

//Kitao追加
BOOL
APP_GetWindows9x()
{
	return _bWindows9x;
}

//Kitao追加
BOOL
APP_GetWindowsVista()
{
	return _bWindowsVista;
}

//Kitao追加
BOOL
APP_GetWindows7()
{
	return _bWindows7;
}

//Kitao追加
BOOL
APP_GetPauseNoRelease()
{
	return _bPauseNoRelease;
}

//Kitao追加。マウスを自動的に隠すためのカウンタをリセットする。
void
APP_ResetMouseStopCount()
{
	_MouseStopCount = 60*2; //2秒(120フレーム)。
}

//Kitao追加
void
APP_SetF1NoReset(
	BOOL	bF1NoReset)
{
	_bF1NoReset = bF1NoReset;
}

//Kitao追加
BOOL
APP_GetF1NoReset()
{
	return	_bF1NoReset;
}

//Kitao追加
void
APP_UpdateMenu()
{
	updateMenu();
}

//Kitao追加。installフォルダをマイコンピュータで開く。MainBoard.c からも使用。
void
APP_OpenInstallFolder()
{
	char	fn[MAX_PATH+1];

	strcpy(fn, _AppPath);
	strcat(fn, "install");
	CreateDirectory(fn, NULL);//installディレクトリがない場合作る
	if (_VirtualStorePath[0] != 0) //v0.95追加。v2.20更新。Vista/7で"Program Files"にインストールした場合、セーブファイルがリダイレクトされているのでリダイレクト先のフォルダを開くようにする。
	{
		strcpy(fn, _VirtualStorePath);
		strcat(fn, "install");
	}
	ShellExecute(NULL, "open", fn, NULL, NULL, SW_SHOW);
}

//Kitao追加。saveフォルダをマイコンピュータで開く。
void
APP_OpenSaveFolder()
{
	char	fn[MAX_PATH+1];

	strcpy(fn, _AppPath);
	strcat(fn, "save");
	CreateDirectory(fn, NULL);//saveディレクトリがない場合作る
	if (_VirtualStorePath[0] != 0) //v0.95追加。v2.20更新。Vista/7で"Program Files"にインストールした場合、セーブファイルがリダイレクトされているのでリダイレクト先のフォルダを開くようにする。
	{
		strcpy(fn, _VirtualStorePath);
		strcat(fn, "save");
	}
	ShellExecute(NULL, "open", fn, NULL, NULL, SW_SHOW);
}

//Kitao追加
void
APP_SetStartFullScreen(
	BOOL	bStartFullScreen)
{
	_bStartFullScreen = bStartFullScreen;
	CONFIG_Set("[APP DISPLAY] Full Screen Mode", &_bStartFullScreen, sizeof(_bStartFullScreen));
}

//Kitao追加
BOOL
APP_GetStartFullScreen()
{
	return	_bStartFullScreen;
}

//Kitao追加
void
APP_SetSoundType(
	Sint32	soundType)
{
	_SoundType = soundType;
	CONFIG_Set("[APP SOUND] Sound Initialize Type", &_SoundType, sizeof(_SoundType));
}

//Kitao追加
Sint32
APP_GetSoundType()
{
	return	_SoundType;
}

//Kitao追加
BOOL
APP_GetAutoStereo()
{
	return	_bAutoStereo;
}

//Kitao追加
BOOL
APP_GetAutoGradiusII()
{
	return	_bAutoGradiusII;
}

//Kitao追加
BOOL
APP_GetAutoShinMegamiTensei()
{
	return	_bAutoShinMegamiTensei;
}

//Kitao追加
BOOL
APP_GetOutputWavNext()
{
	return	_bOutputWavNext;
}

//Kitao追加。WAV保存用のファイル名(パスは含まず)だけを返す。AudioOut.c からも使用。
char*
APP_GetWavFileName()
{
	return	extractFileName(_WavFilePathName);
}

//Kitao追加
BOOL
APP_GetUseVideoCardMemory()
{
	if ((_bUseSystemMemoryOnlyWindow)&&(_bFullScreen))
		return TRUE;
	else
		return _bUseVideoCardMemory;
}

//Kitao追加
BOOL
APP_GetWindowTopMost()
{
	return _bWindowTopMost;
}

//Kitao追加。v2.48
void
APP_SetWindowTopMostTemp(
	BOOL	bWindowTopMost)
{
	_bWindowTopMost = bWindowTopMost;
	if (_bFullScreen)
		WINMAIN_SetFullScreenWindow(_ScreenWidth, _ScreenHeight);
	else
		WINMAIN_SetNormalWindow(_ScreenWidth, _ScreenHeight);
}

//Kitao追加
Sint32
APP_GetCustomWidth1()
{
	return _CustomWidth1;
}
Sint32
APP_GetCustomHeight1()
{
	return _CustomHeight1;
}
Sint32
APP_GetCustomWidth2()
{
	return _CustomWidth2;
}
Sint32
APP_GetCustomHeight2()
{
	return _CustomHeight2;
}
Sint32
APP_GetCustomWidth3()
{
	return _CustomWidth3;
}
Sint32
APP_GetCustomHeight3()
{
	return _CustomHeight3;
}

//Kitao追加
BOOL
APP_GetResolutionAutoChange()
{
	return _bResolutionAutoChange;
}

//Kitao追加
BOOL
APP_AutoChangeScreenMode(
	Sint32	magnification)
{
	return change_screen_mode(magnification, _bFullScreen, _FullScreenColor, TRUE);
}

//Kitao追加
Sint32
APP_GetSoundBufferSize()
{
	return _SoundBufferSize;
}

//Kitao追加。Startup.cpp から使用。
void
APP_SetSoundBufferSize(
	Sint32	bufSize)
{
	_SoundBufferSize = bufSize;
	CONFIG_Set("[APP SOUND] Sound Buffer Size", &_SoundBufferSize, sizeof(_SoundBufferSize));
}

//Kitao追加
Sint32
APP_GetPsgQuality()
{
	return _PsgQuality;
}

//Kitao追加。Startup.cpp からも使用。
void
APP_SetPsgQuality(
	Sint32	psgQuality)
{
	_PsgQuality = psgQuality;
	CONFIG_Set("[APP SOUND] PSG Quality", &_PsgQuality, sizeof(_PsgQuality));
}

//Kitao追加
BOOL
APP_GetJoypadBackground()
{
	return _bJoypadBackground;
}

//Kitao追加。v2.48
void
APP_SetJoypadBackgroundTemp(
	BOOL	bJoypadBackground)
{
	_bJoypadBackground = bJoypadBackground; //CONFIGファイルにはセーブしない
	INPUT_Deinit();
	INPUT_Init();
	APP_SetInputConfiguration();
}

//Kitao追加
BOOL
APP_GetKeyboardBackground()
{
	return _bKeyboardBackground;
}

//Kitao追加
BOOL
APP_GetInit()
{
	return _bInit;
}

//Kitao追加
void
APP_SetShowOverscanTop(
	Sint32	showOverscanTop)
{
	_ShowOverscanTop = showOverscanTop;
}

//Kitao追加
void
APP_SetShowOverscanBottom(
	Sint32	showOverscanBottom)
{
	_ShowOverscanBottom = showOverscanBottom;
}

//Kitao追加
void
APP_SetShowOverscanLeft(
	Sint32	showOverscanLeft)
{
	_ShowOverscanLeft = showOverscanLeft;
}

//Kitao追加
void
APP_SetShowOverscanRight(
	Sint32	showOverscanRight)
{
	_ShowOverscanRight = showOverscanRight;
}

//Kitao追加
void
APP_SetShowOverscanHeight(
	Sint32	showOverscanHeight)
{
	_ShowOverscanHeight = showOverscanHeight;
}

//Kitao追加
BOOL
APP_GetOverscanHideBlackTop()
{
	if ((_bFullScreen)||(!_bStretched)||(_StartShowOverscanTop == 0))
		return FALSE; //フルスクリーンモード時,ノンストレッチ時は常にFALSEを返す。
	else
		return _bOverscanHideBlackBelt;
}

//Kitao追加
BOOL
APP_GetOverscanHideBlackBottom()
{
	if ((_bFullScreen)||(!_bStretched)||(_StartShowOverscanBottom == 0))
		return FALSE; //フルスクリーンモード時,ノンストレッチ時は常にFALSEを返す。
	else
		return _bOverscanHideBlackBelt;
}

//Kitao追加
BOOL
APP_GetOverscanHideBlackLR()
{
	if ((_bFullScreen)||(!_bStretched)||(_StartShowOverscanLeft == 0))
		return FALSE; //フルスクリーンモード時,ノンストレッチ時は常にFALSEを返す。
	else
		return _bOverscanHideBlackBelt;
}

//Kitao追加
Sint32
APP_GetWindowsVolume()
{
	return _WindowsVolume;
}

//Kitao追加。レジュームファイルを読み出している場合、音量を復元する。※通常のステートロード時には復元しない。
//			 現在、APU.c のステートロード部分から使用。
void
APP_ResumeWindowsVolume(
	Sint32	windowsVolume)
{
	if ((_bLoadingResumeFile)&&(windowsVolume != -1)) //※"Use Windows Default"(=-1)のときには復元しない。
	{
		_PrevWindowsVolume = _WindowsVolume;//レジュームで変更される前の音量を退避しておく。次に他のゲームに切り替えたときには、この値に戻す。
		_WindowsVolume = windowsVolume;
		setWaveVolume(_WindowsVolume);
	}
}

//Kitao追加
Sint32
APP_GetMySetOverClockType()
{
	return _MySetOverClockType;
}

//Kitao追加
Uint32
APP_GetVolumeEffect()
{
	return _VolumeEffect;
}

//Kitao追加
void
APP_ReturnCaption()
{
	if ((!_bFullScreen)&&(_bHideMessage))
		PRINTF(_AppName); //キャプションをノーマルに戻す。
	else
		PRINTF("");
}

//Kitao追加
BOOL
APP_GetShowFPS()
{
	return _bShowFPS;
}

//Kitao追加
void
APP_SetShowFPS(
	BOOL	bShowFPS)
{
	_bShowFPS = bShowFPS;
}

//Kitao追加
Sint32
APP_GetDrawMethod()
{
	return _DrawMethod;
}

//Kitao追加。Startup.cpp から使用。
void
APP_SetDrawMethod(
	Sint32	drawMethod)
{
	_DrawMethod = drawMethod;
	CONFIG_Set("[APP DISPLAY] Draw Method", &_DrawMethod, sizeof(_DrawMethod));
}

//Kitao追加
Sint32
APP_GetD3DEffect()
{
	if (_bTvMode)
		return _D3DEffectTV; //TVモードのとき
	else
		return _D3DEffectRGB; //TVモード以外のとき
}

//Kitao追加
Sint32
APP_GetScreenWidth()
{
	return _ScreenWidth;
}

//Kitao追加
Sint32
APP_GetScreenHeight()
{
	return _ScreenHeight;
}

//Kitao追加
void
APP_SetScreenWH(
	Sint32	screenW,
	Sint32	screenH)
{
	_ScreenWidth = screenW;
	_ScreenHeight = screenH;
}

//Kitao追加。Startup.cpp から使用。
void
APP_SetFullScreen640()
{
	_FullMagnification = 2; //640x480
	CONFIG_Set("[APP DISPLAY] FullMagnification", &_FullMagnification, sizeof(_FullMagnification));
	_bResolutionAutoChange = FALSE; //オートチェンジを解除
	CONFIG_Set("[APP DISPLAY] Resolution AutoChange", &_bResolutionAutoChange, sizeof(_bResolutionAutoChange));
}

//Kitao追加。Startup.cpp から使用。
void
APP_SetMagnification(
	Sint32	magnification)
{
	_Magnification = magnification;
	CONFIG_Set("[APP DISPLAY] Magnification", &_Magnification, sizeof(_Magnification));
}

//Kitao追加
BOOL
APP_GetSpriteLayer()
{
	return _bSpriteLayer;
}

//Kitao追加
BOOL
APP_GetSprite2Layer()
{
	return _bSprite2Layer;
}

//Kitao追加
BOOL
APP_GetBGLayer()
{
	return _bBGLayer;
}

//Kitao追加
BOOL
APP_GetBG2Layer()
{
	return _bBG2Layer;
}

//Kitao追加
BOOL
APP_GetInvalidateCdInstall()
{
	return _bInvalidateCdInstall;
}

//Kitao追加。獣王記で使用。v2.07
char*
APP_ChangeToOtherSysCard1()
{
	FILE*	fp;

	_OtherSysCard = 1;
	set_openFilePathName(_OtherSysCardPath1, _OpenFilePathName, _GameFilePathName); //zipファイルだった場合にも対応
	if ((fp = fopen(_OpenFilePathName, "r")) != NULL) //システムカードのイメージファイルが存在すれば
		fclose(fp);
	else //システムカードのイメージファイルが存在しなければ
	{
		//システムカードのイメージファイルを指定してもらう
		if (!file_dialog(3, WINMAIN_GetHwnd(), _OpenFilePathName, _GameFilePathName))
			return NULL; //失敗した場合
	}
	return _OpenFilePathName; //成功
}

//Kitao追加。v2.15
Sint32
APP_GetInputRecordMode()
{
	return _InputRecordMode;
}

//Kitao追加。v2.15
Sint32
APP_GetInputPlayRecordNumber()
{
	return _InputPlayRecordNumber;
}

//Kitao追加。レコード記録中か再生中の場合TRUEを返す。v2.15
BOOL
APP_CheckRecordingNow()
{
	if ((_InputRecordingNumber != -1)||(_InputPlayRecordNumber != -1))
		return TRUE;
	else
		return FALSE;	
}

//Kitao追加
Sint32
APP_GetSoftVersion()
{
	Sint32	softVersion;
	char	c[4];

	c[0] = _SoftVersion[0];
	c[1] = _SoftVersion[2];
	c[2] = _SoftVersion[3];
	c[3] = 0;
	softVersion = atoi(c);
	return softVersion;
}

//Kitao追加。v2.21
BOOL
APP_GetDisableWindowsAero()
{
	return _bDisableWindowsAero;
}

//Kitao追加。v2.21
BOOL
APP_DwmIsCompositionEnabled()
{
	BOOL	bEnabled;// = FALSE;

	_FuncDwmIsCompositionEnabled(&bEnabled);
	return bEnabled;
}

//Kitao追加。v2.24
BOOL
APP_GetCueFile()
{
	return _bCueFile;
}

//Kitao追加。v2.24
char*
APP_GetCueFilePathName()
{
	return _CueFilePathName;
}

//Kitao追加。v2.26
BOOL
APP_GetInactivePause()
{
	return _bInactivePause;
}

//Kitao追加。v2.32
Sint32
APP_GetCddaAdjust()
{
	Sint32	cddaAdjust;

	if (_bFullScreen)
		cddaAdjust = _CddaAdjustFull[_FullMagnification];
	else
		cddaAdjust = _CddaAdjust;
	switch (cddaAdjust)
	{
		case 5930: return   -75;
		case 5935: return  -112; //音が早いのは良くないので四捨五入せず切り捨ての-112。
		case 5940: return  -150;
		case 5945: return  -225;
		case 5950: return  -300;
		case 5955: return  -450;
		case 5960: return  -600; //-600フレームに１度足踏みしてテンポを遅らせる。
		case 5965: return  -900;
		case 5970: return -1200; //-1200フレームに１度足踏みしてテンポを遅らせる。
		case 5975: return     0; //基準
		case 5980: return  1200; //1200フレームに１度スキップしてテンポを早める。
		case 5985: return   900;
		case 5990: return   600; //600フレームに１度スキップしてテンポを早める。
		case 5995: return   450;
		case 6000: return   300;
		case 6005: return   225;
		case 6010: return   150;
		case 6015: return   113; //音が早いのは良くないので四捨五入せず切り上げの113。
		case 6020: return    75; 
		case 6025: return    57; //音が早いのは良くないので四捨五入せず切り上げの57。
		case 6030: return    38; //現在非使用。音が早いと良くないので四捨五入は切り上げて(遅いほうよりの)38。※ここまで補正してしまうと音のキー変化が目立つので、多少の遅れより60.2Hzを利用したほうがいいのでカット。
	}
	return 0;
}

//Kitao追加。v2.35
double
APP_GetCddaAdjustHz()
{
	if (_bFullScreen)
		return ((double)_CddaAdjustFull[_FullMagnification] / 100);
	else
		return ((double)_CddaAdjust / 100);
}

//Kitao追加。v2.33
Sint32
APP_GetCddaDelayFrame()
{
	return _CddaDelayFrame;
}

//Kitao追加。v2.36
BOOL
APP_GetLoadStateSpeedSetting()
{
	if (_fpPlayRecord != NULL) //プレイレコードを再生中だった場合
		return TRUE; //プレイレコード時は必ず速度設定も反映させる。v2.38
	else
		return _bLoadStateSpeedSetting;
}

//Kitao追加。v2.38
Sint32
APP_GetFunctionButton()
{
	return _FunctionButton;
}

//Kitao追加。v2.39
BOOL
APP_ShowWriteMemoryForm(
	HWND		hWnd,
	HINSTANCE	hInstance)
{
	Sint32		setOk = 0;
	MSG			msg;
	Uint32		mpr;
	Uint32		addr;
	Uint8		data;
	BOOL		bContinuous;
	BOOL		bWindowTopMostR;

	//レジュームモードの場合、レジュームが解除されることを確認する。
	if (non_resume_ok() == FALSE)
		return FALSE;

	//レジュームモードだった場合、一時的に解除する。
	save_resume(); //レジューム一時停止前に、ここまでの状態をレジュームファイルに保存。
	non_resume_temporarily(); //メニュー表示も更新される。

	_bPauseNoRelease = TRUE; //アクティブになってもポーズを解除しないようにする。
	bWindowTopMostR = _bWindowTopMost;
	APP_SetWindowTopMostTemp(FALSE); //DirectDrawでフルスクリーン時に必要。Direct3DはなくてもOK。
	WRITEMEM_Init(hWnd, hInstance, &mpr, &addr, &data, &bContinuous, &setOk);
	//メッセージループ
	while (setOk == 0) //ダイアログが結果を返すまでループ
	{ 
		GetMessage(&msg ,NULL, 0, 0);
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	APP_SetWindowTopMostTemp(bWindowTopMostR);
	_bPauseNoRelease = FALSE;

	if (setOk == 1) //設定が成功した場合
	{
		CPU_WriteMemoryMpr(mpr, addr, data, bContinuous);
		PRINTF("WriteMemory [ %s ]", WRITEMEM_GetCode());
		PRINTF_SetSaveLoadMessage(); //PRINTFメッセージの表示時間を少し短くする
		//PRINTF("WMEM Test [ %X, %X, %d, %d ]", mpr, addr, data, bContinuous); //Test用
		return TRUE;
	}
	else if (setOk == 2) //空欄で設定完了した場合
	{
		MAINBOARD_SetContinuousWriteValue(0, 0, 0, FALSE); //Continuousしていた場合解除される。
		PRINTF("Canceled WriteMemory Continuous.");
		PRINTF_SetSaveLoadMessage(); //PRINTFメッセージの表示時間を少し短くする
		return TRUE;
	}
	else
		return FALSE;
}

//Kitao追加。v2.42
BOOL
APP_GetHesFile()
{
	return _bHesFile;
}

//Kitao追加。v2.43
Sint32
APP_GetNonVSyncTiming()
{
	return _NonVSyncTiming;
}

//Kitao追加。v2.48
char*
APP_GetRecentRom(
	Sint32	n)
{
	SetGameFileNameBuf(FALSE,FALSE, _RecentRom[n], _RecentRomTmp, TRUE);
	return _RecentRomTmp;
}

//Kitao追加。v2.48
BOOL
APP_CheckButtonState(
	Sint32	n)
{
	JOYPAD_SetRenshaSpeedButton1(0); //連射を一時強制オフ
	JOYPAD_SetRenshaSpeedButton2(0);
	JOYPAD_SetRenshaSpeedButtonRun(0);
	INPUT_UpdateState(FALSE); //FALSE=記録・再生はしない
	JOYPAD_SetRenshaSpeedButton1(_RenshaSpeedSel); //連射復帰
	JOYPAD_SetRenshaSpeedButton2(_RenshaSpeedSel);
	JOYPAD_SetRenshaSpeedButtonRun(_RenshaSpeedSel);

	switch (n)
	{
		case 2: //DOWN
			if (JOYPAD_GetConnectSixButton())
				return INPUT_CheckButtonState(_PcePad6[1].buttonD);
			else if (JOYPAD_GetConnectThreeButton())
				return INPUT_CheckButtonState(_PcePad3[1].buttonD);
			else //２ボタンパッド時
				return INPUT_CheckButtonState(_PcePad2[1].buttonD);
		case 4: //LEFT
			if (JOYPAD_GetConnectSixButton())
				return INPUT_CheckButtonState(_PcePad6[1].buttonL);
			else if (JOYPAD_GetConnectThreeButton())
				return INPUT_CheckButtonState(_PcePad3[1].buttonL);
			else //２ボタンパッド時
				return INPUT_CheckButtonState(_PcePad2[1].buttonL);
		case 6: //RIGHT
			if (JOYPAD_GetConnectSixButton())
				return INPUT_CheckButtonState(_PcePad6[1].buttonR);
			else if (JOYPAD_GetConnectThreeButton())
				return INPUT_CheckButtonState(_PcePad3[1].buttonR);
			else //２ボタンパッド時
				return INPUT_CheckButtonState(_PcePad2[1].buttonR);
		case 8: //UP
			if (JOYPAD_GetConnectSixButton())
				return INPUT_CheckButtonState(_PcePad6[1].buttonU);
			else if (JOYPAD_GetConnectThreeButton())
				return INPUT_CheckButtonState(_PcePad3[1].buttonU);
			else //２ボタンパッド時
				return INPUT_CheckButtonState(_PcePad2[1].buttonU);
		case 11: //Iボタン
			if (JOYPAD_GetConnectSixButton())
				return INPUT_CheckButtonState(_PcePad6[1].button1);
			else if (JOYPAD_GetConnectThreeButton())
				return INPUT_CheckButtonState(_PcePad3[1].button1);
			else //２ボタンパッド時
				return INPUT_CheckButtonState(_PcePad2[1].button1);
		case 12: //IIボタン
			if (JOYPAD_GetConnectSixButton())
				return INPUT_CheckButtonState(_PcePad6[1].button2);
			else if (JOYPAD_GetConnectThreeButton())
				return INPUT_CheckButtonState(_PcePad3[1].button2);
			else //２ボタンパッド時
				return INPUT_CheckButtonState(_PcePad2[1].button2);
		case 17: //SELECTボタン
			if (JOYPAD_GetConnectSixButton())
				return INPUT_CheckButtonState(_PcePad6[1].buttonSel);
			else if (JOYPAD_GetConnectThreeButton())
				return INPUT_CheckButtonState(_PcePad3[1].buttonSel);
			else //２ボタンパッド時
				return INPUT_CheckButtonState(_PcePad2[1].buttonSel);
		case 18: //RUNボタン
			if (JOYPAD_GetConnectSixButton())
				return INPUT_CheckButtonState(_PcePad6[1].buttonRun);
			else if (JOYPAD_GetConnectThreeButton())
				return INPUT_CheckButtonState(_PcePad3[1].buttonRun);
			else //２ボタンパッド時
				return INPUT_CheckButtonState(_PcePad2[1].buttonRun);
	}
	return FALSE;
}

//Kitao追加。v2.61
//ファンクションボタンとSELECTボタンが同じボタンに設定されていた場合FALSEを返す。通常はTRUE(問題なし)を返す。RecentRom.cppから使用。
BOOL
APP_CheckFuncAndSelConflict()
{
	if (JOYPAD_GetConnectSixButton())
		return (_FunctionButton != _PcePad6[1].buttonSel);
	else if (JOYPAD_GetConnectThreeButton())
		return (_FunctionButton != _PcePad3[1].buttonSel);
	else //２ボタンパッド時
		return (_FunctionButton != _PcePad2[1].buttonSel);
}

//Kitao追加。v2.50
BOOL
APP_GetCDSpeedDown()
{
	return _bCDSpeedDown;
}

//Kitao追加。v2.59
BOOL
APP_GetCheckSimultaneouslyPush()
{
	return _bCheckSimultaneouslyPush;
}

//Kitao追加。v2.63
void
APP_CheckStateTime(
	Sint32	num,
	char*	pBufS,
	char*	pBufL)
{
	checkStateTime(num, pBufS, pBufL);
}

//Kitao追加。レジュームファイルをロード中かどうかを返す。v2.63
BOOL
APP_GetLoadingResumeFile()
{
	return _bLoadingResumeFile;
}

//Kitao追加。v2.64
Sint32
APP_GetFullMagnification()
{
	return _FullMagnification;
}

//Kitao追加。v2.64
Sint32
APP_GetShowOverscanTop()
{
	return _ShowOverscanTop;
}
//Kitao追加。v2.64
Sint32
APP_GetShowOverscanBottom()
{
	return _ShowOverscanBottom;
}
//Kitao追加。v2.64
Sint32
APP_GetShowOverscanLeft()
{
	return _ShowOverscanLeft;
}
//Kitao追加。v2.64
Sint32
APP_GetShowOverscanRight()
{
	return _ShowOverscanRight;
}

//Kitao追加。v2.65
BOOL
APP_GetWindowsAutoVSyncBool()
{
	if ((_WindowsAutoVSync == 1)||
		((_WindowsAutoVSync == 2)&&(_bFullScreen)))
		return TRUE;
	else
		return FALSE;
}

//Kitao追加。v2.65
Sint32
APP_GetVSyncAdjust()
{
	return _VSyncAdjust;
}

