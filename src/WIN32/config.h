#ifndef	CONFIG_H_INCLUDED
#define	CONFIG_H_INCLUDED


/*----------------------------------------------------------------------*/
/* WIN32 固有の定義							*/
/*----------------------------------------------------------------------*/

#include <windows.h>


/* WIN32版 QUASI88 のための識別用 */

#ifndef	QUASI88_WIN32
#define	QUASI88_WIN32
#endif



/* エンディアンネス */

#define	LSB_FIRST



/* メニューのタイトル／バージョン表示にて追加で表示する言葉 (任意の文字列) */

#define	Q_COMMENT	"WIN32 port"



/* WIN32版は 32bpp(bit per pixel) 固定とする */

#undef	SUPPORT_8BPP
#undef	SUPPORT_16BPP
#define	SUPPORT_32BPP



/* VC のインラインキーワード */
#define	INLINE		__inline


/* サウンドドライバ用に、PI(π)とM_PI(π)を定義 */
#ifndef PI
#define PI 3.14159265358979323846
#endif
#ifndef	M_PI
#define	M_PI	PI
#endif

#endif	/* CONFIG_H_INCLUDED */
