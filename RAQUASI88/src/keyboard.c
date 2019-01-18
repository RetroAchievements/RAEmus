#include <stdlib.h>
#include <string.h>

#include "quasi88.h"
#include "keyboard.h"
#include "romaji.h"
#include "event.h"

#include "soundbd.h"    /* sound_reg[]          */
#include "graph.h"  /* set_key_and_mouse()      */
#include "pc88cpu.h"    /* z80main_cpu          */
#include "intr.h"   /* state_of_cpu         */

#include "snddrv.h" /* xmame_XXX            */
#include "wait.h"   /* wait_rate            */
#include "pc88main.h"   /* boot_clock_4mhz      */

#include "drive.h"

#include "emu.h"
#include "status.h"
#include "pause.h"
#include "menu.h"
#include "screen.h"
#include "snapshot.h"

#include "suspend.h"

/******************************************************************************
 *
 *****************************************************************************/

/*
 *  内部状態 (ステートセーブ不要?)
 */

int     mouse_x;        /* 現在の マウス x座標      */
int     mouse_y;        /* 現在の マウス y座標      */

static  int mouse_dx;       /* マウス x方向移動量       */
static  int mouse_dy;       /* マウス y方向移動量       */

static  int mouse_sx;       /* シリアルマウス x方向移動量   */
static  int mouse_sy;       /* シリアルマウス y方向移動量   */
static  int mouse_sb;       /* シリアルマウス ボタン  */


unsigned char   key_scan[ 0x10 ];   /* IN(00h)〜(0Eh) キースキャン   */
                    /* key_scan[0]〜[14] が、    */
                    /* I/Oポート 00H〜0EH と等価。  */
                    /* key_scan[15] は、ジョイ */
                    /* スティックの一時ワークに使用 */

static  int key_func[ KEY88_END ];  /* キーの代替コード,機能割当    */


/*
 *  PC88状態 / 内部状態
 */

static  int jop1_step;  /* 汎用I/Oポートのリードステップ   */
static  int jop1_dx;    /* 汎用I/Oポートの値 (マウス x方向変位)   */
static  int jop1_dy;    /* 汎用I/Oポートの値 (マウス y方向変位)   */
static  int jop1_time;  /* 汎用I/Oポートのストローブ処理した時  */

    int romaji_input_mode = FALSE;  /* 真:ローマ字入力中    */



/*
  mouse_x, mouse_dx, jop1_dx の関係
    マウスが移動した時
        mouse_x にその絶対座標をセットする。
        前回座標との変位 mouse_dx にセットする。
    汎用I/Oポートからマウス座標をリードした時、
        mouse_dx をクリッピングした値を、 jop1_dx にセットする。
*/



/*
 *  設定
 */

int mouse_mode  = 0;        /* マウス・ジョイステック処理  */

int mouse_sensitivity = 100;    /* マウス感度          */
int mouse_swap_button = FALSE;  /* マウスボタンを入れ替える     */


int mouse_key_mode  = 0;        /* マウス入力をキーに反映    */
int mouse_key_assign[6];        /*     0:なし 1:テンキー 2:任意 */
static const int mouse_key_assign_tenkey[6] =
{
  KEY88_KP_8, KEY88_KP_2, KEY88_KP_4, KEY88_KP_6,
  KEY88_x,    KEY88_z,
};


int joy_key_mode    = 0;        /* ジョイ入力をキーに反映    */
int joy_key_assign[12];     /*     0:なし 1:テンキー 2:任意 */
static const int joy_key_assign_tenkey[12] =
{
  KEY88_KP_8, KEY88_KP_2, KEY88_KP_4, KEY88_KP_6,
  KEY88_x,    KEY88_z,    0, 0, 0, 0, 0, 0,
};
int joy_swap_button   = FALSE;  /* ボタンのABを入れ替える     */


int joy2_key_mode   = 0;        /* ジョイ２入力をキーに反映 */
int joy2_key_assign[12];        /*     0:なし 1:テンキー 2:任意 */
int joy2_swap_button   = FALSE; /* ボタンのABを入れ替える     */



int cursor_key_mode = 0;        /* カーソルキーを別キーに反映  */
int cursor_key_assign[4];       /*     0:なし 1:テンキー 2:任意 */
static const int cursor_key_assign_tenkey[4] =
{
  KEY88_KP_8, KEY88_KP_2, KEY88_KP_4, KEY88_KP_6,
};      /* Cursor KEY -> 10 KEY , original by funa. (thanks!) */
        /* Cursor Key -> 任意のキー , original by floi. (thanks!) */



int tenkey_emu      = FALSE;    /* 真:数字キーをテンキーに   */
int numlock_emu     = FALSE;    /* 真:ソフトウェアNumLockを行う   */




int function_f[ 1 + 20 ] =      /* ファンクションキーの機能     */
{
  FN_FUNC,    /* [0] はダミー */
  FN_FUNC,    FN_FUNC,    FN_FUNC,    FN_FUNC,    FN_FUNC,  /* f1 〜f5  */
  FN_FUNC,    FN_FUNC,    FN_FUNC,    FN_FUNC,    FN_FUNC,  /* f6 〜f10 */
  FN_STATUS,  FN_MENU,    FN_FUNC,    FN_FUNC,    FN_FUNC,  /* f11〜f15 */
  FN_FUNC,    FN_FUNC,    FN_FUNC,    FN_FUNC,    FN_FUNC,  /* f16〜f20 */
};


int fn_max_speed = 1600;
double  fn_max_clock = CONST_4MHZ_CLOCK*16;
int fn_max_boost = 16;


int romaji_type = 0;        /* ローマ字変換のタイプ       */





/*
 *  キー操作 記録・再生
 */

char    *file_rec   = NULL;     /* キー入力記録のファイル名 */
char    *file_pb    = NULL;     /* キー入力再生のファイル名 */

static  OSD_FILE *fp_rec;
static  OSD_FILE *fp_pb;

static struct {             /* キー入力記録構造体      */
  Uchar key[16];            /*  I/O 00H〜0FH       */
   char dx_h;               /*  マウス dx 上位     */
  Uchar dx_l;               /*  マウス dx 下位     */
   char dy_h;               /*  マウス dy 上位     */
  Uchar dy_l;               /*  マウス dy 下位     */
   char image[2];           /*  イメージNo -1空,0同,1〜  */
   char resv[2];
} key_record;               /* 24 bytes         */






/*---------------------------------------------------------------------------
 * キーのバインディング変更 (キーコード、機能)
 *---------------------------------------------------------------------------*/
static  void    clr_key_function(void)
{
    int i;
    for (i=0; i<COUNTOF(key_func); i++) { key_func[i] = 0; }

    /* この2個のキーは、機能固定 */
    key_func[ KEY88_SYS_STATUS ] = FN_STATUS;
    key_func[ KEY88_SYS_MENU   ] = FN_MENU;
}


static  void    set_key_function(int keycode, int func_no)
{
    key_func[ keycode ] = func_no;

    /* この2個のキーは、機能固定 */
    key_func[ KEY88_SYS_STATUS ] = FN_STATUS;
    key_func[ KEY88_SYS_MENU   ] = FN_MENU;
}


static  void    swap_key_function(int keycode1, int keycode2)
{
    int              tmp = key_func[ keycode1 ];
    key_func[ keycode1 ] = key_func[ keycode2 ];
    key_func[ keycode2 ] = tmp;
}


/*---------------------------------------------------------------------------
 * マウスの座標を、画面サイズに合わせて補正する
 *---------------------------------------------------------------------------*/
static void mouse_movement_adjust(int *x, int *y)
{
    (*x) -= SCREEN_DX;
    (*y) -= SCREEN_DY;

    switch (quasi88_cfg_now_size()) {
    case SCREEN_SIZE_HALF:  (*x) *= 2;  (*y) *= 2;  break;
    case SCREEN_SIZE_FULL:                  break;
#ifdef  SUPPORT_DOUBLE
    case SCREEN_SIZE_DOUBLE:    (*x) /= 2;  (*y) /= 2;  break;
#endif
    }
}


/****************************************************************************
 * キーのバインディング変更
 *  オプションや、メニューでの設定に基づき、キーバインドを変更する/戻す。
 *****************************************************************************/
