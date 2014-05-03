/******************************************************************************
Ootake
・DirectInputのアクセス権の取得を、ウィンドウがアクティブになったときに１度だ
  けおこなうようにした。（高速化）
・PAINT処理時のイベント発生を省略した。v0.95。（高速化）
・Windowsのボリュームミキサーは、他のアプリへの影響も出てしまうため使用しないよ
  うにした。
・ショートカットキーを追加・変更した。ステートセーブとロードは、逆押しをしない
  ように頭文字をとって'S'と'L'にした。
・マウスホイールの回転操作でセレクトボタン(上回転)・ランボタン(した回転)を機能
  させるようにした。ホイール(真ん中ボタン)をクリックすることでも、ランボタンを
  機能させるようにした。
・ウィンドウモード時に、常に手前に表示するようにした。
・起動時は横棒状態で起動するようにした。

Copyright(C)2006-2010 Kitao Nakamura.
    Attach the source code when you open the remodeling version and the
    succession version to the public. and, please contact me by E-mail.
    Business use is prohibited.
	Additionally, it applies to "GNU General Public License". 
	改造版・後継版を公開なさるときは必ずソースコードを添付してください。
	その際に事後でかまいませんので、ひとことお知らせいただけると幸いです。
	商的な利用は禁じます。
	あとは「GNU General Public License(一般公衆利用許諾契約書)」に準じます。

*******************************************************************************
	[WinMain.c]

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

#define _WIN32_WINNT 0x0400 //WM_MOUSEWHEEL使用のため必要
#define DIRECTINPUT_VERSION	0x0800	//Kitao追加。環境にもよるかもしれないが、DirectInput5が軽い。7だとやや遅延あり。スペースハリアーがわかりやすい。

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "WinMain.h"
#include "resource.h"
#include "AppEvent.h"
#include "MainBoard.h"
#include "AudioConfig.h"
#include "App.h"
#include "Input.h"
#include "Printf.h"

//	##RA
#include "RA_Resource.h"
#include "RA_Interface.h"

static BOOL _bBreakTrap; //Kitao追加。デバッグ用

static HINSTANCE	_hInstance;
static HWND			_hMainWnd;

static char			_Caption[2048]; //Kitao追加。現在表示中のキャプション


static int
init_argc(
	LPSTR		lpCmdLine)
{
	int		argc = 1;

	while (lpCmdLine != NULL)
	{
		while (*lpCmdLine == ' ') // ' 'はスキップする 
			++lpCmdLine;

		if (strlen(lpCmdLine) > 0)
			++argc;
		else
			break;

		// " を発見した場合は次の " までを１つの argument とする 
		if (*lpCmdLine == '"')
		{
			++lpCmdLine;

			// " をもう一つみつける 
			if (strchr(lpCmdLine, '"') != NULL)
				lpCmdLine = strchr(lpCmdLine, '"') + 1; // " をスキップする
			else
			{
				// みつからなかった場合は argument なしとみなす 
				argc = 1;
				break;
			}
		}
		else
		{
			if (strchr(lpCmdLine, ' ') != NULL)
				lpCmdLine = strchr(lpCmdLine, ' ');
			else
				break;
		}
	}

	return argc;
}


static char**
init_argv(
	int			argc,
	LPSTR		lpCmdLine)
{
	char**		ppArgv;
	char*		p;
	int			i;

	if (argc == 0)
		return NULL;

	ppArgv = (char**)GlobalAlloc(GMEM_FIXED, sizeof(char*) * argc);

	if (ppArgv == NULL)
		exit(-1);


	for (i = 0; i < argc; i++)
	{
		ppArgv[i] = (char*)GlobalAlloc(GMEM_FIXED, MAX_PATH+1);

		if (i == 0)
			GetModuleFileName(NULL, ppArgv[i], MAX_PATH+1); //argv[0]
		else
		{
			// ' ' はスキップする 
			while (*lpCmdLine == ' ')
				++lpCmdLine;

			// " を発見した場合は次の " までを argument とする 
			if (*lpCmdLine == '"')
			{
				p = lpCmdLine + 1;
				strcpy(ppArgv[i], p);

				// 次の " を探す 
				if ((p = strchr(ppArgv[i], '"')) != NULL)
				{
					*p = '\0';
					// " ２個分スキップする 
					lpCmdLine += strlen(ppArgv[i]) + 2;
				}
				else
				{
					// 次の " がなかった場合はループをぬける 
					// (argc の初期化で検査済みなのでここにはこないはず) 
					break;
				}
			}
			else
			{
				strcpy(ppArgv[i], lpCmdLine);

				if ((p = strchr(ppArgv[i], ' ')) != NULL)
					*p = '\0';

				lpCmdLine += strlen(ppArgv[i]);
			}
		}
	}

	return ppArgv;
}


static void
set_fullscreen_windowstyle(
	HWND	hWnd,
	int		width,
	int		height)
{
	SetWindowLong(hWnd, GWL_STYLE, GetWindowLong(hWnd, GWL_STYLE) & ~(WS_MINIMIZEBOX | WS_SYSMENU | WS_CAPTION | WS_DLGFRAME)); //Kitao更新
	SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, width, height, SWP_FRAMECHANGED | SWP_NOCOPYBITS | SWP_NOREDRAW | SWP_DEFERERASE | SWP_NOSENDCHANGING); //Kitao更新。フルスクリーン切り替え時の速度アップ。v2.23。v2.65
}


static void
set_normal_windowstyle(
	HWND	hWnd,
	int		width,
	int		height)
{
	RECT		rc;
	DWORD		dwStyle;
	int			x;
	int			y;

	dwStyle = GetWindowLong(hWnd, GWL_STYLE) | (WS_MINIMIZEBOX | WS_SYSMENU | WS_CAPTION | WS_DLGFRAME); //Kitao更新。ウィンドウサイズ変更不可に。
	SetWindowLong(hWnd, GWL_STYLE, dwStyle);

	GetWindowRect(hWnd, &rc);
	x = rc.left;
	y = rc.top;

	SetRect(&rc, 0, 0, width, height);

	AdjustWindowRect(&rc, GetWindowLong(hWnd, GWL_STYLE), GetMenu(hWnd) != NULL);
	if (APP_GetWindowTopMost())
		SetWindowPos(hWnd, HWND_TOPMOST, x, y, rc.right - rc.left, rc.bottom - rc.top, SWP_FRAMECHANGED); //Kitao更新。常に手前に表示。
	else
		SetWindowPos(hWnd, HWND_NOTOPMOST, x, y, rc.right - rc.left, rc.bottom - rc.top, SWP_FRAMECHANGED);
}


static void
cancelPause()
{
	RECT	rc;

	if (MAINBOARD_GetPause() == FALSE)
		return;

	if (APP_GetPauseNoRelease()) 
		return;

	if (!APP_GetFullScreen()) //ウィンドウ表示の場合
		MAINBOARD_ScreenUpdate(TRUE); //スクリーンを再描画

	if (APP_GetRunning()) //メニューが表示されていない場合
	{
		GetWindowRect(_hMainWnd, &rc);
		if ((rc.right <= GetSystemMetrics(SM_CXSCREEN))&&
			(rc.bottom <= GetSystemMetrics(SM_CYSCREEN))) //右側と下側がはみ出していなければ
		{
			if ((rc.left < 0)&&(rc.top < 0)) //左側も上側もはみ出していた場合
				MoveWindow(_hMainWnd, 0, 0,
							rc.right-rc.left, rc.bottom-rc.top, TRUE);//ウィンドウの位置を変える
			else
			if (rc.left < 0) //左側がはみ出していた場合
				MoveWindow(_hMainWnd, 0, rc.top,
							rc.right-rc.left, rc.bottom-rc.top, TRUE);//ウィンドウの位置を変える
			else
			if (rc.top < 0)
				MoveWindow(_hMainWnd, rc.left, 0,
							rc.right-rc.left, rc.bottom-rc.top, TRUE);

			MAINBOARD_Pause(FALSE);
		}
	}
}

static LRESULT CALLBACK
wnd_proc(
	HWND		hWnd,
	UINT		uMsg,
	WPARAM		wParam,
	LPARAM		lParam)
{
	short		delta;
	Sint32		a;
	HWND		hP;

	switch(uMsg)
	{

	case WM_ACTIVATE:
		if (wParam == 0) 
		{
			if ((APP_GetDrawMethod() == 2)&&(APP_GetFullScreen()))
			{
				hP = GetWindow((HWND)lParam, GW_OWNER);
				if (hP != _hMainWnd)
				{
					APP_ToggleFullscreen();
					APP_RunEmulator(FALSE);
				}
			}
			if (APP_GetInactivePause())
				if (!MAINBOARD_GetPause())
					MAINBOARD_Pause(TRUE);
		}
		else
		{
			INPUT_Acquire();
			if (IsIconic(hWnd) == FALSE)
				cancelPause();
		}
		break;


	case WM_ENTERSIZEMOVE:
		if (!MAINBOARD_GetPause())
			MAINBOARD_Pause(TRUE);
		break;

	case WM_EXITSIZEMOVE:
		if (!APP_GetFullScreen())
		{
			APP_SaveWindowPosition();
			cancelPause();
		}
		break;

	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_F1:
			if (GetKeyState(VK_CONTROL) < 0)
				APPEVENT_Set(APPEVENT_CDCHANGE, NULL); //CD Play
			else //通常
			{
				if (APP_GetF1NoReset())
					APPEVENT_Set(APPEVENT_CHANGE_CDC, NULL);
				else
				{
					if ((APP_GetCDGame())&&(!APP_GetCueFile()))
						APPEVENT_Set(APPEVENT_CDCHANGE, NULL);
					else
						APPEVENT_Set(APPEVENT_RESET, NULL);
				}
			}
			break;
		case VK_ESCAPE:		APPEVENT_Set(APPEVENT_RUN, NULL);					break;//[Esc]キー Kitao更新
		case 'O':			APPEVENT_Set(APPEVENT_FILEDIALOG, NULL);			break;//[O]キー Kitao更新
		case 'P':			APPEVENT_Set(APPEVENT_CD_PLAYINSTALL, NULL);		break;//[P]キー Kitao更新
		//case 'S':			APPEVENT_Set(APPEVENT_SAVESTATE, NULL);				break;//[S]キー Kitao更新
		//case 'L':			APPEVENT_Set(APPEVENT_LOADSTATE, NULL);				break;//[L]キー Kitao更新
		//case VK_F4:		APPEVENT_Set(APPEVENT_PARTIALMEMORYLOAD, NULL);		break;
		case VK_F4:			APPEVENT_Set(APPEVENT_SHOW_DEBUG, NULL);			break;//[F4]でDEBUGウィンドウを表示 Kitao更新
		case VK_F5:			APPEVENT_Set(APPEVENT_RECORDING, NULL);				break;//[F5]でRECORD Kitao更新
		case VK_F6:			APPEVENT_Set(APPEVENT_PLAYRECORD, NULL);			break;//[F6]でPLAYBACK Kitao更新
		case VK_F8: //[F8]で音量MUTE。v1.63
				if (APP_GetVolumeEffect() == 0) //すでにMUTEだった場合ノーマルに戻す。
				{
					APP_ReturnCaption(); //メッセージを消去
					APPEVENT_Set(APPEVENT_VOLUME_NORMAL, NULL);
				}
				else
				{
					PRINTF("Mute Volume. ([F8]=Return)");
					APPEVENT_Set(APPEVENT_VOLUME_MUTE, NULL);
				}
				break;
		case VK_F9:			APPEVENT_Set(APPEVENT_SCREEN_MINIMIZE, NULL);		break;//Kitao追加
		case VK_F11: //Kitao追加
			if (GetKeyState(VK_CONTROL) < 0) //Ctrlキーを押しながらの場合
				APPEVENT_Set(APPEVENT_SCREEN_TV, NULL); //TVモード切替
			else //通常
				APPEVENT_Set(APPEVENT_SCREEN_SHOWOVERSCAN, NULL); //オーバースキャン領域表示切替
			break;
		case VK_F12:
			if (GetKeyState(VK_CONTROL) < 0) //Ctrlキーを押しながらの場合
			{	//"Windows Aero"(Win7/Vista)の無効・有効切替。v2.21追加
				if (APP_GetDisableWindowsAero())
					APPEVENT_Set(APPEVENT_SCREEN_USEAERO, NULL);
				else
					APPEVENT_Set(APPEVENT_SCREEN_DISABLEAERO, NULL);
			}
			else //通常
				APPEVENT_Set(APPEVENT_SCREEN_FULLSCREEN, NULL); //フルスクリーン切替
			break;
		case 'R': //Kitao追加。[R]キー
			keybd_event(VK_MENU, 0, 0, 0);//Altキー
			keybd_event('R', 0, 0, 0);
			keybd_event('R', 0, 0x0002, 0); //0x0002…KEYEVENTF_KEYUP
			keybd_event(VK_MENU, 0, 0x0002, 0); //0x0002…KEYEVENTF_KEYUP
			break;
		case VK_PRIOR: //Kitao追加。[PageUp]キー
			//音量アップ
			if (GetKeyState(VK_CONTROL) < 0) //Ctrlキーを押しながらの場合
				APPEVENT_Set(APPEVENT_VOLUME_DETAILUP, NULL); //細かく音量調整
			else //通常
				APPEVENT_Set(APPEVENT_VOLUME_UP, NULL);
			break;
		case VK_NEXT: //Kitao追加。[PageDown]キー
			//音量ダウン
			if (GetKeyState(VK_CONTROL) < 0) //Ctrlキーを押しながらの場合
				APPEVENT_Set(APPEVENT_VOLUME_DETAILDN, NULL); //細かく音量調整
			else //通常
				APPEVENT_Set(APPEVENT_VOLUME_DOWN, NULL);
			break;
		case VK_HOME: //Kitao追加。[Home]キー
			//Videoスピードをアップ
			APPEVENT_Set(APPEVENT_SPEED_VUP, NULL);
			break;
		case VK_END: //Kitao追加。[End]キー
			//Videoスピードをダウン
			APPEVENT_Set(APPEVENT_SPEED_VDOWN, NULL);
			break;
		case VK_INSERT: //Kitao追加。[Insert]キー
			APP_ToggleUseVideoSpeedUpButton();
			break;
		case VK_DELETE: //Kitao追加。[Delete]キー
			if (GetKeyState(VK_SHIFT) < 0) //Shiftキーを押しながらの場合
			{	//倍率を上げる。最高速の次はノーマル。
				switch (VDC_GetOverClockType())
				{
					case   0:	APPEVENT_Set(APPEVENT_SPEED_P1, NULL);	break;
					case   1:	APPEVENT_Set(APPEVENT_SPEED_P2, NULL);	break;
					case   2:	APPEVENT_Set(APPEVENT_SPEED_P3, NULL);	break;
					case   3:	APPEVENT_Set(APPEVENT_SPEED_P4, NULL);	break;
					case   4:	APPEVENT_Set(APPEVENT_SPEED_P5, NULL);	break;
					case   5:	APPEVENT_Set(APPEVENT_SPEED_P6, NULL);	break;
					case   6:	APPEVENT_Set(APPEVENT_SPEED_T1, NULL);	break;
					case 100:	APPEVENT_Set(APPEVENT_SPEED_T2, NULL);	break;
					case 200:	APPEVENT_Set(APPEVENT_SPEED_T3, NULL);	break;
					case 300:	APPEVENT_Set(APPEVENT_SPEED_P0, NULL);	break;
				}
			}
			else if (GetKeyState(VK_CONTROL) < 0) //Ctrlキーを押しながらの場合
			{	//倍率を下げる。ノーマルの次は最高速。
				switch (VDC_GetOverClockType())
				{
					case   0:	APPEVENT_Set(APPEVENT_SPEED_T3, NULL);	break;
					case   1:	APPEVENT_Set(APPEVENT_SPEED_P0, NULL);	break;
					case   2:	APPEVENT_Set(APPEVENT_SPEED_P1, NULL);	break;
					case   3:	APPEVENT_Set(APPEVENT_SPEED_P2, NULL);	break;
					case   4:	APPEVENT_Set(APPEVENT_SPEED_P3, NULL);	break;
					case   5:	APPEVENT_Set(APPEVENT_SPEED_P4, NULL);	break;
					case   6:	APPEVENT_Set(APPEVENT_SPEED_P5, NULL);	break;
					case 100:	APPEVENT_Set(APPEVENT_SPEED_P6, NULL);	break;
					case 200:	APPEVENT_Set(APPEVENT_SPEED_T1, NULL);	break;
					case 300:	APPEVENT_Set(APPEVENT_SPEED_T2, NULL);	break;
				}
			}
			else //通常押し
			{	//マイセッティングした速度に切り替える。すでにセッティングした速度になっていた場合はノーマルに戻す。v1.61
				
				if (VDC_GetOverClockType() == APP_GetMySetOverClockType())
				{
					if (VDC_GetAutoOverClock() != -1) //自動でオーバークロックしているゲームの場合。v2.20
						a = VDC_GetAutoOverClock();
					else
						a = 0; //ノーマルに戻す
				}
				else
					a = APP_GetMySetOverClockType();
				switch (a)
				{
					case   0:	APPEVENT_Set(APPEVENT_SPEED_P0, NULL);	break;
					case 300:	APPEVENT_Set(APPEVENT_SPEED_T3, NULL);	break;
					case 200:	APPEVENT_Set(APPEVENT_SPEED_T2, NULL);	break;
					case 100:	APPEVENT_Set(APPEVENT_SPEED_T1, NULL);	break;
					case   6:	APPEVENT_Set(APPEVENT_SPEED_P6, NULL);	break;
					case   5:	APPEVENT_Set(APPEVENT_SPEED_P5, NULL);	break;
					case   4:	APPEVENT_Set(APPEVENT_SPEED_P4, NULL);	break;
					case   3:	APPEVENT_Set(APPEVENT_SPEED_P3, NULL);	break;
					case   2:	APPEVENT_Set(APPEVENT_SPEED_P2, NULL);	break;
					case   1:	APPEVENT_Set(APPEVENT_SPEED_P1, NULL);	break;
					case  -1:	APPEVENT_Set(APPEVENT_SPEED_M1, NULL);	break;
					case  -2:	APPEVENT_Set(APPEVENT_SPEED_M2, NULL);	break;
				}
			}
			break;
		case VK_BACK: //Kitao追加。[BackSpace]キー
			APP_SetSpeedNormal();
			break;
		case VK_F2:		APPEVENT_Set(APPEVENT_ADVANCEFRAME_1, NULL);		break;//Kitao追加
		case '1':		APPEVENT_Set(APPEVENT_INPUT_TURBO_1, NULL);			break;//Kitao追加
		case '2':		APPEVENT_Set(APPEVENT_INPUT_TURBO_2, NULL);			break;//Kitao追加
		case '3':		APPEVENT_Set(APPEVENT_INPUT_TURBO_RUN, NULL);		break;//Kitao追加
		case '4':		APPEVENT_Set(APPEVENT_INPUT_TURBO_HIGH, NULL);		break;//Kitao追加
		case '5':		APPEVENT_Set(APPEVENT_INPUT_TURBO_MIDDLE, NULL);	break;//Kitao追加
		case '6':		APPEVENT_Set(APPEVENT_INPUT_TURBO_LOW, NULL);		break;//Kitao追加
		case '7':		APPEVENT_Set(APPEVENT_INPUT_TURBO_OFF, NULL);		break;//Kitao追加
		case '8':		APPEVENT_Set(APPEVENT_INPUT_SWAP_SELRUN, NULL);		break;//Kitao追加
		//case 'Q':		WINMAIN_SetBreakTrap(~WINMAIN_GetBreakTrap());		break;//Kitao追加。デバッグ用
		}
		break;

	// F10 とかはこっちを使う 
	case WM_SYSKEYDOWN:
		switch (wParam)
		{
		case VK_MENU:		APPEVENT_Set(APPEVENT_SHOWMENU, NULL);				break;
//		case VK_F10:
//			break;
		}
		break;

	case WM_LBUTTONDOWN:
		//if ((!MOUSE_IsConnected())||(MAINBOARD_GetPause()))
		//		APPEVENT_Set(APPEVENT_RUN, NULL);
		MOUSE_LButtonDown(TRUE);
		break;

	case WM_LBUTTONUP:
		MOUSE_LButtonDown(FALSE);
		break;

	case WM_RBUTTONDOWN:
		//if ((!MOUSE_IsConnected())||(MAINBOARD_GetPause()))
		//	APPEVENT_Set(APPEVENT_RUN, NULL);
		MOUSE_RButtonDown(TRUE);
		break;

	case WM_RBUTTONUP:
		MOUSE_RButtonDown(FALSE);
		break;

	case WM_MBUTTONDOWN:
		if (!MOUSE_IsConnected())
			APPEVENT_Set(APPEVENT_RUN, NULL);
		MOUSE_RunButtonDown(TRUE);
		break;

	case WM_MBUTTONUP: //Kitao追加。マウスの真ん中ボタンが離されたとき
		MOUSE_RunButtonDown(FALSE);
		break;

	case WM_MOUSEWHEEL: //Kitao追加。マウスのホイールを動かしたとき
		if (MOUSE_GetMouseWheelFlg() == 0)
		{
			delta = HIWORD(wParam);
			if (delta < 0) //下方向
			{
				MOUSE_SelButtonDown(FALSE);//リセット操作になってしまわないようにSELECTボタンは離す
				MOUSE_RunButtonDown(TRUE);//RUNボタンを押したことにする
				MOUSE_SetMouseWheelFlg();
			}
			else if (delta > 0) //上方向
			{
				MOUSE_RunButtonDown(FALSE);//リセット操作になってしまわないようにRUNボタンは離す
				MOUSE_SelButtonDown(TRUE);//SELECTボタンを押したことにする
				MOUSE_SetMouseWheelFlg();
			}
		}
		break;

	case WM_DROPFILES:
		{
			HDROP		hDrop;
			char		dragFilePathName[MAX_PATH+1];

			hDrop = (HDROP)wParam;
			DragQueryFile(hDrop, 0, dragFilePathName, MAX_PATH+1);

			APPEVENT_Set(APPEVENT_OPENGAME, dragFilePathName);

			DragFinish(hDrop);
		}
		break;

	case WM_ENTERMENULOOP:
		//APP_RunEmulator(FALSE); //Kitao追加
		APPEVENT_Set(APPEVENT_SHOWMENU, NULL);
		break;

	case WM_CLOSE:
		if (APP_GetFullScreen()) //Kitao追加。フルスクリーンのときはデスクトップを乱してしまわないようにウィンドウモードに戻してから終了する。ウィンドウがCLOSEする前に行わないと処理されない。
			APP_ToggleFullscreen();
		APPEVENT_Set(APPEVENT_EXIT, NULL);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_SET_FULLSCREEN_WS:
		set_fullscreen_windowstyle(hWnd, (int)wParam, (int)lParam);
		APPEVENT_Set(APPEVENT_REDRAWSCREEN, NULL);
		return 0;

	case WM_SET_NORMAL_WS:
		set_normal_windowstyle(hWnd, (int)wParam, (int)lParam);
		APPEVENT_Set(APPEVENT_REDRAWSCREEN, NULL);
		return 0;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case WM_OPEN_FILE:				APPEVENT_Set(APPEVENT_FILEDIALOG, NULL);			break;
		case WM_CD_CHANGE:				APPEVENT_Set(APPEVENT_CDCHANGE, NULL);				break;//Kitao追加
		case WM_RESET_EMULATOR:			APPEVENT_Set(APPEVENT_RESET, NULL);					break;
		case WM_RUN_EMULATOR:			APPEVENT_Set(APPEVENT_RUN, NULL);					break;
//		case WM_STOP_EMULATOR:			APPEVENT_Set(APPEVENT_PAUSE, NULL);					break;
		case WM_PAUSE_BUTTON:			APPEVENT_Set(APPEVENT_PAUSE_BUTTON, NULL);			break;//Kitao追加
		case WM_SET_RESUME:				APPEVENT_Set(APPEVENT_RESUME, NULL);				break;//Kitao追加
		case WM_AUTO_RESUME:			APPEVENT_Set(APPEVENT_AUTO_RESUME, NULL);			break;//Kitao追加
		case WM_SAVE_STATE:				APPEVENT_Set(APPEVENT_SAVESTATE, NULL);				break;
		case WM_SAVE_STATE_1:			APPEVENT_Set(APPEVENT_SAVESTATE_1, NULL);			break;//Kitao追加
		case WM_SAVE_STATE_2:			APPEVENT_Set(APPEVENT_SAVESTATE_2, NULL);			break;//Kitao追加
		case WM_SAVE_STATE_3:			APPEVENT_Set(APPEVENT_SAVESTATE_3, NULL);			break;//Kitao追加
		case WM_SAVE_STATE_4:			APPEVENT_Set(APPEVENT_SAVESTATE_4, NULL);			break;//Kitao追加
		case WM_SAVE_STATE_5:			APPEVENT_Set(APPEVENT_SAVESTATE_5, NULL);			break;//Kitao追加
		case WM_SAVE_STATE_6:			APPEVENT_Set(APPEVENT_SAVESTATE_6, NULL);			break;//Kitao追加
		case WM_SAVE_STATE_7:			APPEVENT_Set(APPEVENT_SAVESTATE_7, NULL);			break;//Kitao追加
		case WM_SAVE_STATE_8:			APPEVENT_Set(APPEVENT_SAVESTATE_8, NULL);			break;//Kitao追加
		case WM_SAVE_STATE_9:			APPEVENT_Set(APPEVENT_SAVESTATE_9, NULL);			break;//Kitao追加
		case WM_SAVE_STATE_10:			APPEVENT_Set(APPEVENT_SAVESTATE_10, NULL);			break;//Kitao追加
		case WM_SAVE_STATE_11:			APPEVENT_Set(APPEVENT_SAVESTATE_11, NULL);			break;//Kitao追加
		case WM_SAVE_STATE_12:			APPEVENT_Set(APPEVENT_SAVESTATE_12, NULL);			break;//Kitao追加
		case WM_SAVE_STATE_13:			APPEVENT_Set(APPEVENT_SAVESTATE_13, NULL);			break;//Kitao追加
		case WM_SAVE_STATE_14:			APPEVENT_Set(APPEVENT_SAVESTATE_14, NULL);			break;//Kitao追加
		case WM_SAVE_STATE_15:			APPEVENT_Set(APPEVENT_SAVESTATE_15, NULL);			break;//Kitao追加
		case WM_SAVE_STATE_16:			APPEVENT_Set(APPEVENT_SAVESTATE_16, NULL);			break;//Kitao追加
		case WM_SAVE_STATE_17:			APPEVENT_Set(APPEVENT_SAVESTATE_17, NULL);			break;//Kitao追加
		case WM_SAVE_STATE_18:			APPEVENT_Set(APPEVENT_SAVESTATE_18, NULL);			break;//Kitao追加
		case WM_SAVE_STATE_19:			APPEVENT_Set(APPEVENT_SAVESTATE_19, NULL);			break;//Kitao追加
		case WM_SAVE_STATE_20:			APPEVENT_Set(APPEVENT_SAVESTATE_20, NULL);			break;//Kitao追加
		case WM_SAVE_STATE_21:			APPEVENT_Set(APPEVENT_SAVESTATE_21, NULL);			break;//Kitao追加
		case WM_SAVE_STATE_22:			APPEVENT_Set(APPEVENT_SAVESTATE_22, NULL);			break;//Kitao追加
		case WM_SAVE_STATE_23:			APPEVENT_Set(APPEVENT_SAVESTATE_23, NULL);			break;//Kitao追加
		case WM_SAVE_STATE_24:			APPEVENT_Set(APPEVENT_SAVESTATE_24, NULL);			break;//Kitao追加
		case WM_SAVE_STATE_25:			APPEVENT_Set(APPEVENT_SAVESTATE_25, NULL);			break;//Kitao追加
		case WM_SAVE_STATE_26:			APPEVENT_Set(APPEVENT_SAVESTATE_26, NULL);			break;//Kitao追加
		case WM_SAVE_STATE_27:			APPEVENT_Set(APPEVENT_SAVESTATE_27, NULL);			break;//Kitao追加
		case WM_SAVE_STATE_28:			APPEVENT_Set(APPEVENT_SAVESTATE_28, NULL);			break;//Kitao追加
		case WM_SAVE_STATE_29:			APPEVENT_Set(APPEVENT_SAVESTATE_29, NULL);			break;//Kitao追加
		case WM_SAVE_STATE_30:			APPEVENT_Set(APPEVENT_SAVESTATE_30, NULL);			break;//Kitao追加
		case WM_SAVE_STATE_31:			APPEVENT_Set(APPEVENT_SAVESTATE_31, NULL);			break;//Kitao追加
		case WM_SAVE_STATE_32:			APPEVENT_Set(APPEVENT_SAVESTATE_32, NULL);			break;//Kitao追加
		case WM_SAVE_STATE_33:			APPEVENT_Set(APPEVENT_SAVESTATE_33, NULL);			break;//Kitao追加
		case WM_SAVE_STATE_34:			APPEVENT_Set(APPEVENT_SAVESTATE_34, NULL);			break;//Kitao追加
		case WM_SAVE_STATE_35:			APPEVENT_Set(APPEVENT_SAVESTATE_35, NULL);			break;//Kitao追加
		case WM_SAVE_STATE_36:			APPEVENT_Set(APPEVENT_SAVESTATE_36, NULL);			break;//Kitao追加
		case WM_SAVE_STATE_37:			APPEVENT_Set(APPEVENT_SAVESTATE_37, NULL);			break;//Kitao追加
		case WM_SAVE_STATE_38:			APPEVENT_Set(APPEVENT_SAVESTATE_38, NULL);			break;//Kitao追加
		case WM_SAVE_STATE_39:			APPEVENT_Set(APPEVENT_SAVESTATE_39, NULL);			break;//Kitao追加
		case WM_SAVE_STATE_40:			APPEVENT_Set(APPEVENT_SAVESTATE_40, NULL);			break;//Kitao追加
		case WM_SAVE_STATE_41:			APPEVENT_Set(APPEVENT_SAVESTATE_41, NULL);			break;//Kitao追加
		case WM_SAVE_STATE_42:			APPEVENT_Set(APPEVENT_SAVESTATE_42, NULL);			break;//Kitao追加
		case WM_SAVE_STATE_43:			APPEVENT_Set(APPEVENT_SAVESTATE_43, NULL);			break;//Kitao追加
		case WM_SAVE_STATE_44:			APPEVENT_Set(APPEVENT_SAVESTATE_44, NULL);			break;//Kitao追加
		case WM_SAVE_STATE_45:			APPEVENT_Set(APPEVENT_SAVESTATE_45, NULL);			break;//Kitao追加
		case WM_SAVE_STATE_46:			APPEVENT_Set(APPEVENT_SAVESTATE_46, NULL);			break;//Kitao追加
		case WM_SAVE_STATE_47:			APPEVENT_Set(APPEVENT_SAVESTATE_47, NULL);			break;//Kitao追加
		case WM_SAVE_STATE_48:			APPEVENT_Set(APPEVENT_SAVESTATE_48, NULL);			break;//Kitao追加
		case WM_SAVE_STATE_49:			APPEVENT_Set(APPEVENT_SAVESTATE_49, NULL);			break;//Kitao追加
		case WM_SAVE_STATE_50:			APPEVENT_Set(APPEVENT_SAVESTATE_50, NULL);			break;//Kitao追加
		case WM_LOAD_STATE:				APPEVENT_Set(APPEVENT_LOADSTATE, NULL);				break;
		case WM_LOAD_STATE_P:			APPEVENT_Set(APPEVENT_LOADSTATE_P, NULL);			break;//Kitao追加
		case WM_LOAD_STATE_1:			APPEVENT_Set(APPEVENT_LOADSTATE_1, NULL);			break;//Kitao追加
		case WM_LOAD_STATE_2:			APPEVENT_Set(APPEVENT_LOADSTATE_2, NULL);			break;//Kitao追加
		case WM_LOAD_STATE_3:			APPEVENT_Set(APPEVENT_LOADSTATE_3, NULL);			break;//Kitao追加
		case WM_LOAD_STATE_4:			APPEVENT_Set(APPEVENT_LOADSTATE_4, NULL);			break;//Kitao追加
		case WM_LOAD_STATE_5:			APPEVENT_Set(APPEVENT_LOADSTATE_5, NULL);			break;//Kitao追加
		case WM_LOAD_STATE_6:			APPEVENT_Set(APPEVENT_LOADSTATE_6, NULL);			break;//Kitao追加
		case WM_LOAD_STATE_7:			APPEVENT_Set(APPEVENT_LOADSTATE_7, NULL);			break;//Kitao追加
		case WM_LOAD_STATE_8:			APPEVENT_Set(APPEVENT_LOADSTATE_8, NULL);			break;//Kitao追加
		case WM_LOAD_STATE_9:			APPEVENT_Set(APPEVENT_LOADSTATE_9, NULL);			break;//Kitao追加
		case WM_LOAD_STATE_10:			APPEVENT_Set(APPEVENT_LOADSTATE_10, NULL);			break;//Kitao追加
		case WM_LOAD_STATE_11:			APPEVENT_Set(APPEVENT_LOADSTATE_11, NULL);			break;//Kitao追加
		case WM_LOAD_STATE_12:			APPEVENT_Set(APPEVENT_LOADSTATE_12, NULL);			break;//Kitao追加
		case WM_LOAD_STATE_13:			APPEVENT_Set(APPEVENT_LOADSTATE_13, NULL);			break;//Kitao追加
		case WM_LOAD_STATE_14:			APPEVENT_Set(APPEVENT_LOADSTATE_14, NULL);			break;//Kitao追加
		case WM_LOAD_STATE_15:			APPEVENT_Set(APPEVENT_LOADSTATE_15, NULL);			break;//Kitao追加
		case WM_LOAD_STATE_16:			APPEVENT_Set(APPEVENT_LOADSTATE_16, NULL);			break;//Kitao追加
		case WM_LOAD_STATE_17:			APPEVENT_Set(APPEVENT_LOADSTATE_17, NULL);			break;//Kitao追加
		case WM_LOAD_STATE_18:			APPEVENT_Set(APPEVENT_LOADSTATE_18, NULL);			break;//Kitao追加
		case WM_LOAD_STATE_19:			APPEVENT_Set(APPEVENT_LOADSTATE_19, NULL);			break;//Kitao追加
		case WM_LOAD_STATE_20:			APPEVENT_Set(APPEVENT_LOADSTATE_20, NULL);			break;//Kitao追加
		case WM_LOAD_STATE_21:			APPEVENT_Set(APPEVENT_LOADSTATE_21, NULL);			break;//Kitao追加
		case WM_LOAD_STATE_22:			APPEVENT_Set(APPEVENT_LOADSTATE_22, NULL);			break;//Kitao追加
		case WM_LOAD_STATE_23:			APPEVENT_Set(APPEVENT_LOADSTATE_23, NULL);			break;//Kitao追加
		case WM_LOAD_STATE_24:			APPEVENT_Set(APPEVENT_LOADSTATE_24, NULL);			break;//Kitao追加
		case WM_LOAD_STATE_25:			APPEVENT_Set(APPEVENT_LOADSTATE_25, NULL);			break;//Kitao追加
		case WM_LOAD_STATE_26:			APPEVENT_Set(APPEVENT_LOADSTATE_26, NULL);			break;//Kitao追加
		case WM_LOAD_STATE_27:			APPEVENT_Set(APPEVENT_LOADSTATE_27, NULL);			break;//Kitao追加
		case WM_LOAD_STATE_28:			APPEVENT_Set(APPEVENT_LOADSTATE_28, NULL);			break;//Kitao追加
		case WM_LOAD_STATE_29:			APPEVENT_Set(APPEVENT_LOADSTATE_29, NULL);			break;//Kitao追加
		case WM_LOAD_STATE_30:			APPEVENT_Set(APPEVENT_LOADSTATE_30, NULL);			break;//Kitao追加
		case WM_LOAD_STATE_31:			APPEVENT_Set(APPEVENT_LOADSTATE_31, NULL);			break;//Kitao追加
		case WM_LOAD_STATE_32:			APPEVENT_Set(APPEVENT_LOADSTATE_32, NULL);			break;//Kitao追加
		case WM_LOAD_STATE_33:			APPEVENT_Set(APPEVENT_LOADSTATE_33, NULL);			break;//Kitao追加
		case WM_LOAD_STATE_34:			APPEVENT_Set(APPEVENT_LOADSTATE_34, NULL);			break;//Kitao追加
		case WM_LOAD_STATE_35:			APPEVENT_Set(APPEVENT_LOADSTATE_35, NULL);			break;//Kitao追加
		case WM_LOAD_STATE_36:			APPEVENT_Set(APPEVENT_LOADSTATE_36, NULL);			break;//Kitao追加
		case WM_LOAD_STATE_37:			APPEVENT_Set(APPEVENT_LOADSTATE_37, NULL);			break;//Kitao追加
		case WM_LOAD_STATE_38:			APPEVENT_Set(APPEVENT_LOADSTATE_38, NULL);			break;//Kitao追加
		case WM_LOAD_STATE_39:			APPEVENT_Set(APPEVENT_LOADSTATE_39, NULL);			break;//Kitao追加
		case WM_LOAD_STATE_40:			APPEVENT_Set(APPEVENT_LOADSTATE_40, NULL);			break;//Kitao追加
		case WM_LOAD_STATE_41:			APPEVENT_Set(APPEVENT_LOADSTATE_41, NULL);			break;//Kitao追加
		case WM_LOAD_STATE_42:			APPEVENT_Set(APPEVENT_LOADSTATE_42, NULL);			break;//Kitao追加
		case WM_LOAD_STATE_43:			APPEVENT_Set(APPEVENT_LOADSTATE_43, NULL);			break;//Kitao追加
		case WM_LOAD_STATE_44:			APPEVENT_Set(APPEVENT_LOADSTATE_44, NULL);			break;//Kitao追加
		case WM_LOAD_STATE_45:			APPEVENT_Set(APPEVENT_LOADSTATE_45, NULL);			break;//Kitao追加
		case WM_LOAD_STATE_46:			APPEVENT_Set(APPEVENT_LOADSTATE_46, NULL);			break;//Kitao追加
		case WM_LOAD_STATE_47:			APPEVENT_Set(APPEVENT_LOADSTATE_47, NULL);			break;//Kitao追加
		case WM_LOAD_STATE_48:			APPEVENT_Set(APPEVENT_LOADSTATE_48, NULL);			break;//Kitao追加
		case WM_LOAD_STATE_49:			APPEVENT_Set(APPEVENT_LOADSTATE_49, NULL);			break;//Kitao追加
		case WM_LOAD_STATE_50:			APPEVENT_Set(APPEVENT_LOADSTATE_50, NULL);			break;//Kitao追加
		case WM_SAVE_BUTTON:			APPEVENT_Set(APPEVENT_SAVE_BUTTON, NULL);			break;//Kitao追加
		case WM_LOAD_BUTTON:			APPEVENT_Set(APPEVENT_LOAD_BUTTON, NULL);			break;//Kitao追加
		case WM_SAVE_DEFAULT:			APPEVENT_Set(APPEVENT_SAVE_DEFAULT, NULL);			break;//Kitao追加
		case WM_FOLDER_STATE:			APPEVENT_Set(APPEVENT_FOLDER_STATE, NULL);			break;//Kitao追加
		case WM_RECORDING_GAMEPLAY:		APPEVENT_Set(APPEVENT_RECORDING, NULL);				break;
		case WM_PLAYRECORD_GAMEPLAY:	APPEVENT_Set(APPEVENT_PLAYRECORD, NULL);			break;
		case WM_RECORDING_1:			APPEVENT_Set(APPEVENT_RECORDING_1, NULL);			break;//Kitao追加
		case WM_RECORDING_2:			APPEVENT_Set(APPEVENT_RECORDING_2, NULL);			break;//Kitao追加
		case WM_RECORDING_3:			APPEVENT_Set(APPEVENT_RECORDING_3, NULL);			break;//Kitao追加
		case WM_RECORDING_4:			APPEVENT_Set(APPEVENT_RECORDING_4, NULL);			break;//Kitao追加
		case WM_RECORDING_5:			APPEVENT_Set(APPEVENT_RECORDING_5, NULL);			break;//Kitao追加
		case WM_RECORDING_6:			APPEVENT_Set(APPEVENT_RECORDING_6, NULL);			break;//Kitao追加
		case WM_RECORDING_7:			APPEVENT_Set(APPEVENT_RECORDING_7, NULL);			break;//Kitao追加
		case WM_RECORDING_8:			APPEVENT_Set(APPEVENT_RECORDING_8, NULL);			break;//Kitao追加
		case WM_RECORDING_9:			APPEVENT_Set(APPEVENT_RECORDING_9, NULL);			break;//Kitao追加
		case WM_RECORDING_10:			APPEVENT_Set(APPEVENT_RECORDING_10, NULL);			break;//Kitao追加
		case WM_RECORDING_HELP:			APPEVENT_Set(APPEVENT_RECORDING_HELP, NULL);		break;//Kitao追加
		case WM_PLAYRECORD_1:			APPEVENT_Set(APPEVENT_PLAYRECORD_1, NULL);			break;//Kitao追加
		case WM_PLAYRECORD_2:			APPEVENT_Set(APPEVENT_PLAYRECORD_2, NULL);			break;//Kitao追加
		case WM_PLAYRECORD_3:			APPEVENT_Set(APPEVENT_PLAYRECORD_3, NULL);			break;//Kitao追加
		case WM_PLAYRECORD_4:			APPEVENT_Set(APPEVENT_PLAYRECORD_4, NULL);			break;//Kitao追加
		case WM_PLAYRECORD_5:			APPEVENT_Set(APPEVENT_PLAYRECORD_5, NULL);			break;//Kitao追加
		case WM_PLAYRECORD_6:			APPEVENT_Set(APPEVENT_PLAYRECORD_6, NULL);			break;//Kitao追加
		case WM_PLAYRECORD_7:			APPEVENT_Set(APPEVENT_PLAYRECORD_7, NULL);			break;//Kitao追加
		case WM_PLAYRECORD_8:			APPEVENT_Set(APPEVENT_PLAYRECORD_8, NULL);			break;//Kitao追加
		case WM_PLAYRECORD_9:			APPEVENT_Set(APPEVENT_PLAYRECORD_9, NULL);			break;//Kitao追加
		case WM_PLAYRECORD_10:			APPEVENT_Set(APPEVENT_PLAYRECORD_10, NULL);			break;//Kitao追加
		case WM_MOVERECORD_1:			APPEVENT_Set(APPEVENT_MOVERECORD_1, NULL);			break;//Kitao追加
		case WM_MOVERECORD_2:			APPEVENT_Set(APPEVENT_MOVERECORD_2, NULL);			break;//Kitao追加
		case WM_MOVERECORD_3:			APPEVENT_Set(APPEVENT_MOVERECORD_3, NULL);			break;//Kitao追加
		case WM_MOVERECORD_4:			APPEVENT_Set(APPEVENT_MOVERECORD_4, NULL);			break;//Kitao追加
		case WM_MOVERECORD_5:			APPEVENT_Set(APPEVENT_MOVERECORD_5, NULL);			break;//Kitao追加
		case WM_MOVERECORD_6:			APPEVENT_Set(APPEVENT_MOVERECORD_6, NULL);			break;//Kitao追加
		case WM_MOVERECORD_7:			APPEVENT_Set(APPEVENT_MOVERECORD_7, NULL);			break;//Kitao追加
		case WM_MOVERECORD_8:			APPEVENT_Set(APPEVENT_MOVERECORD_8, NULL);			break;//Kitao追加
		case WM_MOVERECORD_9:			APPEVENT_Set(APPEVENT_MOVERECORD_9, NULL);			break;//Kitao追加
		case WM_MOVERECORD_10:			APPEVENT_Set(APPEVENT_MOVERECORD_10, NULL);			break;//Kitao追加
		case WM_FOLDER_GAMEPLAY:		APPEVENT_Set(APPEVENT_FOLDER_GAMEPLAY, NULL);		break;//Kitao追加
		case WM_FOLDER_BRAM:			APPEVENT_Set(APPEVENT_FOLDER_BRAM, NULL);			break;//Kitao追加
		case WM_FOLDER_MB128:			APPEVENT_Set(APPEVENT_FOLDER_MB128, NULL);			break;//Kitao追加
		case WM_TRACE_1_FRAME:			APPEVENT_Set(APPEVENT_ADVANCEFRAME_1, NULL);		break;
		case WM_TRACE_10_FRAME:			APPEVENT_Set(APPEVENT_ADVANCEFRAME_10, NULL);		break;
		case WM_TRACE_100_FRAME:		APPEVENT_Set(APPEVENT_ADVANCEFRAME_100, NULL);		break;
		case WM_WRITE_MEMORY:			APPEVENT_Set(APPEVENT_WRITE_MEMORY, NULL);			break;//Kitao追加
		case WM_ABOUT_WRITEMEM:			APPEVENT_Set(APPEVENT_ABOUT_WRITEMEM, NULL);		break;//Kitao追加

		case WM_RECENT_1:				APPEVENT_Set(APPEVENT_RECENT_1, NULL);				break;//Kitao追加
		case WM_RECENT_2:				APPEVENT_Set(APPEVENT_RECENT_2, NULL);				break;//Kitao追加
		case WM_RECENT_3:				APPEVENT_Set(APPEVENT_RECENT_3, NULL);				break;//Kitao追加
		case WM_RECENT_4:				APPEVENT_Set(APPEVENT_RECENT_4, NULL);				break;//Kitao追加
		case WM_RECENT_5:				APPEVENT_Set(APPEVENT_RECENT_5, NULL);				break;//Kitao追加
		case WM_RECENT_6:				APPEVENT_Set(APPEVENT_RECENT_6, NULL);				break;//Kitao追加
		case WM_RECENT_7:				APPEVENT_Set(APPEVENT_RECENT_7, NULL);				break;//Kitao追加
		case WM_RECENT_8:				APPEVENT_Set(APPEVENT_RECENT_8, NULL);				break;//Kitao追加
		case WM_RECENT_9:				APPEVENT_Set(APPEVENT_RECENT_9, NULL);				break;//Kitao追加
		case WM_RECENT_10:				APPEVENT_Set(APPEVENT_RECENT_10, NULL);				break;//Kitao追加
		case WM_RECENT_11:				APPEVENT_Set(APPEVENT_RECENT_11, NULL);				break;//Kitao追加
		case WM_RECENT_12:				APPEVENT_Set(APPEVENT_RECENT_12, NULL);				break;//Kitao追加
		case WM_RECENT_13:				APPEVENT_Set(APPEVENT_RECENT_13, NULL);				break;//Kitao追加
		case WM_RECENT_14:				APPEVENT_Set(APPEVENT_RECENT_14, NULL);				break;//Kitao追加
		case WM_RECENT_15:				APPEVENT_Set(APPEVENT_RECENT_15, NULL);				break;//Kitao追加
		case WM_RECENT_16:				APPEVENT_Set(APPEVENT_RECENT_16, NULL);				break;//Kitao追加
		case WM_RECENT_17:				APPEVENT_Set(APPEVENT_RECENT_17, NULL);				break;//Kitao追加
		case WM_RECENT_18:				APPEVENT_Set(APPEVENT_RECENT_18, NULL);				break;//Kitao追加
		case WM_RECENT_19:				APPEVENT_Set(APPEVENT_RECENT_19, NULL);				break;//Kitao追加
		case WM_RECENT_20:				APPEVENT_Set(APPEVENT_RECENT_20, NULL);				break;//Kitao追加

		case WM_SCREEN_CS:				APPEVENT_Set(APPEVENT_SCREEN_CS, NULL);				break;//Kitao追加
		case WM_SCREEN_X1:				APPEVENT_Set(APPEVENT_SCREEN_X1, NULL);				break;
		case WM_SCREEN_X2:				APPEVENT_Set(APPEVENT_SCREEN_X2, NULL);				break;
		case WM_SCREEN_X3:				APPEVENT_Set(APPEVENT_SCREEN_X3, NULL);				break;
		case WM_SCREEN_X4:				APPEVENT_Set(APPEVENT_SCREEN_X4, NULL);				break;
		case WM_SCREEN_F1:				APPEVENT_Set(APPEVENT_SCREEN_F1, NULL);				break;//Kitao追加
		case WM_SCREEN_F2:				APPEVENT_Set(APPEVENT_SCREEN_F2, NULL);				break;//Kitao追加
		case WM_SCREEN_DISABLEAERO:		APPEVENT_Set(APPEVENT_SCREEN_DISABLEAERO, NULL);	break;//Kitao追加
		case WM_SCREEN_USEAERO:			APPEVENT_Set(APPEVENT_SCREEN_USEAERO, NULL);		break;//Kitao追加
		case WM_SCREEN_GAMMA1:			APPEVENT_Set(APPEVENT_SCREEN_GAMMA1, NULL);			break;//Kitao追加
		case WM_SCREEN_GAMMA2:			APPEVENT_Set(APPEVENT_SCREEN_GAMMA2, NULL);			break;//Kitao追加
		case WM_SCREEN_GAMMA3:			APPEVENT_Set(APPEVENT_SCREEN_GAMMA3, NULL);			break;//Kitao追加
		case WM_SCREEN_GAMMA4:			APPEVENT_Set(APPEVENT_SCREEN_GAMMA4, NULL);			break;//Kitao追加
		case WM_SCREEN_GAMMA5:			APPEVENT_Set(APPEVENT_SCREEN_GAMMA5, NULL);			break;//Kitao追加
		case WM_SCREEN_GAMMA6:			APPEVENT_Set(APPEVENT_SCREEN_GAMMA6, NULL);			break;//Kitao追加
		case WM_SCREEN_GAMMA7:			APPEVENT_Set(APPEVENT_SCREEN_GAMMA7, NULL);			break;//Kitao追加
		case WM_SCREEN_BRIGHT1:			APPEVENT_Set(APPEVENT_SCREEN_BRIGHT1, NULL);		break;//Kitao追加
		case WM_SCREEN_BRIGHT2:			APPEVENT_Set(APPEVENT_SCREEN_BRIGHT2, NULL);		break;//Kitao追加
		case WM_SCREEN_BRIGHT3:			APPEVENT_Set(APPEVENT_SCREEN_BRIGHT3, NULL);		break;//Kitao追加
		case WM_SCREEN_BRIGHT4:			APPEVENT_Set(APPEVENT_SCREEN_BRIGHT4, NULL);		break;//Kitao追加
		case WM_SCREEN_BRIGHT5:			APPEVENT_Set(APPEVENT_SCREEN_BRIGHT5, NULL);		break;//Kitao追加
		case WM_SCREEN_BRIGHT6:			APPEVENT_Set(APPEVENT_SCREEN_BRIGHT6, NULL);		break;//Kitao追加
		case WM_SCREEN_BRIGHT7:			APPEVENT_Set(APPEVENT_SCREEN_BRIGHT7, NULL);		break;//Kitao追加
		case WM_SCREEN_BRIGHT8:			APPEVENT_Set(APPEVENT_SCREEN_BRIGHT8, NULL);		break;//Kitao追加
		case WM_SCREEN_BRIGHT9:			APPEVENT_Set(APPEVENT_SCREEN_BRIGHT9, NULL);		break;//Kitao追加
		case WM_SCREEN_MINIMIZE:		APPEVENT_Set(APPEVENT_SCREEN_MINIMIZE, NULL);		break;//Kitao追加
		case WM_SCREEN_NONSTRETCHED:	APPEVENT_Set(APPEVENT_SCREEN_NONSTRETCHED, NULL);	break;//Kitao追加
		case WM_SCREEN_STRETCHED:		APPEVENT_Set(APPEVENT_SCREEN_STRETCHED, NULL);		break;
		case WM_SCREEN_FULLSTRETCHED:	APPEVENT_Set(APPEVENT_SCREEN_FULLSTRETCHED, NULL);	break;//Kitao追加
		case WM_SCREEN_VERTICAL:		APPEVENT_Set(APPEVENT_SCREEN_VERTICAL, NULL);		break;//Kitao追加
		case WM_SCREEN_TV:				APPEVENT_Set(APPEVENT_SCREEN_TV, NULL);				break;//Kitao追加
		case WM_SCREEN_MONOCOLOR:		APPEVENT_Set(APPEVENT_SCREEN_MONOCOLOR, NULL);		break;//Kitao追加
		case WM_SCREEN_SHOWOVERSCAN:	APPEVENT_Set(APPEVENT_SCREEN_SHOWOVERSCAN, NULL);	break;//Kitao追加
		case WM_SCREEN_OVERTB:			APPEVENT_Set(APPEVENT_SCREEN_OVERTB, NULL);			break;//Kitao追加
		case WM_SCREEN_OVERTOP:			APPEVENT_Set(APPEVENT_SCREEN_OVERTOP, NULL);		break;//Kitao追加
		case WM_SCREEN_OVERBOTTOM:		APPEVENT_Set(APPEVENT_SCREEN_OVERBOTTOM, NULL);		break;//Kitao追加
		case WM_SCREEN_OVERNONETB:		APPEVENT_Set(APPEVENT_SCREEN_OVERNONETB, NULL);		break;//Kitao追加
		case WM_SCREEN_OVERHEIGHT8:		APPEVENT_Set(APPEVENT_SCREEN_OVERHEIGHT8, NULL);	break;//Kitao追加
		case WM_SCREEN_OVERHEIGHT7:		APPEVENT_Set(APPEVENT_SCREEN_OVERHEIGHT7, NULL);	break;//Kitao追加
		case WM_SCREEN_OVERHEIGHT6:		APPEVENT_Set(APPEVENT_SCREEN_OVERHEIGHT6, NULL);	break;//Kitao追加
		case WM_SCREEN_OVERHEIGHT4:		APPEVENT_Set(APPEVENT_SCREEN_OVERHEIGHT4, NULL);	break;//Kitao追加
		case WM_SCREEN_OVERHEIGHT2:		APPEVENT_Set(APPEVENT_SCREEN_OVERHEIGHT2, NULL);	break;//Kitao追加
		case WM_SCREEN_OVERHEIGHT1:		APPEVENT_Set(APPEVENT_SCREEN_OVERHEIGHT1, NULL);	break;//Kitao追加
		case WM_SCREEN_OVERLR:			APPEVENT_Set(APPEVENT_SCREEN_OVERLR, NULL);			break;//Kitao追加
		case WM_SCREEN_OVERNONELR:		APPEVENT_Set(APPEVENT_SCREEN_OVERNONELR, NULL);		break;//Kitao追加
		case WM_SCREEN_OVERSTART:		APPEVENT_Set(APPEVENT_SCREEN_OVERSTART, NULL);		break;//Kitao追加
		case WM_SCREEN_OVERBLACK:		APPEVENT_Set(APPEVENT_SCREEN_OVERBLACK, NULL);		break;//Kitao追加
		case WM_SCREEN_SOVERTB:			APPEVENT_Set(APPEVENT_SCREEN_SOVERTB, NULL);		break;//Kitao追加
		case WM_SCREEN_SOVERTOP:		APPEVENT_Set(APPEVENT_SCREEN_SOVERTOP, NULL);		break;//Kitao追加
		case WM_SCREEN_SOVERBOTTOM:		APPEVENT_Set(APPEVENT_SCREEN_SOVERBOTTOM, NULL);	break;//Kitao追加
		case WM_SCREEN_SOVERNONETB:		APPEVENT_Set(APPEVENT_SCREEN_SOVERNONETB, NULL);	break;//Kitao追加
		case WM_SCREEN_SOVERHEIGHT8:	APPEVENT_Set(APPEVENT_SCREEN_SOVERHEIGHT8, NULL);	break;//Kitao追加
		case WM_SCREEN_SOVERHEIGHT7:	APPEVENT_Set(APPEVENT_SCREEN_SOVERHEIGHT7, NULL);	break;//Kitao追加
		case WM_SCREEN_SOVERHEIGHT6:	APPEVENT_Set(APPEVENT_SCREEN_SOVERHEIGHT6, NULL);	break;//Kitao追加
		case WM_SCREEN_SOVERHEIGHT4:	APPEVENT_Set(APPEVENT_SCREEN_SOVERHEIGHT4, NULL);	break;//Kitao追加
		case WM_SCREEN_SOVERHEIGHT2:	APPEVENT_Set(APPEVENT_SCREEN_SOVERHEIGHT2, NULL);	break;//Kitao追加
		case WM_SCREEN_SOVERHEIGHT1:	APPEVENT_Set(APPEVENT_SCREEN_SOVERHEIGHT1, NULL);	break;//Kitao追加
		case WM_SCREEN_SOVERLR:			APPEVENT_Set(APPEVENT_SCREEN_SOVERLR, NULL);		break;//Kitao追加
		case WM_SCREEN_SOVERNONELR:		APPEVENT_Set(APPEVENT_SCREEN_SOVERNONELR, NULL);	break;//Kitao追加
		case WM_SCREEN_FULLSCREEN:		APPEVENT_Set(APPEVENT_SCREEN_FULLSCREEN, NULL);		break;
		case WM_SCREEN_FULLSCREEN640:	APPEVENT_Set(APPEVENT_SCREEN_FULLSCREEN640, NULL);	break;//Kitao追加
		case WM_SCREEN_FULLSCREENCS1:	APPEVENT_Set(APPEVENT_SCREEN_FULLSCREENCS1, NULL);	break;//Kitao追加
		case WM_SCREEN_FULLSCREENCS2:	APPEVENT_Set(APPEVENT_SCREEN_FULLSCREENCS2, NULL);	break;//Kitao追加
		case WM_SCREEN_FULLSCREENCS3:	APPEVENT_Set(APPEVENT_SCREEN_FULLSCREENCS3, NULL);	break;//Kitao追加
		case WM_SCREEN_FULLSCREENCSA:	APPEVENT_Set(APPEVENT_SCREEN_FULLSCREENCSA, NULL);	break;//Kitao追加
		case WM_SCREEN_NONSCANLINED:	APPEVENT_Set(APPEVENT_SCREEN_NONSCANLINED, NULL);	break;//Kitao追加
		case WM_SCREEN_SPSCANLINED:		APPEVENT_Set(APPEVENT_SCREEN_SPSCANLINED, NULL);	break;//Kitao追加
		case WM_SCREEN_HRSCANLINED:		APPEVENT_Set(APPEVENT_SCREEN_HRSCANLINED, NULL);	break;//Kitao追加
		case WM_SCREEN_STARTTV:			APPEVENT_Set(APPEVENT_SCREEN_STARTTV, NULL);		break;//Kitao追加
		case WM_SCREEN_SCANDENSITY0:	APPEVENT_Set(APPEVENT_SCREEN_SCANDENSITY0, NULL);	break;//Kitao追加
		case WM_SCREEN_SCANDENSITY10:	APPEVENT_Set(APPEVENT_SCREEN_SCANDENSITY10, NULL);	break;//Kitao追加
		case WM_SCREEN_SCANDENSITY20:	APPEVENT_Set(APPEVENT_SCREEN_SCANDENSITY20, NULL);	break;//Kitao追加
		case WM_SCREEN_SCANDENSITY30:	APPEVENT_Set(APPEVENT_SCREEN_SCANDENSITY30, NULL);	break;//Kitao追加
		case WM_SCREEN_SCANDENSITY40:	APPEVENT_Set(APPEVENT_SCREEN_SCANDENSITY40, NULL);	break;//Kitao追加
		case WM_SCREEN_SCANDENSITY50:	APPEVENT_Set(APPEVENT_SCREEN_SCANDENSITY50, NULL);	break;//Kitao追加
		case WM_SCREEN_SCANDENSITY60:	APPEVENT_Set(APPEVENT_SCREEN_SCANDENSITY60, NULL);	break;//Kitao追加
		case WM_SCREEN_SCANDENSITY70:	APPEVENT_Set(APPEVENT_SCREEN_SCANDENSITY70, NULL);	break;//Kitao追加
		case WM_SCREEN_SCANDENSITY80:	APPEVENT_Set(APPEVENT_SCREEN_SCANDENSITY80, NULL);	break;//Kitao追加
		case WM_SCREEN_SCANGAMMA:		APPEVENT_Set(APPEVENT_SCREEN_SCANGAMMA, NULL);		break;//Kitao追加
		case WM_SCREEN_SYNC_VBLANK:		APPEVENT_Set(APPEVENT_SCREEN_SYNC_VBLANK, NULL);	break;
		case WM_SCREEN_SYNC_WINDOWS:	APPEVENT_Set(APPEVENT_SCREEN_SYNC_WINDOWS, NULL);	break;//Kitao追加。v2.65
		case WM_SCREEN_SYNC_WINDOWSF:	APPEVENT_Set(APPEVENT_SCREEN_SYNC_WINDOWSF, NULL);	break;//Kitao追加。v2.65
		case WM_SCREEN_SYNC_NON:		APPEVENT_Set(APPEVENT_SCREEN_SYNC_NON, NULL);		break;//Kitao追加。v2.65
		case WM_SCREEN_SYNC_ADJUST:		APPEVENT_Set(APPEVENT_SCREEN_SYNC_ADJUST, NULL);	break;//Kitao追加。v2.65
		case WM_SCREEN_DIRECT3D:		APPEVENT_Set(APPEVENT_SCREEN_DIRECT3D, NULL);		break;//Kitao追加
		case WM_SCREEN_DIRECTDRAW:		APPEVENT_Set(APPEVENT_SCREEN_DIRECTDRAW, NULL);		break;//Kitao追加
		case WM_SCREEN_USE3DT_LINEAR:	APPEVENT_Set(APPEVENT_SCREEN_USE3DT_LINEAR, NULL);	break;//Kitao追加
		case WM_SCREEN_USE3DT_POINT:	APPEVENT_Set(APPEVENT_SCREEN_USE3DT_POINT, NULL);	break;//Kitao追加
		case WM_SCREEN_USE3DT_NONE:		APPEVENT_Set(APPEVENT_SCREEN_USE3DT_NONE, NULL);	break;//Kitao追加
		case WM_SCREEN_USE3DR_LINEAR:	APPEVENT_Set(APPEVENT_SCREEN_USE3DR_LINEAR, NULL);	break;//Kitao追加
		case WM_SCREEN_USE3DR_POINT:	APPEVENT_Set(APPEVENT_SCREEN_USE3DR_POINT, NULL);	break;//Kitao追加
		case WM_SCREEN_USE3DR_NONE:		APPEVENT_Set(APPEVENT_SCREEN_USE3DR_NONE, NULL);	break;//Kitao追加
		case WM_SCREEN_USE3D_HELP:		APPEVENT_Set(APPEVENT_SCREEN_USE3D_HELP, NULL);		break;//Kitao追加
		case WM_SCREEN_USE_VIDEOMEM:	APPEVENT_Set(APPEVENT_SCREEN_USE_VIDEOMEM, NULL);	break;//Kitao追加
		case WM_SCREEN_USE_SYSTEMMEM:	APPEVENT_Set(APPEVENT_SCREEN_USE_SYSTEMMEM, NULL);	break;//Kitao追加
		case WM_SCREEN_USE_SYSTEMMEMW:	APPEVENT_Set(APPEVENT_SCREEN_USE_SYSTEMMEMW, NULL);	break;//Kitao追加
		case WM_SCREEN_FULL16BITCOLOR:	APPEVENT_Set(APPEVENT_SCREEN_FULL16BITCOLOR, NULL);	break;//Kitao追加
		case WM_SCREEN_FULL32BITCOLOR:	APPEVENT_Set(APPEVENT_SCREEN_FULL32BITCOLOR, NULL);	break;//Kitao追加
		case WM_SCREEN_STARTWINDOW:		APPEVENT_Set(APPEVENT_SCREEN_STARTWINDOW, NULL);	break;//Kitao追加
		case WM_SCREEN_STARTFULL:		APPEVENT_Set(APPEVENT_SCREEN_STARTFULL, NULL);		break;//Kitao追加
		case WM_SCREEN_TOPMOST:			APPEVENT_Set(APPEVENT_SCREEN_TOPMOST, NULL);		break;//Kitao追加
		case WM_SCREEN_ACTIVATE:		APPEVENT_Set(APPEVENT_SCREEN_ACTIVATE, NULL);		break;//Kitao追加
		case WM_SCREEN_NONACTIVATE:		APPEVENT_Set(APPEVENT_SCREEN_NONACTIVATE, NULL);	break;//Kitao追加
		case WM_SCREEN_UNPAUSE:			APPEVENT_Set(APPEVENT_SCREEN_UNPAUSE, NULL);		break;//v2.26追加

		case WM_INPUT_TURBO_1:			APPEVENT_Set(APPEVENT_INPUT_TURBO_1, NULL);			break;//Kitao追加
		case WM_INPUT_TURBO_2:			APPEVENT_Set(APPEVENT_INPUT_TURBO_2, NULL);			break;//Kitao追加
		case WM_INPUT_TURBO_RUN:		APPEVENT_Set(APPEVENT_INPUT_TURBO_RUN, NULL);		break;//Kitao追加
		case WM_INPUT_TURBO_HIGH:		APPEVENT_Set(APPEVENT_INPUT_TURBO_HIGH, NULL);		break;//Kitao追加
		case WM_INPUT_TURBO_MIDDLE:		APPEVENT_Set(APPEVENT_INPUT_TURBO_MIDDLE, NULL);	break;//Kitao追加
		case WM_INPUT_TURBO_LOW:		APPEVENT_Set(APPEVENT_INPUT_TURBO_LOW, NULL);		break;//Kitao追加
		case WM_INPUT_TURBO_OFF:		APPEVENT_Set(APPEVENT_INPUT_TURBO_OFF, NULL);		break;//Kitao追加
		case WM_INPUT_TWO_BUTTON_PAD:	APPEVENT_Set(APPEVENT_INPUT_TWO_BUTTON_PAD, NULL);	break;
		case WM_INPUT_THR_BUTTON_PAD:	APPEVENT_Set(APPEVENT_INPUT_THR_BUTTON_PAD, NULL);	break;//Kitao追加
		case WM_INPUT_SIX_BUTTON_PAD:	APPEVENT_Set(APPEVENT_INPUT_SIX_BUTTON_PAD, NULL);	break;
		case WM_INPUT_MOUSE:			APPEVENT_Set(APPEVENT_INPUT_MOUSE, NULL);			break;
		case WM_INPUT_MULTI_TAP:		APPEVENT_Set(APPEVENT_INPUT_MULTI_TAP, NULL);		break;
		case WM_INPUT_MB128:			APPEVENT_Set(APPEVENT_INPUT_MB128, NULL);			break;
		case WM_INPUT_SWAP_SELRUN:		APPEVENT_Set(APPEVENT_INPUT_SWAP_SELRUN, NULL);		break;//Kitao追加
		case WM_INPUT_SWAP_IANDII:		APPEVENT_Set(APPEVENT_INPUT_SWAP_IANDII, NULL);		break;//Kitao追加
		case WM_INPUT_CHECKPAD_LR:		APPEVENT_Set(APPEVENT_INPUT_CHECKPAD_LR, NULL);		break;//Kitao追加

		case WM_INPUT_CONFIGURE_PAD1:	APPEVENT_Set(APPEVENT_INPUT_CONFIGURE_PAD1, NULL);	break;
		case WM_INPUT_CONFIGURE_PAD2:	APPEVENT_Set(APPEVENT_INPUT_CONFIGURE_PAD2, NULL);	break;
		case WM_INPUT_CONFIGURE_PAD3:	APPEVENT_Set(APPEVENT_INPUT_CONFIGURE_PAD3, NULL);	break;
		case WM_INPUT_CONFIGURE_PAD4:	APPEVENT_Set(APPEVENT_INPUT_CONFIGURE_PAD4, NULL);	break;
		case WM_INPUT_CONFIGURE_PAD5:	APPEVENT_Set(APPEVENT_INPUT_CONFIGURE_PAD5, NULL);	break;
		case WM_INPUT_CONFIGURE_TB1:	APPEVENT_Set(APPEVENT_INPUT_CONFIGURE_TB1, NULL);	break;//Kitao追加
		case WM_INPUT_CONFIGURE_TB2:	APPEVENT_Set(APPEVENT_INPUT_CONFIGURE_TB2, NULL);	break;//Kitao追加
		case WM_INPUT_CONFIGURE_TB3:	APPEVENT_Set(APPEVENT_INPUT_CONFIGURE_TB3, NULL);	break;//Kitao追加
		case WM_INPUT_CONFIGURE_TB4:	APPEVENT_Set(APPEVENT_INPUT_CONFIGURE_TB4, NULL);	break;//Kitao追加
		case WM_INPUT_CONFIGURE_TB5:	APPEVENT_Set(APPEVENT_INPUT_CONFIGURE_TB5, NULL);	break;//Kitao追加
		case WM_INPUT_CONFIGURE_INIT:	APPEVENT_Set(APPEVENT_INPUT_CONFIGURE_INIT, NULL);	break;//Kitao追加
		case WM_INPUT_CONFIGURE_KEYBG:	APPEVENT_Set(APPEVENT_INPUT_CONFIGURE_KEYBG, NULL);	break;//Kitao追加
		case WM_INPUT_CONFIGURE_JOYBG:	APPEVENT_Set(APPEVENT_INPUT_CONFIGURE_JOYBG, NULL);	break;//Kitao追加
		case WM_INPUT_FUNCTION:			APPEVENT_Set(APPEVENT_INPUT_FUNCTION, NULL);		break;//Kitao追加
		case WM_INPUT_FB_CURSOR:		APPEVENT_Set(APPEVENT_INPUT_FB_CURSOR, NULL);		break;//Kitao追加
		case WM_INPUT_FB_IandII:		APPEVENT_Set(APPEVENT_INPUT_FB_IandII, NULL);		break;//Kitao追加
		case WM_INPUT_FB_SEL:			APPEVENT_Set(APPEVENT_INPUT_FB_SEL, NULL);			break;//Kitao追加
		case WM_INPUT_FB_RUN:			APPEVENT_Set(APPEVENT_INPUT_FB_RUN, NULL);			break;//Kitao追加
		case WM_INPUT_FB_VSPEEDUP:		APPEVENT_Set(APPEVENT_INPUT_FB_VSPEEDUP, NULL);		break;//Kitao追加
		case WM_INPUT_FB_SAVELOAD:		APPEVENT_Set(APPEVENT_INPUT_FB_SAVELOAD, NULL);		break;//Kitao追加

/*		case WM_AUDIO_SR96000:			APPEVENT_Set(APPEVENT_AUDIO_SR96000, NULL);			break;
		case WM_AUDIO_SR88200:			APPEVENT_Set(APPEVENT_AUDIO_SR88200, NULL);			break;
		case WM_AUDIO_SR64000:			APPEVENT_Set(APPEVENT_AUDIO_SR64000, NULL);			break;
		case WM_AUDIO_SR48000:			APPEVENT_Set(APPEVENT_AUDIO_SR48000, NULL);			break;
		case WM_AUDIO_SR44100:			APPEVENT_Set(APPEVENT_AUDIO_SR44100, NULL);			break;
		case WM_AUDIO_SR32000:			APPEVENT_Set(APPEVENT_AUDIO_SR32000, NULL);			break;
		case WM_AUDIO_SR22050:			APPEVENT_Set(APPEVENT_AUDIO_SR22050, NULL);			break;
		case WM_AUDIO_SR11025:			APPEVENT_Set(APPEVENT_AUDIO_SR11025, NULL);			break;
*/
		case WM_AUDIO_SB1024:			APPEVENT_Set(APPEVENT_AUDIO_SB1024, NULL);			break;
		case WM_AUDIO_SB1152:			APPEVENT_Set(APPEVENT_AUDIO_SB1152, NULL);			break;//v2.37追加
		case WM_AUDIO_SB1280:			APPEVENT_Set(APPEVENT_AUDIO_SB1280, NULL);			break;
		case WM_AUDIO_SB1408:			APPEVENT_Set(APPEVENT_AUDIO_SB1408, NULL);			break;//v2.37追加
		case WM_AUDIO_SB1536:			APPEVENT_Set(APPEVENT_AUDIO_SB1536, NULL);			break;
		case WM_AUDIO_SB1664:			APPEVENT_Set(APPEVENT_AUDIO_SB1664, NULL);			break;//v1.28追加
		case WM_AUDIO_SB1792:			APPEVENT_Set(APPEVENT_AUDIO_SB1792, NULL);			break;
		case WM_AUDIO_SB2048:			APPEVENT_Set(APPEVENT_AUDIO_SB2048, NULL);			break;
		case WM_AUDIO_SB2176:			APPEVENT_Set(APPEVENT_AUDIO_SB2176, NULL);			break;//v2.20追加
		case WM_AUDIO_SB2304:			APPEVENT_Set(APPEVENT_AUDIO_SB2304, NULL);			break;
		case WM_AUDIO_SB2560:			APPEVENT_Set(APPEVENT_AUDIO_SB2560, NULL);			break;
		case WM_AUDIO_SB3072:			APPEVENT_Set(APPEVENT_AUDIO_SB3072, NULL);			break;
		case WM_AUDIO_HQPSG1:			APPEVENT_Set(APPEVENT_AUDIO_HQPSG1, NULL);			break;//v1.39追加
		case WM_AUDIO_HQPSG2:			APPEVENT_Set(APPEVENT_AUDIO_HQPSG2, NULL);			break;//v1.39追加
		case WM_AUDIO_HQPSG3:			APPEVENT_Set(APPEVENT_AUDIO_HQPSG3, NULL);			break;//v1.39追加

		case WM_AUDIO_STEREO:			APPEVENT_Set(APPEVENT_AUDIO_STEREO, NULL);			break;//Kitao追加
		case WM_AUDIO_MONO:				APPEVENT_Set(APPEVENT_AUDIO_MONO, NULL);			break;//Kitao追加

		case WM_AUDIO_INI1:				APPEVENT_Set(APPEVENT_AUDIO_INI1, NULL);			break;//Kitao追加
		case WM_AUDIO_INI2:				APPEVENT_Set(APPEVENT_AUDIO_INI2, NULL);			break;//Kitao追加

		case WM_AUDIO_NORMALBUFFER:		APPEVENT_Set(APPEVENT_AUDIO_NORMALBUFFER, NULL);	break;//Kitao追加
		case WM_AUDIO_BIGBUFFER:		APPEVENT_Set(APPEVENT_AUDIO_BIGBUFFER, NULL);		break;//Kitao追加
		case WM_AUDIO_MOSTBUFFER:		APPEVENT_Set(APPEVENT_AUDIO_MOSTBUFFER, NULL);		break;//Kitao追加

		case WM_AUDIO_CDDAAUTO:			APPEVENT_Set(APPEVENT_AUDIO_CDDAAUTO, NULL);		break;//Kitao追加
		case WM_AUDIO_CDDA593:			APPEVENT_Set(APPEVENT_AUDIO_CDDA593, NULL);			break;//Kitao追加
		case WM_AUDIO_CDDA594:			APPEVENT_Set(APPEVENT_AUDIO_CDDA594, NULL);			break;//Kitao追加
		case WM_AUDIO_CDDA595:			APPEVENT_Set(APPEVENT_AUDIO_CDDA595, NULL);			break;//Kitao追加
		case WM_AUDIO_CDDA596:			APPEVENT_Set(APPEVENT_AUDIO_CDDA596, NULL);			break;//Kitao追加
		case WM_AUDIO_CDDA597:			APPEVENT_Set(APPEVENT_AUDIO_CDDA597, NULL);			break;//Kitao追加
		case WM_AUDIO_CDDA598:			APPEVENT_Set(APPEVENT_AUDIO_CDDA598, NULL);			break;//Kitao追加
		case WM_AUDIO_CDDA599:			APPEVENT_Set(APPEVENT_AUDIO_CDDA599, NULL);			break;//Kitao追加
		case WM_AUDIO_CDDA600:			APPEVENT_Set(APPEVENT_AUDIO_CDDA600, NULL);			break;//Kitao追加
		case WM_AUDIO_CDDA601:			APPEVENT_Set(APPEVENT_AUDIO_CDDA601, NULL);			break;//Kitao追加
		case WM_AUDIO_CDDA602:			APPEVENT_Set(APPEVENT_AUDIO_CDDA602, NULL);			break;//Kitao追加
		case WM_AUDIO_CDDAP000:			APPEVENT_Set(APPEVENT_AUDIO_CDDAP000, NULL);		break;//Kitao追加
		case WM_AUDIO_CDDAP005:			APPEVENT_Set(APPEVENT_AUDIO_CDDAP005, NULL);		break;//Kitao追加
		case WM_AUDIO_SYNC_VBLANK:		APPEVENT_Set(APPEVENT_AUDIO_SYNC_VBLANK, NULL);		break;//Kitao追加。v2.65
		case WM_AUDIO_DELAYFRAME0:		APPEVENT_Set(APPEVENT_AUDIO_DELAYFRAME0, NULL);		break;//Kitao追加
		case WM_AUDIO_DELAYFRAME1:		APPEVENT_Set(APPEVENT_AUDIO_DELAYFRAME1, NULL);		break;//Kitao追加
		case WM_AUDIO_DELAYFRAME2:		APPEVENT_Set(APPEVENT_AUDIO_DELAYFRAME2, NULL);		break;//Kitao追加
		case WM_AUDIO_DELAYFRAME3:		APPEVENT_Set(APPEVENT_AUDIO_DELAYFRAME3, NULL);		break;//Kitao追加
		case WM_AUDIO_DELAYFRAME4:		APPEVENT_Set(APPEVENT_AUDIO_DELAYFRAME4, NULL);		break;//Kitao追加

		case WM_AUDIO_SETVOLUME:		APPEVENT_Set(APPEVENT_AUDIO_SETVOLUME, NULL);		break;
		case WM_AUDIO_DEFAULTVOLUME:	APPEVENT_Set(APPEVENT_AUDIO_DEFAULTVOLUME, NULL);	break;//Kitao追加
		case WM_VOLUME_NORMAL:			APPEVENT_Set(APPEVENT_VOLUME_NORMAL, NULL);			break;//Kitao追加
		case WM_VOLUME_3QUARTERS:		APPEVENT_Set(APPEVENT_VOLUME_3QUARTERS, NULL);		break;//Kitao追加
		case WM_VOLUME_HALF:			APPEVENT_Set(APPEVENT_VOLUME_HALF, NULL);			break;//Kitao追加
		case WM_VOLUME_QUARTER:			APPEVENT_Set(APPEVENT_VOLUME_QUARTER, NULL);		break;//Kitao追加
		case WM_VOLUME_MUTE:			APPEVENT_Set(APPEVENT_VOLUME_MUTE, NULL);			break;//Kitao追加
		case WM_VOLUME_DEFAULT:			APPEVENT_Set(APPEVENT_VOLUME_DEFAULT, NULL);		break;//Kitao追加
		case WM_VOLUME_12:				APPEVENT_Set(APPEVENT_VOLUME_12, NULL);				break;//Kitao追加。v2.62
		case WM_VOLUME_11:				APPEVENT_Set(APPEVENT_VOLUME_11, NULL);				break;//Kitao追加。v2.62
		case WM_VOLUME_10:				APPEVENT_Set(APPEVENT_VOLUME_10, NULL);				break;//Kitao追加
		case WM_VOLUME_9:				APPEVENT_Set(APPEVENT_VOLUME_9, NULL);				break;//Kitao追加
		case WM_VOLUME_8:				APPEVENT_Set(APPEVENT_VOLUME_8, NULL);				break;//Kitao追加
		case WM_VOLUME_7:				APPEVENT_Set(APPEVENT_VOLUME_7, NULL);				break;//Kitao追加
		case WM_VOLUME_6:				APPEVENT_Set(APPEVENT_VOLUME_6, NULL);				break;//Kitao追加
		case WM_VOLUME_5:				APPEVENT_Set(APPEVENT_VOLUME_5, NULL);				break;//Kitao追加
		case WM_VOLUME_4:				APPEVENT_Set(APPEVENT_VOLUME_4, NULL);				break;//Kitao追加
		case WM_VOLUME_3:				APPEVENT_Set(APPEVENT_VOLUME_3, NULL);				break;//Kitao追加
		case WM_VOLUME_2:				APPEVENT_Set(APPEVENT_VOLUME_2, NULL);				break;//Kitao追加
		case WM_VOLUME_1:				APPEVENT_Set(APPEVENT_VOLUME_1, NULL);				break;//Kitao追加
		case WM_VOLUME_STEP10:			APPEVENT_Set(APPEVENT_VOLUME_STEP10, NULL);			break;//Kitao追加
		case WM_VOLUME_STEP8:			APPEVENT_Set(APPEVENT_VOLUME_STEP8, NULL);			break;//Kitao追加
		case WM_VOLUME_STEP6:			APPEVENT_Set(APPEVENT_VOLUME_STEP6, NULL);			break;//Kitao追加
		case WM_VOLUME_STEP5:			APPEVENT_Set(APPEVENT_VOLUME_STEP5, NULL);			break;//Kitao追加
		case WM_VOLUME_STEP4:			APPEVENT_Set(APPEVENT_VOLUME_STEP4, NULL);			break;//Kitao追加
		case WM_VOLUME_STEP3:			APPEVENT_Set(APPEVENT_VOLUME_STEP3, NULL);			break;//Kitao追加
		case WM_VOLUME_STEP2:			APPEVENT_Set(APPEVENT_VOLUME_STEP2, NULL);			break;//Kitao追加
		case WM_VOLUME_STEP1:			APPEVENT_Set(APPEVENT_VOLUME_STEP1, NULL);			break;//Kitao追加
		case WM_VOLUME_DETAILUP:		APPEVENT_Set(APPEVENT_VOLUME_DETAILUP, NULL);		break;//Kitao追加
		case WM_VOLUME_DETAILDN:		APPEVENT_Set(APPEVENT_VOLUME_DETAILDN, NULL);		break;//Kitao追加
		case WM_VOLUME_ATTENTION:		APPEVENT_Set(APPEVENT_VOLUME_ATTENTION, NULL);		break;//Kitao追加
		case WM_VOLUME_CONTROL:			APPEVENT_Set(APPEVENT_VOLUME_CONTROL, NULL);		break;//Kitao追加
		case WM_VOLUME_MUTE1:			APPEVENT_Set(APPEVENT_VOLUME_MUTE1, NULL);			break;//Kitao追加
		case WM_VOLUME_MUTE2:			APPEVENT_Set(APPEVENT_VOLUME_MUTE2, NULL);			break;//Kitao追加
		case WM_VOLUME_MUTE3:			APPEVENT_Set(APPEVENT_VOLUME_MUTE3, NULL);			break;//Kitao追加
		case WM_VOLUME_MUTE4:			APPEVENT_Set(APPEVENT_VOLUME_MUTE4, NULL);			break;//Kitao追加
		case WM_VOLUME_MUTE5:			APPEVENT_Set(APPEVENT_VOLUME_MUTE5, NULL);			break;//Kitao追加
		case WM_VOLUME_MUTE6:			APPEVENT_Set(APPEVENT_VOLUME_MUTE6, NULL);			break;//Kitao追加
		case WM_VOLUME_MUTEA:			APPEVENT_Set(APPEVENT_VOLUME_MUTEA, NULL);			break;//Kitao追加
		case WM_VOLUME_MUTEU:			APPEVENT_Set(APPEVENT_VOLUME_MUTEU, NULL);			break;//Kitao追加

		case WM_INFO_SHOWFPS:			APPEVENT_Set(APPEVENT_INFO_SHOWFPS, NULL);			break;//Kitao追加
		case WM_INFO_TESTDELAY:			APPEVENT_Set(APPEVENT_INFO_TESTDELAY, NULL);		break;//Kitao追加
		case WM_INFO_MANUENGLISH:		APPEVENT_Set(APPEVENT_INFO_MANUENGLISH, NULL);		break;//Kitao追加
		case WM_INFO_MANUJAPANESE:		APPEVENT_Set(APPEVENT_INFO_MANUJAPANESE, NULL);		break;//Kitao追加
		case WM_INFO_README:			APPEVENT_Set(APPEVENT_INFO_README, NULL);			break;//Kitao追加
		case WM_INFO_HOMEPAGE:			APPEVENT_Set(APPEVENT_INFO_HOMEPAGE, NULL);			break;//Kitao追加
		case WM_INFO_VERSION:			APPEVENT_Set(APPEVENT_INFO_VERSION, NULL);			break;//Kitao追加

		case WM_DEVICE_CD0:				APPEVENT_Set(APPEVENT_DEVICE_CD0, NULL);			break;//Kitao追加
		case WM_DEVICE_CD1:				APPEVENT_Set(APPEVENT_DEVICE_CD1, NULL);			break;//Kitao追加
		case WM_DEVICE_CD2:				APPEVENT_Set(APPEVENT_DEVICE_CD2, NULL);			break;//Kitao追加
		case WM_DEVICE_CD3:				APPEVENT_Set(APPEVENT_DEVICE_CD3, NULL);			break;//Kitao追加
		case WM_DEVICE_CD4:				APPEVENT_Set(APPEVENT_DEVICE_CD4, NULL);			break;//Kitao追加
		case WM_DEVICE_CD5:				APPEVENT_Set(APPEVENT_DEVICE_CD5, NULL);			break;//Kitao追加
		case WM_DEVICE_CD6:				APPEVENT_Set(APPEVENT_DEVICE_CD6, NULL);			break;//Kitao追加
		case WM_DEVICE_CD7:				APPEVENT_Set(APPEVENT_DEVICE_CD7, NULL);			break;//Kitao追加
		case WM_DEVICE_CD8:				APPEVENT_Set(APPEVENT_DEVICE_CD8, NULL);			break;//Kitao追加
		case WM_DEVICE_CD9:				APPEVENT_Set(APPEVENT_DEVICE_CD9, NULL);			break;//Kitao追加
		case WM_CD_PLAYINSTALL:			APPEVENT_Set(APPEVENT_CD_PLAYINSTALL, NULL);		break;//Kitao追加
		case WM_CD_INSTALL:				APPEVENT_Set(APPEVENT_CD_INSTALL, NULL);			break;//Kitao追加
		case WM_CD_FULLINSTALL:			APPEVENT_Set(APPEVENT_CD_FULLINSTALL, NULL);		break;//Kitao追加
		case WM_CD_UNINSTALL:			APPEVENT_Set(APPEVENT_CD_UNINSTALL, NULL);			break;//Kitao追加
		case WM_CD_OPENINSTALL:			APPEVENT_Set(APPEVENT_CD_OPENINSTALL, NULL);		break;//Kitao追加
		case WM_CD_SETSYSCARD:			APPEVENT_Set(APPEVENT_CD_SETSYSCARD, NULL);			break;//Kitao追加
		case WM_CD_SETSYSCARD1:			APPEVENT_Set(APPEVENT_CD_SETSYSCARD1, NULL);		break;//Kitao追加
		case WM_CD_SETSYSCARD2:			APPEVENT_Set(APPEVENT_CD_SETSYSCARD2, NULL);		break;//Kitao追加
		case WM_CD_JUUOUKI:				APPEVENT_Set(APPEVENT_CD_JUUOUKI, NULL);			break;//Kitao追加
		case WM_CD_OSYSCARD1:			APPEVENT_Set(APPEVENT_CD_OSYSCARD1, NULL);			break;//Kitao追加
		case WM_CD_OSYSCARD2:			APPEVENT_Set(APPEVENT_CD_OSYSCARD2, NULL);			break;//Kitao追加
		case WM_CD_BACKUPFULL:			APPEVENT_Set(APPEVENT_CD_BACKUPFULL, NULL);			break;//Kitao追加
		case WM_CD_ARCADECARD:			APPEVENT_Set(APPEVENT_CD_ARCADECARD, NULL);			break;//Kitao追加
		case WM_CD_HELP:				APPEVENT_Set(APPEVENT_CD_HELP, NULL);				break;//Kitao追加

		case WM_CHANGE_CDC:				APPEVENT_Set(APPEVENT_CHANGE_CDC, NULL);			break;//Kitao追加
		case WM_CHANGE_CD0:				APPEVENT_Set(APPEVENT_CHANGE_CD0, NULL);			break;//Kitao追加
		case WM_CHANGE_CD1:				APPEVENT_Set(APPEVENT_CHANGE_CD1, NULL);			break;//Kitao追加
		case WM_CHANGE_CD2:				APPEVENT_Set(APPEVENT_CHANGE_CD2, NULL);			break;//Kitao追加
		case WM_CHANGE_CD3:				APPEVENT_Set(APPEVENT_CHANGE_CD3, NULL);			break;//Kitao追加
		case WM_CHANGE_CD4:				APPEVENT_Set(APPEVENT_CHANGE_CD4, NULL);			break;//Kitao追加
		case WM_CHANGE_CD5:				APPEVENT_Set(APPEVENT_CHANGE_CD5, NULL);			break;//Kitao追加
		case WM_CHANGE_CD6:				APPEVENT_Set(APPEVENT_CHANGE_CD6, NULL);			break;//Kitao追加
		case WM_CHANGE_CD7:				APPEVENT_Set(APPEVENT_CHANGE_CD7, NULL);			break;//Kitao追加
		case WM_CHANGE_CD8:				APPEVENT_Set(APPEVENT_CHANGE_CD8, NULL);			break;//Kitao追加
		case WM_CHANGE_CD9:				APPEVENT_Set(APPEVENT_CHANGE_CD9, NULL);			break;//Kitao追加
		case WM_F1_NORESET:				APPEVENT_Set(APPEVENT_F1_NORESET, NULL);			break;//Kitao追加

		case WM_STARTFASTCD_ON:			APPEVENT_Set(APPEVENT_STARTFASTCD_ON, NULL);		break;//Kitao追加
		case WM_STARTFASTCD_OFF:		APPEVENT_Set(APPEVENT_STARTFASTCD_OFF, NULL);		break;//Kitao追加
		case WM_STARTFASTCD_PRE:		APPEVENT_Set(APPEVENT_STARTFASTCD_PRE, NULL);		break;//Kitao追加

		case WM_STARTFASTSEEK_ON:		APPEVENT_Set(APPEVENT_STARTFASTSEEK_ON, NULL);		break;//Kitao追加
		case WM_STARTFASTSEEK_OFF:		APPEVENT_Set(APPEVENT_STARTFASTSEEK_OFF, NULL);		break;//Kitao追加
		case WM_STARTFASTSEEK_PRE:		APPEVENT_Set(APPEVENT_STARTFASTSEEK_PRE, NULL);		break;//Kitao追加

		case WM_AUTO_GRADIUS2:			APPEVENT_Set(APPEVENT_AUTO_GRADIUS2, NULL);			break;//Kitao追加
		case WM_AUTO_MEGATEN:			APPEVENT_Set(APPEVENT_AUTO_MEGATEN, NULL);			break;//Kitao追加
		case WM_STARTSPRITE_OFF:		APPEVENT_Set(APPEVENT_STARTSPRITE_OFF, NULL);		break;//Kitao追加
		case WM_STARTSPRITE_ON:			APPEVENT_Set(APPEVENT_STARTSPRITE_ON, NULL);		break;//Kitao追加
		case WM_STARTSPRITE_PRE:		APPEVENT_Set(APPEVENT_STARTSPRITE_PRE, NULL);		break;//Kitao追加

		case WM_MENU_HIDEMENU:			APPEVENT_Set(APPEVENT_MENU_HIDEMENU, NULL);			break;//Kitao追加
		case WM_MENU_HIDEMESSAGE:		APPEVENT_Set(APPEVENT_MENU_HIDEMESSAGE, NULL);		break;//Kitao追加

		case WM_PRIORITY_HIGH:			APPEVENT_Set(APPEVENT_PRIORITY_HIGH, NULL);			break;//Kitao追加
		case WM_PRIORITY_NORMAL:		APPEVENT_Set(APPEVENT_PRIORITY_NORMAL, NULL);		break;//Kitao追加

		case WM_SPEED_V0:				APPEVENT_Set(APPEVENT_SPEED_V0, NULL);				break;//Kitao追加
		case WM_SPEED_V1:				APPEVENT_Set(APPEVENT_SPEED_V1, NULL);				break;//Kitao追加
		case WM_SPEED_V2:				APPEVENT_Set(APPEVENT_SPEED_V2, NULL);				break;//Kitao追加
		case WM_SPEED_V3:				APPEVENT_Set(APPEVENT_SPEED_V3, NULL);				break;//Kitao追加
		case WM_SPEED_V4:				APPEVENT_Set(APPEVENT_SPEED_V4, NULL);				break;//Kitao追加
		case WM_SPEED_V5:				APPEVENT_Set(APPEVENT_SPEED_V5, NULL);				break;//Kitao追加
		case WM_SPEED_V6:				APPEVENT_Set(APPEVENT_SPEED_V6, NULL);				break;//Kitao追加
		case WM_SPEED_V7:				APPEVENT_Set(APPEVENT_SPEED_V7, NULL);				break;//Kitao追加
		case WM_SPEED_V8:				APPEVENT_Set(APPEVENT_SPEED_V8, NULL);				break;//Kitao追加
		case WM_SPEED_V9:				APPEVENT_Set(APPEVENT_SPEED_V9, NULL);				break;//Kitao追加
		case WM_SPEED_VAL:				APPEVENT_Set(APPEVENT_SPEED_VAL, NULL);				break;//Kitao追加
		case WM_SPEED_VUSE:				APPEVENT_Set(APPEVENT_SPEED_VUSE, NULL);			break;//Kitao追加
		case WM_SPEED_VSET:				APPEVENT_Set(APPEVENT_SPEED_VSET, NULL);			break;//Kitao追加
		case WM_SPEED_P0:				APPEVENT_Set(APPEVENT_SPEED_P0, NULL);				break;//Kitao追加
		case WM_SPEED_T3:				APPEVENT_Set(APPEVENT_SPEED_T3, NULL);				break;//Kitao追加
		case WM_SPEED_T2:				APPEVENT_Set(APPEVENT_SPEED_T2, NULL);				break;//Kitao追加
		case WM_SPEED_T1:				APPEVENT_Set(APPEVENT_SPEED_T1, NULL);				break;//Kitao追加
		case WM_SPEED_P6:				APPEVENT_Set(APPEVENT_SPEED_P6, NULL);				break;//Kitao追加
		case WM_SPEED_P5:				APPEVENT_Set(APPEVENT_SPEED_P5, NULL);				break;//Kitao追加
		case WM_SPEED_P4:				APPEVENT_Set(APPEVENT_SPEED_P4, NULL);				break;//Kitao追加
		case WM_SPEED_P3:				APPEVENT_Set(APPEVENT_SPEED_P3, NULL);				break;//Kitao追加
		case WM_SPEED_P2:				APPEVENT_Set(APPEVENT_SPEED_P2, NULL);				break;//Kitao追加
		case WM_SPEED_P1:				APPEVENT_Set(APPEVENT_SPEED_P1, NULL);				break;//Kitao追加
		case WM_SPEED_M1:				APPEVENT_Set(APPEVENT_SPEED_M1, NULL);				break;//Kitao追加
		case WM_SPEED_M2:				APPEVENT_Set(APPEVENT_SPEED_M2, NULL);				break;//Kitao追加
		case WM_SPEED_CSET:				APPEVENT_Set(APPEVENT_SPEED_CSET, NULL);			break;//Kitao追加
		case WM_SPEED_UNLOAD:			APPEVENT_Set(APPEVENT_SPEED_UNLOAD, NULL);			break;//Kitao追加
		case WM_SPEED_LOAD:				APPEVENT_Set(APPEVENT_SPEED_LOAD, NULL);			break;//Kitao追加
		case WM_SPEED_FASTCD:			APPEVENT_Set(APPEVENT_SPEED_FASTCD, NULL);			break;//Kitao追加
		case WM_SPEED_FASTSEEK:			APPEVENT_Set(APPEVENT_SPEED_FASTSEEK, NULL);		break;//Kitao追加
		case WM_SPEED_HELP:				APPEVENT_Set(APPEVENT_SPEED_HELP, NULL);			break;//Kitao追加

		case WM_RASTERTIMING_MEARLY:	APPEVENT_Set(APPEVENT_RASTERTIMING_MEARLY, NULL);	break;//Kitao追加
		case WM_RASTERTIMING_EARLY:		APPEVENT_Set(APPEVENT_RASTERTIMING_EARLY, NULL);	break;//Kitao追加
		case WM_RASTERTIMING_MIDDLE:	APPEVENT_Set(APPEVENT_RASTERTIMING_MIDDLE, NULL);	break;//Kitao追加
		case WM_RASTERTIMING_LATE:		APPEVENT_Set(APPEVENT_RASTERTIMING_LATE, NULL);		break;//Kitao追加
		case WM_RASTERTIMING_MLATE:		APPEVENT_Set(APPEVENT_RASTERTIMING_MLATE, NULL);	break;//Kitao追加

		case WM_INVALIDATE_CDINST:		APPEVENT_Set(APPEVENT_INVALIDATE_CDINST, NULL);		break;//Kitao追加
		case WM_SUPERGRAFX:				APPEVENT_Set(APPEVENT_SUPERGRAFX, NULL);			break;//Kitao追加
		case WM_SPRITEOVER:				APPEVENT_Set(APPEVENT_SPRITEOVER, NULL);			break;//Kitao追加

		case WM_LAYER_SPRITE:			APPEVENT_Set(APPEVENT_LAYER_SPRITE, NULL);			break;//Kitao追加
		case WM_LAYER_BG:				APPEVENT_Set(APPEVENT_LAYER_BG, NULL);				break;//Kitao追加
		case WM_LAYER_SPRITE2:			APPEVENT_Set(APPEVENT_LAYER_SPRITE2, NULL);			break;//Kitao追加
		case WM_LAYER_BG2:				APPEVENT_Set(APPEVENT_LAYER_BG2, NULL);				break;//Kitao追加

		case WM_BIT_CONVERT:			APPEVENT_Set(APPEVENT_BIT_CONVERT, NULL);			break;//Kitao追加
		case WM_SHOW_DEBUG:				APPEVENT_Set(APPEVENT_SHOW_DEBUG, NULL);			break;//Kitao追加

		case WM_ALL_DEFAULT:			APPEVENT_Set(APPEVENT_ALL_DEFAULT, NULL);			break;//Kitao追加

		case WM_OUTPUT_SCREENSHOT:		APPEVENT_Set(APPEVENT_OUTPUT_SCREENSHOT, NULL);		break;//Kitao追加
		case WM_FOLDER_SCREENSHOT:		APPEVENT_Set(APPEVENT_FOLDER_SCREENSHOT, NULL);		break;//Kitao追加
		case WM_SCREENSHOT_BUTTON:		APPEVENT_Set(APPEVENT_SCREENSHOT_BUTTON, NULL);		break;//Kitao追加
		case WM_SCREENSHOT_DEFAULT:		APPEVENT_Set(APPEVENT_SCREENSHOT_DEFAULT, NULL);	break;//Kitao追加
		case WM_SCREENSHOT_X1:			APPEVENT_Set(APPEVENT_SCREENSHOT_X1, NULL);			break;//Kitao追加
		case WM_SCREENSHOT_X2:			APPEVENT_Set(APPEVENT_SCREENSHOT_X2, NULL);			break;//Kitao追加
		case WM_SCREENSHOT_X3:			APPEVENT_Set(APPEVENT_SCREENSHOT_X3, NULL);			break;//Kitao追加
		case WM_SCREENSHOT_X4:			APPEVENT_Set(APPEVENT_SCREENSHOT_X4, NULL);			break;//Kitao追加
		case WM_SCREENSHOT_XN:			APPEVENT_Set(APPEVENT_SCREENSHOT_XN, NULL);			break;//Kitao追加
		case WM_SCREENSHOT_SIZE:		APPEVENT_Set(APPEVENT_SCREENSHOT_SIZE, NULL);		break;//Kitao追加
		case WM_SSHOT_SAMEPLAYING:		APPEVENT_Set(APPEVENT_SSHOT_SAMEPLAYING, NULL);		break;//Kitao追加
		case WM_SSHOT_SCANLINED:		APPEVENT_Set(APPEVENT_SSHOT_SCANLINED, NULL);		break;//Kitao追加
		case WM_SSHOT_TVMODE:			APPEVENT_Set(APPEVENT_SSHOT_TVMODE, NULL);			break;//Kitao追加
		case WM_SSHOT_HRSCANLINED:		APPEVENT_Set(APPEVENT_SSHOT_HRSCANLINED, NULL);		break;//Kitao追加
		case WM_SSHOT_NONSCANLINED:		APPEVENT_Set(APPEVENT_SSHOT_NONSCANLINED, NULL);	break;//Kitao追加
		case WM_SSHOT_SAVEFOLDER:		APPEVENT_Set(APPEVENT_SSHOT_SAVEFOLDER, NULL);		break;//Kitao追加
		case WM_SSHOT_SAVEDIALOG:		APPEVENT_Set(APPEVENT_SSHOT_SAVEDIALOG, NULL);		break;//Kitao追加
		case WM_OUTPUT_WAV1:			APPEVENT_Set(APPEVENT_OUTPUT_WAV1, NULL);			break;//Kitao追加
		case WM_OUTPUT_WAV123:			APPEVENT_Set(APPEVENT_OUTPUT_WAV123, NULL);			break;//Kitao追加
		case WM_OUTPUT_WAV12:			APPEVENT_Set(APPEVENT_OUTPUT_WAV12, NULL);			break;//Kitao追加
		case WM_OUTPUT_WAV2:			APPEVENT_Set(APPEVENT_OUTPUT_WAV2, NULL);			break;//Kitao追加
		case WM_OUTPUT_WAV3:			APPEVENT_Set(APPEVENT_OUTPUT_WAV3, NULL);			break;//Kitao追加
		case WM_OUTPUT_WAV0:			APPEVENT_Set(APPEVENT_OUTPUT_WAV0, NULL);			break;//Kitao追加
		case WM_OUTPUT_WAVS1:			APPEVENT_Set(APPEVENT_OUTPUT_WAVS1, NULL);			break;//Kitao追加
		case WM_OUTPUT_WAVS2:			APPEVENT_Set(APPEVENT_OUTPUT_WAVS2, NULL);			break;//Kitao追加
		case WM_OUTPUT_WAVNT:			APPEVENT_Set(APPEVENT_OUTPUT_WAVNT, NULL);			break;//Kitao追加
		case WM_OUTPUT_WAVBE:			APPEVENT_Set(APPEVENT_OUTPUT_WAVBE, NULL);			break;//Kitao追加
		case WM_OUTPUT_AVI:				APPEVENT_Set(APPEVENT_OUTPUT_AVI, NULL);			break;//Kitao追加
		case WM_ABOUT_AVI:				APPEVENT_Set(APPEVENT_ABOUT_AVI, NULL);				break;//Kitao追加

		case WM_EXIT:					SendMessage(hWnd, WM_CLOSE, 0, 0);					break;
		}

		//	##RA
		if( LOWORD(wParam) >= IDM_RA_MENUSTART &&
			LOWORD(wParam) < IDM_RA_MENUEND )
		{
			RA_InvokeDialog( LOWORD(wParam) );
			break;
		}

		break;

	case WM_SYSCOMMAND:
		if ((wParam == SC_SCREENSAVE)||(wParam == SC_MONITORPOWER))
			return 1;
		break;

	default:
		break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


