/***********************************************************************
 * グラフィック処理 (システム依存)
 *
 *	詳細は、 graph.h 参照
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>

#include "quasi88.h"
#include "graph.h"
#include "device.h"

/************************************************************************/

/*#define	DEBUG_PRINTF*/


/* 以下は static な変数。オプションで変更できるのでグローバルにしてある */

    int	use_hwsurface	= TRUE;		/* HW SURFACE を使うかどうか	*/
    int	use_doublebuf	= FALSE;	/* ダブルバッファを使うかどうか	*/


/* 以下は、 event.c などで使用する、 OSD なグローバル変数 */

    int	sdl_mouse_rel_move;		/* マウス相対移動量検知可能か	*/



/************************************************************************/

static	T_GRAPH_SPEC	graph_spec;		/* 基本情報		*/

static	int		graph_exist;		/* 真で、画面生成済み	*/
static	T_GRAPH_INFO	graph_info;		/* その時の、画面情報	*/


/************************************************************************
 *	SDLの初期化
 *	SDLの終了
 ************************************************************************/

int	sdl_init(void)
{
    const SDL_version *libver;
    libver = SDL_Linked_Version();

    if (verbose_proc) {
	printf("Initializing SDL (%d.%d.%d) ... ",
	       libver->major, libver->minor, libver->patch); fflush(NULL);
    }

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {

	if (verbose_proc) printf("Failed\n");
	fprintf(stderr, "SDL Error: %s\n", SDL_GetError());

	return FALSE;

    } else {

	if (verbose_proc) printf("OK\n");
	return TRUE;
    }
}

/************************************************************************/

void	sdl_exit(void)
{
    SDL_Quit();
}


/************************************************************************
 *	グラフィック処理の初期化
 *	グラフィック処理の動作
 *	グラフィック処理の終了
 ************************************************************************/

static	char	sdl_vname[16];
static	int	sdl_depth;
static	int	sdl_byte_per_pixel;

static	SDL_Rect **sdl_mode;
static	Uint32	   sdl_mode_flags;


