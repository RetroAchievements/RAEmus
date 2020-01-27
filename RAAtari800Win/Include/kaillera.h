/****************************************************************************
File    : kaillera.h
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Kaillera interface methods and objects prototypes
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 04.10.2002
*/

#ifndef __KAILLERA_H__
#define __KAILLERA_H__

#include "kailleraclient.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Constants declarations */

#define KS_SKIP_FRAMES				0x00000001
#define KS_SYNC_GAME				0x00000002

#define DEF_KAILLERA_STATE			0
#define DEF_KAILLERA_LOCAL_PORT		0
#define DEF_KAILLERA_FRAME_SKIP		1

#define GAME_NAME_LENGTH			255

#define KA_ERROR_LOAD				1
#define KA_ERROR_FUNC				2

#define ST_KAILLERA_ACTIVE			(Kaillera_IsDlgActive())
#define ST_NETGAME_ACTIVE			(Kaillera_IsGameActive())

/* Exported methods */

void Kaillera_Initialise  ( void );
void Kaillera_Exit        ( void );
void Kaillera_GameStart   ( void );
void Kaillera_GameStop    ( void );
BOOL Kaillera_IsGameActive( void );
BOOL Kaillera_IsDlgActive ( void );
int  Kaillera_GetPlayerNo ( void );
void Kaillera_Frame       ( void );
void Kaillera_SelectServer( void );
void Kaillera_SetInfo     ( char *pszGameName );
void Kaillera_GetVersion  ( char *pBuffer );
int  Kaillera_LoadLibrary ( PHANDLE pHandle );
void Kaillera_FreeLibrary ( void );
int  Kaillera_IsCapable   ( void );

/* Exported globals */

struct KailleraCtrl_t
{
	ULONG ulState;				/* Kaillera option flags */
	char  szGameName[ GAME_NAME_LENGTH + 2 ];
	UINT  unBootImage;
	int   nLocalPort;
	int   nFrameSkip;
};
extern struct KailleraCtrl_t g_Kaillera;

#ifdef __cplusplus
}
#endif

#endif /*__KAILLERA_H__*/
