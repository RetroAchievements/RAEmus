/***********************************************************************
 * グラフィック処理 (システム依存)
 *
 *	詳細は、 graph.h 参照
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <unistd.h>

#include "quasi88.h"
#include "graph.h"
#include "device.h"

#include "screen.h"


#ifdef MITSHM
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
#endif

/************************************************************************/

/* 以下は static な変数。オプションで変更できるのでグローバルにしてある */

	int	colormap_type	= 0;	 /* カラーマップ指定	 0〜2	*/
	int	use_xsync	= TRUE;	 /* XSync を使用するかどうか	*/
	int	use_xv		= FALSE; /* Xv を使用するかどうか	*/
#ifdef MITSHM
	int	use_SHM		= TRUE;	 /* MIT-SHM を使用するかどうか	*/
#endif


/* 以下は、 event.c などで使用する、 OSD なグローバル変数 */

	Display	*x11_display;
	Window	x11_window;
	Atom	x11_atom_kill_type;
	Atom	x11_atom_kill_data;

	int	x11_width;		/* 現在のウインドウの幅 */
	int	x11_height;		/* 現在のウインドウの高 */

	int	x11_mouse_rel_move;	/* マウス相対移動量検知させるか	*/

/************************************************************************/

static	T_GRAPH_SPEC	graph_spec;		/* 基本情報		*/

static	int		graph_exist;		/* 真で、画面生成済み	*/
static	T_GRAPH_INFO	graph_info;		/* その時の、画面情報	*/


static	int	x11_enable_fullscreen;		/* 真で、全画面可能	*/

static	Screen	*x11_screen;
static	GC	x11_gc;
static	Visual	*x11_visual;

static	int	x11_depth;
static	int	x11_byte_per_pixel;


/* 現在の属性 */
static	int	x11_mouse_show   = TRUE;
static	int	x11_grab         = FALSE;
static	int	x11_keyrepeat_on = TRUE;

/************************************************************************/

/* ウインドウの生成時・リサイズ時は、ウインドウサイズ変更不可を指示する */
static	void	set_wm_hints(int w, int h, int fullscreen);

#ifdef MITSHM
static	XShmSegmentInfo	SHMInfo;

/* MIT-SHM の失敗をトラップ */
static	int	private_handler(Display *display, XErrorEvent *E);
#endif

/************************************************************************
 *	X11 Windod & DGA / XV
 ************************************************************************/


#include "graph-x11dga.c"


#ifdef	USE_XV
#include "graph-xv.c"
#endif


/************************************************************************
 *	X11の初期化
 *	X11の終了
 ************************************************************************/

void	x11_init(void)
{
    x11_enable_fullscreen = FALSE;

    x11_display = XOpenDisplay(NULL);
    if (! x11_display) return;


    /* この時点では use_xv は未設定のため、 DGA・XV ともに初期化しておく */

#ifdef	USE_DGA
    dga_init();		/* DGA 初期化 */
#endif
#ifdef	USE_XV
    xv_init();		/* XV 初期化 */
#endif

    /* 初期化の結果、全画面が可能なら、 x11_enable_fullscreen は真になる */
}

static	void	init_verbose(void)
{
    if (verbose_proc) {

	if (! x11_display) { printf("FAILED\n"); return; }
	else               { printf("OK");               }

#if	defined(USE_DGA) || defined(USE_XV)
 #ifdef	USE_DGA
	if (use_xv == FALSE) {
	    dga_verbose();
	}
 #endif
 #ifdef	USE_XV
	if (use_xv) {
	    xv_verbose();
	}
 #endif
#else
	printf(" (fullscreen not supported)\n");
#endif
  }
}

/************************************************************************/

void	x11_exit(void)
{
    if (x11_display) {
	XAutoRepeatOn(x11_display);	/* オートリピート設定をもとに戻す */

#ifdef	USE_DGA
	dga_exit();
#endif
#ifdef	USE_XV
	xv_exit();
#endif

	XSync(x11_display, True);

	/* DGA有効時、XCloseDisplayでエラーがでる。なぜに? */
	if (use_xv ||
	    x11_enable_fullscreen == FALSE) {	/* とりあえずDGAでない時だけ */
	    XCloseDisplay(x11_display);
	}

	x11_display = NULL;
    }
}


/************************************************************************
 *	グラフィック処理の初期化
 *	グラフィック処理の動作
 *	グラフィック処理の終了
 ************************************************************************/

/* マウス非表示を実現するため、透明マウスカーソルを用意しよう。
   グラフィック初期化時にカーソルを生成、終了時に破棄する。*/
