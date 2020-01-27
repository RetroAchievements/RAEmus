/****************************************************************************
File    : avisave.c
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Implementation of AVI API
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 04.09.2002
*/

/**************************************************************************
 *
 *  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 *  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 *  PURPOSE.
 *
 *  Copyright (C) 1992 - 1997 Microsoft Corporation.  All Rights Reserved.
 *
 **************************************************************************/
/*
	This file has been modified for Atari800Win PLus purposes
*/

#define AVIIF_KEYFRAME  0x00000010L /* This frame is a key frame */

#include <stdio.h>
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <vfw.h>
#include "WinConfig.h"
#include "Resource.h"
#include "Debug.h"
#include "Helpers.h"
#include "atari800.h"
#include "globals.h"
#include "macros.h"
#include "timing.h"
#include "display_win.h"
#include "sound_win.h"
#include "misc_win.h"
#include "avisave.h"


/* Private objects */

static PAVISTREAM s_psSound = NULL; 
static PAVISTREAM s_psVideo = NULL; 
static PAVISTREAM s_psCodec = NULL; 

static DWORD s_dwVideoFrame = 0;
static DWORD s_dwSoundFrame = 0;

#ifdef _DEBUG
static void CheckAVIError( DWORD dwError );
#endif


/*========================================================
Function : AVI_Init
=========================================================*/
/* #FN#
   Initializes Video for Windows environment */
static
BOOL
/* #AS#
   TRUE if succeeded, otherwise FALSE */
AVI_Init( void )
{
	/* First let's make sure we are running on 1.1 */
	WORD wVer = HIWORD( VideoForWindowsVersion() );
	if( wVer < 0x010a )
	{
		/* Oops, we are too old, blow out of here */
		return FALSE;
	}
	AVIFileInit();

	return TRUE;

} /* #OF# AVI_Init */

/*========================================================
Function : AVI_FileOpenWrite
=========================================================*/
/* #FN#
   Opens an AVI file */
static
BOOL
/* #AS#
   TRUE if succeeded, otherwise FALSE */
AVI_FileOpenWrite(
	PAVIFILE *pFile,      /* #IN# */
	LPSTR     pszFileName /* #IN# */
)
{
	if( AVIERR_OK !=
		AVIFileOpen( pFile,					/* Returned file pointer  */
					 pszFileName,			/* File name              */
					 OF_WRITE | OF_CREATE,	/* Mode to open file with */
					 NULL )					/* Use handler determined from file extension... */
	  )
		return FALSE;

	return TRUE;

} /* #OF# AVI_FileOpenWrite */

/*========================================================
Function : AVI_CreateStreamVideo
=========================================================*/
/* #FN#
   Opens a video stream for AVI file */
static
BOOL
/* #AS#
   TRUE if succeeded, otherwise FALSE */
AVI_CreateStreamVideo(
	PAVIFILE    pFile,       /* #IN# */
	PAVISTREAM *psStream,    /* #OUT# */
	int         nRate,       /* #IN# Sample per second */
	ULONG       nBufferSize, /* #IN# */
	int         nWidth,      /* #IN# */
	int         nHeight      /* #IN# */
)
{
	AVISTREAMINFO strhdr;

	ZeroMemory( &strhdr, sizeof(strhdr) );
	/* Fill in the stream header for the video stream */
	strhdr.fccType               = streamtypeVIDEO;
	strhdr.fccHandler            = 0;
	strhdr.dwScale               = 1;
	strhdr.dwRate                = nRate;
	strhdr.dwSuggestedBufferSize = nBufferSize;

	SetRect( &strhdr.rcFrame, 0, 0, nWidth, nHeight );

	/* Create the stream */
	if( AVIERR_OK !=
		AVIFileCreateStream( pFile,		/* File pointer            */
							 psStream,	/* Returned stream pointer */
							 &strhdr )	/* Stream header           */
	  )
		return FALSE;

	return TRUE;

} /* #OF# AVI_CreateStreamVideo */

