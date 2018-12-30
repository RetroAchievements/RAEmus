/***********************************************************************
 * グラフィック処理 (システム依存)
 *
 *	詳細は、 graph.h 参照
 ************************************************************************/

/*----------------------------------------------------------------------*
 * Classicバージョンのソースコードの大部分は、                          *
 * Koichi NISHIDA 氏の Classic iP6 PC-6001/mk2/6601 emulator のソースを *
 * 参考にさせていただきました。                                         *
 *                                                   (c) Koichi NISHIDA *
 *----------------------------------------------------------------------*/

#include <stdio.h>

#include "quasi88.h"
#include "graph.h"
#include "device.h"


#ifdef	SUPPORT_DOUBLE
#error !
#endif


/********************** グローバル変数 **********************/

// main window & graphic world
WindowRef	macWin;
GWorldPtr	macGw;

QDGlobals	macQd;

int		mac_8bpp = TRUE;	/* 優先的に、256色モードで動作させる */


/************************************************************************/
/* 初期表示位置・・・らしい */
#define WIN_X 20
#define WIN_Y 60

/* これだけの色数を使う */
#define	ALL_COLORS	(24)

// default palette
static PaletteHandle defPalette;
static int	nr_color_used;

static void InitColor(void);


static	T_GRAPH_SPEC	graph_spec;		/* 基本情報		*/

static	int		graph_exist;		/* 真で、画面生成済み	*/
static	T_GRAPH_INFO	graph_info;		/* その時の、画面情報	*/


/************************************************************************
 *	CLASSICの初期化
 *	CLASSICの終了
 ************************************************************************/
static unsigned long displayOrigDepth;
static unsigned long displayDepth;
static unsigned long chkSysDepth(void);

void	mac_init(void)
{
    SysEnvRec sys;

    // initialize
    InitGraf((Ptr) &macQd.thePort);
    InitFonts();
    InitWindows();
    InitMenus();
    TEInit();
    InitDialogs(NULL);
    InitCursor();

    // color QD check
    SysEnvirons(1, &sys);
    if (! sys.hasColorQD) ExitToShell();

    // depth check
#ifdef	SUPPORT_16BPP
    if (mac_8bpp) displayDepth = 8;	/* 256色モード  から先にチェックする */
    else          displayDepth = 16;	/* 32000色モードから先にチェックする */
    if (! HasDepth(GetMainDevice(), displayDepth, 1, 1)) {
	if (displayDepth == 8) displayDepth = 16;
	else                   displayDepth = 8;
	if (! HasDepth(GetMainDevice(), displayDepth, 1, 1)) {
	    mac_error_dialog( "Sorry, 256 or 32000 Color mode only!" );
	    ExitToShell();
	}
    }
#else
    displayDepth = 8;
    if (! HasDepth(GetMainDevice(), displayDepth, 1, 1)) {
	mac_error_dialog( "Sorry, 256 Color mode only!" );
	ExitToShell();
    }
#endif

    // change depth
    displayOrigDepth = chkSysDepth();
    SetDepth(GetMainDevice(), displayDepth, 1, 1);

    // menu resource
#if 0
    {
	Handle menuBar;
	menuBar = GetNewMBar(128);
	if (! menuBar) ExitToShell();
	SetMenuBar(menuBar);
	DisposeHandle(menuBar);
	AppendResMenu(GetMenuHandle(128),'DRVR');
	DrawMenuBar();
    }
#else
	if (! mac_create_menubar()) ExitToShell();
#endif

    /*キー入力をRomanに*/
    KeyScript(smKeyRoman);
}

/************************************************************************/

void	mac_exit(void)
{
    /*キー入力を元に戻す*/
    KeyScript(smKeySwapScript);

    SetDepth(GetMainDevice(), displayOrigDepth, 1, 1);
    ExitToShell();
}

/*----------------------------------------------------------------------*/
// get depth of main device
static unsigned long chkSysDepth(void)
{
    GDHandle ghd;
    PixMapHandle phd;
    int depth = 1;
	
    ghd = GetMainDevice();
    phd = (*ghd)->gdPMap;
    if (!(depth = (*phd)->pixelSize))
	depth = 1;
    return depth;
}


