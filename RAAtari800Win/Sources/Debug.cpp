/****************************************************************************
File    : Debug.cpp
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Implementation of debugging time helpers
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 26.09.2001
*/

#include "StdAfx.h"
#include "Debug.h"

extern "C" { /* Do not decorate the function name, please */

#ifdef _DEBUG

void DebugTrace(
	char *pszFormat,
	...
)
{
	char szMessage[ 1024 ];

	va_list argList;
	va_start( argList, pszFormat );

	vsprintf( szMessage, pszFormat, argList );
	AfxTrace( szMessage, argList );

	va_end( argList );
}

#endif

} //extern "C"
