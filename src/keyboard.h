#ifndef KEYBOARD_H_INCLUDED
#define KEYBOARD_H_INCLUDED


                /* mouse_mode の値 */
#define MOUSE_NONE  0   /* マウス・ジョイスティックなし             */
#define MOUSE_MOUSE 1   /* マウスを装着                             */
#define MOUSE_JOYMOUSE  2   /* マウスを擬似ジョイスティックモードで装着 */
#define MOUSE_JOYSTICK  3   /* ジョイスティックを装着                   */

extern  int mouse_mode;     /* マウス 0:No 1:Yes 2:Joy   */

extern  int mouse_sensitivity;  /* マウス感度          */
extern  int mouse_swap_button;  /* マウスボタンを入れ替える     */

extern  int mouse_key_mode;     /* マウス入力をキーに反映    */
extern  int mouse_key_assign[6];

extern  int joy_key_mode;       /* ジョイ入力をキーに反映    */
extern  int joy_swap_button;    /* ボタンのABを入れ替える     */
extern  int joy_key_assign[12];

extern  int joy2_key_mode;      /* ジョイ２入力をキーに反映 */
extern  int joy2_swap_button;   /* ボタンのABを入れ替える     */
extern  int joy2_key_assign[12];

extern  int cursor_key_mode;    /* カーソルキーを別キーに反映  */
extern  int cursor_key_assign[4];
        /* Cursor KEY -> 10 KEY , original by funa. (thanks!) */
        /* Cursor Key -> 任意のキー , original by floi. (thanks!) */


extern  int tenkey_emu;     /* 10 KEY の入力を生成 */
extern  int numlock_emu;        /* software NUM lock    */


enum {                  /* ファンクションキーの機能 */
  FN_FUNC,              /* 現在の仕様では 31種類までしかだめ */
  FN_FRATE_UP,
  FN_FRATE_DOWN,
  FN_VOLUME_UP,
  FN_VOLUME_DOWN,
  FN_PAUSE,
  FN_RESIZE,
  FN_NOWAIT,
  FN_SPEED_UP,
  FN_SPEED_DOWN,
  FN_FULLSCREEN,
  FN_IMAGE_NEXT1,
  FN_IMAGE_PREV1,
  FN_IMAGE_NEXT2,
  FN_IMAGE_PREV2,
  FN_NUMLOCK,
  FN_RESET,
  FN_KANA,
  FN_ROMAJI,
  FN_CAPS,
  FN_SNAPSHOT,
  FN_STATUS,
  FN_MENU,
  FN_MAX_SPEED,
  FN_MAX_CLOCK,
  FN_MAX_BOOST,
  FN_end

  /* この値はステートファイルに記録されてしまう。ということは、この値を
     変更するとステートファイルに互換がなくなってしまう (泣)。
     今回は、ステートセーブ/ステートロードの際に以前の値と相互変換する
     ようにしたが、面倒なので今後は互換性無しであきらめよう・・・ */
};

extern  int function_f[ 1 + 20 ];       /* ファンクションキーの機能  */

extern  int fn_max_speed;
extern  double  fn_max_clock;
extern  int fn_max_boost;


extern  int romaji_type;            /* ローマ字変換のタイプ        */


extern  byte    key_scan[0x10];         /* IN[00-0F] キースキャン    */

extern  int romaji_input_mode;      /* 真:ローマ字入力中         */

extern  int mouse_x;            /* マウス座標           */
extern  int mouse_y;


extern  int need_focus;         /* フォーカスアウト停止あり */

extern  char    *file_rec;          /* キー入力記録のファイル名 */
extern  char    *file_pb;           /* キー入力再生のファイル名 */


void    keyboard_reset(void);
void    keyboard_update(void);
void    keyboard_switch(void);

void    init_serial_mouse_data(void);
int get_serial_mouse_data(void);

void    key_record_playback_init(void);
void    key_record_playback_exit(void);

void    keyboard_jop1_reset(void);
void    keyboard_jop1_strobe(void);


int softkey_is_pressed(int code);       /* メニューのソフトキー用 */
void    softkey_press(int code);
void    softkey_release(int code);
void    softkey_release_all(void);
void    softkey_bug(void);


void    quasi88_cfg_key_numlock(int on);
void    quasi88_cfg_key_kana(int on);
void    quasi88_cfg_key_romaji(int on);




/* キーバインディング変更時の、指定した値の指す内容 */

#define KEYCODE_INVALID (0) /* 指定した値は、無効             */
#define KEYCODE_SYM (1) /* 指定した値は、キーシンボル値   */
#define KEYCODE_SCAN    (2) /* 指定した値は、スキャンコード値 */


