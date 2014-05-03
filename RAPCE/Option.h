/******************************************************************************
Ootake

 [Option.h]
	スタート時に設定を変更するためのフォーム

Copyright(C)2006-2008 Kitao Nakamura.
	改造版・後継版を公開なさるときは必ずソースコードを添付してください。
	その際に事後でかまいませんので、ひとことお知らせいただけると幸いです。
	商的な利用は禁じます。
	あとは「GNU General Public License(一般公衆利用許諾契約書)」に準じます。

******************************************************************************/
#ifndef OPTION_H_INCLUDED
#define OPTION_H_INCLUDED

#include "TypeDefs.h"
#include <windows.h>


BOOL
OPTION_Init(
	HINSTANCE	hInstance,
	Sint32*		screenType,
	Sint32*		drawMode,
	Sint32*		drawMethod,
	Sint32*		vsync,
	Sint32*		psgType,
	Sint32*		soundBuffer);

void
OPTION_Deinit();


#endif /* OPTION_H_INCLUDED */
