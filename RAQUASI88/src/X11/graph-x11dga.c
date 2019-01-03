/***********************************************************************
 * グラフィック処理 (X11 Window & DGA 1.0)
 *
 *
 ************************************************************************/

#ifdef	USE_DGA
#include <unistd.h>
#include <sys/types.h>
#if 1	/* obsolete? */
#include <X11/extensions/xf86dga.h>
#else
#include <X11/extensions/Xxf86dga.h>
#endif
#include <X11/extensions/xf86vmode.h>
#endif

/************************************************************************/

#ifdef	USE_DGA
static	char			*dga_addr;
static	int			dga_width;
static	int			dga_bank;
static	int			dga_ram;
static	XF86VidModeModeInfo	**dga_mode = NULL;
static	int			dga_mode_count;

static	int			dga_mode_selected = 0;
#endif

/************************************************************************
 *	DGAの初期化
 *	DGAの終了
 ************************************************************************/

#ifdef	USE_DGA
enum {
    DGA_ERR_NONE = 0,
    DGA_ERR_AVAILABLE,
    DGA_ERR_ROOT_RIGHTS,
    DGA_ERR_LOCAL_DISPLAY,
    DGA_ERR_QUERY_VERSION,
    DGA_ERR_QUERY_EXTENSION,
    DGA_ERR_QUERY_DIRECT_VIDEO,
    DGA_ERR_QUERY_DIRECT_PRESENT,
    DGA_ERR_GET_VIDEO,
    DGA_ERR_BAD_VALUE,
    DGA_ERR_MANY_BANKS,
    DGA_ERR_XVM_QUERY_VERSION,
    DGA_ERR_XVM_QUERY_EXTENSION,
    DGA_ERR_XVM_GET_ALL_MODE_LINES
};
static	int	DGA_error = DGA_ERR_AVAILABLE;

static	void	dga_init(void)
{
    int i, j;
    char *s;

    if (geteuid()) {
	DGA_error = DGA_ERR_ROOT_RIGHTS;
    }
    else
    if (! (s = getenv("DISPLAY")) || (s[0] != ':')) {
	DGA_error = DGA_ERR_LOCAL_DISPLAY;
    }
    else
    if (! XF86DGAQueryVersion(x11_display, &i, &j)) {
	DGA_error = DGA_ERR_QUERY_VERSION;
    }
    else
    if (! XF86DGAQueryExtension(x11_display, &i, &j)) {
	DGA_error = DGA_ERR_QUERY_EXTENSION;
    }
    else
    if (! XF86DGAQueryDirectVideo(x11_display,
				  DefaultScreen(x11_display), &i)) {
	DGA_error = DGA_ERR_QUERY_DIRECT_VIDEO;
    }
    else
    if (! (i & XF86DGADirectPresent)) {
	DGA_error = DGA_ERR_QUERY_DIRECT_PRESENT;
    }
    else
    if (! XF86DGAGetVideo(x11_display, DefaultScreen(x11_display),
			  &dga_addr, &dga_width, &dga_bank, &dga_ram)) {
	DGA_error = DGA_ERR_GET_VIDEO;
    }
    else
    if (dga_addr==NULL || dga_width==0 || dga_bank==0 || dga_ram==0) {
	DGA_error = DGA_ERR_BAD_VALUE;
    }
    else
    if (dga_ram * 1024 != dga_bank) {
	DGA_error = DGA_ERR_MANY_BANKS;
    }
    else
    if (! XF86VidModeQueryVersion(x11_display, &i, &j)) {
	DGA_error = DGA_ERR_XVM_QUERY_VERSION;
    }
    else
    if (! XF86VidModeQueryExtension(x11_display, &i, &j)) {
	DGA_error = DGA_ERR_XVM_QUERY_EXTENSION;
    }
    else
    if (! XF86VidModeGetAllModeLines(x11_display,
				     DefaultScreen(x11_display),
				     &dga_mode_count, &dga_mode)) {
	DGA_error = DGA_ERR_XVM_GET_ALL_MODE_LINES;
    }
    else
    {
	DGA_error = DGA_ERR_NONE;
	x11_enable_fullscreen = 1;
    }
}


