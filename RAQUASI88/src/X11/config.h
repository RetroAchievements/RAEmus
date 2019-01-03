#ifndef	CONFIG_H_INCLUDED
#define	CONFIG_H_INCLUDED


/*----------------------------------------------------------------------*/
/* UNIX/X11 バージョン固有の定義					*/
/*----------------------------------------------------------------------*/

/* X11版 QUASI88 のための識別用 */

#ifndef	QUASI88_X11
#define	QUASI88_X11
#endif



/*
  エンディアンネスは、コンパイル時に (Makefile) 与える。
  リトルエンディアンなら、
	LSB_FIRST
  を定義しておく
*/



/* メニューのタイトル／バージョン表示にて追加で表示する言葉 (任意の文字列) */

#define	Q_COMMENT	"UNIX/X11 port"



/*
  サポートする bpp は、コンパイル時に (Makefile) 与える。
	SUPPORT_8BPP
	SUPPORT_16BPP
	SUPPORT_32BPP
  のうち、必要なものを定義しておく。
	SUPPORT_DOUBLE
  も好みで定義しておく。
*/



/*
  サウンドのサポートは、コンパイル時に (Makefile) 与える。

  MAME/XMAME のサウンドを組み込む場合、
	USE_SOUND
  を定義しておく。

  FMGEN を組み込む場合は、
	USE_FMGEN
  も定義しておく。

  詳細は、 Makefile をみてちょ。
*/

#endif	/* CONFIG_H_INCLUDED */
