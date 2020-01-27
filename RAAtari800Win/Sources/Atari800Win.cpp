/****************************************************************************
File    : Atari800Win.cpp
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Defines the class behaviors for the application.
@(#) #BY# Richard Lawrence, Tomasz Szymankowski
@(#) #LM# 10.10.2003
*/

#include "StdAfx.h"
#include "Atari800Win.h"
#include "Atari800WinDoc.h"
#include "Atari800WinView.h"
#include "MainFrame.h"
#include "Helpers.h"
#include "RetroAchievements.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// Static objects

static int s_nCmdIndex;


/////////////////////////////////////////////////////////////////////////////
// CAtari800WinCommandLineInfo

/*========================================================
Method   : CAtari800WinCommandLineInfo::ParseParam
=========================================================*/
/* #FN#
   The framework calls this function to parse/interpret individual
   parameters from the command line */
void
/* #AS#
   Nothing */
CAtari800WinCommandLineInfo::
ParseParam(
	LPCSTR pszParam,
	BOOL   bFlag,
	BOOL   bLast
)
{
	LPSTR pszTmpParam = (LPSTR)pszParam;

	/* It would be nice to remember a long form of the file name... */
	if( strchr( pszTmpParam, '\\' ) )
	{
		WIN32_FIND_DATA fileData;
		HANDLE hFile = ::FindFirstFile( pszTmpParam, &fileData );

		if( INVALID_HANDLE_VALUE != hFile )
		{
			char szLongPath[ MAX_PATH + 1 ];

			::FindClose( hFile );
			::GetCurrentDirectory( MAX_PATH, szLongPath );
			strcat( szLongPath, "\\" );
			strcat( szLongPath, fileData.cFileName );
			/* Test what we have got, please */
			if( INVALID_HANDLE_VALUE != (hFile = ::FindFirstFile( szLongPath, &fileData )) )
			{
				::FindClose( hFile );
				/* OK, it seems to be correct */
				pszTmpParam = szLongPath;
			}
		}
	}
	g_argv[ g_argc ] = &g_szCmdLine[ s_nCmdIndex ];
	g_argc++;
	if( bFlag )
		g_szCmdLine[ s_nCmdIndex++ ] = '-';
	strcpy( &g_szCmdLine[ s_nCmdIndex ], pszTmpParam );
	s_nCmdIndex += strlen( &g_szCmdLine[ s_nCmdIndex ] ) + 1;

//	CCommandLineInfo::ParseParam( pszParam, bFlag, bLast );

} /* #OF# CAtari800WinCommandLineInfo::ParseParam */


/////////////////////////////////////////////////////////////////////////////
// CAtari800WinApp

BEGIN_MESSAGE_MAP(CAtari800WinApp, CWinApp)
	//{{AFX_MSG_MAP(CAtari800WinApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard print setup command
//	ON_COMMAND(ID_FILE_PRINT_SETUP, OnFilePrintSetup)
END_MESSAGE_MAP()


/*========================================================
Method   : CAtari800WinApp::CAtari800WinApp
=========================================================*/
/* #FN#
   Standard constructor */
CAtari800WinApp::
CAtari800WinApp()
{
	m_nLoopCounter   = 127;
	m_ulStartMeasure = timeGetTime();
	m_ulTotalTime    = 0L;
	m_ulSpeed        = 100L;
	m_ulEqualizer    = 1L;
	m_bSPIPrevState  = FALSE;

} /* #OF# CAtari800WinApp::CAtari800WinApp */


/////////////////////////////////////////////////////////////////////////////
// The one and only CAtari800WinApp object

CAtari800WinApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CAtari800WinApp message handlers

/*========================================================
Method   : CAtari800WinApp::InitInstance
=========================================================*/
/* #FN#
   Instance initialization that runs each time a copy of the
   program runs */
BOOL
/* #AS#
   Nonzero if initialization is successful; otherwise 0 */
