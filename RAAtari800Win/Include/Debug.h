/****************************************************************************
File    : Helpers.h
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Declaration of debug time helpers
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 01.09.2002
*/

#ifndef __DEBUG_H__
#define __DEBUG_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _DEBUG

void DebugTrace( char *pszFormat, ... );

#define _TRACE0(sz)                  DebugTrace("%s", sz)
#define _TRACE1(sz, p1)              DebugTrace(sz, p1)
#define _TRACE2(sz, p1, p2)          DebugTrace(sz, p1, p2)
#define _TRACE3(sz, p1, p2, p3)      DebugTrace(sz, p1, p2, p3)
#define _TRACE4(sz, p1, p2, p3, p4)  DebugTrace(sz, p1, p2, p3, p4)

#else

#define _TRACE0(sz)
#define _TRACE1(sz, p1)
#define _TRACE2(sz, p1, p2)
#define _TRACE3(sz, p1, p2, p3)
#define _TRACE4(sz, p1, p2, p3, p4)

#endif

#ifdef __cplusplus
}
#endif


#endif /*__DEBUG_H__*/
