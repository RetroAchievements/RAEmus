/*-----------------------------------------------------------------------------
	{WinMain.h]

	Copyright (C) 2004 Ki

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
-----------------------------------------------------------------------------*/
#ifndef WINMAIN_H_INCLUDED
#define WINMAIN_H_INCLUDED

#include "TypeDefs.h"


//Kitao追加。MAINのCAPTIONは他からも使うようにしたのでここで定義
#ifdef DEBUGBUILD
#define MAINCAPTION		"Ootake DEBUG BUILD " __DATE__
#else
#define MAINCAPTION		"Ootake/RAPCE"
#endif


enum UserDefinedWM
{
	WM_SET_FULLSCREEN_WS = WM_APP,
	WM_SET_NORMAL_WS,
};

/* メニューコマンドは 100 から定義する */
enum MenuCommand
{
	WM_OPEN_FILE = 100,
	WM_CD_CHANGE, //Kitao追加
	WM_RESET_EMULATOR,
	WM_RUN_EMULATOR,
	WM_STOP_EMULATOR,
	WM_TRACE_1_FRAME,
	WM_TRACE_10_FRAME,
	WM_TRACE_100_FRAME,
	WM_PAUSE_BUTTON, //Kitao追加
	WM_WRITE_MEMORY, //Kitao追加
	WM_ABOUT_WRITEMEM, //Kitao追加
	WM_SET_RESUME, //Kitao追加
	WM_AUTO_RESUME, //Kitao追加
	WM_SAVE_STATE,
	WM_SAVE_STATE_1, //Kitao追加
	WM_SAVE_STATE_2, //Kitao追加
	WM_SAVE_STATE_3, //Kitao追加
	WM_SAVE_STATE_4, //Kitao追加
	WM_SAVE_STATE_5, //Kitao追加
	WM_SAVE_STATE_6, //Kitao追加
	WM_SAVE_STATE_7, //Kitao追加
	WM_SAVE_STATE_8, //Kitao追加
	WM_SAVE_STATE_9, //Kitao追加
	WM_SAVE_STATE_10, //Kitao追加
	WM_SAVE_STATE_11, //Kitao追加
	WM_SAVE_STATE_12, //Kitao追加
	WM_SAVE_STATE_13, //Kitao追加
	WM_SAVE_STATE_14, //Kitao追加
	WM_SAVE_STATE_15, //Kitao追加
	WM_SAVE_STATE_16, //Kitao追加
	WM_SAVE_STATE_17, //Kitao追加
	WM_SAVE_STATE_18, //Kitao追加
	WM_SAVE_STATE_19, //Kitao追加
	WM_SAVE_STATE_20, //Kitao追加
	WM_SAVE_STATE_21, //Kitao追加
	WM_SAVE_STATE_22, //Kitao追加
	WM_SAVE_STATE_23, //Kitao追加
	WM_SAVE_STATE_24, //Kitao追加
	WM_SAVE_STATE_25, //Kitao追加
	WM_SAVE_STATE_26, //Kitao追加
	WM_SAVE_STATE_27, //Kitao追加
	WM_SAVE_STATE_28, //Kitao追加
	WM_SAVE_STATE_29, //Kitao追加
	WM_SAVE_STATE_30, //Kitao追加
	WM_SAVE_STATE_31, //Kitao追加
	WM_SAVE_STATE_32, //Kitao追加
	WM_SAVE_STATE_33, //Kitao追加
	WM_SAVE_STATE_34, //Kitao追加
	WM_SAVE_STATE_35, //Kitao追加
	WM_SAVE_STATE_36, //Kitao追加
	WM_SAVE_STATE_37, //Kitao追加
	WM_SAVE_STATE_38, //Kitao追加
	WM_SAVE_STATE_39, //Kitao追加
	WM_SAVE_STATE_40, //Kitao追加
	WM_SAVE_STATE_41, //Kitao追加
	WM_SAVE_STATE_42, //Kitao追加
	WM_SAVE_STATE_43, //Kitao追加
	WM_SAVE_STATE_44, //Kitao追加
	WM_SAVE_STATE_45, //Kitao追加
	WM_SAVE_STATE_46, //Kitao追加
	WM_SAVE_STATE_47, //Kitao追加
	WM_SAVE_STATE_48, //Kitao追加
	WM_SAVE_STATE_49, //Kitao追加
	WM_SAVE_STATE_50, //Kitao追加
	WM_LOAD_STATE,
	WM_LOAD_STATE_P, //Kitao追加
	WM_LOAD_STATE_1, //Kitao追加
	WM_LOAD_STATE_2, //Kitao追加
	WM_LOAD_STATE_3, //Kitao追加
	WM_LOAD_STATE_4, //Kitao追加
	WM_LOAD_STATE_5, //Kitao追加
	WM_LOAD_STATE_6, //Kitao追加
	WM_LOAD_STATE_7, //Kitao追加
	WM_LOAD_STATE_8, //Kitao追加
	WM_LOAD_STATE_9, //Kitao追加
	WM_LOAD_STATE_10, //Kitao追加
	WM_LOAD_STATE_11, //Kitao追加
	WM_LOAD_STATE_12, //Kitao追加
	WM_LOAD_STATE_13, //Kitao追加
	WM_LOAD_STATE_14, //Kitao追加
	WM_LOAD_STATE_15, //Kitao追加
	WM_LOAD_STATE_16, //Kitao追加
	WM_LOAD_STATE_17, //Kitao追加
	WM_LOAD_STATE_18, //Kitao追加
	WM_LOAD_STATE_19, //Kitao追加
	WM_LOAD_STATE_20, //Kitao追加
	WM_LOAD_STATE_21, //Kitao追加
	WM_LOAD_STATE_22, //Kitao追加
	WM_LOAD_STATE_23, //Kitao追加
	WM_LOAD_STATE_24, //Kitao追加
	WM_LOAD_STATE_25, //Kitao追加
	WM_LOAD_STATE_26, //Kitao追加
	WM_LOAD_STATE_27, //Kitao追加
	WM_LOAD_STATE_28, //Kitao追加
	WM_LOAD_STATE_29, //Kitao追加
	WM_LOAD_STATE_30, //Kitao追加
	WM_LOAD_STATE_31, //Kitao追加
	WM_LOAD_STATE_32, //Kitao追加
	WM_LOAD_STATE_33, //Kitao追加
	WM_LOAD_STATE_34, //Kitao追加
	WM_LOAD_STATE_35, //Kitao追加
	WM_LOAD_STATE_36, //Kitao追加
	WM_LOAD_STATE_37, //Kitao追加
	WM_LOAD_STATE_38, //Kitao追加
	WM_LOAD_STATE_39, //Kitao追加
	WM_LOAD_STATE_40, //Kitao追加
	WM_LOAD_STATE_41, //Kitao追加
	WM_LOAD_STATE_42, //Kitao追加
	WM_LOAD_STATE_43, //Kitao追加
	WM_LOAD_STATE_44, //Kitao追加
	WM_LOAD_STATE_45, //Kitao追加
	WM_LOAD_STATE_46, //Kitao追加
	WM_LOAD_STATE_47, //Kitao追加
	WM_LOAD_STATE_48, //Kitao追加
	WM_LOAD_STATE_49, //Kitao追加
	WM_LOAD_STATE_50, //Kitao追加
	WM_SAVE_BUTTON, //Kitao追加
	WM_LOAD_BUTTON, //Kitao追加
	WM_SAVE_DEFAULT, //Kitao追加
	WM_FOLDER_STATE, //Kitao追加
	WM_RECORDING_GAMEPLAY,
	WM_PLAYRECORD_GAMEPLAY,
	WM_RECORDING_1, //Kitao追加
	WM_RECORDING_2, //Kitao追加
	WM_RECORDING_3, //Kitao追加
	WM_RECORDING_4, //Kitao追加
	WM_RECORDING_5, //Kitao追加
	WM_RECORDING_6, //Kitao追加
	WM_RECORDING_7, //Kitao追加
	WM_RECORDING_8, //Kitao追加
	WM_RECORDING_9, //Kitao追加
	WM_RECORDING_10, //Kitao追加
	WM_RECORDING_HELP, //Kitao追加
	WM_PLAYRECORD_1, //Kitao追加
	WM_PLAYRECORD_2, //Kitao追加
	WM_PLAYRECORD_3, //Kitao追加
	WM_PLAYRECORD_4, //Kitao追加
	WM_PLAYRECORD_5, //Kitao追加
	WM_PLAYRECORD_6, //Kitao追加
	WM_PLAYRECORD_7, //Kitao追加
	WM_PLAYRECORD_8, //Kitao追加
	WM_PLAYRECORD_9, //Kitao追加
	WM_PLAYRECORD_10, //Kitao追加
	WM_MOVERECORD_1, //Kitao追加
	WM_MOVERECORD_2, //Kitao追加
	WM_MOVERECORD_3, //Kitao追加
	WM_MOVERECORD_4, //Kitao追加
	WM_MOVERECORD_5, //Kitao追加
	WM_MOVERECORD_6, //Kitao追加
	WM_MOVERECORD_7, //Kitao追加
	WM_MOVERECORD_8, //Kitao追加
	WM_MOVERECORD_9, //Kitao追加
	WM_MOVERECORD_10, //Kitao追加
	WM_FOLDER_GAMEPLAY, //Kitao追加
	WM_FOLDER_BRAM, //Kitao追加
	WM_FOLDER_MB128, //Kitao追加

