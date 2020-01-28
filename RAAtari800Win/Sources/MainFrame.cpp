/****************************************************************************
File    : MainFrame.cpp
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# CMainFrame implementation file
@(#) #BY# Tomasz Szymankowski, Richard Lawrence
@(#) #LM# 15.10.2003
*/

#include "StdAfx.h"
#include "Atari800Win.h"
#include "Atari800WinDoc.h"
#include "Atari800WinView.h"
#include "Helpers.h"
#include "FileService.h"
#include "DriveDlg.h"
#include "TapeDlg.h"
#include "SettingsDlg.h"
#include "FileAssociationsBase.h"
#include "FileAssociationsDlg.h"
#include "PerformanceDlg.h"
#include "RomImagesBase.h"
#include "RomImagesDlg.h"
#include "GraphicsDlg.h"
#include "SoundDlg.h"
#include "HarddiskDlg.h"
#include "KeyboardDlg.h"
#include "KeyTemplateDlg.h"
#include "JoystickDlg.h"
#include "MouseDlg.h"
#include "ErrorLogDlg.h"
#include "PaletteDlg.h"
#include "CheatDlg.h"
#include "WizardDlg.h"
#include "WizardStep1.h"
#include "WizardStep2.h"
#include "WizardStep3.h"
#include "ConvertTypeDlg.h"
#include "BootTypeDlg.h"
#include "AboutDlg.h"
#include "MainFrame.h"
#include "cfg.h"
#include "RetroAchievements.h"

#ifdef WIN_NETWORK_GAMES
#include "KailleraDlg.h"
#endif


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define ID_FILE_ATTACHDISKIMAGE_BASE	ID_FILE_ATTACHDISKIMAGE_DRIVE1
#define ID_FILE_DETACHDISKIMAGE_BASE	ID_FILE_DETACHDISKIMAGE_DRIVE1
#define ID_ATARI_MACHINETYPE_BASE		ID_ATARI_MACHINETYPE_OSA
#define ID_ATARI_MEMORYSIZE_BASE		ID_ATARI_MEMORYSIZE_16KB
#define ID_ATARI_VIDEOSYSTEM_BASE		ID_ATARI_VIDEOSYSTEM_PAL
#define ID_VIEW_STRETCHMODE_BASE		ID_VIEW_STRETCHMODE_PIXELDOUBLING
#define ID_VIEW_ARTIFACTING_BASE		ID_VIEW_ARTIFACTING_NONE
#define ID_INPUT_AUTOFIRE_BASE			ID_INPUT_AUTOFIRE_OFF
#define ID_INPUT_MOUSEDEVICE_BASE		ID_INPUT_MOUSEDEVICE_NONE
#define ID_INPUT_ARROWKEYS_BASE			ID_INPUT_ARROWKEYS_CONTROLARROWS

#define LABEL_FAILURE		0
#define LABEL_STOPPED		1
#define LABEL_NOTRUN		2
#define LABEL_MONITOR		3
#define LABEL_PAUSED		4
#define LABEL_JOYKEY		5
#define LABEL_SPEED			6


WORD s_aShowMenuBarCmd[] =
{
	ID_FILE_BOOTATARIIMAGE,
	ID_FILE_LOADEXECUTABLE,
	ID_FILE_ATTACHDISKIMAGE_DRIVE1,
	ID_FILE_ATTACHDISKIMAGE_DRIVE2,
	ID_FILE_ATTACHDISKIMAGE_DRIVE3,
	ID_FILE_ATTACHDISKIMAGE_DRIVE4,
	ID_FILE_ATTACHDISKIMAGE_DRIVE5,
	ID_FILE_ATTACHDISKIMAGE_DRIVE6,
	ID_FILE_ATTACHDISKIMAGE_DRIVE7,
	ID_FILE_ATTACHDISKIMAGE_DRIVE8,
	ID_FILE_ATTACHCARTRIDGEIMAGE,
	ID_FILE_ATTACHTAPEIMAGE,
	ID_FILE_READSTATE,
	ID_FILE_SAVESTATE_NORMAL,
	ID_FILE_SAVESTATE_VERBOSE,
	ID_ATARI_SETTINGS,
	ID_ATARI_PERFORMANCE,
	ID_ATARI_ROMIMAGES,
	ID_ATARI_DISKDRIVES,
	ID_ATARI_TAPECONTROL,
	ID_ATARI_HARDDISKS,
	ID_VIEW_GRAPHICSOPTIONS,
	ID_VIEW_PALETTE,
	ID_VIEW_SAVESCREENSHOT_NORMAL,
	ID_VIEW_SAVESCREENSHOT_INTERLACED,
	ID_VIEW_SAVEVIDEO,
	ID_VIEW_SAVEVIDEOANDSOUND,
	ID_VIEW_PERFORMANCETEST,
	ID_SOUND_SOUNDOPTIONS,
	ID_SOUND_PERFORMANCETEST,
	ID_SOUND_SAVESOUND,
	ID_INPUT_JOYSTICKS,
	ID_INPUT_MOUSE,
	ID_INPUT_KEYBOARD,
	ID_INPUT_NETWORKGAME,
	ID_MISC_FILEASSOCIATIONS,
	ID_MISC_CONVERT_ROMTOCART,
	ID_MISC_CONVERT_DCMTOATR,
	ID_MISC_CONVERT_XFDTOATR,
	ID_MISC_CONVERT_ATASCIITOASCII,
	ID_MISC_CONVERT_ASCIITOATASCII,
	ID_MISC_CHEATOPTIONS,
	ID_MISC_VIEWLOGFILE,
	ID_FILE_PRINT,
	ID_FILE_PRINT_DIRECT,
	ID_FILE_PRINT_PREVIEW,
//	ID_FILE_PRINT_SETUP,
	ID_HELP_SETUPWIZARD,
	ID_APP_ABOUT
};

const int s_nShowMenuBarCmdNo = sizeof(s_aShowMenuBarCmd)/sizeof(s_aShowMenuBarCmd[0]);

CAtari800WinApp *CMainFrame::m_pMainApp = NULL;

#ifdef WIN_NETWORK_GAMES
static struct
{
	BOOL  bLocked;
	ULONG ulMiscState;
	BOOL  bHoldStart;
	int   nCartType;
	char  szSnapFile[ MAX_PATH + 1 ];
	char  szCartFile[ MAX_PATH + 1 ];
	char  szExecFile[ MAX_PATH + 1 ];
	char  szDiskFile[ MAX_PATH + 1 ];
} s_Settings;

/* Some options have to be turn off when network game is active */
#define KA_MS_FLAGS_TO_CLEAR	(MS_STOP_WHEN_NO_FOCUS | MS_DISABLE_COLLISIONS | MS_REUSE_WINDOW | MS_NO_DRAW_DISPLAY)

#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame window

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_COMMAND(ID_FILE_BOOTATARIIMAGE, OnFileBootAtariImage)
	ON_COMMAND(ID_FILE_LOADEXECUTABLE, OnFileLoadExecutable)
	ON_COMMAND(ID_FILE_ATTACHDISKIMAGE_DRIVE1, OnFileAttachDiskImageDrive1)
	ON_COMMAND(ID_FILE_ATTACHDISKIMAGE_DRIVE2, OnFileAttachDiskImageDrive2)
	ON_COMMAND(ID_FILE_ATTACHDISKIMAGE_DRIVE3, OnFileAttachDiskImageDrive3)
	ON_COMMAND(ID_FILE_ATTACHDISKIMAGE_DRIVE4, OnFileAttachDiskImageDrive4)
	ON_COMMAND(ID_FILE_ATTACHDISKIMAGE_DRIVE5, OnFileAttachDiskImageDrive5)
	ON_COMMAND(ID_FILE_ATTACHDISKIMAGE_DRIVE6, OnFileAttachDiskImageDrive6)
	ON_COMMAND(ID_FILE_ATTACHDISKIMAGE_DRIVE7, OnFileAttachDiskImageDrive7)
	ON_COMMAND(ID_FILE_ATTACHDISKIMAGE_DRIVE8, OnFileAttachDiskImageDrive8)
	ON_COMMAND(ID_FILE_DETACHDISKIMAGE_ALL, OnFileDetachDiskImageAll)
	ON_COMMAND(ID_FILE_DETACHDISKIMAGE_DRIVE1, OnFileDetachDiskImageDrive1)
	ON_COMMAND(ID_FILE_DETACHDISKIMAGE_DRIVE2, OnFileDetachDiskImageDrive2)
	ON_COMMAND(ID_FILE_DETACHDISKIMAGE_DRIVE3, OnFileDetachDiskImageDrive3)
	ON_COMMAND(ID_FILE_DETACHDISKIMAGE_DRIVE4, OnFileDetachDiskImageDrive4)
	ON_COMMAND(ID_FILE_DETACHDISKIMAGE_DRIVE5, OnFileDetachDiskImageDrive5)
	ON_COMMAND(ID_FILE_DETACHDISKIMAGE_DRIVE6, OnFileDetachDiskImageDrive6)
	ON_COMMAND(ID_FILE_DETACHDISKIMAGE_DRIVE7, OnFileDetachDiskImageDrive7)
	ON_COMMAND(ID_FILE_DETACHDISKIMAGE_DRIVE8, OnFileDetachDiskImageDrive8)
	ON_COMMAND(ID_FILE_TURNDISK, OnFileTurnDisk)
	ON_COMMAND(ID_FILE_ATTACHTAPEIMAGE, OnFileAttachTapeImage)
	ON_COMMAND(ID_FILE_DETACHTAPEIMAGE, OnFileDetachTapeImage)
	ON_COMMAND(ID_FILE_ATTACHCARTRIDGEIMAGE, OnFileAttachCartridgeImage)
	ON_COMMAND(ID_FILE_DETACHCARTRIDGEIMAGE, OnFileDetachCartridgeImage)
	ON_COMMAND(ID_FILE_READSTATE, OnFileReadState)
	ON_COMMAND(ID_FILE_SAVESTATE_NORMAL, OnFileSaveStateNormal)
	ON_COMMAND(ID_FILE_SAVESTATE_VERBOSE, OnFileSaveStateVerbose)
	ON_COMMAND(ID_ATARI_MACHINETYPE, OnAtariMachineType)
	ON_COMMAND(ID_ATARI_MACHINETYPE_OSA, OnAtariMachineTypeOsa)
	ON_COMMAND(ID_ATARI_MACHINETYPE_OSB, OnAtariMachineTypeOsb)
	ON_COMMAND(ID_ATARI_MACHINETYPE_XLXE, OnAtariMachineTypeXlXe)
	ON_COMMAND(ID_ATARI_MACHINETYPE_5200, OnAtariMachineType5200)
	ON_COMMAND(ID_ATARI_MEMORYSIZE_16KB, OnAtariMemorySize16Kb)
	ON_COMMAND(ID_ATARI_MEMORYSIZE_48KB, OnAtariMemorySize48Kb)
	ON_COMMAND(ID_ATARI_MEMORYSIZE_52KB, OnAtariMemorySize52Kb)
	ON_COMMAND(ID_ATARI_MEMORYSIZE_128KB, OnAtariMemorySize128Kb)
	ON_COMMAND(ID_ATARI_MEMORYSIZE_64KB, OnAtariMemorySize64Kb)
	ON_COMMAND(ID_ATARI_MEMORYSIZE_320KBCOMPY, OnAtariMemorySize320KbCompy)
	ON_COMMAND(ID_ATARI_MEMORYSIZE_320KBRAMBO, OnAtariMemorySize320KbRambo)
	ON_COMMAND(ID_ATARI_MEMORYSIZE_576KB, OnAtariMemorySize576Kb)
	ON_COMMAND(ID_ATARI_MEMORYSIZE_1088KB, OnAtariMemorySize1088Kb)
	ON_COMMAND(ID_ATARI_VIDEOSYSTEM, OnAtariVideoSystem)
	ON_COMMAND(ID_ATARI_VIDEOSYSTEM_NTSC, OnAtariVideoSystemNtsc)
	ON_COMMAND(ID_ATARI_VIDEOSYSTEM_PAL, OnAtariVideoSystemPal)
	ON_COMMAND(ID_ATARI_SETTINGS, OnAtariSettings)
	ON_COMMAND(ID_ATARI_PERFORMANCE, OnAtariPerformance)
	ON_COMMAND(ID_ATARI_FULLSPEED, OnAtariFullSpeed)
	ON_COMMAND(ID_ATARI_PAUSE, OnAtariPause)
	ON_COMMAND(ID_ATARI_SIOPATCH, OnAtariSioPatch)
	ON_COMMAND(ID_ATARI_HPATCH, OnAtariHPatch)
	ON_COMMAND(ID_ATARI_PPATCH, OnAtariPPatch)
	ON_COMMAND(ID_ATARI_RPATCH, OnAtariRPatch)
	ON_COMMAND(ID_ATARI_DISABLEBASIC, OnAtariDisableBasic)
	ON_COMMAND(ID_ATARI_ROMIMAGES, OnAtariRomImages)
	ON_COMMAND(ID_ATARI_DISKDRIVES, OnAtariDiskDrives)
	ON_COMMAND(ID_ATARI_TAPECONTROL, OnAtariTapeControl)
	ON_COMMAND(ID_ATARI_HARDDISKS, OnAtariHardDisks)
	ON_COMMAND(ID_ATARI_WARMSTART, OnAtariWarmstart)
	ON_COMMAND(ID_ATARI_COLDSTART, OnAtariColdstart)
	ON_COMMAND(ID_INPUT_CAPTUREMOUSE, OnInputCaptureMouse)
	ON_COMMAND(ID_INPUT_JOYSTICKS, OnInputJoysticks)
	ON_COMMAND(ID_INPUT_MOUSE, OnInputMouse)
	ON_COMMAND(ID_INPUT_KEYBOARD, OnInputKeyboard)
	ON_COMMAND(ID_INPUT_AUTOFIRE, OnInputAutofire)
	ON_COMMAND(ID_INPUT_AUTOFIRE_OFF, OnInputAutofireOff)
	ON_COMMAND(ID_INPUT_AUTOFIRE_FIREDEPENDENT, OnInputAutofireFireDependent)
	ON_COMMAND(ID_INPUT_AUTOFIRE_ALLTIME, OnInputAutofireAllTime)
	ON_COMMAND(ID_INPUT_MOUSEDEVICE_NONE, OnInputMouseDeviceNone)
	ON_COMMAND(ID_INPUT_MOUSEDEVICE_PADDLES, OnInputMouseDevicePaddles)
	ON_COMMAND(ID_INPUT_MOUSEDEVICE_TOUCHTABLET, OnInputMouseDeviceTouchTablet)
	ON_COMMAND(ID_INPUT_MOUSEDEVICE_KOALAPAD, OnInputMouseDeviceKoalaPad)
	ON_COMMAND(ID_INPUT_MOUSEDEVICE_LIGHTPEN, OnInputMouseDeviceLightPen)
	ON_COMMAND(ID_INPUT_MOUSEDEVICE_LIGHTGUN, OnInputMouseDeviceLightGun)
	ON_COMMAND(ID_INPUT_MOUSEDEVICE_AMIGAMOUSE, OnInputMouseDeviceAmigaMouse)
	ON_COMMAND(ID_INPUT_MOUSEDEVICE_STMOUSE, OnInputMouseDeviceStMouse)
	ON_COMMAND(ID_INPUT_MOUSEDEVICE_TRAKBALL, OnInputMouseDeviceTrakBall)
	ON_COMMAND(ID_INPUT_MOUSEDEVICE_JOYSTICK, OnInputMouseDeviceJoystick)
	ON_COMMAND(ID_INPUT_ARROWKEYS_ARROWS, OnInputArrowKeysArrows)
	ON_COMMAND(ID_INPUT_ARROWKEYS_CONTROLARROWS, OnInputArrowKeysControlArrows)
	ON_COMMAND(ID_INPUT_ARROWKEYS_F1F4, OnInputArrowKeysF1F4)
	ON_COMMAND(ID_INPUT_NETWORKGAME, OnInputNetworkGame)
	ON_COMMAND(ID_VIEW_GRAPHICSOPTIONS, OnViewOptions)
	ON_COMMAND(ID_VIEW_TOGGLEMODES, OnViewToggleModes)
	ON_COMMAND(ID_VIEW_GDIFORWINDOWS, OnViewGdiForWindows)
	ON_COMMAND(ID_VIEW_STRETCHMODE_PIXELDOUBLING, OnViewStretchModePixelDoubling)
	ON_COMMAND(ID_VIEW_STRETCHMODE_INTERPOLATION, OnViewStretchModeInterpolation)
	ON_COMMAND(ID_VIEW_STRETCHMODE_SCANLINES, OnViewStretchModeScanlines)
	ON_COMMAND(ID_VIEW_STRETCHMODE_HIEND, OnViewStretchModeHiEnd)
	ON_COMMAND(ID_VIEW_SHOW_DISKACTIVITY, OnViewShowDiskActivity)
	ON_COMMAND(ID_VIEW_SHOW_SECTORCOUNTER, OnViewShowSectorCounter)
	ON_COMMAND(ID_VIEW_SHOW_STATEINDICATORS, OnViewShowStateIndicators)
	ON_COMMAND(ID_VIEW_ARTIFACTING, OnViewArtifacting)
	ON_COMMAND(ID_VIEW_ARTIFACTING_NONE, OnViewArtifactingNone)
	ON_COMMAND(ID_VIEW_ARTIFACTING_BLUEBROWN1, OnViewArtifactingBluebrown1)
	ON_COMMAND(ID_VIEW_ARTIFACTING_BLUEBROWN2, OnViewArtifactingBluebrown2)
	ON_COMMAND(ID_VIEW_ARTIFACTING_GTIA, OnViewArtifactingGtia)
	ON_COMMAND(ID_VIEW_ARTIFACTING_CTIA, OnViewArtifactingCtia)
	ON_COMMAND(ID_VIEW_PALETTE, OnViewPalette)
	ON_COMMAND(ID_VIEW_SAVESCREENSHOT_NORMAL, OnViewSaveScreenshotNormal)
	ON_COMMAND(ID_VIEW_SAVESCREENSHOT_INTERLACED, OnViewSaveScreenshotInterlaced)
	ON_COMMAND(ID_VIEW_SAVEVIDEO, OnViewSaveVideo)
	ON_COMMAND(ID_VIEW_SAVEVIDEOANDSOUND, OnViewSaveVideoAndSound)
	ON_COMMAND(ID_VIEW_PERFORMANCETEST, OnViewPerformanceTest)
	ON_COMMAND(ID_SOUND_SOUNDOPTIONS, OnSoundOptions)
	ON_COMMAND(ID_SOUND_STEREO, OnSoundStereo)
	ON_COMMAND(ID_SOUND_MUTE, OnSoundMute)
	ON_COMMAND(ID_SOUND_SAVESOUND, OnSoundSaveSound)
	ON_COMMAND(ID_SOUND_PERFORMANCETEST, OnSoundPerformanceTest)
	ON_COMMAND(ID_MISC_FILEASSOCIATIONS, OnMiscFileAssociations)
	ON_COMMAND(ID_MISC_CONVERT_ROMTOCART, OnMiscConvertRomToCart)
	ON_COMMAND(ID_MISC_CONVERT_DCMTOATR, OnMiscConvertDcmToAtr)
	ON_COMMAND(ID_MISC_CONVERT_XFDTOATR, OnMiscConvertXfdToAtr)
	ON_COMMAND(ID_MISC_CONVERT_ATASCIITOASCII, OnMiscConvertAtasciiToAscii)
	ON_COMMAND(ID_MISC_CONVERT_ASCIITOATASCII, OnMiscConvertAsciiToAtascii)
	ON_COMMAND(ID_MISC_PAUSEINACTIVE, OnMiscPauseInactive)
	ON_COMMAND(ID_MISC_REUSEWINDOW, OnMiscReuseWindow)
	ON_COMMAND(ID_MISC_HIGHPRIORITY, OnMiscHighPriority)
	ON_COMMAND(ID_MISC_CONFIRMONEXIT, OnMiscConfirmOnExit)
	ON_COMMAND(ID_MISC_THREADED_EMULATION, OnMiscThreadedEmulation)
	ON_COMMAND(ID_MISC_CHEATOPTIONS, OnMiscCheatOptions)
	ON_COMMAND(ID_MISC_DISABLECOLLISIONS, OnMiscDisableCollisions)
	ON_COMMAND(ID_MISC_CLEARALLSETTINGS, OnMiscClearAllSettings)
	ON_COMMAND(ID_MISC_RESTARTEMULATION, OnMiscRestartEmulation)
	ON_COMMAND(ID_MISC_VIEWLOGFILE, OnMiscViewLogFile)
	ON_COMMAND(ID_MISC_MONITOR, OnMiscMonitor)
	ON_COMMAND(ID_MISC_MONITORASK, OnMiscMonitorAsk)
	ON_COMMAND(ID_MISC_MONITOROPEN, OnMiscMonitorOpen)
	ON_COMMAND(ID_MISC_MONITORSTOP, OnMiscMonitorStop)
	ON_COMMAND(ID_HELP_KEYBOARD_LAYOUT, OnHelpKeyboardLayout)
	ON_COMMAND(ID_HELP_KEYBOARD_SHORTCUTS, OnHelpKeyboardShortcuts)
	ON_COMMAND(ID_HELP_SETUPWIZARD, OnHelpSetupWizard)
	ON_COMMAND(ID_HELP_WEB_PAGE, OnHelpWebPage)
	ON_COMMAND(ID_APP_ABOUT, OnHelpAbout)
	ON_UPDATE_COMMAND_UI(ID_FILE_BOOTATARIIMAGE, OnUpdateFileBootAtariImage)
	ON_UPDATE_COMMAND_UI(ID_FILE_LOADEXECUTABLE, OnUpdateFileLoadExecutable)
	ON_UPDATE_COMMAND_UI(ID_FILE_ATTACHDISKIMAGE_DRIVE1, OnUpdateFileAttachDiskImageDrive)
	ON_UPDATE_COMMAND_UI(ID_FILE_DETACHDISKIMAGE_ALL, OnUpdateFileDetachDiskImageAll)
	ON_UPDATE_COMMAND_UI(ID_FILE_DETACHDISKIMAGE_DRIVE1, OnUpdateFileDetachDiskImageDrive)
	ON_UPDATE_COMMAND_UI(ID_FILE_DETACHDISKIMAGE_DRIVE2, OnUpdateFileDetachDiskImageDrive)
	ON_UPDATE_COMMAND_UI(ID_FILE_DETACHDISKIMAGE_DRIVE3, OnUpdateFileDetachDiskImageDrive)
	ON_UPDATE_COMMAND_UI(ID_FILE_DETACHDISKIMAGE_DRIVE4, OnUpdateFileDetachDiskImageDrive)
	ON_UPDATE_COMMAND_UI(ID_FILE_DETACHDISKIMAGE_DRIVE5, OnUpdateFileDetachDiskImageDrive)
	ON_UPDATE_COMMAND_UI(ID_FILE_DETACHDISKIMAGE_DRIVE6, OnUpdateFileDetachDiskImageDrive)
	ON_UPDATE_COMMAND_UI(ID_FILE_DETACHDISKIMAGE_DRIVE7, OnUpdateFileDetachDiskImageDrive)
	ON_UPDATE_COMMAND_UI(ID_FILE_DETACHDISKIMAGE_DRIVE8, OnUpdateFileDetachDiskImageDrive)
	ON_UPDATE_COMMAND_UI(ID_FILE_TURNDISK, OnUpdateFileTurnDisk)
	ON_UPDATE_COMMAND_UI(ID_FILE_ATTACHTAPEIMAGE, OnUpdateFileAttachTapeImage)
	ON_UPDATE_COMMAND_UI(ID_FILE_DETACHTAPEIMAGE, OnUpdateFileDetachTapeImage)
	ON_UPDATE_COMMAND_UI(ID_FILE_ATTACHCARTRIDGEIMAGE, OnUpdateFileAttachCartridgeImage)
	ON_UPDATE_COMMAND_UI(ID_FILE_DETACHCARTRIDGEIMAGE, OnUpdateFileDetachCartridgeImage)
	ON_UPDATE_COMMAND_UI(ID_FILE_READSTATE, OnUpdateFileReadState)
	ON_UPDATE_COMMAND_UI(ID_ATARI_MACHINETYPE_OSA, OnUpdateAtariMachineType)
	ON_UPDATE_COMMAND_UI(ID_ATARI_MACHINETYPE_OSB, OnUpdateAtariMachineType)
	ON_UPDATE_COMMAND_UI(ID_ATARI_MACHINETYPE_XLXE, OnUpdateAtariMachineType)
	ON_UPDATE_COMMAND_UI(ID_ATARI_MACHINETYPE_5200, OnUpdateAtariMachineType)
	ON_UPDATE_COMMAND_UI(ID_ATARI_MEMORYSIZE_48KB, OnUpdateAtariMemorySize)
	ON_UPDATE_COMMAND_UI(ID_ATARI_MEMORYSIZE_52KB, OnUpdateAtariMemorySize)
	ON_UPDATE_COMMAND_UI(ID_ATARI_MEMORYSIZE_128KB, OnUpdateAtariMemorySize)
	ON_UPDATE_COMMAND_UI(ID_ATARI_MEMORYSIZE_64KB, OnUpdateAtariMemorySize)
	ON_UPDATE_COMMAND_UI(ID_ATARI_MEMORYSIZE_320KBCOMPY, OnUpdateAtariMemorySize)
	ON_UPDATE_COMMAND_UI(ID_ATARI_MEMORYSIZE_320KBRAMBO, OnUpdateAtariMemorySize)
	ON_UPDATE_COMMAND_UI(ID_ATARI_MEMORYSIZE_576KB, OnUpdateAtariMemorySize)
	ON_UPDATE_COMMAND_UI(ID_ATARI_MEMORYSIZE_1088KB, OnUpdateAtariMemorySize)
	ON_UPDATE_COMMAND_UI(ID_ATARI_VIDEOSYSTEM_NTSC, OnUpdateAtariVideoSystem)
	ON_UPDATE_COMMAND_UI(ID_ATARI_MEMORYSIZE_16KB, OnUpdateAtariMemorySize)
	ON_UPDATE_COMMAND_UI(ID_ATARI_VIDEOSYSTEM_PAL, OnUpdateAtariVideoSystem)
	ON_UPDATE_COMMAND_UI(ID_ATARI_FULLSPEED, OnUpdateAtariFullSpeed)
	ON_UPDATE_COMMAND_UI(ID_ATARI_PAUSE, OnUpdateAtariPause)
	ON_UPDATE_COMMAND_UI(ID_ATARI_SIOPATCH, OnUpdateAtariSioPatch)
	ON_UPDATE_COMMAND_UI(ID_ATARI_HPATCH, OnUpdateAtariHPatch)
	ON_UPDATE_COMMAND_UI(ID_ATARI_PPATCH, OnUpdateAtariPPatch)
	ON_UPDATE_COMMAND_UI(ID_ATARI_RPATCH, OnUpdateAtariRPatch)
	ON_UPDATE_COMMAND_UI(ID_ATARI_DISABLEBASIC, OnUpdateAtariDisableBasic)
	ON_UPDATE_COMMAND_UI(ID_INPUT_CAPTUREMOUSE, OnUpdateInputCaptureMouse)
	ON_UPDATE_COMMAND_UI(ID_INPUT_AUTOFIRE_OFF, OnUpdateInputAutofire)
	ON_UPDATE_COMMAND_UI(ID_INPUT_AUTOFIRE_FIREDEPENDENT, OnUpdateInputAutofire)
	ON_UPDATE_COMMAND_UI(ID_INPUT_AUTOFIRE_ALLTIME, OnUpdateInputAutofire)
	ON_UPDATE_COMMAND_UI(ID_INPUT_MOUSEDEVICE_NONE, OnUpdateInputMouseDevice)
	ON_UPDATE_COMMAND_UI(ID_INPUT_MOUSEDEVICE_TOUCHTABLET, OnUpdateInputMouseDevice)
	ON_UPDATE_COMMAND_UI(ID_INPUT_MOUSEDEVICE_KOALAPAD, OnUpdateInputMouseDevice)
	ON_UPDATE_COMMAND_UI(ID_INPUT_MOUSEDEVICE_LIGHTPEN, OnUpdateInputMouseDevice)
	ON_UPDATE_COMMAND_UI(ID_INPUT_MOUSEDEVICE_LIGHTGUN, OnUpdateInputMouseDevice)
	ON_UPDATE_COMMAND_UI(ID_INPUT_MOUSEDEVICE_PADDLES, OnUpdateInputMouseDevice)
	ON_UPDATE_COMMAND_UI(ID_INPUT_MOUSEDEVICE_STMOUSE, OnUpdateInputMouseDevice)
	ON_UPDATE_COMMAND_UI(ID_INPUT_MOUSEDEVICE_AMIGAMOUSE, OnUpdateInputMouseDevice)
	ON_UPDATE_COMMAND_UI(ID_INPUT_MOUSEDEVICE_TRAKBALL, OnUpdateInputMouseDevice)
	ON_UPDATE_COMMAND_UI(ID_INPUT_MOUSEDEVICE_JOYSTICK, OnUpdateInputMouseDevice)
	ON_UPDATE_COMMAND_UI(ID_INPUT_ARROWKEYS_CONTROLARROWS, OnUpdateInputArrowKeys)
	ON_UPDATE_COMMAND_UI(ID_INPUT_ARROWKEYS_ARROWS, OnUpdateInputArrowKeys)
	ON_UPDATE_COMMAND_UI(ID_INPUT_ARROWKEYS_F1F4, OnUpdateInputArrowKeys)
	ON_UPDATE_COMMAND_UI(ID_INPUT_NETWORKGAME, OnUpdateInputNetworkGame)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TOGGLEMODES, OnUpdateViewToggleModes)
	ON_UPDATE_COMMAND_UI(ID_VIEW_GDIFORWINDOWS, OnUpdateViewGdiForWindows)
	ON_UPDATE_COMMAND_UI(ID_VIEW_STRETCHMODE_PIXELDOUBLING, OnUpdateViewStretchMode)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOW_DISKACTIVITY, OnUpdateViewShowDiskActivity)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOW_SECTORCOUNTER, OnUpdateViewShowSectorCounter)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOW_STATEINDICATORS, OnUpdateViewShowStateIndicators)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ARTIFACTING_NONE, OnUpdateViewArtifacting)
	ON_UPDATE_COMMAND_UI(ID_VIEW_STRETCHMODE_INTERPOLATION, OnUpdateViewStretchMode)
	ON_UPDATE_COMMAND_UI(ID_VIEW_STRETCHMODE_SCANLINES, OnUpdateViewStretchMode)
	ON_UPDATE_COMMAND_UI(ID_VIEW_STRETCHMODE_HIEND, OnUpdateViewStretchMode)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ARTIFACTING_BLUEBROWN1, OnUpdateViewArtifacting)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ARTIFACTING_BLUEBROWN2, OnUpdateViewArtifacting)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ARTIFACTING_GTIA, OnUpdateViewArtifacting)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ARTIFACTING_CTIA, OnUpdateViewArtifacting)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SAVEVIDEO, OnUpdateViewSaveVideo)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SAVEVIDEOANDSOUND, OnUpdateViewSaveVideoAndSound)
	ON_UPDATE_COMMAND_UI(ID_SOUND_STEREO, OnUpdateSoundStereo)
	ON_UPDATE_COMMAND_UI(ID_SOUND_MUTE, OnUpdateSoundMute)
	ON_UPDATE_COMMAND_UI(ID_SOUND_SAVESOUND, OnUpdateSoundSaveSound)
	ON_UPDATE_COMMAND_UI(ID_MISC_PAUSEINACTIVE, OnUpdateMiscPauseInactive)
	ON_UPDATE_COMMAND_UI(ID_MISC_REUSEWINDOW, OnUpdateMiscReuseWindow)
	ON_UPDATE_COMMAND_UI(ID_MISC_HIGHPRIORITY, OnUpdateMiscHighPriority)
	ON_UPDATE_COMMAND_UI(ID_MISC_CONFIRMONEXIT, OnUpdateMiscConfirmOnExit)
	ON_UPDATE_COMMAND_UI(ID_MISC_THREADED_EMULATION, OnUpdateMiscThreadedEmulation)
	ON_UPDATE_COMMAND_UI(ID_MISC_CLEARALLSETTINGS, OnUpdateMiscClearAllSettings)
	ON_UPDATE_COMMAND_UI(ID_MISC_DISABLECOLLISIONS, OnUpdateMiscDisableCollisions)
	ON_UPDATE_COMMAND_UI(ID_MISC_MONITOR, OnUpdateMiscMonitor)
	ON_UPDATE_COMMAND_UI(ID_MISC_MONITORASK, OnUpdateMiscMonitorAsk)
	ON_UPDATE_COMMAND_UI(ID_MISC_MONITOROPEN, OnUpdateMiscMonitorOpen)
	ON_UPDATE_COMMAND_UI(ID_MISC_MONITORSTOP, OnUpdateMiscMonitorStop)
	ON_UPDATE_COMMAND_UI(ID_HELP_SETUPWIZARD, OnUpdateHelpSetupWizard)
	ON_WM_MENUSELECT()
	ON_WM_WINDOWPOSCHANGING()
	ON_WM_DROPFILES()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_ACTIVATE()
	ON_WM_CLOSE()
	ON_WM_MOVE()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONDBLCLK()
	ON_WM_NCMOUSEMOVE()
	ON_WM_NCLBUTTONDOWN()
	ON_WM_NCLBUTTONDBLCLK()
	//}}AFX_MSG_MAP
	/* This message is not supported by ClassWizard */
	ON_WM_EXITMENULOOP()
	/* User defined commands */
	ON_MESSAGE(WM_PLUS_CMDLINE, OnMessageCommandLine)
	ON_MESSAGE(WM_PLUS_KARESET, OnMessageKailleraReset)
	ON_MESSAGE(WM_PLUS_KASTART, OnMessageKailleraStart)
	ON_MESSAGE(WM_PLUS_KASTOP, OnMessageKailleraStop)
	ON_MESSAGE(WM_PLUS_KAGAMESTART, OnMessageKailleraGameStart)
	ON_MESSAGE(WM_PLUS_KAGAMESTOP, OnMessageKailleraGameStop)
	/* Global help commands */
	ON_COMMAND(ID_HELP_FINDER, OnHelpFinder)
