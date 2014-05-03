/*===================================================================
   UNZIP
   Copyright(c) 1989-1994 by Samuel H. Smith and Info-ZIP. All rights reserved.

   UNZIP32.H
   Windows' DLL Version
   Copyright(c) 1993-2006 by CSD,inc. and shoda T. All rights reserved.

			version 5.42	2006/ 3/ 1
			version 5.41	1999/11/20
			version 5.40	1999/ 2/ 4
			version 0.98	1999/ 1/28
			version 0.97	1999/ 1/25
			version 0.96	1999/ 1/23
			version 0.95	1999/ 1/20
			version 0.94	1999/ 1/19
			version 0.93	1999/ 1/18
			version 0.92	1999/ 1/15
			version 0.91	1999/ 1/14
			version 0.90	1998/12/15
			version 0.82	1997/10/10
			version 0.81	1997/9/15
			version 0.80	1997/8/15
			version 0.78	1997/2/15
			version 0.77	1997/2/9
			version 0.76	1997/2/4
			version 0.75	1997/1/13
			version 0.74	1997/1/8
			version 0.70	1996/12/1
			version 0.60	1994/3/14
=====================================================================*/
#if !defined(UNZIP32_H)
#define UNZIP32_H

#define UNZIP_DLL_VERSION	542
#define	UNZIP_DLL_MINORVERSION	0

#if !defined(FNAME_MAX32)
#define FNAME_MAX32		512
#define	FNAME_MAX	FNAME_MAX32
#else
#if !defined(FNAME_MAX)
#define	FNAME_MAX	128
#endif
#endif

#if !defined(COMMENT_MAX)
#define	COMMENT_MAX	2048
#endif

#if defined(__BORLANDC__)
#pragma option -a-
#else
#pragma pack(1)
#endif

typedef	HGLOBAL	HARCHIVE;

#ifndef ARC_DECSTRACT
#define ARC_DECSTRACT
typedef	HGLOBAL	HARC;

typedef struct {
	DWORD	dwOriginalSize;		/* ファイルのサイズ。		*/
	DWORD	dwCompressedSize;	/* 圧縮後のサイズ。		*/
	DWORD	dwCRC;			/* 格納ファイルのチェックサム/CRC */
	UINT	uFlag;			/* 解凍やテストの処理結果	*/
	UINT	uOSType;		/* このファイルの作成に使われたＯＳ。*/
	WORD	wRatio;			/* 圧縮率（パーミル)		*/
	WORD	wDate;			/* 格納ファイルの日付。		*/
	WORD	wTime;			/* 格納ファイルの時刻。		*/
	char	szFileName[FNAME_MAX32 + 1];/* アーカイブファイル名・*/
	char	dummy1[3];
	char	szAttribute[8];		/* 格納ファイルの属性。		*/
	char	szMode[8];		/* 格納ファイルの格納モード。	*/
} INDIVIDUALINFO, FAR *LPINDIVIDUALINFO;

typedef struct {
	DWORD	dwFileSize;			/* 格納ファイルのサイズ	*/
	DWORD	dwWriteSize;			/* 解凍して書き込んだサイズ */
	char	szSourceFileName[FNAME_MAX32 + 1]; /* 処理を行う格納ファイル名 */
	char	dummy1[3];
	char	szDestFileName[FNAME_MAX32 + 1];	/* 実際に書き込まれるパス名 */
	char	dummy[3];
} EXTRACTINGINFO, FAR *LPEXTRACTINGINFO;

typedef struct {
	EXTRACTINGINFO exinfo;
	DWORD dwCompressedSize;	/* 圧縮後のサイズ。*/
	DWORD dwCRC;			/* 格納ファイルのチェックサム/CRC */
	UINT  uOSType;			/* このファイルの作成に使われたＯＳ */
	WORD  wRatio;			/* 圧縮率（パーミル) */
	WORD  wDate;			/* 格納ファイルの日付。*/
	WORD  wTime;			/* 格納ファイルの時刻。*/
	char  szAttribute[8];	/* 格納ファイルの属性。*/
	char  szMode[8];		/* 格納ファイルの格納モード。*/
} EXTRACTINGINFOEX, *LPEXTRACTINGINFOEX;
#endif

