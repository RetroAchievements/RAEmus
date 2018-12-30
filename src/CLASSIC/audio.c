/***********************************************************************
 * サウンド出力処理 (システム依存)
 *
 *      詳細は、 snddrv.h / mame-quasi88.h 参照
 ************************************************************************/

/*----------------------------------------------------------------------*
 * Classicバージョンのソースコードの大部分は、                          *
 * Koichi NISHIDA 氏の Classic iP6 PC-6001/mk2/6601 emulator のソースを *
 * 参考にさせていただきました。                                         *
 *                                                   (c) Koichi NISHIDA *
 *                                       based on MESS/MAME source code *
 *----------------------------------------------------------------------*/


#include <OSUtils.h>
#include <Sound.h>
#include <Timer.h>
#include <QuickTime.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#ifdef	USE_SOUND

#include "mame-quasi88.h"

#include "snddrv.h"
static	int use_audiodevice = 1;	/* use audio-devide for audio output */


static Boolean InitializeSound(void);
static void TearDownSound(void);
static void PauseSound(Boolean inPause);
static Boolean isFullSoundStream(int dataNum);
static Boolean isLessSoundStream(int dataNum, long tick);


static Boolean alreadyInit = false;

#define	REFRESH_RATE	60.0
#define	CHANNELS		sNumOfChannles

