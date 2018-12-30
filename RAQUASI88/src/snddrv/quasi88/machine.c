
/* src/driver.h ============================================================== */

/* start/end tags for the machine driver */
#define MACHINE_DRIVER_START(game) 										\
	void construct_##game(machine_config *machine)						\
	{																	\
		sound_config *sound = NULL;										\

#define MACHINE_DRIVER_END 												\
	}																	\


/* core parameters */
#define MDRV_FRAMES_PER_SECOND(rate)									\
	machine->frames_per_second = (rate);								\


/* add/remove speakers */
#define MDRV_SPEAKER_ADD(tag, x, y, z)									\
	driver_add_speaker(machine, (tag), (x), (y), (z));					\

#define MDRV_SPEAKER_REMOVE(tag)										\
	driver_remove_speaker(machine, (tag));								\

#define MDRV_SPEAKER_STANDARD_MONO(tag)									\
	MDRV_SPEAKER_ADD(tag,   0.0F, 0.0F, 1.0F)							\

#define MDRV_SPEAKER_STANDARD_STEREO(tagl, tagr)						\
	MDRV_SPEAKER_ADD(tagl, -0.2F, 0.0F, 1.0F)							\
	MDRV_SPEAKER_ADD(tagr,  0.2F, 0.0F, 1.0F)							\


