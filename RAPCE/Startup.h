/******************************************************************************
Ootake

 [Srartup.h]
	スタート時のメッセージ表示＆CD-ROM・Huカードの選択

Copyright(C)2006 Kitao Nakamura.
	改造版・後継版を公開なさるときは必ずソースコードを添付してください。
	その際に事後でかまいませんので、ひとことお知らせいただけると幸いです。
	商的な利用は禁じます。
	あとは「GNU General Public License(一般公衆利用許諾契約書)」に準じます。

******************************************************************************/
#ifndef STARTUP_H_INCLUDED
#define STARTUP_H_INCLUDED

#include "TypeDefs.h"
#include <windows.h>


BOOL
STARTUP_Init(
	HINSTANCE	hInstance,
	Sint32*		CDorHu);

void
STARTUP_Deinit();

const HWND
STARTUP_GetHwnd();


#endif /* STARTUP_H_INCLUDED */
