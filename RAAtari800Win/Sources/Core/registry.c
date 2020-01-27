/****************************************************************************
File    : registry.c
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Implementation of registry handling API
@(#) #BY# Richard Lawrence, Tomasz Szymankowski
@(#) #LM# 10.10.2003
*/

#include <stdio.h>
#include <crtdbg.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
#include <windows.h>
#include "WinConfig.h"
#include "Resource.h"
#include "Helpers.h"
#include "FileService.h"
#include "atari800.h"
#include "globals.h"
#include "macros.h"
#include "display_win.h"
#include "misc_win.h"
#include "input_win.h"
#include "sound_win.h"
#include "registry.h"
#include "screen.h"
#include "cfg.h"

#ifdef WIN_NETWORK_GAMES
#include "kaillera.h"
#endif

#define PLUS_REGKEY		"Atari800WinPLus"

/* Private objects */

static int s_nVersion = CURRENT_REV;

/* Macro-definitions */

#define _RegReadString(hk, key, sz, def, len) \
	(ReadRegBinary( hk, key, sz, len, FALSE ) == READ_REG_FAIL && (_strncpy( sz, def, len ), TRUE))

#define _RegWriteString(hk, key, val) \
	WriteRegString( hk, key, val )

#define _RegReadNumber(hk, key, val, def) \
	(ReadRegDWORD( hk, key, (DWORD*)&(val), TRUE ) == READ_REG_FAIL && (val = def, TRUE))

#define _RegWriteNumber(hk, key, val) \
	WriteRegDWORD( hk, key, val )


/*========================================================
Function : DeleteAllRegKeys
=========================================================*/
/* #FN#
   DeleteAllRegKeys will recursively delete everything from a supplied initial
   Key. All subkeys are enumerated and deleted as found. Note that ALL values
   underneath a key are deleted when that key is deleted. */
void
/* #AS#
   Nothing */
DeleteAllRegKeys(
	HKEY  hkInput,
    LPSTR pszName
)
{
	HKEY     hkKey     = hkInput;
	DWORD    dwIndex   = 0;
	DWORD    dwBufSize = 256;
	FILETIME ftDummy;
	char     szSubKeyName[ 256 ];

	if( NULL == hkKey )
	{
		if( RegOpenKeyEx( HKEY_CURRENT_USER, pszName, 0, KEY_ALL_ACCESS, &hkKey ) != ERROR_SUCCESS )
			return;
	}
	while( RegEnumKeyEx( hkKey, dwIndex++, szSubKeyName, &dwBufSize, NULL, NULL, NULL, &ftDummy ) == ERROR_SUCCESS )
		DeleteAllRegKeys( hkKey, szSubKeyName );

	RegDeleteKey( HKEY_CURRENT_USER, pszName );

} /* #OF# DeleteAllRegKeys */

/*========================================================
Function : WriteRegDWORD
=========================================================*/
/* #FN#
   WriteRegDWORD writes out an int to the preset Registry key HKEY_CURRENT_USER\REGNAME.
   If the HKEY passed in is valid it is used, otherwise the key is grabbed and released
   within the function. Note that RegOpenKey is used here instead of RegCreateKey, which
   is only used at init time. No calls should be made to this prior to HandleRegistry().
   Any write to the Registry that doesn't work is skipped with user notification. */
void
/* #AS#
   Nothing */
WriteRegDWORD(
	HKEY  hkInput,
	LPSTR pszItem,
	DWORD dwValue
)
{
	HKEY hkKey = hkInput;

	if( NULL == hkKey )
	{
		if( RegOpenKeyEx( HKEY_CURRENT_USER, REGNAME, 0, KEY_WRITE, &hkKey ) != ERROR_SUCCESS )
		{
			DisplayMessage( NULL, IDS_REG_OPEN_ERROR, 0, MB_ICONSTOP | MB_OK );
			return;
		}
	}
	if( RegSetValueEx( hkKey, pszItem, 0, REG_DWORD, (UCHAR *)&dwValue, sizeof(DWORD) ) != ERROR_SUCCESS )
		DisplayMessage( NULL, IDS_REG_WRITE_ERROR, 0, MB_ICONSTOP | MB_OK );

	if( NULL == hkInput )
		RegCloseKey( hkKey );

} /* #OF# WriteRegDWORD */

/*========================================================
Function : ReadRegDWORD
=========================================================*/
/* #FN#
   ReadRegDWORD retrieves an existing value. To make it bulletproof the
   calling routine can request to display errors or not, depending on how
   fatal they are considered. */
int
/* #AS#
   The value was read */
ReadRegDWORD(
	HKEY   hkInput,
	LPSTR  pszItem,
	DWORD *pdwData,
	BOOL   bShowError
)
{
	HKEY  hkKey = hkInput;
	DWORD dwType;
	DWORD dwSize;
	DWORD dwValue;
	
	if( NULL == hkKey )
	{
		if( RegOpenKeyEx( HKEY_CURRENT_USER, REGNAME, 0, KEY_READ, &hkKey ) != ERROR_SUCCESS )
		{
			if( bShowError )
			{
				DisplayMessage( NULL, IDS_REG_OPEN_ERROR, 0, MB_ICONSTOP | MB_OK );
				return READ_REG_FAIL;
			}
		}
	}
	/* Win95 is really picky about having this size set; WinNT doesn't care. Go figure. */
	dwSize = sizeof(DWORD);

	if( RegQueryValueEx( hkKey, pszItem, 0, &dwType, (UCHAR *)&dwValue, &dwSize ) != ERROR_SUCCESS )
	{
		if( bShowError )
		{
			DisplayMessage( NULL, IDS_REG_LOAD_ERROR, 0, MB_ICONEXCLAMATION | MB_OK, pszItem );
		}
		else if( NULL == hkInput )
			RegCloseKey( hkKey );

		if( NULL == hkInput )
			RegCloseKey( hkKey );

		return READ_REG_FAIL;
	}
	
	if( dwType != REG_DWORD || dwSize != sizeof(DWORD) )
	{
		DisplayMessage( NULL, IDS_REG_WRONG_SIZE, 0, MB_ICONSTOP | MB_OK );
		if( NULL == hkInput )
			RegCloseKey( hkKey );

		return READ_REG_FAIL;
	}

	if( NULL == hkInput )
		RegCloseKey( hkKey );
	
	*pdwData = dwValue;

	return TRUE;

} /* #OF# ReadRegDWORD */

/*========================================================
Function : ReadRegBinary
=========================================================*/
/* #FN#
   Retrieves an existing value */
DWORD
/* #AS#
   The value was read */
ReadRegBinary(
	HKEY  hkInput,
	LPSTR pszItem,
	LPSTR pszBuffer,
	ULONG ulMaxSize,
	BOOL  bShowError
)
{
	HKEY  hkKey     = hkInput;
	UINT  nFullSize = ulMaxSize;
	DWORD dwType    = 0;
	
	if( NULL == hkKey )
	{
		if( RegOpenKeyEx( HKEY_CURRENT_USER, REGNAME, 0, KEY_READ, &hkKey ) != ERROR_SUCCESS )
		{
			if( bShowError )
			{
				DisplayMessage( NULL, IDS_REG_OPEN_ERROR, 0, MB_ICONSTOP | MB_OK );
				return READ_REG_FAIL;
			}
		}
	}
	if( RegQueryValueEx( hkKey, pszItem, 0, &dwType, (UCHAR *)pszBuffer, &ulMaxSize ) != ERROR_SUCCESS )
	{
		if( bShowError )
		{
			DisplayMessage( NULL, IDS_REG_KEY_ERROR, 0, MB_ICONSTOP | MB_OK, pszItem );
		}
		else
		{
			if( NULL == hkInput )
				RegCloseKey( hkKey );

			*pszBuffer = '\0';
		}
		if( NULL == hkInput )
			RegCloseKey( hkKey );

		return READ_REG_FAIL;
	}

	if( nFullSize > ulMaxSize + 1 )
		pszBuffer[ ulMaxSize + 1 ] = '\0';
	
	if( NULL == hkInput )
		RegCloseKey( hkKey );
	
	return dwType;

} /* #OF# ReadRegBinary */

