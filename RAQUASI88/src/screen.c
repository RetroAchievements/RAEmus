/************************************************************************/
/*                                  */
/* 画面の表示                              */
/*                                  */
/************************************************************************/

#include <string.h>

#include "quasi88.h"
#include "initval.h"
#include "screen.h"
#include "screen-func.h"
#include "graph.h"

#include "crtcdmac.h"
#include "pc88main.h"

#include "status.h"
#include "suspend.h"

#include "intr.h"
#include "q8tk.h"

#include "pause.h"          /* pause_event_focus_in_when_pause() */



PC88_PALETTE_T  vram_bg_palette;    /* OUT[52/54-5B]        */
PC88_PALETTE_T  vram_palette[8];    /*      各種パレット  */

byte    sys_ctrl;           /* OUT[30] SystemCtrl       */
byte    grph_ctrl;          /* OUT[31] GraphCtrl        */
byte    grph_pile;          /* OUT[53] 重ね合わせ      */



char    screen_dirty_flag[ 0x4000*2 ];      /* メイン領域 差分更新 */
int screen_dirty_all = TRUE;        /* メイン領域 全域更新 */
int screen_dirty_palette = TRUE;        /* 色情報 更新     */
int screen_dirty_status = FALSE;        /* ステータス領域 更新 */
int screen_dirty_status_hide = FALSE;   /* ステータス領域 消去 */
int screen_dirty_status_show = FALSE;   /* ステータス領域 初期化*/
int screen_dirty_frame = TRUE;      /* 全領域 更新         */



int frameskip_rate  = DEFAULT_FRAMESKIP;    /* 画面表示の更新間隔  */
int monitor_analog  = TRUE;         /* アナログモニター     */
int use_auto_skip   = TRUE;         /* 自動フレームスキップ   */


static  int do_skip_draw = FALSE;       /* 今回スキップするか? */
static  int already_skip_draw = FALSE;  /* 前回スキップしたか? */

static  int skip_counter = 0;   /* 連続何回スキップしたか    */
static  int skip_count_max = 15;    /* これ以上連続スキップしたら
                        一旦、強制的に描画する   */

static  int frame_counter = 0;  /* フレームスキップ用のカウンタ   */



static  int blink_ctrl_cycle   = 1; /* カーソル表示用のカウンタ */
static  int blink_ctrl_counter = 0; /*              〃     */





/*CFG*/ int hide_mouse = FALSE; /* マウスを隠すかどうか       */
/*CFG*/ int grab_mouse = FALSE; /* グラブするかどうか      */

/*CFG*/ int use_swcursor = FALSE;   /* メニュー専用カーソル表示有無   */
    int now_swcursor;       /* 現在専用カーソル表示中?   */




/*CFG*/ int use_interlace = 0;      /* インターレース表示  */

static  int enable_half_interp = FALSE; /* HALF時、色補間可能か否か */
/*CFG*/ int use_half_interp = TRUE;     /* HALF時、色補間する        */
static  int now_half_interp = FALSE;    /* 現在、色補完中なら真       */



typedef struct{             /* 画面サイズのリスト      */
    int     w,  h;
} SCREEN_SIZE_TABLE;

static const SCREEN_SIZE_TABLE screen_size_tbl[ SCREEN_SIZE_END ] =
{
    /*  w     h   */
    {  320,  200, },        /* SCREEN_SIZE_HALF 320x200 */
    {  640,  400, },        /* SCREEN_SIZE_FULL 640x400 */
#ifdef  SUPPORT_DOUBLE
    { 1280,  800, },        /* SCREEN_SIZE_DOUBLE   1280x800*/
#endif
};


static  int screen_size_max = SCREEN_SIZE_END - 1; /*変更可能な最大サイズ*/
static  int screen_size_min = SCREEN_SIZE_HALF;    /*変更可能な最小サイズ*/
/*CFG*/ int screen_size = SCREEN_SIZE_FULL;    /*画面サイズ指定      */
/*CFG*/ int now_screen_size;               /*実際の、画面サイズ  */


static  int enable_fullscreen = 0;  /* 全画面表示可能かどうか    */
/*CFG*/ int use_fullscreen = FALSE; /* 全画面表示指定        */
static  int now_fullscreen = FALSE; /* 現在、全画面表示中なら真 */


/*CFG*/ double  mon_aspect = 0.0;   /* モニターのアスペクト比    */


int WIDTH  = 0;         /* 描画バッファ横サイズ       */
int HEIGHT = 0;         /* 描画バッファ縦サイズ       */
int DEPTH  = 8;         /* 色ビット数  (8 or 16 or 32) */
int SCREEN_W = 0;           /* 画面横サイズ (320/640/1280)    */
int SCREEN_H = 0;           /* 画面縦サイズ (200/400/800) */

int SCREEN_DX = 0;          /* ウインドウ左上と、      */
int SCREEN_DY = 0;          /* 画面エリア左上とのオフセット   */

char    *screen_buf;            /* 描画バッファ先頭     */
char    *screen_start;          /* 画面先頭         */

static  int screen_bx;      /* ボーダー(枠)の幅 x (ドット)    */
static  int screen_by;      /*     〃      y (ドット)   */

double screen_scale_x = 0.0;  /* 画面のスケール因子 */
double screen_scale_y = 0.0;
int screen_scale_dx = 0; /* 画面のスケール後のオフセット */
int screen_scale_dy = 0;



/*CFG*/ int status_fg = 0x000000;   /* ステータス前景色     */
/*CFG*/ int status_bg = 0xd6d6d6;   /* ステータス背景色     */

static  int enable_status = TRUE;   /* ステータス表示可能かどうか  */
/*CFG*/ int show_status = TRUE; /* ステータス表示有無      */
static  int now_status = FALSE; /* 現在、ステータス表示中なら真   */

char    *status_buf;            /* ステータス全域 先頭     */
char    *status_start[3];       /* ステータス描画 先頭     */
int status_sx[3];           /* ステータス描画サイズ       */
int status_sy[3];


Ulong   color_pixel[16];            /* 色コード     */
Ulong   color_half_pixel[16][16];       /* 色補完時の色コード  */
Ulong   black_pixel;                /* 黒の色コード       */
Ulong   status_pixel[ STATUS_COLOR_END ];   /* ステータスの色コード   */



static  int screen_write_only;  /* 画面バッファ読出不可なら、真   */

static  void    (*draw_start)(void);    /* 描画前のコールバック関数 */
static  void    (*draw_finish)(void);   /* 描画後のコールバック関数 */
static  int dont_frameskip;     /* フレームスキップ禁止なら、真   */



static  int drawn_count;        /* fps計算用の描画回数カウンタ  */

static  int broken_mouse;       /* システムのマウス表示が異常  */
static  int auto_mouse;     /* 真で、マウスを自動で隠すモード   */
static  int auto_mouse_timer;   /* 自動で隠すまでの残り時間タイマー */
static  int auto_grab;      /* 真で、マウス自動グラブモード     */


enum {
    SETUP_START,
    SETUP_MOVE,
    SETUP_TIMEUP,
    SETUP_CLICK
};
static  void    screen_attr_setup(int stat);

static  void    check_half_interp(void);
static  void    trans_palette(PC88_PALETTE_T syspal[]);
static  void    set_vram2screen_list(void);
static  void    clear_all_screen(void);
static  void    put_image_all(void);

/***********************************************************************
 * 画面処理の初期化・終了
 ************************************************************************/
static  const T_GRAPH_SPEC *spec;

static  int open_window(void);
static  void    open_window_or_exit(void);