/* キーシンボルの文字列 (XK_xxx や SDLK_xxx) を int値に変換するテーブル */

typedef struct {
    char    *name;      /* keysym (キーシンボル) 文字列 */
    int     val;        /* 対応する、 int値       */
} T_SYMBOL_TABLE;


int     keyboard_str2key88(const char *str);
const char  *keyboard_key882str(int key88);

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
                            int numlock_key88));



/*----------------------------------------------------------------------
 * QUASI88 キーコード定義
 *  ここで定義しているキーコードは以下のキーである。
 *      ・PC-8801 に必要なキー
 *      ・ASCII入力に必要なキー
 *      ・マウス、ジョイスティックのボタン
 *      ・QUASI88 の制御にあると便利なキー
 *----------------------------------------------------------------------*/

#define KEY88_PAD_OFFSET    (12)
#define KEY88_PAD_MAX       (2)
#define KEY88_PAD_BUTTON_MAX    (8)

/*#define   KEY88_MENU_NUM      (31)*/

enum {

  KEY88_INVALID     = 0,

  /* 1〜31 は特殊機能制御用にリサーブしておく */

  /* 文字キー用の定義 (ASCIIコードに合致) */

  KEY88_SPACE       = 32,
  KEY88_EXCLAM      = 33,
  KEY88_QUOTEDBL    = 34,
  KEY88_NUMBERSIGN  = 35,
  KEY88_DOLLAR      = 36,
  KEY88_PERCENT     = 37,
  KEY88_AMPERSAND   = 38,
  KEY88_APOSTROPHE  = 39,
  KEY88_PARENLEFT   = 40,
  KEY88_PARENRIGHT  = 41,
  KEY88_ASTERISK    = 42,
  KEY88_PLUS        = 43,
  KEY88_COMMA       = 44,
  KEY88_MINUS       = 45,
  KEY88_PERIOD      = 46,
  KEY88_SLASH       = 47,
  KEY88_0       = 48,
  KEY88_1       = 49,
  KEY88_2       = 50,
  KEY88_3       = 51,
  KEY88_4       = 52,
  KEY88_5       = 53,
  KEY88_6       = 54,
  KEY88_7       = 55,
  KEY88_8       = 56,
  KEY88_9       = 57,
  KEY88_COLON       = 58,
  KEY88_SEMICOLON   = 59,
  KEY88_LESS        = 60,
  KEY88_EQUAL       = 61,
  KEY88_GREATER     = 62,
  KEY88_QUESTION    = 63,
  KEY88_AT      = 64,
  KEY88_A       = 65,
  KEY88_B       = 66,
  KEY88_C       = 67,
  KEY88_D       = 68,
  KEY88_E       = 69,
  KEY88_F       = 70,
  KEY88_G       = 71,
  KEY88_H       = 72,
  KEY88_I       = 73,
  KEY88_J       = 74,
  KEY88_K       = 75,
  KEY88_L       = 76,
  KEY88_M       = 77,
  KEY88_N       = 78,
  KEY88_O       = 79,
  KEY88_P       = 80,
  KEY88_Q       = 81,
  KEY88_R       = 82,
  KEY88_S       = 83,
  KEY88_T       = 84,
  KEY88_U       = 85,
  KEY88_V       = 86,
  KEY88_W       = 87,
  KEY88_X       = 88,
  KEY88_Y       = 89,
  KEY88_Z       = 90,
  KEY88_BRACKETLEFT = 91,
  KEY88_YEN     = 92,
  KEY88_BRACKETRIGHT    = 93,
  KEY88_CARET       = 94,
  KEY88_UNDERSCORE  = 95,
  KEY88_BACKQUOTE   = 96,
  KEY88_a       = 97,
  KEY88_b       = 98,
  KEY88_c       = 99,
  KEY88_d       = 100,
  KEY88_e       = 101,
  KEY88_f       = 102,
  KEY88_g       = 103,
  KEY88_h       = 104,
  KEY88_i       = 105,
  KEY88_j       = 106,
  KEY88_k       = 107,
  KEY88_l       = 108,
  KEY88_m       = 109,
  KEY88_n       = 110,
  KEY88_o       = 111,
  KEY88_p       = 112,
  KEY88_q       = 113,
  KEY88_r       = 114,
  KEY88_s       = 115,
  KEY88_t       = 116,
  KEY88_u       = 117,
  KEY88_v       = 118,
  KEY88_w       = 119,
  KEY88_x       = 120,
  KEY88_y       = 121,
  KEY88_z       = 122,
  KEY88_BRACELEFT   = 123,
  KEY88_BAR     = 124,
  KEY88_BRACERIGHT  = 125,
  KEY88_TILDE       = 126,

