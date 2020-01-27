/****************************************************************************
File    : FileService.cpp
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# File management methods and objects
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 24.12.2003
*/

#include "StdAfx.h"
#include "Atari800Win.h"
#include "MainFrame.h"
#include "FileSmallDlg.h"
#include "CartridgeTypeDlg.h"
#include "WarningDlg.h"
#include "Helpers.h"
#include "FileService.h"
#include "cassette.h"

/////////////////////////////////////////////////////////////////////////////
// Public objects

extern "C" {

LPCSTR DEFAULT_VID = "atarivid.avi";
LPCSTR DEFAULT_SND = "atarisnd.wav";
LPCSTR DEFAULT_PIC = "atari000.pcx";
LPCSTR DEFAULT_A8S = "atarisav.a8s";
LPCSTR DEFAULT_A8T = "ataritrn.a8t";
LPCSTR DEFAULT_A8K = FILE_NONE;
LPCSTR DEFAULT_BIN = FILE_NONE;
LPCSTR DEFAULT_ACT = FILE_NONE;
LPCSTR DEFAULT_TAP = FILE_NONE;
LPCSTR DEFAULT_OSA = FILE_NONE;
LPCSTR DEFAULT_OSB = FILE_NONE;
LPCSTR DEFAULT_OXL = FILE_NONE;
LPCSTR DEFAULT_O52 = FILE_NONE;
LPCSTR DEFAULT_BAS = FILE_NONE;
LPCSTR DEFAULT_HDD = PATH_NONE;

char   g_szCurrentRom  [ MAX_PATH + 1 ];
char   g_szOtherRom    [ MAX_PATH + 1 ];
char   g_szTemplateFile[ MAX_PATH + 1 ];
char   g_szPaletteFile [ MAX_PATH + 1 ];
char   g_szTemplateDesc[ TEMPLATE_DESC_LENGTH + 1 ];
char   g_szAutobootFile[ MAX_PATH + 1 ];
char   g_szBinaryFile  [ MAX_PATH + 1 ];
char   g_szSnapshotFile[ MAX_PATH + 1 ];
char   g_szHardExePath [ FILENAME_MAX ];

}

/////////////////////////////////////////////////////////////////////////////
//
//   C++ stuff implementation
//

/////////////////////////////////////////////////////////////////////////////
// File requesters

/*========================================================
Function : PickFileName
=========================================================*/
/* #FN#
   Displays file dialog and ask user for a file name */
BOOL
/* #AS#
   TRUE if the file was picked, otherwise FALSE */
