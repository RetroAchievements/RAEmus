/****************************************************************************
File    : timing.c
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Implementation of timer handling API
@(#) #BY# Richard Lawrence, Tomasz Szymankowski
@(#) #LM# 14.10.2003
*/

#include <stdio.h>
#include <windows.h>
#include <limits.h>
#include <crtdbg.h>
#include "WinConfig.h"
#include "atari800.h"
#include "macros.h"
#include "misc_win.h"
#include "timing.h"
#include "sound_win.h"
#include "debug.h"

#define SLEEP_TIME_IN_MS		3

/* Public objects */

struct TimerCtrl_t g_Timer =
{
	DEF_PAL_FREQUENCY,
	DEF_NTSC_FREQUENCY
};

/* Private objects */

static ULONG s_ulAtariHWNextTime = 0L;
static ULONG s_ulDeltaT          = 0L;
static BOOL  s_bTimerRollover    = FALSE;
static long  s_nSleepThreshold   = 0L;


/*========================================================
Function : Timer_Reset
=========================================================*/
/* #FN#
   Re-starts the timer an emulated Atari is based on */
void
/* #AS#
   Nothing */
Timer_Reset( void )
{
	LARGE_INTEGER lnTimeFreq;
	int nSysFreq = (Atari800_TV_PAL == Atari800_tv_mode ? g_Timer.nPalFreq : g_Timer.nNtscFreq);

	QueryPerformanceFrequency( &lnTimeFreq );
	s_ulDeltaT = lnTimeFreq.LowPart / (nSysFreq ? nSysFreq : 1);

	s_nSleepThreshold = MulDiv( lnTimeFreq.LowPart, SLEEP_TIME_IN_MS + 1, 1000L );
	Timer_Start(FALSE);

} /* #OF# Timer_Reset */

/*========================================================
Function : Timer_Start
=========================================================*/
/* #FN#
   Runs a timer that an emulated Atari is based on */
void
/* #AS#
   Nothing */
Timer_Start(
	BOOL bCheckRollover
)
{
	ULONG ulTimerLastVal = s_ulAtariHWNextTime;
	LARGE_INTEGER lnTime;

	QueryPerformanceCounter( &lnTime );
	s_ulAtariHWNextTime = lnTime.LowPart + s_ulDeltaT;

	if( bCheckRollover )
		s_bTimerRollover = (BOOL)(ulTimerLastVal > s_ulAtariHWNextTime);

} /* #OF# Timer_Start */

/*========================================================
Function : Timer_Examine
=========================================================*/
/* #FN#
   Checks if the timer resolution is enough for emulation */
BOOL
/* #AS#
   Nothing */
Timer_Examine( void )
{
	LARGE_INTEGER lnTimerRes;

	QueryPerformanceFrequency( &lnTimerRes ); 
	if( lnTimerRes.LowPart == 0 )
		return FALSE;

	return TRUE;

} /* #OF# Timer_Examine */

/*========================================================
Function : Timer_WaitForVBI
=========================================================*/
/* #FN#
   The main timing function that an emulated Atari is based on */
void
/* #AS#
   Nothing */
