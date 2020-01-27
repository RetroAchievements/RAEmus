/****************************************************************************
File    : kaillera.c
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Implementation of Kaillera related stuff (network games)
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 26.10.2002
*/

#include "WinConfig.h"

#ifdef WIN_NETWORK_GAMES

#include <windows.h>
#include <crtdbg.h>
#include "Resource.h"
#include "Debug.h"
#include "Helpers.h"
#include "FileService.h"
#include "atari800.h"
#include "globals.h"
#include "macros.h"
#include "display_win.h"
#include "misc_win.h"
#include "input_win.h"
#include "kaillera.h"

/* Definition of constants */

#define KA_CLIENT_VERSION		"0.9"

#define MIN_PLAYER_NO			1
#define MAX_PLAYER_NO			4

#define KA_CTRL_PORT			0x00000001 /* Information bits */
#define KA_CTRL_CONSOLE			0x00000002
#define KA_CTRL_STICK			0x00000004
#define KA_CTRL_KEY				0x00000008
#define KA_CTRL_SHIFT			0x00000010
#define KA_CTRL_BREAK			0x00000020
#define KA_CTRL_TRIG			0x00000040
#define KA_CTRL_MASK			0x0000007f

#define KA_DATA_PORT			0x00000180 /* Input data bits */
#define KA_DATA_CONSOLE			0x00000e00
#define KA_DATA_STICK			0x0000f000
#define KA_DATA_KEY				0x00ff0000
#define KA_DATA_SHIFT			0x01000000
#define KA_DATA_BREAK			0x02000000
#define KA_DATA_TRIG			0x04000000
#define KA_DATA_MASK			0x07ffff80

#define NUMBER_OF_EVENTS		3
#define GET_EVENT				0
#define END_EVENT				1
#define PUT_EVENT				2

#define	SYNC_VALUE				0x5f05f05f	/* For synchronization purpose */
#define	SYNC_LIMIT				200

#define NO_SHIFT				0
#define NO_BREAK				0
#define NO_TRIG					1

#define KA_DLG1_TITLE			"Kaillera Client v"
#define KA_DLG2_TITLE			"Kaillera v"
#define KA_DLG3_TITLE			"Enter Server IP address..."
#define KA_DLG4_TITLE			"About Kaillera..."
#define KA_BTN1_LABEL			"Start game"

/* Public objects */

struct KailleraCtrl_t g_Kaillera =
{
	DEF_KAILLERA_STATE,
	"",
	0,
	DEF_KAILLERA_LOCAL_PORT,
	DEF_KAILLERA_FRAME_SKIP
};

/* Private objects */

static HANDLE s_hKailleraClient = NULL;

static BOOL   s_bInitialized    = FALSE;
static BOOL   s_bRunningGame    = FALSE;
static BOOL   s_bNeedSync       = TRUE;
static int    s_nNumPlayers     = 0;
static int    s_nPlayerNo       = -1;
static int    s_nFrameCount     = 0;
static int    s_nRecvPlayValues = 0;

static volatile BOOL s_bStartThread = FALSE; /* Is the thread running? (used by spinlock) */

static HANDLE s_hStartThread    = NULL;  /* Thread kernel object */
static DWORD  s_dwStartThreadId = 0;
static BOOL   s_bFrameThread    = FALSE;
static HANDLE s_hFrameThread    = NULL;
static DWORD  s_dwFrameThreadId = 0;

/* We spawn a special thread that starts a network game, but the
   main thread is still allowed to finish the game. So, we need
   critical section to ensure that the threads will stay friends.
*/
CRITICAL_SECTION s_csGameControl;

static ULONG  s_anPlayValues[ MAX_PLAYER_NO ];	/* Kaillera buffer */

static HANDLE s_arrEvents[ NUMBER_OF_EVENTS ];
static char   s_szVersion[ 16 ];

static int    GetTargetPort  ( ULONG nValue );
static void   SetTargetPort  ( int nPort, PULONG pValue );

static ULONG  GetJoystickMask( int nPort );
static void   SetJoystickMask( ULONG nValue, int nPort );
static ULONG  GetKeyboardMask( void );
static void   SetKeyboardMask( ULONG nValue );

/* Kailleraclient starters */

static FARPROC s_pfnGetVersion         = NULL;
static FARPROC s_pfnInit               = NULL;
static FARPROC s_pfnShutDown           = NULL;
static FARPROC s_pfnSetInfos           = NULL;
static FARPROC s_pfnSelectServerDialog = NULL;
static FARPROC s_pfnModifyPlayValues   = NULL;
static FARPROC s_pfnChatSend           = NULL;
static FARPROC s_pfnEndGame            = NULL;

/* Wrappers for Kaillera exports */

