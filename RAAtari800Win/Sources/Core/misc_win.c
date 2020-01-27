/****************************************************************************
File    : misc_win.c
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Implementation of some helpers
@(#) #BY# Richard Lawrence, Tomasz Szymankowski
@(#) #LM# 10.10.2003
*/

#include <windows.h>
#include <shellapi.h>
#include <stdlib.h>
#include <crtdbg.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <mmsystem.h>
#include <time.h>
#include <excpt.h>
#include "WinConfig.h"
#include "Resource.h"
#include "Debug.h"
#include "Helpers.h"
#include "FileService.h"
#include "atari800.h"
#include "globals.h"
#include "macros.h"
#include "timing.h"
#include "crc.h"
#include "registry.h"
#include "rdevice.h"
#include "display_win.h"
#include "sound_win.h"
#include "input_win.h"
#include "misc_win.h"
#include "cfg.h"

#ifdef WIN_NETWORK_GAMES
#include "kaillera.h"
#endif

/* Definition of constants */

#define EC_BUFFER_LEN		32
#define EC_MAX_LINE_LEN		2048

/* Public objects */

struct MiscCtrl_t g_Misc =
{
	DEF_MISC_STATE,
	DEF_FILE_ASSOCIATIONS,
	DEF_DONT_SHOW_FLAGS,
	DEF_SPEED_PERCENT,
	0,
	ATTRACT_CHECK_FRAMES,
	/* Cheat */
	{
		DEF_CHEAT_COLLISIONS,
		DEF_CHEAT_MEMO,
		DEF_CHEAT_SEARCH,
		DEF_CHEAT_LOCK
	},
	/* Refresh */
	{
		DEF_REFRESH_DOUBLEWND,
		DEF_REFRESH_FULLSPEED
	}
};

struct RomTypeInfo_t g_aRomTypeInfo[] =
{
	{ RTI_OSA, 2370568491L, "Rev.A  \t| 4/800\t| PAL" },
	{ RTI_OSB, 3252116993L, "Rev.B  \t| 4/800\t| PAL" },
	{ RTI_OSB, 4051249634L, "Rev.B  \t| 4/800\t| NTSC" },
	{ RTI_XLE, 2613326695L, "Rev.1  \t| 600XL\t| 03/11/1983" },
	{ RTI_XLE, 3764596111L, "Rev.2  \t| XL/XE\t| 05/10/1983" },
	{ RTI_XLE, 3591293960L, "Rev.3  \t| 800XE\t| 03/01/1985" },
	{ RTI_XLE, 3780165629L, "Rev.4  \t| XEGS \t| 05/07/1987" },
	{ RTI_XLE, 257804588L,  "Arabic \t| 65XE \t| 07/21/1987" },
	{ RTI_XLE, 2766230028L, "Rev.3  \t| ARGS \t| ---" },
	{ RTI_XLE, 1577346053L, "Rev.2.3\t| QMEG \t| ---" },
	{ RTI_XLE, 1932228838L, "Rev.3.2\t| QMEG \t| ---" },
	{ RTI_XLE, 2265792253L, "Rev.3.8\t| QMEG \t| ---" },
	{ RTI_XLE, 2603811756L, "Rev.4.2\t| QMEG \t| ---" },
	{ RTI_XLE, 1105050917L, "Rev.4.3\t| QMEG \t| ---" },
	{ RTI_XLE, 2258206520L, "---    \t| TOMS \t| ---" },
	{ RTI_A52, 3182898204L, "---    \t| 5200 \t| ---" },
//	{ RTI_A52, 2155009960L, "FoxOS  \t| 5200 \t| ---" },
	{ RTI_BAS, 3021189661L, "Rev.A  \t| ---  \t| ---" },
	{ RTI_BAS, 266326092L,  "Rev.B  \t| ---  \t| ---" },
	{ RTI_BAS, 2190982779L, "Rev.C  \t| ---  \t| ---" }
};

const int g_nRomTypeInfoNo = sizeof(g_aRomTypeInfo)/sizeof(g_aRomTypeInfo[0]);

/* Private objects */

static BOOL   s_bStopSearch = FALSE;
//static HANDLE s_hMonCon   = NULL;
static BOOL   s_bCompress   = TRUE;
static BOOL   s_bRDevice    = TRUE;
#ifdef WIN_NETWORK_GAMES
static BOOL   s_bKaillera   = TRUE;
#endif



/*========================================================
Function : Misc_UpdateCollisions
=========================================================*/
void Misc_UpdateCollisions() {
	if (g_Misc.ulState & MS_DISABLE_COLLISIONS) {
		GTIA_collisions_mask_missile_playfield = g_Misc.Cheat.ulCollisions &
											DC_MISSILE_PLAYFIELD ? 0 : 0x0f;
		GTIA_collisions_mask_player_playfield = g_Misc.Cheat.ulCollisions &
											DC_PLAYER_PLAYFIELD ? 0 : 0x0f;
		GTIA_collisions_mask_missile_player = g_Misc.Cheat.ulCollisions &
											DC_MISSILE_PLAYER ? 0 : 0x0f;
		GTIA_collisions_mask_player_player = g_Misc.Cheat.ulCollisions &
											DC_PLAYER_PLAYER ? 0 : 0x0f;
	}
	else {
		GTIA_collisions_mask_missile_playfield = 0x0f;
		GTIA_collisions_mask_player_playfield = 0x0f;
		GTIA_collisions_mask_missile_player = 0x0f;
		GTIA_collisions_mask_player_player = 0x0f;
	}
}

