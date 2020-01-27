/****************************************************************************
File    : globals.h
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Shared C-core attributes declarations
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 12.07.2003
*/

#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Shared C-core specific const definitions */

#define VERSION_INFO			"Atari800Win PLus 4.1"

#define ATARI_UNINITIALIZED		0x0001	/* Various machine states the Atari can be in */
#define ATARI_RUNNING			0x0002
#define ATARI_PAUSED			0x0004
#define ATARI_NO_FOCUS			0x0008
#define ATARI_LOAD_FAILED		0x0010
#define ATARI_CRASHED			0x0020
#define ATARI_CLOSING			0x0040
#define ATARI_MONITOR			0x0080

#define DEF_START_X				0		/* Default values */
#define DEF_START_Y				0

#define DEF_RAMSIZE_OSA			48
#define DEF_RAMSIZE_OSB			48
#define DEF_RAMSIZE_XLXE		64

#define DEF_PRINT_COMMAND		"Notepad %s"
#define PRINT_CMD_LENGTH		255

#define CX_BORDER				2		/* Note: afxData.cxBorder and afxData.cyBorder aren't used anymore */
#define CY_BORDER				2

#define LOADSTRING_SIZE_S		128
#define LOADSTRING_SIZE_M		512
#define LOADSTRING_SIZE_L		1536

#define ID_INDICATOR_MSG		0
#define ID_INDICATOR_RUN		1
#define ID_INDICATOR_VID		2
#define ID_INDICATOR_SIO		3
#define ID_INDICATOR_MSE		4
#define ID_INDICATOR_JOY		5
#define ID_INDICATOR_AVI		6
#define ID_INDICATOR_WAV		7
#define ID_INDICATOR_NET		8

#define WM_PLUS_CMDLINE			(WM_APP + 1)
#define WM_PLUS_KARESET			(WM_APP + 2)
#define WM_PLUS_KASTART			(WM_APP + 3)
#define WM_PLUS_KASTOP			(WM_APP + 4)
#define WM_PLUS_KAGAMESTART		(WM_APP + 5)
#define WM_PLUS_KAGAMESTOP		(WM_APP + 6)

/* AtariWin C modules shared objects */

extern HWND      g_hMainWnd;
extern HWND      g_hViewWnd;
extern HINSTANCE g_hInstance;

extern int       g_nStartX;
extern int       g_nStartY;
extern int       g_nTestVal;

extern ULONG     g_ulAtariState;
extern int       g_InitDone;

extern int       g_argc;
extern char     *g_argv          [];
extern char      g_szCmdLine     [];

extern int       g_anRamSize     [];

extern char atari_exe_dir[FILENAME_MAX];
extern char atari_state_dir[FILENAME_MAX]; 

#ifdef __cplusplus
}
#endif

#endif /*__GLOBALS_H__*/