const T_GRAPH_SPEC	*graph_init(void)
{
    int	win_w, win_h;
    int	ful_w, ful_h;
    int i;
    const SDL_VideoInfo *vi;

    sdl_vname[0] = '\0';
    SDL_VideoDriverName(sdl_vname, sizeof(sdl_vname));
    vi = SDL_GetVideoInfo();

    if (verbose_proc) {
	printf("Initializing Graphic System (SDL:%s) ... \n", sdl_vname);
    }

    /* 色深度と、ピクセルあたりのバイト数をチェック */

    sdl_depth          = vi->vfmt->BitsPerPixel;
    sdl_byte_per_pixel = vi->vfmt->BytesPerPixel;

#if	defined(SUPPORT_16BPP) && defined(SUPPORT_32BPP)
    if        (sdl_byte_per_pixel == 2 ||
	       sdl_byte_per_pixel == 4) {
	/* OK */ ;
    } else {
	sdl_depth          = 16;
	sdl_byte_per_pixel = 2;
    }
#elif	defined(SUPPORT_16BPP)
	sdl_depth          = 16;
	sdl_byte_per_pixel = 2;
#elif	defined(SUPPORT_32BPP)
	sdl_depth          = 32;
	sdl_byte_per_pixel = 4;
#endif



#ifdef	DEBUG_PRINTF
    printf("  <VideoInfo> %s\n", sdl_vname);
    printf("  hw_available  %d  ", vi->hw_available);
    printf("  wm_available  %d\n", vi->wm_available);
    printf("  blit_hw       %d  ", vi->blit_hw     );
    printf("  blit_hw_CC    %d  ", vi->blit_hw_CC  );
    printf("  blit_hw_A     %d\n", vi->blit_hw_A   );
    printf("  blit_sw       %d  ", vi->blit_sw     );
    printf("  blit_sw_CC    %d  ", vi->blit_sw_CC  );
    printf("  blit_sw_A     %d\n", vi->blit_sw_A   );
    printf("  blit_fill     %d  ", vi->blit_fill   );
    printf("  video_mem     %d\n", vi->video_mem   );
    printf("  palette       %p\n", vi->vfmt->palette       );
    printf("  BitsPerPixel  %2d  ", vi->vfmt->BitsPerPixel  );
    printf("  BytesPerPixel %2d\n", vi->vfmt->BytesPerPixel );
    printf("  Rmask   %8x  ", vi->vfmt->Rmask         );
    printf("  Gmask   %8x  ", vi->vfmt->Gmask         );
    printf("  Bmask   %8x  ", vi->vfmt->Bmask         );
    printf("  Amask   %8x\n", vi->vfmt->Amask         );
    printf("  Rshift        %2d  ", vi->vfmt->Rshift        );
    printf("  Gshift        %2d  ", vi->vfmt->Gshift        );
    printf("  Bshift        %2d  ", vi->vfmt->Bshift        );
    printf("  Ashift        %2d\n", vi->vfmt->Ashift        );
    printf("  Rloss          %d  ", vi->vfmt->Rloss         );
    printf("  Gloss          %d  ", vi->vfmt->Gloss         );
    printf("  Bloss          %d  ", vi->vfmt->Bloss         );
    printf("  Aloss          %d\n", vi->vfmt->Aloss         );
    printf("  colorkey       %x  ", vi->vfmt->colorkey      );
    printf("  alpha          %d\n", vi->vfmt->alpha         );
    printf("\n");
#endif

    /* 利用可能なウインドウのサイズを調べておく */
    for (i = 0; i < 2; i++) {
	Uint32 flags = 0;
	int w, h;

	if (i == 0) flags = 0;			/* 1回目はウインドウ、     */
	else        flags = SDL_FULLSCREEN;	/* 2回目は全画面をチェック */

	if (use_hwsurface) flags |= SDL_HWPALETTE | SDL_HWSURFACE;
	else               flags |= SDL_HWPALETTE | SDL_SWSURFACE;

	if (use_doublebuf) flags |= SDL_DOUBLEBUF;

	/* 利用可能な最大サイズを取得 */
	sdl_mode = SDL_ListModes(NULL, flags);

	if        (sdl_mode == (SDL_Rect**) 0) {	/* 全モード不可 */
	    w = 0;
	    h = 0;
	} else if (sdl_mode == (SDL_Rect**)-1) {	/* 全モード可 */
	    w = 10000;
	    h = 10000;
	} else {					/* モードをチェック */
	    w = sdl_mode[0]->w;					/* 最初が   */
	    h = sdl_mode[0]->h;					/*   最大値 */

#ifdef	DEBUG_PRINTF
	    {
	      int j;
	      for (j=0; sdl_mode[j]; j++)
		printf("  %sSize %3d:  %4d x %4d  (%.4f)\n",
		       (i==0) ? "Window" : "Fullscreen", j, sdl_mode[j]->w,
		       sdl_mode[j]->h, (double)sdl_mode[j]->w/sdl_mode[j]->h);
	    }
#endif
	}

	if (i == 0) { win_w = w;  win_h = h; }
	else        { ful_w = w;  ful_h = h; }

	sdl_mode_flags = flags;
    }
    /* この時点で、 sdl_mode には、全画面時のモード一覧がセットされている。
       sdl_mode_flags には、全画面時のモードのフラグがセットされている。*/

    graph_spec.window_max_width      = win_w;
    graph_spec.window_max_height     = win_h;
    graph_spec.fullscreen_max_width  = ful_w;
    graph_spec.fullscreen_max_height = ful_h;
    graph_spec.forbid_status         = FALSE;
    graph_spec.forbid_half           = FALSE;

    if (verbose_proc)
	printf("  INFO:%dbpp(%dbyte), Maxsize=win(%d,%d),full(%d,%d)\n",
	       sdl_depth, sdl_byte_per_pixel, win_w, win_h, ful_w, ful_h);

    return &graph_spec;
}

/************************************************************************/

static	int	search_mode(int w, int h, double aspect);

static	SDL_Surface	*sdl_display;
static	SDL_Surface	*sdl_offscreen;


