/****************************************************************************
File    : rdevice.h
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# R: device public methods and objects prototypes
@(#) #BY# Daniel Noguerol, Piotr Fusik
@(#) #LM# 13.07.2003
*/

#ifndef __RDEVICE_H__
#define __RDEVICE_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Constants declarations */

#define R_DEVICE_BEGIN	0xd180
#define R_TABLE_ADDRESS	0xd180
#define R_PATCH_OPEN	0xd190
#define R_PATCH_CLOS	0xd193
#define R_PATCH_READ	0xd196
#define	R_PATCH_WRIT	0xd199
#define R_PATCH_STAT	0xd19c
#define R_PATCH_SPEC	0xd19f
#define R_DEVICE_END	0xd1a1

#define WS_ERROR_LOAD	1
#define WS_ERROR_FUNC	2

/* these codes shouldn't collide with other escape codes in atari.h */
enum {
	ESC_RHOPEN = 0xd0,
	ESC_RHCLOS = 0xd1,
	ESC_RHREAD = 0xd2,
	ESC_RHWRIT = 0xd3,
	ESC_RHSTAT = 0xd4,
	ESC_RHSPEC = 0xd5,
	ESC_RHINIT = 0xd6
};

/* Exported methods */

void RDevice_Frame        ( void );		/* Call before Atari800_Frame() */
void RDevice_UpdatePatches( void );		/* Call before or after ESC_UpdatePatches */
void RDevice_Exit         ( void );		/* Call in Atari_Exit() */
int  RDevice_LoadLibrary  ( PHANDLE pHandle );
void RDevice_FreeLibrary  ( void );
int  RDevice_IsCapable    ( void );

#ifdef __cplusplus
}
#endif

#endif /*__RDEVICE_H__*/
