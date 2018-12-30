/***********************************************************************
 * サウンド出力処理 (システム依存)
 *
 *      詳細は、 snddrv.h / mame-quasi88.h 参照
 ************************************************************************/

#include "quasi88.h"
#include "device.h"

#include "snddrv.h"

#ifdef	USE_SOUND

#include "mame-quasi88.h"

/*===========================================================================*/
/*              QUASI88 から呼び出される、MAME の処理関数                    */
/*===========================================================================*/

/******************************************************************************
 * サウンド系オプション処理の初期化／終了関数
 *
 * int xmame_config_init(void)
 *      config_init() より、オプションの解析を開始する前に呼び出される。
 *      MAME依存ワークの初期化などが必要な場合は、ここで行う。
 *
 * void xmame_config_exit(void)
 *      config_exit() より、処理の最後に呼び出される。
 *      xmame_config_init() の処理の後片付けが必要なら、ここで行う。
 *
 *****************************************************************************/
int	xmame_config_init(void)
{
    return 0;		/*OSD_OK;*/
}
void	xmame_config_exit(void)
{
}


/******************************************************************************
 * サウンド系オプションのオプションテーブル取得
 *
 * const T_CONFIG_TABLE *xmame_config_get_opt_tbl(void)
 *      config_init() より、 xmame_config_init() の後に呼び出される。
 *
 *      サウンド系オプションの解析処理において、 QUASI88 のオプションテーブル
 *      T_CONFIG_TABLE を使用する場合、そのポインタを返す。
 *      独自方式で解析する場合は、 NULL を返す。
 *****************************************************************************/

/* 以下は、標準的なオプションのテーブル例 */

static	const	T_CONFIG_TABLE xmame_options[] =
{
  /* 350〜399: サウンド依存オプション */

  { 351, "sound",        X_FIX,  &use_sound,       TRUE,                  0,0, OPT_SAVE },
  { 351, "nosound",      X_FIX,  &use_sound,       FALSE,                 0,0, OPT_SAVE },

  { 353, "fmgen",        X_FIX,  &use_fmgen,       TRUE,                  0,0, OPT_SAVE },
  { 353, "nofmgen",      X_FIX,  &use_fmgen,       FALSE,                 0,0, OPT_SAVE },

/*{ 354, "volume",       X_INT,  &attenuation,     -32, 0,                  0, OPT_SAVE },*/

  { 355, "fmvol",        X_INT,  &fmvol,           0, 100,                  0, OPT_SAVE },
  { 356, "psgvol",       X_INT,  &psgvol,          0, 100,                  0, OPT_SAVE },
  { 357, "beepvol",      X_INT,  &beepvol,         0, 100,                  0, OPT_SAVE },
  { 358, "rhythmvol",    X_INT,  &rhythmvol,       0, 200,                  0, OPT_SAVE },
  { 359, "adpcmvol",     X_INT,  &adpcmvol,        0, 200,                  0, OPT_SAVE },
  { 360, "fmgenvol",     X_INT,  &fmgenvol,        0, 100,                  0, OPT_SAVE },
  { 361, "samplevol",    X_INT,  &samplevol,       0, 100,                  0, OPT_SAVE },

  { 362, "samplefreq",   X_INT,  &options.samplerate, 8000, 48000,          0, OPT_SAVE },

  { 363, "samples",      X_FIX,  &options.use_samples, 1,                 0,0, OPT_SAVE },
  { 363, "nosamples",    X_FIX,  &options.use_samples, 0,                 0,0, OPT_SAVE },

  /* 終端 */
  {   0, NULL,           X_INV,                                       0,0,0,0, 0        },
};

const T_CONFIG_TABLE *xmame_config_get_opt_tbl(void)
{
    return xmame_options;
}


/******************************************************************************
 * サウンド系オプションのヘルプメッセージを表示
 *
 * void xmame_config_show_option(void)
 *      config_init() より、オプション -help の処理の際に呼び出される。
 *      標準出力にヘルプメッセージを表示する。
 *****************************************************************************/
#ifdef	XMAME_SNDDRV_071
#define	XMAME_VER "0.71.1"
#else	/* ver 0.106 */
#define	XMAME_VER " 0.106"
#endif

