/****************************************************************************
File    : Helpers.cpp
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Implementation of usefull methods and objects
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 30.09.2002
*/

#include "StdAfx.h"
#include "Atari800Win.h"
#include "MainFrame.h"
#include "FileService.h"
#include "FileSmallDlg.h"
#include "WarningDlg.h"
#include "Helpers.h"


/////////////////////////////////////////////////////////////////////////////
// Public objects

BOOL g_bLargeFonts = LOWORD(GetDialogBaseUnits()) >= 10;


extern "C" { /* Do not decorate the function name, please */

/*========================================================
Function : InitialiseMachine
=========================================================*/
/* #FN#
   Initialises an emulated machine */
BOOL
/* #AS#
   Non zero if succeeded, otherwise zero */
InitialiseMachine( void )
{
	BOOL bResult = TRUE;

	if( !Atari800_InitialiseMachine() )
	{
		Log_print( "Failed loading specified Atari OS ROM or BASIC!\r\n"
				"Check filename under the 'Atari/ROM images' menu." );

		_SetFlag( g_ulAtariState, ATARI_LOAD_FAILED );

		if( g_hViewWnd )
			InvalidateRect( g_hViewWnd, NULL, FALSE );

		bResult = FALSE;
	}
	/* We also have to initialize R: device (not used by kernel) */
	RDevice_UpdatePatches();

	ReadRegDrives();

	return bResult;

} /* #OF# InitialiseMachine */

/*========================================================
Function : RestartEmulation
=========================================================*/
/* #FN#
   Restarts the emulator (kernel and Windows stuff) */
void
/* #AS#
   Nothing */
RestartEmulation(
	BOOL bIgnoreParams
)
{
	/* Backup an Atari state flags */
	ULONG ulAtariState = g_ulAtariState;
	int argc = 1;

	SIO_Exit();

	/* Reinitialise an emulator kernel */
	Atari800_Initialise( bIgnoreParams ? &argc : &g_argc, bIgnoreParams ? NULL : g_argv );
	Atari_Initialise( bIgnoreParams ? &argc : &g_argc, bIgnoreParams ? NULL : g_argv );

	if( /* Drag&Drop: don't worry about loosing the focus,
		   the emulated Atari won't start in this case */
		ST_ATARI_FAILED || _IsFlagSet( ulAtariState, ATARI_UNINITIALIZED | ATARI_CRASHED ) )
	{
		RECT rc;
		theApp.m_pMainWindow->GetClientRect( &rc );
		rc.left = rc.right / 2; rc.right = rc.left + 1;
		rc.top = rc.bottom / 2; rc.bottom = rc.top + 1;

		/* Clear the screen content */
		theApp.m_pMainWindow->CleanScreen();

		g_hViewWnd = NULL;
		/* Reinitialise Windows stuff */
		theApp.m_pMainWindow->RedrawWindow( &rc, NULL, RDW_INVALIDATE /*RDW_NOFRAME | RDW_NOERASE | RDW_UPDATENOW*/ );
	}
	else
	{
		g_ulAtariState = ulAtariState;
		/* Restart sound if necessary */
		Sound_Restart();
	}
} /* #OF# RestartEmulation */

/*========================================================
Function : HandleResetEvent
=========================================================*/
/* #FN#
   Performs some additional action when the emulated machine is reset */
void
/* #AS#
   Nothing */
HandleResetEvent( void )
{
	/* Turn the cheat mode off */
	ResetCheatServer();

#ifdef WIN_NETWORK_GAMES
	if( !_IsFlagSet( g_Kaillera.ulState, KS_SYNC_GAME ) )
	{
#endif
		if( 0 == BINLOAD_start_binloading )
			strcpy( g_szBinaryFile, FILE_NONE );

		strcpy( g_szSnapshotFile, FILE_NONE );

#ifdef WIN_NETWORK_GAMES
		Kaillera_GameStop();
	}
#endif
} /* #OF# HandleResetEvent */

/*========================================================
Function : UpdateIndicator
=========================================================*/
/* #FN#
   Redraws the pointed icon on the status bar */
void
/* #AS#
   Nothing */
UpdateIndicator(
	int nPane
)
{
	theApp.m_pMainWindow->UpdateIndicator( nPane );
} /* #OF# UpdateIndicator */

/*========================================================
Function : UpdateStatus
=========================================================*/
/* #FN#
   Refreshes descriptions on the caption/status bar */
void
/* #AS#
   Nothing */
UpdateStatus(
	BOOL bForceShow,
	int  nSpeed,
	int  nPane,
	BOOL bWinMode
)
{
	theApp.m_pMainWindow->UpdateStatus( bForceShow, nSpeed, nPane, bWinMode );
} /* #OF# UpdateStatus */

/*========================================================
Function : UpdateWindowTitle
=========================================================*/
/* #FN#
   Refreshes descriptions on the caption without updating the indicators */
void
/* #AS#
   Nothing */
UpdateWindowTitle(
	BOOL bForceShow,
	int  nSpeed,
	int  nPane,
	BOOL bWinMode
)
{
	theApp.m_pMainWindow->UpdateStatus( bForceShow, nSpeed, nPane, bWinMode, FALSE );
} /* #OF# UpdateWindowTitle */

/*========================================================
Function : ResetLoopCounter
=========================================================*/
/* #FN#
   Resets emulator speed-calculation counters */
void
/* #AS#
   Nothing */
ResetLoopCounter(
	BOOL bSetSpeed
)
{
	CAtari800WinApp *pMainApp = (CAtari800WinApp *)AfxGetApp();
	ASSERT(NULL != pMainApp);

	/* Update descriptions on the caption/status bar */
	if( pMainApp )
		pMainApp->ResetLoopCounter( bSetSpeed );

} /* #OF# ResetLoopCounter */

/*========================================================
Function : LoadStringEx
=========================================================*/
/* #FN#
   Loads a string from resources basing on a given string ID */
char*
/* #AS#
   Pointer to a passed output buffer */
LoadStringEx(
	UINT  uStringID,
	char *pszBuffer,
	int   nBufLen
)
{
	if( NULL != pszBuffer )
	{
		if( !LoadString( NULL, uStringID, pszBuffer, nBufLen ) )
			strcpy( pszBuffer, "?" );
	}
	return pszBuffer;

} /* #OF# LoadStringEx */

/*========================================================
Function : ResetCheatServer
=========================================================*/
/* #FN#
   Resets the cheat server internal state */
void
/* #AS#
   Nothing */
ResetCheatServer( void )
{
	CAtari800WinApp *pMainApp = (CAtari800WinApp *)AfxGetApp();
	ASSERT(NULL != pMainApp);

	/* Update descriptions on the caption/status bar */
	if( pMainApp )
	{
		CCheatServer *pCheatServer = pMainApp->GetCheatServer();
		if( pCheatServer )
			pCheatServer->ResetState();
	}
} /* #OF# ResetCheatServer */

/*========================================================
Function : StopStreamRecording
=========================================================*/
/* #FN#
   Stops video or sound stream recording */
static
BOOL
/* #AS#
   TRUE if the video/sound recording was/has been stopped, otherwise FALSE */
StopStreamRecording(
	UINT uStreamType, /* #IN# Stream type */
	UINT uContextID,  /* #IN# Resource ID of string describing reason */
	BOOL bForceStop   /* #IN# Stop forcing flag */
)
{
	int nResult = IDOK;

	BOOL bStreamOpened = FALSE;
	UINT uDontShowFlag = 0;
	UINT uStreamID     = 0;
	UINT uIndicatorID  = 0;
	void (*pfCloseStream)( void ) = NULL;

	switch( uStreamType )
	{
		case SRW_VIDEO_STREAM:
			bStreamOpened = (BOOL)g_Screen.pfOutput;
			uDontShowFlag = DONT_SHOW_RECAVI_WARN;
			uStreamID     = IDS_WARN_RECORD_VIDEO;
			uIndicatorID  = ID_INDICATOR_AVI;
			pfCloseStream = Video_CloseOutput;
			break;
		case SRW_SOUND_STREAM:
			bStreamOpened = (BOOL)g_Sound.pfOutput;
			uDontShowFlag = DONT_SHOW_RECWAV_WARN;
			uStreamID     = IDS_WARN_RECORD_SOUND;
			uIndicatorID  = ID_INDICATOR_WAV;
			pfCloseStream = Sound_CloseOutput;
			break;
		default:
			ASSERT(FALSE);
			return FALSE;
	}

	if( bStreamOpened )
	{
		if( !bForceStop )
		{
			if( !_IsFlagSet( g_Misc.ulDontShow, uDontShowFlag ) )
			{
				CString strWarningText;
				char szStream[ LOADSTRING_SIZE_S + 1 ];
				char szBuffer[ LOADSTRING_SIZE_S + 1 ];

				CWarningDlg	dlgWarning;

				/* Prepare the emulator display for using the GUI */
				UINT uAlloc = Screen_AllocUserScreen();

				ASSERT(uContextID);
				strWarningText.Format( IDS_WARN_RECORD,
									   _LoadStringSx( uContextID, szBuffer ),
									   _LoadStringSx( uStreamID, szStream ),
									   szStream );

				strcpy( dlgWarning.m_szWarningText, strWarningText );
				dlgWarning.m_nWarningFlag = uDontShowFlag;

				Screen_ShowMousePointer( TRUE );
				nResult = dlgWarning.DoModal();

				/* Free the allocated user screen */
				if( IDOK != nResult )
					Screen_FreeUserScreen( uAlloc );
			}
		}
		if( IDOK == nResult )
		{
			/* Close Stream Output file */
			pfCloseStream();
			/* The tray bar icon has to be redrawn */
			UpdateIndicator( uIndicatorID );
		}
	}
	return (IDOK == nResult);

} /* #OF# StopStreamRecording */

/*========================================================
Function : StopVideoRecording
=========================================================*/
/* #FN#
   Stops video stream recording to an AVI file */
BOOL
/* #AS#
   TRUE if the video recording was/has been stopped, otherwise FALSE */
StopVideoRecording(
	UINT uContextID, /* #IN# Resource ID of string describing reason */
	BOOL bForceStop  /* #IN# Stop forcing flag */
)
{
	return StopStreamRecording( SRW_VIDEO_STREAM, uContextID, bForceStop );

} /* #OF# StopVideoRecording */

/*========================================================
Function : StopSoundRecording
=========================================================*/
/* #FN#
   Stops sound stream recording to a WAV file */
BOOL
/* #AS#
   TRUE if the sound recording was/has been stopped, otherwise FALSE */
StopSoundRecording(
	UINT uContextID, /* #IN# Resource ID of string describing reason */
	BOOL bForceStop  /* #IN# Stop forcing flag */
)
{
	return StopStreamRecording( SRW_SOUND_STREAM, uContextID, bForceStop );

} /* #OF# StopSoundRecording */

/*========================================================
Function : StreamWarning
=========================================================*/
/* #FN#
   Stops video and/or sound stream recording */
BOOL
/* #AS#
   TRUE if the video/sound recording was/has been stopped, otherwise FALSE */
StreamWarning(
	UINT uWarningID, /* #IN# Resource ID of warning */
	UINT nFlags      /* #IN# Stream(s) to stop */
)
{
	if( _IsFlagSet( nFlags, SRW_VIDEO_STREAM ) )
		if( !StopVideoRecording( uWarningID, FALSE ) )
			return FALSE;

	if( _IsFlagSet( nFlags, SRW_SOUND_STREAM ) )
		if( !StopSoundRecording( uWarningID, FALSE ) )
			return FALSE;

	return TRUE;

} /* #OF# StreamWarning */

/*========================================================
Function : DisplayWarning
=========================================================*/
/* #FN#
   Creates, displays, and operates a warning box */
BOOL
/* #AS#
   TRUE if the user chose OK button, otherwise FALSE */
DisplayWarning(
	UINT uWarningID,    /* #IN# Text resource id */
	UINT uDontShowFlag,
	BOOL bEnableCancel,
	...
)
{
	va_list argList;

	int nResult = IDOK;

	if( !_IsFlagSet( g_Misc.ulDontShow, uDontShowFlag ) )
	{
		char szFormat [ LOADSTRING_SIZE_L + 1 ];
		char szWarning[ (LOADSTRING_SIZE_L + 1) * 3 ];

		CWarningDlg	dlgWarning;

		/* Prepare the emulator display for using the GUI */
		UINT uAlloc = Screen_AllocUserScreen();

		if( !(uWarningID &&
			LoadString( NULL, uWarningID, szFormat, LOADSTRING_SIZE_L )) )
		{
			strcpy( szFormat, "An internal error has occured while loading resource!" );
		}
		/* Format the message string */
		va_start( argList, bEnableCancel );
		vsprintf( szWarning, szFormat, argList );

		strcpy( dlgWarning.m_szWarningText, szWarning );
		dlgWarning.m_nWarningFlag = uDontShowFlag;
		dlgWarning.m_bCancel      = bEnableCancel;

		Screen_ShowMousePointer( TRUE );
		nResult = dlgWarning.DoModal();

		/* Free the allocated user screen */
		Screen_FreeUserScreen( uAlloc );
	}
	va_end( argList );

	return (IDOK == nResult);

} /* #OF# DisplayWarning */

/*========================================================
Function : DisplayMessage
=========================================================*/
/* #FN#
   Creates, displays, and operates a message box */
int
/* #AS#
   The MessageBox API function return value */
DisplayMessage(
	HWND hWnd,       /* #IN# Parent window     */
	UINT uMessageID, /* #IN# Text resource id  */
	UINT uCaptionID, /* #IN# Title resource id */
	UINT uType,      /* #IN# Message box style */
	...
)
{
	va_list argList;

	char szFormat [ LOADSTRING_SIZE_L + 1 ];
	char szCaption[ LOADSTRING_SIZE_S + 1 ];
	char szMessage[ (LOADSTRING_SIZE_L + 1) * 3 ];
	int  nResult;

	/* Prepare the emulator display for using the GUI */
	UINT uAlloc = Screen_AllocUserScreen();

	if( !(uMessageID &&
		LoadString( NULL, uMessageID, szFormat, LOADSTRING_SIZE_L )) )
	{
		strcpy( szFormat, "An internal error has occured while loading resource!" );
	}
	/* Format the message string */
	va_start( argList, uType );
	vsprintf( szMessage, szFormat, argList );

	if( !(uCaptionID &&
		LoadString( NULL, uCaptionID, szCaption, LOADSTRING_SIZE_S )) )
	{
		strcpy( szCaption, "Atari800Win PLus" );
	}
	if( !uType )
		uType = MB_ICONSTOP | MB_OK;

	Screen_ShowMousePointer( TRUE );

	/* Display the message box */
	nResult = MessageBox( (hWnd ? hWnd : g_hMainWnd), szMessage, szCaption, uType );

	/* Free the allocated user screen */
	Screen_FreeUserScreen( uAlloc );

	va_end( argList );

	return nResult;

} /* #OF# DisplayMessage */

} //extern "C"