int screen_init(void)
{
    int i;
    int w, h, max, min;

    status_init();          /* ステータス表示のワーク初期化   */

    spec = graph_init();
    if (spec == NULL) {
    return FALSE;
    }


    /* spec によって、ウインドウの最大・最小サイズを決定 */

    min = -1;
    for (i = 0; i < SCREEN_SIZE_END; i++) {
    if (i == SCREEN_SIZE_HALF && spec->forbid_half) { continue; }

    if (spec->window_max_width  >= screen_size_tbl[ i ].w &&
        spec->window_max_height >= screen_size_tbl[ i ].h) {
        min = i;
        break;
    }
    }
    max = -1;
    for (i = SCREEN_SIZE_END - 1; i >= 0; i--) {
    if (spec->window_max_width  >= screen_size_tbl[ i ].w &&
        spec->window_max_height >= screen_size_tbl[ i ].h) {
        max = i;
        break;
    }
    }
    if (min < 0 || max < 0 || max < min) {
    if (verbose_proc) printf("  Not found drawable window size (bug?)\n");
    return FALSE;
    }
    screen_size_max = max;
    screen_size_min = min;


    /* spec によって、全画面可能かどうかを決定 */

    if (spec->forbid_half) { i = SCREEN_SIZE_FULL; }
    else                   { i = SCREEN_SIZE_HALF; }

    if (spec->fullscreen_max_width  >= screen_size_tbl[ i ].w &&
    spec->fullscreen_max_height >= screen_size_tbl[ i ].h) {
    enable_fullscreen = TRUE;
    } else {
    enable_fullscreen = FALSE;
    }



    /* screen_size, WIDTH, HEIGHT にコマンドラインで指定したウインドウサイズが
       セット済みなので、それをもとにボーダー(枠)のサイズを算出する */

    w = screen_size_tbl[ screen_size ].w;
    h = screen_size_tbl[ screen_size ].h;

    screen_bx = ((MAX(WIDTH,  w) - w) / 2) & ~7;    /* 8の倍数 */
    screen_by = ((MAX(HEIGHT, h) - h) / 2);

    if (open_window()) {
    clear_all_screen();
    put_image_all();
    return TRUE;
    } else {
    return FALSE;
    }
}


void    screen_exit(void)
{
    graph_exit();
}



/*----------------------------------------------------------------------
 * ウインドウの生成
 *----------------------------------------------------------------------*/
static int added_color;
static unsigned long added_pixel[120+16];

#define SET_N_COLOR(n, rgb)                 \
        color[ n ].red   = ((rgb) >> 16) & 0xff;    \
        color[ n ].green = ((rgb) >>  8) & 0xff;    \
        color[ n ].blue  = ((rgb)      ) & 0xff;    \

/*
  以下の変数に基づき、画面を生成し、以下の変数をセットする
    use_fullscreen, enable_fullscreen       → now_fullscreen
    screen_size   , screen_size_max, screen_size_min
    show_status                 → now_status
    screen_bx     , screen_by
 */

static  int open_window(void)
{
    int i, size, found = FALSE;
    int w = 0, h = 0, status_displayable = FALSE;
    const T_GRAPH_INFO *info;


    added_color = 0;

    if (enable_fullscreen == FALSE) {   /* 全画面不可なら、全画面指示は却下 */
    use_fullscreen = FALSE;
    }


    /* フルスクリーン表示が可能なサイズを、大きいほうから探し出す */

    if (use_fullscreen) {
    for (size = screen_size; size >= screen_size_min; size--) {

        for (i = 0; i < 3; i++) {   /* 3パターンのサイズにて確認 */

        w = screen_size_tbl[ size ].w;
        h = screen_size_tbl[ size ].h;

        switch (i) {
        case 0:     /* 指定サイズ (ステータス不可かも) */
            if (screen_bx == 0 && screen_by == 0) { continue; }
            w += screen_bx * 2;
            h += screen_by * 2;
            status_displayable = FALSE;
            break;

        case 1:     /* 最適サイズ + ステータス */
            if (spec->forbid_status) { continue; }
            w += (0) * 2;
            h += STATUS_HEIGHT * 2;
            status_displayable = TRUE;
            break;

        case 2:     /* 最適サイズ (ステータス不可) */
            status_displayable = FALSE;
            break;
        }

        if (w <= spec->fullscreen_max_width &&
            h <= spec->fullscreen_max_height) {
            found = TRUE;
            break;
        }
        }
        if (found) break;
    }

    if (found == FALSE) {       /* 表示可能サイズ無しなら全画面不可 */
        use_fullscreen = FALSE;
    }
    }


    /* ウインドウ表示が表示な可能サイズを、大きいほうから探し出す */

    if (use_fullscreen == FALSE) {
    for (size = screen_size; size >= screen_size_min; size--) {

        for (i = 0; i < 4; i++) {   /* 4パターンのサイズにて確認 */

        w = screen_size_tbl[ size ].w;
        h = screen_size_tbl[ size ].h;

        switch (i) {
        case 0:     /* 指定サイズ + ステータス */
            if (screen_bx == 0 && screen_by == 0) { continue; }
            w += screen_bx * 2;
            h += screen_by * 2 + STATUS_HEIGHT;
            status_displayable = TRUE;
            break;

        case 1:     /* 指定サイズ (ステータス不可) */
            if (screen_bx == 0 && screen_by == 0) { continue; }
            w += screen_bx * 2;
            h += screen_by * 2;
            status_displayable = FALSE;
            break;

        case 2:     /* 最適サイズ + ステータス */
            w += 0;
            h += STATUS_HEIGHT;
            status_displayable = TRUE;
            break;

        case 3:     /* 最適サイズ (ステータス不可) */
            status_displayable = FALSE;
            break;
        }

        if (w * (mon_aspect ? mon_aspect : 1) <= spec->window_max_width &&
            h <= spec->window_max_height) {
            found = TRUE;
            break;
        }
        }
        if (found) break;
    }

    /* ステータス表示が可能なサイズが見つかった場合でも、
       ステータス表示しないならば、その分のサイズを減らしておく */
    if (status_displayable) {
        if (spec->forbid_status || show_status == FALSE) {
        h -= STATUS_HEIGHT;
        }
    }
    }


    /* サイズが決まったので、いよいよ画面を生成する */

    if (found == FALSE) {   /* これはありえないハズだが、念のため… */
    size = screen_size_min;
    w = screen_size_tbl[ size ].w;
    h = screen_size_tbl[ size ].h;
    status_displayable = FALSE;
    }
    now_screen_size = size;
    info = graph_setup(w, h, use_fullscreen, (float)mon_aspect);

    if (info) {

    screen_scale_x = info->scaled_width / info->width;
    screen_scale_y = info->scaled_height / info->height;
    screen_scale_dx = info->scaled_offx;
    screen_scale_dy = info->scaled_offy + (info->fullscreen && status_displayable ? STATUS_HEIGHT : 0);

    /* フルスクリーンで、ステータス表示が可能なサイズが確保できたか確認 */
    if ((info->fullscreen) &&
        (info->height >= screen_size_tbl[ size ].h + STATUS_HEIGHT * 2)) {
        status_displayable = TRUE;
    }

    /* 本当にステータス表示が可能かどうかを、最終判断 */
    if (status_displayable && (spec->forbid_status == FALSE)) {
        enable_status = TRUE;
        /* show_status は現在値のまま */
    } else {
        enable_status = FALSE;
        show_status   = FALSE;
    }

    /* サイズの諸言を計算 */
    WIDTH      = info->byte_per_line / info->byte_per_pixel;
    if (info->fullscreen) {
        HEIGHT = info->height;
    } else {
        HEIGHT = info->height - ((show_status) ? STATUS_HEIGHT : 0);
    }

    SCREEN_W  = screen_size_tbl[ size ].w;
    SCREEN_H  = screen_size_tbl[ size ].h;
    SCREEN_DX = (info->width  - SCREEN_W) / 2;
    SCREEN_DY = (HEIGHT       - SCREEN_H) / 2;

    if (info->fullscreen) {
        if (enable_status) {
        HEIGHT -= STATUS_HEIGHT;
        }
        now_fullscreen = TRUE;
    } else {
        now_fullscreen = FALSE;
    }

    DEPTH     = info->byte_per_pixel * 8;


    /* 使える色の数をチェック */
    if        (info->nr_color >= 144) { /* いっぱい使える */
        enable_half_interp = TRUE;

    } else if (info->nr_color >= 24) {  /* 半分モードの色補間はだめ */
        enable_half_interp = FALSE;

    } else if (info->nr_color >= 16) {  /* ステータス表示もままならん*/
        enable_half_interp = FALSE;

    } else {                /* ぜんぜん色が足りない */
        return FALSE;
    }

    /* HALFサイズ時の色補完有無を設定 */
    check_half_interp();


    /* スクリーンバッファの、描画開始位置を設定 */
    screen_buf = (char *)info->buffer;
    screen_start = &screen_buf[ (WIDTH*SCREEN_DY + SCREEN_DX)
                        * info->byte_per_pixel ];

    /* ステータス用のバッファなどを算出 */
    status_sx[0] = info->width / 5;
    status_sx[1] = info->width - status_sx[0]*2;
    status_sx[2] = info->width / 5;

    status_sy[0] = 
    status_sy[1] = 
    status_sy[2] = STATUS_HEIGHT - 2;

    status_buf = &screen_buf[ WIDTH * HEIGHT * info->byte_per_pixel ];

    /* ステータスの描画開始位置は、バッファの 2ライン下 */
    status_start[0]= status_buf + 2*(WIDTH * info->byte_per_pixel);
    status_start[1]= status_start[0] + (status_sx[0]*info->byte_per_pixel);
    status_start[2]= status_start[1] + (status_sx[1]*info->byte_per_pixel);


    /* ステータス用の色ピクセルを定義 */
    if (info->nr_color >= 24) {
        PC88_PALETTE_T color[8];
        unsigned long pixel[8];

        SET_N_COLOR( STATUS_BG,    status_bg);
        SET_N_COLOR( STATUS_FG,    status_fg);
        SET_N_COLOR( STATUS_BLACK, 0x000000 );
        SET_N_COLOR( STATUS_WHITE, 0xffffff );
        SET_N_COLOR( STATUS_RED,   0xff0000 );
        SET_N_COLOR( STATUS_GREEN, 0x00ff00 );
        SET_N_COLOR( 6, 0x000000 );
        SET_N_COLOR( 7, 0xffffff );

        graph_add_color(color, 8, pixel);

        status_pixel[STATUS_BG   ] = pixel[0];
        status_pixel[STATUS_FG   ] = pixel[1];
        status_pixel[STATUS_BLACK] = pixel[2];
        status_pixel[STATUS_WHITE] = pixel[3];
        status_pixel[STATUS_RED  ] = pixel[4];
        status_pixel[STATUS_GREEN] = pixel[5];
        black_pixel                = pixel[6];

    } else {
        PC88_PALETTE_T syspal[16];
        unsigned long black, white;

        /* syspal[0]〜[14] は黒、 syspal[15] は白 */
        memset(&syspal[0],  0x00, sizeof(PC88_PALETTE_T) * 15);
        memset(&syspal[15], 0xff, sizeof(PC88_PALETTE_T));

        /* ステータス用の色が確保できないので、とりあえず適当に色を確保 */
        /* syspal[8] はテキストの黒、 syspal[15] はテキストの白を想定   */
        trans_palette(syspal);

        black = color_pixel[ 8];
        white = color_pixel[15];

        status_pixel[STATUS_BG   ] = black;
        status_pixel[STATUS_FG   ] = white;
        status_pixel[STATUS_BLACK] = black;
        status_pixel[STATUS_WHITE] = white;
        status_pixel[STATUS_RED  ] = white;
        status_pixel[STATUS_GREEN] = white;

        black_pixel                = black;
    }

    now_status = show_status;

    screen_write_only = info->write_only;

    draw_start  = info->draw_start;
    draw_finish = info->draw_finish;
    dont_frameskip = info->dont_frameskip;


    /* 転送・表示関数のリストを設定 */
    set_vram2screen_list();


    /* システムのマウス表示状態 */
    broken_mouse = info->broken_mouse;

    screen_switch();

    /* ステータスの設定 */
    status_setup(now_status);

    return TRUE;
    } else {
    return FALSE;
    }
}