#if !defined(__BORLANDC__)
#pragma pack()
#else
#pragma option -a.
#endif

#if !defined(__BORLANDC__)
#define	_export
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* LHA.DLL Compatible API (LHA.DLL V1.1 と互換性を考慮した API 群です）
 */
int WINAPI _export UnZip(const HWND hWnd,LPCSTR szCmdLine,LPSTR szOutput,const DWORD wSize);
WORD WINAPI _export UnZipGetVersion(VOID);
WORD WINAPI _export UnZipGetSubVersion(VOID);
BOOL WINAPI _export UnZipGetRunning(VOID);
BOOL WINAPI _export UnZipGetBackGroundMode(VOID);
BOOL WINAPI _export UnZipSetBackGroundMode(const BOOL bBackGroundMode);
BOOL WINAPI _export UnZipGetCursorMode(VOID);
BOOL WINAPI _export UnZipSetCursorMode(const BOOL bCursorMode);
WORD WINAPI _export UnZipGetCursorInterval(VOID);
BOOL WINAPI _export UnZipSetCursorInterval(const WORD wInterval);

int WINAPI _export UnZipCommandLine(HWND hWnd,HINSTANCE hInst,LPCSTR szCmdLine,DWORD nCmdShow);


/* True Compatible API (『統合アーカイバＡＰＩ』独自の、互換性の高い API 群です）
	新規に使用される場合は、出来るだけこちらをお使いください。
	注：現在策定中で、具体的に決定したものは下記のみです。
 */

BOOL WINAPI _export UnZipCheckArchive(LPCSTR szFileName,const int iMode);
/**/
#if !defined(CHECKARCHIVE_RAPID)
#define	CHECKARCHIVE_RAPID	0 /* 簡易型（高速） */
#define	CHECKARCHIVE_BASIC	1 /* 標準型（ヘッダーのみ） */
#define	CHECKARCHIVE_FULLCRC	2 /* 完全型（ＣＲＣ等のチェックを含む） */
#endif

BOOL WINAPI _export UnZipConfigDialog(const HWND hwnd,LPSTR szOption,const int iMode);
/**/
#if !defined(UNPACK_CONFIG_MODE)
#define	UNPACK_CONFIG_MODE	1 /* 解凍（復元）系のコマンド */
#define	PACK_CONFIG_MODE	2 /* 圧縮（作成）系のコマンド */
#endif

BOOL WINAPI _export UnZipQueryFunctionList(const int iFunction);
/**/
#if !defined(ISARC_FUNCTION_START)
#define ISARC_FUNCTION_START			0
#define ISARC					0
#define ISARC_GET_VERSION			1
#define ISARC_GET_CURSOR_INTERVAL		2
#define ISARC_SET_CURSOR_INTERVAL		3
#define ISARC_GET_BACK_GROUND_MODE		4
#define ISARC_SET_BACK_GROUND_MODE		5
#define ISARC_GET_CURSOR_MODE			6
#define ISARC_SET_CURSOR_MODE			7
#define ISARC_GET_RUNNING			8

#define ISARC_CHECK_ARCHIVE			16
#define ISARC_CONFIG_DIALOG			17
#define ISARC_GET_FILE_COUNT			18
#define ISARC_QUERY_FUNCTION_LIST		19
#define ISARC_HOUT				20
#define ISARC_STRUCTOUT				21
#define ISARC_GET_ARC_FILE_INFO			22

#define ISARC_OPEN_ARCHIVE			23
#define ISARC_CLOSE_ARCHIVE			24
#define ISARC_FIND_FIRST			25
#define ISARC_FIND_NEXT				26
#define ISARC_EXTRACT				27
#define ISARC_ADD				28
#define ISARC_MOVE				29
#define ISARC_DELETE				30
#define ISARC_SETOWNERWINDOW			31
#define ISARC_CLEAROWNERWINDOW			32
#define ISARC_SETOWNERWINDOWEX			33
#define ISARC_KILLOWNERWINDOWEX			34

#define ISARC_GET_ARC_FILE_NAME			40
#define ISARC_GET_ARC_FILE_SIZE			41
#define ISARC_GET_ARC_ORIGINAL_SIZE		42
#define ISARC_GET_ARC_COMPRESSED_SIZE		43
#define ISARC_GET_ARC_RATIO			44
#define ISARC_GET_ARC_DATE			45
#define ISARC_GET_ARC_TIME			46
#define ISARC_GET_ARC_OS_TYPE			47
#define ISARC_GET_ARC_IS_SFX_FILE		48
#define ISARC_GET_ARC_CREATE_TIME_EX	50
#define	ISARC_GET_ARC_ACCESS_TIME_EX	51
#define	ISARC_GET_ARC_CREATE_TIME_EX2	52
#define ISARC_GET_FILE_NAME			57
#define ISARC_GET_ORIGINAL_SIZE			58
#define ISARC_GET_COMPRESSED_SIZE		59
#define ISARC_GET_RATIO				60
#define ISARC_GET_DATE				61
#define ISARC_GET_TIME				62
#define ISARC_GET_CRC				63
#define ISARC_GET_ATTRIBUTE			64
#define ISARC_GET_OS_TYPE			65
#define ISARC_GET_METHOD				66
#define ISARC_GET_WRITE_TIME			67
#define ISARC_GET_CREATE_TIME			68
#define ISARC_GET_ACCESS_TIME			69
#define ISARC_GET_WRITE_TIME_EX			70
#define ISARC_GET_CREATE_TIME_EX		71
#define ISARC_GET_ACCESS_TIME_EX		72
#define ISARC_SET_ENUM_MEMBERS_PROC		80
#define ISARC_CLEAR_ENUM_MEMBERS_PROC	81

#define ISARC_FUNCTION_END			81
#endif	/* ISARC_FUNCTION_START */



/* 格納されているファイルの情報の取得。
 */
//int WINAPI _export UnZipGetArcFileInfo(LPSTR szFileName,LPMAININFO lpMainInfo);

int WINAPI _export UnZipGetFileCount(LPCSTR szArcFile);

HARC WINAPI _export UnZipOpenArchive(const HWND hWnd,LPCSTR szFileName,const DWORD dwMode);
int WINAPI _export UnZipCloseArchive(HARC harc);
int WINAPI _export UnZipFindFirst(HARC harc,LPCSTR szWildName,LPINDIVIDUALINFO lpSubInfo);
int WINAPI _export UnZipFindNext(HARC harc,LPINDIVIDUALINFO lpSubInfo);
int WINAPI _export UnZipExtract(HARC harc,LPCSTR szFileName,LPCSTR szDirName,DWORD dwMode);
int WINAPI _export UnZipAdd(HARC harc,LPSTR szFileName,DWORD dwMode);
int WINAPI _export UnZipMove(HARC harc,LPSTR szFileName,DWORD dwMode);
int WINAPI _export UnZipDelete(HARC harc,LPSTR szFileName,DWORD dwMode);
int WINAPI _export UnZipGetArcFileName(HARC harc,LPSTR lpBuffer,const int nSize);
DWORD WINAPI _export UnZipGetArcFileSize(HARC harc);
DWORD WINAPI _export UnZipGetArcOriginalSize(HARC harc);
DWORD WINAPI _export UnZipGetArcCompressedSize(HARC harc);
WORD WINAPI _export UnZipGetArcRatio(HARC harc);
WORD WINAPI _export UnZipGetArcDate(HARC harc);
WORD WINAPI _export UnZipGetArcTime(HARC harc);
UINT WINAPI _export UnZipGetArcOSType(HARC harc);
int WINAPI _export UnZipGetFileName(HARC harc,LPSTR lpBuffer,const int nSize);
int WINAPI _export UnZipGetMethod(HARC harc,LPSTR lpBuffer,const int nSize);
DWORD WINAPI _export UnZipGetOriginalSize(HARC harc);
DWORD WINAPI _export UnZipGetCompressedSize(HARC harc);
WORD WINAPI _export UnZipGetRatio(HARC harc);
WORD WINAPI _export UnZipGetDate(HARC harc);
WORD WINAPI _export UnZipGetTime(HARC harc);

DWORD WINAPI _export UnZipGetWriteTime(HARC harc);
DWORD WINAPI _export UnZipGetAccessTime(HARC harc);
DWORD WINAPI _export UnZipGetCreateTime(HARC harc);

DWORD WINAPI _export UnZipGetCRC(HARC harc);
int WINAPI _export UnZipGetAttribute(HARC harc);
UINT WINAPI _export UnZipGetOSType(HARC harc);

int WINAPI _export UnZipIsSFXFile(HARC harc);

BOOL WINAPI _export UnZipSetOwnerWindow(const HWND hwnd);
BOOL WINAPI _export UnZipClearOwnerWindow(VOID);

#ifndef WM_ARCEXTRACT
#define	WM_ARCEXTRACT	"wm_arcextract"

#define	ARCEXTRACT_BEGIN	0	/* 該当ファイルの処理の開始 */
#define	ARCEXTRACT_INPROCESS	1	/* 該当ファイルの展開中 */
#define	ARCEXTRACT_END		2	/* 処理終了、関連メモリを開放 */
/* 以下は UNZIP では使用していない */
#define ARCEXTRACT_OPEN			3	/* 該当書庫の処理の開始 */
#define ARCEXTRACT_COPY			4	/* ワークファイルの書き戻し */

typedef BOOL CALLBACK ARCHIVERPROC(HWND hWnd,UINT uMsg,UINT nState,LPEXTRACTINGINFOEX lpEis);
typedef ARCHIVERPROC *LPARCHIVERPROC;

BOOL WINAPI _export UnZipSetOwnerWindowEx(HWND hWnd,LPARCHIVERPROC lpArcProc);
BOOL WINAPI _export UnZipKillOwnerWindowEx(HWND hWnd);
#endif


/* DMZIP32.DLL との互換のためにあります。
   UNZIP32.DLL のバージョンを返しますが、UNZIP32.DLL を示すために、0x2000 が
   加算されています（UnZipGetVersion() + 0x2000）。
 */
WORD WINAPI _export MicUnZipGetVersion(VOID);


#if !defined(EXTRACT_FOUND_FILE)
/* MODE (for OpenArchive) */
#define M_INIT_FILE_USE			0x00000001L
#define M_REGARDLESS_INIT_FILE	0x00000002L
#define M_NOT_USE_TIME_STAMP	0x00000008L
#define M_EXTRACT_REPLACE_FILE	0x00000010L
#define M_EXTRACT_NEW_FILE		0x00000020L
#define M_EXTRACT_UPDATE_FILE	0x00000040L
#define M_CHECK_ALL_PATH		0x00000100L
#define M_CHECK_FILENAME_ONLY	0x00000200L
#define M_USE_DRIVE_LETTER		0x00001000L
#define M_NOT_USE_DRIVE_LETTER	0x00002000L
#define M_INQUIRE_DIRECTORY		0x00004000L
#define M_NOT_INQUIRE_DIRECTORY 0x00008000L
#define M_INQUIRE_WRITE			0x00010000L
#define M_NOT_INQUIRE_WRITE		0x00020000L
#define M_REGARD_E_COMMAND		0x00100000L
#define M_REGARD_X_COMMAND		0x00200000L
#define M_ERROR_MESSAGE_ON		0x00400000L
#define M_ERROR_MESSAGE_OFF		0x00800000L
#define M_BAR_WINDOW_ON			0x01000000L
#define M_BAR_WINDOW_OFF		0x02000000L
#define M_RECOVERY_ON			0x08000000L /* 破損ヘッダの読み飛ばし */

#define EXTRACT_FOUND_FILE		0x40000000L
#define EXTRACT_NAMED_FILE		0x80000000L

#endif /* EXTRACT_FOUND_FILE */


#ifndef FA_RDONLY
/* Attribute */
#define FA_RDONLY       0x01            /* Read only attribute */
#define FA_HIDDEN       0x02            /* Hidden file */
#define FA_SYSTEM       0x04            /* System file */
#define FA_LABEL        0x08            /* Volume label */
#define FA_DIREC        0x10            /* Directory */
#define FA_ARCH         0x20            /* Archive */
#endif
#define FA_ENCRYPTED    0x40            /* Encripted by password */

#ifdef __cplusplus
}
#endif