/*	ON_COMMAND(ID_HELP, OnHelp)
	ON_COMMAND(ID_DEFAULT_HELP, OnHelpFinder)
	ON_COMMAND(ID_CONTEXT_HELP, OnContextHelp)
	/* Vacats */
//	ON_COMMAND(ID_HELP_INDEX, CFrameWnd::OnHelpIndex)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR, /* Status line indicator */
	ID_SEPARATOR,
	ID_SEPARATOR,
	ID_SEPARATOR,
	ID_SEPARATOR,
	ID_SEPARATOR,
	ID_SEPARATOR,
	ID_SEPARATOR
#ifdef WIN_NETWORK_GAMES
   ,ID_SEPARATOR
#endif
};


/*========================================================
Method   : CMainFrame::CMainFrame
=========================================================*/
/* #FN#
   Standard constructor */
CMainFrame::
CMainFrame()
{
	ULONG ulInputState = g_Input.ulState;
	ULONG ulMiscState  = g_Misc.ulState;

	m_bPosChanging   = TRUE;
	m_pMainApp       = NULL;
	m_pMainView      = NULL;
	m_pSBCtrl        = NULL;
	m_strWindowTitle = "";

	/* These variables have not been written in the Registry */
	m_strVideoName      = DEFAULT_VID;
	m_strSoundName      = DEFAULT_SND;
	m_strScreenshotName = DEFAULT_PIC;

	g_Timer.nPalFreq  = (DEF_PAL_FREQUENCY  * g_Misc.nSpeedPercent) / 100;
	g_Timer.nNtscFreq = (DEF_NTSC_FREQUENCY * g_Misc.nSpeedPercent) / 100;

	/* Cache some title and status messages */
	_LoadStringSx( IDS_TITLE_FAILURE, m_szLabels[ LABEL_FAILURE ] );
	_LoadStringSx( IDS_TITLE_STOPPED, m_szLabels[ LABEL_STOPPED ] );
	_LoadStringSx( IDS_TITLE_NOTRUN,  m_szLabels[ LABEL_NOTRUN  ] );
	_LoadStringSx( IDS_TITLE_MONITOR, m_szLabels[ LABEL_MONITOR ] );
	_LoadStringSx( IDS_TITLE_PAUSED,  m_szLabels[ LABEL_PAUSED  ] );
	_LoadStringSx( IDS_TITLE_JOYKEY,  m_szLabels[ LABEL_JOYKEY  ] );
	_LoadStringSx( IDS_STATUS_SPEED,  m_szLabels[ LABEL_SPEED   ] );

	/* The last session was crashed */
	if( _IsFlagSet( g_Misc.ulState, MS_LAST_BOOT_FAILED ) && !Misc_FindWindow( VERSION_INFO, 0 ) )
	{
		if( IDYES == DisplayMessage( NULL, IDS_WARN_LOCKUP, 0, MB_ICONQUESTION | MB_YESNO ) )
			InitialiseRegistry( NULL, TRUE );
	}
	/* Assign functions to arrow keys */
	Input_SetArrowKeys( g_Input.Key.nArrowsMode );

	/* Set appropriate memory size */
	switch( Atari800_machine_type )
	{
		case Atari800_MACHINE_OSA:
			MEMORY_ram_size = g_anRamSize[ Atari800_MACHINE_OSA ];
			break;

		case Atari800_MACHINE_OSB:
			MEMORY_ram_size = g_anRamSize[ Atari800_MACHINE_OSB ];
			break;

		case Atari800_MACHINE_XLXE:
			MEMORY_ram_size = g_anRamSize[ Atari800_MACHINE_XLXE ];
			break;

		case Atari800_MACHINE_5200:
			MEMORY_ram_size = 16; /* Always 16 KB */
			break;

		default:
			ASSERT(FALSE);
			break;
	}

	/* Initialize the Atari800 core */
	Atari800_Initialise( &g_argc, g_argv );
	Atari_Initialise( &g_argc, g_argv );

	/* Load keys template and external palette */
	if( _IsFlagSet( g_Input.ulState, IS_KEY_USE_TEMPLATE ) &&
		!CKeyTemplateDlg::ReadKeyTemplate( g_szTemplateFile, g_szTemplateDesc, g_Input.Key.anKBTable, GetSafeHwnd() ) )
	{
		_ClrFlag( g_Input.ulState, IS_KEY_USE_TEMPLATE );
		DisplayMessage( NULL, IDS_ERROR_NO_KEYTEMP, 0, MB_ICONEXCLAMATION | MB_OK, g_szTemplateFile );
	}
	if( g_Input.ulState != ulInputState )
		WriteRegDWORD( NULL, REG_INPUT_STATE, g_Input.ulState );

	if( _IsFlagSet( g_Misc.ulState, MS_USE_EXT_PALETTE ) &&
		!Palette_Read( g_szPaletteFile ) )
	{
		_ClrFlag( g_Misc.ulState, MS_USE_EXT_PALETTE );
		DisplayMessage( NULL, IDS_ERROR_NO_PALETTE, 0, MB_ICONEXCLAMATION | MB_OK, g_szPaletteFile );
	}
	Palette_Generate( g_Screen.Pal.nBlackLevel, g_Screen.Pal.nWhiteLevel, g_Screen.Pal.nSaturation, g_Screen.Pal.nContrast, 
		g_Screen.Pal.nBrightness, g_Screen.Pal.nGamma, _IsFlagSet( g_Misc.ulState, MS_USE_EXT_PALETTE ) );

	if( g_Misc.ulState != ulMiscState )
		WriteRegDWORD( NULL, REG_MISC_STATE, g_Misc.ulState );

	/* Disable/enable task switching */
	Input_EnableEscCapture( _IsFlagSet( g_Input.ulState, IS_CAPTURE_CTRLESC ) );

	/* Set the emulator priority level */
	Misc_SetProcessPriority();

	/* Register the extensions automatically if the user wants it */
	if( _IsFlagSet( g_Misc.ulState, MS_AUTO_REG_FILES ) )
	{
		char szHomeDir[ MAX_PATH + 1 ];

		if( !Misc_GetHomeDirectory( szHomeDir ) )
			GetCurrentDirectory( MAX_PATH, szHomeDir );

		CFileAssociationsBase::WriteRegFileExt( g_Misc.ulFileAssociations, szHomeDir, g_szCmdLine );
	}
} /* #OF# CMainFrame::CMainFrame */

/*========================================================
Method   : CMainFrame::~CMainFrame
=========================================================*/
/* #FN#
   Destructor */
CMainFrame::
~CMainFrame()
{
	/* In case desktop isn't getting refreshed when exiting directly from
	   DirectDraw full screen */
	::InvalidateRect( NULL, NULL, FALSE );

} /* #OF# CMainFrame::~CMainFrame */


/////////////////////////////////////////////////////////////////////////////
// CMainFrame implementation

/*========================================================
Method   : CMainFrame::PreCreateWindow
=========================================================*/
/* #FN#
   Called by the framework before the creation of the Windows window */
BOOL
/* #AS#
   Nonzero if the window creation should continue, 0 to indicate creation
   failure */
CMainFrame::
PreCreateWindow(
	CREATESTRUCT& cs
)
{
	cs.x = g_nStartX;
	cs.y = g_nStartY;

	cs.style |= CS_OWNDC;
	cs.style &= ~WS_THICKFRAME;

	return CFrameWnd::PreCreateWindow( cs );

} /* #OF# CMainFrame::PreCreateWindow */

/*========================================================
Method   : CMainFrame::ActivateFrame
=========================================================*/
/* #FN#
   Activates and restores the frame window */
void
/* #AS#
   Nothing */
CMainFrame::
ActivateFrame(
	int nCmdShow /* #IN# Parameter to pass to CWnd::ShowWindow */
)
{
	/* Get a real size of the status bar */
	CRect rect;
	m_wndStatusBar.GetWindowRect( rect );
	g_Screen.nStatusSize = rect.Height();

	CFrameWnd::ActivateFrame( nCmdShow );

} /* #OF# CMainFrame::ActivateFrame */

/*========================================================
Method   : CMainFrame::StartThread
=========================================================*/
// Starts the emulation thread
void CMainFrame::StartThread()
{
	int i;
	ThreadState = TRUE;
	EmuThread = AfxBeginThread( Loop, &i );
}

/*========================================================
Method   : CMainFrame::StopThread
=========================================================*/
// Stops the emulation thread
void CMainFrame::StopThread()
{
	DWORD ExitCode;

	if ( EmuThread ) {
		ResumeThread();
		ThreadState = FALSE;
		do {
			if ( !GetExitCodeThread( EmuThread->m_hThread, &ExitCode ) )
				break;
			SleepEx( 20, TRUE );
		} while ( ExitCode == STILL_ACTIVE );
		EmuThread = NULL;
		Surface1Mutex->Unlock();
		Surface2Mutex->Unlock();
		Surface1Done = Surface2Done = FALSE;
		Screen_atari = (ULONG *) s_Buffer.pMainScr;
		s_Buffer.pSource = s_Buffer.pMainScr;
	}
}

/*========================================================
Method   : CMainFrame::ResumeThread
=========================================================*/
// Resumes the emulation thread
void CMainFrame::ResumeThread()
{
/*	if ( _IsFlagSet( g_Misc.ulState, MS_THREADED_EMULATION ) && EmuThread ) {
		for (;;) {
			switch ( EmuThread->ResumeThread() ) {
				case 0xffff:
				case 0:
				case 1:
					return;					
			}
			SleepEx( 20, TRUE );
		}
	}*/
	Surface1Mutex->Unlock();
	Surface2Mutex->Unlock();
}

/*========================================================
Method   : CMainFrame::SuspendThread
=========================================================*/
// Suspends the emulation thread
void CMainFrame::SuspendThread()
{
/*	if ( _IsFlagSet( g_Misc.ulState, MS_THREADED_EMULATION ) && EmuThread )
		EmuThread->SuspendThread();*/
	Surface1Mutex->Lock();
	Surface2Mutex->Lock();
}

/*========================================================
Method   : CMainFrame::DriveInsert
=========================================================*/
/* #FN#
   Inserts disk image to a pointed virtual drive */
void
/* #AS#
   Nothing */
CMainFrame::
DriveInsert(
	int nDriveNum
)
{
	char    szDiskName[ MAX_PATH + 1 ];
	CString strPrompt;

	/* Get the most recently used file name to use in a file dialog */
	_strncpy( szDiskName, SIO_filename[ nDriveNum - 1 ], MAX_PATH );
	strPrompt.Format( IDS_SELECT_DSK_LOAD, nDriveNum );

	if( PickFileName( TRUE, szDiskName, strPrompt, IDS_FILTER_DSK,
					  "atr", PF_LOAD_FLAGS ) &&
		*szDiskName != '\0' )
	{
		SIO_Dismount( nDriveNum );
		SIO_Mount( nDriveNum, szDiskName, _IsFlagSet(g_Misc.ulState, MS_DRIVE_READONLY) );
		WriteRegDrives( NULL );
	}
} /* #OF# CMainFrame::DriveInsert */

/*========================================================
Method   : CMainFrame::DriveRemove
=========================================================*/
/* #FN#
   Removes disk image from a pointed virtual drive */
void
/* #AS#
   Nothing */
CMainFrame::
DriveRemove(
	int nDriveNum
)
{
	ASSERT(nDriveNum > 0 && nDriveNum < 9);

	SIO_Dismount( nDriveNum );
	if( _IsFlagSet( g_Misc.ulState, MS_TURN_DRIVES_OFF ) )
	{
		SIO_drive_status[ nDriveNum - 1 ] = SIO_OFF;
		strcpy( SIO_filename[ nDriveNum - 1 ], "Off" );
	}
} /* #OF# CMainFrame::DriveRemove */

/*========================================================
Method   : CMainFrame::SaveState
=========================================================*/
/* #FN#
   Saves state of the Atari to a file */
void
/* #AS#
   Nothing */
CMainFrame::
SaveState(
	int nSaveRom
)
{
#ifndef HAVE_LIBZ
	DisplayMessage( NULL, IDS_ERROR_NO_ZLIB, 0, MB_ICONEXCLAMATION | MB_OK );
#else /*HAVE_LIBZ*/
	if( !nSaveRom || (nSaveRom &&
		IDYES == DisplayMessage( NULL, IDS_WARN_VERBOSE, 0, MB_ICONQUESTION | MB_YESNO )) )
	{
		char szFileName[ MAX_PATH + 1 ];
		/* Get the most recently used file name to use in a file dialog */
		_strncpy( szFileName, atari_state_dir, MAX_PATH );

		if( PickFileName( FALSE, szFileName, IDS_SELECT_A8S_SAVE, IDS_FILTER_A8S,
						  "a8s", PF_SAVE_FLAGS, TRUE, DEFAULT_A8S ) &&
			*szFileName != '\0' )
		{
			if( _stricmp( szFileName, atari_state_dir ) != 0 )
			{
				strcpy( atari_state_dir, szFileName );
				WriteRegString( NULL, REG_FILE_STATE, atari_state_dir );
			}
			if( !StateSav_SaveAtariState( atari_state_dir, "wb", nSaveRom ) )
				DisplayMessage( NULL, IDS_ERROR_A8S_SAVE, 0, MB_ICONEXCLAMATION | MB_OK );
		}
		UpdateStatus();
	}
	CleanScreen();
#endif /*HAVE_LIBZ*/
} /* #OF# CMainFrame::SaveState */

/*========================================================
Method   : CMainFrame::SaveSnapshot
=========================================================*/
/* #FN#
   Saves snapshot of the Atari screen to .PCX file */
void
/* #AS#
   Nothing */
CMainFrame::
SaveSnapshot(
	int nInterlace
)
{
	static BOOL bChanged = FALSE;

	char filename[FILENAME_MAX];

	char    szBuffer[ LOADSTRING_SIZE_S + 1 ];
	CString strScreenshotName;
	CString strPrompt;

	if( !bChanged ) {
		Screen_FindScreenshotFilename(filename, FILENAME_MAX);
		m_strScreenshotName = filename;
	}

	strScreenshotName = m_strScreenshotName;
	strPrompt.Format( IDS_SELECT_PCX_SAVE, nInterlace ?
					  _LoadStringSx( IDS_SELECT_PCX_INTERLACED, szBuffer ) :
					  _LoadStringSx( IDS_SELECT_PCX_NORMAL, szBuffer ) );

	if( PickFileName( FALSE, m_strScreenshotName, strPrompt, IDS_FILTER_PCX,
					  "png", PF_SAVE_FLAGS, FALSE, DEFAULT_PIC ) &&
		!m_strScreenshotName.IsEmpty() )
	{
		if( !Screen_SaveScreenshot( (LPSTR)(LPCSTR)m_strScreenshotName , nInterlace ) )
		{
			DisplayMessage( NULL, IDS_ERROR_PCX_SAVE, 0, MB_ICONEXCLAMATION | MB_OK );
		}
		if( !bChanged &&
			strScreenshotName.CompareNoCase( m_strScreenshotName.Right( m_strScreenshotName.GetLength() - m_strScreenshotName.ReverseFind( '\\' ) - 1 ) ) != 0 )
			bChanged = TRUE;
	}
	CleanScreen();

} /* #OF# CMainFrame::SaveSnapshot */

/*========================================================
Method   : CMainFrame::PreTranslateMessage
=========================================================*/
/* #FN#
   Translates window messages before they are dispatched to the
   TranslateMessage and DispatchMessage Windows functions */
BOOL
/* #AS#
   Nonzero if the message was translated and should not be dispatched;
   0 if the message was not translated and should be dispatched */
CMainFrame::
PreTranslateMessage(
	MSG *pMsg
)
{
#ifdef _DEBUG
//	if( WM_KEYDOWN == pMsg->message || WM_KEYUP == pMsg->message ) // if( VK_MENU == wp )
//		TRACE2("msg=0x%x, wp=0x%x\n", pMsg->message, pMsg->wParam);
#endif
	switch( pMsg->message )
	{
		/* This needs to be handled here instead of a OnXXX message
		   because we want to intercept control keys before they are
		   parsed as accelerators */
		case WM_KEYDOWN:
			if( Input_KeyDown( pMsg->wParam, pMsg->lParam ) )
				return TRUE;
			break;

		case WM_KEYUP:
			if( Input_KeyUp( pMsg->wParam, pMsg->lParam ) )
				return TRUE;
			break;

		case WM_SYSKEYUP:
			if( Input_SysKeyUp( pMsg->wParam, pMsg->lParam ) )
				return TRUE;
			break;
	}
	return CFrameWnd::PreTranslateMessage( pMsg );

} /* #OF# CMainFrame::PreTranslateMessage */

/*========================================================
Method   : CMainFrame::OnActivate
=========================================================*/
/* #FN#
   The framework calls this member function when a CWnd object is
   being activated or deactivated */
void
/* #AS#
   Nothing */
CMainFrame::
OnActivate(
	UINT  nState,    /* #IN# Specifies whether the CWnd is being activated/deactivated */
	CWnd *pWndOther, /* #IN# Pointer to the CWnd being activated/deactivated */
	BOOL  bMinimized /* #IN# Specifies the minimized state of the CWnd being activated/deactivated */
)
{
	CFrameWnd::OnActivate( nState, pWndOther, bMinimized );

	if( WA_INACTIVE == nState )
	{
		if( !_IsFlagSet( g_ulAtariState, ATARI_NO_FOCUS ) )
		{
			if( _IsFlagSet( g_Input.ulState, IS_CAPTURE_MOUSE ) )
				Screen_ShowMousePointer( TRUE );

			if( _IsFlagSet( g_ulAtariState, ATARI_RUNNING ) )
			{
				_SetFlag( g_ulAtariState, ATARI_NO_FOCUS );
				UpdateStatus();

				/* Clear only if emulation is stopped when no focus */
//				if( _IsFlagSet( g_Misc.ulState, MS_STOP_WHEN_NO_FOCUS ) )
//					Sound_Clear( FALSE, FALSE );

				Input_ResetKeys();
			}
		}
	}
	else
	{
		/* TO DO...
		   I really don't know why I have to call this to redraw *all* of the
		   indicators when the main window gets a focus under Windows 2000.
		   Looks like a nasty bug in MFC library (maybe I am wrong?).
		*/
		UpdateIndicator( ID_INDICATOR_RUN );

		if( !ST_ATARI_FAILED )
		{
			if( _IsFlagSet( g_ulAtariState, ATARI_NO_FOCUS ) )
			{
				_ClrFlag( g_ulAtariState, ATARI_NO_FOCUS );
				UpdateStatus();

				/* Always restart in this place */
//				Sound_Restart();
				Timer_Start( TRUE );

				if( _IsFlagSet( g_Misc.ulState, MS_STOP_WHEN_NO_FOCUS ) )
				{
					if( m_pMainApp )
						m_pMainApp->ResetLoopCounter( FALSE );
				}
				/* Force screen refreshing */
				g_nTestVal = _GetRefreshRate() - 1;
			}
		}
	}
} /* #OF# CMainFrame::OnActivate */


/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void
CMainFrame::
AssertValid() const
{
	CFrameWnd::AssertValid();
}

void
CMainFrame::
Dump( CDumpContext &dc ) const
{
	CFrameWnd::Dump( dc );
}

#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

/*========================================================
Method   : CMainFrame::OnCreate
=========================================================*/
/* #FN#
   Called by the framework when an application requests that the Windows
   window be created */
int
/* #AS#
   0 to continue the creation, –1 to destroy the Windows window */
CMainFrame::
OnCreate(
	LPCREATESTRUCT lpCreateStruct /* #IN# Pointer to the creation info structure */
)
{
	if( CFrameWnd::OnCreate( lpCreateStruct ) == -1 )
		return -1;

	if( !m_wndStatusBar.Create( this ) ||
		!m_wndStatusBar.SetIndicators( indicators, sizeof(indicators)/sizeof(UINT) ) )
	{
		TRACE0("Failed to create status bar\n");
		return -1; /* Fail to create */
	}
	m_wndStatusBar.ConfigureTray( indicators, sizeof(indicators)/sizeof(UINT), TRUE, this );
	m_pSBCtrl = &m_wndStatusBar.GetStatusBarCtrl();
	m_pSBCtrl->SetMinHeight( 20 );

	/* Remove the "Maximize" option of the sys menu */
	CMenu *pSysMenu = GetSystemMenu( FALSE );
	if( pSysMenu != NULL )
	{
		pSysMenu->RemoveMenu( SC_MAXIMIZE, MF_BYCOMMAND );
		pSysMenu->RemoveMenu( SC_SIZE, MF_BYCOMMAND );
	}
	DragAcceptFiles( TRUE );

	m_pMainApp = (CAtari800WinApp *)AfxGetApp();

	/* Get global handles */
	g_hMainWnd = GetSafeHwnd(); /* "C" modules need this */

	return 0;

} /* #OF# CMainFrame::OnCreate */

/*========================================================
Method   : CMainFrame::OnFileBootAtariImage
=========================================================*/
/* #FN#
   Loads and boots an Atari image */
void
/* #AS#
   Nothing */
CMainFrame::
OnFileBootAtariImage()
{
	SuspendThread();

	char szFileName[ MAX_PATH + 1 ];

	/* Get the most recently used file name to use in a file dialog */
	_strncpy( szFileName, g_szAutobootFile, MAX_PATH );

	if( PickFileName( TRUE, szFileName, IDS_SELECT_ALL_LOAD, IDS_FILTER_ALL,
					  "atr", PF_LOAD_FLAGS ) &&
		*szFileName != '\0' )
	{
		if( _stricmp( szFileName, g_szAutobootFile ) != 0 )
		{
			strcpy( g_szAutobootFile, szFileName );
			WriteRegString( NULL, REG_FILE_AUTOBOOT, g_szAutobootFile );
		}
		if( !AutobootAtariImage( g_szAutobootFile ) )
		{
			DisplayMessage( NULL, IDS_ERROR_AUTOBOOT, 0, MB_ICONEXCLAMATION | MB_OK, szFileName );
		}
	}
	UpdateStatus();
	CleanScreen();

	ResumeThread();
} /* #OF# CMainFrame::OnFileBootAtariImage */

/*========================================================
Method   : CMainFrame::OnFileLoadExecutable
=========================================================*/
/* #FN#
   Loads an Atari executable file directly */
void
/* #AS#
   Nothing */
CMainFrame::
OnFileLoadExecutable()
{
	SuspendThread();

	char szFileName[ MAX_PATH + 1 ];
	_strncpy( szFileName, atari_exe_dir, MAX_PATH );

	if( PickFileName( TRUE, szFileName, IDS_SELECT_BIN_LOAD, IDS_FILTER_BIN,
					  "xex", PF_LOAD_FLAGS, TRUE, DEFAULT_BIN ) &&
		*szFileName != '\0' )
	{
		UINT unFileType = IAF_BIN_IMAGE;

		if( IsAtariFile( szFileName, &unFileType ) )
		{
			if( ST_ATARI_FAILED )
				RestartEmulation( TRUE );

			if( !RunExecutable( szFileName ) )
			{
				DisplayMessage( NULL, IDS_ERROR_BIN_RUN, 0, MB_ICONEXCLAMATION | MB_OK, szFileName );
			}
		}
		else
			DisplayMessage( NULL, IDS_ERROR_BIN_LOAD, 0, MB_ICONEXCLAMATION | MB_OK, szFileName );
	}
	UpdateStatus();
	/* Clean up the screen */
	CleanScreen();

	ResumeThread();
} /* #OF# CMainFrame::OnFileLoadExecutable */

/*========================================================
Method   : CMainFrame::OnFileAttachDiskImageDrive1
=========================================================*/
/* #FN#
   Attaches a disk image to an emulated drive 1 */
void
/* #AS#
   Nothing */
CMainFrame::
OnFileAttachDiskImageDrive1()
{
	SuspendThread();
	DriveInsert( 1 );
	CleanScreen();
	ResumeThread();
} /* #OF# CMainFrame::OnFileAttachDiskImageDrive1 */

/*========================================================
Method   : CMainFrame::OnFileAttachDiskImageDrive2
=========================================================*/
/* #FN#
   Attaches a disk image to an emulated drive 2 */
void
/* #AS#
   Nothing */
CMainFrame::
OnFileAttachDiskImageDrive2()
{
	SuspendThread();
	DriveInsert( 2 );
	CleanScreen();
	ResumeThread();
} /* #OF# CMainFrame::OnFileAttachDiskImageDrive2 */

/*========================================================
Method   : CMainFrame::OnFileAttachDiskImageDrive3
=========================================================*/
/* #FN#
   Attaches a disk image to an emulated drive 3 */
void
/* #AS#
   Nothing */
CMainFrame::
OnFileAttachDiskImageDrive3()
{
	SuspendThread();
	DriveInsert( 3 );
	CleanScreen();
	ResumeThread();
} /* #OF# CMainFrame::OnFileAttachDiskImageDrive3 */

/*========================================================
Method   : CMainFrame::OnFileAttachDiskImageDrive4
=========================================================*/
/* #FN#
   Attaches a disk image to an emulated drive 4 */
void
/* #AS#
   Nothing */
CMainFrame::
OnFileAttachDiskImageDrive4()
{
	SuspendThread();
	DriveInsert( 4 );
	CleanScreen();
	ResumeThread();
} /* #OF# CMainFrame::OnFileAttachDiskImageDrive4 */

/*========================================================
Method   : CMainFrame::OnFileAttachDiskImageDrive5
=========================================================*/
/* #FN#
   Attaches a disk image to an emulated drive 5 */
void
/* #AS#
   Nothing */
CMainFrame::
OnFileAttachDiskImageDrive5()
{
	SuspendThread();
	DriveInsert( 5 );
	CleanScreen();
	ResumeThread();
} /* #OF# CMainFrame::OnFileAttachDiskImageDrive5 */

/*========================================================
Method   : CMainFrame::OnFileAttachDiskImageDrive6
=========================================================*/
/* #FN#
   Attaches a disk image to an emulated drive 6 */
void
/* #AS#
   Nothing */
CMainFrame::
OnFileAttachDiskImageDrive6()
{
	SuspendThread();
	DriveInsert( 6 );
	CleanScreen();
	ResumeThread();
} /* #OF# CMainFrame::OnFileAttachDiskImageDrive6 */

/*========================================================
Method   : CMainFrame::OnFileAttachDiskImageDrive7
=========================================================*/
/* #FN#
   Attaches a disk image to an emulated drive 7 */
void
/* #AS#
   Nothing */
CMainFrame::
OnFileAttachDiskImageDrive7()
{
	SuspendThread();
	DriveInsert( 7 );
	CleanScreen();
	ResumeThread();
} /* #OF# CMainFrame::OnFileAttachDiskImageDrive7 */