/*========================================================
Function : WriteRegBinary
=========================================================*/
/* #FN#
   WriteRegBinary is similar to WriteRegDWORD except it dumps an arbitrary
   binary section of data */
void
/* #AS#
   Nothing */
WriteRegBinary(
	HKEY   hkInput,
	LPSTR  pszItem,
	UCHAR *pszData,
	int    nSize
)
{
	HKEY hkKey = hkInput;
	
	if( NULL == hkKey )
	{
		if( RegOpenKeyEx( HKEY_CURRENT_USER, REGNAME, 0, KEY_WRITE, &hkKey ) != ERROR_SUCCESS )
		{
			DisplayMessage( NULL, IDS_REG_OPEN_ERROR, 0, MB_ICONSTOP | MB_OK );
			return;
		}
	}
	if( RegSetValueEx( hkKey, pszItem, 0, REG_BINARY, pszData, nSize ) != ERROR_SUCCESS )
		DisplayMessage( NULL, IDS_REG_WRITE_ERROR, 0, MB_ICONSTOP | MB_OK );

	if( NULL == hkInput )
		RegCloseKey( hkKey );

} /* #OF# WriteRegBinary */

/*========================================================
Function : WriteRegString
=========================================================*/
/* #FN#
   WriteRegString is similar to WriteRegBinary except it writes a null-terminated
   string */
void
/* #AS#
   Nothing */
WriteRegString(
	HKEY  hkInput,
	LPSTR pszItem,
	LPSTR pszData
)
{
	HKEY hkKey = hkInput;
	
	if( NULL == hkKey )
	{
		if( RegOpenKeyEx( HKEY_CURRENT_USER, REGNAME, 0, KEY_WRITE, &hkKey ) != ERROR_SUCCESS )
		{
			DisplayMessage( NULL, IDS_REG_OPEN_ERROR, 0, MB_ICONSTOP | MB_OK );
			return;
		}
	}
	if( RegSetValueEx( hkKey, pszItem,	0, REG_SZ, (const UCHAR *)pszData, strlen( pszData ) ) != ERROR_SUCCESS )
		DisplayMessage( NULL, IDS_REG_WRITE_ERROR, 0, MB_ICONSTOP | MB_OK );

	if( NULL == hkInput )
		RegCloseKey( hkKey );

} /* #OF# WriteRegString */

/*========================================================
Function : ReadRegKeyset
=========================================================*/
/* #FN#
   */
int
/* #AS#
   */
ReadRegKeyset(
	HKEY hkInput,
	int  nKeyset
)
{
	HKEY  hkKey        = hkInput;
	BOOL  bFail        = FALSE;
	ULONG ulKeysetReg1 = 0L;
	ULONG ulKeysetReg2 = 0L;
	ULONG ulKeysetReg3 = 0L;
	int   i;

	if( NULL == hkKey )
	{
		if( RegOpenKeyEx( HKEY_CURRENT_USER, REGNAME, 0, KEY_READ, &hkKey ) != ERROR_SUCCESS )
		{
			DisplayMessage( NULL, IDS_REG_OPEN_ERROR, 0, MB_ICONSTOP | MB_OK );
			return FALSE;
		}
	}
	if( KEYS_A_JOYSTICK == nKeyset )
	{
		bFail |= _RegReadNumber( hkKey, REG_KEYSET_A1, ulKeysetReg1, 0L );
		bFail |= _RegReadNumber( hkKey, REG_KEYSET_A2, ulKeysetReg2, 0L );
		bFail |= _RegReadNumber( hkKey, REG_KEYSET_A3, ulKeysetReg3, 0L );
	}
	else
	{
		bFail |= _RegReadNumber( hkKey, REG_KEYSET_B1, ulKeysetReg1, 0L );
		bFail |= _RegReadNumber( hkKey, REG_KEYSET_B2, ulKeysetReg2, 0L );
		bFail |= _RegReadNumber( hkKey, REG_KEYSET_B3, ulKeysetReg3, 0L );
	}
	_ASSERT(NUM_KBJOY_KEYS - 2 <= sizeof(ULONG) * 2);

	i = NUM_KBJOY_KEYS - 1;

	g_Input.Joy.anKeysets[ nKeyset + NUM_KBJOY_DEVICES ][ i-- ] = (WORD)(ulKeysetReg3 & 0x000000ffL);
	ulKeysetReg3 = ulKeysetReg3 >> 8;
	g_Input.Joy.anKeysets[ nKeyset + NUM_KBJOY_DEVICES ][ i-- ] = (WORD)(ulKeysetReg3 & 0x000000ffL);

	for( ; i > -1; )
	{
		g_Input.Joy.anKeysets[ nKeyset + NUM_KBJOY_DEVICES ][ i-- ] = (WORD)(ulKeysetReg2 & 0x000000ffL);
		ulKeysetReg2 = ulKeysetReg2 >> 8;
		g_Input.Joy.anKeysets[ nKeyset + NUM_KBJOY_DEVICES ][ i-- ] = (WORD)(ulKeysetReg1 & 0x000000ffL);
		ulKeysetReg1 = ulKeysetReg1 >> 8;
	}

	if( NULL == hkInput )
		RegCloseKey( hkKey );

	return !bFail;

} /* #OF# ReadRegKeyset */

/*========================================================
Function : WriteRegKeyset
=========================================================*/
/* #FN#
   */
void
/* #AS#
   Nothing */
WriteRegKeyset(
	HKEY hkInput,
	int  nKeyset
)
{
	HKEY  hkKey        = hkInput;
	ULONG ulKeysetReg1 = 0L;
	ULONG ulKeysetReg2 = 0L;
	ULONG ulKeysetReg3 = 0L;
	int   i;

	if( NULL == hkKey )
	{
		if( RegOpenKeyEx( HKEY_CURRENT_USER, REGNAME, 0, KEY_WRITE, &hkKey ) != ERROR_SUCCESS )
		{
			DisplayMessage( NULL, IDS_REG_OPEN_ERROR, 0, MB_ICONSTOP | MB_OK );
			return;
		}
	}
	/* The key values will be packed into DWORDs */
	_ASSERT(NUM_KBJOY_KEYS - 2 <= sizeof(ULONG) * 2);

	for( i = 0; i < NUM_KBJOY_KEYS - 2; )
	{
		ulKeysetReg1 = ulKeysetReg1 << 8;
		ulKeysetReg1 |= g_Input.Joy.anKeysets[ nKeyset + NUM_KBJOY_DEVICES ][ i++ ] & 0x000000ffL;
		ulKeysetReg2 = ulKeysetReg2 << 8;
		ulKeysetReg2 |= g_Input.Joy.anKeysets[ nKeyset + NUM_KBJOY_DEVICES ][ i++ ] & 0x000000ffL;
	}
	ulKeysetReg3 |= g_Input.Joy.anKeysets[ nKeyset + NUM_KBJOY_DEVICES ][ i++ ] & 0x000000ffL;
	ulKeysetReg3 = ulKeysetReg3 << 8;
	ulKeysetReg3 |= g_Input.Joy.anKeysets[ nKeyset + NUM_KBJOY_DEVICES ][ i ] & 0x000000ffL;

	if( KEYS_A_JOYSTICK == nKeyset )
	{
		_RegWriteNumber( hkKey, REG_KEYSET_A1, ulKeysetReg1 );
		_RegWriteNumber( hkKey, REG_KEYSET_A2, ulKeysetReg2 );
		_RegWriteNumber( hkKey, REG_KEYSET_A3, ulKeysetReg3 );
	}
	else
	{
		_RegWriteNumber( hkKey, REG_KEYSET_B1, ulKeysetReg1 );
		_RegWriteNumber( hkKey, REG_KEYSET_B2, ulKeysetReg2 );
		_RegWriteNumber( hkKey, REG_KEYSET_B3, ulKeysetReg3 );
	}
} /* #OF# WriteRegKeyset */

