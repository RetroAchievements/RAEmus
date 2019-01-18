#ifndef SCREEN_H_INCLUDED
#define SCREEN_H_INCLUDED



typedef struct {
    unsigned    char    blue;           /* B面輝度 (0〜7/255)   */
    unsigned    char    red;            /* R面輝度 (0〜7/255)   */
    unsigned    char    green;          /* G面輝度 (0〜7/255)   */
    unsigned    char    padding;
} PC88_PALETTE_T;


/*
 *  PC-88 Related
 */

extern  PC88_PALETTE_T  vram_bg_palette;    /* 背景パレット   */
extern  PC88_PALETTE_T  vram_palette[8];    /* 各種パレット   */

extern  byte    sys_ctrl;           /* OUT[30] SystemCtrl     */
extern  byte    grph_ctrl;          /* OUT[31] GraphCtrl      */
extern  byte    grph_pile;          /* OUT[53] 重ね合わせ     */

#define SYS_CTRL_80     (0x01)      /* TEXT COLUMN80 / COLUMN40*/
#define SYS_CTRL_MONO       (0x02)      /* TEXT MONO     / COLOR   */

#define GRPH_CTRL_200       (0x01)      /* VRAM-MONO 200 / 400 line*/
#define GRPH_CTRL_64RAM     (0x02)      /* RAM   64K-RAM / ROM-RAM */
#define GRPH_CTRL_N     (0x04)      /* BASIC       N / N88     */
#define GRPH_CTRL_VDISP     (0x08)      /* VRAM  DISPLAY / UNDISP  */
#define GRPH_CTRL_COLOR     (0x10)      /* VRAM  COLOR   / MONO    */
#define GRPH_CTRL_25        (0x20)      /* TEXT  LINE25  / LINE20  */

#define GRPH_PILE_TEXT      (0x01)      /* 重ね合わせ 非表示 TEXT  */
#define GRPH_PILE_BLUE      (0x02)      /*             B   */
#define GRPH_PILE_RED       (0x04)      /*             R   */
#define GRPH_PILE_GREEN     (0x08)      /*             G   */



/*
 *  描画処理用ワーク
 */

    /* 描画差分管理 */

extern  char    screen_dirty_flag[ 0x4000*2 ];  /* メイン領域 差分更新 */
extern  int screen_dirty_all;       /* メイン領域 全域更新 */
extern  int screen_dirty_palette;       /* 色情報 更新     */
extern  int screen_dirty_status;        /* ステータス領域 更新 */
extern  int screen_dirty_status_hide;   /* ステータス領域 消去 */
extern  int screen_dirty_status_show;   /* ステータス領域 初期化*/
extern  int screen_dirty_frame;     /* 全領域 更新     */

#define screen_set_dirty_flag(x)    screen_dirty_flag[x] = 1
#define screen_set_dirty_all()      screen_dirty_all = TRUE
#define screen_set_dirty_palette()  do {                \
                      screen_dirty_palette = TRUE;  \
                      screen_dirty_all = TRUE;  \
                    } while(0)
#define screen_set_dirty_status()   screen_dirty_status = 0xff
#define screen_set_dirty_status_hide()  screen_dirty_status_hide = TRUE
#define screen_set_dirty_status_show()  screen_dirty_status_show = TRUE
#define screen_set_dirty_frame()    screen_dirty_frame = TRUE;


    /* その他 */

extern  int frameskip_rate;     /* 画面表示の更新間隔      */
extern  int monitor_analog;     /* アナログモニター     */
extern  int use_auto_skip;      /* 自動フレームスキップ       */



/*
 *  表示設定
 */

enum {
    SCREEN_INTERLACE_NO = 0,        /* インターレス表示しない    */
    SCREEN_INTERLACE_YES = 1,       /* インターレス表示する       */
    SCREEN_INTERLACE_SKIP = -1      /* 1ラインおき表示する     */
};
extern  int use_interlace;      /* インターレース表示      */

extern  int use_half_interp;    /* 画面サイズ半分時、色補間する */