CAtari800WinApp::
InitInstance()
{
	char szHomeDir[ MAX_PATH + 1 ];
	

	g_hInstance = AfxGetInstanceHandle();

	/* We must prepare a DOS-style command line */
	g_argc = 1;
	g_argv[ 0 ] = g_szCmdLine;
	strcpy( g_szCmdLine, this->m_pszAppName );
	s_nCmdIndex = strlen( g_szCmdLine ) + 1;

	/* Parse command line for shell commands, DDE, file open */
	CAtari800WinCommandLineInfo cmdInfo;
	ParseCommandLine( cmdInfo );

	/* Needed for ROM searching when the Registry are not initialized */
	BuildCRCTable();
	/* Read the Registry stored values */
	if( HandleRegistry() )
	{
		BOOL bReboot = FALSE;
		/* This is the first time start-up, launch the Setup Wizard */
		if( !CMainFrame::LaunchWizard( NULL, bReboot ) )
		{
			DisplayMessage( NULL, IDS_WIZARD_TIP, 0, MB_ICONINFORMATION | MB_OK );
		}
	}
	/* Disable multiple instances? */
	if( _IsFlagSet( g_Misc.ulState, MS_REUSE_WINDOW ) && (hAppWnd = Misc_FindWindow( VERSION_INFO, 0 )) )
	{
		HANDLE hFileMap  = NULL;
		HWND   hPopupWnd = GetLastActivePopup( hAppWnd );

		BringWindowToTop( hAppWnd );
		if( IsIconic( hPopupWnd ) )
		{
			ShowWindow( hPopupWnd, SW_RESTORE );
		}
		else
			SetForegroundWindow( hPopupWnd );

		/* The current command line has to be sent to an active
		   instance of the emulator */
		if( NULL != (hFileMap = CreateFileMapping( INVALID_HANDLE_VALUE, NULL,
			PAGE_READWRITE, 0, s_nCmdIndex, "Atari800CmdLine" )) )
		{
			LPSTR pszMsg = (LPSTR)MapViewOfFile( hFileMap, FILE_MAP_WRITE, 0, 0, 0 );
			if( pszMsg )
			{
				CopyMemory( pszMsg, g_szCmdLine, s_nCmdIndex );
				UnmapViewOfFile( (LPCVOID)pszMsg );
				SendMessage( hAppWnd, WM_PLUS_CMDLINE, g_argc, s_nCmdIndex );
			}
			CloseHandle( hFileMap );
		}
		return FALSE;
	}
	/* Check Hight Performance Timer resolution */
	if( !Timer_Examine() )
	{
		DisplayMessage( NULL, IDS_ERROR_NO_HITIMER, 0, MB_ICONSTOP | MB_OK );
		return FALSE;
	}
	/* Get information about the system the emulator is running on */
	Misc_GetSystemInfo( &g_Misc.unSystemInfo );

	/* Install Low Level Keyboard Hook when running on NT */
	if( _IsFlagSet( g_Misc.unSystemInfo, SYS_WIN_NT ) )
		Input_InstallKeyboardHook( TRUE );

	/* Enable menu underlines when mouse is used for accessing the options */
#if WINVER >= 0x0500
	if( _IsFlagSet( g_Misc.unSystemInfo, SYS_WIN_NT5 ) )
	{
		BOOL bEnable = TRUE;
		SystemParametersInfo( SPI_GETMENUUNDERLINES, 0, &m_bSPIPrevState, 0 );
		SystemParametersInfo( SPI_SETMENUUNDERLINES, 0, (LPVOID)bEnable, 0 );
	}
#endif

	/* Standard initialization
	   If you are not using these features and wish to reduce the
	   size of your final executable, you should remove from the
	   following the specific initialization routines you do not need.
	*/

	/* Register the application's document templates. Document templates
	   serve as the connection between documents, frame windows and views. */

	CSingleDocTemplate *pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CAtari800WinDoc),
		RUNTIME_CLASS(CMainFrame),       /* main SDI frame window */
		RUNTIME_CLASS(CAtari800WinView));
	AddDocTemplate(pDocTemplate);

	/* Dispatch commands specified on the command line */
	if( !ProcessShellCommand( cmdInfo ) )
		return FALSE;

	m_pMainWindow = (CMainFrame *)m_pMainWnd;
	/* Describe a current state of the emulator */
	m_pMainWindow->UpdateStatus( TRUE, -1, ID_INDICATOR_RUN, TRUE );

	/* The one and only window has been initialized, so show and update it */
	m_pMainWnd->ShowWindow( SW_SHOW );
	m_pMainWnd->UpdateWindow();

	/* For a variety of reasons it's better to set the current working
	   directory to where Atari800Win (the executable) is located */
	if( Misc_GetHomeDirectory( szHomeDir ) )
		SetCurrentDirectory( szHomeDir );

	/* Reset Loop Counter */
	ResetLoopCounter();

	Surface1Done = Surface2Done = FALSE;
	Surface1Mutex = new CMutex();
	Surface2Mutex = new CMutex();
	EmuThread = NULL;
	ThreadState = FALSE;
	if ( _IsFlagSet( g_Misc.ulState, MS_THREADED_EMULATION ) )
	{
		int i;
		ThreadState = TRUE;
		EmuThread = AfxBeginThread( Loop, &i );
		if ( !EmuThread ) {
			_ClrFlag( g_Misc.ulState, MS_THREADED_EMULATION );
			ThreadState = FALSE;
		}
	}

	RA_InitSystem();
	RA_InitShared();

	g_InitDone = TRUE;
	return TRUE;
} /* #OF# CAtari800WinApp::InitInstance */