void    keyboard_switch(void)
{
    int swap;
    const int *p;

#if USE_RETROACHIEVEMENTS
    if (quasi88_is_exec() || quasi88_is_pause()) {  /* エミュ中もオーバーレイ表示中もキーバインディング変更 */
#else
    if (quasi88_is_exec()) {    /* エミュ中は、キーバインディング変更 */
#endif

    clr_key_function();

    if (numlock_emu) {
        event_numlock_on();     /* Num lock を有効に (システム依存) */
    }

    if (tenkey_emu) {       /* 数字キーをテンキーに設定 */
        set_key_function( KEY88_1, KEY88_KP_1 );
        set_key_function( KEY88_2, KEY88_KP_2 );
        set_key_function( KEY88_3, KEY88_KP_3 );
        set_key_function( KEY88_4, KEY88_KP_4 );
        set_key_function( KEY88_5, KEY88_KP_5 );
        set_key_function( KEY88_6, KEY88_KP_6 );
        set_key_function( KEY88_7, KEY88_KP_7 );
        set_key_function( KEY88_8, KEY88_KP_8 );
        set_key_function( KEY88_9, KEY88_KP_9 );
        set_key_function( KEY88_0, KEY88_KP_0 );
    }

    if (cursor_key_mode) {      /* カーソルの入力割り当て変更 */
        if (cursor_key_mode == 1) p = cursor_key_assign_tenkey;
        else                      p = cursor_key_assign;

        set_key_function( KEY88_UP,    *p );    p ++;
        set_key_function( KEY88_DOWN,  *p );    p ++;
        set_key_function( KEY88_LEFT,  *p );    p ++;
        set_key_function( KEY88_RIGHT, *p );
    }

    /*
      システムからのマウス入力、およびジョイスティック入力が、
      QUASI88 のポートにどのように反映されるかの一覧。

             |  マウス移動    マウスボタン | ジョイスティック
      ---------------+--------------+--------------+-----------------
      MOUSE_NONE     | なし         | なし         | なし
      ---------------+--------------+--------------+-----------------
      MOUSE_JOYMOUSE | 方向を反映   | 押下を反映   | なし
      ---------------+--------------+--------------+-----------------
      MOUSE_MOUSE    | 移動量を反映 | 押下を反映   | なし
      ---------------+--------------+--------------+-----------------
      MOUSE_JOYSTICK | なし         | なし         | 押下を反映

      「なし」の欄は、QUASI88には反映されないので、自由にキー割り当て可能。
    */
    swap = FALSE;

    switch (mouse_mode) {       /* マウスの入力割り当て変更 */
    case MOUSE_NONE:
    case MOUSE_JOYSTICK:
        if (mouse_key_mode) {
        if (mouse_key_mode == 1) p = mouse_key_assign_tenkey;
        else                     p = mouse_key_assign;

        set_key_function( KEY88_MOUSE_UP,    *p );  p ++;
        set_key_function( KEY88_MOUSE_DOWN,  *p );  p ++;
        set_key_function( KEY88_MOUSE_LEFT,  *p );  p ++;
        set_key_function( KEY88_MOUSE_RIGHT, *p );  p ++;
        set_key_function( KEY88_MOUSE_L,     *p );  p ++;
        set_key_function( KEY88_MOUSE_R,     *p );
        if (mouse_key_mode == 1) {
            swap = mouse_swap_button;
        }
        }
        break;

    case MOUSE_JOYMOUSE:
        set_key_function( KEY88_MOUSE_UP,    KEY88_PAD1_UP    );
        set_key_function( KEY88_MOUSE_DOWN,  KEY88_PAD1_DOWN  );
        set_key_function( KEY88_MOUSE_LEFT,  KEY88_PAD1_LEFT  );
        set_key_function( KEY88_MOUSE_RIGHT, KEY88_PAD1_RIGHT );
        set_key_function( KEY88_MOUSE_L,     KEY88_PAD1_A     );
        set_key_function( KEY88_MOUSE_R,     KEY88_PAD1_B     );
        swap = mouse_swap_button;
        break;

    case MOUSE_MOUSE:
        set_key_function( KEY88_MOUSE_L,     KEY88_PAD1_A     );
        set_key_function( KEY88_MOUSE_R,     KEY88_PAD1_B     );
        swap = mouse_swap_button;
        break;
    }
    if (swap) {
        swap_key_function(KEY88_MOUSE_L, KEY88_MOUSE_R);
    }


    swap = FALSE;

    switch (mouse_mode) {       /* ジョイスティック入力割り当て変更 */
    case MOUSE_NONE:
    case MOUSE_MOUSE:
    case MOUSE_JOYMOUSE:
        if (joy_key_mode) {
        if (joy_key_mode == 1) p = joy_key_assign_tenkey;
        else                   p = joy_key_assign;

        set_key_function( KEY88_PAD1_UP,    *p );   p ++;
        set_key_function( KEY88_PAD1_DOWN,  *p );   p ++;
        set_key_function( KEY88_PAD1_LEFT,  *p );   p ++;
        set_key_function( KEY88_PAD1_RIGHT, *p );   p ++;
        set_key_function( KEY88_PAD1_A,     *p );   p ++;
        set_key_function( KEY88_PAD1_B,     *p );   p ++;
        set_key_function( KEY88_PAD1_C,     *p );   p ++;
        set_key_function( KEY88_PAD1_D,     *p );   p ++;
        set_key_function( KEY88_PAD1_E,     *p );   p ++;
        set_key_function( KEY88_PAD1_F,     *p );   p ++;
        set_key_function( KEY88_PAD1_G,     *p );   p ++;
        set_key_function( KEY88_PAD1_H,     *p );   p ++;
        if (joy_key_mode == 1) {
            swap = joy_swap_button;
        }
        }
        break;

    case MOUSE_JOYSTICK:
        /* KEY88_PAD1_C 〜 KEY88_PAD1_H は割り当てなし */
        swap = joy_swap_button;
        break;
    }
    if (swap) {
        swap_key_function(KEY88_PAD1_A, KEY88_PAD1_B);
    }


    if (joy2_key_mode) {          /* ジョイスティック２入力割り当て変更 */
        if (joy2_key_mode == 1) p = joy_key_assign_tenkey;
        else                    p = joy2_key_assign;

        set_key_function( KEY88_PAD2_UP,    *p );   p ++;
        set_key_function( KEY88_PAD2_DOWN,  *p );   p ++;
        set_key_function( KEY88_PAD2_LEFT,  *p );   p ++;
        set_key_function( KEY88_PAD2_RIGHT, *p );   p ++;
        set_key_function( KEY88_PAD2_A,     *p );   p ++;
        set_key_function( KEY88_PAD2_B,     *p );   p ++;
        set_key_function( KEY88_PAD2_C,     *p );   p ++;
        set_key_function( KEY88_PAD2_D,     *p );   p ++;
        set_key_function( KEY88_PAD2_E,     *p );   p ++;
        set_key_function( KEY88_PAD2_F,     *p );   p ++;
        set_key_function( KEY88_PAD2_G,     *p );   p ++;
        set_key_function( KEY88_PAD2_H,     *p );   p ++;
        if (joy2_key_mode == 1) {
        swap = joy2_swap_button;
        }
    }
    if (swap) {
        swap_key_function(KEY88_PAD2_A, KEY88_PAD2_B);
    }


                    /* ファンクションキーの機能割り当て */
    set_key_function( KEY88_F1,  function_f[  1 ] );
    set_key_function( KEY88_F2,  function_f[  2 ] );
    set_key_function( KEY88_F3,  function_f[  3 ] );
    set_key_function( KEY88_F4,  function_f[  4 ] );
    set_key_function( KEY88_F5,  function_f[  5 ] );
    set_key_function( KEY88_F6,  function_f[  6 ] );
    set_key_function( KEY88_F7,  function_f[  7 ] );
    set_key_function( KEY88_F8,  function_f[  8 ] );
    set_key_function( KEY88_F9,  function_f[  9 ] );
    set_key_function( KEY88_F10, function_f[ 10 ] );
    set_key_function( KEY88_F11, function_f[ 11 ] );
    set_key_function( KEY88_F12, function_f[ 12 ] );
    set_key_function( KEY88_F13, function_f[ 13 ] );
    set_key_function( KEY88_F14, function_f[ 14 ] );
    set_key_function( KEY88_F15, function_f[ 15 ] );
    set_key_function( KEY88_F16, function_f[ 16 ] );
    set_key_function( KEY88_F17, function_f[ 17 ] );
    set_key_function( KEY88_F18, function_f[ 18 ] );
    set_key_function( KEY88_F19, function_f[ 19 ] );
    set_key_function( KEY88_F20, function_f[ 20 ] );


    } else {            /* メニュー中などは、キーバインディング戻す */

    romaji_clear();         /* ローマ字変換ワークを初期化 */

    event_numlock_off();        /* Num lock を無効に (システム依存) */
    }


                /* マウス入力に備えて、マウス位置を再設定 */
    event_get_mouse_pos(&mouse_x, &mouse_y);
    mouse_movement_adjust(&mouse_x, &mouse_y);

    mouse_dx = 0;
    mouse_dy = 0;

    mouse_sx = 0;
    mouse_sy = 0;
    mouse_sb = 0;
}



/****************************************************************************
 * キー入力のワーク(ポート)初期化
 *****************************************************************************/
void    keyboard_reset(void)
{
    size_t i;
    for (i=0; i<sizeof(key_scan); i++) key_scan[i] = 0xff;

    romaji_init();
}



/****************************************************************************
 * キー入力をワーク(ポート)に反映     エミュモードのみ
 *****************************************************************************/
static  void    record_playback(void);

void    keyboard_update(void)
{
    int status;

    /* ローマ字入力モード時は、ローマ字変換後のカナを key_scan[] に反映 */

    if (romaji_input_mode) romaji_output();


    /* マウス入力を key_scan[] に反映 */

    switch (mouse_mode) {

    case MOUSE_NONE:
    case MOUSE_JOYSTICK:
    if (mouse_key_mode == 0) {  /* マウスキー割り当て無しなら */
        mouse_dx = 0;       /* マウス移動量は不要         */
        mouse_dy = 0;
        break;
    }
    /* FALLTHROUGH */       /* マウスキー割り当て有りか、 */
                    /* ジョイスティックモードなら */
    case MOUSE_JOYMOUSE:        /* マウス移動量をポートに反映 */
    if (mouse_dx == 0) {
        if      (mouse_dy == 0) status = 0;     /* ---- */
        else if (mouse_dy >  0) status = (PadD);    /* ↓   */
        else                    status = (PadU);    /* ↑   */
    } else if (mouse_dx > 0) {
        int a = mouse_dy * 100 / mouse_dx;
        if      (a >  241) status = (PadD);     /* ↓   */
        else if (a >   41) status = (PadD | PadR);  /* ↓→ */
        else if (a >  -41) status = (       PadR);  /*   → */
        else if (a > -241) status = (PadU | PadR);  /* ↑→ */
        else               status = (PadU);     /* ↑   */
    } else {
        int a = -mouse_dy * 100 / mouse_dx;
        if      (a >  241) status = (PadD);     /* ↓   */
        else if (a >   41) status = (PadD | PadL);  /* ↓← */
        else if (a >  -41) status = (       PadL);  /*   ← */
        else if (a > -241) status = (PadU | PadL);  /* ↑← */
        else               status = (PadU);     /* ↑   */
    }

    quasi88_mouse( KEY88_MOUSE_UP,    (status & PadU) );
    quasi88_mouse( KEY88_MOUSE_DOWN,  (status & PadD) );
    quasi88_mouse( KEY88_MOUSE_LEFT,  (status & PadL) );
    quasi88_mouse( KEY88_MOUSE_RIGHT, (status & PadR) );

    mouse_dx = 0;       /* ポート反映後は移動量クリア */
    mouse_dy = 0;
    break;

    case MOUSE_MOUSE:       /* マウス装着時なら、         */
    break;          /* マウス移動量は保持しておく */
    }


    /* キー操作(scan_key[], mouse_dx, mouse_dy) を記録／再生 */

    record_playback();


    /* key_scan[0x0f] (ジョイスティック) をサウンド汎用入力ポートに反映 */

    switch (mouse_mode) {
    case MOUSE_NONE:
    sound_reg[ 0x0e ] = 0xff;
    sound_reg[ 0x0f ] = 0xff;
    break;

    case MOUSE_MOUSE:
    sound_reg[ 0x0f ] = (IS_JOY_STATUS() >> 4) | 0xfc;
    break;

    case MOUSE_JOYMOUSE:
    case MOUSE_JOYSTICK:
    sound_reg[ 0x0e ] = (IS_JOY_STATUS()     ) | 0xf0;
    sound_reg[ 0x0f ] = (IS_JOY_STATUS() >> 4) | 0xfc;
    /*printf("%02x\n",sound_reg[ 0x0e ]&0xff);*/
    break;
    }

}



/****************************************************************************
 * シリアルマウスからのシリアル入力値取得
 *
 *  ストップビット 1、データ長 7bit、パリティなし、ボーレート 1200bps？
 *          +--+--+--+--+--+--+--+--+
 *  1バイト目   |０|１|Ｌ|Ｒ|Y7|Y6|X7|X6|
 *          +--+--+--+--+--+--+--+--+
 *          +--+--+--+--+--+--+--+--+
 *  2バイト目   |０|０|X5|X4|X3|X2|X1|X0|
 *          +--+--+--+--+--+--+--+--+
 *          +--+--+--+--+--+--+--+--+
 *  3バイト目   |０|０|Y5|Y4|Y3|Y2|Y1|Y0|
 *          +--+--+--+--+--+--+--+--+
 *
 *  Ｌ Ｒ            … 左ボタン、右ボタン (押下時、1)
 *  X7X6X5X4X3X2X1X0 … X方向移動量 (符合付き)
 *  Y7Y6Y5Y4Y3Y2Y1Y0 … X方向移動量 (符合付き)
 *
 *****************************************************************************/
static int serial_mouse_x;
static int serial_mouse_y;
static int serial_mouse_step;

void    init_serial_mouse_data(void)
{
    serial_mouse_x = 0;
    serial_mouse_y = 0;
    serial_mouse_step = 0;
}
int get_serial_mouse_data(void)
{
    int result;
    switch (serial_mouse_step) {
    case 0:
    if      (mouse_sx >  127) serial_mouse_x =  127;
    else if (mouse_sx < -127) serial_mouse_x = -127;
    else                      serial_mouse_x = mouse_sx;
    if      (mouse_sy >  127) serial_mouse_y =  127;
    else if (mouse_sy < -127) serial_mouse_y = -127;
    else                      serial_mouse_y = mouse_sy;
    mouse_sx = 0;
    mouse_sy = 0;

    result = 
        0x40 | 
        mouse_sb |
        ((serial_mouse_y & 0xc0) >> 4) |
        ((serial_mouse_x & 0xc0) >> 6);
    /*printf("%02x\n", result);*/
    break;

    case 1:
    result = (serial_mouse_x & 0x3f);
    /*printf("   %02x\n", result);*/
    break;

    default:
    result = (serial_mouse_y & 0x3f);
    /*printf("      %02x\n", result);*/
    break;
    }
    if (++serial_mouse_step >= 3) serial_mouse_step = 0;
    return result;
}



/****************************************************************************
 * キー入力 記録・再生
 *****************************************************************************/
void    key_record_playback_init(void)
{
  int i;

  for( i=0; i<16; i++ ) key_record.key[i]     = 0xff;
  key_record.dx_h = 0;
  key_record.dx_l = 0;
  key_record.dy_h = 0;
  key_record.dy_l = 0;
  key_record.image[0] = -1;
  key_record.image[1] = -1;


  fp_pb  = NULL;
  fp_rec = NULL;

  if( file_pb && file_pb[0] ){          /* 再生用ファイルをオープン */

    fp_pb = osd_fopen( FTYPE_KEY_PB, file_pb, "rb" );

    if( fp_pb ){
      if( verbose_proc )
    printf( "Key-Input Playback file <%s> ... OK\n", file_pb );
    }else{
      printf( "Can't open <%s>\nKey-Input PlayBack is invalid\n", file_pb );
    }
  }

  if( file_rec && file_rec[0] ){        /* 記録用ファイルをオープン */

    fp_rec = osd_fopen( FTYPE_KEY_REC, file_rec, "wb" );

    if( fp_rec ){
      if( verbose_proc )
    printf( "Key-Input Record file <%s> ... OK\n", file_rec );
    }else{
      printf( "Can't open <%s>\nKey-Input Record is invalid\n", file_rec );
    }
  }
}

void    key_record_playback_exit(void)
{
  if( fp_pb ){
    osd_fclose( fp_pb );
    fp_pb = NULL;
    if( file_pb ) file_pb[0] = '\0';
  }
  if( fp_rec ){
    osd_fclose( fp_rec );
    fp_rec = NULL;
    if( file_rec ) file_rec[0] = '\0';
  }
}



/*----------------------------------------------------------------------
 * キー入力 記録・再生処理
 *----------------------------------------------------------------------*/
static  void    record_playback(void)
{
  int i, img;

  if (quasi88_is_exec() == FALSE) return;

  if( fp_rec ){
    for( i=0; i<0x10; i++ )
      key_record.key[i] = key_scan[i];

    key_record.dx_h = (mouse_dx>>8) & 0xff;
    key_record.dx_l =  mouse_dx     & 0xff;
    key_record.dy_h = (mouse_dy>>8) & 0xff;
    key_record.dy_l =  mouse_dy     & 0xff;

    for( i=0; i<2; i++ ){
      if( disk_image_exist( i ) &&
      drive_check_empty( i ) == FALSE )
    img = disk_image_selected(i) + 1;
      else
    img = -1;
      if( key_record.image[i] != img ) key_record.image[i] = img;
      else                             key_record.image[i] = 0;
    }

    if( osd_fwrite( &key_record, sizeof(char), sizeof(key_record), fp_rec )
                            == sizeof(key_record)){
      ;
    }else{
      printf( "Can't write Record file <%s>\n", file_rec );
      osd_fclose( fp_rec );
      fp_rec = NULL;
    }
  }


  if( fp_pb ){

    if( osd_fread( &key_record, sizeof(char), sizeof(key_record), fp_pb )
                            == sizeof(key_record)){
      for( i=0; i<0x10; i++ )
    key_scan[i] = key_record.key[i];

      mouse_dx  = (int)key_record.dx_h << 8;
      mouse_dx |=      key_record.dx_l;
      mouse_dy  = (int)key_record.dy_h << 8;
      mouse_dy |=      key_record.dy_l;

      for( i=0; i<2; i++ ){
    if( key_record.image[i]==-1 ){
      drive_set_empty( i );
    }else if( disk_image_exist( i ) &&
          key_record.image[i] > 0 &&
          key_record.image[i] <= disk_image_num( i ) ){
      drive_unset_empty( i );
      disk_change_image( i, key_record.image[i]-1 );
    }
      }

    }else{
      printf(" (( %s : Playback file EOF ))\n", file_pb );
      status_message( 1, STATUS_INFO_TIME, "Playback  [EOF]" );
      osd_fclose( fp_pb );
      fp_pb = NULL;
    }
  }

  /* シリアルマウスは、反映タイミングが VSYNC とは異なるので、適用しない */
}



/****************************************************************************
 * 汎用 I/O ポートのワーク初期化
 *****************************************************************************/
/* 汎用 I/O ポートが、出力→入力 に切り替わった時の処理 */
void    keyboard_jop1_reset(void)
{
#if 0   /* - - - - - - - - - - - - - - - - - マウス関連のワークを初期化する  */

#if 0                       /* マウス座標も初期化したほう*/
  event_get_mouse_pos(&mouse_x, &mouse_y);  /* がいいかもしれないけど、  */
  mouse_movement_adjust(&mouse_x, &mouse_y);    /* そこまでしなくてもいっか  */
#endif
  mouse_dx = 0;
  mouse_dy = 0;

#endif  /* - - - - - - - - でも、実際にはこれをしなくても実害はでないと思う。*/
    /*          実害があったので、ver 0.6.2 以降では削除     */

    jop1_step = 0;
    jop1_dx = 0;
    jop1_dy = 0;
}



/****************************************************************************
 * 汎用 I/O ポート ストローブ ON/OFF
 *****************************************************************************/

/* ストローブ処理は 720state 以内に完了させる。   (×1.25はマージン)  */
/*  (8MHzの場合は 1440state なんだけどまあいっか)         */
#define JOP1_STROBE_LIMIT       ((int)(720 * 1.25))

void    keyboard_jop1_strobe(void)
{
  if( mouse_mode==MOUSE_MOUSE       &&      /* マウス 有効 */
      (sound_reg[ 0x07 ] & 0x80)==0 ){      /* 汎用I/O 入力設定時 */

    {
      int now = state_of_cpu + z80main_cpu.state0;

      /*int interval = now - jop1_time;
    if( interval < 0 ) interval += state_of_vsync;
    printf("JOP %d (%d)\n",jop1_step,interval);*/

      if( jop1_step == 2 ){
    int interval = now - jop1_time;
    if( interval < 0 ) interval += state_of_vsync;
    if( interval > JOP1_STROBE_LIMIT ) keyboard_jop1_reset();
      }

      jop1_time = now;
    }

    switch( jop1_step ){

    case 0:     /* 最初のストローブ(ON)で、マウス移動量の値を確定し  */
            /* 2回目以降のストローブで、この確定した値を転送する */
        {
          int dx = mouse_dx;            /* x 方向 変位 */
          int dy = mouse_dy;            /* y 方向 変位 */

#if 1           /* 変位を±127の範囲内にクリッピングする */
          int f = 0;

            /* x、yのうち 変位が ±127を超えている方を探す。     */
            /* ともに超えてたら、変位の大きいほうが超えたとする。*/
          if( dx < -127 || 127 < dx ) f |= 0x01;
          if( dy < -127 || 127 < dy ) f |= 0x02;
          if( f==0x03 ){
            if( ABS(dx) > ABS(dy) ) f = 0x01;
            else                    f = 0x02;
          }
          if( f==0x01 ){        /* x変位が ±127を超えた場合 */
                            /* x変位をmax値にしてyを補正 */
            dy = 127 * SGN(dx) * dy / dx;
            dx = 127 * SGN(dx);
          }
          else if( f==0x02 ){       /* y変位が ±127を超えた場合 */
                        /* y変位をmax値にしてxを補正 */
            dx = 127 * SGN(dy) * dx / dy;
            dy = 127 * SGN(dy);
          }
#endif
          mouse_dx -= dx;
          mouse_dy -= dy;

          jop1_dx = dx;
          jop1_dy = dy;
          /*printf("%d,%d\n",jop1_dx,jop1_dy);*/
        }
        sound_reg[ 0x0e ] = ((-jop1_dx)>>4) & 0x0f; break;
    case 1: sound_reg[ 0x0e ] =  (-jop1_dx)     & 0x0f; break;
    case 2: sound_reg[ 0x0e ] = ((-jop1_dy)>>4) & 0x0f; break;
    case 3: sound_reg[ 0x0e ] =  (-jop1_dy)     & 0x0f; break;
    }

  }else{
            sound_reg[ 0x0e ] = 0xff;
  }

  jop1_step = (jop1_step + 1) & 0x03;
}



/***********************************************************************
 * ソフトウェアNumLock
 * カナ（ローマ字）キーロック
 ************************************************************************/
void    quasi88_cfg_key_numlock(int on)
{
    if (((numlock_emu)          && (on == FALSE)) ||
    ((numlock_emu == FALSE) && (on))) {

    if (numlock_emu) event_numlock_off();
    numlock_emu ^= 1;
    keyboard_switch();
    }
}
void    quasi88_cfg_key_kana(int on)
{
    if (((IS_KEY88_PRESS(KEY88_KANA))          && (on == FALSE)) ||
    ((IS_KEY88_PRESS(KEY88_KANA) == FALSE) && (on))) {

    KEY88_TOGGLE(KEY88_KANA);
    romaji_input_mode = FALSE;
    }
}
void    quasi88_cfg_key_romaji(int on)
{
    if (((IS_KEY88_PRESS(KEY88_KANA))          && (on == FALSE)) ||
    ((IS_KEY88_PRESS(KEY88_KANA) == FALSE) && (on))) {

    KEY88_TOGGLE(KEY88_KANA);
    if (IS_KEY88_PRESS(KEY88_KANA)) {
        romaji_input_mode = TRUE;
        romaji_clear();
    } else {
        romaji_input_mode = FALSE;
    }
    }
}



/****************************************************************************
 * 【機種依存部より、キー押下時に呼び出される】
 *
 *  code は、キーコードで、 KEY88_SPACE <= code <= KEY88_SHIFTR
 *  on   は、キー押下なら真、キー開放なら偽
 *****************************************************************************/
static  void    do_lattertype(int code, int on);
static  int do_func(int func, int on);

void    quasi88_key(int code, int on)
{
    if (quasi88_is_exec()) {        /*===================================*/

    if (key_func[ code ]) {         /* 特殊処理割り当て済の場合 */
        code = do_func(key_func[ code ], on);   /*  特殊機能処理を実行  */
        if (code == 0) return;          /*  戻値は 新キーコード */
    }

    if (romaji_input_mode && on) {      /* ローマ字入力モードの場合 */
        if (romaji_input(code) == 0) {  /*  変換処理を実行      */
        return;
        }
    }

    if (IS_KEY88_LATTERTYPE(code)) {    /* 後期型キーボードの処理   */
        do_lattertype(code, on);
    }
                        /* キー押下をIOポートに反映 */
    if (on) KEY88_PRESS(code);
    else    KEY88_RELEASE(code);

    /*
      if(code == KEY88_RETURNL){
      if (on) printf("+%d\n",code);
      else    printf("-%d\n",code);
      }
    */

    } else
    if (quasi88_is_menu()) {        /*===================================*/
    /*printf("%d\n",code);*/
                        /* メニュー用の読み替え… */
    switch (code) {
    case KEY88_KP_0:    code = KEY88_0;     break;
    case KEY88_KP_1:    code = KEY88_1;     break;
    case KEY88_KP_2:    code = KEY88_2;     break;
    case KEY88_KP_3:    code = KEY88_3;     break;
    case KEY88_KP_4:    code = KEY88_4;     break;
    case KEY88_KP_5:    code = KEY88_5;     break;
    case KEY88_KP_6:    code = KEY88_6;     break;
    case KEY88_KP_7:    code = KEY88_7;     break;
    case KEY88_KP_8:    code = KEY88_8;     break;
    case KEY88_KP_9:    code = KEY88_9;     break;
    case KEY88_KP_MULTIPLY: code = KEY88_ASTERISK;  break;
    case KEY88_KP_ADD:  code = KEY88_PLUS;  break;
    case KEY88_KP_EQUAL:    code = KEY88_EQUAL; break;
    case KEY88_KP_COMMA:    code = KEY88_COMMA; break;
    case KEY88_KP_PERIOD:   code = KEY88_PERIOD;    break;
    case KEY88_KP_SUB:  code = KEY88_MINUS; break;
    case KEY88_KP_DIVIDE:   code = KEY88_SLASH; break;

    case KEY88_INS_DEL: code = KEY88_BS;    break;
      /*case KEY88_DEL:     code = KEY88_BS;    break;  DELのまま */
    case KEY88_KETTEI:  code = KEY88_SPACE; break;
    case KEY88_HENKAN:  code = KEY88_SPACE; break;
    case KEY88_RETURNL: code = KEY88_RETURN;    break;
    case KEY88_RETURNR: code = KEY88_RETURN;    break;
    case KEY88_SHIFTL:  code = KEY88_SHIFT; break;
    case KEY88_SHIFTR:  code = KEY88_SHIFT; break;
    }
    if (on) q8tk_event_key_on(code);
    else    q8tk_event_key_off(code);

    } else
    if (quasi88_is_pause()) {       /*===================================*/
#if USE_RETROACHIEVEMENTS
        /* オーバーレイ表示中はキー押下をIOポートに反映 */
        if (on) KEY88_PRESS(code);
        else    KEY88_RELEASE(code);
#endif
    if (on) {
        if (key_func[ code ]) {
        if (key_func[ code ] == FN_MENU) {

            pause_event_key_on_menu();

        } else if (key_func[ code ] == FN_PAUSE) {

            pause_event_key_on_esc();

        }

        } else if (code == KEY88_ESC) {

        pause_event_key_on_esc();

        }

    }
    }
}





/*----------------------------------------------------------------------
 * 後期型キーボードのキー押下/開放時の処理
 *      シフトキーや、同一機能キーのポートを同時処理する
 *----------------------------------------------------------------------*/
static  void    do_lattertype(int code, int on)
{
                  /* KEY88_XXX を KEY88_EXT_XXX に変換 */
    int code2 = code - KEY88_F6 + KEY88_END;

    switch (code) {
    case KEY88_F6:
    case KEY88_F7:
    case KEY88_F8:
    case KEY88_F9:
    case KEY88_F10:
    case KEY88_INS:     /* KEY88_SHIFTR, KEY88_SHIFTL は ? */
    if (on) { KEY88_PRESS  (KEY88_SHIFT);  KEY88_PRESS  (code2); }
    else    { KEY88_RELEASE(KEY88_SHIFT);  KEY88_RELEASE(code2); }
    break;

    case KEY88_DEL:
    case KEY88_BS:
    case KEY88_HENKAN:
    case KEY88_KETTEI:
    case KEY88_ZENKAKU:
    case KEY88_PC:
    case KEY88_RETURNR:     /* KEY88_RETURNL は ? */
    case KEY88_RETURNL:     /* KEY88_RETURNR は ? */
    case KEY88_SHIFTR:      /* KEY88_SHIFTL  は ? */
    case KEY88_SHIFTL:      /* KEY88_SHIFTR  は ? */
    if (on) { KEY88_PRESS  (code2); }
    else    { KEY88_RELEASE(code2); }
    break;

    default:
    return;
    }
}

/*----------------------------------------------------------------------
 * ファンクションキーに割り当てた機能の処理
 *      戻り値は、新たなキーコード (0ならキー押下なし扱い)
 *----------------------------------------------------------------------*/
static void change_framerate(int sign);
static void change_volume(int sign);
static void change_wait(int sign);
static void change_max_speed(int new_speed);
static void change_max_clock(double new_clock);
static void change_max_boost(int new_boost);

static  int do_func(int func, int on)
{
    switch (func) {
    case FN_FUNC:               /* 機能なし */
    return 0;

    case FN_FRATE_UP:               /* フレーム */
    if (on) change_framerate(+1);
    return 0;
    case FN_FRATE_DOWN:             /* フレーム */
    if (on) change_framerate(-1);
    return 0;

    case FN_VOLUME_UP:              /* 音量 */
    if (on) change_volume(-1);
    return 0;
    case FN_VOLUME_DOWN:            /* 音量 */
    if (on) change_volume(+1);
    return 0;

    case FN_PAUSE:              /* 一時停止 */
    if (on) quasi88_pause();
    return 0;

    case FN_RESIZE:             /* リサイズ */
    if (on) {
        quasi88_cfg_set_size_large();
    }
    return 0;

    case FN_NOWAIT:             /* ウエイト */
    if (on) change_wait(0);
    return 0;
    case FN_SPEED_UP:
    if (on) change_wait(+1);
    return 0;
    case FN_SPEED_DOWN:
    if (on) change_wait(-1);
    return 0;

    case FN_FULLSCREEN:             /* 全画面切り替え */
    if (on) {
        if (quasi88_cfg_can_fullscreen()) {
        int now  = quasi88_cfg_now_fullscreen();
        int next = (now) ? FALSE : TRUE;
        quasi88_cfg_set_fullscreen(next);
        }
    }
    return 0;

    case FN_IMAGE_NEXT1:            /* DRIVE1: イメージ変更 */
    if (on) quasi88_disk_image_empty(DRIVE_1);
    else    quasi88_disk_image_next(DRIVE_1);
    return 0;
    case FN_IMAGE_PREV1:
    if (on) quasi88_disk_image_empty(DRIVE_1);
    else    quasi88_disk_image_prev(DRIVE_1);
    return 0;
    case FN_IMAGE_NEXT2:            /* DRIVE2: イメージ変更 */
    if (on) quasi88_disk_image_empty(DRIVE_2);
    else    quasi88_disk_image_next(DRIVE_2);
    return 0;
    case FN_IMAGE_PREV2:
    if (on) quasi88_disk_image_empty(DRIVE_2);
    else    quasi88_disk_image_prev(DRIVE_2);
    return 0;

    case FN_NUMLOCK:                /* NUM lock */
    if (on) {
        if (numlock_emu) event_numlock_off();
        numlock_emu ^= 1;
        keyboard_switch();
    }
    return 0;

    case FN_RESET:              /* リセット */
    if (on) quasi88_reset(NULL);
    return 0;

    case FN_KANA:               /* カナ */
    if (on) {
        KEY88_TOGGLE(KEY88_KANA);
        romaji_input_mode = FALSE;
    }
    return 0;

    case FN_ROMAJI:             /* カナ(ローマ字) */
    if (on) {
        KEY88_TOGGLE(KEY88_KANA);
        if (IS_KEY88_PRESS(KEY88_KANA)) {
        romaji_input_mode = TRUE;
        romaji_clear();
        } else {
        romaji_input_mode = FALSE;
        }
    }
    return 0;

    case FN_CAPS:               /* CAPS */
    if (on) {
        KEY88_TOGGLE(KEY88_CAPS);
    }
    return 0;

    case FN_SNAPSHOT:               /* スクリーンスナップショット*/
    if (on) quasi88_screen_snapshot();
    return 0;

    case FN_MAX_SPEED:
    if (on) change_max_speed(fn_max_speed);
    return 0;
    case FN_MAX_CLOCK:
    if (on) change_max_clock(fn_max_clock);
    return 0;
    case FN_MAX_BOOST:
    if (on) change_max_boost(fn_max_boost);
    return 0;

    case FN_STATUS:             /* FDDステータス表示 */
    if (on) {
        if (quasi88_cfg_can_showstatus()) {
        int now  = quasi88_cfg_now_showstatus();
        int next = (now) ? FALSE : TRUE;
        quasi88_cfg_set_showstatus(next);
        }
    }
    return 0;
    case FN_MENU:               /* メニューモード */
    if (on) quasi88_menu();
    return 0;

    }
    return func;
}



/*
 * フレームスキップ数変更 : -frameskip の値を、変更する。
 */
static void change_framerate(int sign)
{
    int i, rate;
                /*     framerate up ←                  */
    static const int list[] = { 1, 2, 3, 4, 5, 6, 10, 12, 15, 20, 30, 60, };
                /*                  → framerate down   */

    rate = quasi88_cfg_now_frameskip_rate();

    if (sign > 0) {     /* framerate up */

    for (i=COUNTOF(list)-1; i>=0; i--) {
        if (list[i] < rate) { rate = list[i]; break; }
    }
    if (i < 0) rate = list[COUNTOF(list)-1];

    } else {            /* framerate down*/

    for (i=0; i<COUNTOF(list); i++) {
        if (rate < list[i]) { rate = list[i]; break; }
    }
    if (i == COUNTOF(list)) rate = list[0];
    }

    quasi88_cfg_set_frameskip_rate(rate);
}



/*
 * ボリューム変更 : -vol の値を、変更する。
 */
static void change_volume(int sign)
{
#ifdef USE_SOUND
    int diff, vol;
    char str[32];

    if (xmame_has_sound()) {
    if (xmame_has_mastervolume()) {
        diff = (sign > 0) ? +1 : ((sign < 0) ? -1 : 0);
        if (diff){
        vol = xmame_cfg_get_mastervolume() + diff;
        if (vol >   0) vol = 0;
        if (vol < -32) vol = -32;
        xmame_cfg_set_mastervolume(vol);
        }
    
        sprintf(str, "VOLUME  %3d[db]", xmame_cfg_get_mastervolume());
        status_message(1, STATUS_INFO_TIME, str);
        /* 変更した後は、しばらく画面に音量を表示させる */
    }
    }
#endif
}



/*
 * ウェイト量変更 : -nowait, -speed の値を、変更する。
 */
static void change_wait(int sign)
{
    int w;

    if (sign == 0) {        /* ウェイト有無の変更 */

    w = quasi88_cfg_now_no_wait();
    w ^= 1;
    quasi88_cfg_set_no_wait(w);

    } else {            /* ウェイト比率の増減 */

    w = quasi88_cfg_now_wait_rate();

    if (sign < 0) {
        w -= 10;
        if (w < 10) w = 10;
    } else {
        w += 10;
        if (w > 200) w = 200;
    }

    quasi88_cfg_set_wait_rate(w);
    }
}


/*
 * 速度変更 (speed/clock/boost)
 */
static void change_max_speed(int new_speed)
{
    char str[32];

    if (! (5 <= new_speed || new_speed <= 5000)) {
    new_speed = 1600;
    }

    if (wait_rate < new_speed) wait_rate = new_speed;
    else                       wait_rate = 100;

    wait_vsync_switch();

    no_wait = 0;

    sprintf(str, "WAIT  %4d[%%]", wait_rate);
    status_message(1, STATUS_INFO_TIME, str);
}
static void change_max_clock(double new_clock)
{
    double def_clock = (boot_clock_4mhz ? CONST_4MHZ_CLOCK : CONST_8MHZ_CLOCK);
    char str[32];

    if (! (0.1 <= new_clock && new_clock < 1000.0)) {
    new_clock = CONST_4MHZ_CLOCK * 16;
    }

    if (cpu_clock_mhz < new_clock) cpu_clock_mhz = new_clock;
    else                           cpu_clock_mhz = def_clock;
    interval_work_init_all();

    sprintf(str, "CLOCK %8.4f[MHz]", cpu_clock_mhz);
    status_message(1, STATUS_INFO_TIME, str);
}
static void change_max_boost(int new_boost)
{
    char str[32];

    if (! (1 <= new_boost || new_boost <= 100)) {
    new_boost = 16;
    }

    if (boost < new_boost) boost_change(new_boost);
    else                   boost_change(1);
  
    sprintf(str, "BOOST [x%2d]", boost);
    status_message(1, STATUS_INFO_TIME, str);
}













/******************************************************************************
 * 【機種依存部より、マウスのボタン押下時に呼び出される】
 *
 *  code は、キーコードで、 KEY88_MOUSE_L <= code <= KEY88_MOUSE_DOWN
 *  on   は、キー押下なら真、キー開放なら偽
 *****************************************************************************/
void    quasi88_mouse(int code, int on)
{
    if (quasi88_is_exec()) {        /*===================================*/
    /*
      if (on) printf("+%d\n",code);
      else    printf("-%d\n",code);
    */

    if (key_func[ code ]) {         /* キーコードの読み替え      */
        code = do_func(key_func[ code ], on);   /* 特殊機能があれば、    */
        if (code == 0) return;          /* 処理されるので注意    */
    }                       /* ローマ字入力や、      */
                            /* 後期型キーは処理しない*/

    if (on) KEY88_PRESS(code);      /* I/Oポートへ反映           */
    else    KEY88_RELEASE(code);

    /* シリアルマウス用のワークセット */
    if (code == KEY88_MOUSE_L) {
        if (on) mouse_sb |=  0x20;
        else    mouse_sb &= ~0x20;
    }
    if (code == KEY88_MOUSE_R) {
        if (on) mouse_sb |=  0x10;
        else    mouse_sb &= ~0x10;
    }

    }
    else if (quasi88_is_menu()) {   /*===================================*/

    if (on) q8tk_event_mouse_on(code);
    else    q8tk_event_mouse_off(code);
    }

    if (on) screen_attr_mouse_click();  /* マウス自動グラブ…ここでグラブ */
}




/******************************************************************************
 * 【機種依存部より、ジョイスティック入力時に呼び出される】
 *
 *  code は、キーコードで、 KEY88_PAD1_UP <= code <= KEY88_PAD1_H
 *  on   は、キー押下なら真、キー開放なら偽
 *****************************************************************************/
void    quasi88_pad(int code, int on)
{
    if (quasi88_is_exec()) {        /*===================================*/

    if (key_func[ code ]) {         /* キーコードの読み替え      */
        code = do_func(key_func[ code ], on);   /* 特殊機能があれば、    */
        if (code == 0) return;          /* 処理されるので注意    */
    }                       /* ローマ字入力や、      */
                            /* 後期型キーは処理しない*/

    if (on) KEY88_PRESS(code);      /* I/Oポートへ反映           */
    else    KEY88_RELEASE(code);

    }
}






/****************************************************************************
 * 【機種依存部より、マウス移動時に呼び出される。】
 *
 *  abs_coord が、真なら、x,y はマウスの移動先の座標を示す。
 *      座標は、graph_setup() の戻り値にて応答した 画面サイズ
 *      width × height に対する値をセットすること。
 *      (なお、範囲外の値でも可とする)
 *
 *  abs_coord が 偽なら、x,y はマウスの移動量を示す。
 *****************************************************************************/
void    quasi88_mouse_move(int x, int y, int abs_coord)
{
    if (abs_coord) {
    mouse_movement_adjust(&x, &y);
    } else {
    /* サイズによる補正は？ */
    }


    if (quasi88_is_exec()) {        /*===================================*/
    if (abs_coord) {
        mouse_dx += x - mouse_x;
        mouse_dy += y - mouse_y;
        mouse_x = x;
        mouse_y = y;

    } else {

        /* マウスの速度調整 */
        x = x * mouse_sensitivity / 100;
        y = y * mouse_sensitivity / 100;

        mouse_dx += x;
        mouse_dy += y;
    }
    mouse_sx = mouse_dx;
    mouse_sy = mouse_dy;

    }
    else if (quasi88_is_menu()) {   /*===================================*/

    if (abs_coord) {
        mouse_x = x;
        mouse_y = y;

    } else {
        mouse_x += x;
        if (mouse_x <   0) mouse_x = 0;
        if (mouse_x > 640) mouse_x = 640;

        mouse_y += y;
        if (mouse_y <   0) mouse_y = 0;
        if (mouse_y > 400) mouse_y = 400;
    }

    q8tk_event_mouse_moved(mouse_x, mouse_y);
    }

    screen_attr_mouse_move();   /* マウス自動で隠す…ここで解除 */
}



/****************************************************************************
 * メニューモードのソフトウェアキーボード処理 (ワークとして scan_key を使用)
 *****************************************************************************/

/* ソフトウェアキーボードが押された状態なら真を返す */
int softkey_is_pressed(int code)
{
    if (IS_KEY88_LATTERTYPE(code)) {
    code = code - KEY88_F6 + KEY88_END;
    }
    return ((key_scan[ keyport[(code)].port ] & keyport[(code)].mask) == 0);
}

/* ソフトウェアキーボードを押す */
void    softkey_press(int code)
{
    if (IS_KEY88_LATTERTYPE(code)) {
    do_lattertype(code, TRUE);
    }
    KEY88_PRESS(code);
}

/* ソフトウェアキーボードを離す */
void    softkey_release(int code)
{
    if (IS_KEY88_LATTERTYPE(code)) {
    do_lattertype(code, FALSE);
    }
    KEY88_RELEASE(code);
}

/* ソフトウェアキーボードを全て離す */
void    softkey_release_all(void)
{
    size_t i;
    for (i=0; i<sizeof(key_scan); i++) key_scan[i] = 0xff;
}

/* ソフトウェアキーボードの、キーボードバグを再現 */
void    softkey_bug(void)
{
    int  my_port, your_port;
    byte my_val,  your_val,  save_val;

    save_val = key_scan[8] & 0xf0;  /* port 8 の 上位 4bit は対象外 */
    key_scan[8] |= 0xf0;

    /* port 0〜11(初期型キーボードの範囲) のみ、処理する */
    for (my_port=0; my_port<12; my_port++) {
    for (your_port=0; your_port<12; your_port++) {

        if (my_port == your_port) continue;

        my_val   = key_scan[ my_port ];
        your_val = key_scan[ your_port ];

        if ((my_val | your_val) != 0xff) {
        key_scan[ my_port ]   =
            key_scan[ your_port ] = my_val & your_val;
        }

    }
    }

    key_scan[8] &= ~0xf0;
    key_scan[8] |= save_val;
}







/***********************************************************************
 * ステートロード／ステートセーブ
 ************************************************************************/
/* ver 0.6.2 以前と ver 0.6.3 以降でファンクションキーの機能が変わったので、
   ステートロード／ステートセーブの際に変換する。*/

enum {          /* ver 0.6.2以前の、ファンクションキーの機能    */
  OLD_FN_FUNC,
  OLD_FN_FRATE_UP,
  OLD_FN_FRATE_DOWN,
  OLD_FN_VOLUME_UP,
  OLD_FN_VOLUME_DOWN,
  OLD_FN_PAUSE,
  OLD_FN_RESIZE,
  OLD_FN_NOWAIT,
  OLD_FN_SPEED_UP,
  OLD_FN_SPEED_DOWN,
  OLD_FN_MOUSE_HIDE,
  OLD_FN_FULLSCREEN,
  OLD_FN_IMAGE_NEXT1,
  OLD_FN_IMAGE_PREV1,
  OLD_FN_IMAGE_NEXT2,
  OLD_FN_IMAGE_PREV2,
  OLD_FN_NUMLOCK,
  OLD_FN_RESET,
  OLD_FN_KANA,
  OLD_FN_ROMAJI,
  OLD_FN_CAPS,
  OLD_FN_KETTEI,
  OLD_FN_HENKAN,
  OLD_FN_ZENKAKU,
  OLD_FN_PC,
  OLD_FN_SNAPSHOT,
  OLD_FN_STOP,
  OLD_FN_COPY,
  OLD_FN_STATUS,
  OLD_FN_MENU,
  OLD_FN_end
};
static struct{      /* ver 0.6.3以降との、機能の対応表 */
    int old;        int now;
} func_f_convert[] =
{
  { OLD_FN_FUNC,        FN_FUNC,    },
  { OLD_FN_FRATE_UP,    FN_FRATE_UP,    },
  { OLD_FN_FRATE_DOWN,  FN_FRATE_DOWN,  },
  { OLD_FN_VOLUME_UP,   FN_VOLUME_UP,   },
  { OLD_FN_VOLUME_DOWN, FN_VOLUME_DOWN, },
  { OLD_FN_PAUSE,       FN_PAUSE,   },
  { OLD_FN_RESIZE,      FN_RESIZE,  },
  { OLD_FN_NOWAIT,      FN_NOWAIT,  },
  { OLD_FN_SPEED_UP,    FN_SPEED_UP,    },
  { OLD_FN_SPEED_DOWN,  FN_SPEED_DOWN,  },
  { OLD_FN_MOUSE_HIDE,  FN_FUNC,    },
  { OLD_FN_FULLSCREEN,  FN_FULLSCREEN,  },
  { OLD_FN_IMAGE_NEXT1, FN_IMAGE_NEXT1, },
  { OLD_FN_IMAGE_PREV1, FN_IMAGE_PREV1, },
  { OLD_FN_IMAGE_NEXT2, FN_IMAGE_NEXT2, },
  { OLD_FN_IMAGE_PREV2, FN_IMAGE_PREV2, },
  { OLD_FN_NUMLOCK,     FN_NUMLOCK, },
  { OLD_FN_RESET,       FN_RESET,   },
  { OLD_FN_KANA,        FN_KANA,    },
  { OLD_FN_ROMAJI,      FN_ROMAJI,  },
  { OLD_FN_CAPS,        FN_CAPS,    },
  { OLD_FN_KETTEI,      KEY88_KETTEI,   },
  { OLD_FN_HENKAN,      KEY88_HENKAN,   },
  { OLD_FN_ZENKAKU,     KEY88_ZENKAKU,  },
  { OLD_FN_PC,      KEY88_PC,   },
  { OLD_FN_SNAPSHOT,    FN_SNAPSHOT,    },
  { OLD_FN_STOP,        KEY88_STOP, },
  { OLD_FN_COPY,        KEY88_COPY, },
  { OLD_FN_STATUS,      FN_STATUS,  },
  { OLD_FN_MENU,        FN_MENU,    },
  { OLD_FN_FUNC,        FN_MAX_SPEED,   },
  { OLD_FN_FUNC,        FN_MAX_CLOCK,   },
  { OLD_FN_FUNC,        FN_MAX_BOOST,   },
};
static  int old_func_f[ 1 + 20 ];
static  void    function_new2old( void )
{
  int i, j;
  for( i=1; i<=20; i++ ){
    old_func_f[i] = OLD_FN_FUNC;
    for( j=0; j<COUNTOF(func_f_convert); j++ ){
      if( function_f[i] == func_f_convert[j].now ){
    old_func_f[i] = func_f_convert[j].old;
    break;
      }
    }
  }
}
static  void    function_old2new( void )
{
  int i, j;
  for( i=1; i<=20; i++ ){
    function_f[i] = FN_FUNC;
    for( j=0; j<COUNTOF(func_f_convert); j++ ){
      if( old_func_f[i] == func_f_convert[j].old ){
    function_f[i] = func_f_convert[j].now;
    break;
      }
    }
  }
}



#define SID "KYBD"
#define SID2    "KYB2"
#define SID3    "KYB3"
#define SID4    "KYB4"

static  T_SUSPEND_W suspend_keyboard_work[] =
{
  { TYPE_INT,   &jop1_step      },
  { TYPE_INT,   &jop1_dx        },
  { TYPE_INT,   &jop1_dy        },

  { TYPE_INT,   &romaji_input_mode  },

  { TYPE_INT,   &mouse_mode     },
  { TYPE_INT,   &mouse_key_mode     },
  { TYPE_INT,   &mouse_key_assign[ 0]   },
  { TYPE_INT,   &mouse_key_assign[ 1]   },
  { TYPE_INT,   &mouse_key_assign[ 2]   },
  { TYPE_INT,   &mouse_key_assign[ 3]   },
  { TYPE_INT,   &mouse_key_assign[ 4]   },
  { TYPE_INT,   &mouse_key_assign[ 5]   },

  { TYPE_INT,   &joy_key_mode       },
  { TYPE_INT,   &joy_key_assign[ 0] },
  { TYPE_INT,   &joy_key_assign[ 1] },
  { TYPE_INT,   &joy_key_assign[ 2] },
  { TYPE_INT,   &joy_key_assign[ 3] },
  { TYPE_INT,   &joy_key_assign[ 4] },
  { TYPE_INT,   &joy_key_assign[ 5] },
  { TYPE_INT,   &joy_key_assign[ 6] },
  { TYPE_INT,   &joy_key_assign[ 7] },
  { TYPE_INT,   &joy_key_assign[ 8] },
  { TYPE_INT,   &joy_key_assign[ 9] },
  { TYPE_INT,   &joy_key_assign[10] },
  { TYPE_INT,   &joy_key_assign[11] },
  { TYPE_INT,   &joy_swap_button    },

  { TYPE_INT,   &cursor_key_mode    },
  { TYPE_INT,   &cursor_key_assign[0]   },
  { TYPE_INT,   &cursor_key_assign[1]   },
  { TYPE_INT,   &cursor_key_assign[2]   },
  { TYPE_INT,   &cursor_key_assign[3]   },

  { TYPE_INT,   &tenkey_emu     },
  { TYPE_INT,   &numlock_emu        },

  { TYPE_INT,   &old_func_f[ 1]     },
  { TYPE_INT,   &old_func_f[ 2]     },
  { TYPE_INT,   &old_func_f[ 3]     },
  { TYPE_INT,   &old_func_f[ 4]     },
  { TYPE_INT,   &old_func_f[ 5]     },
  { TYPE_INT,   &old_func_f[ 6]     },
  { TYPE_INT,   &old_func_f[ 7]     },
  { TYPE_INT,   &old_func_f[ 8]     },
  { TYPE_INT,   &old_func_f[ 9]     },
  { TYPE_INT,   &old_func_f[10]     },
  { TYPE_INT,   &old_func_f[11]     },
  { TYPE_INT,   &old_func_f[12]     },
  { TYPE_INT,   &old_func_f[13]     },
  { TYPE_INT,   &old_func_f[14]     },
  { TYPE_INT,   &old_func_f[15]     },
  { TYPE_INT,   &old_func_f[16]     },
  { TYPE_INT,   &old_func_f[17]     },
  { TYPE_INT,   &old_func_f[18]     },
  { TYPE_INT,   &old_func_f[19]     },
  { TYPE_INT,   &old_func_f[20]     },

  { TYPE_INT,   &romaji_type        },

  { TYPE_END,   0           },
};

static  T_SUSPEND_W suspend_keyboard_work2[] =
{
  { TYPE_INT,   &jop1_time      },
  { TYPE_END,   0           },
};

static  T_SUSPEND_W suspend_keyboard_work3[] =
{
  { TYPE_INT,   &function_f[ 1]     },
  { TYPE_INT,   &function_f[ 2]     },
  { TYPE_INT,   &function_f[ 3]     },
  { TYPE_INT,   &function_f[ 4]     },
  { TYPE_INT,   &function_f[ 5]     },
  { TYPE_INT,   &function_f[ 6]     },
  { TYPE_INT,   &function_f[ 7]     },
  { TYPE_INT,   &function_f[ 8]     },
  { TYPE_INT,   &function_f[ 9]     },
  { TYPE_INT,   &function_f[10]     },
  { TYPE_INT,   &function_f[11]     },
  { TYPE_INT,   &function_f[12]     },
  { TYPE_INT,   &function_f[13]     },
  { TYPE_INT,   &function_f[14]     },
  { TYPE_INT,   &function_f[15]     },
  { TYPE_INT,   &function_f[16]     },
  { TYPE_INT,   &function_f[17]     },
  { TYPE_INT,   &function_f[18]     },
  { TYPE_INT,   &function_f[19]     },
  { TYPE_INT,   &function_f[20]     },
  { TYPE_END,   0           },
};

static  T_SUSPEND_W suspend_keyboard_work4[] =
{
  { TYPE_INT,   &mouse_sensitivity  },
  { TYPE_INT,   &mouse_swap_button  },

  { TYPE_INT,   &joy2_key_mode      },
  { TYPE_INT,   &joy2_key_assign[ 0]    },
  { TYPE_INT,   &joy2_key_assign[ 1]    },
  { TYPE_INT,   &joy2_key_assign[ 2]    },
  { TYPE_INT,   &joy2_key_assign[ 3]    },
  { TYPE_INT,   &joy2_key_assign[ 4]    },
  { TYPE_INT,   &joy2_key_assign[ 5]    },
  { TYPE_INT,   &joy2_key_assign[ 6]    },
  { TYPE_INT,   &joy2_key_assign[ 7]    },
  { TYPE_INT,   &joy2_key_assign[ 8]    },
  { TYPE_INT,   &joy2_key_assign[ 9]    },
  { TYPE_INT,   &joy2_key_assign[10]    },
  { TYPE_INT,   &joy2_key_assign[11]    },
  { TYPE_INT,   &joy2_swap_button   },

  { TYPE_INT,   &serial_mouse_x     },
  { TYPE_INT,   &serial_mouse_y     },
  { TYPE_INT,   &serial_mouse_step  },

  { TYPE_END,   0           },
};


int statesave_keyboard( void )
{
  function_new2old();

  if( statesave_table( SID, suspend_keyboard_work ) != STATE_OK ) return FALSE;

  if( statesave_table( SID2,suspend_keyboard_work2) != STATE_OK ) return FALSE;

  if( statesave_table( SID3,suspend_keyboard_work3) != STATE_OK ) return FALSE;

  if( statesave_table( SID4,suspend_keyboard_work4) != STATE_OK ) return FALSE;

  return TRUE;
}

int stateload_keyboard( void )
{
  if( stateload_table( SID, suspend_keyboard_work ) != STATE_OK ) return FALSE;

  if( stateload_table( SID2,suspend_keyboard_work2) != STATE_OK ){

    /* 旧バージョンなら、みのがす */

    printf( "stateload : Statefile is old. (ver 0.6.0 or 1?)\n" );

    goto NOT_HAVE_SID2;
  }

  if( stateload_table( SID3,suspend_keyboard_work3) != STATE_OK ){

    /* 旧バージョンなら、みのがす */

    printf( "stateload : Statefile is old. (ver 0.6.0, 1 or 2?)\n" );

    goto NOT_HAVE_SID3;
  }

  if( stateload_table( SID4,suspend_keyboard_work4) != STATE_OK ){

    /* 旧バージョンなら、みのがす */

    printf( "stateload : Statefile is old. (ver 0.6.0, 1, 2 or 3?)\n" );
  }

  return TRUE;



 NOT_HAVE_SID2:
  /* この関数の呼び出し以前に、 stateload_pc88main と stateload_intr が
     呼び出されていなければ、以下の初期化は意味がない */

  jop1_time = state_of_cpu + z80main_cpu.state0;


 NOT_HAVE_SID3:
  /* function_f[] を差し替える */
  function_old2new();


  return TRUE;
}










/****************************************************************************
 *
 *  ユーティリティ
 *
 *****************************************************************************/

/* QUASI88 キーコードの文字列を int 値に変換するテーブル */

static const T_SYMBOL_TABLE key88sym_list[] =
{
    { "KEY88_INVALID"       ,   KEY88_INVALID       },
    { "KEY88_SPACE"         ,   KEY88_SPACE             },
    { "KEY88_EXCLAM"        ,   KEY88_EXCLAM            },
    { "KEY88_QUOTEDBL"      ,   KEY88_QUOTEDBL          },
    { "KEY88_NUMBERSIGN"    ,   KEY88_NUMBERSIGN        },
    { "KEY88_DOLLAR"        ,   KEY88_DOLLAR            },
    { "KEY88_PERCENT"       ,   KEY88_PERCENT           },
    { "KEY88_AMPERSAND"     ,   KEY88_AMPERSAND         },
    { "KEY88_APOSTROPHE"    ,   KEY88_APOSTROPHE        },
    { "KEY88_PARENLEFT"     ,   KEY88_PARENLEFT         },
    { "KEY88_PARENRIGHT"    ,   KEY88_PARENRIGHT        },
    { "KEY88_ASTERISK"      ,   KEY88_ASTERISK          },
    { "KEY88_PLUS"          ,   KEY88_PLUS              },
    { "KEY88_COMMA"         ,   KEY88_COMMA             },
    { "KEY88_MINUS"     ,   KEY88_MINUS             },
    { "KEY88_PERIOD"        ,   KEY88_PERIOD            },
    { "KEY88_SLASH"     ,   KEY88_SLASH             },
    { "KEY88_0"         ,   KEY88_0                 },
    { "KEY88_1"         ,   KEY88_1                 },
    { "KEY88_2"         ,   KEY88_2                 },
    { "KEY88_3"         ,   KEY88_3                 },
    { "KEY88_4"         ,   KEY88_4                 },
    { "KEY88_5"         ,   KEY88_5                 },
    { "KEY88_6"         ,   KEY88_6                 },
    { "KEY88_7"         ,   KEY88_7                 },
    { "KEY88_8"         ,   KEY88_8                 },
    { "KEY88_9"         ,   KEY88_9                 },
    { "KEY88_COLON"     ,   KEY88_COLON             },
    { "KEY88_SEMICOLON"     ,   KEY88_SEMICOLON         },
    { "KEY88_LESS"          ,   KEY88_LESS              },
    { "KEY88_EQUAL"     ,   KEY88_EQUAL             },
    { "KEY88_GREATER"       ,   KEY88_GREATER           },
    { "KEY88_QUESTION"      ,   KEY88_QUESTION          },
    { "KEY88_AT"        ,   KEY88_AT                },
    { "KEY88_A"         ,   KEY88_A                 },
    { "KEY88_B"         ,   KEY88_B                 },
    { "KEY88_C"         ,   KEY88_C                 },
    { "KEY88_D"         ,   KEY88_D                 },
    { "KEY88_E"         ,   KEY88_E                 },
    { "KEY88_F"         ,   KEY88_F                 },
    { "KEY88_G"         ,   KEY88_G                 },
    { "KEY88_H"         ,   KEY88_H                 },
    { "KEY88_I"         ,   KEY88_I                 },
    { "KEY88_J"         ,   KEY88_J                 },
    { "KEY88_K"         ,   KEY88_K                 },
    { "KEY88_L"         ,   KEY88_L                 },
    { "KEY88_M"         ,   KEY88_M                 },
    { "KEY88_N"         ,   KEY88_N                 },
    { "KEY88_O"         ,   KEY88_O                 },
    { "KEY88_P"         ,   KEY88_P                 },
    { "KEY88_Q"         ,   KEY88_Q                 },
    { "KEY88_R"         ,   KEY88_R                 },
    { "KEY88_S"         ,   KEY88_S                 },
    { "KEY88_T"         ,   KEY88_T                 },
    { "KEY88_U"         ,   KEY88_U                 },
    { "KEY88_V"         ,   KEY88_V                 },
    { "KEY88_W"         ,   KEY88_W                 },
    { "KEY88_X"         ,   KEY88_X                 },
    { "KEY88_Y"         ,   KEY88_Y                 },
    { "KEY88_Z"         ,   KEY88_Z                 },
    { "KEY88_BRACKETLEFT"   ,   KEY88_BRACKETLEFT       },
    { "KEY88_YEN"       ,   KEY88_YEN           },
    { "KEY88_BRACKETRIGHT"  ,   KEY88_BRACKETRIGHT      },
    { "KEY88_CARET"         ,   KEY88_CARET             },
    { "KEY88_UNDERSCORE"    ,   KEY88_UNDERSCORE        },
    { "KEY88_BACKQUOTE"     ,   KEY88_BACKQUOTE         },
    { "KEY88_a"             ,   KEY88_a                 },
    { "KEY88_b"             ,   KEY88_b                 },
    { "KEY88_c"         ,   KEY88_c                 },
    { "KEY88_d"         ,   KEY88_d                 },
    { "KEY88_e"         ,   KEY88_e                 },
    { "KEY88_f"         ,   KEY88_f                 },
    { "KEY88_g"         ,   KEY88_g                 },
    { "KEY88_h"         ,   KEY88_h                 },
    { "KEY88_i"         ,   KEY88_i                 },
    { "KEY88_j"         ,   KEY88_j                 },
    { "KEY88_k"         ,   KEY88_k                 },
    { "KEY88_l"         ,   KEY88_l                 },
    { "KEY88_m"         ,   KEY88_m                 },
    { "KEY88_n"         ,   KEY88_n                 },
    { "KEY88_o"         ,   KEY88_o                 },
    { "KEY88_p"         ,   KEY88_p                 },
    { "KEY88_q"         ,   KEY88_q                 },
    { "KEY88_r"         ,   KEY88_r                 },
    { "KEY88_s"         ,   KEY88_s                 },
    { "KEY88_t"         ,   KEY88_t                 },
    { "KEY88_u"         ,   KEY88_u                 },
    { "KEY88_v"         ,   KEY88_v                 },
    { "KEY88_w"         ,   KEY88_w                 },
    { "KEY88_x"         ,   KEY88_x                 },
    { "KEY88_y"         ,   KEY88_y                 },
    { "KEY88_z"         ,   KEY88_z                 },
    { "KEY88_BRACELEFT"     ,   KEY88_BRACELEFT         },
    { "KEY88_BAR"           ,   KEY88_BAR               },
    { "KEY88_BRACERIGHT"    ,   KEY88_BRACERIGHT        },
    { "KEY88_TILDE"         ,   KEY88_TILDE             },
    { "KEY88_KP_0"      ,   KEY88_KP_0              },
    { "KEY88_KP_1"      ,   KEY88_KP_1              },
    { "KEY88_KP_2"      ,   KEY88_KP_2              },
    { "KEY88_KP_3"      ,   KEY88_KP_3              },
    { "KEY88_KP_4"      ,   KEY88_KP_4              },
    { "KEY88_KP_5"      ,   KEY88_KP_5              },
    { "KEY88_KP_6"      ,   KEY88_KP_6              },
    { "KEY88_KP_7"      ,   KEY88_KP_7              },
    { "KEY88_KP_8"      ,   KEY88_KP_8              },
    { "KEY88_KP_9"      ,   KEY88_KP_9              },
    { "KEY88_KP_MULTIPLY"   ,   KEY88_KP_MULTIPLY       },
    { "KEY88_KP_ADD"        ,   KEY88_KP_ADD            },
    { "KEY88_KP_EQUAL"      ,   KEY88_KP_EQUAL      },
    { "KEY88_KP_COMMA"      ,   KEY88_KP_COMMA          },
    { "KEY88_KP_PERIOD"     ,   KEY88_KP_PERIOD         },
    { "KEY88_KP_SUB"        ,   KEY88_KP_SUB            },
    { "KEY88_KP_DIVIDE"     ,   KEY88_KP_DIVIDE         },
    { "KEY88_RETURN"        ,   KEY88_RETURN            },
    { "KEY88_HOME"      ,   KEY88_HOME              },
    { "KEY88_UP"        ,   KEY88_UP                },
    { "KEY88_RIGHT"     ,   KEY88_RIGHT             },
    { "KEY88_INS_DEL"       ,   KEY88_INS_DEL           },
    { "KEY88_GRAPH"         ,   KEY88_GRAPH             },
    { "KEY88_KANA"      ,   KEY88_KANA              },
    { "KEY88_SHIFT"     ,   KEY88_SHIFT             },
    { "KEY88_CTRL"      ,   KEY88_CTRL              },
    { "KEY88_STOP"      ,   KEY88_STOP              },
    { "KEY88_ESC"       ,   KEY88_ESC               },
    { "KEY88_TAB"       ,   KEY88_TAB               },
    { "KEY88_DOWN"      ,   KEY88_DOWN              },
    { "KEY88_LEFT"      ,   KEY88_LEFT              },
    { "KEY88_HELP"      ,   KEY88_HELP              },
    { "KEY88_COPY"      ,   KEY88_COPY              },
    { "KEY88_CAPS"      ,   KEY88_CAPS              },
    { "KEY88_ROLLUP"        ,   KEY88_ROLLUP            },
    { "KEY88_ROLLDOWN"      ,   KEY88_ROLLDOWN          },
    { "KEY88_F1"        ,   KEY88_F1                },
    { "KEY88_F2"        ,   KEY88_F2                },
    { "KEY88_F3"        ,   KEY88_F3                },
    { "KEY88_F4"        ,   KEY88_F4                },
    { "KEY88_F5"        ,   KEY88_F5                },
    { "KEY88_F11"       ,   KEY88_F11               },
    { "KEY88_F12"       ,   KEY88_F12               },
    { "KEY88_F13"       ,   KEY88_F13               },
    { "KEY88_F14"       ,   KEY88_F14               },
    { "KEY88_F15"       ,   KEY88_F15               },
    { "KEY88_F16"       ,   KEY88_F16               },
    { "KEY88_F17"       ,   KEY88_F17               },
    { "KEY88_F18"       ,   KEY88_F18               },
    { "KEY88_F19"       ,   KEY88_F19               },
    { "KEY88_F20"       ,   KEY88_F20               },
    { "KEY88_F6"        ,   KEY88_F6                },
    { "KEY88_F7"        ,   KEY88_F7                },
    { "KEY88_F8"        ,   KEY88_F8                },
    { "KEY88_F9"        ,   KEY88_F9                },
    { "KEY88_F10"       ,   KEY88_F10               },
    { "KEY88_BS"        ,   KEY88_BS                },
    { "KEY88_INS"       ,   KEY88_INS               },
    { "KEY88_DEL"       ,   KEY88_DEL               },
    { "KEY88_HENKAN"        ,   KEY88_HENKAN            },
    { "KEY88_KETTEI"        ,   KEY88_KETTEI            },
    { "KEY88_PC"        ,   KEY88_PC                },
    { "KEY88_ZENKAKU"       ,   KEY88_ZENKAKU           },
    { "KEY88_RETURNL"       ,   KEY88_RETURNL           },
    { "KEY88_RETURNR"       ,   KEY88_RETURNR           },
    { "KEY88_SHIFTL"        ,   KEY88_SHIFTL            },
    { "KEY88_SHIFTR"        ,   KEY88_SHIFTR            },

    { "KEY88_SYS_MENU"      ,   KEY88_SYS_MENU          },
    { "KEY88_SYS_STATUS"    ,   KEY88_SYS_STATUS        },
};


/***********************************************************************
 * 与えられた文字列を、QUASI88 キーコードに変換する
 *  キー定義ファイルの解析などに使おう
 *
 *   例)
 *  "KEY88_SPACE" -> KEY88_SPACE    定義そのままの文字列は、直に変換
 *  "key88_SPACE" -> KEY88_SPACE    小文字混在でもよい
 *  "KEY88_Z"     -> KEY88_Z    これも、定義そのままの例
 *  "KEY88_z"     -> KEY88_z    これも、定義そのままの例
 *  "key88_z"     -> KEY88_Z    小文字混在の場合は大文字になるよ
 *  "0x20"        -> KEY88_SPACE    0x20〜0xf7 は直にキーコードに変換
 *  "0x01"        -> KEY88_INVALID  上記の範囲外なら無効(0)を返す
 *  "32"          -> KEY88_SPACE    10進数や8進数でも同様
 *  "KP1"         -> KEY88_KP_1 KP と 1文字 で、テンキーとする
 *  "KP+"         -> KEY88_KP_ADD   記号でもよい
 *  "Kp9"         -> KEY88_KP_9 小文字混在でもよい
 *  "Err"         -> -1     どれにも合致しなかったら、負を返す
 ************************************************************************/

int keyboard_str2key88(const char *str)
{
  static const T_SYMBOL_TABLE tenkey_list[] =
  {             /* テンキーに限り、例外的な表記が可能 */
    { "KP0"         ,   KEY88_KP_0              },
    { "KP1"         ,   KEY88_KP_1              },
    { "KP2"         ,   KEY88_KP_2              },
    { "KP3"         ,   KEY88_KP_3              },
    { "KP4"         ,   KEY88_KP_4              },
    { "KP5"         ,   KEY88_KP_5              },
    { "KP6"         ,   KEY88_KP_6              },
    { "KP7"         ,   KEY88_KP_7              },
    { "KP8"         ,   KEY88_KP_8              },
    { "KP9"         ,   KEY88_KP_9              },
    { "KP*"         ,   KEY88_KP_MULTIPLY       },
    { "KP+"         ,   KEY88_KP_ADD            },
    { "KP="         ,   KEY88_KP_EQUAL          },
    { "KP,"         ,   KEY88_KP_COMMA          },
    { "KP."         ,   KEY88_KP_PERIOD         },
    { "KP-"         ,   KEY88_KP_SUB            },
    { "KP/"         ,   KEY88_KP_DIVIDE         },
  };

    int len, i;
    char *conv_end;
    unsigned long l;

    if (str == NULL) return -1;
    len = strlen(str);
    if (len == 0) return -1;


                    /* 0〜9 で始まれば、数字に変換 */
    if ('0'<=str[0] && str[0]<='9') {
    l = strtoul(str, &conv_end, 0);     /* 10進,16進,8進数が可能 */
    if (*conv_end == '\0') {
        if (32 <= l && l <= 247) {
        return l;
        } else {
        return KEY88_INVALID;
        }
    }
    return -1;
    }
                    /* 3文字なら、テンキーかも */
    if (len == 3) {
    for (i=0; i<COUNTOF(tenkey_list); i++) {
        if (strcmp(tenkey_list[i].name, str) == 0) {
        return tenkey_list[i].val;
        }
    }
    for (i=0; i<COUNTOF(tenkey_list); i++) {
        if (my_strcmp(tenkey_list[i].name, str) == 0) {
        return tenkey_list[i].val;
        }
    }
    }
                    /* 定義文字列に合致するのを探す */
    for (i=0; i<COUNTOF(key88sym_list); i++) {
    if (strcmp(key88sym_list[i].name, str) == 0) {
        return key88sym_list[i].val;
    }
    }
    for (i=0; i<COUNTOF(key88sym_list); i++) {
    if (my_strcmp(key88sym_list[i].name, str) == 0) {
        return key88sym_list[i].val;
    }
    }

    return -1;
}


const char  *keyboard_key882str(int key88)
{
    int i;
    for (i=0; i<COUNTOF(key88sym_list); i++) {
    if (key88sym_list[i].val == KEY88_INVALID) continue;
    if (key88sym_list[i].val == key88) {
        return key88sym_list[i].name;
    }
    }
    return NULL;
}





/****************************************************************************
 * キー設定ファイルを読み込んで、設定を行う。
 *
 *  内部的には、キー設定ファイルを１行読むたびに、コールバック関数を呼ぶ。
 *  この行の内容を設定に反映するかどうかは、コールバック関数次第。
 *
 * --------------------------------------------------------------------------
 * キー設定ファイルの書式は、こんな感じ
 * (一行につき 最大3個のトークンを並べられる。 # から行末まではコメントとする)
 *
 *  [SDL]           dga
 *  SDLK_ESCAPE     KEY88_ESC       KEY88_KP_4
 *  <49>            KEY88_ZENKAKU
 *
 * 1行目のように、[〜] で始まる行は、「識別タグ行」とする。
 * 1番目のトークンは [〜] の形式で、2番目、3番目のトークンは任意 (無くてもよい)
 * 「識別タグ行」を読むたびに、以下のコールバック関数が呼び出される。
 *
 *  identify_callback(const char *param1, const char *param2,
 *            const char *param3)
 *
 *  上の例の場合、引数は以下がセットされる
 *      param1 … "[SDL]"
 *      param2 … "dga"
 *      param3 … NULL (3番目のトークンがないことを意味する)
 *
 * このコールバック関数は、この「識別タグ行」が有効であれば、 NULL を返すので、
 * この次の行から、次の「識別タグ行」 ([〜] で始まる行) までを、処理する。
 * 返り値が NULL でない場合は、次の「識別タグ行」までスキップする。
 * なお、返り値が空文字列 "" でなければ、ワーニングとしてそれを表示する。
 *
 * 2行目は、キーの「設定行」とする。
 * 1番目、2番目のトークンは必須で、3番目のトークンは任意 (無くてもよい)
 * 「設定行」のたびに、以下のコールバック関数が呼び出される。
 *
 *  setting_callback(int type, int code, int key88, int numlock_key88);
 *
 *  上の例の場合、引数は以下がセットされる。
 *      type          … 1
 *      code          … "SDLK_ESCAPE" を int に変換したもの (後述 ※1)
 *      key88         … KEY88_ESC の enum値
 *      numlock_key88 … KEY88_KP_4 の enum値
 *
 * 3行目も、キーの「設定行」だが、書式が若干異なっている。
 * この場合も、同様にコールバック関数が呼び出される。
 *
 *  callback(int type, int code, int key88, int numlock_key88);
 *
 *  上の例の場合、引数は以下がセットされる。
 *      type          … 2
 *      code          … 49
 *      key88         … KEY88_ZENKAKU の enum値
 *      numlock_key88 … -1 (3番目のトークンがないことを意味する)
 *
 * つまり、2行目の形式の場合は、1番目のトークンを int に変換した値が code に
 * セットされるが、3行目の形式の場合は、 <〜> の中の数値がセットされる。
 *
 * このコールバック関数は、設定が有効であれば NULL を返す。返り値が NULL で
 * なく、空文字列 "" でもない場合はワーニングとしてそれを表示する。
 *
 *
 * さて、※1 のような、 SDLK_ESCAPE を int に変換する方法は、以下の配列による。
 *
 *  T_SYMBOL_TABLE  table_symbol2int[]; (配列のサイズは、 table_size)
 *
 * この配列を先頭からチェックして、 table_symbol2int[].name と1番目のトークン
 * が一致した場合に、table_symbol2int[].val の値を返す。
 *
 * table_ignore_case が真の場合、大文字小文字は無視してチェックするので、
 * 上の例の場合は "SDLK_ESCAPE" "sdlk_escape" どちらでも一致とみなす。
 *
 *
 *
 *
 *****************************************************************************/
#include <file-op.h>

static  int symbol2int(const char *str,
               const T_SYMBOL_TABLE table_symbol2int[],
               int                  table_size,
               int                  table_ignore_case);

/* キー設定ファイル1行あたりの最大文字数 */
#define MAX_KEYFILE_LINE    (256)

int config_read_keyconf_file(
            const char *keyconf_filename,
            const char *(*identify_callback)(const char *parm1,
                             const char *parm2,
                             const char *parm3),
            const T_SYMBOL_TABLE table_symbol2int[],
            int                  table_size,
            int                  table_ignore_case,
            const char *(*setting_callback)(int type,
                            int code,
                            int key88,
                            int numlock_key88))
{
    const char *filename;
    OSD_FILE *fp = NULL;
    int  working   = FALSE;
    int  effective = FALSE;

    int    line_cnt = 0;
    char   line[ MAX_KEYFILE_LINE ];
    char buffer[ MAX_KEYFILE_LINE ];

    char *parm1, *parm2, *parm3, *parm4;

    int  type, code, key88, numlock_key88;
    const char *err_mes;


    /* キー設定ファイルを開く */

    if (keyconf_filename == NULL) {        /* ファイル名未指定ならば */
    filename = filename_alloc_keyboard_cfgname();  /* デフォルト名を取得 */
    } else {
    filename = keyconf_filename;
    }

    if (filename) {
    fp = osd_fopen(FTYPE_CFG, filename, "r");
    if (verbose_proc) {
        if (fp) {
        printf("\"%s\" read and initialize\n", filename);
        } else {
        printf("can't open keyboard configuration file \"%s\"\n",
               filename);
        printf("\n");
        }
    }
    if (keyconf_filename == NULL) {         /* デフォルトならば */
        free((void*)filename);          /* メモリ解放しとく */
    }
    }

    if (fp == NULL) return FALSE;       /* 開けなかったら偽を返す */



    /* キー設定ファイルを1行づつ解析 */

    while (osd_fgets(line, MAX_KEYFILE_LINE, fp)) {

    line_cnt ++;
    parm1 = parm2 = parm3 = parm4 = NULL;

    /* 行の内容をトークンに分解。各トークンは、parm1 〜 parm4 にセット */

    { char *str = line;
    char *b; {             b = &buffer[0];      str = my_strtok(b, str); }
    if (str) { parm1 = b;  b += strlen(b) + 1;  str = my_strtok(b, str); }
    if (str) { parm2 = b;  b += strlen(b) + 1;  str = my_strtok(b, str); }
    if (str) { parm3 = b;  b += strlen(b) + 1;  str = my_strtok(b, str); }
    if (str) { parm4 = b;  }
    }


    /* トークンがなければ次の行へ */
    if (parm1 == NULL) continue;

    /* トークンが四個以上あれば、その行はエラーなので次の行へ */
    if (parm4 != NULL) {
        if (working) {
        fprintf(stderr,
            "warning: too many argument in line %d\n", line_cnt);
        }
        continue;
    }

    /* トークンが一個〜三個なら解析 */
    if (parm1[0] == '[') {          /* 「識別タグ行」を処理 */

        /* コールバック関数を呼び出して、有効な識別タグかを判定 */
        err_mes = (identify_callback)(parm1, parm2, parm3);

        if (err_mes == NULL) {      /* 有効な識別タグだった */
        working   = TRUE;
        effective = TRUE;

        if (verbose_proc)
            printf("(read start in line %d)\n", line_cnt);

        } else {                /* 無効な識別タグだった */

        if (working) {              /* 処理中なら終了 */
            if (verbose_proc)
            printf("(read stop  in line %d)\n", line_cnt - 1);
        }                   /* でなければ無視 */

        if (err_mes[0] != '\0') {
            fprintf(stderr,
                "warning: %s in %d (ignored)\n",
                err_mes, line_cnt);
        }

        working = FALSE;
        }

    } else {                /* 「設定行」を処理 */

        if (working) {

        /* 「設定行」で、トークン一個だけは、エラー。次の行へ */
        if (parm2 == NULL) {
            fprintf(stderr,
                "warning: error in line %d (ignored)\n", line_cnt);
        } else {

            code = symbol2int(parm1, table_symbol2int,
                      table_size, table_ignore_case);
            key88 = keyboard_str2key88(parm2);

            if (parm3) { numlock_key88 = keyboard_str2key88(parm3); }
            else       { numlock_key88 = -1; }

            if (code < 0 ||
            key88 < 0 ||
            (parm3 && numlock_key88 < 0)) {
            fprintf(stderr,
                "warning: error in line %d (ignored)\n", line_cnt);
            } else {

            if (parm1[0] == '<') { type = KEYCODE_SCAN; }
            else                 { type = KEYCODE_SYM;  }

            err_mes = (setting_callback)(type, code,
                             key88, numlock_key88);

            if (err_mes == NULL) {  /* 有効な設定だった */
                /* OK */ ;
            } else {        /* 無効な設定だった */
                /* NG */
                if (err_mes[0] != '\0') {
                fprintf(stderr,
                    "warning: %s in %d (ignored)\n",
                    err_mes, line_cnt);
                }
            }
            }
        }
        }
    }
    }
    osd_fclose(fp);


    if (working) {
    if(verbose_proc) printf("(read end   in line %d)\n", line_cnt-1);
    }

    if (effective == FALSE) {
    fprintf(stderr, "warning: not configured (use initial config)\n");
    }

    if (verbose_proc) {
    printf("\n");
    }

    return (effective) ? TRUE : FALSE;
}


static  int symbol2int(const char *str,
               const T_SYMBOL_TABLE table_symbol2int[],
               int                  table_size,
               int                  table_ignore_case)
{
    int i;
    char *conv_end;
    unsigned long l;

    if ( str == NULL) return -1;
    if (*str == '\0') return -1;

    if (str[0] == '<') {        /* <数字> の場合 */
    l = strtoul(&str[1], &conv_end, 0);
    if (*conv_end == '>') {
        return l;
    }
    return -1;
    }
                    /* 定義文字列に合致するのを探す */
    for (i=0; i<table_size; i++) {
    if (strcmp(table_symbol2int[i].name, str) == 0) {
        return table_symbol2int[i].val;
    }
    }
    if (table_ignore_case) {
    for (i=0; i<table_size; i++) {
        if (my_strcmp(table_symbol2int[i].name, str) == 0) {
        return table_symbol2int[i].val;
        }
    }
    }

    return -1;
}