/*
 * ウインドウの再生成。失敗したら exit
 */
static  void    open_window_or_exit(void)
{
    if (open_window() == FALSE) {
    fprintf(stderr,"Sorry : Graphic System Fatal Error !!!\n");

    quasi88_exit(1);
    }
}



/*----------------------------------------------------------------------
 * HALFサイズ色補間の有無ワークを設定
 *----------------------------------------------------------------------*/
static  void    check_half_interp(void)
{
    if (now_screen_size == SCREEN_SIZE_HALF &&  /* 現在 HALFサイズで    */
    enable_half_interp  &&          /* フィルタリング可能で   */
    use_half_interp) {          /* 色補完してありなら   */

    now_half_interp = TRUE;

    } else {
    now_half_interp = FALSE;
    }
}







/***********************************************************************
 * モード切り替え時の、各種再設定
 ************************************************************************/
void    screen_switch(void)
{
    char *title;

    /* 全エリア強制描画の準備 */

    screen_set_dirty_frame();       /* 全領域 初期化(==更新) */
    screen_set_dirty_palette();     /* 色情報 初期化(==更新) */

    if (now_status) {           /* ステータス表示 */

    screen_set_dirty_status_show();     /* ステータス領域 初期化 */

    } else {                /* ステータス非表示 */

    if (now_fullscreen && enable_status) {  /* 全画面なら */
        screen_set_dirty_status_hide(); /* ステータス領域 消去 */
    }                   /* ウインドウなら処理なし */

    }

    frameskip_counter_reset();      /* 次回描画 */


    /* マウス、キーリピートの設定 */

    screen_attr_setup(SETUP_START);


    /* システムのマウス表示が異常、またはメニュー専用カーソル使用の場合 */

    if ((broken_mouse) ||
    (! quasi88_is_exec() && /*now_fullscreen &&*/ use_swcursor)) {
    now_swcursor = TRUE;
    } else {
    now_swcursor = FALSE;
    }


    /* ウインドウタイトルを表示 */

    if      (quasi88_is_exec())    title = Q_TITLE " ver " Q_VERSION;
    else if (quasi88_is_menu())    title = Q_TITLE " ver " Q_VERSION;
    else if (quasi88_is_pause())   title = Q_TITLE " (PAUSE)";
    else if (quasi88_is_monitor()) title = Q_TITLE " (MONITOR)";
    else                           title = "";

    graph_set_window_title(title);


    /*
      SDLメモ

      フルスクリーンの問題点 (Winにて発生)
      ダブルバッファの場合、マウスが正常に表示されない？
      シングルバッファでハードウェアサーフェスの場合、
      マウスをONにした瞬間、マウスの表示すべき位置にゴミが残る？

      ↓
      メニュー画面遷移だけの問題なので、ソフトウェアカーソルでごまかそう
    */
}

/*-----------------------------------------------------------------------------
  マウス表示、グラブ、キーリピートを設定する。

            EMU     MENU/PAUSE
    マウス表示 ※ 設定による する
    グラブ       ※ 設定による しない
    キーリピート  オフ      オン

  ※ EMU における、マウス表示・グラブは以下のようになる。

    A … グラブしない／マウス表示する
    B … グラブしない／マウス表示しない
    C … グラブする    ／マウス表示しない

    D … グラブしない／マウス表示は、自動で判断

    ウインドウ時
                |マウス表示する|マウス表示しない||マウス自動表示
    ------------+--------------+----------------++--------------
    グラブしない|       A      |        B       ||       D
    ------------+--------------+----------------++--------------
    グラブする  |       C   † |        C       ||       C

    † グラブする／マウス表示する の組合せは無効
       ウインドウ端でマウスが停止するのだが、あまり役に立ちそうにない。

    フルスクリーン時
                |マウス表示する|マウス表示しない||マウス自動表示
    ------------+--------------+----------------++--------------
    グラブしない|       A   ‡ |        C    § ||       C
    ------------+--------------+----------------++--------------
    グラブする  |       C   † |        C       ||       C

    † グラブする／マウス表示する の組合せは無効
       画面端でマウスが停止するのだが、あまり役に立ちそうにない。
    ‡ グラブしない／マウス表示する の組合せは、有効
       画面端でマウスが停止するのだが、Win+SDL でマルチディスプレイの場合
       マウスは停止せずに隣のディスプレイに移動することがあるらしい。
    § グラブしない／マウス表示しない の組合せは無効
       画面端でマウスが停止するが、マウスが表示されないので、わかりにくい

-----------------------------------------------------------------------------*/

/*#define DEBUG_ALL_MOUSE_PATTERN*/  /* デバッグ(全マウス設定の組合せを検証) */

#ifdef  DEBUG_ALL_MOUSE_PATTERN
int screen_attr_mouse_debug(void) { return TRUE; }
#else
int screen_attr_mouse_debug(void) { return FALSE; }
#endif


#define AUTO_MOUSE_TIMEOUT  (2 * 60)

