/******************************************************************************
	[App.h]
		アプリケーションのインタフェイスを定義します．

	Copyright (C) 2004 Ki

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
******************************************************************************/
#ifndef APP_H_INCLUDED
#define APP_H_INCLUDED

#include "TypeDefs.h"

#define	APP_CONFIG_FILENAME	"Ootake.dat" //Kitao更新

//Kitao追加
#ifndef SPI_GETFOREGROUNDLOCKTIMEOUT
#define SPI_GETFOREGROUNDLOCKTIMEOUT 0x2000
#endif
#ifndef SPI_SETFOREGROUNDLOCKTIMEOUT
#define SPI_SETFOREGROUNDLOCKTIMEOUT 0x2001
#endif


enum AppEvent
{
	APP_IDLE,
	APP_QUIT,
};

BOOL
APP_Init(
	int		argc,
	char**	argv);

Sint32
APP_ProcessEvents();

BOOL
APP_Deinit();

//Kitao追加
void
APP_RunEmulator(
	BOOL	bRun);

//Kitao追加
BOOL
APP_GetCDGame();

//Kitao追加
void
APP_SetCDGame();

//Kitao追加
void
APP_SetGameFileNameBuf();

//Kitao追加。ウィンドウの位置を保存。WinMain.cからも使用。
void
APP_SaveWindowPosition();

//Kitao追加。V-SyncメニューをEnableまたは灰色に。
void
APP_EnableVSyncMenu(
	BOOL screen60hz);

//Kitao追加。メニューを表示中かどうかを返す
BOOL
APP_GetRunning();

//Kitao追加。アプリケーション(Ootake)のパスを返す。
char*
APP_GetAppPath();

//Kitao追加。使用するCDドライブのナンバーを返す。※0なら１台目。1なら２台目。
Sint32
APP_GetCurrentCdDrive();

//Kitao追加。PSGのボリュームを返す。
Uint32
APP_GetPsgVolume();

//Kitao追加。ADPCMのボリュームを返す。
Uint32
APP_GetAdpcmVolume();

//Kitao追加。CDのボリュームを返す。
Uint32
APP_GetCdVolume();

//Kitao追加
void
APP_SetStretched(
	BOOL	bStretched,
	BOOL	bVStretched,
	BOOL	bFullStretched);

//Kitao追加
void
APP_SetStartStretchMode(
	Sint32	startStretchMode);

//v2.64追加
Sint32
APP_GetStartStretchMode();

//Kitao追加
void
APP_TurboCycleMenu(
	BOOL	bAuto);

//Kitao追加
void
APP_SetAutoOverClock(
	Sint32	n);

//Kitao追加
void
APP_SetSpeedNormal();

//Kitao追加
Sint32
APP_GetRenshaSpeedSel();

//Kitao追加
void
APP_SetRenshaSpeedSel(
	Sint32	renshaSpeedSel);

//Kitao追加
void
APP_EndRecording();

//Kitao追加
void
APP_EndPlayRecord();

//Kitao追加
BOOL
APP_GetFullScreen();

//Kitao追加
BOOL
APP_GetHideMessage();

//Kitao追加
BOOL
APP_GetFullHideMessage();

//Kitao追加
void
APP_SetHideMessage(
	BOOL	bHideMessage);

//Kitao追加
void
APP_SetFullHideMessage(
	BOOL	bFullHideMessage);

//Kitao追加
BOOL
APP_GetDefaultFastCD();

//Kitao追加
BOOL
APP_GetDefaultFastSeek();

//Kitao追加
BOOL
APP_GetDefaultSpriteLimit();

//Kitao追加
void
APP_SetInputConfiguration();

//Kitao追加
char*
APP_GetGameFileNameBuf();

//Kitao追加
char*
APP_GetGameFilePathName();

//Kitao追加
void
APP_SetAppName(
	char*	pAppName);

//Kitao追加
char*
APP_GetAppName();

//Kitao追加
BOOL
APP_GetStretched();

//Kitao追加
BOOL
APP_GetVStretched();

//Kitao追加
BOOL
APP_GetFullStretched(
	BOOL	bFilter);

//Kitao追加
BOOL
APP_GetSyncTo60HzScreen();

//Kitao追加
void
APP_SetSyncTo60HzScreen(
	BOOL	bSyncTo60HzScreen);

//Kitao追加
BOOL
APP_GetForceVSync();

//Kitao追加
Sint32
APP_GetScanLineType();

//Kitao追加
void
APP_SetScanLineType(
	Sint32	scanLineType,
	BOOL	configWrite);

//Kitao追加
Sint32
APP_GetScanLineDensity();