static	void	create_invisible_mouse(void);
static	void	destroy_invisible_mouse(void);


const T_GRAPH_SPEC	*graph_init(void)
{
    const T_GRAPH_SPEC *spec = NULL;

#ifndef	USE_XV
    use_xv = FALSE;
    x11_scaling = FALSE;
#endif

    if (verbose_proc) {
	printf("Initializing Graphic System (X11) ... ");
	init_verbose();
    }

    if (! x11_display) {
	return NULL;
    }


    x11_screen = DefaultScreenOfDisplay(x11_display);
    x11_gc     = DefaultGCOfScreen(x11_screen);
    x11_visual = DefaultVisualOfScreen(x11_screen);


    if (use_xv == FALSE) {
	spec = x11_graph_init();
    }
#ifdef	USE_XV
    if (use_xv) {
	spec = xv_graph_init();
    }
#endif


    if (spec) {

	/* マウス非表示のための、透明マウスカーソルを生成 */
	create_invisible_mouse();

	/* Drag & Drop 初期化 */
	xdnd_initialize();
    }

    return spec;
}

/************************************************************************/

const T_GRAPH_INFO	*graph_setup(int width, int height,
				     int fullscreen, double aspect)
{
    if (use_xv == FALSE) {
	return x11_graph_setup(width, height, fullscreen, aspect);
    }
#ifdef	USE_XV
    if (use_xv) {
	return xv_graph_setup(width, height, fullscreen, aspect);
    }
#endif

    return NULL;
}

/************************************************************************/

void	graph_exit(void)
{
    if (use_xv == FALSE) {
	x11_graph_exit();
    }
#ifdef	USE_XV
    if (use_xv) {
	xv_graph_exit();
    }
#endif

    /* 透明マウスカーソルを破棄 */
    destroy_invisible_mouse();
}

/*======================================================================*/

static	Cursor x11_cursor_id;
static	Pixmap x11_cursor_pix;

static	void	create_invisible_mouse(void)
{
    char data[1] = { 0x00 };
    XColor color;

    x11_cursor_pix = XCreateBitmapFromData(x11_display,
					   DefaultRootWindow(x11_display),
					   data, 8, 1);
    color.pixel    = BlackPixelOfScreen(x11_screen);
    x11_cursor_id  = XCreatePixmapCursor(x11_display,
					 x11_cursor_pix, x11_cursor_pix,
					 &color, &color, 0, 0);
}

static	void	destroy_invisible_mouse(void)
{
    if (x11_mouse_show == FALSE) {
	XUndefineCursor(x11_display, DefaultRootWindow(x11_display));
    }
    XFreePixmap(x11_display, x11_cursor_pix);
}

/*======================================================================*/

#ifdef MITSHM
/* MIT-SHM の失敗をトラップ */
static	int	private_handler(Display *display, XErrorEvent *E)
{
    char str[256];

    if (E->error_code == BadAccess ||
	E->error_code == BadAlloc) {
	use_SHM = FALSE;
	return 0;
    }

    XGetErrorText(display, E->error_code, str, 256);
    fprintf(stderr, "X Error (%s)\n", str);
    fprintf(stderr, " Error Code   %d\n", E->error_code);
    fprintf(stderr, " Request Code %d\n", E->request_code);
    fprintf(stderr, " Minor code   %d\n", E->minor_code);

    exit(-1);

    return 1;
}
#endif

/*======================================================================*/

/* ウインドウマネージャにサイズ変更不可を指示する */
static	void	set_wm_hints(int w, int h, int fullscreen)
{
    XSizeHints Hints;
    XWMHints WMHints;

    if (fullscreen) {
#if 1
	/* 何のおまじないか知らんが、これをするとウインドウのタイトルや枠が
	   無くなるので、全画面サイズのウインドウが画面にフィットする。
	   xmame のソースからコピペ */
	#define MWM_HINTS_DECORATIONS   2
	typedef struct {
	    long flags;
	    long functions;
	    long decorations;
	    long input_mode;
	} MotifWmHints;

	Atom mwmatom;
	MotifWmHints mwmhints;
	mwmhints.flags = MWM_HINTS_DECORATIONS;
	mwmhints.decorations = 0;
	mwmatom = XInternAtom(x11_display,"_MOTIF_WM_HINTS",0);

	XChangeProperty(x11_display, x11_window,
			mwmatom, mwmatom, 32,
			PropModeReplace, (unsigned char *)&mwmhints, 4);
#endif

	Hints.x      = 0;
	Hints.y      = 0;
	Hints.flags  = PMinSize|PMaxSize|USPosition|USSize;
	Hints.win_gravity = NorthWestGravity;
    } else {
	Hints.flags      = PSize|PMinSize|PMaxSize;
    }
    Hints.min_width  = Hints.max_width  = Hints.base_width  = w;
    Hints.min_height = Hints.max_height = Hints.base_height = h;
    WMHints.input = True;
    WMHints.flags = InputHint;

    XSetWMHints(x11_display, x11_window, &WMHints);
    XSetWMNormalHints(x11_display, x11_window, &Hints);
}