/* マウスが動いたら呼び出される */
void    screen_attr_mouse_move(void)
{
    if (auto_mouse) {
    if (auto_mouse_timer == 0) {
        screen_attr_setup(SETUP_MOVE);
    } else {
        auto_mouse_timer = AUTO_MOUSE_TIMEOUT;
    }
    }
}

/* マウスのクリック時に呼び出される */
void    screen_attr_mouse_click(void)
{
    if (auto_grab) {
    screen_attr_setup(SETUP_CLICK);
    auto_grab = FALSE;
    }
}

/* 1/60sec毎に呼び出される */
static  void    screen_attr_update(void)
{
    if (auto_mouse) {
    if (auto_mouse_timer > 0) {
        if (--auto_mouse_timer == 0) {
        screen_attr_setup(SETUP_TIMEUP);
        }
    }
    }
}

/* マウス表示、グラブ、キーリピートを設定する */
static  void    screen_attr_setup(int stat)
{
    int repeat;     /* オートリピートの有無   */
    int mouse;      /* マウス表示の有無 */
    int grab;       /* グラブの有無       */

    if (stat == SETUP_START) {
    auto_mouse = FALSE;
    auto_mouse_timer = 0;
    auto_grab  = FALSE;
    }


    if (quasi88_is_exec()) {

    repeat = FALSE;

#ifdef  DEBUG_ALL_MOUSE_PATTERN /* デバッグ用:全てのマウス設定の組合せを検証 */
    if      (grab_mouse == UNGRAB_MOUSE){ grab = FALSE; }
    else if (grab_mouse == GRAB_MOUSE)  { grab = TRUE;  }
    else {
        if      (stat == SETUP_START)   { grab = FALSE; auto_grab = TRUE; }
        else if (stat == SETUP_CLICK)   { grab = TRUE;  auto_grab = FALSE;}
        else {
        if (auto_grab)              { grab = FALSE; }
        else                        { grab = TRUE;  }
        }
    }
    goto DEBUG;
#endif

    if (now_fullscreen) {   /* 全画面表示の場合 -------- */

        if (grab_mouse == UNGRAB_MOUSE &&
        hide_mouse == SHOW_MOUSE) {

        /* グラブなし && マウスあり の場合のみ、その通りにする */
        mouse = TRUE;
        grab  = FALSE;

        } else {

        /* グラブありor自動 || マウスなしor自動 ならば、以下で固定 */
        mouse = FALSE;
        grab  = TRUE;

        }

    } else {        /* ウインドウ表示の場合 -------- */

        if (grab_mouse == GRAB_MOUSE) {

        /* グラブありなら、マウスは消す */
        grab  = TRUE;
        mouse = FALSE;

        } else {

        if (grab_mouse == AUTO_MOUSE &&
            stat == SETUP_CLICK) {

            /* 自動グラブで、ボタンクリック時は、マウス消す */
            grab  = TRUE;
            mouse = FALSE;

            /* 以下はクリア */
            auto_mouse = FALSE;
            auto_mouse_timer = 0;
            auto_grab  = FALSE;

        } else {

            /* グラブなしなら、マウスの有無は設定による */
            if (grab_mouse == AUTO_MOUSE) {
            auto_grab = TRUE;
            }

            grab  = FALSE;

DEBUG:
            switch (hide_mouse) {
            case AUTO_MOUSE:
            auto_mouse = TRUE;
            if (stat == SETUP_START ||
                stat == SETUP_MOVE) {
                auto_mouse_timer = AUTO_MOUSE_TIMEOUT;
                mouse = TRUE;
            } else {
                auto_mouse_timer = 0;
                mouse = FALSE;
            }
            break;

            case SHOW_MOUSE:
            mouse = TRUE;
            break;

            case HIDE_MOUSE:
            default:
            mouse = FALSE;
            break;
            }
        }
        }
    }

    } else {

    repeat = TRUE;
    mouse  = TRUE;
    grab   = FALSE;

    /* 全画面モードで、ソフトウェアカーソルを使うなら、マウスは消す */
    if (/*now_fullscreen &&*/ use_swcursor) mouse = FALSE;
    }

    graph_set_attribute(mouse, grab, repeat);
}






/***********************************************************************
 * HALFサイズ時の色補完の有効・無効関連の関数
 ***********************************************************************/

/* 色補完の可否を返す */
int quasi88_cfg_can_interp(void) { return enable_half_interp; }

/* 色補完の現在状態を返す */
int quasi88_cfg_now_interp(void) { return use_half_interp; }

/* 色補完の有無を設定する */
void    quasi88_cfg_set_interp(int enable)
{
    use_half_interp = enable;
    check_half_interp();
    set_vram2screen_list();
    screen_set_dirty_all();     /* メイン領域 初期化(==更新) */
    frameskip_counter_reset();      /* 次回描画 */
}



/***********************************************************************
 * INTERLACEの設定関連の関数
 ***********************************************************************/

/* 現在のインタレース状態を返す */
int quasi88_cfg_now_interlace(void) { return use_interlace; }

/* インタレース状態を設定する */
void    quasi88_cfg_set_interlace(int interlace_mode)
{
    use_interlace = interlace_mode;
    set_vram2screen_list();
    screen_set_dirty_frame();       /* 全領域 初期化(==更新) */
    frameskip_counter_reset();      /* 次回描画 */
}



/***********************************************************************
 * ステータス表示設定関連の関数
 ***********************************************************************/

/* ステータス表示の可否を返す */
int quasi88_cfg_can_showstatus(void) { return enable_status; }

/* ステータス表示の現在状態を返す */
int quasi88_cfg_now_showstatus(void) { return now_status; }

/* ステータスの表示を設定する */
void    quasi88_cfg_set_showstatus(int show)
{
    if (now_status != show) {       /* ステータス表示有無が変わった */
    show_status = show;

    open_window_or_exit();          /* 画面サイズ切替     */

    status_setup(now_status);       /* ステータス変数等初期化 */
    }
}



/***********************************************************************
 * 全画面設定・画面サイズ設定関連の関数
 ***********************************************************************/

/* 全画面の可否を返す */
int quasi88_cfg_can_fullscreen(void) { return enable_fullscreen; }

/* 全画面の現在状態を返す */
int quasi88_cfg_now_fullscreen(void) { return now_fullscreen; }

/* 全画面の切替を設定する */
void    quasi88_cfg_set_fullscreen(int fullscreen)
{
    use_fullscreen = fullscreen;

    if (now_fullscreen != use_fullscreen) {
    open_window_or_exit();          /* 画面サイズ切替     */
    }
}

/* 画面サイズの最大を返す */
int quasi88_cfg_max_size(void) { return screen_size_max; }

/* 画面サイズの最小を返す */
int quasi88_cfg_min_size(void) { return screen_size_min; }

/* 画面サイズの現在状態を返す */
int quasi88_cfg_now_size(void) { return now_screen_size; }

/* 画面サイズを設定する */
void    quasi88_cfg_set_size(int new_size)
{
    screen_size = new_size;

    if (now_screen_size != screen_size) {
    open_window_or_exit();          /* 画面サイズ切替     */
    }
}

/* 画面サイズを大きくする */
void    quasi88_cfg_set_size_large(void)
{
    if (++screen_size > screen_size_max) screen_size = screen_size_min;

    open_window_or_exit();          /* 画面サイズ切替     */
}

/* 画面サイズを小さくする */
void    quasi88_cfg_set_size_small(void)
{
    if (--screen_size < screen_size_min) screen_size = screen_size_max;

    open_window_or_exit();          /* 画面サイズ切替     */
}













/*----------------------------------------------------------------------
 * パレット設定
 *----------------------------------------------------------------------*/