/*========================================================
Method   : CMainFrame::OnFileAttachDiskImageDrive8
=========================================================*/
/* #FN#
   Attaches a disk image to an emulated drive 8 */
void
/* #AS#
   Nothing */
CMainFrame::
OnFileAttachDiskImageDrive8()
{
	SuspendThread();
	DriveInsert( 8 );
	CleanScreen();
	ResumeThread();
} /* #OF# CMainFrame::OnFileAttachDiskImageDrive8 */

/*========================================================
Method   : CMainFrame::OnFileDetachDiskImageAll
=========================================================*/
/* #FN#
   Detaches disk images from all the emulated drives */
void
/* #AS#
   Nothing */
CMainFrame::
OnFileDetachDiskImageAll()
{
	SuspendThread();

	for( int i = 1; i <= SIO_MAX_DRIVES; i++ )
	{
#ifdef WIN_NETWORK_GAMES
		if( 1 != i || !ST_KAILLERA_ACTIVE )
#endif
			DriveRemove( i );
	}
	WriteRegDrives( NULL );

	ResumeThread();
} /* #OF# CMainFrame::OnFileDetachDiskImageAll */

/*========================================================
Method   : CMainFrame::OnFileDetachDiskImageDrive1
=========================================================*/
/* #FN#
   Detaches a disk image from an emulated drive 1 */
void
/* #AS#
   Nothing */
CMainFrame::
OnFileDetachDiskImageDrive1()
{
	SuspendThread();
	DriveRemove( 1 );
	WriteRegDrives( NULL );
	ResumeThread();
} /* #OF# CMainFrame::OnFileDetachDiskImageDrive1 */

/*========================================================
Method   : CMainFrame::OnFileDetachDiskImageDrive2
=========================================================*/
/* #FN#
   Detaches a disk image from an emulated drive 2 */
void
/* #AS#
   Nothing */
CMainFrame::
OnFileDetachDiskImageDrive2()
{
	SuspendThread();
	DriveRemove( 2 );
	WriteRegDrives( NULL );
	ResumeThread();
} /* #OF# CMainFrame::OnFileDetachDiskImageDrive2 */

/*========================================================
Method   : CMainFrame::OnFileDetachDiskImageDrive3
=========================================================*/
/* #FN#
   Detaches a disk image from an emulated drive 3 */
void
/* #AS#
   Nothing */
CMainFrame::
OnFileDetachDiskImageDrive3()
{
	SuspendThread();
	DriveRemove( 3 );
	WriteRegDrives( NULL );
	ResumeThread();
} /* #OF# CMainFrame::OnFileDetachDiskImageDrive3 */

/*========================================================
Method   : CMainFrame::OnFileDetachDiskImageDrive4
=========================================================*/
/* #FN#
   Detaches a disk image from an emulated drive 4 */
void
/* #AS#
   Nothing */
CMainFrame::
OnFileDetachDiskImageDrive4()
{
	SuspendThread();
	DriveRemove( 4 );
	WriteRegDrives( NULL );
	ResumeThread();
} /* #OF# CMainFrame::OnFileDetachDiskImageDrive4 */

/*========================================================
Method   : CMainFrame::OnFileDetachDiskImageDrive5
=========================================================*/
/* #FN#
   Detaches a disk image from an emulated drive 5 */
void
/* #AS#
   Nothing */
CMainFrame::
OnFileDetachDiskImageDrive5()
{
	SuspendThread();
	DriveRemove( 5 );
	WriteRegDrives( NULL );
	ResumeThread();
} /* #OF# CMainFrame::OnFileDetachDiskImageDrive5 */

/*========================================================
Method   : CMainFrame::OnFileDetachDiskImageDrive6
=========================================================*/
/* #FN#
   Detaches a disk image from an emulated drive 6 */
void
/* #AS#
   Nothing */
CMainFrame::
OnFileDetachDiskImageDrive6()
{
	SuspendThread();
	DriveRemove( 6 );
	WriteRegDrives( NULL );
	ResumeThread();
} /* #OF# CMainFrame::OnFileDetachDiskImageDrive6 */

/*========================================================
Method   : CMainFrame::OnFileDetachDiskImageDrive7
=========================================================*/
/* #FN#
   Detaches a disk image from an emulated drive 7 */
void
/* #AS#
   Nothing */
CMainFrame::
OnFileDetachDiskImageDrive7()
{
	SuspendThread();
	DriveRemove( 7 );
	WriteRegDrives( NULL );
	ResumeThread();
} /* #OF# CMainFrame::OnFileDetachDiskImageDrive7 */

/*========================================================
Method   : CMainFrame::OnFileDetachDiskImageDrive8
=========================================================*/
/* #FN#
   Detaches a disk image from an emulated drive 8 */
void
/* #AS#
   Nothing */
CMainFrame::
OnFileDetachDiskImageDrive8()
{
	SuspendThread();
	DriveRemove( 8 );
	WriteRegDrives( NULL );
	ResumeThread();
} /* #OF# CMainFrame::OnFileDetachDiskImageDrive8 */

/*========================================================
Method   : CMainFrame::OnFileTurnDisk
=========================================================*/
/* #FN#
   Turns disk images in an emulated drive 1 */
void
/* #AS#
   Nothing */
CMainFrame::
OnFileTurnDisk()
{
	SuspendThread();

	char szDiskPath[ MAX_PATH + 1 ];

	_strncpy( szDiskPath, SIO_filename[ 0 ], MAX_PATH );
	if( strcmp( szDiskPath, "Empty" ) != 0 &&
		strcmp( szDiskPath, "Off" ) != 0 )
	{
		char  szDrive [ _MAX_DRIVE + 1 ];
		char  szDir   [ _MAX_DIR   + 1 ];
		char  szFile  [ _MAX_FNAME + 1 ];
		char  szExt   [ _MAX_EXT   + 1 ];
		char  szCurDir[ MAX_PATH   + 1 ];
		char  szNewDir[ MAX_PATH   + 1 ];

		const char acSides[ 3 ][ 2 ] = { {'1', '9'}, {'A', 'Z'}, {'a', 'z'} };
		char  cDiskSide, cNewDiskSide;
		BOOL  bTurnDisk = FALSE;
		int   i; /* Loops index */

		CFile cfFile;

		GetCurrentDirectory( MAX_PATH, szCurDir );
		_strncpy( szNewDir, szDiskPath, MAX_PATH );

		for( i = strlen( szNewDir ); i > 0 && szNewDir[ i ] != '\\'; i-- );
		if( i > 0 )
			szNewDir[ i ] = '\0';

		SetCurrentDirectory( szNewDir );

		/* Split disk full path */
		_tsplitpath( szDiskPath, szDrive, szDir, szFile, szExt );

		cNewDiskSide = cDiskSide = szFile[ strlen( szFile ) - 1 ];

		for( i = 0; i < 3 && !bTurnDisk; i++ )
		{
			if( cNewDiskSide >= acSides[ i ][ 0 ] && cNewDiskSide <= acSides[ i ][ 1 ] )
			{
				do
				{
					if( ++cNewDiskSide > acSides[ i ][ 1 ] )
						cNewDiskSide = acSides[ i ][ 0 ];

					szFile[ strlen( szFile ) - 1 ] = cNewDiskSide;
					_makepath( szDiskPath, szDrive, szDir, szFile, szExt );
					if( cfFile.Open( szDiskPath, CFile::modeRead | CFile::typeBinary, NULL ) )
					{
						bTurnDisk = TRUE;
						cfFile.Close();
						/* Display an attached file name on the status bar */
						if( _IsFlagSet( g_Screen.ulState, SM_MODE_WIND ) || /* The full-screen modes condition */
							ST_MENU_VISIBLE )
						{
							m_pSBCtrl->SetText( szDiskPath, 0, SBT_NOBORDERS );
							if( m_pMainApp )
								m_pMainApp->ResetLoopCounter();
						}
						break;
					}
				}
				while( cNewDiskSide != cDiskSide );
			}
		}
		if( bTurnDisk )
		{
			SIO_Dismount( 1 );
			SIO_Mount( 1, szDiskPath, SIO_drive_status[0]==SIO_READ_ONLY );
			WriteRegDrives( NULL );
		}
		/* Restore directory */
		SetCurrentDirectory( szCurDir );
	}

	ResumeThread();
} /* #OF# CMainFrame::OnFileTurnDisk */

/*========================================================
Method   : CMainFrame::OnFileAttachTapeImage
=========================================================*/
/* #FN#
   Attaches a tape image to an emulated tape player */
void
/* #AS#
   Nothing */
CMainFrame::
OnFileAttachTapeImage()
{
	SuspendThread();

	char szFileName[ FILENAME_MAX ];
	_strncpy( szFileName, cassette_filename, FILENAME_MAX - 1 );

	if( PickFileName( TRUE, szFileName, IDS_SELECT_CAS_LOAD, IDS_FILTER_CAS,
					  "cas", PF_LOAD_FLAGS, TRUE, DEFAULT_TAP ) &&
		*szFileName != '\0' )
	{
		UINT unFileType = IAF_CAS_IMAGE;

		if( IsAtariFile( szFileName, &unFileType ) ||
			DisplayWarning( IDS_WARN_CASFILE, DONT_SHOW_CASFILE_WARN, TRUE ) )
		{
			CASSETTE_Remove(); /* Prevents from resource leaks */
			if( CASSETTE_Insert( szFileName ) )
			{
				WriteRegString( NULL, REG_FILE_TAPE, szFileName );
			}
			else
				DisplayMessage( NULL, IDS_ERROR_CAS_READ, 0, MB_ICONEXCLAMATION | MB_OK );
		}
	}
	/* Clean up the screen */
	CleanScreen();

	ResumeThread();
} /* #OF# CMainFrame::OnFileAttachTapeImage */

/*========================================================
Method   : CMainFrame::OnFileDetachTapeImage
=========================================================*/
/* #FN#
   Detaches an inserted tape image */
void
/* #AS#
   Nothing */
CMainFrame::
OnFileDetachTapeImage()
{
	SuspendThread();

	CASSETTE_Remove();
	WriteRegString( NULL, REG_FILE_TAPE, cassette_filename );

	cassette_current_block = 1;
	cassette_max_block = 1;

	ResumeThread();
} /* #OF# CMainFrame::OnFileDetachTapeImage */

/*========================================================
Method   : CMainFrame::OnFileAttachCartridgeImage
=========================================================*/
/* #FN#
   Displays Attach Cartridge file dialog */
void
/* #AS#
   Nothing */
CMainFrame::
OnFileAttachCartridgeImage()
{
	SuspendThread();

	char szNewCart[ MAX_PATH + 1 ];

	_strncpy( szNewCart, g_szOtherRom, MAX_PATH );

	if( PickCartridge( szNewCart ) )
	{
		if( CARTRIDGE_NONE != AttachCartridge( szNewCart, CARTRIDGE_NONE ) )
		{
			strcpy( g_szOtherRom, g_szCurrentRom );
			WriteRegString( NULL, REG_ROM_OTHER, g_szOtherRom );
		}
		WriteRegString( NULL, REG_ROM_CURRENT, g_szCurrentRom );
		WriteRegDWORD ( NULL, REG_CART_TYPE,   CARTRIDGE_type );

		if( _IsFlagSet( g_Misc.ulState, MS_REBOOT_WHEN_CART ) )
			OnAtariColdstart();
	}
	CleanScreen();

	ResumeThread();
} /* #OF# CMainFrame::OnFileAttachCartridgeImage */

/*========================================================
Method   : CMainFrame::OnFileDetachCartridgeImage
=========================================================*/
/* #FN#
   Detaches an inserted cartridge image */
void
/* #AS#
   Nothing */
CMainFrame::
OnFileDetachCartridgeImage()
{
	SuspendThread();

	CARTRIDGE_Remove();
	strcpy( g_szCurrentRom, FILE_NONE );

	WriteRegString( NULL, REG_ROM_CURRENT, g_szCurrentRom );
	WriteRegDWORD ( NULL, REG_CART_TYPE, CARTRIDGE_type );

	if( _IsFlagSet( g_Misc.ulState, MS_REBOOT_WHEN_CART ) )
		OnAtariColdstart();

	ResumeThread();
} /* #OF# CMainFrame::OnFileDetachCartridgeImage */

/*========================================================
Method   : CMainFrame::OnFileReadState
=========================================================*/
/* #FN#
   Restores saved state file */
void
/* #AS#
   Nothing */
CMainFrame::
OnFileReadState()
{
	SuspendThread();

#ifndef HAVE_LIBZ
	DisplayMessage( NULL, IDS_ERROR_NO_ZLIB, 0, MB_ICONEXCLAMATION | MB_OK );
#else /*HAVE_LIBZ*/
	char szFileName[ MAX_PATH + 1 ];
	/* Get the most recently used file name to use in a file dialog */
	_strncpy( szFileName, atari_state_dir, MAX_PATH );

	if( PickFileName( TRUE, szFileName, IDS_SELECT_A8S_LOAD, IDS_FILTER_A8S,
						"a8s", PF_LOAD_FLAGS, TRUE, DEFAULT_A8S ) &&
		*szFileName != '\0' )
	{
		if( RunSnapshot( szFileName ) )
		{
			if( !_IsFlagSet( g_ulAtariState, ATARI_PAUSED ) )
				Misc_TogglePause();

			UpdateStatus();
		}
		else
			DisplayMessage( NULL, IDS_ERROR_A8S_READ, 0, MB_ICONEXCLAMATION | MB_OK );
	}
	else
		_ClrFlag( g_ulAtariState, ATARI_PAUSED );
	CleanScreen();
#endif /*HAVE_LIBZ*/
	ResumeThread();
} /* #OF# CMainFrame::OnFileReadState */

/*========================================================
Method   : CMainFrame::OnFileSaveStateNormal
=========================================================*/
/* #FN#
   Dumps the exact state of the Atari to a file */
void
/* #AS#
   Nothing */
CMainFrame::
OnFileSaveStateNormal()
{
	SuspendThread();
	SaveState( 0 );
	ResumeThread();
} /* #OF# CMainFrame::OnFileSaveStateNormal */

/*========================================================
Method   : CMainFrame::OnFileSaveStateVerbose
=========================================================*/
/* #FN#
   Saves a memory image with the ROMs */
void
/* #AS#
   Nothing */
CMainFrame::
OnFileSaveStateVerbose()
{
	SuspendThread();
	SaveState( 1 );
	ResumeThread();
} /* #OF# CMainFrame::OnFileSaveStateVerbose */

/*========================================================
Method   : CMainFrame::OnAtariMachineType
=========================================================*/
/* #FN#
   Flips emulated machines */
void
/* #AS#
   Nothing */
CMainFrame::
OnAtariMachineType()
{
	SuspendThread();

	int nMachineType = Atari800_machine_type;

	if( ++nMachineType > Atari800_MACHINE_5200 )
		nMachineType = Atari800_MACHINE_OSA;

	MachineTypeChanged( nMachineType );

	ResumeThread();
} /* #OF# CMainFrame::OnAtariMachineType */

/*========================================================
Method   : CMainFrame::MachineTypeChanged
=========================================================*/
/* #FN#
   Makes an emulated machine changing visible for the emulator */
void
/* #AS#
   Nothing */
CMainFrame::
MachineTypeChanged(
	int nMachineType
)
{
	if( nMachineType != Atari800_machine_type )
	{
		if( StreamWarning( IDS_WARN_RECORD_SYSTEM, SRW_VIDEO_STREAM | SRW_SOUND_STREAM ) )
		{
			if( Atari800_MACHINE_5200 == Atari800_machine_type ||
				Atari800_MACHINE_5200 == nMachineType )
			{
				if( CARTRIDGE_type != CARTRIDGE_NONE )
				{
					CARTRIDGE_Remove();
					strcpy( g_szCurrentRom, FILE_NONE );

					WriteRegString( NULL, REG_ROM_CURRENT, g_szCurrentRom );
					WriteRegDWORD ( NULL, REG_CART_TYPE,   CARTRIDGE_type );
				}
			}
			Atari800_machine_type = nMachineType;
			WriteRegDWORD( NULL, REG_MACHINE_TYPE, Atari800_machine_type );

			switch( Atari800_machine_type )
			{
				case Atari800_MACHINE_OSA:
					MEMORY_ram_size = g_anRamSize[ Atari800_MACHINE_OSA ];
					break;

				case Atari800_MACHINE_OSB:
					MEMORY_ram_size = g_anRamSize[ Atari800_MACHINE_OSB ];
					break;

				case Atari800_MACHINE_XLXE:
					MEMORY_ram_size = g_anRamSize[ Atari800_MACHINE_XLXE ];
					break;

				case Atari800_MACHINE_5200:
					MEMORY_ram_size = 16; /* Always 16 KB */
					break;

				default:
					ASSERT(FALSE);
					break;
			}
			RestartEmulation( TRUE );
			UpdateStatus();
		}
	}
} /* #OF# CMainFrame::MachineTypeChanged */

/*========================================================
Method   : CMainFrame::OnAtariMachineTypeOsa
=========================================================*/
/* #FN#
   Switches the emulator to an Atari OS-A emulation */
void
/* #AS#
   Nothing */
CMainFrame::
OnAtariMachineTypeOsa()
{
	SuspendThread();
	MachineTypeChanged( Atari800_MACHINE_OSA );
	ResumeThread();
} /* #OF# CMainFrame::OnAtariMachineTypeOsa */

/*========================================================
Method   : CMainFrame::OnAtariMachineTypeOsb
=========================================================*/
/* #FN#
   Switches the emulator to an Atari OS-B emulation */
void
/* #AS#
   Nothing */
CMainFrame::
OnAtariMachineTypeOsb()
{
	SuspendThread();
	MachineTypeChanged( Atari800_MACHINE_OSB );
	ResumeThread();
} /* #OF# CMainFrame::OnAtariMachineTypeOsb */

/*========================================================
Method   : CMainFrame::OnAtariMachineTypeXlXe
=========================================================*/
/* #FN#
   Switches the emulator to an Atari XL/XE emulation */
void
/* #AS#
   Nothing */
CMainFrame::
OnAtariMachineTypeXlXe()
{
	SuspendThread();
	MachineTypeChanged( Atari800_MACHINE_XLXE );
	ResumeThread();
} /* #OF# CMainFrame::OnAtariMachineType800xl */

/*========================================================
Method   : CMainFrame::OnAtariMachineType5200
=========================================================*/
/* #FN#
   Switches the emulator to an Atari 5200 emulation */
void
/* #AS#
   Nothing */
CMainFrame::
OnAtariMachineType5200()
{
	SuspendThread();
	MachineTypeChanged( Atari800_MACHINE_5200 );
	ResumeThread();
} /* #OF# CMainFrame::OnAtariMachineType5200 */

/*========================================================
Method   : CMainFrame::OnAtariVideoSystem
=========================================================*/
/* #FN#
   Flips emulated video standards */
void
/* #AS#
   Nothing */
CMainFrame::
OnAtariVideoSystem()
{
	SuspendThread();

	if( StreamWarning( IDS_WARN_RECORD_SYSTEM, SRW_VIDEO_STREAM | SRW_SOUND_STREAM ) )
	{
		Atari800_tv_mode = (Atari800_TV_PAL == Atari800_tv_mode ? Atari800_TV_NTSC : Atari800_TV_PAL);
		WriteRegDWORD( NULL, REG_TV_MODE, Atari800_tv_mode );
		Colours_Update();

		if( _IsFlagSet( g_Misc.ulState, MS_REBOOT_WHEN_VIDEO ) )
		{
			RestartEmulation( TRUE );
			Sound_Initialise( TRUE );
		}
		else
		{
			/* At least the Atari timer stuff has to be reinitialised */
			Timer_Reset();
			Sound_Initialise( FALSE );
			Timer_Start( FALSE );
		}
		UpdateIndicator( ID_INDICATOR_VID );
	}

	ResumeThread();
} /* #OF# CMainFrame::OnAtariVideoSystem */

/*========================================================
Method   : CMainFrame::OnAtariVideoSystemNtsc
=========================================================*/
/* #FN#
	Sets a state of the object regarding to an appropriate menu item */
void
/* #AS#
   Nothing */
CMainFrame::
OnAtariVideoSystemNtsc()
{
	if( Atari800_TV_NTSC != Atari800_tv_mode )
		OnAtariVideoSystem();

} /* #OF# CMainFrame::OnAtariVideoSystemNtsc */

/*========================================================
Method   : CMainFrame::OnAtariVideoSystemPal
=========================================================*/
/* #FN#
	Sets a state of the object regarding to an appropriate menu item */
void
/* #AS#
   Nothing */
CMainFrame::
OnAtariVideoSystemPal()
{
	if( Atari800_TV_PAL != Atari800_tv_mode )
		OnAtariVideoSystem();

} /* #OF# CMainFrame::OnAtariVideoSystemPal */

/*========================================================
Method   : CMainFrame::MemorySizeChanged
=========================================================*/
/* #FN#
   Makes a memory size changing visible for the emulator */
void
/* #AS#
   Nothing */
CMainFrame::
MemorySizeChanged(
	int nRamSize
)
{
	if( nRamSize != MEMORY_ram_size )
	{
		MEMORY_ram_size = nRamSize;

		switch( Atari800_machine_type )
		{
			case Atari800_MACHINE_OSA:
				g_anRamSize[ Atari800_MACHINE_OSA ] = MEMORY_ram_size;
				WriteRegDWORD( NULL, REG_RAMSIZE_OSA, MEMORY_ram_size );
				break;

			case Atari800_MACHINE_OSB:
				g_anRamSize[ Atari800_MACHINE_OSB ] = MEMORY_ram_size;
				WriteRegDWORD( NULL, REG_RAMSIZE_OSB, MEMORY_ram_size );
				break;

			case Atari800_MACHINE_XLXE:
				g_anRamSize[ Atari800_MACHINE_XLXE ] = MEMORY_ram_size;
				WriteRegDWORD( NULL, REG_RAMSIZE_XLXE, MEMORY_ram_size );
				break;

			case Atari800_MACHINE_5200:
				/* Always 16 KB */
				break;

			default:
				ASSERT(FALSE);
				break;
		}
		InitialiseMachine();
		UpdateStatus();
	}
} /* #OF# CMainFrame::MemorySizeChanged */

/*========================================================
Method   : CMainFrame::OnAtariMemorySize16Kb
=========================================================*/
/* #FN#
   Sets an emulated memory size to 16 KB */
void
/* #AS#
   Nothing */
CMainFrame::
OnAtariMemorySize16Kb()
{
	SuspendThread();
	MemorySizeChanged( 16 );
	ResumeThread();
} /* #OF# CMainFrame::OnAtariMemorySize16Kb */

/*========================================================
Method   : CMainFrame::OnAtariMemorySize48Kb
=========================================================*/
/* #FN#
   Sets an emulated memory size to 48 KB */
void
/* #AS#
   Nothing */
CMainFrame::
OnAtariMemorySize48Kb()
{
	SuspendThread();
	MemorySizeChanged( 48 );
	ResumeThread();
} /* #OF# CMainFrame::OnAtariMemorySize48Kb */

/*========================================================
Method   : CMainFrame::OnAtariMemorySize52Kb
=========================================================*/
/* #FN#
   Sets an emulated memory size to 52 KB */
void
/* #AS#
   Nothing */
CMainFrame::
OnAtariMemorySize52Kb()
{
	SuspendThread();
	MemorySizeChanged( 52 );
	ResumeThread();
} /* #OF# CMainFrame::OnAtariMemorySize52Kb */

/*========================================================
Method   : CMainFrame::OnAtariMemorySize64Kb
=========================================================*/
/* #FN#
   Sets an emulated memory size to 64 KB */
void
/* #AS#
   Nothing */
CMainFrame::
OnAtariMemorySize64Kb()
{
	SuspendThread();
	MemorySizeChanged( 64 );
	ResumeThread();
} /* #OF# CMainFrame::OnAtariMemorySize64Kb */

/*========================================================
Method   : CMainFrame::OnAtariMemorySize128Kb
=========================================================*/
/* #FN#
   Sets an emulated memory size to 128 KB */
void
/* #AS#
   Nothing */
CMainFrame::
OnAtariMemorySize128Kb()
{
	SuspendThread();
	MemorySizeChanged( 128 );
	ResumeThread();
} /* #OF# CMainFrame::OnAtariMemorySize128Kb */

/*========================================================
Method   : CMainFrame::OnAtariMemorySize320KbCompy
=========================================================*/
/* #FN#
   Sets an emulated memory size to 320 KB (Compy) */
void
/* #AS#
   Nothing */
CMainFrame::
OnAtariMemorySize320KbCompy()
{
	SuspendThread();
	MemorySizeChanged( MEMORY_RAM_320_COMPY_SHOP );
	ResumeThread();
} /* #OF# CMainFrame::OnAtariMemorySize320KbCompy */

/*========================================================
Method   : CMainFrame::OnAtariMemorySize320KbRambo
=========================================================*/
/* #FN#
   Sets an emulated memory size to 320 KB (Rambo) */
void
/* #AS#
   Nothing */
CMainFrame::
OnAtariMemorySize320KbRambo()
{
	SuspendThread();
	MemorySizeChanged( MEMORY_RAM_320_RAMBO );
	ResumeThread();
} /* #OF# CMainFrame::OnAtariMemorySize320KbRambo */

/*========================================================
Method   : CMainFrame::OnAtariMemorySize576Kb
=========================================================*/
/* #FN#
   Sets an emulated memory size to 576 KB */
void
/* #AS#
   Nothing */
CMainFrame::
OnAtariMemorySize576Kb()
{
	SuspendThread();
	MemorySizeChanged( 576 );
	ResumeThread();
} /* #OF# CMainFrame::OnAtariMemorySize576Kb */

/*========================================================
Method   : CMainFrame::OnAtariMemorySize1088Kb
=========================================================*/
/* #FN#
   Sets an emulated memory size to 1088 KB */
void
/* #AS#
   Nothing */
CMainFrame::
OnAtariMemorySize1088Kb()
{
	SuspendThread();
	MemorySizeChanged( 1088 );
	ResumeThread();
} /* #OF# CMainFrame::OnAtariMemorySize1088Kb */

/*========================================================
Method   : CMainFrame::OnAtariSettings
=========================================================*/
/* #FN#
   Displays Atari Settings dialog box */
void
/* #AS#
   Nothing */
CMainFrame::
OnAtariSettings()
{
	SuspendThread();

	CSettingsDlg dlgSettings( this );

	if( IDOK == dlgSettings.DoModal() )
	{
		if( dlgSettings.m_bReboot || _IsFlagSet( g_ulAtariState, ATARI_UNINITIALIZED ) )
		{
			/* If the Atari load failed, we just reset g_hViewWnd to NULL and the
			   Atari will be rebooted when the screen is refreshed in OnDraw */
			RestartEmulation( TRUE );
		}
	}
	UpdateIndicator( ID_INDICATOR_SIO );
	CleanScreen();

	ResumeThread();
} /* #OF# CMainFrame::OnAtariSettings */

/*========================================================
Method   : CMainFrame::OnAtariPerformance
=========================================================*/
/* #FN#
   Displays Performance dialog box */
void
/* #AS#
   Nothing */
CMainFrame::
OnAtariPerformance()
{
	SuspendThread();

	CPerformanceDlg dlgPerformance( this );

	dlgPerformance.DoModal();
	CleanScreen();

	ResumeThread();
} /* #OF# CMainFrame::OnAtariPerformance */

/*========================================================
Method   : CMainFrame::OnAtariFullSpeed
=========================================================*/
/* #FN#
   Toggles a full speed mode between on and off */
void
/* #AS#
   Nothing */
CMainFrame::
OnAtariFullSpeed()
{
	if( StreamWarning( IDS_WARN_RECORD_FULLSPEED, SRW_VIDEO_STREAM ) )
	{
		if( !g_Screen.bLocked ) /* This option may be toggled by joystick button */
			Misc_ToggleFullSpeed();
		
		if( m_pMainApp )
			m_pMainApp->ResetLoopCounter();

		UpdateStatus();
	}
} /* #OF# CMainFrame::OnAtariFullSpeed */

/*========================================================
Method   : CMainFrame::OnAtariPause
=========================================================*/
/* #FN#
   Toggles a pause mode between on and off */
void
/* #AS#
   Nothing */
CMainFrame::
OnAtariPause()
{
	if( !g_Screen.bLocked ) /* This option may be toggled by joystick button */
		Misc_TogglePause();

	UpdateStatus();

} /* #OF# CMainFrame::OnAtariPause */

/*========================================================
Method   : CMainFrame::OnAtariSioPatch
=========================================================*/
/* #FN#
   Toggles a SIO patch between on and off */
void
/* #AS#
   Nothing */
CMainFrame::
OnAtariSioPatch()
{
	SuspendThread();

	if( !g_Screen.bLocked ) /* This option may be toggled by joystick button */
		Misc_ToggleSIOPatch();

	UpdateStatus( FALSE, -1, ID_INDICATOR_SIO );

	ResumeThread();
} /* #OF# CMainFrame::OnAtariSioPatch */

/*========================================================
Method   : CMainFrame::OnAtariHPatch
=========================================================*/
/* #FN#
   Toggles a H patch between on and off */
void
/* #AS#
   Nothing */
CMainFrame::
OnAtariHPatch()
{
	SuspendThread();

	Devices_enable_h_patch = Devices_enable_h_patch ? 0 : 1;
	WriteRegDWORD( NULL, REG_ENABLE_H_PATCH, Devices_enable_h_patch );

	ESC_UpdatePatches();

	ResumeThread();
} /* #OF# CMainFrame::OnAtariHPatch */

/*========================================================
Method   : CMainFrame::OnAtariPPatch
=========================================================*/
/* #FN#
   Toggles a P patch between on and off */
void
/* #AS#
   Nothing */
CMainFrame::
OnAtariPPatch()
{
	SuspendThread();

	Devices_enable_p_patch = Devices_enable_p_patch ? 0 : 1;
	WriteRegDWORD( NULL, REG_ENABLE_P_PATCH, Devices_enable_p_patch );

	ESC_UpdatePatches();

	ResumeThread();
} /* #OF# CMainFrame::OnAtariPPatch */

/*========================================================
Method   : CMainFrame::OnAtariRPatch
=========================================================*/
/* #FN#
   Toggles a R patch between on and off */
void
/* #AS#
   Nothing */