PickFileName(
	BOOL    bOpenFileDialog,		/* #IN# */
	LPSTR   pszFileName,			/* #IN/OUT# */
	LPCSTR  pszPrompt,				/* #IN# */
	LPCSTR  pszFilter,				/* #IN# */
	LPCSTR  pszDefExt,				/* #IN# */
	DWORD   dwFlags,				/* #IN# */
	BOOL    bCheckPath /*=TRUE*/,   /* #IN# */
	LPCSTR  pszDefFile /*="None"*/, /* #IN# */
	CWnd   *pParentWnd /*=NULL*/    /* #IN# */
)
{
	char szFilePath[ MAX_PATH + 1 ];
	int  nResult = IDOK;

	_strncpy( szFilePath, pszFileName, MAX_PATH );

	if( bCheckPath )
	{
		CFileStatus fsStatus;
		int i;

		/* Standard FileDialog wants that */
		do
		{
			/* Remove the ending backslash */
			for( i = strlen( szFilePath ) - 1; i >= 0 && szFilePath[ i ] == '\\'; i-- )
				szFilePath[ i ] = '\0';
			/* We have got a drive here, don't process */
			if( strlen( szFilePath ) == 2 && szFilePath[ 1 ] == ':' )
			{
				strcat( szFilePath, "\\" );
				strcat( szFilePath, pszDefFile );
				break;
			}
			/* That's the static method, don't panic */
			if( CFile::GetStatus( szFilePath, fsStatus ) )
			{
				if( fsStatus.m_attribute & CFile::directory )
				{
					strcat( szFilePath, "\\" );
					strcat( szFilePath, pszDefFile );
				}
				break;
			}
			else
			{
				for( i = strlen( szFilePath );
					 i >= 0 && szFilePath[ i ] != '\\'; i-- )
					szFilePath[ i ] = '\0';
			}
		}
		while( i > 0 );
	}
	if( *szFilePath == '\0' )
		strcpy( szFilePath, pszDefFile );

	/* Open the file dialog */
	_CursorBusy();
	if( g_Screen.ulState & SM_ATTR_SMALL_DLG )
	{
		CFileSmallDlg dlgFileSmall( bOpenFileDialog,
									pszDefExt,
									pszPrompt,
									szFilePath,
									FALSE,
									(pParentWnd ? pParentWnd : AfxGetMainWnd()) );

		if( IDOK == (nResult = dlgFileSmall.DoModal()) )
			_strncpy( pszFileName, dlgFileSmall.GetPathName(), MAX_PATH );
	}
	else
	{
		if( g_Screen.ulState & SM_MODE_FULL )
			dwFlags |= OFN_ENABLEHOOK;

		CFileDialog	fdlgFile( bOpenFileDialog,
							  pszDefExt,
							  szFilePath,
							  dwFlags | OFN_ENABLESIZING,
							  pszFilter,
							  (pParentWnd ? pParentWnd : AfxGetMainWnd())
//							  ,OPENFILENAME_SIZE_VERSION_400
							  );

		fdlgFile.m_ofn.lpstrTitle = pszPrompt;
		if( g_Screen.ulState & SM_MODE_FULL )
			fdlgFile.m_ofn.lpfnHook = FileDialogHookProc;

		if( IDOK == (nResult = fdlgFile.DoModal()) )
			_strncpy( pszFileName, fdlgFile.GetPathName(), MAX_PATH );
	}
	_CursorFree();

	return (nResult == IDOK ? TRUE : FALSE);

} /* #OF# PickFileName */

/*========================================================
Function : PickFileName
=========================================================*/
/* #FN#
   Displays file dialog and ask user for a file name */
BOOL
/* #AS#
   TRUE if the file was picked, otherwise FALSE */
PickFileName(
	BOOL     bOpenFileDialog,		 /* #IN# */
	LPSTR    pszFileName,			 /* #IN/OUT# */
	LPCSTR   pszPrompt,				 /* #IN# */
	UINT     uFilterID,				 /* #IN# */
	LPCSTR   pszDefExt,				 /* #IN# */
	DWORD    dwFlags,				 /* #IN# */
	BOOL     bCheckPath /*=TRUE*/,   /* #IN# */
	LPCSTR   pszDefFile /*="None"*/, /* #IN# */
	CWnd    *pParentWnd /*=NULL*/    /* #IN# */
)
{
	char szFilter[ LOADSTRING_SIZE_M + 1 ];

	BOOL bResult =
		PickFileName( bOpenFileDialog,
					  pszFileName,
					  pszPrompt,
					  _LoadStringMx( uFilterID, szFilter ),
					  pszDefExt,
					  dwFlags,
					  bCheckPath,
					  pszDefFile,
					  pParentWnd );

	return bResult;

} /* #OF# PickFileName */

/*========================================================
Function : PickFileName
=========================================================*/
/* #FN#
   Displays file dialog and ask user for a file name */
BOOL
/* #AS#
   TRUE if the file was picked, otherwise FALSE */
