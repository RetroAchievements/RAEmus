/****************************************************************************
File    : CheatServer.cpp
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# CCheatServer implementation file
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 15.09.2002
*/

#include "StdAfx.h"
#include "CheatServer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MAX_ITEM_LENGTH		16
#define LOCK_MASK			0x80000000
#define ADDR_MASK			0x7fffffff


/////////////////////////////////////////////////////////////////////////////
// CCheatServer class

/*========================================================
Method   : CCheatServer::CCheatServer
=========================================================*/
/* #FN#
   Standard constructor */
CCheatServer::
CCheatServer()
	: m_pMemory   ( NULL ),
	  m_dwSize    ( 0 ),
	  m_nMemoCount( 0 ),
	  m_nAddrCount( 0 ),
	  m_nLockCount( 0 )
{
} /* #OF# CCheatServer::CCheatServer */

/*========================================================
Method   : CCheatServer::CCheatServer
=========================================================*/
/* #FN#
   Copy constructor */
CCheatServer::
CCheatServer(
	CCheatServer& refCheatServer /* #IN# Reference to a source object */
)
	: m_pMemory   ( refCheatServer.m_pMemory ),
	  m_dwSize    ( refCheatServer.m_dwSize ),
	  m_nMemoCount( refCheatServer.m_nMemoCount ),
	  m_nAddrCount( refCheatServer.m_nAddrCount ),
	  m_nLockCount( refCheatServer.m_nLockCount )
{
	CopyAddressList( &m_listMemo, &refCheatServer.m_listMemo );
	CopyAddressList( &m_listAddr, &refCheatServer.m_listAddr );

} /* #OF# CCheatServer::CCheatServer */

/*========================================================
Method   : CCheatServer::~CCheatServer
=========================================================*/
/* #FN#
   Destructor */
CCheatServer::
~CCheatServer()
{
	ResetState();

} /* #OF# CCheatServer::~CCheatServer */


/////////////////////////////////////////////////////////////////////////////
// CCheatServer implementation

/*========================================================
Method   : CCheatServer::operator=
=========================================================*/
/* #FN#
   Overloads the operator= */
CCheatServer&
/* #AS#
   Reference to the target object */
CCheatServer::
operator=(
	CCheatServer& refCheatServer /* #IN# Reference to a source object */
)
{
	m_pMemory    = refCheatServer.m_pMemory;
	m_dwSize     = refCheatServer.m_dwSize;
	m_nMemoCount = refCheatServer.m_nMemoCount;
	m_nAddrCount = refCheatServer.m_nAddrCount;
	m_nLockCount = refCheatServer.m_nLockCount;

	CopyAddressList( &m_listMemo, &refCheatServer.m_listMemo );
	CopyAddressList( &m_listAddr, &refCheatServer.m_listAddr );

	return *this;

} /* #OF# CCheatServer::operator= */

/*========================================================
Method   : CCheatServer::CopyAddressList
=========================================================*/
/* #FN#
   Copies one address list to another */
void
/* #AS#
   Nothing */
CCheatServer::
CopyAddressList(
	CAddressList *pTargetList, /* #OUT# Destination list of addresses */
	const CAddressList *pSourceList /* #IN# Source list of addresses */
)
{
	if( pTargetList != pSourceList )
	{
		POSITION pos = pSourceList->GetHeadPosition();
		pTargetList->RemoveAll();

		while( pos )
			pTargetList->AddTail( pSourceList->GetNext( pos ) );
	}
} /* #OF# CCheatServer::CopyAddressList */

/*========================================================
Method   : CCheatServer::ResetState
=========================================================*/
/* #FN#
   Resets state of the object */
void
/* #AS#
   Nothing */
CCheatServer::
ResetState()
{
	m_pMemory    = NULL;
	m_dwSize     = 0;
	m_nMemoCount = 0;
	m_nAddrCount = 0;
	m_nLockCount = 0;

	m_listMemo.RemoveAll();
	m_listAddr.RemoveAll();

} /* #OF# CCheatServer::ResetState */

/*========================================================
Method   : CCheatServer::ShotMemo
=========================================================*/
/* #FN#
   Finds out memory addresses that contain a given value */
int
/* #AS#
   Number of found memory addresses */
CCheatServer::
ShotMemo(
	UBYTE nMatch,
	const UBYTE *pMemory,
	DWORD dwSize
)
{
	m_nMemoCount = 0;
	m_listMemo.RemoveAll();

	if( pMemory )
	{
		m_pMemory = (UBYTE *)pMemory;
		m_dwSize  = dwSize;

		for( DWORD dwAddr = 0; dwAddr < dwSize; dwAddr++ )
		{
			if( nMatch == pMemory[ dwAddr ] )
			{
				m_listMemo.AddTail( dwAddr );
				m_nMemoCount++;
			}
		}
	}
	return m_nMemoCount;

} /* #OF# CCheatServer::ShotMemo */

/*========================================================
Method   : CCheatServer::ShotAddr
=========================================================*/
/* #FN#
   Finds out memory addresses with changed value */
int
/* #AS#
   Number of found memory addresses */
CCheatServer::
ShotAddr(
	UBYTE nMatch
)
{
	POSITION pos = m_listMemo.GetHeadPosition();
	DWORD dwAddr = 0;

	m_nAddrCount = m_nLockCount = 0;
	m_listAddr.RemoveAll();

	while( pos )
	{
		dwAddr = m_listMemo.GetNext( pos );

		if( dwAddr < m_dwSize && nMatch == m_pMemory[ dwAddr ] )
		{
			m_listAddr.AddTail( dwAddr );
			m_nAddrCount++;
		}
	}
	return m_nAddrCount;

} /* #OF# CCheatServer::ShotAddr */