/*========================================================
Function : WriteRegDrives
=========================================================*/
/* #FN#
   */
void
/* #AS#
   Nothing */
WriteRegDrives(
	HKEY hkInput
)
{
	HKEY hkKey = hkInput;

	if( NULL == hkKey )
	{
		if( RegOpenKeyEx( HKEY_CURRENT_USER, REGNAME, 0, KEY_WRITE, &hkKey ) != ERROR_SUCCESS )
		{
			DisplayMessage( NULL, IDS_REG_OPEN_ERROR, 0, MB_ICONSTOP | MB_OK );
			return;
		}
	}
	_RegWriteString( hkKey, REG_HD1, Devices_atari_h_dir[0] );
	_RegWriteString( hkKey, REG_HD2, Devices_atari_h_dir[1] );
	_RegWriteString( hkKey, REG_HD3, Devices_atari_h_dir[2] );
	_RegWriteString( hkKey, REG_HD4, Devices_atari_h_dir[3] );
	_RegWriteString( hkKey, REG_HDE_PATH, g_szHardExePath );

	_RegWriteString( hkKey, REG_DRIVE1, SIO_filename[ 0 ] );
	_RegWriteString( hkKey, REG_DRIVE2, SIO_filename[ 1 ] );
	_RegWriteString( hkKey, REG_DRIVE3, SIO_filename[ 2 ] );
	_RegWriteString( hkKey, REG_DRIVE4, SIO_filename[ 3 ] );
	_RegWriteString( hkKey, REG_DRIVE5, SIO_filename[ 4 ] );
	_RegWriteString( hkKey, REG_DRIVE6, SIO_filename[ 5 ] );
	_RegWriteString( hkKey, REG_DRIVE7, SIO_filename[ 6 ] );
	_RegWriteString( hkKey, REG_DRIVE8, SIO_filename[ 7 ] );

	_RegWriteString( hkKey, REG_EXE_PATH, atari_exe_dir );
	_RegWriteString( hkKey, REG_FILE_STATE, atari_state_dir );

	if( NULL == hkInput )
		RegCloseKey( hkKey );

} /* #OF# WriteRegDrives */

/*========================================================
Function : WriteAtari800Registry
=========================================================*/
/* #FN#
   */
void
/* #AS#
   Nothing */