  /* テンキー文字用の定義 */

  KEY88_KP_0        = 128,
  KEY88_KP_1        = 129,
  KEY88_KP_2        = 130,
  KEY88_KP_3        = 131,
  KEY88_KP_4        = 132,
  KEY88_KP_5        = 133,
  KEY88_KP_6        = 134,
  KEY88_KP_7        = 135,
  KEY88_KP_8        = 136,
  KEY88_KP_9        = 137,
  KEY88_KP_MULTIPLY = 138,
  KEY88_KP_ADD      = 139,
  KEY88_KP_EQUAL    = 140,
  KEY88_KP_COMMA    = 141,
  KEY88_KP_PERIOD   = 142,
  KEY88_KP_SUB      = 143,
  KEY88_KP_DIVIDE   = 144,

  /* 特殊キー用の定義 */

  KEY88_RETURN      = 145,
  KEY88_HOME        = 146,
  KEY88_UP      = 147,
  KEY88_RIGHT       = 148,
  KEY88_INS_DEL     = 149,
  KEY88_GRAPH       = 150,
  KEY88_KANA        = 151,
  KEY88_SHIFT       = 152,
  KEY88_CTRL        = 153,
  KEY88_STOP        = 154,
/*KEY88_SPACE       = 155,*/
  KEY88_ESC     = 156,
  KEY88_TAB     = 157,
  KEY88_DOWN        = 158,
  KEY88_LEFT        = 159,
  KEY88_HELP        = 160,
  KEY88_COPY        = 161,
  KEY88_CAPS        = 162,
  KEY88_ROLLUP      = 163,
  KEY88_ROLLDOWN    = 164,

  /* ファンクションキー用の定義 */

  KEY88_F1      = 165,
  KEY88_F2      = 166,
  KEY88_F3      = 167,
  KEY88_F4      = 168,
  KEY88_F5      = 169,

  /* メニュー用ファンクションキーの定義 */

  KEY88_F11     = 170,
  KEY88_F12     = 171,
  KEY88_F13     = 172,
  KEY88_F14     = 173,
  KEY88_F15     = 174,
  KEY88_F16     = 175,
  KEY88_F17     = 176,
  KEY88_F18     = 177,
  KEY88_F19     = 178,
  KEY88_F20     = 179,

  /* 後期型ファンクションキー用の定義 */

  KEY88_F6      = 180,
  KEY88_F7      = 181,
  KEY88_F8      = 182,
  KEY88_F9      = 183,
  KEY88_F10     = 184,

  /* 後期型特殊キー用の定義 */

  KEY88_BS      = 185,
  KEY88_INS     = 186,
  KEY88_DEL     = 187,
  KEY88_HENKAN      = 188,
  KEY88_KETTEI      = 189,
  KEY88_PC      = 190,
  KEY88_ZENKAKU     = 191,
  KEY88_RETURNL     = 192,
  KEY88_RETURNR     = 193,
  KEY88_SHIFTL      = 194,
  KEY88_SHIFTR      = 195,


  /* マウス用の定義 */

  KEY88_MOUSE_UP    = 208,
  KEY88_MOUSE_DOWN  = 209,
  KEY88_MOUSE_LEFT  = 210,
  KEY88_MOUSE_RIGHT = 211,
  KEY88_MOUSE_L     = 212,
  KEY88_MOUSE_M     = 213,
  KEY88_MOUSE_R     = 214,
  KEY88_MOUSE_WUP   = 215,
  KEY88_MOUSE_WDN   = 216,

  /* ジョイパッド用の定義 */

  KEY88_PAD1_UP     = 224,
  KEY88_PAD1_DOWN   = 225,
  KEY88_PAD1_LEFT   = 226,
  KEY88_PAD1_RIGHT  = 227,
  KEY88_PAD1_A      = 228,
  KEY88_PAD1_B      = 229,
  KEY88_PAD1_C      = 230,
  KEY88_PAD1_D      = 231,
  KEY88_PAD1_E      = 232,
  KEY88_PAD1_F      = 233,
  KEY88_PAD1_G      = 234,
  KEY88_PAD1_H      = 235,