static  void    trans_palette(PC88_PALETTE_T syspal[])
{
    PC88_PALETTE_T color[120+16];
    int i, j, num;

    if (added_color) {
    graph_remove_color(added_color, added_pixel);
    added_color = 0;
    }

        /* 88のパレット16色分を設定 */

    for (i = 0; i < 16; i++) {
    color[i].red   = syspal[i].red;
    color[i].green = syspal[i].green;
    color[i].blue  = syspal[i].blue;
    }
    num = 16;

    /* HALFサイズフィルタリング可能時はフィルタパレット値を計算 */
    /* (フィルタリング用には、120色を設定) */

    if (now_half_interp) {
    for (i = 0; i < 16; i++) {
        for (j = i+1; j < 16; j++) {
           color[num].red   = (color[i].red  >>1) + (color[j].red  >>1);
           color[num].green = (color[i].green>>1) + (color[j].green>>1);
           color[num].blue  = (color[i].blue >>1) + (color[j].blue >>1);
           num++;
        }
    }
    }

    graph_add_color(color, num, added_pixel);
    added_color = num;

    for (i = 0; i < 16; i++) {
    color_pixel[i] = added_pixel[i];
    }

    if (now_half_interp) {
    for (i = 0; i < 16; i++) {
        color_half_pixel[i][i] = color_pixel[i];
    }
    num = 16;
    for (i = 0; i < 16; i++) {
        for (j = i+1; j < 16; j++) {
        color_half_pixel[i][j] = added_pixel[num];
        color_half_pixel[j][i] = color_half_pixel[i][j];
        num++;
        }
    }
    }
}


/*----------------------------------------------------------------------
 * GVRAM/TVRAMを画像バッファに転送する関数の、リストを作成
 *  この関数は、 bpp ・ サイズ ・ エフェクト の変更時に呼び出す。
 *----------------------------------------------------------------------*/
static  int (*vram2screen_list[4][4][2])(void);
static  void    (*screen_buf_init_p)(void);

static  int (*menu2screen_p)(void);

static  void    (*status2screen_p)(int kind, byte pixmap[], int w, int h);
/*static    void    (*status_buf_init_p)(void);*/
static  void    (*status_buf_clear_p)(void);

static  void    set_vram2screen_list(void)
{
    typedef int     (*V2S_FUNC_TYPE)(void);
    typedef V2S_FUNC_TYPE   V2S_FUNC_LIST[4][4][2];
    V2S_FUNC_LIST *list = NULL;


    if (DEPTH <= 8) {       /* ----------------------------------------- */

#ifdef  SUPPORT_8BPP
    switch (now_screen_size) {
    case SCREEN_SIZE_FULL:
        if      (use_interlace == 0) {
        if (screen_write_only)   { list = &vram2screen_list_F_N__8_d; }
        else                     { list = &vram2screen_list_F_N__8; }
        }
        else if (use_interlace >  0) { list = &vram2screen_list_F_I__8; }
        else                         { list = &vram2screen_list_F_S__8; }
        menu2screen_p = menu2screen_F_N__8;
        break;
    case SCREEN_SIZE_HALF:
        if (now_half_interp) { list = &vram2screen_list_H_P__8;
                menu2screen_p = menu2screen_H_P__8; }
        else                 { list = &vram2screen_list_H_N__8;
                menu2screen_p = menu2screen_H_N__8; }
        break;
#ifdef  SUPPORT_DOUBLE
    case SCREEN_SIZE_DOUBLE:
        if (screen_write_only) {
        if      (use_interlace == 0) { list=&vram2screen_list_D_N__8_d; }
        else if (use_interlace >  0) { list=&vram2screen_list_D_I__8_d; }
        else                         { list=&vram2screen_list_D_S__8_d; }
        } else {
        if      (use_interlace == 0) { list=&vram2screen_list_D_N__8; }
        else if (use_interlace >  0) { list=&vram2screen_list_D_I__8; }
        else                         { list=&vram2screen_list_D_S__8; }
        }
        menu2screen_p = menu2screen_D_N__8;
        break;
#endif
    }
    screen_buf_init_p  = screen_buf_init__8;
    status2screen_p    = status2screen__8;
/*  status_buf_init_p  = status_buf_init__8;*/
    status_buf_clear_p = status_buf_clear__8;
#else
    fprintf(stderr, "Error! This version is not support %dbpp !\n", DEPTH);
    exit(1);
#endif

    } else if (DEPTH <= 16) {   /* ----------------------------------------- */

#ifdef  SUPPORT_16BPP
    switch (now_screen_size) {
    case SCREEN_SIZE_FULL:
        if      (use_interlace == 0) {
        if (screen_write_only)   { list = &vram2screen_list_F_N_16_d; }
        else                     { list = &vram2screen_list_F_N_16; }
        }
        else if (use_interlace >  0) { list = &vram2screen_list_F_I_16; }
        else                         { list = &vram2screen_list_F_S_16; }
        menu2screen_p = menu2screen_F_N_16;
        break;
    case SCREEN_SIZE_HALF:
        if (now_half_interp) { list = &vram2screen_list_H_P_16;
                menu2screen_p = menu2screen_H_P_16; }
        else                 { list = &vram2screen_list_H_N_16;
                menu2screen_p = menu2screen_H_N_16; }
        break;
#ifdef  SUPPORT_DOUBLE
    case SCREEN_SIZE_DOUBLE:
        if (screen_write_only) {
        if      (use_interlace == 0) { list=&vram2screen_list_D_N_16_d; }
        else if (use_interlace >  0) { list=&vram2screen_list_D_I_16_d; }
        else                         { list=&vram2screen_list_D_S_16_d; }
        } else {
        if      (use_interlace == 0) { list=&vram2screen_list_D_N_16; }
        else if (use_interlace >  0) { list=&vram2screen_list_D_I_16; }
        else                         { list=&vram2screen_list_D_S_16; }
        }
        menu2screen_p = menu2screen_D_N_16;
        break;
#endif
    }
    screen_buf_init_p  = screen_buf_init_16;
    status2screen_p    = status2screen_16;
/*  status_buf_init_p  = status_buf_init_16;*/
    status_buf_clear_p = status_buf_clear_16;
#else
    fprintf(stderr, "Error! This version is not support %dbpp !\n", DEPTH);
    exit(1);
#endif

    } else if (DEPTH <= 32) {   /* ----------------------------------------- */

#ifdef  SUPPORT_32BPP
    switch (now_screen_size) {
    case SCREEN_SIZE_FULL:
        if      (use_interlace == 0) {
        if (screen_write_only)   { list = &vram2screen_list_F_N_32_d; }
        else                     { list = &vram2screen_list_F_N_32; }
        }
        else if (use_interlace >  0) { list = &vram2screen_list_F_I_32; }
        else                         { list = &vram2screen_list_F_S_32; }
        menu2screen_p = menu2screen_F_N_32;
        break;
    case SCREEN_SIZE_HALF:
        if (now_half_interp) { list = &vram2screen_list_H_P_32;
                menu2screen_p = menu2screen_H_P_32; }
        else                 { list = &vram2screen_list_H_N_32;
                menu2screen_p = menu2screen_H_N_32; }
        break;
#ifdef  SUPPORT_DOUBLE
    case SCREEN_SIZE_DOUBLE:
        if (screen_write_only) {
        if      (use_interlace == 0) { list=&vram2screen_list_D_N_32_d; }
        else if (use_interlace >  0) { list=&vram2screen_list_D_I_32_d; }
        else                         { list=&vram2screen_list_D_S_32_d; }
        } else {
        if      (use_interlace == 0) { list=&vram2screen_list_D_N_32; }
        else if (use_interlace >  0) { list=&vram2screen_list_D_I_32; }
        else                         { list=&vram2screen_list_D_S_32; }
        }
        menu2screen_p = menu2screen_D_N_32;
        break;
#endif
    }
    screen_buf_init_p  = screen_buf_init_32;
    status2screen_p    = status2screen_32;
/*  status_buf_init_p  = status_buf_init_32;*/
    status_buf_clear_p = status_buf_clear_32;
#else
    fprintf(stderr, "Error! This version is not support %dbpp !\n", DEPTH);
    exit(1);
#endif

    }

    memcpy(vram2screen_list, list, sizeof(vram2screen_list));
}


static  void    clear_all_screen(void)
{
    if (draw_start) { (draw_start)(); }     /* システム依存の描画前処理 */

    (screen_buf_init_p)();          /* 画面全クリア(ボーダー含) */
    if (now_status) {
    (status_buf_clear_p)();         /* ステータス領域 消去 */
    }

    if (draw_finish) { (draw_finish)(); }   /* システム依存の描画後処理 */
}



