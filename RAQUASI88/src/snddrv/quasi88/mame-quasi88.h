/*
 * MAME の サウンドドライバに必要な宣言のよせあつめ
 */

#ifndef MAME_QUASI88_H_INCLUDED
#define MAME_QUASI88_H_INCLUDED


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "quasi88.h"
#include "snddrv.h"			/* VOL_MAX			*/
#include "z80.h"
#include "pc88cpu.h"		/* z80main_cpu		*/
#include "intr.h"			/* state_of_cpu		*/
#include "initval.h"		/* SOUND_I			*/
#include "soundbd.h"		/* sound_board		*/
#include "file-op.h"		/* OSD_FILE, ...	*/



/* src/mame.mak
   src/sound/sound.mak ===================================================== */

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

#ifdef _MSC_VER
#ifndef CLIB_DECL
#define CLIB_DECL
#endif
#endif

typedef unsigned char      UINT8;
typedef signed   char      INT8;

typedef unsigned short     UINT16;
typedef signed   short     INT16;

typedef unsigned int       UINT32;
typedef signed   int       INT32;

#if		defined(DENY_LONG_LONG)
/* can't use 64bit-int */
typedef double             UINT64;	/* avoid compiler error */
typedef double             INT64;   /* avoid compiler error */
#else
#ifdef _MSC_VER
typedef unsigned __int64   UINT64;
typedef signed __int64     INT64;
#else
typedef unsigned long long UINT64;
typedef signed   long long INT64;
#endif
#endif


#ifdef	macintosh	/* SC depend */
#define CLIB_DECL
#endif



/* src/mamecore.h ========================================================== */

/* Suppress warnings about redefining the macro 'PPC' on LinuxPPC. */
#ifdef PPC
#undef PPC
#endif


/* Some optimizations/warnings cleanups for GCC */
#if defined(__GNUC__) && (__GNUC__ >= 3)
#define ATTR_UNUSED				__attribute__((__unused__))
#define ATTR_NORETURN			__attribute__((noreturn))
#define ATTR_PRINTF(x,y)		__attribute__((format(printf, x, y)))
#define ATTR_MALLOC				__attribute__((malloc))
#define ATTR_PURE				__attribute__((pure))
#define ATTR_CONST				__attribute__((const))
#define UNEXPECTED(exp)			__builtin_expect((exp), 0)
#define TYPES_COMPATIBLE(a,b)	__builtin_types_compatible_p(a, b)
#define RESTRICT				__restrict__
#else
#define ATTR_UNUSED
#define ATTR_NORETURN
#define ATTR_PRINTF(x,y)
#define ATTR_MALLOC
#define ATTR_PURE
#define ATTR_CONST
#define UNEXPECTED(exp)			(exp)
#define TYPES_COMPATIBLE(a,b)	1
#define RESTRICT
#endif


/* And some MSVC optimizations/warnings */
#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#define DECL_NORETURN			__declspec(noreturn)
#else
#define DECL_NORETURN
#endif



/* genf is a type that can be used for function pointer casting in a way
   that doesn't confuse some compilers */
typedef void genf(void);


/* These are forward struct declarations that are used to break
   circular dependencies in the code */
typedef struct _running_machine running_machine;
typedef struct _machine_config machine_config;

#define	mame_file	OSD_FILE


/* stream_sample_t is used to represent a single sample in a sound stream */
typedef INT32 stream_sample_t;


/* Make sure we have a path separator (default to /) */
#define	PATH_SEPARATOR	""


/* Standard MAME assertion macros */
#undef assert
#undef assert_always

#define assert(x)
#define assert_always(x, msg) do { if (!(x)) fatalerror("Fatal error: %s (%s:%d)", msg, __FILE__, __LINE__); } while (0)



/* Macros for normalizing data into big or little endian formats */
#define FLIPENDIAN_INT16(x)	(((((UINT16) (x)) >> 8) | ((x) << 8)) & 0xffff)

#ifdef LSB_FIRST
#define LITTLE_ENDIANIZE_INT16(x)	(x)
#else
#define LITTLE_ENDIANIZE_INT16(x)	(FLIPENDIAN_INT16(x))
#endif /* LSB_FIRST */





/* Used by assert(), so definition here instead of mame.h */
DECL_NORETURN void CLIB_DECL fatalerror(const char *text,...) ATTR_PRINTF(1,2) ATTR_NORETURN;
/* INLINE	DECL_NORETURN void CLIB_DECL fatalerror(const char *text,...) { exit(-1); } */
/* #define fatalerror		exit(-1)			*/



/* src/memory.h ============================================================ */

/* ----- typedefs for data and offset types ----- */
typedef UINT32			offs_t;

/* ----- typedefs for the various common data access handlers ----- */
typedef UINT8			(*read8_handler)  (ATTR_UNUSED offs_t offset);
typedef void			(*write8_handler) (ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT8 data);

/* ----- macros for declaring the various common data access handlers ----- */
#define READ8_HANDLER(name) 	UINT8  name(ATTR_UNUSED offs_t offset)
#define WRITE8_HANDLER(name) 	void   name(ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT8 data)
#define READ16_HANDLER(name)	UINT16 name(ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT16 mem_mask)
#define WRITE16_HANDLER(name)	void   name(ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT16 data, ATTR_UNUSED UINT16 mem_mask)

/* ----- 16-bit memory accessing ----- */
#define ACCESSING_LSB16				((mem_mask & 0x00ff) == 0)
#define ACCESSING_MSB16				((mem_mask & 0xff00) == 0)
#define ACCESSING_LSB				ACCESSING_LSB16
#define ACCESSING_MSB				ACCESSING_MSB16