  KEY88_PAD2_UP     = 236,
  KEY88_PAD2_DOWN   = 237,
  KEY88_PAD2_LEFT   = 238,
  KEY88_PAD2_RIGHT  = 239,
  KEY88_PAD2_A      = 240,
  KEY88_PAD2_B      = 241,
  KEY88_PAD2_C      = 242,
  KEY88_PAD2_D      = 243,
  KEY88_PAD2_E      = 244,
  KEY88_PAD2_F      = 245,
  KEY88_PAD2_G      = 246,
  KEY88_PAD2_H      = 247,


  /* 248〜255 はシステム用にリサーブしておく */

  KEY88_SYS_STATUS  = 254,
  KEY88_SYS_MENU    = 255,

  KEY88_END     = 256
};



/*---------------------------------------------------------------------------
 *  キーコード と I/O ポートの対応
 *---------------------------------------------------------------------------*/

#define Port0   0x00
#define Port1   0x01
#define Port2   0x02
#define Port3   0x03
#define Port4   0x04
#define Port5   0x05
#define Port6   0x06
#define Port7   0x07
#define Port8   0x08
#define Port9   0x09
#define PortA   0x0a
#define PortB   0x0b
#define PortC   0x0c
#define PortD   0x0d
#define PortE   0x0e
#define PortX   0x0f

#define Bit0    0x01
#define Bit1    0x02
#define Bit2    0x04
#define Bit3    0x08
#define Bit4    0x10
#define Bit5    0x20
#define Bit6    0x40
#define Bit7    0x80

#define PadA    Bit4
#define PadB    Bit5

#define PadU    Bit0
#define PadD    Bit1
#define PadL    Bit2
#define PadR    Bit3


enum {
    /* 後期型キーの別名定義 */

    KEY88_EXT_F6 = KEY88_END + 0,
    KEY88_EXT_F7 = KEY88_END + 1,
    KEY88_EXT_F8 = KEY88_END + 2,
    KEY88_EXT_F9 = KEY88_END + 3,
    KEY88_EXT_F10 = KEY88_END + 4,
    KEY88_EXT_BS = KEY88_END + 5,
    KEY88_EXT_INS = KEY88_END + 6,
    KEY88_EXT_DEL = KEY88_END + 7,
    KEY88_EXT_HENKAN = KEY88_END + 8,
    KEY88_EXT_KETTEI = KEY88_END + 9,
    KEY88_EXT_PC = KEY88_END + 10,
    KEY88_EXT_ZENKAKU = KEY88_END + 11,
    KEY88_EXT_RETURNL = KEY88_END + 12,
    KEY88_EXT_RETURNR = KEY88_END + 13,
    KEY88_EXT_SHIFTL = KEY88_END + 14,
    KEY88_EXT_SHIFTR = KEY88_END + 15,

    KEY88_EXT_END = KEY88_END + 16
};


typedef struct {
    unsigned char   port;
    unsigned char   mask;
} T_KEYPORT;

