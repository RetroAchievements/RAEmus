/*
 * MAME の サウンドドライバに必要な宣言のよせあつめ
 */

#ifndef MAME_QUASI88_H_INCLUDED
#define MAME_QUASI88_H_INCLUDED


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "quasi88.h"
#include "snddrv.h"
#include "z80.h"
#include "pc88cpu.h"
#include "intr.h"
#include "initval.h"
#include "soundbd.h"
#include "file-op.h"



/* src/mame.mak
   src/rules.mak =========================================================== */

#define	HAS_YM2203			(1)
#define	HAS_YM2608			(1)
#define	HAS_SAMPLES			(1)

#define	HAS_BEEP88			(1)

#ifdef	USE_FMGEN
#define	HAS_FMGEN2203		(1)
#define	HAS_FMGEN2608		(1)
#endif



/* src/windows/osd_cpu.h
   src/unix/osd_cpu.h ====================================================== */

typedef signed   char      INT8;
typedef signed   short     INT16;
typedef signed   int       INT32;
typedef unsigned char      UINT8;
typedef unsigned short     UINT16;
typedef unsigned int       UINT32;



/* src/memory.h ============================================================= */

#ifdef __GNUC__
#if (__GNUC__ < 2) || ((__GNUC__ == 2) && (__GNUC_MINOR__ <= 7))
#define UNUSEDARG
#else
#ifdef	__cplusplus
#define UNUSEDARG
#else
#define UNUSEDARG __attribute__((__unused__))
#endif
#endif
#else
#define UNUSEDARG
#endif


/* ----- typedefs for data and offset types ----- */
typedef UINT8			data8_t;
typedef UINT16			data16_t;
typedef UINT32			data32_t;
typedef UINT32			offs_t;

/* ----- typedefs for the various common memory/port handlers ----- */
typedef data8_t			(*read8_handler)  (UNUSEDARG offs_t offset);
typedef void			(*write8_handler) (UNUSEDARG offs_t offset, UNUSEDARG data8_t data);

/* ----- typedefs for the various common memory handlers ----- */
typedef read8_handler	mem_read_handler;
typedef write8_handler	mem_write_handler;

/* ----- typedefs for the various common port handlers ----- */
typedef read8_handler	port_read_handler;
typedef write8_handler	port_write_handler;

/* ----- macros for declaring the various common memory/port handlers ----- */
#define READ_HANDLER(name) 		data8_t  name(UNUSEDARG offs_t offset)
#define WRITE_HANDLER(name) 	void     name(UNUSEDARG offs_t offset, UNUSEDARG data8_t data)
#define READ16_HANDLER(name)	data16_t name(UNUSEDARG offs_t offset, UNUSEDARG data16_t mem_mask)
#define WRITE16_HANDLER(name)	void     name(UNUSEDARG offs_t offset, UNUSEDARG data16_t data, UNUSEDARG data16_t mem_mask)

/* ----- 16/32-bit memory accessing ----- */
#define COMBINE_DATA(varptr)		(*(varptr) = (*(varptr) & mem_mask) | (data & ~mem_mask))

/* ----- 16-bit memory accessing ----- */
#define ACCESSING_LSB				0
#define ACCESSING_MSB				0



/* src/osdepend.h ========================================================== */

/* The Win32 port requires this constant for variable arg routines. */
#ifndef CLIB_DECL
#define CLIB_DECL
#endif

void osd_update_video_and_audio(void);

int osd_start_audio_stream(int stereo);
int osd_update_audio_stream(INT16 *buffer);
void osd_stop_audio_stream(void);

void osd_set_mastervolume(int attenuation);
int osd_get_mastervolume(void);

void osd_sound_enable(int enable);


INLINE	void CLIB_DECL logerror(UNUSEDARG const char *text,...){}
/* #define logerror		(void)			*/
/* #define logerror		if(1){}else printf	*/



/* src/fileio.h ============================================================ */

#define	FILETYPE_SAMPLE		(4)

#define	mame_file	OSD_FILE

