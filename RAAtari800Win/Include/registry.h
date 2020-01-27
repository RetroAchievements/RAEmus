/****************************************************************************
File    : registry.h
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Registry public methods and objects prototypes
@(#) #BY# Richard Lawrence, Tomasz Szymankowski
@(#) #LM# 10.10.2003
*/

#ifndef __REGISTRY_H__
#define __REGISTRY_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Constants declarations */

#define	REGNAME					"Software\\Atari800WinPLus"
#define	READ_REG_FAIL			999999

/* Whenever any value is changed/deleted/added in this file,
   change CURRENT_REV */
#define CURRENT_REV				20

#define REG_MACHINE_TYPE		"sysMachineType"
#define REG_RAMSIZE_OSA			"sysRamSizeOSA"
#define REG_RAMSIZE_OSB			"sysRamSizeOSB"
#define REG_RAMSIZE_XLXE		"sysRamSizeXLXE"
#define REG_TV_MODE				"sysTVMode"
#define REG_DISABLE_BASIC		"sysDisableBasic"
#define REG_ENABLE_RTIME		"sysEnableRTime"
#define REG_ENABLE_SIO_PATCH	"sysEnableSIOPatch"
#define REG_ENABLE_H_PATCH		"sysEnableHPatch"
#define REG_ENABLE_P_PATCH		"sysEnablePPatch"
#define REG_ENABLE_R_PATCH		"sysEnableRPatch"
#define REG_HD_READ_ONLY		"sysHDReadOnly"
#define REG_PRINT_COMMAND		"sysPrintCommand"
#define REG_CART_TYPE			"sysCartType"

#define REG_SCREEN_STATE		"screenState"
#define REG_STRETCH_MODE		"screenStretchMode"
#define REG_MEMORY_TYPE			"screenMemoryType"
#define REG_REQUEST_RATE		"screenRequestRate"
#define REG_ARTIF_MODE			"screenArtifactMode"
#define REG_COLOR_BLACK			"screenColorBlack"
#define REG_COLOR_WHITE			"screenColorWhite"
#define REG_COLOR_SATURATION	"screenColorSaturation"
#define REG_COLOR_CONTRAST		"screenColorContrast"
#define REG_COLOR_BRIGHTNESS	"screenColorBrightness"
#define REG_COLOR_GAMMA			"screenColorGamma"
#define REG_START_XPOS			"screenStartXPos"
#define REG_START_YPOS			"screenStartYPos"

#define REG_ROM_OSA				"fileRomOSA"
#define REG_ROM_OSB				"fileRomOSB"
#define REG_ROM_XLXE			"fileRomXLXE"
#define REG_ROM_5200			"fileRom5200"
#define REG_ROM_BASIC			"fileRomBASIC"
#define REG_ROM_OTHER			"fileRomCartridge"
#define REG_ROM_CURRENT			"fileRomCurrent"
#define REG_FILE_TEMPLATE		"fileTemplate"
#define REG_FILE_PALETTE		"filePalette"
#define REG_FILE_TAPE			"fileTape"
#define REG_FILE_STATE			"fileState"
#define REG_FILE_AUTOBOOT		"fileAutoboot"

#define REG_DRIVE1				"pathDiskDrive1"
#define REG_DRIVE2				"pathDiskDrive2"
#define REG_DRIVE3				"pathDiskDrive3"
#define REG_DRIVE4				"pathDiskDrive4"
#define REG_DRIVE5				"pathDiskDrive5"
#define REG_DRIVE6				"pathDiskDrive6"
#define REG_DRIVE7				"pathDiskDrive7"
#define REG_DRIVE8				"pathDiskDrive8"
#define REG_HD1					"pathHardDisk1"
#define REG_HD2					"pathHardDisk2"
#define REG_HD3					"pathHardDisk3"
#define REG_HD4					"pathHardDisk4"
#define REG_HDE_PATH			"pathHardDiskExe"
#define REG_EXE_PATH			"pathExe"