	WM_RECENT_1, //Kitao追加
	WM_RECENT_2, //Kitao追加
	WM_RECENT_3, //Kitao追加
	WM_RECENT_4, //Kitao追加
	WM_RECENT_5, //Kitao追加
	WM_RECENT_6, //Kitao追加
	WM_RECENT_7, //Kitao追加
	WM_RECENT_8, //Kitao追加
	WM_RECENT_9, //Kitao追加
	WM_RECENT_10, //Kitao追加
	WM_RECENT_11, //Kitao追加
	WM_RECENT_12, //Kitao追加
	WM_RECENT_13, //Kitao追加
	WM_RECENT_14, //Kitao追加
	WM_RECENT_15, //Kitao追加
	WM_RECENT_16, //Kitao追加
	WM_RECENT_17, //Kitao追加
	WM_RECENT_18, //Kitao追加
	WM_RECENT_19, //Kitao追加
	WM_RECENT_20, //Kitao追加

	WM_SCREEN_CS, //Kitao追加
	WM_SCREEN_X1,
	WM_SCREEN_X2,
	WM_SCREEN_X3,
	WM_SCREEN_X4,
	WM_SCREEN_F1,
	WM_SCREEN_F2,
	WM_SCREEN_DISABLEAERO, //Kitao追加
	WM_SCREEN_USEAERO, //Kitao追加
	WM_SCREEN_GAMMA, //Kitao追加
	WM_SCREEN_GAMMA1,
	WM_SCREEN_GAMMA2,
	WM_SCREEN_GAMMA3,
	WM_SCREEN_GAMMA4,
	WM_SCREEN_GAMMA5,
	WM_SCREEN_GAMMA6,
	WM_SCREEN_GAMMA7,
	WM_SCREEN_BRIGHT, //Kitao追加
	WM_SCREEN_BRIGHT1,
	WM_SCREEN_BRIGHT2,
	WM_SCREEN_BRIGHT3,
	WM_SCREEN_BRIGHT4,
	WM_SCREEN_BRIGHT5,
	WM_SCREEN_BRIGHT6,
	WM_SCREEN_BRIGHT7,
	WM_SCREEN_BRIGHT8,
	WM_SCREEN_BRIGHT9,
	WM_SCREEN_MINIMIZE, //Kitao追加
	WM_SCREEN_NONSTRETCHED, //Kitao追加
	WM_SCREEN_STRETCHED,
	WM_SCREEN_FULLSTRETCHED, //Kitao追加
	WM_SCREEN_VERTICAL, //Kitao追加
	WM_SCREEN_TV, //Kitao追加
	WM_SCREEN_MONOCOLOR, //Kitao追加
	WM_SCREEN_SHOWOVERSCAN, //Kitao追加
	WM_SCREEN_OVERTB, //Kitao追加
	WM_SCREEN_OVERTOP, //Kitao追加
	WM_SCREEN_OVERBOTTOM, //Kitao追加
	WM_SCREEN_OVERNONETB, //Kitao追加
	WM_SCREEN_OVERHEIGHT8, //Kitao追加
	WM_SCREEN_OVERHEIGHT7, //Kitao追加
	WM_SCREEN_OVERHEIGHT6, //Kitao追加
	WM_SCREEN_OVERHEIGHT4, //Kitao追加
	WM_SCREEN_OVERHEIGHT2, //Kitao追加
	WM_SCREEN_OVERHEIGHT1, //Kitao追加
	WM_SCREEN_OVERLR, //Kitao追加
	WM_SCREEN_OVERNONELR, //Kitao追加
	WM_SCREEN_OVERSTART, //Kitao追加
	WM_SCREEN_OVERBLACK, //Kitao追加
	WM_SCREEN_SOVERTB, //Kitao追加
	WM_SCREEN_SOVERTOP, //Kitao追加
	WM_SCREEN_SOVERBOTTOM, //Kitao追加
	WM_SCREEN_SOVERNONETB, //Kitao追加
	WM_SCREEN_SOVERHEIGHT8, //Kitao追加
	WM_SCREEN_SOVERHEIGHT7, //Kitao追加
	WM_SCREEN_SOVERHEIGHT6, //Kitao追加
	WM_SCREEN_SOVERHEIGHT4, //Kitao追加
	WM_SCREEN_SOVERHEIGHT2, //Kitao追加
	WM_SCREEN_SOVERHEIGHT1, //Kitao追加
	WM_SCREEN_SOVERLR, //Kitao追加
	WM_SCREEN_SOVERNONELR, //Kitao追加
	WM_SCREEN_FULLSCREEN,
	WM_SCREEN_FULLSCREEN640, //Kitao追加
	WM_SCREEN_FULLSCREENCS1, //Kitao追加
	WM_SCREEN_FULLSCREENCS2, //Kitao追加
	WM_SCREEN_FULLSCREENCS3, //Kitao追加
	WM_SCREEN_FULLSCREENCSA, //Kitao追加
	WM_SCREEN_SCANLINE, //Kitao追加
	WM_SCREEN_SPSCANLINED, //Kitao追加
	WM_SCREEN_STARTTV, //Kitao追加
	WM_SCREEN_HRSCANLINED, //Kitao追加
	WM_SCREEN_NONSCANLINED, //Kitao追加
	WM_SCREEN_SCANDENSITY, //Kitao追加
	WM_SCREEN_SCANDENSITY0, //Kitao追加
	WM_SCREEN_SCANDENSITY10, //Kitao追加
	WM_SCREEN_SCANDENSITY20, //Kitao追加
	WM_SCREEN_SCANDENSITY30, //Kitao追加
	WM_SCREEN_SCANDENSITY40, //Kitao追加
	WM_SCREEN_SCANDENSITY50, //Kitao追加
	WM_SCREEN_SCANDENSITY60, //Kitao追加
	WM_SCREEN_SCANDENSITY70, //Kitao追加
	WM_SCREEN_SCANDENSITY80, //Kitao追加
	WM_SCREEN_SCANGAMMA, //Kitao追加
	WM_SCREEN_SYNC_VBLANK,
	WM_SCREEN_SYNC_WINDOWS, //Kitao追加
	WM_SCREEN_SYNC_WINDOWSF, //Kitao追加
	WM_SCREEN_SYNC_NON, //Kitao追加
	WM_SCREEN_SYNC_ADJUST, //Kitao追加
	WM_SCREEN_DIRECT3D, //Kitao追加
	WM_SCREEN_DIRECTDRAW, //Kitao追加
	WM_SCREEN_SETDIRECT3D, //Kitao追加
	WM_SCREEN_SETDIRECTDRAW, //Kitao追加
	WM_SCREEN_USE3DT_LINEAR, //Kitao追加
	WM_SCREEN_USE3DT_POINT, //Kitao追加
	WM_SCREEN_USE3DT_NONE, //Kitao追加
	WM_SCREEN_USE3DR_LINEAR, //Kitao追加
	WM_SCREEN_USE3DR_POINT, //Kitao追加
	WM_SCREEN_USE3DR_NONE, //Kitao追加
	WM_SCREEN_USE3D_HELP, //Kitao追加
	WM_SCREEN_USE_VIDEOMEM, //Kitao追加
	WM_SCREEN_USE_SYSTEMMEM, //Kitao追加
	WM_SCREEN_USE_SYSTEMMEMW, //Kitao追加
	WM_SCREEN_FULL16BITCOLOR, //Kitao追加
	WM_SCREEN_FULL32BITCOLOR, //Kitao追加
	WM_SCREEN_STARTWINDOW, //Kitao追加
	WM_SCREEN_STARTFULL, //Kitao追加
	WM_SCREEN_TOPMOST, //Kitao追加
	WM_SCREEN_ACTIVATE, //Kitao追加
	WM_SCREEN_NONACTIVATE, //Kitao追加
	WM_SCREEN_UNPAUSE, //Kitao追加

