/****************************************************************************
File    : Helpers.h
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Global declaration of usefull methods and objects
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 03.10.2002
*/

#ifndef __HELPERS_H__
#define __HELPERS_H__

#ifdef __cplusplus
extern "C" {
#endif

#define SRW_VIDEO_STREAM	0x01
#define SRW_SOUND_STREAM	0x02

/* Initialisation and restarting */

BOOL  InitialiseMachine ( void );
void  RestartEmulation  ( BOOL bIgnoreParams );
void  HandleResetEvent  ( void );
void  ResetLoopCounter  ( BOOL bSetSpeed );
void  ResetCheatServer  ( void );

/* Updating the status bar */

void  UpdateIndicator   ( int nPane );
void  UpdateStatus      ( BOOL bForceShow, int nSpeed, int nPane, BOOL bWinMode );
void  UpdateWindowTitle ( BOOL bForceShow, int nSpeed, int nPane, BOOL bWinMode );

/* Displaying messages and warnings */

int   DisplayMessage    ( HWND hWnd, UINT uMessageID, UINT uCaptionID, UINT uType, ... );
BOOL  DisplayWarning    ( UINT uWarningID, UINT uDontShowFlag, BOOL bEnableCancel, ... );
BOOL  StopVideoRecording( UINT uContextID, BOOL bForceStop );
BOOL  StopSoundRecording( UINT uContextID, BOOL bForceStop );
BOOL  StreamWarning     ( UINT uWarningID, UINT nFlags );

char* LoadStringEx      ( UINT uStringID, char *pszBuffer, int nBufLen );

#ifdef __cplusplus
}
#endif

/* Exported globals */

extern BOOL g_bLargeFonts;


#endif /*__HELPERS_H__*/