static HWND
create_main_window(
	HINSTANCE		hInstance,
	int				nCmdShow)
{
	WNDCLASS	wc;
	HWND		hWnd;
	const char	className[] = MAINCAPTION;
	RECT		rc;

	ZeroMemory(&wc, sizeof(wc));
	wc.style		 = 0;
	wc.lpfnWndProc	 = wnd_proc;
	wc.cbClsExtra	 = 0;
	wc.cbWndExtra	 = 0;
	wc.hInstance	 = hInstance;
	wc.hIcon		 = LoadIcon(hInstance, MAKEINTRESOURCE(OOTAKEICON)); //Kitao更新。アイコンを読み込み。v2.00更新
	wc.hCursor		 = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName	 = "";
	wc.lpszClassName = className;

	if (RegisterClass(&wc) == 0)
		return NULL;

	hWnd = CreateWindow(
		className,
		className,
		WS_MINIMIZEBOX | WS_SYSMENU | WS_CAPTION | WS_DLGFRAME, //Kitao更新。ウィンドウサイズ変更不可に。
		20,
		16,
		598,
		16, //Kitao更新。見栄えがいいように起動時は横棒状態で起動するようにした。
		NULL,
		NULL,
		hInstance,
		NULL
	);

	if (hWnd == NULL)
		return NULL;

	//Kitao更新
	ShowWindow(hWnd, nCmdShow);
	SetWindowLong(hWnd, GWL_STYLE, GetWindowLong(hWnd, GWL_STYLE) | (WS_MINIMIZEBOX | WS_SYSMENU | WS_CAPTION | WS_DLGFRAME));
	SetRect(&rc, 0, 0, 598, 16);
	AdjustWindowRect(&rc, GetWindowLong(hWnd, GWL_STYLE), GetMenu(hWnd) != NULL);
	SetWindowPos(hWnd, HWND_TOP, 20, 16, rc.right - rc.left, rc.bottom - rc.top, SWP_FRAMECHANGED); //Kitao更新。ゲーム動作中は常に手前に表示。DirectXでタスクバーに被ったときにゴミが出ないようにするため。
	UpdateWindow(hWnd);
	ImmAssociateContext(hWnd, 0); //Kitao追加。IMEを無効にする。v0.79

	// Drag&Dropを許可する 
	DragAcceptFiles(hWnd, TRUE);

	return hWnd;
}