	WM_INPUT_TWO_BUTTON_PAD,
	WM_INPUT_THR_BUTTON_PAD, //Kitao追加
	WM_INPUT_SIX_BUTTON_PAD,
	WM_INPUT_MOUSE,
	WM_INPUT_MULTI_TAP,
	WM_INPUT_MB128,
	WM_INPUT_SWAP_SELRUN, //Kitao追加
	WM_INPUT_SWAP_IANDII, //Kitao追加
	WM_INPUT_TURBO_1, //Kitao追加
	WM_INPUT_TURBO_2, //Kitao追加
	WM_INPUT_TURBO_RUN, //Kitao追加
	WM_INPUT_TURBO_HIGH, //Kitao追加
	WM_INPUT_TURBO_MIDDLE, //Kitao追加
	WM_INPUT_TURBO_LOW, //Kitao追加
	WM_INPUT_TURBO_OFF, //Kitao追加
	WM_INPUT_CHECKPAD_LR, //Kitao追加

	WM_INPUT_CONFIGURE_PAD1,
	WM_INPUT_CONFIGURE_PAD2,
	WM_INPUT_CONFIGURE_PAD3,
	WM_INPUT_CONFIGURE_PAD4,
	WM_INPUT_CONFIGURE_PAD5,
	WM_INPUT_CONFIGURE_TB1,
	WM_INPUT_CONFIGURE_TB2,
	WM_INPUT_CONFIGURE_TB3,
	WM_INPUT_CONFIGURE_TB4,
	WM_INPUT_CONFIGURE_TB5,
	WM_INPUT_CONFIGURE_INIT, //Kitao追加
	WM_INPUT_CONFIGURE_KEYBG, //Kitao追加
	WM_INPUT_CONFIGURE_JOYBG, //Kitao追加
	WM_INPUT_FUNCTION, //Kitao追加
	WM_INPUT_FB_CURSOR, //Kitao追加
	WM_INPUT_FB_IandII, //Kitao追加
	WM_INPUT_FB_SEL, //Kitao追加
	WM_INPUT_FB_RUN, //Kitao追加
	WM_INPUT_FB_VSPEEDUP, //Kitao追加
	WM_INPUT_FB_SAVELOAD, //Kitao追加