static	void	dga_verbose(void)
{
    printf("\n");
    printf("  DGA : ");

    if      (DGA_error == DGA_ERR_NONE)
	printf("OK");
    else if (DGA_error == DGA_ERR_ROOT_RIGHTS)
	printf("FAILED (Must be suid root)");
    else if (DGA_error == DGA_ERR_LOCAL_DISPLAY)
	printf("FAILED (Only works on a local display)");
    else if (DGA_error == DGA_ERR_QUERY_VERSION)
	printf("FAILED (XF86DGAQueryVersion)");
    else if (DGA_error == DGA_ERR_QUERY_EXTENSION)
	printf("FAILED (XF86DGAQueryExtension)");
    else if (DGA_error == DGA_ERR_QUERY_DIRECT_VIDEO)
	printf("FAILED (XF86DGAQueryDirectVideo)");
    else if (DGA_error == DGA_ERR_QUERY_DIRECT_PRESENT)
	printf("FAILED (Xserver not support DirectVideo)");
    else if (DGA_error == DGA_ERR_GET_VIDEO)
	printf("FAILED (XF86DGAGetVideo)");
    else if (DGA_error == DGA_ERR_BAD_VALUE)
	printf("FAILED (XF86DGAGetVideo Bad Value)");
    else if (DGA_error == DGA_ERR_MANY_BANKS)
	printf("FAILED (banked graphics modes not supported)");
    else if (DGA_error == DGA_ERR_XVM_QUERY_VERSION)
	printf("FAILED (XF86VidModeQueryVersion)");
    else if (DGA_error == DGA_ERR_XVM_QUERY_EXTENSION)
	printf("FAILED (XF86VidModeQueryExtension)");
    else if (DGA_error == DGA_ERR_XVM_GET_ALL_MODE_LINES)
	printf("FAILED (XF86VidModeGetAllModeLines)");
    else
	printf("FAILED (Not Support)");

    if (DGA_error == DGA_ERR_NONE) printf(", fullscreen available\n");
    else                           printf(", fullscreen not available\n");
}


static	void	dga_exit(void)
{
    if (dga_mode) XFree(dga_mode);
}
#endif


/************************************************************************
 *	グラフィック処理の初期化
 *	グラフィック処理の動作
 *	グラフィック処理の終了
 ************************************************************************/

static	const T_GRAPH_SPEC	*x11_graph_init(void)
{
    int	win_w, win_h;
    int	ful_w, ful_h;

    int i, count;
    XPixmapFormatValues *pixmap;


    /* 色深度と、ピクセルあたりのバイト数をチェック */

    pixmap = XListPixmapFormats(x11_display, &count);
    if (pixmap == NULL) {
	if (verbose_proc) printf("  X11 error (Out of memory ?)\n");
	return NULL;
    }
    for (i = 0; i < count; i++) {
	if (pixmap[i].depth == DefaultDepthOfScreen(x11_screen)) {
	    x11_depth = pixmap[i].depth;
	    if      (x11_depth <=  8 && pixmap[i].bits_per_pixel ==  8) {
		x11_byte_per_pixel = 1;
	    }
	    else if (x11_depth <= 16 && pixmap[i].bits_per_pixel == 16) {
		x11_byte_per_pixel = 2;
	    }
	    else if (x11_depth <= 32 && pixmap[i].bits_per_pixel == 32) {
		x11_byte_per_pixel = 4;
	    }
	    else {		/* 上記以外のフォーマットは面倒なので NG */
		x11_byte_per_pixel = 0;
	    }
	    break;
	}
    }
    XFree(pixmap);


    {				/* 非対応の depth なら弾く */
	const char *s = NULL;
	switch (x11_byte_per_pixel) {
	case 0:	s = "this bpp is not supported";	break;
#ifndef	SUPPORT_8BPP
	case 1:	s = "8bpp is not supported";		break;
#endif
#ifndef	SUPPORT_16BPP
	case 2:	s = "16bpp is not supported";		break;
#endif
#ifndef	SUPPORT_32BPP
	case 4:	s = "32bpp is not supported";		break;
#endif
	}
	if (s) {
	    if (verbose_proc) printf("  %s\n",s);    
	    return NULL;
	}
    }

    if (x11_depth < 4) {
	if (verbose_proc) printf("  < 4bpp is not supported\n");
	return NULL;
    }


    /* 利用可能なウインドウのサイズを調べておく */

    win_w = 10000;	/* ウインドウは制約なし。適当に大きな値をセット */
    win_h = 10000;

#ifdef	USE_DGA		/* 全画面モードは、一覧から幅の最も大きなのをセット */
    if (x11_enable_fullscreen) {
	int i;
	ful_w = 0;
	ful_h = 0;
	for (i = 0; i < dga_mode_count; i++) {
	    if (ful_w < dga_mode[i]->hdisplay) {
		ful_w = dga_mode[i]->hdisplay;
		ful_h = dga_mode[i]->vdisplay;
	    }
	    if (verbose_proc)
		printf("  VidMode %3d: %dx%d %d\n",
		       i, dga_mode[i]->hdisplay, dga_mode[i]->vdisplay,
		       dga_mode[i]->privsize);
	}
    }
    else
#endif
    {
	ful_w = 0;
	ful_h = 0;
    }

    graph_spec.window_max_width      = win_w;
    graph_spec.window_max_height     = win_h;
    graph_spec.fullscreen_max_width  = ful_w;
    graph_spec.fullscreen_max_height = ful_h;
    graph_spec.forbid_status         = FALSE;
    graph_spec.forbid_half           = FALSE;

    if (verbose_proc)
	printf("  INFO: %dbpp(%dbyte), Maxsize=win(%d,%d),full(%d,%d)\n",
	       x11_depth, x11_byte_per_pixel,
	       win_w, win_h, ful_w, ful_h);


    return &graph_spec;
}

