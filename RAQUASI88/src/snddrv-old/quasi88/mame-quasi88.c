/*
 * MAME と QUASI88 とのインターフェイス関数
 */

#include	<ctype.h>
#include	"mame-quasi88.h"

/*-------------------------------------------------------------------------*/

int use_sound		= TRUE;		/* 1:use sound / 0:not use */
int close_device	= FALSE;	/* 1:close audio device at menu mode / 0:not */
int fmvol			= 100;		/* level of FM    (0-100)[%] */
int psgvol			=  20;		/* level of PSG   (0-100)[%] */
int beepvol			=  60;		/* level of BEEP  (0-100)[%] */
int rhythmvol		= 100;		/* level of RHYTHM(0-100)[%] depend on fmvol */
int adpcmvol		= 100;		/* level of ADPCM (0-100)[%] depend on fmvol */
int fmgenvol		= 100;		/* level of fmgen (0-100)[%] */
int samplevol		=  50;		/* level of SAMPLE(0-100)[%] */
int use_fmgen		= FALSE;	/* 1:use fmgen / 0:not use */
int has_samples		= FALSE;	/* 1:use samples / 0:not use */

typedef struct {				/* list of mame-sound-I/F functions */

	int		(*sound_timer_over)(int n, int c);

	data8_t	(*sound_in_data)  (UNUSEDARG offs_t offset);
	data8_t	(*sound_in_status)(UNUSEDARG offs_t offset);
	void	(*sound_out_reg)  (UNUSEDARG offs_t offset, UNUSEDARG data8_t data);
	void	(*sound_out_data) (UNUSEDARG offs_t offset, UNUSEDARG data8_t data);

	data8_t	(*sound2_in_data)  (UNUSEDARG offs_t offset);
	data8_t	(*sound2_in_status)(UNUSEDARG offs_t offset);
	void	(*sound2_out_reg)  (UNUSEDARG offs_t offset, UNUSEDARG data8_t data);
	void	(*sound2_out_data) (UNUSEDARG offs_t offset, UNUSEDARG data8_t data);

	void	(*beep_out_data)   (UNUSEDARG offs_t offset, UNUSEDARG data8_t data);
	void	(*beep_out_ctrl)   (UNUSEDARG offs_t offset, UNUSEDARG data8_t data);

	void	(*sample_motoron)  (void);
	void	(*sample_motoroff) (void);
	void	(*sample_headdown) (void);
	void	(*sample_headup)   (void);
	void	(*sample_seek)     (void);

} T_XMAME_FUNC;

static T_XMAME_FUNC xmame_func_sound;
static T_XMAME_FUNC xmame_func_nosound =
{
	NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,
	NULL,  NULL,  NULL,  NULL,  NULL,  NULL,
};

static T_XMAME_FUNC *xmame_func = &xmame_func_nosound;


/****************************************************************
 * 起動時／終了時に呼ぶ
 ****************************************************************/

#include	"machine.c"

/*-------------------------------------------------------------------------*/

/* the active machine */
static struct RunningMachine active_machine;
struct RunningMachine *Machine = &active_machine;

/* the active game driver */
static struct InternalMachineDriver internal_drv;

/* various game options filled in by the OSD */
struct GameOptions options =
{
	22050,						/* サンプリングレート   8000 〜 48000 */
	0,							/* サンプル音使用可否   1:可 0:否     */
	0,							/* FIRフィルター???     1:可 0:否     */
};



/*	run_game()			[src/mame.c] */
static void	f_create_machine(void)
{
	/* first give the machine a good cleaning */
	memset(Machine, 0, sizeof(*Machine));

	/* initialize the driver-related variables in the Machine */
	Machine->drv = &internal_drv;

	/* expand_machine_driver() [src/mame.c] */
	{
		memset(&internal_drv, 0, sizeof(internal_drv));

#ifdef	USE_FMGEN
		if (use_fmgen) {
			if (sound_board == SOUND_I) {
				construct_quasi88fmgen(&internal_drv);
			} else {
				construct_quasi88fmgen2(&internal_drv);
			}
		}
		else
#endif
		{
			if (sound_board == SOUND_I) {
				construct_quasi88(&internal_drv);
			} else {
				construct_quasi88sd2(&internal_drv);
			}
		}
	}

	/* init_game_options()	[src/mame.c] */
	{
		/* initialize the samplerate */
		Machine->sample_rate = options.samplerate;
	}


	/* auto_malloc 関連をどうにかしたい */
}


