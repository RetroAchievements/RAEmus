/****************************************************************************
File    : RomImagesBase.cpp
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# CRomImagesBase implementation file
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 22.09.2002
*/

#include "StdAfx.h"
#include "Atari800Win.h"
#include "Helpers.h"
#include "FileService.h"
#include "FileSmallDlg.h"
#include "RomImagesBase.h"
#include "cfg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// Static objects

static char s_szSelectRom[ ROM_TYPES_NO ][ LOADSTRING_SIZE_S + 1 ];

static CRomImagesBase::RomData_t s_aRomData[ ROM_TYPES_NO ] =
{
	{ CFG_osa_filename,   FILE_NONE, 0, _LoadStringSx( IDS_SELECT_ROM_OSA,   s_szSelectRom[ 0 ] ), REG_ROM_OSA,   DONT_SHOW_OSA_WARN   },
	{ CFG_osb_filename,   FILE_NONE, 0, _LoadStringSx( IDS_SELECT_ROM_OSB,   s_szSelectRom[ 1 ] ), REG_ROM_OSB,   DONT_SHOW_OSB_WARN   },
	{ CFG_xlxe_filename,  FILE_NONE, 0, _LoadStringSx( IDS_SELECT_ROM_XLXE,  s_szSelectRom[ 2 ] ), REG_ROM_XLXE,  DONT_SHOW_XLXE_WARN  },
	{ CFG_5200_filename,  FILE_NONE, 0, _LoadStringSx( IDS_SELECT_ROM_5200,  s_szSelectRom[ 3 ] ), REG_ROM_5200,  DONT_SHOW_5200_WARN  },
	{ CFG_basic_filename, FILE_NONE, 0, _LoadStringSx( IDS_SELECT_ROM_BASIC, s_szSelectRom[ 4 ] ), REG_ROM_BASIC, DONT_SHOW_BASIC_WARN }
};


/////////////////////////////////////////////////////////////////////////////
// CRomImagesBase class

/*========================================================
Method   : CRomImagesBase::CRomImagesBase
=========================================================*/
/* #FN#
   Standard constructor */
CRomImagesBase::
CRomImagesBase()
{
	m_pRomData = s_aRomData;

} /* #OF# RomImagesBase::CRomImagesBase */

/*========================================================
Method   : CRomImagesBase::~CRomImagesBase
=========================================================*/
/* #FN#
   Destructor */
CRomImagesBase::
~CRomImagesBase()
{
} /* #OF# RomImagesBase::~CRomImagesBase */

/*========================================================
Method   : RomImagesBase::KillfocusEditRom
=========================================================*/
/* #FN#
	Sets the specific ROM image */
void
/* #AS#
   Nothing */
CRomImagesBase::
KillfocusEditRom(
	RomType rtType /* #IN# Type of a ROM image */
)
{
	char szNewRom[ MAX_PATH + 1 ];

	GetWnd()->GetDlgItemText( m_pRomData[ rtType ].nCtrlID, szNewRom, MAX_PATH );
	_NullPathToNone( szNewRom );
	/* If the names are equal then don't go */
	if( _stricmp( m_pRomData[ rtType ].szNewName, szNewRom ) != 0 )
	{
		int   nCFOut;
		ULONG ulCRC   = CheckFile( szNewRom, &nCFOut );
		BOOL  bResult = TRUE;

		if( ulCRC && !nCFOut )
		{
			BOOL bFound = FALSE;
			for( int i = 0; i < g_nRomTypeInfoNo; i++ )
			{
				if( g_aRomTypeInfo[ i ].rtType == rtType &&
					g_aRomTypeInfo[ i ].ulCRC  == ulCRC )
				{
					bFound = TRUE;
					break;
				}
			}
			if( !bFound )
				bResult = DisplayWarning( IDS_WARN_CORRUPT, m_pRomData[ rtType ].nWarnBit, TRUE );
		}
		if( bResult )
			strcpy( m_pRomData[ rtType ].szNewName, szNewRom );

		/* Apply changes to dialog window */
		GetWnd()->SetDlgItemText( m_pRomData[ rtType ].nCtrlID, m_pRomData[ rtType ].szNewName );
	}
} /* #OF# RomImagesBase::OnKillfocusEditRom */

/*========================================================
Method   : RomImagesBase::BrowseRomFile
=========================================================*/
/* #FN#
   Allows selecting ROM image using FileDialog window */
void
/* #AS#
   Nothing */
CRomImagesBase::
BrowseRomFile(
	RomType rtType /* #IN# Type of a ROM image */
)
{
	char szNewRom[ MAX_PATH + 1 ];

	GetWnd()->GetDlgItemText( m_pRomData[ rtType ].nCtrlID, szNewRom, MAX_PATH );

	if( PickFileName( TRUE, szNewRom, m_pRomData[ rtType ].pszOpenPrompt, IDS_FILTER_ROM,
					  "rom", PF_LOAD_FLAGS, TRUE, FILE_NONE, GetWnd() ) &&
		*szNewRom != '\0' )
	{
		GetWnd()->SetDlgItemText( m_pRomData[ rtType ].nCtrlID, szNewRom );
		KillfocusEditRom( rtType );
	}
} /* #OF# RomImagesBase::BrowseRomFile */

