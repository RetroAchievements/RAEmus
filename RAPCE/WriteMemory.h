/******************************************************************************
Ootake

 [WriteMemory.h]
	メモリ内容書き換えのコマンドを入力するためのフォーム

Copyright(C)2006-2010 Kitao Nakamura.
	改造版・後継版を公開なさるときは必ずソースコードを添付してください。
	その際に事後でかまいませんので、ひとことお知らせいただけると幸いです。
	商的な利用は禁じます。
	あとは「GNU General Public License(一般公衆利用許諾契約書)」に準じます。

******************************************************************************/
#ifndef WRITEMEM_H_INCLUDED
#define WRITEMEM_H_INCLUDED

#include "TypeDefs.h"
#include <windows.h>


BOOL
WRITEMEM_Init(
	HWND		hWnd,
	HINSTANCE	hInstance,
	Uint32*		mpr,
	Uint32*		addr,
	Uint8*		data,
	BOOL*		bContinuous,
	Sint32*		setOk);

void
WRITEMEM_Deinit();

char*
WRITEMEM_GetCode();

void
WRITEMEM_ClearCode();


#endif /* WRITEMEM_H_INCLUDED */