/*========================================================
Function : AVI_CreateStreamSound
=========================================================*/
/* #FN#
   Opens a video stream for AVI file */
static
BOOL
/* #AS#
   TRUE if succeeded, otherwise FALSE */
AVI_CreateStreamSound(
	PAVIFILE    pFile,       /* #IN# */
	PAVISTREAM *psStream,    /* #OUT# */
	int         nRate,       /* #IN# Sample per second */
	ULONG       nBufferSize, /* #IN# */
	int         nBlockAlign  /* #IN# */
)
{
	AVISTREAMINFO strhdr;

	ZeroMemory( &strhdr, sizeof(strhdr) );
	/* Fill in the stream header for the sound stream */
	strhdr.fccType               = streamtypeAUDIO;
	strhdr.fccHandler            = 0;
	strhdr.dwScale               = 1;//nBlockAlign;
	strhdr.dwRate                = nRate;
	strhdr.dwSuggestedBufferSize = nBufferSize;
	strhdr.dwSampleSize          = 0;//nBlockAlign;

	/* Create the stream */
	if( AVIERR_OK !=
		AVIFileCreateStream( pFile,		/* File pointer            */
							 psStream,	/* Returned stream pointer */
							 &strhdr )	/* Stream header           */
	  )
		return FALSE;

	return TRUE;

} /* #OF# AVI_CreateStreamSound */

/*========================================================
Function : AVI_SetOptionsVideo
=========================================================*/
/* #FN#
   Sets options for a video stream */
static
BOOL
/* #AS#
   TRUE if succeeded, otherwise FALSE */
AVI_SetOptionsVideo(
	PAVISTREAM *psVideo, /* #IN# */
	PAVISTREAM *psCodec, /* #OUT# */
	HWND        hWnd,    /* #IN# */
	BOOL       *pError   /* #OUT# */
)
{
	AVICOMPRESSOPTIONS opts;
	AVICOMPRESSOPTIONS FAR* aopts[ 1 ] = { &opts };
	*pError = FALSE;

	ZeroMemory( &opts, sizeof(opts) );

	if( !AVISaveOptions( hWnd, 0, 1, psVideo, (LPAVICOMPRESSOPTIONS FAR *)&aopts ) )
		return FALSE;

	if( AVIERR_OK !=
		AVIMakeCompressedStream( psCodec, *psVideo, &opts, NULL )
	  )
	{
		*pError = TRUE;
		return FALSE;
	}
	return TRUE;

} /* #OF# AVI_SetOptionsVideo */

/*========================================================
Function : AVI_SetFormatVideo
=========================================================*/
/* #FN#
   Sets frame format for a video stream */
static
BOOL
/* #AS#
   TRUE if succeeded, otherwise FALSE */
AVI_SetFormatVideo(
	PAVISTREAM        *psVideo, /* #IN# */
	LPBITMAPINFOHEADER lpbi     /* #IN# */
)
{
	if( AVIERR_OK !=
		AVIStreamSetFormat( *psVideo, 0,
							 lpbi,	/* Stream format */
							 lpbi->biSize + lpbi->biClrUsed * sizeof(RGBQUAD) )
	  )
		return FALSE;

	return TRUE;

} /* #OF# AVI_SetFormatVideo */

/*========================================================
Function : AVI_SetFormatSound
=========================================================*/
/* #FN#
   Sets frame format for a video stream */
static
BOOL
/* #AS#
   TRUE if succeeded, otherwise FALSE */
AVI_SetFormatSound(
	PAVISTREAM    *psSound, /* #IN# */
	LPWAVEFORMATEX lpwfx    /* #IN# */
)
{
	if( AVIERR_OK !=
		AVIStreamSetFormat( *psSound, 0,
							 lpwfx,	/* Stream format */
							 sizeof(WAVEFORMATEX) )
	  )
		return FALSE;

	return TRUE;

} /* #OF# AVI_SetFormatSound */