/*========================================================
Function : Misc_GetSystemInfo
=========================================================*/
/* #FN#
   Gets information about the system an emulator is run on */
BOOL
/* #AS#
   TRUE if succeeded, otherwise FALSE */
Misc_GetSystemInfo(
	UINT *pSystemInfo
)
{
	OSVERSIONINFO osvi;
	*pSystemInfo = 0;

	/* Determine the Windows OS version */
	ZeroMemory( &osvi, sizeof(OSVERSIONINFO) );
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	if( !GetVersionEx( (OSVERSIONINFO *)&osvi ) )
		return FALSE;

	switch( osvi.dwPlatformId )
	{
		case VER_PLATFORM_WIN32_NT:
		{
			/* Test for the product */
			if( osvi.dwMajorVersion == 4 )
				*pSystemInfo = SYS_WIN_NT4;

			if( osvi.dwMajorVersion == 5 )
				*pSystemInfo = SYS_WIN_NT5;

			break;
		}

		case VER_PLATFORM_WIN32_WINDOWS:
		{
			if( osvi.dwMajorVersion  > 4 ||
			   (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion > 0) )
			{
				*pSystemInfo = SYS_WIN_98;
			}
			else
				*pSystemInfo = SYS_WIN_95;

			break;
		}
	}
	/* Check if an enhanced processor instruction set is available */
	__try
	{
		int nFeatureFlags = 0;

		/* NOTE: The following sample code contains a Pentium-specific instruction.
		   It should only be run on Pentium processors. The program generates
		   unhandled exception if running on non-Pentium family processors. */
		__asm
		{
			mov eax, 1
			_emit 0x0f		; CPUID (00001111 10100010) - This is a Pentium specific
			_emit 0xa2		; instruction which gets information on the processor.
			mov nFeatureFlags, edx
		}
		/* Is IA MMX technology bit (bit 23 of edx) in feature flags set? */
		if( _IsFlagSet( nFeatureFlags, 0x800000 ) )
			*pSystemInfo |= SYS_PRC_MMX;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		/* Do nothing */
	}
	return TRUE;

} /* #OF# Misc_GetSystemInfo */

/*========================================================
Function : prepend_tmpfile_path
=========================================================*/
/* #FN#
   Inserts path to a temp dir to supplied buffer */
int
/* #AS#
   A length of the path */
prepend_tmpfile_path(
	char *pszBuffer
)
{
	int	nOffset = 0;

	nOffset += GetTempPath( MAX_PATH, pszBuffer ) - 1;
	if( nOffset == -1 || pszBuffer[ nOffset ] != '\\' )
		nOffset++;

	return nOffset;

} /* #OF# prepend_tmpfile_path */

/*========================================================
Function : Atari_Initialise
=========================================================*/
/* #FN#
   Prepares Windows stuff for emulation; this function is invoked
   by Atari800 kernel */
void
/* #AS#
   Nothing */
Atari_Initialise(
	int  *argc,
	char *argv[]
)
{
	int nResult = 0;

//	if( s_bCompress ) /* Load ZLIB library */
/*	{
		s_bCompress = FALSE;

		nResult = Compress_LoadLibrary( NULL );
		if( nResult > 0 )
		{
			if( ZL_ERROR_LOAD == nResult )
				DisplayWarning( IDS_WARN_ZLIBLOAD, DONT_SHOW_ZLIBLOAD_WARN, FALSE );
			else
			if( ZL_ERROR_FUNC == nResult )
				DisplayWarning( IDS_WARN_ZLIBUSE, DONT_SHOW_ZLIBUSE_WARN, FALSE );
		}
	}*/
	if( s_bRDevice ) /* Load WS2_32 library */
	{
		s_bRDevice = FALSE;

		nResult = RDevice_LoadLibrary( NULL );
		if( nResult > 0 )
		{
			if( WS_ERROR_LOAD == nResult )
				DisplayWarning( IDS_WARN_SOCKLOAD, DONT_SHOW_SOCKLOAD_WARN, FALSE );
			else
			if( WS_ERROR_FUNC == nResult )
				DisplayWarning( IDS_WARN_SOCKUSE, DONT_SHOW_SOCKUSE_WARN, FALSE );

			/* Turn off R: device emulation if there is no winsocket library available */
			if( Devices_enable_r_patch )
			{
				Devices_enable_r_patch = 0;
				WriteRegDWORD( NULL, REG_ENABLE_R_PATCH, Devices_enable_r_patch );
			}
		}
	}
#ifdef WIN_NETWORK_GAMES

	if( s_bKaillera ) /* Load KAILLERACLIENT library */
	{
		s_bKaillera = FALSE;

		nResult = Kaillera_LoadLibrary( NULL );
		if( nResult > 0 )
		{
			if( KA_ERROR_LOAD == nResult )
				DisplayWarning( IDS_WARN_KALOAD, DONT_SHOW_KALOAD_WARN, FALSE );
			else
			if( KA_ERROR_FUNC == nResult )
				DisplayWarning( IDS_WARN_KAUSE, DONT_SHOW_KAUSE_WARN, FALSE );
		}
		else
			/* Initialize the Kaillera stuff */
			Kaillera_Initialise();
	}
#endif /*WIN_NETWORK_GAMES*/

	/* Check what DirectX modes are available */
	if( !g_Screen.ulModesAvail )
	{
		Screen_CheckDDrawModes();
	}
	/* Clean up the Atari timer stuff */
	Timer_Reset();
	/* Clean up the input stuff */
	Input_Reset();

} /* #OF# Atari_Initialise */

