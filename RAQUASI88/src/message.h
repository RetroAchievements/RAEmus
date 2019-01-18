/************************************************************************/
/*                                                                      */
/* これはメッセージの文字列を定義しているファイルです。           */
/* ここでは、日本語の文字列 を char 型の変数にセットしているため、  */
/* 8bit文字リテラルの扱えるコンパイラでないと、正常に動作しません。   */
/*                                                                      */
/************************************************************************/

/*----------------------------------------------------------------------*/
/* このファイルは、menu.c ファイル内にて include されています。       */
/*----------------------------------------------------------------------*/
/*                                  */
/* QUASI88 の タイトル表示およびメニュー画面表示にて表示される文字列は */
/* 全てこのファイルで定義されています。                   */
/*                                  */
/*----------------------------------------------------------------------*/

/*
#ifndef NR_DRIVE
#define NR_DRIVE    2
#endif
*/

/* 0xb4, 0xc1, 0xbb, 0xfa なら、 EUC-Japan */
/* 0x8a, 0xbf, 0x8e, 0x9a なら、 Shift-JIS */
/* それ以外なら、                不明……  */

/* このファイルが EUC-japan か Shift-JIS かをチェックする・・・ */

const char *menu_kanji_code = "漢字";
const char menu_kanji_code_euc[]  = { 0xb4, 0xc1, 0xbb, 0xfa, 0x00 };
const char menu_kanji_code_sjis[] = { 0x8a, 0xbf, 0x8e, 0x9a, 0x00 };
const char menu_kanji_code_utf8[] = { 0xe6, 0xbc, 0xa2, 0xe5, 0xad, 0x97, 0x00 };


typedef struct {
    char    *str[2];    /* [0]…ANK文字列  [1]…日本語文字列 */
    int     val;        /* int値 汎用                */
} t_menudata;

typedef struct {
    char    *str[2];    /* [0]…ANK文字列  [1]…日本語文字列 */
} t_menulabel;




/***************************************************************
 * QUASI88 の メニューにて使用する文字列
 ****************************************************************/

/*--------------------------------------------------------------
 *  メニューの '*' タブにて表示される情報
 *--------------------------------------------------------------*/

static const char *data_about_en[] =
{
#ifdef  USE_SOUND
    "MAME Sound Driver ... Available",
    "   " Q_MAME_COPYRIGHT,
    "@MAMEVER",
#ifdef  USE_FMGEN
    "",
    "FM Sound Generator ... Available",
    "   " Q_FMGEN_COPYRIGHT,
    "@FMGENVER",
#endif
#else
    "SOUND OUTPUT ... Not available",
#endif  
    "",
#ifdef  USE_MONITOR
    "Monitor mode ... Supported",
    "",
#endif

    NULL,   /* 終端 */
};


static const char *data_about_jp[] =
{
#ifdef  USE_SOUND
    "MAME サウンドドライバ が組み込まれています",
    "   " Q_MAME_COPYRIGHT,
    "@MAMEVER",
#ifdef  USE_FMGEN
    "",
    "FM Sound Generator が組み込まれています",
    "   " Q_FMGEN_COPYRIGHT,
    "@FMGENVER",
#endif
#else
    "サウンド出力 は組み込まれていません",
#endif
    "",
#ifdef  USE_MONITOR
    "モニターモードが使用できます",
    "",
#endif

    NULL,   /* 終端 */
};





/*--------------------------------------------------------------
 *  メインメニュー画面
 *--------------------------------------------------------------*/
enum {
  DATA_TOP_RESET,
  DATA_TOP_CPU,
  DATA_TOP_GRAPH,
  DATA_TOP_VOLUME,
  DATA_TOP_DISK,
  DATA_TOP_KEY,
  DATA_TOP_MOUSE,
  DATA_TOP_TAPE,
  DATA_TOP_MISC,
  DATA_TOP_ABOUT
};
static const t_menudata data_top[] =
{
  { { " RESET ",        " リセット ",       }, DATA_TOP_RESET  },
  { { " CPU ",          " CPU  ",           }, DATA_TOP_CPU    },
  { { " SCREEN ",       " 画面 ",           }, DATA_TOP_GRAPH  },
  { { " VOLUME ",       " 音量 ",           }, DATA_TOP_VOLUME },
  { { " DISK ",         " ディスク ",       }, DATA_TOP_DISK   },
  { { " KEY ",          " キー ",           }, DATA_TOP_KEY    },
  { { " MOUSE ",        " マウス ",         }, DATA_TOP_MOUSE  },
  { { " TAPE ",         " テープ ",         }, DATA_TOP_TAPE   },
  { { " MISC ",         "  他  ",           }, DATA_TOP_MISC   },
  { { " ABOUT ",        "※",               }, DATA_TOP_ABOUT  },
};


enum {
    DATA_TOP_STATUS_PAD,
    DATA_TOP_STATUS_CHK,
    DATA_TOP_STATUS_KEY
};
static const t_menulabel data_top_status[] =
{
  { { "              ",  "              " } },
  { { "Status",          "ステータス"     } },
  { { "        (F11) ",  "         (F11)" } },
};


enum {
    DATA_TOP_MONITOR_PAD,
    DATA_TOP_MONITOR_BTN
};
static const t_menulabel data_top_monitor[] =
{
  { { "            ",  "            ", } },
  { { " MONITOR  ",    " モニター ",   } },
};


enum {
  DATA_TOP_SAVECFG,
  DATA_TOP_QUIT,
  DATA_TOP_EXIT
};
static const t_menudata data_top_button[] =
{
  { { " Save Cfg.",    " 設定保存 ",      }, DATA_TOP_SAVECFG },
  { { " QUIT(F12) ",   " 終了(F12) ",     }, DATA_TOP_QUIT    },
  { { " EXIT(ESC) ",   " 戻る(ESC) ",     }, DATA_TOP_EXIT    },
};


enum {
  DATA_TOP_SAVECFG_TITLE,
  DATA_TOP_SAVECFG_INFO,
  DATA_TOP_SAVECFG_AUTO,
  DATA_TOP_SAVECFG_OK,
  DATA_TOP_SAVECFG_CANCEL
};
static const t_menulabel data_top_savecfg[] =
{
  { { "Save settings in following file. ", "現在の設定を、以下の環境設定ファイルに保存します" } },
  { { "(Some settings are not saved)    ", "（一部の設定は、保存されません）                " } },
  { { "Save when QUASI88 exit. ",          "終了時に、自動で保存する"                         } },
  { { "   OK   ",                          " 保存 "                                           } },
  { { " CANCEL ",                          " 取消 "                                           } },
};


enum {
  DATA_TOP_QUIT_TITLE,
  DATA_TOP_QUIT_OK,
  DATA_TOP_QUIT_CANCEL
};
static const t_menulabel data_top_quit[] =
{
  { { " *** QUIT NOW, REALLY ? *** ", "本当に終了して、よろしいか？" } },
  { { "   OK   (F12) ",               " 終了 (F12) "                 } },
  { { " CANCEL (ESC) ",               " 取消 (ESC) "                 } },
};


static const t_menudata data_quickres_basic[] =
{
  { { "V2 ", "V2 ", }, BASIC_V2,  },
  { { "V1H", "V1H", }, BASIC_V1H, },
  { { "V1S", "V1S", }, BASIC_V1S, },
  { { "N",   "N",   }, BASIC_N,   },    /* 非表示… */
};
static const t_menudata data_quickres_clock[] =
{
  { { "4MHz", "4MHz", }, CLOCK_4MHZ, },
  { { "8MHz", "8MHz", }, CLOCK_8MHZ, },
};
static const t_menulabel data_quickres_reset[] =
{
  { { "RST", "RST" } },
};






/*--------------------------------------------------------------
 *  「画面」 タブ
 *--------------------------------------------------------------*/
enum {
  DATA_GRAPH_FRATE,
  DATA_GRAPH_RESIZE,
  DATA_GRAPH_PCG,
  DATA_GRAPH_FONT
};
static const t_menulabel data_graph[] =
{
  { { " <<< FRAME RATE >>> ", " フレームレート " } },
  { { " <<< RESIZE >>> ",     " 画面サイズ "     } },
  { { " <<< PCG-8100 >>>",    " PCG-8100 "       } },
  { { " <<< FONT >>> ",       " フォント "       } },
};

static const t_menudata data_graph_frate[] =
{
  { { "60", "60", },   1, },
  { { "30", "30", },   2, },
  { { "20", "20", },   3, },
  { { "15", "15", },   4, },
  { { "12", "12", },   5, },
  { { "10", "10", },   6, },
  { { "6",  "6",  },  10, },
  { { "5",  "5",  },  12, },
  { { "4",  "4",  },  15, },
  { { "3",  "3",  },  20, },
  { { "2",  "2",  },  30, },
  { { "1",  "1",  },  60, },
};

static const t_menulabel data_graph_autoskip[] =
{
  { { "Auto frame skip (-autoskip) ", "オートスキップを有効にする (-autoskip) ", } },
};

static const t_menudata data_graph_resize[] =
{
  { { " HALF SIZE (-half) ",     " 半分サイズ (-half) ", }, SCREEN_SIZE_HALF,   },
  { { " FULL SIZE (-full) ",     " 標準サイズ (-full) ", }, SCREEN_SIZE_FULL,   },
#ifdef  SUPPORT_DOUBLE
  { { " DOUBLE SIZE (-double) ", " 倍サイズ (-double) ", }, SCREEN_SIZE_DOUBLE, },
#endif
};

static const t_menulabel data_graph_fullscreen[] =
{
  { { "Full Screen (-fullscreen)", "フルスクリーン (-fullscreen) ", } },
};

enum {
  DATA_GRAPH_MISC_15K,
  DATA_GRAPH_MISC_DIGITAL,
  DATA_GRAPH_MISC_NOINTERP
};
static const t_menudata data_graph_misc[] =
{
  { { "Monitor Freq. 15k       (-15k)",         "モニタ周波数を15kに設定      (-15k)"        }, DATA_GRAPH_MISC_15K        },
  { { "Digital Monitor         (-digital)",     "デジタルモニタに設定         (-digital)"    }, DATA_GRAPH_MISC_DIGITAL    },
  { { "No reduce interpolation (-nointerp)",    "半分サイズ時に縮小補間しない (-nointerp)"   }, DATA_GRAPH_MISC_NOINTERP   },
};

static const t_menudata data_graph_misc2[] =
{
  { { "Fill-Line Display       (-noskipline)",  "ラインの隙間を埋める         (-noskipline)" }, SCREEN_INTERLACE_NO   },
  { { "Skip-Line Display       (-skipline)",    "1ラインおきに表示する        (-skipline)"   }, SCREEN_INTERLACE_SKIP },
  { { "Interlace Display       (-interlace)",   "インターレース表示する       (-interlace)"  }, SCREEN_INTERLACE_YES  },
};

static const t_menudata data_graph_pcg[] =
{
  { { " Noexist ", " なし "  }, FALSE },
  { { " Exist ",   " あり  " }, TRUE  },
};

#if 0
static const t_menudata data_graph_font[] =
{
  { { " Standard Font ",  " 標準フォント " }, 0 },
  { { " 2nd Font ",       " 第２フォント " }, 1 },
  { { " 3rd Font ",       " 第３フォント " }, 2 },
};
#else
static const t_menudata data_graph_font1[2] =
{
  { { " Built-in Font ",  " 内 蔵 フォント " }, 0 },
  { { " Standard Font ",  " 標 準 フォント " }, 0 },
};
static const t_menudata data_graph_font2[2] =
{
  { { " Hiragana Font ",  " 平仮名フォント " }, 1 },
  { { " 2nd Font ",       " 第 ２ フォント " }, 1 },
};
static const t_menudata data_graph_font3[2] =
{
  { { " Transparent Font ", " 透 明 フォント " }, 2 },
  { { " 3rd Font ",         " 第 ３ フォント " }, 2 },
};
#endif



/*--------------------------------------------------------------
 *  「CPU」 タブ
 *--------------------------------------------------------------*/
enum {
  DATA_CPU_CPU,
  DATA_CPU_CLOCK,
  DATA_CPU_WAIT,
  DATA_CPU_BOOST,
  DATA_CPU_HELP
};
static const t_menulabel data_cpu[] =
{
  { { " <<< SUB-CPU MODE >>> ", " SUB-CPU駆動 <変更時はリセットを推奨> ", } },
  { { " << CLOCK >> ",          " CPU クロック (-clock) ",                } },
  { { " << WAIT >> ",           " 速度 (-speed, -nowait) ",               } },
  { { " << BOOST >> ",          " ブースト (-boost) ",                    } },
  { { " HELP ",                 " 説明 ",                                 } },
};



static const t_menudata data_cpu_cpu[] =
{
  { { "   0  Run SUB-CPU only during the disk access. (-cpu 0)  ", "   0  ディスク処理中、サブCPUのみ駆動させる (-cpu 0)  ", }, 0 },
  { { "   1  Run both CPUs during the disk access.    (-cpu 1)  ", "   1  ディスク処理中、両CPUを駆動させる     (-cpu 1)  ", }, 1 },
  { { "   2  Always run both CPUs.                    (-cpu 2)  ", "   2  常時、両CPUを駆動させる               (-cpu 2)  ", }, 2 },
};


enum {
  DATA_CPU_CLOCK_CLOCK,
  DATA_CPU_CLOCK_MHZ,
  DATA_CPU_CLOCK_INFO
};
static const t_menulabel data_cpu_clock[] =
{
  { { " CLOCK     ",          " 周波数 "              }, },
  { { "[MHz] ",               "[MHz] "                }, },
#if USE_RETROACHIEVEMENTS
  { { "(Range = 4.0-998.4) ", "（範囲＝4.0〜998.4） " }, },
#else
  { { "(Range = 0.1-998.4) ", "（範囲＝0.1〜998.4） " }, },
#endif
};
static const t_menudata data_cpu_clock_combo[] =
{
#if !USE_RETROACHIEVEMENTS
  { { " ( 1MHz) ",  " ( 1MHz) ", }, (int)(CONST_4MHZ_CLOCK * 1000000.0/4) },
  { { " ( 2MHz) ",  " ( 2MHz) ", }, (int)(CONST_4MHZ_CLOCK * 1000000.0/2) },
#endif
  { { "== 4MHz==",  "== 4MHz==", }, (int)(CONST_4MHZ_CLOCK * 1000000.0)   },
  { { " ( 8MHz) ",  " ( 8MHz) ", }, (int)(CONST_8MHZ_CLOCK * 1000000.0)   },
  { { " (16MHz) ",  " (16MHz) ", }, (int)(CONST_8MHZ_CLOCK * 1000000.0*2) },
  { { " (32MHz) ",  " (32MHz) ", }, (int)(CONST_8MHZ_CLOCK * 1000000.0*4) },
  { { " (64MHz) ",  " (64MHz) ", }, (int)(CONST_8MHZ_CLOCK * 1000000.0*8) },
};