void	xmame_config_show_option(void)
{
  fprintf(stdout,
  "\n"
  "==========================================\n"
  "== SOUND OPTIONS ( dependent on XMAME ) ==\n"
  "==                     [ XMAME " XMAME_VER " ] ==\n"
  "==========================================\n"
  "    -[no]sound              Enable/disable sound (if available) [-sound]\n"
  "    -[no]fmgen              Use/don't use cisc's fmgen library\n"
  "                                               (if compiled in)  [-nofmgen]\n"
/*"    -volume <n>             Set volume to <n> db, (-32 (soft) - 0(loud)) [0]\n"*/
  "    -fmvol <level>          Set FM     level to <level> %%, (0 - 100) [100]\n"
  "    -psgvol <level>         Set PSG    level to <level> %%, (0 - 100) [20]\n"
  "    -beepvol <level>        Set BEEP   level to <level> %%, (0 - 100) [60]\n"
  "    -rhythmvol <level>      Set RHYTHM level to <level> %%, (0 - 100) [100]\n"
  "    -adpcmvol <level>       Set ADPCM  level to <level> %%, (0 - 100) [100]\n"
  "    -fmgenvol <level>       Set fmgen  level to <level> %%, (0 - 100) [100]\n"
  "    -samplevol <level>      Set SAMPLE level to <level> %%, (0 - 100) [100]\n"
  "    -samplefreq <rate>      Set the playback sample-frequency/rate [44100]\n"
  "    -[no]samples            Use/don't use samples (if available) [-nosamples]\n"
/*"    -[no]close              Close/no close sound device in MENU mode [-noclose]\n"*/
  );
}


/******************************************************************************
 * サウンド系オプションの解析処理
 *
 * int xmame_config_check_option(char *opt1, char *opt2, int priority)
 *      config_init() より、引数や設定ファイルの解析を行なう際に、
 *      規定のオプションのいずれにも合致しない場合、この関数が呼び出される。
 *
 *              opt1     … 最初の引数(文字列)
 *              opt2     … 次のの引数(文字列 ないし NULL)
 *              priority … 優先度 (値が大きいほど優先度が高い)
 *
 *      戻り値  1  … 処理した引数が1個 (opt1 のみ処理。 opt2 は未処理)
 *              2  … 処理した引数が2個 (opt1 と opt2 を処理)
 *              0  … opt1 が未知の引数のため、 opt1 opt2 ともに未処理
 *              -1 … 内部で致命的な異常が発生
 *
 *      処理中の異常 (引数の指定値が範囲外など) や、優先度により処理がスキップ
 *      されたような場合は、正しく処理できた場合と同様に、 1 か 2 を返す。
 *
 *      ※ この関数は、独自方式でオプションを解析するための関数なので、
 *         オプションテーブル T_CONFIG_TABLE を使用する場合は、ダミーでよい。
 *****************************************************************************/
int	xmame_config_check_option(char *opt1, char *opt2, int priority)
{
    return 0;
}


/******************************************************************************
 * サウンド系オプションを保存するための関数
 *
 * int  xmame_config_save_option(void (*real_write)
 *                                 (const char *opt_name, const char *opt_arg))
 *
 *      設定ファイルの保存の際に、呼び出される。
 *              「opt_name にオプションを、 opt_arg にオプション引数を
 *                セットし、real_write を呼び出す」
 *              という動作を、保存したい全オプションに対して繰り返し行なう。
 *
 *              (例) "-sound" を設定ファイルに保存したい場合
 *                      (real_write)("sound", NULL) を呼び出す。
 *              (例) "-fmvol 80" を設定ファイルに保存したい場合
 *                      (real_write)("fmvol", "80") を呼び出す。
 *
 *      戻り値  常に 0 を返す
 *
 *      ※ この関数は、独自方式でオプションを解析するための関数なので、
 *         オプションテーブル T_CONFIG_TABLE を使用する場合は、ダミーでよい。
 *****************************************************************************/
int	xmame_config_save_option(void (*real_write)
				   (const char *opt_name, const char *opt_arg))
{
    return 0;
}


/******************************************************************************
 * サウンド系オプションをメニューから変更するためのテーブル取得関数
 *
 * T_SNDDRV_CONFIG *xmame_config_get_sndopt_tbl(void)
 *
 *      メニューモードの開始時に呼び出される。
 *              変更可能なサウンド系オプションの情報を、T_SNDDRV_CONFIG 型の
 *              配列として用意し、その先頭ポインタを返す。
 *              配列は最大5個まで、さらに末尾には終端データをセットしておく。
 *
 *              特に変更したい／できるものが無い場合は NULL を返す。
 *****************************************************************************/
T_SNDDRV_CONFIG *xmame_config_get_sndopt_tbl(void)
{
    return NULL;
}


/******************************************************************************
 * サウンド機能の情報を取得する関数
 *
 * int xmame_has_audiodevice(void)
 *      サウンドオデバイス出力の可否を返す。
 *      真ならデバイス出力可。偽なら不可。
 *
 * int xmame_has_mastervolume(void)
 *      サウンドデバイスの音量が変更可能かどうかを返す。
 *      真なら変更可能。偽なら不可。
 *
 *****************************************************************************/