/*========================================================
Method   : RomImagesBase::SearchRomImages
=========================================================*/
/* #FN#
   Finds automatically known ROM images */
BOOL
/* #AS#
   TRUE if at least one ROM image has been found, otherwise FALSE */
CRomImagesBase::
SearchRomImages()
{
	CString	strPath = "";
	BOOL bResult = FALSE;

	CFileSmallDlg dlgFileSmall( TRUE, NULL, NULL, strPath, TRUE, GetWnd() );

	if( IDOK == dlgFileSmall.DoModal() )
	{
		char acRomNamesBackup[ ROM_TYPES_NO ][ MAX_PATH + 1 ];

		for( int i = 0; i < ROM_TYPES_NO; i++ )
			_strncpy( &acRomNamesBackup[ i ][ 0 ], m_pRomData[ i ].pszName, MAX_PATH );

		strPath = dlgFileSmall.GetPathName();	
		if( Misc_TestRomPaths( (LPSTR)(LPCSTR)strPath, GetWnd()->GetSafeHwnd() ) )
		{
			/* The Cancel button must work after using Search */
			for( int i = 0; i < ROM_TYPES_NO; i++ )
			{
				_strncpy( m_pRomData[ i ].szNewName, m_pRomData[ i ].pszName, MAX_PATH );
				strcpy( m_pRomData[ i ].pszName, &acRomNamesBackup[ i ][ 0 ] );
			}
			bResult = TRUE;
		}
		else
			DisplayMessage( GetWnd()->GetSafeHwnd(), IDS_ERROR_ROM_SEARCH, 0, MB_ICONINFORMATION | MB_OK );
	}
	return bResult;

} /* #OF# RomImagesBase::SearchRomImages */

/*========================================================
Method   : RomImagesBase::SearchRomImages
=========================================================*/
/* #FN#
   Displays information about selected ROM images */
void
/* #AS#
   Nothing */
CRomImagesBase::
DisplayRomInfo()
{
	LPCSTR pszLabel[ ROM_TYPES_NO ] = { "OS-A", "OS-B", "XL/XE", "5200", "BASIC" };
	char   szBuffer[ LOADSTRING_SIZE_S + 1 ];
	char   szOutput[ (LOADSTRING_SIZE_S + 1) * ROM_TYPES_NO ];
	LPCSTR pszInfo = NULL;
	LPSTR  pszPtr  = szOutput;

	for( int i = 0; i < ROM_TYPES_NO; i++ )
	{
		if( _IsPathAvailable( m_pRomData[ i ].szNewName ) )
		{
			pszInfo = GetRomInfo( m_pRomData[ i ].szNewName, (RomType)i );
			if( pszInfo )
				sprintf( pszPtr, "%s\t| %s\n", pszLabel[ i ], pszInfo );
			else
				sprintf( pszPtr, "%s\t| %s\n", pszLabel[ i ], _LoadStringSx( IDS_ROM_UNKNOWN_TYPE, szBuffer ) );
		}
		else
			sprintf( pszPtr, "%s\t| %s\n", pszLabel[ i ], _LoadStringSx( IDS_ROM_NOT_AVAILABLE, szBuffer ) );

		pszPtr = szOutput + strlen( szOutput );
	}
	GetWnd()->MessageBox( szOutput, _LoadStringSx( IDS_ROM_INFORMATION, szBuffer ), MB_ICONINFORMATION | MB_OK );

} /* #OF# RomImagesBase::DisplayRomInfo */

/*========================================================
Method   : CRomImagesBase::GetRomInfo
=========================================================*/
/* #FN#
   Returns a description of the pointed ROM image */
LPCSTR
/* #AS#
   Description of the ROM image if was found, otherwise NULL */
CRomImagesBase::
GetRomInfo(
	LPSTR pszFileName, /* #IN# Path to the ROM image */
	RomType rtType     /* #IN# Type of a ROM image */
)
{
	ULONG ulCRC;
	int   nResult;

	if( NULL != pszFileName )
	{
		ulCRC = CheckFile( pszFileName, &nResult );

		if( ulCRC != 0 )
		{
			int i;
			for( i = 0; i < g_nRomTypeInfoNo; i++ )
			{
				if( g_aRomTypeInfo[ i ].rtType == rtType &&
					g_aRomTypeInfo[ i ].ulCRC  == ulCRC )
				{
					return g_aRomTypeInfo[ i ].pszDesc;
				}
			}
		}
	}
	return NULL;

} /* #OF# CRomImagesBase::GetRomInfo */