enum {
  DATA_CPU_WAIT_NOWAIT,
  DATA_CPU_WAIT_RATE,
  DATA_CPU_WAIT_PERCENT,
  DATA_CPU_WAIT_INFO
};
static const t_menulabel data_cpu_wait[] =
{
  { { "No Wait             ", "ウエイトなしにする  "  } },
  { { " Rate of Speed ",      " 速度比     "          } },
  { { "[%]   ",               "[％]  "                } },
#if USE_RETROACHIEVEMENTS
  { { "(Range =   100-5000)  ", "（範囲＝  100〜5000）  " } },
#else
  { { "(Range =   5-5000)  ", "（範囲＝  5〜5000）  " } },
#endif
};
static const t_menudata data_cpu_wait_combo[] =
{
  { { "  25",  "  25" },   25 },
  { { "  50",  "  50" },   50 },
  { { " 100",  " 100" },  100 },
  { { " 200",  " 200" },  200 },
  { { " 400",  " 400" },  400 },
  { { " 800",  " 800" },  800 },
  { { "1600",  "1600" }, 1600 },
};


enum {
  DATA_CPU_BOOST_MAGNIFY,
  DATA_CPU_BOOST_UNIT,
  DATA_CPU_BOOST_INFO
};
static const t_menulabel data_cpu_boost[] =
{
  { { " Power         ",      " 倍率       "          } },
  { { "      ",               "倍    "                } },
  { { "(Range =   1-100)   ", "（範囲＝  1〜100）   " } },
};
static const t_menudata data_cpu_boost_combo[] =
{
  { { "   1", "   1", },   1, },
  { { "   2", "   2", },   2, },
  { { "   4", "   4", },   4, },
  { { "   8", "   8", },   8, },
  { { "  16", "  16", },  16, },
};



enum {
  DATA_CPU_MISC_FDCWAIT,
  DATA_CPU_MISC_FDCWAIT_X,
  DATA_CPU_MISC_BLANK,
  DATA_CPU_MISC_HSBASIC,
  DATA_CPU_MISC_HSBASIC_X,
  DATA_CPU_MISC_BLANK2,
  DATA_CPU_MISC_MEMWAIT,
  DATA_CPU_MISC_MEMWAIT_X,
  DATA_CPU_MISC_CMDSING,
};
static const t_menudata data_cpu_misc[] =
{
  { { "FDC Wait ON",        "FDCウエイトあり ",       }, DATA_CPU_MISC_FDCWAIT },
  { { "(-fdc_wait)",        "(-fdc_wait)",            }, -1                    },
  { { "",                   "",                       }, -1                    },
  { { "HighSpeed BASIC ON", "高速BASIC処理 有効 ",    }, DATA_CPU_MISC_HSBASIC },
  { { "(-hsbasic)",         "(-hsbasic)",             }, -1                    },
  { { "",                   "",                       }, -1                    },
  { { "Memory Wait(dummy)", "偽メモリウェイト",       }, DATA_CPU_MISC_MEMWAIT },
  { { "(-mem_wait)",        "(-mem_wait)",            }, -1                    },

#if 0
  { { "",                   "",                       }, -1                    },
  { { "CMD SING",           "CMD SING",               }, DATA_CPU_MISC_CMDSING },
#endif
};



/*--------------------------------------------------------------
 *  「リセット」 タブ
 *--------------------------------------------------------------*/
enum {
  DATA_RESET_CURRENT,
  DATA_RESET_BASIC,
  DATA_RESET_CLOCK,
  DATA_RESET_VERSION,
  DATA_RESET_DIPSW,
  DATA_RESET_DIPSW_BTN,
  DATA_RESET_SOUND,
  DATA_RESET_EXTRAM,
  DATA_RESET_JISHO,
  DATA_RESET_NOTICE,
  DATA_RESET_DIPSW_SET,
  DATA_RESET_DIPSW_QUIT,
  DATA_RESET_BOOT,
  DATA_RESET_NOW,
  DATA_RESET_INFO
};
static const t_menulabel data_reset[] =
{
  { { " Current Mode : ",         " 現在のモード ："               } },
  { { " BASIC MODE ",             " BASIC モード "                 } },
  { { " CPU CLOCK ",              " CPU クロック "                 } },
  { { " ROM VERSION ",            " ROM バージョン "               } },
  { { " DIP-Switch ",             " ディップスイッチ "             } },
  { { " Setting ",                " 設定 "                         } },
  { { " Sound Board",             " サウンドボード "               } },
  { { " ExtRAM",                  " 拡張RAM "                      } },
  { { " Dict.ROM",                " 辞書ROM "                      } },
  { { "(*) When checked, Real CPU clock depend on the 'CPU'-TAB setting. ",
      "(＊) チェックが入っている場合、実際のクロックは『CPU』タブ設定のままとなります" } },
  { { " <<< DIP-SW Setting >>> ", " <<< ディップスイッチ設定 >>> " } },
  { { " EXIT ",                   " 戻る "                         } },
  { { " BOOT ",                   " 起動 "                         } },
  { { " RESET now ! ",            " この設定でリセットする "       } },
  { { " (Without a reset, the setting is not applied.) ",
      " （リセットをしないと、設定は反映されません）"              } },
};

static const t_menudata data_reset_basic[] =
{
  { { " N88 V2  ", " N88 V2  ", }, BASIC_V2,  },
  { { " N88 V1H ", " N88 V1H ", }, BASIC_V1H, },
  { { " N88 V1S ", " N88 V1S ", }, BASIC_V1S, },
  { { " N       ", " N       ", }, BASIC_N,   },
};

static const t_menudata data_reset_clock[] =
{
  { { " 4MHz ", " 4MHz ", }, CLOCK_4MHZ, },
  { { " 8MHz ", " 8MHz ", }, CLOCK_8MHZ, },
};

static const t_menulabel data_reset_clock_async[] =
{
  { { "Async (*) ",   "非連動 (＊) ", }, },
};

static const t_menudata data_reset_version[] =
{
  { { "Default",  " 既定値" }, 0   },
  { { "  1.0",    "  1.0"   }, '0' },
  { { "  1.1",    "  1.1"   }, '1' },
  { { "  1.2",    "  1.2"   }, '2' },
  { { "  1.3",    "  1.3"   }, '3' },
  { { "  1.4",    "  1.4"   }, '4' },
  { { "  1.5",    "  1.5"   }, '5' },
  { { "  1.6",    "  1.6"   }, '6' },
  { { "  1.7",    "  1.7"   }, '7' },
  { { "  1.8",    "  1.8"   }, '8' },
  { { "  1.9*",   "  1.9*"  }, '9' },
};

static const t_menulabel data_reset_boot[] =
{
  { { " Boot from DISK  ", "  ディスク " } },
  { { " Boot from ROM   ", "  ＲＯＭ   " } },
};

static const t_menudata data_reset_sound[] =
{
  { { " Sound board    (OPN)  ", " サウンドボード   (OPN)  ", }, SOUND_I  },
  { { " Sound board II (OPNA) ", " サウンドボードII (OPNA) ", }, SOUND_II },
};

static const t_menudata data_reset_extram[] =
{
  { { " Nothing ",   " なし    "  },  0 },
  { { "    128KB",   "    128KB"  },  1 },
  { { "    256KB",   "    256KB"  },  2 },
  { { "    384KB",   "    384KB"  },  3 },
  { { "    512KB",   "    512KB"  },  4 },
  { { "      1MB",   "      1MB"  },  8 },
  { { " 1M+128KB",   " 1M+128KB"  },  9 },
  { { " 1M+256KB",   " 1M+256KB"  }, 10 },
  { { "      2MB",   "      2MB"  }, 16 },
};

static const t_menudata data_reset_jisho[] =
{
  { { " no-jisho ",  " なし ", }, 0 },
  { { " has jisho ", " あり ", }, 1 },
};

static const t_menulabel data_reset_current[] =
{
  { { " ExtRAM", "拡張RAM" } },
  { { "DictROM", "辞書ROM" } },
};

static const t_menulabel data_reset_detail[] =
{
  { { " Misc. << ", " その他 << " } },
  { { " Misc. >> ", " その他 >> " } },
};



/*--------------------------------------------------------------
 *  「音量」 タブ
 *--------------------------------------------------------------*/
enum {
  DATA_VOLUME_TOTAL,
  DATA_VOLUME_LEVEL,
  DATA_VOLUME_DEPEND,
  DATA_VOLUME_AUDIO,
  DATA_VOLUME_AUDIO_SET,
  DATA_VOLUME_AUDIO_QUIT,
  DATA_VOLUME_AUDIO_INFO,
};
static const t_menulabel data_volume[] =
{
  { { " Volume ",             " 音量 "                       } },
  { { " Level ",              " レベル "                     } },
  { { " depend on FM-level ", " 以下はＦＭ音量に依存します " } },

  { { " Setting ",                                               " 詳細設定 "                                        } },
  { { " <<< Sound-device Setting >>> ",                          " <<< サウンドデバイス詳細設定 >>> "                } },
  { { " EXIT ",                                                  " 戻る "                                            } },
  { { " The settings are applied in the return from the menu. ", " 設定は、メニューモードから戻る際に反映されます。" } },
};



static const t_menulabel data_volume_no[] =
{
  { { " SoundBoard (OPN)                        [ Sound Output is not available. ] "," サウンドボード (OPN)                [ サウンド出力は組み込まれていません ] " } },
  { { " SoundBoard II (OPNA)                    [ Sound Output is not available. ] "," サウンドボードII (OPNA)             [ サウンド出力は組み込まれていません ] " } },
  { { " SoundBoard (OPN)                   [ Sound Output is OFF. ] ",               " サウンドボード (OPN)       [ サウンド出力はオフ状態です ] " } },
  { { " SoundBoard II (OPNA)               [ Sound Output is OFF. ] ",               " サウンドボードII (OPNA)    [ サウンド出力はオフ状態です ] " } },
};



static const t_menulabel data_volume_type[] =
{
  { { " SoundBoard (OPN)            [ MAME built-in FM Generator ] ",                " サウンドボード (OPN)      [ FM音源ジェネレータ：MAME内蔵 ] " } },
  { { " SoundBoard II (OPNA)        [ MAME built-in FM-Generator ] ",                " サウンドボードII (OPNA)   [ FM音源ジェネレータ：MAME内蔵 ] " } },
  { { " SoundBoard (OPN)                    [ fmgen FM-Generator ] ",                " サウンドボード (OPN)         [ FM音源ジェネレータ：fmgen ] " } },
  { { " SoundBoard II (OPNA)                [ fmgen FM-Generator ] ",                " サウンドボードII (OPNA)      [ FM音源ジェネレータ：fmgen ] " } },
};



typedef struct {
  char  *str[2];
  int   val;
  int   min;
  int   max;
  int   step;
  int   page;
} t_volume;



enum {
  VOL_TOTAL,
  VOL_FM,
  VOL_PSG,
  VOL_BEEP,
  VOL_RHYTHM,
  VOL_ADPCM,
  VOL_FMGEN,
  VOL_SAMPLE
};

static const t_volume data_volume_total[] =
{
  { { " VOLUME [db]    :",  " 音量 [ｄｂ]    ：" }, VOL_TOTAL, VOL_MIN,      VOL_MAX,      1, 4},
};

static const t_volume data_volume_level[] =
{
  { { " FM sound   [%] :",  " ＦＭ音量   [％]：" }, VOL_FM,    FMVOL_MIN,    FMVOL_MAX,    1,10},
  { { " PSG sound  [%] :",  " ＰＳＧ音量 [％]：" }, VOL_PSG,   PSGVOL_MIN,   PSGVOL_MAX,   1,10},
  { { " BEEP sound [%] :",  " ＢＥＥＰ音 [％]：" }, VOL_BEEP,  BEEPVOL_MIN,  BEEPVOL_MAX,  1,10},
};

static const t_volume data_volume_rhythm[] =
{
  { { " RHYTHM     [%] :",  " リズム音量 [％]：" }, VOL_RHYTHM,RHYTHMVOL_MIN,RHYTHMVOL_MAX,1,10},
  { { " ADPCM      [%] :",  " ADPCM 音量 [％]：" }, VOL_ADPCM, ADPCMVOL_MIN, ADPCMVOL_MAX, 1,10},
};

static const t_volume data_volume_fmgen[] =
{
  { { " FM & PSG   [%] :",  " FM/PSG音量 [％]：" }, VOL_FMGEN, FMGENVOL_MIN, FMGENVOL_MAX, 1,10},
  { { " BEEP sound [%] :",  " ＢＥＥＰ音 [％]：" }, VOL_BEEP,  BEEPVOL_MIN,  BEEPVOL_MAX,  1,10},
};

static const t_volume data_volume_sample[] =
{
  { { " SAMPLE snd [%] :",  " サンプル音 [％]：" }, VOL_SAMPLE,SAMPLEVOL_MIN,SAMPLEVOL_MAX,1,10},
};


enum {
  DATA_VOLUME_AUDIO_FMGEN,
  DATA_VOLUME_AUDIO_FREQ,
  DATA_VOLUME_AUDIO_SAMPLE,
};
static const t_menulabel data_volume_audio[] =
{
  { { " FM Generator     ",                            " FM音源ジェネレータ       ",                    } },
  { { " Sample-Frequency ([Hz], Range = 8000-48000) ", " サンプリング周波数 ([Hz],範囲＝8000〜48000) ", } },
  { { " Sample Data      ",                            " サンプル音の使用有無     ",                    } },
};

static const t_menudata data_volume_audio_fmgen[] =
{
  { { " MAME built-in ", " MAME 内蔵  ", }, FALSE  },
  { { " fmgen",          " fmgen",       }, TRUE   },
};
static const t_menudata data_volume_audio_freq_combo[] =
{
  { { "48000", "48000" }, 48000 },
  { { "44100", "44100" }, 44100 },
  { { "22050", "22050" }, 22050 },
  { { "11025", "11025" }, 11025 },
};
static const t_menudata data_volume_audio_sample[] =
{
  { { " Not Use       ", " 使用しない ", }, FALSE  },
  { { " Use",            " 使用する ",   }, TRUE   },
};