int	xmame_has_audiodevice(void)
{
    return FALSE;
}

int	xmame_has_mastervolume(void)
{
    return FALSE;
}


/*===========================================================================*/
/*              MAME の処理関数から呼び出される、システム依存処理関数        */
/*===========================================================================*/

/******************************************************************************
 * サウンドデバイス制御
 *      これらの関数は、グローバル変数 use_sound が偽の場合は、呼び出されない。
 *
 * int osd_start_audio_stream(int stereo)
 *      サウンドデバイスを初期化する。
 *          stereo が真ならステレオ出力、偽ならモノラル出力で初期化する。
 *          (モノラル出力は、古いバージョンの MAME/XMAME で YM2203 を処理する
 *           場合のみ、使用している。それ以外はすべてステレオ出力を使用する。)
 *      この関数は、エミュレーションの開始時に呼び出される。
 *          成功時は、1フレームあたりのサンプル数を返す。
 *          失敗時は、0 を返す。
 *              が、ここで 0 を返すと QUASI88 が強制終了してしまうので、
 *              サウンドデバイスの初期化に失敗した場合でも、サウンド出力なしで
 *              処理を進めたいという場合、とにかく適当な値を返す必要がある。
 *
 * int osd_update_audio_stream(INT16 *buffer)
 *      サウンドデバイスに出力する。
 *      この関数は、1フレーム処理毎に呼び出される。buffer には1フレーム分
 *      (Machine->sample_rate / Machine->drv->frames_per_second) のサウンド
 *      データが格納されている。データは 16bit符号付きで、ステレオの場合は
 *      左と右のサンプルが交互に並んでいる。
 *
 *      実際にこの関数が呼び出されたタイミングでデバイスに出力するか、あるいは
 *      内部でバッファリングして別途出力するかは、実装次第である。
 *
 *      戻値は、 osd_start_audio_stream() と同じ
 *
 * void osd_stop_audio_stream(void)
 *      サウンドデバイスを終了する。
 *      この関数は、エミュレーションの終了時に呼び出される。
 *      以降、 osd_update_audio_stream() などは呼び出されない。エミュレーション
 *      を再開する場合は、 osd_start_audio_stream() から呼び出される。
 *
 * void osd_update_video_and_audio(void)
 *      サウンドデバイスの出力その２
 *      タイミング的には、 osd_update_audio_stream() の直後に呼び出される。
 *      XMAME 0.106 に合わせて定義されているが、 osd_update_audio_stream() が
 *      きちんと実装できれいれば、こちらの関数はダミーでよい。
 *
 * void osd_sound_enable(int enable)
 *      サウンドデバイスへの出力あり・なしを設定する。
 *          enable が真なら出力あり、偽なら出力なし。
 *      この関数は、グローバル変数 close_device が真の場合のみ、呼び出される。
 *          メニューモードに入った際に、引数を偽として呼び出す。
 *          メニューモードから出る際に、引数を真として呼び出す。
 *      この関数は、引数が偽の場合に、サウンドデバイスを解放 (close) し、
 *      真の場合に確保 (open) するような実装を期待しているが、サウンドデバイス
 *      を常時確保したままにするような実装であれば、ダミーの関数でよい。
 *      なお、サウンドデバイスへの出力なしの場合も osd_update_audio_stream() 
 *      などの関数は呼び出される。
 *
 *****************************************************************************/
int	osd_start_audio_stream(int stereo)
{
    return 44100 / DEFAULT_VSYNC_FREQ_HZ;
}

int	osd_update_audio_stream(INT16 *buffer)
{
    return 44100 / DEFAULT_VSYNC_FREQ_HZ;
}

void	osd_stop_audio_stream(void)
{
}

void	osd_update_video_and_audio(void)
{
}

void	osd_sound_enable(int enable)
{
}


/******************************************************************************
 * 音量制御
 *
 * void osd_set_mastervolume(int attenuation)
 *      サウンドデバイスの音量を設定する。 attenuation は 音量で、 -32〜0 
 *      (単位は db)。 音量変更のできないデバイスであれば、ダミーでよい。
 *
 * int osd_get_mastervolume(void)
 *      現在のサウンドデバイスの音量を取得する。 戻値は -32〜0 (単位は db)。
 *      音量変更のできないデバイスであれば、ダミーでよい。
 *
 *****************************************************************************/
void	osd_set_mastervolume(int attenuation)
{
}

int	osd_get_mastervolume(void)
{
    return VOL_MIN;
}

#endif	/* USE_SOUND */
