/****************************************************************************
File    : sound_win.c
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Implementation of sound handling API
@(#) #BY# Richard Lawrence, Tomasz Szymankowski
@(#) #LM# 09.11.2003
*/

#define DIRECTSOUND_VERSION		0x0500

#include <stdio.h>
#include <windows.h>
#include <mmsystem.h>
#include <limits.h>
#include <crtdbg.h>
#include <dsound.h>
#include "WinConfig.h"
#include "Resource.h"
#include "Helpers.h"
#include "Debug.h"
#include "atari800.h"
#include "globals.h"
#include "macros.h"
#include "registry.h"
#include "timing.h"
#include "avisave.h"
#include "display_win.h"
#include "misc_win.h"
#include "sound_win.h"


#define MM_NUMBER_OF_FRAGS			8
#define MM_FRAMES_PER_FRAG			2

#define DS_NUMBER_OF_FRAGS			4 /* Do not change the value! */
#define DS_FRAMES_PER_FRAG			2
/* TIP:
   Frames per fragment = sound latency in frames at present,
   because the interval between play and save cursors equals 1
*/
#define MM_PLAY_FRAG_NO				0
#define MM_SAVE_FRAG_NO				MM_PLAY_FRAG_NO
#define DS_PLAY_FRAG_NO				0
#define DS_SAVE_FRAG_NO				DS_PLAY_FRAG_NO

#define NUMBER_OF_EVENTS			2
#define POS_EVENT					0
#define END_EVENT					1

/* Public objects */

struct SoundCtrl_t g_Sound =
{
	DEF_SOUND_STATE,
	DEF_SOUND_RATE,
	DEF_SOUND_VOL,
	DEF_SKIP_UPDATE,
	DEF_SOUND_LATENCY,
	DEF_SOUND_QUALITY,
	DEF_SOUND_DIGITIZED,
	0,
	NULL
};

/* Private objects */

static WAVEHDR      s_arrWaveHDR[ MM_NUMBER_OF_FRAGS ];
static WAVEFORMATEX s_wfxWaveFormat;
static HWAVEOUT     s_hWaveOut              = 0;
char        *s_pSoundBuffer          = NULL;
static char        *s_pPlayCursor           = NULL;
char        *s_pSaveCursor           = NULL;
volatile int		s_nPlayFragNo           = MM_PLAY_FRAG_NO;
volatile int		s_nSaveFragNo           = MM_SAVE_FRAG_NO;
int          s_nNumberOfFrags        = MM_NUMBER_OF_FRAGS;
int          s_nFramesPerFrag        = MM_FRAMES_PER_FRAG;
int          s_nFrameCnt             = 1;
static int          s_nUpdatesPerFrag       = Atari800_TV_PAL * MM_FRAMES_PER_FRAG / DEF_SKIP_UPDATE;
int          s_nUpdateCnt            = 1;
static int          s_nSkipUpdate           = DEF_SKIP_UPDATE;
static DWORD        s_dwBufferSize          = 0;
DWORD        s_dwFragSize            = 0;
DWORD        s_dwFragPos             = 0;
static int          s_n16BitSnd             = 0; // 0 for 8-bit sound, 1 for 16-bit one
static UBYTE		s_nSilenceData8         = 0x80;
static UWORD		s_nSilenceData16		= 0x8000;
static DWORD        s_dwStartVolume         = 0;
BOOL         s_bSoundIsPaused        = TRUE;

static const GUID IID_IDirectSoundNotify = { 0xb0210783, 0x89cd, 0x11d0, { 0xaf, 0x08, 0x00, 0xa0, 0xc9, 0x25, 0xcd, 0x16 } };

static LPDIRECTSOUND       s_lpDirectSound  = NULL;
LPDIRECTSOUNDBUFFER s_lpDSBuffer     = NULL;
static LPDIRECTSOUNDNOTIFY s_lpDSNotify     = NULL;
static LPDSBPOSITIONNOTIFY s_lpDSNotifyPos  = NULL;
static HANDLE              s_hNotifyThread  = NULL;
static DWORD               s_dwThreadID     = 0;
static HANDLE              s_arrEvents[ NUMBER_OF_EVENTS ];

static void  SndPlay_NoSound( void );
static void  SndPlay_MMSound( void );
static void  SndPlay_DSSound( void );

static DWORD WINAPI DSSoundThreadProc( LPVOID lpParam );

void (*Atari_PlaySound)(void) = SndPlay_NoSound;

static BOOL DS_GetErrorString( HRESULT  hResult,  LPSTR pszErrorBuff, DWORD dwError );
static BOOL MM_GetErrorString( MMRESULT mmResult, LPSTR pszErrorBuff, DWORD dwError );

/* Macro-definitions */

#ifdef _DEBUG
#define ServeMMError( nUID, hResult, bQuit ) \
		ShowMMError( nUID, hResult, bQuit, __FILE__, __LINE__ )
#define ServeDSError( nUID, hResult, bQuit ) \
		ShowDSError( nUID, hResult, bQuit, __FILE__, __LINE__ )
#else /*_DEBUG*/
#define ServeMMError( nUID, hResult, bQuit ) \
		ShowMMError( nUID, hResult, bQuit )
#define ServeDSError( nUID, hResult, bQuit ) \
		ShowDSError( nUID, hResult, bQuit )
#endif /*_DEBUG*/

#define _PosEvent	s_arrEvents[ POS_EVENT ]
#define _EndEvent	s_arrEvents[ END_EVENT ]


/*========================================================
Function : ShowMMError
=========================================================*/
/* #FN#
   Displays a multimedia sound error description */
static
void
/* #AS#
   Nothing */
ShowMMError(
	UINT     nUID,
	MMRESULT mmResult,
	BOOL     bQuit
#ifdef _DEBUG
  , char   *pszFile,
	DWORD   dwLine
#endif /*_DEBUG*/
)
{
	char szError [ LOADSTRING_SIZE_S + 1 ];
	char szAction[ LOADSTRING_SIZE_L + 1 ];

#ifdef _DEBUG
	Log_print( "Multimedia Sound error: %s@%ld", pszFile, dwLine );
#endif /*_DEBUG*/

	/* Get us back to a GDI display and disable sound */
	Screen_SetSafeDisplay( FALSE );
	Sound_Disable( TRUE );

	/* Get the error string and present it to the user */
	MM_GetErrorString( mmResult, szError, LOADSTRING_SIZE_S );
	DisplayMessage( NULL, IDS_MMERR_PROMPT, IDS_MMERR_HDR, MB_ICONSTOP | MB_OK, _LoadStringLx( nUID, szAction ), szError );

	/* Start a quit (this will end up in Atari_Exit()) */
	if( bQuit )
	{
		/* Make sure the atari is turned off */
		g_ulAtariState = ATARI_UNINITIALIZED;
		PostMessage( g_hMainWnd, WM_CLOSE, 0, 0L );
	}
} /* #OF# ShowMMError */

/*========================================================
Function : ShowDSError
=========================================================*/
/* #FN#
   Displays a direct sound error description */
static
void
/* #AS#
   Nothing */
