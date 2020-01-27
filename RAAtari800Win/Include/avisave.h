/****************************************************************************
File    : avisave.h
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# API AVI public methods and objects prototypes
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 30.09.2001
*/

#ifndef __AVISAVE_H__
#define __AVISAVE_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Exported methods */

void Video_OpenOutput ( char *pszOutFileName, BOOL bWithSound );
void Video_SaveFrame  ( UCHAR *pVideoBits, LONG nVideoBufLen, UCHAR *pSoundBits, LONG nSoundBufLen );
void Video_CloseOutput( void );

#ifdef __cplusplus
}
#endif

#endif /*__AVISAVE_H__*/