/*----------------------------------------------------------------------*/
int		attenuation = 0;		/* ボリューム -32〜0 [db] */

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
int		xmame_config_init(void)
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
static	int	invalid_arg;			/* 無効なオプション用のダミー変数 */
static	const	T_CONFIG_TABLE xmame_options[] =
{
  /* 350〜399: サウンド依存オプション */

  { 351, "sound",        X_FIX,  &use_sound,       TRUE,                  0,0, OPT_SAVE },
  { 351, "nosound",      X_FIX,  &use_sound,       FALSE,                 0,0, OPT_SAVE },

  { 352, "audio",        X_FIX,  &use_audiodevice, TRUE,                  0,0, OPT_SAVE },
  { 352, "noaudio",      X_FIX,  &use_audiodevice, FALSE,                 0,0, OPT_SAVE },

  { 353, "fmgen",        X_FIX,  &use_fmgen,       TRUE,                  0,0, OPT_SAVE },
  { 353, "nofmgen",      X_FIX,  &use_fmgen,       FALSE,                 0,0, OPT_SAVE },

  { 354, "volume",       X_INT,  &attenuation,     -32, 0,                  0, OPT_SAVE },

  { 355, "fmvol",        X_INT,  &fmvol,           0, 100,                  0, OPT_SAVE },
  { 356, "psgvol",       X_INT,  &psgvol,          0, 100,                  0, OPT_SAVE },
  { 357, "beepvol",      X_INT,  &beepvol,         0, 100,                  0, OPT_SAVE },
  { 358, "rhythmvol",    X_INT,  &rhythmvol,       0, 200,                  0, OPT_SAVE },
  { 359, "adpcmvol",     X_INT,  &adpcmvol,        0, 200,                  0, OPT_SAVE },
  { 360, "fmgenvol",     X_INT,  &fmgenvol,        0, 100,                  0, OPT_SAVE },
  { 361, "samplevol",    X_INT,  &samplevol,       0, 100,                  0, OPT_SAVE },

  { 362, "samplefreq",   X_INT,  &options.samplerate, 8000, 44100,          0, OPT_SAVE },

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
  "    -volume <n>             Set volume to <n> db, (-32 (soft) - 0(loud)) [0]\n"
  "    -fmvol <level>          Set FM     level to <level> %%, (0 - 100) [100]\n"
  "    -psgvol <level>         Set PSG    level to <level> %%, (0 - 100) [20]\n"
  "    -beepvol <level>        Set BEEP   level to <level> %%, (0 - 100) [60]\n"
  "    -rhythmvol <level>      Set RHYTHM level to <level> %%, (0 - 100) [100]\n"
  "    -adpcmvol <level>       Set ADPCM  level to <level> %%, (0 - 100) [100]\n"
  "    -fmgenvol <level>       Set fmgen  level to <level> %%, (0 - 100) [100]\n"
  "    -samplevol <level>      Set SAMPLE level to <level> %%, (0 - 100) [100]\n"
  "    -samplefreq <rate>      Set the playback sample-frequency/rate [44100]\n"
  "    -[no]samples            Use/don't use samples (if available) [-nosamples]\n"
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
int		xmame_config_check_option(char *opt1, char *opt2, int priority)
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
int		xmame_config_save_option(void (*real_write)
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
int		xmame_has_audiodevice(void)
{
    if (use_audiodevice) return TRUE;
    else                 return FALSE;
}

int		xmame_has_mastervolume(void)
{
	return TRUE;
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

static int sound_samples_per_frame = 0;


// C string <> Pascal string conversion
static void CopyCStringToPascal(const char *src, unsigned char *dst)
{
	int num = 0;
	int i;
	const char *srcBak = src;

	while (*(src++) != '\0') {
		num++;
	}
	src = srcBak;
	dst[0] = num;
	for (i=0; i<num; i++) {
		dst[i+1]=*(src++);
	}
}
static void CopyPascalStringToC(const unsigned char *src, char *dst)
{
	int i;
	for (i=0; i < src[0]; i++) {
		*(dst++) = src[i+1];
	}
	*(dst) = '\0';
}



// constants
enum
{
	kSoundStreamFramesAt44kHz	= 12288*1,
	kTotalSoundBuffers			= 2,
	kQueuedSoundBuffers			= 2
};

// our buffered data
static SInt16	sSoundStream[kSoundStreamFramesAt44kHz * 2];
static UInt32	sSoundStreamFrames;
static UInt32	sSoundIn;
static UInt32	sSoundOut;

// the sound channel
static SndChannelPtr sSoundChannel;

// sound timing
ComponentInstance 	gSoundClock;

// sound buffer parameters
static UInt32			sSoundBufferFrames;
static UInt32			sSoundBufferBytes;
static Boolean			sSoundIsPaused;

// sound buffers
static SndCallBackUPP	sSoundCallback;
static ExtSoundHeader	*sSoundBuffer[kTotalSoundBuffers];
static SndCommand		sSoundBufferCmd[kTotalSoundBuffers];
static SndCommand		sSoundCallbackCmd[kTotalSoundBuffers];

static int				sNumOfChannles = 2;

// function prototypes
static pascal void SoundCallback(SndChannelPtr inChannel, SndCommand *inCommand);
static void FillSoundBuffer(SInt16 *inBuffer);
static Boolean InitializeSoundChannel(void);
static Boolean InitializeSoundBuffers(void);
static void FreeSoundBuffers(void);

//#pragma segment SegMain

static void memcpyW(register void *dst, register void *src, register unsigned long num)
{
	register int i;
	if (num&1) {
		for (i = 0; i < num; i++)
			*(((short *)dst)++) = *(((short *)src)++);
	} else {
		for (i = 0; i < num>>1; i++)
			*(((long *)dst)++) = *(((long *)src)++);
	}
}

// examine whether the sSoundStreamFrames is full or not
static Boolean isFullSoundStream(int dataNum)
{
	UInt32		soundIn = sSoundIn;
	Boolean		updateOk = false;

	// check	
	if (soundIn < sSoundOut) {
		if ((soundIn + dataNum) <= sSoundOut) updateOk = true;
	} else if ((soundIn + dataNum) > sSoundStreamFrames) {
		if ((soundIn + dataNum - sSoundStreamFrames) <= sSoundOut) updateOk = true;
	} else updateOk = true;
	if (!updateOk) return true;
	else return false;
}

// examine whether the sSoundStreamFrames will be empty soon or not
static Boolean isLessSoundStream(int dataNum, long tick)
{
	Boolean		updateOk = false;

	// check	
	if (sSoundOut < sSoundIn) {
		return sSoundIn - sSoundOut <= dataNum*tick;
	} else if (sSoundIn < sSoundOut) {
		return sSoundIn - (sSoundOut - sSoundStreamFrames) <= dataNum*tick;
	} else return true;
}

//#pragma segment SegInit

// start the audio system going
int osd_start_audio_stream(int stereo)
{
	CHANNELS = (stereo) ? 2 : 1;

	if (Machine->sample_rate < 8000) {
		Machine->sample_rate = 8000;
	} else if (Machine->sample_rate > 44100) {
		Machine->sample_rate = 44100;
	}

#ifdef	XMAME_SNDDRV_071

#if 0
	sound_samples_per_frame = (int)(Machine->sample_rate /
         Machine->drv->frames_per_second);
#else
	sound_samples_per_frame = (int)(Machine->sample_rate / REFRESH_RATE);
#endif

#else	/* ver 0.106 */

#if 0
	sound_samples_per_frame = Machine->sample_rate / Machine->refresh_rate;
#else
	sound_samples_per_frame = (int)(Machine->sample_rate / REFRESH_RATE);
#endif

#endif


	if (use_audiodevice == FALSE) { return sound_samples_per_frame; }

	if (alreadyInit) return sound_samples_per_frame;
	alreadyInit = true;

	InitializeSound();

	////////
	sSoundIsPaused = false;
	
	// reset the sound buffer
	sSoundIn = sSoundOut = 0;
	sSoundStreamFrames = kSoundStreamFramesAt44kHz * CHANNELS * Machine->sample_rate / 44100;

	// pick the appropriate sound buffer size
	sSoundBufferFrames = 512 * Machine->sample_rate / 44100;
	sSoundBufferBytes = sSoundBufferFrames << CHANNELS;
	
	// allocate the sound channel and initialize the buffering
	if (!InitializeSoundBuffers())
	{
		return 0;
	}
	
	// set the initial volume (-0 dB)
	osd_set_mastervolume(attenuation);
	////////

	osd_sound_enable(1);

	return sound_samples_per_frame;
}

//#pragma segment SegMain

// pushes data into the audio stream
int osd_update_audio_stream(INT16 *buffer)
{
	register UInt32	soundIn = sSoundIn;
	register UInt32	framesToCopy;
	int dataNum;
	INT16 *bufferX;

	static long prevTick = 0;
	register int times;

	if (use_audiodevice == FALSE) { return sound_samples_per_frame; }

#if 1
	{
		register long tick = TickCount() - prevTick;
		if (tick > 4) tick = 4;
		prevTick = TickCount();

		if (isFullSoundStream(sound_samples_per_frame * CHANNELS)) return sound_samples_per_frame;
		times = isLessSoundStream(sound_samples_per_frame * CHANNELS, tick+1) ? (tick+1) : 1;
	}
#else
	times = 1;
#endif

	for (; times; times--) {

		bufferX = buffer;
		dataNum = sound_samples_per_frame * CHANNELS;

		// copy up to the end of the stream buffer
		while (dataNum > 0)
		{
			// determine how many frames we can copy
			framesToCopy = sSoundStreamFrames - soundIn;
			if (framesToCopy > dataNum)
				framesToCopy = dataNum;

			// copy and count the samples
			memcpyW(&sSoundStream[soundIn], bufferX, framesToCopy);
			dataNum -= framesToCopy;
			bufferX += framesToCopy;

			// adjust the output pointer
			soundIn += framesToCopy;
			if (soundIn >= sSoundStreamFrames)
				soundIn -= sSoundStreamFrames;
		}
	}
	sSoundIn = soundIn;

	/* return the samples to play this next frame */
	return sound_samples_per_frame;
}

void osd_stop_audio_stream(void)
{
	if (use_audiodevice == FALSE) { return; }

	if (!alreadyInit) return;
	alreadyInit = false;

	osd_sound_enable(0);
	TearDownSound();
}

void osd_update_video_and_audio(void)
{
	/* nothing */
}

void osd_sound_enable(int enable_it)
{
	if (use_audiodevice == FALSE) { return; }

	PauseSound(!enable_it);
}

//#pragma segment SegInit

//	sets the main volume ( -Attenuation dB )
static int now_attenuation = 0;
void osd_set_mastervolume(int attenuation)
{
	float multiplier = 1.0;
	UInt32 volume;

	if (use_audiodevice == FALSE) { return; }

	now_attenuation = attenuation;

	// compute a multiplier from the original volume
	while (attenuation++ < 0)
		multiplier *= 1.0 / 1.122018454;	// = (10 ^ (1/20)) = 1dB

	// apply it
	volume = (UInt32)(multiplier * 0x100);
	if (volume > 0x100) volume = 0x100;

	{
		SndCommand mySndCmd;
		//OSErr myErr;

		mySndCmd.cmd = volumeCmd;
		mySndCmd.param1 = 0;		// unused with volumeCmd
		mySndCmd.param2 = (volume << 16) | volume;
		SndDoCommand(sSoundChannel, &mySndCmd, false);
	}
}

int osd_get_mastervolume (void)
{
	if (use_audiodevice == FALSE) { return -32; }

	return now_attenuation;
}

// allocates a sound channel and starts it running
// first this must be called ?
static Boolean InitializeSound(void)
{
	SndCommand 		command;
	OSErr			err;
	
	// attempt to initialize the sound buffers
	if (!InitializeSoundChannel()) return false;

	// nuke any existing clock references
	gSoundClock = NULL;

	// first turn on the clock
	command.cmd = clockComponentCmd;
	command.param1 = true;
	err = SndDoImmediate(sSoundChannel, &command);
	if (err != noErr)
		return false;
		
	// then get a component instance
	command.cmd = getClockComponentCmd;
	command.param2 = (SInt32)&gSoundClock;
	err = SndDoImmediate(sSoundChannel, &command);
	if (err != noErr)
	{
		sSoundChannel = NULL;
		return false;
	}
	return true;
}

// tearDownSound
static void TearDownSound(void)
{
	// free the channel
	if (sSoundChannel != NULL)
		SndDisposeChannel(sSoundChannel, true);
	sSoundChannel = NULL;

	// attempt to free the sound buffers
	FreeSoundBuffers();
}

// pauses/resumes sound output
static void PauseSound(Boolean inPause)
{
	sSoundIsPaused = inPause;
}

//#pragma segment SegMain

// common code to fill a sound buffer.
// called from call back
static void FillSoundBuffer(register SInt16 *inBuffer)
{
	register SInt32		framesAvailable;
	register UInt32		framesToCopy;

	if (sSoundIsPaused) {
		memset(inBuffer, 0, sSoundBufferFrames << CHANNELS);
		return;
	}
	
#if 1
	{
		static UInt16 skipCounter = 0;

		if (sSoundIn == sSoundOut) {
			if (skipCounter > 20) {
				memset(inBuffer, 0, sSoundBufferFrames << CHANNELS);
				return;
			}
			skipCounter ++;
		} else {
			skipCounter = 0;
		}
	}
#endif

	framesAvailable = sSoundIn - sSoundOut;

	// account for the circular buffer
	if (framesAvailable < 0)
		framesAvailable += sSoundStreamFrames;

	// if we have more than enough data, clip to the maximum per buffer
	if (framesAvailable >= sSoundBufferFrames * CHANNELS)
		framesAvailable = sSoundBufferFrames * CHANNELS;

	// copy up to the end of the stream buffer
	while (framesAvailable > 0)
	{
		// determine how many frames we can copy
		framesToCopy = sSoundStreamFrames - sSoundOut;
		if (framesToCopy > framesAvailable)
			framesToCopy = framesAvailable;

		// copy and count the samples
		memcpyW(inBuffer, &sSoundStream[sSoundOut], framesToCopy);
		
		framesAvailable -= framesToCopy;
		inBuffer += framesToCopy;

		// adjust the output pointer
		sSoundOut += framesToCopy;

		// wrap the output pointer
		if (sSoundOut >= sSoundStreamFrames)
			sSoundOut -= sSoundStreamFrames;
	}
}

//#pragma segment SegInit

// initialize the sound channel and any other one-shot data
static Boolean InitializeSoundChannel(void)
{
	OSErr err;
	long initOptions;

	// allocate a UPP callback
	sSoundCallback = NewSndCallBackUPP(SoundCallback);
	
	// now allocate the channel
	sSoundChannel = NULL;

    if (CHANNELS == 1) {
		initOptions = initMono   + initNoInterp + initNoDrop;
	} else {
		initOptions = initStereo + initNoInterp + initNoDrop;
	}
	err = SndNewChannel(&sSoundChannel, sampledSynth, initOptions, sSoundCallback);
	return (err == noErr);
}

// initialize the sound buffers and create the sound channel
static Boolean InitializeSoundBuffers(void)
{
	extended80 extFreq;
	int i;
	OSErr err;
	Handle itlHandle;               // The 'itl4' resource handle
	long offset, length;            // Offset-length of parts table
	NumFormatStringRec myFormatRec; // Canonical format record
	Str15 pRate;
	char cRate[16];
	
	sprintf(cRate, "%5d", Machine->sample_rate); 
	CopyCStringToPascal(cRate, pRate);
	GetIntlResourceTable (smCurrentScript, smNumberPartsTable, &itlHandle, &offset, &length);
	StringToFormatRec ("\p#####", (NumberParts *)(*itlHandle + offset), &myFormatRec);
	StringToExtended (pRate, &myFormatRec, (NumberParts *)(*itlHandle + offset), &extFreq);

	for (i = 0; i < kTotalSoundBuffers; i++)
	{
		// allocate and clear the sound buffer
		sSoundBuffer[i] = malloc(sizeof(ExtSoundHeader) + sSoundBufferBytes);
		// R. Nabet 0001212 : added error handling
		if (sSoundBuffer[i] == NULL)
		{
			while (--i >= 0)
				free(sSoundBuffer[i]);
			return false;
		}
		memset(sSoundBuffer[i], 0, sizeof(ExtSoundHeader) + sSoundBufferBytes);
		
		// intialize the buffer structures
		sSoundBuffer[i]->numChannels 	= CHANNELS;
		sSoundBuffer[i]->sampleRate 	= (UInt32)Machine->sample_rate << 16;
		sSoundBuffer[i]->encode 		= extSH;
		sSoundBuffer[i]->numFrames 		= sSoundBufferFrames;
		sSoundBuffer[i]->AIFFSampleRate = extFreq;
		sSoundBuffer[i]->sampleSize 	= 16;
		
		// initialize the sound commands
		sSoundBufferCmd[i].cmd 			= bufferCmd;
		sSoundBufferCmd[i].param2 		= (SInt32)sSoundBuffer[i];
		sSoundCallbackCmd[i].cmd 		= callBackCmd;
		sSoundCallbackCmd[i].param1 	= (i + kQueuedSoundBuffers) % kTotalSoundBuffers;
	}
	
	// start stuff playing
	for (i = 0; i < kQueuedSoundBuffers; i++)
	{
		err = SndDoCommand(sSoundChannel, &sSoundBufferCmd[i], true);
		if (err != noErr)
			return false;
		err = SndDoCommand(sSoundChannel, &sSoundCallbackCmd[i], true);
		if (err != noErr)
			return false;
	}
	return true;
}

// free the sound buffers and the sound channel
static void FreeSoundBuffers(void)
{
	UInt32 i;
	
	// free the buffers
	for (i = 0; i < kTotalSoundBuffers; i++)
	{
		if (sSoundBuffer[i] != NULL)
			free(sSoundBuffer[i]);
		sSoundBuffer[i] = NULL;
	}
}

//#pragma segment SegMain

// Callback command to stream data
static pascal void SoundCallback(SndChannelPtr inChannel, SndCommand *inCommand)
{
	UInt32		bufferIndex = inCommand->param1;

	// fill the sound buffer
	FillSoundBuffer((SInt16 *)sSoundBuffer[bufferIndex]->sampleArea);

	// queue up the commands

	SndDoCommand(sSoundChannel, &sSoundBufferCmd[bufferIndex], true);
	SndDoCommand(sSoundChannel, &sSoundCallbackCmd[bufferIndex], true);
}

#endif		/* USE_SOUND */