CMainFrame::
OnAtariRPatch()
{
	SuspendThread();

	Devices_enable_r_patch = Devices_enable_r_patch ? 0 : 1;
	WriteRegDWORD( NULL, REG_ENABLE_R_PATCH, Devices_enable_r_patch );

	RDevice_UpdatePatches();
	ESC_UpdatePatches();

	ResumeThread();
} /* #OF# CMainFrame::OnAtariRPatch */

/*========================================================
Method   : CMainFrame::OnAtariDisableBasic
=========================================================*/
/* #FN#
   Toggles a BASIC activity between on and off */
void
/* #AS#
   Nothing */
CMainFrame::
OnAtariDisableBasic()
{
	SuspendThread();

	Atari800_disable_basic = Atari800_disable_basic ? 0 : 1;
	WriteRegDWORD( NULL, REG_DISABLE_BASIC, Atari800_disable_basic );
	if ( _IsFlagSet( g_Misc.ulState, MS_REBOOT_WHEN_BASIC) )
		OnAtariColdstart();

	ResumeThread();
} /* #OF# CMainFrame::OnAtariDisableBasic */

/*========================================================
Method   : CMainFrame::OnAtariRomImages
=========================================================*/
/* #FN#
   Displays ROM Images dialog box */
void
/* #AS#
   Nothing */
CMainFrame::
OnAtariRomImages()
{
	SuspendThread();

	CRomImagesDlg dlgRomImages( this );

	if( IDOK == dlgRomImages.DoModal() )
	{
		if( CRomImagesDlg::RBT_NONE != dlgRomImages.m_eReboot ||
			_IsFlagSet( g_ulAtariState, ATARI_UNINITIALIZED ) )
		{
			if( CRomImagesDlg::RBT_RESTART == dlgRomImages.m_eReboot )
			{
				RestartEmulation( TRUE );
			}
			else
			{
				if( _IsFlagSet( g_Misc.ulState, MS_REBOOT_WHEN_CART ) )
					/* Fortunately coldstart is all what we need here */
					OnAtariColdstart();
			}
		}
	}
	CleanScreen();

	ResumeThread();
} /* #OF# CMainFrame::OnAtariRomImages */

/*========================================================
Method   : CMainFrame::OnAtariDiskDrives
=========================================================*/
/* #FN#
   Displays Drive Selections dialog box */
void
/* #AS#
   Nothing */
CMainFrame::
OnAtariDiskDrives()
{
	SuspendThread();

	CDriveDlg dlgDrive( this );

	dlgDrive.DoModal();
	CleanScreen();

	ResumeThread();
} /* #OF# CMainFrame::OnAtariDiskDrives */

/*========================================================
Method   : CMainFrame::OnAtariTapeControl
=========================================================*/
/* #FN#
   Displays Cassette Control dialog box */
void
/* #AS#
   Nothing */
CMainFrame::
OnAtariTapeControl()
{
	SuspendThread();

	CTapeDlg dlgTape( this );

	dlgTape.DoModal();
	CleanScreen();

	ResumeThread();
} /* #OF# CMainFrame::OnAtariTapeControl */

/*========================================================
Method   : CMainFrame::OnAtariHardDisks
=========================================================*/
/* #FN#
   Displays Pick Virtual Harddisk Directories dialog box */
void
/* #AS#
   Nothing */
CMainFrame::
OnAtariHardDisks()
{
	SuspendThread();

	CHarddiskDlg dlgHardDisk( this );

	dlgHardDisk.DoModal();
	CleanScreen();

	ResumeThread();
} /* #OF# CMainFrame::OnAtariHardDisks */

/*========================================================
Method   : CMainFrame::OnAtariWarmstart
=========================================================*/
/* #FN#
   Forces a warm start of an emulated machine */
void
/* #AS#
   Nothing */
CMainFrame::
OnAtariWarmstart()
{
	SuspendThread();

	if( Atari800_MACHINE_5200 == Atari800_machine_type ) /* 5200 has no warmstart */
	{
		OnAtariColdstart();
		ResumeThread();
		return;
	}

	if( _IsFlagSet( g_ulAtariState, ATARI_UNINITIALIZED ) )
	{
		if( StreamWarning( IDS_WARN_RECORD_RESET, SRW_VIDEO_STREAM | SRW_SOUND_STREAM ) )
		{
			/* If the Atari load failed, we just reset g_hViewWnd to NULL and the
			   Atari will be rebooted when the screen is refreshed in OnDraw */
			RestartEmulation( TRUE );
		}
	}
	else
	{
		if( _IsFlagSet( g_ulAtariState, ATARI_CRASHED ) )
		{
			g_ulAtariState = ATARI_RUNNING;
			ANTIC_wsync_halt = 0;	/* Turn on CPU */
			g_nTestVal = 0;
			/* OnSetFocus won't invoke that */
			Sound_Restart();
			CleanScreen( TRUE );
		}
		/* Do warm reset */
		Atari800_Warmstart();
	}
	UpdateStatus();

	ResumeThread();
} /* #OF# CMainFrame::OnAtariWarmstart */

/*========================================================
Method   : CMainFrame::OnAtariColdstart
=========================================================*/
/* #FN#
   Forces a cold start of an emulated machine */
void
/* #AS#
   Nothing */
CMainFrame::
OnAtariColdstart()
{
	SuspendThread();

	if( _IsFlagSet( g_ulAtariState, ATARI_UNINITIALIZED ) )
	{
		if( StreamWarning( IDS_WARN_RECORD_RESET, SRW_VIDEO_STREAM | SRW_SOUND_STREAM ) )
		{
			/* If the Atari load failed, we just reset g_hViewWnd to NULL and the
			   Atari will be rebooted when the screen is refreshed in OnDraw */
			RestartEmulation( TRUE );
		}
	}
	else
	{
		if( _IsFlagSet( g_ulAtariState, ATARI_CRASHED ) )
		{
			g_ulAtariState = ATARI_RUNNING;
			ANTIC_wsync_halt = 0;	/* Turn on CPU */
			g_nTestVal = 0;
			/* OnSetFocus won't invoke that */
			Sound_Restart();
			CleanScreen( TRUE );
		}
		/* Do cold reset */
		Atari800_Coldstart();
	}
	UpdateStatus();

	ResumeThread();
} /* #OF# CMainFrame::OnAtariColdstart */

/*========================================================
Method   : CMainFrame::OnViewOptions
=========================================================*/
/* #FN#
   Displays Graphics Options dialog box */
void
/* #AS#
   Nothing */
CMainFrame::
OnViewOptions()
{
	SuspendThread();

	CGraphicsDlg dlgGraphics( this );

	if( IDOK == dlgGraphics.DoModal() )
	{
		/* Show the window because if we changed DDraw modes the
		   frame will be missing until forced to redraw here */
		ShowWindow( SW_SHOWNORMAL );

		if( dlgGraphics.m_bModeChanged )
		{
			Screen_ChangeMode( FALSE );
		}
	}
	CleanScreen();

	ResumeThread();
} /* #OF# CMainFrame::OnViewOptions */

/*========================================================
Method   : CMainFrame::OnViewToggleModes
=========================================================*/
/* #FN#
   Toggles between windowed and full-screen modes */
void
/* #AS#
   Nothing */
CMainFrame::
OnViewToggleModes()
{
	if( StreamWarning( IDS_WARN_RECORD_DISPLAY, SRW_VIDEO_STREAM ) )
	{
		if ( _IsFlagSet( g_Screen.ulState, SM_MODE_WIND) ) {
			StopThread();
			if( !Screen_ToggleModes() ) {
				Screen_SetSafeDisplay( TRUE );
				if ( _IsFlagSet( g_Misc.ulState, MS_THREADED_EMULATION) ) {
					StartThread();
					if ( !EmuThread ) {
						_ClrFlag( g_Misc.ulState, MS_THREADED_EMULATION );
						ThreadState = FALSE;
					}
				}
			}
		}
		else {
			if( !Screen_ToggleModes() )
				Screen_SetSafeDisplay( TRUE );
			if ( _IsFlagSet( g_Misc.ulState, MS_THREADED_EMULATION) ) {
				StartThread();
				if ( !EmuThread ) {
					_ClrFlag( g_Misc.ulState, MS_THREADED_EMULATION );
					ThreadState = FALSE;
				}
			}
		}

		/* There should be an info about pause mode on the status bar
		   in a full-screen mode and none about it in a windowed mode */
		UpdateStatus( _IsFlagSet( g_ulAtariState, ATARI_PAUSED ) );
	}
} /* #OF# CMainFrame::OnViewToggleModes */

/*========================================================
Method   : CMainFrame::OnViewGdiForWindows
=========================================================*/
/* #FN#
   Switches between GDI and DirectDraw for windowed modes */
void
/* #AS#
   Nothing */
CMainFrame::
OnViewGdiForWindows()
{
	if( StreamWarning( IDS_WARN_RECORD_DISPLAY, SRW_VIDEO_STREAM ) )
	{
		_ToggleFlag( g_Screen.ulState, SM_OPTN_USE_GDI );
		WriteRegDWORD( NULL, REG_SCREEN_STATE, g_Screen.ulState );

		Screen_ChangeMode( FALSE );
	}
} /* #OF# CMainFrame::OnViewGdiForWindows */

/*========================================================
Method   : CMainFrame::OnViewShowDiskActivity
=========================================================*/
/* #FN#
   Turns on the disk activity led displaying on the status bar */
void
/* #AS#
   Nothing */
CMainFrame::
OnViewShowDiskActivity()
{
	_ToggleFlag( g_Misc.ulState, MS_SHOW_DRIVE_LED );
	WriteRegDWORD( NULL, REG_MISC_STATE, g_Misc.ulState );
	Screen_show_disk_led = _IsFlagSet( g_Misc.ulState, MS_SHOW_DRIVE_LED ) ? 1 : 0;

} /* #OF# CMainFrame::OnViewShowDiskActivity */

/*========================================================
Method   : CMainFrame::OnViewShowSectorCounter
=========================================================*/
/* #FN#
   Turns on the indicators displaying on the status bar */
void
/* #AS#
   Nothing */
CMainFrame::
OnViewShowSectorCounter()
{
	_ToggleFlag( g_Misc.ulState, MS_SHOW_SECTOR_COUNTER );
	WriteRegDWORD( NULL, REG_MISC_STATE, g_Misc.ulState );
	Screen_show_sector_counter = _IsFlagSet( g_Misc.ulState, MS_SHOW_SECTOR_COUNTER ) ? 1 : 0;

} /* #OF# CMainFrame::OnViewShowSectorCounter */

/*========================================================
Method   : CMainFrame::OnViewShowStateIndicators
=========================================================*/
/* #FN#
   Turns on the indicators displaying on the status bar */
void
/* #AS#
   Nothing */
CMainFrame::
OnViewShowStateIndicators()
{
	_ToggleFlag( g_Misc.ulState, MS_SHOW_INDICATORS );
	WriteRegDWORD( NULL, REG_MISC_STATE, g_Misc.ulState );

	m_wndStatusBar.ConfigureTray( indicators, sizeof(indicators)/sizeof(UINT), !ST_FLIPPED_BUFFERS );

	if( ST_FLIPPED_BUFFERS )
		/* Remove the status bar from the screen */
		CleanScreen();

} /* #OF# CMainFrame::OnViewShowStateIndicators */

/*========================================================
Method   : CMainFrame::StretchModeChanged
=========================================================*/
/* #FN#
   Makes an emulated device changing visible for the emulator */
void
/* #AS#
   Nothing */
CMainFrame::
StretchModeChanged(
	int nStretchMode
)
{
	if( g_Screen.nStretchMode != nStretchMode )
	{
		g_Screen.nStretchMode = nStretchMode;
		WriteRegDWORD( NULL, REG_STRETCH_MODE, g_Screen.nStretchMode );

		Screen_ChangeMode( FALSE );
	}
} /* #OF# CMainFrame::StretchModeChanged */

/*========================================================
Method   : CMainFrame::OnViewStretchModePixelDoubling
=========================================================*/
/* #FN#
   Turn the pixel-doubling on */
void
/* #AS#
   Nothing */
CMainFrame::
OnViewStretchModePixelDoubling()
{
	if( StreamWarning( IDS_WARN_RECORD_DISPLAY, SRW_VIDEO_STREAM ) )
	{
		StretchModeChanged( STRETCH_PIXELDOUBLING );
	}
} /* #OF# CMainFrame::OnViewStretchModePixelDoubling */

/*========================================================
Method   : CMainFrame::OnViewStretchModeInterpolation
=========================================================*/
/* #FN#
   Turn the interpolation on */
void
/* #AS#
   Nothing */
CMainFrame::
OnViewStretchModeInterpolation()
{
	if( StreamWarning( IDS_WARN_RECORD_DISPLAY, SRW_VIDEO_STREAM ) )
	{
		StretchModeChanged( STRETCH_INTERPOLATION );
		Screen_PrepareInterp( FALSE );
	}
} /* #OF# CMainFrame::OnViewStretchModeInterpolation */

/*========================================================
Method   : CMainFrame::OnViewStretchModeScanlines
=========================================================*/
/* #FN#
   Turn the scanlines on */
void
/* #AS#
   Nothing */
CMainFrame::
OnViewStretchModeScanlines()
{
	if( StreamWarning( IDS_WARN_RECORD_DISPLAY, SRW_VIDEO_STREAM ) )
	{
		StretchModeChanged( STRETCH_SCANLINES );
	}
} /* #OF# CMainFrame::OnViewStretchModeScanlines */

/*========================================================
Method   : CMainFrame::OnViewStretchHiEnd
=========================================================*/
/* #FN#
   Turn the scanlines on */
void
/* #AS#
   Nothing */
CMainFrame::
OnViewStretchModeHiEnd()
{
	if( StreamWarning( IDS_WARN_RECORD_DISPLAY, SRW_VIDEO_STREAM ) )
	{
		StretchModeChanged( STRETCH_HIEND );
	}
} /* #OF# CMainFrame::OnViewStretchModeHiEnd */

/*========================================================
Method   : CMainFrame::OnViewArtifacting
=========================================================*/
/* #FN#
   Flips artifacting modes */
void
/* #AS#
   Nothing */
CMainFrame::
OnViewArtifacting()
{
	if( ++ANTIC_artif_mode > 4 )
		ANTIC_artif_mode = 0;
	ANTIC_UpdateArtifacting();

	WriteRegDWORD( NULL, REG_ARTIF_MODE, ANTIC_artif_mode );

} /* #OF# CMainFrame::OnViewArtifacting */

/*========================================================
Method   : CMainFrame::OnViewArtifactingNone
=========================================================*/
/* #FN#
   Turns off artifacting */
void
/* #AS#
   Nothing */
CMainFrame::
OnViewArtifactingNone()
{
	ANTIC_artif_mode = 0;

	ANTIC_UpdateArtifacting();
	WriteRegDWORD( NULL, REG_ARTIF_MODE, ANTIC_artif_mode );

} /* #OF# CMainFrame::OnViewArtifactingNone */

/*========================================================
Method   : CMainFrame::OnViewArtifactingBluebrown1
=========================================================*/
/* #FN#
   Turns on the blue-brown 1 artifacting */
void
/* #AS#
   Nothing */
CMainFrame::
OnViewArtifactingBluebrown1()
{
	ANTIC_artif_mode = 1;

	ANTIC_UpdateArtifacting();
	WriteRegDWORD( NULL, REG_ARTIF_MODE, ANTIC_artif_mode );

} /* #OF# CMainFrame::OnViewArtifactingBluebrown1 */

/*========================================================
Method   : CMainFrame::OnViewArtifactingBluebrown2
=========================================================*/
/* #FN#
   Turns on the blue-brown 2 artifacting */
void
/* #AS#
   Nothing */
CMainFrame::
OnViewArtifactingBluebrown2()
{
	ANTIC_artif_mode = 2;

	ANTIC_UpdateArtifacting();
	WriteRegDWORD( NULL, REG_ARTIF_MODE, ANTIC_artif_mode );

} /* #OF# CMainFrame::OnViewArtifactingBluebrown2 */

/*========================================================
Method   : CMainFrame::OnViewArtifactingGtia
=========================================================*/
/* #FN#
   Turns on the GTIA artifacting */
void
/* #AS#
   Nothing */
CMainFrame::
OnViewArtifactingGtia()
{
	ANTIC_artif_mode = 3;

	ANTIC_UpdateArtifacting();
	WriteRegDWORD( NULL, REG_ARTIF_MODE, ANTIC_artif_mode );

} /* #OF# CMainFrame::OnViewArtifactingGtia */

/*========================================================
Method   : CMainFrame::OnViewArtifactingCtia
=========================================================*/
/* #FN#
   Turns on the CTIA artifacting */
void
/* #AS#
   Nothing */
CMainFrame::
OnViewArtifactingCtia()
{
	ANTIC_artif_mode = 4;

	ANTIC_UpdateArtifacting();
	WriteRegDWORD( NULL, REG_ARTIF_MODE, ANTIC_artif_mode );

} /* #OF# CMainFrame::OnViewArtifactingCtia */

/*========================================================
Method   : CMainFrame::OnViewPalette
=========================================================*/
/* #FN#
   Displays the "Palette Options" dialog box */
void
/* #AS#
   Nothing */
CMainFrame::
OnViewPalette()
{
	SuspendThread();

	CPaletteDlg dlgPalette( this );

	dlgPalette.DoModal();
	CleanScreen();

	ResumeThread();
} /* #OF# CMainFrame::OnViewPalette */

/*========================================================
Method   : CMainFrame::OnViewSaveScreenshotNormal
=========================================================*/
/* #FN#
   Saves normal screen snapshot */
void
/* #AS#
   Nothing */
CMainFrame::
OnViewSaveScreenshotNormal()
{
	SuspendThread();
	SaveSnapshot( 0 );
	ResumeThread();
} /* #OF# CMainFrame::OnViewSaveScreenshotNormal */

/*========================================================
Method   : CMainFrame::OnViewSaveScreenshotInterlaced
=========================================================*/
/* #FN#
   Saves interlaced screen snapshot */
void
/* #AS#
   Nothing */
CMainFrame::
OnViewSaveScreenshotInterlaced()
{
	SuspendThread();
	SaveSnapshot( 1 );
	ResumeThread();
} /* #OF# CMainFrame::OnViewSaveScreenshotInterlaced */

/*========================================================
Method   : CMainFrame::SaveVideo
=========================================================*/
/* #FN#
   Saves the output of video to a file */
void
/* #AS#
   Nothing */
CMainFrame::
SaveVideo(
	BOOL bWithSound
)
{
	if( g_Screen.pfOutput ) /* Close Video Output file */
	{
		Video_CloseOutput(); /* Turn off the AVI output */
	}
	else
	{
		if( DisplayWarning( IDS_WARN_VIDEOFILE, DONT_SHOW_VIDEOFILE_WARN, TRUE ) )
		{
			if( PickFileName( FALSE, m_strVideoName, IDS_SELECT_AVI_SAVE, IDS_FILTER_AVI,
							  "avi", PF_SAVE_FLAGS, FALSE, DEFAULT_VID ) &&
				!m_strVideoName.IsEmpty() )
			{
				/* Open Video Output file */
				Video_OpenOutput( (LPSTR)(LPCSTR)m_strVideoName, bWithSound );
			}
		}
	}
	UpdateIndicator( ID_INDICATOR_AVI );
	CleanScreen();

} /* #OF# CMainFrame::SaveVideo */

/*========================================================
Method   : CMainFrame::OnViewSaveVideo
=========================================================*/
/* #FN#
   Saves the output of video to a file */
void
/* #AS#
   Nothing */
CMainFrame::
OnViewSaveVideo()
{
	SuspendThread();
	_ClrFlag( g_Misc.ulState, MS_VIDEO_AND_SOUND );
	SaveVideo( FALSE );
	ResumeThread();
} /* #OF# CMainFrame::OnViewSaveVideo */

/*========================================================
Method   : CMainFrame::OnViewSaveVideoAndSound
=========================================================*/
/* #FN#
   Saves the output of video and sound to a file */
void
/* #AS#
   Nothing */
CMainFrame::
OnViewSaveVideoAndSound()
{
	SuspendThread();
	_SetFlag( g_Misc.ulState, MS_VIDEO_AND_SOUND );
	SaveVideo( TRUE );
	ResumeThread();
} /* #OF# CMainFrame::OnViewSaveVideoAndSound */

/*========================================================
Method   : CMainFrame::OnViewPerformanceTest
=========================================================*/
/* #FN#
   Renders speed of the current graphics mode */
void
/* #AS#
   Nothing */
CMainFrame::
OnViewPerformanceTest()
{
	SuspendThread();

	if( IDOK == DisplayMessage( NULL, IDS_GFX_TEST_BEGIN, 0, MB_ICONINFORMATION | MB_OKCANCEL ) )
	{
		char   szReport[ LOADSTRING_SIZE_S + 1 ] = { " " };
		ULONG  ulStartTime;
		ULONG  ulTotalTime;
		double nResult;

		/* Clean the whole screen before drawing the test */
		CleanScreen( ST_ATARI_STOPPED );

		ulStartTime = timeGetTime();

		for( int i = 0; i < 30; i++ )
			PLATFORM_DisplayScreen();

		ulTotalTime = timeGetTime() - ulStartTime;
		nResult = (double)(ulTotalTime / 30.0f);

		if( nResult > 20.0f )
			_LoadStringSx( IDS_WARN_SPEED, szReport );

		DisplayMessage( NULL, IDS_GFX_TEST_RESULT, IDS_BENCHMARK_RESULTS, MB_ICONINFORMATION | MB_OK, ulTotalTime, nResult, szReport );

		/* We don't need the CleanScreen invocation here, because
		   the DisplayMessage function has done this work for us */
	}
	else
		CleanScreen();

	ResumeThread();
} /* #OF# CMainFrame::OnViewPerformanceTest */

/*========================================================
Method   : CMainFrame::OnSoundOptions
=========================================================*/
/* #FN#
   Displays Sound Options dialog box */
void
/* #AS#
   Nothing */
CMainFrame::
OnSoundOptions()
{
	SuspendThread();

	CSoundDlg dlgSound( this );

	if( IDOK == dlgSound.DoModal() )
	{
		if( dlgSound.m_bModeChanged )
		{
			Sound_Initialise( TRUE );
		}
	}
	CleanScreen();

	ResumeThread();
} /* #OF# CMainFrame::OnSoundOptions */

/*========================================================
Method   : CMainFrame::OnSoundStereo
=========================================================*/
/* #FN#
   Toggles a stereo support between on and off */
void
/* #AS#
   Nothing */
CMainFrame::
OnSoundStereo()
{
	if( _IsFlagSet( g_Misc.ulState, MS_VIDEO_AND_SOUND ) ) /* When streaming video with sound */
	{
		if( !StreamWarning( IDS_WARN_RECORD_SNDOUT, SRW_VIDEO_STREAM ) )
			return;
	}

	SuspendThread();

	if( StreamWarning( IDS_WARN_RECORD_SNDOUT, SRW_SOUND_STREAM ) )
	{
		POKEYSND_stereo_enabled = POKEYSND_stereo_enabled ? 0 : 1;
		WriteRegDWORD( NULL, REG_ENABLE_STEREO, POKEYSND_stereo_enabled );

		Sound_Initialise( FALSE );
	}

	ResumeThread();
} /* #OF# CMainFrame::OnSoundStereo */

/*========================================================
Method   : CMainFrame::OnSoundMute
=========================================================*/
/* #FN#
   Toggles a sound playback between on and off */
void
/* #AS#
   Nothing */
CMainFrame::
OnSoundMute()
{
	SuspendThread();

	if( StreamWarning( IDS_WARN_RECORD_SNDOUT, SRW_SOUND_STREAM ) )
	{
		if( _IsFlagSet( g_Sound.ulState, SS_NO_SOUND ) )
		{
			_ClrFlag( g_Sound.ulState, SS_NO_SOUND );
			/* Registry is corrupted? */
			if( !_IsFlagSet( g_Sound.ulState, SS_MM_SOUND | SS_DS_SOUND ) )
				_SetFlag( g_Sound.ulState, SS_MM_SOUND );
		}
		else
			_SetFlag( g_Sound.ulState, SS_NO_SOUND );

		WriteRegDWORD( NULL, REG_SOUND_STATE, g_Sound.ulState );

		Sound_Initialise( FALSE );
	}

	ResumeThread();
} /* #OF# CMainFrame::OnSoundMute */

/*========================================================
Method   : CMainFrame::OnSoundSaveSound
=========================================================*/
/* #FN#
   Saves the output of the Pokey processor to a file */
void
/* #AS#
   Nothing */
CMainFrame::
OnSoundSaveSound()
{
	SuspendThread();

	if( g_Sound.pfOutput ) /* Close Sound Output file */
	{
		Sound_CloseOutput(); /* Turn off the sound */
	}
	else
	{
		if( _IsFlagSet( g_Sound.ulState, SS_NO_SOUND ) )
			DisplayMessage( NULL, IDS_SFX_RECORD, 0, MB_ICONINFORMATION | MB_OK );

		if( DisplayWarning( IDS_WARN_SOUNDFILE, DONT_SHOW_SOUNDFILE_WARN, TRUE ) )
		{
			if( PickFileName( FALSE, m_strSoundName, IDS_SELECT_WAV_SAVE, IDS_FILTER_WAV,
							  "wav", PF_SAVE_FLAGS, FALSE, DEFAULT_SND ) &&
				!m_strSoundName.IsEmpty() )
			{
				/* Open Sound Output file */
				Sound_OpenOutput( (LPSTR)(LPCSTR)m_strSoundName );
			}
		}
	}
	UpdateIndicator( ID_INDICATOR_WAV );
	CleanScreen();

	ResumeThread();
} /* #OF# CMainFrame::OnSoundSaveSound */

/*========================================================
Method   : CMainFrame::OnSoundPerformanceTest
=========================================================*/
/* #FN#
   Tests the Pokey processor overhead with current settings */
void
/* #AS#
   Nothing */
CMainFrame::
OnSoundPerformanceTest()
{
	SuspendThread();

	if( !_IsFlagSet( g_ulAtariState, ATARI_RUNNING ) || _IsFlagSet( g_Sound.ulState, SS_NO_SOUND ) )
	{
		DisplayMessage( NULL, IDS_SFX_TEST_WARNING, 0, MB_ICONINFORMATION | MB_OK );
	}
	else
	{
		int    n16BitSnd   = _IsFlagSet( g_Sound.ulState, SS_16BIT_AUDIO ) ? 1 : 0;
		int    nSampleSize = (g_Sound.nRate / (Atari800_TV_PAL == Atari800_tv_mode ? g_Timer.nPalFreq : g_Timer.nNtscFreq)) << n16BitSnd;
		UCHAR *pszBuffer   = NULL;

		if( pszBuffer = (UCHAR*)calloc( 1, nSampleSize + 1 ) )
		{
			ULONG ulStartTime;
			ULONG ulTotalTime;

			if( IDOK == DisplayMessage( NULL, IDS_SFX_TEST_BEGIN, 0, MB_ICONINFORMATION | MB_OKCANCEL ) )
			{
				ulStartTime = timeGetTime();
				/* Begin the performance test */
				for( int i = 0; i < 60; i++ )
					POKEYSND_Process_ptr( pszBuffer, nSampleSize >> n16BitSnd );

				ulTotalTime = timeGetTime() - ulStartTime;
				free( pszBuffer );

				DisplayMessage( NULL, IDS_SFX_TEST_RESULT, IDS_BENCHMARK_RESULTS, MB_ICONINFORMATION | MB_OK, nSampleSize, ulTotalTime, (float)(ulTotalTime / 60.0f) );
			}
		}
	}
	CleanScreen();

	ResumeThread();
} /* #OF# CMainFrame::OnSoundPerformanceTest */

/*========================================================
Method   : CMainFrame::OnInputCaptureMouse
=========================================================*/
/* #FN#
   Toggles the mouse pointer capturing between on and off */
void
/* #AS#
   Nothing */
CMainFrame::
OnInputCaptureMouse()
{
	SuspendThread();
	Input_ToggleMouseCapture();
	UpdateIndicator( ID_INDICATOR_MSE );
	ResumeThread();
} /* #OF# CMainFrame::OnInputCaptureMouse */

/*========================================================
Method   : CMainFrame::OnInputJoysticks
=========================================================*/
/* #FN#
   Displays Joystick Options dialog box */
void
/* #AS#
   Nothing */
CMainFrame::
OnInputJoysticks()
{
	SuspendThread();

	CJoystickDlg dlgJoystick( this );

	dlgJoystick.DoModal();
	CleanScreen();

	ResumeThread();
} /* #OF# CMainFrame::OnInputJoysticks */

/*========================================================
Method   : CMainFrame::OnInputMouse
=========================================================*/
/* #FN#
   Displays Mouse Options dialog box */
void
/* #AS#
   Nothing */
CMainFrame::
OnInputMouse()
{
	SuspendThread();

	CMouseDlg dlgMouse( this );

	dlgMouse.DoModal();
	CleanScreen();

	ResumeThread();
} /* #OF# CMainFrame::OnInputMouse */

/*========================================================
Method   : CMainFrame::OnInputKeyboard
=========================================================*/
/* #FN#
   Displays Keyboard Templates dialog box */
void
/* #AS#
   Nothing */
CMainFrame::
OnInputKeyboard()
{
	SuspendThread();

	CKeyboardDlg dlgKeyboard( this );

	dlgKeyboard.DoModal();
	CleanScreen();

	ResumeThread();
} /* #OF# CMainFrame::OnInputKeyboard */

/*========================================================
Method   : CMainFrame::ChangeAutofire
=========================================================*/
/* #FN#
   Changes the autofire mode */
void
/* #AS#
   Nothing */
CMainFrame::
ChangeAutofire(
	BOOL bForward /*=TRUE*/
)
{
	int nAutofireMode = g_Input.Joy.nAutoMode;

	if( bForward )
	{
		if( ++nAutofireMode > INPUT_AUTOFIRE_CONT )
			nAutofireMode = INPUT_AUTOFIRE_OFF;
	}
	else
		if( --nAutofireMode < INPUT_AUTOFIRE_OFF )
			nAutofireMode = INPUT_AUTOFIRE_CONT;

	AutofireChanged( nAutofireMode );

} /* #OF# CMainFrame::ChangeAutofire */

/*========================================================
Method   : CMainFrame::AutofireChanged
=========================================================*/
/* #FN#
   Makes the autofire mode changing visible for the emulator */
void
/* #AS#
   Nothing */