const T_GRAPH_INFO	*graph_setup(int width, int height,
				     int fullscreen, double aspect)
{
    Uint32 flags;

    /* サイズ変更や、ウインドウ⇔全画面切替の際は、再度 SDL_SetVideoMode() を
       呼ぶが、その前に一旦ビデオサブシステムを終了させる必要があるらしい(?)。
       (ビデオドライバ依存か？ x11, windib, directx は、終了は不要) */

    if (graph_exist) {
	if (verbose_proc) printf("Re-Initializing Graphic System (SDL:%s) ...",
				 sdl_vname);

	if ((graph_info.fullscreen == FALSE && fullscreen == FALSE) &&
	    (! (sdl_display->flags & SDL_FULLSCREEN))) {

	    /* ウインドウのサイズ変更時は、終了の必要はなさそうだが…… */
	    if (verbose_proc) printf("\n");

	} else {
	    SDL_QuitSubSystem(SDL_INIT_VIDEO);
	    graph_exist = FALSE;
	}
    }


    /* VIDEOを一旦終了したなら、VIDEOの再初期化 */
    if (! SDL_WasInit(SDL_INIT_VIDEO)) {
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
	    if (verbose_proc) printf(" FAILED\n");
	    return NULL;
	}
	/* VIDEOを一旦終了すると、 sdl_mode が無効になる。(デバイスによる？) */
	sdl_mode = SDL_ListModes(NULL, sdl_mode_flags);

		/* sdl_mode の内容が、以前と変わってしまったらどうしよう？ */

	if (verbose_proc) printf(" OK\n");
    }


    /* 全画面モードの場合、適切なモードを選択 */
    if (fullscreen) {
	int fit = search_mode(width, height, aspect);
	if (fit < 0) {
	    fullscreen = FALSE;
	} else {
	    width  = sdl_mode[fit]->w;
	    height = sdl_mode[fit]->h;
	}
    }

    /* ウインドウ（全画面）を開く */
    if (verbose_proc) {
	if (fullscreen) printf("  Trying full screen mode ... ");
	else            printf("  Opening window ... ");
    }

    if (fullscreen) flags = SDL_FULLSCREEN;
    else            flags = 0;

    if (use_hwsurface) flags |= SDL_HWPALETTE | SDL_HWSURFACE;
    else               flags |= SDL_HWPALETTE | SDL_SWSURFACE;

    if (use_doublebuf) flags |= SDL_DOUBLEBUF;

    sdl_display = SDL_SetVideoMode(width, height, sdl_depth, flags);

    if (verbose_proc)
	printf("%s (%dx%d)\n", (sdl_display ? "OK" : "FAILED"), width, height);

    if (sdl_display == NULL) return NULL;


    /* スクリーンバッファを確保 */

    if (verbose_proc) printf("  Allocating screen buffer ... ");

    sdl_offscreen = SDL_CreateRGBSurface(SDL_SWSURFACE,
					 width, height, sdl_depth,
					 0, 0, 0, 0);
    if (verbose_proc) printf("%s\n", (sdl_offscreen ? "OK" : "FAILED"));

    if (sdl_offscreen == NULL) return NULL;



    /* 画面情報をセットして、返す */

    graph_info.fullscreen	= fullscreen;
    graph_info.width		= sdl_offscreen->w;
    graph_info.height		= sdl_offscreen->h;
    graph_info.byte_per_pixel	= sdl_byte_per_pixel;
    graph_info.byte_per_line	= sdl_offscreen->pitch;
    graph_info.buffer		= sdl_offscreen->pixels;
    graph_info.nr_color		= 255;
    graph_info.write_only	= FALSE;
    graph_info.broken_mouse	= FALSE;
    graph_info.draw_start	= NULL;
    graph_info.draw_finish	= NULL;
    graph_info.dont_frameskip	= FALSE;

    graph_exist = TRUE;

    if (verbose_proc)
	printf("    VideoMode %dx%d -> %dx%dx%d(%d)  %c%c%c%c  R:%x G:%x B:%x\n",
	       width, height, sdl_display->w, sdl_display->h,
	       sdl_display->format->BitsPerPixel,
	       sdl_display->format->BytesPerPixel,
	       (sdl_display->flags & SDL_SWSURFACE) ? 'S' : '-',
	       (sdl_display->flags & SDL_HWSURFACE) ? 'H' : 'S',
	       (sdl_display->flags & SDL_DOUBLEBUF) ? 'D' : '-',
	       (sdl_display->flags & SDL_FULLSCREEN) ? 'F' : '-',
	       sdl_display->format->Rmask,
	       sdl_display->format->Gmask, sdl_display->format->Bmask);

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



/*======================================================================*/
#define	FABS(a)		(((a) >= 0.0) ? (a) : -(a))