ShowDSError(
	UINT    nUID,
	HRESULT hResult,
	BOOL    bQuit
#ifdef _DEBUG
  , char   *pszFile,
	DWORD   dwLine
#endif /*_DEBUG*/
)
{
	char szError [ LOADSTRING_SIZE_S + 1 ];
	char szAction[ LOADSTRING_SIZE_L + 1 ];

#ifdef _DEBUG
	Log_print( "DirectSound error: %s@%ld", pszFile, dwLine );
#endif /*_DEBUG*/

	/* Get us back to a GDI display and disable sound */
	Screen_SetSafeDisplay( FALSE );
	Sound_Disable( TRUE );

	/* Get the error string and present it to the user */
	DS_GetErrorString( hResult, szError, LOADSTRING_SIZE_S );
	DisplayMessage( NULL, IDS_DSERR_PROMPT, IDS_DSERR_HDR, MB_ICONSTOP | MB_OK, _LoadStringLx( nUID, szAction ), szError );

	/* Start a quit (this will end up in Atari_Exit()) */
	if( bQuit )
	{
		/* Make sure the atari is turned off */
		g_ulAtariState = ATARI_UNINITIALIZED;
		PostMessage( g_hMainWnd, WM_CLOSE, 0, 0L );
	}
} /* #OF# ShowDSError */

/*========================================================
Function : Sound_GetInterParms
=========================================================*/
/* #FN#
   Returns sound information structure */
void
/* #AS#
   Nothing */
Sound_GetInterParms(
	struct SoundInterParms_t *pInfo
)
{
	/* Make copy of an used WAVEFORMATEX structure */ 
	if( pInfo->dwMask & SIP_WAVEFORMAT )
		CopyMemory( &pInfo->wfxFormat, &s_wfxWaveFormat, sizeof(WAVEFORMATEX) );

	/* Make copy of sample size value */ 
	if( pInfo->dwMask & SIP_SAMPLESIZE )
		pInfo->dwSampleSize = s_dwFragSize;

} /* #OF# Sound_GetInterParms */

/*========================================================
Function : DetermineHardwareCaps
=========================================================*/
/* #FN#
   Determines sound hardware capabilities */
static
BOOL
/* #AS#
   TRUE if succeeded, otherwise FALSE */
DetermineHardwareCaps(
	LPDIRECTSOUND lpDirectSound,
	BOOL          bPrimary
)
{
	DSCAPS  dscaps;
	char	szFailed[ (LOADSTRING_SIZE_L + 1) * 2 ];
	char	szError [ LOADSTRING_SIZE_M + 1 ];
	HRESULT hResult;

	dscaps.dwSize = sizeof(DSCAPS);
	*szFailed = '\0';

	hResult = IDirectSound_GetCaps( lpDirectSound, &dscaps );

	if( SUCCEEDED(hResult) )
	{
		if( bPrimary )
		{
			if( s_n16BitSnd )
			{
				if( !_IsFlagSet( dscaps.dwFlags, DSCAPS_PRIMARY16BIT ) )
					strcat( szFailed, _LoadStringMx( IDS_DSERR_NO_16BIT, szError ) );
			}
			else if( !_IsFlagSet( dscaps.dwFlags, DSCAPS_PRIMARY8BIT ) )
				strcat( szFailed, _LoadStringMx( IDS_DSERR_NO_8BIT, szError ) );

			if( POKEYSND_stereo_enabled )
			{
				if( !_IsFlagSet( dscaps.dwFlags, DSCAPS_PRIMARYSTEREO ) )
					strcat( szFailed, _LoadStringMx( IDS_DSERR_NO_STEREO, szError ) );
			}
			else if( !_IsFlagSet( dscaps.dwFlags, DSCAPS_PRIMARYMONO ) )
				strcat( szFailed, _LoadStringMx( IDS_DSERR_NO_MONO, szError ) );

		}
		if( _IsFlagSet( dscaps.dwFlags, DSCAPS_EMULDRIVER ) )
			strcat( szFailed, _LoadStringMx( IDS_DSERR_NO_DRIVER, szError ) );

		if( *szFailed )
		{
			Sound_Disable( TRUE );
			DisplayMessage( NULL, IDS_DSERR_INIT, IDS_DSERR_HDR, MB_ICONEXCLAMATION | MB_OK, szFailed );
			return FALSE;
		}
	}
	else
	{
		ServeDSError( IDS_DSERR_QUERY, hResult, FALSE );
		return FALSE;
	}
	return TRUE;

} /* #OF# DetermineHardwareCaps */

/*========================================================
Function : Sound_Initialise
=========================================================*/
/* #FN#
   Initializes the Waveout/DirectSound playback */
BOOL
/* #AS#
   TRUE if succeeded, otherwise FALSE */