CMainFrame::
AutofireChanged(
	int nAutofireMode
)
{
	for( int i = 0; i < MAX_ATARI_JOYPORTS; i++ )
	{
		if( g_Input.Joy.ulAutoSticks & (1 << i) )
			INPUT_joy_autofire[ i ] = nAutofireMode;
	}
	if( g_Input.Joy.nAutoMode != nAutofireMode )
	{
		g_Input.Joy.nAutoMode = nAutofireMode;
		WriteRegDWORD( NULL, REG_AUTOFIRE_MODE, g_Input.Joy.nAutoMode );

		UpdateIndicator( ID_INDICATOR_JOY );
	}
} /* #OF# CMainFrame::AutofireChanged */


/*========================================================
Method   : CMainFrame::OnInputAutofire
=========================================================*/
/* #FN#
   Flips autofire modes */
void
/* #AS#
   Nothing */
CMainFrame::
OnInputAutofire()
{
	ChangeAutofire();

} /* #OF# CMainFrame::OnInputAutofire */

/*========================================================
Method   : CMainFrame::OnInputAutofireOff
=========================================================*/
/* #FN#
   Turns the autofire off */
void
/* #AS#
   Nothing */
CMainFrame::
OnInputAutofireOff()
{
	AutofireChanged( INPUT_AUTOFIRE_OFF );

} /* #OF# CMainFrame::OnInputAutofireOff */

/*========================================================
Method   : CMainFrame::OnInputAutofireFireDependent
=========================================================*/
/* #FN#
   Switches the autofire mode to fire dependent */
void
/* #AS#
   Nothing */
CMainFrame::
OnInputAutofireFireDependent()
{
	AutofireChanged( INPUT_AUTOFIRE_FIRE );

} /* #OF# CMainFrame::OnInputAutofireFireDependent */

/*========================================================
Method   : CMainFrame::OnInputAutofireAllTime
=========================================================*/
/* #FN#
   Switches the autofire mode to continuous */
void
/* #AS#
   Nothing */
CMainFrame::
OnInputAutofireAllTime()
{
	AutofireChanged( INPUT_AUTOFIRE_CONT );

} /* #OF# CMainFrame::OnInputAutofireAllTime */

/*========================================================
Method   : CMainFrame::ChangeMouseDevice
=========================================================*/
/* #FN#
   Changes a device emulated with mouse */
void
/* #AS#
   Nothing */
CMainFrame::
ChangeMouseDevice(
	BOOL bForward /*=TRUE*/
)
{
	int nMouseMode = INPUT_mouse_mode;

	if( bForward )
	{
		if( ++nMouseMode > INPUT_MOUSE_JOY )
			nMouseMode = INPUT_MOUSE_OFF;
	}
	else
		if( --nMouseMode < INPUT_MOUSE_OFF )
			nMouseMode = INPUT_MOUSE_JOY;

	MouseDeviceChanged( nMouseMode );

} /* #OF# CMainFrame::ChangeMouseDevice */

/*========================================================
Method   : CMainFrame::MouseDeviceChanged
=========================================================*/
/* #FN#
   Makes an emulated device changing visible for the emulator */
void
/* #AS#
   Nothing */
CMainFrame::
MouseDeviceChanged(
	int nMouseMode
)
{
	if( INPUT_mouse_mode != nMouseMode )
	{
		INPUT_mouse_mode = nMouseMode;
		WriteRegDWORD( NULL, REG_MOUSE_MODE, INPUT_mouse_mode );

		UpdateIndicator( ID_INDICATOR_MSE );
	}
} /* #OF# CMainFrame::MouseDeviceChanged */

/*========================================================
Method   : CMainFrame::OnInputMouseDeviceNone
=========================================================*/
/* #FN#
   Turn the mouse emulation off */
void
/* #AS#
   Nothing */
CMainFrame::
OnInputMouseDeviceNone()
{
	MouseDeviceChanged( INPUT_MOUSE_OFF );

} /* #OF# CMainFrame::OnInputMouseDeviceNone */

/*========================================================
Method   : CMainFrame::OnInputMouseDevicePaddles
=========================================================*/
/* #FN#
   Switches the mouse emulation to paddles */
void
/* #AS#
   Nothing */
CMainFrame::
OnInputMouseDevicePaddles()
{
	MouseDeviceChanged( INPUT_MOUSE_PAD );

} /* #OF# CMainFrame::OnInputMouseDevicePaddles */

/*========================================================
Method   : CMainFrame::OnInputMouseDeviceTouchTablet
=========================================================*/
/* #FN#
   Switches the mouse emulation to an Atari touch tablet */
void
/* #AS#
   Nothing */
CMainFrame::
OnInputMouseDeviceTouchTablet()
{
	MouseDeviceChanged( INPUT_MOUSE_TOUCH );

} /* #OF# CMainFrame::OnInputMouseDeviceTouchTablet */

/*========================================================
Method   : CMainFrame::OnInputMouseDeviceKoalaPad
=========================================================*/
/* #FN#
   Switches the mouse emulation to a Koala pad */
void
/* #AS#
   Nothing */
CMainFrame::OnInputMouseDeviceKoalaPad()
{
	MouseDeviceChanged( INPUT_MOUSE_KOALA );

} /* #OF# CMainFrame::OnInputMouseDeviceKoalaPad */

/*========================================================
Method   : CMainFrame::OnInputMouseDeviceLightPen
=========================================================*/
/* #FN#
   Switches the mouse emulation to a light pen */
void
/* #AS#
   Nothing */
CMainFrame::
OnInputMouseDeviceLightPen()
{
	MouseDeviceChanged( INPUT_MOUSE_PEN );

} /* #OF# CMainFrame::OnInputMouseDeviceLightPen */

/*========================================================
Method   : CMainFrame::OnInputMouseDeviceLightGun
=========================================================*/
/* #FN#
   Switches the mouse emulation to a light gun */
void
/* #AS#
   Nothing */
CMainFrame::
OnInputMouseDeviceLightGun()
{
	MouseDeviceChanged( INPUT_MOUSE_GUN );

} /* #OF# CMainFrame::OnInputMouseDeviceLightGun */

/*========================================================
Method   : CMainFrame::OnInputMouseDeviceAmigaMouse
=========================================================*/
/* #FN#
   Switches the mouse emulation to an Amiga mouse */
void
/* #AS#
   Nothing */
CMainFrame::
OnInputMouseDeviceAmigaMouse()
{
	MouseDeviceChanged( INPUT_MOUSE_AMIGA );

} /* #OF# CMainFrame::OnInputMouseDeviceAmigaMouse */

/*========================================================
Method   : CMainFrame::OnInputMouseDeviceStMouse
=========================================================*/
/* #FN#
   Switches the mouse emulation to a ST mouse */
void
/* #AS#
   Nothing */
CMainFrame::
OnInputMouseDeviceStMouse()
{
	MouseDeviceChanged( INPUT_MOUSE_ST );

} /* #OF# CMainFrame::OnInputMouseDeviceStMouse */

/*========================================================
Method   : CMainFrame::OnInputMouseDeviceTrakBall
=========================================================*/
/* #FN#
   Switches the mouse emulation to an Atari trak-ball */
void
/* #AS#
   Nothing */
CMainFrame::
OnInputMouseDeviceTrakBall()
{
	MouseDeviceChanged( INPUT_MOUSE_TRAK );

} /* #OF# CMainFrame::OnInputMouseDeviceTrakBall */

/*========================================================
Method   : CMainFrame::OnInputMouseDeviceJoystick
=========================================================*/
/* #FN#
   Switches the mouse emulation to a joystick */
void
/* #AS#
   Nothing */
CMainFrame::
OnInputMouseDeviceJoystick()
{
	MouseDeviceChanged( INPUT_MOUSE_JOY );

} /* #OF# CMainFrame::OnInputMouseDeviceJoystick */

/*========================================================
Method   : CMainFrame::ArrowKeysChanged
=========================================================*/
/* #FN#
   Makes PC arrow keys assigment changing visible for the emulator */
void
/* #AS#
   Nothing */
CMainFrame::
ArrowKeysChanged(
	int nArrowsMode
)
{
	if( g_Input.Key.nArrowsMode != nArrowsMode )
	{
		g_Input.Key.nArrowsMode = nArrowsMode;
		WriteRegDWORD( NULL, REG_ARROWS_MODE, g_Input.Key.nArrowsMode );

		Input_SetArrowKeys( g_Input.Key.nArrowsMode );
	}
} /* #OF# CMainFrame::ArrowKeysChanged */

/*========================================================
Method   : CMainFrame::OnInputArrowKeysArrows
=========================================================*/
/* #FN#
   Switches the PC arrows assigment to arrows */
void
/* #AS#
   Nothing */
CMainFrame::
OnInputArrowKeysArrows()
{
	ArrowKeysChanged( ARROWKEYS_ARROWS );

} /* #OF# CMainFrame::OnInputArrowKeysArrows */

/*========================================================
Method   : CMainFrame::OnInputArrowKeysControlArrows
=========================================================*/
/* #FN#
   Switches the PC arrows assigment to control + arrows */
void
/* #AS#
   Nothing */
CMainFrame::
OnInputArrowKeysControlArrows()
{
	ArrowKeysChanged( ARROWKEYS_CTRLARROWS );

} /* #OF# CMainFrame::OnInputArrowKeysControlArrows */

/*========================================================
Method   : CMainFrame::OnInputArrowKeysF1F4
=========================================================*/
/* #FN#
   Switches the PC arrows assigment to function keys */
void
/* #AS#
   Nothing */
CMainFrame::
OnInputArrowKeysF1F4()
{
	ArrowKeysChanged( ARROWKEYS_F1F4 );

} /* #OF# CMainFrame::OnInputArrowKeysF1F4 */

/*========================================================
Method   : CMainFrame::OnInputNetworkGame
=========================================================*/
/* #FN#
   Displays Network Game Options dialog box */
void
/* #AS#
   Nothing */
CMainFrame::
OnInputNetworkGame()
{
	SuspendThread();

#ifdef WIN_NETWORK_GAMES
	CKailleraDlg dlgKaillera( this );

	if( IDOK == dlgKaillera.DoModal() )
	{
		Kaillera_GameStart();
	}
	CleanScreen();
#endif /*WIN_NETWORK_GAMES*/

	ResumeThread();
} /* #OF# CMainFrame::OnInputNetworkGame */

/*========================================================
Method   : CMainFrame::OnMiscFileAssociations
=========================================================*/
/* #FN#
   Displays the "File Associations" dialog box */
void
/* #AS#
   Nothing */
CMainFrame::
OnMiscFileAssociations()
{
	SuspendThread();

	CFileAssociationsDlg dlgFileAssociations( this );

	dlgFileAssociations.DoModal();
	CleanScreen();

	ResumeThread();
} /* #OF# CMainFrame::OnMiscFileAssociations */

/*========================================================
Method   : CMainFrame::OnMiscConvertDcmToAtr
=========================================================*/
/* #FN#
   Converts .DCM files to .ATR files */
void
/* #AS#
   Nothing */
CMainFrame::
OnMiscConvertDcmToAtr()
{
	ConvertDcmToAtr();
	CleanScreen();

} /* #OF# CMainFrame::OnMiscConvertDcmToAtr */

/*========================================================
Method   : CMainFrame::ConvertDcmToAtr
=========================================================*/
/* #FN#
   Does convertion between .DCM files and .ATR ones */
void
/* #AS#
   Nothing */
CMainFrame::
ConvertDcmToAtr()
{
	char  szLocalFileBuffer[ 8192 ] = { '\0' };
	char  szPrompt[ LOADSTRING_SIZE_S + 1 ];
	char  szFilter[ LOADSTRING_SIZE_M + 1 ];
	DWORD dwFlags = OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_HIDEREADONLY;

	if( _IsFlagSet( g_Screen.ulState, SM_MODE_FULL ) )
		dwFlags |= OFN_ENABLEHOOK;

	CFileDialog	dlgDiskImage( TRUE, NULL, NULL, dwFlags, _LoadStringMx( IDS_FILTER_DCM, szFilter ), this );

	dlgDiskImage.m_ofn.lpstrTitle = _LoadStringSx( IDS_SELECT_DCM_CONV, szPrompt );
	dlgDiskImage.m_ofn.lpstrFile  = szLocalFileBuffer;
	dlgDiskImage.m_ofn.nMaxFile   = 8192;
	if( _IsFlagSet( g_Screen.ulState, SM_MODE_FULL ) )
		dlgDiskImage.m_ofn.lpfnHook = FileDialogHookProc;

	if( IDOK == dlgDiskImage.DoModal() )
	{
		char     szDestDir[ MAX_PATH + 1 ];
		CString  strInput;
		FILE    *fpInput, *fpOutput;
		int      i;

		strcpy( szDestDir, dlgDiskImage.GetPathName() );
		for( i = strlen( szDestDir ); i > 0 && szDestDir[ i ] != '\\' && szDestDir[i] != '.'; i-- );
		if( i == 0 || szDestDir[ i ] != '.' )
			i = strlen( szDestDir );
		strcpy( &szDestDir[ i ], ".atr" );

		dwFlags = OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_HIDEREADONLY;

		if( _IsFlagSet( g_Screen.ulState, SM_MODE_FULL ) )
			dwFlags |= OFN_ENABLEHOOK;

		CFileDialog	dlgDestDir( FALSE, NULL, szDestDir, dwFlags, NULL, this );
		
		dlgDestDir.m_ofn.lpstrTitle = _LoadStringSx( IDS_SELECT_OUT_SAVE, szPrompt );
		if( _IsFlagSet( g_Screen.ulState, SM_MODE_FULL ) )
			dlgDestDir.m_ofn.lpfnHook = FileDialogHookProc;

		if( IDOK == dlgDestDir.DoModal()  )
		{
			strcpy( szDestDir, dlgDestDir.GetPathName() );

			for( i = strlen( szDestDir ); i > 0 && szDestDir[ i ] != '\\'; i-- );
			szDestDir[ i ] = '\0';

			if( *szDestDir != '\0' )
			{
				POSITION pos = dlgDiskImage.GetStartPosition();
				ASSERT(pos);
				if( !pos )
					return;

				strInput = dlgDiskImage.GetNextPathName( pos );
				while( strInput != "" )
				{
					char szFullDestName[ MAX_PATH + 1 ];
					char szThisFileName[ MAX_PATH + 1 ];

					strcpy( szThisFileName, strInput );
					fpInput = fopen( szThisFileName, "rb" );
					if( !fpInput )
					{
						DisplayMessage( NULL, IDS_ERROR_FILE_OPEN, 0, MB_ICONEXCLAMATION | MB_OK, szThisFileName );
						return;
					}
					for( i = strlen( szThisFileName ); i > 0 && szThisFileName[ i ]!= '\\'; i-- );
					if( szThisFileName[ i ] == '\\' )
						i++;

					strcpy( szFullDestName, szDestDir );
					strcat( szFullDestName, "\\" );
					strcat( szFullDestName, &szThisFileName[ i ] );

					for( i = strlen( szFullDestName ); i > 0 && szFullDestName[ i ] != '\\' && szFullDestName[ i ] != '.'; i-- );
					if( i == 0 || szFullDestName[ i ] != '.' )
						i = strlen( szFullDestName );
					strcpy( &szFullDestName[ i ], ".atr" );

					fpOutput = fopen( szFullDestName, "rb" );
					if( fpOutput )
					{
						if( IDNO == DisplayMessage( NULL, IDS_WARN_FILE_EXISTS, 0, MB_ICONQUESTION | MB_YESNO, szFullDestName ) )
						{
							fclose( fpInput );
							return;
						}
						fclose( fpOutput );
					}
					fpOutput = fopen( szFullDestName, "wb" );
					if( !fpOutput )
					{
						DisplayMessage( NULL, IDS_DCM2ATR_MSG3, 0, MB_ICONEXCLAMATION | MB_OK, szFullDestName );
						return;
					}
					if( !CompFile_DCMtoATR( fpInput, fpOutput ) )
					{
						int nResult;
						nResult = DisplayMessage( NULL, IDS_DCM2ATR_MSG1, 0, MB_ICONQUESTION | MB_YESNO );
						fclose( fpInput );
						fclose( fpOutput );
						if( nResult == IDYES )
						{
							CErrorLogDlg dlgErrorLog( this );
							dlgErrorLog.DoModal();
						}
						return;
					}
					fclose( fpOutput );
					fclose( fpInput );
					strInput = (pos ? dlgDiskImage.GetNextPathName( pos ) : "");
				}
			}
			DisplayMessage( NULL, IDS_DCM2ATR_MSG2, 0, MB_ICONINFORMATION | MB_OK );
		}
	}
} /* #OF# CMainFrame::ConvertDcmToAtr */

/*========================================================
Method   : CMainFrame::OnMiscConvertXfdToAtr
=========================================================*/
/* #FN#
   Converts .XFD file to .ATR one */
void
/* #AS#
   Nothing */
CMainFrame::
OnMiscConvertXfdToAtr()
{
	ConvertXfdToAtr();
	CleanScreen();

} /* #OF# CMainFrame::OnMiscConvertXfdToAtr */

/*========================================================
Method   : CMainFrame::ConvertXfdToAtr
=========================================================*/
/* #FN#
   Does convertion between .XFD file and .ATR one */
void
/* #AS#
   Nothing */
CMainFrame::
ConvertXfdToAtr()
{
	char szSrcFile[ MAX_PATH + 1 ] = { '\0' };

	/* Choose XFD file to load */
	if( PickFileName( TRUE, szSrcFile, IDS_SELECT_XFD_CONV, IDS_FILTER_XFD,
					  "xfd", PF_LOAD_FLAGS ) &&
		*szSrcFile != '\0' )
	{
		int  nBootType = XFD2ATR_BOOT_LOGICAL;
		char szDstFile[ MAX_PATH + 1 ] = { '\0' };

		int  nResult = XFD2ATR_Open( szSrcFile );

		if( XFD2ATR_OK != nResult )
		{
			nBootType = -1;

			/* Select the boot sectors type */
			if( XFD2ATR_QUERY_BOOT == nResult )
			{
				CBootTypeDlg dlgBootType;

				if( IDOK == dlgBootType.DoModal() )
					nBootType = dlgBootType.GetBootType();
			}
			else
				DisplayMessage( NULL, XFD2ATR_ERR_XFD_CORRUPT == nResult ? IDS_ERROR_XFD_CORRUPT : IDS_ERROR_XFD_READ, 0, MB_ICONEXCLAMATION | MB_OK );
		}
		/* Save the ATR file if there were no errors */
		if( -1 != nBootType )
		{
			/* Choose ATR file name to save */
			if( PickFileName( FALSE, szDstFile, IDS_SELECT_ATR_SAVE, IDS_FILTER_ATR,
							  "atr", PF_SAVE_FLAGS, FALSE ) &&
				*szDstFile != '\0' )
			{
				if( XFD2ATR_OK != XFD2ATR_Convert( szDstFile, nBootType ) )
					DisplayMessage( NULL, IDS_ERROR_ATR_WRITE, 0, MB_ICONEXCLAMATION | MB_OK );
			}
		}
		XFD2ATR_Close();
	}
} /* #OF# CMainFrame::ConvertXfdToAtr */

/*========================================================
Method   : CMainFrame::OnMiscConvertRomToCart
=========================================================*/
/* #FN#
   Converts .ROM/.BIN file to .CAR one */
void
/* #AS#
   Nothing */
CMainFrame::
OnMiscConvertRomToCart()
{
	ConvertRomToCart();
	CleanScreen();

} /* #OF# CMainFrame::OnMiscConvertRomToCart */

/*========================================================
Method   : CMainFrame::ConvertRomToCart
=========================================================*/
/* #FN#
   Does convertion between .ROM/.BIN file and .CAR one */
void
/* #AS#
   Nothing */
CMainFrame::
ConvertRomToCart()
{
	char szFileName[ MAX_PATH + 1 ];

	_strncpy( szFileName, g_szOtherRom, MAX_PATH );
	GetFolderPath( szFileName, NULL );

	if( PickFileName( TRUE, szFileName, IDS_SELECT_ROM_CONV, IDS_FILTER_ROM,
					  "rom", PF_LOAD_FLAGS ) &&
		*szFileName != '\0' )
	{
		UBYTE *pImage = new UBYTE[ CARTRIDGE_MAX_SIZE + 1 ];

		if( pImage )
		{
			int   nBytes = 0;
			FILE *fFile  = NULL;

			fFile = fopen( szFileName, "rb" );
			if( !fFile )
			{
				DisplayMessage( NULL, IDS_ERROR_R2C_READ, 0, MB_ICONEXCLAMATION | MB_OK );
				return;
			}
			nBytes = fread( pImage, 1, CARTRIDGE_MAX_SIZE + 1, fFile );
			fclose( fFile );

			if( (nBytes & 0x3ff) == 0 )
			{
				int nCartType = SelectCartType( nBytes / 1024 );
				if( nCartType != CARTRIDGE_NONE )
				{
					int nCheckSum = CARTRIDGE_Checksum( pImage, nBytes );
					int i;

					for( i = strlen( szFileName ) - 1; i > 0 && szFileName[ i ] != '.' && szFileName[ i ] != '\\'; i-- );
					if( i > 0 )
					{
						if( szFileName[ i ] == '.' )
							szFileName[ i ] = '\0';
						strcat( szFileName, ".car" );
					}

					if( nCartType != CARTRIDGE_NONE &&
						PickFileName( FALSE, szFileName, IDS_SELECT_CAR_SAVE, IDS_FILTER_CAR,
									  "car", PF_SAVE_FLAGS, FALSE ) &&
						*szFileName != '\0' )
					{
						struct {
							UBYTE ubId      [ 4 ];
							UBYTE ubType    [ 4 ];
							UBYTE ubCheckSum[ 4 ];
							UBYTE ubGash    [ 4 ];
						} header;

						header.ubId       [ 0 ] = 'C';
						header.ubId       [ 1 ] = 'A';
						header.ubId       [ 2 ] = 'R';
						header.ubId       [ 3 ] = 'T';
						header.ubType     [ 0 ] = (nCartType >> 24) & 0xff;
						header.ubType     [ 1 ] = (nCartType >> 16) & 0xff;
						header.ubType     [ 2 ] = (nCartType >> 8)  & 0xff;
						header.ubType     [ 3 ] = nCartType & 0xff;
						header.ubCheckSum [ 0 ] = (nCheckSum >> 24) & 0xff;
						header.ubCheckSum [ 1 ] = (nCheckSum >> 16) & 0xff;
						header.ubCheckSum [ 2 ] = (nCheckSum >> 8)  & 0xff;
						header.ubCheckSum [ 3 ] = nCheckSum & 0xff;
						header.ubGash     [ 0 ] = '\0';
						header.ubGash     [ 1 ] = '\0';
						header.ubGash     [ 2 ] = '\0';
						header.ubGash     [ 3 ] = '\0';

						fFile = fopen( szFileName, "wb" );
						if( !fFile )
						{
							DisplayMessage( NULL, IDS_ERROR_R2C_SAVE, 0, MB_ICONEXCLAMATION | MB_OK );
							return;
						}
						fwrite( &header, 1, sizeof(header), fFile );
						fwrite( pImage, 1, nBytes, fFile );
						fclose( fFile );
					}
				}
			}
		}
		if( pImage )
			delete [] pImage;
	}
} /* #OF# CMainFrame::ConvertRomToCart */

/*========================================================
Method   : CMainFrame::OnMiscConvertAsciiToAtascii
=========================================================*/
/* #FN#
   Converts a PC file to Atari one */
void
/* #AS#
   Nothing */
CMainFrame::
OnMiscConvertAsciiToAtascii()
{
	ConvertAtasciiToAscii( FALSE );
	CleanScreen();

} /* #OF# CMainFrame::OnMiscConvertAsciiToAtascii */

/*========================================================
Method   : CMainFrame::OnMiscConvertAtasciiToAscii
=========================================================*/
/* #FN#
   Converts an Atari file to PC one */
void
/* #AS#
   Nothing */
CMainFrame::
OnMiscConvertAtasciiToAscii()
{
	ConvertAtasciiToAscii( TRUE );
	CleanScreen();

} /* #OF# CMainFrame::OnMiscConvertAtasciiToAscii */

/*========================================================
Method   : CMainFrame::ConvertAtasciiToAscii
=========================================================*/
/* #FN#
   Does conversion between Atari files and PC ones */
void
/* #AS#
   Nothing */
CMainFrame::
ConvertAtasciiToAscii(
	BOOL bA2PC /* #IN# TRUE if Atari to PC conversion is requested, otherwise FALSE */
)
{
	char szSrcFile[ MAX_PATH + 1 ] = { '\0' };
	/* Choose a text file to load */
	if( PickFileName( TRUE, szSrcFile, IDS_SELECT_TXT_CONV, IDS_FILTER_TXT,
					  "txt", PF_LOAD_FLAGS ) &&
		*szSrcFile != '\0' )
	{
		/* Select the conversion type */
		CConvertTypeDlg dlgConvertType( bA2PC );

		if( IDOK == dlgConvertType.DoModal() )
		{
			char szDstFile[ MAX_PATH + 1 ] = { '\0' };
			/* Choose a text file name to save */
			if( PickFileName( FALSE, szDstFile, IDS_SELECT_TXT_SAVE, IDS_FILTER_TXT,
							  "txt", PF_SAVE_FLAGS, FALSE ) &&
				*szDstFile != '\0' )
			{
				/* Do the convertion work */
				CFile cfSrc, cfDst;
				/* We'll use a CFileException object to get error information */
				CFileException e;
				char szError[ 1024 ];

				if( !cfSrc.Open( szSrcFile, CFile::modeRead | CFile::shareDenyWrite, &e ) )
				{
					/* Complain if an error happened no need to delete the e object */
					e.GetErrorMessage( szError, 1024 );
					DisplayMessage( NULL, IDS_ERROR_A2PC_READ, 0, MB_ICONEXCLAMATION | MB_OK, szError );
					return;
				}
				else
				{
					if( !cfDst.Open( szDstFile, CFile::modeWrite | CFile::shareExclusive | CFile::modeCreate, &e ) )
					{
						e.GetErrorMessage( szError, 1024 );
						DisplayMessage( NULL, IDS_ERROR_A2PC_SAVE, 0, MB_ICONEXCLAMATION | MB_OK, szError );

						cfSrc.Close();
						return;
					}
					UBYTE ubSrcBuf[ 4096 ];
					UBYTE ubDstBuf[ 8192 ];
					DWORD dwSrcLen, dwDstLen;

					/* Read in 4096-byte blocks.
					   This loop will ends when there are no more bytes to read */
					do
					{
						dwSrcLen = cfSrc.Read( ubSrcBuf, 4096 );

						/* Convert EOLs */
						dwDstLen = dlgConvertType.Atascii2Ascii( ubSrcBuf, ubDstBuf, dwSrcLen, bA2PC );

						/* Convert national "bushes" if requested */
						dlgConvertType.A8Std2Win1250( ubDstBuf,
													  ubDstBuf,
													  dwDstLen,
													  dlgConvertType.GetConvertType(),
													  bA2PC );

						/* Write the bunch to the output */
						cfDst.Write( ubDstBuf, dwDstLen );
					}
					while( dwSrcLen > 0 );

					/* Close both files */
					cfDst.Close();
					cfSrc.Close();
				}
			}
		}
	}
} /* #OF# CMainFrame::ConvertAtasciiToAscii */

/*========================================================
Method   : CMainFrame::OnMiscPauseInactive
=========================================================*/
/* #FN#
   Toggles the Pause Inactive option */
void
/* #AS#
   Nothing */
CMainFrame::
OnMiscPauseInactive()
{
	_ToggleFlag( g_Misc.ulState, MS_STOP_WHEN_NO_FOCUS );
	WriteRegDWORD( NULL, REG_MISC_STATE, g_Misc.ulState );

} /* #OF# CMainFrame::OnMiscPauseInactive */

/*========================================================
Method   : CMainFrame::OnMiscReuseWindow
=========================================================*/
/* #FN#
   Toggles the Reuse Window option */
void
/* #AS#
   Nothing */
CMainFrame::
OnMiscReuseWindow()
{
	_ToggleFlag( g_Misc.ulState, MS_REUSE_WINDOW );
	WriteRegDWORD( NULL, REG_MISC_STATE, g_Misc.ulState );

} /* #OF# CMainFrame::OnMiscReuseWindow */

/*========================================================
Method   : CMainFrame::OnMiscHighPriority
=========================================================*/
/* #FN#
   Toggles the High Priority option */
void
/* #AS#
   Nothing */
CMainFrame::
OnMiscHighPriority()
{
	_ToggleFlag( g_Misc.ulState, MS_HIGH_PRIORITY );
	WriteRegDWORD( NULL, REG_MISC_STATE, g_Misc.ulState );

	Misc_SetProcessPriority();

} /* #OF# CMainFrame::OnMiscHighPriority */

/*========================================================
Method   : CMainFrame::OnMiscConfirmOnExit
=========================================================*/
/* #FN#
   Toggles the Confirm On Exit option */
void
/* #AS#
   Nothing */
CMainFrame::
OnMiscConfirmOnExit()
{
	_ToggleFlag( g_Misc.ulState, MS_CONFIRM_ON_EXIT );
	WriteRegDWORD( NULL, REG_MISC_STATE, g_Misc.ulState );

} /* #OF# CMainFrame::OnMiscConfirmOnExit */

/*========================================================
Method   : CMainFrame::OnMiscThreadedEmulation
=========================================================*/
/* #FN#
   Switches between GDI and DirectDraw for windowed modes */
void
/* #AS#
   Nothing */
CMainFrame::
OnMiscThreadedEmulation()
{
	if ( _IsFlagSet( g_Misc.ulState, MS_THREADED_EMULATION ) ) {
		if ( _IsFlagSet( g_Screen.ulState, SM_MODE_WIND) )
			StopThread();
		_ClrFlag( g_Misc.ulState, MS_THREADED_EMULATION );
	}
	else {
		if ( _IsFlagSet( g_Screen.ulState, SM_MODE_WIND) ) {
			StartThread();
			if ( EmuThread ) {
				_SetFlag( g_Misc.ulState, MS_THREADED_EMULATION );
			}
			else ThreadState = FALSE;
		}
		else
			_SetFlag( g_Misc.ulState, MS_THREADED_EMULATION );
	}
} /* #OF# CMainFrame::OnMiscThreadedEmulation */

/*========================================================
Method   : CMainFrame::OnMiscCheatOptions
=========================================================*/
/* #FN#
   Displays Cheat Options dialog box */
void
/* #AS#
   Nothing */
