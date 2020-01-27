/****************************************************************************
File    : Atari800Win.h
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Main header file for the Atari800Win PLus application
@(#) #BY# Richard Lawrence, Tomasz Szymankowski
@(#) #LM# 10.03.2002
*/

#ifndef __ATARI800WIN_H__
#define __ATARI800WIN_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'StdAfx.h' before including this file for PCH
#endif

#include "Resource.h"       // Main symbols
#include "CheatServer.h"	// Peek & Poke


/////////////////////////////////////////////////////////////////////////////
// CAtari800WinApp:
// See Atari800Win.cpp for the implementation of this class
//

class CMainFrame;

class CAtari800WinCommandLineInfo : public CCommandLineInfo
{
public:
	virtual void ParseParam( LPCSTR pszParam, BOOL bFlag, BOOL bLast );
};

class CAtari800WinApp : public CWinApp
{
// Construction
public:
	CAtari800WinApp();

// Interface: operations
public:
	ULONG GetCurrentSpeed () { return m_ulSpeed; };
	void  ResetLoopCounter( BOOL bSetSpeed = TRUE );

	CCheatServer* GetCheatServer() { return &m_cheatServer; }

// Interface: attributes
public:
	CMainFrame *m_pMainWindow;

// Implementation: operations
private:

// Implementation: overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAtari800WinApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual int Run();
	//}}AFX_VIRTUAL

	BOOL Emulate();

// Implementation: attributes
private:
	int   m_nLoopCounter;
	ULONG m_ulStartMeasure;
	ULONG m_ulTotalTime;
	ULONG m_ulSpeed;
	ULONG m_ulEqualizer;
	BOOL  m_bSPIPrevState;

	CCheatServer m_cheatServer;

// Generated message map functions
protected:
	//{{AFX_MSG(CAtari800WinApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	afx_msg void OnFilePrintSetup();

	DECLARE_MESSAGE_MAP()
};

extern CAtari800WinApp theApp;

// Data for threaded emulation
extern CWinThread* EmuThread;
extern BOOL ThreadState;
extern CMutex* Surface1Mutex;
extern CMutex* Surface2Mutex;
extern BOOL Surface1Done, Surface2Done;

UINT Loop(LPVOID pParam);

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif //__ATARI800WIN_H__