/*========================================================
Function : AVI_AddFrame
=========================================================*/
/* #FN#
   Adds a sample to an opened stream */
static
BOOL
/* #AS#
   TRUE if succeeded, otherwise FALSE */
AVI_AddFrame(
	PAVISTREAM psStream,    /* #IN# */
	int        nTime,       /* #IN# */
	UCHAR     *pSampleBits, /* #IN# */
	long       nBufferLen   /* #IN# */
)
{
	if( AVIERR_OK !=
		AVIStreamWrite( psStream,				/* Stream pointer     */
						nTime,					/* Time of this frame */
						1,						/* Number to write    */
						(LPBYTE)pSampleBits,	/* Pointer to data    */
						nBufferLen,				/* Size of this frame */
						AVIIF_KEYFRAME,			/* Flags....          */
						NULL, NULL )
	  )
	{
		return FALSE;
	}
	return TRUE;

} /* #OF# AVI_AddFrame */

/*========================================================
Function : AVI_SaveInfo
=========================================================*/
/* #FN#
   Saves simple information to an AVI file */
static
BOOL
/* #AS#
   Always TRUE */
AVI_SaveInfo(
	PAVIFILE pFile,  /* #IN# */
	char    *pszInfo /* #IN# */
)
{
	int nInfoLen = strlen( pszInfo ) + 1;
	int nHeadLen = 3 * sizeof(DWORD);

	LPSTR psz = (LPSTR)calloc( nHeadLen + nInfoLen, 1 );
	if( psz )
	{
		PDWORD pdw = (PDWORD)psz;
		pdw[ 0 ] = mmioStringToFOURCC( "INFO", 0 );
		pdw[ 1 ] = mmioStringToFOURCC( "ISBJ", 0 ); /* Subject */
		pdw[ 2 ] = nInfoLen;
		_strncpy( psz + nHeadLen, pszInfo, nInfoLen - 1 );

		if( !AVIFileWriteData( pFile, mmioStringToFOURCC( "LIST", 0 ), psz, nHeadLen + nInfoLen ) )
		{
			free( psz );
			return TRUE;
		}
		free( psz );
	}
	return FALSE;

} /* #OF# AVI_SaveInfo */

/*========================================================
Function : AVI_CloseStream
=========================================================*/
/* #FN#
   Closes a video stream */
static
BOOL
/* #AS#
   Always TRUE */
AVI_CloseStream(
	PAVISTREAM *psStream /* #IN# */
)
{
	if( *psStream )
	{
		AVIStreamClose( *psStream );
		*psStream = NULL;
	}
	return TRUE;

} /* #OF# AVI_CloseStream */

/*========================================================
Function : AVI_CloseFile
=========================================================*/
/* #FN#
   Closes an AVI file */
static
BOOL
/* #AS#
   Always TRUE */
AVI_CloseFile(
	PAVIFILE pFile /* #IN# */
)
{
	if( pFile )
		AVIFileRelease( pFile );

	return TRUE;

} /* #OF# AVI_CloseFile */

/*========================================================
Function : AVI_Exit
=========================================================*/
/* #FN#
   Closes a video stream */
static
BOOL
/* #AS#
   Always TRUE */
AVI_Exit( void )
{
	AVIFileExit();

	return TRUE;

} /* #OF# AVI_Exit */

/*========================================================
Function : Video_OpenOutput
=========================================================*/
/* #FN#
   Opens a video file for writing */
void
/* #AS#
   Nothing */
