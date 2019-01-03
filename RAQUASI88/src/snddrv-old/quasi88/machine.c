
/* src/driver.h ============================================================== */

/* start/end tags for the machine driver */
#define MACHINE_DRIVER_START(game) 										\
	void construct_##game(struct InternalMachineDriver *machine)		\
	{																	\

#define MACHINE_DRIVER_END 												\
	}																	\


/* core parameters */
#define MDRV_FRAMES_PER_SECOND(rate)									\
	machine->frames_per_second = (rate);								\


/* core sound parameters */
#define MDRV_SOUND_ATTRIBUTES(flags)									\
	machine->sound_attributes = (flags);								\


/* add/remove/replace sounds */
#define MDRV_SOUND_ADD_TAG(tag, type, interface)						\
	machine_add_sound(machine, (tag), SOUND_##type, &(interface));		\

#define MDRV_SOUND_ADD(type, interface)									\
	MDRV_SOUND_ADD_TAG(NULL, type, interface)							\

#define MDRV_SOUND_REMOVE(tag)											\
	machine_remove_sound(machine, tag);									\

#define MDRV_SOUND_REPLACE(tag, type, interface)						\
	{																	\
		struct MachineSound *sound = machine_find_sound(machine, tag);	\
		if (sound)														\
		{																\
			sound->sound_type = SOUND_##type;							\
			sound->sound_interface = &(interface);						\
		}																\
	}																	\



/* src/drivers/... ============================================================== */

#include "2203intf.h"
#include "2608intf.h"

#include "beep.h"
#include "samples.h"

#ifdef	USE_FMGEN
#include "2203fmgen.h"
#include "2608fmgen.h"
#endif



static struct YM2203interface ym2203_interface =
{
	1,						/* num				1 chips			*/
	4000000,				/* baseclock		4.0 MHz			*/
	{ YM2203_VOL(10,15) },	/* mixing_level[]	FMVol, SSGVol	*//*OverWrite*/
	{ 0 },					/* portA read handler				*/
	{ 0 },					/* portB read handler				*/
	{ 0 },					/* portA write handler				*/
	{ 0 },					/* portB write handler				*/
	{ 0 }					/* IRQ handler						*/
};

static struct YM2608interface ym2608_interface =
{
	1,						/* num				1 chips			*/
	8000000,				/* baseclock		8.0 MHz			*/
	{ 10 },					/* volumeSSG[]		SSGVol			*//*OverWrite*/
	{ 0 },					/* portA read handler				*/
	{ 0 },					/* portB read handler				*/
	{ 0 },					/* portA write handler				*/
	{ 0 },					/* portB write handler				*/
	{ 0 },					/* IRQ handler						*/
	{ -1 /*dummy*/ },		/* Delta-T memory region ram/rom	*/
	{ YM3012_VOL(15,MIXER_PAN_LEFT,15,MIXER_PAN_RIGHT) }
							/* use YM3012_VOL macro				*//*OverWrite*/
};

static struct BEEP88interface beep88_interface =
{
	1,						/* num				1 chips			*/
	2400,					/* baseclock		2400 Hz			*/
	{ 10 },					/* mixing_level[]	BEEPVol			*/
	{ 0 },					/* port40 read handler				*/
	{ 0 }					/* port40 write handler				*/
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
	100,					/* volume */
	quasi88_sample_names	/* サンプルファイル名一覧		*/
};


static void SAMPLE_motoron(void)
{
	sample_start(0, SAMPLE_NUM_MOTORON,  0);
}
static void SAMPLE_motoroff(void)
{
	sample_start(1, SAMPLE_NUM_MOTOROFF, 0);
}
static void SAMPLE_headdown(void)
{
	sample_start(2, SAMPLE_NUM_HEADDOWN, 0);
}
static void SAMPLE_headup(void)
{
	sample_start(3, SAMPLE_NUM_HEADUP,   0);
}
static void SAMPLE_seek(void)
{
	sample_start(4, SAMPLE_NUM_SEEK,     0);
}


static	T_XMAME_FUNC pc88_sound_func =
{
	YM2203TimerOver,
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
	YM2608TimerOver,
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




/* void construct_quasi88(struct InternalMachineDriver *machine) */
static MACHINE_DRIVER_START( quasi88 )

	/* basic machine hardware */
	MDRV_FRAMES_PER_SECOND((float)(vsync_freq_hz))

	/* sound hardware */
	MDRV_SOUND_ADD(YM2203, ym2203_interface)
	MDRV_SOUND_ADD(BEEP88, beep88_interface)

	if (options.use_samples) {
		MDRV_SOUND_ADD(SAMPLES, quasi88_samples_interface)
	}

	SETUP_FUNC_AND_WORK(pc88_sound_func)

MACHINE_DRIVER_END



/* void construct_quasi88sd2(struct InternalMachineDriver *machine) */
static MACHINE_DRIVER_START( quasi88sd2 )

	/* basic machine hardware */
	MDRV_FRAMES_PER_SECOND((float)(vsync_freq_hz))

	/* sound hardware */
	MDRV_SOUND_ATTRIBUTES(SOUND_SUPPORTS_STEREO)
	MDRV_SOUND_ADD(YM2608, ym2608_interface)
	MDRV_SOUND_ADD(BEEP88, beep88_interface)

	if (options.use_samples) {
		MDRV_SOUND_ADD(SAMPLES, quasi88_samples_interface)
	}

	SETUP_FUNC_AND_WORK(pc88_sound2_func)

MACHINE_DRIVER_END



#ifdef	USE_FMGEN
/* void construct_quasi88fmgen(struct InternalMachineDriver *machine) */
static MACHINE_DRIVER_START( quasi88fmgen )

	/* basic machine hardware */
	MDRV_FRAMES_PER_SECOND((float)(vsync_freq_hz))

	/* sound hardware */
	MDRV_SOUND_ATTRIBUTES(SOUND_SUPPORTS_STEREO)
	MDRV_SOUND_ADD(FMGEN2203, ym2203_interface)
	MDRV_SOUND_ADD(BEEP88,    beep88_interface)

	if (options.use_samples) {
		MDRV_SOUND_ADD(SAMPLES, quasi88_samples_interface)
	}

	SETUP_FUNC_AND_WORK(pc88_fmgen_func)

MACHINE_DRIVER_END



/* void construct_quasi88fmgen2(struct InternalMachineDriver *machine) */
static MACHINE_DRIVER_START( quasi88fmgen2 )

	/* basic machine hardware */
	MDRV_FRAMES_PER_SECOND((float)(vsync_freq_hz))

	/* sound hardware */
	MDRV_SOUND_ATTRIBUTES(SOUND_SUPPORTS_STEREO)
	MDRV_SOUND_ADD(FMGEN2608, ym2608_interface)
	MDRV_SOUND_ADD(BEEP88,    beep88_interface)

	if (options.use_samples) {
		MDRV_SOUND_ADD(SAMPLES, quasi88_samples_interface)
	}

	SETUP_FUNC_AND_WORK(pc88_fmgen2_func)

MACHINE_DRIVER_END

#endif