PickFileName(
	BOOL     bOpenFileDialog,		 /* #IN# */
	LPSTR    pszFileName,			 /* #IN/OUT# */
	UINT     uPromptID,				 /* #IN# */
	LPCSTR   pszFilter,				 /* #IN# */
	LPCSTR   pszDefExt,				 /* #IN# */
	DWORD    dwFlags,				 /* #IN# */
	BOOL     bCheckPath /*=TRUE*/,   /* #IN# */
	LPCSTR   pszDefFile /*="None"*/, /* #IN# */
	CWnd    *pParentWnd /*=NULL*/    /* #IN# */
)
{
	char szPrompt[ LOADSTRING_SIZE_S + 1 ];

	BOOL bResult =
		PickFileName( bOpenFileDialog,
					  pszFileName,
					  _LoadStringSx( uPromptID, szPrompt ),
					  pszFilter,
					  pszDefExt,
					  dwFlags,
					  bCheckPath,
					  pszDefFile,
					  pParentWnd );

	return bResult;

} /* #OF# PickFileName */

/*========================================================
Function : PickFileName
=========================================================*/
/* #FN#
   Displays file dialog and ask user for a file name */
BOOL
/* #AS#
   TRUE if the file was picked, otherwise FALSE */
PickFileName(
	BOOL     bOpenFileDialog,		 /* #IN# */
	LPSTR    pszFileName,			 /* #IN/OUT# */
	UINT     uPromptID,				 /* #IN# */
	UINT     uFilterID,				 /* #IN# */
	LPCSTR   pszDefExt,				 /* #IN# */
	DWORD    dwFlags,				 /* #IN# */
	BOOL     bCheckPath /*=TRUE*/,   /* #IN# */
	LPCSTR   pszDefFile /*="None"*/, /* #IN# */
	CWnd    *pParentWnd /*=NULL*/    /* #IN# */
)
{
	char szPrompt[ LOADSTRING_SIZE_S + 1 ];
	char szFilter[ LOADSTRING_SIZE_M + 1 ];

	BOOL bResult =
		PickFileName( bOpenFileDialog,
					  pszFileName,
					  _LoadStringSx( uPromptID, szPrompt ),
					  _LoadStringMx( uFilterID, szFilter ),
					  pszDefExt,
					  dwFlags,
					  bCheckPath,
					  pszDefFile,
					  pParentWnd );

	return bResult;

} /* #OF# PickFileName */

/*========================================================
Function : PickFileName
=========================================================*/
/* #FN#
   Displays file dialog and ask user for a file name */
BOOL
/* #AS#
   TRUE if the file was picked, otherwise FALSE */
PickFileName(
	BOOL     bOpenFileDialog,		 /* #IN# */
	CString &strFileName,			 /* #IN/OUT# */
	LPCSTR   pszPrompt,				 /* #IN# */
	LPCSTR   pszFilter,				 /* #IN# */
	LPCSTR   pszDefExt,				 /* #IN# */
	DWORD    dwFlags,				 /* #IN# */
	BOOL     bCheckPath /*=TRUE*/,   /* #IN# */
	LPCSTR   pszDefFile /*="None"*/, /* #IN# */
	CWnd    *pParentWnd /*=NULL*/    /* #IN# */
)
{
	BOOL bResult =
		PickFileName( bOpenFileDialog,
					  strFileName.GetBuffer( MAX_PATH ),
					  pszPrompt,
					  pszFilter,
					  pszDefExt,
					  dwFlags,
					  bCheckPath,
					  pszDefFile,
					  pParentWnd );

	strFileName.ReleaseBuffer();

	return bResult;

} /* #OF# PickFileName */

/*========================================================
Function : PickFileName
=========================================================*/
/* #FN#
   Displays file dialog and ask user for a file name */
BOOL
/* #AS#
   TRUE if the file was picked, otherwise FALSE */