/* src/mame.h ============================================================== */

struct _running_machine
{
	const machine_config *	drv;				/* points to the constructed machine_config */

	float					refresh_rate;		/* current video refresh rate */
	int						sample_rate;		/* the digital audio sample rate */
};

typedef struct _global_options global_options;
struct _global_options
{
	int		samplerate;		/* sound sample playback rate, in Hz */
	int		use_samples;	/* 1 to enable external .wav samples */
};

extern global_options options;
extern running_machine *Machine;


/* get the current pause state */
#define mame_is_paused(dummy)		(quasi88_is_paused)


/* log to the standard error.log file */
/* void CLIB_DECL logerror(const char *text,...); */
INLINE	void CLIB_DECL logerror(ATTR_UNUSED const char *text,...){}
/* #define logerror		(void)			*/
/* #define logerror		if(1){}else printf	*/



/* src/restrack.h ========================================================== */

/* initialize the resource tracking system */
void init_resource_tracking(void);

/* tear down the resource tracking system */
void exit_resource_tracking(void);

/* begin tracking resources */
void begin_resource_tracking(void);

/* stop tracking resources and free everything since the last begin */
void end_resource_tracking(void);


/* allocate memory and fatalerror if there's a problem */
#define malloc_or_die(s)	_malloc_or_die(s, __FILE__, __LINE__)
void *_malloc_or_die(size_t size, const char *file, int line) ATTR_MALLOC;

/* allocate memory that will be freed at the next end_resource_tracking */
#define auto_malloc(s)		_auto_malloc(s, __FILE__, __LINE__)
void *_auto_malloc(size_t size, const char *file, int line) ATTR_MALLOC;

/* allocate memory and duplicate a string that will be freed at the next end_resource_tracking */
#define auto_strdup(s)		_auto_strdup(s, __FILE__, __LINE__)
char *_auto_strdup(const char *str, const char *file, int line) ATTR_MALLOC;



/* src/state.h ============================================================= */

#define state_save_register_item(_mod, _inst, _val)
#define state_save_register_item_array(_mod, _inst, _val)

#define	state_save_get_reg_count()							(0)
#define	state_save_register_func_postload_ptr(f,p)



/* src/driver.h ============================================================ */

#include "sndintrf.h"
#ifdef	QUASI88_CLASSIC
#include "sound-alias.h"
/* Classic Mac OS では 「sound.h」 がシステムのヘッダファイル名と被ってしまう
   ので、別名でしのぐことにしよう。
   具体的には、ビルド前に予め、ファイル 「src/snddrv/src/sound.h」 を
   リネームして、 「src/snddrv/src/sound-alias.h」 に変えておく。
   なお、インクルードパス中に、 sound.h というファイル名が存在すると、
   それだけでビルドに失敗するので注意 */
#else
#include "sound.h"
#endif

/* maxima */
#define MAX_SOUND		32
#define MAX_SPEAKER 	4

/* In mamecore.h: typedef struct _machine_config machine_config; */
struct _machine_config
{
	float				frames_per_second;			/* number of frames per second */

	sound_config		sound[MAX_SOUND];			/* array of sound chips in the system */
	speaker_config		speaker[MAX_SPEAKER];		/* array of speakers in the system */

	int					(*sound_start)(void);		/* one-time sound start callback */
	void				(*sound_reset)(void);		/* sound reset callback */
};


speaker_config *driver_add_speaker(machine_config *machine, const char *tag, float x, float y, float z);
speaker_config *driver_find_speaker(machine_config *machine, const char *tag);
void driver_remove_speaker(machine_config *machine, const char *tag);

sound_config *driver_add_sound(machine_config *machine, const char *tag, int type, int clock);
sound_config *driver_find_sound(machine_config *machine, const char *tag);
void driver_remove_sound(machine_config *machine, const char *tag);



/* src/timer.h ============================================================= */

/* opaque type for representing a timer */
typedef void mame_timer;

#define mame_timer_alloc(c)				(NULL)

#define timer_alloc_ptr(c,p)			(NULL)
#define timer_adjust_ptr(w,d,e)		
#define timer_enable(w,e)				(1)

#define	mame_timer_adjust(w,d,pa,pe)



/* src/osdepend.h ========================================================== */

void osd_update_video_and_audio(void);

int osd_start_audio_stream(int stereo);
int osd_update_audio_stream(INT16 *buffer);
void osd_stop_audio_stream(void);

void osd_set_mastervolume(int attenuation);
int osd_get_mastervolume(void);

void osd_sound_enable(int enable);



/* src/cpuintrf.h ========================================================== */

#define		activecpu_get_pc()					0



/* src/profiler.h ========================================================== */

#define	PROFILER_SOUND		(0)
#define profiler_mark(type)



/* src/osdcore.h ============================================================ */

#define OPEN_FLAG_READ			0x0001		/* open for read */
#define	FILERR_NONE				(0)

typedef	int	mame_file_error;



/* src/fileio.h ============================================================ */

#define	SEARCHPATH_SAMPLE	NULL

/* open a file in the given search path with the specified filename */
mame_file_error mame_fopen(const char *dummypath, char *filename, UINT32 dummyflags, mame_file **file);

#define mame_fclose(file)					osd_fclose(file)
#define mame_fseek(file, offset, whence)	osd_fseek(file, offset, whence)
#define mame_fread(file, buffer, length)	osd_fread(buffer, 1, length, file)

char *assemble_3_strings(const char *dummy1, const char *summy2, const char *s3);



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
extern	int quasi88_is_paused;	/* for mame_is_paused() */

#endif		/* MAME_QUASI88_H_INCLUDED */