#define REG_INPUT_STATE			"inputState"
#define REG_JOYSTICKS			"inputJoySticks"
#define REG_AUTOFIRE_MODE		"inputJoyAutofireMode"
#define REG_AUTOFIRE_STICKS		"inputJoyAutofireSticks"
#define REG_BLOCK_OPPOSITE		"inputJoyBlockOpposite"
#define REG_MULTI_JOY			"inputJoyMultiJoy"
#define REG_KEYSET_A1			"inputJoyKeysetA1"
#define REG_KEYSET_A2			"inputJoyKeysetA2"
#define REG_KEYSET_A3			"inputJoyKeysetA3"
#define REG_KEYSET_B1			"inputJoyKeysetB1"
#define REG_KEYSET_B2			"inputJoyKeysetB2"
#define REG_KEYSET_B3			"inputJoyKeysetB3"
#define REG_ARROWS_MODE			"inputKeyArrowsMode"
#define REG_MOUSE_MODE			"inputMouseMode"
#define REG_MOUSE_PORT			"inputMousePort"
#define REG_MOUSE_SPEED			"inputMouseSpeed"
#define REG_PEN_XOFFSET			"inputMousePenXOffset"
#define REG_PEN_YOFFSET			"inputMousePenYOffset"
#define REG_POT_MIN				"inputMousePotMin"
#define REG_POT_MAX				"inputMousePotMax"
#define REG_JOY_INERTIA			"inputMouseJoyInertia"

#define REG_SOUND_STATE			"soundState"
#define REG_SOUND_RATE			"soundRate"
#define REG_SOUND_VOLUME		"soundVolume"
#define REG_SOUND_UPDATE		"soundUpdate"
#define REG_SOUND_LATENCY		"soundLatency"
#define REG_SOUND_QUALITY		"soundQuality"
#define REG_SOUND_DIGITIZED		"soundDigitized"
#define REG_ENABLE_STEREO		"soundEnableStereo"
#define REG_BIENIAS_FIX			"soundBieniasFix"

#define REG_MISC_STATE			"miscState"
#define REG_DONT_SHOW			"miscDontShowFlags"
#define REG_FILE_ASSOCIATIONS	"miscFileAssociations"
#define REG_CHEAT_COLLISIONS	"miscCheatCollisions"
#define REG_CHEAT_MEMO			"miscCheatMemo"
#define REG_CHEAT_SEARCH		"miscCheatSearch"
#define REG_CHEAT_LOCK			"miscCheatLock"
#define REG_SECTOR_COUNTER		"miscSectorCounter"
#define REG_REFRESH_RATE		"miscRefreshRate"
#define REG_REFRESH_DOUBLEWND	"miscRefreshDoubleWnd"
#define REG_REFRESH_FULLSPEED	"miscRefreshFullSpeed"
#define REG_SPEED_PERCENT		"miscSpeedPercent"

#define REG_KAILLERA_STATE		"kailleraState"
#define REG_KAILLERA_LOCAL_PORT	"kailleraLocalPort"
#define REG_KAILLERA_FRAME_SKIP	"kailleraFrameSkip"

#define REG_CURRENT_REV			"VERSION"


/* Exported methods */

/* These are functions to handle initialization and reading/writing to the 
   registry of persistent data elements */

void  InitialiseRegistry    ( HKEY hkInput, BOOL bErasePaths );
BOOL  HandleRegistry        ( void );
void  WriteAtari800Registry ( void );

int   ReadRegDWORD          ( HKEY hkInput, LPSTR pszItem, DWORD *pdwDate, BOOL bShowError );
DWORD ReadRegBinary         ( HKEY hkInput, LPSTR pszItem, LPSTR pszBuffer, ULONG ulMaxSize, BOOL bShowError );
void  ReadRegDrives         ( void );
int   ReadRegKeyset         ( HKEY hkInput, int nKeyset );

void  WriteRegDWORD         ( HKEY hkInput, LPSTR pszItem, DWORD dwValue);
void  WriteRegBinary        ( HKEY hkInput, LPSTR pszItem, UCHAR *pszData, int nSize );
void  WriteRegString        ( HKEY hkInput, LPSTR pszItem, LPSTR pszData );
void  WriteRegDrives        ( HKEY hkInput );
void  WriteRegKeyset		( HKEY hkInput, int nKeyset );

void  DeleteAllRegKeys      ( HKEY hkInput, LPSTR pszName );

HKEY  GetRegKeyHandle       ( LPCSTR pszKeyName, BOOL bCreateKey );


#ifdef __cplusplus
}
#endif

#endif /*__REGISTRY_H__*/
