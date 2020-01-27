/****************************************************************************
File    : FileService.h
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Declaration of file management methods and objects
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 14.07.2003
*/

#ifndef __FILESERVICE_H__
#define __FILESERVICE_H__

#define IAF_A8S_IMAGE			0x01
#define IAF_ROM_IMAGE			0x02
#define IAF_BIN_IMAGE			0x04
#define IAF_DSK_IMAGE			0x08
#define IAF_CAS_IMAGE			0x10
#define IAF_CAR_IMAGE			0x20
#define IAF_ATARI_FILE			0xff

#define TEMPLATE_DESC_LENGTH	255

#define FILE_NONE				"None"
#define PATH_NONE				"."


#ifdef __cplusplus

/*
   C++ specific declarations
*/

/* File requesters */

#define PF_LOAD_FLAGS	(OFN_EXPLORER | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST)
#define PF_SAVE_FLAGS	(OFN_EXPLORER | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT)

BOOL PickFileName ( BOOL bOpenFileDialog, LPSTR pszFileName, LPCSTR pszPrompt, LPCSTR pszFilter, LPCSTR pszDefExt, DWORD dwFlags, BOOL bCheckPath = TRUE, LPCSTR pszDefFile = FILE_NONE, CWnd *pParentWnd = NULL );
BOOL PickFileName ( BOOL bOpenFileDialog, LPSTR pszFileName, LPCSTR pszPrompt, UINT uFilterID, LPCSTR pszDefExt, DWORD dwFlags, BOOL bCheckPath = TRUE, LPCSTR pszDefFile = FILE_NONE, CWnd *pParentWnd = NULL );
BOOL PickFileName ( BOOL bOpenFileDialog, LPSTR pszFileName, UINT uPromptID, LPCSTR pszFilter, LPCSTR pszDefExt, DWORD dwFlags, BOOL bCheckPath = TRUE, LPCSTR pszDefFile = FILE_NONE, CWnd *pParentWnd = NULL );
BOOL PickFileName ( BOOL bOpenFileDialog, LPSTR pszFileName, UINT uPromptID, UINT uFilterID, LPCSTR pszDefExt, DWORD dwFlags, BOOL bCheckPath = TRUE, LPCSTR pszDefFile = FILE_NONE, CWnd *pParentWnd = NULL );
BOOL PickFileName ( BOOL bOpenFileDialog, CString &strFileName, LPCSTR pszPrompt, LPCSTR pszFilter, LPCSTR pszDefExt, DWORD dwFlags, BOOL bCheckPath = TRUE, LPCSTR pszDefFile = FILE_NONE, CWnd *pParentWnd = NULL );
BOOL PickFileName ( BOOL bOpenFileDialog, CString &strFileName, LPCSTR pszPrompt, UINT uFilterID, LPCSTR pszDefExt, DWORD dwFlags, BOOL bCheckPath = TRUE, LPCSTR pszDefFile = FILE_NONE, CWnd *pParentWnd = NULL );
BOOL PickFileName ( BOOL bOpenFileDialog, CString &strFileName, UINT uPromptID, LPCSTR pszFilter, LPCSTR pszDefExt, DWORD dwFlags, BOOL bCheckPath = TRUE, LPCSTR pszDefFile = FILE_NONE, CWnd *pParentWnd = NULL );
BOOL PickFileName ( BOOL bOpenFileDialog, CString &strFileName, UINT uPromptID, UINT uFilterID, LPCSTR pszDefExt, DWORD dwFlags, BOOL bCheckPath = TRUE, LPCSTR pszDefFile = FILE_NONE, CWnd *pParentWnd = NULL );
BOOL PickCartridge( LPSTR pszCurrentCart, CWnd *pParentWnd = NULL );

UINT CALLBACK FileDialogHookProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );

#endif /*__cplusplus*/

#ifdef __cplusplus
extern "C" {
#endif

/*
   Common declarations
*/

BOOL IsAtariFile     ( LPCSTR pszFileName, PUINT pFileType );
BOOL IsCompressedFile( LPCSTR pszFileName );
void GetFolderPath   ( LPSTR pszPathName, LPSTR pszFileName );
BOOL RunExecutable   ( LPSTR pszFileName );
BOOL RunSnapshot     ( LPSTR pszFileName );
int  AttachCartridge ( LPSTR pszFileName, int nType );
int  SelectCartType  ( int nCartSize );
BOOL GetBootFileInfo ( LPSTR pszFileName, int nBufferLen, PUINT pFileType );

/* Exported globals */

extern LPCSTR DEFAULT_VID;
extern LPCSTR DEFAULT_SND;
extern LPCSTR DEFAULT_PIC;
extern LPCSTR DEFAULT_A8S;
extern LPCSTR DEFAULT_A8T;
extern LPCSTR DEFAULT_A8K;
extern LPCSTR DEFAULT_BIN;
extern LPCSTR DEFAULT_ACT;
extern LPCSTR DEFAULT_TAP;
extern LPCSTR DEFAULT_OSA;
extern LPCSTR DEFAULT_OSB;
extern LPCSTR DEFAULT_OXL;
extern LPCSTR DEFAULT_O52;
extern LPCSTR DEFAULT_BAS;
extern LPCSTR DEFAULT_HDD;

extern char   g_szCurrentRom  [];
extern char   g_szOtherRom    [];
extern char   g_szTemplateFile[];
extern char   g_szTemplateDesc[];
extern char   g_szPaletteFile [];
extern char   g_szAutobootFile[];
extern char   g_szBinaryFile  [];
extern char   g_szSnapshotFile[];
extern char   g_szHardExePath [];

#ifdef __cplusplus
}
#endif

#endif /*__FILESERVICE_H__*/