static const t_menulabel data_volume_audiodevice_stop[] =
{
  { { " (The sound device is stopping.)", " （サウンドデバイスは、停止中です）", } },
};







/*--------------------------------------------------------------
 *  「DIP-SW」
 *--------------------------------------------------------------*/
enum {
  DATA_DIPSW_B,
  DATA_DIPSW_R
};
static const t_menulabel data_dipsw[] =
{
  { { " Boot up ", " 初期設定 "    } },
  { { " RC232C ",  " RS232C 設定 " } },
};


typedef struct{
  char  *str[2];
  int   val;
  const t_menudata *p;
} t_dipsw;




static const t_menudata data_dipsw_b_term[] =
{
  { { "TERMINAL   ", "ターミナル " }, (0<<1) | 0 },
  { { "BASIC      ", "ＢＡＳＩＣ " }, (0<<1) | 1 },
};
static const t_menudata data_dipsw_b_ch80[] =
{
  { { "80ch / line", "８０字     " }, (1<<1) | 0 },
  { { "40ch / line", "４０字     " }, (1<<1) | 1 },
};
static const t_menudata data_dipsw_b_ln25[] =
{
  { { "25line/scrn", "２５行     " }, (2<<1) | 0 },
  { { "20line/scrn", "２０行     " }, (2<<1) | 1 },
};
static const t_menudata data_dipsw_b_boot[] =
{
  { { "DISK       ", "ディスク   " }, FALSE },
  { { "ROM        ", "ＲＯＭ     " }, TRUE  },
};

static const t_dipsw data_dipsw_b[] =
{
  { { "BOOT MODE           :", "立ち上げモード     ：" },  1, data_dipsw_b_term },
  { { "Chars per Line      :", "１行あたりの文字数 ：" },  2, data_dipsw_b_ch80 },
  { { "Lines per screen    :", "１画面あたりの行数 ：" },  3, data_dipsw_b_ln25 },
};
static const t_dipsw data_dipsw_b2[] =
{
  { { "Boot Up from        :", "システムの立ち上げ ：" }, -1, data_dipsw_b_boot },
};



static const t_menudata data_dipsw_r_baudrate[] =
{
  { {    "75",    "75" }, 0 },
  { {   "150",   "150" }, 1 },
  { {   "300",   "300" }, 2 },
  { {   "600",   "600" }, 3 },
  { {  "1200",  "1200" }, 4 },
  { {  "2400",  "2400" }, 5 },
  { {  "4800",  "4800" }, 6 },
  { {  "9600",  "9600" }, 7 },
  { { "19200", "19200" }, 8 },
};


static const t_menudata data_dipsw_r_hdpx[] =
{
  { { "HALF       ", "半二重     " }, (0<<1) | 0 },
  { { "FULL       ", "全二重     " }, (0<<1) | 1 },
};
static const t_menudata data_dipsw_r_xprm[] =
{
  { { "Enable     ", "有  効     " }, (1<<1) | 0 },
  { { "Disable    ", "無  効     " }, (1<<1) | 1 },
};
static const t_menudata data_dipsw_r_st2b[] =
{
  { { "2 bit      ", "２ bit     " }, (2<<1) | 0 },
  { { "1 bit      ", "１ bit     " }, (2<<1) | 1 },
};
static const t_menudata data_dipsw_r_dt8b[] =
{
  { { "8 bit      ", "８ bit     " }, (3<<1) | 0 },
  { { "7 bit      ", "７ bit     " }, (3<<1) | 1 },
};
static const t_menudata data_dipsw_r_sprm[] =
{
  { { "Enable     ", "有  効     " }, (4<<1) | 0 },
  { { "Disable    ", "無  効     " }, (4<<1) | 1 },
};
static const t_menudata data_dipsw_r_pdel[] =
{
  { { "Enable     ", "有  効     " }, (5<<1) | 0 },
  { { "Disable    ", "無  効     " }, (5<<1) | 1 },
};
static const t_menudata data_dipsw_r_enpty[] =
{
  { { "Yes        ", "有  り     " }, (6<<1) | 0 },
  { { "No         ", "無  し     " }, (6<<1) | 1 },
};
static const t_menudata data_dipsw_r_evpty[] =
{
  { { "Even       ", "偶  数     " }, (7<<1) | 0 },
  { { "Odd        ", "奇  数     " }, (7<<1) | 1 },
};

static const t_menulabel data_dipsw_r2[] =
{
  { { "Baud Rate (BPS)     :", "通信速度［ボー］   ：" } },
};
static const t_dipsw data_dipsw_r[] =
{
  { { "Duplex              :", "通  信  方  式     ：" }, 5 +8, data_dipsw_r_hdpx  },
  { { "X parameter         :", "Ｘパラメータ       ：" }, 4 +8, data_dipsw_r_xprm  },
  { { "Stop Bit            :", "ストップビット長   ：" }, 3 +8, data_dipsw_r_st2b  },
  { { "Data Bit            :", "データビット長     ：" }, 2 +8, data_dipsw_r_dt8b  },
  { { "S parameter         :", "Ｓパラメータ       ：" }, 4,    data_dipsw_r_sprm  },
  { { "DEL code            :", "ＤＥＬコード       ：" }, 5,    data_dipsw_r_pdel  },
  { { "Patiry Check        :", "パリティチェック   ：" }, 0 +8, data_dipsw_r_enpty },
  { { "Patiry              :", "パ  リ  ティ       ：" }, 1 +8, data_dipsw_r_evpty },
};


/*--------------------------------------------------------------
 *  「ディスク」 タブ
 *--------------------------------------------------------------*/
static const t_menulabel data_disk_image_drive[] =
{
  { { " <<< DRIVE [1:] >>> ", " <<< DRIVE [1:] >>> " } },
  { { " <<< DRIVE [2:] >>> ", " <<< DRIVE [2:] >>> " } },
};
static const t_menulabel data_disk_info_drive[] =
{
  { { "   DRIVE [1:]   ", "   DRIVE [1:]   " } },
  { { "   DRIVE [2:]   ", "   DRIVE [2:]   " } },
};

enum {
  DATA_DISK_IMAGE_EMPTY,
  DATA_DISK_IMAGE_BLANK
};
static const t_menulabel data_disk_image[] =
{
  { { "< EMPTY >                 ", "< なし >                  " } },
  { { "  Create Blank  ",           " ブランクの作成 "           } },
};

enum {
  DATA_DISK_INFO_STAT,      /* "STATUS     READY" で16文字 */
  DATA_DISK_INFO_STAT_READY,
  DATA_DISK_INFO_STAT_BUSY,
  DATA_DISK_INFO_ATTR,      /* "ATTR  READ/WRITE" で16文字 */
  DATA_DISK_INFO_ATTR_RW,
  DATA_DISK_INFO_ATTR_RO,
  DATA_DISK_INFO_NR,        /* "IMAGE  xxxxxxxxx" で16文字、 x は 9文字 */
  DATA_DISK_INFO_NR_BROKEN,
  DATA_DISK_INFO_NR_OVER
};
static const t_menulabel data_disk_info[] =
{
  { { "STATUS     ",         "状態       "      } },
  { {            "READY",               "READY" } },
  { {            "BUSY ",               "BUSY " } },
  { { "ATTR  ",              "属性  "           } },
  { {       "  Writable",          "    書込可" } },
  { {       " Read Only",          "  読込専用" } },
  { { "IMAGE  ",             "総数   "          } },
  { {          "+BROKEN",              " +破損" } },
  { {            " OVER",              " 以上 " } },
};



enum {
  IMG_OPEN,
  IMG_CLOSE,
  IMG_BOTH,
  IMG_COPY,
  IMG_ATTR
};
static const t_menulabel data_disk_button_drv1[] =
{
  { { " DRIVE [1:]           OPEN ", " DRIVE [1:]           開く " } },
  { { " DRIVE [1:]          CLOSE ", " DRIVE [1:]         閉じる " } },
  { { " DRIVE [1:][2:] BOTH  OPEN ", " DRIVE [1:][2:] 両方に開く " } },
  { { " DRIVE [1:] <= [2:]   OPEN ", " DRIVE [1:] ← [2:]   開く " } },
  { { " CHANGE ATTRIBUTE of IMAGE ", " イメージの 属性を変更する " } },
};
static const t_menulabel data_disk_button_drv2[] =
{
  { { " DRIVE [2:]           OPEN ", " DRIVE [2:]           開く " } },
  { { " DRIVE [2:]          CLOSE ", " DRIVE [2:]         閉じる " } },
  { { " DRIVE [1:][2:] BOTH  OPEN ", " DRIVE [1:][2:] 両方に開く " } },
  { { " DRIVE [1:] => [2:]   OPEN ", " DRIVE [1:] → [2:]   開く " } },
  { { " CHANGE ATTRIBUTE of IMAGE ", " イメージの 属性を変更する " } },
};
static const t_menulabel data_disk_button_drv1swap[] =
{
  { { " OPEN           DRIVE [1:] ", " 開く           DRIVE [1:] " } },
  { { " CLOSE          DRIVE [1:] ", " 閉じる         DRIVE [1:] " } },
  { { " OPEN  BOTH DRIVE [1:][2:] ", " 両方に開く DRIVE [2:][1:] " } },
  { { " OPEN   DRIVE [1:] => [2:] ", " 開く   DRIVE [2:] → [1:] " } },
  { { " CHANGE ATTRIBUTE of IMAGE ", " イメージの 属性を変更する " } },
};
static const t_menulabel data_disk_button_drv2swap[] =
{
  { { " OPEN           DRIVE [2:] ", " 開く           DRIVE [2:] " } },
  { { " CLOSE          DRIVE [2:] ", " 閉じる         DRIVE [2:] " } },
  { { " OPEN  BOTH DRIVE [1:][2:] ", " 両方に開く DRIVE [2:][1:] " } },
  { { " OPEN   DRIVE [1:] <= [2:] ", " 開く   DRIVE [2:] ← [1:] " } },
  { { " CHANGE ATTRIBUTE of IMAGE ", " イメージの 属性を変更する " } },
};



enum {
  DATA_DISK_OPEN_OPEN,
  DATA_DISK_OPEN_BOTH
};
static const t_menulabel data_disk_open_drv1[] =
{
  { { " OPEN FILE in DRIVE [1:] ",        " DRIVE [1:] にイメージファイルをセットします "         } },
  { { " OPEN FILE in DRIVE [1:] & [2:] ", " DRIVE [1:] と [2:] にイメージファイルをセットします " } },
};
static const t_menulabel data_disk_open_drv2[] =
{
  { { " OPEN FILE in DRIVE [2:] ",        " DRIVE [2:] にイメージファイルをセットします "         } },
  { { " OPEN FILE in DRIVE [1:] & [2:] ", " DRIVE [1:] と [2:] にイメージファイルをセットします " } },
};



enum {
  DATA_DISK_ATTR_TITLE1,
  DATA_DISK_ATTR_TITLE1_,
  DATA_DISK_ATTR_TITLE2,
  DATA_DISK_ATTR_RENAME,
  DATA_DISK_ATTR_PROTECT,
  DATA_DISK_ATTR_FORMAT,
  DATA_DISK_ATTR_BLANK,
  DATA_DISK_ATTR_CANCEL
};
static const t_menulabel data_disk_attr[] =
{
  { { " Change Attribute of the image at drive 1: ", " ドライブ 1: のイメージ "    } },
  { { " Change Attribute of the image at drive 2: ", " ドライブ 2: のイメージ "    } },
  { { " ",                                           " の 属性変更などを行います " } },
  { { "RENAME",                                      "名前変更"                    } },
  { { "PROTECT",                                     "属性変更"                    } },
  { { "(UN)FORMAT",                                  "(アン)フォーマット"          } },
  { { "APPEND BLANK",                                "ブランクの追加"              } },
  { { "CANCEL",                                      " 取消 "                      } },
};

enum {
  DATA_DISK_ATTR_RENAME_TITLE1,
  DATA_DISK_ATTR_RENAME_TITLE1_,
  DATA_DISK_ATTR_RENAME_TITLE2,
  DATA_DISK_ATTR_RENAME_OK,
  DATA_DISK_ATTR_RENAME_CANCEL
};
static const t_menulabel data_disk_attr_rename[] =
{
  { { " Rename the image at drive 1: ", " ドライブ 1: のイメージ " } },
  { { " Rename the image at drive 2: ", " ドライブ 2: のイメージ " } },
  { { " ",                              " の 名前を変更します "    } },
  { { "  OK  ",                         " 変更 "                   } },
  { { "CANCEL",                         " 取消 "                   } },
};

enum {
  DATA_DISK_ATTR_PROTECT_TITLE1,
  DATA_DISK_ATTR_PROTECT_TITLE1_,
  DATA_DISK_ATTR_PROTECT_TITLE2,
  DATA_DISK_ATTR_PROTECT_SET,
  DATA_DISK_ATTR_PROTECT_UNSET,
  DATA_DISK_ATTR_PROTECT_CANCEL
};
static const t_menulabel data_disk_attr_protect[] =
{
  { { " (Un)Peotect the image at drive 1: ", " ドライブ 1: のイメージ "        } },
  { { " (Un)Peotect the image at drive 2: ", " ドライブ 2: のイメージ "        } },
  { { " ",                                   " の プロテクト状態を変更します " } },
  { { " SET PROTECT ",                       " プロテクト状態にする "          } },
  { { " UNSET PROTECT ",                     " プロテクトを解除する "          } },
  { { " CANCEL ",                            " 取消 "                          } },
};

enum {
  DATA_DISK_ATTR_FORMAT_TITLE1,
  DATA_DISK_ATTR_FORMAT_TITLE1_,
  DATA_DISK_ATTR_FORMAT_TITLE2,
  DATA_DISK_ATTR_FORMAT_WARNING,
  DATA_DISK_ATTR_FORMAT_DO,
  DATA_DISK_ATTR_FORMAT_NOT,
  DATA_DISK_ATTR_FORMAT_CANCEL
};
static const t_menulabel data_disk_attr_format[] =
{
  { { " (Un)Format the image at drive 1: ",       " ドライブ 1: のイメージ "                  } },
  { { " (Un)Format the image at drive 2: ",       " ドライブ 2: のイメージ "                  } },
  { { " ",                                        " を （アン）フォーマットします "           } },
  { { "[WARNING : data in the image will lost!]", "[注意:イメージ内のデータは消去されます！]" } },
  { { " FORMAT ",                                 " フォーマットする "                        } },
  { { " UNFORMAT ",                               " アンフォーマットする "                    } },
  { { " CANCEL ",                                 " 取消 "                                    } },
};

