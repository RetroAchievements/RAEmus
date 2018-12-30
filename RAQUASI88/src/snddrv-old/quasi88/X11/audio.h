/*
 * MAME/XMAME の サウンドドライバとのインターフェイス
 */

#ifndef SNDDRV_X11_H_INCLUDED
#define SNDDRV_X11_H_INCLUDED


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


/* src/unix/xmame.h ======================================================== */

#include "sysdep/rc.h"
#include "sysdep/sound_stream.h"


#define OSD_OK			(0)
#define OSD_NOT_OK		(1)


/* global variables and miscellaneous flags */
EXTERN int		sound_enabled;
EXTERN struct sound_stream_struct *sound_stream;



/* QUASI88 ***************************************************************** */

extern	int	osd_has_sound_mixer(void);			/* 1:Volume adjustable / 0:not */



#undef EXTERN
#endif		/* SNDDRV_X11_H_INCLUDED */