enum {                  /* 画面サイズ          */
    SCREEN_SIZE_HALF,           /*      320 x 200   */
    SCREEN_SIZE_FULL,           /*      640 x 400   */
#ifdef  SUPPORT_DOUBLE
    SCREEN_SIZE_DOUBLE,         /*      1280x 800   */
#endif
    SCREEN_SIZE_END
};
extern  int screen_size;        /* 画面サイズ指定        */
extern  int now_screen_size;    /*実際の、画面サイズ  */

extern  int use_fullscreen;     /* 全画面表示指定        */

extern  double  mon_aspect;     /* モニターのアスペクト比    */

extern  int status_fg;      /* ステータス前景色     */
extern  int status_bg;      /* ステータス背景色     */

extern  int show_status;        /* ステータス表示有無      */


/*
 *
 */

enum {
    SHOW_MOUSE = 0,
    HIDE_MOUSE = 1,
    AUTO_MOUSE = 2
};
extern  int hide_mouse;     /* マウスを隠すかどうか       */
enum {
    UNGRAB_MOUSE = 0,
    GRAB_MOUSE   = 1
};
extern  int grab_mouse;     /* グラブするかどうか      */

extern  int use_swcursor;       /* メニュー専用カーソル表示する？*/
extern  int now_swcursor;       /* 現在専用カーソル表示中?   */


/*
 *  表示デバイス用ワーク
 */

#define STATUS_HEIGHT   (20)

extern  int WIDTH;          /* 描画バッファ横サイズ       */
extern  int HEIGHT;         /* 描画バッファ縦サイズ       */
extern  int DEPTH;          /* 色ビット数  (8/16/32)   */
extern  int SCREEN_W;       /* 画面横サイズ (320/640/1280)    */
extern  int SCREEN_H;       /* 画面縦サイズ (200/400/800) */

extern  int SCREEN_DX;      /* ウインドウ左上と、      */
extern  int SCREEN_DY;      /* 画面エリア左上とのオフセット   */

extern  char    *screen_buf;        /* 描画バッファ先頭     */
extern  char    *screen_start;      /* 画面先頭         */

extern  char    *status_buf;        /* ステータス全域 先頭     */
extern  char    *status_start[3];   /* ステータス描画 先頭     */
extern  int status_sx[3];       /* ステータス描画サイズ       */
extern  int status_sy[3];




extern  Ulong   color_pixel[16];        /* 色コード     */
extern  Ulong   color_half_pixel[16][16];   /* 色補完時の色コード  */
extern  Ulong   black_pixel;            /* 黒の色コード       */
enum {                      /* ステータスに使う色  */
    STATUS_BG,                  /*  背景色(白)  */
    STATUS_FG,                  /*  前景色(黒)  */
    STATUS_BLACK,               /*  黒色      */
    STATUS_WHITE,               /*  白色      */
    STATUS_RED,                 /*  赤色      */
    STATUS_GREEN,               /*  緑色      */
    STATUS_COLOR_END
};
extern  Ulong   status_pixel[STATUS_COLOR_END]; /* ステータスの色コード   */



/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
                          WIDTH
     ←───────────────────→
    ┌────────────────────┐ ↑
    │              ↑                        │ │
    │              │SCREEN_DY               │ │
    │              ↓                        │ │
    │←─────→┌─────────┐    │ │
    │   SCREEN_DX  │  ↑              │    │ │
    │              │←───────→│    │ │HEIGHT
    │              │  │   SCREEN_W   │    │ │
    │              │  │              │    │ │
    │              │  │SCREEN_H      │    │ │
    │              │  ↓              │    │ │
    │              └─────────┘    │ │
    │                                        │ ↓
    ├──────┬──────┬──────┤ ↑
    │ステータス0 │ステータス1 │ステータス2 │ │STATUS_HEIGHT
    └──────┴──────┴──────┘ ↓
        ステータス0〜2のサイズ比率は、 1:3:1

    screen_buf  描画バッファ全域の、先頭ポインタ
    WIDTH       描画バッファ全域の、横ピクセル数
    HEIGHT              〃          縦ピクセル数

    screen_size 画面サイズ
    screen_start    画面バッファの、先頭ポインタ
    SCREEN_W    画面バッファの、横ピクセル数 (320/640/1280)
    SCREEN_H          〃        縦ピクセル数 (200/400/800)

    DEPTH       色深度 (バッファのビット幅、8/16/32)

    status_buf  ステータスバッファ全域の、先頭ポインタ
    status_start[3] ステータス 0〜2 のバッファの、先頭ポインタ
    status_sx[3]        〃           横ピクセル数
    status_sy[3]        〃           縦ピクセル数


    ※ ウインドウ表示の場合、
        WIDTH * (HEIGHT + STATUS_HEIGHT) のサイズで、
        ウインドウを生成します。
            (ステータス非表示なら、 WIDTH * HEIGHT)

    ※ 全画面表示の場合、
        SCREEN_SX * (SCREEN_SY + STATUS_HEIGHT) 以上のサイズで
        全画面化します。
            (ステータス非表示なら、下の部分は黒で塗りつぶす)

   - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