enum {
  DATA_DISK_ATTR_BLANK_TITLE1,
  DATA_DISK_ATTR_BLANK_TITLE1_,
  DATA_DISK_ATTR_BLANK_TITLE2,
  DATA_DISK_ATTR_BLANK_OK,
  DATA_DISK_ATTR_BLANK_CANCEL,
  DATA_DISK_ATTR_BLANK_END
};
static const t_menulabel data_disk_attr_blank[] =
{
  { { " Append Blank image at drive 1: ", " ドライブ 1: のファイルに "     } },
  { { " Append Blank image at drive 2: ", " ドライブ 2: のファイルに "     } },
  { { " ",                                " ブランクイメージを追加します " } },
  { { " APPEND ",                         " ブランクイメージの追加 "       } },
  { { " CANCEL ",                         " 取消 "                         } },
};



enum {
  DATA_DISK_BLANK_FSEL,
  DATA_DISK_BLANK_WARN_0,
  DATA_DISK_BLANK_WARN_1,
  DATA_DISK_BLANK_WARN_APPEND,
  DATA_DISK_BLANK_WARN_CANCEL
};
static const t_menulabel data_disk_blank[] =
{
  { { " Create a new file as blank image file.", " ブランクイメージファイルを新規作成します " } },
  { { " This File Already Exist. ",              " 指定したファイルはすでに存在します。 "     } },
  { { " Append a blank image ? ",                " ブランクイメージを追加しますか？ "         } },
  { { " APPEND ",                                " 追加する "                                 } },
  { { " CANCEL ",                                " 取消 "                                     } },
};


enum {
  DATA_DISK_FNAME,
  DATA_DISK_FNAME_TITLE,
  DATA_DISK_FNAME_LINE,
  DATA_DISK_FNAME_SAME,
  DATA_DISK_FNAME_SEP,
  DATA_DISK_FNAME_RO,
  DATA_DISK_FNAME_RO_1,
  DATA_DISK_FNAME_RO_2,
  DATA_DISK_FNAME_RO_X,
  DATA_DISK_FNAME_RO_Y,
  DATA_DISK_FNAME_OK
};
static const t_menulabel data_disk_fname[] =
{
  { { " Show Filename  ",                                 " ファイル名確認 "                                 } },
  { { " Disk Image Filename ",                            " ディスクイメージファイル名確認 "                 } },
  { { "------------------------------------------------", "------------------------------------------------" } },
  { { " Same file Drive 1: as Drive 2 ",                  " ドライブ 1: と 2: は同じファイルです "           } },
  { { " ",                                                " "                                                } },
  { { "    * The disk image file(s) is read-only.      ", "●読込専用のディスクイメージファイルがあります。" } },
  { { "      All images in this file are regarded      ", "  このファイルに含まれるすべてのイメージは、    " } },
  { { "      as WRITE-PROTECTED.                       ", "  ライトプロテクト状態と同様に扱われます。      " } },
  { { "      Writing to the image is ignored, but      ", "  このファイルへの書き込みは全て無視されますが、" } },
  { { "      not error depending on situation.         ", "  エラーとは認識されない場合があります。        " } },
  { { "  OK  ",                                           " 確認 "                                           } },
};



enum {
  DATA_DISK_DISPSWAP,
  DATA_DISK_DISPSWAP_INFO_1,
  DATA_DISK_DISPSWAP_INFO_2,
  DATA_DISK_DISPSWAP_OK
};
static const t_menulabel data_disk_dispswap[] =
{
  { { "Swap Drv-Disp",                                   "表示 左右入換"                                     } },
  { { "Swap Drive-Display placement",                    "DRIVE [1:] と [2:] の表示位置を、左右入れ換えます" } },
  { { "This setting effects next time. ",                "この設定は次回のメニューモードより有効となります " } },
  { { "  OK  ",                                          " 確認 "                                            } },
};



enum {
  DATA_DISK_DISPSTATUS,
  DATA_DISK_DISPSTATUS_INFO,
  DATA_DISK_DISPSTATUS_OK
};
static const t_menulabel data_disk_dispstatus[] =
{
  { { "Show in status",                                  "表示 ステータス"                                   } },
  { { "Display image name in status area. ",             "ステータス部に、イメージ名を表示します"            } },
  { { "  OK  ",                                          " 確認 "                                            } },
};



/*--------------------------------------------------------------
 *  「その他」 タブ
 *--------------------------------------------------------------*/

enum {
  DATA_MISC_SUSPEND,
  DATA_MISC_SNAPSHOT,
  DATA_MISC_WAVEOUT,
};
static const t_menulabel data_misc[] =
{
  { { "State Save     ",    "ステートセーブ " } },
  { { "Screen Shot    ",    "画面保存       " } },
  { { "Sound Record   ",    "サウンド保存   " } },
};





enum {
  DATA_MISC_SUSPEND_CHANGE,
  DATA_MISC_SUSPEND_SAVE,
  DATA_MISC_SUSPEND_LOAD,
  DATA_MISC_SUSPEND_NUMBER,
  DATA_MISC_SUSPEND_FSEL
};
static const t_menulabel data_misc_suspend[] =
{
  { { " Change ",                           " ファイル変更 "                       } },
  { { " SAVE ",                             " セーブ "                             } },
  { { " LOAD ",                             " ロード "                             } },
  { { "                         number : ", "                 連番： "             } },
  { { " Input (Select) a state filename. ", " ステートファイル名を入力して下さい " } },
};


static const t_menudata data_misc_suspend_num[] =
{
  { { "(none)",  "(なし)" }, 0   },
  { { "0",       "0"      }, '0' },
  { { "1",       "1"      }, '1' },
  { { "2",       "2"      }, '2' },
  { { "3",       "3"      }, '3' },
  { { "4",       "4"      }, '4' },
  { { "5",       "5"      }, '5' },
  { { "6",       "6"      }, '6' },
  { { "7",       "7"      }, '7' },
  { { "8",       "8"      }, '8' },
  { { "9",       "9"      }, '9' },
};




enum {
  DATA_MISC_SUSPEND_OK,
  DATA_MISC_RESUME_OK,
  DATA_MISC_SUSPEND_LINE,
  DATA_MISC_SUSPEND_INFO,
  DATA_MISC_SUSPEND_AGREE,
  DATA_MISC_SUSPEND_ERR,
  DATA_MISC_RESUME_ERR,
  DATA_MISC_SUSPEND_REALLY,
  DATA_MISC_SUSPEND_OVERWRITE,
  DATA_MISC_SUSPEND_CANCEL,
  DATA_MISC_RESUME_CANTOPEN
};
static const t_menulabel data_misc_suspend_err[] =
{
  { { "State save Finished.",                           "状態を保存しました。"                               } },
  { { "State load Finished.",                           "状態を復元しました。"                               } },
  { { "----------------------------------------------", "----------------------------------------------"     } },
  { { "      ( Following image files are set )       ", " ( 以下のイメージファイルが設定されています ) "     } },
  { { " OK ",                                           "確認"                                               } },
  { { "Error / State save failed.",                     "エラー／状態は保存されませんでした。"               } },
  { { "Error / State load failed. Reset done",          "エラー／状態の復元に失敗しました。リセットします。" } },
  { { "State-file already exist, Over write ?",         "ファイルはすでに存在します。上書きしますか？"       } },
  { { " Over Write ",                                   "上書き"                                             } },
  { { " Cancel ",                                       "取消"                                               } },
  { { "State-file not exist or broken.",                "ステートファイルが無いか、壊れています。"           } },
};



enum {
  DATA_MISC_SNAPSHOT_FORMAT,
  DATA_MISC_SNAPSHOT_CHANGE,
  DATA_MISC_SNAPSHOT_PADDING,
  DATA_MISC_SNAPSHOT_BUTTON,
  DATA_MISC_SNAPSHOT_FSEL,
  DATA_MISC_SNAPSHOT_CMD
};
static const t_menulabel data_misc_snapshot[] =
{
  { { " Format   ",                                        " 画像形式   "                                   } },
  { { " Change ",                                          " ベース名変更 "                                 } },
  { { "                    ",                              "            "                                   } },
  { { " SAVE ",                                            " 保存 "                                         } },
  { { " Input (Select) a screen-snapshot base-filename. ", " 保存するファイル (ベース名) を入力して下さい " } },
  { { "Exec following Command",                            "次のコマンドを実行する"                         } },
};

static const t_menudata data_misc_snapshot_format[] =
{
  { { " BMP ", " BMP " }, 0 },
  { { " PPM ", " PPM " }, 1 },
  { { " RAW ", " RAW " }, 2 },
};



enum {
  DATA_MISC_WAVEOUT_CHANGE,
  DATA_MISC_WAVEOUT_START,
  DATA_MISC_WAVEOUT_STOP,
  DATA_MISC_WAVEOUT_PADDING,
  DATA_MISC_WAVEOUT_FSEL
};
static const t_menulabel data_misc_waveout[] =
{
  { { " Change ",                                       " ベース名変更 "                                 } },
  { { " START ",                                        " 開始 "                                         } },
  { { " STOP ",                                         " 停止 "                                         } },
  { { "                                            ",   "                                       "        } },
  { { " Input (Select) a sound-record base-filename. ", " 出力するファイル (ベース名) を入力して下さい " } },
};



static const t_menulabel data_misc_sync[] =
{
  { { "synchronize filename with disk-image filename", "各ファイル名をディスクイメージのファイル名に合わせる", } },
};





/*--------------------------------------------------------------
 *  「キー」 タブ
 *--------------------------------------------------------------*/
enum {
  DATA_KEY_FKEY,
  DATA_KEY_CURSOR,
  DATA_KEY_CURSOR_SPACING,
  DATA_KEY_SKEY,
  DATA_KEY_SKEY2
};
static const t_menulabel data_key[] =
{
  { { " Function key Config ",             " ファンクションキー設定 ",         } },
  { { " Curosr Key Config ",               " カーソルキー設定 ",               } }, 
  { { "                                 ", "                                ", } },
  { { " Software ",                        " ソフトウェア ",                   } },
  { { "   Keyboard  ",                     "  キーボード  ",                   } },
};



enum {
  DATA_KEY_CFG_TENKEY,
  DATA_KEY_CFG_NUMLOCK
};
static const t_menudata data_key_cfg[] =
{
  { { "Set numeric key to TEN-key (-tenkey) ",  "数字キーをテンキーに割り当てる   (-tenkey)  ", }, DATA_KEY_CFG_TENKEY,  },
  { { "software NUM-Lock ON       (-numlock)",  "ソフトウェア NUM Lock をオンする (-numlock) ", }, DATA_KEY_CFG_NUMLOCK, },
};



static const t_menudata data_key_fkey[] =
{
  { { "   f6  key ",  "   f6  キー ", },  6 },
  { { "   f7  key ",  "   f7  キー ", },  7 },
  { { "   f8  key ",  "   f8  キー ", },  8 },
  { { "   f9  key ",  "   f9  キー ", },  9 },
  { { "   f10 key ",  "   f10 キー ", }, 10 },
};
static const t_menudata data_key_fkey_fn[] =
{
  { { "----------- : function or another key",  "----------- : ファンクションまたは任意キー", },  FN_FUNC,        },
  { { "FRATE-UP    : Frame Rate  Up",           "FRATE-UP    : フレームレート 上げる ",       },  FN_FRATE_UP,    },
  { { "FRATE-DOWN  : Frame Rate  Down",         "FRATE-DOWN  : フレームレート 下げる ",       },  FN_FRATE_DOWN,  },
  { { "VOLUME-UP   : Volume  Up",               "VOLUME-UP   : 音量 上げる",                  },  FN_VOLUME_UP,   },
  { { "VOLUME-DOWN : Volume  Down",             "VOLUME-DOWN : 音量 下げる",                  },  FN_VOLUME_DOWN, },
  { { "PAUSE       : Pause",                    "PAUSE       : 一時停止",                     },  FN_PAUSE,       },
  { { "RESIZE      : Resize",                   "RESIZE      : 画面サイズ変更",               },  FN_RESIZE,      },
  { { "NOWAIT      : No-Wait",                  "NOWAIT      : ウエイトなし",                 },  FN_NOWAIT,      },
  { { "SPEED-UP    : Speed Up",                 "SPEED-UP    : 速度 上げる ",                 },  FN_SPEED_UP,    },
  { { "SPEED-DOWN  : Speed Down",               "SPEED-DOWN  : 速度 下げる ",                 },  FN_SPEED_DOWN,  },
  { { "FULLSCREEN  : Full Screen Mode",         "FULLSCREEN  : フルスクリーン切替",           },  FN_FULLSCREEN,  },
  { { "SNAPSHOT    : Save Screen Snapshot",     "SNAPSHOT    : スクリーンスナップショット",   },  FN_SNAPSHOT,    },
  { { "IMAGE-NEXT1 : Drive 1:  Next Image",     "IMAGE-NEXT1 : Drive 1:  次イメージ",         },  FN_IMAGE_NEXT1, },
  { { "IMAGE-PREV1 : Drive 1:  Prev Image",     "IMAGE-PREV1 : Drive 1:  前イメージ",         },  FN_IMAGE_PREV1, },
  { { "IMAGE-NEXT2 : Drive 2:  Next Image",     "IMAGE-NEXT2 : Drive 2:  次イメージ",         },  FN_IMAGE_NEXT2, },
  { { "IMAGE-PREV2 : Drive 2:  Prev Image",     "IMAGE-PREV2 : Drive 2:  前イメージ",         },  FN_IMAGE_PREV2, },
  { { "NUMLOCK     : Software NUM Lock",        "NUMLOCK     : ソフトウェア NUM Lock",        },  FN_NUMLOCK,     },
  { { "RESET       : Reset switch",             "RESET       : リセット スイッチ",            },  FN_RESET,       },
  { { "KANA        : KANA key",                 "KANA        : カナ キー",                    },  FN_KANA,        },
  { { "ROMAJI      : KANA(ROMAJI) Key",         "ROMAJI      : カナ(ローマ字入力) キー",      },  FN_ROMAJI,      },
  { { "CAPS        : CAPS Key",                 "CAPS        : CAPS キー",                    },  FN_CAPS,        },
  { { "MAX-SPEED   : Max Speed",                "MAX-SPEED   : 速度最大設定値",               },  FN_MAX_SPEED,   },
  { { "MAX-CLOCK   : Max CPU-Clock",            "MAX-CLOCK   : CPUクロック最大設定値",        },  FN_MAX_CLOCK,   },
  { { "MAX-BOOST   : Max Boost",                "MAX-BOOST   : ブースト最大設定値",           },  FN_MAX_BOOST,   },
  { { "STATUS      : Display status",           "STATUS      : ステータス表示のオン／オフ",   },  FN_STATUS,      },
  { { "MENU        : Go Menu-Mode",             "MENU        : メニュー",                     },  FN_MENU,        },
};