WriteAtari800Registry(
	void
)
{
	HKEY hkKey = NULL;

	if( RegOpenKeyEx( HKEY_CURRENT_USER, REGNAME, 0, KEY_ALL_ACCESS, &hkKey ) != ERROR_SUCCESS )
	{
		DisplayMessage( NULL, IDS_REG_OPEN_ERROR, 0, MB_ICONSTOP | MB_OK );
		return;
	}
	_RegWriteNumber( hkKey, REG_REFRESH_RATE,        Atari800_refresh_rate         );
	_RegWriteNumber( hkKey, REG_MACHINE_TYPE,        Atari800_machine_type         );
	_RegWriteNumber( hkKey, REG_TV_MODE,             Atari800_tv_mode              );
	_RegWriteNumber( hkKey, REG_HD_READ_ONLY,        Devices_h_read_only           );
	_RegWriteNumber( hkKey, REG_DISABLE_BASIC,       Atari800_disable_basic        );
	_RegWriteNumber( hkKey, REG_ENABLE_SIO_PATCH,    ESC_enable_sio_patch          );
	_RegWriteNumber( hkKey, REG_ENABLE_H_PATCH,      Devices_enable_h_patch        );
	_RegWriteNumber( hkKey, REG_ENABLE_P_PATCH,      Devices_enable_p_patch        );
	_RegWriteNumber( hkKey, REG_ENABLE_R_PATCH,      Devices_enable_r_patch        );
	_RegWriteNumber( hkKey, REG_ENABLE_RTIME,        RTIME_enabled                 );
	_RegWriteNumber( hkKey, REG_ENABLE_STEREO,       POKEYSND_stereo_enabled       );
	_RegWriteNumber( hkKey, REG_ARTIF_MODE,          ANTIC_artif_mode              );
	_RegWriteNumber( hkKey, REG_CART_TYPE,           CARTRIDGE_type                );
	_RegWriteString( hkKey, REG_ROM_OSA,             CFG_osa_filename              );
	_RegWriteString( hkKey, REG_ROM_OSB,             CFG_osb_filename              );
	_RegWriteString( hkKey, REG_ROM_XLXE,            CFG_xlxe_filename             );
	_RegWriteString( hkKey, REG_ROM_5200,            CFG_5200_filename             );
	_RegWriteString( hkKey, REG_ROM_BASIC,           CFG_basic_filename            );
	_RegWriteString( hkKey, REG_FILE_TAPE,           cassette_filename             );
	_RegWriteString( hkKey, REG_PRINT_COMMAND,       Devices_print_command         );
	_RegWriteNumber( hkKey, REG_MOUSE_MODE,          INPUT_mouse_mode              );
	_RegWriteNumber( hkKey, REG_MOUSE_PORT,          INPUT_mouse_port              );
	_RegWriteNumber( hkKey, REG_MOUSE_SPEED,         INPUT_mouse_speed             );
	_RegWriteNumber( hkKey, REG_POT_MIN,             INPUT_mouse_pot_min           );
	_RegWriteNumber( hkKey, REG_POT_MAX,             INPUT_mouse_pot_max           );
	_RegWriteNumber( hkKey, REG_JOY_INERTIA,         INPUT_mouse_joy_inertia       );
	_RegWriteNumber( hkKey, REG_PEN_XOFFSET,         INPUT_mouse_pen_ofs_h         );
	_RegWriteNumber( hkKey, REG_PEN_YOFFSET,         INPUT_mouse_pen_ofs_v         );
	_RegWriteNumber( hkKey, REG_BLOCK_OPPOSITE,      INPUT_joy_block_opposite_directions );
	_RegWriteNumber( hkKey, REG_MULTI_JOY,           INPUT_joy_multijoy            );
	_RegWriteString( hkKey, REG_ROM_CURRENT,         g_szCurrentRom                );
	_RegWriteString( hkKey, REG_ROM_OTHER,           g_szOtherRom                  );
	_RegWriteString( hkKey, REG_FILE_TEMPLATE,       g_szTemplateFile              );
	_RegWriteString( hkKey, REG_FILE_PALETTE,        g_szPaletteFile               );
	_RegWriteString( hkKey, REG_FILE_AUTOBOOT,       g_szAutobootFile              );
	_RegWriteNumber( hkKey, REG_START_XPOS,          g_nStartX                     );
	_RegWriteNumber( hkKey, REG_START_YPOS,          g_nStartY                     );
	_RegWriteNumber( hkKey, REG_SCREEN_STATE,        g_Screen.ulState              );
	_RegWriteNumber( hkKey, REG_STRETCH_MODE,        g_Screen.nStretchMode         );
	_RegWriteNumber( hkKey, REG_MEMORY_TYPE,         g_Screen.nMemoryType          );
	_RegWriteNumber( hkKey, REG_REQUEST_RATE,        g_Screen.nRequestRate         );
	_RegWriteNumber( hkKey, REG_COLOR_BLACK,         g_Screen.Pal.nBlackLevel      );
	_RegWriteNumber( hkKey, REG_COLOR_WHITE,         g_Screen.Pal.nWhiteLevel      );
	_RegWriteNumber( hkKey, REG_COLOR_SATURATION,    g_Screen.Pal.nSaturation      );
	_RegWriteNumber( hkKey, REG_COLOR_CONTRAST,      g_Screen.Pal.nContrast        );
	_RegWriteNumber( hkKey, REG_COLOR_BRIGHTNESS,    g_Screen.Pal.nBrightness      );
	_RegWriteNumber( hkKey, REG_COLOR_GAMMA,         g_Screen.Pal.nGamma           );
	_RegWriteNumber( hkKey, REG_SOUND_STATE,         g_Sound.ulState               );
	_RegWriteNumber( hkKey, REG_SOUND_RATE,          g_Sound.nRate                 );
	_RegWriteNumber( hkKey, REG_SOUND_VOLUME,        g_Sound.nVolume               );
	_RegWriteNumber( hkKey, REG_SOUND_UPDATE,        g_Sound.nSkipUpdate           );
	_RegWriteNumber( hkKey, REG_SOUND_LATENCY,       g_Sound.nLatency              );
	_RegWriteNumber( hkKey, REG_SOUND_QUALITY,       g_Sound.nQuality              );
	_RegWriteNumber( hkKey, REG_SOUND_DIGITIZED,     g_Sound.nDigitized            );
	_RegWriteNumber( hkKey, REG_BIENIAS_FIX,         g_Sound.nBieniasFix           );
	_RegWriteNumber( hkKey, REG_INPUT_STATE,         g_Input.ulState               );
	_RegWriteNumber( hkKey, REG_JOYSTICKS,           g_Input.Joy.ulSelected        );
	_RegWriteNumber( hkKey, REG_AUTOFIRE_MODE,       g_Input.Joy.nAutoMode         );
	_RegWriteNumber( hkKey, REG_AUTOFIRE_STICKS,     g_Input.Joy.ulAutoSticks      );
	_RegWriteNumber( hkKey, REG_ARROWS_MODE,         g_Input.Key.nArrowsMode       );
	_RegWriteNumber( hkKey, REG_MISC_STATE,          g_Misc.ulState                );
	_RegWriteNumber( hkKey, REG_DONT_SHOW,           g_Misc.ulDontShow             );
	_RegWriteNumber( hkKey, REG_FILE_ASSOCIATIONS,   g_Misc.ulFileAssociations     );
	_RegWriteNumber( hkKey, REG_CHEAT_COLLISIONS,    g_Misc.Cheat.ulCollisions     );
	_RegWriteNumber( hkKey, REG_CHEAT_MEMO,          g_Misc.Cheat.nMemo            );
	_RegWriteNumber( hkKey, REG_CHEAT_SEARCH,        g_Misc.Cheat.nSearch          );
	_RegWriteNumber( hkKey, REG_CHEAT_LOCK,          g_Misc.Cheat.nLock            );
	_RegWriteNumber( hkKey, REG_REFRESH_DOUBLEWND,   g_Misc.Refresh.nDoubleWnd     );
	_RegWriteNumber( hkKey, REG_REFRESH_FULLSPEED,   g_Misc.Refresh.nFullSpeed     );
	_RegWriteNumber( hkKey, REG_SPEED_PERCENT,       g_Misc.nSpeedPercent          );
	_RegWriteNumber( hkKey, REG_RAMSIZE_OSA,         g_anRamSize[ Atari800_MACHINE_OSA  ]   );
	_RegWriteNumber( hkKey, REG_RAMSIZE_OSB,         g_anRamSize[ Atari800_MACHINE_OSB  ]   );
	_RegWriteNumber( hkKey, REG_RAMSIZE_XLXE,        g_anRamSize[ Atari800_MACHINE_XLXE ]   );
	_RegWriteNumber( hkKey, REG_CURRENT_REV,         s_nVersion                    );
#ifdef WIN_NETWORK_GAMES
	_RegWriteNumber( hkKey, REG_KAILLERA_STATE,      g_Kaillera.ulState            );
	_RegWriteNumber( hkKey, REG_KAILLERA_LOCAL_PORT, g_Kaillera.nLocalPort         );
	_RegWriteNumber( hkKey, REG_KAILLERA_FRAME_SKIP, g_Kaillera.nFrameSkip         );
#endif

	WriteRegDrives( hkKey );

	WriteRegKeyset( hkKey, KEYS_A_JOYSTICK );
	WriteRegKeyset( hkKey, KEYS_B_JOYSTICK );

	RegCloseKey( hkKey );

} /* #OF# WriteAtari800Registry */

/*========================================================
Function : InitialiseRegistry
=========================================================*/
/* #FN#
   */
void
/* #AS#
   Nothing */
