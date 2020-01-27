/****************************************************************************
File    : timing.h
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Timer public methods and objects prototypes
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 02.02.2001
*/

#ifndef __TIMING_H__
#define __TIMING_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Constants declarations */

#define DEF_PAL_FREQUENCY		50
#define DEF_NTSC_FREQUENCY		60

/* Exported methods */

BOOL Timer_Examine   ( void );
void Timer_Reset     ( void );
void Timer_Start     ( BOOL bCheckRollover );
void Timer_WaitForVBI( void );

/* Exported globals */

struct TimerCtrl_t
{
	int nPalFreq;	/* Frames per second for PAL  */
	int nNtscFreq;	/* Frames per second for NTSC */
};
extern struct TimerCtrl_t g_Timer;

#ifdef __cplusplus
}
#endif

#endif /*__TIMING_H__*/
