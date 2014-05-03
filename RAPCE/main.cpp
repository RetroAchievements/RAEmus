/******************************************************************************
Ootake
・二重起動をすると問題が起こる場合があるので、二重起動は防止するようにした。
・COMの初期化と開放をここでやるようにした。Vistaでファイルダイアログ使用時に
  初期化しておかないと不安定だった。v1.05
・マルチメディアタイマの精度をここで上げておくようにした。v1.55

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
	[main]
		本プロジェクトのメイン関数です．

		The main function of the project.

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

#define _WIN32_DCOM //v2.17更新

#include <objbase.h>
#include "App.h"
#include "RA_ImageFactory.h"

// gcc は main が定義されていると
// WinMain よりも先に main を呼んでしまうらしい．．．
// __main__ というのはその workaround.
int
__main__(
	int			argc,
	char**		argv)
{
	HANDLE		hMutex;
	TIMECAPS	tc;

	//Kitao追加。二重起動を防止
	hMutex = CreateMutex(NULL, TRUE, "Ootake Emulator"); //ミューテックスの作成
	if (GetLastError() == ERROR_ALREADY_EXISTS) //すでにOotakeを起動していたら
		return 0; //起動せずに終了

	//CoInitializeEx(NULL, COINIT_MULTITHREADED); //Kitao追加。v2.17更新。参考：アパートメント(COINIT_APARTMENTTHREADED)だと音が少し硬い感じになる(おそらく直のMTAよりSTAのほうが処理間隔が長い)。v2.19記
	//CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE); //Kitao追加。v2.17更新。参考：アパートメント(COINIT_APARTMENTTHREADED)だと音が少し硬い感じになる(おそらく直のMTAよりSTAのほうが処理間隔が長い)。v2.19記
	timeGetDevCaps(&tc, sizeof(tc));
	timeBeginPeriod(tc.wPeriodMin); //Kitao追加。タイマ精度をここで上げておくようにした。

	if (!APP_Init(argc, argv))
		return -1;

	while (APP_ProcessEvents() != APP_QUIT);

	APP_Deinit();

	timeEndPeriod(tc.wPeriodMin); //Kitao追加
	//CoUninitialize(); //Kitao追加

	return 0;
}