/*========================================================
Method   : CCheatServer::ImportAddr
=========================================================*/
/* #FN#
   Allows to set memory addresses of the cheat object */
void
/* #AS#
   Nothing */
CCheatServer::
ImportAddr(
	const PDWORD pValues,
	int   nCount,
	const UBYTE *pMemory /*=NULL*/,
	DWORD dwSize /*=65536*/
)
{
	if( NULL != pValues )
	{
		DWORD dwAddr = 0;

		m_nMemoCount = m_nAddrCount = m_nLockCount = 0;
		m_dwSize = dwSize;

		if( pMemory )
			m_pMemory = (UBYTE *)pMemory;

		m_listMemo.RemoveAll();
		m_listAddr.RemoveAll();

		for( int i = 0; i < nCount; i++ )
		{
			dwAddr = pValues[ i ];

			if( (dwAddr & ADDR_MASK) < dwSize )
			{
				m_listAddr.AddTail( dwAddr );

				if( 0 != (dwAddr & LOCK_MASK) )
					m_nLockCount++;

				m_nAddrCount++;
			}
		}
	}
} /* #OF# CCheatServer::ImportAddr */

/*========================================================
Method   : CCheatServer::ExportAddr
=========================================================*/
/* #FN#
   Allows to get memory addresses from the cheat object */
void
/* #AS#
   Nothing */
CCheatServer::
ExportAddr(
	PDWORD *ppValues,         /* #OUT# Pointer to allocated buffer */
	int    *pCount,           /* #OUT# Number of exported items    */
	BOOL    bLocked /*=TRUE*/ /* #IN#  Export only locked elements */
)
{
	int nCount = 0;

	if( NULL != ppValues && !m_listAddr.IsEmpty() )
	{
		*ppValues = (PDWORD)calloc( m_listAddr.GetCount(), sizeof(DWORD) );

		if( NULL != *ppValues )
		{
			POSITION pos = m_listAddr.GetHeadPosition();
			DWORD dwAddr = 0;

			while( pos )
			{
				dwAddr = m_listAddr.GetNext( pos );

				if( !bLocked || 0 != (dwAddr & LOCK_MASK) )
				{
					(*ppValues)[ nCount++ ] = dwAddr;
				}
			}
		}
	}
	*pCount = nCount;

} /* #OF# CCheatServer::ExportAddr */

/*========================================================
Method   : CCheatServer::ListToCtrl
=========================================================*/
/* #FN#
   Polulates a list control basing on state of the object */
int
/* #AS#
   Number of inserted items */
CCheatServer::
ListToCtrl(
	CListCtrl *pList
)
{
	int nCount = 0;
	if( pList )
	{
		POSITION pos = m_listAddr.GetHeadPosition();
		DWORD  dwAddr = 0;
		char   szAddr[ MAX_ITEM_LENGTH ];

		LVITEM itemAddr;
		ZeroMemory( &itemAddr, sizeof(LVITEM) );

		pList->DeleteAllItems();

		while( pos )
		{
			dwAddr = m_listAddr.GetNext( pos );
			sprintf( szAddr, "$%.4X (%.5ld)", dwAddr & ADDR_MASK, dwAddr & ADDR_MASK );

			itemAddr.mask    = LVIF_TEXT | LVIF_PARAM;
			itemAddr.iItem   = nCount;
			itemAddr.pszText = (LPTSTR)szAddr;
			itemAddr.lParam  = dwAddr & ADDR_MASK;

			pList->InsertItem( &itemAddr );

			if( 0 != (dwAddr & LOCK_MASK) )
				pList->SetCheck( nCount );

			nCount++;
		}
	}
	TRACE1("CCheatServer::ListToCtrl: %d items\n", nCount);
	return nCount;

} /* #OF# CCheatServer::ListToCtrl */

/*========================================================
Method   : CCheatServer::CtrlToList
=========================================================*/
/* #FN#
   Updates a state of the object basing on a list control */
int
/* #AS#
   Number of updated items */
CCheatServer::
CtrlToList(
	CListCtrl *pList
)
{
	int nCount = 0;

	m_nLockCount = 0;
	if( pList )
	{
		POSITION pos = m_listAddr.GetHeadPosition();
		DWORD dwAddr = 0;

		while( pos )
		{
			dwAddr = pList->GetItemData( nCount );

			if( dwAddr == (m_listAddr.GetAt( pos ) & ADDR_MASK) )
			{
				if( pList->GetCheck( nCount ) )
				{
					m_listAddr.SetAt( pos, dwAddr |= LOCK_MASK );
					m_nLockCount++;
				}
				else
					m_listAddr.SetAt( pos, dwAddr &= ADDR_MASK );

				nCount++;
			}
			m_listAddr.GetNext( pos );
		}
	}
	return nCount;

} /* #OF# CCheatServer::CtrlToList */

/*========================================================
Method   : CCheatServer::Frame
=========================================================*/
/* #FN#
   Updates memory registers basing on a state of the object */
void
/* #AS#
   Nothing */
CCheatServer::
Frame()
{
	if( m_nLockCount > 0 )
	{
		POSITION pos = m_listAddr.GetHeadPosition();
		DWORD dwAddr = 0;

		while( pos )
		{
			dwAddr = m_listAddr.GetNext( pos );
			if( 0 != (dwAddr & LOCK_MASK) )
			{
				m_pMemory[ dwAddr & ADDR_MASK ] = (UBYTE)g_Misc.Cheat.nLock;
			}
		}
	}
} /* #OF# CCheatServer::Frame */