static const t_menudata data_key_fkey2[] =
{
  { { "   ",  "   ", },  6 },
  { { "   ",  "   ", },  7 },
  { { "   ",  "   ", },  8 },
  { { "   ",  "   ", },  9 },
  { { "   ",  "   ", }, 10 },
};



enum {
  DATA_SKEY_BUTTON_SETUP,
  DATA_SKEY_BUTTON_OFF,
  DATA_SKEY_BUTTON_QUIT
};
static const t_menulabel data_skey_set[] =
{
  { { "Setting",                " 設定 ",               } },
  { { "All key release & QUIT", " 全てオフにして戻る ", } },
  { { " QUIT ",                 " 戻る ",               } },
};



static const t_menudata data_key_cursor_mode[] =
{
  { { " Default(CursorKey)",  " 標準(カーソルキー)",    },  0, },
  { { " Assign to 2,4,6,8",   " 2,4,6,8 を割り当て",    },  1, },
  { { " Assign arbitrarily ", " 任意のキーを割り当て ", },  2, },
};
static const t_menudata data_key_cursor[] =
{
  { { "             ",          "             ",     },   0, },
  { { "                \036",   "               ↑", },  -1, },
  { { " ",                      " ",                 },   2, },
  { { "\035           \034 ",   "←        → ",     },   3, },
  { { "                \037",   "               ↓", },  -1, },
  { { "             ",          "             ",     },   1, },
};



/*--------------------------------------------------------------
 *  「マウス」 タブ
 *--------------------------------------------------------------*/
enum {
  DATA_MOUSE_MODE,
  DATA_MOUSE_SERIAL,

  DATA_MOUSE_SYSTEM,

  DATA_MOUSE_DEVICE_MOUSE,
  DATA_MOUSE_DEVICE_JOY,
  DATA_MOUSE_DEVICE_JOY2,
  DATA_MOUSE_DEVICE_ABOUT,

  DATA_MOUSE_DEVICE_NUM,

  DATA_MOUSE_CONNECTING,
  DATA_MOUSE_SWAP_MOUSE,
  DATA_MOUSE_SWAP_JOY,
  DATA_MOUSE_SWAP_JOY2
};
static const t_menulabel data_mouse[] =
{
  { { " Mouse / Joystick setting ", " マウス／ジョイスティック接続 "  } },
  { { " Serial-mouse ",             " シリアルマウス "                } },

  { { " [ System Setup  (Some settings are disabled in some systems.)]               ",
      " 【システム設定  (設定の一部は、システムによっては無効です)】                 ", } },

  { { " Mouse ",                         " マウス入力 ",                         } },
  { { " Joystick ",                      " ジョイスティック入力 ",               } },
  { { " Joystick(2) ",                   " ジョイスティック(2)入力 "             } },
  { { " About ",                         " ※ "                                  } },

  { { " %d Joystick(s) is found.",       "  %d 個のジョイスティックが使用できます。" } },

  { { "  Connecting mouse-port   ",         "  マウスポートに接続中    "         } },
  { { "Swap mouse buttons",                 "左右ボタンを入れ替える"             } },
  { { "Swap joystick buttons (-joy_swap)",  "ＡＢボタンを入れ替える (-joyswap)"  } },
  { { "Swap joystick buttons",              "ＡＢボタンを入れ替える"             } },
};



static const t_menudata data_mouse_mode[] =
{
  { { "Not Connect               (-nomouse) ", "なにも接続しない                     (-nomouse) " }, MOUSE_NONE     },
  { { "Connect Mouse             (-mouse)   ", "マウスを接続                         (-mouse)   " }, MOUSE_MOUSE    },
  { { "Connect Mouse as joystick (-joymouse)", "マウスをジョイスティックモードで接続 (-joymouse)" }, MOUSE_JOYMOUSE },
  { { "Connect joystick          (-joystick)", "ジョイスティックを接続               (-joystick)" }, MOUSE_JOYSTICK },
};



static const t_menulabel data_mouse_serial[] =
{
  { { "Connect (-serialmouse)", "接続 (-serialmouse)" } },
};



static const t_menudata data_mouse_mouse_key_mode[] =
{
  { { " Not Assigned ",          " キー割り当てなし"       },  0 },
  { { " Assign to 2,4,6,8,x,z ", " 2,4,6,8,x,zを割り当て " },  1 },
  { { " Assign arbitrarily    ", " 任意のキーを割り当て"   },  2 },
};
static const t_menudata data_mouse_mouse[] =
{
  { { "             ",          "             ",     },   0, },
  { { "                \036",   "               ↑", },  -1, },
  { { " ",                      " ",                 },   2, },
  { { "\035           \034 ",   "←        → ",     },   3, },
  { { "                \037",   "               ↓", },  -1, },
  { { "             ",          "             ",     },   1, },
  { { "",                       "",                  },  -1, },
  { { " L ",                    " 左 "               },   4, },
  { { "",                       "",                  },  -1, },
  { { " R ",                    " 右 "               },   5, },
};



static const t_menudata data_mouse_joy_key_mode[] =
{
  { { " Not Assigned ",          " キー割り当てなし"       },  0 },
  { { " Assign to 2,4,6,8,x,z ", " 2,4,6,8,x,zを割り当て " },  1 },
  { { " Assign arbitrarily    ", " 任意のキーを割り当て"   },  2 },
};
static const t_menudata data_mouse_joy[] =
{
  { { "             ",          "             ",     },   0, },
  { { "                \036",   "               ↑", },  -1, },
  { { " ",                      " ",                 },   2, },
  { { "\035           \034 ",   "←        → ",     },   3, },
  { { "                \037",   "               ↓", },  -1, },
  { { "             ",          "             ",     },   1, },
  { { " A ",                    " Ａ "               },   4, },
  { { " B ",                    " Ｂ "               },   5, },
  { { " C ",                    " Ｃ "               },   6, },
  { { " D ",                    " Ｄ "               },   7, },
  { { " E ",                    " Ｅ "               },   8, },
  { { " F ",                    " Ｆ "               },   9, },
  { { " G ",                    " Ｇ "               },  10, },
  { { " H ",                    " Ｈ "               },  11, },
};
static const t_menudata data_mouse_joy2_key_mode[] =
{
  { { " Not Assigned ",          " キー割り当てなし"       },  0 },
  { { " Assign to 2,4,6,8,x,z ", " 2,4,6,8,x,zを割り当て " },  1 },
  { { " Assign arbitrarily    ", " 任意のキーを割り当て"   },  2 },
};
static const t_menudata data_mouse_joy2[] =
{
  { { "             ",          "             ",     },   0, },
  { { "                \036",   "               ↑", },  -1, },
  { { " ",                      " ",                 },   2, },
  { { "\035           \034 ",   "←        → ",     },   3, },
  { { "                \037",   "               ↓", },  -1, },
  { { "             ",          "             ",     },   1, },
  { { " A ",                    " Ａ "               },   4, },
  { { " B ",                    " Ｂ "               },   5, },
  { { " C ",                    " Ｃ "               },   6, },
  { { " D ",                    " Ｄ "               },   7, },
  { { " E ",                    " Ｅ "               },   8, },
  { { " F ",                    " Ｆ "               },   9, },
  { { " G ",                    " Ｇ "               },  10, },
  { { " H ",                    " Ｈ "               },  11, },
};



static const t_volume data_mouse_sensitivity[] =
{
  { { " Sensitivity [%] :",  " マウス感度 [％]：" }, -1, 10, 200,  1, 10},
};



static const t_menulabel data_mouse_misc_msg[] =
{
  { { " Mouse Cursor : ", " マウスカーソルを " } },
};
static const t_menudata data_mouse_misc[] =
{
  { { "Always show the mouse cursor            (-show_mouse) ", "常に表示する             (-show_mouse) " }, SHOW_MOUSE },
  { { "Always Hide the mouse cursor            (-hide_mouse) ", "常に隠す                 (-hide_mouse) " }, HIDE_MOUSE },
  { { "Auto-hide the mouse cutsor              (-auto_mouse) ", "自動的に隠す             (-auto_mouse) " }, AUTO_MOUSE },
  { { "Confine the mouse cursor on the screend (-grab_mouse) ", "画面に閉じ込める（隠す） (-grab_mouse) " }, -1         },
  { { "Confine the mouse cursor when mouse clicked           ", "クリックで閉じ込める                   " }, -2         },
};



static const t_menudata data_mouse_debug_hide[] =
{
  { { " SHOW ", " 表示 ", }, SHOW_MOUSE, },
  { { " HIDE ", " 隠す ", }, HIDE_MOUSE, },
  { { " AUTO ", " 自動 ", }, AUTO_MOUSE, },
};
static const t_menudata data_mouse_debug_grab[] =
{
  { { " UNGRAB ", " 離す ", }, UNGRAB_MOUSE, },
  { { " GRAB   ", " 掴む ", }, GRAB_MOUSE,   },
  { { " AUTO   ", " 自動 ", }, AUTO_MOUSE,   },
};


/*--------------------------------------------------------------
 *  「テープ」 タブ
 *--------------------------------------------------------------*/
enum {
  DATA_TAPE_IMAGE,
  DATA_TAPE_INTR
};
static const t_menulabel data_tape[] =
{
  { { " Tape image ",       " テープイメージ "         } },
  { { " Tape Load Timing ", " テープロードの処理方法 " } },
};



enum {
  DATA_TAPE_FOR,
  DATA_TAPE_CHANGE,
  DATA_TAPE_EJECT,
  DATA_TAPE_FSEL,
  DATA_TAPE_REWIND,
  DATA_TAPE_WARN_0,
  DATA_TAPE_WARN_1,
  DATA_TAPE_WARN_APPEND,
  DATA_TAPE_WARN_CANCEL
};
static const t_menulabel data_tape_load[] =
{
  { { " for Load :",                                           " ロード用："                                      } },
  { { " Change File ",                                         " ファイル変更 "                                   } },
  { { " Eject  ",                                              " 取出し "                                         } },
  { { " Input (Select) a tape-load-image filename. (CMT/T88)", " ロード用テープイメージ(CMT/T88)を入力して下さい" } },
  { { " Rewind ",                                              " 巻戻し "                                         } },
};
static const t_menulabel data_tape_save[] =
{
  { { " for Save :",                                           " セーブ用："                                      } },
  { { " Change File ",                                         " ファイル変更 "                                   } },
  { { " Eject  ",                                              " 取出し "                                         } },
  { { " Input (Select) a tape-save-image filename. (CMT)",     " セーブ用テープイメージ(CMT)を入力して下さい"     } },
  { { NULL,                                                    NULL,                                              } },
  { { " This File Already Exist. ",                            " 指定したファイルはすでに存在します。 "           } },
  { { " Append a tape image ? ",                               " テープイメージを追記していきますか？ "           } },
  { { " OK ",                                                  " 追記する "                                       } },
  { { " CANCEL ",                                              " 取消 "                                           } },
};


static const t_menudata data_tape_intr[] =
{
  { { " Use Interrupt     (Choose in N88-BASIC mode) ",                 " 割り込みを使う     (N88-BASIC では、必ずこちらを選択してください) "   }, TRUE  },
  { { " Not Use Interrupt (Choose in N-BASIC mode for LOAD speed-up) ", " 割り込みを使わない (N-BASIC は、こちらでも可。ロードが速くなります) " }, FALSE },
};


/*--------------------------------------------------------------
 * ファイル操作エラーダイアログ
 *--------------------------------------------------------------*/

enum {
  ERR_NO,
  ERR_CANT_OPEN,
  ERR_READ_ONLY,
  ERR_MAYBE_BROKEN,
  ERR_SEEK,
  ERR_WRITE,
  ERR_OVERFLOW,
  ERR_UNEXPECTED
};
static const t_menulabel data_err_drive[] =
{
  { { " OK ",                                                     " 確認 "                                                       } },
  { { "File in DRIVE %d: / can't open the file, or bad format.",  "ドライブ %d:／ファイルが開けないか、ファイル形式が違います。" } },
  { { "File in DRIVE %d: / can't write the file.",                "ドライブ %d:／このファイルには書き込みができません。"         } },
  { { "File in DRIVE %d: / maybe broken.",                        "ドライブ %d:／ファイルが(多分)壊れています。"                 } },
  { { "File in DRIVE %d: / SEEK Error.",                          "ドライブ %d:／シークエラーが発生しました。"                   } },
  { { "File in DRIVE %d: / WRITE Error.",                         "ドライブ %d:／書き込みエラーが発生しました。"                 } },
  { { "File in DRIVE %d: / strings too long.",                    "ドライブ %d:／入力文字列が長過ぎます。"                       } },
  { { "File in DRIVE %d: / UNEXPECTED Error.",                    "ドライブ %d:／予期せぬエラーが発生しました。"                 } },   
};
static const t_menulabel data_err_file[] =
{
  { { " OK ",                           " 確認 "                                         } },
  { { "Error / can't open the file.",   "エラー／ファイルが開けません。"                 } },
  { { "Error / can't write the file.",  "エラー／このファイルには書き込みができません。" } },
  { { "Error / maybe broken.",          "エラー／ファイルが(多分)壊れています。"         } },
  { { "Error / SEEK Error.",            "エラー／シークエラーが発生しました。"           } },
  { { "Error / WRITE Error.",           "エラー／書き込みエラーが発生しました。"         } },
  { { "Error / strings too long.",      "エラー／入力文字列が長過ぎます。"               } },
  { { "Error / UNEXPECTED Error.",      "エラー／予期せぬエラーが発生しました。"         } },
};


/*--------------------------------------------------------------
 *
 *--------------------------------------------------------------*/

