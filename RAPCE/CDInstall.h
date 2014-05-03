/******************************************************************************
Ootake

 [CDInstall.h]
	CDのデータをインストールすることでアクセスの高速化を図る

Copyright(C)2006-2007 Kitao Nakamura.
	改造版・後継版を公開なさるときは必ずソースコードを添付してください。
	その際に事後でかまいませんので、ひとことお知らせいただけると幸いです。
	商的な利用は禁じます。
	あとは「GNU General Public License(一般公衆利用許諾契約書)」に準じます。

******************************************************************************/
#ifndef CDINSTALL_H_INCLUDED
#define CDINSTALL_H_INCLUDED

#include "TypeDefs.h"
#include <windows.h>


BOOL
CDINSTALL_Init(
	HINSTANCE		hInstance,
	BOOL			bFullInstall);

void
CDINSTALL_Deinit();

Sint32
CDINSTALL_GetResult();


#endif /* CDINSTALL_H_INCLUDED */
