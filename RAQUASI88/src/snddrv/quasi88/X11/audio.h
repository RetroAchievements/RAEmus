/*
 * XMAME の サウンドドライバに必要な宣言のよせあつめ
 */

#ifndef SNDDRV_X11_H_INCLUDED
#define SNDDRV_X11_H_INCLUDED


#undef	EXTERN
#ifdef	SNDDRV_WORK_DEFINE
#define EXTERN
#else
#define EXTERN extern
#endif

#ifdef openstep
#include <libc.h>
#include <math.h>
#endif /* openstep */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "quasi88.h"
#include "snddrv.h"
#include "initval.h"
#include "soundbd.h"


/* src/unix/xmame.h ======================================================== */

#include "sysdep/rc.h"
#include "sysdep/sysdep_sound_stream.h"


#define OSD_OK			(0)
#define OSD_NOT_OK		(1)


/* Used for the rc handling. */
EXTERN struct rc_struct *rc;

/* global variables and miscellaneous flags */
EXTERN struct sysdep_sound_stream_struct *sysdep_sound_stream;

/* File descripters for stdout / stderr redirection, without svgalib inter
   fering */
#ifndef	stderr_file
#define stderr_file stderr
#endif

/* option structs */
extern struct rc_option sound_opts[];



/* QUASI88 ***************************************************************** */

extern	int	osd_has_sound_mixer(void);			/* 1:Volume adjustable / 0:not */
extern	int	osd_has_audio_device(void);			/* 1:Use audio device / 0:not */



#undef EXTERN
#endif		/* SNDDRV_X11_H_INCLUDED */
