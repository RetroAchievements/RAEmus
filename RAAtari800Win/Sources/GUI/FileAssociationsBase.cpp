/****************************************************************************
File    : FileAssociationsBase.cpp
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# CFileAssociationsBase implementation file
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 30.09.2002
*/

#include "StdAfx.h"
#include "Atari800Win.h"
#include "FileAssociationsBase.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/* Definition of constants */

#define FILE_TYPES_NO	5

#define FILE_ATR		0x0001 /* File associations registry flags */
#define FILE_XFD		0x0002
#define FILE_DCM		0x0004
#define FILE_ATZ		0x0008
#define FILE_XFZ		0x0010
#define FILE_XEX		0x0020
#define FILE_CAS		0x0040
#define FILE_ROM		0x0080
#define FILE_BIN		0x0100
#define FILE_CAR		0x0200
#define FILE_A8S		0x0400

/* Private objects */

static char *s_apszRootTypes[ FILE_TYPES_NO ] =
{
	"Atari8.dsk", "Atari8.bin", "Atari8.tap", "Atari8.crt", "Atari8.a8s"
};

static CFileAssociationsBase::FileExtInfo_t s_aFileExtInfo[] =
{
	{ CFileAssociationsBase::FEI_DSK, FILE_ATR, ".atr", 0 },
	{ CFileAssociationsBase::FEI_DSK, FILE_XFD, ".xfd", 0 },
	{ CFileAssociationsBase::FEI_DSK, FILE_DCM, ".dcm", 0 },
	{ CFileAssociationsBase::FEI_DSK, FILE_ATZ, ".atz", 0 },
	{ CFileAssociationsBase::FEI_DSK, FILE_XFZ, ".xfz", 0 },
	{ CFileAssociationsBase::FEI_BIN, FILE_XEX, ".xex", 0 },
	{ CFileAssociationsBase::FEI_TAP, FILE_CAS, ".cas", 0 },
	{ CFileAssociationsBase::FEI_CRT, FILE_ROM, ".rom", 0 },
	{ CFileAssociationsBase::FEI_CRT, FILE_BIN, ".bin", 0 },
	{ CFileAssociationsBase::FEI_CRT, FILE_CAR, ".car", 0 },
	{ CFileAssociationsBase::FEI_A8S, FILE_A8S, ".a8s", 0 }
};

const int CFileAssociationsBase::m_nFileExtInfoNo = sizeof(s_aFileExtInfo)/sizeof(s_aFileExtInfo[0]);

CFileAssociationsBase::FileExtInfo_t *CFileAssociationsBase::m_pFileExtInfo = s_aFileExtInfo;


/////////////////////////////////////////////////////////////////////////////
// CFileAssociationsBase class

/*========================================================
Method   : CFileAssociationsBase::CFileAssociationsBase
=========================================================*/
/* #FN#
   Standard constructor */
CFileAssociationsBase::
CFileAssociationsBase()
{
} /* #OF# CFileAssociationsBase::CFileAssociationsBase */

/*========================================================
Method   : CFileAssociationsBase::~CFileAssociationsBase
=========================================================*/
/* #FN#
   Destructor */
CFileAssociationsBase::
~CFileAssociationsBase()
{
} /* #OF# CFileAssociationsBase::~CFileAssociationsBase */


/////////////////////////////////////////////////////////////////////////////
// CFileAssociationsBase implementation

/*========================================================
Method   : CFileAssociationsBase::InitDialog
=========================================================*/
/* #FN#
   Performs special processing when the dialog box is initialized */
BOOL
/* #AS#
   TRUE unless you set the focus to a control */
CFileAssociationsBase::
InitDialog() 
{
	/* Always modify file associations */
	if( !Misc_GetHomeDirectory( m_szHomeDir ) )
		GetCurrentDirectory( MAX_PATH, m_szHomeDir );

	m_ulFileAssociations = ReadRegFileExt( m_szHomeDir, g_szCmdLine );

	/* The global variable is only a not important shadow, update it now */
	if( m_ulFileAssociations != g_Misc.ulFileAssociations )
	{
		g_Misc.ulFileAssociations = m_ulFileAssociations;
		WriteRegDWORD( NULL, REG_FILE_ASSOCIATIONS, g_Misc.ulFileAssociations );
	}
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
} /* #OF# CFileAssociationsBase::InitDialog */