/************************************************************************
 *	グラフィック処理の初期化
 *	グラフィック処理の動作
 *	グラフィック処理の終了
 ************************************************************************/

const T_GRAPH_SPEC	*graph_init(void)
{
    int	win_w, win_h;
    int	ful_w, ful_h;

    if (verbose_proc) {
	printf("Initializing Graphic System ... \n");
    }


    /* 画面サイズを取得する方法は? */
    win_w = 10000;
    win_h = 10000;
    ful_w = 640;
    ful_h = 480;

    graph_spec.window_max_width      = win_w;
    graph_spec.window_max_height     = win_h;
    graph_spec.fullscreen_max_width  = ful_w;
    graph_spec.fullscreen_max_height = ful_h;
    graph_spec.forbid_status         = FALSE;
    graph_spec.forbid_half           = FALSE;

    if (verbose_proc)
	printf("  INFO: %dbpp->%dbpp(%dbyte), Maxsize=win(%d,%d),full(%d,%d)\n",
	       (int)displayOrigDepth, (int)displayDepth, (int)displayDepth/8,
	       win_w, win_h, ful_w, ful_h);

    // to get key up event
    SetEventMask(everyEvent);

    /* パレットを初期化する */
    if (verbose_proc) printf("  Color Initialized\n");
    InitColor();

    return &graph_spec;
}

/************************************************************************/

static void createWindow(int width, int height);
static void toWindowMode(void);
static void toFullscreenMode(int width, int height);


const T_GRAPH_INFO	*graph_setup(int width, int height,
				     int fullscreen, double aspect)
{
    T_GRAPH_INFO *info = &graph_info;

    /* aspect は未使用 */

    if (verbose_proc){
	if (graph_exist) printf("Re-Initializing Graphic System ... \n");
	if (fullscreen) printf("  Trying full screen mode <%dx%d> ... ",
			       width, height);
	else            printf("  Opening window ... ");
    }

    if (graph_exist == FALSE) {			/* 初回 */
	createWindow(width, height);
	if (fullscreen) {
	    toFullscreenMode(width, height);
	}
    } else {					/* 2回目以降 */
	if (info->fullscreen) {
	    if (fullscreen) {				/* FULL → FULL */

		toWindowMode();
		createWindow(width, height);
		toFullscreenMode(width, height);

	    } else {					/* FULL → WIN */
		toWindowMode();
		/* ↓引数が直前の toFullscreenMode のと同じなら処理不要… */
		createWindow(width, height);
	    }

	} else {
	    if (fullscreen) {				/* WIN → FULL */

		/* ↓引数が直前の createWindow のと同じなら処理不要… */
		createWindow(width, height);
		toFullscreenMode(width, height);

	    } else {					/* WIN → WIN */
		createWindow(width, height);
	    }
	}
    }

    if (verbose_proc) printf("OK (%dx%d)\n", info->width, info->height);

    graph_exist = TRUE;

    info->byte_per_pixel = displayDepth / 8;
    info->nr_color	 = ALL_COLORS;
    info->write_only	 = FALSE;
    info->broken_mouse	 = (info->fullscreen) ? TRUE : FALSE;
    info->draw_start	 = NULL;
    info->draw_finish	 = NULL;
    info->dont_frameskip = FALSE;

    /* 処理の都度、使用済みの色数を初期値に戻す */
    nr_color_used = 1;
    return info;
}

/*======================================================================*/