static int KAGETVERSION        ( char *pszVersion );
static int KAINIT              ( void );
static int KASHUTDOWN          ( void );
static int KASETINFOS          ( kailleraInfos *pInfos );
static int KASELECTSERVERDIALOG( HWND hParent );
static int KAMODIFYPLAYVALUES  ( void *pValues, int nSize );
static int KACHATSEND          ( char *pszText );
static int KAENDGAME           ( void );

/* Macro-definitions */

#define _GetEvent	s_arrEvents[ GET_EVENT ]
#define _EndEvent	s_arrEvents[ END_EVENT ]
#define _PutEvent	s_arrEvents[ PUT_EVENT ]

#define _SetCtrl( ctrl, value )			(value |= (ctrl & 0x7f))
#define _GetCtrl( value )				(value & KA_CTRL_MASK)
#define _TstCtrl( ctrl, value )			(0 != (_GetCtrl( value ) & ctrl))

#define _SetPort( port, value )			(value |= ((port & 0x03) << 7))
#define _GetPort( value )				((value & KA_DATA_PORT) >> 7)
#define _SetConsole( console, value )	(value |= ((console & 0x07) << 9))
#define _GetConsole( value )			((value & KA_DATA_CONSOLE) >> 9)
#define _SetStick( stick, value )	 	(value |= ((stick & 0x0f) << 12))
#define _GetStick( value )				((value & KA_DATA_STICK) >> 12)
#define _SetKey( key, value )			(value |= ((key & 0xff) << 16))
#define _GetKey( value )				((value & KA_DATA_KEY) >> 16)
#define _SetShift( shift, value )		(value |= ((shift & 0x01) << 24))
#define _GetShift( value )				((value & KA_DATA_SHIFT) >> 24)
#define _SetBreak( braek, value )		(value |= ((braek & 0x01) << 25))
#define _GetBreak( value )				((value & KA_DATA_BREAK) >> 25)
#define _SetTrig( trig, value )			(value |= ((trig & 0x01) << 26))
#define _GetTrig( value )				((value & KA_DATA_TRIG) >> 26)

#define _KailleraReset()				SendMessage( g_hMainWnd, WM_PLUS_KARESET, 0, 0 )
#define _KailleraStart( filetype )		SendMessage( g_hMainWnd, WM_PLUS_KASTART, (WPARAM)filetype, 0 )
#define _KailleraStop()					SendMessage( g_hMainWnd, WM_PLUS_KASTOP, 0, 0 )
#define _KailleraGameStart()			PostMessage( g_hMainWnd, WM_PLUS_KAGAMESTART, 0, 0 )
#define _KailleraGameStop()				PostMessage( g_hMainWnd, WM_PLUS_KAGAMESTOP, 0, 0 )

/* Private data types */

typedef enum { eDialog1, eDialog2, eDialog3, eDialog4 } DialogType;


/*========================================================
Function : ExitStartThread
=========================================================*/
/* #FN#
   Releases the 'start thread' kernel objects */
static
void
/* #AS#
   Nothing */
ExitStartThread( void )
{
	if( !s_bStartThread && s_hStartThread )
	{
		CloseHandle( s_hStartThread );
		s_hStartThread    = NULL;
		s_dwStartThreadId = 0;
	}
} /* #OF# ExitStartThread */

/*========================================================
Function : ExitFrameThread
=========================================================*/
/* #FN#
   Releases the 'frame thread' kernel objects */
static
void
/* #AS#
   Nothing */
ExitFrameThread( void )
{
	if( s_bFrameThread )
	{
		if( s_hFrameThread )
		{
			DWORD dwExitCode = 0;
			/* Stop and kill Kaillera frame thread */
			SetEvent( _EndEvent );
			/* Wait for thread termination */
			do
			{
				Sleep( 1 ); /* Wait a moment, please */
				GetExitCodeThread( s_hFrameThread, &dwExitCode );
			}
			while( STILL_ACTIVE == dwExitCode );

			CloseHandle( s_hFrameThread );
			s_hFrameThread    = NULL;
			s_dwFrameThreadId = 0;

		}
		s_bFrameThread = FALSE;
	}
	if( _GetEvent )
	{
		CloseHandle( _GetEvent );
		_GetEvent = NULL;
	}
	if( _EndEvent )
	{
		CloseHandle( _EndEvent );
		_EndEvent = NULL;
	}
	if( _PutEvent )
	{
		CloseHandle( _PutEvent );
		_PutEvent = NULL;
	}
} /* #OF# ExitFrameThread */

/*========================================================
Function : KailleraStartThreadProc
=========================================================*/
/* #FN#
   A separate thread procedure for Kaillera "Select server" window */
static
DWORD WINAPI
/* #AS#
   Thread return code (success or failure) */
KailleraStartThreadProc(
	LPVOID lpParam
)
{
	Kaillera_SetInfo( (char *)lpParam );
	Kaillera_SelectServer();

	ExitFrameThread(); /* Terminate Kaillera frame helper thread */
	s_bStartThread = FALSE;

	/* Inform the main window the Kaillera dialog has been closed */
	_KailleraStop();

	return 0;

} /* #OF# KailleraStartThreadProc */