/*----------------------------------------------------------------------
 * GVRAM/TVRAM を screen_buf に転送する
 *
 *  int method == V_DIF … screen_dirty_flag に基づき、差分だけを転送
 *         == V_ALL … 画面すべてを転送
 *
 *  戻り値     == -1    … 転送なし (画面に変化なし)
 *         != -1    … 上位から 8ビットずつに、x0, y0, x1, y1 の
 *                 4個の unsigned 値がセットされる。ここで、
 *                  (x0 * 8, y0 * 2) - (x1 * 8, y1 * 2)
 *                 で表される範囲が、転送した領域となる。
 *
 *  予め、 set_vram2screen_list で関数リストを生成しておくこと
 *----------------------------------------------------------------------*/
static  int vram2screen(int method)
{
    int vram_mode, text_mode;

    if (sys_ctrl & SYS_CTRL_80) {       /* テキストの行・桁 */
    if (CRTC_SZ_LINES == 25) { text_mode = V_80x25; }
    else                     { text_mode = V_80x20; }
    } else {
    if (CRTC_SZ_LINES == 25) { text_mode = V_40x25; }
    else                     { text_mode = V_40x20; }
    }

    if (grph_ctrl & GRPH_CTRL_VDISP) {      /* VRAM 表示する */

    if (grph_ctrl & GRPH_CTRL_COLOR) {      /* カラー */
        vram_mode = V_COLOR;
    } else {
        if (grph_ctrl & GRPH_CTRL_200) {        /* 白黒 */
        vram_mode = V_MONO;
        } else {                    /* 400ライン */
        vram_mode = V_HIRESO;
        }
    }

    } else {                    /* VRAM 表示しない */

    vram_mode = V_UNDISP;
    }

#if 0
{ /*現在どのモードで表示中? */
  static int vram_mode0=-1, text_mode0=-1;
  if (text_mode0 != text_mode) {
    if      (text_mode==V_80x25) printf("      80x25\n");
    else if (text_mode==V_80x20) printf("      80x20\n");
    else if (text_mode==V_40x25) printf("      40x25\n");
    else if (text_mode==V_40x20) printf("      40x20\n");
    text_mode0 = text_mode;
  }
  if (vram_mode0 != vram_mode) {
    if      (vram_mode==V_COLOR)  printf("COLOR\n");
    else if (vram_mode==V_MONO)   printf("mono \n");
    else if (vram_mode==V_HIRESO) printf("H=400\n");
    else if (vram_mode==V_UNDISP) printf("-----\n");
    vram_mode0 = vram_mode;
  }
}
#endif

    return (vram2screen_list[ vram_mode ][ text_mode ][ method ])();
}



/*----------------------------------------------------------------------
 * 画面表示 ボーダー(枠)領域、メイン領域、ステータス領域の全てを表示
 *----------------------------------------------------------------------*/
static  void    put_image_all(void)
{
    T_GRAPH_RECT rect[1];

    rect[0].x = 0;
    rect[0].y = 0;
    rect[0].width  = WIDTH;
    rect[0].height = HEIGHT + ((now_status || now_fullscreen) ? STATUS_HEIGHT
                                      : 0);

    graph_update(1, &rect[0]);
}


/*----------------------------------------------------------------------
 * 画面表示 メイン領域の (x0,y0)-(x1,y1) と 指定されたステータス領域を表示
 *----------------------------------------------------------------------*/
static  void    put_image(int x0, int y0, int x1, int y1,
              int st0, int st1, int st2)
{
    int n = 0;
    T_GRAPH_RECT rect[4];

    if (x0 >= 0) {
    if        (now_screen_size == SCREEN_SIZE_FULL) {
        ;
    } else if (now_screen_size == SCREEN_SIZE_HALF) {
        x0 /= 2;  x1 /= 2;  y0 /= 2;  y1 /= 2;
    } else  /* now_screen_size == SCREEN_SIZE_DOUBLE */ {
        x0 *= 2;  x1 *= 2;  y0 *= 2;  y1 *= 2;
    }

    rect[n].x = SCREEN_DX + x0;
    rect[n].y = SCREEN_DY + y0;
    rect[n].width  = x1 - x0;
    rect[n].height = y1 - y0;
    n ++;
    }

    if (now_status || now_fullscreen ){ /* 全画面時は、ステータス消去がある */
    if (st0) {
        rect[n].x = 0;
        rect[n].y = HEIGHT;
        rect[n].width  = status_sx[0];
        rect[n].height = STATUS_HEIGHT;
        n ++;
    }
    if (st1) {
        rect[n].x = status_sx[0];
        rect[n].y = HEIGHT;
        rect[n].width  = status_sx[1];
        rect[n].height = STATUS_HEIGHT;
        n ++;
    }
    if (st2) {
        rect[n].x = status_sx[0] + status_sx[1];
        rect[n].y = HEIGHT;
        rect[n].width  = status_sx[2];
        rect[n].height = STATUS_HEIGHT;
        n ++;
    }
    }

    graph_update(n, &rect[0]);
}








/***********************************************************************
 * 描画の際に使用する、実際のパレット情報を引数 syspal にセットする
 ************************************************************************/
void    screen_get_emu_palette(PC88_PALETTE_T pal[16])
{
    int i;

    /* VRAM の カラーパレット設定   pal[0]〜[7] */

    if (grph_ctrl & GRPH_CTRL_COLOR) {      /* VRAM カラー */

    if (monitor_analog) {
        for (i = 0; i < 8; i++) {
        pal[i].red   = vram_palette[i].red   * 73 / 2;
        pal[i].green = vram_palette[i].green * 73 / 2;
        pal[i].blue  = vram_palette[i].blue  * 73 / 2;
        }
    } else {
        for (i = 0; i < 8; i++) {
        pal[i].red   = vram_palette[i].red   ? 0xff : 0;
        pal[i].green = vram_palette[i].green ? 0xff : 0;
        pal[i].blue  = vram_palette[i].blue  ? 0xff : 0;
        }
    }

    } else {                    /* VRAM 白黒 */

    if (monitor_analog) {
        pal[0].red   = vram_bg_palette.red   * 73 / 2;
        pal[0].green = vram_bg_palette.green * 73 / 2;
        pal[0].blue  = vram_bg_palette.blue  * 73 / 2;
    } else {
        pal[0].red   = vram_bg_palette.red   ? 0xff : 0;
        pal[0].green = vram_bg_palette.green ? 0xff : 0;
        pal[0].blue  = vram_bg_palette.blue  ? 0xff : 0;
    }
    for (i = 1; i < 8; i++) {
        pal[i].red   = 0;
        pal[i].green = 0;
        pal[i].blue  = 0;
    }

    }


    /* TEXT の カラーパレット設定   pal[8]〜[15] */

    if (grph_ctrl & GRPH_CTRL_COLOR) {      /* VRAM カラー */

    for (i = 8; i < 16; i++) {          /* TEXT 白黒の場合は */
        pal[i].red   = (i & 0x02) ? 0xff : 0;   /* 黒=[8],白=[15] を */
        pal[i].green = (i & 0x04) ? 0xff : 0;   /* 使うので問題なし  */
        pal[i].blue  = (i & 0x01) ? 0xff : 0;
    }

    } else {                    /* VRAM 白黒   */

    if (misc_ctrl & MISC_CTRL_ANALOG) {     /* アナログパレット時*/

        if (monitor_analog) {
        for (i = 8; i < 16; i++) {
            pal[i].red   = vram_palette[i & 0x7].red   * 73 / 2;
            pal[i].green = vram_palette[i & 0x7].green * 73 / 2;
            pal[i].blue  = vram_palette[i & 0x7].blue  * 73 / 2;
        }
        } else {
        for (i = 8; i < 16; i++) {
            pal[i].red   = vram_palette[i & 0x7].red   ? 0xff : 0;
            pal[i].green = vram_palette[i & 0x7].green ? 0xff : 0;
            pal[i].blue  = vram_palette[i & 0x7].blue  ? 0xff : 0;
        }
        }

    } else {                    /* デジタルパレット時*/
        for (i = 8; i < 16; i++) {
        pal[i].red   = (i & 0x02) ? 0xff : 0;
        pal[i].green = (i & 0x04) ? 0xff : 0;
        pal[i].blue  = (i & 0x01) ? 0xff : 0;
        }
    }

    }
}



/***********************************************************************
 * イメージ転送 (表示)
 *
 *  この関数は、表示タイミング (約 1/60秒毎) に呼び出される。
 ************************************************************************/