CMainFrame::
OnMiscCheatOptions()
{
	SuspendThread();

	CCheatServer *pCheatServer = m_pMainApp->GetCheatServer();
	/* Let's make rollback available */
	CCheatServer cheatServer( *pCheatServer );

	CCheatDlg dlgCheat( pCheatServer, this );

	if( IDCANCEL == dlgCheat.DoModal() )
	{
		if( dlgCheat.m_bTrainerChanged )
			/* Cancel changes the user has made */
			*pCheatServer = cheatServer;
	}
	CleanScreen();

	ResumeThread();
} /* #OF# CMainFrame::OnMiscCheatOptions */

/*========================================================
Method   : CMainFrame::OnMiscDisableCollisions
=========================================================*/
/* #FN#
   Toggles a sprite collisions detection between on and off */
void
/* #AS#
   Nothing */
CMainFrame::
OnMiscDisableCollisions()
{
	_ToggleFlag( g_Misc.ulState, MS_DISABLE_COLLISIONS );
	WriteRegDWORD( NULL, REG_MISC_STATE, g_Misc.ulState );
	Misc_UpdateCollisions();
} /* #OF# CMainFrame::OnMiscDisableCollisions */

/*========================================================
Method   : CMainFrame::OnMiscClearAllSettings
=========================================================*/
/* #FN#
   Clears all saved settings/drives/ROMs */
void
/* #AS#
   Nothing */
CMainFrame::
OnMiscClearAllSettings()
{
	SuspendThread();

//	if( StreamWarning( IDS_WARN_RECORD_RESET, SRW_VIDEO_STREAM | SRW_SOUND_STREAM ) )
//	{
		if( IDYES == DisplayMessage( NULL, IDS_WARN_CLEAR, 0, MB_ICONQUESTION | MB_YESNO ) )
		{
			ULONG ulMiscState = g_Misc.ulState;

			DeleteAllRegKeys( NULL, REGNAME );
			HandleRegistry();

			/* Re-generate a default palette */
			Palette_Generate( g_Screen.Pal.nBlackLevel, g_Screen.Pal.nWhiteLevel, g_Screen.Pal.nSaturation, g_Screen.Pal.nContrast, 
				g_Screen.Pal.nBrightness, g_Screen.Pal.nGamma, _IsFlagSet( g_Misc.ulState, MS_USE_EXT_PALETTE ) );

			/* Apply the changes to the main window */
			Screen_UseAtariPalette( TRUE );

			/* These are not stored in the Registry, but should be re-initialized */
			g_ulAtariState = ATARI_UNINITIALIZED;

			if( g_Misc.ulState != ulMiscState )
				WriteRegDWORD( NULL, REG_MISC_STATE, g_Misc.ulState );

			/* Restart the emulation now */
			RestartEmulation( TRUE );
		}
//	}

	ResumeThread();
} /* #OF# CMainFrame::OnMiscClearAllSettings */

/*========================================================
Method   : CMainFrame::OnMiscRestartEmulation
=========================================================*/
/* #FN#
   Reinitializes the Windows stuff and emulated Atari */
void
/* #AS#
   Nothing */
CMainFrame::
OnMiscRestartEmulation()
{
	SuspendThread();

	if( StreamWarning( IDS_WARN_RECORD_RESET, SRW_VIDEO_STREAM | SRW_SOUND_STREAM ) )
	{
		/* Force Windows stuff initialization */
		RestartEmulation( TRUE );
	}

	ResumeThread();
} /* #OF# CMainFrame::OnMiscRestartEmulation */

/*========================================================
Method   : CMainFrame::OnMiscViewLogFile
=========================================================*/
/* #FN#
   Displays the "Error Log" dialog box */
void
/* #AS#
   Nothing */
CMainFrame::
OnMiscViewLogFile()
{
	CErrorLogDlg dlgErrorLog( this );

	dlgErrorLog.DoModal();
	CleanScreen();

} /* #OF# CMainFrame::OnMiscViewLogFile */

/*========================================================
Method   : CMainFrame::OnMiscMonitor
=========================================================*/
/* #FN#
   Launches a monitor console */
void
/* #AS#
   Nothing */
CMainFrame::
OnMiscMonitor()
{
	SuspendThread();

	/* Launch the monitor, switching to windowed mode if necessary */
	Misc_LaunchMonitor();

	/* Update state description */
	UpdateStatus();
	/* Clean up the screen */
	CleanScreen();

	ResumeThread();
} /* #OF# CMainFrame::OnMiscMonitor */

/*========================================================
Method   : CMainFrame::OnMiscMonitorAsk
=========================================================*/
/* #FN#
   Enables the ask dialog after Atari crash */
void
/* #AS#
   Nothing */
CMainFrame::
OnMiscMonitorAsk()
{
	_ClrFlag( g_Misc.ulState, MS_MONITOR_ALWAYS );
	_ClrFlag( g_Misc.ulState, MS_MONITOR_NEVER );
	WriteRegDWORD( NULL, REG_MISC_STATE, g_Misc.ulState );

} /* #OF# CMainFrame::OnMiscMonitorAsk */

/*========================================================
Method   : CMainFrame::OnMiscMonitorOpen
=========================================================*/
/* #FN#
   Monitor will always open after Atari crash */
void
/* #AS#
   Nothing */
CMainFrame::
OnMiscMonitorOpen()
{
	_SetFlag( g_Misc.ulState, MS_MONITOR_ALWAYS );
	_ClrFlag( g_Misc.ulState, MS_MONITOR_NEVER );
	WriteRegDWORD( NULL, REG_MISC_STATE, g_Misc.ulState );

} /* #OF# CMainFrame::OnMiscMonitorOpen */

/*========================================================
Method   : CMainFrame::OnMiscMonitorStop
=========================================================*/
/* #FN#
   Monitor will never open after Atari crashes */
void
/* #AS#
   Nothing */
CMainFrame::
OnMiscMonitorStop()
{
	_ClrFlag( g_Misc.ulState, MS_MONITOR_ALWAYS );
	_SetFlag( g_Misc.ulState, MS_MONITOR_NEVER );
	WriteRegDWORD( NULL, REG_MISC_STATE, g_Misc.ulState );

} /* #OF# CMainFrame::OnMiscMonitorStop */

/*========================================================
Method   : CMainFrame::OnContextHelp
=========================================================*/
/* #FN#
   Enables context-sensitive help mode */
void
/* #AS#
   Nothing */
CMainFrame::
OnContextHelp()
{
	if( _IsFlagSet( g_Screen.ulState, SM_MODE_FULL ) )
		return;

	/* Go to context help mode (show the "question mark" mouse pointer) */
	CFrameWnd::OnContextHelp();

} /* #OF# CMainFrame::OnContextHelp */

/*========================================================
Method   : CMainFrame::OnHelpFinder
=========================================================*/
/* #FN#
   Displays the "Index" help page */
void
/* #AS#
   Nothing */
CMainFrame::
OnHelpFinder()
{
	/* Unfortunately, there are some problems with automatic return to
	   flipped full-screen mode, go to windowed instead */
	if( _IsFlagSet( g_Screen.ulState, SM_MODE_FULL ) /*&& _IsFlagSet( g_Screen.ulState, SM_OPTN_FLIP_BUFFERS )*/ )
		Screen_SetSafeDisplay( FALSE );

	/* Invoke the help contents page */
	CFrameWnd::OnHelpFinder();

} /* #OF# CMainFrame::OnHelpFinder */

/*========================================================
Method   : CMainFrame::OnHelp
=========================================================*/
/* #FN#
   Displays the help contents */
void
/* #AS#
   Nothing */
CMainFrame::
OnHelp()
{
	if( _IsFlagSet( g_Screen.ulState, SM_MODE_FULL ) )
		Screen_SetSafeDisplay( FALSE );

	/* Invoke the help contents page */
	CFrameWnd::OnHelp();

} /* #OF# CMainFrame::OnHelp */

/*========================================================
Method   : CMainFrame::OnHelpKeyboardLayout
=========================================================*/
/* #FN#
   Displays the "Keyboard Layout" help page */
void
/* #AS#
   Nothing */
CMainFrame::
OnHelpKeyboardLayout()
{
	if( _IsFlagSet( g_Screen.ulState, SM_MODE_FULL ) )
		Screen_SetSafeDisplay( FALSE );

	/* Invoke the default keyboard layout help topic */
	AfxGetApp()->WinHelp( HID_BASE_COMMAND + ID_HELP_KEYBOARD_LAYOUT, HELP_CONTEXT );

} /* #OF# CMainFrame::OnHelpKeyboardLayout */

/*========================================================
Method   : CMainFrame::OnHelpKeyboardShortcuts
=========================================================*/
/* #FN#
   Displays the "Keyboard Shortcuts" help page */
void
/* #AS#
   Nothing */
CMainFrame::
OnHelpKeyboardShortcuts()
{
	if( _IsFlagSet( g_Screen.ulState, SM_MODE_FULL ) )
		Screen_SetSafeDisplay( FALSE );

	/* Invoke the keyboard shortcuts help topic */
	AfxGetApp()->WinHelp( HID_BASE_COMMAND + ID_HELP_KEYBOARD_SHORTCUTS, HELP_CONTEXT );

} /* #OF# CMainFrame::OnHelpKeyboardShortcuts */

/*========================================================
Method   : CMainFrame::LaunchWizard
=========================================================*/
/* #FN#
   Handles the Setup Wizard creating and using */
BOOL
/* #AS#
   Nothing */
CMainFrame::
LaunchWizard(
	CWnd *pWnd,
	BOOL &bReboot
)
{
	BOOL bResult = TRUE;

	CWizardDlg dlgWizard( pWnd );

	CWizardStep1 dlgStep1;
	CWizardStep2 dlgStep2;
	CWizardStep3 dlgStep3;

	dlgWizard.AddPage( &dlgStep1 );
	dlgWizard.AddPage( &dlgStep2 );
	dlgWizard.AddPage( &dlgStep3 );

	if( IDOK == dlgWizard.DoModal() )
	{
		/* Commit all the wizard settings */
		dlgWizard.Commit();

		if( dlgWizard.Reboot() )
			bReboot = TRUE;
	}
	else
		bResult = FALSE;

	return bResult;

} /* #OF# CMainFrame::LaunchWizard */

/*========================================================
Method   : CMainFrame::OnHelpSetupWizard
=========================================================*/
/* #FN#
   Displays the Setup Wizard window */
void
/* #AS#
   Nothing */
CMainFrame::
OnHelpSetupWizard()
{
	SuspendThread();

	BOOL bReboot = FALSE;

	if( _IsFlagSet( g_Screen.ulState, SM_MODE_FULL ) )
		Screen_SetSafeDisplay( FALSE );

	LaunchWizard( this, bReboot );

	if( bReboot || _IsFlagSet( g_ulAtariState, ATARI_UNINITIALIZED ) )
		RestartEmulation( TRUE );

	CleanScreen();

	ResumeThread();
} /* #OF# CMainFrame::OnHelpSetupWizard */

/*========================================================
Method   : CMainFrame::OnHelpWebPage
=========================================================*/
/* #FN#
   Opens the project home page */
void
/* #AS#
   Nothing */
CMainFrame::
OnHelpWebPage()
{
	char szWebPage[ LOADSTRING_SIZE_S + 1 ];

	if( _LoadStringSx( IDS_WEB_PAGE, szWebPage ) )
	{
		if( _IsFlagSet( g_Screen.ulState, SM_MODE_FULL ) )
			Screen_SetSafeDisplay( FALSE );

		/* Open Windows Explorer */
		ShellExecute( g_hMainWnd, "open", szWebPage, NULL, NULL, SW_SHOWNORMAL );
	}
} /* #OF# CMainFrame::OnHelpWebPage */

/*========================================================
Method   : CMainFrame::OnHelpAbout
=========================================================*/
/* #FN#
   Displays the "About" window */
void
/* #AS#
   Nothing */
CMainFrame::
OnHelpAbout()
{
	CAboutDlg dlgAbout( this );

	dlgAbout.DoModal();
	CleanScreen();

} /* #OF# CMainFrame::OnHelpAbout */

/*========================================================
Method   : CMainFrame::OnTimer
=========================================================*/
/* #FN#
   The framework calls this member function after each interval specified
   in the SetTimer member function used to install a timer */
void
/* #AS#
   Nothing */
CMainFrame::
OnTimer(
	UINT nIDEvent /* #IN# Specifies the identifier of the timer */
)
{
	if( TIMER_READ_JOYSTICK == nIDEvent )
	{
		Input_ReadJoystick( 0, 0 );
		Input_ReadJoystick( 1, 0 );
		Input_ReadJoystick( 2, 0 );
		Input_ReadJoystick( 3, 0 );

		if( !_IsFlagSet( g_ulAtariState, ATARI_PAUSED ) )
			KillTimer( TIMER_READ_JOYSTICK );
	}
	CFrameWnd::OnTimer( nIDEvent );

} /* #OF# CMainFrame::OnTimer */

/*========================================================
Method   : CMainFrame::OnDropFiles
=========================================================*/
/* #FN#
   The framework calls this member function when the user releases
   the left mouse button over a window that has registered itself
   as the recipient of dropped files */
void
/* #AS#
   Nothing */
CMainFrame::
OnDropFiles(
	HDROP hDropInfo
)
{
	SuspendThread();

	BOOL bPauseInactive = _IsFlagSet( g_Misc.ulState, MS_STOP_WHEN_NO_FOCUS );

	/* Retrieve the file names of dropped files that have resulted from
	   a drag-and-drop operation */
	if( ::DragQueryFile( hDropInfo, (UINT)-1, NULL, 0 ) )
	{
		if( ::DragQueryFile( hDropInfo, 0, NULL, 0 ) < MAX_PATH )
		{
			::DragQueryFile( hDropInfo, 0, g_szAutobootFile, MAX_PATH );
		}
	}
	::DragFinish( hDropInfo );

#ifdef WIN_NETWORK_GAMES
	if( !ST_KAILLERA_ACTIVE )
	{
#endif
		/* It is not a good idea to restart the emulation if the 'Pause when
		   inactive' option is enabled and the main window has lost the focus */
		AutobootAtariImage( g_szAutobootFile, !bPauseInactive, !_IsKeyPressed( VK_SHIFT ) );

#ifdef WIN_NETWORK_GAMES
	}
#endif
	UpdateStatus();
//	CFrameWnd::OnDropFiles( hDropInfo );

	ResumeThread();
} /* #OF# CMainFrame::OnDropFiles */

/*========================================================
Method   : CMainFrame::AutobootAtariImage
=========================================================*/
/* #FN#
   Allows to load an Atari image without a bother of its type */
BOOL
/* #AS#
   Nothing */
CMainFrame::
AutobootAtariImage(
	LPSTR pszFileName,
	BOOL  bReinit /*=TRUE*/,
	BOOL  bReboot /*=TRUE*/
)
{
	UINT unFileType = IAF_ATARI_FILE;

	if( IsAtariFile( pszFileName, &unFileType ) )
	{
		if( bReboot || bReinit )
		{
			/* Detach currently used cartridge first */
			CARTRIDGE_Remove();
			strcpy( g_szCurrentRom, FILE_NONE );

			WriteRegString( NULL, REG_ROM_CURRENT, g_szCurrentRom );
			WriteRegDWORD ( NULL, REG_CART_TYPE, CARTRIDGE_type );
		}
		/*
		   Is it an Atari executable?
		*/
		if( IAF_BIN_IMAGE == unFileType )
		{
			if( ST_ATARI_FAILED && bReinit )
				OnAtariColdstart();

			if( !RunExecutable( pszFileName ) )
			{
				DisplayMessage( NULL, IDS_ERROR_BIN_RUN, 0, MB_ICONEXCLAMATION | MB_OK, pszFileName );
			}
		}
		else
		/*
		   Is it an Atari disk image?
		*/
		if( IAF_DSK_IMAGE == unFileType )
		{
			SIO_Dismount( 1 );
			SIO_Mount( 1, pszFileName, _IsFlagSet(g_Misc.ulState, MS_DRIVE_READONLY) );
			WriteRegDrives( NULL );

			if( bReboot && (!ST_ATARI_FAILED || bReinit) ) /* Do coldstart */
				OnAtariColdstart();
		}
		/*
		   Is it an Atari cassette image?
		*/
		if( IAF_CAS_IMAGE == unFileType )
		{
			if( _stricmp( pszFileName, cassette_filename ) != 0 )
				WriteRegString( NULL, REG_FILE_TAPE, pszFileName );

			CASSETTE_Remove(); /* Prevents from resource leaks */
			if( !CASSETTE_Insert( pszFileName ) )
				DisplayMessage( NULL, IDS_ERROR_CAS_READ, 0, MB_ICONEXCLAMATION | MB_OK );
			else
			{
				if( bReboot && (!ST_ATARI_FAILED || bReinit) ) /* Do coldstart */
				{
					CASSETTE_hold_start = CASSETTE_press_space = 1;
					OnAtariColdstart();
				}
			}
		}
		else
		/*
		   Is it an Atari state file?
		*/
		if( IAF_A8S_IMAGE == unFileType )
		{
#ifndef HAVE_LIBZ
			DisplayMessage( NULL, IDS_ERROR_NO_ZLIB, 0, MB_ICONEXCLAMATION | MB_OK );
			return TRUE;
#endif /*HAVE_LIBZ*/
			if( ST_ATARI_FAILED && bReinit )
				OnAtariColdstart();

			if( !RunSnapshot( pszFileName ) )
			{
				DisplayMessage( NULL, IDS_ERROR_A8S_READ, 0, MB_ICONEXCLAMATION | MB_OK );
			}
		}
		else
		/*
		   Is it an Atari cartridge image?
		*/
		if( IAF_ROM_IMAGE == unFileType || IAF_CAR_IMAGE == unFileType )
		{
			if( CARTRIDGE_NONE != AttachCartridge( pszFileName, CARTRIDGE_NONE ) )
			{
				strcpy( g_szOtherRom, g_szCurrentRom );
				WriteRegString( NULL, REG_ROM_OTHER, g_szOtherRom );
			}
			WriteRegString( NULL, REG_ROM_CURRENT, g_szCurrentRom );
			WriteRegDWORD ( NULL, REG_CART_TYPE,   CARTRIDGE_type );

			if( _IsFlagSet( g_Misc.ulState, MS_REBOOT_WHEN_CART ) && bReboot &&
				(!ST_ATARI_FAILED || bReinit) )
			{
				OnAtariColdstart();
			}
		}
		return TRUE;
	}
	return FALSE;

} /* #OF# CMainFrame::AutobootAtariFile */

/*========================================================
Method   : CMainFrame::OnMove
=========================================================*/
/* #FN#
   The framework calls this function after the CWnd object has been moved */
void
/* #AS#
   Nothing */
CMainFrame::
OnMove(
	int x, /* #IN# New x-coordinate location of the upper-left corner of the client area */
	int y  /* #IN# New y-coordinate location of the upper-left corner of the client area */
)
{
	CFrameWnd::OnMove( x, y );

	HWND hViewWnd = g_hViewWnd;
	if( !hViewWnd && m_pMainView )
		hViewWnd = m_pMainView->GetSafeHwnd();

	if( hViewWnd )
	{
		if( _IsFlagSet( g_Screen.ulState, SM_MODE_WIND ) )
		{
			RECT rc;
			::GetClientRect( hViewWnd, &rc );
			/* We must be sure that we are in the windowed mode */
			if( rc.right == ATARI_VIS_WIDTH || rc.right == ATARI_DOUBLE_VIS_WIDTH )
			{
				GetWindowRect( &rc );

				g_nStartX = rc.left;
				g_nStartY = rc.top;
			}
		}
		/* Compute area for DirectDraw display */
		Screen_ComputeClipArea( hViewWnd );
		/* Cache boundaries of the view window */
		Input_RefreshBounds( hViewWnd, FALSE );
	}
	if( g_hViewWnd )
		/* Force screen refreshing */
		g_nTestVal = _GetRefreshRate() - 1;

} /* #OF# CMainFrame::OnMove */

/*========================================================
Method   : CMainFrame::OnSize
=========================================================*/
/* #FN#
   The framework calls this member function after the window’s size has
   changed */
void
/* #AS#
   Nothing */
CMainFrame::
OnSize(
	UINT nType, /* #IN# Type of resizing requested    */
	int cx,     /* #IN# New width of the client area  */
	int cy      /* #IN# New height of the client area */
)
{
	CFrameWnd::OnSize( nType, cx, cy );

	HWND hViewWnd = g_hViewWnd;
	if( !hViewWnd && m_pMainView )
		hViewWnd = m_pMainView->GetSafeHwnd();

	if( hViewWnd )
	{
		/* Compute area for DirectDraw display */
		Screen_ComputeClipArea( hViewWnd );
		/* Cache boundaries of the view window */
		Input_RefreshBounds( hViewWnd, FALSE );
	}
	if( g_hViewWnd )
		/* Force screen refreshing */
		g_nTestVal = _GetRefreshRate() - 1;

} /* #OF# CMainFrame::OnSize */

/*========================================================
Method   : CMainFrame::OnClose
=========================================================*/
/* #FN#
   The framework calls this function as a signal that the CWnd or an
   application is to terminate */
void
/* #AS#
   Nothing */
CMainFrame::
OnClose()
{
	SuspendThread();

	if( StreamWarning( IDS_WARN_RECORD_EXIT, SRW_VIDEO_STREAM | SRW_SOUND_STREAM ) )
	{
		/* Make sure the user wants to drop the emulation ;-) */
		if( _IsFlagSet( g_Misc.ulState, MS_CONFIRM_ON_EXIT ) )
		{
			if( IDNO == DisplayMessage( NULL, IDS_WARN_EXIT, 0, MB_ICONQUESTION | MB_YESNO ) ) {
				ResumeThread();
				return;
			}
		}
		WriteRegDWORD( NULL, REG_START_XPOS, g_nStartX );
		WriteRegDWORD( NULL, REG_START_YPOS, g_nStartY );

#ifdef WIN_NETWORK_GAMES
		if( s_Settings.bLocked )
		{
			/* When the user interrupts emulation when a network game is in
			   progress, this is the only way to restore emulation settings */
			OnMessageKailleraStop( 0, 0 );
		}
#endif
		CFrameWnd::OnClose();
	}
} /* #OF# CMainFrame::OnClose */

/*========================================================
Method   : CMainFrame::OnUpdateFileBootAtariImage
=========================================================*/
/* #FN#
   Updates a state of the menu option */
void
/* #AS#
   Nothing */
CMainFrame::
OnUpdateFileBootAtariImage(
	CCmdUI *pCmdUI /* #IN# The CCmdUI object that handles the update */
)
{
#ifdef WIN_NETWORK_GAMES
	pCmdUI->Enable( !ST_KAILLERA_ACTIVE );
#endif
} /* #OF# CMainFrame::OnUpdateFileBootAtariImage */

/*========================================================
Method   : CMainFrame::OnUpdateFileLoadExecutable
=========================================================*/
/* #FN#
   Updates a state of the menu option */
void
/* #AS#
   Nothing */
CMainFrame::
OnUpdateFileLoadExecutable(
	CCmdUI *pCmdUI /* #IN# The CCmdUI object that handles the update */
)
{
#ifdef WIN_NETWORK_GAMES
	pCmdUI->Enable( !ST_KAILLERA_ACTIVE );
#endif
} /* #OF# CMainFrame::OnUpdateFileLoadExecutable */

/*========================================================
Method   : CMainFrame::OnUpdateFileAttachDiskImageDrive
=========================================================*/
/* #FN#
   Updates a state of the menu option */
void
/* #AS#
   Nothing */
CMainFrame::
OnUpdateFileAttachDiskImageDrive(
	CCmdUI *pCmdUI /* #IN# The CCmdUI object that handles the update */
)
{
#ifdef WIN_NETWORK_GAMES
	if( 0 == (pCmdUI->m_nID - ID_FILE_ATTACHDISKIMAGE_BASE) )
		pCmdUI->Enable( !ST_KAILLERA_ACTIVE );
#endif
} /* #OF# CMainFrame::OnUpdateFileAttachDiskImageDrive */

/*========================================================
Method   : CMainFrame::OnUpdateFileDetachDiskImageAll
=========================================================*/
/* #FN#
   Updates a state of the menu option */
void
/* #AS#
   Nothing */
CMainFrame::
OnUpdateFileDetachDiskImageAll(
	CCmdUI *pCmdUI /* #IN# The CCmdUI object that handles the update */
)
{
	BOOL bEnable = FALSE;
	for( int i = 0; i < SIO_MAX_DRIVES; i++ )
	{
		if( *SIO_filename[ i ] &&
			 strcmp( SIO_filename[ i ], "Off" )   != 0 &&
			(strcmp( SIO_filename[ i ], "Empty" ) != 0 || _IsFlagSet( g_Misc.ulState, MS_TURN_DRIVES_OFF ))
#ifdef WIN_NETWORK_GAMES
			 && (0 != i || !ST_KAILLERA_ACTIVE)
#endif
		  )
		{
			bEnable = TRUE;
			break;
		}
	}
	pCmdUI->Enable( bEnable );

} /* #OF# CMainFrame::OnUpdateFileDetachDiskImageAll */

/*========================================================
Method   : CMainFrame::OnUpdateFileDetachDiskImageDrive
=========================================================*/
/* #FN#
   Updates a state of the menu option */
void
/* #AS#
   Nothing */
CMainFrame::
OnUpdateFileDetachDiskImageDrive(
	CCmdUI *pCmdUI /* #IN# The CCmdUI object that handles the update */
)
{
	int nDriveNum = pCmdUI->m_nID - ID_FILE_DETACHDISKIMAGE_BASE;

	pCmdUI->Enable(
		_IsPathAvailable( SIO_filename[ nDriveNum ] ) &&
		strcmp( SIO_filename[ nDriveNum ], "Off" )   != 0 &&
		strcmp( SIO_filename[ nDriveNum ], "Empty" ) != 0
#ifdef WIN_NETWORK_GAMES
		&& (0 != nDriveNum || !ST_KAILLERA_ACTIVE)
#endif
	);
} /* #OF# CMainFrame::OnUpdateFileDetachDiskImageDrive */

/*========================================================
Method   : CMainFrame::OnUpdateFileTurnDisk
=========================================================*/
/* #FN#
   Updates a state of the menu option */
void
/* #AS#
   Nothing */
CMainFrame::
OnUpdateFileTurnDisk(
	CCmdUI *pCmdUI /* #IN# The CCmdUI object that handles the update */
)
{
	pCmdUI->Enable(
		strcmp( SIO_filename[ 0 ], "Off" )   != 0 &&
		strcmp( SIO_filename[ 0 ], "Empty" ) != 0
#ifdef WIN_NETWORK_GAMES
		&& !ST_KAILLERA_ACTIVE
#endif
	);
} /* #OF# CMainFrame::OnUpdateFileTurnDisk */

/*========================================================
Method   : CMainFrame::OnUpdateFileAttachTapeImage
=========================================================*/
/* #FN#
   Updates a state of the menu option */
void
/* #AS#
   Nothing */
CMainFrame::
OnUpdateFileAttachTapeImage(
	CCmdUI *pCmdUI /* #IN# The CCmdUI object that handles the update */
)
{
#ifdef WIN_NETWORK_GAMES
	pCmdUI->Enable( !ST_KAILLERA_ACTIVE );
#endif
} /* #OF# CMainFrame::OnUpdateFileAttachTapeImage */

/*========================================================
Method   : CMainFrame::OnUpdateFileDetachTapeImage
=========================================================*/
/* #FN#
   Updates a state of the menu option */
void
/* #AS#
   Nothing */
CMainFrame::
OnUpdateFileDetachTapeImage(
	CCmdUI *pCmdUI /* #IN# The CCmdUI object that handles the update */
)
{
	pCmdUI->Enable( _IsPathAvailable( cassette_filename )
#ifdef WIN_NETWORK_GAMES
		&& !ST_KAILLERA_ACTIVE
#endif
	);
} /* #OF# CMainFrame::OnUpdateFileDetachTapeImage */

/*========================================================
Method   : CMainFrame::OnUpdateFileAttachCartridgeImage
=========================================================*/
/* #FN#
   Updates a state of the menu option */
void
/* #AS#
   Nothing */
CMainFrame::
OnUpdateFileAttachCartridgeImage(
	CCmdUI *pCmdUI /* #IN# The CCmdUI object that handles the update */
)
{
#ifdef WIN_NETWORK_GAMES
	pCmdUI->Enable( !ST_KAILLERA_ACTIVE );
#endif
} /* #OF# CMainFrame::OnUpdateFileAttachCartridgeImage */

/*========================================================
Method   : CMainFrame::OnUpdateFileDetachCartridgeImage
=========================================================*/
/* #FN#
   Updates a state of the menu option */
void
/* #AS#
   Nothing */
CMainFrame::
OnUpdateFileDetachCartridgeImage(
	CCmdUI *pCmdUI /* #IN# The CCmdUI object that handles the update */
)
{
	pCmdUI->Enable( CARTRIDGE_NONE != CARTRIDGE_type
#ifdef WIN_NETWORK_GAMES
		&& !ST_KAILLERA_ACTIVE
#endif
	);
} /* #OF# CMainFrame::OnUpdateFileDetachCartridgeImage */

/*========================================================
Method   : CMainFrame::OnUpdateFileReadState
=========================================================*/
/* #FN#
   Updates a state of the menu option */
void
/* #AS#
   Nothing */
CMainFrame::
OnUpdateFileReadState(
	CCmdUI *pCmdUI /* #IN# The CCmdUI object that handles the update */
)
{
#ifdef WIN_NETWORK_GAMES
	pCmdUI->Enable( !ST_KAILLERA_ACTIVE );
#endif
} /* #OF# CMainFrame::OnUpdateFileReadState */

/*========================================================
Method   : CMainFrame::OnUpdateAtariMachineType
=========================================================*/
/* #FN#
   Updates a state of the menu option */
void
/* #AS#
   Nothing */
CMainFrame::
OnUpdateAtariMachineType(
	CCmdUI *pCmdUI /* #IN# The CCmdUI object that handles the update */
)
{
#ifdef WIN_NETWORK_GAMES
	if( ST_KAILLERA_ACTIVE )
	{
		pCmdUI->Enable( FALSE );
	}
#endif
	pCmdUI->SetRadio( (pCmdUI->m_nID - ID_ATARI_MACHINETYPE_BASE) == DWORD(Atari800_machine_type) );

} /* #OF# CMainFrame::OnUpdateAtariMachineType */

/*========================================================
Method   : CMainFrame::OnUpdateAtariVideoSystem
=========================================================*/
/* #FN#
   Updates a state of the menu option */
