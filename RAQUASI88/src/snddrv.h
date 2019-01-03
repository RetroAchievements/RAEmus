#ifndef SNDDRV_H_INCLUDED
#define SNDDRV_H_INCLUDED


enum {
    XMAME_MIXER_PSG,
    XMAME_MIXER_FM,
    XMAME_MIXER_BEEP,
    XMAME_MIXER_RHYTHM,
    XMAME_MIXER_ADPCM,
    XMAME_MIXER_FMGEN,
    XMAME_MIXER_SAMPLE,
    XMAME_MIXER_END
};

#define	VOL_MAX		(0)
#define	FMVOL_MAX	(100)
#define	PSGVOL_MAX	(100)
#define	BEEPVOL_MAX	(100)
#define	RHYTHMVOL_MAX	(200)
#define	ADPCMVOL_MAX	(200)
#define	FMGENVOL_MAX	(100)
#define	SAMPLEVOL_MAX	(100)

#define	VOL_MIN		(-32)
#define	FMVOL_MIN	(0)
#define	PSGVOL_MIN	(0)
#define	BEEPVOL_MIN	(0)
#define	RHYTHMVOL_MIN	(0)
#define	ADPCMVOL_MIN	(0)
#define	FMGENVOL_MIN	(0)
#define	SAMPLEVOL_MIN	(0)



/* サウンド系オプションをメニューから変更するための情報テーブル		*/
/*	メニューでは、ここで設定した変数をエントリにて入力可能となる。	*/

typedef enum {
    SNDDRV_NULL = 0,			/* 終端マーク			*/
    SNDDRV_INT,				/* 変数 work は int型		*/
    SNDDRV_FLOAT,			/* 変数 work は float型		*/
} SNDDRV_TYPE;

typedef struct T_SNDDRV_CONFIG {
    int		type;			/* SNDDRV_TYPE型参照		*/
    char	*title;			/* メニューの見出し (ASCII)	*/
    void	*work;			/* 変更対象となる変数のポインタ	*/
    double	low;			/* 変更可能な範囲		*/
    double	high;			/*       (low <= *work <= high)	*/
} T_SNDDRV_CONFIG;



#ifdef	USE_SOUND

#include "getconf.h"
int	xmame_config_init(void);
void	xmame_config_exit(void);

const T_CONFIG_TABLE *xmame_config_get_opt_tbl(void);
void	xmame_config_show_option(void);

int	xmame_config_check_option(char *arg1, char *arg2, int priority);
int	xmame_config_save_option(void (*real_write)(const char *opt_name, const char *opt_arg));

T_SNDDRV_CONFIG *xmame_config_get_sndopt_tbl(void);

int	xmame_has_sound(void);
int	xmame_has_audiodevice(void);
int	xmame_has_mastervolume(void);



int	xmame_sound_start(void);
void	xmame_sound_update(void);
void	xmame_update_video_and_audio(void);
void	xmame_sound_stop(void);
void	xmame_sound_suspend(void);
void	xmame_sound_resume(void);
void	xmame_sound_reset(void);

byte	xmame_dev_sound_in_data(void);
byte	xmame_dev_sound_in_status(void);
void	xmame_dev_sound_out_reg(byte data);
void	xmame_dev_sound_out_data(byte data);
byte	xmame_dev_sound2_in_data(void);
byte	xmame_dev_sound2_in_status(void);
void	xmame_dev_sound2_out_reg(byte data);
void	xmame_dev_sound2_out_data(byte data);
void	xmame_dev_beep_out_data(byte data);
void	xmame_dev_beep_cmd_sing(byte flag);
void	xmame_dev_sample_motoron(void);
void	xmame_dev_sample_motoroff(void);
void	xmame_dev_sample_headdown(void);
void	xmame_dev_sample_headup(void);
void	xmame_dev_sample_seek(void);
void	xmame_dev_sound_timer_over(int timer);

int	xmame_cfg_get_mastervolume(void);
void	xmame_cfg_set_mastervolume(int vol);
int	xmame_cfg_get_mixer_volume(int ch);
void	xmame_cfg_set_mixer_volume(int ch, int level);
int	xmame_cfg_get_use_fmgen(void);
int	xmame_cfg_set_use_fmgen(int enable);
int	xmame_cfg_get_use_samples(void);
int	xmame_cfg_set_use_samples(int enable);
int	xmame_cfg_get_sample_freq(void);
int	xmame_cfg_set_sample_freq(int freq);

int	xmame_wavout_open(const char *filename);
int	xmame_wavout_opened(void);
void	xmame_wavout_close(void);
int	xmame_wavout_damaged(void);

const char *xmame_version_mame(void);
const char *xmame_version_fmgen(void);

#else


#define	xmame_config_init()			(TRUE)
#define	xmame_config_exit()

#define	xmame_config_get_opt_tbl()		(NULL)
#define	xmame_config_show_option()

#define	xmame_config_check_option(a1, a2, p)	(0)
#define	xmame_config_save_option(f)		(NULL)

#define	xmame_config_get_sndopt_tbl()		(NULL)

#define	xmame_has_sound()			(FALSE)
#define	xmame_has_audiodevice()			(FALSE)
#define	xmame_has_mastervolume()		(FALSE)



#define	xmame_sound_start()			(TRUE)
#define	xmame_sound_update()
#define	xmame_update_video_and_audio()
#define	xmame_sound_stop()
#define	xmame_sound_suspend()
#define	xmame_sound_resume()
#define	xmame_sound_reset()

#define	xmame_dev_sound_in_data()		(0xff)
#define	xmame_dev_sound_in_status()		(0x00)
#define	xmame_dev_sound_out_reg(d)
#define	xmame_dev_sound_out_data(d)
#define	xmame_dev_sound2_in_data()		(0xff)
#define	xmame_dev_sound2_in_status()		(0xff)
#define	xmame_dev_sound2_out_reg(d)
#define	xmame_dev_sound2_out_data(d)
#define	xmame_dev_beep_out_data(d)
#define	xmame_dev_beep_cmd_sing(f)
#define	xmame_dev_sample_motoron()
#define	xmame_dev_sample_motoroff()
#define	xmame_dev_sample_headdown()
#define	xmame_dev_sample_headup()
#define	xmame_dev_sample_seek()
#define	xmame_dev_sound_timer_over( t )

#define	xmame_cfg_get_mastervolume()		(0)
#define	xmame_cfg_set_mastervolume(v)
#define	xmame_cfg_get_mixer_volume(c)		(0)
#define	xmame_cfg_set_mixer_volume(c, l)
#define	xmame_cfg_get_use_fmgen()		(FALSE)
#define	xmame_cfg_set_use_fmgen(e)		(FALSE)
#define	xmame_cfg_get_use_samples()		(FALSE)
#define	xmame_cfg_set_use_samples(e)		(FALSE)
#define	xmame_cfg_get_sample_freq()		(44100)
#define	xmame_cfg_set_sample_freq(f)		(44100)

#define	xmame_wavout_open(f)			(FALSE)
#define	xmame_wavout_opened()			(FALSE)
#define	xmame_wavout_close()
#define	xmame_wavout_damaged()			(FALSE)

#define	xmame_version_mame()			""
#define	xmame_version_fmgen()			""

#endif


#endif	/* SNDDRV_H_INCLUDED */