static void createWindow(int width, int height)
{
    T_GRAPH_INFO *info = &graph_info;

    Rect rect;
    PixMapHandle pmh;	


    SetRect(&rect, WIN_X, WIN_Y, WIN_X + width, WIN_Y + height);
    // re-size window
    if (macWin) DisposeWindow(macWin);
    macWin = NewCWindow(NULL, &rect, "\p", true, documentProc, (WindowRef)-1L, false, 0);


    // re-create off screen
    if (macGw) {
	pmh = GetGWorldPixMap(macGw);
	UnlockPixels(pmh);
	DisposeGWorld(macGw);
    }
    SetRect(&rect, 0, 0, width, height);
    NewGWorld(&macGw, displayDepth, &rect, NULL, NULL, 0);
    pmh = GetGWorldPixMap(macGw);
    LockPixels(pmh);


    info->fullscreen    = FALSE;
    info->width         = width;
    info->height        = height;
    info->byte_per_line = ((**pmh).rowBytes & 0x3fff);
    info->buffer        = GetPixBaseAddr(pmh);

    //memset(info->buffer, 0x00, info->byte_per_line * info->height);

    if (displayDepth == 8) {
	PixMapHandle pixmap;
	CTabHandle ctab;
	GDHandle mainhd;
	RGBColor rgb;
	int i;

	NSetPalette(macWin, defPalette, pmAllUpdates);
	ActivatePalette(macWin);

	pixmap = GetGWorldPixMap(macGw);
	ctab = (*pixmap)->pmTable;
	mainhd = GetMainDevice();

	for (i=0; i<ALL_COLORS + 1; i++) {
	    GetEntryColor(defPalette, i, &rgb);
	    (*ctab)->ctTable[i].rgb = rgb;
	    (*ctab)->ctTable[i].value = i;
	}
	(*ctab)->ctSeed = (*(*(*mainhd)->gdPMap)->pmTable)->ctSeed;
    }
}


// for full screen
static Ptr screenRestore;
static WindowRef macWinRestore;


// change to window mode
static void toWindowMode(void)
{
    T_GRAPH_INFO *info = &graph_info;

    PixMapHandle pmh;

    if (displayDepth == 8) {
	macWin = macWinRestore;
    }

    EndFullScreen(screenRestore, 0);
    SetDepth(GetMainDevice(), displayDepth, 1, 1);	
    pmh = GetGWorldPixMap(macGw);

    info->fullscreen    = FALSE;
    /* info->width      = ;	toFullscreenMode で	*/
    /* info->height     = ;	設定したサイズになる	*/
    info->byte_per_line = ((**pmh).rowBytes & 0x3fff);
    info->buffer        = GetPixBaseAddr(pmh);

    if (displayDepth == 8) {
	NSetPalette(macWin, defPalette, pmAllUpdates);
	ActivatePalette(macWin);
    }
}

// change to full screen mode
static void toFullscreenMode(int width, int height)
{
    T_GRAPH_INFO *info = &graph_info;

    GDHandle mainDevice = GetMainDevice();
    PixMapHandle pmh;
    RGBColor black = {0, 0, 0};
    short screenWidth  = width;
    short screenHeight = height;


    if (displayDepth == 8) {
	macWinRestore = macWin;

	if (BeginFullScreen(&screenRestore, mainDevice, &screenWidth, &screenHeight,
			    &macWin, &black, fullScreenHideCursor) != noErr) return;
    } else {
	if (BeginFullScreen(&screenRestore, mainDevice, &screenWidth, &screenHeight,
			    NULL,    &black, fullScreenHideCursor) != noErr) return;
    }

    pmh = (**mainDevice).gdPMap;

    info->fullscreen    = TRUE;
    info->width         = screenWidth;
    info->height        = screenHeight;
    info->byte_per_line = ((**pmh).rowBytes & 0x3fff);
    info->buffer        = GetPixBaseAddr(pmh);

    //memset(info->buffer, 0x00, info->byte_per_line * info->height);

    if (displayDepth == 8) {
	NSetPalette(macWin, defPalette, pmAllUpdates);
	ActivatePalette(macWin);
    }
}


/************************************************************************/

void	graph_exit(void)
{
    if (graph_exist) {
	if (graph_info.fullscreen) toWindowMode();
	CloseWindow(macWin);
    }
}



int	mac_is_fullscreen(void)
{
    if (graph_exist) {
	if (graph_info.fullscreen) return TRUE;
    }
    return FALSE;
}



/************************************************************************
 *	色の確保
 *	色の解放
 ************************************************************************/

