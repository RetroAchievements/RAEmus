/******************************************************************************
Ootake

 [Debug.h]
	デバッグ用ウィンドウの表示。ディスアセンブルなどを行う。

Copyright(C)2006-2007 Kitao Nakamura.
	改造版・後継版を公開なさるときは必ずソースコードを添付してください。
	その際に事後でかまいませんので、ひとことお知らせいただけると幸いです。
	商的な利用は禁じます。
	あとは「GNU General Public License(一般公衆利用許諾契約書)」に準じます。

******************************************************************************/
#ifndef DEBUG_H_INCLUDED
#define DEBUG_H_INCLUDED

#include "TypeDefs.h"
#include <windows.h>


BOOL
DEBUG_Init(
	HINSTANCE	hInstance);

void
DEBUG_Deinit();

const HWND
DEBUG_GetHwnd();

void
DEBUG_SetPause(
	BOOL	pause);

BOOL
DEBUG_GetPause();

BOOL
DEBUG_GetPauseLong();


#endif /* DEBUG_H_INCLUDED */