/*========================================================
Method   : CAtari800WinApp::ExitInstance
=========================================================*/
/* #FN#
   Called by the framework from within the Run member function
   to exit this instance of the application */
int
/* #AS#
   The application’s exit code; 0 indicates no errors, and
   values greater than 0 indicate an error */
CAtari800WinApp::
ExitInstance()
{
	if (EmuThread) {
		EmuThread->SuspendThread();
	}
	delete Surface1Mutex, Surface2Mutex;

#if WINVER >= 0x0500
	/* Disable menu underlines */
	if( _IsFlagSet( g_Misc.unSystemInfo, SYS_WIN_NT5 ) )
		SystemParametersInfo( SPI_SETMENUUNDERLINES, 0, (LPVOID)m_bSPIPrevState, 0 );
#endif

	/* Uninstall Low Level Keyboard Hook */
	if( _IsFlagSet( g_Misc.unSystemInfo, SYS_WIN_NT ) )
		Input_InstallKeyboardHook( FALSE );

	

	return CWinApp::ExitInstance();

} /* #OF# CAtari800WinApp::ExitInstance */

/*========================================================
Method   : CAtari800WinApp::OnFilePrintSetup
=========================================================*/
/* #FN#
   Handles execution of the File Print command */
void
/* #AS#
   Nothing */
CAtari800WinApp::
OnFilePrintSetup()
{
	CWinApp::OnFilePrintSetup();
	m_pMainWindow->CleanScreen();

} /* #OF# CAtari800WinApp::OnFilePrintSetup */


/////////////////////////////////////////////////////////////////////////////
// CAtari800WinApp commands

/*========================================================
Method   : CAtari800WinApp::ResetLoopCounter
=========================================================*/
/* #FN#
   Resets stuff related to an emulation speed measuring */
void
/* #AS#
   Nothing */
CAtari800WinApp::
ResetLoopCounter(
	BOOL bSetSpeed /*=TRUE*/
)
{
	m_ulStartMeasure = timeGetTime();
	m_nLoopCounter = -1;

	if( bSetSpeed && !_IsFlagSet( g_Misc.ulState, MS_FULL_SPEED ) )
		m_ulSpeed = g_Misc.nSpeedPercent;

	m_ulEqualizer = m_ulSpeed + 1;

} /* #OF# CAtari800WinApp::ResetLoopCounter */

