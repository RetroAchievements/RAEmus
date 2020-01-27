/****************************************************************************
File    : core.h
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# C-core methods and objects declarations
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 10.10.2003
*/

#ifndef __CORE_H__
#define __CORE_H__

#include "avisave.h"
#include "crc.h"
#include "directdraw.h"
#include "display_win.h"
#include "globals.h"
#include "hq.h"
#include "input_win.h"
#include "kaillera.h"
#include "misc_win.h"
#include "rdevice.h"
#include "registry.h"
#include "sound_win.h"
#include "timing.h"
#include "xfd2atr.h"

#include "macros.h"
#include "png.h"
#include "pngconf.h"
#include "zconf.h"
#include "zlib.h"

#ifdef WIN_NETWORK_GAMES
#include "kaillera.h"
#include "kailleraclient.h"
#endif

#endif /*__CORE_H__*/
