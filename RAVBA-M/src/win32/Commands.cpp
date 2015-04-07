#include "stdafx.h"
#include "AcceleratorManager.h"
#include "resource.h"
#include <afxres.h>

#include <afxtempl.h>  // MFC Templates extension
#ifndef CMapStringToWord
typedef CMap< CString, LPCTSTR, WORD, WORD& > CMapStringToWord;
#endif

static CMapStringToWord winAccelStrings;
static bool initialized = false;

struct {
  const TCHAR* command;
  WORD id;
} winAccelCommands[] = {
  { _T("FileOpenGBA"), ID_FILE_OPEN_GBA },
  { _T("FileOpenGBC"), ID_FILE_OPEN_GBC },
  { _T("FileOpenGB"), ID_FILE_OPEN_GB },
  { _T("FileLoad"), ID_FILE_LOAD },
  { _T("FileSave"), ID_FILE_SAVE },
  { _T("FileLoadGame01"), ID_FILE_LOADGAME_SLOT1 },
  { _T("FileLoadGame02"), ID_FILE_LOADGAME_SLOT2 },
  { _T("FileLoadGame03"), ID_FILE_LOADGAME_SLOT3 },
  { _T("FileLoadGame04"), ID_FILE_LOADGAME_SLOT4 },
  { _T("FileLoadGame05"), ID_FILE_LOADGAME_SLOT5 },
  { _T("FileLoadGame06"), ID_FILE_LOADGAME_SLOT6 },
  { _T("FileLoadGame07"), ID_FILE_LOADGAME_SLOT7 },
  { _T("FileLoadGame08"), ID_FILE_LOADGAME_SLOT8 },
  { _T("FileLoadGame09"), ID_FILE_LOADGAME_SLOT9 },
  { _T("FileLoadGame10"), ID_FILE_LOADGAME_SLOT10 },
  { _T("FileLoadGameAutoLoad"), ID_FILE_LOADGAME_AUTOLOADMOSTRECENT },
  { _T("FileLoadGameRecent"), ID_FILE_LOADGAME_MOSTRECENT },
  { _T("FileSaveGame01"), ID_FILE_SAVEGAME_SLOT1 },
  { _T("FileSaveGame02"), ID_FILE_SAVEGAME_SLOT2 },
  { _T("FileSaveGame03"), ID_FILE_SAVEGAME_SLOT3 },
  { _T("FileSaveGame04"), ID_FILE_SAVEGAME_SLOT4 },
  { _T("FileSaveGame05"), ID_FILE_SAVEGAME_SLOT5 },
  { _T("FileSaveGame06"), ID_FILE_SAVEGAME_SLOT6 },
  { _T("FileSaveGame07"), ID_FILE_SAVEGAME_SLOT7 },
  { _T("FileSaveGame08"), ID_FILE_SAVEGAME_SLOT8 },
  { _T("FileSaveGame09"), ID_FILE_SAVEGAME_SLOT9 },
  { _T("FileSaveGame10"), ID_FILE_SAVEGAME_SLOT10 },
  { _T("FileSaveGameOldest"), ID_FILE_SAVEGAME_OLDESTSLOT },
  { _T("FileRecentReset"), ID_FILE_RECENT_RESET },
  { _T("FileRecentFreeze"), ID_FILE_RECENT_FREEZE },
  { _T("FileRecent01"), ID_FILE_MRU_FILE1 },
  { _T("FileRecent02"), ID_FILE_MRU_FILE2 },
  { _T("FileRecent03"), ID_FILE_MRU_FILE3 },
  { _T("FileRecent04"), ID_FILE_MRU_FILE4 },
  { _T("FileRecent05"), ID_FILE_MRU_FILE5 },
  { _T("FileRecent06"), ID_FILE_MRU_FILE6 },
  { _T("FileRecent07"), ID_FILE_MRU_FILE7 },
  { _T("FileRecent08"), ID_FILE_MRU_FILE8 },
  { _T("FileRecent09"), ID_FILE_MRU_FILE9 },
  { _T("FileRecent10"), ID_FILE_MRU_FILE10 },
  { _T("FilePause"), ID_FILE_PAUSE },
  { _T("FileReset"), ID_FILE_RESET },
  { _T("FileImportBatteryFile"), ID_FILE_IMPORT_BATTERYFILE },
  { _T("FileImportGamesharkCodeFile"), ID_FILE_IMPORT_GAMESHARKCODEFILE },
  { _T("FileImportGamesharkActionReplaySnapshot"), ID_FILE_IMPORT_GAMESHARKSNAPSHOT },
  { _T("FileExportBatteryFile"), ID_FILE_EXPORT_BATTERYFILE },
  { _T("FileExportGamesharkSnapshot"), ID_FILE_EXPORT_GAMESHARKSNAPSHOT },
  { _T("FileScreenCapture"), ID_FILE_SCREENCAPTURE },
  { _T("FileRomInformation"), ID_FILE_ROMINFORMATION },
  { _T("FileToggleFullscreen"), ID_FILE_TOGGLEMENU },
  { _T("FileClose"), ID_FILE_CLOSE },
  { _T("FileExit"), ID_FILE_EXIT },
  { _T("OptionsFrameSkip0"), ID_OPTIONS_VIDEO_FRAMESKIP_0 },
  { _T("OptionsFrameSkip1"), ID_OPTIONS_VIDEO_FRAMESKIP_1 },
  { _T("OptionsFrameSkip2"), ID_OPTIONS_VIDEO_FRAMESKIP_2 },
  { _T("OptionsFrameSkip3"), ID_OPTIONS_VIDEO_FRAMESKIP_3 },
  { _T("OptionsFrameSkip4"), ID_OPTIONS_VIDEO_FRAMESKIP_4 },
  { _T("OptionsFrameSkip5"), ID_OPTIONS_VIDEO_FRAMESKIP_5 },
  { _T("OptionsFrameSkip6"), ID_OPTIONS_VIDEO_FRAMESKIP_6 },
  { _T("OptionsFrameSkip7"), ID_OPTIONS_VIDEO_FRAMESKIP_7 },
  { _T("OptionsFrameSkip8"), ID_OPTIONS_VIDEO_FRAMESKIP_8 },
  { _T("OptionsFrameSkip9"), ID_OPTIONS_VIDEO_FRAMESKIP_9 },
  { _T("OptionsThrottleNone"), ID_OPTIONS_FRAMESKIP_THROTTLE_NOTHROTTLE },
  { _T("OptionsThrottle025%"), ID_OPTIONS_FRAMESKIP_THROTTLE_25 },
  { _T("OptionsThrottle050%"), ID_OPTIONS_FRAMESKIP_THROTTLE_50 },
  { _T("OptionsThrottle100%"), ID_OPTIONS_FRAMESKIP_THROTTLE_100 },
  { _T("OptionsThrottle150%"), ID_OPTIONS_FRAMESKIP_THROTTLE_150 },
  { _T("OptionsThrottle200%"), ID_OPTIONS_FRAMESKIP_THROTTLE_200 },
  { _T("OptionsThrottleOther"), ID_OPTIONS_FRAMESKIP_THROTTLE_OTHER },
  { _T("OptionsVideoRenderDDRAW"), ID_OPTIONS_VIDEO_RENDERMETHOD_DIRECTDRAW },
  { _T("OptionsVideoRenderD3D"), ID_OPTIONS_VIDEO_RENDERMETHOD_DIRECT3D },
  { _T("OptionsVideoRenderOGL"), ID_OPTIONS_VIDEO_RENDERMETHOD_OPENGL },
  { _T("OptionsVideoVsync"), ID_OPTIONS_VIDEO_VSYNC },
  { _T("OptionsVideoX1"), ID_OPTIONS_VIDEO_X1 },
  { _T("OptionsVideoX2"), ID_OPTIONS_VIDEO_X2 },
  { _T("OptionsVideoX3"), ID_OPTIONS_VIDEO_X3 },
  { _T("OptionsVideoX4"), ID_OPTIONS_VIDEO_X4 },
  { _T("OptionsVideoX5"), ID_OPTIONS_VIDEO_X5 },
  { _T("OptionsVideoX6"), ID_OPTIONS_VIDEO_X6 },
  { _T("OptionsVideo320x240"), ID_OPTIONS_VIDEO_FULLSCREEN320X240 },
  { _T("OptionsVideo640x480"), ID_OPTIONS_VIDEO_FULLSCREEN640X480 },
  { _T("OptionsVideo800x600"), ID_OPTIONS_VIDEO_FULLSCREEN800X600 },
  { _T("OptionsVideoFullscreen"), ID_OPTIONS_VIDEO_FULLSCREEN },
  { _T("OptionsVideoFullscreenMaxScale"), ID_OPTIONS_VIDEO_FULLSCREENMAXSCALE },
  { _T("OptionsVideoLayersBG0"), ID_OPTIONS_VIDEO_LAYERS_BG0 },
  { _T("OptionsVideoLayersBG1"), ID_OPTIONS_VIDEO_LAYERS_BG1 },
  { _T("OptionsVideoLayersBG2"), ID_OPTIONS_VIDEO_LAYERS_BG2 },
  { _T("OptionsVideoLayersBG3"), ID_OPTIONS_VIDEO_LAYERS_BG3 },
  { _T("OptionsVideoLayersOBJ"), ID_OPTIONS_VIDEO_LAYERS_OBJ },
  { _T("OptionsVideoLayersWIN0"), ID_OPTIONS_VIDEO_LAYERS_WIN0 },
  { _T("OptionsVideoLayersWIN1"), ID_OPTIONS_VIDEO_LAYERS_WIN1 },
  { _T("OptionsVideoLayersOBJWIN"), ID_OPTIONS_VIDEO_LAYERS_OBJWIN },
  { _T("OptionsVideoLayersReset"), ID_OPTIONS_VIDEO_LAYERS_RESET },
  { _T("OptionsEmulatorAssociate"), ID_OPTIONS_EMULATOR_ASSOCIATE },
  { _T("OptionsEmulatorDirectories"), ID_OPTIONS_EMULATOR_DIRECTORIES },
  { _T("OptionsEmulatorGameOverrides"), ID_OPTIONS_EMULATOR_GAMEOVERRIDES },
  { _T("OptionsEmulatorShowSpeedNone"), ID_OPTIONS_EMULATOR_SHOWSPEED_NONE },
  { _T("OptionsEmulatorShowSpeedPercentage"), ID_OPTIONS_EMULATOR_SHOWSPEED_PERCENTAGE },
  { _T("OptionsEmulatorShowSpeedDetailed"), ID_OPTIONS_EMULATOR_SHOWSPEED_DETAILED },
  { _T("OptionsEmulatorShowSpeedTransparent"), ID_OPTIONS_EMULATOR_SHOWSPEED_TRANSPARENT },
  { _T("OptionsEmulatorSpeedupToggle"), ID_OPTIONS_EMULATOR_SPEEDUPTOGGLE },
  { _T("OptionsEmulatorSaveAuto"), ID_OPTIONS_EMULATOR_SAVETYPE_AUTOMATIC },
  { _T("OptionsEmulatorSaveEEPROM"), ID_OPTIONS_EMULATOR_SAVETYPE_EEPROM },
  { _T("OptionsEmulatorSaveSRAM"), ID_OPTIONS_EMULATOR_SAVETYPE_SRAM },
  { _T("OptionsEmulatorSaveFLASH"), ID_OPTIONS_EMULATOR_SAVETYPE_FLASH },
  { _T("OptionsEmulatorSaveEEPROMSensor"), ID_OPTIONS_EMULATOR_SAVETYPE_EEPROMSENSOR },
  { _T("OptionsEmulatorSaveFlash64K"), ID_OPTIONS_EMULATOR_SAVETYPE_FLASH512K },
  { _T("OptionsEmulatorSaveFlash128K"), ID_OPTIONS_EMULATOR_SAVETYPE_FLASH1M },
  { _T("OptionsEmulatorSaveDetectNow"), ID_OPTIONS_EMULATOR_SAVETYPE_DETECTNOW },
  { _T("OptionsEmulatorAutoApplyPatchFiles"), ID_OPTIONS_EMULATOR_AUTOMATICALLYAPPLYPATCHFILES },
  { _T("OptionsEmulatorAGBPrint"), ID_OPTIONS_EMULATOR_AGBPRINT },
  { _T("OptionsEmulatorRTC"), ID_OPTIONS_EMULATOR_REALTIMECLOCK },
  { _T("OptionsEmulatorRewindInterval"), ID_OPTIONS_EMULATOR_REWINDINTERVAL },
  { _T("OptionsSoundChannel1"), ID_OPTIONS_SOUND_CHANNEL1 },
  { _T("OptionsSoundChannel2"), ID_OPTIONS_SOUND_CHANNEL2 },
  { _T("OptionsSoundChannel3"), ID_OPTIONS_SOUND_CHANNEL3 },
  { _T("OptionsSoundChannel4"), ID_OPTIONS_SOUND_CHANNEL4 },
  { _T("OptionsSoundDirectSoundA"), ID_OPTIONS_SOUND_DIRECTSOUNDA },
  { _T("OptionsSoundDirectSoundB"), ID_OPTIONS_SOUND_DIRECTSOUNDB },
  { _T("OptionsGameboyBorder"), ID_OPTIONS_GAMEBOY_BORDER },
  { _T("OptionsGameboyBorderAutomatic"), ID_OPTIONS_GAMEBOY_BORDERAUTOMATIC },
  { _T("OptionsGameboyColors"), ID_OPTIONS_GAMEBOY_COLORS },
  { _T("OptionsFilterNormal"), ID_OPTIONS_FILTER_NORMAL },
  { _T("OptionsFilterTVMode"), ID_OPTIONS_FILTER_TVMODE },
  { _T("OptionsFilter2xSaI"), ID_OPTIONS_FILTER_2XSAI },
  { _T("OptionsFilterSuper2xSaI"), ID_OPTIONS_FILTER_SUPER2XSAI },
  { _T("OptionsFilterSuperEagle"), ID_OPTIONS_FILTER_SUPEREAGLE },
  { _T("OptionsFilterPixelate"), ID_OPTIONS_FILTER16BIT_PIXELATEEXPERIMENTAL },
  { _T("OptionsFilterMotionBlur"), ID_OPTIONS_FILTER16BIT_MOTIONBLUREXPERIMENTAL },
  { _T("OptionsFilterAdMameScale2x"), ID_OPTIONS_FILTER16BIT_ADVANCEMAMESCALE2X },
  { _T("OptionsFilterSimple2x"), ID_OPTIONS_FILTER16BIT_SIMPLE2X },
  { _T("OptionsFilterBilinear"), ID_OPTIONS_FILTER_BILINEAR },
  { _T("OptionsFilterBilinearPlus"), ID_OPTIONS_FILTER_BILINEARPLUS },
  { _T("OptionsFilterScanlines"), ID_OPTIONS_FILTER_SCANLINES },
  { _T("OptionsFilterHq2x"), ID_OPTIONS_FILTER_HQ2X },
  { _T("OptionsFilterLq2x"), ID_OPTIONS_FILTER_LQ2X },
  { _T("OptionsFilterIFBNone"), ID_OPTIONS_FILTER_INTERFRAMEBLENDING_NONE },
  { _T("OptionsFilterIFBMotionBlur"), ID_OPTIONS_FILTER_INTERFRAMEBLENDING_MOTIONBLUR },
  { _T("OptionsFilterIFBSmart"), ID_OPTIONS_FILTER_INTERFRAMEBLENDING_SMART },
  { _T("OptionsFilterDisableMMX"), ID_OPTIONS_FILTER_DISABLEMMX },
  { _T("OptionsJoypadConfigure1"), ID_OPTIONS_JOYPAD_CONFIGURE_1 },
  { _T("OptionsJoypadConfigure2"), ID_OPTIONS_JOYPAD_CONFIGURE_2 },
  { _T("OptionsJoypadConfigure3"), ID_OPTIONS_JOYPAD_CONFIGURE_3 },
  { _T("OptionsJoypadConfigure4"), ID_OPTIONS_JOYPAD_CONFIGURE_4 },
  { _T("OptionsJoypadMotionConfigure"), ID_OPTIONS_JOYPAD_MOTIONCONFIGURE },
  { _T("OptionsJoypadAutofireA"), ID_OPTIONS_JOYPAD_AUTOFIRE_A },
  { _T("OptionsJoypadAutofireB"), ID_OPTIONS_JOYPAD_AUTOFIRE_B },
  { _T("OptionsJoypadAutofireL"), ID_OPTIONS_JOYPAD_AUTOFIRE_L },
  { _T("OptionsJoypadAutofireR"), ID_OPTIONS_JOYPAD_AUTOFIRE_R },
  { _T("CheatsSearch"), ID_CHEATS_SEARCHFORCHEATS },
  { _T("CheatsList"), ID_CHEATS_CHEATLIST },
  { _T("CheatsLoad"), ID_CHEATS_LOADCHEATLIST },
  { _T("CheatsSave"), ID_CHEATS_SAVECHEATLIST },
  { _T("CheatsDisable"), ID_CHEATS_DISABLECHEATS },
  { _T("ToolsDebugGDB"), ID_TOOLS_DEBUG_GDB },
  { _T("ToolsDebugGDBLoad"), ID_TOOLS_DEBUG_LOADANDWAIT },
  { _T("ToolsDebugGDBBreak"), ID_TOOLS_DEBUG_BREAK },
  { _T("ToolsDebugGDBDisconnect"), ID_TOOLS_DEBUG_DISCONNECT },
  { _T("ToolsDisassemble"), ID_TOOLS_DISASSEMBLE },
  { _T("ToolsIOViewer"), ID_TOOLS_IOVIEWER },
  { _T("ToolsLogging"), ID_TOOLS_LOGGING },
  { _T("ToolsMapViewer"), ID_TOOLS_MAPVIEW },
  { _T("ToolsMemoryViewer"), ID_TOOLS_MEMORYVIEWER },
  { _T("ToolsOAMViewer"), ID_TOOLS_OAMVIEWER },
  { _T("ToolsPaletteViewer"), ID_TOOLS_PALETTEVIEW },
  { _T("ToolsTileViewer"), ID_TOOLS_TILEVIEWER },
  { _T("ToolsNextFrame"), ID_DEBUG_NEXTFRAME },
  { _T("ToolsRecordSoundStartRecording"), ID_OPTIONS_SOUND_STARTRECORDING },
  { _T("ToolsRecordSoundStopRecording"), ID_OPTIONS_SOUND_STOPRECORDING },
  { _T("ToolsRecordAVIStartRecording"), ID_TOOLS_RECORD_STARTAVIRECORDING },
  { _T("ToolsRecordAVIStopRecording"), ID_TOOLS_RECORD_STOPAVIRECORDING },
  { _T("ToolsRecordMovieStartRecording"), ID_TOOLS_RECORD_STARTMOVIERECORDING },
  { _T("ToolsRecordMovieStopRecording"), ID_TOOLS_RECORD_STOPMOVIERECORDING },
  { _T("ToolsPlayMovieStartPlaying"), ID_TOOLS_PLAY_STARTMOVIEPLAYING },
  { _T("ToolsPlayMovieStopPlaying"), ID_TOOLS_PLAY_STOPMOVIEPLAYING },
  { _T("ToolsRewind"), ID_TOOLS_REWIND },
  { _T("ToolsCustomize"), ID_TOOLS_CUSTOMIZE },
  { _T("HelpBugReport"), ID_HELP_BUGREPORT },
  { _T("HelpFAQ"), ID_HELP_FAQ },
  { _T("HelpAbout"), ID_HELP_ABOUT },
  { _T("SystemMinimize"), ID_SYSTEM_MINIMIZE }
};

bool winAccelGetID(const TCHAR *command, WORD& id)
{
  if(!initialized) {
    int count = sizeof(winAccelCommands)/sizeof(winAccelCommands[0]);

    for(int i = 0; i < count; i++) {
      winAccelStrings.SetAt(winAccelCommands[i].command, winAccelCommands[i].id);
    }
    initialized = true;
  }

  return winAccelStrings.Lookup(command, id) ? true : false;
}

void winAccelAddCommands(CAcceleratorManager& mgr)
{
  int count = sizeof(winAccelCommands)/sizeof(winAccelCommands[0]);

  for(int i = 0; i < count; i++) {
    if(!mgr.AddCommandAccel(winAccelCommands[i].id, winAccelCommands[i].command, false))
      mgr.CreateEntry(winAccelCommands[i].id, winAccelCommands[i].command);
  }

}