Video_OpenOutput(
	char *pszOutFileName, /* #IN# */
	BOOL  bWithSound
)
{
	if( g_Screen.pfOutput )
		/* Close AVI Output file */
		Video_CloseOutput();

	/* Initialise AVI stuff */
	if( AVI_Init() )
	{
		PAVIFILE pfVideoOutput = NULL;

		if( AVI_FileOpenWrite( &pfVideoOutput, pszOutFileName ) )
		{
			/* Calculate the frame rate */
			int  nFrameRate = (Atari800_TV_PAL == Atari800_tv_mode ? g_Timer.nPalFreq : g_Timer.nNtscFreq) / _GetRefreshRate();
			BOOL bShowMsg   = TRUE;

			struct ScreenInterParms_t dipInfo;
			dipInfo.dwMask = DIP_BITMAPINFO;

			/* Some critical display parameters are not available as global variables */
			Screen_GetInterParms( &dipInfo );

			if( AVI_CreateStreamVideo( pfVideoOutput, &s_psVideo, nFrameRate,
									   dipInfo.pBitmapInfo->bmiHeader.biSizeImage,
									   dipInfo.pBitmapInfo->bmiHeader.biWidth,
									  -dipInfo.pBitmapInfo->bmiHeader.biHeight ) )
			{
				if( AVI_SetOptionsVideo( &s_psVideo, &s_psCodec, g_hMainWnd, &bShowMsg ) )
				{
					bShowMsg = TRUE;

					/* Video compressors do not support bitmap mirroring */
					dipInfo.pBitmapInfo->bmiHeader.biHeight *= -1;

					if( AVI_SetFormatVideo( &s_psCodec, &dipInfo.pBitmapInfo->bmiHeader ) )
					{
						/* Add the sound stream only when sound is not muted */
						if( bWithSound )
						{
							struct SoundInterParms_t sipInfo;
							sipInfo.dwMask = SIP_WAVEFORMAT | SIP_SAMPLESIZE;

							/* Some critical sound parameters are not available as global variables */
							Sound_GetInterParms( &sipInfo );

							if( AVI_CreateStreamSound( pfVideoOutput, &s_psSound, g_Sound.nRate,
													   sipInfo.dwSampleSize,
													   sipInfo.wfxFormat.nBlockAlign ) )
							{
								if( !AVI_SetFormatSound( &s_psSound, &sipInfo.wfxFormat ) )
								{
									AVI_CloseStream( &s_psSound );
								}
							}
							if( !s_psSound )
								DisplayMessage( NULL, IDS_ERROR_VFW_SOUND, 0, MB_ICONEXCLAMATION | MB_OK );
						}
						s_dwVideoFrame = 0;
						s_dwSoundFrame = 0;

						/* Unleash the video recording */
						g_Screen.pfOutput = pfVideoOutput;

						/* Initialization succeeded, exit */
						return;
					}
					dipInfo.pBitmapInfo->bmiHeader.biHeight *= -1;
				}
				AVI_CloseStream( &s_psVideo );
				AVI_CloseStream( &s_psCodec );
			}
			AVI_CloseFile( pfVideoOutput );

			if( bShowMsg )
				DisplayMessage( NULL, IDS_ERROR_VFW_VIDEO, 0, MB_ICONEXCLAMATION | MB_OK );
		}
		else
			DisplayMessage( NULL, IDS_ERROR_FILE_OPEN, 0, MB_ICONEXCLAMATION | MB_OK, pszOutFileName );

		AVI_Exit();
	}
	else
		DisplayMessage( NULL, IDS_ERROR_VFW_INIT, 0, MB_ICONEXCLAMATION | MB_OK );

} /* #OF# Video_OpenOutput */

/*========================================================
Function : Video_SaveFrame
=========================================================*/
/* #FN#
   Saves one frame to the video stream */
void
/* #AS#
   Nothing */