	WM_AUDIO_SR96000,		/* not supported */
	WM_AUDIO_SR88200,		/* not supported */
	WM_AUDIO_SR64000,
	WM_AUDIO_SR48000,
	WM_AUDIO_SR44100,
	WM_AUDIO_SR32000,
	WM_AUDIO_SR22050,
	WM_AUDIO_SR11025,

	WM_AUDIO_SB1024, //Kitao更新
	WM_AUDIO_SB1152,
	WM_AUDIO_SB1280,
	WM_AUDIO_SB1408,
	WM_AUDIO_SB1536,
	WM_AUDIO_SB1664,
	WM_AUDIO_SB1792,
	WM_AUDIO_SB2048,
	WM_AUDIO_SB2176,
	WM_AUDIO_SB2304,
	WM_AUDIO_SB2560,
	WM_AUDIO_SB3072,
	WM_AUDIO_HQPSG1, //Kitao追加
	WM_AUDIO_HQPSG2, //Kitao追加
	WM_AUDIO_HQPSG3, //Kitao追加

	WM_AUDIO_STEREO, //Kitao追加
	WM_AUDIO_MONO, //Kitao追加

	WM_AUDIO_INI1, //Kitao追加
	WM_AUDIO_INI2, //Kitao追加

	WM_AUDIO_NORMALBUFFER, //Kitao追加
	WM_AUDIO_BIGBUFFER, //Kitao追加
	WM_AUDIO_MOSTBUFFER, //Kitao追加