InitialiseRegistry(
	HKEY hkInput,
	BOOL bErasePaths
)
{
	HKEY hkKey = hkInput;
	int  i;

	if( NULL == hkKey )
	{
		DWORD dwDisposition = REG_OPENED_EXISTING_KEY;
		
		if( RegOpenKeyEx( HKEY_CURRENT_USER, REGNAME, 0, KEY_ALL_ACCESS, &hkKey ) != ERROR_SUCCESS )
		{
			if( RegCreateKeyEx( HKEY_CURRENT_USER, REGNAME, 0, PLUS_REGKEY, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkKey, &dwDisposition ) != ERROR_SUCCESS )
			{
				DisplayMessage( NULL, IDS_REG_OPEN_ERROR, 0, MB_ICONSTOP | MB_OK );
				/* Probably should make this fault in a more elegant manner */
				Atari_Exit( 0 );
			}
		}
	}
	ANTIC_artif_mode              = 0;
	Atari800_machine_type         = Atari800_MACHINE_XLXE;
	Atari800_tv_mode              = Atari800_TV_PAL;
	Atari800_refresh_rate         = 1;
	Devices_h_read_only                   = 1;
	Atari800_disable_basic        = 1;
	RTIME_enabled                 = 1;
	ESC_enable_sio_patch          = 1;
	Devices_enable_h_patch        = 0;
	Devices_enable_p_patch        = 0;
	Devices_enable_r_patch        = 0;
	POKEYSND_stereo_enabled       = 0;
	CARTRIDGE_type                = CARTRIDGE_NONE;
	INPUT_joy_block_opposite_directions = 1;
	INPUT_joy_multijoy                  = 0;
	INPUT_mouse_mode              = DEF_MOUSE_MODE;
	INPUT_mouse_port                    = DEF_MOUSE_PORT;
	INPUT_mouse_speed                   = DEF_MOUSE_SPEED;
	INPUT_mouse_pot_min                 = DEF_POT_MIN;
	INPUT_mouse_pot_max                 = DEF_POT_MAX;
	INPUT_mouse_joy_inertia             = DEF_JOY_INERTIA;
	INPUT_mouse_pen_ofs_h               = DEF_PEN_OFFSET_X;
	INPUT_mouse_pen_ofs_v               = DEF_PEN_OFFSET_Y;
	g_anRamSize[ Atari800_MACHINE_OSA  ]   = DEF_RAMSIZE_OSA;
	g_anRamSize[ Atari800_MACHINE_OSB  ]   = DEF_RAMSIZE_OSB;
	g_anRamSize[ Atari800_MACHINE_XLXE ]   = DEF_RAMSIZE_XLXE;
	g_Misc.ulState                = DEF_MISC_STATE;
	g_Misc.ulDontShow             = DEF_DONT_SHOW_FLAGS;
	g_Misc.Cheat.ulCollisions     = DEF_CHEAT_COLLISIONS;
	g_Misc.Cheat.nMemo            = DEF_CHEAT_MEMO;
	g_Misc.Cheat.nSearch          = DEF_CHEAT_SEARCH;
	g_Misc.Cheat.nLock            = DEF_CHEAT_LOCK;
	g_Misc.Refresh.nDoubleWnd     = DEF_REFRESH_DOUBLEWND;
	g_Misc.Refresh.nFullSpeed     = DEF_REFRESH_FULLSPEED;
	g_Misc.nSpeedPercent          = DEF_SPEED_PERCENT;
	g_Sound.ulState               = DEF_SOUND_STATE;
	g_Sound.nRate                 = DEF_SOUND_RATE;
	g_Sound.nVolume               = DEF_SOUND_VOL;
	g_Sound.nSkipUpdate           = DEF_SKIP_UPDATE;
	g_Sound.nLatency              = DEF_SOUND_LATENCY;
	g_Sound.nQuality              = DEF_SOUND_QUALITY;
	g_Sound.nDigitized            = DEF_SOUND_DIGITIZED;
	g_Sound.nBieniasFix           = 0;
	g_Input.ulState               = DEF_INPUT_STATE;
	g_Input.Joy.ulSelected        = DEF_JOY_SELECTS;
	g_Input.Joy.nAutoMode         = DEF_AUTOFIRE_MODE;
	g_Input.Joy.ulAutoSticks      = DEF_AUTOFIRE_STICKS;
	g_Input.Key.nArrowsMode       = DEF_ARROWS_MODE;
	g_Screen.ulState              = DEF_SCREEN_STATE;
	g_Screen.nStretchMode         = DEF_STRETCH_MODE;
	g_Screen.nMemoryType          = DEF_MEMORY_TYPE;
	g_Screen.Pal.nBlackLevel      = DEF_CLR_BLACK_LEVEL;
	g_Screen.Pal.nWhiteLevel      = DEF_CLR_WHITE_LEVEL;
	g_Screen.Pal.nSaturation      = DEF_CLR_SATURATION;
	g_Screen.Pal.nContrast        = DEF_CLR_CONTRAST;
	g_Screen.Pal.nBrightness      = DEF_CLR_BRIGHTNESS;
	g_Screen.Pal.nGamma           = DEF_CLR_GAMMA;
#ifdef WIN_NETWORK_GAMES
	g_Kaillera.ulState            = DEF_KAILLERA_STATE;
	g_Kaillera.nLocalPort         = DEF_KAILLERA_LOCAL_PORT;
	g_Kaillera.nFrameSkip         = DEF_KAILLERA_FRAME_SKIP;
#endif

	/* Clear keysets A & B */
	for( i = 0; i < NUM_KBJOY_KEYS; i++ )
	{
		g_Input.Joy.anKeysets[ KEYS_A_JOYSTICK + NUM_KBJOY_DEVICES ][ i ] = 0;
		g_Input.Joy.anKeysets[ KEYS_B_JOYSTICK + NUM_KBJOY_DEVICES ][ i ] = 0;
	}
	
	if( bErasePaths || '\0' == *CFG_osa_filename ) /* WinNT wants this */
		strcpy( CFG_osa_filename, DEFAULT_OSA );
	if( bErasePaths || '\0' == *CFG_osb_filename )
		strcpy( CFG_osb_filename, DEFAULT_OSB );
	if( bErasePaths || '\0' == *CFG_xlxe_filename )
		strcpy( CFG_xlxe_filename, DEFAULT_OXL );
	if( bErasePaths || '\0' == *CFG_5200_filename )
		strcpy( CFG_5200_filename, DEFAULT_O52 );
	if( bErasePaths || '\0' == *CFG_basic_filename )
		strcpy( CFG_basic_filename, DEFAULT_BAS );

	if( bErasePaths || '\0' == *g_szTemplateFile )
		strcpy( g_szTemplateFile, DEFAULT_A8K );
	if( bErasePaths || '\0' == *g_szPaletteFile )
		strcpy( g_szPaletteFile, DEFAULT_ACT );
	if( bErasePaths || '\0' == *atari_state_dir )
		strcpy( atari_state_dir, DEFAULT_A8S );
	if( bErasePaths || '\0' == *cassette_filename )
		strcpy( cassette_filename, DEFAULT_TAP );
	if( bErasePaths || '\0' == *g_szAutobootFile )
		strcpy( g_szAutobootFile, FILE_NONE );

	if( bErasePaths || '\0' == *Devices_atari_h_dir[0] )
		strcpy( Devices_atari_h_dir[0], DEFAULT_HDD );
	if( bErasePaths || '\0' == *Devices_atari_h_dir[1] )
		strcpy( Devices_atari_h_dir[1], DEFAULT_HDD );
	if( bErasePaths || '\0' == *Devices_atari_h_dir[2] )
		strcpy( Devices_atari_h_dir[2], DEFAULT_HDD );
	if( bErasePaths || '\0' == *Devices_atari_h_dir[3] )
		strcpy( Devices_atari_h_dir[3], DEFAULT_HDD );

	if( bErasePaths || '\0' == *g_szHardExePath )
		strcpy( g_szHardExePath, DEFAULT_H_PATH );

	if( bErasePaths || '\0' == *atari_exe_dir )
		strcpy( atari_exe_dir, PATH_NONE );

	strcpy( g_szCurrentRom, FILE_NONE );
	strcpy( g_szOtherRom,   FILE_NONE );

	strcpy( Devices_print_command, DEF_PRINT_COMMAND );

	for( i = 0; i < SIO_MAX_DRIVES; i++ )
	{
		strcpy( SIO_filename[ i ], "Off" ) ;
		SIO_drive_status[ i ] = SIO_OFF;
	}

	WriteAtari800Registry();

	if( NULL == hkInput )
		RegCloseKey( hkKey );

} /* #OF# InitialiseRegistry */

/*========================================================
Function : ReadRegPaths
=========================================================*/
/* #FN#
   */
BOOL
/* #AS#
   */
