/*
 * XMAME の サウンドドライバに必要な宣言のよせあつめ
 */

#ifndef SNDDRV_SDL_H_INCLUDED
#define SNDDRV_SDL_H_INCLUDED


#undef	EXTERN
#ifdef	SNDDRV_WORK_DEFINE
#define EXTERN
#else
#define EXTERN extern
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "quasi88.h"
#include "snddrv.h"
#include "initval.h"
#include "soundbd.h"



/* src/unix/sysdep/sysdep_dsp_priv.h ======================================= */

struct sysdep_dsp_info {
   int samplerate;
   int type;
   int bufsize;
};

struct sysdep_dsp_struct {
   struct sysdep_dsp_info hw_info;
/* struct sysdep_dsp_info emu_info; */
   unsigned char *convert_buf;
/* uclock_t last_update; */
   void *_priv;
/* int (*get_freespace)(struct sysdep_dsp_struct *dsp); */
   int (*write)(struct sysdep_dsp_struct *dsp, unsigned char *data,
      int count);
   void (*destroy)(struct sysdep_dsp_struct *dsp);
};

struct sysdep_dsp_create_params {
   float bufsize;
   const char *device;
   int samplerate;
   int type;
   int flags;
};


/* src/unix/sysdep/sysdep_dsp.h ============================================ */

/* valid flags for type */
#define SYSDEP_DSP_8BIT   0x00
#define SYSDEP_DSP_16BIT  0x01
#define SYSDEP_DSP_MONO   0x00
#define SYSDEP_DSP_STEREO 0x02





/* QUASI88 ***************************************************************** */


extern void *sdl_dsp_create(const void *flags);

extern	int	sdl_buffersize;	/* audio buffer size (512..8192, power of 2) */





#endif		/* SNDDRV_SDL_H_INCLUDED */