void    screen_update(void)
{
    int i;
    int skip = FALSE;
    int all_area  = FALSE;  /* 全エリア転送フラグ  */
    int rect = -1;      /* 画面転送フラグ    */
    int flag = 0;       /* ステータス転送フラグ   */
    PC88_PALETTE_T syspal[16];
    int is_exec = (quasi88_is_exec()) ? TRUE : FALSE;


    screen_attr_update();   /* マウス自動で隠す…呼び出し場所がいまいち */


    if (is_exec) {
    profiler_lapse( PROF_LAPSE_BLIT );
    }

    status_update();        /* ステータス領域の画像データを更新 */


    /* メイン領域は、描画をスキップする場合があるので、以下で判定 */
    /* (メニューなどは、常時 frame_counter==0 なので、毎回描画)   */

    if ((frame_counter % frameskip_rate) == 0) { /* 描画の時が来た。       */
                         /* 以下のいずれかなら描画 */
    if (no_wait ||                 /* ウェイトなし設定時     */
        use_auto_skip == FALSE ||          /* 自動スキップなし設定時 */
        do_skip_draw  == FALSE) {          /* 今回スキップ対象でない */

        skip = FALSE;

    } else {                 /* 以外はスキップ */

        skip = TRUE;

        /* 描画タイミングなのにスキップした場合は、そのことを覚えておく */
        already_skip_draw = TRUE;
    }

    /* カーソル点滅のワーク更新 */
    if (is_exec) {
        if (--blink_ctrl_counter == 0) {
        blink_ctrl_counter = blink_ctrl_cycle;
        blink_counter ++;
        }
    }
    } else {
    skip = TRUE;
    }


    /* メイン領域を描画する (スキップしない) 場合の処理 */

    if (skip == FALSE) {

    /* 色転送 */

    if (screen_dirty_palette) {     /* 色をシステムに転送 */
        if (quasi88_is_menu()) {
        screen_get_menu_palette(syspal);
        } else {
        screen_get_emu_palette(syspal);
        }
        trans_palette(syspal);
    }

    /* 必要に応じて、フラグをセット */

    if (screen_dirty_frame) {
        screen_set_dirty_all();
        screen_set_dirty_status();
        all_area = TRUE;
    }

    if (screen_dirty_palette) {
        screen_set_dirty_all();
        screen_dirty_palette = FALSE; 
    }

    /* フラグに応じて、描画 */

    if (draw_start) { (draw_start)(); } /* システム依存の描画前処理 */

    if (screen_dirty_frame) {
        (screen_buf_init_p)();      /* 画面全クリア(ボーダー含) */
        screen_dirty_frame = FALSE;
        /* ボーダー部は黒固定。色変更可とするなら、先に色転送が必要… */
    }

    /* VRAM転送 */

    if (quasi88_is_menu()) {

        if (screen_dirty_all ||
        screen_dirty_flag[0]) {

        if (screen_dirty_all) {
            /* 裏画面をクリア。これで差分＝全部になる */
            memset(&menu_screen[menu_screen_current^1],
               0,
               sizeof(menu_screen[0]));
        }

        rect = (menu2screen_p)();

        /* 表画面と裏画面を一致させておかないとおかしくなる… */
        memcpy(&menu_screen[menu_screen_current^1],
               &menu_screen[menu_screen_current],
               sizeof(menu_screen[0]));

        menu_screen_current ^= 1;
        screen_dirty_flag[0] = 0;
        screen_dirty_all = FALSE;
        }

    } else {

        /* VRAM更新フラグ screen_dirty_flag の例外処理             */
        /*      VRAM非表示の場合、更新フラグは意味無いのでクリアする */
        /*      400ラインの場合、更新フラグを画面下半分にも拡張する  */

        if (screen_dirty_all == FALSE) {
        if (! (grph_ctrl & GRPH_CTRL_VDISP)) {
            /* 非表示 */
            memset(screen_dirty_flag, 0, sizeof(screen_dirty_flag) / 2);
        }
        if (! (grph_ctrl & (GRPH_CTRL_COLOR|GRPH_CTRL_200))) {
            /* 400ライン */
            memcpy(&screen_dirty_flag[80*200], screen_dirty_flag, 80*200);
        }
        }

        crtc_make_text_attr();  /* TVRAM の 属性一覧作成   */
                    /* VRAM/TEXT → screen_buf 転送    */
        rect = vram2screen(screen_dirty_all ? V_ALL : V_DIF);

        text_attr_flipflop ^= 1;
        memset(screen_dirty_flag, 0, sizeof(screen_dirty_flag));
        screen_dirty_all = FALSE;
    }

    if (draw_finish) { (draw_finish)(); }   /* システム依存の描画後処理 */
    }


    /* ステータスエリアの処理 (ステータスは、表示する限りスキップしない) */

    if (draw_start) { (draw_start)(); }     /* システム依存の描画前処理 */

    if (screen_dirty_status_hide) {
    (status_buf_clear_p)();         /* ステータス領域 消去 */
    screen_dirty_status_hide = FALSE;
    all_area = TRUE;
    }

    if (screen_dirty_status_show) {
    (status_buf_clear_p)();         /* ステータス領域 初期化 */
    screen_dirty_status_show = FALSE;
    all_area = TRUE;
    }

    if (now_status) {
    if (screen_dirty_status) {
        flag = screen_dirty_status;
        for (i = 0; i < 3; i++) {
        if (flag & (1 << i)) {
            (status2screen_p)(i, status_info[i].pixmap,
                      status_info[i].w, status_info[i].h);
        }
        }
        screen_dirty_status = 0;
    }
    }

    if (draw_finish) { (draw_finish)(); }   /* システム依存の描画後処理 */

    if (is_exec) {
    profiler_video_output(((frame_counter % frameskip_rate) == 0),
                  skip, (all_area || rect != -1));
    }


    if ((is_exec) && (dont_frameskip == FALSE)) {
    ++ frame_counter;
    } else {    /* menu,pause,monitor */
    frame_counter = 0;
    }

    if (is_exec) {
    profiler_lapse( PROF_LAPSE_VIDEO );
    }

#if USE_RETROACHIEVEMENTS
    put_image_all();
    drawn_count++;
#else
    if (all_area) {

    put_image_all();
    drawn_count ++;

    } else {
    if (rect != -1) {
        put_image(((rect >> 24)       ) * 8, ((rect >> 16) & 0xff) * 2,
              ((rect >>  8) & 0xff) * 8, ((rect      ) & 0xff) * 2,
              (flag & 1), (flag & 2), (flag & 4));
        drawn_count ++;
    }
    else if (flag) {
        put_image(-1, -1, -1, -1,
              (flag & 1), (flag & 2), (flag & 4));
    }
    }
#endif
}

void    screen_update_immidiate(void)
{
    screen_set_dirty_frame();       /* 全領域 更新 */
    frameskip_counter_reset();      /* 次回描画 */

    screen_update();            /* 描画処理 */
}

int quasi88_info_draw_count(void)
{
    return drawn_count;
}













/***********************************************************************
 * システムイベント処理 EXPOSE / FOCUS-IN / FOCUS-OUT
 ***********************************************************************/

/*======================================================================
 *
 *======================================================================*/
void    quasi88_expose(void)
{
    screen_set_dirty_frame();       /* 全領域 更新 */

    frameskip_counter_reset();      /* 次回描画 */
}


/*======================================================================
 *
 *======================================================================*/
void    quasi88_focus_in(void)
{
    if (quasi88_is_pause()) {

    pause_event_focus_in_when_pause();

    }
}

void    quasi88_focus_out(void)
{
    int kana_on, caps_on;

    kana_on = IS_KEY88_PRESS(KEY88_KANA);
    caps_on = IS_KEY88_PRESS(KEY88_CAPS);

    softkey_release_all(); /* キーの押下解除 */

    /* カナ、CAPSの状態を戻す */
    if (kana_on) KEY88_PRESS(KEY88_KANA);
    if (caps_on) KEY88_PRESS(KEY88_CAPS);

    if (quasi88_is_exec()) {

    pause_event_focus_out_when_exec();

    }
}













/***********************************************************************
 * テキスト点滅 (カーソルおよび、文字属性の点滅) 処理のワーク設定
 *  CRTC の frameskip_rate, blink_cycle が変更されるたびに呼び出す
 ************************************************************************/