static const T_KEYPORT keyport[KEY88_EXT_END] =
{
  { 0,0 },          /*    KEY88_INVALID     = 0,    */

  { 0,0 },{ 0,0 },{ 0,0 },
  { 0,0 },{ 0,0 },{ 0,0 },{ 0,0 },
  { 0,0 },{ 0,0 },{ 0,0 },{ 0,0 },
  { 0,0 },{ 0,0 },{ 0,0 },{ 0,0 },
  { 0,0 },{ 0,0 },{ 0,0 },{ 0,0 },
  { 0,0 },{ 0,0 },{ 0,0 },{ 0,0 },
  { 0,0 },{ 0,0 },{ 0,0 },{ 0,0 },
  { 0,0 },{ 0,0 },{ 0,0 },{ 0,0 },

  { Port9, Bit6 },      /*    KEY88_SPACE       = 32,   */
  { Port6, Bit1 },      /*    KEY88_EXCLAM      = 33,   */
  { Port6, Bit2 },      /*    KEY88_QUOTEDBL    = 34,   */
  { Port6, Bit3 },      /*    KEY88_NUMBERSIGN  = 35,   */
  { Port6, Bit4 },      /*    KEY88_DOLLAR      = 36,   */
  { Port6, Bit5 },      /*    KEY88_PERCENT     = 37,   */
  { Port6, Bit6 },      /*    KEY88_AMPERSAND   = 38,   */
  { Port6, Bit7 },      /*    KEY88_APOSTROPHE  = 39,   */
  { Port7, Bit0 },      /*    KEY88_PARENLEFT   = 40,   */
  { Port7, Bit1 },      /*    KEY88_PARENRIGHT  = 41,   */
  { Port7, Bit2 },      /*    KEY88_ASTERISK    = 42,   */
  { Port7, Bit3 },      /*    KEY88_PLUS        = 43,   */
  { Port7, Bit4 },      /*    KEY88_COMMA       = 44,   */
  { Port5, Bit7 },      /*    KEY88_MINUS       = 45,   */
  { Port7, Bit5 },      /*    KEY88_PERIOD      = 46,   */
  { Port7, Bit6 },      /*    KEY88_SLASH       = 47,   */
  { Port6, Bit0 },      /*    KEY88_0       = 48,   */
  { Port6, Bit1 },      /*    KEY88_1       = 49,   */
  { Port6, Bit2 },      /*    KEY88_2       = 50,   */
  { Port6, Bit3 },      /*    KEY88_3       = 51,   */
  { Port6, Bit4 },      /*    KEY88_4       = 52,   */
  { Port6, Bit5 },      /*    KEY88_5       = 53,   */
  { Port6, Bit6 },      /*    KEY88_6       = 54,   */
  { Port6, Bit7 },      /*    KEY88_7       = 55,   */
  { Port7, Bit0 },      /*    KEY88_8       = 56,   */
  { Port7, Bit1 },      /*    KEY88_9       = 57,   */
  { Port7, Bit2 },      /*    KEY88_COLON       = 58,   */
  { Port7, Bit3 },      /*    KEY88_SEMICOLON   = 59,   */
  { Port7, Bit4 },      /*    KEY88_LESS        = 60,   */
  { Port5, Bit7 },      /*    KEY88_EQUAL       = 61,   */
  { Port7, Bit5 },      /*    KEY88_GREATER     = 62,   */
  { Port7, Bit6 },      /*    KEY88_QUESTION    = 63,   */
  { Port2, Bit0 },      /*    KEY88_AT      = 64,   */
  { Port2, Bit1 },      /*    KEY88_A       = 65,   */
  { Port2, Bit2 },      /*    KEY88_B       = 66,   */
  { Port2, Bit3 },      /*    KEY88_C       = 67,   */
  { Port2, Bit4 },      /*    KEY88_D       = 68,   */
  { Port2, Bit5 },      /*    KEY88_E       = 69,   */
  { Port2, Bit6 },      /*    KEY88_F       = 70,   */
  { Port2, Bit7 },      /*    KEY88_G       = 71,   */
  { Port3, Bit0 },      /*    KEY88_H       = 72,   */
  { Port3, Bit1 },      /*    KEY88_I       = 73,   */
  { Port3, Bit2 },      /*    KEY88_J       = 74,   */
  { Port3, Bit3 },      /*    KEY88_K       = 75,   */
  { Port3, Bit4 },      /*    KEY88_L       = 76,   */
  { Port3, Bit5 },      /*    KEY88_M       = 77,   */
  { Port3, Bit6 },      /*    KEY88_N       = 78,   */
  { Port3, Bit7 },      /*    KEY88_O       = 79,   */
  { Port4, Bit0 },      /*    KEY88_P       = 80,   */
  { Port4, Bit1 },      /*    KEY88_Q       = 81,   */
  { Port4, Bit2 },      /*    KEY88_R       = 82,   */
  { Port4, Bit3 },      /*    KEY88_S       = 83,   */
  { Port4, Bit4 },      /*    KEY88_T       = 84,   */
  { Port4, Bit5 },      /*    KEY88_U       = 85,   */
  { Port4, Bit6 },      /*    KEY88_V       = 86,   */
  { Port4, Bit7 },      /*    KEY88_W       = 87,   */
  { Port5, Bit0 },      /*    KEY88_X       = 88,   */
  { Port5, Bit1 },      /*    KEY88_Y       = 89,   */
  { Port5, Bit2 },      /*    KEY88_Z       = 90,   */
  { Port5, Bit3 },      /*    KEY88_BRACKETLEFT = 91,   */
  { Port5, Bit4 },      /*    KEY88_YEN     = 92,   */
  { Port5, Bit5 },      /*    KEY88_BRACKETRIGHT    = 93,   */
  { Port5, Bit6 },      /*    KEY88_CARET       = 94,   */
  { Port7, Bit7 },      /*    KEY88_UNDERSCORE  = 95,   */
  { Port2, Bit0 },      /*    KEY88_BACKQUOTE   = 96,   */
  { Port2, Bit1 },      /*    KEY88_a       = 97,   */
  { Port2, Bit2 },      /*    KEY88_b       = 98,   */
  { Port2, Bit3 },      /*    KEY88_c       = 99,   */
  { Port2, Bit4 },      /*    KEY88_d       = 100,  */
  { Port2, Bit5 },      /*    KEY88_e       = 101,  */
  { Port2, Bit6 },      /*    KEY88_f       = 102,  */
  { Port2, Bit7 },      /*    KEY88_g       = 103,  */
  { Port3, Bit0 },      /*    KEY88_h       = 104,  */
  { Port3, Bit1 },      /*    KEY88_i       = 105,  */
  { Port3, Bit2 },      /*    KEY88_j       = 106,  */
  { Port3, Bit3 },      /*    KEY88_k       = 107,  */
  { Port3, Bit4 },      /*    KEY88_l       = 108,  */
  { Port3, Bit5 },      /*    KEY88_m       = 109,  */
  { Port3, Bit6 },      /*    KEY88_n       = 110,  */
  { Port3, Bit7 },      /*    KEY88_o               = 111,  */
  { Port4, Bit0 },      /*    KEY88_p               = 112,  */
  { Port4, Bit1 },      /*    KEY88_q               = 113,  */
  { Port4, Bit2 },      /*    KEY88_r               = 114,  */
  { Port4, Bit3 },      /*    KEY88_s               = 115,  */
  { Port4, Bit4 },      /*    KEY88_t               = 116,  */
  { Port4, Bit5 },      /*    KEY88_u               = 117,  */
  { Port4, Bit6 },      /*    KEY88_v               = 118,  */
  { Port4, Bit7 },      /*    KEY88_w               = 119,  */
  { Port5, Bit0 },      /*    KEY88_x       = 120,  */
  { Port5, Bit1 },      /*    KEY88_y       = 121,  */
  { Port5, Bit2 },      /*    KEY88_z       = 122,  */
  { Port5, Bit3 },      /*    KEY88_BRACELEFT   = 123,  */
  { Port5, Bit4 },      /*    KEY88_BAR     = 124,  */
  { Port5, Bit5 },      /*    KEY88_BRACERIGHT  = 125,  */
  { Port5, Bit6 },      /*    KEY88_TILDE       = 126,  */
  {     0,    0 },
  { Port0, Bit0 },      /*    KEY88_KP_0        = 128,  */
  { Port0, Bit1 },      /*    KEY88_KP_1        = 129,  */
  { Port0, Bit2 },      /*    KEY88_KP_2        = 130,  */
  { Port0, Bit3 },      /*    KEY88_KP_3        = 131,  */
  { Port0, Bit4 },      /*    KEY88_KP_4        = 132,  */
  { Port0, Bit5 },      /*    KEY88_KP_5        = 133,  */
  { Port0, Bit6 },      /*    KEY88_KP_6        = 134,  */
  { Port0, Bit7 },      /*    KEY88_KP_7        = 135,  */
  { Port1, Bit0 },      /*    KEY88_KP_8        = 136,  */
  { Port1, Bit1 },      /*    KEY88_KP_9        = 137,  */
  { Port1, Bit2 },      /*    KEY88_KP_MULTIPLY = 138,  */
  { Port1, Bit3 },      /*    KEY88_KP_ADD      = 139,  */
  { Port1, Bit4 },      /*    KEY88_KP_EQUAL    = 140,  */
  { Port1, Bit5 },      /*    KEY88_KP_COMMA    = 141,  */
  { Port1, Bit6 },      /*    KEY88_KP_PERIOD   = 142,  */
  { PortA, Bit5 },      /*    KEY88_KP_SUB      = 143,  */
  { PortA, Bit6 },      /*    KEY88_KP_DIVIDE   = 144,  */

  { Port1, Bit7 },      /*    KEY88_RETURN      = 145,  */
  { Port8, Bit0 },      /*    KEY88_HOME        = 146,  */
  { Port8, Bit1 },      /*    KEY88_UP      = 147,  */
  { Port8, Bit2 },      /*    KEY88_RIGHT       = 148,  */
  { Port8, Bit3 },      /*    KEY88_INS_DEL     = 149,  */
  { Port8, Bit4 },      /*    KEY88_GRAPH       = 150,  */
  { Port8, Bit5 },      /*    KEY88_KANA        = 151,  */
  { Port8, Bit6 },      /*    KEY88_SHIFT       = 152,  */
  { Port8, Bit7 },      /*    KEY88_CTRL        = 153,  */
  { Port9, Bit0 },      /*    KEY88_STOP        = 154,  */
  { Port9, Bit6 },      /*    KEY88_SPACE       = 155,  */
  { Port9, Bit7 },      /*    KEY88_ESC     = 156,  */
  { PortA, Bit0 },      /*    KEY88_TAB     = 157,  */
  { PortA, Bit1 },      /*    KEY88_DOWN        = 158,  */
  { PortA, Bit2 },      /*    KEY88_LEFT        = 159,  */
  { PortA, Bit3 },      /*    KEY88_HELP        = 160,  */
  { PortA, Bit4 },      /*    KEY88_COPY        = 161,  */
  { PortA, Bit7 },      /*    KEY88_CAPS        = 162,  */
  { PortB, Bit0 },      /*    KEY88_ROLLUP      = 163,  */
  { PortB, Bit1 },      /*    KEY88_ROLLDOWN    = 164,  */

  { Port9, Bit1 },      /*    KEY88_F1      = 165,  */
  { Port9, Bit2 },      /*    KEY88_F2      = 166,  */
  { Port9, Bit3 },      /*    KEY88_F3      = 167,  */
  { Port9, Bit4 },      /*    KEY88_F4      = 168,  */
  { Port9, Bit5 },      /*    KEY88_F5      = 169,  */

  { 0,0 },{ 0,0 },
  { 0,0 },{ 0,0 },{ 0,0 },{ 0,0 },
  { 0,0 },{ 0,0 },{ 0,0 },{ 0,0 },

  { Port9, Bit1 },  /*f-1*/ /*    KEY88_F6      = 180,  */
  { Port9, Bit2 },  /*f-2*/ /*    KEY88_F7      = 181,  */
  { Port9, Bit3 },  /*f-3*/ /*    KEY88_F8      = 182,  */
  { Port9, Bit4 },  /*f-4*/ /*    KEY88_F9      = 183,  */
  { Port9, Bit5 },  /*f-5*/ /*    KEY88_F10     = 184,  */
  { Port8, Bit3 },  /*del*/ /*    KEY88_BS      = 185,  */
  { Port8, Bit3 },  /*del*/ /*    KEY88_INS     = 186,  */
  { Port8, Bit3 },  /*del*/ /*    KEY88_DEL     = 187,  */
  { Port9, Bit6 },  /*spc*/ /*    KEY88_HENKAN      = 188,  */
  { Port9, Bit6 },  /*spc*/ /*    KEY88_KETTEI      = 189,  */
  {     0,    0 },      /*    KEY88_PC      = 190,  */
  {     0,    0 },      /*    KEY88_ZENKAKU     = 191,  */
  { Port1, Bit7 },  /*ret*/ /*    KEY88_RETURNL     = 192,  */
  { Port1, Bit7 },  /*ret*/ /*    KEY88_RETURNR     = 193,  */
  { Port8, Bit6 },  /*sft*/ /*    KEY88_SHIFTL      = 194,  */
  { Port8, Bit6 },  /*sft*/ /*    KEY88_SHIFTR      = 195,  */

  { 0,0 },{ 0,0 },{ 0,0 },{ 0,0 },
  { 0,0 },{ 0,0 },{ 0,0 },{ 0,0 },
  { 0,0 },{ 0,0 },{ 0,0 },{ 0,0 },

  {     0,    0 },      /*    KEY88_MOUSE_UP        = 208,  */
  {     0,    0 },      /*    KEY88_MOUSE_DOWN      = 209,  */
  {     0,    0 },      /*    KEY88_MOUSE_LEFT      = 210,  */
  {     0,    0 },      /*    KEY88_MOUSE_RIGHT     = 211,  */
  {     0,    0 },      /*    KEY88_MOUSE_L         = 212,  */
  {     0,    0 },      /*    KEY88_MOUSE_M         = 213,  */
  {     0,    0 },      /*    KEY88_MOUSE_R         = 214,  */
  {     0,    0 },      /*    KEY88_MOUSE_WUP       = 215,  */
  {     0,    0 },      /*    KEY88_MOUSE_WDN       = 216,  */

  { 0,0 },{ 0,0 },{ 0,0 },
  { 0,0 },{ 0,0 },{ 0,0 },{ 0,0 },

  { PortX, PadU },      /*    KEY88_PAD1_UP         = 224,  */
  { PortX, PadD },      /*    KEY88_PAD1_DOWN       = 225,  */
  { PortX, PadL },      /*    KEY88_PAD1_LEFT       = 226,  */
  { PortX, PadR },      /*    KEY88_PAD1_RIGHT      = 227,  */
  { PortX, PadA },      /*    KEY88_PAD1_A          = 228,  */
  { PortX, PadB },      /*    KEY88_PAD1_B          = 229,  */
  {     0,    0 },      /*    KEY88_PAD1_C          = 230,  */
  {     0,    0 },      /*    KEY88_PAD1_D          = 231,  */
  {     0,    0 },      /*    KEY88_PAD1_E          = 232,  */
  {     0,    0 },      /*    KEY88_PAD1_F          = 233,  */
  {     0,    0 },      /*    KEY88_PAD1_G          = 234,  */
  {     0,    0 },      /*    KEY88_PAD1_H          = 235,  */

  {     0,    0 },      /*    KEY88_PAD2_UP         = 236,  */
  {     0,    0 },      /*    KEY88_PAD2_DOWN       = 237,  */
  {     0,    0 },      /*    KEY88_PAD2_LEFT       = 238,  */
  {     0,    0 },      /*    KEY88_PAD2_RIGHT      = 239,  */
  {     0,    0 },      /*    KEY88_PAD2_A          = 240,  */
  {     0,    0 },      /*    KEY88_PAD2_B          = 241,  */
  {     0,    0 },      /*    KEY88_PAD2_C          = 242,  */
  {     0,    0 },      /*    KEY88_PAD2_D          = 243,  */
  {     0,    0 },      /*    KEY88_PAD2_E          = 244,  */
  {     0,    0 },      /*    KEY88_PAD2_F          = 245,  */
  {     0,    0 },      /*    KEY88_PAD2_G          = 246,  */
  {     0,    0 },      /*    KEY88_PAD2_H          = 247,  */

  { 0,0 },{ 0,0 },{ 0,0 },{ 0,0 },
  { 0,0 },{ 0,0 },
  { 0,0 },          /*    KEY88_SYS_STATUS      = 254,  */
  { 0,0 },          /*    KEY88_SYS_MENU        = 255,  */

  { PortC, Bit0 },      /*    KEY88_EXT_F6      = 256,  */
  { PortC, Bit1 },      /*    KEY88_EXT_F7      = 257,  */
  { PortC, Bit2 },      /*    KEY88_EXT_F8      = 258,  */
  { PortC, Bit3 },      /*    KEY88_EXT_F9      = 259,  */
  { PortC, Bit4 },      /*    KEY88_EXT_F10     = 260,  */
  { PortC, Bit5 },      /*    KEY88_EXT_BS      = 261,  */
  { PortC, Bit6 },      /*    KEY88_EXT_INS     = 262,  */
  { PortC, Bit7 },      /*    KEY88_EXT_DEL     = 263,  */
  { PortD, Bit0 },      /*    KEY88_EXT_HENKAN  = 264,  */
  { PortD, Bit1 },      /*    KEY88_EXT_KETTEI  = 265,  */
  { PortD, Bit2 },      /*    KEY88_EXT_PC      = 266,  */
  { PortD, Bit3 },      /*    KEY88_EXT_ZENKAKU = 267,  */
  { PortE, Bit0 },      /*    KEY88_EXT_RETURNL = 268,  */
  { PortE, Bit1 },      /*    KEY88_EXT_RETURNR = 269,  */
  { PortE, Bit2 },      /*    KEY88_EXT_SHIFTL  = 270,  */
  { PortE, Bit3 },      /*    KEY88_EXT_SHIFTR  = 271,  */
};



/*---------------------------------------------------------------------------
 *
 *---------------------------------------------------------------------------*/

#define IS_JOY_STATUS()     key_scan[ PortX ]


#define KEY88_PRESS(code)   \
    key_scan[ keyport[(code)].port ] &= ~keyport[(code)].mask

#define KEY88_RELEASE(code) \
    key_scan[ keyport[(code)].port ] |=  keyport[(code)].mask

#define KEY88_TOGGLE(code)  \
    key_scan[ keyport[(code)].port ] ^=  keyport[(code)].mask

#define IS_KEY88_PRESS(code)    \
    (~key_scan[ keyport[(code)].port ] & keyport[(code)].mask)

#define IS_KEY88_RELEASE(code)  \
    ( key_scan[ keyport[(code)].port ] & keyport[(code)].mask)


#define IS_KEY88_PRINTTABLE(c)  (32 <= (c) && (c) <= 144)
#define IS_KEY88_FUNCTION(c)    (KEY88_F1 <= (c) && (c) <= KEY88_F20)
#define IS_KEY88_LATTERTYPE(c)  (KEY88_F6 <= (c) && (c) <= KEY88_SHIFTR)


#endif  /* KEYBOARD_H_INCLUDED */
