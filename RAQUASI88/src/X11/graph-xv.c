/***********************************************************************
 * グラフィック処理 (X11 Window & DGA 1.0)
 *
 *
 ************************************************************************/

#ifdef	USE_XV

#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
#include <X11/extensions/Xv.h>
#include <X11/extensions/Xvlib.h>

#ifndef MITSHM
#error "X11_XV defined but X11_MITSHM is not defined, XV needs X11_MITSHM!"
#endif

/************************************************************************/

static	int	xv_port = -1;
static	int	xv_format;
static	int	xv_width;		/* 現在のウインドウの幅 */
static	int	xv_height;		/* 現在のウインドウの高 */

static	GC	xv_gc;
static	enum {
    RGB555,
    RGB565,
    RGB888,
}		xv_rgb_format;


/************************************************************************
 *	XVの初期化
 *	XVの終了
 ************************************************************************/

enum {
    XV_ERR_NONE = 0,
    XV_ERR_AVAILABLE,
    XV_ERR_QUERY_MITSHM,
    XV_ERR_QUERY_EXTENSION,
};
static	int	XV_error = XV_ERR_AVAILABLE;

static	void	xv_init(void)
{
    int i;
    unsigned int u;

    if (! XQueryExtension(x11_display, "MIT-SHM", &i, &i, &i)) {
	XV_error = XV_ERR_QUERY_MITSHM;
    }
    else
    if (XvQueryExtension(x11_display, &u, &u, &u, &u, &u) != Success) {
	XV_error = XV_ERR_QUERY_EXTENSION;
    }
    else
    {
	XV_error = XV_ERR_NONE;
	x11_enable_fullscreen = 1;
    }
}

static	void	xv_verbose(void)
{
    printf("\n");
    printf("  XV : ");

    if      (XV_error == XV_ERR_NONE)
	printf("OK");
    else if (XV_error == XV_ERR_QUERY_MITSHM)
	printf("FAILED (Xserver not support MIT-SHM)");
    else if (XV_error == XV_ERR_QUERY_EXTENSION)
	printf("FAILED (XvQueryExtension");
    else
	printf("FAILED (Not Support)");

    if (XV_error == XV_ERR_NONE) printf(", fullscreen available\n");
    else                         printf(", fullscreen not available\n");
}

/************************************************************************/

static	void	xv_exit(void)
{
    if (xv_port > -1) {
	XvUngrabPort(x11_display, xv_port, CurrentTime);
    }
}


/************************************************************************
 *	グラフィック処理の初期化
 *	グラフィック処理の動作
 *	グラフィック処理の終了
 ************************************************************************/

/* 知ってるフォーマットを探す。見つけたらそれをロック */
static	int	xv_search_format_and_grab(void);