PickFileName(
	BOOL     bOpenFileDialog,		 /* #IN# */
	CString &strFileName,			 /* #IN/OUT# */
	LPCSTR   pszPrompt,				 /* #IN# */
	UINT     uFilterID,				 /* #IN# */
	LPCSTR   pszDefExt,				 /* #IN# */
	DWORD    dwFlags,				 /* #IN# */
	BOOL     bCheckPath /*=TRUE*/,   /* #IN# */
	LPCSTR   pszDefFile /*="None"*/, /* #IN# */
	CWnd    *pParentWnd /*=NULL*/    /* #IN# */
)
{
	char szFilter[ LOADSTRING_SIZE_M + 1 ];

	BOOL bResult =
		PickFileName( bOpenFileDialog,
					  strFileName.GetBuffer( MAX_PATH ),
					  pszPrompt,
					  _LoadStringMx( uFilterID, szFilter ),
					  pszDefExt,
					  dwFlags,
					  bCheckPath,
					  pszDefFile,
					  pParentWnd );

	strFileName.ReleaseBuffer();

	return bResult;

} /* #OF# PickFileName */

/*========================================================
Function : PickFileName
=========================================================*/
/* #FN#
   Displays file dialog and ask user for a file name */
BOOL
/* #AS#
   TRUE if the file was picked, otherwise FALSE */
PickFileName(
	BOOL     bOpenFileDialog,		 /* #IN# */
	CString &strFileName,			 /* #IN/OUT# */
	UINT     uPromptID,				 /* #IN# */
	LPCSTR   pszFilter,				 /* #IN# */
	LPCSTR   pszDefExt,				 /* #IN# */
	DWORD    dwFlags,				 /* #IN# */
	BOOL     bCheckPath /*=TRUE*/,   /* #IN# */
	LPCSTR   pszDefFile /*="None"*/, /* #IN# */
	CWnd    *pParentWnd /*=NULL*/    /* #IN# */
)
{
	char szPrompt[ LOADSTRING_SIZE_S + 1 ];

	BOOL bResult =
		PickFileName( bOpenFileDialog,
					  strFileName.GetBuffer( MAX_PATH ),
					  _LoadStringSx( uPromptID, szPrompt ),
					  pszFilter,
					  pszDefExt,
					  dwFlags,
					  bCheckPath,
					  pszDefFile,
					  pParentWnd );

	strFileName.ReleaseBuffer();

	return bResult;

} /* #OF# PickFileName */

/*========================================================
Function : PickFileName
=========================================================*/
/* #FN#
   Displays file dialog and ask user for a file name */
BOOL
/* #AS#
   TRUE if the file was picked, otherwise FALSE */
PickFileName(
	BOOL     bOpenFileDialog,		 /* #IN# */
	CString &strFileName,			 /* #IN/OUT# */
	UINT     uPromptID,				 /* #IN# */
	UINT     uFilterID,				 /* #IN# */
	LPCSTR   pszDefExt,				 /* #IN# */
	DWORD    dwFlags,				 /* #IN# */
	BOOL     bCheckPath /*=TRUE*/,   /* #IN# */
	LPCSTR   pszDefFile /*="None"*/, /* #IN# */
	CWnd    *pParentWnd /*=NULL*/    /* #IN# */
)
{
	char szPrompt[ LOADSTRING_SIZE_S + 1 ];
	char szFilter[ LOADSTRING_SIZE_M + 1 ];

	BOOL bResult =
		PickFileName( bOpenFileDialog,
					  strFileName.GetBuffer( MAX_PATH ),
					  _LoadStringSx( uPromptID, szPrompt ),
					  _LoadStringMx( uFilterID, szFilter ),
					  pszDefExt,
					  dwFlags,
					  bCheckPath,
					  pszDefFile,
					  pParentWnd );

	strFileName.ReleaseBuffer();

	return bResult;

} /* #OF# PickFileName */

/*========================================================
Function : PickCartridge
=========================================================*/
/* #FN#
   Displays file dialog and allows selecting a cartridge image */
BOOL
/* #AS#
   TRUE if the cartridge was picked, otherwise FALSE */