//Kitao追加
BOOL
APP_GetOptimizeGamma();

//Kitao追加
double
APP_GetGammaValue();

//Kitao追加
Sint32
APP_GetBrightValue();

//Kitao追加
void
APP_SetTvMode(
	BOOL	bTvMode,
	BOOL	screenUpdate);

//Kitao追加
BOOL
APP_GetTvMode();

//Kitao追加
Sint32
APP_GetMonoColorMode();

//Kitao追加
void
APP_SetUseVideoSpeedUpButton(
	BOOL	bUseVideoSpeedUpButton);

//Kitao追加
BOOL
APP_GetUseVideoSpeedUpButton();

//Kitao追加
void
APP_ToggleUseVideoSpeedUpButton();

//Kitao追加
void
APP_MenuShow(
	BOOL	bShow);

//Kitao追加
void
APP_SetForegroundWindowOotake();

//Kitao追加
BOOL
APP_GetWindows9x();

//Kitao追加
BOOL
APP_GetWindowsVista();

//Kitao追加
BOOL
APP_GetWindows7();

//Kitao追加
BOOL
APP_GetPauseNoRelease();

//Kitao追加
void
APP_ResetMouseStopCount();

//Kitao追加
void
APP_SetF1NoReset(
	BOOL	bF1NoReset);

//Kitao追加
BOOL
APP_GetF1NoReset();

//Kitao追加
void
APP_UpdateMenu();

//Kitao追加
void
APP_OpenInstallFolder();

//Kitao追加
void
APP_OpenSaveFolder();

//Kitao追加
void
APP_SetStartFullScreen(
	BOOL	bStartFullScreen);

//Kitao追加
BOOL
APP_GetStartFullScreen();

//Kitao追加
void
APP_SetSoundType(
	Sint32	soundType);

//Kitao追加
Sint32
APP_GetSoundType();

//Kitao追加
BOOL
APP_GetAutoStereo();

//Kitao追加
BOOL
APP_GetAutoGradiusII();

//Kitao追加
BOOL
APP_GetAutoShinMegamiTensei();

//Kitao追加
BOOL
APP_OutputWavEnd();

//Kitao追加
BOOL
APP_GetOutputWavNext();

//Kitao追加
char*
APP_GetWavFileName();

//Kitao追加
BOOL
APP_GetUseVideoCardMemory();

//Kitao追加
BOOL
APP_GetWindowTopMost();

//Kitao追加。v2.48
void
APP_SetWindowTopMostTemp(
	BOOL	bWindowTopMost);

//Kitao追加
Sint32
APP_GetCustomWidth1();
Sint32
APP_GetCustomHeight1();
Sint32
APP_GetCustomWidth2();
Sint32
APP_GetCustomHeight2();
Sint32
APP_GetCustomWidth3();
Sint32
APP_GetCustomHeight3();

//Kitao追加
BOOL
APP_GetResolutionAutoChange();

//Kitao追加
BOOL
APP_AutoChangeScreenMode(
	Sint32	magnification);

//Kitao追加
Sint32
APP_GetSoundBufferSize();

//Kitao追加
void
APP_SetSoundBufferSize(
	Sint32	bufSize);

//Kitao追加
Sint32
APP_GetPsgQuality();

//Kitao追加
void
APP_SetPsgQuality(
	Sint32	psgQuality);

//Kitao追加
BOOL
APP_GetJoypadBackground();

//Kitao追加。v2.48
void
APP_SetJoypadBackgroundTemp(
	BOOL	bJoypadBackground);

//Kitao追加
BOOL
APP_GetKeyboardBackground();

//Kitao追加
BOOL
APP_UpdateScreenMode(
	BOOL	bWindowCenter);

//Kitao追加
BOOL
APP_GetInit();

//Kitao追加
void
APP_SetShowOverscanTop(
	Sint32	showOverscanTop);

//Kitao追加
void
APP_SetShowOverscanBottom(
	Sint32	showOverscanBottom);

//Kitao追加
void
APP_SetShowOverscanLeft(
	Sint32	showOverscanLeft);

//Kitao追加
void
APP_SetShowOverscanRight(
	Sint32	showOverscanRight);

//Kitao追加
void
APP_SetShowOverscanHeight(
	Sint32	showOverscanHeight);

//Kitao追加
BOOL
APP_GetOverscanHideBlackTop();

//Kitao追加
BOOL
APP_GetOverscanHideBlackBottom();

//Kitao追加
BOOL
APP_GetOverscanHideBlackLR();

//Kitao追加
Sint32
APP_GetWindowsVolume();