static void	f_destroy_machine(void)
{
	/* auto_malloc 関連をどうにかしたい */
}





/****************************************************************
 * サウンドの開始
 * サウンドの更新
 * サウンドの出力
 * サウンドの終了
 * サウンドの中断
 * サウンドの再開
 * サウンドのリセット
 ****************************************************************/
int		xmame_sound_start(void)
{
	if (verbose_proc) printf("Initializing Sound System ... ");

	xmame_func = &xmame_func_nosound;

	if (use_sound == FALSE) {
		if (verbose_proc) printf("Canceled\n");
		return 1;
	}

#ifndef	USE_FMGEN
	if (use_fmgen) {
		if (verbose_proc) printf("\n(cisc's fmgen liblary not available)...");
		use_fmgen = 0;
	}
#endif

	if (verbose_proc) printf("\n");

	f_create_machine();

	/* 各音源の出力レベルを設定 (テーブル上の初期値を上書きすることで実現) */
	if (sound_board == SOUND_I) {
		if (use_fmgen == FALSE) {
			ym2203_interface.mixing_level[0] = YM2203_VOL(fmvol,psgvol);
		} else {
			ym2203_interface.mixing_level[0] = 
				YM3012_VOL(fmgenvol,MIXER_PAN_LEFT,fmgenvol,MIXER_PAN_RIGHT);
		}
	} else {
		if (use_fmgen == FALSE) {
			ym2608_interface.volumeSSG[0] = psgvol;
			ym2608_interface.volumeFM[0]  = 
				YM3012_VOL(fmvol,MIXER_PAN_LEFT,fmvol,MIXER_PAN_RIGHT);
		} else {
			ym2608_interface.volumeSSG[0] = psgvol;	/* not effect */
			ym2608_interface.volumeFM[0]  = 
				YM3012_VOL(fmgenvol,MIXER_PAN_LEFT,fmgenvol,MIXER_PAN_RIGHT);
		}
	}
	beep88_interface.mixing_level[0] = beepvol;

	if (has_samples) {
		quasi88_samples_interface.volume = samplevol;
	}


	/* ↓ 内部で osd_start_audio_stream() が呼び出される */
	if (sound_start() == 0) {

		xmame_func = &xmame_func_sound;

		if (verbose_proc) printf("Done\n");

		sound_reset();

		return 1;

	} else {

		/* 実は、ここには来ない */

		if (verbose_proc) printf("...FAILED, abort\n");

		return 0;
	}
}

void	xmame_sound_update(void)
{
	if (use_sound) {
		/* ↓ 内部で osd_update_audio_stream() が呼び出される */
		sound_update();
	}
}

void	xmame_update_video_and_audio(void)
{
	if (use_sound) {
		osd_update_video_and_audio();
	}
}

void	xmame_sound_stop(void)
{
	if (use_sound) {
		/* ↓ 内部で osd_stop_audio_stream() が呼び出される */
		sound_stop();

		f_destroy_machine();
	}
}

void	xmame_sound_suspend(void)
{
	if (use_sound) {
		if (close_device) {
			osd_sound_enable(0);
		}

		mixer_sound_enable_global_w(FALSE);
	}
}
void	xmame_sound_resume(void)
{
	if (use_sound) {
		if (close_device) {
			osd_sound_enable(1);
		}

		mixer_sound_enable_global_w(TRUE);
	}
}
void	xmame_sound_reset(void)
{
	if (use_sound) {
		sound_reset();
	}
}







/****************************************************************
 * サウンドポート入出力毎に呼ぶ
 ****************************************************************/
byte	xmame_dev_sound_in_data(void)
{
	if (xmame_func->sound_in_data) return (xmame_func->sound_in_data)(0);
	else                           return 0xff;
}
byte	xmame_dev_sound_in_status(void)
{
	if (xmame_func->sound_in_status) return (xmame_func->sound_in_status)(0);
	else                             return 0;
}
void	xmame_dev_sound_out_reg(byte data)
{
	if (xmame_func->sound_out_reg) (xmame_func->sound_out_reg)(0,data);
}
void	xmame_dev_sound_out_data(byte data)
{
	if (xmame_func->sound_out_data) (xmame_func->sound_out_data)(0,data);
}