/*========================================================
Function : Misc_TogglePause
=========================================================*/
/* #FN#
   Toggles between the pause on and off */
void
/* #AS#
   Nothing */
Misc_TogglePause( void )
{
	if( _IsFlagSet( g_ulAtariState, ATARI_RUNNING ) )
	{
		if( _IsFlagSet( g_ulAtariState, ATARI_PAUSED ) )
		{
			_ClrFlag( g_ulAtariState, ATARI_PAUSED );
			Sound_Restart();

			/* Release the redrawing stuff if necessary */
			Screen_FreeRedraw();

			if( ST_FLIPPED_BUFFERS )
				Screen_Clear( FALSE, FALSE );

			ResetLoopCounter( FALSE );
 		}
		else
		{
			_SetFlag( g_ulAtariState, ATARI_PAUSED );
			Sound_Clear( FALSE, FALSE );

			if( g_hMainWnd )
				SetTimer( g_hMainWnd, TIMER_READ_JOYSTICK, 100, NULL );

			/* Redraw paused screen if necessary; snapshot screen
			   for redrawing */
			Screen_DrawPaused( TRUE, FALSE, TRUE, TRUE );

			Screen_ShowMousePointer( TRUE );
		}
	}
} /* #OF# Misc_TogglePause */

/*========================================================
Function : Misc_ToggleFullSpeed
=========================================================*/
/* #FN#
   Toggles between the full speed mode on and off */
void
/* #AS#
   Nothing */
Misc_ToggleFullSpeed( void )
{
	if( _IsFlagSet( g_ulAtariState, ATARI_RUNNING ) )
	{
		if( _IsFlagSet( g_Misc.ulState, MS_FULL_SPEED ) )
		{
			_ClrFlag( g_Misc.ulState, MS_FULL_SPEED );
			Sound_Restart();
		}
		else
		{
			_SetFlag( g_Misc.ulState, MS_FULL_SPEED );
			Sound_Clear( FALSE, FALSE );
		}
		/* Force screen refreshing (keep in mind that there may
		   be different refresh rate set for full speed mode */
		g_nTestVal = _GetRefreshRate() - 1;

		WriteRegDWORD( NULL, REG_MISC_STATE, g_Misc.ulState );
	}
} /* #OF# Misc_ToggleFullSpeed */

/*========================================================
Function : Misc_ToggleSIOPatch
=========================================================*/
/* #FN#
   Toggles between the SIO patch mode on and off */
void
/* #AS#
   Nothing */
Misc_ToggleSIOPatch( void )
{
	ESC_enable_sio_patch = ESC_enable_sio_patch ? 0 : 1;
	WriteRegDWORD( NULL, REG_ENABLE_SIO_PATCH, ESC_enable_sio_patch );

	ESC_UpdatePatches();

} /* #OF# Misc_ToggleSIOPatch */

/*========================================================
Function : Misc_CheckAttractCounter
=========================================================*/
/* #FN#
   Forbids activating the attract mode (routine by Piotr Fusik) */
void
/* #AS#
   Nothing */
Misc_CheckAttractCounter( void )
{
	static int nPreviousCounter = 0;

	if( MEMORY_dGetByte( 0x4D ) == (UBYTE)(nPreviousCounter + (MEMORY_dGetByte( 0x14 ) == 0)) )
	{
		if( --g_Misc.nAttractCounter <= 0 )
		{
			MEMORY_dPutByte( 0x4D, 0 );
			g_Misc.nAttractCounter = ATTRACT_CHECK_FRAMES;
		}
	}
	else
		g_Misc.nAttractCounter = ATTRACT_CHECK_FRAMES;

	nPreviousCounter = MEMORY_dGetByte( 0x4D );

} /* #OF# Misc_CheckAttractCounter */

/*========================================================
Function : MonitorThreadProc
=========================================================*/
/* #FN#
   A separate thread procedure for a monitor console */
static
DWORD WINAPI
/* #AS#
   Thread return code (success or failure) */
MonitorThreadProc(
	LPVOID lpParam
)
{
	return MONITOR_Run();

} /* #OF# MonitorThreadProc */

/*========================================================
Function : Misc_LaunchMonitor
=========================================================*/
/* #FN#
   Launches the emulator monitor console */
int
/* #AS#
   An integer value: 1 to go back to emulation, 0 to exit */