mame_file *mame_fopen(const char *gamename, const char *filename, int filetype, int openforwrite);

#define mame_fread(file, buffer, length)	osd_fread(buffer, 1, length, file)
#define mame_fwrite(file, buffer, length)
UINT32 mame_fread_swap(mame_file *file, void *buffer, UINT32 length);
#ifdef LSB_FIRST
#define mame_fread_msbfirst mame_fread_swap
#define mame_fread_lsbfirst mame_fread
#else
#define mame_fread_msbfirst mame_fread
#define mame_fread_lsbfirst mame_fread_swap
#endif
#define mame_fseek(file, offset, whence)	osd_fseek(file, offset, whence)
#define mame_fclose(file)					osd_fclose(file)



/* src/timer.h ============================================================= */

#define TIME_IN_HZ(hz)        (1.0 / (double)(hz))

#define TIME_NOW              (0.0)

typedef void		mame_timer;

#define	timer_alloc(callback)							(NULL)
#define	timer_adjust(which, duration, param, period)
#define	timer_set(duration, param, callback)
#define	timer_enable(which, enable)						(1)



/* src/cpuintrf.h ========================================================== */

#define	activecpu_get_pc()	0



/* src/common.h ============================================================ */

struct GameSample
{
	int length;
	int smpfreq;
	int resolution;
	signed char data[1];	/* extendable */
};


struct GameSamples
{
	int total;	/* total number of samples */
	struct GameSample *sample[1];	/* extendable */
};


struct GameSamples *readsamples(const char **samplenames,const char *name);


/* automatically-freeing memory */
#define	auto_malloc(size)		malloc(size)



/* src/profiler.h ========================================================== */

#define profiler_mark(type)



/* src/driver.h ============================================================ */

#include "sndintrf.h"
#include "samples.h"


struct InternalMachineDriver;
struct MachineSound *machine_add_sound(struct InternalMachineDriver *machine, const char *tag, int type, void *sndintf);


#define MAX_SOUND 5	/* MAX_SOUND is the maximum number of sound subsystems */
					/* which can run at the same time. Currently, 5 is enough. */

struct InternalMachineDriver
{
	float frames_per_second;

	UINT32 sound_attributes;
	struct MachineSound sound[MAX_SOUND];
};

/* ----- flags for sound_attributes ----- */
#define	SOUND_SUPPORTS_STEREO		0x0001



/* src/mame.h ============================================================== */

struct RunningMachine
{
	/* points to the constructed MachineDriver */
	const struct InternalMachineDriver *drv;

	/* the digital audio sample rate; 0 if sound is disabled. */
	int						sample_rate;

	/* samples loaded from disk */
	struct GameSamples *	samples;
};



struct GameOptions
{
	int		samplerate;		/* sound sample playback rate, in Hz */
	int		use_samples;	/* 1 to enable external .wav samples */
	int		use_filter;		/* 1 to enable FIR filter on final mixer output */
};


extern struct GameOptions options;
extern struct RunningMachine *Machine;



/****************************************************************************
 *	QUASI88
 *****************************************************************************/

extern	int use_sound;			/* 1:use sound / 0:not use */
extern	int close_device;		/* 1:close audio device at menu mode / 0:not */
extern	int fmvol;				/* level of FM     (0-100)[%] */
extern	int psgvol;				/* level of PSG    (0-100)[%] */
extern	int beepvol;			/* level of BEEP   (0-100)[%] */
extern	int rhythmvol;			/* level of RHYTHM (0-100)[%] depend on fmvol */
extern	int adpcmvol;			/* level of ADPCM  (0-100)[%] depend on fmvol */
extern	int fmgenvol;			/* level of fmgen  (0-100)[%] */
extern	int samplevol;			/* level of SAMPLE (0-100)[%] */
extern	int use_fmgen;			/* 1:use fmgen / 0:not use */
extern	int has_samples;		/* 1:use samples / 0:not use */


#define	XMAME_SNDDRV_071

#endif		/* MAME_QUASI88_H_INCLUDED */