Timer_WaitForVBI( void )
{
	long  lSpareTicks;
	ULONG ulTimerLastVal = s_ulAtariHWNextTime;
	LARGE_INTEGER lnTicks;
	int nDelay = 50;

#ifdef _DEBUG
	DWORD   dwPlayCursor   = 0;
	DWORD   dwSaveCursor   = 0;
//	IDirectSoundBuffer_GetCurrentPosition( s_lpDSBuffer, &dwPlayCursor, &dwSaveCursor );
//	if (abs(dwSaveCursor-dwPlayCursor)<1000)_TRACE2("Sound cursor: %d, %d\n", dwPlayCursor, dwSaveCursor);
#endif _DEBUG

	QueryPerformanceCounter( &lnTicks );
	if( s_bTimerRollover )
	{
		while( lnTicks.LowPart > s_ulAtariHWNextTime && (s_ulAtariHWNextTime - lnTicks.LowPart < s_ulDeltaT) )
		{
			QueryPerformanceCounter( &lnTicks );
			lSpareTicks = ULONG_MAX - lnTicks.LowPart + 1;
			_ASSERT(lSpareTicks <= (long)s_ulDeltaT);
			if( lSpareTicks > s_nSleepThreshold )
			{
				SleepEx( SLEEP_TIME_IN_MS, TRUE );
				QueryPerformanceCounter( &lnTicks );
			}
		}
		s_bTimerRollover = FALSE;
	}
	lSpareTicks = (long)(s_ulAtariHWNextTime - lnTicks.LowPart);

	if( lSpareTicks > 0L )
	{
		if( !_IsFlagSet( g_Misc.ulState, MS_FULL_SPEED ) )
		{
			while( lSpareTicks > s_nSleepThreshold )
			{
				SleepEx( SLEEP_TIME_IN_MS, TRUE );
				QueryPerformanceCounter( &lnTicks );
				lSpareTicks = (long)(s_ulAtariHWNextTime - lnTicks.LowPart);
#ifdef _DEBUG
//				IDirectSoundBuffer_GetCurrentPosition( s_lpDSBuffer, &dwPlayCursor, &dwSaveCursor );
//				_TRACE3("Sound cursor: %d, %d, %d\n", dwPlayCursor, dwSaveCursor, Atari800_nframes);
#endif
			}
			while( s_ulAtariHWNextTime > lnTicks.LowPart && lnTicks.LowPart > ulTimerLastVal )
				QueryPerformanceCounter( &lnTicks );

			s_ulAtariHWNextTime += s_ulDeltaT;
		}
		else
		{
			if( lSpareTicks > (long)s_ulDeltaT )
				s_ulAtariHWNextTime = lnTicks.LowPart + s_ulDeltaT;
			else
				s_ulAtariHWNextTime += s_ulDeltaT;
		}
	}
	else
	{
		if( -lSpareTicks > (long)s_ulDeltaT )
			s_ulAtariHWNextTime = lnTicks.LowPart + 1;
		else
			s_ulAtariHWNextTime += s_ulDeltaT;
	}
	if( ulTimerLastVal > s_ulAtariHWNextTime )
		s_bTimerRollover = TRUE;

	if( _IsFlagSet( g_Sound.ulState, SS_DS_SOUND ) )
	{
		if( s_nFrameCnt > s_nFramesPerFrag )
		{
		
			/* SoundIsPaused indicates full speed mode in this case; if
			   the emulated Atari is paused, this routine is not invoked */
			if( !s_bSoundIsPaused )
			{
	//			_TRACE2("PrimaryThread.SndPlay_DSSound: s_nSaveFragNo: %d, s_nPlayFragNo: %d\n", s_nSaveFragNo, s_nPlayFragNo);

				/* There is spinlock used to synchronize the threads. We should
				   avoid this solution but it's rather fast and we don't really
				   need the interlocked functions or critical sections here */
				while( s_nSaveFragNo != s_nPlayFragNo && --nDelay )
					Sleep( 1 );
	#ifdef _DEBUG
				if( 0 == nDelay ) _TRACE0("!PrimaryThread.SndPlay_DSSound: Delay = 0!\n");
	#endif _DEBUG
			}
			/* The PlayFragNo indicator is incremented by a secondary thread so we can
			   synchronize the thread with primary thread's writes to a stream buffer */

			if( ++s_nSaveFragNo == s_nNumberOfFrags )
				s_nSaveFragNo = 0;

			s_pSaveCursor = &s_pSoundBuffer[ s_nSaveFragNo * s_dwFragSize ];

			s_dwFragPos  = 0;
			s_nUpdateCnt = 1;
			s_nFrameCnt  = 1;
		}
	}

} /* #OF# Timer_WaitForVBI */