	WM_AUDIO_CDDAAUTO, //Kitao追加
	WM_AUDIO_CDDA593, //Kitao追加
	WM_AUDIO_CDDA594, //Kitao追加
	WM_AUDIO_CDDA595, //Kitao追加
	WM_AUDIO_CDDA596, //Kitao追加
	WM_AUDIO_CDDA597, //Kitao追加
	WM_AUDIO_CDDA598, //Kitao追加
	WM_AUDIO_CDDA599, //Kitao追加
	WM_AUDIO_CDDA600, //Kitao追加
	WM_AUDIO_CDDA601, //Kitao追加
	WM_AUDIO_CDDA602, //Kitao追加
	WM_AUDIO_CDDAP000, //Kitao追加
	WM_AUDIO_CDDAP005, //Kitao追加
	WM_AUDIO_SYNC_VBLANK, //Kitao追加
	WM_AUDIO_DELAYFRAME0, //Kitao追加
	WM_AUDIO_DELAYFRAME1, //Kitao追加
	WM_AUDIO_DELAYFRAME2, //Kitao追加
	WM_AUDIO_DELAYFRAME3, //Kitao追加
	WM_AUDIO_DELAYFRAME4, //Kitao追加

	WM_AUDIO_SETVOLUME,
	WM_AUDIO_DEFAULTVOLUME, //Kitao追加