Video_SaveFrame(
	UCHAR *pVideoBits,   /* #IN# */
	long   nVideoBufLen, /* #IN# */
	UCHAR *pSoundBits,   /* #IN# */
	long   nSoundBufLen  /* #IN# */
)
{
	if( g_Screen.pfOutput )
	{
		if( s_psCodec && pVideoBits )
			/* Add a next frame to a video stream */
			AVI_AddFrame( s_psCodec, ++s_dwVideoFrame, pVideoBits, nVideoBufLen );

		if( s_psSound && pSoundBits )
			/* Add a next frame to a sound stream */
			AVI_AddFrame( s_psSound, ++s_dwSoundFrame, pSoundBits, nSoundBufLen );
	}
} /* #OF# Video_SaveFrame */

/*========================================================
Function : Video_CloseOutput
=========================================================*/
/* #FN#
   Closes the video file */
void
/* #AS#
   Nothing */
Video_CloseOutput( void )
{
	if( g_Screen.pfOutput )
	{
		struct ScreenInterParms_t dipInfo;
		dipInfo.dwMask = DIP_BITMAPINFO;

		Screen_GetInterParms( &dipInfo );

		AVI_CloseStream( &s_psVideo );
		AVI_CloseStream( &s_psCodec );
		AVI_CloseStream( &s_psSound );

		AVI_SaveInfo( g_Screen.pfOutput, "Created by Atari800Win PLus" );
		AVI_CloseFile( g_Screen.pfOutput );
		AVI_Exit();

		DisplayMessage( NULL, IDS_GFX_FILE_CLOSED, 0, MB_ICONINFORMATION | MB_OK );

		/* Restore mirroring of the top-down bitmaps */
		dipInfo.pBitmapInfo->bmiHeader.biHeight *= -1;
		g_Screen.pfOutput = NULL;
	}
} /* #OF# Video_CloseOutput */

#ifdef _DEBUG

static
void
CheckAVIError(
	DWORD dwError
)
{
	switch( dwError )
	{
		case AVIERR_UNSUPPORTED:
			_TRACE0("AVIERR_UNSUPPORTED\n");
			break;
		case AVIERR_BADFORMAT:
			_TRACE0("AVIERR_BADFORMAT\n");
			break;
		case AVIERR_MEMORY:
			_TRACE0("AVIERR_MEMORY\n");
			break;
		case AVIERR_INTERNAL:
			_TRACE0("AVIERR_INTERNAL\n");
			break;
		case AVIERR_BADFLAGS:
			_TRACE0("AVIERR_BADFLAGS\n");
			break;
		case AVIERR_BADPARAM:
			_TRACE0("AVIERR_BADPARAM\n");
			break;
		case AVIERR_BADSIZE:
			_TRACE0("AVIERR_BADSIZE\n");
			break;
		case AVIERR_BADHANDLE:
			_TRACE0("AVIERR_BADHANDLE\n");
			break;
		case AVIERR_FILEREAD:
			_TRACE0("AVIERR_FILEREAD\n");
			break;
		case AVIERR_FILEWRITE:
			_TRACE0("AVIERR_FILEWRITE\n");
			break;
		case AVIERR_FILEOPEN:
			_TRACE0("AVIERR_FILEOPEN\n");
			break;
		case AVIERR_COMPRESSOR:
			_TRACE0("AVIERR_COMPRESSOR\n");
			break;
		case AVIERR_NOCOMPRESSOR:
			_TRACE0("AVIERR_NOCOMPRESSOR\n");
			break;
		case AVIERR_READONLY:	
			_TRACE0("AVIERR_READONLY\n");
			break;
		case AVIERR_NODATA:
			_TRACE0("AVIERR_NODATA\n");
			break;
		case AVIERR_BUFFERTOOSMALL:
			_TRACE0("AVIERR_BUFFERTOOSMALL\n");
			break;
		case AVIERR_CANTCOMPRESS:	
			_TRACE0("AVIERR_CANTCOMPRESS\n");
			break;
		case AVIERR_USERABORT:
			_TRACE0("AVIERR_USERABORT\n");
			break;
		case AVIERR_ERROR:
			_TRACE0("AVIERR_ERROR\n");
			break;
	} 
}
#endif /*_DEBUG*/