Sound_Initialise(
	BOOL bClearRegs
)
{
	DWORD dwBufferSize    = s_dwBufferSize;
	DWORD dwFragSize      = 0;
	int   nNumberOfFrags  = 0;
	int   nFramesPerFrag  = 0;
	int   nUpdatesPerFrag = 0;
	int   n16BitSnd       = _IsFlagSet( g_Sound.ulState, SS_16BIT_AUDIO ) ? 1 : 0;
	int   nSkipUpdate     = g_Sound.nSkipUpdate;
	int   nChannels       = POKEYSND_stereo_enabled ? 2 : 1;
	int   i; /* Loop counter */

	_ASSERT(_IsFlagSet( g_Sound.ulState, SS_MM_SOUND | SS_DS_SOUND ));

	/* We have to clear sound resources first */
	if( _IsFlagSet( g_Sound.ulState, SS_MM_SOUND ) )
	{
		nNumberOfFrags = MM_NUMBER_OF_FRAGS;
		nFramesPerFrag = MM_FRAMES_PER_FRAG;
	}
	else
	if( _IsFlagSet( g_Sound.ulState, SS_DS_SOUND ) )
	{
		nNumberOfFrags = DS_NUMBER_OF_FRAGS;
		nFramesPerFrag = g_Sound.nLatency;//DS_FRAMES_PER_FRAG;

//		_ASSERT(2 == nNumberOfFrags);
	}
	/* Compute the values related to sound buffer size */
	if( Atari800_TV_PAL == Atari800_tv_mode )
	{
		dwFragSize = (g_Sound.nRate / g_Timer.nPalFreq) * nFramesPerFrag * nChannels * (n16BitSnd + 1);
		nUpdatesPerFrag = Atari800_TV_PAL * nFramesPerFrag / nSkipUpdate;
	}
	else
	{
		dwFragSize = (g_Sound.nRate / g_Timer.nNtscFreq) * nFramesPerFrag * nChannels * (n16BitSnd + 1);
		nUpdatesPerFrag = Atari800_TV_NTSC * nFramesPerFrag / nSkipUpdate;
	}
	/* If size of the sound buffer has not been changed, there is no need to realloc it */
	Sound_Clear( TRUE, !(nNumberOfFrags * dwFragSize == dwBufferSize) );

	s_dwFragSize      = dwFragSize;
	s_nNumberOfFrags  = nNumberOfFrags;
	s_nFramesPerFrag  = nFramesPerFrag;
	s_nUpdatesPerFrag = nUpdatesPerFrag;
	s_dwFragPos       = 0;
	s_n16BitSnd       = n16BitSnd;
	s_nSkipUpdate     = nSkipUpdate;
	s_nUpdateCnt      = 1;
	
	/* Calculate and save the sound buffer size */
	s_dwBufferSize = s_nNumberOfFrags * s_dwFragSize;

	if( !s_pSoundBuffer &&
		!(s_pSoundBuffer = calloc( 1, s_dwBufferSize )) )
	{
		Sound_Disable( TRUE );
		return FALSE;
	}

	if( _IsFlagSet( g_Sound.ulState, SS_MM_SOUND ) )
	{
		s_nPlayFragNo = MM_PLAY_FRAG_NO;
		s_nSaveFragNo = MM_SAVE_FRAG_NO;
	}
	else
	if( _IsFlagSet( g_Sound.ulState, SS_DS_SOUND ) )
	{
		s_nPlayFragNo = DS_PLAY_FRAG_NO;
		s_nSaveFragNo = DS_SAVE_FRAG_NO;
	}
	_ASSERT(s_nSaveFragNo < s_nNumberOfFrags);

	s_pPlayCursor = &s_pSoundBuffer[ s_nPlayFragNo * s_dwFragSize ];
	s_pSaveCursor = &s_pSoundBuffer[ s_nSaveFragNo * s_dwFragSize ];

	/* Set sound quality */
	Sound_SetQuality( g_Sound.nQuality );

	/* Clear the sound buffer */
	if (n16BitSnd)
		for( i = s_nPlayFragNo * s_dwFragSize; i < s_nSaveFragNo * (int)s_dwFragSize; i+=2 )
			((UWORD *)s_pSoundBuffer)[ i/2 ] = s_nSilenceData16;
	else
		for( i = s_nPlayFragNo * s_dwFragSize; i < s_nSaveFragNo * (int)s_dwFragSize; i++ )
			s_pSoundBuffer[ i ] = s_nSilenceData8;
	
	/* Initialize the kernel sound machine */
	POKEYSND_Init( POKEYSND_FREQ_17_EXACT,
					  (UWORD)g_Sound.nRate,
					  (UBYTE)nChannels,
					  (n16BitSnd ? POKEYSND_BIT16 : 0),
					  bClearRegs );

	/* Set this up for PCM, 1/2 channels, 8/16 bits unsigned samples */
	ZeroMemory( &s_wfxWaveFormat, sizeof(WAVEFORMATEX) );
	s_wfxWaveFormat.wFormatTag      = WAVE_FORMAT_PCM; /* The only tag valid with DirectSound */
	s_wfxWaveFormat.nChannels       = nChannels;
	s_wfxWaveFormat.nSamplesPerSec  = g_Sound.nRate;
	s_wfxWaveFormat.wBitsPerSample  = n16BitSnd ? 16 : 8;
	s_wfxWaveFormat.nBlockAlign     = s_wfxWaveFormat.nChannels * s_wfxWaveFormat.wBitsPerSample / 8;
	s_wfxWaveFormat.nAvgBytesPerSec = s_wfxWaveFormat.nSamplesPerSec * s_wfxWaveFormat.nBlockAlign;
	s_wfxWaveFormat.cbSize          = 0; /* This member is always zero for PCM formats */

	/* CAUTION:
	   It is necessary to initialise WAVEFORMATEX structure even if the sound
	   has been muted! (vide Sound_VolumeCapable() function)
	*/
	if( _IsFlagSet( g_Sound.ulState, SS_NO_SOUND ) )
	{
		Atari_PlaySound = SndPlay_NoSound;
		return TRUE; /* Exit if SS_NO_SOUND flag is set */
	}

	if( _IsFlagSet( g_Sound.ulState, SS_MM_SOUND ) )
	{
		MMRESULT mmResult = MMSYSERR_NOERROR;

		_ASSERT(s_nNumberOfFrags <= MM_NUMBER_OF_FRAGS);

		for( i = 0; i < s_nNumberOfFrags; i++ )
			ZeroMemory( &s_arrWaveHDR[ i ], sizeof(WAVEHDR) );

		if( !s_hWaveOut )
		{
			mmResult = waveOutOpen( &s_hWaveOut, WAVE_MAPPER, &s_wfxWaveFormat, 0, 0, CALLBACK_NULL );
			if( mmResult != MMSYSERR_NOERROR )
			{
				ServeMMError( IDS_MMERR_OPEN, mmResult, FALSE );
				return FALSE;
			}
		}
		for( i = 0; i < s_nNumberOfFrags; i++ )
		{
			s_arrWaveHDR[ i ].lpData          = &s_pSoundBuffer[ i * s_dwFragSize ];
			s_arrWaveHDR[ i ].dwBufferLength  = s_dwFragSize;
			s_arrWaveHDR[ i ].dwBytesRecorded = 0;
			s_arrWaveHDR[ i ].dwUser          = 0;
			s_arrWaveHDR[ i ].dwFlags         = 0;
			s_arrWaveHDR[ i ].dwLoops         = 0;

			mmResult = waveOutPrepareHeader( s_hWaveOut, &s_arrWaveHDR[ i ], sizeof(WAVEHDR) );
			if( mmResult != MMSYSERR_NOERROR )
			{
				ServeMMError( IDS_MMERR_PREP_HDR, mmResult, FALSE );
				return FALSE;
			}
		}
		Atari_PlaySound = SndPlay_MMSound;
	}
	else
	if( _IsFlagSet( g_Sound.ulState, SS_DS_SOUND ) )
	{
		DSBUFFERDESC dsbdesc;
		HRESULT	hResult;

		/* Create IDirectSound using the primary sound device */
		if( !s_lpDirectSound && FAILED(
			hResult = DirectSoundCreate( NULL, &s_lpDirectSound, NULL )) )
		{
			ServeDSError( IDS_DSERR_CREATE_OBJ, hResult, FALSE );
			return FALSE;
		}
		_ASSERT(s_lpDirectSound);

		if( !DetermineHardwareCaps( s_lpDirectSound, _IsFlagSet( g_Sound.ulState, SS_CUSTOM_RATE ) ) )
			return FALSE;

		/* Set up DSBUFFERDESC structure */
		ZeroMemory( &dsbdesc, sizeof(DSBUFFERDESC) );
		dsbdesc.dwSize  = sizeof(DSBUFFERDESC);
		dsbdesc.dwFlags = DSBCAPS_PRIMARYBUFFER;

		if( _IsFlagSet( g_Sound.ulState, SS_CUSTOM_RATE ) )
		{
			LPDIRECTSOUNDBUFFER lpDSBPrimary;

			/* Obtain priority cooperative level */
			hResult = IDirectSound_SetCooperativeLevel( s_lpDirectSound, g_hMainWnd, DSSCL_PRIORITY );
			if( FAILED(hResult) )
			{
				ServeDSError( IDS_DSERR_COOP_PRIORITY, hResult, FALSE );
				return FALSE;
			}
			/* The DirectSound mixer produces the best sound quality if all
			   application's sounds use the same wave format and the hardware
			   output format is matched to the format of the sounds. If this
			   is done, the mixer doesn't need perform any format conversion.
			   Note that this primary buffer is for control purposes only;
			   creating it is not the same as obtaining write access to the
			   primary buffer */

			/* Try to create the primary buffer */
			hResult = IDirectSound_CreateSoundBuffer( s_lpDirectSound, &dsbdesc, &lpDSBPrimary, NULL );
			if( FAILED(hResult) )
			{
				lpDSBPrimary = NULL;
				ServeDSError( IDS_DSERR_CREATE_PRIMARY, hResult, FALSE );
				return FALSE;
			}
			/* Set primary buffer to desired format */
			hResult = IDirectSoundBuffer_SetFormat( lpDSBPrimary, &s_wfxWaveFormat );
			if( FAILED(hResult) )
			{
				ServeDSError( IDS_DSERR_FORMAT_PRIMARY, hResult, FALSE );
				return FALSE;
			}
			/* Release primary buffer */
			IDirectSoundBuffer_Release( lpDSBPrimary );
		}
		else
		{
			/* Obtain normal cooperative level (22kHz) */
			hResult = IDirectSound_SetCooperativeLevel( s_lpDirectSound, g_hMainWnd, DSSCL_NORMAL );
			if( FAILED(hResult) )
			{
				ServeDSError( IDS_DSERR_COOP_NORMAL, hResult, FALSE );
				return FALSE;
			}
		}
		/* Create secondary DirectSound buffers */

		/* Set up DSBUFFERDESC structure */
		ZeroMemory( &dsbdesc, sizeof(DSBUFFERDESC) );
		dsbdesc.dwSize = sizeof(DSBUFFERDESC);
		/* Need default controls (volume, frequency) */
		dsbdesc.dwFlags = DSBCAPS_CTRLPOSITIONNOTIFY |	/* Needed for notification */
			              DSBCAPS_CTRLVOLUME |
						  DSBCAPS_CTRLPAN |
						  DSBCAPS_CTRLFREQUENCY |
						  DSBCAPS_GETCURRENTPOSITION2 | /* Always a good idea */
						  DSBCAPS_GLOBALFOCUS;			/* Allows background playing */
		dsbdesc.dwBufferBytes = s_dwBufferSize;
		dsbdesc.lpwfxFormat   = &s_wfxWaveFormat;

		hResult = IDirectSound_CreateSoundBuffer( s_lpDirectSound, &dsbdesc, &s_lpDSBuffer, NULL );
		if( FAILED(hResult) )
		{
			s_lpDSBuffer = NULL;
			ServeDSError( IDS_DSERR_CREATE_BUFF, hResult, FALSE );
			return FALSE;
		}

		/* Check if IDirectSoundNotify interface is available. It should be
		   there if DirectX 5.0 or higher is installed */

		hResult = IDirectSoundNotify_QueryInterface( s_lpDSBuffer, (GUID *)&IID_IDirectSoundNotify, (LPVOID FAR *)&s_lpDSNotify );
		if( FAILED(hResult) )
		{
			s_lpDSNotify = NULL;
			ServeDSError( IDS_DSERR_NO_NOTIFY, hResult, FALSE );
			return FALSE;
		}
		/* Set up notification events for a playback buffer */
		s_lpDSNotifyPos = calloc( 1, s_nNumberOfFrags * sizeof(DSBPOSITIONNOTIFY) );
		if( !s_lpDSNotifyPos )
		{
			ServeDSError( IDS_DSERR_SET_NOTIFY, DSERR_OUTOFMEMORY, FALSE );
			return FALSE;
		}
		if( !(_PosEvent = CreateEvent( NULL, FALSE, FALSE, NULL )) ||
			!(_EndEvent = CreateEvent( NULL, FALSE, FALSE, NULL )) )
		{
			ServeDSError( IDS_DSERR_CREATE_THREAD, 0 - GetLastError(), FALSE );
			return FALSE;
		}
		for( i = 0; i < s_nNumberOfFrags; i++ )
		{
			s_lpDSNotifyPos[ i ].dwOffset = s_dwFragSize * i;
			s_lpDSNotifyPos[ i ].hEventNotify = _PosEvent;
		}
		hResult = IDirectSoundNotify_SetNotificationPositions( s_lpDSNotify, s_nNumberOfFrags, s_lpDSNotifyPos );
		if( FAILED(hResult) )
		{
			ServeDSError( IDS_DSERR_SET_NOTIFY, hResult, FALSE );
			return FALSE;
		}
		/* Spawn a secondary sound thread */
		if( !(s_hNotifyThread = _CreateThreadEx( NULL,
												 0,
												 DSSoundThreadProc,
												 NULL,
												 0,
												 &s_dwThreadID )) )
		{
			ServeDSError( IDS_DSERR_CREATE_THREAD, 0 - GetLastError(), FALSE );
			return FALSE;
		}
		/* Set priority of the secondary thread */
		SetThreadPriority( s_hNotifyThread, THREAD_PRIORITY_ABOVE_NORMAL );

		Atari_PlaySound = SndPlay_DSSound;
	}
	else
		return FALSE;

	/* Set sound volume */
	Sound_SetVolume();

	if( !_IsFlagSet( g_Misc.ulState, MS_FULL_SPEED ) )
		Sound_Restart();

	return TRUE;

} /* #OF# Sound_Initialise */