Misc_LaunchMonitor( void )
{
	DWORD  dwThreadId   = 0;
	DWORD  dwExitCode   = 0;
	HANDLE hThread      = NULL;
	HWND   hPopupWnd    = NULL;
	ULONG  ulScreenMode = 0L;

	/* OnActivate (when WA_INACTIVE) will invoke that */
//	Sound_Clear( FALSE, FALSE );

	/* Show the mouse pointer if needed */
	Screen_ShowMousePointer( TRUE );

	/* Unfortunately, there are some problems with automatically return to
	   (flipped) full-screen mode, go to windowed and then restore instead */
	if( _IsFlagSet( g_Screen.ulState, SM_MODE_FULL ) )
	{
		ulScreenMode = g_Screen.ulState;

		_ClrFlag( g_Screen.ulState, SM_MODE_MASK );
		/* Set a windowed mode */
		_SetFlag( g_Screen.ulState, SM_MODE_WIND );

		if( !Screen_ChangeMode( TRUE ) )
			ulScreenMode = 0L;
	}
	/* Disable the main window */
	_SetFlag( g_ulAtariState, ATARI_MONITOR );
	EnableWindow( g_hMainWnd, FALSE );

	/* Launch an emulated Atari monitor */
	if( (hThread = _CreateThreadEx( NULL,
									0,
									MonitorThreadProc,
									(LPVOID)NULL,
									0,
									&dwThreadId )) )
	{
		/* This message loop will solve some problems with
           the main window repainting */
		do
		{
			MSG msg;
			if( PeekMessage( &msg, 0, 0, 0, PM_REMOVE ) )
			{
				TranslateMessage( &msg );
				DispatchMessage( &msg );
			}
			Sleep( 1 ); /* Wait a moment, please */
			GetExitCodeThread( hThread, &dwExitCode );
		}
		while( STILL_ACTIVE == dwExitCode );

		CloseHandle( hThread );
	}
	else
		/* We had few luck to create an additional thread */
		dwExitCode = MONITOR_Run();

	/* Enable the main window */
	EnableWindow( g_hMainWnd, TRUE );
	_ClrFlag( g_ulAtariState, ATARI_MONITOR );

	hPopupWnd = GetLastActivePopup( g_hMainWnd );

	BringWindowToTop( g_hMainWnd );
	if( IsIconic( hPopupWnd ) )
	{
		ShowWindow( hPopupWnd, SW_RESTORE );
	}
	else
		SetForegroundWindow( hPopupWnd );

	if( ulScreenMode )
	{
		g_Screen.ulState = ulScreenMode;

		Screen_ChangeMode( TRUE );
	}
	if( dwExitCode )
	{
		/* Go back to the emulation */
		return 1;
	}
	/* Start a quit (this will end up in Atari_Exit(0)) */
	PostMessage( g_hMainWnd, WM_CLOSE, 0, 0L );

	return 0;

} /* #OF# Misc_LaunchMonitor */

/*========================================================
Function : Atari_Exit
=========================================================*/
/* #FN#
   This function is called by Atari800 kernel when emulation is
   about exit */
int
/* #AS#
   Nonzero to go back to emulation, 0 to exit */
Atari_Exit(
	int nPanic
)
{
	if ( ATARI_CRASHED == g_ulAtariState )
	{
		ANTIC_wsync_halt = 1;	/* turn off CPU */
		return 0;
	}

	if( nPanic )
	{
		BOOL bContinue = FALSE;

		Screen_DrawFrozen( FALSE, TRUE, TRUE, TRUE );
		
		if ( _IsFlagSet( g_ulAtariState, ATARI_CRASHED ) )
		{
			ANTIC_wsync_halt = 1; /* turn off CPU */
			if ( _IsFlagSet( g_Misc.ulState, MS_MONITOR_ALWAYS) )
			{
				bContinue = Misc_LaunchMonitor();
			}
			else
			{
				if ( !_IsFlagSet( g_Misc.ulState, MS_MONITOR_NEVER) &&
					IDYES == DisplayMessage( NULL, IDS_WARN_PANIC, 0, MB_ICONSTOP | MB_YESNO ) )
				{
					/* Launch the monitor */
					bContinue = Misc_LaunchMonitor();
				}
			}
		}
		else
		{
			bContinue = Misc_LaunchMonitor();
		}

		Screen_FreeRedraw();
		Screen_UseAtariPalette( FALSE );

		if( bContinue )
		{
			_ClrFlag( g_ulAtariState, ATARI_CRASHED ); /* For the CIM service */
			UpdateWindowTitle( FALSE, -1, ID_INDICATOR_RUN, FALSE );

			return 1;
		}
		g_ulAtariState = ATARI_CRASHED;
		ANTIC_wsync_halt = 1;	/* Turn off CPU */
		g_nTestVal = 32767;

		InvalidateRect( g_hMainWnd, NULL, TRUE );
	}
	else
	{
		/* Reset everything DirectDraw */
		Screen_Clear( TRUE, FALSE );
		/* Reset everything MultiMedia/DirectSound */
		Sound_Clear( TRUE, TRUE );
		/* Reset everything DirectInput */
		Input_Clear();

		g_ulAtariState = ATARI_UNINITIALIZED | ATARI_CLOSING;

		CARTRIDGE_Remove();
		CASSETTE_Remove();
		SIO_Exit();

		RDevice_Exit();
		RDevice_FreeLibrary();

//		Compress_FreeLibrary();

#ifdef WIN_NETWORK_GAMES
		Kaillera_Exit();
		Kaillera_FreeLibrary();
#endif
	}
	return 0;

} /* #OF# Atari_Exit */

#define WND_TITLE_LENGTH		32

typedef struct _FindWnd_t
{
	char  szWndTitle[ WND_TITLE_LENGTH + 1 ];
	DWORD dwThreadId;
	HWND  hWnd;
} FindWnd_t;

/*========================================================
Function : EnumWindowsProc
=========================================================*/
/* #FN#
   Receives top-level window handles */
static
BOOL CALLBACK
/* #AS#
   To continue enumeration, the callback function must return TRUE, otherwise FALSE */
EnumWindowsProc(
	HWND   hWnd,   /* #IN# Handle to parent window   */
	LPARAM lParam  /* #IN# Application-defined value */
)
{
	BOOL bResult = TRUE; /* Continue enumeration */
	char szTitle[ WND_TITLE_LENGTH + 1 ] = { 0 };

	FindWnd_t *pFindWnd = (FindWnd_t *)lParam;

	if( 0 == pFindWnd->dwThreadId || /* It's optional */
		pFindWnd->dwThreadId == GetWindowThreadProcessId( hWnd, NULL ) )
	{
		int nLen = strlen( pFindWnd->szWndTitle );

		GetWindowText( hWnd, szTitle, nLen + 1 );
		if( strncmp( pFindWnd->szWndTitle, szTitle, nLen ) == 0 )
		{
			pFindWnd->hWnd = hWnd;
			bResult = FALSE;
		}
	}
	return bResult;

} /* #OF# EnumWindowsProc */