static void
deinit_argv(
	int		argc,
	char**	ppArgv)
{
	while (argc--)
		GlobalFree((HGLOBAL)ppArgv[argc]);

	GlobalFree((HGLOBAL)ppArgv);
}


const HINSTANCE
WINMAIN_GetHInstance()
{
	return _hInstance;
}

const HWND
WINMAIN_GetHwnd()
{
	return _hMainWnd;
}


void
WINMAIN_SetCaption(
	const char*		pCaption)
{
	if (pCaption)
	{
		sprintf(_Caption, "%s", &(*pCaption)); //Kitao追加。v1.50fixed(for gcc). Thanks for Nao-san report.
		RA_UpdateAppTitle( pCaption );
		//SetWindowText(_hMainWnd, pCaption);
	}
	else
	{
		RA_UpdateAppTitle( "" );
		//SetWindowText(_hMainWnd, APP_GetAppName());
	}
}


//Kitao追加
char*
WINMAIN_GetCaption()
{
	return _Caption;
}


Uint32
WINMAIN_ShowCursor(
	BOOL		bShow)
{
	if (bShow)
	{
		while (ShowCursor(bShow) < 0);
		APP_ResetMouseStopCount(); //Kitao追加。マウスを自動的に隠すためのカウンタをリセットする。
	}
	else
		while (ShowCursor(bShow) >= 0);

	return 0;
}

