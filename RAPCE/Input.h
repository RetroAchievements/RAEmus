/******************************************************************************
	[Input.h]

		入力系のインタフェイスを定義します．

		Define input interface.

	Copyright (C) 2004 Ki

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
******************************************************************************/
#ifndef INPUT_INTERFACE_H_INCLUDED
#define INPUT_INTERFACE_H_INCLUDED

#include <stdio.h>
#include <dinput.h>
#include "TypeDefs.h"

#define INPUT_NUM_BUTTON	24 //Kitao更新。ハットスイッチ＋16ボタンパッドにも対応。


enum KeyType
{
	INPUT_JOYSTICK_UP       = 0x00000001,
	INPUT_JOYSTICK_DOWN     = 0x00000002,
	INPUT_JOYSTICK_LEFT     = 0x00000004,
	INPUT_JOYSTICK_RIGHT    = 0x00000008,
	INPUT_JOYSTICK_POVUP    = 0x00000010,//Kitao追加。ハットスイッチにも対応。
	INPUT_JOYSTICK_POVDOWN  = 0x00000020,//Kitao追加
	INPUT_JOYSTICK_POVLEFT  = 0x00000040,//Kitao追加
	INPUT_JOYSTICK_POVRIGHT = 0x00000080,//Kitao追加
	INPUT_JOYSTICK_BUTTON1  = 0x00000100,
	INPUT_JOYSTICK_BUTTON2  = 0x00000200,
	INPUT_JOYSTICK_BUTTON3  = 0x00000400,
	INPUT_JOYSTICK_BUTTON4  = 0x00000800,
	INPUT_JOYSTICK_BUTTON5  = 0x00001000,
	INPUT_JOYSTICK_BUTTON6  = 0x00002000,
	INPUT_JOYSTICK_BUTTON7  = 0x00004000,
	INPUT_JOYSTICK_BUTTON8  = 0x00008000,
	INPUT_JOYSTICK_BUTTON9  = 0x00010000,
	INPUT_JOYSTICK_BUTTON10 = 0x00020000,
	INPUT_JOYSTICK_BUTTON11 = 0x00040000,
	INPUT_JOYSTICK_BUTTON12 = 0x00080000,
	INPUT_JOYSTICK_BUTTON13 = 0x00100000,//Kitao追加
	INPUT_JOYSTICK_BUTTON14 = 0x00200000,//Kitao追加
	INPUT_JOYSTICK_BUTTON15 = 0x00400000,//Kitao追加
	INPUT_JOYSTICK_BUTTON16 = 0x00800000 //Kitao追加
};


//Kitao追加。多人数プレイでもキーボードを使えるようにした。そのために、各PCEパッドの設定変数を用意。
typedef struct
{
	Sint16	buttonU; //上キーの設定。アスキーコード(1～255)。ジョイパッド１(300～399。100ボタンまで対応)。ジョイパッド２(400～499)。以下ジョイパッド５本まで同様。
	Sint16	buttonR; //右キーの設定
	Sint16	buttonD; //下キーの設定
	Sint16	buttonL; //左キーの設定
	Sint16	buttonSel; //Selectボタンの設定
	Sint16	buttonRun; //Runボタンの設定
	Sint16	button1; //Iボタンの設定
	Sint16	button2; //IIボタンの設定
	Sint16	button3; //IIIボタンの設定
	Sint16	button4; //IVボタンの設定
	Sint16	button5; //Vボタンの設定
	Sint16	button6; //VIボタンの設定
} PCEPAD;


BOOL	INPUT_Init();
void	INPUT_Deinit();

//Kitao更新
void
INPUT_UpdateState(
	BOOL	bRecord);

BOOL
INPUT_IsPressed(Sint32 padID, Sint32 userButtonID);

BOOL	INPUT_IsTriggered(Sint32 userButtonID);

//Kitao更新。App.cで読み込んだ_PcePad[]を、こちらにも反映させる関数に。
void
INPUT_ConnectButton(
	Sint32	padID,
	PCEPAD*	pcepad);

void	INPUT_Record(BOOL bRecord);
void	INPUT_PlayRecord(BOOL bPlayrecord); //Kitao更新

Sint32	INPUT_GetNumJoystick();

BOOL	INPUT_WriteBuffer(FILE*	fp);
BOOL	INPUT_ReadBuffer(FILE* fp);

//Kitao追加
void
INPUT_Acquire();

//Kitao追加
BOOL
INPUT_CheckButtonState(
	Sint32	a);

//Kitao追加。各ゲームごとの自動操作設定をリセット(オフに)する。
void
INPUT_ResetGameSetting();

//Kitao追加
void
INPUT_SetKonamiStereo(
	BOOL	bKonamiStereo);

//Kitao追加
void
INPUT_SetGradiusII(
	BOOL	bGradiusII);

#endif // INPUT_INTERFACE_H_INCLUDED