/*========================================================
Function : Sound_SetQuality
=========================================================*/
/* #FN#
   Sets quality of a sound playback */
void
/* #AS#
   Nothing */
Sound_SetQuality(
  int nQuality /* #IN# Sound quality */
)
{
	if( nQuality > 1 )
	{
		/* Michael Borisov's High Fidelity Pokey emulation */
		POKEYSND_SetMzQuality( (nQuality - 2) % 3 ); /* 0 for the fastest rendering */
		POKEYSND_enable_new_pokey = 1;

		_TRACE1("mz sound quality: %d\n", (nQuality - 2) % 3);
	}
	else
		/* Ron Fries' old Pokey emulation */
		POKEYSND_enable_new_pokey = 0;
//	POKEYSND_DoInit();

} /* #OF# Sound_SetQuality */

/*========================================================
Function : SndPlay_NoSound
=========================================================*/
/* #FN#
   Generates sound stream but doesn't play the sound */
static
void
/* #AS#
   Nothing */
SndPlay_NoSound( void )
{
	int samples_written = MZPOKEYSND_UpdateProcessBuffer();
	int bytes_written = (s_n16BitSnd ? samples_written*2 : samples_written);
	if ( s_dwFragPos + bytes_written > s_dwFragSize)
		bytes_written = s_dwFragSize - s_dwFragPos;
	memcpy(s_pSaveCursor + s_dwFragPos, MZPOKEYSND_process_buffer, bytes_written);
	s_dwFragPos += bytes_written;

	if( ++s_nFrameCnt > s_nFramesPerFrag )
	{
		_ASSERT(s_dwFragPos <= s_dwFragSize);

//		if( s_dwFragPos < s_dwFragSize )
//			POKEYSND_Process_ptr( s_pSaveCursor + s_dwFragPos, (s_dwFragSize - s_dwFragPos) >> s_n16BitSnd );

		if( g_Sound.pfOutput )
			fwrite( s_pSaveCursor, s_dwFragSize, 1, g_Sound.pfOutput );

		Video_SaveFrame( NULL, 0, s_pSaveCursor, s_dwFragSize );

		if( ++s_nPlayFragNo == s_nNumberOfFrags )
			s_nPlayFragNo = 0;

		if( ++s_nSaveFragNo == s_nNumberOfFrags )
			s_nSaveFragNo = 0;

		s_pPlayCursor = &s_pSoundBuffer[ s_nPlayFragNo * s_dwFragSize ];
		s_pSaveCursor = &s_pSoundBuffer[ s_nSaveFragNo * s_dwFragSize ];

		s_dwFragPos  = 0;
		s_nUpdateCnt = 1;
		s_nFrameCnt  = 1;
	}
} /* #OF# SndPlay_NoSound */

/*========================================================
Function : SndPlay_MMSound
=========================================================*/
/* #FN#
   Generates sound stream and plays it via WaveOut */
static
void
/* #AS#
   Nothing */