/************************************************************************
 *	色の確保
 *	色の解放
 ************************************************************************/

void	graph_add_color(const PC88_PALETTE_T color[],
			int nr_color, unsigned long pixel[])
{
    if (use_xv == FALSE) {
	x11_graph_add_color(color, nr_color, pixel);
    }
#ifdef	USE_XV
    if (use_xv) {
	xv_graph_add_color(color, nr_color, pixel);
    }
#endif
}

/************************************************************************/

void	graph_remove_color(int nr_pixel, unsigned long pixel[])
{
    if (use_xv == FALSE) {
	x11_graph_remove_color(nr_pixel, pixel);
    }
#ifdef	USE_XV
    if (use_xv) {
	xv_graph_remove_color(nr_pixel, pixel);
    }
#endif
}


/************************************************************************
 *	グラフィックの更新
 ************************************************************************/

void	graph_update(int nr_rect, T_GRAPH_RECT rect[])
{
    if (use_xv == FALSE) {
	x11_graph_update(nr_rect, rect);
    }
#ifdef	USE_XV
    if (use_xv) {
	xv_graph_update(nr_rect, rect);
    }
#endif
}


/************************************************************************
 *	タイトルの設定
 *	属性の設定
 ************************************************************************/

void	graph_set_window_title(const char *title)
{
    static char saved_title[128];

    if (title) {
	saved_title[0] = '\0';
	strncat(saved_title, title, sizeof(saved_title)-1);
    }

    if (graph_exist) {
	XStoreName(x11_display, x11_window, saved_title);
    }
}

/************************************************************************/

void	graph_set_attribute(int mouse_show, int grab, int keyrepeat_on)
{
    x11_mouse_show   = mouse_show;
    x11_grab         = grab;
    x11_keyrepeat_on = keyrepeat_on;

    if (x11_get_focus) {
	x11_set_attribute_focus_in();
    }
}


/***********************************************************************
 *
 *	X11 独自関数
 *
 ************************************************************************/

/* フォーカスを失った時、マウス表示、アングラブ、キーリピートありにする */
void	x11_set_attribute_focus_out(void)
{
    XUndefineCursor(x11_display, x11_window);
    XUngrabPointer(x11_display, CurrentTime);
    XAutoRepeatOn(x11_display);
}

/* フォーカスを得た時、、マウス・グラブ・キーリピートを、設定通りに戻す */
void	x11_set_attribute_focus_in(void)
{
    int dga = (use_xv == FALSE && graph_info.fullscreen) ? TRUE : FALSE;

    if (x11_mouse_show) XUndefineCursor(x11_display, x11_window);
    else                XDefineCursor(x11_display, x11_window, x11_cursor_id);

    if (x11_grab || dga)		/* グラブ指示あり、または */
					/* DGA 使用時はグラブする */
			  XGrabPointer(x11_display, x11_window, True,
				       PointerMotionMask | ButtonPressMask |
				       ButtonReleaseMask,
				       GrabModeAsync, GrabModeAsync,
				       x11_window, None, CurrentTime);
    else		  XUngrabPointer(x11_display, CurrentTime);

    if (x11_keyrepeat_on) XAutoRepeatOn(x11_display);
    else                  XAutoRepeatOff(x11_display);


    /* マウス移動によるイベント発生時の処理方法を設定 */

    if      (dga)                     x11_mouse_rel_move = 1;
    else if (x11_grab &&
	     x11_mouse_show == FALSE) x11_mouse_rel_move = -1;
    else                              x11_mouse_rel_move = 0;

    /* マウス移動時のイベントについて (event.c)

       DGA の場合、マウス移動量が通知される   → 1: 相対移動
       X11 の場合、マウス絶対位置が通知される → 0: 絶対移動

       ウインドウグラブ時は、マウスがウインドウの端にたどり着くと
       それ以上動けなくなる。
       そこで、マウスを常にウインドウの中央にジャンプさせることで、
       無限にマウスを動かすことができるかのようにする。
       この時のマウスの移動処理だが、計算により相対量として扱う。
       また、マウスを非表示にしておかないと、無様な様子が見えるので
       この機能はマウスなし時のみとする       → -1: むりやり相対移動
    */
}