/************************************************************************/

/* ウインドウの生成時／リサイズ時／破棄時 および、
   全画面モード開始時／解像度切替時／終了時 の、 実際の処理をする関数 */

static	int	create_window(int width, int height,
			      void **ret_buffer, int *ret_nr_color);
static	int	resize_window(int width, int height, void *old_buffer,
			      void **ret_buffer, int *ret_nr_color);
static	void	destroy_window(void *old_buffer);

#ifdef	USE_DGA
static	int	create_DGA(int *width, int *height, double aspect,
			   int *ret_nr_color);
static	int	resize_DGA(int *width, int *height, double aspect,
			   int *ret_nr_color);
static	void	destroy_DGA(void);
#else
#define		create_DGA(w, h, aspect, ret_nr_color)	(FALSE)
#define		resize_DGA(w, h, screen, ret_nr_color)	(FALSE)
#define		destroy_DGA()
#endif


static	const T_GRAPH_INFO	*x11_graph_setup(int width, int height,
						 int fullscreen, double aspect)
{

    /*
        ＼要求は|           Window            |           全画面            |
          ＼    |-----------------------------|-----------------------------|
      現在は＼  |  同じサイズ  |  違うサイズ  |  同じサイズ  |  違うサイズ  |
    ------------+-----------------------------+-----------------------------+
      未初期化  |          Window生成         |          全画面生成         |
    ------------+--------------+--------------+-----------------------------+
     ウインドウ |              |Windowリサイズ|  Window破棄 ⇒ 全画面生成   |
    ------------+--------------+--------------+--------------+--------------+
       全画面   |  全画面破棄 ⇒ Window生成   |              |全画面リサイズ|
    ------------+-----------------------------+--------------+--------------+
    */

    int nr_color = 0;
    void *buf = NULL;
    int success;

    /* 全画面不可なら、全画面要求は無視 */
    if ((x11_enable_fullscreen == FALSE) && (fullscreen)) {
	fullscreen = FALSE;
    }


    /* ウインドウ⇔全画面切替の場合、予め現在の状態を破棄しておく */
    if (graph_exist) {
	if (verbose_proc) printf("Re-Initializing Graphic System (X11)\n");

	if ((graph_info.fullscreen == FALSE) && (fullscreen)) {
	    /* ウインドウ → 全画面 切り替え */
	    destroy_window(graph_info.buffer);
	    graph_exist = FALSE;
	}
	else if ((graph_info.fullscreen) && (fullscreen == FALSE)) {
	    /* 全画面 → ウインドウ 切り替え */
	    destroy_DGA();
	    graph_exist = FALSE;
	}
    }


    /* □ → 全画面 の場合 */
    if (fullscreen) {
	if (graph_exist == 0) {
	    success = create_DGA(&width, &height, aspect, &nr_color);
	} else {
	    success = resize_DGA(&width, &height, aspect, &nr_color);
	}

	if (success) {		/* 成功 */
	    goto SUCCESS;
	} else {		/* 失敗したらウインドウでやり直す */
	    fullscreen = FALSE;
	    graph_exist = 0;
	}
    }


    /* □ → ウインドウ の場合 */
    {
	if (graph_exist == 0) {
	    success = create_window(width, height, &buf, &nr_color);
	} else {
	    success = resize_window(width, height,
				    graph_info.buffer, &buf, &nr_color);
	}

	if (success) {		/* 成功 */
	    goto SUCCESS;
	}
    }

    /* ことごとく失敗 */
    graph_exist = FALSE;
    return NULL;


 SUCCESS:
    graph_exist = TRUE;

#ifdef	USE_DGA
    if (fullscreen) {
	graph_info.fullscreen		= TRUE;
	graph_info.width		= width;
	graph_info.height		= height;
	graph_info.byte_per_pixel	= x11_byte_per_pixel;
	graph_info.byte_per_line	= dga_width * x11_byte_per_pixel;
	graph_info.buffer		= dga_addr;
	graph_info.nr_color		= nr_color;
	graph_info.write_only		= TRUE;
      /*graph_info.write_only		= FALSE;*/	/*TEST*/
	graph_info.broken_mouse		= TRUE;
	graph_info.draw_start		= NULL;
	graph_info.draw_finish		= NULL;
	graph_info.dont_frameskip	= FALSE;
    } else
#endif
    {
	graph_info.fullscreen		= FALSE;
	graph_info.width		= width;
	graph_info.height		= height;
	graph_info.byte_per_pixel	= x11_byte_per_pixel;
	graph_info.byte_per_line	= width * x11_byte_per_pixel;
	graph_info.buffer		= buf;
	graph_info.nr_color		= nr_color;
	graph_info.write_only		= FALSE;
	graph_info.broken_mouse		= FALSE;
	graph_info.draw_start		= NULL;
	graph_info.draw_finish		= NULL;
	graph_info.dont_frameskip	= FALSE;

	/* ウインドウのタイトルバーを復元 */
	graph_set_window_title(NULL);

	XMapRaised(x11_display, x11_window);
    }

    x11_width  = width;
    x11_height = height;

#if 0	/* debug */
printf("@ fullscreen      %d\n",    graph_info.fullscreen    );
printf("@ width           %d\n",    graph_info.width         );
printf("@ height          %d\n",    graph_info.height        );
printf("@ byte_per_pixel  %d\n",    graph_info.byte_per_pixel);
printf("@ byte_per_line   %d\n",    graph_info.byte_per_line );
printf("@ buffer          %p\n",    graph_info.buffer        );
printf("@ nr_color        %d\n",    graph_info.nr_color      );
printf("@ write_only      %d\n",    graph_info.write_only    );
printf("@ broken_mouse    %d\n",    graph_info.broken_mouse  );
printf("@ dont_frameskip  %d\n",    graph_info.dont_frameskip);
#endif

    return &graph_info;
}

