/******************************************************************************
Ootake

 [RecentRom.h]
	最近起動したゲームの一覧を表示するためのフォーム

Copyright(C)2006-2011 Kitao Nakamura.
	改造版・後継版を公開なさるときは必ずソースコードを添付してください。
	その際に事後でかまいませんので、ひとことお知らせいただけると幸いです。
	商的な利用は禁じます。
	あとは「GNU General Public License(一般公衆利用許諾契約書)」に準じます。

******************************************************************************/
#ifndef RECENT_H_INCLUDED
#define RECENT_H_INCLUDED

#include "TypeDefs.h"
#include <windows.h>


BOOL
RECENT_Init(
	HINSTANCE	hInstance,
	Sint32*		selectedrom);

void
RECENT_Deinit();


#endif /* RECENT_H_INCLUDED */