PickCartridge(
	LPSTR pszCurrentCart,
	CWnd *pParentWnd /*=NULL*/
)
{
	BOOL bResult = FALSE;

	if( PickFileName( TRUE, pszCurrentCart, IDS_SELECT_ROM_LOAD, IDS_FILTER_CRT,
					  "rom", PF_LOAD_FLAGS, TRUE, FILE_NONE, pParentWnd ) &&
		*pszCurrentCart != '\0' )
	{
		bResult = TRUE;
	}
	return bResult;

} /* #OF# PickCartridge */

/*========================================================
Function : FileDialogHookProc
=========================================================*/
/* #FN#
   Callback function used with the Explorer-style Open and Save As dialog boxes */
UINT CALLBACK
/* #AS#
   If zero, the default dialog box procedure processes the message */
FileDialogHookProc(
	HWND   hDlg,   /* #IN# Handle to child dialog window    */
	UINT   uMsg,   /* #IN# WM_NOTIFY                        */
	WPARAM wParam, /* #IN# Not used                         */
	LPARAM lParam  /* #IN# Message information (LPOFNOTIFY) */
)
{
	LPNMHDR lpNMHDR = (LPNMHDR)lParam;

	if( WM_NOTIFY == uMsg && lpNMHDR && CDN_INITDONE == lpNMHDR->code )
	{
		CWnd *pWnd = pWnd->FromHandle( lpNMHDR->hwndFrom );
		ASSERT(NULL != pWnd);

		SetWindowLong( lpNMHDR->hwndFrom, GWL_EXSTYLE, ~WS_EX_CONTEXTHELP & GetWindowLong( lpNMHDR->hwndFrom, GWL_EXSTYLE ) );
		pWnd->CenterWindow();

		return 1;
	}
	return 0;

} /* #OF# FileDialogHookProc */


/////////////////////////////////////////////////////////////////////////////
//
// Common stuff implementation
//