/************************************************************************/

static	void	x11_graph_exit(void)
{
    if (graph_exist) {

	if (graph_info.fullscreen) {
	    destroy_DGA();
	} else {
	    destroy_window(graph_info.buffer);
	}

	graph_exist = FALSE;
    }
}


/*======================================================================*/

/* ウインドウの生成時／リサイズ時／破棄時 あるいは、
   全画面モード開始時／解像度切替時／終了時 に、
   それぞれ カラーマップを確保／再初期化／解放 という処理が必要 */
static	int	create_colormap(int fullscreen);
static	int	reuse_colormap(void);
static	void	destroy_colormap(void);

/* ウインドウの生成時・リサイズ時・破棄時には、イメージの確保・解放が必要 */
static	void	*create_image(int width, int height);
static	void	destroy_image(void *buf);


static	int	create_window(int width, int height,
			      void **ret_buffer, int *ret_nr_color)
{
    if (verbose_proc) printf("  Opening window ... ");
    x11_window = XCreateSimpleWindow(x11_display,
				     RootWindowOfScreen(x11_screen),
				     0, 0,
				     width, height,
				     0,
				     WhitePixelOfScreen(x11_screen),
				     BlackPixelOfScreen(x11_screen));
    if (verbose_proc)
	printf("%s (%dx%d)\n", (x11_window ? "OK" : "FAILED"), width, height);

    if (! x11_window) {
	return FALSE; 
    }

    /* ウインドウマネージャーへ特性(サイズ変更不可)を指示する */
    set_wm_hints(width, height, FALSE);

    /* カラーを確保する */
    (*ret_nr_color) = create_colormap(FALSE);

    /* イベントの設定 */
    XSelectInput(x11_display, x11_window,
		 FocusChangeMask | ExposureMask |
		 KeyPressMask | KeyReleaseMask |
		 ButtonPressMask | ButtonReleaseMask | PointerMotionMask);

    /* リサイズ時 (ステータス ON/OFF切替時) に、画面か消えないようにする(?) */
    {
	XSetWindowAttributes attributes;
	attributes.bit_gravity = NorthWestGravity;
	XChangeWindowAttributes(x11_display, x11_window,
				CWBitGravity, &attributes);
    }

    /* 強制終了、中断に備えて、アトムを設定 */
    x11_atom_kill_type = XInternAtom(x11_display, "WM_PROTOCOLS", False);
    x11_atom_kill_data = XInternAtom(x11_display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(x11_display, x11_window, &x11_atom_kill_data, 1);

    /* スクリーンバッファ と image を確保 */
    (*ret_buffer) = create_image(width, height);

    /* Drag & Drop 受け付け開始 */
    xdnd_start();

    return (((*ret_nr_color) >= 16) && (*ret_buffer)) ? TRUE : FALSE;
}


static	int	resize_window(int width, int height, void *old_buffer,
			      void **ret_buffer, int *ret_nr_color)
{
    Window child;
    int x, y;

    if (verbose_proc) printf("  Resizing window ... ");

    /* ウインドウマネージャーへ新たなサイズを指示する */
    set_wm_hints(width, height, FALSE);

    /* リサイズしてウインドウが画面外に出てしまったらイヤなので、その場合は
       ウインドウを画面内に移動させようと思う。が環境によっては XGetGeometry()
       を使ってもちゃんと座標が取得できないし、 XMoveWindow() を使っても、
       ウインドウの枠とかを考慮せずに移動する場合がある。ウインドウマネージャー
       が関わっているからだと思うのだが、どうするのが正しいんでしょう ? */
#if 1
    /* とりあえずルートウインドウからの相対位置を求めて、原点が上か左の画面外
       だったら移動させる。仮想ウインドウマネージャーでも大丈夫だろう */

    XTranslateCoordinates(x11_display, x11_window,
			  DefaultRootWindow(x11_display), 0, 0,
			  &x, &y, &child);
    if (x < 0 || y < 0) {
	if (x < 0) x = 0;
	if (y < 0) y = 0;
	XMoveResizeWindow(x11_display, x11_window, x, y, width, height);
    } else
#endif
    {
	XResizeWindow(x11_display, x11_window, width, height);
    }

    if (verbose_proc)
	printf("%s (%dx%d)\n", (x11_window ? "OK" : "FAILED"), width, height);

    /* image を破棄する */
    if (old_buffer) {
	destroy_image(old_buffer);
    }

    /* カラーマップ状態の再設定 */
    (*ret_nr_color) = reuse_colormap();

    /* スクリーンバッファ と image を確保 */
    (*ret_buffer) = create_image(width, height);

    return (((*ret_nr_color) >= 16) && (*ret_buffer)) ? TRUE : FALSE;
}


static	void	destroy_window(void *old_buffer)
{
    if (verbose_proc) printf("  Closing Window\n");

    /* カラーマップ破棄 */
    destroy_colormap();

    /* イメージ破棄 */
    if (old_buffer) {
	destroy_image(old_buffer);
    }

    /* ウインドウ破棄 */
    XDestroyWindow(x11_display, x11_window);

    if (x11_grab) {
	XUngrabPointer(x11_display, CurrentTime);
	x11_grab = FALSE;
    }

    XSync(x11_display, True);		/* 全イベント破棄 */
}



#ifdef	USE_DGA

#define	FABS(a)		(((a) >= 0.0) ? (a) : -(a))

static	int	search_mode(int w, int h, double aspect)
{
    int i;
    int fit = -1;
    int fit_w = 0, fit_h = 0;
    float fit_a = 0.0;

    for (i = 0; i < dga_mode_count; i++) {
	/* 画面サイズに収まっていること */
	if (w <= dga_mode[i]->hdisplay &&
	    h <= dga_mode[i]->vdisplay) {

	    int tmp_w = dga_mode[i]->hdisplay;
	    int tmp_h = dga_mode[i]->vdisplay;
	    double tmp_a = FABS(((float)tmp_w / tmp_h) - aspect);

	    /* 最初に見つかったものをまずはチョイス */
	    if (fit == -1) {
		fit = i;
		fit_w = tmp_w;
		fit_h = tmp_h;
		fit_a = tmp_a;

	    } else {
	    /* 次からは、前回のと比べて、よりフィットすればチョイス */

		/* 横長モニター、ないし、アスペクト未指定の場合 */
		if (aspect >= 1.0 || aspect < 0.01) {

		    /* 縦の差の少ないほう、またはアスペクト比の近いほう */
		    if (((tmp_h - h) < (fit_h - h)) ||
			((tmp_h == fit_h) && (tmp_a < fit_a))) {
			fit = i;
			fit_w = tmp_w;
			fit_h = tmp_h;
			fit_a = tmp_a;
		    }

		} else {	/* 縦長モニターの場合 (使ったことないけど) */

		    /* 横の差の少ないほう、またはアスペクト比の近いほう */
		    if (((tmp_w - w) < (fit_w - w)) ||
			((tmp_w == fit_w) && (tmp_a < fit_a))) {
			fit = i;
			fit_w = tmp_w;
			fit_h = tmp_h;
			fit_a = tmp_a;
		    }
		}
	    }
	}
    }
    /* 該当するのが全くない場合は、 -1 が返る */
    return fit;
}


static	int	create_DGA(int *width, int *height, double aspect,
			   int *ret_nr_color)
{
    int fit = search_mode(*width, *height, aspect);

    if (fit < 0 || dga_mode_count <= fit) {
	return FALSE;
    }

    if (verbose_proc) printf("  Starting DGA <%dx%d> ... ", *width, *height);


    /* 実際の画面サイズをセットする */
    *width  = dga_mode[ fit ]->hdisplay;
    *height = dga_mode[ fit ]->vdisplay;

    /* モードラインを切り替え */
    XF86VidModeSwitchToMode(x11_display, DefaultScreen(x11_display),
			    dga_mode[fit]);

    /* DGAを有効にする */
    XF86DGADirectVideo(x11_display, DefaultScreen(x11_display), 
		       XF86DGADirectGraphics |
		       XF86DGADirectMouse |
		       XF86DGADirectKeyb);

    XF86DGASetViewPort(x11_display, DefaultScreen(x11_display), 0, 0);

    if (verbose_proc) printf("OK (%dx%d)\n", *width, *height);

    /* キーボード・マウスをグラブする */
    x11_window = DefaultRootWindow(x11_display);

    XGrabKeyboard(x11_display, x11_window, True, GrabModeAsync,
		  GrabModeAsync,  CurrentTime);

    XGrabPointer(x11_display, x11_window, True,
		 PointerMotionMask | ButtonPressMask | ButtonReleaseMask,
		 GrabModeAsync, GrabModeAsync, x11_window, None, CurrentTime);

    /* カラーを確保する */
    (*ret_nr_color) = create_colormap(TRUE);

    /* イベントを設定すると怒られるので、設定しない */

    x11_get_focus = TRUE;
    x11_set_attribute_focus_in();

    return TRUE;
}


static	int	resize_DGA(int *width, int *height, double aspect,
			   int *ret_nr_color)
{
    int fit = search_mode(*width, *height, aspect);

    if (fit < 0 || dga_mode_count <= fit) {
	return FALSE;
    }

    if (verbose_proc) printf("  Switching DGA <%dx%d> ... ", *width, *height);


    /* 実際の画面サイズをセットする */
    *width  = dga_mode[ fit ]->hdisplay;
    *height = dga_mode[ fit ]->vdisplay;

    /* DGA有効のままだと失敗する? */
    XF86DGADirectVideo(x11_display, DefaultScreen(x11_display), 0);

    XF86VidModeSwitchToMode(x11_display, DefaultScreen(x11_display),
			    dga_mode[fit]);

    /* モード切替の際にDGAを無効にしてしまったので、再度有効にする */
    XF86DGADirectVideo(x11_display, DefaultScreen(x11_display),
		       XF86DGADirectGraphics |
		       XF86DGADirectMouse |
		       XF86DGADirectKeyb);

    if (verbose_proc) printf("OK (%dx%d)\n", *width, *height);

    /* カラーマップ状態の再設定 */
    (*ret_nr_color) = reuse_colormap();

    return TRUE;
}


static	void	destroy_DGA(void)
{
    if (verbose_proc) printf("  Stopping DGA\n");

    /* カラーマップ破棄 */
    destroy_colormap();

    /* DGA 停止 */
    XF86DGADirectVideo(x11_display, DefaultScreen(x11_display), 0);

    /* 解像度を元に戻す */
    XF86VidModeSwitchToMode(x11_display, DefaultScreen(x11_display),
			    dga_mode[0]);
    /* XF86VidModeSwitchMode(x11_display, DefaultScreen(x11_display), -1);
       XF86VidModeSwitchMode(x11_display, DefaultScreen(x11_display), +1); */

    dga_mode_selected = 0;	/* 現在選択中のDGAモード情報クリア */

    /* キーボード・マウスのグラブを解除 */
    XUngrabPointer(x11_display, CurrentTime);
    XUngrabKeyboard(x11_display, CurrentTime);

    x11_get_focus = FALSE;
    x11_set_attribute_focus_out();

    XSync(x11_display, True);		/* 全イベント破棄 */
}

#endif


/*======================================================================*/

static	Colormap	x11_colormap;		/* 使用中のカラーマップID */
static	int		x11_cmap_type;		/* 現在のカラーマップ処理 */

/* 使用中の色の数を自力で管理するので、そのための変数を用意 */
static	unsigned long	color_cell[256];	/* ピクセル値の配列	*/
static	int		nr_color_cell_used;	/* 配列の使用済み位置	*/
static	int		sz_color_cell;		/* 配列の最大数		*/


static	int	create_colormap(int fullscreen)
{
    int i, j, max;
    unsigned long plane[1];	/* dummy */
    unsigned long pixel[256];

    if (verbose_proc) printf("  Colormap: ");

    x11_colormap = DefaultColormapOfScreen(x11_screen);

    sz_color_cell = 0;

    switch (colormap_type) {
    case 0:				/* 共有カラーセルを使用 */
	if (fullscreen == FALSE) {
	    if (verbose_proc) printf("shared ... ");
	    for (i = 0; i < 3; i++) {
		if      (i == 0) max = 144;	/* 最初は、144色確保     */
		else if (i == 1) max = 24;	/* だめなら 24色、       */
		else             max = 16;	/* さらには 16色、と試す */

		if (XAllocColorCells(x11_display, 
				     DefaultColormapOfScreen(x11_screen),
				     False, plane, 0, pixel, max)) {
		    /* ok */
		    nr_color_cell_used = 0;
		    sz_color_cell      = max;
		    for (j = 0; j < sz_color_cell; j++) {
			color_cell[j] = pixel[j];
		    }
		    break;
		}
	    }
	    if (sz_color_cell > 0) {
		if (verbose_proc) printf("OK (%d colors)\n", sz_color_cell);
		x11_cmap_type = 0;
		return sz_color_cell;
	    }
	    if (verbose_proc) printf("FAILED, ");
	}
	/* FALLTHROUGH */

    case 1:				/* プライベートカラーマップを確保 */
	if (x11_visual->class == PseudoColor) {
	    if (verbose_proc) printf("private ... ");

	    x11_colormap = XCreateColormap(x11_display, x11_window,
					   x11_visual, AllocAll);

	    if (fullscreen == FALSE) {
		XSetWindowColormap(x11_display, x11_window, x11_colormap);
	    }

	    /* 本当は bpp に依存した値のはずだが・・・ */
	    nr_color_cell_used = 0;
	    sz_color_cell      = 144;
	    for (j = 0; j < sz_color_cell; j++) {
		color_cell[j] = j;
	    }

	    if (verbose_proc) printf("OK\n");
	    x11_cmap_type = 1;
	    return sz_color_cell;
	}
	/* FALLTHROUGH */

    case 2:				/* 色は必要時に動的に確保 */
	if (verbose_proc) printf("no color allocated\n");
#if 0
	if (x11_visual->class == PseudoColor ||
	    x11_visual->class == DirectColor) {
	    if        (x11_depth <= 4) {
		nr_color_cell_used = 0;
		sz_color_cell      = 16;
	    } else if (x11_depth <= 8) {
		nr_color_cell_used = 0;
		sz_color_cell      = 24;
	    } else {
		nr_color_cell_used = 0;
		sz_color_cell      = 144;
	    }
	} else
#endif
	{
	    nr_color_cell_used = 0;
	    sz_color_cell      = 144;
	}
	x11_cmap_type = 2;
	return sz_color_cell;
    }

    return 0;
}


static	int	reuse_colormap(void)
{
    switch (x11_cmap_type) {
    case 0:
	/* 引き続き、同じ共有カラーセルを使用 */
	break;

    case 1:
	/* 引き続き、同じカラーマップを使用 */
	break;

    case 2:
	XFreeColors(x11_display, x11_colormap,
		    color_cell, nr_color_cell_used, 0);
	break;
    }

    nr_color_cell_used = 0;

    return sz_color_cell;
}


static	void	destroy_colormap(void)
{
    switch (x11_cmap_type) {
    case 0:
	XFreeColors(x11_display, DefaultColormapOfScreen(x11_screen),
		    color_cell, sz_color_cell, 0);
	break;

    case 1:
	XFreeColormap(x11_display, x11_colormap);
#if 0		/* DGAでカラーマップをセットした場合、必ずデフォルトに戻す ! */
	XSetWindowColormap(x11_display, x11_window,
			   DefaultColormapOfScreen(x11_screen));
#endif
	break;

    case 2:
	XFreeColors(x11_display, x11_colormap,
		    color_cell, nr_color_cell_used, 0);
	break;
    }

    sz_color_cell = 0;
}


/*======================================================================*/

static	XImage   *image;


static	void	*create_image(int width, int height)
{
    void *buf = NULL;

#ifdef MITSHM
    if (use_SHM) {			/* MIS-SHM が実装されてるかを判定 */
	int tmp;
	if (! XQueryExtension(x11_display, "MIT-SHM", &tmp, &tmp, &tmp)) {
	    if (verbose_proc) printf("  X-Server not support MIT-SHM\n");
	    use_SHM = FALSE;
	}
    }

    if (use_SHM) {

	if (verbose_proc) printf("  Using shared memory (MIT-SHM):\n"
				 "    CreateImage ... ");
	image = XShmCreateImage(x11_display, x11_visual, x11_depth,
				ZPixmap, NULL, &SHMInfo,
				width, height);

	if (image) {

	    if (verbose_proc) printf("GetInfo ... ");
	    SHMInfo.shmid = shmget(IPC_PRIVATE,
				   image->bytes_per_line * image->height,
				   IPC_CREAT | 0777);
	    if (SHMInfo.shmid < 0) {
		use_SHM = FALSE;
	    }

	    XSetErrorHandler(private_handler);	/* エラーハンドラを横取り */
						/* (XShmAttach()異常検出) */
	    if (use_SHM) {

		if (verbose_proc) printf("Allocate ... ");
		image->data =
		    SHMInfo.shmaddr = (char *)shmat(SHMInfo.shmid, 0, 0);
		if (image->data == NULL) {
		    use_SHM = FALSE;
		}

		if (use_SHM) {
		    if (verbose_proc) printf("Attach ... ");
		    SHMInfo.readOnly = False;

		    if (! XShmAttach(x11_display, &SHMInfo)) {
			use_SHM = FALSE;
		    }

		    XSync(x11_display, False);
		    /* sleep(2); */
		}
	    }

	    if (SHMInfo.shmid >= 0) shmctl(SHMInfo.shmid, IPC_RMID, 0);


	    if (use_SHM) {				/* すべて成功 */
		buf = image->data;
		if (verbose_proc) printf("OK\n");
	    } else {					/* どっかで失敗 */
		if (verbose_proc) printf("FAILED(can't use shared memory)\n");
		if (SHMInfo.shmaddr) shmdt(SHMInfo.shmaddr);
		XDestroyImage(image);
		image = NULL;
	    }

	    XSetErrorHandler(None);		/* エラーハンドラを戻す */

	} else {
	    if (verbose_proc) printf("FAILED(can't use shared memory)\n");
	    use_SHM = FALSE;
	}
    }

    if (use_SHM == FALSE)
#endif
    {
	/* スクリーンバッファを確保 */

	if (verbose_proc) printf("  Screen buffer: Memory allocate ... ");
	buf = malloc(width * height * x11_byte_per_pixel);
	if (verbose_proc) { if (buf == NULL) printf("FAILED\n"); }

	if (buf) {
	    /* スクリーンバッファをイメージに割り当て */

	    if (verbose_proc) printf("CreateImage ... ");
	    image = XCreateImage(x11_display, x11_visual, x11_depth,
				 ZPixmap, 0, buf,
				 width, height, 8, 0);
	    if (verbose_proc) printf("%s\n", (image ? "OK" : "FAILED"));
	    if (image == NULL) {
		free(buf);
		buf = NULL;
	    }
	}
    }

    return buf;
}


static	void	destroy_image(void *buf)
{

#ifdef MITSHM
    if (use_SHM) {
	XShmDetach(x11_display, &SHMInfo);
	if (SHMInfo.shmaddr) shmdt(SHMInfo.shmaddr);
	/*if (SHMInfo.shmid >= 0) shmctl(SHMInfo.shmid,IPC_RMID,0);*/
    }
#endif

    if (image) {
	XDestroyImage(image);
	image = NULL;
    }

#if 0		/* buf はもう不要なので、ここで free しようとしたが、
		   image のほうでまだ使用中らしく、free するとコケてしまう。
		   というか、 XDestroyImage してるのに………。
		   XSync をしてもだめな様子。じゃあ、いつ free するの ?? */
#ifdef MITSHM
    if (use_SHM == FALSE)
#endif
    {
	free(buf);
    }
#endif
}


/************************************************************************
 *	色の確保
 *	色の解放
 ************************************************************************/

static	void	x11_graph_add_color(const PC88_PALETTE_T color[],
				    int nr_color, unsigned long pixel[])
{
    

    int i;
    XColor xcolor[256];

    /* debug */
    if (nr_color_cell_used + nr_color > sz_color_cell) {
	/* 追加すべき色が多すぎ */
	printf("color add err? %d %d\n", nr_color, nr_color_cell_used);
	return;
    }


    for (i = 0; i < nr_color; i++) {
	xcolor[i].red   = (unsigned short)color[i].red   << 8;
	xcolor[i].green = (unsigned short)color[i].green << 8;
	xcolor[i].blue  = (unsigned short)color[i].blue  << 8;
	xcolor[i].flags = DoRed | DoGreen | DoBlue;
    }


    switch (x11_cmap_type) {
    case 0:
    case 1:
	for (i = 0; i < nr_color; i++) {
	    pixel[i] = 
		xcolor[i].pixel = color_cell[ nr_color_cell_used ];
	    nr_color_cell_used ++;
	}

	XStoreColors(x11_display, x11_colormap, xcolor, nr_color);
#ifdef	USE_DGA
	if (graph_info.fullscreen) {
	    XF86DGAInstallColormap(x11_display, DefaultScreen(x11_display),
				   x11_colormap);
	}
#endif
	break;

    case 2:
	for (i = 0; i < nr_color; i++) {
	    if (XAllocColor(x11_display, x11_colormap, &xcolor[i])) {
		/* 成功 */;	/* DO NOTHING */
	    } else {
		/* 失敗したら、黒色を確保。これは失敗しないだろう… */
		xcolor[i].red = xcolor[i].green = xcolor[i].blue = 0;
		XAllocColor(x11_display, x11_colormap, &xcolor[i]);
	    }
	    pixel[i] = 
		color_cell[ nr_color_cell_used ] = xcolor[i].pixel;
	    nr_color_cell_used ++;
	}
    }
}

/************************************************************************/

static	void	x11_graph_remove_color(int nr_pixel, unsigned long pixel[])
{

    /* debug */
    if (nr_pixel > nr_color_cell_used) {
	/* 削除すべき色が多すぎ */
	printf("color remove err? %d %d\n", nr_pixel, nr_color_cell_used);
    } else {
	if (memcmp(&color_cell[ nr_color_cell_used - nr_pixel ], pixel,
		   sizeof(unsigned long) * nr_pixel) != 0) {
	    /* 削除すべき色が、直前に追加した色と違う */
	    printf("color remove unmatch???\n");
	}
    }


    switch (x11_cmap_type) {
    case 0:
    case 1:
	nr_color_cell_used -= nr_pixel;
	break;
    case 2:
	nr_color_cell_used -= nr_pixel;
	XFreeColors(x11_display, x11_colormap, pixel, nr_pixel, 0);
	break;
    }
}


/************************************************************************
 *	グラフィックの更新
 ************************************************************************/

static	void	x11_graph_update(int nr_rect, T_GRAPH_RECT rect[])
{
    int i;

    if (graph_info.fullscreen == FALSE) {

	for (i = 0; i < nr_rect; i++) {
#ifdef MITSHM
	    if (use_SHM) {
		XShmPutImage(x11_display, x11_window, x11_gc, image,
			     rect[i].x, rect[i].y, 
			     rect[i].x, rect[i].y, 
			     rect[i].width, rect[i].height, False);
	    } else
#endif
	    {
		XPutImage(x11_display, x11_window, x11_gc, image,
			  rect[i].x, rect[i].y, 
			  rect[i].x, rect[i].y, 
			  rect[i].width, rect[i].height);
	    }
	}

	if (use_xsync) {
	    XSync(x11_display, False);
	} else {
	    XFlush(x11_display);
	}
    }
}
