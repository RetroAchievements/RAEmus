/****************************************************************************
File    : CheatServer.h
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Interface of the CCheatServer class
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 15.09.2002
*/

#ifndef __CHEATSERVER_H__
#define __CHEATSERVER_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


/////////////////////////////////////////////////////////////////////////////
// CCheatServer helper class

typedef	CList<DWORD,DWORD>	CAddressList;

class CCheatServer
{
// Construction/Destruction
public:
	CCheatServer();   // standard constructor
	CCheatServer( CCheatServer& refCheatServer );	// copy construktor
	~CCheatServer();

// Interface: operations
public:
	void ResetState();

	int  ShotMemo( UBYTE nMatch, const UBYTE *pMemory, DWORD dwSize );
	int  ShotAddr( UBYTE nMatch );

	void ImportAddr( const PDWORD pValues, int nCount, const UBYTE *pMemory = NULL, DWORD dwSize = 65536 );
	void ExportAddr( PDWORD *ppValues, int *pCount, BOOL bLocked = TRUE );

	void Frame();

	int  GetMemoCount() { return m_nMemoCount; }
	int  GetAddrCount() { return m_nAddrCount; }
	int  GetLockCount() { return m_nLockCount; }

	int  ListToCtrl( CListCtrl *pList );
	int  CtrlToList( CListCtrl *pList );

	CCheatServer& operator=( CCheatServer& refCheatServer );

// Interface: attributes
public:

// Implementation: operations
private:
	void CopyAddressList( CAddressList *pTargetList, const CAddressList *pSourceList );

// Implementation: attributes
private:
	UBYTE *m_pMemory;
	DWORD  m_dwSize;

	int m_nMemoCount;
	int m_nAddrCount;
	int m_nLockCount;

	CAddressList m_listMemo;
	CAddressList m_listAddr;
};

/////////////////////////////////////////////////////////////////////////////

#endif //__CHEATSERVER_H__