byte	xmame_dev_sound2_in_data(void)
{
	if (use_sound) {
		if (sound_board == SOUND_I) return 0xff;
		else                        return 0;
	} else {
		return 0xff;
	}
}
byte	xmame_dev_sound2_in_status(void)
{
	if (xmame_func->sound2_in_status) return (xmame_func->sound2_in_status)(0);
	else                              return 0xff;
}
void	xmame_dev_sound2_out_reg(byte data)
{
	if (xmame_func->sound2_out_reg) (xmame_func->sound2_out_reg)(0,data);
}
void	xmame_dev_sound2_out_data(byte data)
{
	if (xmame_func->sound2_out_data) (xmame_func->sound2_out_data)(0,data);
}


void	xmame_dev_beep_out_data(byte data)
{
	if (xmame_func->beep_out_data) (xmame_func->beep_out_data)(0,data);
}
void	xmame_dev_beep_cmd_sing(byte flag)
{
	if (xmame_func->beep_out_ctrl) (xmame_func->beep_out_ctrl)(0,flag);
}


void	xmame_dev_sample_motoron(void)
{
	if (xmame_func->sample_motoron) (xmame_func->sample_motoron)();
}
void	xmame_dev_sample_motoroff(void)
{
	if (xmame_func->sample_motoroff) (xmame_func->sample_motoroff)();
}
void	xmame_dev_sample_headdown(void)
{
	if (xmame_func->sample_headdown) (xmame_func->sample_headdown)();
}
void	xmame_dev_sample_headup(void)
{
	if (xmame_func->sample_headup) (xmame_func->sample_headup)();
}
void	xmame_dev_sample_seek(void)
{
	if (xmame_func->sample_seek) (xmame_func->sample_seek)();
}





/****************************************************************
 * サウンドのタイマーオーバーフロー時に呼ぶ
 *		timer = 0 TimerAOver / 1 TimerBOver
 ****************************************************************/
void	xmame_dev_sound_timer_over(int timer)
{
	if (xmame_func->sound_timer_over) (xmame_func->sound_timer_over)(0, timer);
}




/****************************************************************
 * サウンド機能有無を取得
 *      真ならサウンドあり。偽なら無し。
 ****************************************************************/
int		xmame_has_sound(void)
{
    if (use_sound) return TRUE;
    else           return FALSE;
}

/****************************************************************
 * ボリューム取得
 *		現在の音量を取得する。範囲は、-32[db]〜0[db]
 ****************************************************************/
int		xmame_cfg_get_mastervolume(void)
{
	if (use_sound) {
		return osd_get_mastervolume();
	} else {
		return -32;
	}
}

/****************************************************************
 * ボリューム変更
 *		引数に、音量を与える。範囲は、-32[db]〜0[db]
 ****************************************************************/
void	xmame_cfg_set_mastervolume(int vol)
{
	if (use_sound) {
		if (vol > VOL_MAX) vol = VOL_MAX;
		if (vol < VOL_MIN) vol = VOL_MIN;
		osd_set_mastervolume(vol);
	}
}



/****************************************************************
 * 音源別レベル変更
 *		引数の、音源の種類とレベルを与える
 *		レベルは、    0〜100 まで
 *		音源の種類は、XMAME_MIXER_XXX だが、
 *					  これを mame 内部でのミキサーチャンネル番号に
 *					  変換しないといけない。
 *
 *	※ MAME 内部でのチャンネル
 *	    サウンドボードI		CH0〜2	PSG(ch1〜ch3)
 *							CH3		FM
 *							CH4		BEEP
 *							CH5〜9	SAMPLE
 *
 *	    サウンドボードII	CH0〜2  PSG(ch1〜3)
 *							CH3		FM(L)
 *							CH4		FM(R)
 *							CH5		BEEP
 *							CH6〜10	SAMPLE
 *
 *	    fmgen (I/IIとも)	CH0		PSG/FM(L)
 *							CH1		PSG/FM(R)
 *							CH2		BEEP
 *							CH3〜7	SAMPLE
 ****************************************************************/