/*========================================================
Method   : CAtari800WinApp::Run
=========================================================*/
int CAtari800WinApp::Run()
{
	ASSERT_VALID(this);
	_AFX_THREAD_STATE* pState = AfxGetThreadState();

	// for tracking the idle time state
	BOOL bIdle = TRUE;
	LONG lIdleCount = 0;

	// acquire and dispatch messages until a WM_QUIT message is received.
	for (;;)
	{
		int nRefreshRate = _GetRefreshRate();

		// phase1: check to see if we can do idle work
		while (bIdle &&
			!::PeekMessage(&(pState->m_msgCur), NULL, NULL, NULL, PM_NOREMOVE))
		{
			if ( _IsFlagSet( g_Misc.ulState, MS_THREADED_EMULATION ) &&
				!_IsFlagSet( g_Screen.ulState, SM_MODE_FULL) ) {
				if ( !ST_ATARI_STOPPED ) {
					if ( Surface1Done ) {
						if ( Surface1Mutex->Lock( 1 ) ) {
							s_Buffer.pSource = s_Buffer.pMainScr;

							if( ++g_nTestVal == nRefreshRate )
							{
								PLATFORM_DisplayScreen();
								Atari800_nframes++; /* For autofire */

								g_nTestVal = 0;
							}

							Surface1Done = FALSE;
							Surface1Mutex->Unlock();

							continue;
						}
					}
					else if ( Surface2Done ) {
						if ( Surface2Mutex->Lock( 1 ) ) {
							s_Buffer.pSource = s_Buffer.pBackScr;

							if( ++g_nTestVal == nRefreshRate )
							{
								PLATFORM_DisplayScreen();
								Atari800_nframes++; /* For autofire */

								g_nTestVal = 0;
							}

							Surface2Done = FALSE;
							Surface2Mutex->Unlock();

							continue;
						}
					}
				}

				SleepEx( 1, TRUE );
				bIdle = OnIdle( lIdleCount);
			}
			else {
				bIdle = Emulate();
				if ( bIdle )
					bIdle = OnIdle( lIdleCount );
			}
		}

		RA_HandleHTTPResults();
		RA_DoAchievementsFrame();

		// phase2: pump messages while available
		do
		{
			// pump message, but quit on WM_QUIT
			if (!PumpMessage())
				return ExitInstance();

			// reset "no idle" state after pumping "normal" message
			//if (IsIdleMessage(&m_msgCur))
			if (IsIdleMessage(&(pState->m_msgCur)))
			{
				bIdle = TRUE;
				lIdleCount = 0;
			}
		} while (::PeekMessage(&(pState->m_msgCur), NULL, NULL, NULL, PM_NOREMOVE));
	}
}