/*========================================================
Function : KailleraFrameThreadProc
=========================================================*/
/* #FN#
   A separate thread procedure for talking to Kaillera */
static
DWORD WINAPI
/* #AS#
   Thread return code (success or failure) */
KailleraFrameThreadProc(
	LPVOID lpParam
)
{
	while( END_EVENT + WAIT_OBJECT_0 !=
		   WaitForMultipleObjects( NUMBER_OF_EVENTS - 1, s_arrEvents, FALSE, INFINITE ) )
	{
		s_nRecvPlayValues = KAMODIFYPLAYVALUES( (void *)s_anPlayValues, sizeof(s_anPlayValues[ 0 ]) );
		SetEvent( _PutEvent );
	}
	return 0;

} /* #OF# KailleraFrameThreadProc */

/*========================================================
Function : ModifyPlayValues
=========================================================*/
/* #FN#
   Schedules kailleraModifyPlayValues function to spread the input state
   and wait until input values from other players are received */
static
int
/* #AS#
   kailleraModifyPlayValues return value */
ModifyPlayValues( void )
{
	DWORD dwEventResult = 0;

	SetEvent( _GetEvent );
	dwEventResult = WaitForSingleObject( _PutEvent, 10000 );

	if( WAIT_TIMEOUT == dwEventResult || WAIT_FAILED == dwEventResult )
	{
		_TRACE1("!-[%d] ModifyPlayValues::WAIT_TIMEOUT || WAIT_FAILED\n", MEMORY_mem[ 20 ]);
		return -1;
	}
	return s_nRecvPlayValues;

} /* #OF# ModifyPlayValues */

/*========================================================
Function : GetKailleraDlgHandle
=========================================================*/
/* #FN#
   Returns handle to one of the Kaillera dialog windows */
static
HWND
/* #AS#
   Handle to the dialog window */
GetKailleraDlgHandle(
	DialogType eDlgType
)
{
	char szTitle[ 64 ];

	switch( eDlgType )
	{
		case eDialog1:
			strcpy( szTitle, KA_DLG1_TITLE );
			strcat( szTitle, s_szVersion );
			break;
		case eDialog2:
			strcpy( szTitle, KA_DLG2_TITLE );
			strcat( szTitle, s_szVersion );
			break;
		case eDialog3:
			strcpy( szTitle, KA_DLG3_TITLE );
			break;
		case eDialog4:
			strcpy( szTitle, KA_DLG4_TITLE );
			break;
	}
	return Misc_FindWindow( szTitle, s_dwStartThreadId );

} /* #OF# GetKailleraDlgHandle */

/*========================================================
Function : ModifyDialogs
=========================================================*/
/* #FN#
   Enables or disables the CLOSE menu item of Kaillera dialogs */
static
void
/* #AS#
   Nothing */
ModifyDialogs(
	BOOL bGameActive
)
{
	/* Modify the first Kaillera dialog (that allows connecting to a server) */
	HWND hWnd = GetKailleraDlgHandle( eDialog1 );
	if( NULL != hWnd )
	{
		Misc_EnableCloseItem( hWnd, !bGameActive );
	}
	/* Modify the second Kaillera dialog (that allows starting a network game) */
	hWnd = GetKailleraDlgHandle( eDialog2 );
	if( NULL != hWnd )
	{
		Misc_EnableCloseItem( hWnd, !bGameActive );
		hWnd = Misc_FindChildWindow( hWnd, KA_BTN1_LABEL );
		if( NULL != hWnd )
		{
			EnableWindow( hWnd, !bGameActive );
		}
	}
} /* #OF# ModifyDialogs */

/*========================================================
Function : GameCallback
=========================================================*/
/* #FN#
   Called when a new game starts */
static
int
WINAPI
/* #AS#
   0 if succeeded, otherwise 1 */
GameCallback(
	char *pszGame,
	int   nPlayerNo,
	int   nNumPlayers
)
{
	int nResult = 0;

	EnterCriticalSection( &s_csGameControl );

	if( !s_bRunningGame )
	{
		if( nPlayerNo < MIN_PLAYER_NO || nPlayerNo > MAX_PLAYER_NO )
		{
			DisplayMessage( NULL, IDS_KAERR_PLAYER_NO, IDS_KAERR_HDR, MB_ICONSTOP | MB_OK, nPlayerNo );
			nResult = 1;
		}
		else
		{
			_TRACE1( "Starting network game: %s\n", pszGame );
			_TRACE1( "Number of players: %d\n", nNumPlayers );
			_TRACE2( "Player no: %d (port: %d)\n", nPlayerNo, g_Kaillera.nLocalPort + 1 );

			Log_print( "Starting network game: %s", pszGame );
			Log_print( "Number of players: %d", nNumPlayers );
			Log_print( "Player no: %d (port: %d)", nPlayerNo, g_Kaillera.nLocalPort + 1 );

			s_nPlayerNo   = nPlayerNo - 1;
			s_nNumPlayers = nNumPlayers;

			ModifyDialogs( TRUE );

			/* Start the emulation */
			s_bNeedSync    = TRUE;
			s_bRunningGame = TRUE; /* A network game is about to be activated */

			/* Inform the main window the network game has been started */
			_KailleraGameStart();
		}
	}
	LeaveCriticalSection( &s_csGameControl );

	return nResult;

} /* #OF# GameCallback */