void	xmame_cfg_set_mixer_volume(int ch, int level)
{
	if (use_sound) {
		switch (ch) {
		case XMAME_MIXER_PSG:
			if (level < PSGVOL_MIN) level = PSGVOL_MIN;
			if (level > PSGVOL_MAX) level = PSGVOL_MAX;
			if (use_fmgen == FALSE) {
				mixer_set_mixing_level(0, level);
				mixer_set_mixing_level(1, level);
				mixer_set_mixing_level(2, level);
			}
			psgvol = level;
			break;

		case XMAME_MIXER_FM:
			if (level < FMVOL_MIN) level = FMVOL_MIN;
			if (level > FMVOL_MAX) level = FMVOL_MAX;
			if (use_fmgen == FALSE) {
				if (sound_board == SOUND_I) {
					mixer_set_mixing_level(3, level);
				} else {
					mixer_set_mixing_level(3, level);
					mixer_set_mixing_level(4, level);
				}
			}
			fmvol = level;
			break;

		case XMAME_MIXER_BEEP:
			if (level < BEEPVOL_MIN) level = BEEPVOL_MIN;
			if (level > BEEPVOL_MAX) level = BEEPVOL_MAX;
			if (use_fmgen == FALSE) {
				if (sound_board == SOUND_I) {
					mixer_set_mixing_level(4, level);
				} else {
					mixer_set_mixing_level(5, level);
				}
			} else {
				mixer_set_mixing_level(2, level);
			}
			beepvol = level;
			break;

		case XMAME_MIXER_RHYTHM:
			if (level < RHYTHMVOL_MIN) level = RHYTHMVOL_MIN;
			if (level > RHYTHMVOL_MAX) level = RHYTHMVOL_MAX;
			rhythmvol = level;
			break;

		case XMAME_MIXER_ADPCM:
			if (level < ADPCMVOL_MIN) level = ADPCMVOL_MIN;
			if (level > ADPCMVOL_MAX) level = ADPCMVOL_MAX;
			adpcmvol = level;
			break;

		case XMAME_MIXER_FMGEN:
			if (level < FMGENVOL_MIN) level = FMGENVOL_MIN;
			if (level > FMGENVOL_MAX) level = FMGENVOL_MAX;
			if (use_fmgen) {
				mixer_set_mixing_level(0, level);
				mixer_set_mixing_level(1, level);
			}
			fmgenvol = level;
			break;

		case XMAME_MIXER_SAMPLE:
			if (level < SAMPLEVOL_MIN) level = SAMPLEVOL_MIN;
			if (level > SAMPLEVOL_MAX) level = SAMPLEVOL_MAX;
			if (has_samples) {
				if (use_fmgen == FALSE) {
					if (sound_board == SOUND_I) {
						mixer_set_mixing_level(5, level);
						mixer_set_mixing_level(6, level);
						mixer_set_mixing_level(7, level);
						mixer_set_mixing_level(8, level);
						mixer_set_mixing_level(9, level);
					} else {
						mixer_set_mixing_level(6, level);
						mixer_set_mixing_level(7, level);
						mixer_set_mixing_level(8, level);
						mixer_set_mixing_level(9, level);
						mixer_set_mixing_level(10,level);
					}
				} else {
					mixer_set_mixing_level(3, level);
					mixer_set_mixing_level(4, level);
					mixer_set_mixing_level(5, level);
					mixer_set_mixing_level(6, level);
					mixer_set_mixing_level(7, level);
				}
			}
			samplevol = level;
			break;

		default:
			/* モニター用。各ミキサーのレベルを表示 */
			for (ch=0; ch<MIXER_MAX_CHANNELS; ch++) {
				const char *name = mixer_get_name(ch);
				if (name) printf("%d[ch] %s\t:%d\n",
								 ch, name, mixer_get_mixing_level(ch));
			}
			break;
		}
	}
}

/****************************************************************
 * チャンネル別レベル取得 (レベルは、 0〜100)
 *		引数に、チャンネルを与える
 *		チャンネルは、XMAME_MIXER_XXX
 *
 *	※ MAME 内部から、取得したレベルを返す。
 *	   (素直に、ワークの値をそのまま返すほうがいいのでは？)
 ****************************************************************/
int		xmame_cfg_get_mixer_volume(int ch)
{
	switch (ch) {
	case XMAME_MIXER_PSG:
		if (use_fmgen == FALSE) {
			ch = 0;
		} else {
			return psgvol;
		}
		break;

	case XMAME_MIXER_FM:
		if (use_fmgen == FALSE) {
			ch = 3;
		} else {
			return fmvol;
		}
		break;

	case XMAME_MIXER_BEEP:
		if (use_fmgen == FALSE) {
			if (sound_board == SOUND_I) {
				ch = 4;
			} else {
				ch = 5;
			}
		} else {
			ch = 2;
		}
		break;

	case XMAME_MIXER_RHYTHM:
		return rhythmvol;

	case XMAME_MIXER_ADPCM:
		return adpcmvol;

	case XMAME_MIXER_FMGEN:
		if (use_fmgen) {
			ch = 0;
		} else {
			return fmgenvol;
		}
		break;

	case XMAME_MIXER_SAMPLE:
		if (use_fmgen == FALSE) {
			if (sound_board == SOUND_I) {
				ch = 5;
			} else {
				ch = 6;
			}
		} else {
			ch = 3;
		}
		break;

	default:
		return 0;
	}

	if (use_sound) {
		return mixer_get_mixing_level(ch);
	} else {
		return 0;
	}
}