	WM_VOLUME_TEMP, //Kitao追加
	WM_VOLUME_NORMAL, //Kitao追加
	WM_VOLUME_3QUARTERS, //Kitao追加
	WM_VOLUME_HALF, //Kitao追加
	WM_VOLUME_QUARTER, //Kitao追加
	WM_VOLUME_MUTE, //Kitao追加
	WM_VOLUME_DEFAULT, //Kitao追加
	WM_VOLUME_12, //Kitao追加
	WM_VOLUME_11, //Kitao追加
	WM_VOLUME_10, //Kitao追加
	WM_VOLUME_9, //Kitao追加
	WM_VOLUME_8, //Kitao追加
	WM_VOLUME_7, //Kitao追加
	WM_VOLUME_6, //Kitao追加
	WM_VOLUME_5, //Kitao追加
	WM_VOLUME_4, //Kitao追加
	WM_VOLUME_3, //Kitao追加
	WM_VOLUME_2, //Kitao追加
	WM_VOLUME_1, //Kitao追加
	WM_VOLUME_UP, //Kitao追加
	WM_VOLUME_DOWN, //Kitao追加
	WM_VOLUME_STEP, //Kitao追加
	WM_VOLUME_STEP10, //Kitao追加
	WM_VOLUME_STEP8, //Kitao追加
	WM_VOLUME_STEP6, //Kitao追加
	WM_VOLUME_STEP5, //Kitao追加
	WM_VOLUME_STEP4, //Kitao追加
	WM_VOLUME_STEP3, //Kitao追加
	WM_VOLUME_STEP2, //Kitao追加
	WM_VOLUME_STEP1, //Kitao追加
	WM_VOLUME_DETAIL, //Kitao追加
	WM_VOLUME_DETAILUP, //Kitao追加
	WM_VOLUME_DETAILDN, //Kitao追加
	WM_VOLUME_ATTENTION, //Kitao追加
	WM_VOLUME_CONTROL, //Kitao追加
	WM_VOLUME_MUTEPSG, //Kitao追加
	WM_VOLUME_MUTE1, //Kitao追加
	WM_VOLUME_MUTE2, //Kitao追加
	WM_VOLUME_MUTE3, //Kitao追加
	WM_VOLUME_MUTE4, //Kitao追加
	WM_VOLUME_MUTE5, //Kitao追加
	WM_VOLUME_MUTE6, //Kitao追加
	WM_VOLUME_MUTEA, //Kitao追加
	WM_VOLUME_MUTEU, //Kitao追加

	WM_INFO_SHOWFPS, //Kitao追加
	WM_INFO_TESTDELAY, //Kitao追加
	WM_INFO_MANUENGLISH, //Kitao追加
	WM_INFO_MANUJAPANESE, //Kitao追加
	WM_INFO_README, //Kitao追加
	WM_INFO_HOMEPAGE, //Kitao追加
	WM_INFO_VERSION, //Kitao追加

