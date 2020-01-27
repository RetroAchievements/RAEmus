/****************************************************************************
File    : misc_win.h
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# MiscWin public methods and objects prototypes
@(#) #BY# Richard Lawrence, Tomasz Szymankowski
@(#) #LM# 12.07.2003
*/

#ifndef __MISC_WIN_H__
#define __MISC_WIN_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Constants declarations */

#define MS_FULL_SPEED				0x00000001 /* Misc states to be saved out to the Registry */
#define MS_AUTO_REG_FILES			0x00000002
#define MS_LAST_BOOT_FAILED			0x00000004
#define MS_DISABLE_COLLISIONS		0x00000008
#define MS_USE_EXT_PALETTE			0x00000010
#define MS_REUSE_WINDOW				0x00000020
#define MS_USE_DOUBLEWND_REFRESH	0x00000040
#define MS_TRANS_LOADED_PAL			0x00000080
#define MS_USE_PRINT_COMMAND		0x00000100
#define MS_SHOW_DRIVE_LED			0x00000200
#define MS_STOP_WHEN_NO_FOCUS		0x00000400
#define MS_SHOW_INDICATORS			0x00000800
#define MS_NO_DRAW_DISPLAY			0x00001000
#define MS_HIGH_PRIORITY			0x00002000
#define MS_CONFIRM_ON_EXIT			0x00004000
#define MS_REBOOT_WHEN_CART			0x00008000
#define MS_REBOOT_WHEN_VIDEO		0x00010000
#define MS_TURN_DRIVES_OFF			0x00020000
#define MS_VIDEO_AND_SOUND			0x00040000
#define MS_CHEAT_LOCK				0x00080000
#define MS_NO_ATTRACT_MODE			0x00100000
#define MS_USE_FULLSPEED_REFRESH	0x00200000
#define MS_MONITOR_ALWAYS			0x00400000
#define MS_MONITOR_NEVER			0x00800000
#define MS_DRIVE_READONLY			0x01000000
#define MS_REBOOT_WHEN_BASIC		0x02000000
#define MS_SHOW_SECTOR_COUNTER		0x04000000
#define MS_THREADED_EMULATION		0x08000000

#define DONT_SHOW_DOUBLE_WARN		0x00000001 /* "Don't show" warning flags */
#define DONT_SHOW_SOUNDFILE_WARN	0x00000002
#define DONT_SHOW_MENU_WARN			0x00000004
#define DONT_SHOW_OSA_WARN			0x00000008
#define DONT_SHOW_OSB_WARN			0x00000010
#define DONT_SHOW_XLXE_WARN			0x00000020
#define DONT_SHOW_5200_WARN			0x00000040
#define DONT_SHOW_BASIC_WARN		0x00000080
#define DONT_SHOW_VBLWAIT_WARN		0x00000100
#define DONT_SHOW_KBJOY_WARN		0x00000200
#define DONT_SHOW_VIDEOFILE_WARN	0x00000400
#define DONT_SHOW_CASFILE_WARN		0x00000800
#define DONT_SHOW_RECAVI_WARN		0x00001000
#define DONT_SHOW_RECWAV_WARN		0x00002000
#define DONT_SHOW_ZLIBLOAD_WARN		0x00004000
#define DONT_SHOW_ZLIBUSE_WARN		0x00008000
#define DONT_SHOW_SOCKLOAD_WARN		0x00010000
#define DONT_SHOW_SOCKUSE_WARN		0x00020000
#define DONT_SHOW_KALOAD_WARN		0x00040000
#define DONT_SHOW_KAUSE_WARN		0x00080000
#define DONT_SHOW_KAVERSION_WARN	0x00100000
#define DONT_SHOW_PANIC_WARN		0x00200000

#define DC_PLAYER_PLAYER			0x0001 /* Collisions detection flags */
#define DC_PLAYER_PLAYFIELD			0x0002
#define DC_MISSILE_PLAYER			0x0004
#define DC_MISSILE_PLAYFIELD		0x0008

#define ATTRACT_CHECK_FRAMES		550