/*========================================================
Function : ClientDroppedCallback
=========================================================*/
/* #FN#
   Called when a client drops from the current game */
static
void
WINAPI
/* #AS#
   Nothing */
ClientDroppedCallback(
	char *pszNick,
	int   nPlayerNo
)
{
	Log_print( "Network game has been stopped (player %d dropped)", nPlayerNo );
	Kaillera_GameStop();

} /* #OF# ClientDroppedCallback */

/*========================================================
Function : ChatReceivedCallback
=========================================================*/
/* #FN#
   Called when a chat line text has been received */
static
void
WINAPI
/* #AS#
   Nothing */
ChatReceivedCallback(
	char *pszNick,
	char *pszText
)
{
	_TRACE2("ChatReceivedCallback::%s: %s\n", pszNick, pszText);

} /* #OF# ChatReceivedCallback */

/*========================================================
Function : MoreInfosCallback
=========================================================*/
/* #FN#
   Called when the user selects "More infos about this game..."
   in the game list context menu */
static
void
WINAPI
/* #AS#
   Nothing */
MoreInfosCallback(
	char *pszGameName
)
{
	_TRACE1("MoreInfosCallback: %s\n", pszGameName);

} /* #OF# MoreInfosCallback */

/*========================================================
Function : Kaillera_Initialise
=========================================================*/
/* #FN#
   */
void
/* #AS#
   Nothing */
Kaillera_Initialise( void )
{
	if( !s_bInitialized )
	{
		InitializeCriticalSection( &s_csGameControl );

		KAINIT();
		Kaillera_GetVersion( s_szVersion );

		/* Print out the version of Kaillera client */
		Log_print( "Kaillera version: %s", s_szVersion );

		if( strcmp( KA_CLIENT_VERSION, s_szVersion ) != 0 )
		{
			DisplayWarning( IDS_WARN_KAVERSION, DONT_SHOW_KAVERSION_WARN, FALSE, s_szVersion, KA_CLIENT_VERSION );
		}
		s_bInitialized = TRUE;
	}
} /* #OF# Kaillera_Initialise */

/*========================================================
Function : Kaillera_Exit
=========================================================*/
/* #FN#
   */
void
/* #AS#
   Nothing */
Kaillera_Exit( void )
{
	if( s_bInitialized )
	{
		Kaillera_GameStop();

		/* Close the Kaillera dialog windows if any */
		if( s_bStartThread )
		{
			DWORD dwResult = 0;
			HWND  hWnd     = NULL;
			int   nDelay   = 1000;
			int   i;

			int aDialogs[] = { eDialog4, eDialog3, eDialog2, eDialog1 };

			for( i = 0; i < sizeof(aDialogs)/sizeof(aDialogs[0]); i++ )
			{
				if( NULL != (hWnd = GetKailleraDlgHandle( aDialogs[ i ] )) )
					SendMessageTimeout( hWnd, WM_CLOSE, 0, 0, SMTO_ABORTIFHUNG, 1000, &dwResult );
			}
			while( s_bStartThread && nDelay-- )
				Sleep( 1 );
		}
		/* Clean up the Kaillera client stuff */
		KASHUTDOWN();

		s_bInitialized = FALSE;

		ExitFrameThread();
		ExitStartThread();

		DeleteCriticalSection( &s_csGameControl );
	}
} /* #OF# Kaillera_Exit */

/*========================================================
Function : Kaillera_GameStart
=========================================================*/
/* #FN#
   */
void
/* #AS#
   Nothing */