/***********************************************************************
 * 画面処理の初期化・終了
 *  screen_init()   画面を生成する。起動時に呼ばれる。
 *  screen_exit()   後かたづけする。終了時に呼ばれる。
 ************************************************************************/
int screen_init(void);
void    screen_exit(void);


/***********************************************************************
 * モード切り替え時の、各種再設定
 *  全エリアを強制描画する必要があるので、その準備をする。
 *  grab_mouse 、 hide_mouse などに基づき、マウスの設定をする。
 *  キーリピートや、ステータスも設定する。
 ************************************************************************/
void    screen_switch(void);


/***********************************************************************
 *
 ************************************************************************/
void    screen_attr_mouse_move(void);
void    screen_attr_mouse_click(void);
int screen_attr_mouse_debug(void);


/***********************************************************************
 * PC-8801の最終的な色を取得する
 ************************************************************************/
void    screen_get_emu_palette(PC88_PALETTE_T pal[16]);
void    screen_get_menu_palette(PC88_PALETTE_T pal[16]);


/***********************************************************************
 * 描画
 ************************************************************************/
void    screen_update(void);        /* 描画   (1/60sec毎)  */
void    screen_update_immidiate(void);  /* 即描画 (モニター用) */


/***********************************************************************
 * フレームスキップ
 ************************************************************************/
void    frameskip_blink_reset(void);    /* 点滅処理 再初期化        */
void    frameskip_counter_reset(void);  /* フレームスキップ 再初期化    */
void    frameskip_check(int on_time);   /* フレームスキップ 判定  */

int quasi88_cfg_now_frameskip_rate(void);
void    quasi88_cfg_set_frameskip_rate(int rate);



/***********************************************************************
 * HALFサイズ時の色補完の有効・無効関連の関数
 ***********************************************************************/
int quasi88_cfg_can_interp(void);
int quasi88_cfg_now_interp(void);
void    quasi88_cfg_set_interp(int enable);

/***********************************************************************
 * INTERLACEの設定関連の関数
 ***********************************************************************/
int quasi88_cfg_now_interlace(void);
void    quasi88_cfg_set_interlace(int interlace_mode);

/***********************************************************************
 * ステータス表示設定関連の関数
 ***********************************************************************/
int quasi88_cfg_can_showstatus(void);
int quasi88_cfg_now_showstatus(void);
void    quasi88_cfg_set_showstatus(int show);

/***********************************************************************
 * 全画面設定・画面サイズ設定関連の関数
 ***********************************************************************/
int quasi88_cfg_can_fullscreen(void);
int quasi88_cfg_now_fullscreen(void);
void    quasi88_cfg_set_fullscreen(int fullscreen);
int quasi88_cfg_max_size(void);
int quasi88_cfg_min_size(void);
int quasi88_cfg_now_size(void);
void    quasi88_cfg_set_size(int new_size);
void    quasi88_cfg_set_size_large(void);
void    quasi88_cfg_set_size_small(void);

/***********************************************************************
 * ???
 ***********************************************************************/
int quasi88_info_draw_count(void);

#endif  /* SCREEN_H_INCLUDED */