static	const T_GRAPH_SPEC	*xv_graph_init(void)
{
    int	win_w, win_h;
    int	ful_w, ful_h;

    XGCValues xgcv;

    xv_gc = XCreateGC(x11_display, DefaultRootWindow(x11_display), 0, &xgcv);


    /* 知ってるフォーマットを探す。見つけたらそれをロック */
    if (xv_search_format_and_grab() == FALSE) {
	return NULL;
    }


    /* 利用可能なウインドウのサイズを調べておく */

    win_w = x11_screen->width;
    win_h = x11_screen->height;
    ful_w = x11_screen->width;
    ful_h = x11_screen->height;


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

/*======================================================================*/

/* XVのフォーマットで、知っているのがあるかをチェック
	(x11_depth と x11_byte_per_pixel をここでセットする) */

#define FOURCC_YUY2 0x32595559
static	int	xv_search_format_and_grab(void)
{
    unsigned int num_adaptor;
    XvAdaptorInfo *adaptor;
    unsigned int i, p;
    int n, j, num_format;
    XvImageFormatValues *f;
    XvAttribute *attr;

    if (XvQueryAdaptors(x11_display, DefaultRootWindow(x11_display),
			&num_adaptor, &adaptor) != Success) {

	if (verbose_proc) printf("  XvQueryAdaptors failed\n");
	return FALSE;
    }

    for (n = 0; n < 2; n++) {	/* n==0 なら RGBフォーマットを
				   n==1 なら YUY2フォーマットを探す */

	if (n == 1) break;	/* YUY2 は未実装… */

	for (i = 0; i < num_adaptor; i++) {

	    for (p = adaptor[i].base_id;
		 p < adaptor[i].base_id + adaptor[i].num_ports; p++) {

		f = XvListImageFormats(x11_display, p, &num_format);

		for (j = 0; j < num_format; j++) {

		    if (n == 0) {	/* RGBフォーマットを探す */

			if ((f[j].type   == XvRGB) &&
			    (f[j].format == XvPacked)) {

			    /* 知っている形式かな？ */
			    x11_byte_per_pixel = 0;
#ifdef	SUPPORT_16BPP
			    if (/*f[j].depth <= 16 &&*/
				f[j].bits_per_pixel == 16) {
				if (f[j].red_mask   == (0x1F << 10) &&
				    f[j].green_mask == (0x1F <<  5) &&
				    f[j].blue_mask  == (0x1F      )) {

				    x11_depth = f[j].depth;
				    x11_byte_per_pixel = 2;
				    xv_rgb_format = RGB555;
				}
				else
				if (f[j].red_mask   == (0x1F << 11) &&
				    f[j].green_mask == (0x3F <<  5) &&
				    f[j].blue_mask  == (0x1F      )) {

				    x11_depth = f[j].depth;
				    x11_byte_per_pixel = 2;
				    xv_rgb_format = RGB565;
				}
			    }
#endif
#ifdef	SUPPORT_32BPP
			    if (/*f[j].depth <= 32 &&*/
				f[j].bits_per_pixel == 32) {

				if (f[j].red_mask   == (0xFF << 16) &&
				    f[j].green_mask == (0xFF <<  8) &&
				    f[j].blue_mask  == (0xFF      )) {

				    x11_depth = f[j].depth;
				    x11_byte_per_pixel = 4;
				    xv_rgb_format = RGB888;
				}
			    }
#endif
			    if (x11_byte_per_pixel == 0) continue;

			    /* よし、知っている形式だ */
			    if (XvGrabPort(x11_display, p, CurrentTime)
								  == Success) {
				xv_port   = p;
				xv_format = f[j].id;
#if 0
			printf("\n");
			printf("depth          %d\n",x11_depth);
			printf("bits_per_pixel %d\n",f[j].bits_per_pixel);
			printf("x11_byte_per_pixel %d\n",x11_byte_per_pixel);
			printf("xv_rgb_format      %d\n",xv_rgb_format);
			printf("xv_port   %d\n",xv_port);
			printf("xv_format %08x\n",xv_format);
#endif
				goto SUCCESS;
			    }
			}

		    } else {		/* YUY2フォーマットを探す */

			if (f[j].id == FOURCC_YUY2) {

			    if (XvGrabPort(x11_display, p, CurrentTime)
								  == Success) {
				xv_port   = p;
				xv_format = FOURCC_YUY2;
				/*fourcc_format = format;*/
				/*bpp = f[j].bits_per_pixel;*/
				goto SUCCESS;
			    }
			}
		    }
		}
		XFree(f);
	    }
	}
    }
    XvFreeAdaptorInfo(adaptor);

    if (verbose_proc) printf("  XV : FAILED (no suitable format)\n");
    return FALSE;


 SUCCESS:
    XFree(f);
    XvFreeAdaptorInfo(adaptor);

    /* これは何？ */
    attr = XvQueryPortAttributes(x11_display, xv_port, &n);
    for (j = 0; j < n; j++) {
	if (strcmp(attr[j].name, "XV_AUTOPAINT_COLORKEY") == 0) {
            Atom atom = XInternAtom(x11_display, "XV_AUTOPAINT_COLORKEY", False);
            XvSetPortAttribute(x11_display, xv_port, atom, 1);
            break;
	}
    }

    return TRUE;
}

/************************************************************************/

/* ウインドウの生成時／リサイズ時／破棄時 および、
   全画面モード開始時／解像度切替時／終了時 の、 実際の処理をする関数 */

static	int	create_XV(int *width, int *height,
			  void **ret_buffer, int fullscreen, double aspect);
static	int	resize_XV(int *width, int *height, void *old_buffer,
			  void **ret_buffer, int fillscreen, double aspect);
static	void	destroy_XV(void *old_buffer);



static	const T_GRAPH_INFO	*xv_graph_setup(int width, int height,
						int fullscreen, double aspect)
{
    void *buf = NULL;
    int success;

    /* 全画面不可なら、全画面要求は無視 */
    if ((x11_enable_fullscreen == FALSE) && (fullscreen)) {
	fullscreen = FALSE;
    }


    /* 全画面 → ウインドウ の場合 */

    /* ウインドウ → 全画面 切り替えの場合、
       一旦ウインドウを破棄しないと、全画面ウインドウが原点に移動しない
       全画面 → ウインドウ 切り替えのばあい、
       一旦ウインドウを破棄しないと、ウインドウ枠が消えたまま */

    if (graph_exist) {
	if (((graph_info.fullscreen == FALSE) && (fullscreen))         ||
	    ((graph_info.fullscreen)          && (fullscreen == FALSE))) {
	    destroy_XV(graph_info.buffer);
	    graph_exist = FALSE;
	}
    }


    /* □ → ウインドウ の場合 */
    if (graph_exist == 0) {
	success = create_XV(&width, &height, &buf, fullscreen, aspect);
    } else {
	success = resize_XV(&width, &height,
			    graph_info.buffer, &buf, fullscreen, aspect);
    }

    if (success == FALSE) {
	graph_exist = FALSE;
	return NULL;
    }


    graph_exist = TRUE;

    if (fullscreen) {
	graph_info.fullscreen	= TRUE;
    } else {
	graph_info.fullscreen	= FALSE;
    }
    graph_info.width		= width;
    graph_info.height		= height;
    graph_info.byte_per_pixel	= x11_byte_per_pixel;
    graph_info.byte_per_line	= width * x11_byte_per_pixel;
    graph_info.buffer		= buf;
    graph_info.nr_color		= 255;
    graph_info.write_only	= FALSE;
    graph_info.broken_mouse	= FALSE;
    graph_info.draw_start	= NULL;
    graph_info.draw_finish	= NULL;
    graph_info.dont_frameskip	= FALSE;

    if (fullscreen == FALSE) {
	/* ウインドウのタイトルバーを復元 */
	graph_set_window_title(NULL);
    }

    XMapRaised(x11_display, x11_window);


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

static	void	xv_graph_exit(void)
{
    if (graph_exist) {

	destroy_XV(graph_info.buffer);

	graph_exist = FALSE;
    }
}

/*======================================================================*/

/* ウインドウの生成時・リサイズ時・破棄時には、イメージの確保・解放が必要 */
static	void	*create_XVimage(int width, int height);
static	void	destroy_XVimage(void *buf);

/* fulscreen と aspect により、*width, *height を計算しなおし再設定する */
static	void	calc_size(int *width, int *height,
			  int fullscreen, double aspect)
{
    if (fullscreen) {

	/* アスペクト比が未指定の場合、スクリーンの縦横比で代用 */

	if (aspect < 0.01) {
	    aspect = (double) x11_screen->width / x11_screen->height;
	}

	/* アスペクト比に応じて、縦横サイズを計算し直す */
	/* (横は 16の倍数、縦は 2の倍数にしておくこと)  */

	if (aspect <= ((double)(*width) / (*height))) {
	    /* モニターのほうが縦長 */
	    /* *width はそのまま */
	    *height = ((int)((*width) / aspect) + 1) & ~1;
	} else {
	    /* モニターのほうが横長 */
	    *width = ((int)((*height) * aspect) + 15) & ~15;
	    /* *height はそのまま */
	}

	xv_width  = x11_screen->width;
	xv_height = x11_screen->height;

	if (*width  == xv_width &&
	    *height == xv_height) {

	    x11_scaling = FALSE;

	} else {
	    x11_scaling = TRUE;

	    x11_scale_x_num = *width;
	    x11_scale_x_den = x11_screen->width;

	    x11_scale_y_num = *height;
	    x11_scale_y_den = x11_screen->height;

	    /*printf("scale x = %d/%d, y = %d/%d\n", x11_scale_x_num, x11_scale_x_den, x11_scale_y_num, x11_scale_y_den);*/
	}

    } else {
	xv_width  = *width;
	xv_height = *height;

	x11_scaling = FALSE;
    }
}


static	int	create_XV(int *width, int *height,
			  void **ret_buffer, int fullscreen, double aspect)
{
    /* fulscreen と aspect により、*width, *height を計算しなおし再設定する */
    calc_size(width, height, fullscreen, aspect);


    if (verbose_proc) printf("  Opening window ... ");
    x11_window = XCreateSimpleWindow(x11_display,
				     RootWindowOfScreen(x11_screen),
				     0, 0,
				     xv_width, xv_height,
				     0,
				     WhitePixelOfScreen(x11_screen),
				     BlackPixelOfScreen(x11_screen));
    if (verbose_proc)
	printf("%s (%dx%d => %dx%d)\n", (x11_window ? "OK" : "FAILED"), *width, *height, xv_width, xv_height);

    if (! x11_window) {
	return FALSE; 
    }

    /* ウインドウマネージャーへ特性(サイズ変更不可)を指示する */
    set_wm_hints(xv_width, xv_height, fullscreen);

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
    (*ret_buffer) = create_XVimage(*width, *height);

    /* Drag & Drop 受け付け開始 */
    xdnd_start();

    return (*ret_buffer) ? TRUE : FALSE;
}


static	int	resize_XV(int *width, int *height, void *old_buffer,
			  void **ret_buffer, int fullscreen, double aspect)
{
    Window child;
    int x, y;

    /* fulscreen と aspect により、*width, *height を計算しなおし再設定する */
    calc_size(width, height, fullscreen, aspect);


    if (verbose_proc) printf("  Resizing window ... ");

    /* ウインドウマネージャーへ新たなサイズを指示する */
    set_wm_hints(xv_width, xv_height, fullscreen);

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
	XMoveResizeWindow(x11_display, x11_window, x, y, xv_width, xv_height);
    } else
#endif
    {
	XResizeWindow(x11_display, x11_window, xv_width, xv_height);
    }

    if (verbose_proc)
	printf("%s (%dx%d => %dx%d)\n", (x11_window ? "OK" : "FAILED"), *width, *height, xv_width, xv_height);

    /* image を破棄する */
    if (old_buffer) {
	destroy_XVimage(old_buffer);
    }

    /* スクリーンバッファ と image を確保 */
    (*ret_buffer) = create_XVimage(*width, *height);

    return (*ret_buffer) ? TRUE : FALSE;
}


static	void	destroy_XV(void *old_buffer)
{
    if (verbose_proc) printf("  Closing Window\n");

    /* イメージ破棄 */
    if (old_buffer) {
	destroy_XVimage(old_buffer);
    }

    /* ウインドウ破棄 */
    XDestroyWindow(x11_display, x11_window);

    if (x11_grab) {
	XUngrabPointer(x11_display, CurrentTime);
	x11_grab = FALSE;
    }

    XSync(x11_display, True);		/* 全イベント破棄 */
}

/*======================================================================*/

static	XvImage		*xvimage = NULL;

static	void	*create_XVimage(int width, int height)
{
    void *buf = NULL;

    use_SHM = TRUE;				/* SHM が必須 */

    {
	if (verbose_proc) printf("  Using shared memory (MIT-SHM):\n"
				 "    CreateImage (%dx%d)... ", width, height);
	xvimage = XvShmCreateImage(x11_display,
				   xv_port,
				   xv_format,
				   0,
				   width,
				   height,
				   &SHMInfo);

	if (xvimage &&
	    (xvimage->width >= width) && (xvimage->height >= height)) {

	    if (verbose_proc) printf("GetInfo ... ");
	    SHMInfo.shmid = shmget(IPC_PRIVATE,
				   xvimage->data_size,
				   IPC_CREAT | 0777);
	    if (SHMInfo.shmid < 0) {
		use_SHM = FALSE;
	    }

	    XSetErrorHandler(private_handler);	/* エラーハンドラを横取り */
						/* (XShmAttach()異常検出) */
	    if (use_SHM) {

		if (verbose_proc) printf("Allocate ... ");
		xvimage->data =
		    SHMInfo.shmaddr = (char *)shmat(SHMInfo.shmid, 0, 0);
		if (xvimage->data == NULL) {
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
		buf = xvimage->data;
		if (verbose_proc) printf("OK\n");
	    } else {					/* どっかで失敗 */
		if (verbose_proc) printf("FAILED(can't use shared memory)\n");
		if (SHMInfo.shmaddr) shmdt(SHMInfo.shmaddr);
		XFree(xvimage);
		xvimage = NULL;
	    }

	    XSetErrorHandler(None);		/* エラーハンドラを戻す */

	} else {
	    if (verbose_proc) printf("FAILED(can't use shared memory)\n");
	}
    }

    return buf;
}


static	void	destroy_XVimage(void *buf)
{
    if (use_SHM == FALSE) {
	XShmDetach(x11_display, &SHMInfo);
	if (SHMInfo.shmaddr) shmdt(SHMInfo.shmaddr);
	/*if (SHMInfo.shmid >= 0) shmctl(SHMInfo.shmid,IPC_RMID,0);*/
	SHMInfo.shmaddr = NULL;
    }

    if (xvimage) {
       XFree(xvimage);
       xvimage = NULL;
    }
}


/************************************************************************
 *	色の確保
 *	色の解放
 ************************************************************************/

static	void	xv_graph_add_color(const PC88_PALETTE_T color[],
				   int nr_color, unsigned long pixel[])
{
    int i;

    switch (xv_rgb_format) {

    case RGB555:
	for (i=0; i<nr_color; i++) {
	    pixel[i] = (((((unsigned long) color[i].red)   >> 3) << 10) |
			((((unsigned long) color[i].green) >> 3) <<  5) |
			( ((unsigned long) color[i].blue)  >> 3));
	}
	break;

    case RGB565:
	for (i=0; i<nr_color; i++) {
	    pixel[i] = (((((unsigned long) color[i].red)   >> 3) << 11) |
			((((unsigned long) color[i].green) >> 2) <<  5) |
			( ((unsigned long) color[i].blue)  >> 3));
	}
	break;

    case RGB888:
	for (i=0; i<nr_color; i++) {
	    pixel[i] = ((((unsigned long) color[i].red)   << 16) |
			(((unsigned long) color[i].green) <<  8) |
			(((unsigned long) color[i].blue)));
	}
	break;
    }
}

/************************************************************************/

static	void	xv_graph_remove_color(int nr_pixel, unsigned long pixel[])
{
    /* 色に関しては何も管理しないので、ここでもなにもしない */
}


/************************************************************************
 *	グラフィックの更新
 ************************************************************************/

static	void	xv_graph_update(int nr_rect, T_GRAPH_RECT rect[])
{
    int i;

    if (use_SHM == FALSE) return;

    XvShmPutImage(x11_display, xv_port, x11_window,
		  xv_gc, xvimage,
		  0, 0,			/* src */
		  x11_width, x11_height,
		  0, 0,			/* dst */
		  xv_width, xv_height,
		  True);

    if (use_xsync) {
	XSync(x11_display, False);
    } else {
	XFlush(x11_display);
    }
}


#endif