Kaillera_GameStart( void )
{
	if( s_bInitialized )
	{
		if( !s_bFrameThread )
		{
			ExitFrameThread();

			if( (_GetEvent = CreateEvent( NULL, FALSE, FALSE, NULL )) &&
				(_EndEvent = CreateEvent( NULL, FALSE, FALSE, NULL )) &&
				(_PutEvent = CreateEvent( NULL, FALSE, FALSE, NULL )) )
			{
				/* Launch a helper thread */
				if( s_hFrameThread = _CreateThreadEx( NULL,
													  0,
													  KailleraFrameThreadProc,
													  (LPVOID)NULL,
													  0,
													  &s_dwFrameThreadId ) )
				{
					s_bFrameThread = TRUE;
				}
			}
		}
		if( s_bFrameThread && !s_bStartThread )
		{
			ExitStartThread();

			/* Launch the main Kaillera thread */
			if( s_hStartThread = _CreateThreadEx( NULL,
												  0,
												  KailleraStartThreadProc,
												  (LPVOID)g_Kaillera.szGameName,
												  0,
												  &s_dwStartThreadId ) )
			{
				s_bStartThread = TRUE;
			}
		}
		if( !s_bStartThread || !s_bFrameThread )
		{
			ExitFrameThread();
			ExitStartThread(); /* Not really necessary */

			DisplayMessage( NULL, IDS_KAERR_GAME_START, IDS_KAERR_HDR, MB_ICONSTOP | MB_OK );
		}
		else
			/* Inform the main window the Kaillera dialog has been shown */
			_KailleraStart( g_Kaillera.unBootImage );
	}
} /* #OF# Kaillera_GameStart */

/*========================================================
Function : Kaillera_GameStop
=========================================================*/
/* #FN#
   */
void
/* #AS#
   Nothing */
Kaillera_GameStop( void )
{
	if( s_bInitialized ) /* Critical section has to be initialized before using */
	{
		EnterCriticalSection( &s_csGameControl );

		if( s_bRunningGame )
		{
			s_bRunningGame = FALSE;

			KAENDGAME();

			/* Inform the main window the network game has been finished */
			_KailleraGameStop();

			ModifyDialogs( FALSE );
		}
		LeaveCriticalSection( &s_csGameControl );
	}
} /* #OF# Kaillera_GameStop */

/*========================================================
Function : Kaillera_GetVersion
=========================================================*/
/* #FN#
   */
void
/* #AS#
   Nothing */
Kaillera_GetVersion(
	char *pBuffer
)
{
	KAGETVERSION( pBuffer );

} /* #OF# Kaillera_GetVersion */

/*========================================================
Function : Kaillera_SelectServer
=========================================================*/
/* #FN#
   */
void
/* #AS#
   Nothing */
Kaillera_SelectServer( void )
{
	KASELECTSERVERDIALOG( NULL /*g_hMainWnd*/ );

} /* #OF# Kaillera_SelectServer */
 
/*========================================================
Function : Kaillera_SetInfo
=========================================================*/
/* #FN#
   */
void
/* #AS#
   Nothing */
Kaillera_SetInfo(
	char *pszGameName
)
{
	kailleraInfos kaInfo =
	{
		VERSION_INFO,
		pszGameName,
		GameCallback,
		NULL, //ChatReceivedCallback,
		ClientDroppedCallback,
		NULL  //MoreInfosCallback
	};

	KASETINFOS( &kaInfo );

} /* #OF# Kaillera_SetInfo */

/*========================================================
Function : Kaillera_Frame
=========================================================*/
/* #FN#
   Records/bufferizes the values the player send in and sets state of
   input of the emulator basing on values received from other players */
void
/* #AS#
   Nothing */