ReadRegPaths(
	void
)
{
	HKEY hkKey = NULL;
	BOOL bFail = FALSE;

	if( RegOpenKeyEx( HKEY_CURRENT_USER, REGNAME, 0, KEY_READ, &hkKey ) != ERROR_SUCCESS )
	{
		DisplayMessage( NULL, IDS_REG_OPEN_ERROR, 0, MB_ICONSTOP | MB_OK );
		return FALSE;
	}
	bFail |= _RegReadString( hkKey, REG_ROM_BASIC,     CFG_basic_filename, DEFAULT_BAS,    MAX_PATH );
	bFail |= _RegReadString( hkKey, REG_ROM_OSA,       CFG_osa_filename,   DEFAULT_OSA,    MAX_PATH );
	bFail |= _RegReadString( hkKey, REG_ROM_OSB,       CFG_osb_filename,   DEFAULT_OSB,    MAX_PATH );
	bFail |= _RegReadString( hkKey, REG_ROM_XLXE,      CFG_xlxe_filename,  DEFAULT_OXL,    MAX_PATH );
	bFail |= _RegReadString( hkKey, REG_ROM_5200,      CFG_5200_filename,  DEFAULT_O52,    MAX_PATH );
	bFail |= _RegReadString( hkKey, REG_FILE_TAPE,     cassette_filename,  DEFAULT_TAP,    FILENAME_MAX - 1 );
	bFail |= _RegReadString( hkKey, REG_FILE_STATE,    atari_state_dir,    DEFAULT_A8S,    MAX_PATH );
	bFail |= _RegReadString( hkKey, REG_EXE_PATH,      atari_exe_dir,      PATH_NONE,      MAX_PATH );
	bFail |= _RegReadString( hkKey, REG_HD1,           Devices_atari_h_dir[0],     DEFAULT_HDD,    MAX_PATH );
	bFail |= _RegReadString( hkKey, REG_HD2,           Devices_atari_h_dir[1],     DEFAULT_HDD,    MAX_PATH );
	bFail |= _RegReadString( hkKey, REG_HD3,           Devices_atari_h_dir[2],     DEFAULT_HDD,    MAX_PATH );
	bFail |= _RegReadString( hkKey, REG_HD4,           Devices_atari_h_dir[3],     DEFAULT_HDD,    MAX_PATH );
	bFail |= _RegReadString( hkKey, REG_HDE_PATH,      g_szHardExePath,    DEFAULT_H_PATH, FILENAME_MAX - 1 );
	bFail |= _RegReadString( hkKey, REG_FILE_TEMPLATE, g_szTemplateFile,   DEFAULT_A8K,    MAX_PATH );
	bFail |= _RegReadString( hkKey, REG_FILE_PALETTE,  g_szPaletteFile,    DEFAULT_ACT,    MAX_PATH );
	bFail |= _RegReadString( hkKey, REG_FILE_AUTOBOOT, g_szAutobootFile,   FILE_NONE,      MAX_PATH );
	bFail |= _RegReadString( hkKey, REG_ROM_CURRENT,   g_szCurrentRom,     FILE_NONE,      MAX_PATH );
	bFail |= _RegReadString( hkKey, REG_ROM_OTHER,     g_szOtherRom,       FILE_NONE,      MAX_PATH );

	RegCloseKey( hkKey );

	return !bFail;

} /* #OF# ReadRegPaths */

/*========================================================
Function : ReadRegDrives
=========================================================*/
/* #FN#
   */
void
/* #AS#
   Nothing */
ReadRegDrives(
	void
)
{
	HKEY hkKey = NULL;
	BOOL bFail = FALSE;
	int  i;
	char szFileName[ MAX_PATH ];

	char *reg_drives[] = { REG_DRIVE1, REG_DRIVE2, REG_DRIVE3, REG_DRIVE4,
						  REG_DRIVE5, REG_DRIVE6, REG_DRIVE7, REG_DRIVE8};

	if( RegOpenKeyEx( HKEY_CURRENT_USER, REGNAME, 0, KEY_READ, &hkKey ) != ERROR_SUCCESS )
	{
		DisplayMessage( NULL, IDS_REG_OPEN_ERROR, 0, MB_ICONSTOP | MB_OK );
		return;
	}

	for( i = 0; i < SIO_MAX_DRIVES; i++ )
	{
		if ( SIO_drive_status[ i ] ==  SIO_OFF || SIO_drive_status[ i ] == SIO_NO_DISK ) {
			bFail |= _RegReadString( hkKey, reg_drives[i], SIO_filename[ i ], "Off", MAX_PATH );

			if( *SIO_filename[ i ] == '\0' )
				strcpy( SIO_filename[ i ], "Off" );

			if( strcmp( SIO_filename[ i ], "Off" ) == 0 )
				SIO_drive_status[ i ] = SIO_OFF;
			else if( strcmp( SIO_filename[ i ], "Empty" ) == 0 )
					SIO_drive_status[ i ] = SIO_NO_DISK;
			else {
				strcpy( szFileName, SIO_filename[ i ] );
				if( !SIO_Mount( i + 1, szFileName, _IsFlagSet(g_Misc.ulState, MS_DRIVE_READONLY) ) )
					Log_print( "Disk File %s not found", SIO_filename[ i ] );
			}
		}
	}

	RegCloseKey( hkKey );

	if( bFail )
		WriteAtari800Registry();

} /* #OF# ReadRegDrives */

/*========================================================
Function : HandleRegistry
=========================================================*/
/* #FN#
   Creates the Registry entries if they don't exist and read all
   the defaults in at runtime (this is called from MainFrame) */
BOOL
/* #AS#
   TRUE if the Registry has been initialized, otherwise FALSE */