/*========================================================
Function : Misc_FindWindow
=========================================================*/
/* #FN#
   Tries to find the window with a given title */
HWND
/* #AS#
   Handle of the window if it has been found, otherwise NULL */
Misc_FindWindow(
	LPCSTR pszTitle,
	DWORD  dwThreadId
)
{
	FindWnd_t FindWnd;

	_strncpy( FindWnd.szWndTitle, pszTitle, WND_TITLE_LENGTH );
	FindWnd.dwThreadId = dwThreadId;
	FindWnd.hWnd       = NULL;

	/* Enumerate top-level windows */
	EnumWindows( EnumWindowsProc, (LPARAM)&FindWnd );

	return FindWnd.hWnd;

} /* #OF# Misc_FindWindow */

/*========================================================
Function : Misc_FindChildWindow
=========================================================*/
/* #FN#
   Tries to find parent's child window with a given title */
HWND
/* #AS#
   Handle of the window if it has been found, otherwise NULL */
Misc_FindChildWindow(
	HWND   hParent,
	LPCSTR pszTitle
)
{
	FindWnd_t FindWnd;

	_strncpy( FindWnd.szWndTitle, pszTitle, WND_TITLE_LENGTH );
	FindWnd.dwThreadId = 0;
	FindWnd.hWnd       = NULL;

	/* Enumerate child windows */
	EnumChildWindows( hParent, EnumWindowsProc, (LPARAM)&FindWnd );

	return FindWnd.hWnd;

} /* #OF# Misc_FindChildWindow */

/*========================================================
Function : Misc_EnableCloseItem
=========================================================*/
/* #FN#
   Enables or disables the CLOSE menu item of a given window */
void
/* #AS#
   Nothing */
Misc_EnableCloseItem(
	HWND hWnd,
	BOOL bEnable
)
{
	if( NULL != hWnd )
	{
		/* Disable the CLOSE button */
		HMENU hMenu = GetSystemMenu( hWnd, 0 );
		if( NULL != hMenu )
		{
			EnableMenuItem( hMenu, SC_CLOSE, MF_BYCOMMAND | (bEnable ? MF_ENABLED : MF_GRAYED) );
			DrawMenuBar( hWnd );
		}
	}
} /* #OF# Misc_EnableCloseItem */

/*========================================================
Function : Misc_GetHomeDirectory
=========================================================*/
/* #FN#
   Retrieves the path for the Atari800Win home directory */
BOOL
/* #AS#
   TRUE if the path was retrieved, otherwise FALSE */
Misc_GetHomeDirectory(
	LPSTR pszHomeDir
)
{
	BOOL bResult = FALSE;

	if( GetModuleFileName( NULL, pszHomeDir, MAX_PATH ) )
	{
		size_t i;
		if( i = strlen( pszHomeDir ) )
		{
			while( i && pszHomeDir[ i ] != '\\' )
				i--;
			if( i )
				pszHomeDir[ i ] = '\0';
			bResult = TRUE;
		}
	}
	return bResult;

} /* #OF# Misc_GetHomeDirectory */

/*========================================================
Function : TestAndSetPath
=========================================================*/
/* #FN#
   Searches the Atari system ROMs in home folder and its subfolders */
static
BOOL
/* #AS#
   TRUE if the ROM files has been found, otherwise FALSE */
TestAndSetPath(
	LPSTR pszFileName,
	enum  RomType rtType,
	LPSTR pszPath
)
{
	char   szStartPath[ MAX_PATH + 1 ];
	char   szImagePath[ MAX_PATH + 1 ];
	char   szPattern  [ MAX_PATH + 1 ];
	ULONG  ulCRC;
	int    nResult;

	WIN32_FIND_DATA fileData;
	HANDLE hFile;

	if( NULL == pszPath )
	{
		/* We begin searching at an Atari800Win home directory */
		Misc_GetHomeDirectory( szStartPath );
	}
	else
		strcpy( szStartPath, pszPath );

	if( szStartPath[ strlen( szStartPath ) - 1 ] != '\\' )
		strcat( szStartPath, "\\" );

	strcpy( szPattern, szStartPath );
	strcat( szPattern, "*.rom" );

	hFile = FindFirstFile( szPattern, &fileData );
	/* If any file has been found... */
	while( INVALID_HANDLE_VALUE != hFile && !s_bStopSearch )
	{
		if( !_IsFlagSet( fileData.dwFileAttributes, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_DIRECTORY ) )
		{
			strcpy( szImagePath, szStartPath );
			strcat( szImagePath, fileData.cFileName );

			ulCRC = CheckFile( szImagePath, &nResult );
			if( ulCRC != 0 )
			{
				int i;
				for( i = 0; i < g_nRomTypeInfoNo; i++ )
				{
					if( g_aRomTypeInfo[ i ].rtType == rtType &&
						g_aRomTypeInfo[ i ].ulCRC  == ulCRC )
					{
						/* Again, strncpy gives the strange effects here */
						_ASSERT(strlen( szImagePath ) < MAX_PATH);
						strcpy( pszFileName, szImagePath );

						/* The searched file has been found */
						FindClose( hFile );
						return TRUE;
					}
				}
			}
		}
		/* Try to find a next file */
		if( !FindNextFile( hFile, &fileData ) )//&& (GetLastError() == ERROR_NO_MORE_FILES) )
		{
			FindClose( hFile );
			hFile = INVALID_HANDLE_VALUE; /* Ending the loop */
		}
	}
	strcpy( szPattern, szStartPath );
	strcat( szPattern, "*.*" );

	hFile = FindFirstFile( szPattern, &fileData );
	/* If any folder has been found... */
	while( INVALID_HANDLE_VALUE != hFile && !s_bStopSearch )
	{
		if( !_IsFlagSet( fileData.dwFileAttributes, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM ) &&
			 _IsFlagSet( fileData.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY ) && (*fileData.cFileName != '.') )
		{
			strcpy( szImagePath, szStartPath );
			strcat( szImagePath, fileData.cFileName );

			/* Search the ROM file in this subfolder */
			if( TestAndSetPath( pszFileName, rtType, szImagePath ) )
			{
				FindClose( hFile );
				return TRUE;
			}
		}
		/* Try to find a next folder */
		if( !FindNextFile( hFile, &fileData ) )//&& (GetLastError() == ERROR_NO_MORE_FILES) )
		{
			FindClose( hFile );
			hFile = INVALID_HANDLE_VALUE; /* Ending the loop */
		}
	}
	return FALSE;

} /* #OF# TestAndSetPath */