Kaillera_Frame( void )
{
	if( s_bRunningGame )
	{
		int nRecv = 0;

		/* We have to synchronize the connection and set the random counter to
		   the same value for all the players when starting the network game */
		if( s_bNeedSync )
		{
			int nSyncLimit = SYNC_LIMIT;
			s_bNeedSync = FALSE;

			_SetFlag( g_Kaillera.ulState, KS_SYNC_GAME );

			/* Do reset of the emulated machine
			*/
			_KailleraReset();

			/* Synchronize all the players with the game master's host
			*/
			ZeroMemory( s_anPlayValues, sizeof(s_anPlayValues) );
			/* Wait for a special sync value from master of the game */
			do
			{
				s_anPlayValues[ 0 ] = (0 == s_nPlayerNo) ? SYNC_VALUE : 0 /* Don't send anything, just receive */;
				nRecv = KAMODIFYPLAYVALUES( (void *)s_anPlayValues, sizeof(s_anPlayValues[ 0 ]) );
			}
			while( 0 != s_nPlayerNo && SYNC_VALUE != s_anPlayValues[ 0 ] && --nSyncLimit );

			if( nSyncLimit > 0 )
			{
				ULONG nRandCount = POKEY_GetRandomCounter(); /* Backup the random counter value */

				/* Broadcast the master's random counter value */
				s_anPlayValues[ 0 ] = (0 == s_nPlayerNo) ? nRandCount : 0;
				nRecv = KAMODIFYPLAYVALUES( (void *)s_anPlayValues, sizeof(s_anPlayValues[ 0 ]) );

				if( nRecv > 0 )
				{
					if( 0 != s_nPlayerNo )
					{
						POKEY_SetRandomCounter( s_anPlayValues[ 0 ] );
					}
					ANTIC_xpos = 0;
				}
				if( 0 == s_nPlayerNo )
				{
					nSyncLimit = SYNC_LIMIT;
					/* Wait for the random counter value comming back */
					do
					{
						s_anPlayValues[ 0 ] = 0; /* Don't send anything, just receive */
						nRecv = KAMODIFYPLAYVALUES( (void *)s_anPlayValues, sizeof(s_anPlayValues[ 0 ]) );
					}
					while( nRandCount != s_anPlayValues[ 0 ] && --nSyncLimit );
				}
			}
			if( 0 == nSyncLimit || -1 == nRecv ) /* Player no more in the game */
			{
				Kaillera_GameStop();
			}
			else
			{
				s_nFrameCount = 0;
				/* If we didn't reset the put event here, it wouldn't be possible
				   to start a new network game after breaking the previous one */
				ResetEvent( _PutEvent );
			}
			_ClrFlag( g_Kaillera.ulState, KS_SYNC_GAME );
		}
		else
		{
			BOOL bSkipFrame = FALSE;

			if( _IsFlagSet( g_Kaillera.ulState, KS_SKIP_FRAMES ) )
			{
				bSkipFrame = 0 != s_nFrameCount % (g_Kaillera.nFrameSkip + 1);
				s_nFrameCount++; /* We must not skip the first frame! */
			}
			if( !bSkipFrame )
			{
				ZeroMemory( s_anPlayValues, sizeof(s_anPlayValues) );

				/* Set information about the target port */
				SetTargetPort( g_Kaillera.nLocalPort, &s_anPlayValues[ 0 ] );

				if( 0 == s_nPlayerNo )
					s_anPlayValues[ 0 ] |= GetKeyboardMask();

				s_anPlayValues[ 0 ] |= GetJoystickMask( g_Kaillera.nLocalPort );

				/* Spread the emulator input flags and receive ones from other players */
				nRecv = ModifyPlayValues();
			}
			if( nRecv > 0 || bSkipFrame )
			{
				int nTargetPort, i;
				/* The first player (game creator) has an exclusive control over the keyboard */
				SetKeyboardMask( s_anPlayValues[ 0 ] );

				for( i = 0; i < MAX_ATARI_JOYPORTS; i++ )
				{
					SetJoystickMask( 0, i ); /* We must depend on Kaillera entirely */
				}
				for( i = 0; i < s_nNumPlayers; i++ )
				{
					SetJoystickMask( s_anPlayValues[ i ], nTargetPort = GetTargetPort( s_anPlayValues[ i ] ) );

					if( i != s_nPlayerNo && g_Kaillera.nLocalPort == nTargetPort )
					{
						/* At least two players have selected the same joystick ports as their local ports */
						DisplayMessage( NULL, IDS_KAERR_PORT_NO, IDS_KAERR_HDR, MB_ICONWARNING | MB_OK, i + 1 );
						nRecv = -1;
					}
				}
			}
			if( -1 == nRecv ) /* Player no more in the game */
			{
				Kaillera_GameStop();
			}
		}
	}
} /* #OF# Kaillera_Frame */

/*========================================================
Function : Kaillera_IsGameActive
=========================================================*/
/* #FN#
   Checks if a network game is active */
BOOL
/* #AS#
   TRUE if a network game is active, otherwise FALSE */
Kaillera_IsGameActive( void )
{
	return s_bRunningGame;

} /* #OF# Kaillera_IsGameActive */

/*========================================================
Function : Kaillera_IsDlgActive
=========================================================*/
/* #FN#
   Checks if a Kaillera dialog window is open */
BOOL
/* #AS#
   TRUE if a Kaillera dialog window is open, otherwise FALSE */
Kaillera_IsDlgActive( void )
{
	return s_bStartThread;

} /* #OF# Kaillera_IsDlgActive */

/*========================================================
Function : Kaillera_GetPlayerNo
=========================================================*/
/* #FN#
   Returns the Kaillera player number */
int
/* #AS#
   Number of the player */
Kaillera_GetPlayerNo( void )
{
	return s_nPlayerNo;

} /* #OF# Kaillera_GetPlayerNo */

/*========================================================
Function : GetTargetPort
=========================================================*/
/* #FN#
   Reads target port number from a received value */
static
int
/* #AS#
   Target port number */
GetTargetPort(
	ULONG nValue /* #IN# Portion of Kaillera buffer */
)
{
	return (_TstCtrl( nValue, KA_CTRL_PORT ) ? _GetPort( nValue ) : -1);

} /* #OF# GetTargetPort */

/*========================================================
Function : SetTargetPort
=========================================================*/
/* #FN#
   Writes target port number into a value to send */
static
void
/* #AS#
   Nothing */
SetTargetPort(
	int    nPort, /* #IN# Stick port number */
	PULONG pValue
)
{
	_SetPort( nPort, *pValue );
	_SetCtrl( KA_CTRL_PORT, *pValue ); 

} /* #OF# SetTargetPort */