static const char *help_jp[] =
{
  "  ディスクイメージを使用するアプリケーションをエミュレートする場合、",
  "『SUB-CPU駆動』と『FDCウェイト』の設定が適切でないと、正常に動作",
  "しないことがあります。",
  "",
  "『SUB-CPU駆動』と『FDCウェイト』の設定の組合せは、以下のとおりです。",
  "",
  "           設定 ｜ SUB-CPU駆動  ｜ FDCウェイト｜        ",
  "          −−−＋−−−−−−−＋−−−−−−＋−−−−",
  "            (1) ｜ 0  (-cpu 0)  ｜    なし    ｜  高速  ",
  "            (2) ｜ 1  (-cpu 1)  ｜    なし    ｜   ↑   ",
  "            (3) ｜ 1  (-cpu 1)  ｜    あり    ｜        ",
  "            (4) ｜ 2  (-cpu 2)  ｜    なし    ｜   ↓   ",
  "            (5) ｜ 2  (-cpu 2)  ｜    あり    ｜  正確  ",
  "",
  "  設定(1) …… 最も高速で、アプリケーションの大部分が動作します。",
  "               デフォルトの設定はこれになります。",
  "  設定(2) …… やや高速です。一部のアプリケーションはこの設定でない",
  "               と動作しません。",
  "  設定(3) …… やや低速です。まれにこの設定でないと動作しないアプリ",
  "               ケーションがあります",
  "  設定(4)(5)… 最も低速です。この設定でないと動作しないアプリケー",
  "               ションはほとんど無いと思います。多分。",
  "",
  "  設定(1)でアプリケーションが動作しない場合、設定(2)、(3) … と変え",
  "てみてください。 また、動作するけれどもディスクのアクセス時に速度が",
  "低下する、サウンドが途切れる、などの場合も設定を変えると改善する",
  "可能性があります。",
  0,
};

static const char *help_en[] =
{
  " I'm waiting for translator... ",
  0,
};

/*--------------------------------------------------------------
 *
 *--------------------------------------------------------------*/

/* キーボード配列 (新旧別) のウィジット生成用データ */

typedef struct{
  char  *str;       /* キートップの文字 or パディング用空白     */
  int   code;       /* キーコード       or  0          */
} t_keymap;

static const t_keymap keymap_old0[] =
{
  { "STOP",       KEY88_STOP,    },
  { "COPY",       KEY88_COPY,    },
  { " ",          0,             },
  { "   f1   ",   KEY88_F1,      },
  { "   f2   ",   KEY88_F2,      },
  { "   f3   ",   KEY88_F3,      },
  { "   f4   ",   KEY88_F4,      },
  { "   f5   ",   KEY88_F5,      },
  { "   ",        0,             },
  { "R-UP",       KEY88_ROLLUP,  },
  { "R-DN",       KEY88_ROLLDOWN,},
  { "   ",        0,             },
  { " \036 ",     KEY88_UP,      },
  { " \037 ",     KEY88_DOWN,    },
  { " \035 ",     KEY88_LEFT,    },
  { " \034 ",     KEY88_RIGHT,   },
  { 0,0 },
};
static const t_keymap keymap_old1[] =
{
  { " ESC ", KEY88_ESC,       },
  { " 1 ",   KEY88_1,         },
  { " 2 ",   KEY88_2,         },
  { " 3 ",   KEY88_3,         },
  { " 4 ",   KEY88_4,         },
  { " 5 ",   KEY88_5,         },
  { " 6 ",   KEY88_6,         },
  { " 7 ",   KEY88_7,         },
  { " 8 ",   KEY88_8,         },
  { " 9 ",   KEY88_9,         },
  { " 0 ",   KEY88_0,         },
  { " - ",   KEY88_MINUS,     },
  { " ^ ",   KEY88_CARET,     },
  { " \\ ",  KEY88_YEN,       },
  { " BS ",  KEY88_INS_DEL,   },
  { "   ",   0,               },
  { "CLR",   KEY88_HOME,      },
  { "HLP",   KEY88_HELP,      },
  { " - ",   KEY88_KP_SUB,    },
  { " / ",   KEY88_KP_DIVIDE, },
  { 0,0 },
};
static const t_keymap keymap_old2[] =
{
  { "  TAB  ", KEY88_TAB,         },
  { " Q ",     KEY88_q,           },
  { " W ",     KEY88_w,           },
  { " E ",     KEY88_e,           },
  { " R ",     KEY88_r,           },
  { " T ",     KEY88_t,           },
  { " Y ",     KEY88_y,           },
  { " U ",     KEY88_u,           },
  { " I ",     KEY88_i,           },
  { " O ",     KEY88_o,           },
  { " P ",     KEY88_p,           },
  { " @ ",     KEY88_AT,          },
  { " [ ",     KEY88_BRACKETLEFT, },
  { "RETURN ", KEY88_RETURN,      },
  { "   ",     0,                 },
  { " 7 ",     KEY88_KP_7,        },
  { " 8 ",     KEY88_KP_8,        },
  { " 9 ",     KEY88_KP_9,        },
  { " * ",     KEY88_KP_MULTIPLY, },
  { 0,0 },
};
static const t_keymap keymap_old3[] =
{
  { "CTRL",      KEY88_CTRL,         },
  { "CAPS",      KEY88_CAPS,         },
  { " A ",       KEY88_a,            },
  { " S ",       KEY88_s,            },
  { " D ",       KEY88_d,            },
  { " F ",       KEY88_f,            },
  { " G ",       KEY88_g,            },
  { " H ",       KEY88_h,            },
  { " J ",       KEY88_j,            },
  { " K ",       KEY88_k,            },
  { " L ",       KEY88_l,            },
  { " ; ",       KEY88_SEMICOLON,    },
  { " : ",       KEY88_COLON,        },
  { " ] ",       KEY88_BRACKETRIGHT, },
  { "         ", 0,                  },
  { " 4 ",       KEY88_KP_4,         },
  { " 5 ",       KEY88_KP_5,         },
  { " 6 ",       KEY88_KP_6,         },
  { " + ",       KEY88_KP_ADD,       },
  { 0,0 },
};
static const t_keymap keymap_old4[] =
{
  { "    SHIFT   ", KEY88_SHIFT,      },
  { " Z ",          KEY88_z,          },
  { " X ",          KEY88_x,          },
  { " C ",          KEY88_c,          },
  { " V ",          KEY88_v,          },
  { " B ",          KEY88_b,          },
  { " N ",          KEY88_n,          },
  { " M ",          KEY88_m,          },
  { " , ",          KEY88_COMMA,      },
  { " . ",          KEY88_PERIOD,     },
  { " / ",          KEY88_SLASH,      },
  { " _ ",          KEY88_UNDERSCORE, },
  { " SHIFT ",      KEY88_SHIFT,      },
  { "   ",          0,                },
  { " 1 ",          KEY88_KP_1,       },
  { " 2 ",          KEY88_KP_2,       },
  { " 3 ",          KEY88_KP_3,       },
  { " = ",          KEY88_KP_EQUAL,   },
  { 0,0 },
};
static const t_keymap keymap_old5[] =
{
  { "       ",                                     0,               },
  { "KANA",                                        KEY88_KANA,      },
  { "GRPH",                                        KEY88_GRAPH,     },
  { "                                           ", KEY88_SPACE,     },
  { "                 ",                           0,               },
  { " 0 ",                                         KEY88_KP_0,      },
  { " , ",                                         KEY88_KP_COMMA,  },
  { " . ",                                         KEY88_KP_PERIOD, },
  { "RET",                                         KEY88_RETURN,    },
  { 0,0 },
};

static const t_keymap keymap_new0[] =
{
  { "STOP",   KEY88_STOP,     },
  { "COPY",   KEY88_COPY,     },
  { "  ",     0,              },
  { " f1 ",   KEY88_F1,       },
  { " f2 ",   KEY88_F2,       },
  { " f3 ",   KEY88_F3,       },
  { " f4 ",   KEY88_F4,       },
  { " f5 ",   KEY88_F5,       },
  { "   ",    0,              },
  { " f6 ",   KEY88_F6,       },
  { " f7 ",   KEY88_F7,       },
  { " f8 ",   KEY88_F8,       },
  { " f9 ",   KEY88_F9,       },
  { " f10 ",  KEY88_F10,      },
  { "   ",    0,              },
  { "ROLUP",  KEY88_ROLLUP,   },
  { "ROLDN",  KEY88_ROLLDOWN, },
  { 0,0 },
};
static const t_keymap keymap_new1[] =
{
  { " ESC ", KEY88_ESC,      },
  { " 1 ",   KEY88_1,        },
  { " 2 ",   KEY88_2,        },
  { " 3 ",   KEY88_3,        },
  { " 4 ",   KEY88_4,        },
  { " 5 ",   KEY88_5,        },
  { " 6 ",   KEY88_6,        },
  { " 7 ",   KEY88_7,        },
  { " 8 ",   KEY88_8,        },
  { " 9 ",   KEY88_9,        },
  { " 0 ",   KEY88_0,        },
  { " - ",   KEY88_MINUS,    },
  { " ^ ",   KEY88_CARET,    },
  { " \\ ",  KEY88_YEN,      },
  { " BS ",  KEY88_BS,       },
  { "   ",   0,              },
  { " DEL ", KEY88_DEL,      },
  { " INS ", KEY88_INS,      },
  { "  ",    0,              },
  { "CLR",  KEY88_HOME,      },
  { "HLP",  KEY88_HELP,      },
  { " - ",  KEY88_KP_SUB,    },
  { " / ",  KEY88_KP_DIVIDE, },
  { 0,0 },
};
static const t_keymap keymap_new2[] =
{
  { "  TAB  ",             KEY88_TAB,         },
  { " Q ",                 KEY88_q,           },
  { " W ",                 KEY88_w,           },
  { " E ",                 KEY88_e,           },
  { " R ",                 KEY88_r,           },
  { " T ",                 KEY88_t,           },
  { " Y ",                 KEY88_y,           },
  { " U ",                 KEY88_u,           },
  { " I ",                 KEY88_i,           },
  { " O ",                 KEY88_o,           },
  { " P ",                 KEY88_p,           },
  { " @ ",                 KEY88_AT,          },
  { " [ ",                 KEY88_BRACKETLEFT, },
  { "RETURN ",             KEY88_RETURNL,     },
  { "                   ", 0,                 },
  { " 7 ",                 KEY88_KP_7,        },
  { " 8 ",                 KEY88_KP_8,        },
  { " 9 ",                 KEY88_KP_9,        },
  { " * ",                 KEY88_KP_MULTIPLY, },
  { 0,0 },
};
static const t_keymap keymap_new3[] =
{
  { "CTRL",          KEY88_CTRL,         },
  { "CAPS",          KEY88_CAPS,         },
  { " A ",           KEY88_a,            },
  { " S ",           KEY88_s,            },
  { " D ",           KEY88_d,            },
  { " F ",           KEY88_f,            },
  { " G ",           KEY88_g,            },
  { " H ",           KEY88_h,            },
  { " J ",           KEY88_j,            },
  { " K ",           KEY88_k,            },
  { " L ",           KEY88_l,            },
  { " ; ",           KEY88_SEMICOLON,    },
  { " : ",           KEY88_COLON,        },
  { " ] ",           KEY88_BRACKETRIGHT, },
  { "             ", 0,                  },
  { " \036 ",        KEY88_UP,           },
  { "       ",       0,                  },
  { " 4 ",           KEY88_KP_4,         },
  { " 5 ",           KEY88_KP_5,         },
  { " 6 ",           KEY88_KP_6,         },
  { " + ",           KEY88_KP_ADD,       },
  { 0,0 },
};
static const t_keymap keymap_new4[] =
{
  { "    SHIFT   ", KEY88_SHIFTL,     },
  { " Z ",          KEY88_z,          },
  { " X ",          KEY88_x,          },
  { " C ",          KEY88_c,          },
  { " V ",          KEY88_v,          },
  { " B ",          KEY88_b,          },
  { " N ",          KEY88_n,          },
  { " M ",          KEY88_m,          },
  { " , ",          KEY88_COMMA,      },
  { " . ",          KEY88_PERIOD,     },
  { " / ",          KEY88_SLASH,      },
  { " _ ",          KEY88_UNDERSCORE, },
  { " SHIFT ",      KEY88_SHIFTR,     },
  { "  ",           0,                },
  { " \035 ",       KEY88_LEFT,       },
  { " \037 ",       KEY88_DOWN,       },
  { " \034 ",       KEY88_RIGHT,      },
  { "  ",           0,                },
  { " 1 ",          KEY88_KP_1,       },
  { " 2 ",          KEY88_KP_2,       },
  { " 3 ",          KEY88_KP_3,       },
  { " = ",          KEY88_KP_EQUAL,   },
  { 0,0 },
};
static const t_keymap keymap_new5[] =
{
  { "       ",                           0,              },
  { "KANA",                              KEY88_KANA,     },
  { "GRPH",                              KEY88_GRAPH,    },
  { " KETTEI ",                          KEY88_KETTEI,   },
  { "           ",                       KEY88_SPACE,    },
  { "  HENKAN  ",                        KEY88_HENKAN,   },
  { "PC ",                               KEY88_PC,       },
  { "ZEN",                               KEY88_ZENKAKU,  },
  { "                                 ", 0               },
  { " 0 ",                               KEY88_KP_0,     },
  { " , ",                               KEY88_KP_COMMA, },
  { " . ",                               KEY88_KP_PERIOD,},
  { "RET",                               KEY88_RETURNR,  },
  { 0,0 },
};

static const t_keymap * keymap_line[2][6] =
{
  {
    keymap_old0,
    keymap_old1,
    keymap_old2,
    keymap_old3,
    keymap_old4,
    keymap_old5,
  },
  {
    keymap_new0,
    keymap_new1,
    keymap_new2,
    keymap_new3,
    keymap_new4,
    keymap_new5,
  },
};


/* キー配列変更コンボボックス用 のウィジット生成用データ */