#if !defined(ERROR_START)
#define ERROR_START			0x8000

/* WARNING */
#define ERROR_DISK_SPACE		0x8005
#define ERROR_READ_ONLY			0x8006
#define ERROR_USER_SKIP			0x8007
#define ERROR_UNKNOWN_TYPE		0x8008
#define ERROR_METHOD			0x8009
#define ERROR_PASSWORD_FILE		0x800A
#define ERROR_VERSION			0x800B
#define ERROR_FILE_CRC			0x800C
#define ERROR_FILE_OPEN			0x800D
#define ERROR_MORE_FRESH		0x800E
#define ERROR_NOT_EXIST			0x800F
#define ERROR_ALREADY_EXIST		0x8010

#define ERROR_TOO_MANY_FILES		0x8011

/* ERROR */
//#define ERROR_DIRECTORY			0x8012
#define ERROR_MAKEDIRECTORY		0x8012
#define ERROR_CANNOT_WRITE		0x8013
#define ERROR_HUFFMAN_CODE		0x8014
#define ERROR_COMMENT_HEADER		0x8015
#define ERROR_HEADER_CRC		0x8016
#define ERROR_HEADER_BROKEN		0x8017
#define ERROR_ARCHIVE_FILE_OPEN		0x8018
//#define ERROR_ARC_FILE_OPEN		ERROR_ARCHIVE_FILE_OPEN	
#define ERROR_NOT_ARC_FILE		0x8019
//#define ERROR_NOT_ARCHIVE_FILE		ERROR_NOT_ARC_FILE
#define ERROR_CANNOT_READ		0x801A
#define ERROR_FILE_STYLE		0x801B
#define ERROR_COMMAND_NAME		0x801C
#define ERROR_MORE_HEAP_MEMORY		0x801D
#define ERROR_ENOUGH_MEMORY		0x801E
#if !defined(ERROR_ALREADY_RUNNING)
#define ERROR_ALREADY_RUNNING		0x801F
#endif
#define ERROR_USER_CANCEL		0x8020

#define ERROR_HARC_ISNOT_OPENED	0x8021
#define ERROR_NOT_SEARCH_MODE	0x8022
#define ERROR_NOT_SUPPORT		0x8023
#define ERROR_TIME_STAMP		0x8024

//#define ERROR_NULL_POINTER		0x8025
//#define ERROR_ILLEGAL_PARAMETER		0x8026
#define ERROR_TMP_OPEN			0x8025
#define ERROR_LONG_FILE_NAME	0x8026
#define ERROR_ARC_READ_ONLY		0x8027
#define ERROR_SAME_NAME_FILE	0x8028
#define ERROR_NOT_FIND_ARC_FILE 0x8029
#define ERROR_RESPONSE_READ		0x802A
#define ERROR_NOT_FILENAME		0x802B

#define ERROR_INVALID_PATH		0x8049

#define ERROR_END	ERROR_NOT_FILENAME

#endif /* ERROR_START */

#endif	/* UNZIP32_H */