#define DEF_MISC_STATE				(MS_THREADED_EMULATION | MS_SHOW_DRIVE_LED | MS_SHOW_INDICATORS | MS_CONFIRM_ON_EXIT | MS_REBOOT_WHEN_CART | MS_REBOOT_WHEN_VIDEO | MS_TURN_DRIVES_OFF)
#define DEF_DONT_SHOW_FLAGS			0L
#define DEF_FILE_ASSOCIATIONS		0L
#define DEF_CHEAT_COLLISIONS		(DC_PLAYER_PLAYER | DC_MISSILE_PLAYER)
#define DEF_CHEAT_MEMO				3
#define DEF_CHEAT_SEARCH			2
#define DEF_CHEAT_LOCK				3
#define DEF_REFRESH_DOUBLEWND		2
#define DEF_REFRESH_FULLSPEED		2
#define DEF_SPEED_PERCENT			100

#define SYS_WIN_95					0x0001
#define SYS_WIN_98					0x0002
#define SYS_WIN_9x					0x000f
#define SYS_WIN_NT4					0x0010
#define SYS_WIN_NT5					0x0020
#define SYS_WIN_NT					0x00f0
#define SYS_PRC_MMX					0x0100
#define SYS_PRC_3DN					0x0200
#define SYS_PRC_SSE					0x0400
#define SYS_PRC_EN					0x0f00

#define ROM_TYPES_NO				5

/* Exported types */

enum RomType { RTI_OSA = 0, RTI_OSB, RTI_XLE, RTI_A52, RTI_BAS };

struct RomTypeInfo_t
{
	enum   RomType rtType;
	ULONG  ulCRC;
	LPCSTR pszDesc;		/* The pointer to a static text */
};

/* Exported methods */

void Atari_Initialise( int *argc, char *argv[] );
void  Misc_ToggleFullSpeed    ( void );
void  Misc_TogglePause        ( void );
void  Misc_ToggleSIOPatch     ( void );
void  Misc_CheckAttractCounter( void );
BOOL  Misc_AllocMonitorConsole( FILE **pOutput, FILE **pInput );
void  Misc_FreeMonitorConsole ( FILE *pOutput, FILE *pInput );
int   Misc_LaunchMonitor      ( void );
BOOL  Misc_TestRomPaths       ( LPSTR pszStartPath, HWND hWnd );
BOOL  Misc_GetHomeDirectory   ( LPSTR pszHomeDir );
HWND  Misc_FindWindow         ( LPCSTR pszTitle, DWORD dwThreadId );
HWND  Misc_FindChildWindow    ( HWND hParent, LPCSTR pszTitle );
void  Misc_EnableCloseItem    ( HWND hWnd, BOOL bEnable );
void  Misc_UpdateCollisions   ();
BOOL  Misc_GetSystemInfo      ( UINT *pSystemInfo );
BOOL  Misc_ExecutePrintCmd    ( LPSTR pszPrintFile );
void  Misc_SetProcessPriority ( void );
void  Misc_PrintTime          ( void );
/* This one is invoked directly by kernel */
int   Atari_Exit              ( int nPanic );

/* Exported globals */

struct MiscCtrl_t
{
	ULONG ulState;				/* Emulator state flags    */
	ULONG ulFileAssociations;	/* File associations flags */
	ULONG ulDontShow;			/* "Do not show" flags     */
	int   nSpeedPercent;
	UINT  unSystemInfo;
	int   nAttractCounter;
	/* Cheat */
	struct CheatCtrl_t
	{
		ULONG ulCollisions;		/* Collisions detection flags */
		int   nMemo;
		int   nSearch;
		int   nLock;			/* Lock value */
	} Cheat;
	/* Refresh */
	struct RefreshCtrl_t
	{
		int nDoubleWnd;			/* Refresh rate for double-windowed mode */
		int nFullSpeed;			/* Refresh rate for full-speed mode */
	} Refresh;
};
extern struct MiscCtrl_t g_Misc;

extern const int g_nRomTypeInfoNo;

extern struct RomTypeInfo_t g_aRomTypeInfo[];

#ifdef __cplusplus
}
#endif

#endif /*__MISC_WIN_H__*/