static const t_keymap keymap_assign[] =
{
  { "(none)",    KEY88_INVALID      },
  { "0 (10)",    KEY88_KP_0         },
  { "1 (10)",    KEY88_KP_1         },
  { "2 (10)",    KEY88_KP_2         },
  { "3 (10)",    KEY88_KP_3         },
  { "4 (10)",    KEY88_KP_4         },
  { "5 (10)",    KEY88_KP_5         },
  { "6 (10)",    KEY88_KP_6         },
  { "7 (10)",    KEY88_KP_7         },
  { "8 (10)",    KEY88_KP_8         },
  { "9 (10)",    KEY88_KP_9         },
  { "* (10)",    KEY88_KP_MULTIPLY  },
  { "+ (10)",    KEY88_KP_ADD       },
  { "= (10)",    KEY88_KP_EQUAL     },
  { ", (10)",    KEY88_KP_COMMA     },
  { ". (10)",    KEY88_KP_PERIOD    },
  { "- (10)",    KEY88_KP_SUB       },
  { "/ (10)",    KEY88_KP_DIVIDE    },
  { "A     ",    KEY88_a            },
  { "B     ",    KEY88_b            },
  { "C     ",    KEY88_c            },
  { "D     ",    KEY88_d            },
  { "E     ",    KEY88_e            },
  { "F     ",    KEY88_f            },
  { "G     ",    KEY88_g            },
  { "H     ",    KEY88_h            },
  { "I     ",    KEY88_i            },
  { "J     ",    KEY88_j            },
  { "K     ",    KEY88_k            },
  { "L     ",    KEY88_l            },
  { "M     ",    KEY88_m            },
  { "N     ",    KEY88_n            },
  { "O     ",    KEY88_o            },
  { "P     ",    KEY88_p            },
  { "Q     ",    KEY88_q            },
  { "R     ",    KEY88_r            },
  { "S     ",    KEY88_s            },
  { "T     ",    KEY88_t            },
  { "U     ",    KEY88_u            },
  { "V     ",    KEY88_v            },
  { "W     ",    KEY88_w            },
  { "X     ",    KEY88_x            },
  { "Y     ",    KEY88_y            },
  { "Z     ",    KEY88_z            },
  { "0     ",    KEY88_0            },
  { "1 (!) ",    KEY88_1            },
  { "2 (\") ",   KEY88_2            },
  { "3 (#) ",    KEY88_3            },
  { "4 ($) ",    KEY88_4            },
  { "5 (%) ",    KEY88_5            },
  { "6 (&) ",    KEY88_6            },
  { "7 (') ",    KEY88_7            },
  { "8 (() ",    KEY88_8            },
  { "9 ()) ",    KEY88_9            },
  { ", (<) ",    KEY88_COMMA        },
  { "- (=) ",    KEY88_MINUS        },
  { ". (>) ",    KEY88_PERIOD       },
  { "/ (?) ",    KEY88_SLASH        },
  { ": (*) ",    KEY88_COLON        },
  { "; (+) ",    KEY88_SEMICOLON    },
  { "@ (~) ",    KEY88_AT           },
  { "[ ({) ",    KEY88_BRACKETLEFT  },
  { "\\ (|) ",   KEY88_YEN          },
  { "] (}) ",    KEY88_BRACKETRIGHT },
  { "^     ",    KEY88_CARET        },
  { "  (_) ",    KEY88_UNDERSCORE   },
  { "space ",    KEY88_SPACE        },
  { "RETURN",    KEY88_RETURN       },
  { "SHIFT ",    KEY88_SHIFT        },
  { "CTRL  ",    KEY88_CTRL         },
  { "CAPS  ",    KEY88_CAPS         },
  { "kana  ",    KEY88_KANA         },
  { "GRPH  ",    KEY88_GRAPH        },
  { "HM-CLR",    KEY88_HOME         },
  { "HELP  ",    KEY88_HELP         },
  { "DELINS",    KEY88_INS_DEL      },
  { "STOP  ",    KEY88_STOP         },
  { "COPY  ",    KEY88_COPY         },
  { "ESC   ",    KEY88_ESC          },
  { "TAB   ",    KEY88_TAB          },
  { "\036     ", KEY88_UP           },
  { "\037     ", KEY88_DOWN         },
  { "\035     ", KEY88_LEFT         },
  { "\034     ", KEY88_RIGHT        },
  { "ROLLUP",    KEY88_ROLLUP       },
  { "ROLLDN",    KEY88_ROLLDOWN     },
  { "f1    ",    KEY88_F1           },
  { "f2    ",    KEY88_F2           },
  { "f3    ",    KEY88_F3           },
  { "f4    ",    KEY88_F4           },
  { "f5    ",    KEY88_F5           },
#if 1
  { "f6    ",    KEY88_F6           },
  { "f7    ",    KEY88_F7           },
  { "f8    ",    KEY88_F8           },
  { "f9    ",    KEY88_F9           },
  { "f10   ",    KEY88_F10          },
  { "BS    ",    KEY88_BS           },
  { "INS   ",    KEY88_INS          },
  { "DEL   ",    KEY88_DEL          },
  { "henkan",    KEY88_HENKAN       },
  { "kettei",    KEY88_KETTEI       },
  { "PC    ",    KEY88_PC           },
  { "zenkak",    KEY88_ZENKAKU      },
  { "RET  L",    KEY88_RETURNL      },
  { "RET  R",    KEY88_RETURNR      },
  { "SHIFTL",    KEY88_SHIFTL       },
  { "SHIFTR",    KEY88_SHIFTR       },
#endif
};




/************************************************************************/
/*                                  */
/* この QUASI88 メニュー用 Tool Kit のAPIは、どうも似た処理の繰り返しが  */
/* 多くなるので、似たような処理をまとめた関数を作ってみた。     */
/*                                  */
/************************************************************************/

/* t_menulabel の index 番目の文字列を取得するマクロ -------------------*/

#define     GET_LABEL(l, index) (l[index].str[menu_lang])


/* フレームを生成する --------------------------------------------------
                box != NULL なら、これに PACK する。
                label   フレームのラベル
                widget  != NULL なら、これを乗せる。
*/
static  Q8tkWidget *PACK_FRAME(Q8tkWidget *box,
                   const char *label, Q8tkWidget *widget)
{
    Q8tkWidget *frame = q8tk_frame_new(label);

    if (widget)
    q8tk_container_add(frame, widget);

    q8tk_widget_show(frame);
    if (box)
    q8tk_box_pack_start(box, frame);

    return frame;
}


/* HBOXを生成する ------------------------------------------------------
                box != NULL なら、これに PACK する。
*/
static  Q8tkWidget *PACK_HBOX(Q8tkWidget *box)
{
    Q8tkWidget *hbox = q8tk_hbox_new();

    q8tk_widget_show(hbox);
    if (box)
    q8tk_box_pack_start(box, hbox);

    return hbox;
}


/* VBOXを生成する ------------------------------------------------------
                box != NULL なら、これに PACK する。
*/
static  Q8tkWidget *PACK_VBOX(Q8tkWidget *box)
{
    Q8tkWidget *vbox = q8tk_vbox_new();

    q8tk_widget_show(vbox);
    if (box)
    q8tk_box_pack_start(box, vbox);

    return vbox;
}


/* LABEL を生成する ----------------------------------------------------
                box != NULL なら、これに PACK する。
                label   ラベル
*/
static  Q8tkWidget *PACK_LABEL(Q8tkWidget *box, const char *label)
{
    Q8tkWidget *labelwidget = q8tk_label_new(label);

    q8tk_widget_show(labelwidget);
    if (box)
    q8tk_box_pack_start(box, labelwidget);

    return labelwidget;
}


/* VSEPATATOR を生成する -----------------------------------------------
                box != NULL なら、これに PACK する。
*/
static  Q8tkWidget *PACK_VSEP(Q8tkWidget *box)
{
    Q8tkWidget *vsep = q8tk_vseparator_new();

    q8tk_widget_show(vsep);
    if (box)
    q8tk_box_pack_start(box, vsep);

    return vsep;
}


/* HSEPATATOR を生成する -----------------------------------------------
                box != NULL なら、これに PACK する。
*/
static  Q8tkWidget *PACK_HSEP(Q8tkWidget *box)
{
    Q8tkWidget *hsep = q8tk_hseparator_new();

    q8tk_widget_show(hsep);
    if (box)
    q8tk_box_pack_start(box, hsep);

    return hsep;
}


/* ボタンを生成する ----------------------------------------------------
                box != NULL なら、これに PACK する。
                label   ラベル
                callback "clicked" 時のコールバック関数
                parm    そのパラメータ
*/
static  Q8tkWidget *PACK_BUTTON(Q8tkWidget *box,
                const char *label,
                Q8tkSignalFunc callback, void *parm)
{
    Q8tkWidget *button = q8tk_button_new_with_label(label);

    q8tk_signal_connect(button, "clicked", callback, parm);
    q8tk_widget_show(button);
    if (box)
    q8tk_box_pack_start(box, button);

    return button;
}



/* チェックボタンを生成する --------------------------------------------
                box != NULL なら、これに PACK する。
                label   ラベル
                on  真なら、チェック状態とする
                callback "clicked" 時のコールバック関数
                parm    そのパラメータ
*/
static  Q8tkWidget *PACK_CHECK_BUTTON(Q8tkWidget *box,
                      const char *label, int on,
                      Q8tkSignalFunc callback, void *parm)
{
    Q8tkWidget *button = q8tk_check_button_new_with_label(label);

    if (on)
    q8tk_toggle_button_set_state(button, TRUE);

    q8tk_signal_connect(button, "toggled", callback, parm);
    q8tk_widget_show(button);
    if (box)
    q8tk_box_pack_start(box, button);

    return button;
}


/* ラジオボタンを生成する --------------------------------------------
                box != NULL なら、これに PACK する。
                button  グループを形成するボタン
                label   ラベル
                on  真なら、チェック状態とする
                callback "clicked" 時のコールバック関数
                parm    そのパラメータ
*/
static  Q8tkWidget *PACK_RADIO_BUTTON(Q8tkWidget *box,
                      Q8tkWidget *button,
                      const char *label, int on,
                      Q8tkSignalFunc callback, void *parm)
{
    Q8tkWidget *b = q8tk_radio_button_new_with_label(button, label);

    q8tk_widget_show(b);
    q8tk_signal_connect(b, "clicked", callback, parm);

    if (on)
    q8tk_toggle_button_set_state(b, TRUE);

    if (box)
    q8tk_box_pack_start(box, b);

    return b;
}


/* コンボボックスを生成する --------------------------------------------
                box != NULL なら、これに PACK する。
                p   t_menudata 型配列の先頭ポインタ。
                count   その配列の数。
                    p[0] 〜 p[count-1] までのデータの
                    文字列をコンボボックス化する。
                initval p[].val == initval の場合、その
                    文字列を初期文字列とする。
                initstr 上記該当が無い場合の初期文字列
                width   表示サイズ。0で自動
                act_callback "activate"時のコールバック関数
                act_parm     そのパラメータ
                chg_callback "changed"時のコールバック関数
                chg_parm     そのパラメータ
*/
static  Q8tkWidget *PACK_COMBO(Q8tkWidget *box,
                   const t_menudata *p, int count,
                   int initval, const char *initstr, int width,
                   Q8tkSignalFunc act_callback, void *act_parm,
                   Q8tkSignalFunc chg_callback, void *chg_parm)
{
    int i;
    Q8tkWidget *combo = q8tk_combo_new();

    for (i=0; i<count; i++, p++) {
    q8tk_combo_append_popdown_strings(combo, p->str[menu_lang], NULL);

    if (initval == p->val) initstr = p->str[menu_lang];
    }

    q8tk_combo_set_text(combo, initstr ? initstr : " ");
    q8tk_signal_connect(combo, "activate", act_callback, act_parm);
    if (chg_callback) {
    q8tk_combo_set_editable(combo, TRUE);
    q8tk_signal_connect(combo, "changed",  chg_callback, chg_parm);
    }
    q8tk_widget_show(combo);

    if (width)
    q8tk_misc_set_size(combo, width, 0);

    if (box)
    q8tk_box_pack_start(box, combo);

    return combo;
}


/* エントリーを生成する ------------------------------------------------
                box != NULL なら、これに PACK する。
                length  入力文字列長。0で無限
                width   表示文字列長。0で自動
                text    初期文字列
                act_callback "activate"時のコールバック関数
                act_parm     そのパラメータ
                chg_callback "changed"時のコールバック関数
                chg_parm     そのパラメータ
*/
static  Q8tkWidget *PACK_ENTRY(Q8tkWidget *box,
                   int length, int width, const char *text,
                   Q8tkSignalFunc act_callback, void *act_parm,
                   Q8tkSignalFunc chg_callback, void *chg_parm)
{
    Q8tkWidget *e;

    e = q8tk_entry_new_with_max_length(length);

    if (width)
    q8tk_misc_set_size(e, width, 1);

    if (text)
    q8tk_entry_set_text(e, text);

    if (act_callback)
    q8tk_signal_connect(e, "activate", act_callback, act_parm);
    if (chg_callback)
    q8tk_signal_connect(e, "changed",  chg_callback, chg_parm);

    q8tk_misc_set_placement(e, 0, Q8TK_PLACEMENT_Y_CENTER);
    q8tk_widget_show(e);

    if (box)
    q8tk_box_pack_start(box, e);

    return e;
}


/*======================================================================*/

/* チェックボタンを複数生成する ----------------------------------------
                box これに PACK する。(NULLは禁止)
                p   t_menudata 型配列の先頭ポインタ。
                count   その配列の数。
                    p[0] 〜 p[count-1] までのデータの
                    文字列でチェックボタンを生成する。
                f_initval 関数 (*f_initval)(p[].val) が真
                      なら、チェック状態とする
                callback "toggled"時のコールバック関数
                     パラメータは (void*)(p[].val)
*/
static  void    PACK_CHECK_BUTTONS(Q8tkWidget *box,
                   const t_menudata *p, int count,
                   int (*f_initval)(int),
                   Q8tkSignalFunc callback)
{
    int i;
    Q8tkWidget *button;

    for (i=0; i<count; i++, p++) {

    button = q8tk_check_button_new_with_label(p->str[menu_lang]);

    if ((*f_initval)(p->val))
        q8tk_toggle_button_set_state(button, TRUE);

    q8tk_signal_connect(button, "toggled", callback, (void *)(p->val));

    q8tk_widget_show(button);
    q8tk_box_pack_start(box, button);
    }
}


/* ラジオボタンを複数生成する ------------------------------------------
                box これに PACK する。(NULLは禁止)
                p   t_menudata 型配列の先頭ポインタ。
                count   その配列の数。
                    p[0] 〜 p[count-1] までのデータの
                    文字列でラジオボタンを生成する。
                initval p[].val == initval ならば、
                    そのボタンをON状態とする
                callback "clicked"時のコールバック関数
                     パラメータは (void*)(p[].val)
*/
static  Q8List  *PACK_RADIO_BUTTONS(Q8tkWidget *box,
                    const t_menudata *p, int count,
                    int initval, Q8tkSignalFunc callback)
{
    int i;
    Q8tkWidget *button = NULL;

    for (i=0; i<count; i++, p++) {

    button = q8tk_radio_button_new_with_label(button, p->str[menu_lang]);

    q8tk_widget_show(button);
    q8tk_box_pack_start(box, button);
    q8tk_signal_connect(button, "clicked", callback, (void *)(p->val));

    if (initval == p->val) {
        q8tk_toggle_button_set_state(button, TRUE);
    }
    }
    return q8tk_radio_button_get_list(button);
}