static	int	search_mode(int w, int h, double aspect)
{
    int i;
    int fit = -1;
    int fit_w = 0, fit_h = 0;
    double fit_a = 0.0;

    for (i=0; sdl_mode[i]; i++) {
	/* 画面サイズに収まっていること */
	if (w <= sdl_mode[i]->w &&
	    h <= sdl_mode[i]->h) {

	    int tmp_w = sdl_mode[i]->w;
	    int tmp_h = sdl_mode[i]->h;
	    double tmp_a = FABS(((double)tmp_w / tmp_h) - aspect);

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

		} else {	/* 縦長モニター (なんて一般的なの?) の場合 */

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

/************************************************************************/

void	graph_exit(void)
{
    SDL_WM_GrabInput(SDL_GRAB_OFF);

    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}


/************************************************************************
 *	色の確保
 *	色の解放
 ************************************************************************/

void	graph_add_color(const PC88_PALETTE_T color[],
			int nr_color, unsigned long pixel[])
{
    int i;
    for (i=0; i<nr_color; i++) {
	pixel[i] = SDL_MapRGB(sdl_offscreen->format,
			      color[i].red, color[i].green, color[i].blue);
    }
}

/************************************************************************/

void	graph_remove_color(int nr_pixel, unsigned long pixel[])
{
    /* 色に関しては何も管理しないので、ここでもなにもしない */
}



/************************************************************************
 *	グラフィックの更新
 ************************************************************************/

void	graph_update(int nr_rect, T_GRAPH_RECT rect[])
{
    SDL_Rect srect[16], drect;
    int i;

    if (nr_rect > 16) {
	fprintf(stderr, "SDL: Maybe Update Failied...\n");
	nr_rect = 16;
    }

    for (i=0; i<nr_rect; i++) {
	srect[i].x = rect[i].x;
	srect[i].y = rect[i].y;
	srect[i].w = rect[i].width;
	srect[i].h = rect[i].height;

	drect = srect[i];

	if (SDL_BlitSurface(sdl_offscreen, &srect[i], sdl_display, &drect) <0) {
	    fprintf(stderr, "SDL: Unsuccessful blitting\n");
	}
    }

    if (sdl_display->flags & SDL_DOUBLEBUF) {

	SDL_Flip(sdl_display);

	for (i=0; i<nr_rect; i++) {
	    drect = srect[i];
	    SDL_BlitSurface(sdl_offscreen, &srect[i], sdl_display, &drect);
	}

    } else {

	SDL_UpdateRects(sdl_display, nr_rect, srect);
    }
}


/************************************************************************
 *	タイトルの設定
 *	属性の設定
 ************************************************************************/

void	graph_set_window_title(const char *title)
{
    SDL_WM_SetCaption(title, title);
}

/************************************************************************/

void	graph_set_attribute(int mouse_show, int grab, int keyrepeat_on)
{
    if (mouse_show) SDL_ShowCursor(SDL_ENABLE);
    else            SDL_ShowCursor(SDL_DISABLE);

    if (grab) SDL_WM_GrabInput(SDL_GRAB_ON);
    else      SDL_WM_GrabInput(SDL_GRAB_OFF);

    if (keyrepeat_on) SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,
					  SDL_DEFAULT_REPEAT_INTERVAL);
    else              SDL_EnableKeyRepeat(0, 0);

    sdl_mouse_rel_move = (mouse_show == FALSE && grab) ? TRUE : FALSE;

    /* SDL は、グラブ中かつマウスオフなら、ウインドウの端にマウスが
       ひっかかっても、マウス移動の相対量を検知できる。

       なので、この条件を sdl_mouse_rel_move にセットしておき、
       真なら、マウス移動は相対量、偽なら絶対位置とする (event.c)

       メニューでは、かならずグラブなし (マウスはあり or なし) なので、
       この条件にはかからず、常にウインドウの端でマウスは停止する。
    */
}

/*
  -videodrv directx について

  グラブあり、マウスありの場合、グラブされない・・・

  全画面で、グラブなし、マウスなしにすると、
  マウスが画面の端で停止してしまう。あたりまえだが…
  全画面の場合、グラブなしは意味があるのか？ マルチディスプレイで検証



  -videodrv dga について

  ウインドウでも全画面でも、全画面フラグが立っている。
  デフォルトで -hwsurface になっている。 -swsurface の指定は可能。
  -doublebuf を指定すると、 -hwsurface もセットで有効になる。

  全画面←→ウインドウを繰り返すとコアを吐く。

  -hwsurface では、マウスの表示に時々残骸が残る。
  -swsurface は問題さなげ。

  -doublebuf を指定すると、マウスは表示されなくなる。
*/