void
/* #AS#
   Nothing */
CMainFrame::
OnUpdateAtariVideoSystem(
	CCmdUI *pCmdUI /* #IN# The CCmdUI object that handles the update */
)
{
#ifdef WIN_NETWORK_GAMES
	if( ST_KAILLERA_ACTIVE )
	{
		pCmdUI->Enable( FALSE );
	}
#endif
	pCmdUI->SetRadio( (pCmdUI->m_nID - ID_ATARI_VIDEOSYSTEM_BASE) == DWORD(Atari800_TV_PAL == Atari800_tv_mode ? 0 : 1) );

} /* #OF# CMainFrame::OnUpdateAtariVideoSystem */

/*========================================================
Method   : CMainFrame::OnUpdateAtariMemorySize
=========================================================*/
/* #FN#
   Updates a state of the menu option */
void
/* #AS#
   Nothing */
CMainFrame::
OnUpdateAtariMemorySize(
	CCmdUI *pCmdUI /* #IN# The CCmdUI object that handles the update */
)
{
	/* CAUTION:
	   The code below works only if there are the following const
	   values defined in atari.h:
			#define Atari800_MACHINE_OSA		0
			#define Atari800_MACHINE_OSB		1
			#define Atari800_MACHINE_XLXE	2
			#define Atari800_MACHINE_5200	3
	*/
	static int  anMemVal[] = { 16, 48, 52, 64, 128, MEMORY_RAM_320_COMPY_SHOP, MEMORY_RAM_320_RAMBO, 576, 1088 };
	const  int  nMemValNo  = sizeof(anMemVal)/sizeof(anMemVal[0]);
	static BOOL abMemFlg[ 4 ][ nMemValNo ] =
	{
		{ TRUE,  TRUE,  TRUE,  FALSE, FALSE, FALSE, FALSE, FALSE, FALSE },
		{ TRUE,  TRUE,  TRUE,  FALSE, FALSE, FALSE, FALSE, FALSE, FALSE },
		{ TRUE,  FALSE, FALSE, TRUE,  TRUE,  TRUE,  TRUE,  TRUE,  TRUE  },
		{ TRUE,  FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE }
	};
	int nSelItem = pCmdUI->m_nID - ID_ATARI_MEMORYSIZE_BASE;

#ifdef WIN_NETWORK_GAMES
	if( ST_KAILLERA_ACTIVE )
	{
		pCmdUI->Enable( FALSE );
	}
	else
#endif
		pCmdUI->Enable( abMemFlg[ Atari800_machine_type ][ nSelItem ] );

	pCmdUI->SetRadio( anMemVal[ nSelItem ] == MEMORY_ram_size );

} /* #OF# CMainFrame::OnUpdateAtariMemorySize */

/*========================================================
Method   : CMainFrame::OnUpdateAtariFullSpeed
=========================================================*/
/* #FN#
   Updates a state of the menu option */
void
/* #AS#
   Nothing */
CMainFrame::
OnUpdateAtariFullSpeed(
	CCmdUI *pCmdUI /* #IN# The CCmdUI object that handles the update */
)
{
	pCmdUI->SetCheck( _IsFlagSet( g_Misc.ulState, MS_FULL_SPEED ) );

} /* #OF# CMainFrame::OnUpdateAtariFullSpeed */

/*========================================================
Method   : CMainFrame::OnUpdateAtariPause
=========================================================*/
/* #FN#
   Updates a state of the menu option */
void
/* #AS#
   Nothing */
CMainFrame::
OnUpdateAtariPause(
	CCmdUI *pCmdUI /* #IN# The CCmdUI object that handles the update */
)
{
	pCmdUI->SetCheck( _IsFlagSet( g_ulAtariState, ATARI_PAUSED ) );
#ifdef WIN_NETWORK_GAMES
	pCmdUI->Enable( !ST_KAILLERA_ACTIVE );
#endif
} /* #OF# CMainFrame::OnUpdateAtariPause */

/*========================================================
Method   : CMainFrame::OnUpdateAtariSioPatch
=========================================================*/
/* #FN#
   Updates a state of the menu option */
void
/* #AS#
   Nothing */
CMainFrame::
OnUpdateAtariSioPatch(
	CCmdUI *pCmdUI /* #IN# The CCmdUI object that handles the update */
)
{
	pCmdUI->SetCheck( 0 != ESC_enable_sio_patch );
#ifdef WIN_NETWORK_GAMES
	pCmdUI->Enable( !ST_KAILLERA_ACTIVE );
#endif
} /* #OF# CMainFrame::OnUpdateAtariSioPatch */

/*========================================================
Method   : CMainFrame::OnUpdateAtariHPatch
=========================================================*/
/* #FN#
   Updates a state of the menu option */
void
/* #AS#
   Nothing */
CMainFrame::
OnUpdateAtariHPatch(
	CCmdUI *pCmdUI /* #IN# The CCmdUI object that handles the update */
)
{
	pCmdUI->SetCheck( 0 != Devices_enable_h_patch );
#ifdef WIN_NETWORK_GAMES
	pCmdUI->Enable( !ST_KAILLERA_ACTIVE );
#endif
} /* #OF# CMainFrame::OnUpdateAtariHPatch */

/*========================================================
Method   : CMainFrame::OnUpdateAtariPPatch
=========================================================*/
/* #FN#
   Updates a state of the menu option */
void
/* #AS#
   Nothing */
CMainFrame::
OnUpdateAtariPPatch(
	CCmdUI *pCmdUI /* #IN# The CCmdUI object that handles the update */
)
{
	pCmdUI->SetCheck( 0 != Devices_enable_p_patch );
#ifdef WIN_NETWORK_GAMES
	pCmdUI->Enable( !ST_KAILLERA_ACTIVE );
#endif
} /* #OF# CMainFrame::OnUpdateAtariPPatch */

/*========================================================
Method   : CMainFrame::OnUpdateAtariRPatch
=========================================================*/
/* #FN#
   Updates a state of the menu option */
void
/* #AS#
   Nothing */
CMainFrame::
OnUpdateAtariRPatch(
	CCmdUI *pCmdUI /* #IN# The CCmdUI object that handles the update */
)
{
	pCmdUI->SetCheck( 0 != Devices_enable_r_patch );
	pCmdUI->Enable( RDevice_IsCapable() != -1
#ifdef WIN_NETWORK_GAMES
		&& !ST_KAILLERA_ACTIVE
#endif
	);
} /* #OF# CMainFrame::OnUpdateAtariRPatch */

/*========================================================
Method   : CMainFrame::OnUpdateAtariDisableBasic
=========================================================*/
/* #FN#
   Updates a state of the menu option */
void
/* #AS#
   Nothing */
CMainFrame::
OnUpdateAtariDisableBasic(
	CCmdUI *pCmdUI /* #IN# The CCmdUI object that handles the update */
)
{
	pCmdUI->SetCheck( Atari800_disable_basic );
#ifdef WIN_NETWORK_GAMES
	pCmdUI->Enable( !ST_KAILLERA_ACTIVE );
#endif
} /* #OF# CMainFrame::OnUpdateAtariDisableBasic */

/*========================================================
Method   : CMainFrame::OnUpdateInputAutofire
=========================================================*/
/* #FN#
   Updates a state of the menu option */
void
/* #AS#
   Nothing */
CMainFrame::
OnUpdateInputAutofire(
	CCmdUI *pCmdUI /* #IN# The CCmdUI object that handles the update */
)
{
	pCmdUI->SetRadio( (pCmdUI->m_nID - ID_INPUT_AUTOFIRE_BASE) == DWORD(g_Input.Joy.nAutoMode) );

} /* #OF# CMainFrame::OnUpdateInputAutofire */

/*========================================================
Method   : CMainFrame::OnUpdateViewToggleModes
=========================================================*/
/* #FN#
   Updates a state of the menu option */
void
/* #AS#
   Nothing */
CMainFrame::
OnUpdateViewToggleModes(
	CCmdUI *pCmdUI /* #IN# The CCmdUI object that handles the update */
)
{
	pCmdUI->SetCheck( _IsFlagSet( g_Screen.ulState, SM_MODE_FULL ) );

} /* #OF# CMainFrame::OnUpdateViewToggleModes */

/*========================================================
Method   : CMainFrame::OnUpdateViewGdiForWindows
=========================================================*/
/* #FN#
   Updates a state of the menu option */
void
/* #AS#
   Nothing */
CMainFrame::
OnUpdateViewGdiForWindows(
	CCmdUI *pCmdUI /* #IN# The CCmdUI object that handles the update */
)
{
	pCmdUI->SetCheck( _IsFlagSet( g_Screen.ulState, SM_OPTN_USE_GDI ) );

} /* #OF# CMainFrame::OnUpdateViewGdiForWindows */

/*========================================================
Method   : CMainFrame::OnUpdateViewStretchMode
=========================================================*/
/* #FN#
   Updates a state of the menu option */
void
/* #AS#
   Nothing */
CMainFrame::
OnUpdateViewStretchMode(
	CCmdUI *pCmdUI /* #IN# The CCmdUI object that handles the update */
)
{
	switch (pCmdUI->m_nID) {
		case ID_VIEW_STRETCHMODE_PIXELDOUBLING:
			pCmdUI->SetRadio( g_Screen.nStretchMode == 0);
			break;
		case ID_VIEW_STRETCHMODE_INTERPOLATION:
			pCmdUI->SetRadio( g_Screen.nStretchMode == 1);
			break;
		case ID_VIEW_STRETCHMODE_SCANLINES:
			pCmdUI->SetRadio( g_Screen.nStretchMode == 2);
			break;
		case ID_VIEW_STRETCHMODE_HIEND:
			pCmdUI->SetRadio( g_Screen.nStretchMode == 3);
			break;
		default:
			pCmdUI->SetRadio( 0 );
			break;
	}

} /* #OF# CMainFrame::OnUpdateViewStretchMode */

/*========================================================
Method   : CMainFrame::OnUpdateViewShowDiskActivity
=========================================================*/
/* #FN#
   Updates a state of the menu option */
void
/* #AS#
   Nothing */
CMainFrame::
OnUpdateViewShowDiskActivity(
	CCmdUI *pCmdUI /* #IN# The CCmdUI object that handles the update */
)
{
	pCmdUI->SetCheck( _IsFlagSet( g_Misc.ulState, MS_SHOW_DRIVE_LED ) );

} /* #OF# CMainFrame::OnUpdateViewShowDiskActivity */

/*========================================================
Method   : CMainFrame::OnUpdateViewShowSectorCounter
=========================================================*/
/* #FN#
   Updates a state of the menu option */
void
/* #AS#
   Nothing */
CMainFrame::
OnUpdateViewShowSectorCounter(
	CCmdUI *pCmdUI /* #IN# The CCmdUI object that handles the update */
)
{
	pCmdUI->SetCheck( _IsFlagSet( g_Misc.ulState, MS_SHOW_SECTOR_COUNTER ) );
	pCmdUI->Enable( _IsFlagSet( g_Misc.ulState, MS_SHOW_DRIVE_LED ) );

} /* #OF# CMainFrame::OnUpdateViewShowSectorCounter */

/*========================================================
Method   : CMainFrame::OnUpdateViewShowStateIndicators
=========================================================*/
/* #FN#
   Updates a state of the menu option */
void
/* #AS#
   Nothing */
CMainFrame::
OnUpdateViewShowStateIndicators(
	CCmdUI *pCmdUI /* #IN# The CCmdUI object that handles the update */
)
{
	pCmdUI->SetCheck( _IsFlagSet( g_Misc.ulState, MS_SHOW_INDICATORS ) );

} /* #OF# CMainFrame::OnUpdateViewShowStateIndicators */

/*========================================================
Method   : CMainFrame::OnUpdateViewArtifacting
=========================================================*/
/* #FN#
   Updates a state of the menu option */
void
/* #AS#
   Nothing */
CMainFrame::
OnUpdateViewArtifacting(
	CCmdUI *pCmdUI /* #IN# The CCmdUI object that handles the update */
)
{
	pCmdUI->SetRadio( (pCmdUI->m_nID - ID_VIEW_ARTIFACTING_BASE) == (DWORD)ANTIC_artif_mode );

} /* #OF# CMainFrame::OnUpdateViewArtifacting */

/*========================================================
Method   : CMainFrame::OnUpdateViewSaveVideo
=========================================================*/
/* #FN#
   Updates a state of the menu option */
void
/* #AS#
   Nothing */
CMainFrame::
OnUpdateViewSaveVideo(
	CCmdUI *pCmdUI /* #IN# The CCmdUI object that handles the update */
)
{
	pCmdUI->SetCheck( g_Screen.pfOutput != NULL && !_IsFlagSet( g_Misc.ulState, MS_VIDEO_AND_SOUND ) );
	pCmdUI->Enable( _IsFlagSet( g_Screen.ulState, SM_MODE_WIND ) &&
					_IsFlagSet( g_Screen.ulState, SM_OPTN_USE_GDI ) &&
					(g_Screen.pfOutput == NULL || !_IsFlagSet( g_Misc.ulState, MS_VIDEO_AND_SOUND )) );

} /* #OF# CMainFrame::OnUpdateViewSaveVideo */

/*========================================================
Method   : CMainFrame::OnUpdateViewSaveVideoAndSound
=========================================================*/
/* #FN#
   Updates a state of the menu option */
void
/* #AS#
   Nothing */
CMainFrame::
OnUpdateViewSaveVideoAndSound(
	CCmdUI *pCmdUI /* #IN# The CCmdUI object that handles the update */
)
{
	pCmdUI->SetCheck( g_Screen.pfOutput != NULL && _IsFlagSet( g_Misc.ulState, MS_VIDEO_AND_SOUND ) );
	pCmdUI->Enable( _IsFlagSet( g_Screen.ulState, SM_MODE_WIND ) &&
					_IsFlagSet( g_Screen.ulState, SM_OPTN_USE_GDI ) &&
					(g_Screen.pfOutput == NULL || _IsFlagSet( g_Misc.ulState, MS_VIDEO_AND_SOUND )) );

} /* #OF# CMainFrame::OnUpdateViewSaveVideoAndSound */

/*========================================================
Method   : CMainFrame::OnUpdateSoundStereo
=========================================================*/
/* #FN#
   Updates a state of the menu option */
void
/* #AS#
   Nothing */
CMainFrame::
OnUpdateSoundStereo(
	CCmdUI *pCmdUI /* #IN# The CCmdUI object that handles the update */
)
{
	pCmdUI->SetCheck( 0 != POKEYSND_stereo_enabled );

} /* #OF# CMainFrame::OnUpdateSoundStereo */

/*========================================================
Method   : CMainFrame::OnUpdateSoundMute
=========================================================*/
/* #FN#
   Updates a state of the menu option */
void
/* #AS#
   Nothing */
CMainFrame::
OnUpdateSoundMute(
	CCmdUI *pCmdUI /* #IN# The CCmdUI object that handles the update */
)
{
	pCmdUI->SetCheck( _IsFlagSet( g_Sound.ulState, SS_NO_SOUND ) );

} /* #OF# CMainFrame::OnUpdateSoundMute */

/*========================================================
Method   : CMainFrame::OnUpdateSoundSaveSound
=========================================================*/
/* #FN#
   Updates a state of the menu option */
void
/* #AS#
   Nothing */
CMainFrame::
OnUpdateSoundSaveSound(
	CCmdUI *pCmdUI /* #IN# The CCmdUI object that handles the update */
)
{
	pCmdUI->SetCheck( NULL != g_Sound.pfOutput );

} /* #OF# CMainFrame::OnUpdateSoundSaveSound */

/*========================================================
Method   : CMainFrame::OnUpdateInputCaptureMouse
=========================================================*/
/* #FN#
   Updates a state of the menu option */
void
/* #AS#
   Nothing */
CMainFrame::
OnUpdateInputCaptureMouse(
	CCmdUI *pCmdUI /* #IN# The CCmdUI object that handles the update */
)
{
	pCmdUI->SetCheck( _IsFlagSet( g_Input.ulState, IS_CAPTURE_MOUSE ) );
#ifdef WIN_NETWORK_GAMES
	pCmdUI->Enable( !ST_KAILLERA_ACTIVE );
#endif
} /* #OF# CMainFrame::OnUpdateInputCaptureMouse */

/*========================================================
Method   : CMainFrame::OnUpdateInputMouseDevice
=========================================================*/
/* #FN#
   Updates a state of the menu option */
void
/* #AS#
   Nothing */
CMainFrame::
OnUpdateInputMouseDevice(
	CCmdUI *pCmdUI /* #IN# The CCmdUI object that handles the update */
)
{
	pCmdUI->SetRadio( (pCmdUI->m_nID - ID_INPUT_MOUSEDEVICE_BASE) == DWORD(INPUT_mouse_mode) );

} /* #OF# CMainFrame::OnUpdateInputMouseDevice */

/*========================================================
Method   : CMainFrame::OnUpdateInputArrowKeys
=========================================================*/
/* #FN#
   Updates a state of the menu option */
void
/* #AS#
   Nothing */
CMainFrame::
OnUpdateInputArrowKeys(
	CCmdUI *pCmdUI /* #IN# The CCmdUI object that handles the update */
)
{
	pCmdUI->SetRadio( (pCmdUI->m_nID - ID_INPUT_ARROWKEYS_BASE) == DWORD(g_Input.Key.nArrowsMode) );

} /* #OF# CMainFrame::OnUpdateInputArrowKeys */

/*========================================================
Method   : CMainFrame::OnUpdateInputNetworkGame
=========================================================*/
/* #FN#
   Updates a state of the menu option */
void
/* #AS#
   Nothing */
CMainFrame::
OnUpdateInputNetworkGame(
	CCmdUI *pCmdUI /* #IN# The CCmdUI object that handles the update */
)
{
#ifdef WIN_NETWORK_GAMES
	pCmdUI->Enable( Kaillera_IsCapable() != -1 && !ST_KAILLERA_ACTIVE );
#else
	pCmdUI->Enable( FALSE );
#endif
} /* #OF# CMainFrame::OnUpdateInputNetworkGame */

/*========================================================
Method   : CMainFrame::OnUpdateMiscReuseWindow
=========================================================*/
/* #FN#
   Updates a state of the menu option */
void
/* #AS#
   Nothing */
CMainFrame::
OnUpdateMiscReuseWindow(
	CCmdUI *pCmdUI /* #IN# The CCmdUI object that handles the update */
)
{
	pCmdUI->SetCheck( _IsFlagSet( g_Misc.ulState, MS_REUSE_WINDOW ) );
#ifdef WIN_NETWORK_GAMES
	pCmdUI->Enable( !ST_KAILLERA_ACTIVE );
#endif
} /* #OF# CMainFrame::OnUpdateMiscReuseWindow */

/*========================================================
Method   : CMainFrame::OnUpdateMiscPauseInactive
=========================================================*/
/* #FN#
   Updates a state of the menu option */
void
/* #AS#
   Nothing */
CMainFrame::
OnUpdateMiscPauseInactive(
	CCmdUI *pCmdUI /* #IN# The CCmdUI object that handles the update */
)
{
	pCmdUI->SetCheck( _IsFlagSet( g_Misc.ulState, MS_STOP_WHEN_NO_FOCUS ) );
#ifdef WIN_NETWORK_GAMES
	pCmdUI->Enable( !ST_KAILLERA_ACTIVE );
#endif
} /* #OF# CMainFrame::OnUpdateMiscPauseInactive */

/*========================================================
Method   : CMainFrame::OnUpdateMiscHighPriority
=========================================================*/
/* #FN#
   Updates a state of the menu option */
void
/* #AS#
   Nothing */
CMainFrame::
OnUpdateMiscHighPriority(
	CCmdUI *pCmdUI /* #IN# The CCmdUI object that handles the update */
)
{
	pCmdUI->SetCheck( _IsFlagSet( g_Misc.ulState, MS_HIGH_PRIORITY ) );

} /* #OF# CMainFrame::OnUpdateMiscHighPriority */

/*========================================================
Method   : CMainFrame::OnUpdateMiscConfirmOnExit
=========================================================*/
/* #FN#
   Updates a state of the menu option */
void
/* #AS#
   Nothing */
CMainFrame::
OnUpdateMiscConfirmOnExit(
	CCmdUI *pCmdUI /* #IN# The CCmdUI object that handles the update */
)
{
	pCmdUI->SetCheck( _IsFlagSet( g_Misc.ulState, MS_CONFIRM_ON_EXIT ) );

} /* #OF# CMainFrame::OnUpdateMiscConfirmOnExit */

/*========================================================
Method   : CMainFrame::OnUpdateMiscThreadedEmulation
=========================================================*/
/* #FN#
   Updates a state of the menu option */
void
/* #AS#
   Nothing */
CMainFrame::
OnUpdateMiscThreadedEmulation(
	CCmdUI *pCmdUI /* #IN# The CCmdUI object that handles the update */
)
{
	pCmdUI->SetCheck( _IsFlagSet( g_Misc.ulState, MS_THREADED_EMULATION ) );

} /* #OF# CMainFrame::OnUpdateMiscThreadedEmulation */

/*========================================================
Method   : CMainFrame::OnUpdateMiscClearAllSettings
=========================================================*/
/* #FN#
   Updates a state of the menu option */
void
/* #AS#
   Nothing */
CMainFrame::
OnUpdateMiscClearAllSettings(
	CCmdUI *pCmdUI /* #IN# The CCmdUI object that handles the update */
)
{
#ifdef WIN_NETWORK_GAMES
	pCmdUI->Enable( !ST_KAILLERA_ACTIVE );
#endif
} /* #OF# CMainFrame::OnUpdateMiscClearAllSettings */

/*========================================================
Method   : CMainFrame::OnUpdateMiscDisableCollisions
=========================================================*/
/* #FN#
   Updates a state of the menu option */
void
/* #AS#
   Nothing */
CMainFrame::
OnUpdateMiscDisableCollisions(
	CCmdUI *pCmdUI /* #IN# The CCmdUI object that handles the update */
)
{
	pCmdUI->SetCheck( _IsFlagSet( g_Misc.ulState, MS_DISABLE_COLLISIONS ) );
	pCmdUI->Enable( 0 != g_Misc.Cheat.ulCollisions
#ifdef WIN_NETWORK_GAMES
		&& !ST_KAILLERA_ACTIVE
#endif
	);
} /* #OF# CMainFrame::OnUpdateMiscDisableCollisions */

/*========================================================
Method   : CMainFrame::OnUpdateMiscMonitor
=========================================================*/
/* #FN#
   Updates a state of the menu option */
void
/* #AS#
   Nothing */
CMainFrame::
OnUpdateMiscMonitor(
	CCmdUI *pCmdUI /* #IN# The CCmdUI object that handles the update */
)
{
#ifdef WIN_NETWORK_GAMES
	pCmdUI->Enable( !ST_KAILLERA_ACTIVE );
#endif
} /* #OF# CMainFrame::OnUpdateMiscMonitor */

/*========================================================
Method   : CMainFrame::OnUpdateMiscMonitorAsk
=========================================================*/
/* #FN#
   Updates a state of the menu option */
void
/* #AS#
   Nothing */
CMainFrame::
OnUpdateMiscMonitorAsk(
	CCmdUI *pCmdUI /* #IN# The CCmdUI object that handles the update */
)
{
	pCmdUI->SetRadio( !(_IsFlagSet( g_Misc.ulState, MS_MONITOR_ALWAYS ) || 
						_IsFlagSet( g_Misc.ulState, MS_MONITOR_NEVER ) ) );

} /* #OF# CMainFrame::OnUpdateMiscMonitorAsk */

/*========================================================
Method   : CMainFrame::OnUpdateMiscMonitorOpen
=========================================================*/
/* #FN#
   Updates a state of the menu option */
void
/* #AS#
   Nothing */
CMainFrame::
OnUpdateMiscMonitorOpen(
	CCmdUI *pCmdUI /* #IN# The CCmdUI object that handles the update */
)
{
	pCmdUI->SetRadio( _IsFlagSet( g_Misc.ulState, MS_MONITOR_ALWAYS ) );

} /* #OF# CMainFrame::OnUpdateMiscMonitorOpen */

/*========================================================
Method   : CMainFrame::OnUpdateMiscMonitorStop
=========================================================*/
/* #FN#
   Updates a state of the menu option */
void
/* #AS#
   Nothing */
CMainFrame::
OnUpdateMiscMonitorStop(
	CCmdUI *pCmdUI /* #IN# The CCmdUI object that handles the update */
)
{
	pCmdUI->SetRadio( _IsFlagSet( g_Misc.ulState, MS_MONITOR_NEVER) );

} /* #OF# CMainFrame::OnUpdateMiscMonitorAsk */

/*========================================================
Method   : CMainFrame::OnUpdateHelpSetupWizard
=========================================================*/
/* #FN#
   Updates a state of the menu option */
void
/* #AS#
   Nothing */
CMainFrame::
OnUpdateHelpSetupWizard(
	CCmdUI *pCmdUI /* #IN# The CCmdUI object that handles the update */
)
{
#ifdef WIN_NETWORK_GAMES
	pCmdUI->Enable( !ST_KAILLERA_ACTIVE );
#endif
} /* #OF# CMainFrame::OnUpdateHelpSetupWizard */

/*========================================================
Method   : CMainFrame::OnNcLButtonDown
=========================================================*/
/* #FN#
   The framework calls this member function when the user
   presses the left mouse button while the cursor is within
   a nonclient area of the CWnd object */
void
/* #AS#
   Nothing */
CMainFrame::
OnNcLButtonDown(
	UINT   nHitTest, /* #IN# The hit-test code */
	CPoint point     /* #IN# The x and y screen coordinates of the cursor position */
)
{
	if( HTMAXBUTTON == nHitTest && _IsFlagSet( g_Screen.ulState, SM_MODE_WIND ) )
	{
		CRect rect;
		POINT pt;

		m_bPosChanging = FALSE;
		Default(); /* Draw a pushed 'maximize' button */
		m_bPosChanging = TRUE;

		GetCursorPos( &pt );

		GetWindowRect( rect );
		rect.top    += GetSystemMetrics( SM_CYDLGFRAME );
		rect.bottom  = rect.top + GetSystemMetrics( SM_CYCAPTION );
		rect.right  -= GetSystemMetrics( SM_CXDLGFRAME ) + GetSystemMetrics( SM_CXSIZE );
		rect.left    = rect.right - GetSystemMetrics( SM_CXSIZE );
		rect.InflateRect( 1, 1, 1, 0 );

		if( rect.PtInRect( pt ) )
		{
			if( StreamWarning( IDS_WARN_RECORD_DISPLAY, SRW_VIDEO_STREAM ) )
			{
				if( !Screen_ToggleWindowed() )
					Screen_SetSafeDisplay( TRUE );
			}
		}
		return;
	}
	CFrameWnd::OnNcLButtonDown( nHitTest, point );

} /* #OF# CMainFrame::OnNcLButtonDown */

/*========================================================
Method   : CMainFrame::OnNcLButtonDblClk
=========================================================*/
/* #FN#
   The framework calls this member function when the user
   double-clicks the left mouse button while the cursor is
   within a nonclient area of CWnd */
void
/* #AS#
   Nothing */
CMainFrame::
OnNcLButtonDblClk(
	UINT   nHitTest, /* #IN# The hit-test code */
	CPoint point     /* #IN# The x and y screen coordinates of the cursor position */
)
{
	if( HTCAPTION == nHitTest && _IsFlagSet( g_Screen.ulState, SM_MODE_WIND ) )
	{
		if( StreamWarning( IDS_WARN_RECORD_DISPLAY, SRW_VIDEO_STREAM ) )
		{
			if( !Screen_ToggleWindowed() )
				Screen_SetSafeDisplay( TRUE );
		}
		return;
	}
	CFrameWnd::OnNcLButtonDblClk( nHitTest, point );

} /* #OF# CMainFrame::OnNcLButtonDblClk */

/*========================================================
Method   : CMainFrame::OnNcMouseMove
=========================================================*/
/* #FN#
   The framework calls this member function when the cursor
   is moved within a nonclient area */
void
/* #AS#
   Nothing */
CMainFrame::
OnNcMouseMove(
	UINT   nHitTest, /* #IN# The hit-test code */
	CPoint point     /* #IN# The x and y screen coordinates of the cursor position */
)
{
	if( !_IsFlagSet( g_Input.ulState, IS_CAPTURE_MOUSE ) )
		Screen_ShowMousePointer( TRUE );

	CFrameWnd::OnNcMouseMove( nHitTest, point );

} /* #OF# CMainFrame::OnNcMouseMove */

/*========================================================
Method   : CMainFrame::SetIndicatorDesc
=========================================================*/
/* #FN#
   Prints an appropriate description on the status bar when mouse
   cursor is moved within an indicator area */
void
/* #AS#
   Nothing */