/* HSCALEを生成する ----------------------------------------------------
                box != NULL なら、これに PACK する。
                p   t_volume 型配列の先頭ポインタ。
                    この情報をもとにHSCALEを生成。
                initval 初期値
                callback "value_changed"時のコールバック関数
                parm     そのパラメータ
*/
static  Q8tkWidget *PACK_HSCALE(Q8tkWidget *box,
                const t_volume *p,
                int initval,
                Q8tkSignalFunc callback, void *parm)
{
    Q8tkWidget *adj, *scale;

    adj = q8tk_adjustment_new(initval,
                  p->min, p->max, p->step, p->page);

    q8tk_signal_connect(adj, "value_changed", callback, parm);

    scale = q8tk_hscale_new(adj);
    q8tk_adjustment_set_arrow(scale->stat.scale.adj, TRUE);
    /*q8tk_adjustment_set_length(scale->stat.scale.adj, 11);*/
    q8tk_scale_set_draw_value(scale, TRUE);
    q8tk_scale_set_value_pos(scale, Q8TK_POS_LEFT);

    q8tk_widget_show(scale);

    if (box)
    q8tk_box_pack_start(box, scale);

    return scale;
}



/* キーアサイン変更用ウィジットを生成する ------------------------------ */
static  Q8tkWidget *MAKE_KEY_COMBO(Q8tkWidget *box,
                   const t_menudata *p,
                   int (*f_initval)(int),
                   Q8tkSignalFunc callback)
{
    {
    Q8tkWidget *label = q8tk_label_new(GET_LABEL(p, 0));
    q8tk_box_pack_start(box, label);
    q8tk_widget_show(label);
    }
    {
    int i;
    const t_keymap *k = keymap_assign;
    const char     *initstr = " ";
    int             initval = (*f_initval)(p->val);

    Q8tkWidget *combo = q8tk_combo_new();

    for (i=0; i<COUNTOF(keymap_assign); i++, k++) {
        q8tk_combo_append_popdown_strings(combo, k->str, NULL);

        if (initval == k->code) initstr = k->str;
    }

    q8tk_combo_set_text(combo, initstr);
    q8tk_misc_set_size(combo, 6, 0);
    q8tk_signal_connect(combo, "activate", callback, (void*)(p->val));

    q8tk_box_pack_start(box, combo);
    q8tk_widget_show(combo);

    return combo;
    }
}

static  Q8tkWidget *PACK_KEY_ASSIGN(Q8tkWidget *box,
                    const t_menudata *p, int count,
                    int (*f_initval)(int),
                    Q8tkSignalFunc callback)
{
    int i;
    Q8tkWidget *vbox, *hbox, *allbox;

    vbox = q8tk_vbox_new();
    {
    {                           /* combo */
        hbox = q8tk_hbox_new();
        {
        MAKE_KEY_COMBO(hbox, p, f_initval, callback);
        p++;
        }
        q8tk_widget_show(hbox);
        q8tk_box_pack_start(vbox, hbox);
    }

    PACK_LABEL(vbox, GET_LABEL(p, 0));          /* ↑ */
    p++;

    {                       /* combo ← → combo */
        hbox = q8tk_hbox_new();
        {
        MAKE_KEY_COMBO(hbox, p, f_initval, callback);
        p++;
        MAKE_KEY_COMBO(hbox, p, f_initval, callback);
        p++;
        }
        q8tk_widget_show(hbox);
        q8tk_box_pack_start(vbox, hbox);
    }

    PACK_LABEL(vbox, GET_LABEL(p, 0));          /* ↓ */
    p++;

    {                           /* combo */
        hbox = q8tk_hbox_new();
        {
        MAKE_KEY_COMBO(hbox, p, f_initval, callback);
        p++;
        }
        q8tk_widget_show(hbox);
        q8tk_box_pack_start(vbox, hbox);
    }
    }
    q8tk_widget_show(vbox);


    if (count < 6) {        /* 方向キーだけで処理終わり */

    allbox = vbox;

    } else {            /* 他にも処理するキーあり */

    allbox = q8tk_hbox_new();
    q8tk_box_pack_start(allbox, vbox);

    {
        vbox = q8tk_vbox_new();
        for (i=6; i<count; i++) {
        if (p->val < 0) {
            PACK_LABEL(vbox, GET_LABEL(p, 0));
            p++;
        } else {
            hbox = q8tk_hbox_new();
            {
            MAKE_KEY_COMBO(hbox, p, f_initval, callback);
            p++;
            }
            q8tk_widget_show(hbox);
            q8tk_box_pack_start(vbox, hbox);
        }
        }
        q8tk_widget_show(vbox);
        q8tk_box_pack_start(allbox, vbox);
    }

    q8tk_widget_show(allbox);
    }


    if (box)
    q8tk_box_pack_start(box, allbox);
    
    return allbox;
}


/*======================================================================*/

/* ファイルセレクションを生成する --------------------------------------
    生成後、ウインドウをグラブする
    CANCEL時は、なにも処理せずにグラブを離す。
    OK 時は、 (*ok_button)()を呼び出す。この時、ファイル名は
    get_filename に、リードオンリー属性は get_ro にセットされている。
    なお、呼び出した時点ではすでにグラブを離す。
                label   ラベル
                select_ro >=0なら、ReadOnly選択可(1でチェック)
                filename  初期ファイル(ディレクトリ)名
                ok_button OK時に呼び出す関数
                get_filename    OK時にファイル名をこの
                        バッファにセット
                sz_get_filename このバッファのサイズ
                get_ro  select_ro が >=0 の時、リードオンリー
                    選択情報がここにセット
*/
static struct{
    void    (*ok_button)(void); /* OK押下時の呼び出す関数   */
    char    *get_filename;      /* 選択したファイル名格納先 */
    int     sz_get_filename;    /* そのバッファサイズ       */
    int     *get_ro;        /* RO かどうかのフラグ      */
    Q8tkWidget  *accel;
} FSEL;
static void cb_fsel_ok(UNUSED_WIDGET, Q8tkWidget *f);
static void cb_fsel_cancel(UNUSED_WIDGET, Q8tkWidget *f);

static void START_FILE_SELECTION(const char *label, /* タイトル       */
                 int select_ro,     /* RO選択状態     */
                 const char *filename,  /* 初期ファイル名 */

                 void (*ok_button)(void),
                 char *get_filename,
                 int  sz_get_filename,
                 int  *get_ro)
{
    Q8tkWidget *f;

    f = q8tk_file_selection_new(label, select_ro);
    q8tk_widget_show(f);
    q8tk_grab_add(f);

    if (filename)
    q8tk_file_selection_set_filename(f, filename);

    q8tk_signal_connect(Q8TK_FILE_SELECTION(f)->ok_button,
            "clicked", cb_fsel_ok, f);
    q8tk_signal_connect(Q8TK_FILE_SELECTION(f)->cancel_button,
            "clicked", cb_fsel_cancel, f);
    q8tk_widget_set_focus(Q8TK_FILE_SELECTION(f)->cancel_button);

    FSEL.ok_button       = ok_button;
    FSEL.get_filename    = get_filename;
    FSEL.sz_get_filename = sz_get_filename;
    FSEL.get_ro          = (select_ro >= 0) ? get_ro : NULL;

    FSEL.accel = q8tk_accel_group_new();

    q8tk_accel_group_attach(FSEL.accel, f);
    q8tk_accel_group_add(FSEL.accel, Q8TK_KEY_ESC,
             Q8TK_FILE_SELECTION(f)->cancel_button, "clicked");
}


static void cb_fsel_cancel(UNUSED_WIDGET, Q8tkWidget *f)
{
    q8tk_grab_remove(f);
    q8tk_widget_destroy(f);
    q8tk_widget_destroy(FSEL.accel);
}

static void cb_fsel_ok(UNUSED_WIDGET, Q8tkWidget *f)
{
    *FSEL.get_filename = '\0';
    strncat(FSEL.get_filename, q8tk_file_selection_get_filename(f), 
        FSEL.sz_get_filename - 1);

    if (FSEL.get_ro)
    *FSEL.get_ro = q8tk_file_selection_get_readonly(f);

    q8tk_grab_remove(f);
    q8tk_widget_destroy(f);
    q8tk_widget_destroy(FSEL.accel);

    if (FSEL.ok_button)
    (*FSEL.ok_button)();
}


/*======================================================================*/

/* ダイアログをを生成する ----------------------------------------------

    ダイアログは、以下の構成とする
    +-----------------------------------+
    |               見出し 1            |    見出しラベル   (1個以上)
    |                 ：                |
    |               見出し 2            |
    |                 ：                |
    |          [チェックボタン]         |    チェックボタン (1個以上)
    | --------------------------------- |   セパレータ     (1個以上)
    | [エントリ] [ボタン] …… [ボタン] |   エントリ       (最大1個)
    +-----------------------------------+   ボタン         (1個以上)

    ラベル、セパレータ、ボタン、エントリは全部合わせて最大で、
    DIA_MAX 個まで。
    最後に追加したウィジット (ボタンかエントリ) にフォーカスがくる。
*/

#define DIA_MAX     (12)

static  Q8tkWidget  *dialog[ DIA_MAX ];
static  Q8tkWidget  *dialog_main;
static  int     dialog_num;
static  Q8tkWidget  *dialog_entry;
static  Q8tkWidget  *dialog_accel;


/* ダイアログ作成開始 */

static  void    dialog_create(void)
{
    int i;
    Q8tkWidget *d = q8tk_dialog_new();
    Q8tkWidget *a = q8tk_accel_group_new();

    q8tk_misc_set_placement(Q8TK_DIALOG(d)->action_area,
                Q8TK_PLACEMENT_X_CENTER, Q8TK_PLACEMENT_Y_CENTER);

    q8tk_accel_group_attach(a, d);


    for (i=0; i<DIA_MAX; i++) dialog[ i ] = NULL;
    dialog_num   = 0;
    dialog_entry = NULL;

    dialog_accel = a;
    dialog_main  = d;
}

/* ダイアログにラベル（見出し）を追加。複数個、追加できる */

static  void    dialog_set_title(const char *label)
{
    Q8tkWidget *l = q8tk_label_new(label);

    if (dialog_num>=DIA_MAX) {fprintf(stderr, "%s %d\n", __FILE__, __LINE__);}

    q8tk_box_pack_start(Q8TK_DIALOG(dialog_main)->vbox, l);
    q8tk_widget_show(l);
    q8tk_misc_set_placement(l, Q8TK_PLACEMENT_X_CENTER, Q8TK_PLACEMENT_Y_TOP);

    dialog[ dialog_num ++ ] = l;
}

/* ダイアログにチェックボタンを追加 (引数…ボタン名,状態,コールバック関数) */

static  void    dialog_set_check_button(const char *label, int on,
                    Q8tkSignalFunc callback, void *parm)
{
    Q8tkWidget *b = q8tk_check_button_new_with_label(label);

    if (dialog_num>=DIA_MAX) {fprintf(stderr, "%s %d\n", __FILE__, __LINE__);}

    if (on)
    q8tk_toggle_button_set_state(b, TRUE);

    q8tk_box_pack_start(Q8TK_DIALOG(dialog_main)->vbox, b);
    q8tk_widget_show(b);
    q8tk_signal_connect(b, "toggled", callback, parm);

    dialog[ dialog_num ++ ] = b;
}

/* ダイアログにセパレータを追加。 */

static  void    dialog_set_separator(void)
{
    Q8tkWidget *s = q8tk_hseparator_new();

    if (dialog_num>=DIA_MAX) {fprintf(stderr, "%s %d\n", __FILE__, __LINE__);}

    q8tk_box_pack_start(Q8TK_DIALOG(dialog_main)->vbox, s);
    q8tk_widget_show(s);

    dialog[ dialog_num ++ ] = s;
}

/* ダイアログにボタンを追加 (引数…ボタンの名称,コールバック関数) */

static  void    dialog_set_button(const char *label,
                  Q8tkSignalFunc callback, void *parm)
{
    Q8tkWidget *b = q8tk_button_new_with_label(label);

    if (dialog_num>=DIA_MAX) {fprintf(stderr, "%s %d\n", __FILE__, __LINE__);}

    q8tk_box_pack_start(Q8TK_DIALOG(dialog_main)->action_area, b);
    q8tk_widget_show(b);
    q8tk_signal_connect(b, "clicked", callback, parm);

    dialog[ dialog_num ++ ] = b;
}

/* ダイアログにエントリを追加 (引数…初期文字列,最大文字数,コールバック関数) */

static  void    dialog_set_entry(const char *text, int max_length,
                 Q8tkSignalFunc callback, void *parm)
{
    Q8tkWidget *e = q8tk_entry_new_with_max_length(max_length);

    q8tk_box_pack_start(Q8TK_DIALOG(dialog_main)->action_area, e);
    q8tk_widget_show(e);
    q8tk_signal_connect(e, "activate", callback, parm);
    q8tk_misc_set_size(e, max_length+1, 0);
    q8tk_misc_set_placement(e, 0, Q8TK_PLACEMENT_Y_CENTER);
    q8tk_entry_set_text(e, text);

    dialog_entry = e;

    dialog[ dialog_num ++ ] = e;
}

/* ダイアログ内の、エントリの文字列をとり出す */

static  const   char    *dialog_get_entry(void)
{
    return q8tk_entry_get_text(dialog_entry);
}

/* 直前に追加したダイアログのボタンに、ショートカットキーを設定 */

static  void    dialog_accel_key(int key)
{
    Q8tkWidget *w = dialog[ dialog_num-1 ];
    q8tk_accel_group_add(dialog_accel, key, w, "clicked");
}

/* ダイアログ表示開始 (グラブされる。フォーカスは最後に追加したボタンへ) */

static  void    dialog_start(void)
{
    q8tk_widget_show(dialog_main);
    q8tk_grab_add(dialog_main);

    if (dialog[ dialog_num -1 ]) {
    q8tk_widget_set_focus(dialog[ dialog_num -1 ]);
    }
}

/* ダイアログを消去 (ウインドウを消去し、グラブを解除する) */

static  void    dialog_destroy(void)
{
    int i;
    for (i=0; i<DIA_MAX; i++) {
    if (dialog[i]) {
        q8tk_widget_destroy(dialog[i]);
        dialog[i] = NULL;
    }
    }

    q8tk_grab_remove(dialog_main);
    q8tk_widget_destroy(dialog_main);
    q8tk_widget_destroy(dialog_accel);

    dialog_num   = 0;
    dialog_main  = NULL;
    dialog_entry = NULL;
    dialog_accel = NULL;
}