//Kitao追加
void
APP_ResumeWindowsVolume(
	Sint32	windowsVolume);

//Kitao追加
Sint32
APP_GetMySetOverClockType();

//Kitao追加
Uint32
APP_GetVolumeEffect();

//Kitao追加
void
APP_ReturnCaption();

//Kitao追加
char*
APP_StrToLower(
	char*	pStr);

//Kitao追加
char*
APP_ExtractFileExt(
	char*	p);

//Kitao追加
BOOL
APP_GetShowFPS();

//Kitao追加
void
APP_SetShowFPS(
	BOOL	bShowFPS);

//Kitao追加
Sint32
APP_GetDrawMethod();

//Kitao追加
void
APP_SetDrawMethod(
	Sint32	drawMethod);

//Kitao追加
Sint32
APP_GetD3DEffect();

//Kitao追加
Sint32
APP_GetScreenWidth();

//Kitao追加
Sint32
APP_GetScreenHeight();

//Kitao追加
void
APP_SetScreenWH(
	Sint32	screenW,
	Sint32	screenH);

//Kitao追加
void
APP_SetFullScreen640();

//Kitao追加
void
APP_SetMagnification(
	Sint32	magnification);

//Kitao追加
BOOL
APP_GetSpriteLayer();

//Kitao追加
BOOL
APP_GetSprite2Layer();

//Kitao追加
BOOL
APP_GetBGLayer();

//Kitao追加
BOOL
APP_GetBG2Layer();

//Kitao追加
BOOL
APP_GetInvalidateCdInstall();

//Kitao追加
char*
APP_ChangeToOtherSysCard1();

//Kitao追加
Sint32
APP_GetNonstretchedWidth();

//Kitao追加
Sint32
APP_GetGameWidth(
	Sint32	magnification);

//Kitao追加
Sint32
APP_GetGameHeight(
	Sint32	magnification);

//Kitao追加
Sint32
APP_GetInputRecordMode();

//Kitao追加
Sint32
APP_GetInputPlayRecordNumber();

//Kitao追加
BOOL
APP_CheckRecordingNow();

//Kitao追加
Sint32
APP_GetSoftVersion();

//Kitao追加
BOOL
APP_GetDisableWindowsAero();

//Kitao追加
BOOL
APP_DwmIsCompositionEnabled();

//Kitao追加
void
APP_WindowWithinScreen();

//Kitao追加
void
APP_ToggleFullscreen();

//Kitao追加
BOOL
APP_GetCueFile();

//Kitao追加
char*
APP_GetCueFilePathName();

//Kitao追加
BOOL
APP_GetInactivePause();

//Kitao追加
Sint32
APP_GetCddaAdjust();

//Kitao追加
double
APP_GetCddaAdjustHz();

//Kitao追加。v2.33
Sint32
APP_GetCddaDelayFrame();

//Kitao追加。v2.36
BOOL
APP_GetLoadStateSpeedSetting();

//Kitao追加。v2.38
Sint32
APP_GetFunctionButton();

//Kitao追加。v2.39
BOOL
APP_ShowWriteMemoryForm(
	HWND		hWnd,
	HINSTANCE	hInstance);

//Kitao追加。v2.42
BOOL
APP_GetHesFile();

//Kitao追加。v2.43
Sint32
APP_GetNonVSyncTiming();

//Kitao追加。v2.48
char*
APP_GetRecentRom(
	Sint32	n);

//Kitao追加。v2.48
BOOL
APP_CheckButtonState(
	Sint32	n);

//Kitao追加。v2.61
BOOL
APP_CheckFuncAndSelConflict();

//Kitao追加。v2.50
BOOL
APP_GetCDSpeedDown();

//Kitao追加。v2.59
BOOL
APP_GetCheckSimultaneouslyPush();

//Kitao追加。v2.63
void
APP_CheckStateTime(
	Sint32	num,
	char*	pBufS,
	char*	pBufL);

//Kitao追加。v2.63
BOOL
APP_GetLoadingResumeFile();

//Kitao追加。v2.64
Sint32
APP_GetFullMagnification();

//Kitao追加。v2.64
Sint32
APP_GetShowOverscanTop();
//Kitao追加。v2.64
Sint32
APP_GetShowOverscanBottom();
//Kitao追加。v2.64
Sint32
APP_GetShowOverscanLeft();
//Kitao追加。v2.64
Sint32
APP_GetShowOverscanRight();

//Kitao追加。v2.65
BOOL
APP_GetWindowsAutoVSyncBool();

//Kitao追加。v2.65
Sint32
APP_GetVSyncAdjust();


#endif // APP_H_INCLUDED