CMainFrame::
SetIndicatorDesc(
	CPoint &point /* #IN# The x and y screen coordinates of the cursor position */
)
{
	if( _IsFlagSet( g_Misc.ulState, MS_SHOW_INDICATORS ) &&
		/* When in fullscreen mode we have to be sure that menu is visible */
		(_IsFlagSet( g_Screen.ulState, SM_MODE_WIND ) || ST_MENU_VISIBLE) )
	{
		BOOL bShowTip = TRUE;
		CString strTip, strMode;

		switch( m_wndStatusBar.GetPaneFromPoint( point ) )
		{
			case ID_INDICATOR_RUN:
				if( _IsFlagSet( g_Misc.ulState, MS_FULL_SPEED ) )
					strMode.LoadString( IDS_TRAYTIP_RUN_FULLSPEED );
				else
					strMode.Format( IDS_TRAYTIP_RUN_SPEED, g_Misc.nSpeedPercent );
				strTip.Format( IDS_TRAYTIP_RUN, strMode );
				break;

			case ID_INDICATOR_VID:
				strMode.LoadString( IDS_TRAYTIP_VID_PAL + (Atari800_TV_PAL == Atari800_tv_mode ? 0 : 1) );
				strTip.Format( IDS_TRAYTIP_VID, strMode );
				break;

			case ID_INDICATOR_SIO:
				strMode.LoadString( IDS_TRAYTIP_SIO_OFF + ESC_enable_sio_patch );
				strTip.Format( IDS_TRAYTIP_SIO, strMode );
				break;

			case ID_INDICATOR_MSE:
				strMode.LoadString( IDS_TRAYTIP_MSE_NONE + INPUT_mouse_mode );
				strTip.Format( IDS_TRAYTIP_MSE, strMode );
				break;

			case ID_INDICATOR_JOY:
				strMode.LoadString( IDS_TRAYTIP_JOY_OFF + g_Input.Joy.nAutoMode );
				strTip.Format( IDS_TRAYTIP_JOY, strMode );
				break;

			case ID_INDICATOR_AVI:
				strMode.LoadString( IDS_TRAYTIP_SAV_INACTIVE + (g_Screen.pfOutput ? 1 : 0) );
				strTip.Format( IDS_TRAYTIP_AVI, strMode );
				break;

			case ID_INDICATOR_WAV:
				strMode.LoadString( IDS_TRAYTIP_SAV_INACTIVE + (g_Sound.pfOutput ? 1 : 0) );
				strTip.Format( IDS_TRAYTIP_WAV, strMode );
				break;

#ifdef WIN_NETWORK_GAMES
			case ID_INDICATOR_NET:
				if( ST_NETGAME_ACTIVE )
					strMode.Format( Kaillera_GetPlayerNo() == 0 ? IDS_TRAYTIP_NET_MASTER : IDS_TRAYTIP_NET_PLAYER, g_Kaillera.nLocalPort + 1 );
				else
					strMode.LoadString( ST_KAILLERA_ACTIVE ? IDS_TRAYTIP_NET_CONNECT : IDS_TRAYTIP_NET_INACTIVE );
				strTip.Format( IDS_TRAYTIP_NET, strMode );
				break;
#endif
			case ID_INDICATOR_MSG:
				UpdateSpeed( TRUE );
				bShowTip = FALSE;
				break;

			default:
				/* Do nothing */
				bShowTip = FALSE;
				break;
		}
		if( bShowTip )
			/* Display the tip */
			m_pSBCtrl->SetText( strTip, 0, SBT_NOBORDERS );

		if( m_pMainApp )
			m_pMainApp->ResetLoopCounter();
	}
} /* #OF# CMainFrame::SetIndicatorDesc */

/*========================================================
Method   : CMainFrame::OnMouseMove
=========================================================*/
/* #FN#
   The framework calls this member function when the cursor
   moves. In PLus frame-view architecture there are only
   the WM_MOUSEMOVE messages passed by the status bar, so we
   rather don't need to register own mesage for this purpose */
void
/* #AS#
   Nothing */
CMainFrame::
OnMouseMove(
	UINT nFlags, /* #IN# Indicates whether various virtual keys are down */
	CPoint point /* #IN# The x and y screen coordinates of the cursor position */
)
{
	static int nOldPane = -1;
	int nPane = m_wndStatusBar.GetPaneFromPoint( point );

	if( !_IsFlagSet( g_Input.ulState, IS_CAPTURE_MOUSE ) )
		Screen_ShowMousePointer( TRUE );

	if( nPane != nOldPane )
	{
		SetIndicatorDesc( point );
		nOldPane = nPane;
	}
	CFrameWnd::OnMouseMove( nFlags, point );

} /* #OF# CMainFrame::OnMouseMove */

/*========================================================
Method   : CMainFrame::OnWindowPosChanging
=========================================================*/
/* #FN#
   The framework calls this member function when the size,
   position, or Z-order is about to change */
void
/* #AS#
   Nothing */
CMainFrame::
OnWindowPosChanging(
	WINDOWPOS FAR *lpWndPos /* #IN# The window’s new size and position */
)
{
	CFrameWnd::OnWindowPosChanging( lpWndPos );

	if( !m_bPosChanging )
		lpWndPos->flags |= SWP_NOMOVE | SWP_NOSIZE;

} /* #OF# CMainFrame::OnWindowPosChanging */

/*========================================================
Method   : CMainFrame::OnExitMenuLoop
=========================================================*/
/* #FN#
   The framework calls this member function when a menu modal loop
   has been exited */
void
/* #AS#
   Nothing */
CMainFrame::
OnExitMenuLoop(
	BOOL /*bIsTrackPopupMenu*/ /* #IN# Specifies whether the menu involved is a pop-up menu */
)
{
	if( _IsFlagSet( g_Screen.ulState, SM_MODE_FULL ) && g_Screen.bLocked )
	{
		if( _IsFlagSet( g_Screen.ulState, SM_OPTN_FLIP_BUFFERS ) )
		{
			/* Clean up the whole screen and free redraw stuff */
			CleanScreen();
		}
		else
		{

			RA_InitUI();
			/* Some strange code here but there are serious problems with
			   cleaning the menus e.g. after expanding them and clicking
			   at the outer area */
			Screen_ShowMenuBar( TRUE );

			if( !_IsFlagSet( g_ulAtariState, ATARI_PAUSED ) )
				/* Release redraw bitmap after using it */
				Screen_FreeRedraw();

			/* Restore the Atari palette if necessary */
			Screen_UseAtariPalette( FALSE );
		}
	}
	g_Screen.bLocked = FALSE;

	/* Update description on the status bar */
	UpdateSpeed();

	if( m_pMainApp )
		m_pMainApp->ResetLoopCounter();

//	CFrameWnd::OnExitMenuLoop( bIsTrackPopupMenu );

} /* #OF# CMainFrame::OnExitMenuLoop */

/*========================================================
Method   : CMainFrame::OnMenuSelect
=========================================================*/
/* #FN#
   This method is called by the framework when the user selects a menu item */
void
/* #AS#
   Nothing */
CMainFrame::
OnMenuSelect(
	UINT  nItemID,
	UINT  nFlags,
	HMENU hSysMenu
)
{
	if( 0xffff != nFlags && hSysMenu )
	{
		if( !g_Screen.bLocked && _IsFlagSet( nFlags, MF_POPUP ) &&
			hSysMenu == ::GetMenu( GetSafeHwnd() ) )
		{
			g_Screen.bLocked = TRUE;

			if( _IsFlagSet( g_Screen.ulState, SM_MODE_FULL ) )
			{
				/* Not really needed for 'atari_crashed' state but it
				   is more ellegant and efficient way for doing that */
				if( ST_ATARI_STOPPED )
					/* Set system palette colours only */
					Screen_UseSystemPalette();
				else
				{	/* Redraw the Atari screen content */
					Screen_DrawFrozen( TRUE, TRUE, TRUE, ST_FLIPPED_BUFFERS/*FALSE*/ );
				}
			}
			Screen_ShowMousePointer( TRUE );
			/* Update description on the status bar */
			UpdateSpeed( TRUE );
		}
	}

	if (nItemID > IDD_RA_RICHPRESENCE && nItemID < IDC_RA_OPENPAGE)
		RA_InvokeDialog(nItemID);

	CFrameWnd::OnMenuSelect( nItemID, nFlags, hSysMenu );

} /* #OF# CMainFrame::OnMenuSelect */

/*========================================================
Method   : CMainFrame::OnCommand
=========================================================*/
/* #FN#
   The framework calls this member function when the user selects
   an item from a menu, when a child control sends a notification
   message, or when an accelerator keystroke is translated */
BOOL
/* #AS#
   Nonzero if this message was processed; otherwise 0 */
CMainFrame::
OnCommand(
	WPARAM wParam,
	LPARAM lParam
)
{
	/* There was menu command or an accelerator keystroke */
	if( 0 == HIWORD(wParam) || 1 == HIWORD(wParam) )
	{
		WORD wCmdID = LOWORD(wParam);
		for( int i = 0; i < s_nShowMenuBarCmdNo; i++ )
		{
			if( wCmdID == s_aShowMenuBarCmd[ i ] )
			{
				g_Screen.bLocked = TRUE;

				if( _IsFlagSet( g_Screen.ulState, SM_MODE_FULL ) )
				{
					/* Necessary for 'atari_crashed' state mainly because
					   the Atari screen is repainted by Screen_DrawFrozen */
					if( ST_ATARI_STOPPED )
						/* Set system palette colours only */
						Screen_UseSystemPalette();
					else
					{	/* Redraw the Atari screen content */
						Screen_DrawFrozen( TRUE, TRUE, TRUE, TRUE );
					}
				}
				Screen_ShowMousePointer( TRUE );
				/* Update description on the status bar */
				UpdateSpeed( TRUE );

				break;
			}
		}
	}
	return CFrameWnd::OnCommand( wParam, lParam );

} /* #OF# CMainFrame::OnCommand */

/*========================================================
Method   : CMainFrame::OnMessageCommandLine
=========================================================*/
/* #FN#
   The WM_PLUS_CMDLINE message handler (a part of a single instance support) */
LRESULT
/* #AS#
   Nonzero if the message was handled; otherwise 0 */
CMainFrame::
OnMessageCommandLine(
	WPARAM wParam,
	LPARAM lParam
)
{
	if( !g_Screen.bLocked && !_IsFlagSet( g_ulAtariState, ATARI_MONITOR ) )
	{
		HANDLE hFileMap = OpenFileMapping( FILE_MAP_READ, FALSE, "Atari800CmdLine" );
		if( hFileMap )
		{
			LPSTR pszMsg = (LPSTR)MapViewOfFile( hFileMap, FILE_MAP_READ, 0, 0, 0 );
			if( pszMsg )
			{
				int nCmdIndex = 0;

				CopyMemory( g_szCmdLine, pszMsg, (int)lParam );
				UnmapViewOfFile( (LPCVOID)pszMsg );

				g_argc = (int)wParam;
				for( int i = 0; i < g_argc; i++ )
				{
					g_argv[ i ] = &g_szCmdLine[ nCmdIndex ];
					nCmdIndex += strlen( &g_szCmdLine[ nCmdIndex ] ) + 1;
				}
				/* Use g_argv and g_argc while rebooting */
				RestartEmulation( FALSE );
			}
		}
	}
	return TRUE;

} /* #OF# CMainFrame::OnMessageCommandLine */

/*========================================================
Method   : CMainFrame::OnMessageKailleraReset
=========================================================*/
/* #FN#
   The WM_PLUS_KARESET message handler (part of network games support) */
LRESULT
/* #AS#
   Nonzero if the message was handled; otherwise 0 */
CMainFrame::
OnMessageKailleraReset(
	WPARAM wParam,
	LPARAM lParam
)
{
	SuspendThread();

#ifdef WIN_NETWORK_GAMES
	RestartEmulation( TRUE );
#endif

	ResumeThread();
	return TRUE;
} /* #OF# CMainFrame::OnMessageKailleraReset */

/*========================================================
Method   : CMainFrame::OnMesasageKailleraStart
=========================================================*/
/* #FN#
   The WM_PLUS_KASTART message handler (part of network games support) */
LRESULT
/* #AS#
   Nonzero if the message was handled; otherwise 0 */
CMainFrame::
OnMessageKailleraStart(
	WPARAM wParam,
	LPARAM lParam
)
{
	SuspendThread();

#ifdef WIN_NETWORK_GAMES
	if( !s_Settings.bLocked )
	{
		ULONG ulMiscState = g_Misc.ulState;

		s_Settings.bLocked = TRUE;

		s_Settings.ulMiscState = g_Misc.ulState;
		s_Settings.bHoldStart  = CASSETTE_hold_start;
		s_Settings.nCartType   = CARTRIDGE_type;
		*s_Settings.szSnapFile = '\0';
		*s_Settings.szCartFile = '\0';
		*s_Settings.szExecFile = '\0';
		*s_Settings.szDiskFile = '\0';

		UINT unBootImage = (UINT)wParam;

		switch( unBootImage )
		{
			case IAF_CAS_IMAGE:
			{
				if( _IsPathAvailable( SIO_filename[ 0 ] ) &&
				  /*_stricmp( SIO_filename[ 0 ], "Empty" ) != 0 &&*/ /* We have to turn the drive off */
					_stricmp( SIO_filename[ 0 ], "Off" ) != 0 )
				{
					_strncpy( s_Settings.szDiskFile, SIO_filename[ 0 ], MAX_PATH );
					/* Detach the disk image */
					SIO_Dismount( 1 );
					SIO_drive_status[ 0 ] = SIO_OFF;
					strcpy( SIO_filename[ 0 ], "Off" );
				}
				CASSETTE_hold_start = 1; /* Autoboot the tape image */
			}
			case IAF_DSK_IMAGE:
			{
				if( _IsPathAvailable( g_szBinaryFile ) )
				{
					_strncpy( s_Settings.szExecFile, g_szBinaryFile, MAX_PATH );
					strcpy( g_szBinaryFile, FILE_NONE );
				}
			}
			case IAF_BIN_IMAGE:
			{
				if( _IsPathAvailable( g_szCurrentRom ) &&
					CARTRIDGE_NONE != CARTRIDGE_type )
				{
					_strncpy( s_Settings.szCartFile, g_szCurrentRom, MAX_PATH );
					/* Remove the cartridge */
					CARTRIDGE_Remove();
					strcpy( g_szCurrentRom, FILE_NONE );
				}
			}
			case IAF_ROM_IMAGE:
			{
				if( _IsPathAvailable( g_szSnapshotFile ) )
				{
					_strncpy( s_Settings.szSnapFile, g_szSnapshotFile, MAX_PATH );
					strcpy( g_szSnapshotFile, FILE_NONE );
				}
				break;
			}
			default:			
				_ASSERT(IAF_A8S_IMAGE == unBootImage);
				break;
		}
		/* Modify some emulation state flags */
		_ClrFlag( g_Misc.ulState, KA_MS_FLAGS_TO_CLEAR );
		Misc_UpdateCollisions();

		if( g_Misc.ulState != ulMiscState )
			WriteRegDWORD( NULL, REG_MISC_STATE, g_Misc.ulState );

		/* Turn the cheat mode off */
		ResetCheatServer();

		if( _IsFlagSet( g_Input.ulState, IS_CAPTURE_MOUSE ) )
		{
			Input_ToggleMouseCapture();
			WriteRegDWORD( NULL, REG_INPUT_STATE, g_Input.ulState );

			UpdateIndicator( ID_INDICATOR_MSE );
		}
		if( _IsFlagSet( g_ulAtariState, ATARI_PAUSED ) )
		{
			Misc_TogglePause();
		}
		/* Update state of the frame */
		UpdateStatus();
		UpdateIndicator( ID_INDICATOR_NET );

		if( _IsFlagSet( g_Screen.ulState, SM_MODE_FULL ) )
			Screen_SetSafeDisplay( FALSE );
	}
#endif

	ResumeThread();
	return TRUE;
} /* #OF# CMainFrame::OnMesasageKailleraStart */

/*========================================================
Method   : CMainFrame::OnMesasageKailleraStop
=========================================================*/
/* #FN#
   The WM_PLUS_KASTOP message handler (part of network games support) */
LRESULT
/* #AS#
   Nonzero if the message was handled; otherwise 0 */
CMainFrame::
OnMessageKailleraStop(
	WPARAM wParam,
	LPARAM lParam
)
{
	SuspendThread();

#ifdef WIN_NETWORK_GAMES
	if( s_Settings.bLocked )
	{
		ULONG ulMiscState = g_Misc.ulState;
		BOOL  bReboot = FALSE;

		if( _IsPathAvailable( s_Settings.szDiskFile ) )
		{
			SIO_Mount( 1, s_Settings.szDiskFile, _IsFlagSet(g_Misc.ulState, MS_DRIVE_READONLY) );
		}
		if( _IsPathAvailable( s_Settings.szExecFile ) )
		{
			_strncpy( g_szBinaryFile, s_Settings.szExecFile, MAX_PATH );
		}
		if( _IsPathAvailable( s_Settings.szCartFile ) )
		{
			AttachCartridge( s_Settings.szCartFile, s_Settings.nCartType );
			bReboot = TRUE;
		}
		if( _IsPathAvailable( s_Settings.szSnapFile ) )
		{
			_strncpy( g_szSnapshotFile, s_Settings.szSnapFile, MAX_PATH );
		}
		/* Restore the emulation state flags */
		_SetFlag( g_Misc.ulState, s_Settings.ulMiscState & KA_MS_FLAGS_TO_CLEAR );
		Misc_UpdateCollisions();

		if( g_Misc.ulState != ulMiscState )
			WriteRegDWORD( NULL, REG_MISC_STATE, g_Misc.ulState );

		CASSETTE_hold_start = s_Settings.bHoldStart ? 1 : 0;

		if( bReboot ) /* Is rebooting necessary? */
			Atari800_Coldstart();

		UpdateStatus();
		UpdateIndicator( ID_INDICATOR_NET );

		s_Settings.bLocked = FALSE;
	}
#endif

	ResumeThread();
	return TRUE;
} /* #OF# CMainFrame::OnMessageKailleraStop */

/*========================================================
Method   : CMainFrame::OnMesasageKailleraGameStart
=========================================================*/
/* #FN#
   The WM_PLUS_KAGAMESTART message handler (part of network games support) */
LRESULT
/* #AS#
   Nonzero if the message was handled; otherwise 0 */
CMainFrame::
OnMessageKailleraGameStart(
	WPARAM wParam,
	LPARAM lParam
)
{
	SuspendThread();
#ifdef WIN_NETWORK_GAMES
	UpdateIndicator( ID_INDICATOR_NET );
#endif

	ResumeThread();
	return TRUE;

} /* #OF# CMainFrame::OnMessageKailleraGameStart */

/*========================================================
Method   : CMainFrame::OnMesasageKailleraGameStop
=========================================================*/
/* #FN#
   The WM_PLUS_KAGAMESTOP message handler (part of network games support) */
LRESULT
/* #AS#
   Nonzero if the message was handled; otherwise 0 */
CMainFrame::
OnMessageKailleraGameStop(
	WPARAM wParam,
	LPARAM lParam
)
{
	SuspendThread();
#ifdef WIN_NETWORK_GAMES
	UpdateIndicator( ID_INDICATOR_NET );
#endif


	ResumeThread();
	return TRUE;
} /* #OF# CMainFrame::OnMessageKailleraGameStop */

/*========================================================
Function : CMainFrame::UpdateStatus
=========================================================*/
/* #FN#
   Prints info about emulated system in main window */
void
/* #AS#
   Nothing */
CMainFrame::
UpdateStatus(
	BOOL bForceShow /*=FALSE*/,
	int  nSpeed /*=-1*/,
	int  nPane /*=ID_INDICATOR_RUN*/,
	BOOL bWinMode /*=FALSE*/,
	BOOL bUpdateIndicator /*=TRUE*/
)
{
	CString strHelper = "";

	if( _IsFlagSet( g_Screen.ulState, SM_MODE_FULL ) && !bWinMode )
		m_strWindowTitle = "  ";
	else
		m_strWindowTitle = VERSION_INFO": ";

	if( _IsFlagSet( g_ulAtariState, ATARI_UNINITIALIZED ) )
		m_strWindowTitle += m_szLabels[ LABEL_FAILURE ];
	else
	if( _IsFlagSet( g_ulAtariState, ATARI_RUNNING ) )
	{
		if( _IsFlagSet( g_ulAtariState, ATARI_MONITOR ) )
			m_strWindowTitle += m_szLabels[ LABEL_MONITOR ];
		else
		if( _IsFlagSet( g_ulAtariState, ATARI_PAUSED ) )
		{
			strHelper.Format( m_szLabels[ LABEL_PAUSED ], _IsFlagSet( g_Input.ulState, IS_JOY_EXIT_PAUSE ) ? m_szLabels[ LABEL_JOYKEY ] : "" );
			m_strWindowTitle += strHelper;
		}
		else
		if( _IsFlagSet( g_Misc.ulState, MS_STOP_WHEN_NO_FOCUS ) && _IsFlagSet( g_ulAtariState, ATARI_NO_FOCUS ) )
			m_strWindowTitle += m_szLabels[ LABEL_STOPPED ];
		else
		{
			/* What about the emulated system type? */
			switch( Atari800_machine_type )
			{
				case Atari800_MACHINE_OSA:
					strHelper.Format( "OS-A (%d KB)", MEMORY_ram_size );
					break;

				case Atari800_MACHINE_OSB:
					strHelper.Format( "OS-B (%d KB)", MEMORY_ram_size );
					break;

				case Atari800_MACHINE_XLXE:
					strHelper.Format( "XL/XE (%d KB", MEMORY_ram_size & ~1 );
					if( MEMORY_RAM_320_COMPY_SHOP == MEMORY_ram_size )
						strHelper += " Compy";
					else
					if( MEMORY_RAM_320_RAMBO == MEMORY_ram_size )
						strHelper += " Rambo";
					strHelper += ")";
					break;

				case Atari800_MACHINE_5200:
					strHelper = "5200 (16 KB)";
					break;

				default:
					strHelper = "?";
					ASSERT(FALSE);
					break;
			}
			m_strWindowTitle += strHelper;
		}
	}
	else
		m_strWindowTitle += m_szLabels[ LABEL_NOTRUN ];

	/* Set window title if in windowed mode */
	if( _IsFlagSet( g_Screen.ulState, SM_MODE_WIND ) || bWinMode )
		SetWindowText( m_strWindowTitle );

	/* Set description on status bar */
	UpdateSpeed( bForceShow, nSpeed, bWinMode );

	/* Update indicator on status bar */
	if (bUpdateIndicator)
		UpdateIndicator( nPane );

} /* #OF# CMainFrame::UpdateStatus */

/*========================================================
Method   : CMainFrame::UpdateSpeed
=========================================================*/
/* #FN#
   Displays an emulation speed on the status bar */
void
/* #AS#
   Nothing */
CMainFrame::
UpdateSpeed(
	BOOL bForceShow /*=FALSE*/,
	int  nSpeed /*=-1*/,
	BOOL bWinMode /*=FALSE*/
)
{
	if( bForceShow || _IsFlagSet( g_Screen.ulState, SM_MODE_WIND ) ||
		/* The full-screen modes condition */
		ST_MENU_VISIBLE )
	{
		static char szSpeed[ LOADSTRING_SIZE_S + 10 + 1 ];

		sprintf( szSpeed, "%s: %d%%", m_szLabels[ LABEL_SPEED ], (-1 != nSpeed ? nSpeed : (m_pMainApp != NULL ? m_pMainApp->GetCurrentSpeed() : 0)) );
		if( _IsFlagSet( g_Screen.ulState, SM_MODE_FULL ) && !bWinMode )
			strcat( szSpeed, m_strWindowTitle );

		/* Display info about speed on the status bar */
		m_pSBCtrl->SetText( szSpeed, 0, SBT_NOBORDERS );
		/* This method doesn't work well for us */
//		m_pStatusBar->SetWindowText( szSpeed );
	}
} /* #OF# CMainFrame::UpdateSpeed */

/*========================================================
Method   : CMainFrame::UpdateIndicator
=========================================================*/
/* #FN#
   Redraws the pointed icon on the status bar */
void
/* #AS#
   Nothing */
CMainFrame::
UpdateIndicator(
	int nPane
)
{
	if( _IsFlagSet( g_Misc.ulState, MS_SHOW_INDICATORS ) &&
		/* When in fullscreen mode we have to be sure that menu is visible */
		(_IsFlagSet( g_Screen.ulState, SM_MODE_WIND ) || ST_MENU_VISIBLE) )
	{
		m_wndStatusBar.UpdatePane( nPane );
	}
} /* #OF# CMainFrame::UpdateIndicator */

/*========================================================
Method   : CMainFrame::MouseButtonDblClk
=========================================================*/
/* #FN#
   Handles the mouse buttons clicks over the status bar */
void
/* #AS#
   Nothing */
CMainFrame::
MouseButtonDblClk(
	UINT   nFlags,
	CPoint point,
	BOOL   bForward /*=TRUE*/
)
{
	if( _IsFlagSet( g_Misc.ulState, MS_SHOW_INDICATORS ) &&
		/* When in fullscreen mode we have to be sure that menu is visible */
		(_IsFlagSet( g_Screen.ulState, SM_MODE_WIND ) || ST_MENU_VISIBLE) )
	{
		switch( m_wndStatusBar.GetPaneFromPoint( point ) )
		{
			case ID_INDICATOR_RUN:
				OnAtariFullSpeed();
				break;

			case ID_INDICATOR_VID:
#ifdef WIN_NETWORK_GAMES
				if( !ST_KAILLERA_ACTIVE )
#endif
					OnAtariVideoSystem();
				break;

			case ID_INDICATOR_SIO:
#ifdef WIN_NETWORK_GAMES
				if( !ST_KAILLERA_ACTIVE )
#endif
					OnAtariSioPatch();
				break;

			case ID_INDICATOR_MSE:
				ChangeMouseDevice( bForward );
				break;

			case ID_INDICATOR_JOY:
				ChangeAutofire( bForward );
				break;

			case ID_INDICATOR_AVI:
				if( _IsFlagSet( g_Screen.ulState, SM_MODE_WIND ) &&
					_IsFlagSet( g_Screen.ulState, SM_OPTN_USE_GDI ) )
				{
					g_Screen.bLocked = TRUE;
					/* Prepare redrawing the Atari screen content */
					Screen_DrawFrozen( FALSE, TRUE, !ST_ATARI_STOPPED, FALSE );

					OnViewSaveVideo();
				}
				break;

			case ID_INDICATOR_WAV:
				g_Screen.bLocked = TRUE;
				/* Prepare redrawing the Atari screen content */
				Screen_DrawFrozen( FALSE, TRUE, !ST_ATARI_STOPPED, FALSE );

				OnSoundSaveSound();
				break;

#ifdef WIN_NETWORK_GAMES
			case ID_INDICATOR_NET:
				if( Kaillera_IsCapable() != -1 && !ST_KAILLERA_ACTIVE )
				{
					g_Screen.bLocked = TRUE;
					/* Prepare redrawing the Atari screen content */
					Screen_DrawFrozen( FALSE, TRUE, !ST_ATARI_STOPPED, FALSE );

					OnInputNetworkGame();
				}
				break;
#endif
			default:
				/* Do nothing */
				break;
		}
		SetIndicatorDesc( point );
	}
} /* #OF# CMainFrame:MouseButtonDblClk */

/*========================================================
Method   : CMainFrame::OnLButtonDblClk
=========================================================*/
/* #FN#
   The framework calls this member function when the user double-clicks the left mouse button */
void
/* #AS#
   Nothing */
CMainFrame::
OnLButtonDblClk(
	UINT   nFlags,
	CPoint point
)
{
	MouseButtonDblClk( nFlags, point );

	CFrameWnd::OnLButtonDblClk( nFlags, point );

} /* #OF# CMainFrame:OnLButtonDblClk */

/*========================================================
Method   : CMainFrame::OnRButtonDblClk
=========================================================*/
/* #FN#
   The framework calls this member function when the user double-clicks the right mouse button */
void
/* #AS#
   Nothing */
CMainFrame::
OnRButtonDblClk(
	UINT   nFlags,
	CPoint point
)
{
	MouseButtonDblClk( nFlags, point, FALSE );

	CFrameWnd::OnRButtonDblClk( nFlags, point );

} /* #OF# CMainFrame:OnRButtonDblClk */

/*========================================================
Method   : CMainFrame::OnMButtonDblClk
=========================================================*/
/* #FN#
   The framework calls this member function when the user double-clicks the middle mouse button */
void
/* #AS#
   Nothing */
CMainFrame::
OnMButtonDblClk(
	UINT nFlags, /* #IN# Indicates whether various virtual keys are down */
	CPoint point /* #IN# Specifies the x- and y-coordinate of the cursor */
)
{
	if( !_IsFlagSet( g_Input.ulState, IS_CAPTURE_MOUSE ) )
		Screen_ShowMousePointer( TRUE );

	CFrameWnd::OnMButtonDblClk( nFlags, point );

} /* #OF# CMainFrame:OnMButtonDblClk */

/*========================================================
Method   : CMainFrame::OnLButtonDown
=========================================================*/
/* #FN#
   The framework calls this member function when the user presses the left mouse button
   (when the mouse cursor is over the status bar) */
void
/* #AS#
   Nothing */
CMainFrame::
OnLButtonDown(
	UINT   nFlags,
	CPoint point
)
{
	if( !_IsFlagSet( g_Input.ulState, IS_CAPTURE_MOUSE ) )
		Screen_ShowMousePointer( TRUE );

	CFrameWnd::OnLButtonDown( nFlags, point );

} /* #OF# CMainFrame:OnLButtonDown */

/*========================================================
Method   : CMainFrame::OnRButtonDown
=========================================================*/
/* #FN#
   The framework calls this member function when the user presses the right mouse button
   (when the mouse cursor is over the status bar) */
void
/* #AS#
   Nothing */
CMainFrame::
OnRButtonDown(
	UINT   nFlags,
	CPoint point
)
{
	if( !_IsFlagSet( g_Input.ulState, IS_CAPTURE_MOUSE ) )
		Screen_ShowMousePointer( TRUE );

	CFrameWnd::OnRButtonDown( nFlags, point );

} /* #OF# CMainFrame:OnRButtonDown */

/*========================================================
Method   : CMainFrame::OnMButtonDown
=========================================================*/
/* #FN#
   The framework calls this member function when the user presses the middle mouse button
   (when the mouse cursor is over the status bar) */
void
/* #AS#
   Nothing */
CMainFrame::
OnMButtonDown(
	UINT nFlags, /* #IN# Indicates whether various virtual keys are down */
	CPoint point /* #IN# Specifies the x- and y-coordinate of the cursor */
)
{
	if( !_IsFlagSet( g_Input.ulState, IS_CAPTURE_MOUSE ) )
		Screen_ShowMousePointer( TRUE );

	CFrameWnd::OnMButtonDown( nFlags, point );

} /* #OF# CMainFrame:OnMButtonDown */

/*========================================================
Method   : CMainFrame::CleanScreen
=========================================================*/
/* #FN#
   Clear the whole screen (including menu bar) */
void
/* #AS#
   Nothing */
CMainFrame::
CleanScreen(
	BOOL bDeepClear /*=FALSE*/
)
{
	if( ST_FLIPPED_BUFFERS && !_IsFlagSet( g_Screen.ulState, SM_ATTR_NO_MENU ) )
		/* Don't be afraid of this, Screen_Clear() doesn't clear the menu
		   bar unless the condition below is false or the bDeepClear is
		   set. We need to redraw the menu bar to properly remove it when
		   pause is active and the run-emulation option is selected */
//		   && !ST_ATARI_STOPPED )
	{
		/* This is necessary for multiple-buffering modes to clean up the
		   menus... (when the user closes the dialog placed on the menu
		   or borders area) */
		Screen_ShowMenuBar( TRUE );
	}
	/* Clean up the screen */
	Screen_Clear( FALSE, bDeepClear );

	g_Screen.bLocked = FALSE;

	if( m_pMainApp )
		m_pMainApp->ResetLoopCounter();

} /* #OF# CMainFrame::CleanScreen */