/* add/remove/replace sounds */
#define MDRV_SOUND_ADD_TAG(tag, type, clock)							\
	sound = driver_add_sound(machine, (tag), SOUND_##type, (clock));	\

#define MDRV_SOUND_ADD(type, clock)										\
	MDRV_SOUND_ADD_TAG(NULL, type, clock)								\

#define MDRV_SOUND_REMOVE(tag)											\
	driver_remove_sound(machine, tag);									\

#define MDRV_SOUND_MODIFY(tag)											\
	sound = driver_find_sound(machine, tag);							\
	if (sound)															\
		sound->routes = 0;												\

#define MDRV_SOUND_CONFIG(_config)										\
	if (sound)															\
		sound->config = &(_config);										\

#define MDRV_SOUND_REPLACE(tag, type, _clock)							\
	sound = driver_find_sound(machine, tag);							\
	if (sound)															\
	{																	\
		sound->sound_type = SOUND_##type;								\
		sound->clock = (_clock);										\
		sound->config = NULL;											\
		sound->routes = 0;												\
	}																	\

#define MDRV_SOUND_ROUTE(_output, _target, _gain)						\
	if (sound)															\
	{																	\
		sound->route[sound->routes].output = (_output);					\
		sound->route[sound->routes].target = (_target);					\
		sound->route[sound->routes].gain = (_gain);						\
		sound->routes++;												\
	}																	\



/* src/drivers/... ============================================================== */

#include "2203intf.h"
#include "2608intf.h"

#include "beepintf.h"
#include "samples.h"

#ifdef	USE_FMGEN
#include "2203fmgen.h"
#include "2608fmgen.h"
#endif



static struct YM2203interface ym2203_interface =
{
	0,		/* SSG port-A リード時に呼ばれる関数 */
	0,		/* SSG port-B リード時の呼ばれる関数 */
	0,		/* SSG port-A ライト時の呼ばれる関数 */
	0,		/* SSG port-B ライト時の呼ばれる関数 */
	0		/* 割込フラグが立った時に呼ばれる関数 */
};

static struct YM2608interface ym2608_interface =
{
	0,		/* SSG port-A リード時に呼ばれる関数 */
	0,		/* SSG port-B リード時の呼ばれる関数 */
	0,		/* SSG port-A ライト時の呼ばれる関数 */
	0,		/* SSG port-B ライト時の呼ばれる関数 */
	0,		/* 割込フラグが立った時に呼ばれる関数 */
	0		/* ADPCMのメモリ番号(QUASI88では未使用) */
};


/* サンプルファイルは、モノラルであれば、bit数、周波数は不問 */
enum {
	SAMPLE_NUM_MOTORON,
	SAMPLE_NUM_MOTOROFF,
	SAMPLE_NUM_HEADDOWN,
	SAMPLE_NUM_HEADUP,
	SAMPLE_NUM_SEEK
};
static const char *quasi88_sample_names[] =
{
	"motoron.wav",			/* サンプル番号 0 */
	"motoroff.wav",			/* サンプル番号 1 */
	"headdown.wav",			/* サンプル番号 2 */
	"headup.wav",			/* サンプル番号 3 */
	"seek.wav",				/* サンプル番号 4 */
	0       /* end of array */
};

static struct Samplesinterface quasi88_samples_interface =
{
	5,						/* 同時に発音するチャンネル数	*/
	quasi88_sample_names,	/* サンプルファイル名一覧		*/
	NULL					/* 初期化成功時に呼び出す関数	*/
};


static void SAMPLE_motoron(void)
{
/*	if (sample_loaded(SAMPLE_NUM_MOTORON))*/
		sample_start(0, SAMPLE_NUM_MOTORON,  0);
}
static void SAMPLE_motoroff(void)
{
/*	if (sample_loaded(SAMPLE_NUM_MOTOROFF))*/
		sample_start(1, SAMPLE_NUM_MOTOROFF, 0);
}
static void SAMPLE_headdown(void)
{
/*	if (sample_loaded(SAMPLE_NUM_HEADDOWN))*/
		sample_start(2, SAMPLE_NUM_HEADDOWN, 0);
}
static void SAMPLE_headup(void)
{
/*	if (sample_loaded(SAMPLE_NUM_HEADUP))*/
		sample_start(3, SAMPLE_NUM_HEADUP,   0);
}
static void SAMPLE_seek(void)
{
/*	if (sample_loaded(SAMPLE_NUM_SEEK))*/
		sample_start(4, SAMPLE_NUM_SEEK,     0);
}


static	T_XMAME_FUNC pc88_sound_func =
{
	YM2203_timer_over_0,
	YM2203_read_port_0_r,
	YM2203_status_port_0_r,
	YM2203_control_port_0_w,
	YM2203_write_port_0_w,
	NULL,
	NULL,
	NULL,
	NULL,
	BEEP88_write_port_0_w,
	BEEP88_control_port_0_w,
	SAMPLE_motoron,
	SAMPLE_motoroff,
	SAMPLE_headdown,
	SAMPLE_headup,
	SAMPLE_seek,
};

static	T_XMAME_FUNC pc88_sound2_func =
{
	YM2608_timer_over_0,
	YM2608_read_port_0_r,
	YM2608_status_port_0_A_r,
	YM2608_control_port_0_A_w,
	YM2608_data_port_0_A_w,
	NULL,
	YM2608_status_port_0_B_r,
	YM2608_control_port_0_B_w,
	YM2608_data_port_0_B_w,
	BEEP88_write_port_0_w,
	BEEP88_control_port_0_w,
	SAMPLE_motoron,
	SAMPLE_motoroff,
	SAMPLE_headdown,
	SAMPLE_headup,
	SAMPLE_seek,
};

#ifdef	USE_FMGEN
static	T_XMAME_FUNC pc88_fmgen_func =
{
	NULL,
	FMGEN2203_read_port_0_r,
	FMGEN2203_status_port_0_r,
	FMGEN2203_control_port_0_w,
	FMGEN2203_write_port_0_w,
	NULL,
	NULL,
	NULL,
	NULL,
	BEEP88_write_port_0_w,
	BEEP88_control_port_0_w,
	SAMPLE_motoron,
	SAMPLE_motoroff,
	SAMPLE_headdown,
	SAMPLE_headup,
	SAMPLE_seek,
};

static	T_XMAME_FUNC pc88_fmgen2_func =
{
	NULL,
	FMGEN2608_read_port_0_r,
	FMGEN2608_status_port_0_A_r,
	FMGEN2608_control_port_0_A_w,
	FMGEN2608_data_port_0_A_w,
	NULL,
	FMGEN2608_status_port_0_B_r,
	FMGEN2608_control_port_0_B_w,
	FMGEN2608_data_port_0_B_w,
	BEEP88_write_port_0_w,
	BEEP88_control_port_0_w,
	SAMPLE_motoron,
	SAMPLE_motoroff,
	SAMPLE_headdown,
	SAMPLE_headup,
	SAMPLE_seek,
};
#endif



#define SETUP_FUNC_AND_WORK(func)										\
	xmame_func_sound = func;											\
																		\
	if (options.use_samples) {											\
		has_samples = TRUE;												\
	} else {															\
		has_samples = FALSE;											\
		xmame_func_sound.sample_motoron  = NULL;						\
		xmame_func_sound.sample_motoroff = NULL;						\
		xmame_func_sound.sample_headdown = NULL;						\
		xmame_func_sound.sample_headup   = NULL;						\
		xmame_func_sound.sample_seek     = NULL;						\
	}																	\




/* void construct_quasi88(machine_config *machine) */
static MACHINE_DRIVER_START( quasi88 )

	/* basic machine hardware */
	MDRV_FRAMES_PER_SECOND((float)(vsync_freq_hz))

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD(YM2203, 4000000)
	MDRV_SOUND_CONFIG(ym2203_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left",  1.0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 1.0)
    /* 0 = PSG(1ch), 1 = PSG(2ch), 2 = PSG(3ch), 3 = FM */

	MDRV_SOUND_ADD(BEEP88, 2400)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left",  1.0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 1.0)
    /* 0 = BEEP */

	if (options.use_samples) {
		MDRV_SOUND_ADD(SAMPLES, 0)
		MDRV_SOUND_CONFIG(quasi88_samples_interface)
		MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left",  1.0)
		MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 1.0)
	    /* 0..4 = samples(1..5) */
	}

	SETUP_FUNC_AND_WORK(pc88_sound_func)

MACHINE_DRIVER_END



/* void construct_quasi88sd2(machine_config *machine) */
static MACHINE_DRIVER_START( quasi88sd2 )

	/* basic machine hardware */
	MDRV_FRAMES_PER_SECOND((float)(vsync_freq_hz))

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD(YM2608, 8000000)
	MDRV_SOUND_CONFIG(ym2608_interface)
	MDRV_SOUND_ROUTE(0, "left",  1.0)
	MDRV_SOUND_ROUTE(0, "right", 1.0)
	MDRV_SOUND_ROUTE(1, "left",  1.0)
	MDRV_SOUND_ROUTE(2, "right", 1.0)
    /* 0 = PSG, 1 = FM(L), 2 = FM(R) */

	MDRV_SOUND_ADD(BEEP88, 2400)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left",  1.0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 1.0)
    /* 0 = BEEP */

	if (options.use_samples) {
		MDRV_SOUND_ADD(SAMPLES, 0)
		MDRV_SOUND_CONFIG(quasi88_samples_interface)
		MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left",  1.0)
		MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 1.0)
	    /* 0..4 = samples(1..5) */
	}

	SETUP_FUNC_AND_WORK(pc88_sound2_func)