/*========================================================
Method   : CFileAssociationsBase::SelectAll
=========================================================*/
/* #FN#
   Sets all the check-boxes to 'checked' state */
void
/* #AS#
   Nothing */
CFileAssociationsBase::
SelectAll()
{
	CButton *pButton = NULL;
	for( int i = 0; i < m_nFileExtInfoNo; i++ )
	{
		pButton = (CButton *)GetWnd()->GetDlgItem( m_pFileExtInfo[ i ].nCtrlID );
		ASSERT(NULL != pButton);
		pButton->SetCheck( 1 );
	}
} /* #OF# CFileAssociationsBase::SelectAll */

/*========================================================
Method   : CFileAssociationsBase::RemoveAll
=========================================================*/
/* #FN#
   Sets all the check-boxes to 'unchecked' state */
void
/* #AS#
   Nothing */
CFileAssociationsBase::
RemoveAll()
{
	CButton *pButton = NULL;
	for( int i = 0; i < m_nFileExtInfoNo; i++ )
	{
		pButton = (CButton *)GetWnd()->GetDlgItem( m_pFileExtInfo[ i ].nCtrlID );
		ASSERT(NULL != pButton);
		pButton->SetCheck( 0 );
	}
} /* #OF# CFileAssociationsBase::RemoveAll */

/*========================================================
Method   : CFileAssociationsBase::WriteRegFileExt
=========================================================*/
/* #FN#
   Sets up the file associations for shell integration */
void
/* #AS#
   Nothing */
CFileAssociationsBase::
WriteRegFileExt(
	ULONG  ulFileAssociations,
	LPSTR  pszAppDir, /* #IN# Current application directory */
	LPCSTR pszAppName /* #IN# Name of application           */
)
{
	char  szKeyValue[ MAX_PATH + 1 ];
	char  szKeyEntry[ 32 ];
	char *apszOptns [ FILE_TYPES_NO ] = { "", "-run", "-boottape", "-cart", "-state" };
	char *apszDescs [ FILE_TYPES_NO ] = { "Disk Image", "Executable", "Tape Image", "Cartridge Image", "State File" };
	int   anIcons   [ FILE_TYPES_NO ] = { 3, 5, 8, 4, 6 };
	int   nAppDirLen = strlen( pszAppDir );
	HKEY  hkKey      = NULL;
	int   i;

	/* Ending backslash is forbidden */
	if( nAppDirLen > 0 && pszAppDir[ nAppDirLen - 1 ] == '\\' )
		pszAppDir[ nAppDirLen - 1 ] = '\0';

	for( i = 0; i < FILE_TYPES_NO; i++ )
	{
		/* Name the main file type entries */
		if( hkKey = GetRegKeyHandle( s_apszRootTypes[ i ], TRUE ) )
		{
			sprintf( szKeyValue, "Atari 8-bit %s", apszDescs[ i ] );
			WriteRegString( hkKey, "", szKeyValue );
			RegCloseKey( hkKey );
		}
		/* Write open command for this file type */
		sprintf( szKeyEntry, "%s\\shell\\open\\command", s_apszRootTypes[ i ] );
		if( hkKey = GetRegKeyHandle( szKeyEntry, TRUE ) )
		{
			sprintf( szKeyValue, "%s\\%s.exe %s \"%%1\"", pszAppDir, pszAppName, apszOptns[ i ] );
			WriteRegString( hkKey, "", szKeyValue );
			RegCloseKey( hkKey );
		}
		/* There is the icon number 2 for shell integration */
		sprintf( szKeyEntry, "%s\\DefaultIcon", s_apszRootTypes[ i ] );
		if( hkKey = GetRegKeyHandle( szKeyEntry, TRUE ) )
		{
			sprintf( szKeyValue, "%s\\%s.exe,%d", pszAppDir, pszAppName, anIcons[ i ] );
			WriteRegString( hkKey, "", szKeyValue );
			RegCloseKey( hkKey );
		}
	}

	for( i = 0; i < m_nFileExtInfoNo; i++ )
	{
		if( hkKey = GetRegKeyHandle( m_pFileExtInfo[ i ].pszFileExt, TRUE ) )
		{
			if( _IsFlagSet( ulFileAssociations, m_pFileExtInfo[ i ].dwRegFlag ) )
			{
				/* See enum FileType values defined in globals.h file */
				WriteRegString( hkKey, "", s_apszRootTypes[ m_pFileExtInfo[ i ].ftType ] );
			}
			else /* delete this association */
				RegDeleteKey( hkKey, "" );/*pFileExtInfo[ i ].pszFileExt );*/
			RegCloseKey( hkKey );
		}
	}
} /* #OF# CFileAssociationsBase::WriteRegFileExt */

