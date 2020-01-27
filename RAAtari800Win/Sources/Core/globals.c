/****************************************************************************
File    : globals.c
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Shared C-core attributes definitions
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 22.09.2002
*/

#include <windows.h>
#include "globals.h"


HWND      g_hMainWnd      = NULL;
HWND      g_hViewWnd      = NULL;
HINSTANCE g_hInstance     = NULL;

int       g_nStartX       = DEF_START_X;
int       g_nStartY       = DEF_START_Y;
int       g_nTestVal      = 0;

ULONG     g_ulAtariState  = ATARI_UNINITIALIZED;
int       g_InitDone      = FALSE;

int       g_argc          = 0;
char     *g_argv          [ 32 ];
char      g_szCmdLine     [ 2048 ];

int       g_anRamSize     [] = { DEF_RAMSIZE_OSA, DEF_RAMSIZE_OSB, DEF_RAMSIZE_XLXE };

char atari_exe_dir[FILENAME_MAX];
char atari_state_dir[FILENAME_MAX]; 