/*========================================================
Function : GetJoystickMask
=========================================================*/
/* #FN#
   Gets joystick state bits */
static
ULONG
/* #AS#
   Joystick state bits */
GetJoystickMask(
	int nPort /* #IN# Stick port number */
)
{
	ULONG nValue = 0;

	if( INPUT_STICK_CENTRE != Input_GetStick( nPort ) )
	{
		_SetStick( Input_GetStick( nPort ), nValue );
		_SetCtrl( KA_CTRL_STICK, nValue ); 
	}
	if( NO_TRIG != Input_GetTrig( nPort ) )
	{
		_SetTrig( Input_GetTrig( nPort ), nValue ); /* Not really necessary */
		_SetCtrl( KA_CTRL_TRIG, nValue );
	}
	return nValue;

} /* #OF# GetJoystickMask */

/*========================================================
Function : SetJoystickMask
=========================================================*/
/* #FN#
   Sets joystick state bits */
static
void
/* #AS#
   Nothing */
SetJoystickMask(
	ULONG nValue, /* #IN# Portion of Kaillera buffer */
	int   nPort   /* #IN# Stick port number */
)
{
	if( -1 != nPort )
	{
		Input_SetStick( nPort, _TstCtrl( nValue, KA_CTRL_STICK ) ? _GetStick( nValue ) : INPUT_STICK_CENTRE );
		Input_SetTrig ( nPort, _TstCtrl( nValue, KA_CTRL_TRIG  ) ? _GetTrig ( nValue ) /* Not really necessary */ : NO_TRIG );
	}
} /* #OF# SetJoystickMask */

/*========================================================
Function : GetKeyboardMask
=========================================================*/
/* #FN#
   Gets keys state bits */
static
ULONG
/* #AS#
   Regular keys state bits */
GetKeyboardMask( void )
{
	ULONG nValue = 0;

	if( INPUT_CONSOL_NONE != g_Input.Key.nConsol )
	{
		_SetConsole( g_Input.Key.nConsol, nValue );
		_SetCtrl( KA_CTRL_CONSOLE, nValue ); 
	}
	if( AKEY_NONE != g_Input.Key.nCurrentKey )
	{
		_SetKey( g_Input.Key.nCurrentKey, nValue );
		_SetCtrl( KA_CTRL_KEY, nValue ); 
	}
	if( NO_SHIFT != g_Input.Key.nShift )
	{
		_SetShift( g_Input.Key.nShift, nValue ); /* Not really necessary */
		_SetCtrl( KA_CTRL_SHIFT, nValue ); 
	}
	return nValue;

} /* #OF# GetKeyboardMask */

/*========================================================
Function : SetKeyboardMask
=========================================================*/
/* #FN#
   Sets keys state bits */
static
void
/* #AS#
   Nothing */
SetKeyboardMask(
	ULONG nValue /* #IN# Portion of Kaillera buffer */
)
{
	INPUT_key_consol = _TstCtrl( nValue, KA_CTRL_CONSOLE ) ? _GetConsole( nValue ) : INPUT_CONSOL_NONE;
	INPUT_key_code   = _TstCtrl( nValue, KA_CTRL_KEY     ) ? _GetKey    ( nValue ) : AKEY_NONE;
	INPUT_key_shift  = _TstCtrl( nValue, KA_CTRL_SHIFT   ) ? _GetShift  ( nValue ) /* Not really necessary */ : NO_SHIFT;

} /* #OF# SetKeyboardMask */

/*========================================================
Function : Kaillera_LoadLibrary
=========================================================*/
/* #FN#
   Loads dynamically KailleraClient.dll library */
int
/* #AS#
   0 if suceeded, otherwise a value greater than 0 */
Kaillera_LoadLibrary(
	PHANDLE pHandle
)
{
	int nResult = 0;

	/* Load KAILLERACLIENT library */
	if( !s_hKailleraClient )
	{
		s_hKailleraClient = LoadLibrary( "KAILLERACLIENT.DLL" );
		if( !s_hKailleraClient )
			nResult = KA_ERROR_LOAD;
		else
		{
			s_pfnGetVersion         = GetProcAddress( s_hKailleraClient, "_kailleraGetVersion@4"         );
			s_pfnInit               = GetProcAddress( s_hKailleraClient, "_kailleraInit@0"               );
			s_pfnShutDown           = GetProcAddress( s_hKailleraClient, "_kailleraShutdown@0"           );
			s_pfnSetInfos           = GetProcAddress( s_hKailleraClient, "_kailleraSetInfos@4"           );
			s_pfnSelectServerDialog = GetProcAddress( s_hKailleraClient, "_kailleraSelectServerDialog@4" );
			s_pfnModifyPlayValues   = GetProcAddress( s_hKailleraClient, "_kailleraModifyPlayValues@8"   );
			s_pfnChatSend           = GetProcAddress( s_hKailleraClient, "_kailleraChatSend@4"           );
			s_pfnEndGame            = GetProcAddress( s_hKailleraClient, "_kailleraEndGame@0"            );

			if( !s_pfnGetVersion         ||
				!s_pfnInit               ||
				!s_pfnShutDown           ||
				!s_pfnSetInfos           ||
				!s_pfnSelectServerDialog ||
				!s_pfnModifyPlayValues   ||
				!s_pfnChatSend           ||
				!s_pfnEndGame )
			{
				FreeLibrary( s_hKailleraClient );
				s_hKailleraClient = NULL;
				nResult = KA_ERROR_FUNC;
			}
		}
	}
	if( NULL != pHandle )
		pHandle = s_hKailleraClient;

	return nResult;

} /* #OF# Kaillera_LoadLibrary */