SndPlay_MMSound( void )
{
	int samples_written = MZPOKEYSND_UpdateProcessBuffer();
	int bytes_written = (s_n16BitSnd ? samples_written*2 : samples_written);
	if ( s_dwFragPos + bytes_written > s_dwFragSize)
		bytes_written = s_dwFragSize - s_dwFragPos;
	memcpy(s_pSaveCursor + s_dwFragPos, MZPOKEYSND_process_buffer, bytes_written);
	s_dwFragPos += bytes_written;

	if( ++s_nFrameCnt > s_nFramesPerFrag )
	{
		_ASSERT(s_dwFragPos <= s_dwFragSize);

//		if( s_dwFragPos < s_dwFragSize )
			/* Write the audio data to the buffer if it is not full */
//			POKEYSND_Process_ptr( s_pSaveCursor + s_dwFragPos, (s_dwFragSize - s_dwFragPos) >> s_n16BitSnd );

		if( g_Sound.pfOutput )
			fwrite( s_pSaveCursor, s_dwFragSize, 1, g_Sound.pfOutput );

		Video_SaveFrame( NULL, 0, s_pSaveCursor, s_dwFragSize );

		/* SoundIsPaused indicates full speed mode in this case; if
		   the emulated Atari is paused, this routine is not invoked */
		if( !s_bSoundIsPaused )
			waveOutWrite( s_hWaveOut, &s_arrWaveHDR[ s_nPlayFragNo ], sizeof(WAVEHDR) );

		if( ++s_nPlayFragNo == s_nNumberOfFrags )
			s_nPlayFragNo = 0;

		if( ++s_nSaveFragNo == s_nNumberOfFrags )
			s_nSaveFragNo = 0;

		s_pPlayCursor = &s_pSoundBuffer[ s_nPlayFragNo * s_dwFragSize ];
		s_pSaveCursor = &s_pSoundBuffer[ s_nSaveFragNo * s_dwFragSize ];

		s_dwFragPos  = 0;
		s_nUpdateCnt = 1;
		s_nFrameCnt  = 1;
	}
} /* #OF# SndPlay_MMSound */

/*========================================================
Function : SndPlay_DSSound
=========================================================*/
/* #FN#
   Generates sound stream and plays it via DirectSound */
static
void
/* #AS#
   Nothing */
SndPlay_DSSound( void )
{
	int samples_written = MZPOKEYSND_UpdateProcessBuffer();
	int bytes_written = (s_n16BitSnd ? samples_written*2 : samples_written);
	if ( s_dwFragPos + bytes_written > s_dwFragSize)
		bytes_written = s_dwFragSize - s_dwFragPos;
	memcpy(s_pSaveCursor + s_dwFragPos, MZPOKEYSND_process_buffer, bytes_written);
	s_dwFragPos += bytes_written;

	if( ++s_nFrameCnt > s_nFramesPerFrag )
	{
		_ASSERT(s_dwFragPos <= s_dwFragSize);

//		if( s_dwFragPos < s_dwFragSize )
//			POKEYSND_Process_ptr( s_pSaveCursor + s_dwFragPos, (s_dwFragSize - s_dwFragPos) >> s_n16BitSnd );

		if( g_Sound.pfOutput )
			fwrite( s_pSaveCursor, s_dwFragSize, 1, g_Sound.pfOutput );

		Video_SaveFrame( NULL, 0, s_pSaveCursor, s_dwFragSize );

	}
} /* #OF# SndPlay_DSSound */

/*========================================================
Function : LockSoundBuffer
=========================================================*/
/* #FN#
   Locks the DirectSound buffer for direct accessing to be safe */
_inline
static
BOOL
/* #AS#
   TRUE if succeeded, otherwise FALSE */
LockSoundBuffer(
	BOOL     bLock,
	DWORD    dwBufferOffset,
	DWORD    dwFragSize,
	LPVOID  *lppvPtr1,
	PDWORD   pBytes1,
	LPVOID  *lppvPtr2,
	PDWORD   pBytes2,
	DWORD    dwFlags
)
{
	if( bLock )
	{
		HRESULT
		/* Obtain a valid write pointer to the sound buffer's audio data */
		hResult = IDirectSoundBuffer_Lock( s_lpDSBuffer, dwBufferOffset, dwFragSize, lppvPtr1, pBytes1, lppvPtr2, pBytes2, dwFlags );
		/* If DSERR_BUFFERLOST is returned, restore and retry lock */
		if( DSERR_BUFFERLOST == hResult )
		{
			hResult = IDirectSoundBuffer_Restore( s_lpDSBuffer );
			if( FAILED(hResult) )
				return FALSE;

			hResult = IDirectSoundBuffer_Lock( s_lpDSBuffer, dwBufferOffset, dwFragSize, lppvPtr1, pBytes1, lppvPtr2, pBytes2, dwFlags );
			if( FAILED(hResult) )
				return FALSE;
		}
	}
	else
		IDirectSoundBuffer_Unlock( s_lpDSBuffer, *lppvPtr1, *pBytes1, *lppvPtr2, *pBytes2 );

	return TRUE;

} /* #OF# LockSoundBuffer */

/*========================================================
Function : DSSoundThreadProc
=========================================================*/
/* #FN#
   The DirectSound notification thread procedure */
static
DWORD WINAPI
/* #AS#
   Always 0 */
DSSoundThreadProc(
	LPVOID lpParam
)
{
	DWORD   dwPlayCursor   = 0;
	DWORD   dwSaveCursor   = 0;
	DWORD   dwBufferOffset = 0;
	BOOL    bBufferCleared = FALSE;
	LPVOID  lpvPtr1        = NULL;
	LPVOID  lpvPtr2        = NULL;
	DWORD   dwBytes1       = 0;
	DWORD   dwBytes2       = 0;
	DWORD   dwEventResult  = 0;
	HRESULT hResult;
	int     i;

#ifdef _DEBUG
	LARGE_INTEGER lnTicks;
#endif

	while( END_EVENT + WAIT_OBJECT_0 != dwEventResult )
	{
		dwEventResult = WaitForMultipleObjects( NUMBER_OF_EVENTS, s_arrEvents, FALSE, INFINITE );
		if( WAIT_OBJECT_0 == dwEventResult )
		{
//			_TRACE2("SecondaryThread.DSSoundThreadProc: s_nSaveFragNo: %d, s_nPlayFragNo: %d\n", s_nSaveFragNo, s_nPlayFragNo);
			if( s_nPlayFragNo == s_nSaveFragNo )
			{
//				_TRACE0("SecondaryThread.DSSoundThreadProc: s_nPlayFragNo == s_nSaveFragNo\n");
				if( !bBufferCleared )
				{
					if( LockSoundBuffer( TRUE, 0, 0, &lpvPtr1, &dwBytes1, &lpvPtr2, &dwBytes2, DSBLOCK_ENTIREBUFFER ) )
					{
						if (s_n16BitSnd)
						{
							for( i = 0; i < (int)dwBytes1; i+=2 )
								((UWORD*)lpvPtr1)[ i/2 ] = s_nSilenceData16;

							if( lpvPtr2 != NULL )
							{
								for( i = 0; i < (int)dwBytes2; i+=2 )
									((UWORD*)lpvPtr2)[ i/2 ] = s_nSilenceData16;
							}
						}
						else
						{
							for( i = 0; i < (int)dwBytes1; i++ )
								((UBYTE*)lpvPtr1)[ i ] = s_nSilenceData8;

							if( lpvPtr2 != NULL )
							{
								for( i = 0; i < (int)dwBytes2; i++ )
									((UBYTE*)lpvPtr2)[ i ] = s_nSilenceData8;
							}
						}

						LockSoundBuffer( FALSE, 0, 0, &lpvPtr1, &dwBytes1, &lpvPtr2, &dwBytes2, 0 );

						bBufferCleared = TRUE;
					}
				}
			}
			else /* s_nPlayFragNo != s_nSaveFragNo */
			{
				hResult = IDirectSoundBuffer_GetCurrentPosition( s_lpDSBuffer, &dwPlayCursor, &dwSaveCursor );
//				_TRACE2("Sound cursor: %d, %d\n", dwPlayCursor, dwSaveCursor);
				if( SUCCEEDED(hResult) )
				{
					dwBufferOffset = s_nPlayFragNo * s_dwFragSize;

					if( !(dwPlayCursor >= dwBufferOffset && dwPlayCursor < dwBufferOffset + s_dwFragSize) )
					{
#ifdef _DEBUG
//						QueryPerformanceCounter( &lnTicks );
//						_TRACE1("Ticks %d\n", lnTicks.LowPart);
#endif
						if( LockSoundBuffer( TRUE, dwBufferOffset, s_dwFragSize, &lpvPtr1, &dwBytes1, &lpvPtr2, &dwBytes2, 0 ) )
						{
							s_pPlayCursor = &s_pSoundBuffer[ dwBufferOffset ];

							CopyMemory( lpvPtr1, s_pPlayCursor, dwBytes1 );
							if( NULL != lpvPtr2 )
								CopyMemory( lpvPtr2, s_pPlayCursor + dwBytes1, dwBytes2 );

							LockSoundBuffer( FALSE, 0, 0, &lpvPtr1, &dwBytes1, &lpvPtr2, &dwBytes2, 0 );

							if( ++s_nPlayFragNo == s_nNumberOfFrags )
								s_nPlayFragNo = 0;

							bBufferCleared = FALSE;
						}
					}
				}
			} /* s_nPlayFragNo == s_nSaveFragNo */
		}
	}
	return 0;

} /* #OF# DSSoundThreadProc */