	WM_DEVICE_CD0, //Kitao追加
	WM_DEVICE_CD1, //Kitao追加
	WM_DEVICE_CD2, //Kitao追加
	WM_DEVICE_CD3, //Kitao追加
	WM_DEVICE_CD4, //Kitao追加
	WM_DEVICE_CD5, //Kitao追加
	WM_DEVICE_CD6, //Kitao追加
	WM_DEVICE_CD7, //Kitao追加
	WM_DEVICE_CD8, //Kitao追加
	WM_DEVICE_CD9, //Kitao追加
	WM_CD_PLAYINSTALL, //Kitao追加
	WM_CD_INSTALL, //Kitao追加
	WM_CD_FULLINSTALL, //Kitao追加
	WM_CD_UNINSTALL, //Kitao追加
	WM_CD_OPENINSTALL, //Kitao追加
	WM_CD_SETSYSCARD, //Kitao追加
	WM_CD_SETSYSCARD1, //Kitao追加
	WM_CD_SETSYSCARD2, //Kitao追加
	WM_CD_JUUOUKI, //Kitao追加
	WM_CD_OSYSCARD1, //Kitao追加
	WM_CD_OSYSCARD2, //Kitao追加
	WM_CD_BACKUPFULL, //Kitao追加
	WM_CD_ARCADECARD, //Kitao追加
	WM_CD_HELP, //Kitao追加

	WM_CHANGE_CDC, //Kitao追加
	WM_CHANGE_CD0, //Kitao追加
	WM_CHANGE_CD1, //Kitao追加
	WM_CHANGE_CD2, //Kitao追加
	WM_CHANGE_CD3, //Kitao追加
	WM_CHANGE_CD4, //Kitao追加
	WM_CHANGE_CD5, //Kitao追加
	WM_CHANGE_CD6, //Kitao追加
	WM_CHANGE_CD7, //Kitao追加
	WM_CHANGE_CD8, //Kitao追加
	WM_CHANGE_CD9, //Kitao追加
	WM_F1_NORESET, //Kitao追加

	WM_STARTFASTCD_ON, //Kitao追加
	WM_STARTFASTCD_OFF, //Kitao追加
	WM_STARTFASTCD_PRE, //Kitao追加

	WM_STARTFASTSEEK_ON, //Kitao追加
	WM_STARTFASTSEEK_OFF, //Kitao追加
	WM_STARTFASTSEEK_PRE, //Kitao追加

	WM_AUTO_GRADIUS2, //Kitao追加
	WM_AUTO_MEGATEN, //Kitao追加
	WM_STARTSPRITE_OFF, //Kitao追加
	WM_STARTSPRITE_ON, //Kitao追加
	WM_STARTSPRITE_PRE, //Kitao追加

	WM_MENU_HIDEMENU, //Kitao追加
	WM_MENU_HIDEMESSAGE, //Kitao追加

	WM_PRIORITY_HIGH, //Kitao追加
	WM_PRIORITY_NORMAL, //Kitao追加

	WM_SPEED_V0, //Kitao追加
	WM_SPEED_V1, //Kitao追加
	WM_SPEED_V2, //Kitao追加
	WM_SPEED_V3, //Kitao追加
	WM_SPEED_V4, //Kitao追加
	WM_SPEED_V5, //Kitao追加
	WM_SPEED_V6, //Kitao追加
	WM_SPEED_V7, //Kitao追加
	WM_SPEED_V8, //Kitao追加
	WM_SPEED_V9, //Kitao追加
	WM_SPEED_VAL, //Kitao追加
	WM_SPEED_VUSE, //Kitao追加
	WM_SPEED_VSET, //Kitao追加
	WM_SPEED_CPU, //Kitao追加
	WM_SPEED_P0, //Kitao追加
	WM_SPEED_T3, //Kitao追加
	WM_SPEED_T2, //Kitao追加
	WM_SPEED_T1, //Kitao追加
	WM_SPEED_P6, //Kitao追加
	WM_SPEED_P5, //Kitao追加
	WM_SPEED_P4, //Kitao追加
	WM_SPEED_P3, //Kitao追加
	WM_SPEED_P2, //Kitao追加
	WM_SPEED_P1, //Kitao追加
	WM_SPEED_M1, //Kitao追加
	WM_SPEED_M2, //Kitao追加
	WM_SPEED_CSET, //Kitao追加
	WM_SPEED_UNLOAD, //Kitao追加
	WM_SPEED_LOAD, //Kitao追加
	WM_SPEED_FASTCD, //Kitao追加
	WM_SPEED_FASTSEEK, //Kitao追加
	WM_SPEED_HELP, //Kitao追加