/*========================================================
Method   : CFileAssociationsBase::ReadRegFileExt
=========================================================*/
/* #FN#
   Reads the current file associations state */
ULONG
/* #AS#
   Current file associations state */
CFileAssociationsBase::
ReadRegFileExt(
	LPSTR  pszAppDir, /* #IN# Current application directory */
	LPCSTR pszAppName /* #IN# Name of application           */
)
{
	char  szKeyValue   [ MAX_PATH + 1 ];
	char  szKeyBuffer  [ MAX_PATH + 1 ];
	char  szKeyEntry   [ 32 ];
	BOOL  abEntryExists[ FILE_TYPES_NO ];
	ULONG ulFileAssociations = 0L;
	int   nAppDirLen         = strlen( pszAppDir );
	HKEY  hkKey              = NULL;
	int   i;

	/* Ending backslash is forbidden */
	if( nAppDirLen > 0 && pszAppDir[ nAppDirLen - 1 ] == '\\' )
		pszAppDir[ nAppDirLen - 1 ] = '\0';

	for( i = 0; i < FILE_TYPES_NO; i++ )
	{
		abEntryExists[ i ] = FALSE;

		if( hkKey = GetRegKeyHandle( s_apszRootTypes[ i ], FALSE ) )
		{
			RegCloseKey( hkKey );
			/* Read open command for this file type */
			sprintf( szKeyEntry, "%s\\shell\\open\\command", s_apszRootTypes[ i ] );
			if( hkKey = GetRegKeyHandle( szKeyEntry, FALSE ) )
			{
				DWORD dwType;
				ULONG ulMaxSize = MAX_PATH;

				if( RegQueryValueEx(
					hkKey,					// handle of key to query 
					NULL,					// address of name of value to query 
					0,						// reserved 
					&dwType,				// address of buffer for value type 
					(UCHAR *)szKeyBuffer,	// address of data buffer 
					&ulMaxSize			 	// address of data buffer size 
					) == ERROR_SUCCESS )
				{
					sprintf( szKeyValue, "%s\\%s.exe", pszAppDir, pszAppName );
					_strupr( szKeyValue ); _strupr( szKeyBuffer );

					if( strstr( szKeyBuffer, szKeyValue ) )
						abEntryExists[ i ] = TRUE;
				}
				RegCloseKey( hkKey );
			}
		}
	}
	for( i = 0; i < m_nFileExtInfoNo; i++ )
	{
		if( hkKey = GetRegKeyHandle( m_pFileExtInfo[ i ].pszFileExt, FALSE ) )
		{
			if( abEntryExists[ m_pFileExtInfo[ i ].ftType ] )
			{
				/* Read this extension flag into variable */
				DWORD dwType;
				ULONG ulMaxSize = MAX_PATH;

				if( RegQueryValueEx(
					hkKey,					// handle of key to query 
					NULL,					// address of name of value to query 
					0,						// reserved 
					&dwType,				// address of buffer for value type 
					(UCHAR *)szKeyBuffer,	// address of data buffer 
					&ulMaxSize			 	// address of data buffer size 
					) == ERROR_SUCCESS )
				{
					if( !strcmp( szKeyBuffer, s_apszRootTypes[ m_pFileExtInfo[ i ].ftType ] ) )
						ulFileAssociations |= m_pFileExtInfo[ i ].dwRegFlag;
				}
				RegCloseKey( hkKey );
			}
		}
	}
	return ulFileAssociations;

} /* #OF# CFileAssociationsBase::ReadRegFileExt */