/*========================================================
Function : Sound_VolumeCapable
=========================================================*/
/* #FN#
   Checks if a sound output device supports a volume control */
BOOL
/* #AS#
   TRUE if volume control is supported, otherwise FALSE */
Sound_VolumeCapable( void )
{
	BOOL bReturn = TRUE;

	if( _IsFlagSet( g_Sound.ulState, SS_MM_SOUND ) )
	{
		WAVEOUTCAPS woc;
		ZeroMemory( &woc, sizeof(WAVEOUTCAPS) );

		if( s_hWaveOut )
		{
			waveOutGetDevCaps( (unsigned int)s_hWaveOut, &woc, sizeof(WAVEOUTCAPS) );
		}
		else
		{
			HWAVEOUT hLocalWave;
			MMRESULT mmResult;

			mmResult = waveOutOpen( &hLocalWave, WAVE_MAPPER, &s_wfxWaveFormat, 0, 0, CALLBACK_NULL );
			if( mmResult == MMSYSERR_NOERROR )
			{
				waveOutGetDevCaps( (unsigned int)hLocalWave, &woc, sizeof(WAVEOUTCAPS) );
				waveOutClose( hLocalWave );
			}
		}
		bReturn = (_IsFlagSet( woc.dwSupport, WAVECAPS_VOLUME ) ? TRUE : FALSE);
	}
	return bReturn;

} /* #OF# Sound_VolumeCapable */

/*========================================================
Function : Sound_SetVolume
=========================================================*/
/* #FN#
   Sets volume of a sound playback */
void
/* #AS#
   Nothing */
Sound_SetVolume( void )
{
	if( _IsFlagSet( g_Sound.ulState, SS_MM_SOUND ) )
	{
		if( s_hWaveOut && Sound_VolumeCapable() )
		{
			MMRESULT mmResult;
			DWORD    dwVolume, dwTempVolume;

			/* Save the volume setting so we can put it back later */
			if( !s_dwStartVolume )
				mmResult = waveOutGetVolume( s_hWaveOut, &s_dwStartVolume );

			/* Hiword is the right channel, low word is the left channel */
			dwVolume = (HIWORD(s_dwStartVolume) + HIWORD(s_dwStartVolume) / 100 * g_Sound.nVolume) << 16;
			dwVolume += LOWORD(s_dwStartVolume) + LOWORD(s_dwStartVolume) / 100 * g_Sound.nVolume;

			mmResult = waveOutSetVolume( s_hWaveOut, dwVolume );
			if( mmResult != MMSYSERR_NOERROR )
			{
				ServeMMError( IDS_MMERR_SET_VOLUME, mmResult, FALSE );
				return;
			}

			/* It's possible this wave device doesn't support 16 bits of volume control,
			   so we'll check the result of the set with waveOutGetVolume, if it's less
			   than what we set, we know the new max and we'll scale to that */

			waveOutGetVolume( s_hWaveOut, &dwTempVolume );
			if( dwTempVolume < dwVolume )
			{
				float fPercentage = ((float)dwTempVolume / dwVolume);
				dwVolume = (DWORD)(dwVolume * fPercentage);
				waveOutSetVolume( s_hWaveOut, dwVolume );
			}
		}
	}
	if( _IsFlagSet( g_Sound.ulState, SS_DS_SOUND ) )
	{
		if( s_lpDSBuffer )
			IDirectSoundBuffer_SetVolume( s_lpDSBuffer, g_Sound.nVolume * 22 );
	}
} /* #OF# Sound_SetVolume */

/*========================================================
Function : Sound_Clear
=========================================================*/
/* #FN#
   Closes/mutes a sound output device */
void
/* #AS#
   Nothing */
Sound_Clear(
	BOOL bPermanent,
	BOOL bFreeBuffer
)
{
	/* Are we shutting down everything, or just pausing the
	   sound that is playing currently? */
	if( bPermanent )
	{
		DWORD dwExitCode = 0;

		Atari_PlaySound = SndPlay_NoSound;

		if( g_Sound.pfOutput )
		{
			/* Close Sound Output file */
			/* CAUTION:
			   After closing the info box the main window receives the
			   OnActivate(::RestartSound) message; that's why we set
			   the SoundIsPaused to TRUE after calling CloseOutput */
			StopSoundRecording( 0, TRUE );
		}
		s_bSoundIsPaused = TRUE;

		/* Clear Multimedia stuff */
		if( s_hWaveOut )
		{
			int i;
			if( s_dwStartVolume )
			{
				waveOutSetVolume( s_hWaveOut, s_dwStartVolume );
				s_dwStartVolume = 0;
			}
			waveOutReset( s_hWaveOut );
			for( i = 0; i < s_nNumberOfFrags; i++ )
			{
				waveOutUnprepareHeader( s_hWaveOut, &s_arrWaveHDR[ i ], sizeof(WAVEHDR) );
				s_arrWaveHDR[ i ].lpData = NULL;
			}
			waveOutClose( s_hWaveOut );
			s_hWaveOut = 0;
		}

		/* Clear DirectSound stuff */
		if( s_lpDSBuffer )
			/* Stop buffer play */
			IDirectSoundBuffer_Stop( s_lpDSBuffer );

		if( s_hNotifyThread )
		{
			/* Stop & Kill streaming thread */
			SetEvent( _EndEvent );
			/* Wait for thread termination */
			do
			{
				Sleep( 1 ); /* Wait a moment, please */
				GetExitCodeThread( s_hNotifyThread, &dwExitCode );
			}
			while( STILL_ACTIVE == dwExitCode );

			CloseHandle( s_hNotifyThread );
			s_hNotifyThread = NULL;
		}
		if( _PosEvent )
		{
			CloseHandle( _PosEvent );
			_PosEvent = NULL;
		}
		if( _EndEvent )
		{
			CloseHandle( _EndEvent );
			_EndEvent = NULL;
		}
		if( s_lpDSNotify )
		{
			IDirectSoundNotify_Release( s_lpDSNotify );
			s_lpDSNotify = NULL;
		}
		if( s_lpDSNotifyPos )
		{
			free( s_lpDSNotifyPos );
			s_lpDSNotifyPos = NULL;
		}
		if( s_lpDSBuffer )
		{
			IDirectSoundBuffer_Release( s_lpDSBuffer );
			s_lpDSBuffer = NULL;
		}
		if( s_lpDirectSound )
		{
			IDirectSound_Release( s_lpDirectSound );
			s_lpDirectSound = NULL;
		}
		if( s_pSoundBuffer && bFreeBuffer )
		{
			free( s_pSoundBuffer );
			s_pSoundBuffer = NULL;
		}
		s_pPlayCursor = NULL;
		s_pSaveCursor = NULL;
	}
	else
	{
		s_bSoundIsPaused = TRUE;

		/* Stop playback and clear waveOut buffer */
		if( s_hWaveOut )
			waveOutPause( s_hWaveOut );

		/* Clear DirectSound buffer and stop playback */
		/* CAUTION:
		   It seems to be not really needed since the
		   secondary buffer clears sound playback (?) */
		if( s_lpDSBuffer )
		{
			IDirectSoundBuffer_Stop( s_lpDSBuffer );
			/* Clear the DirectSound sound buffer
			   CAUTION:
			   We assume that there are only two buffers for
			   DirectSound! */
			SetEvent( _PosEvent );
			SetEvent( _PosEvent );
		}
//		if( s_pSoundBuffer )
//		{
//			int i;
//			for( i = 0; i < (int)s_dwBufferSize; i++ )
//				s_pSoundBuffer[ i ] = s_nSilenceData;
//		}
	}
} /* #OF# Sound_Clear */