/*========================================================
Function : RomSearchingDlgProc
=========================================================*/
/* #FN#
   The "ROM Searching" dialog box procedure */
static
int CALLBACK
/* #AS#
   TRUE if the message has been handled, otherwise FALSE */
RomSearchingDlgProc(
	HWND hDialog,
	UINT uiMsg,
	UINT wParam,
	LONG lParam
)
{
	LPCSTR pszProgress[] = { "|", "/", "-", "\\" };
	static int i = -1;

	switch( uiMsg )
	{
		case WM_USER:
			if( ++i > 3 ) i = 0;
			SetWindowText( hDialog, pszProgress[ i ] );
			return TRUE;

		case WM_COMMAND:
			s_bStopSearch = TRUE;
			DestroyWindow( hDialog );
			return TRUE;
	}
	return FALSE;

} /* #OF# RomSearchingDlgProc */

/*========================================================
Function : SearchingThreadProc
=========================================================*/
/* #FN#
   A separate thread procedure for ROM searching */
static
DWORD WINAPI
/* #AS#
   Thread return code (success or failure) */
SearchingThreadProc(
	LPVOID lpParam
)
{
	LPSTR pszStartPath = (LPSTR)lpParam;
	BOOL  bFoundRom = FALSE;

	if(	!s_bStopSearch &&
		TestAndSetPath( CFG_osa_filename,   RTI_OSA, pszStartPath ) )
		bFoundRom = TRUE;
	if(	!s_bStopSearch &&
		TestAndSetPath( CFG_osb_filename,   RTI_OSB, pszStartPath ) )
		bFoundRom = TRUE;
	if(	!s_bStopSearch &&
		TestAndSetPath( CFG_xlxe_filename,  RTI_XLE, pszStartPath ) )
		bFoundRom = TRUE;
	if(	!s_bStopSearch &&
		TestAndSetPath( CFG_5200_filename,  RTI_A52, pszStartPath ) )
		bFoundRom = TRUE;
	if(	!s_bStopSearch &&
		TestAndSetPath( CFG_basic_filename, RTI_BAS, pszStartPath ) )
		bFoundRom = TRUE;

	return bFoundRom;

} /* #OF# SearchingThreadProc */

/*========================================================
Function : Misc_TestRomPaths
=========================================================*/
/* #FN#
   Routine tries to locate the system ROMs in either the home directory
   or a subdirectory "ROMs" under the home directory */
BOOL
/* #AS#
   TRUE if at least one ROM file was found, otherwise FALSE */
Misc_TestRomPaths(
	LPSTR pszStartPath,
	HWND  hWnd
)
{
	HWND   hDialog = NULL;
	HANDLE hThread = NULL;
	DWORD  dwThreadId, dwExitCode = 0;

	s_bStopSearch = FALSE;

	/* Install the Abort Dialog handler and display the window */
	if( hWnd )
	{
		EnableWindow( hWnd, FALSE );
		hDialog = CreateDialog( g_hInstance, MAKEINTRESOURCE(IDD_ROMSEARCHING),
								hWnd, (DLGPROC)RomSearchingDlgProc );
	}
	/* The searching procedure may take a long time until it
	   ends its activity. */
	if( (hThread = _CreateThreadEx( NULL,
									0,
									SearchingThreadProc,
									(LPVOID)pszStartPath,
									0,
									&dwThreadId )) )
	{
		int nCount = 0;
		/* This message loop will solve some problems with Abort
		   Dialog activity and the main window repainting */
		do
		{
			MSG msg;
			if( PeekMessage( &msg, 0, 0, 0, PM_REMOVE ) )
				if( !hDialog || !IsDialogMessage( hDialog, &msg ) )
				{
					TranslateMessage( &msg );
					DispatchMessage( &msg );
				}
			if( nCount++ == 20 && hDialog )
			{
				SendMessage( hDialog, WM_USER, 0, 0 );
				nCount = 0;
			}
			Sleep( 10 ); /* Wait a moment, please */
			GetExitCodeThread( hThread, &dwExitCode );
		}
		while( STILL_ACTIVE == dwExitCode );

		CloseHandle( hThread );
	}
	if(	hWnd )
	{
		EnableWindow( hWnd, TRUE );
		/* There is the Search window enabled, so we can destroy the
		   Abort box now */
		if( !s_bStopSearch && hDialog )
			DestroyWindow( hDialog );

//		SetFocus( hWnd );
	}
	return (BOOL)dwExitCode;

} /* #OF# Misc_TestRomPaths */