/*========================================================
Method   : CAtari800WinApp::Emulate
=========================================================*/
BOOL CAtari800WinApp::Emulate()
{
	if( (_IsFlagSet( g_Misc.ulState, MS_STOP_WHEN_NO_FOCUS ) &&
		 _IsFlagSet( g_ulAtariState, ATARI_NO_FOCUS )) || ST_ATARI_STOPPED || g_ulAtariState == 0 || g_InitDone == FALSE )
	{

		if( m_ulEqualizer != 1 )
		{
			m_pMainWindow->UpdateSpeed( FALSE, 0 );
			m_ulEqualizer = 1;
		}
		SleepEx(4, TRUE);
		/* Nothing to do at the moment */
		return FALSE;
	}
	/* Calculate the emulation speed */
	if( 128 == ++m_nLoopCounter )
	{
		m_ulTotalTime = timeGetTime() - m_ulStartMeasure;

		/* Full time for PAL would be 1280ms, for NTSC 1067ms */
		m_ulSpeed = MulDiv( Atari800_TV_PAL == Atari800_tv_mode ? 2560L : 2134L, 100, m_ulTotalTime ? m_ulTotalTime : 1 );

		m_ulStartMeasure = timeGetTime();
		m_nLoopCounter = 0;
	}
	/* Update information on the status bar in equal periods of time */
	if( m_ulEqualizer > 0 && --m_ulEqualizer == 0 )
	{
		m_ulEqualizer = m_ulSpeed + 1;
		/* Display an emulation speed */
		m_pMainWindow->UpdateSpeed();
	}

	/* Do the emulation work */
	if( _IsFlagSet( g_ulAtariState, ATARI_RUNNING ) )
	{
		int nRefreshRate = _GetRefreshRate();

		/* The "Hide pointer..." option and the mouse emulation need this */
		Input_UpdateMouse();
		Input_UpdateJoystick();

		INPUT_key_code = Atari_Keyboard();

#ifdef WIN_NETWORK_GAMES
		Kaillera_Frame();

#ifdef _DEBUG
		static int nFrameCount = 1;

		if( Input_GetStick( 0 ) != 0x0f || Input_GetStick( 1 ) != 0x0f ||
			Input_GetTrig( 0 ) != 1 || Input_GetTrig( 1 ) != 1 )
		{
			TRACE2("[%05d:%03d]: ", nFrameCount, MEMORY_mem[ 20 ]);
			TRACE2("rand: %07d, xpos: %02d, ", POKEY_GetRandomCounter(), ANTIC_xpos);
			TRACE2("joy1=0x%02x, joy2=0x%02x, ", Input_GetStick( 0 ), Input_GetStick( 1 ));
			TRACE2("trig1=0x%02x, trig2=0x%02x\n", Input_GetTrig( 0 ), Input_GetTrig( 1 ));
		}
		if( Kaillera_IsGameActive() ) nFrameCount++;
#endif
#endif /*WIN_NETWORK_GAMES*/

		RDevice_Frame();
		Devices_Frame();

		INPUT_Frame();
		GTIA_Frame();

		/* Generate screen */
		ANTIC_Frame( (_IsFlagSet( g_Misc.ulState, MS_NO_DRAW_DISPLAY ) && g_nTestVal != nRefreshRate - 1) ? 0 : 1 );
		POKEY_Frame();

		Atari_PlaySound();

		if( _IsFlagSet( g_Input.ulState, IS_CAPTURE_MOUSE ) )
			INPUT_DrawMousePointer();

		if( _IsFlagSet( g_Misc.ulState, MS_CHEAT_LOCK ) )
			m_cheatServer.Frame();

		if( _IsFlagSet( g_Misc.ulState, MS_NO_ATTRACT_MODE ) )
			/* Reset attract mode timer and flag. Attract mode rotates colors
			   on your screen at low luminance levels when the computer is on
			   but no keyboard input is read for a long time. Unfortunately, it
			   may interfere with using the emulator when in full speed mode. */
			Misc_CheckAttractCounter();

		if( _IsFlagSet( g_Misc.ulState, MS_SHOW_DRIVE_LED ) ) {
			Screen_DrawDiskLED();
		}
		
		if ( !_IsFlagSet( g_Misc.ulState, MS_THREADED_EMULATION ) ||
			_IsFlagSet( g_Screen.ulState, SM_MODE_FULL ) )
			if( ++g_nTestVal == nRefreshRate )
			{
				PLATFORM_DisplayScreen();
				Atari800_nframes++; /* For autofire */

				g_nTestVal = 0;
			}
	}

	Timer_WaitForVBI();
	return TRUE;
}

// Threaded emulation
CWinThread* EmuThread;
BOOL ThreadState;
CMutex* Surface1Mutex;
CMutex* Surface2Mutex;
BOOL Surface1Done, Surface2Done;

UINT Loop(LPVOID pParam)
{
	for (;;) {
		for (;;)
		{
			if ( !ThreadState )
				return 1;
			if( (_IsFlagSet( g_Misc.ulState, MS_STOP_WHEN_NO_FOCUS ) &&
				 _IsFlagSet( g_ulAtariState, ATARI_NO_FOCUS )) || ST_ATARI_STOPPED )
			{
				SleepEx( 4, TRUE );
				continue;
			}
			if ( Surface1Mutex->Lock( 1 ) ) {
				Surface1Done = FALSE;
				Screen_atari = (ULONG *) s_Buffer.pMainScr;
				theApp.Emulate();
				Surface1Done = TRUE;
				Surface1Mutex->Unlock();
				break;
			}
		}
		for (;;) {
			if ( !ThreadState )
				return 1;
			if( (_IsFlagSet( g_Misc.ulState, MS_STOP_WHEN_NO_FOCUS ) &&
				 _IsFlagSet( g_ulAtariState, ATARI_NO_FOCUS )) || ST_ATARI_STOPPED )
			{
				SleepEx( 4, TRUE );
				continue;
			}
			if ( Surface2Mutex->Lock( 1 ) ) {
				Surface2Done = FALSE;
				Screen_atari = (ULONG *) s_Buffer.pBackScr;
				theApp.Emulate();
				Surface2Done = TRUE;
				Surface2Mutex->Unlock();
				break;
			}
		}
	}
}