/*========================================================
Function : Sound_Restart
=========================================================*/
/* #FN#
   Resumes playback on a paused sound output device */
void
/* #AS#
   Nothing */
Sound_Restart( void )
{
	if( _IsFlagSet( g_Misc.ulState, MS_FULL_SPEED ) )
		return;

	if( s_bSoundIsPaused )
	{
		if( s_hWaveOut )
		{
			waveOutRestart( s_hWaveOut );
			s_bSoundIsPaused = FALSE;
		}
		if( s_lpDSBuffer )
		{
			IDirectSoundBuffer_Play( s_lpDSBuffer, 0, 0, DSBPLAY_LOOPING );
			s_bSoundIsPaused = FALSE;
		}
	}
} /* #OF# Sound_Restart */

/*========================================================
Function : Sound_OpenOutput
=========================================================*/
/* #FN#
   Opens a sound file */
void
/* #AS#
   Nothing */
Sound_OpenOutput( char *pszOutFileName )
{
	if( g_Sound.pfOutput )
		/* Close Sound Output file */
		Sound_CloseOutput();

	_ASSERT(0 == POKEYSND_stereo_enabled || 1 == POKEYSND_stereo_enabled);

	if( (g_Sound.pfOutput = fopen( pszOutFileName, "wb" )) )
	{
		WAVEFORMAT wf;
		WORD wBitsPerSample = s_n16BitSnd ? 16 : 8;

//		wf.wFormatTag      = WAVE_FORMAT_PCM;
		wf.nChannels       = POKEYSND_stereo_enabled + 1;
		wf.nSamplesPerSec  = g_Sound.nRate;
		wf.nBlockAlign     = wf.nChannels * wBitsPerSample / 8;
		wf.nAvgBytesPerSec = wf.nSamplesPerSec * wf.nBlockAlign;
		/*
		  Offset  Length   Contents

		The RIFF header:
		  0       4 bytes  'RIFF'
		  4       4 bytes  <file length - 8>
		  8       4 bytes  'WAVE'

		The format (fmt) chunk:
		  12      4 bytes  'fmt '
		  16      4 bytes  0x00000010     // Length of the fmt data (16 bytes)
		  20      2 bytes  0x0001         // Format tag: 1 = PCM
		  22      2 bytes  <channels>     // Channels: 1 = mono, 2 = stereo
		  24      4 bytes  <sample rate>  // Samples per second: e.g., 44100
		  28      4 bytes  <bytes/second> // sample rate * block align
		  32      2 bytes  <block align>  // channels * bits/sample / 8
		  34      2 bytes  <bits/sample>  // 8 or 16

		The data chunk:
		  36      4 bytes  'data'
		  40      4 bytes  <length of the data block>
		  44      ? bytes  <sample data>
		*/
		fwrite( "RIFF\000\000\000\000WAVEfmt\040\020\000\000\000\001\000", 22, 1, g_Sound.pfOutput );
		fwrite( &wf.nChannels,       2, 1, g_Sound.pfOutput );
		fwrite( &wf.nSamplesPerSec,  4, 1, g_Sound.pfOutput );
		fwrite( &wf.nAvgBytesPerSec, 4, 1, g_Sound.pfOutput );
		fwrite( &wf.nBlockAlign,     2, 1, g_Sound.pfOutput );
		fwrite( &wBitsPerSample,     2, 1, g_Sound.pfOutput );
		fwrite( "data\000\000\000\000", 8, 1, g_Sound.pfOutput );
	}
	else
		DisplayMessage( NULL, IDS_ERROR_FILE_OPEN, 0, MB_ICONEXCLAMATION | MB_OK, pszOutFileName );

} /* #OF# Sound_OpenOutput */

/*========================================================
Function : Sound_CloseOutput
=========================================================*/
/* #FN#
   Closes a sound file */
void
/* #AS#
   Nothing */
Sound_CloseOutput( void )
{
	if( g_Sound.pfOutput )
	{
		UINT unPos = 0;

		/* Sound file is finished, so modify header and close it */
		unPos = ftell( g_Sound.pfOutput ) - 8;
		fseek ( g_Sound.pfOutput, 4, SEEK_SET );	// Seek past RIFF
		fwrite( &unPos, 4, 1, g_Sound.pfOutput );	// Write out size of entire data chunk
		fseek ( g_Sound.pfOutput, 40, SEEK_SET );
		unPos -= 36;
		fwrite( &unPos, 4, 1, g_Sound.pfOutput );	// Write out size of just sample data
		fclose( g_Sound.pfOutput );

		g_Sound.pfOutput = NULL;

		DisplayMessage( NULL, IDS_SFX_FILE_CLOSED, 0, MB_ICONINFORMATION | MB_OK );
	}
} /* #OF# Sound_CloseOutput */

/*========================================================
Function : Sound_Disable
=========================================================*/
/* #FN#
   Turns off a sound playback (disable sound) */
BOOL
/* #AS#
   TRUE if succeeded, otherwise FALSE */
Sound_Disable(
	BOOL bClearSound
)
{
	if( bClearSound )
		Sound_Clear( TRUE, FALSE );

	/* Modify sound state of emulator */
//	_ClrFlag( g_Sound.ulState, SS_DS_SOUND | SS_MM_SOUND ); /* Sound/Mute needs these flags */
	_SetFlag( g_Sound.ulState, SS_NO_SOUND );
	WriteRegDWORD( NULL, REG_SOUND_STATE, g_Sound.ulState );

	/* It always is succeeded for SS_NO_SOUND */
	return Sound_Initialise( TRUE );

} /* #OF# Sound_Disable */

/*========================================================
Function : DS_GetErrorString
=========================================================*/
/* #FN#
   Outputs a debug string to debugger */
static
BOOL
/* #AS#
   TRUE if succeeded, otherwise FALSE */