/****************************************************************
 * fmgen 使用有無
 ****************************************************************/
int		xmame_cfg_get_use_fmgen(void)
{
#ifdef	USE_FMGEN
	return use_fmgen;
#else
	return FALSE;
#endif
}
int		xmame_cfg_set_use_fmgen(int enable)
{
#ifdef	USE_FMGEN
	use_fmgen = enable;
	return use_fmgen;
#else
	return FALSE;
#endif
}



/****************************************************************
 * サンプル周波数
 ****************************************************************/
int		xmame_cfg_get_sample_freq(void)
{
	if (use_sound) {
		return Machine->sample_rate;
	} else {
		return options.samplerate;
	}
}
int		xmame_cfg_set_sample_freq(int freq)
{
	if (8000 <= freq && freq <= 48000) {
		options.samplerate = freq;
	} else {
		options.samplerate = 44100;
	}

	return options.samplerate;
}



/****************************************************************
 * サンプル音の使用有無
 ****************************************************************/
int		xmame_cfg_get_use_samples(void)
{
	return options.use_samples;
}
int		xmame_cfg_set_use_samples(int enable)
{
	if (enable) options.use_samples = 1;
	else        options.use_samples = 0;

	return options.use_samples;
}



/****************************************************************
 * WAVファイル出力
 ****************************************************************/
int		xmame_wavout_open(const char *filename)
{
	if (use_sound) {
		return sound_wavfile_open(filename);
	} else {
		return FALSE;
	}
}
int		xmame_wavout_opened(void)
{
	if (use_sound) {
		return sound_wavfile_opened();
	} else {
		return FALSE;
	}
}
void	xmame_wavout_close(void)
{
	if (use_sound) {
		sound_wavfile_close();
	}
}
int		xmame_wavout_damaged(void)
{
	if (use_sound) {
		return sound_wavfile_damaged();
	} else {
		return FALSE;
	}
}



/****************************************************************
 * MAMEバージョン取得関数
 ****************************************************************/
const char *xmame_version_mame(void)
{
	return "   Based on MAME 0.71/XMAME 0.71.1";
}
const char *xmame_version_fmgen(void)
{
#ifdef	USE_FMGEN
	return "   Based on fmgen008-current";
#else
	return "";
#endif
}



/****************************************************************
 * ファイル入出力関数
 ****************************************************************/

/*	mame_fopen()			[src/fileio.c] */
mame_file *mame_fopen(const char *gamename, const char *filename, int filetype, int openforwrite)
{
	OSD_FILE *file = NULL;
	char buf[1024] = "";
	char wavfile[64];
	const char *dir = osd_dir_rom();

	if ((filetype != FILETYPE_SAMPLE) ||
		(openforwrite)) {
		return NULL;
	}


	if (dir == NULL) {
		return NULL;
	}
	if (sizeof(wavfile)-1 < strlen(filename)) {
		return NULL;
	}

	strcpy( wavfile, filename );
	if (osd_path_join( dir, wavfile, buf, 1024 )) {
		file = osd_fopen( FTYPE_ROM, buf, "rb" );
	}

	if (file == NULL) {		/* 開けなかったら、小文字のファイル名を試す */
		char *p = wavfile;
		for (; *p; p++) { *p = tolower(*p); }

		if (osd_path_join( dir, wavfile, buf, 1024 )) {
			file = osd_fopen( FTYPE_ROM, buf, "rb" );
		}
	}

	if (file == NULL) {		/* 開けなかったら、大文字のファイル名を試す */
		char *p = wavfile;
		for (; *p; p++) { *p = toupper(*p); }

		if (osd_path_join( dir, wavfile, buf, 1024 )) {
			file = osd_fopen( FTYPE_ROM, buf, "rb" );
		}
	}

	return file;
}