extern "C" { /* Do not decorate the function names, please */

/*========================================================
Function : IsAtariFile
=========================================================*/
/* #FN#
   Checks if the pointed image is known Atari file */
BOOL
/* #AS#
   TRUE if the image might be an Atari file, otherwise FALSE */
IsAtariFile(
	LPCSTR pszFileName, /* #IN# Name of the file to test */
	PUINT  pFileType    /* #IN/OUT# The file type */
)
{
	char szTempFile[ MAX_PATH + 1 ];
	int  fd;

	strcpy( szTempFile, pszFileName );
	_strupr(	szTempFile );

	if( IAF_CAS_IMAGE & *pFileType )
	{
		char szTapeDesc[ CASSETTE_DESCRIPTION_MAX ];
		int  nLastBlock = 0;
		int  nIsCAS = FALSE;

		CASSETTE_CheckFile( szTempFile, NULL, szTapeDesc, &nLastBlock, &nIsCAS );
		if( nIsCAS )
		{
			*pFileType = IAF_CAS_IMAGE;
			return TRUE;
		}
	}
	if( IAF_DSK_IMAGE & *pFileType )
	{
		if( strstr( szTempFile, ".GZ"  ) ||
			strstr( szTempFile, ".ATZ" ) ||
			strstr( szTempFile, ".XFZ" ) ||
			strstr( szTempFile, ".DCM" ) ||
			strstr( szTempFile, ".ATR" ) ||
			strstr( szTempFile, ".XFD" ) )
		{
			*pFileType = IAF_DSK_IMAGE;
			return TRUE;
		}
	}
	if( IAF_CAR_IMAGE & *pFileType &&
		(fd = _open( pszFileName, _O_RDONLY | _O_BINARY, 0 )) != -1 )
	{
		char cBuffer[ 4 ];
		int  nBytesRead = 0;

		_lseek( fd, 0L, SEEK_SET );
		nBytesRead = _read( fd, cBuffer, 4 );
		_close( fd );

		if( 4 == nBytesRead &&
			'C' == cBuffer[ 0 ] &&
			'A' == cBuffer[ 1 ] &&
			'R' == cBuffer[ 2 ] &&
			'T' == cBuffer[ 3 ] )
		{
			*pFileType = IAF_CAR_IMAGE;
			return TRUE;
		}
	}
	if( IAF_ROM_IMAGE & *pFileType )
	{
		if( strstr( szTempFile, ".ROM" ) ||
			strstr( szTempFile, ".BIN" ) )
		{
			*pFileType = IAF_ROM_IMAGE;
			return TRUE;
		}
	}
	if( IAF_A8S_IMAGE & *pFileType )
	{
		if( strstr( szTempFile, ".A8S" ) )
		{
			*pFileType = IAF_A8S_IMAGE;
			return TRUE;
		}
	}
	/* That's the last check because some carts have $FFFF header */
	if( IAF_BIN_IMAGE & *pFileType &&
		(fd = _open( pszFileName, _O_RDONLY | _O_BINARY, 0 )) != -1 )
	{
		char cBuffer[ 2 ];
		int  nBytesRead = 0;

		_lseek( fd, 0L, SEEK_SET );
		nBytesRead = _read( fd, cBuffer, 2 );
		_close( fd );

		if( 2 == nBytesRead &&
			0xff == (BYTE)cBuffer[ 0 ] &&
			0xff == (BYTE)cBuffer[ 1 ] )
		{
			*pFileType = IAF_BIN_IMAGE;
			return TRUE;
		}
	}
	*pFileType = 0;

	return FALSE;

} /* #OF# IsAtariFile */

/*========================================================
Function : IsCompressedFile
=========================================================*/
/* #FN#
   Simple routine to check if the given filename is any one of the various
   compressed types. Currently the only test for this is by extension, also
   the only way you can actually read a compressed file in, so that seems
   to be valid enough. */
BOOL
/* #AS#
   TRUE if the file is compressed, otherwise FALSE */
IsCompressedFile(
	LPCSTR pszFileName /* #IN# Name of the file to test */
)
{
	char szTempFile[ MAX_PATH + 1 ];

	strcpy( szTempFile, pszFileName );
	_strupr(	szTempFile );

	if( strstr( szTempFile, ".GZ"  ) ||
		strstr( szTempFile, ".ATZ" ) ||
		strstr( szTempFile, ".XFZ" ) ||
		strstr( szTempFile, ".DCM" ) )
		return TRUE;

	return FALSE;

} /* #OF# IsCompressedFile */

/*========================================================
Function : GetFolderPath
=========================================================*/
/* #FN#
   Extracts the folder from full file path */
void
/* #AS#
   Nothing */
GetFolderPath(
	LPSTR pszPathName, /* #IN/OUT# Full file path/file folder name */
	LPSTR pszFileName  /* #OUT#    File name */
)
{
	int nPathLen = strlen( pszPathName ) - 1;
	int i;

	for( i = nPathLen; i > 0 && pszPathName[ i ] != '\\'; i-- );
	if( i > 0 || pszPathName[ i ] == '\\' )
	{
		pszPathName[ i++ ] = '\0';
	}
	if( pszFileName != NULL )
		_strncpy( pszFileName, &pszPathName[ i ], MAX_PATH );

	/* Add ending backslash to drive name */
	if( strlen( pszPathName ) == 2 && pszPathName[ 1 ] == ':' )
		strcat( pszPathName, "\\" );

} /* #OF# GetFolderPath */

/*========================================================
Function : RunExecutable
=========================================================*/
/* #FN#
   Loads and runs an Atari executable file */
BOOL
/* #AS#
   TRUE if succeeded, otherwise FALSE */
RunExecutable(
	LPSTR pszFileName /* #IN# Name of the executable file to run */
)
{
	char szNewDir[ MAX_PATH + 1 ];
	BOOL bResult = FALSE;

	_strncpy( szNewDir, pszFileName, MAX_PATH );
	GetFolderPath( szNewDir, NULL );
	if( _stricmp( szNewDir, atari_exe_dir ) != 0 )
	{
		strcpy( atari_exe_dir, szNewDir );
		WriteRegString( NULL, REG_EXE_PATH, atari_exe_dir );
	}
	if( BINLOAD_Loader( pszFileName ) )
	{
		_strncpy( g_szBinaryFile, pszFileName, MAX_PATH );
		bResult = TRUE;
	}
	else
		strcpy( g_szBinaryFile, DEFAULT_BIN );

	return bResult;

} /* #OF# RunExecutable */

/*========================================================
Function : RunSnapshot
=========================================================*/
/* #FN#
   Loads and runs an Atari saved-state file */
BOOL
/* #AS#
   TRUE if succeeded, otherwise FALSE */
RunSnapshot(
	LPSTR pszFileName /* #IN# Name of the executable file to run */
)
{
	BOOL bResult = FALSE;

	/* The atari_state_dir array used by Atari800 kernel
	   contents a complete path to the file */
	if( _stricmp( pszFileName, atari_state_dir ) != 0 )
	{
		_strncpy( atari_state_dir, pszFileName, MAX_PATH );
		WriteRegString( NULL, REG_FILE_STATE, atari_state_dir );
	}
	if( StateSav_ReadAtariState( pszFileName, "rb" ) )
	{
		_strncpy( g_szSnapshotFile, pszFileName, MAX_PATH );
		bResult = TRUE;
	}
	else
		strcpy( g_szSnapshotFile, DEFAULT_A8S );

	return bResult;

} /* #OF# RunSnapshot */

/*========================================================
Function : AttachCartridge
=========================================================*/
/* #FN#
   Attaches a cartridge image */
int
/* #AS#
   Type of an attached cartridge, otherwise CARTRIDGE_NONE */
AttachCartridge(
	LPSTR pszFileName,
	int   nType
)
{
	int nCartType = CARTRIDGE_NONE;

	int nCartSize = CARTRIDGE_Insert( pszFileName );
	if( nCartSize < 0 )
	{
		char szError[ LOADSTRING_SIZE_S + 1 ];
		/* If there was an error... */
		DisplayMessage( NULL, IDS_ERROR_ROM_LOAD, 0, MB_ICONEXCLAMATION | MB_OK,
						pszFileName,
						nCartSize == CARTRIDGE_CANT_OPEN    ? _LoadStringSx( IDS_ERROR_CANT_OPEN, szError ) :
						nCartSize == CARTRIDGE_BAD_FORMAT   ? _LoadStringSx( IDS_ERROR_BAD_FORMAT, szError ) :
						nCartSize == CARTRIDGE_BAD_CHECKSUM ? _LoadStringSx( IDS_ERROR_BAD_CHECKSUM, szError ) :
						_LoadStringSx( IDS_ERROR_UNKNOWN, szError ) );
	}
	else if( nCartSize > 0 )
	{
		CARTRIDGE_type = CARTRIDGE_NONE == nType ? SelectCartType( nCartSize ) : nType;
		if( CARTRIDGE_NONE != CARTRIDGE_type )
		{
			CARTRIDGE_Start();
			strcpy( g_szCurrentRom, pszFileName );

			nCartType = CARTRIDGE_type;
		}
	}
	else if( nCartSize == 0 )
	{
		strcpy( g_szCurrentRom, pszFileName );
		nCartType = CARTRIDGE_type;
	}
	if( CARTRIDGE_NONE == nCartType )
	{
		CARTRIDGE_Remove();
		strcpy( g_szCurrentRom, FILE_NONE );
	}
	return nCartType;

} /* #OF# AttachCartridge */

/*========================================================
Function : SelectCartType
=========================================================*/
/* #FN#
   Recognizes a cartridge type */
int
/* #AS#
   Cartridge type */
SelectCartType(
	int nCartSize
)
{
	int nResult = CARTRIDGE_NONE;
	int nTypes  = 0;

	CCartridgeTypeDlg dlgCartridgeType( nCartSize );

	nTypes = dlgCartridgeType.CountTypes( nCartSize );
	if( nTypes > 0 )
		if( 1 == nTypes || IDOK == dlgCartridgeType.DoModal() )
		{
			nResult = dlgCartridgeType.GetCartridgeType();
		}
	return nResult;

} /* #OF# SelectCartType */

/*========================================================
Function : GetBootFileInfo
=========================================================*/
/* #FN#
   Returns the name of a file the emulated machine is assumed to boot from */
BOOL
/* #AS#
   TRUE if a file is found, otherwise FALSE */
GetBootFileInfo(
	LPSTR pszFileName,
	int   nBufferLen,
	PUINT pFileType
)
{
	UINT unFileType = *pFileType;
	*pFileType = 0;

	/* Is there any snapshot (saved-state file) loaded? */
	if( 0 != (unFileType & IAF_A8S_IMAGE) )
	{
		if( _IsPathAvailable( g_szSnapshotFile ) )
		{
			*pFileType |= IAF_A8S_IMAGE;
			if( NULL != pszFileName )
			{
				_strncpy( pszFileName, g_szSnapshotFile, nBufferLen );
				return TRUE;
			}
		}
	}
	/* Is there any cartridge inserted into the slot? */
	if( 0 != (unFileType & IAF_ROM_IMAGE) )
	{
		if( _IsPathAvailable( g_szCurrentRom ) &&
			CARTRIDGE_NONE != CARTRIDGE_type )
		{
			*pFileType |= IAF_ROM_IMAGE;
			if( NULL != pszFileName )
			{
				_strncpy( pszFileName, g_szCurrentRom, nBufferLen );
				return TRUE;
			}
		}
	}
	/* Executable, disk and tape images are not used by 5200 */
	if( Atari800_MACHINE_5200 != Atari800_machine_type )
	{
		UINT unType;
		/* Is there any executable about to load/loaded? */
		if( 0 != (unFileType & IAF_BIN_IMAGE) )
		{
			unType = IAF_BIN_IMAGE;
			if( _IsPathAvailable( g_szBinaryFile ) &&
				IsAtariFile( g_szBinaryFile, &unType ) )
			{
				*pFileType |= IAF_BIN_IMAGE;
				if( NULL != pszFileName )
				{
					_strncpy( pszFileName, g_szBinaryFile, nBufferLen );
					return TRUE;
				}
			}
		}
		/* Is there any disk inserted into the drive 1? */
		if( 0 != (unFileType & IAF_DSK_IMAGE) )
		{
			if( _IsPathAvailable( SIO_filename[ 0 ] ) &&
				_stricmp( SIO_filename[ 0 ], "Empty" ) != 0 &&
				_stricmp( SIO_filename[ 0 ], "Off" ) != 0 )
			{
				*pFileType |= IAF_DSK_IMAGE;
				if( NULL != pszFileName )
				{
					_strncpy( pszFileName, SIO_filename[ 0 ], nBufferLen );
					return TRUE;
				}
			}
		}
		/* Is there any cassette inserted into the tape player? */
		if( 0 != (unFileType & IAF_CAS_IMAGE) )
		{
			unType = IAF_CAS_IMAGE;
			if( _IsPathAvailable( cassette_filename ) &&
				IsAtariFile( cassette_filename, &unType ) )
			{
				*pFileType |= IAF_CAS_IMAGE;
				if( NULL != pszFileName )
				{
					_strncpy( pszFileName, cassette_filename, nBufferLen );
					return TRUE;
				}
			}
		}
	}
	return (0 != *pFileType);

} /* #OF# GetBootFileInfo */

} //extern "C"