DS_GetErrorString( HRESULT hResult,
				  LPSTR   pszErrorBuff,
				  DWORD   dwError )
{
	DWORD dwLen;
	LPSTR pszError;
	char  szMsg[ 256 ];

	/* Check parameters */
	if( !pszErrorBuff || !dwError )
	{
		/* Error, invalid parameters */
		return FALSE;
	}

	switch( hResult )
	{
		/* The request completed successfully */
		case DS_OK:
			pszError = "DS_OK";
			break;

		/* The request failed because resources, such as a priority level, were already in use by another caller */
		case DSERR_ALLOCATED:
			pszError = "DSERR_ALLOCATED";
			break;

		/* The object is already initialized */
		case DSERR_ALREADYINITIALIZED:
			pszError = "DSERR_ALREADYINITIALIZED";
			break;

		/* The specified wave format is not supported */
		case DSERR_BADFORMAT:
			pszError = "DSERR_BADFORMAT";
			break;

		/* The buffer memory has been lost and must be restored */
		case DSERR_BUFFERLOST:
			pszError = "DSERR_BUFFERLOST";
			break;

		/* The control (volume, pan, and so forth) requested by the caller is not available */
		case DSERR_CONTROLUNAVAIL:
			pszError = "DSERR_CONTROLUNAVAIL";
			break;

		/* An undetermined error occurred inside the DirectSound subsystem */
		case DSERR_GENERIC:
			pszError = "DSERR_GENERIC";
			break;

		/* This function is not valid for the current state of this object */
		case DSERR_INVALIDCALL:
			pszError = "DSERR_INVALIDCALL";
			break;

		/* An invalid parameter was passed to the returning function */
		case DSERR_INVALIDPARAM:
			pszError = "DSERR_INVALIDPARAM";
			break;

		/* The object does not support aggregation */
		case DSERR_NOAGGREGATION:
			pszError = "DSERR_NOAGGREGATION";
			break;

		/* No sound driver is available for use */
		case DSERR_NODRIVER:
			pszError = "DSERR_NODRIVER";
			break;

		/* The requested COM interface is not available */
		case DSERR_NOINTERFACE:
			pszError = "DSERR_NOINTERFACE";
			break;

		/* Another application has a higher priority level, preventing this call from succeeding */
		case DSERR_OTHERAPPHASPRIO:
			pszError = "DSERR_OTHERAPPHASPRIO";
			break;

		/* The DirectSound subsystem could not allocate sufficient memory to complete the caller's request */
		case DSERR_OUTOFMEMORY:
			pszError = "DSERR_OUTOFMEMORY";
			break;

		/* The caller does not have the priority level required for the function to succeed */
		case DSERR_PRIOLEVELNEEDED:
			pszError = "DSERR_PRIOLEVELNEEDED";
			break;

		/* The IDirectSound::Initialize method has not been called or has not been called successfully before other methods were called */
		case DSERR_UNINITIALIZED:
			pszError = "DSERR_UNINITIALIZED";
			break;

		/* The function called is not supported at this time */
		case DSERR_UNSUPPORTED:
			pszError = "DSERR_UNSUPPORTED";
			break;

		/* Unknown DS Error */
		default:
			sprintf( szMsg, "Error #%ld", (DWORD)(hResult & 0x0000FFFFL) );
			pszError = szMsg;
			break;
	}
	/* Copy DS Error string to buff */
	dwLen = strlen( pszError );
	if( dwLen >= dwError )
	{
		dwLen = dwError - 1;
	}
	if( dwLen )
	{
		_strncpy( pszErrorBuff, pszError, dwLen );
		pszErrorBuff[ dwLen ] = 0;
	}
	return TRUE;

} /* #OF# DS_GetErrorString */

/*========================================================
Function : MM_GetErrorString
=========================================================*/
/* #FN#
   Outputs a debug string to debugger */
static
BOOL
/* #AS#
   TRUE if succeeded, otherwise FALSE */
MM_GetErrorString( MMRESULT mmResult,
				  LPSTR    pszErrorBuff,
				  DWORD    dwError )
{
	char  szMsg[ 256 ];
	LPSTR pszError;
	DWORD dwLen;

	/* Check parameters */
	if( !pszErrorBuff || !dwError )
	{
		/* Error, invalid parameters */
		return FALSE;
	}

	switch( mmResult )
	{
		/* No error */
		case MMSYSERR_NOERROR:
			pszError = "MMSYSERR_NOERROR";
			break;

		/* Wave header is not prepared for output */
		case WAVERR_UNPREPARED:
			pszError =  "WAVERR_UNPREPARED";
			break;

		/* Unspecified error */
		case MMSYSERR_ERROR:
			pszError = "MMSYSERR_ERROR";
			break;

		/* Device ID out of range */
		case MMSYSERR_BADDEVICEID:
			pszError = "MMSYSERR_BADDEVICEID";
			break;

		/* Driver failed enable */
		case MMSYSERR_NOTENABLED:
			pszError = "MMSYSERR_NOTENABLED";
			break;

		/* Device already allocated */
		case MMSYSERR_ALLOCATED:
			pszError = "MMSYSERR_ALLOCATED";
			break;

		/* Device handle is invalid */
		case MMSYSERR_INVALHANDLE:
			pszError = "MMSYSERR_INVALHANDLE";
			break;

		/* No device driver present */
		case MMSYSERR_NODRIVER:
			pszError = "MMSYSERR_NODRIVER";
			break;

		/* Memory allocation error */
		case MMSYSERR_NOMEM:
			pszError = "MMSYSERR_NOMEM";
			break;

		/* Function isn't supported */
		case MMSYSERR_NOTSUPPORTED:
			pszError = "MMSYSERR_NOTSUPPORTED";
			break;

		/* Error value out of range */
		case MMSYSERR_BADERRNUM:
			pszError = "MMSYSERR_BADERRNUM";
			break;

		/* Invalid flag passed */
		case MMSYSERR_INVALFLAG:
			pszError = "MMSYSERR_INVALFLAG";
			break;

		/* Invalid parameter passed */
		case MMSYSERR_INVALPARAM:
			pszError = "MMSYSERR_INVALPARAM";
			break;

		/* Handle being used */
		case MMSYSERR_HANDLEBUSY:
			pszError = "MMSYSERR_HANDLEBUSY";
			break;

		/* Specified alias not found */
		case MMSYSERR_INVALIDALIAS:
			pszError = "MMSYSERR_INVALIDALIAS";
			break;

		/* Bad registry database */
		case MMSYSERR_BADDB:
			pszError = "MMSYSERR_BADDB";
			break;

		/* Registry key not found */
		case MMSYSERR_KEYNOTFOUND:
			pszError = "MMSYSERR_KEYNOTFOUND";
			break;

		/* Registry read error */
		case MMSYSERR_READERROR:
			pszError = "MMSYSERR_READERROR";
			break;

		/* Registry write error */
		case MMSYSERR_WRITEERROR:
			pszError = "MMSYSERR_WRITEERROR";
			break;

		/* Registry delete error */
		case MMSYSERR_DELETEERROR:
			pszError = "MMSYSERR_DELETEERROR";
			break;

		/* Registry value not found */
		case MMSYSERR_VALNOTFOUND:
			pszError = "MMSYSERR_VALNOTFOUND";
			break;

		/* Driver does not call DriverCallback */
		case MMSYSERR_NODRIVERCB:
			pszError = "MMSYSERR_NODRIVERCB";
			break;

		/* Unknown MM Error */
		default:
			sprintf( szMsg, "Unknown Error #%ld", (DWORD)(mmResult) );
			pszError = szMsg;
			break;
	}
	/* Copy MM Error string to buff */
	dwLen = strlen( pszError );
	if( dwLen >= dwError )
	{
		dwLen = dwError - 1;
	}
	if( dwLen )
	{
		_strncpy( pszErrorBuff, pszError, dwLen );
		pszErrorBuff[ dwLen ] = '\0';
	}
	return TRUE;

} /* #OF# MM_GetErrorString */