void
WINMAIN_SetFullScreenWindow(
	Sint32		width,
	Sint32		height)
{
	set_fullscreen_windowstyle(_hMainWnd, (int)width, (int)height); //ウィンドウスタイルの変更はすぐに行うようにした。処理速度アップ。v2.23
	APPEVENT_Set(APPEVENT_REDRAWSCREEN, NULL); //ウィンドウスタイル変更完了後に画面を再描画
}


void
WINMAIN_SetNormalWindow(
	Sint32		width,
	Sint32		height)
{
	set_normal_windowstyle(_hMainWnd, (int)width, (int)height); //ウィンドウスタイルの変更はすぐに行うようにした。処理速度アップ。v2.23
	APPEVENT_Set(APPEVENT_REDRAWSCREEN, NULL); //ウィンドウスタイル変更完了後に画面を再描画
}

//Kitao追加。v2.04
void
WINMAIN_SetPriority(
	DWORD	dwPriorityClass)
{
	HANDLE	hProcess;

	hProcess = OpenProcess(PROCESS_SET_INFORMATION, TRUE, GetCurrentProcessId());
	SetPriorityClass(hProcess, dwPriorityClass);
	CloseHandle(hProcess);
}


int
WINAPI
WinMain(
	HINSTANCE	hInstance,
	HINSTANCE	hPrevInstance,
	LPSTR		lpCmdLine,
	int			nCmdShow)
{
	char**		argv;
	int			argc;
	int			ret;

	_hInstance = hInstance;

	argc = init_argc(lpCmdLine);
	argv = init_argv(argc, lpCmdLine);

	if (argv == NULL)
		return -1;

	//Kitao追加。プロセスの優先度を上げる。(Vistaのウィンドウモードで必須。＆音の安定性向上)
	WINMAIN_SetPriority(NORMAL_PRIORITY_CLASS); //v2.04更新

	//g_UserImageFactoryInst;

	_hMainWnd = create_main_window(hInstance, nCmdShow);

	if (_hMainWnd == NULL)
	{
		deinit_argv(argc, argv);
		return -1;
	}

	ret = __main__(argc, argv);

	deinit_argv(argc, argv);

	return ret;
}


//Kitao追加。セーフティにウェイト処理を行うための関数。v2.42追加
//			 Sleep(0)で無限ループになってしまうとOSごと固まる危険があるので、一定の時間が経過した場合はSleep(1)に切り替えて待つ。
void
WINMAIN_SafetySleepZero(
	DWORD	startTime,
	DWORD	endTime)
{
	DWORD	t = timeGetTime();

	if ((t >= endTime)||(t < startTime)) //一定時間に達したか、もしくはタイマーカウンタがオーバーフローして0に戻った場合
		Sleep(1); //Sleep(0)と違い、CPUを占有しないので、安全にOotakeを終了できる。
	else
		Sleep(0); //Sleep(1)より細かなタイミングでウェイトができる。微妙なタイミングが必要な場合で有効。
}


//Kitao追加。デバッグ用
void
WINMAIN_SetBreakTrap(
	BOOL bBrakTrap)
{
	_bBreakTrap = bBrakTrap;
}

//Kitao追加。デバッグ用
BOOL
WINMAIN_GetBreakTrap()
{
	return _bBreakTrap;
}
