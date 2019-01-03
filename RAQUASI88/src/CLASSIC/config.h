#ifndef	CONFIG_H_INCLUDED
#define	CONFIG_H_INCLUDED


/*----------------------------------------------------------------------*/
/* Classicに関する定義							*/
/*----------------------------------------------------------------------*/

#include <MacTypes.h>


/* Classic版 QUASI88 のための識別用 */

#ifndef	QUASI88_CLASSIC
#define	QUASI88_CLASSIC
#endif



/* エンディアンネス */

#undef	LSB_FIRST



/* メニューのタイトル／バージョン表示にて追加で表示する言葉 (任意の文字列) */

#define	Q_COMMENT	"Classic port"



/* Classic版は 8bpp(bit per pixel) 固定とする */
/* 16bpp も実験中… */
#ifndef	SUPPORT_8BPP
#define	SUPPORT_8BPP
#endif
#ifndef	SUPPORT_16BPP
#define	SUPPORT_16BPP
#endif
#undef	SUPPORT_32BPP



/*
  MAME/XMAME のサウンドを組み込む場合、
	USE_SOUND
  を定義しておく。

  FMGEN を組み込む場合は、
	USE_FMGEN
  も定義しておく。

  上記は、コンパイル時に以下のようにして定義する。
  gcc  の場合、コンパイラにオプション -DUSE_SOUND   などと指定する。
  VC++ の場合、コンパイラにオプション /D"USE_SOUND" などと指定する。
  MPW  の場合、コンパイラにオプション -d USE_SOUND  などと指定する。
*/




/*
 *	SC depend
 */

#ifdef	macintosh

/* サウンドドライバ用に、PI(π)とM_PI(π)を定義 …  SCでも必要? */
#ifndef PI
#define PI 3.14159265358979323846
#endif
#ifndef	M_PI
#define	M_PI	PI
#endif

#ifdef	__SC__

/* SC は long long が使えない? */
#define	DENY_LONG_LONG

#endif

#endif



#endif	/* CONFIG_H_INCLUDED */