void    frameskip_blink_reset(void)
{
    int wk;

    wk = blink_cycle / frameskip_rate;

    if (wk == 0 ||
    ! (blink_cycle-wk*frameskip_rate < (wk+1)*frameskip_rate-blink_cycle))
    wk++;
  
    blink_ctrl_cycle = wk;
    blink_ctrl_counter = blink_ctrl_cycle;
}



/***********************************************************************
 * フレームカウンタ初期化
 *  次フレームは、必ず表示される。(スキップされない)
 ************************************************************************/
void    frameskip_counter_reset(void)
{
    frame_counter = 0;
    do_skip_draw = FALSE;
    already_skip_draw = FALSE;
}



/***********************************************************************
 * 自動フレームスキップ処理     ( by floi, thanks ! )
 ************************************************************************/
void    frameskip_check(int on_time)
{
    if (use_auto_skip) {

    if (on_time) {          /* 時間内に処理できた */

        skip_counter = 0;
        do_skip_draw = FALSE;       /* 次回描画とする */

        if (already_skip_draw) {        /* 前回描画時、スキップしてたら */
        frameskip_counter_reset();  /* 次のVSYNCで強制描画 */
        }

    } else {            /* 時間内に処理できていない */

        do_skip_draw = TRUE;        /* 次回描画スキップ */

        skip_counter++;         /* 但し、スキップしすぎなら */
        if (skip_counter >= skip_count_max) {
        frameskip_counter_reset();  /* 次のVSYNCで強制描画 */
        }
    }
    }
}



/***********************************************************************
 * フレームスキップレートの取得・設定
 ************************************************************************/
int quasi88_cfg_now_frameskip_rate(void)
{
    return frameskip_rate;
}
void    quasi88_cfg_set_frameskip_rate(int rate)
{
    char str[32];

    if (rate <= 0) rate = 1;

    if (rate != frameskip_rate) {
    frameskip_rate = rate;

    frameskip_blink_reset();
    frameskip_counter_reset();

    sprintf(str, "FRAME RATE = %2d/sec", 60/rate);
    status_message(1, STATUS_INFO_TIME, str);
    /* 変更した後は、しばらく画面にフレームレートを表示させる */
    }
}





/***********************************************************************
 * ステートロード／ステートセーブ
 ************************************************************************/

#define SID "SCRN"

static  T_SUSPEND_W suspend_screen_work[]=
{
    { TYPE_CHAR,    &vram_bg_palette.blue,  },
    { TYPE_CHAR,    &vram_bg_palette.red,   },
    { TYPE_CHAR,    &vram_bg_palette.green, },

    { TYPE_CHAR,    &vram_palette[0].blue,  },
    { TYPE_CHAR,    &vram_palette[0].red,   },
    { TYPE_CHAR,    &vram_palette[0].green, },
    { TYPE_CHAR,    &vram_palette[1].blue,  },
    { TYPE_CHAR,    &vram_palette[1].red,   },
    { TYPE_CHAR,    &vram_palette[1].green, },
    { TYPE_CHAR,    &vram_palette[2].blue,  },
    { TYPE_CHAR,    &vram_palette[2].red,   },
    { TYPE_CHAR,    &vram_palette[2].green, },
    { TYPE_CHAR,    &vram_palette[3].blue,  },
    { TYPE_CHAR,    &vram_palette[3].red,   },
    { TYPE_CHAR,    &vram_palette[3].green, },
    { TYPE_CHAR,    &vram_palette[4].blue,  },
    { TYPE_CHAR,    &vram_palette[4].red,   },
    { TYPE_CHAR,    &vram_palette[4].green, },
    { TYPE_CHAR,    &vram_palette[5].blue,  },
    { TYPE_CHAR,    &vram_palette[5].red,   },
    { TYPE_CHAR,    &vram_palette[5].green, },
    { TYPE_CHAR,    &vram_palette[6].blue,  },
    { TYPE_CHAR,    &vram_palette[6].red,   },
    { TYPE_CHAR,    &vram_palette[6].green, },
    { TYPE_CHAR,    &vram_palette[7].blue,  },
    { TYPE_CHAR,    &vram_palette[7].red,   },
    { TYPE_CHAR,    &vram_palette[7].green, },

    { TYPE_BYTE,    &sys_ctrl,      },
    { TYPE_BYTE,    &grph_ctrl,     },
    { TYPE_BYTE,    &grph_pile,     },

    { TYPE_INT,     &frameskip_rate,    },
    { TYPE_INT,     &monitor_analog,    },
    { TYPE_INT,     &use_auto_skip,     },
/*  { TYPE_INT,     &frame_counter,     }, 初期値でも問題ないだろう */
/*  { TYPE_INT,     &blink_ctrl_cycle,  }, 初期値でも問題ないだろう */
/*  { TYPE_INT,     &blink_ctrl_counter,    }, 初期値でも問題ないだろう */

    { TYPE_INT,     &use_interlace,     },
    { TYPE_INT,     &use_half_interp,   },

    { TYPE_END,     0           },
};


int statesave_screen(void)
{
    if (statesave_table(SID, suspend_screen_work) == STATE_OK) return TRUE;
    else                                                       return FALSE;
}

int stateload_screen(void)
{
    if (stateload_table(SID, suspend_screen_work) == STATE_OK) return TRUE;
    else                                                       return FALSE;
}
























/* デバッグ用の関数 */
void attr_misc(int line)
{
int i;

  text_attr_flipflop ^= 1;    
  for(i=0;i<80;i++){
    printf("%02X[%02X] ",
    text_attr_buf[text_attr_flipflop][line*80+i]>>8,
    text_attr_buf[text_attr_flipflop][line*80+i]&0xff );
  }
return;
  for(i=0;i<80;i++){
    printf("%c[%02X] ",
    text_attr_buf[text_attr_flipflop][9*80+i]>>8,
    text_attr_buf[text_attr_flipflop][9*80+i]&0xff );
  }
  for(i=0;i<80;i++){
    printf("%c[%02X] ",
    text_attr_buf[text_attr_flipflop][10*80+i]>>8,
    text_attr_buf[text_attr_flipflop][10*80+i]&0xff );
  }
  for(i=0;i<80;i++){
    printf("%c[%02X] ",
    text_attr_buf[text_attr_flipflop][11*80+i]>>8,
    text_attr_buf[text_attr_flipflop][11*80+i]&0xff );
  }
  printf("\n");
  for(i=0;i<80;i++){
    printf("%c[%02X] ",
    text_attr_buf[text_attr_flipflop][12*80+i]>>8,
    text_attr_buf[text_attr_flipflop][12*80+i]&0xff );
  }
  printf("\n");
  for(i=0;i<80;i++){
    printf("%c[%02X] ",
    text_attr_buf[text_attr_flipflop][13*80+i]>>8,
    text_attr_buf[text_attr_flipflop][13*80+i]&0xff );
  }
  printf("\n");
  for(i=0;i<80;i++){
    printf("%c[%02X] ",
    text_attr_buf[text_attr_flipflop][14*80+i]>>8,
    text_attr_buf[text_attr_flipflop][14*80+i]&0xff );
  }
#if 0
  for(i=0;i<80;i++){
    printf("%c[%02X] ",
    text_attr_buf[0][15*80+i]>>8,
    text_attr_buf[0][15*80+i]&0xff );
  }
  printf("\n");
  for(i=0;i<80;i++){
    printf("%c[%02X] ",
    text_attr_buf[0][16*80+i]>>8,
    text_attr_buf[0][16*80+i]&0xff );
  }
  printf("\n");
  for(i=0;i<80;i++){
    printf("%c[%02X] ",
    text_attr_buf[0][17*80+i]>>8,
    text_attr_buf[0][17*80+i]&0xff );
  }
  printf("\n");
  for(i=0;i<80;i++){
    printf("%c[%02X] ",
    text_attr_buf[0][18*80+i]>>8,
    text_attr_buf[0][18*80+i]&0xff );
  }
  printf("\n");
  for(i=0;i<80;i++){
    printf("%c[%02X] ",
    text_attr_buf[0][19*80+i]>>8,
    text_attr_buf[0][19*80+i]&0xff );
  }
  printf("\n");
#endif
}