/*========================================================
Function : ConsoleHandlerRoutine
=========================================================*/
/* #FN#
   A console process uses this function to handle control signals received
   by the process */
static
BOOL WINAPI
/* #AS#
   If the function handles the control signal, it should return TRUE. If it
   returns FALSE, the next handler function in the list of handlers for this
   process is used */
ConsoleHandlerRoutine(
	DWORD dwCtrlType /* #IN# Control signal type */
)
{
	/* On Windows 95, a console application that has installed a control
	   signal handler function only gets called for the CTRL_C_EVENT and
	   CTRL_BREAK_EVENT signals; the signal handler function is never
	   called for the CTRL_SHUTDOWN_EVENT, CTRL_LOGOFF_EVENT, and
	   CTRL_CLOSE_EVENT signals. */
	switch( dwCtrlType )
	{
		case CTRL_C_EVENT:
		case CTRL_BREAK_EVENT:
		case CTRL_SHUTDOWN_EVENT:
		case CTRL_CLOSE_EVENT:
		case CTRL_LOGOFF_EVENT:
			break;

		default:
			/* Unknown type, better pass it on */
			return FALSE;
	}
	/* Handled all known events */
	return TRUE;

} /* #OF# ConsoleHandlerRoutine */

/*========================================================
Function : Misc_AllocMonitorConsole
=========================================================*/
/* #FN#
   Invokes a monitor console */
BOOL
/* #AS#
   BOOL if succeeded, otherwise FALSE */
Misc_AllocMonitorConsole(
	FILE **pOutput,
	FILE **pInput
)
{
	BOOL bResult = FALSE;

    if( AllocConsole() )
	{
		*pOutput = fopen( "CON", "w" );
		*pInput  = fopen( "CON", "r" );

		if( NULL != *pOutput && NULL != *pInput )
		{
			char szBuffer[ LOADSTRING_SIZE_S + 1 ];
			HWND hMonWnd = NULL;

			/* Print a short instruction */
//			if( NULL == s_hMonCon )
//			{
//				s_hMonCon = CreateFile( "CONOUT$", GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, 0 );
//				if( INVALID_HANDLE_VALUE != s_hMonCon )
//				{
//					DWORD dwWritten = 0;
//
//					_LoadStringSx( IDS_CONSOLE_TIP, szBuffer );
//
//					SetConsoleTextAttribute( s_hMonCon, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY );
//					WriteConsole( s_hMonCon, szBuffer, strlen( szBuffer ), &dwWritten, 0 );
//					SetConsoleTextAttribute( s_hMonCon, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY );
//				}
//				else
//				{
					fprintf( *pOutput, _LoadStringSx( IDS_CONSOLE_TIP, szBuffer ) );
//					s_hMonCon = NULL;
//				}
//			}
			/* Modify the console window a little bit */
			SetConsoleTitle( _LoadStringSx( IDS_CONSOLE_TITLE, szBuffer ) );
			Sleep( 100 ); /* Strange, but necessary... */

			if( !(hMonWnd = FindWindow( "ConsoleWindowClass", szBuffer )) )
			{
				hMonWnd = Misc_FindWindow( szBuffer, 0 /*GetCurrentThreadId()*/ );
			}
			if( NULL != hMonWnd )
			{
				/* Disable the CLOSE button */
				Misc_EnableCloseItem( hMonWnd, FALSE );
				/* Make sure the window has been shown */
				ShowWindow( hMonWnd, SW_SHOW );
			}
			/* Disable Ctrl+C and Ctrl+Break keystrokes */
			SetConsoleCtrlHandler( ConsoleHandlerRoutine, TRUE );

			bResult = TRUE;
		}
	}
	return bResult;

} /* #OF# Misc_AllocMonitorConsole */

/*========================================================
Function : Misc_FreeMonitorConsole
=========================================================*/
/* #FN#
   Closes a monitor console */
void
/* #AS#
   Nothing */
Misc_FreeMonitorConsole(
	FILE *pOutput,
	FILE *pInput
)
{
	/* Detach Console Routine */
	SetConsoleCtrlHandler( ConsoleHandlerRoutine, FALSE );

	if( pInput )
		fclose( pInput );
	if( pOutput )
		fclose( pOutput );

//	s_hMonCon = NULL;

	FreeConsole();

} /* #OF# Misc_FreeMonitorConsole */

/*========================================================
Function : ExecuteCmd
=========================================================*/
/* #FN#
   Executes a system command */
static
BOOL
/* #AS#
   TRUE if succeeded, otherwise FALSE */