void	graph_add_color(const PC88_PALETTE_T color[],
			int nr_color, unsigned long pixel[])
{
    if (displayDepth == 8) {
	PixMapHandle pixmap;
	CTabHandle ctab;
	GDHandle mainhd;
	RGBColor rgb;
	int i;

	for (i=0; i<nr_color; i++) {
	    rgb.red   = (unsigned short)color[i].red   << 8;
	    rgb.green = (unsigned short)color[i].green << 8;
	    rgb.blue  = (unsigned short)color[i].blue  << 8;
	    SetEntryColor (defPalette, nr_color_used + i, &rgb);

	    pixel[i] = nr_color_used + i;
	}
	nr_color_used += nr_color;


	/* この処理は毎回必要なのか？ フルスクリーンでは不要なの？ */
	if (graph_exist &&
	    graph_info.fullscreen == FALSE) {

	    pixmap = GetGWorldPixMap(macGw);
	    ctab = (*pixmap)->pmTable;
	    mainhd = GetMainDevice();
		
	    for (i=0; i<ALL_COLORS + 1; i++) {
		GetEntryColor(defPalette, i, &rgb);
		(*ctab)->ctTable[i].rgb = rgb;
		(*ctab)->ctTable[i].value = i;
	    }
	    (*ctab)->ctSeed = (*(*(*mainhd)->gdPMap)->pmTable)->ctSeed;
	}

	/* これは必須のようだが、画面がちらつく・・・ */
	ActivatePalette(macWin);

    } else {
	int i;
	for (i=0; i<nr_color; i++) {
	    pixel[i] = ( ((unsigned short)(color[i].red   >> 3) << 10) |
			 ((unsigned short)(color[i].green >> 3) <<  5) |
			 ((unsigned short)(color[i].blue  >> 3)) );
	}
    }
}

/************************************************************************/

void	graph_remove_color(int nr_pixel, unsigned long pixel[])
{
    nr_color_used -= nr_pixel;
}

/*======================================================================*/

/* set up coltable, alind8? etc. */
static void InitColor(void)
{
    if (displayDepth == 8) {
	RGBColor rgb;
	int i;
	const int param[ALL_COLORS][3] = // {R,G,B}
	{
	    { 0x0000, 0x0000, 0x0000 },		/* 全色 黒で初期化 */
	};

	defPalette = NewPalette(ALL_COLORS + 1, NULL, pmTolerant + pmExplicit, 0);

	// 0 is preserved by Mac	
	rgb.red = rgb.green = rgb.blue = 255;
	SetEntryColor(defPalette, 0, &rgb);		

	/* とりあえず、デフォルトの色を定義 */
	for (i=0; i<ALL_COLORS; i++) {
	    rgb.red   = param[i][0];
	    rgb.green = param[i][1];
	    rgb.blue  = param[i][2];
	    SetEntryColor(defPalette, i + 1, &rgb);
	}
    }
}

/************************************************************************
 *	グラフィックの更新
 ************************************************************************/

static void draw(int x0, int y0, int x1, int y1);

void	graph_update(int nr_rect, T_GRAPH_RECT rect[])
{
    int i;

    if (graph_info.fullscreen == FALSE) {
	for (i=0; i<nr_rect; i++) {
	    draw(rect[i].x,
		 rect[i].y,
		 rect[i].x + rect[i].width,
		 rect[i].y + rect[i].height);
	}
    }
}

/*======================================================================*/
// draw
static void draw(int x0, int y0, int x1, int y1)
{
    CGrafPtr port;
    GWorldPtr gptr;
    GDHandle ghd;
    Rect rect;

    // blit !
    GetGWorld(&gptr, &ghd);
    port = GetWindowPort(macWin);
    SetGWorld(port, NULL);
    SetRect(&rect, x0, y0, x1, y1);
    //SetRect(&rect, 0, 0, graph_info.width, graph_info.height);
    CopyBits(&((GrafPtr)macGw)->portBits, &(macWin->portBits), &rect, &rect,
	     srcCopy, NULL);
    SetGWorld(gptr, ghd);
}



void	mac_draw_immidiate(void)
{
    if (graph_info.fullscreen == FALSE) {
	draw(0, 0, graph_info.width, graph_info.height);
    }
}



/************************************************************************
 *	タイトルの設定
 *	属性の設定
 ************************************************************************/
// Window title strings
static Str255 wtitle;

void	graph_set_window_title(const char *title)
{
    wtitle[0] = strlen(title);
    memcpy(wtitle+1, title, wtitle[0]);

    if (macWin) {
	SetWTitle(macWin, wtitle);
    }
}

/************************************************************************/

void	graph_set_attribute(int mouse_show, int grab, int keyrepeat_on)
{
    /* 設定の仕方がわからない… */
    (void)mouse_show;
    (void)grab;
    (void)keyrepeat_on;
}