HandleRegistry( void )
{
	HKEY  hkKey         = NULL;
	BOOL  bFail         = FALSE;
	DWORD dwDisposition = REG_OPENED_EXISTING_KEY;
	BOOL  bInitialReg   = FALSE;

	if( RegOpenKeyEx( HKEY_CURRENT_USER, REGNAME, 0, KEY_ALL_ACCESS, &hkKey ) != ERROR_SUCCESS )
	{
		if( RegCreateKeyEx( HKEY_CURRENT_USER, REGNAME, 0, PLUS_REGKEY, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkKey, &dwDisposition ) != ERROR_SUCCESS )
		{
			DisplayMessage( NULL, IDS_REG_OPEN_ERROR, 0, MB_ICONSTOP | MB_OK );
			/* Probably should make this fault in a more elegant manner */
			Atari_Exit( 0 );
		}
	}
	/* If the key doesn't exist, fill in defaults. This is the only time these will
	   be written all at once. From here on out we trust the Registry to hold them
	   (yeah, right) and update when key dialogs are used to configure them */
	if( dwDisposition == REG_CREATED_NEW_KEY )
	{
		bInitialReg = TRUE;
		InitialiseRegistry( hkKey, !Misc_TestRomPaths( NULL, NULL ) );
	}
	else
	{
		/* Read in the values from the Registry. Only fail and return error when it is
		   REALLY fatal (you never know what somebody might do with their registry)
		   For most of these an error will result in the value being the default run-time */
		if( ReadRegDWORD( hkKey, REG_CURRENT_REV, (DWORD *)&s_nVersion, FALSE ) == READ_REG_FAIL )
			s_nVersion = CURRENT_REV - 1;
		
		if( s_nVersion != CURRENT_REV )
		{
			DisplayMessage( NULL, IDS_WARN_OUTDATED, 0, MB_ICONINFORMATION | MB_OK );
			s_nVersion = CURRENT_REV;

			ReadRegPaths();	/* Since we already have a registry, read the paths + filenames at least */
									/* Note that this will have to change if I ever invalidate the path system (unlikely) */
			DeleteAllRegKeys( hkKey, REGNAME );

			RegCloseKey( hkKey );
			if( RegCreateKeyEx( HKEY_CURRENT_USER, REGNAME, 0, PLUS_REGKEY, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkKey, &dwDisposition ) != ERROR_SUCCESS )
			{
				DisplayMessage( NULL, IDS_REG_OPEN_ERROR, 0, MB_ICONSTOP | MB_OK );
				/* Probably should make this fault in a more elegant manner */
				Atari_Exit( 0 );
			}
			InitialiseRegistry( hkKey, FALSE );
			bInitialReg = TRUE;
		}
		else
		{
			bFail |= _RegReadString( hkKey, REG_PRINT_COMMAND,       Devices_print_command,         DEF_PRINT_COMMAND, PRINT_CMD_LENGTH );
			bFail |= _RegReadNumber( hkKey, REG_MACHINE_TYPE,        Atari800_machine_type,         Atari800_MACHINE_XLXE            );
			bFail |= _RegReadNumber( hkKey, REG_TV_MODE,             Atari800_tv_mode,              Atari800_TV_PAL         );
			bFail |= _RegReadNumber( hkKey, REG_CART_TYPE,           CARTRIDGE_type,                CARTRIDGE_NONE          );
			bFail |= _RegReadNumber( hkKey, REG_DISABLE_BASIC,       Atari800_disable_basic,        1                       );
			bFail |= _RegReadNumber( hkKey, REG_REFRESH_RATE,        Atari800_refresh_rate,         1                       );
			bFail |= _RegReadNumber( hkKey, REG_HD_READ_ONLY,        Devices_h_read_only,           1                       );
			bFail |= _RegReadNumber( hkKey, REG_ENABLE_RTIME,        RTIME_enabled,                 1                       );
			bFail |= _RegReadNumber( hkKey, REG_ENABLE_SIO_PATCH,    ESC_enable_sio_patch,          1                       );
			bFail |= _RegReadNumber( hkKey, REG_ENABLE_H_PATCH,      Devices_enable_h_patch,        0                       );
			bFail |= _RegReadNumber( hkKey, REG_ENABLE_P_PATCH,      Devices_enable_p_patch,        0                       );
			bFail |= _RegReadNumber( hkKey, REG_ENABLE_R_PATCH,      Devices_enable_r_patch,        0                       );
			bFail |= _RegReadNumber( hkKey, REG_ENABLE_STEREO,       POKEYSND_stereo_enabled,       0                       );
			bFail |= _RegReadNumber( hkKey, REG_ARTIF_MODE,          ANTIC_artif_mode,              0                       );
			bFail |= _RegReadNumber( hkKey, REG_BLOCK_OPPOSITE,      INPUT_joy_block_opposite_directions, 1                       );
			bFail |= _RegReadNumber( hkKey, REG_MULTI_JOY,           INPUT_joy_multijoy,            0                       );
			bFail |= _RegReadNumber( hkKey, REG_MOUSE_MODE,          INPUT_mouse_mode,              DEF_MOUSE_MODE          );
			bFail |= _RegReadNumber( hkKey, REG_MOUSE_PORT,          INPUT_mouse_port,              DEF_MOUSE_PORT          );
			bFail |= _RegReadNumber( hkKey, REG_MOUSE_SPEED,         INPUT_mouse_speed,             DEF_MOUSE_SPEED         );
			bFail |= _RegReadNumber( hkKey, REG_POT_MIN,             INPUT_mouse_pot_min,           DEF_POT_MIN             );
			bFail |= _RegReadNumber( hkKey, REG_POT_MAX,             INPUT_mouse_pot_max,           DEF_POT_MAX             );
			bFail |= _RegReadNumber( hkKey, REG_JOY_INERTIA,         INPUT_mouse_joy_inertia,       DEF_JOY_INERTIA         );
			bFail |= _RegReadNumber( hkKey, REG_PEN_XOFFSET,         INPUT_mouse_pen_ofs_h,         DEF_PEN_OFFSET_X        );
			bFail |= _RegReadNumber( hkKey, REG_PEN_YOFFSET,         INPUT_mouse_pen_ofs_v,         DEF_PEN_OFFSET_Y        );
			bFail |= _RegReadNumber( hkKey, REG_RAMSIZE_OSA,         g_anRamSize[ Atari800_MACHINE_OSA ],    DEF_RAMSIZE_OSA         );
			bFail |= _RegReadNumber( hkKey, REG_RAMSIZE_OSB,         g_anRamSize[ Atari800_MACHINE_OSB ],    DEF_RAMSIZE_OSB         );
			bFail |= _RegReadNumber( hkKey, REG_RAMSIZE_XLXE,        g_anRamSize[ Atari800_MACHINE_XLXE ],   DEF_RAMSIZE_XLXE        );
			bFail |= _RegReadNumber( hkKey, REG_ARROWS_MODE,         g_Input.Key.nArrowsMode,       DEF_ARROWS_MODE         );
			bFail |= _RegReadNumber( hkKey, REG_SCREEN_STATE,        g_Screen.ulState,              DEF_SCREEN_STATE        );
			bFail |= _RegReadNumber( hkKey, REG_STRETCH_MODE,        g_Screen.nStretchMode,         DEF_STRETCH_MODE        );
			bFail |= _RegReadNumber( hkKey, REG_MEMORY_TYPE,         g_Screen.nMemoryType,          DEF_MEMORY_TYPE         );
			bFail |= _RegReadNumber( hkKey, REG_REQUEST_RATE,        g_Screen.nRequestRate,         DEF_REQUEST_RATE        );
			bFail |= _RegReadNumber( hkKey, REG_COLOR_BLACK,         g_Screen.Pal.nBlackLevel,      DEF_CLR_BLACK_LEVEL     );
			bFail |= _RegReadNumber( hkKey, REG_COLOR_WHITE,         g_Screen.Pal.nWhiteLevel,      DEF_CLR_WHITE_LEVEL     );
			bFail |= _RegReadNumber( hkKey, REG_COLOR_SATURATION,    g_Screen.Pal.nSaturation,      DEF_CLR_SATURATION      );
			bFail |= _RegReadNumber( hkKey, REG_COLOR_CONTRAST,      g_Screen.Pal.nContrast,        DEF_CLR_CONTRAST        );
			bFail |= _RegReadNumber( hkKey, REG_COLOR_BRIGHTNESS,    g_Screen.Pal.nBrightness,      DEF_CLR_BRIGHTNESS      );
			bFail |= _RegReadNumber( hkKey, REG_COLOR_GAMMA,         g_Screen.Pal.nGamma,           DEF_CLR_GAMMA           );
			bFail |= _RegReadNumber( hkKey, REG_SOUND_STATE,         g_Sound.ulState,               DEF_SOUND_STATE         );
			bFail |= _RegReadNumber( hkKey, REG_SOUND_RATE,          g_Sound.nRate,                 DEF_SOUND_RATE          );
			bFail |= _RegReadNumber( hkKey, REG_SOUND_VOLUME,        g_Sound.nVolume,               DEF_SOUND_VOL           );
			bFail |= _RegReadNumber( hkKey, REG_SOUND_UPDATE,        g_Sound.nSkipUpdate,           DEF_SKIP_UPDATE         );
			bFail |= _RegReadNumber( hkKey, REG_SOUND_LATENCY,       g_Sound.nLatency,              DEF_SOUND_LATENCY       );
			bFail |= _RegReadNumber( hkKey, REG_SOUND_QUALITY,       g_Sound.nQuality,              DEF_SOUND_QUALITY       );
			bFail |= _RegReadNumber( hkKey, REG_SOUND_DIGITIZED,     g_Sound.nDigitized,            DEF_SOUND_DIGITIZED     );
			bFail |= _RegReadNumber( hkKey, REG_BIENIAS_FIX,         g_Sound.nBieniasFix,           0                       );
			bFail |= _RegReadNumber( hkKey, REG_INPUT_STATE,         g_Input.ulState,               DEF_INPUT_STATE         );
			bFail |= _RegReadNumber( hkKey, REG_AUTOFIRE_MODE,       g_Input.Joy.nAutoMode,         DEF_DONT_SHOW_FLAGS     );
			bFail |= _RegReadNumber( hkKey, REG_AUTOFIRE_STICKS,     g_Input.Joy.ulAutoSticks,      DEF_AUTOFIRE_STICKS     );
			bFail |= _RegReadNumber( hkKey, REG_JOYSTICKS,           g_Input.Joy.ulSelected,        DEF_JOY_SELECTS         );
			bFail |= _RegReadNumber( hkKey, REG_MISC_STATE,          g_Misc.ulState,                DEF_MISC_STATE          );
			bFail |= _RegReadNumber( hkKey, REG_DONT_SHOW,           g_Misc.ulDontShow,             DEF_DONT_SHOW_FLAGS     );
			bFail |= _RegReadNumber( hkKey, REG_FILE_ASSOCIATIONS,   g_Misc.ulFileAssociations,     DEF_FILE_ASSOCIATIONS   );
			bFail |= _RegReadNumber( hkKey, REG_CHEAT_COLLISIONS,    g_Misc.Cheat.ulCollisions,     DEF_CHEAT_COLLISIONS    );
			bFail |= _RegReadNumber( hkKey, REG_CHEAT_MEMO,          g_Misc.Cheat.nMemo,            DEF_CHEAT_MEMO          );
			bFail |= _RegReadNumber( hkKey, REG_CHEAT_SEARCH,        g_Misc.Cheat.nSearch,          DEF_CHEAT_SEARCH        );
			bFail |= _RegReadNumber( hkKey, REG_CHEAT_LOCK,          g_Misc.Cheat.nLock,            DEF_CHEAT_LOCK          );
			bFail |= _RegReadNumber( hkKey, REG_REFRESH_DOUBLEWND,   g_Misc.Refresh.nDoubleWnd,     DEF_REFRESH_DOUBLEWND   );
			bFail |= _RegReadNumber( hkKey, REG_REFRESH_FULLSPEED,   g_Misc.Refresh.nFullSpeed,     DEF_REFRESH_FULLSPEED   );
			bFail |= _RegReadNumber( hkKey, REG_SPEED_PERCENT,       g_Misc.nSpeedPercent,          DEF_SPEED_PERCENT       );
#ifdef WIN_NETWORK_GAMES
			bFail |= _RegReadNumber( hkKey, REG_KAILLERA_STATE,      g_Kaillera.ulState,            DEF_KAILLERA_STATE      );
			bFail |= _RegReadNumber( hkKey, REG_KAILLERA_LOCAL_PORT, g_Kaillera.nLocalPort,         DEF_KAILLERA_LOCAL_PORT );
			bFail |= _RegReadNumber( hkKey, REG_KAILLERA_FRAME_SKIP, g_Kaillera.nFrameSkip,         DEF_KAILLERA_FRAME_SKIP );
			
#endif
			bFail |= _RegReadNumber( hkKey, REG_START_XPOS,          g_nStartX,                     DEF_START_X             );
			bFail |= _RegReadNumber( hkKey, REG_START_YPOS,          g_nStartY,                     DEF_START_Y             );

			if( g_nStartX > GetSystemMetrics( SM_CXFULLSCREEN ) )
				g_nStartX = GetSystemMetrics( SM_CXFULLSCREEN ) - (_IsFlagSet( g_Screen.ulState, SM_WRES_DOUBLE ) ? ATARI_DOUBLE_VIS_WIDTH : ATARI_VIS_WIDTH);
			if( g_nStartX < 0 )
				g_nStartX = 0;
			
			if( g_nStartY > GetSystemMetrics( SM_CYFULLSCREEN ) )
				g_nStartY = GetSystemMetrics( SM_CYFULLSCREEN ) - (_IsFlagSet( g_Screen.ulState, SM_WRES_DOUBLE ) ? ATARI_DOUBLE_HEIGHT : Screen_HEIGHT);
			if( g_nStartY < 0 )
				g_nStartY = 0;

			if( 22050 == g_Sound.nRate )
				_ClrFlag( g_Sound.ulState, SS_CUSTOM_RATE );

			if( !ReadRegPaths() )
				bFail = TRUE;

			if( !ReadRegKeyset( hkKey, KEYS_A_JOYSTICK ) )
				bFail = TRUE;
			if( !ReadRegKeyset( hkKey, KEYS_B_JOYSTICK ) )
				bFail = TRUE;

			Screen_show_disk_led = _IsFlagSet( g_Misc.ulState, MS_SHOW_DRIVE_LED ) ? 1 : 0;
			Screen_show_sector_counter = _IsFlagSet( g_Misc.ulState, MS_SHOW_SECTOR_COUNTER ) ? 1 : 0;
		}
	}
	RegCloseKey( hkKey );

	if( bFail )
		WriteAtari800Registry();

	return bInitialReg;

} /* #OF# HandleRegistry */