	WM_RASTERTIMING_MEARLY, //Kitao追加
	WM_RASTERTIMING_EARLY, //Kitao追加
	WM_RASTERTIMING_MIDDLE, //Kitao追加
	WM_RASTERTIMING_LATE, //Kitao追加
	WM_RASTERTIMING_MLATE, //Kitao追加

	WM_INVALIDATE_CDINST, //Kitao追加
	WM_SUPERGRAFX, //Kitao追加
	WM_SPRITEOVER, //Kitao追加

	WM_LAYER_SPRITE, //Kitao追加
	WM_LAYER_BG, //Kitao追加
	WM_LAYER_SPRITE2, //Kitao追加
	WM_LAYER_BG2, //Kitao追加

	WM_BIT_CONVERT, //Kitao追加
	WM_SHOW_DEBUG, //Kitao追加

	WM_ALL_DEFAULT, //Kitao追加

	WM_EXIT,

	WM_OUTPUT_SCREENSHOT, //Kitao追加
	WM_FOLDER_SCREENSHOT, //Kitao追加
	WM_SCREENSHOT_BUTTON, //Kitao追加
	WM_SCREENSHOT_DEFAULT, //Kitao追加
	WM_SCREENSHOT_X1, //Kitao追加
	WM_SCREENSHOT_X2, //Kitao追加
	WM_SCREENSHOT_X3, //Kitao追加
	WM_SCREENSHOT_X4, //Kitao追加
	WM_SCREENSHOT_XN, //Kitao追加
	WM_SCREENSHOT_SIZE, //Kitao追加
	WM_SSHOT_SAMEPLAYING, //Kitao追加
	WM_SSHOT_SCANLINED, //Kitao追加
	WM_SSHOT_TVMODE, //Kitao追加
	WM_SSHOT_HRSCANLINED, //Kitao追加
	WM_SSHOT_NONSCANLINED, //Kitao追加
	WM_SSHOT_SAVEFOLDER, //Kitao追加
	WM_SSHOT_SAVEDIALOG, //Kitao追加

	WM_OUTPUT_WAV1, //Kitao追加
	WM_OUTPUT_WAV123, //Kitao追加
	WM_OUTPUT_WAV12, //Kitao追加
	WM_OUTPUT_WAV2, //Kitao追加
	WM_OUTPUT_WAV3, //Kitao追加
	WM_OUTPUT_WAV0, //Kitao追加
	WM_OUTPUT_WAVS1, //Kitao追加
	WM_OUTPUT_WAVS2, //Kitao追加
	WM_OUTPUT_WAVNT, //Kitao追加
	WM_OUTPUT_WAVBE, //Kitao追加
	WM_OUTPUT_AVI, //Kitao追加
	WM_ABOUT_AVI //Kitao追加
};


extern int	__main__(int argc, char** argv);


const HINSTANCE
WINMAIN_GetHInstance();

const HWND
WINMAIN_GetHwnd();

void
WINMAIN_SetFullScreenWindow(
	Sint32		width,
	Sint32		height);

void
WINMAIN_SetNormalWindow(
	Sint32		width,
	Sint32		height);

void
WINMAIN_SetCaption(
	const char*		pCaption);

//Kitao追加
char*
WINMAIN_GetCaption();

Uint32
WINMAIN_ShowCursor(
	BOOL		bShow);

//Kitao追加
void
WINMAIN_SetPriority(
	DWORD	dwPriorityClass);

//Kitao追加
void
WINMAIN_SafetySleepZero(
	DWORD	startTime,
	DWORD	endTime);


//Kitao追加。デバッグ用
void
WINMAIN_SetBreakTrap(
	BOOL bBrakTrap);

//Kitao追加。デバッグ用
BOOL
WINMAIN_GetBreakTrap();


#endif /* WINMAIN_H_INCLUDED */