MACHINE_DRIVER_END



#ifdef	USE_FMGEN
/* void construct_quasi88fmgen(machine_config *machine) */
static MACHINE_DRIVER_START( quasi88fmgen )

	/* basic machine hardware */
	MDRV_FRAMES_PER_SECOND((float)(vsync_freq_hz))

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD(FMGEN2203, 4000000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left",  1.0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 1.0)
    /* 0 = PSG/FM(L), 1 = PSG/FM(R) */

	MDRV_SOUND_ADD(BEEP88, 2400)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left",  1.0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 1.0)
    /* 0 = BEEP */

	if (options.use_samples) {
		MDRV_SOUND_ADD(SAMPLES, 0)
		MDRV_SOUND_CONFIG(quasi88_samples_interface)
		MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left",  1.0)
		MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 1.0)
	    /* 0..4 = samples(1..5) */
	}

	SETUP_FUNC_AND_WORK(pc88_fmgen_func)

MACHINE_DRIVER_END



/* void construct_quasi88fmgen2(machine_config *machine) */
static MACHINE_DRIVER_START( quasi88fmgen2 )

	/* basic machine hardware */
	MDRV_FRAMES_PER_SECOND((float)(vsync_freq_hz))

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD(FMGEN2608, 8000000)
	MDRV_SOUND_ROUTE(0, "left",  1.0)
	MDRV_SOUND_ROUTE(1, "right", 1.0)
    /* 0 = PSG/FM(L), 1 = PSG/FM(R) */

	MDRV_SOUND_ADD(BEEP88, 2400)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left",  1.0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 1.0)
    /* 0 = BEEP */

	if (options.use_samples) {
		MDRV_SOUND_ADD(SAMPLES, 0)
		MDRV_SOUND_CONFIG(quasi88_samples_interface)
		MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left",  1.0)
		MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 1.0)
	    /* 0..4 = samples(1..5) */
	}

	SETUP_FUNC_AND_WORK(pc88_fmgen2_func)

MACHINE_DRIVER_END

#endif