/*========================================================
Function : GetRegKeyHandle
=========================================================*/
/* #FN#
   Opens/Creates registry key pszKeyName in HKEY_CLASSES_ROOT section */
HKEY
/* #AS#
   Opened/Created registry key handle */
GetRegKeyHandle(
	LPCSTR pszKeyName, /* #IN# Name of registry key */
	BOOL   bCreateKey  /* #IN# Create the specified key if the key does not exist in the registry */ )
{
	DWORD dwDisposition = REG_OPENED_EXISTING_KEY;
	HKEY  hkKey         = NULL;
	BOOL  bFail         = TRUE;

	if( RegOpenKeyEx( HKEY_CLASSES_ROOT, pszKeyName, 0, KEY_ALL_ACCESS, &hkKey ) != ERROR_SUCCESS )
	{
		if( bCreateKey )
			if( RegCreateKeyEx(
					HKEY_CLASSES_ROOT,			/* Handle of an open key               */
					pszKeyName,					/* Address of subkey name              */
					0,							/* Reserved                            */
					PLUS_REGKEY,				/* Address of class string             */
					REG_OPTION_NON_VOLATILE,	/* Special options flag                */
					KEY_ALL_ACCESS,				/* Desired security access             */
					NULL,						/* Address of key security structure   */
					&hkKey,						/* Address of buffer for opened handle */  
					&dwDisposition 				/* Address of disposition value buffer */
				) != ERROR_SUCCESS )
			{
				DisplayMessage( NULL, IDS_REG_OPEN_ERROR, 0, MB_ICONSTOP | MB_OK );
			}
			else
				bFail = FALSE;
	}
	else
		bFail = FALSE;

	return (bFail ? NULL : hkKey);

} /* #OF# GetRegKeyHandle */