ExecuteCmd(
	LPSTR pszCommand, /* #IN# System command to execute */
	BOOL  bGetOutput  /* #IN# Output stream to error log */
)
{
	BOOL   bProcess   = FALSE;
	BOOL   bPipe      = FALSE;
	HANDLE hReadPipe  = NULL;
	HANDLE hWritePipe = NULL;

	PROCESS_INFORMATION pi;
	STARTUPINFO si;

	SECURITY_ATTRIBUTES sa;
	sa.nLength              = sizeof(sa);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle       = TRUE;

	if( bGetOutput )
		bPipe = CreatePipe( &hReadPipe, &hWritePipe, &sa, 0 );

	/* Run a child process */
	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	si.wShowWindow = SW_SHOWNORMAL; /* Do not display the console window */
	si.dwFlags = STARTF_USESHOWWINDOW;

	if( bPipe )
	{
		si.dwFlags   |= STARTF_USESTDHANDLES;
		si.hStdInput  = GetStdHandle( STD_INPUT_HANDLE );
		si.hStdOutput = hWritePipe;
		si.hStdError  = hWritePipe;
	}

	/* Run system command */
	bProcess =
		CreateProcess( NULL,		// no module name (use command line)
					   pszCommand,	// command line
					   NULL,		// process handle not inheritable
					   NULL,		// threat handle not inheritable
					   TRUE,		// set handle inheritance to TRUE
					   0,			// no creation flags
					   NULL,		// use parent's environment block
					   NULL,		// use parent's starting directory
					   &si,
					   &pi );

	/* We have to close the reading end of the pipe */
	if( bPipe )
		CloseHandle( hWritePipe );

	if( bProcess )
	{
		if( bPipe )
		{
			char  szBuffer[ EC_BUFFER_LEN ];	/* Buffer that receives data */
			char  szText  [ EC_MAX_LINE_LEN ];	/* Buffer for output line */
			char *pszOut = szText;
			char  c;
			DWORD dwBytesRead;					/* Number of bytes read */
			BOOL  bReadResult;

			/* Output stream to Error View buffer */
			do
			{
				/* Read the executed process output message */
				bReadResult = ReadFile( hReadPipe, (LPVOID)&szBuffer, EC_BUFFER_LEN, &dwBytesRead, NULL );
				if( bReadResult )
				{
					char *pszIn = szBuffer;
					int   i;
					for( i = 0; i < (int)dwBytesRead; i++ )
					{
						c = *(pszIn++);
						if( c >= ' ' && c <= 'z' )
							*(pszOut++) = c;
						if( c == '\0' || c == '\n' )
						{
							*pszOut = '\0';
							/* Print message line to the Error View buffer */
							Log_print( szText );
							pszOut = szText;
						}
					}
				}
			}
			while( bReadResult && dwBytesRead != 0 );
		}
		if( bPipe )
			/* Wait for ending the executed process */
			WaitForSingleObject( pi.hProcess, INFINITE );

		if( pi.hProcess && pi.hProcess != INVALID_HANDLE_VALUE )
			CloseHandle( pi.hProcess );
		if( pi.hThread && pi.hThread != INVALID_HANDLE_VALUE )
			CloseHandle( pi.hThread );
	}

	if( bPipe )
		CloseHandle( hReadPipe );

	return bProcess;

} /* #OF# ExecuteCmd */

/*========================================================
Function : Misc_ExecutePrintCmd
=========================================================*/
/* #FN#
   Executes a print command */
BOOL
/* #AS#
   TRUE if succeeded, otherwise FALSE */
Misc_ExecutePrintCmd(
	LPSTR pszPrintFile /* #IN# Print command to execute */
)
{
	BOOL bResult = TRUE;

	/* Unfortunately, there are some problems with automatically return to
	   flipped full-screen mode, go to windowed instead */
	if( _IsFlagSet( g_Screen.ulState, SM_MODE_FULL ) /*&& _IsFlagSet( g_Screen.ulState, SM_OPTN_FLIP_BUFFERS )*/ )
	{
		/* The only safe method to doing it here */
		PostMessage( g_hMainWnd, WM_COMMAND, ID_VIEW_TOGGLEMODES, 0L );
	}

	if( !_IsFlagSet( g_Misc.ulState, MS_USE_PRINT_COMMAND ) )
	{
		char szPath[ MAX_PATH + 1 ];
		char szFile[ MAX_PATH + 1 ];
		int  nResult;

		_strncpy( szPath, pszPrintFile, MAX_PATH );
		GetFolderPath( szPath, szFile );

		if( (nResult =
			(int)ShellExecute( g_hMainWnd,
							   "print",
							   szFile,
							   NULL,
							   szPath,
							   SW_HIDE )) < 32 )
		{
			Log_print( "Printing error (shell execution code: %d)", nResult );
			bResult = FALSE;
		}
	}
	else
	{
		char szPrintCmd[ PRINT_CMD_LENGTH + 1 ];

		sprintf( szPrintCmd, Devices_print_command, pszPrintFile );
		if( !ExecuteCmd( szPrintCmd, FALSE ) )
			bResult = FALSE;
	}
	_ClrFlag( g_ulAtariState, ATARI_PAUSED );

	return bResult;

} /* #OF# Misc_ExecutePrintCmd */

/*========================================================
Function : Misc_SetProcessPriority
=========================================================*/
/* #FN#
   Sets the process priority level */
void
/* #AS#
   Nothing */
Misc_SetProcessPriority( void )
{
	SetPriorityClass( GetCurrentProcess(),
					  _IsFlagSet( g_Misc.ulState, MS_HIGH_PRIORITY ) ?
					  HIGH_PRIORITY_CLASS :
					  NORMAL_PRIORITY_CLASS );

} /* #OF# Misc_SetProcessPriority */

/*========================================================
Function : Misc_PrintTime
=========================================================*/
/* #FN#
   Prints out the current time in the 'Log' window */
void
/* #AS#
   Nothing */
Misc_PrintTime( void )
{
	time_t long_time;
	struct tm *t;

	time( &long_time );
	/* Convert to a local time */
	t = localtime( &long_time );

	Log_print( "Start log [%04d.%02d.%02d %02d:%02d:%02d]",
			t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
			t->tm_hour, t->tm_min, t->tm_sec );

} /* #OF# Misc_PrintTime */