/*========================================================
Function : Kaillera_FreeLibrary
=========================================================*/
/* #FN#
   Releases KailleraClient.dll library */
void
/* #AS#
   Nothing */
Kaillera_FreeLibrary( void )
{
	if( s_hKailleraClient )
	{
		FreeLibrary( s_hKailleraClient );

		s_hKailleraClient       = NULL;
		s_pfnGetVersion         = NULL;
		s_pfnInit               = NULL;
		s_pfnShutDown           = NULL;
		s_pfnSetInfos           = NULL;
		s_pfnSelectServerDialog = NULL;
		s_pfnModifyPlayValues   = NULL;
		s_pfnChatSend           = NULL;
		s_pfnEndGame            = NULL;
	}
} /* #OF# Kaillera_FreeLibrary */

/*========================================================
Function : Kaillera_IsCapable
=========================================================*/
/* #FN#
   Checks if the KAILLERA library is available */
int
/* #AS#
   1 if KAILLERA has been loaded, otherwise -1 */
Kaillera_IsCapable( void )
{
	if( !s_hKailleraClient )
	{
//		Log_print( "Cannot allow network games without kaillera.dll loaded properly." );
		return -1;
	}
	return 1;

} /* #OF# Kaillera_IsCapable */

/*========================================================
Function : KAGETVERSION
=========================================================*/
/* #FN#
   Call this method to retrieve kailleraclient.dll's version */
static
int
/* #AS#
   ? */
KAGETVERSION(
	char *pszVersion
)
{
	return (int)s_pfnGetVersion( pszVersion );

} /* #OF# KAGETVERSION */

/*========================================================
Function : KAINIT
=========================================================*/
/* #FN#
   Call this method when your program starts */
static
int
/* #AS#
   ? */
KAINIT( void )
{
	return (int)s_pfnInit();

} /* #OF# KAINIT */

/*========================================================
Function : KASHUTDOWN
=========================================================*/
/* #FN#
   Call this method when your program ends */
static
int
/* #AS#
   ? */
KASHUTDOWN( void )
{
	return (int)s_pfnShutDown();

} /* #OF# KASHUTDOWN */

/*========================================================
Function : KASETINFOS
=========================================================*/
/* #FN#
   Use this method for setting up various infos */
static
int
/* #AS#
   ? */
KASETINFOS(
	kailleraInfos *pInfos
)
{
	return (int)s_pfnSetInfos( pInfos );

} /* #OF# KASETINFOS */

/*========================================================
Function : KASELECTSERVERDIALOG
=========================================================*/
/* #FN#
   Use this method for launching the Kaillera server dialog */
static
int
/* #AS#
   ? */
KASELECTSERVERDIALOG(
	HWND hParent
)
{
	return (int)s_pfnSelectServerDialog( hParent );

} /* #OF# KASELECTSERVERDIALOG */

/*========================================================
Function : KAMODIFYPLAYVALUES
=========================================================*/
/* #FN#
   You must call this method at every frame after you retrieved values
   from your input devices */
static
int
/* #AS#
   Length received or -1 on network error (player no more in the game) */
KAMODIFYPLAYVALUES(
	void *pValues,
	int   nSize
)
{
	return (int)s_pfnModifyPlayValues( pValues, nSize );

} /* #OF# KAMODIFYPLAYVALUES */

/*========================================================
Function : KACHATSEND
=========================================================*/
/* #FN#
   Use this function to send a line of chat text during a game */
static
int
/* #AS#
   ? */
KACHATSEND(
	char *pszText
)
{
	return (int)s_pfnChatSend( pszText );

} /* #OF# KACHATSEND */

/*========================================================
Function : KAENDGAME
=========================================================*/
/* #FN#
   Your emulation thread must call this method when the user stops
   the emulation */
static
int
/* #AS#
   ? */
KAENDGAME( void )
{
	return (int)s_pfnEndGame();

} /* #OF# KAENDGAME */

#endif /*WIN_NETWORK_GAMES */
