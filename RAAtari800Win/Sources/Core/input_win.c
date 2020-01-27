/****************************************************************************
File    : input_win.c
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# Implementation of input handling API
@(#) #BY# Richard Lawrence, Tomasz Szymankowski
@(#) #LM# 19.10.2003
*/

#define DIRECTINPUT_VERSION		0x0500
#define _WIN32_WINNT			0x0400  /* for LowLevelKeyboardProc purpose */

#include <stdio.h>
#include <dinput.h>
#include <crtdbg.h>
#include "WinConfig.h"
#include "Resource.h"
#include "Helpers.h"
#include "Debug.h"
#include "atari800.h"
#include "globals.h"
#include "macros.h"
#include "display_win.h"
#include "misc_win.h"
#include "sound_win.h"
#include "registry.h"
#include "input_win.h"

/* Number of buttons should be read and used separately for every detected
   input device. DirectInput recognizes up to 32 buttons per device.
*/
#define MAX_BUTTONS_NO		32

#define FN_BTN_PAUSE		1
#define FN_BTN_FULLSPEED	2
#define FN_BTN_SIOPATCH		3

#define FN_BTN_FIRST		FN_BTN_PAUSE
#define FN_BTN_LAST			FN_BTN_SIOPATCH

#define CTRL				AKEY_CTRL
#define SHFT				AKEY_SHFT
#define CTSH				AKEY_SHFT | AKEY_CTRL

/* Public objects */

extern WORD s_anKeysets[][ NUM_KBJOY_KEYS ];
extern BYTE s_anExtKeys[];
extern int  s_anKBTable[];

struct InputCtrl_t g_Input =
{
	DEF_INPUT_STATE,
	{ NUMPAD_JOYSTICK, NO_JOYSTICK, NO_JOYSTICK, NO_JOYSTICK },
	{ 0 },
	0,
	/* Joystick */
	{
		DEF_JOY_SELECTS,
		DEF_AUTOFIRE_STICKS,
		DEF_AUTOFIRE_MODE,
		FALSE,
		s_anKeysets
	},
	/* Keyboard */
	{
		DEF_ARROWS_MODE,
		AKEY_NONE,
		0,
		AKEY_NONE,
		0,
		0,
		INPUT_CONSOL_NONE,
		0,
		s_anExtKeys,
		s_anKBTable
	}
};

/* Private objects */

static int s_anKBTable [ KEYBOARD_TABLE_SIZE ];

static int s_anNormKeys[ KEYBOARD_TABLE_SIZE ] =
{
/*  0*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_BREAK,		
/*  4*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*  8*/		AKEY_BACKSPACE,			AKEY_TAB,				AKEY_NONE,				AKEY_NONE,
/* 12*/		AKEY_CLEAR,				AKEY_RETURN,			AKEY_NONE,				AKEY_NONE,
/* 16*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_BREAK,
/* 20*/		AKEY_CAPSTOGGLE,		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/* 24*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_ESCAPE,
/* 28*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/* 32*/		AKEY_SPACE,				AKEY_CAPSLOCK,			AKEY_HELP,				AKEY_ATARI,
/* 36*/		AKEY_CLEAR,				AKEY_LEFT,				AKEY_UP,				AKEY_RIGHT,
/* 40*/		AKEY_DOWN,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/* 44*/		AKEY_NONE,				AKEY_INSERT_CHAR,		AKEY_DELETE_CHAR,		AKEY_HELP,
/* 48*/		AKEY_0,					AKEY_1,					AKEY_2,					AKEY_3,
/* 52*/		AKEY_4,					AKEY_5,					AKEY_6,					AKEY_7,
/* 56*/		AKEY_8,					AKEY_9,					AKEY_NONE,				AKEY_NONE,
/* 60*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/* 64*/		AKEY_NONE,				AKEY_a,					AKEY_b,					AKEY_c,
/* 68*/		AKEY_d,					AKEY_e,					AKEY_f,					AKEY_g,
/* 72*/		AKEY_h,					AKEY_i,					AKEY_j,					AKEY_k,
/* 76*/		AKEY_l,					AKEY_m,					AKEY_n,					AKEY_o,
/* 80*/		AKEY_p,					AKEY_q,					AKEY_r,					AKEY_s,
/* 84*/		AKEY_t,					AKEY_u,					AKEY_v,					AKEY_w,
/* 88*/		AKEY_x,					AKEY_y,					AKEY_z,					AKEY_ATARI,
/* 92*/		AKEY_ATARI,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/* 96*/		AKEY_0,					AKEY_1,					AKEY_2,					AKEY_3,
/*100*/		AKEY_4,					AKEY_5,					AKEY_6,					AKEY_7,
/*104*/		AKEY_8,					AKEY_9,					AKEY_ASTERISK,			AKEY_PLUS,
/*108*/		AKEY_NONE,				AKEY_MINUS,				AKEY_FULLSTOP,			AKEY_SLASH,
/*112*/		AKEY_HELP,				AKEY_STARTDWN,			AKEY_SELECTDWN,			AKEY_OPTIONDWN,
/*116*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*120*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*124*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*128*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*132*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*136*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*140*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*144*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*148*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*152*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*156*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*160*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*164*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*168*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*172*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*176*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*180*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*184*/		AKEY_NONE,				AKEY_NONE,				AKEY_SEMICOLON,			AKEY_EQUAL,
/*188*/		AKEY_COMMA,				AKEY_MINUS,				AKEY_FULLSTOP,			AKEY_SLASH,
/*192*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*196*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*200*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*204*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*208*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*212*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*216*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_BRACKETLEFT,
/*220*/		AKEY_BACKSLASH,			AKEY_BRACKETRIGHT,		AKEY_QUOTE,				AKEY_NONE,
/*224*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*228*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*232*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*236*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*240*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*244*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*248*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*252*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE
};

static int s_anShftKeys[ KEYBOARD_TABLE_SIZE ] =
{
/*  0*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_BREAK,		
/*  4*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*  8*/		AKEY_DELETE_CHAR,		AKEY_SETTAB,			AKEY_NONE,				AKEY_NONE,
/* 12*/		AKEY_CLEAR,				AKEY_RETURN|SHFT,		AKEY_NONE,				AKEY_NONE,
/* 16*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_BREAK,
/* 20*/		AKEY_CAPSTOGGLE|SHFT,	AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/* 24*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_ESCAPE|SHFT,
/* 28*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/* 32*/		AKEY_SPACE|SHFT,		AKEY_CAPSLOCK,			AKEY_HELP|SHFT,			AKEY_ATARI|SHFT,
/* 36*/		AKEY_CLEAR,				AKEY_LEFT,				AKEY_UP,				AKEY_RIGHT,
/* 40*/		AKEY_DOWN,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/* 44*/		AKEY_NONE,				AKEY_INSERT_LINE,		AKEY_DELETE_LINE,		AKEY_HELP|SHFT,
/* 48*/		AKEY_0|SHFT,			AKEY_1|SHFT,			AKEY_AT,				AKEY_3|SHFT,
/* 52*/		AKEY_4|SHFT,			AKEY_5|SHFT,			AKEY_CARET,				AKEY_AMPERSAND,
/* 56*/		AKEY_ASTERISK,			AKEY_9|SHFT,			AKEY_NONE,				AKEY_NONE,
/* 60*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/* 64*/		AKEY_NONE,				AKEY_A,					AKEY_B,					AKEY_C,
/* 68*/		AKEY_D,					AKEY_E,					AKEY_F,					AKEY_G,
/* 72*/		AKEY_H,					AKEY_I,					AKEY_J,					AKEY_K,
/* 76*/		AKEY_L,					AKEY_M,					AKEY_N,					AKEY_O,
/* 80*/		AKEY_P,					AKEY_Q,					AKEY_R,					AKEY_S,
/* 84*/		AKEY_T,					AKEY_U,					AKEY_V,					AKEY_W,
/* 88*/		AKEY_X,					AKEY_Y,					AKEY_Z,					AKEY_ATARI|SHFT,
/* 92*/		AKEY_ATARI|SHFT,		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/* 96*/		AKEY_0,					AKEY_1,					AKEY_2,					AKEY_3,
/*100*/		AKEY_4,					AKEY_5,					AKEY_6,					AKEY_7,
/*104*/		AKEY_8,					AKEY_9,					AKEY_ASTERISK,			AKEY_PLUS,
/*108*/		AKEY_NONE,				AKEY_MINUS,				AKEY_GREATER,			AKEY_SLASH,
/*112*/		AKEY_HELP|SHFT,			AKEY_STARTDWN,			AKEY_SELECTDWN,			AKEY_OPTIONDWN,
/*116*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*120*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*124*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*128*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*132*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*136*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*140*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*144*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*148*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*152*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*156*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*160*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*164*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*168*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*172*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*176*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*180*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*184*/		AKEY_NONE,				AKEY_NONE,				AKEY_COLON,				AKEY_PLUS,
/*188*/		AKEY_LESS,				AKEY_UNDERSCORE,		AKEY_GREATER,			AKEY_QUESTION,
/*192*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*196*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*200*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*204*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*208*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*212*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*216*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_BRACKETLEFT,
/*220*/		AKEY_BAR,				AKEY_BRACKETRIGHT,		AKEY_DBLQUOTE,			AKEY_NONE,
/*224*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*228*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*232*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*236*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*240*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*244*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*248*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*252*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE
};

static int s_anCtrlKeys[ KEYBOARD_TABLE_SIZE ] =
{
/*  0*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_BREAK,		
/*  4*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*  8*/		AKEY_BACKSPACE|CTRL,	AKEY_CLRTAB,			AKEY_NONE,				AKEY_NONE,
/* 12*/		AKEY_CLEAR,				AKEY_RETURN|CTRL,		AKEY_NONE,				AKEY_NONE,
/* 16*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_CTRL_1,
/* 20*/		AKEY_CAPSTOGGLE|CTRL,	AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/* 24*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_ESCAPE|CTRL,
/* 28*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/* 32*/		AKEY_SPACE|CTRL,		AKEY_CAPSLOCK|CTRL,		AKEY_HELP|CTRL,			AKEY_ATARI|CTRL,
/* 36*/		AKEY_CLEAR,				AKEY_LEFT,				AKEY_UP,				AKEY_RIGHT,
/* 40*/		AKEY_DOWN,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/* 44*/		AKEY_NONE,				AKEY_INSERT_CHAR|CTRL,	AKEY_DELETE_CHAR|CTRL,	AKEY_HELP|CTRL,
/* 48*/		AKEY_0|CTRL,			AKEY_1|CTRL,			AKEY_2|CTRL,			AKEY_3|CTRL,
/* 52*/		AKEY_4|CTRL,			AKEY_5|CTRL,			AKEY_6|CTRL,			AKEY_7|CTRL,
/* 56*/		AKEY_8|CTRL,			AKEY_9|CTRL,			AKEY_NONE,				AKEY_NONE,
/* 60*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/* 64*/		AKEY_NONE,				AKEY_CTRL_a,			AKEY_CTRL_b,			AKEY_CTRL_c,
/* 68*/		AKEY_CTRL_d,			AKEY_CTRL_e,			AKEY_CTRL_f,			AKEY_CTRL_g,
/* 72*/		AKEY_CTRL_h,			AKEY_CTRL_i,			AKEY_CTRL_j,			AKEY_CTRL_k,
/* 76*/		AKEY_CTRL_l,			AKEY_CTRL_m,			AKEY_CTRL_n,			AKEY_CTRL_o,
/* 80*/		AKEY_CTRL_p,			AKEY_CTRL_q,			AKEY_CTRL_r,			AKEY_CTRL_s,
/* 84*/		AKEY_CTRL_t,			AKEY_CTRL_u,			AKEY_CTRL_v,			AKEY_CTRL_w,
/* 88*/		AKEY_CTRL_x,			AKEY_CTRL_y,			AKEY_CTRL_z,			AKEY_ATARI|CTRL,
/* 92*/		AKEY_ATARI|CTRL,		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/* 96*/		AKEY_0,					AKEY_1,					AKEY_2,					AKEY_3,
/*100*/		AKEY_4,					AKEY_5,					AKEY_6,					AKEY_7,
/*104*/		AKEY_8,					AKEY_9,					AKEY_ASTERISK,			AKEY_PLUS,
/*108*/		AKEY_NONE,				AKEY_UNDERSCORE,		AKEY_FULLSTOP,			AKEY_SLASH,
/*112*/		AKEY_HELP|CTRL,			AKEY_STARTDWN,			AKEY_SELECTDWN,			AKEY_OPTIONDWN,
/*116*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*120*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*124*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*128*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*132*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*136*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*140*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*144*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*148*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*152*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*156*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*160*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*164*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*168*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*172*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*176*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*180*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*184*/		AKEY_NONE,				AKEY_NONE,				AKEY_SEMICOLON|CTRL,	AKEY_EQUAL|CTRL,
/*188*/		AKEY_COMMA|CTRL,		AKEY_MINUS|CTRL,		AKEY_FULLSTOP|CTRL,		AKEY_SLASH|CTRL,
/*192*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*196*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*200*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*204*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*208*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*212*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*216*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_BRACKETLEFT,
/*220*/		AKEY_BACKSLASH,			AKEY_BRACKETRIGHT,		AKEY_QUOTE,				AKEY_NONE,
/*224*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*228*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*232*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*236*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*240*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*244*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*248*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*252*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE
};

/* Following keys cannot be read with both shift and control pressed:
   J K L ; + * Z X C V B F1 F2 F3 F4 HELP */
static int s_anShftCtrlKeys[ KEYBOARD_TABLE_SIZE ] =
{
/*  0*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_BREAK|CTSH,		
/*  4*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*  8*/		AKEY_BACKSPACE|CTSH,	AKEY_TAB|CTSH,			AKEY_NONE,				AKEY_NONE,
/* 12*/		AKEY_CLEAR|CTSH,		AKEY_RETURN|CTSH,		AKEY_NONE,				AKEY_NONE,
/* 16*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_CTRL_1,
/* 20*/		AKEY_CAPSTOGGLE|CTSH,	AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/* 24*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_ESCAPE,
/* 28*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/* 32*/		AKEY_SPACE|CTSH,		AKEY_CAPSLOCK|CTSH,		AKEY_NONE,				AKEY_ATARI,
/* 36*/		AKEY_CLEAR|CTSH,		AKEY_LEFT,				AKEY_UP,				AKEY_RIGHT,
/* 40*/		AKEY_DOWN,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/* 44*/		AKEY_NONE,				AKEY_INSERT_CHAR|CTSH,	AKEY_DELETE_CHAR|CTSH,	AKEY_NONE,
/* 48*/		AKEY_0|CTSH,			AKEY_1|CTSH,			AKEY_2|CTSH,			AKEY_3|CTSH,
/* 52*/		AKEY_4|CTSH,			AKEY_5|CTSH,			AKEY_6|CTSH,			AKEY_7|CTSH,
/* 56*/		AKEY_8|CTSH,			AKEY_9|CTSH,			AKEY_NONE,				AKEY_NONE,
/* 60*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/* 64*/		AKEY_NONE,				AKEY_a|CTSH,			AKEY_NONE,				AKEY_NONE,
/* 68*/		AKEY_d|CTSH,			AKEY_e|CTSH,			AKEY_f|CTSH,			AKEY_g|CTSH,
/* 72*/		AKEY_h|CTSH,			AKEY_i|CTSH,			AKEY_NONE,				AKEY_NONE,
/* 76*/		AKEY_NONE,				AKEY_m|CTSH,			AKEY_n|CTSH,			AKEY_o|CTSH,
/* 80*/		AKEY_p|CTSH,			AKEY_q|CTSH,			AKEY_r|CTSH,			AKEY_s|CTSH,
/* 84*/		AKEY_t|CTSH,			AKEY_u|CTSH,			AKEY_NONE,				AKEY_w|CTSH,
/* 88*/		AKEY_NONE,				AKEY_y|CTSH,			AKEY_NONE,				AKEY_ATARI,
/* 92*/		AKEY_ATARI|CTSH,		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/* 96*/		AKEY_0,					AKEY_1,					AKEY_2,					AKEY_3,
/*100*/		AKEY_4,					AKEY_5,					AKEY_6,					AKEY_7,
/*104*/		AKEY_8,					AKEY_9,					AKEY_ASTERISK,			AKEY_PLUS,
/*108*/		AKEY_NONE,				AKEY_MINUS,				AKEY_FULLSTOP,			AKEY_SLASH,
/*112*/		AKEY_NONE,				AKEY_STARTDWN,			AKEY_SELECTDWN,			AKEY_OPTIONDWN,
/*116*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*120*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*124*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*128*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*132*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*136*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*140*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*144*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*148*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*152*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*156*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*160*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*164*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*168*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*172*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*176*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*180*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*184*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_EQUAL|CTSH,
/*188*/		AKEY_COMMA|CTSH,		AKEY_MINUS|CTSH,		AKEY_FULLSTOP|CTSH,		AKEY_SLASH|CTSH,
/*192*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*196*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*200*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*204*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*208*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*212*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*216*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_BRACKETLEFT,
/*220*/		AKEY_BAR,				AKEY_BRACKETRIGHT,		AKEY_DBLQUOTE,			AKEY_NONE,
/*224*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*228*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*232*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*236*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*240*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*244*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*248*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE,
/*252*/		AKEY_NONE,				AKEY_NONE,				AKEY_NONE,				AKEY_NONE
};

static int s_anToggleKeys[ KEYBOARD_TABLE_SIZE ] =
{
/*  0*/	0,	0,	0,	0,	0,	0,	0,	0,
/*  8*/	0,	0,	0,	0,	0,	0,	0,	0,
/* 16*/	1,	1,	0,	0,	0,	0,	0,	0,	// Shift and Control
/* 24*/	0,	0,	0,	0,	0,	0,	0,	0,
/* 32*/	0,	0,	0,	0,	0,	0,	0,	0,
/* 40*/	0,	0,	0,	0,	0,	0,	0,	0,
/* 48*/	0,	0,	0,	0,	0,	0,	0,	0,
/* 56*/	0,	0,	0,	0,	0,	0,	0,	0,
/* 64*/	0,	0,	0,	0,	0,	0,	0,	0,
/* 72*/	0,	0,	0,	0,	0,	0,	0,	0,
/* 80*/	0,	0,	0,	0,	0,	0,	0,	0,
/* 88*/	0,	0,	0,	0,	0,	0,	0,	0,
/* 96*/	0,	0,	0,	0,	0,	0,	0,	0,
/*104*/	0,	0,	0,	0,	0,	0,	0,	0,
/*112*/	0,	1,	1,	1,	0,	0,	0,	0,	// Option, Select, Start
/*120*/	0,	0,	0,	0,	0,	0,	0,	0,
/*128*/	0,	0,	0,	0,	0,	0,	0,	0,
/*136*/	0,	0,	0,	0,	0,	0,	0,	0,
/*144*/	0,	0,	0,	0,	0,	0,	0,	0,
/*152*/	0,	0,	0,	0,	0,	0,	0,	0,
/*160*/	0,	0,	0,	0,	0,	0,	0,	0,
/*168*/	0,	0,	0,	0,	0,	0,	0,	0,
/*176*/	0,	0,	0,	0,	0,	0,	0,	0,
/*184*/	0,	0,	0,	0,	0,	0,	0,	0,
/*192*/	0,	0,	0,	0,	0,	0,	0,	0,
/*200*/	0,	0,	0,	0,	0,	0,	0,	0,
/*208*/	0,	0,	0,	0,	0,	0,	0,	0,
/*216*/	0,	0,	0,	0,	0,	0,	0,	0,
/*224*/	0,	0,	0,	0,	0,	0,	0,	0,
/*232*/	0,	0,	0,	0,	0,	0,	0,	0,
/*240*/	0,	0,	0,	0,	0,	0,	0,	0,
/*248*/	0,	0,	0,	0,	0,	0,	0,	0
};

static UCHAR s_aucCoveredKeys[ KEYBOARD_TABLE_SIZE ] =
{
/*  0*/	0,	0,	0,	0,	0,	0,	0,	0,
/*  8*/	1,	1,	0,	0,	0,	1,	0,	0,	// BACKSPACE TAB RETURN
/* 16*/	0,	0,	0,	0,	1,	0,	0,	0,	// CAPSLOCK
/* 24*/	0,	0,	0,	1,	0,	0,	0,	0,	// ESCAPE
/* 32*/ 1,	0,	0,	0,	0,	1,	1,	1,	// SPACE LEFT UP RIGHT
/* 40*/	1,	0,	0,	0,	0,	0,	0,	0,	// DOWN
/* 48*/	1,	1,	1,	1,	1,	1,	1,	1,	// 0 1 2 3 4 5 6 7
/* 56*/	1,	1,	0,	0,	0,	0,	0,	0,	// 8 9 
/* 64*/	0,	1,	1,	1,	1,	1,	1,	1,	// a b c d e f g
/* 72*/	1,	1,	1,	1,	1,	1,	1,	1,	// h i j k l m n o
/* 80*/	1,	1,	1,	1,	1,	1,	1,	1,	// p q r s t u v w
/* 88*/	1,	1,	1,	0,	0,	0,	0,	0,	// x y z
/* 96*/	1,	1,	1,	1,	1,	1,	1,	1,	// PAD0 PAD1 PAD2 PAD3 PAD4 PAD5 PAD6 PAD7
/*104*/	1,	1,	1,	1,	0,	1,	1,	1,	// PAD8 PAD9 PAD* PAD+ PAD- PAD. PAD/
/*112*/	0,	0,	0,	0,	0,	0,	0,	0,
/*120*/	0,	0,	0,	0,	0,	0,	0,	0,
/*128*/	0,	0,	0,	0,	0,	0,	0,	0,
/*136*/	0,	0,	0,	0,	0,	0,	0,	0,
/*144*/	0,	0,	0,	0,	0,	0,	0,	0,
/*152*/	0,	0,	0,	0,	0,	0,	0,	0,
/*160*/	0,	0,	0,	0,	0,	0,	0,	0,
/*168*/	0,	0,	0,	0,	0,	0,	0,	0,
/*176*/	0,	0,	0,	0,	0,	0,	0,	0,
/*184*/	0,	0,	1,	1,	1,	1,	1,	1,	// ; = , - . /
/*192*/	1,	0,	0,	0,	0,	0,	0,	0,	// `
/*200*/	0,	0,	0,	0,	0,	0,	0,	0,
/*208*/	0,	0,	0,	0,	0,	0,	0,	0,
/*216*/	0,	0,	0,	1,	1,	1,	1,	0,	// [ BACKSLASH ] '
/*224*/	0,	0,	0,	0,	0,	0,	0,	0,
/*232*/	0,	0,	0,	0,	0,	0,	0,	0,
/*240*/	0,	0,	0,	0,	0,	0,	0,	0,
/*248*/	0,	0,	0,	0,	0,	0,	0,	0
};

static const int s_anNormArrows[ 3 ][ 4 ] =
{
	{ AKEY_LEFT, AKEY_UP,    AKEY_RIGHT,    AKEY_DOWN  },
	{ AKEY_PLUS, AKEY_MINUS, AKEY_ASTERISK, AKEY_EQUAL },
	{ AKEY_F3,   AKEY_F1,    AKEY_F4,       AKEY_F2    }
};

static const int s_anShftArrows[ 3 ][ 4 ] =
{
	{ AKEY_NONE,             AKEY_SHFT | AKEY_UP,    AKEY_NONE,                 AKEY_SHFT | AKEY_DOWN  },
	{ AKEY_SHFT | AKEY_PLUS, AKEY_SHFT | AKEY_MINUS, AKEY_SHFT | AKEY_ASTERISK, AKEY_SHFT | AKEY_EQUAL },
	{ AKEY_SHFT | AKEY_F3,   AKEY_SHFT | AKEY_F1,    AKEY_SHFT | AKEY_F4,       AKEY_SHFT | AKEY_F2    }
};

static const int s_anCtrlArrows[ 3 ][ 4 ] =
{
	{ AKEY_PLUS,             AKEY_MINUS,             AKEY_ASTERISK,             AKEY_EQUAL             },
	{ AKEY_CTRL | AKEY_PLUS, AKEY_CTRL | AKEY_MINUS, AKEY_CTRL | AKEY_ASTERISK, AKEY_CTRL | AKEY_EQUAL },
	{ AKEY_CTRL | AKEY_F3,   AKEY_CTRL | AKEY_F1,    AKEY_CTRL | AKEY_F4,       AKEY_CTRL | AKEY_F2    }
};

static const int s_anShftCtrlArrows[ 3 ][ 4 ] =
{
	{ AKEY_SHFT | AKEY_PLUS, AKEY_SHFT | AKEY_MINUS, AKEY_SHFT | AKEY_ASTERISK, AKEY_SHFT | AKEY_EQUAL },
	{ AKEY_NONE,             AKEY_SHFT | AKEY_UP,    AKEY_NONE,                 AKEY_SHFT | AKEY_DOWN  },
	{ AKEY_NONE,             AKEY_NONE,              AKEY_NONE,                 AKEY_NONE              }
};

#ifndef VK_SLEEP
#define VK_SLEEP  0x5F /* NUMPADENTER */
#endif

BYTE s_anExtKeys[ KEYBOARD_TABLE_SIZE ] = /* Extended keys table */
{
/*000*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, VK_SLEEP, 0, 0,
/*016*/ 0, VK_RCONTROL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/*032*/ 0, VK_PRIOR, VK_NEXT, VK_END, VK_HOME, VK_LEFT, VK_UP, VK_RIGHT, VK_DOWN, 0, 0, 0, 0, VK_INSERT, VK_DELETE, 0,
/*048*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/*064*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/*080*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, VK_LWIN, VK_RWIN, 0, 0, 0,
/*096*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, VK_DIVIDE,
/*112*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/*128*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/*144*/ VK_NUMLOCK, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/*160*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/*176*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/*192*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/*208*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/*224*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/*240*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

WORD s_anKeysets[ NUM_KBJOY_DEVICES ][ NUM_KBJOY_KEYS ] =
{
	/* Default is numpad "joystick" */
	{ VK_NUMPAD7, VK_NUMPAD8, VK_NUMPAD9, VK_NUMPAD6, VK_NUMPAD3, VK_NUMPAD2, VK_NUMPAD1, VK_NUMPAD4, VK_NUMPAD5, VK_NUMPAD0 },
	/* Cursor Keys + RIGHT CTRL */
	{ 0, VK_UP, 0, VK_RIGHT, 0, VK_DOWN, 0, VK_LEFT, 0, VK_RCONTROL },
	/* User defined A & B Keysets */
	{ 0, VK_UP, 0, VK_RIGHT, 0, VK_DOWN, 0, VK_LEFT, 0, VK_CONTROL },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

static LPDIRECTINPUT        s_lpDInput = NULL;
static LPDIRECTINPUTDEVICE2 s_alpDevFound[ MAX_INPUT_DEVICES ];
static LPDIRECTINPUTDEVICE2 s_alpJoyPorts[ MAX_ATARI_JOYPORTS ] =
{
	NULL, NULL, NULL, NULL
};

static RECT  s_rcViewBounds   = { 0, 0 };

static HHOOK s_hKeyboardHook  = NULL;
static int   s_nSPIPrevState  = 1;

static BOOL  s_bBtn2Flip      = FALSE;
static BOOL  s_bBtn3Flip      = FALSE;
static BOOL  s_bBtn4Flip      = FALSE;

static int   s_nCurrentMouseX = 0; /* Used in AMIGA, ST and JOY modes */
static int   s_nCurrentMouseY = 0;

static int   s_anStick   [ MAX_ATARI_JOYPORTS ];
static int   s_anTrig    [ MAX_ATARI_JOYPORTS ];

static int   s_anKeystick[ NUM_KBJOY_DEVICES ] = { INPUT_STICK_CENTRE, INPUT_STICK_CENTRE, INPUT_STICK_CENTRE, INPUT_STICK_CENTRE };
static int   s_anKeytrig [ NUM_KBJOY_DEVICES ] = { 1, 1, 1, 1 };

static const int s_anStickTable[ NUM_KBJOY_KEYS - 1 ] =
{
	INPUT_STICK_UL,
	INPUT_STICK_FORWARD,
	INPUT_STICK_UR,
	INPUT_STICK_RIGHT,
	INPUT_STICK_LR,
	INPUT_STICK_BACK,
	INPUT_STICK_LL,
	INPUT_STICK_LEFT,
	INPUT_STICK_CENTRE
};

static BOOL DI_GetErrorString( HRESULT hResult, LPSTR pszErrorBuff, DWORD dwError );

#ifdef _DEBUG
#define ServeDInputError( nUID, hResult, bQuit ) \
		ShowDInputError( nUID, hResult, bQuit, __FILE__, __LINE__ )
#else /*_DEBUG*/
#define ServeDInputError( nUID, hResult, bQuit ) \
		ShowDInputError( nUID, hResult, bQuit )
#endif /*_DEBUG*/


/*========================================================
Function : ShowDInputError
=========================================================*/
/* #FN#
   Fatal DirectInput error function. Since we cannot continue, bring us back
   to a known working state before displaying the error, then start an exit.
   Make sure to shut down the Atari layer on the way. */
static
void
/* #AS#
   Nothing */
ShowDInputError(
	UINT    nUID,    /* #IN# */
	HRESULT hResult, /* #IN# */
	BOOL    bQuit    /* #IN# */
#ifdef _DEBUG
  , char   *pszFile,
	DWORD   dwLine
#endif /*_DEBUG*/
)
{
	char szError [ LOADSTRING_SIZE_S + 1 ];
	char szAction[ LOADSTRING_SIZE_L + 1 ];

#ifdef _DEBUG
	Log_print( "DirectInput error: %s@%ld", pszFile, dwLine );
#endif /*_DEBUG*/

	/* Get us back to a GDI display and stop making noises */
	Screen_SetSafeDisplay( FALSE );
	Sound_Clear( FALSE, FALSE );

	/* Get the error string and present it to the user */
	DI_GetErrorString( hResult, szError, LOADSTRING_SIZE_S );
	DisplayMessage( NULL, IDS_DIERR_PROMPT, IDS_DIERR_HDR, MB_ICONSTOP | MB_OK, _LoadStringLx( nUID, szAction ), szError );

	/* Start a quit (this will end up in Atari_Exit()) */
	if( bQuit )
	{
		/* Make sure the atari is turned off */
		g_ulAtariState = ATARI_UNINITIALIZED;
		PostMessage( g_hMainWnd, WM_CLOSE, 0, 0L );
	}
	else
		Sound_Restart();

} /* #OF# ShowDInputError */

/*========================================================
Function : Input_ResetKeys
=========================================================*/
/* #FN#
   Resets the keys state to default */
void
/* #AS#
   Nothing */
Input_ResetKeys( void )
{
	int i;
	for( i = 0; i < NUM_KBJOY_DEVICES; i++ )
	{
		s_anKeystick[ i ] = INPUT_STICK_CENTRE;
		s_anKeytrig [ i ] = 1;
	}
	g_Input.Key.nCurrentKey = g_Input.Key.nNewKey = AKEY_NONE;
	g_Input.Key.nCurrentVK  = g_Input.Key.nNewVK  = 0;
	g_Input.Key.nConsol     = INPUT_CONSOL_NONE;
	g_Input.Key.nControl    = 0;
	g_Input.Key.nShift      = 0;

	INPUT_key_consol = g_Input.Key.nConsol;
	INPUT_key_shift  = g_Input.Key.nShift;

} /* #OF# Input_ResetKeys */

/*========================================================
Function : Input_Reset
=========================================================*/
/* #FN#
   Resets the input (keys and joys) state to default */
void
/* #AS#
   Nothing */
Input_Reset( void )
{
	int i;

	s_bBtn2Flip = FALSE;
	s_bBtn3Flip = FALSE;
	s_bBtn4Flip = FALSE;

	Input_ResetKeys();

	for( i = 0; i < MAX_ATARI_JOYPORTS; i++ )
	{
		s_anTrig [ i ] = 1;
		s_anStick[ i ] = INPUT_STICK_CENTRE;
	}
} /* #OF# Input_Reset */

/*========================================================
Function : SetDIDwordProperty
=========================================================*/
/* #FN#
   Helper function just makes sure the Property struct is set correctly */
static
HRESULT
/* #AS#
   DI_OK, DI_PROPNOEFFECT, if the methods succeeds. DIERR_OBJECTNOTFOUND,
   DIERR_UNSUPPORTED, DIERR_NOTINITIALIZED, DIERR_INVALIDPARAM, if the
   method fails */
SetDIDwordProperty(
	LPDIRECTINPUTDEVICE lpInputDev,   /* #IN# */
	REFGUID             guidProperty, /* #IN# */
	DWORD               dwObject,     /* #IN# */
	DWORD               dwHow,        /* #IN# */
	DWORD               dwValue       /* #IN# */
)
{
	DIPROPDWORD dipdw;

	dipdw.diph.dwSize       = sizeof(dipdw);
	dipdw.diph.dwHeaderSize = sizeof(dipdw.diph);
	dipdw.diph.dwObj        = dwObject;
	dipdw.diph.dwHow        = dwHow;
	dipdw.dwData            = dwValue;

	return IDirectInputDevice_SetProperty( lpInputDev, guidProperty, &dipdw.diph );

} /* #OF# SetDIDwordProperty */

/*========================================================
Function : InitJoystickInput
=========================================================*/
/* #FN#
   Callback function that will be called with a description of each
   DirectInput device */
static
BOOL CALLBACK
/* #AS#
   Returns DIENUM_CONTINUE to continue the enumeration or DIENUM_STOP
   to stop the enumeration */
InitJoystickInput(
	LPCDIDEVICEINSTANCE lpDevInstance,
	LPVOID pvRef
)
{
	LPDIRECTINPUT       lpDirectInput = pvRef;
	LPDIRECTINPUTDEVICE pdev;
	DIPROPRANGE         diprg;
	HRESULT             hResult;

	/* Create the DirectInput joystick device */
	hResult = IDirectInput_CreateDevice( lpDirectInput, &lpDevInstance->guidInstance, &pdev, NULL );
	if( FAILED(hResult) )
	{
		ServeDInputError( IDS_DIERR_CREATING_DEVICE, hResult, FALSE );
		return DIENUM_STOP;/*DIENUM_CONTINUE;*/
	}
	/* Set joystick data format */
	hResult = IDirectInputDevice_SetDataFormat( pdev, &c_dfDIJoystick );
	if( FAILED(hResult) )
	{
		ServeDInputError( IDS_DIERR_SETTING_JOY_FORMAT, hResult, FALSE );
		IDirectInputDevice_Release( pdev );
		return DIENUM_STOP;
	}
	/* Set the cooperative level */
	hResult = IDirectInputDevice_SetCooperativeLevel( pdev, g_hMainWnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND );
	if( FAILED(hResult) )
	{
		ServeDInputError( IDS_DIERR_SETTING_COOP, hResult, FALSE );
		IDirectInputDevice_Release( pdev );
		return DIENUM_STOP;
	}
	/* Set X-axis range to (-1000 ... +1000)
	   This lets us test against 0 to see which way the s_anStick is pointed */
	diprg.diph.dwSize       = sizeof(diprg);
	diprg.diph.dwHeaderSize = sizeof(diprg.diph);
	diprg.diph.dwObj        = DIJOFS_X;
	diprg.diph.dwHow        = DIPH_BYOFFSET;
	diprg.lMin              = -1000;
	diprg.lMax              = +1000;

	hResult = IDirectInputDevice_SetProperty( pdev, DIPROP_RANGE, &diprg.diph );
	if( FAILED(hResult) )
	{
		ServeDInputError( IDS_DIERR_SETTING_PROPS, hResult, FALSE );
		IDirectInputDevice_Release( pdev );
		return FALSE;
	}

	/* And again for Y-axis range */
	diprg.diph.dwObj = DIJOFS_Y;

	hResult = IDirectInputDevice_SetProperty( pdev, DIPROP_RANGE, &diprg.diph );
	if( FAILED(hResult) )
	{
		ServeDInputError( IDS_DIERR_SETTING_PROPS, hResult, FALSE );
		IDirectInputDevice_Release( pdev );
		return FALSE;
	}
	/* Set X axis dead zone to 50% (to avoid accidental turning)
	   Units are ten thousandths, so 50% = 5000/10000 */
	hResult = SetDIDwordProperty( pdev, DIPROP_DEADZONE, DIJOFS_X, DIPH_BYOFFSET, 5000 );
	if( FAILED(hResult) )
	{
		ServeDInputError( IDS_DIERR_SETTING_DEADZONE, hResult, FALSE );
		IDirectInputDevice_Release( pdev );
		return FALSE;
	}
	/* Set Y axis dead zone to 50% (to avoid accidental thrust)
	   Units are ten thousandths, so 50% = 5000/10000 */
	hResult = SetDIDwordProperty( pdev, DIPROP_DEADZONE, DIJOFS_Y, DIPH_BYOFFSET, 5000 );
	if( FAILED(hResult) )
	{
		ServeDInputError( IDS_DIERR_SETTING_DEADZONE, hResult, FALSE );
		IDirectInputDevice_Release( pdev );
		return FALSE;
	}
	/* Add it to our list of devices */
	hResult = IDirectInputDevice_QueryInterface( pdev, &IID_IDirectInputDevice2, (LPVOID *)&s_alpDevFound[ g_Input.nDevFoundNum ] );
	if( FAILED(hResult) )
	{
		ServeDInputError( IDS_DIERR_ADDING_DEVICE, hResult, FALSE );
		return FALSE;
	}
	_strncpy( g_Input.acDevNames[ g_Input.nDevFoundNum++ ], lpDevInstance->tszInstanceName, INPUT_DEV_NAMELEN );
	IDirectInputDevice_Release( pdev );

	return DIENUM_CONTINUE;

} /* #OF# InitJoystickInput */

/*========================================================
Function : ReacquireInput
=========================================================*/
/* #FN#
   Obtains access to the input device */
static
BOOL
/* #AS#
   TRUE if succeeded, FALSE if failed */
ReacquireInput(
	int nStickNum
)
{
	/* If we have a current device */
	if( s_alpJoyPorts[ nStickNum ] )
	{
		/* Acquire the device */
		if( SUCCEEDED(
			IDirectInputDevice_Acquire( s_alpJoyPorts[ nStickNum ] )) )
			return TRUE;	/* acquisition succeeded */
	}
	/* Acquisition failed or we don't have a current device */
	return FALSE;

} /* #OF# ReacquireInput */

/*========================================================
Function : PickJoystick
=========================================================*/
/* #FN#
   Selects a new device for the pointed stick */
static
void
/* #AS#
   Nothing */
PickJoystick(
	int nStickNum,
	int nJoy
)
{
	int i;
	for( i = 0; i < MAX_ATARI_JOYPORTS; i++ )
	{
		if( s_alpJoyPorts[ i ] != NULL && (nJoy >= 0 && nJoy < MAX_INPUT_DEVICES) &&
			s_alpJoyPorts[ i ] == s_alpDevFound[ nJoy ] )
		{
			IDirectInputDevice_Unacquire( s_alpJoyPorts[ i ] );
			s_alpJoyPorts[ i ] = NULL;
		}
	}
	/* Select a new device for the pointed stick */
	g_Input.anDevSelected[ nStickNum ] = nJoy;

	if( s_alpJoyPorts[ nStickNum ] )
		IDirectInputDevice_Unacquire( s_alpJoyPorts[ nStickNum ] );
	s_alpJoyPorts[ nStickNum ] = NULL;

	g_Input.Joy.bKBJoystick = FALSE;
	for( i = 0; i < MAX_ATARI_JOYPORTS; i++ )
	{
		if( g_Input.anDevSelected[ i ] < 0 )
		{
			g_Input.Joy.bKBJoystick = TRUE;
			break;
		}
	}
	/* (nJoy < 0) means that the keyboard is used instead */
	if( nJoy >= 0 && nJoy < MAX_INPUT_DEVICES && s_alpDevFound[ nJoy ] )
	{
		s_alpJoyPorts[ nStickNum ] = s_alpDevFound[ nJoy ];
		if( !ReacquireInput( nStickNum ) )
			ServeDInputError( IDS_DIERR_ACQUIRE_INIT, DIERR_NOTACQUIRED, FALSE );
	}
} /* #OF# PickJoystick */

/*========================================================
Function : Input_Initialise
=========================================================*/
/* #FN#
   Initializes the input devices */
BOOL
/* #AS#
   TRUE if succeeded, otherwise FALSE */
Input_Initialise(
	BOOL bInitDInput
)
{
	int i;
	/* Set autofire stuff */
	for( i = 0; i < MAX_ATARI_JOYPORTS; i++ )
	{
		if( _IsFlagSet( g_Input.Joy.ulAutoSticks, 1 << i ) )
			INPUT_joy_autofire[ i ] = g_Input.Joy.nAutoMode;
		else
			INPUT_joy_autofire[ i ] = INPUT_AUTOFIRE_OFF;
	}
	Atari800_nframes = 0;

	/* Initialise the DirectInput devices */
	if( bInitDInput )
	{
		char cValue;
		int  nStick;

		Input_Reset();

		if( !s_lpDInput )
		{
			HRESULT	hResult;

			ZeroMemory( s_alpDevFound, sizeof(LPDIRECTINPUTDEVICE) * MAX_INPUT_DEVICES );
			/* Create the DirectInput 5.0 interface object */
			hResult = DirectInputCreate( g_hInstance, DIRECTINPUT_VERSION, &s_lpDInput, NULL );
			if( FAILED(hResult) )
			{
				if( hResult == DIERR_OLDDIRECTINPUTVERSION )
					s_lpDInput = NULL;
				else
				{
					ServeDInputError( IDS_DIERR_CREATING_OBJ, hResult, FALSE );
					return FALSE;
				}
			}
			else
			{
				/* Enumerate the joystick devices */
				hResult = IDirectInput_EnumDevices( s_lpDInput, DIDEVTYPE_JOYSTICK, InitJoystickInput, s_lpDInput, DIEDFL_ATTACHEDONLY );
				if( FAILED(hResult) )
				{
					ServeDInputError( IDS_DIERR_ENUMERATING, hResult, FALSE );
					return FALSE;
				}
				IDirectInput_Release( s_lpDInput );
			}
		}
		cValue = (char)(g_Input.Joy.ulSelected & 0x000000ff);
		nStick = cValue;
		PickJoystick( 0, nStick );

		nStick = g_Input.Joy.ulSelected & 0x0000ff00;
		nStick >>= 8;
		cValue = (char)nStick;
		nStick = cValue;
		if( nStick == g_Input.anDevSelected[ 0 ] )
			nStick = NO_JOYSTICK;
		PickJoystick( 1, nStick );

		nStick = g_Input.Joy.ulSelected & 0x00ff0000;
		nStick >>= 16;
		cValue = (char)nStick;
		nStick = cValue;
		if( nStick == g_Input.anDevSelected[ 0 ] || nStick == g_Input.anDevSelected[ 1 ] )
			nStick = NO_JOYSTICK;
		PickJoystick( 2, nStick );

		nStick = g_Input.Joy.ulSelected & 0xff000000;
		nStick >>= 24;
		cValue = (char)nStick;
		nStick = cValue;
		if( nStick == g_Input.anDevSelected[ 0 ] || nStick == g_Input.anDevSelected[ 1 ] || nStick == g_Input.anDevSelected[ 2 ] )
			nStick = NO_JOYSTICK;
		PickJoystick( 3, nStick );
	}
	return TRUE;

} /* #OF# Input_Initialise */

/*========================================================
Function : Input_Clear
=========================================================*/
/* #FN#
   Release all the created input devices */
void
/* #AS#
   Nothing */
Input_Clear( void )
{
	int i;
	/* Make sure the device is unacquired it doesn't harm
	   to unacquire a device that isn't acquired */
	for( i = 0; i < MAX_ATARI_JOYPORTS; i++ )
	{
		if( s_alpJoyPorts[ i ] )
		{
			IDirectInputDevice_Unacquire( s_alpJoyPorts[ i ] );
			s_alpJoyPorts[ i ] = NULL;
		}
	}
	/* Release all the devices we created */
	for( i = 0; i < g_Input.nDevFoundNum; i++ )
	{
		if( s_alpDevFound[ i ] )
		{
			IDirectInputDevice_Release( s_alpDevFound[ i ] );
			s_alpDevFound[ i ] = NULL;
		}
	}
} /* #OF# Input_Clear */

/*========================================================
Function : Input_ReadJoystick
=========================================================*/
/* #FN#
   Receives the state of joystick input devices */
BOOL
/* #AS#
   TRUE if succeeded, otherwise FALSE */
Input_ReadJoystick(
	int  nStickNum,
	BOOL bTestButton
)
{
	DIJOYSTATE js;
	int     nStickVal = 0;
	int     i;
	HRESULT hResult;

	if( nStickNum >= MAX_INPUT_DEVICES || !s_alpJoyPorts[ nStickNum ] )
		return INPUT_STICK_CENTRE;

	/* Poll the joystick to read the current state */
	IDirectInputDevice2_Poll( s_alpJoyPorts[ nStickNum ] );

	/* Get data from the joystick */
	hResult = IDirectInputDevice_GetDeviceState( s_alpJoyPorts[ nStickNum ], sizeof(DIJOYSTATE), &js );
	if( FAILED(hResult) )
	{
		/* did the read fail because we lost input for some reason?
		   if so, then attempt to reacquire.  If the second acquire
		   fails, then the error from GetDeviceData will be
		   DIERR_NOTACQUIRED, so we won't get stuck an infinite loop. */
		if( hResult == DIERR_INPUTLOST )
		{
			ReacquireInput( nStickNum );
			/* return the fact that we did not read any data */
			return FALSE;
		}
		ServeDInputError( IDS_DIERR_ACQUIRE, DIERR_NOTACQUIRED, FALSE );
	}

	if( Atari800_MACHINE_5200 == Atari800_machine_type )
	{
		for( i = 0; i < MAX_ATARI_JOYPORTS; i++ )
			s_anTrig[ i ] = _IsFlagSet( js.rgbButtons[ i ], 0x80 ) ? 0 : 1;
	}
	else
	{
		/* A questionable realization of this feature, must be changed */
		if( !_IsFlagSet( g_Input.ulState, IS_JOY_FIRE_ONLY ) )
		{
			if( !s_bBtn2Flip )
			{
				if( _IsFlagSet( js.rgbButtons[ FN_BTN_PAUSE ], 0x80 ) )
				{
					/* Description should be changed */
					PostMessage( g_hMainWnd, WM_COMMAND, ID_ATARI_PAUSE, 0L );
					s_bBtn2Flip = TRUE;
				}
			}
			else if( !_IsFlagSet( js.rgbButtons[ FN_BTN_PAUSE ], 0x80 ) )
				s_bBtn2Flip = FALSE;

			if( !s_bBtn3Flip )
			{
				if( _IsFlagSet( js.rgbButtons[ FN_BTN_FULLSPEED ], 0x80 ) )
				{
					/* Description should be changed */
					PostMessage( g_hMainWnd, WM_COMMAND, ID_ATARI_FULLSPEED, 0L );
					s_bBtn3Flip = TRUE;
				}
			}
			else if( !_IsFlagSet( js.rgbButtons[ FN_BTN_FULLSPEED ], 0x80 ) )
				s_bBtn3Flip = FALSE;

			if( !s_bBtn4Flip )
			{
				if( _IsFlagSet( js.rgbButtons[ FN_BTN_SIOPATCH ], 0x80 ) )
				{
					/* Description should be changed */
					PostMessage( g_hMainWnd, WM_COMMAND, ID_ATARI_SIOPATCH, 0L );
					s_bBtn4Flip = TRUE;
				}
			}
			else if( !_IsFlagSet( js.rgbButtons[ FN_BTN_SIOPATCH ], 0x80 ) )
				s_bBtn4Flip = FALSE;
		}

		if( bTestButton )
		{
			s_anTrig[ nStickNum ] = 1;

			/* No special handling, just return whether it is held down */
			for( i = 0; i < MAX_BUTTONS_NO; i++ )
			{
				if( _IsFlagSet( g_Input.ulState, IS_JOY_FIRE_ONLY ) || i < FN_BTN_FIRST || i > FN_BTN_LAST )
				{
					/* Do fire when one button is pressed at last */
					if( _IsFlagSet( js.rgbButtons[ i ], 0x80 ) )
					{
						s_anTrig[ nStickNum ] = 0;
						break;
					}
				}
			}
		}
	}

	if( js.lX < 0 )
	{
		if( js.lY < 0 )
			nStickVal |= INPUT_STICK_UL;
		else
		if( js.lY > 0 )
			nStickVal |= INPUT_STICK_LL;
		else
		if( js.lY == 0 )
			nStickVal |= INPUT_STICK_LEFT;
	}
	else
	if( js.lX > 0 )
	{
		if( js.lY < 0 )
			nStickVal |= INPUT_STICK_UR;
		else
		if( js.lY > 0 )
			nStickVal |= INPUT_STICK_LR;
		else
		if( js.lY == 0 )
			nStickVal |= INPUT_STICK_RIGHT;
	}
	else
	if( js.lX == 0 )
	{
		if( js.lY < 0 )
			nStickVal |= INPUT_STICK_FORWARD;
		else
		if( js.lY > 0 )
			nStickVal |= INPUT_STICK_BACK;
		else
		if( js.lY == 0 )
			nStickVal |= INPUT_STICK_CENTRE;
	}
	s_anStick[ nStickNum ] = nStickVal;

	return TRUE;

} /* #OF# Input_ReadJoystick */

/*========================================================
Function : Atari_Keyboard
=========================================================*/
/* #FN#
   In other Atari800 versions, keyboard handling is here. In Atari800Win
   the actual key processing is handled in the mainframe of the app and by
   the time it gets here we already have the appropriate Atari key code
   (except for keycodes that require multiple logic operations, like the
   s_anStick) */
int
/* #AS#
   Current Atari key value */
Atari_Keyboard( void )
{
	if( AKEY_NONE != g_Input.Key.nCurrentKey )
	{
		if( _IsFlagSet( g_Input.ulState, IS_KEY_TYPEMATIC_RATE ) )
		{
			g_Input.Key.nCurrentKey = AKEY_NONE;
			return 0x09;
		}
		if( GetAsyncKeyState( g_Input.Key.nCurrentVK ) >= 0 )
		{
			g_Input.Key.nCurrentKey = AKEY_NONE;
			return AKEY_NONE;
		}
	}
	if( AKEY_NONE != g_Input.Key.nNewKey )
	{
		if( Atari800_MACHINE_5200 == Atari800_machine_type )
		{
			switch( g_Input.Key.nNewKey )
			{
				case AKEY_STARTDWN:
					g_Input.Key.nNewKey = AKEY_5200_START;	/* start */
					break;

				case AKEY_p:
					g_Input.Key.nNewKey = AKEY_5200_PAUSE;	/* pause */
					break;

				case AKEY_r:
					g_Input.Key.nNewKey = AKEY_5200_RESET;	/* reset (5200 has no warmstart) */
					break;

				case AKEY_0:
					g_Input.Key.nNewKey = AKEY_5200_0;		/* controller numpad keys (0-9) */
					break;

				case AKEY_1:
					g_Input.Key.nNewKey = AKEY_5200_1;
					break;

				case AKEY_2:
					g_Input.Key.nNewKey = AKEY_5200_2;
					break;

				case AKEY_3:
					g_Input.Key.nNewKey = AKEY_5200_3;
					break;

				case AKEY_4:
					g_Input.Key.nNewKey = AKEY_5200_4;
					break;

				case AKEY_5:
					g_Input.Key.nNewKey = AKEY_5200_5;
					break;

				case AKEY_6:
					g_Input.Key.nNewKey = AKEY_5200_6;
					break;

				case AKEY_7:
					g_Input.Key.nNewKey = AKEY_5200_7;
					break;

				case AKEY_8:
					g_Input.Key.nNewKey = AKEY_5200_8;
					break;

				case AKEY_9:
					g_Input.Key.nNewKey = AKEY_5200_9;
					break;

				case AKEY_MINUS:
					g_Input.Key.nNewKey = AKEY_5200_HASH;	/* '#' key on 5200 controller */
					break;

				case AKEY_ASTERISK:
					g_Input.Key.nNewKey = AKEY_5200_ASTERISK;	/* '*' key on 5200 controller */
					break;

				default:
					g_Input.Key.nNewKey = AKEY_NONE;
					break;
			}
		}

		if ( g_Input.Key.nNewKey == AKEY_STARTDWN ||
			g_Input.Key.nNewKey == AKEY_SELECTDWN ||
			g_Input.Key.nNewKey == AKEY_OPTIONDWN )
			g_Input.Key.nNewKey = AKEY_NONE;

		if( AKEY_NONE != g_Input.Key.nNewKey && AKEY_NONE == g_Input.Key.nCurrentKey )
		{
			g_Input.Key.nCurrentKey = g_Input.Key.nNewKey;
			g_Input.Key.nCurrentVK  = g_Input.Key.nNewVK;
			g_Input.Key.nNewKey     = AKEY_NONE;
			g_Input.Key.nNewVK      = 0;
		}
	}
	return g_Input.Key.nCurrentKey;

} /* #OF# Atari_Keyboard */

/*========================================================
Function : Input_UpdateJoystick
=========================================================*/
/* #FN#
   Updates variables are used for joystick emulation */
void
/* #AS#
   Nothing */
Input_UpdateJoystick( void )
{
	int i;
	for( i = 0; i < MAX_ATARI_JOYPORTS; i++ )
	{
		if( s_alpJoyPorts[ i ] )
			Input_ReadJoystick( i, TRUE );
		else
		{
			if( g_Input.anDevSelected[ i ] < 0 )
			{
				s_anStick[ i ] = s_anKeystick[ g_Input.anDevSelected[ i ] + NUM_KBJOY_DEVICES ];
				s_anTrig [ i ] = s_anKeytrig [ g_Input.anDevSelected[ i ] + NUM_KBJOY_DEVICES ];
			}
			else
			{
				s_anStick[ i ] = INPUT_STICK_CENTRE;
				s_anTrig [ i ] = 1;
			}
		}
	}
} /* #OF# Input_UpdateJoystick */

/*========================================================
Function : Atari_PORT
=========================================================*/
/* #FN#
   This function is called by the Atari800 kernel */
int
/* #AS#
   Two stick port states for requested controller port */
Atari_PORT(
	int nNum
)
{
	return (s_anStick[ (nNum << 1) + 1 ] << 4) | s_anStick[ nNum << 1 ];

} /* #OF# Atari_PORT */

/*========================================================
Function : Atari_TRIG
=========================================================*/
/* #FN#
   This function is called by the Atari800 kernel */
int
/* #AS#
   0 if the trigger is pressed, otherwise 1 */
Atari_TRIG(
	int nNum
)
{
	/* The trigger inputs, TRIG0 through TRIG3, are wired to the controller
	   ports, one to a port. The bottom button on either side of the leftmost
	   controller zeroes the TRIG0 register when pressed, and likewise for
	   the other ports.
	*/
	if( nNum >= 0 && nNum <= 3 )
		return s_anTrig[ nNum ];

    return 0;

} /* #OF# Atari_TRIG */

/*========================================================
Function : Input_UpdateMouse
=========================================================*/
/* #FN#
   Updates variables are used for emulation of miscellaneous devices using the mouse */
void
/* #AS#
   Nothing */
Input_UpdateMouse( void )
{
	if( !_IsFlagSet( g_ulAtariState, ATARI_NO_FOCUS ) )
	{
		if( ST_MOUSE_HIDDEN &&
			g_Screen.nShowCursor > -1 && --g_Screen.nShowCursor == 0 )
		{
			Screen_ShowMousePointer( FALSE ); /* Hide the mouse cursor */
		}
		/* The mouse emulation is now fully supported by the Atari800 kernel */
		if( _IsFlagSet( g_Input.ulState, IS_CAPTURE_MOUSE ) &&
			INPUT_MOUSE_OFF != INPUT_mouse_mode )
		{
			POINT pt;
			GetCursorPos( &pt ); /* Read the pointer position */

			if( pt.x >= s_rcViewBounds.right - 1 )
			{
				s_nCurrentMouseX = pt.x = s_rcViewBounds.left + 2;
				SetCursorPos( pt.x, pt.y );
			}
			else
			if( pt.x <= s_rcViewBounds.left + 1 )
			{
				s_nCurrentMouseX = pt.x = s_rcViewBounds.right - 2;
				SetCursorPos( pt.x, pt.y );
			}
			if( pt.y >= s_rcViewBounds.bottom - 1 )
			{
				s_nCurrentMouseY = pt.y = s_rcViewBounds.top + 2;
				SetCursorPos( pt.x, pt.y );
			}
			else
			if( pt.y <= s_rcViewBounds.top + 1 )
			{
				s_nCurrentMouseY = pt.y = s_rcViewBounds.bottom - 2;
				SetCursorPos( pt.x, pt.y );
			}
			INPUT_mouse_delta_x = pt.x - s_nCurrentMouseX;
			INPUT_mouse_delta_y = pt.y - s_nCurrentMouseY;

			s_nCurrentMouseX = pt.x;
			s_nCurrentMouseY = pt.y;
		}
	}
} /* #OF# Input_UpdateMouse */

/*========================================================
Function : Input_ResetMouse
=========================================================*/
/* #FN#
   Cleans up after emulation of miscellaneous devices using the mouse */
void
/* #AS#
   Nothing */
Input_ResetMouse(
	BOOL bCenter
)
{
	if( bCenter )
	{
		INPUT_mouse_delta_x = INPUT_mouse_delta_y = 0;

		s_nCurrentMouseX = s_rcViewBounds.left + (s_rcViewBounds.right - s_rcViewBounds.left) / 2;
		s_nCurrentMouseY = s_rcViewBounds.top + (s_rcViewBounds.bottom - s_rcViewBounds.top) / 2;

		SetCursorPos( s_nCurrentMouseX, s_nCurrentMouseY );

		if( INPUT_MOUSE_PAD != INPUT_mouse_mode )
		{
			INPUT_CenterMousePointer();
		}
	}
	/* There were some problems with light pen cursor, fixed */
	INPUT_mouse_buttons = 0;

} /* #OF# Input_ResetMouse */

/*========================================================
Function : Input_ToggleMouseCapture
=========================================================*/
/* #FN#
   Toggles between the mouse capture mode on and off */
void
/* #AS#
   Nothing */
Input_ToggleMouseCapture( void )
{
	if( _IsFlagSet( g_Input.ulState, IS_CAPTURE_MOUSE ) )
	{
		Screen_ShowMousePointer( TRUE );

		_ClrFlag( g_Input.ulState, IS_CAPTURE_MOUSE );
		ClipCursor( NULL );

		/* Clean up the mouse emulation settings */
		Input_ResetMouse( TRUE );
	}
	else
	{
		_SetFlag( g_Input.ulState, IS_CAPTURE_MOUSE );
		Input_RefreshBounds( g_hViewWnd, TRUE );

		Screen_ShowMousePointer( TRUE );
		/* The mouse will be hidden by Input_UpdateMouse function */
		g_Screen.nShowCursor = 1;
	}
	WriteRegDWORD( NULL, REG_INPUT_STATE, g_Input.ulState );
	WriteRegDWORD( NULL, REG_MOUSE_MODE, INPUT_mouse_mode );

} /* #OF# Input_ToggleMouseCapture */

/*========================================================
Function : Atari_Set_CONSOL
=========================================================*/
/* #FN#
   Sets a consol keys state */
static
void
/* #AS#
   Nothing */
Atari_Set_CONSOL(
	int nNewConsol
)
{
	switch( nNewConsol )
	{
		case AKEY_OPTIONDWN:
			_ClrFlag( g_Input.Key.nConsol, INPUT_CONSOL_OPTION );
			break;

		case AKEY_SELECTDWN:
			_ClrFlag( g_Input.Key.nConsol, INPUT_CONSOL_SELECT );
			break;

		case AKEY_STARTDWN:
			_ClrFlag( g_Input.Key.nConsol, INPUT_CONSOL_START );
			break;

		case AKEY_OPTIONUP:
			_SetFlag( g_Input.Key.nConsol, INPUT_CONSOL_OPTION );
			break;

		case AKEY_SELECTUP:
			_SetFlag( g_Input.Key.nConsol, INPUT_CONSOL_SELECT );
			break;

		case AKEY_STARTUP:
			_SetFlag( g_Input.Key.nConsol, INPUT_CONSOL_START );
			break;
	}
	INPUT_key_consol = g_Input.Key.nConsol;

} /* #OF# Atari_Set_CONSOL */

/*========================================================
Function : Input_SetArrowKeys
=========================================================*/
/* #FN#
   Sets the PC arrows key working mode */
void
/* #AS#
   Nothing */
Input_SetArrowKeys(
	int nMode /* #IN# Code of the requested working mode */
)
{
	int i;
	for( i = 0; i < 4; i++ )
	{
		s_anNormKeys    [ 37 + i ] = s_anNormArrows    [ nMode ][ i ];
		s_anShftKeys    [ 37 + i ] = s_anShftArrows    [ nMode ][ i ];
		s_anCtrlKeys    [ 37 + i ] = s_anCtrlArrows    [ nMode ][ i ];
		s_anShftCtrlKeys[ 37 + i ] = s_anShftCtrlArrows[ nMode ][ i ];
	}
} /* #OF# Input_SetArrowKeys */

/*========================================================
Function : ServeKBJoystickDown
=========================================================*/
/* #FN#
   Handles a joystick key down event */
static
BOOL
/* #AS#
   TRUE for ending the key down event handling, otherwise FALSE */
ServeKBJoystickDown(
	WPARAM wp,
	LPARAM lp
)
{
	BOOL bResult  = FALSE;
	int  nIDevice = 0;
	int  i, j;

	/* Translate extended codes */
	if( _IsFlagSet( lp, 1 << 24 ) )
	{
		wp = g_Input.Key.anExtKeys[ wp ];
	}
	else if( wp == VK_SHIFT && _IsFlagSet( lp, 1 << 20 ) )
		wp = VK_RSHIFT;

	if( 0 != wp )
	{
		for( i = 0; i < MAX_ATARI_JOYPORTS; i++ )
		{
			/* Check a keyboard joystick */
			if( g_Input.anDevSelected[ i ] < 0 )
			{
				nIDevice = g_Input.anDevSelected[ i ] + NUM_KBJOY_DEVICES;
				if( wp == g_Input.Joy.anKeysets[ nIDevice ][ KEYSET_FIRE ] )
				{
					s_anKeytrig[ nIDevice ] = 0;
					bResult = TRUE;
				}
				else
				{
					for( j = 0; j < NUM_KBJOY_KEYS - 1/* The last is KEYSET_FIRE */; j++ )
						if( wp == g_Input.Joy.anKeysets[ nIDevice ][ j ] )
						{
							if( s_anStickTable[ j ] != INPUT_STICK_CENTRE )
							{
								/* We have to clear some bits according to chosen direction */
								s_anKeystick[ nIDevice ] &= s_anStickTable[ j ];
							}
							else
								s_anKeystick[ nIDevice ] |= INPUT_STICK_CENTRE;

							bResult = TRUE;
							/* The key has been found */
							break;
						}
				}
				/* Turn off the pause if 'keystick' key is pressed */
				if( bResult )
				{
					if( _IsFlagSet( g_ulAtariState, ATARI_PAUSED ) && _IsFlagSet( g_Input.ulState, IS_JOY_EXIT_PAUSE ) )
					{
						Misc_TogglePause();
						UpdateStatus( FALSE, -1, ID_INDICATOR_RUN, FALSE );
					}
					/* The firstly assigned keyset has higher priority */
					break;
				}
			}
		}
	}
	return bResult;

} /* #OF# ServeKBJoystickDown */

/*========================================================
Function : ServeKBJoystickUp
=========================================================*/
/* #FN#
   Handles a joystick key up event */
static
BOOL
/* #AS#
   TRUE for ending the key up event handling, otherwise FALSE */
ServeKBJoystickUp(
	WPARAM wp,
	LPARAM lp
)
{
	int nIDevice;
	int i, j;

	/* Translate extended codes */
	if( _IsFlagSet( lp, 1 << 24 ) )
	{
		wp = g_Input.Key.anExtKeys[ wp ];
	}
	else if( wp == VK_SHIFT && _IsFlagSet( lp, 1 << 20 ) )
		wp = VK_RSHIFT;

	if( 0 != wp )
	{
		for( i = 0; i < MAX_ATARI_JOYPORTS; i++ )
		{
			/* Check a keyboard joystick */
			if( g_Input.anDevSelected[ i ] < 0 )
			{
				nIDevice = g_Input.anDevSelected[ i ] + NUM_KBJOY_DEVICES;
				if( wp == g_Input.Joy.anKeysets[ nIDevice ][ KEYSET_FIRE ] )
				{
					s_anKeytrig[ nIDevice ] = 1;
					return TRUE;
				}
				else
				{
					for( j = 0; j < NUM_KBJOY_KEYS - 1/* The last is KEYSET_FIRE */; j++ )
						if( wp == g_Input.Joy.anKeysets[ nIDevice ][ j ] )
						{
							if( _IsFlagSet( g_Input.ulState, IS_JOY_STICK_RELEASE ) )
							{
								/* Turn off bits for this key */
								s_anKeystick[ nIDevice ] |= (~s_anStickTable[ j ]) & 0x0f;
							}
							return TRUE;
						}
				}
			}
		}
	}
	return FALSE;

} /* #OF# ServeKBJoystickUp */

/*========================================================
Function : ToggleKeyDown
=========================================================*/
/* #FN#
   Handles a toggling key down event */
static
BOOL
/* #AS#
   TRUE to stop event handling, FALSE to continue */
ToggleKeyDown(
	WPARAM wp,
	LPARAM lp
)
{
	/* Skip process of the right ALT key event, that normally is handled
	   as a CTRL+ALT key for non-U.S. enhanced 102-key keyboards.
	*/
	if( VK_CONTROL == wp && _IsFlagSet( lp, 1 << 29 ) )
		return TRUE;

	if( g_Input.Joy.bKBJoystick && !GetKeyState( VK_SCROLL ) &&
		ServeKBJoystickDown( wp, lp ) )
	{
		/* End of the key down event handling? */
		if( !_IsFlagSet( g_Input.ulState, IS_JOY_DONT_EXCLUDE ) )
			return TRUE;
	}
	switch( wp )
	{
		case VK_SHIFT:
			INPUT_key_shift = g_Input.Key.nShift = 1;
			return TRUE;

		case VK_CONTROL:
			g_Input.Key.nControl = 1;
			return TRUE;

		/* Console keys */
		case VK_F4:
			Atari_Set_CONSOL( AKEY_OPTIONDWN );
			break;

		case VK_F3:
			Atari_Set_CONSOL( AKEY_SELECTDWN );
			break;

		case VK_F2:
			Atari_Set_CONSOL( AKEY_STARTDWN );
			break;
	}
	return FALSE;

} /* #OF# ToggleKeyDown */

/*========================================================
Function : ToggleKeyUp
=========================================================*/
/* #FN#
   Handles a toggling key up event */
static
BOOL
/* #AS#
   TRUE to stop event handling, FALSE to continue */
ToggleKeyUp(
	WPARAM wp,
	LPARAM lp
)
{
	if( g_Input.Joy.bKBJoystick && !GetKeyState( VK_SCROLL ) &&
		ServeKBJoystickUp( wp, lp ) )
	{
		/* End of the key down event handling? */
		if( !_IsFlagSet( g_Input.ulState, IS_JOY_DONT_EXCLUDE ) )
			return TRUE;
	}
	switch( wp )
	{
		case VK_SHIFT:
			INPUT_key_shift = g_Input.Key.nShift = 0;
			return TRUE;

		case VK_CONTROL:
			g_Input.Key.nControl = 0;
			return TRUE;

		/* Console keys */
		case VK_F4:
			Atari_Set_CONSOL( AKEY_OPTIONUP );
			break;

		case VK_F3:
			Atari_Set_CONSOL( AKEY_SELECTUP );
			break;

		case VK_F2:
			Atari_Set_CONSOL( AKEY_STARTUP );
			break;
	}
	return FALSE;

} /* #OF# ToggleKeyUp */

/*========================================================
Function : Input_KeyDown
=========================================================*/
/* #FN#
   Handles a key down event */
BOOL
/* #AS#
   TRUE to stop event handling, FALSE to continue */
Input_KeyDown(
	WPARAM wp,
	LPARAM lp
)
{
	if( !_IsFlagSet( g_Input.ulState, IS_KEY_TYPEMATIC_RATE ) )
	{
		if ( s_anNormKeys[ wp ] != AKEY_BREAK ) // for problems with BREAK key
			if( wp == (WPARAM)g_Input.Key.nCurrentVK )
				return TRUE;
	}
	if( g_Input.Joy.bKBJoystick || s_anToggleKeys[ wp ] )
	{
		if( g_Input.Joy.bKBJoystick && VK_SCROLL == wp )
			Input_ResetKeys();
		else
		if( ToggleKeyDown( wp, lp ) )
			return TRUE;
	}
	if( _IsFlagSet( g_Input.ulState, IS_KEY_USE_TEMPLATE ) && s_aucCoveredKeys[ wp ] )
	{
		g_Input.Key.nNewKey = s_anKBTable[ wp ];

		if( g_Input.Key.nNewKey != AKEY_NONE )
		{
			if( g_Input.Key.nShift )
				g_Input.Key.nNewKey |= AKEY_SHFT;
			if( g_Input.Key.nControl )
				g_Input.Key.nNewKey |= AKEY_CTRL;

			g_Input.Key.nNewVK = wp;
		}
		else
			g_Input.Key.nNewVK = 0;
		return TRUE;
	}
	if( g_Input.Key.nShift && g_Input.Key.nControl )
	{
		g_Input.Key.nNewKey = s_anShftCtrlKeys[ wp ];
		g_Input.Key.nNewVK  = (g_Input.Key.nNewKey != AKEY_NONE ? wp : 0);
		if ( g_Input.Key.nNewKey != AKEY_NONE )
			return TRUE;
		else
			return FALSE;
	}
	if( g_Input.Key.nControl )
	{
		g_Input.Key.nNewKey = s_anCtrlKeys[ wp ];
		g_Input.Key.nNewVK  = (g_Input.Key.nNewKey != AKEY_NONE ? wp : 0);
		if ( g_Input.Key.nNewKey != AKEY_NONE )
			return TRUE;
		else
			return FALSE;
	}
	if( g_Input.Key.nShift )
	{
		g_Input.Key.nNewKey = s_anShftKeys[ wp ];
		g_Input.Key.nNewVK  = (g_Input.Key.nNewKey != AKEY_NONE ? wp : 0);
		if ( g_Input.Key.nNewKey != AKEY_NONE )
			return TRUE;
		else
			return FALSE;
	}
	if( s_anNormKeys[ wp ] != AKEY_NONE )
	{
		g_Input.Key.nNewKey = s_anNormKeys[ wp ];
		g_Input.Key.nNewVK  = (g_Input.Key.nNewKey != AKEY_NONE ? wp : 0);
		/* The CapsLock indicator should be used */
//		if( g_Input.Key.nNewVK != VK_CAPITAL ) /* CapsLock */
		return TRUE;
	}
	return FALSE;

} /* #OF# Input_KeyDown */

/*========================================================
Function : Input_KeyUp
=========================================================*/
/* #FN#
   Handles a key up event */
BOOL
/* #AS#
   TRUE to stop event handling, FALSE to continue */
Input_KeyUp(
	WPARAM wp,
	LPARAM lp
)
{
	if( g_Input.Joy.bKBJoystick || s_anToggleKeys[ wp ] )
	{
		if( ToggleKeyUp( wp, lp ) )
			return TRUE;
	}
	if( wp == (WPARAM)g_Input.Key.nCurrentVK )
	{
		g_Input.Key.nCurrentKey = AKEY_NONE;
		g_Input.Key.nCurrentVK  = 0;
	}
	if( wp == (WPARAM)g_Input.Key.nNewVK )
	{
		if( g_Input.Key.nNewKey != AKEY_BREAK ) // for problems with BREAK key
		{
			g_Input.Key.nNewKey = AKEY_NONE;
			g_Input.Key.nNewVK  = 0;
		}
	}
	return FALSE;

} /* #OF# Input_KeyUp */

/*========================================================
Function : Input_SysKeyUp
=========================================================*/
/* #FN#
   Handles a syskey up event */
BOOL
/* #AS#
   TRUE for ending key up event handling, FALSE for continue */
Input_SysKeyUp(
	WPARAM wp,
	LPARAM lp
)
{
	/* Prevents keysticks from locking */
	if( VK_MENU == wp ||
		/* The context code is 1 if the ALT key is
		   down while another key is released */
		_IsFlagSet( lp, 1 << 29 ) )
	{
		int i;
		for( i = 0; i < NUM_KBJOY_DEVICES; i++ )
		{
			s_anKeystick[ i ] = INPUT_STICK_CENTRE;
			s_anKeytrig [ i ] = 1;
		}
	}
	if( ST_CTRLESC_CAPTURED ) /* Not really needed */
	{
		/* Special handling for Ctrl+Esc key sequence */
		if( VK_ESCAPE == wp )
		{
			if( g_Input.Key.nControl )
			{
				/* The Atari_Keyboard function will "release" this key automatically;
				   I must look around if there is a better way for doing that */
				g_Input.Key.nNewKey = s_anCtrlKeys[ wp ];
				g_Input.Key.nNewVK  = (g_Input.Key.nNewKey != AKEY_NONE ? wp : 0);
			}
			/* If the CTRL key is assigned to a keyboard joystick, the g_Input.Key.nControl
			   is not raised and the ESC goes to system, which checks the CTRL key state and
			   then selects Start menu. We have to avoid this using return TRUE */
			return TRUE;
		}
	}
	return FALSE;

} /* #OF# Input_SysKeyUp */

/*========================================================
Function : LowLevelKeyboardProc
=========================================================*/
/* #FN#
   The system calls this function every time a new keyboard input
   event is about to be posted into a thread input queue */
static
LRESULT CALLBACK
/* #AS#
   If nCode is less than zero, the hook procedure must return the
   value returned by CallNextHookEx */
LowLevelKeyboardProc(
	INT    nCode,
	WPARAM wParam,
	LPARAM lParam
)
{
	/* By returning a non-zero value from the hook procedure, the
	   message does not get passed to the target window */
	KBDLLHOOKSTRUCT *pkbhs = (KBDLLHOOKSTRUCT *)lParam;

	switch( nCode )
	{
		case HC_ACTION:
		{
			/* Check to see if the CTRL key is pressed */
			BOOL bControlKeyDown = _IsKeyPressed( VK_CONTROL );

			/* Disable CTRL+ESC */
			if( VK_ESCAPE == pkbhs->vkCode && bControlKeyDown )
			{
				if( _IsFlagSet( g_Input.ulState, IS_CAPTURE_CTRLESC ) )
				{
					if( g_hMainWnd && _IsFlagSet( pkbhs->flags, 0x80 ) )
					{
						/* Send information about releasing CTRL+ESC to main window */
						PostMessage( g_hMainWnd, /*_IsFlagSet( pkbhs->flags, 0x80 ) ?*/ WM_SYSKEYUP /*: WM_SYSKEYDOWN*/, VK_ESCAPE, 0L );
					}
					return 1;
				}
				if( _IsFlagSet( g_Screen.ulState, SM_MODE_FULL ) )
					return 1;
			}
			if( _IsFlagSet( g_Screen.ulState, SM_MODE_FULL ) /*&& !_IsFlagSet( g_Screen.ulState, SM_OPTN_SAFE_MODE )*/ )
			{
				/* Disable ALT+TAB */
				if( VK_TAB == pkbhs->vkCode && _IsFlagSet( pkbhs->flags, LLKHF_ALTDOWN ) )
					return 1;
				/* Disable ALT+ESC */
				if( VK_ESCAPE == pkbhs->vkCode && _IsFlagSet( pkbhs->flags, LLKHF_ALTDOWN ) )
					return 1;
			}
		}
	}
	return CallNextHookEx( s_hKeyboardHook, nCode, wParam, lParam );

} /* #OF# LowLevelKeyboardProc */

/*========================================================
Function : Input_EnableEscCapture
=========================================================*/
/* #FN#
   Enables or disables Ctrl+Esc key sequence */
void
/* #AS#
   Nothing */
Input_EnableEscCapture(
	BOOL bEnable /* #IN# Enable/disable capture flag */
)
{
	/*
	  Windows 95 and Windows 98: applications can enable and
	  disable ALT+TAB and CTRL+ESC, for example, by calling
	  SystemParametersInfo(SPI_SETSCREENSAVERRUNNING). To disable
	  ALT+TAB and CTRL+ESC, set the uiParam parameter to TRUE;
	  to enable the key combinations, set the parameter to FALSE: 

	  NOTE: Applications that use SystemParametersInfo
	  (SPI_SETSCREENSAVERRUNNING) to disable task switching must
	  enable task switching before exiting or task switching
	  remains disabled after the process terminates.

	  Windows NT 4.0 Service Pack 2 and Earlier, Windows NT 3.51
	  and Earlier Applications can disable CTRL+ESC system-wide
	  by replacing the Windows NT Task Manager, but this is not
	  recommended.

	  Windows NT 4.0 Service Pack 3 and Later and Windows 2000
	  Applications can disable ALT+TAB or CTRL+ESC by installing
	  a low-level keyboard hook. A low-level keyboard hook
	  (WH_KEYBOARD_LL) is installed by calling SetWindowsHookEx.

	  Because low-level keyboard hooks are a feature specific to
	  Windows NT 4.0 Service Pack 3 and later, define _WIN32_WINNT
	  >= 0x0400 prior to including winuser.h (or windows.h).
	*/
	_ASSERT(g_Misc.unSystemInfo);

	if( _IsFlagSet( g_Misc.unSystemInfo, SYS_WIN_9x ) )
	{
		if( bEnable )
		{
			/* Disables task switching */
			if( s_nSPIPrevState )
				Input_EnableEscCapture( FALSE );

			SystemParametersInfo( SPI_SETSCREENSAVERRUNNING, TRUE,
								  &s_nSPIPrevState, 0 );
		}
		else
		{
			/* Enables task switching */
			if( !s_nSPIPrevState )
				SystemParametersInfo( SPI_SETSCREENSAVERRUNNING, FALSE,
									  &s_nSPIPrevState, 0 );
		}
	}
} /* #OF# Input_EnableEscCapture */

/*========================================================
Function : Input_InstallKeyboardHook
=========================================================*/
/* #FN#
   Installs keyboard hook if Windows NT is used */
void
/* #AS#
   Nothing */
Input_InstallKeyboardHook(
	BOOL bEnable /* #IN# Enable/disable keyboard hook */
)
{
	if( _IsFlagSet( g_Misc.unSystemInfo, SYS_WIN_NT ) )
	{
		if( bEnable )
		{
			if( s_hKeyboardHook )
				Input_InstallKeyboardHook( FALSE );

			s_hKeyboardHook =
				SetWindowsHookEx( WH_KEYBOARD_LL, LowLevelKeyboardProc,
								  g_hInstance, 0/*GetCurrentThreadId()*/ );
		}
		else
		{
			/* Enables task switching */
			if( s_hKeyboardHook )
				UnhookWindowsHookEx( s_hKeyboardHook );

			s_hKeyboardHook = NULL;
		}
	}
} /* #OF# Input_InstallKeyboardHook */

/*========================================================
Function : Input_RefreshBounds
=========================================================*/
/* #FN#
   Cashes view area bondaries */
void
/* #AS#
   Nothing */
Input_RefreshBounds(
	HWND hViewWnd,
	BOOL bClipCursor
)
{
	if( NULL == hViewWnd )
		return;

	GetWindowRect( hViewWnd, &s_rcViewBounds );
	/* Deflate the rect boundaries */
	s_rcViewBounds.left += 2; s_rcViewBounds.right -= 2;
	s_rcViewBounds.top += 2; s_rcViewBounds.bottom -= 2;

	if( _IsFlagSet( g_Input.ulState, IS_CAPTURE_MOUSE ) && bClipCursor )
	{
		/* Re-confine the cursor to a rectangular area on the view */
		ClipCursor( NULL );
		ClipCursor( &s_rcViewBounds );
		/* Initialise the mouse emulation settings */
		Input_ResetMouse( TRUE );//INPUT_MOUSE_JOY == g_Input.nMouseMode && Atari800_MACHINE_5200 == Atari800_machine_type );
	}
} /* #OF# Input_RefreshBounds */

#ifdef WIN_NETWORK_GAMES

/*========================================================
Function : Input_GetStick
=========================================================*/
/* #FN#
   Gets the state of a stick */
int
Input_GetStick(
	int nPort
)
{
	_ASSERT(0 <= nPort && nPort < MAX_ATARI_JOYPORTS);
	return s_anStick[ nPort ] & 0x0f;

} /* #OF# Input_GetStick */

/*========================================================
Function : Input_SetStick
=========================================================*/
/* #FN#
   Sets the state of a stick */
void
/* #AS#
   Nothing */
Input_SetStick(
	int nPort,
	int nValue
)
{
	_ASSERT(0 <= nPort && nPort < MAX_ATARI_JOYPORTS);
	s_anStick[ nPort ] = nValue & 0x0f;

} /* #OF# Input_SetStick */

/*========================================================
Function : Input_GetTrig
=========================================================*/
/* #FN#
   Gets the state of a trigger */
int
Input_GetTrig(
	int nPort
)
{
	_ASSERT(0 <= nPort && nPort < MAX_ATARI_JOYPORTS);
	return s_anTrig[ nPort ];

} /* #OF# Input_GetTrig */

/*========================================================
Function : Input_SetTrig
=========================================================*/
/* #FN#
   Sets the state of a trigger */
void
/* #AS#
   Nothing */
Input_SetTrig(
	int nPort,
	int nValue
)
{
	_ASSERT(0 <= nPort && nPort < MAX_ATARI_JOYPORTS);
	s_anTrig[ nPort ] = nValue;

} /* #OF# Input_SetTrig */

#endif /*WIN_NETWORK_GAMES*/

/*========================================================
Function : DI_GetErrorString
=========================================================*/
/* #FN#
   Outputs a debug string to debugger */
static
BOOL
/* #AS#
   TRUE if succeeded, otherwise FALSE */
DI_GetErrorString(
	HRESULT hResult,
	LPSTR   pszErrorBuff,
	DWORD   dwError
)
{
    char  szMsg[ 256 ];
    LPSTR pszError;
    DWORD dwLen;

    /* Check parameters */
    if( !pszErrorBuff || !dwError )
    {
        /* Error, invalid parameters */
        return FALSE;
    }

    switch( hResult )
    {
		/* The operation completed successfully. This value is equal to the S_OK standard COM return value */
		case DI_OK:
			pszError = "DI_OK";
			break;

		/* The device buffer overflowed and some input was lost. This value is equal to the S_FALSE standard COM return value */
		case DI_BUFFEROVERFLOW:
			pszError = "DI_BUFFEROVERFLOW";
			break;

		/* The parameters of the effect were successfully updated, but the effect could not be downloaded because the associated
		   device was not acquired in exclusive mode */
		case DI_DOWNLOADSKIPPED:
			pszError = "DI_DOWNLOADSKIPPED";
			break;

		/* The effect was stopped, the parameters were updated, and the effect was restarted */
		case DI_EFFECTRESTARTED:
			pszError = "DI_EFFECTRESTARTED";
			break;

		/* The operation had no effect. This value is equal to the S_FALSE standard COM return value */
//		case DI_NOEFFECT:
//			pszError = "DI_NOEFFECT";
//			break;

		/* The device exists but is not currently attached. This value is equal to the S_FALSE standard COM return value */
//		case DI_NOTATTACHED:
//			pszError = "DI_NOTATTACHED";
//			break;

		/* The device is a polled device. As a result, device buffering will not collect any data and event notifications will not be signaled until the IDirectInputDevice2::Poll method is called */
		case DI_POLLEDDEVICE:
			pszError = "DI_POLLEDDEVICE";
			break;

		/* The change in device properties had no effect. This value is equal to the S_FALSE standard COM return value */
//		case DI_PROPNOEFFECT:
//			pszError = "DI_PROPNOEFFECT";
//			break;

		/* The parameters of the effect were successfully updated, but some of them were beyond the capabilities of the device and were truncated to the nearest supported value */
		case DI_TRUNCATED:
			pszError = "DI_TRUNCATED";
			break;

		/* Equal to DI_EFFECTRESTARTED | DI_TRUNCATED */
		case DI_TRUNCATEDANDRESTARTED:
			pszError = "DI_TRUNCATEDANDRESTARTED";
			break;

		/* The operation cannot be performed while the device is acquired */
		case DIERR_ACQUIRED:
			pszError = "DIERR_ACQUIRED";
			break;

		/* This object is already initialized */
		case DIERR_ALREADYINITIALIZED:
			pszError = "DIERR_ALREADYINITIALIZED";
			break;

		/* The object could not be created due to an incompatible driver version or mismatched or incomplete driver components */
		case DIERR_BADDRIVERVER:
			pszError = "DIERR_BADDRIVERVER";
			break;

		/* The application was written for an unsupported prerelease version of DirectInput */
		case DIERR_BETADIRECTINPUTVERSION:
			pszError = "DIERR_BETADIRECTINPUTVERSION";
			break;

		/* The device is full */
		case DIERR_DEVICEFULL:
			pszError = "DIERR_DEVICEFULL";
			break;

		/* The device or device instance is not registered with DirectInput. This value is equal to the REGDB_E_CLASSNOTREG standard COM return value */
		case DIERR_DEVICENOTREG:
			pszError = "DIERR_DEVICENOTREG";
			break;

		/* The parameters were updated in memory but were not downloaded to the device because the device does not support updating an effect while it is still playing */
		case DIERR_EFFECTPLAYING:
			pszError = "DIERR_EFFECTPLAYING";
			break;

		/* The device cannot be reinitialized because there are still effects attached to it */
		case DIERR_HASEFFECTS:
			pszError = "DIERR_HASEFFECTS";
			break;

		/* An undetermined error occurred inside the DirectInput subsystem. This value is equal to the E_FAIL standard COM return value */
		case DIERR_GENERIC:
			pszError = "DIERR_GENERIC";
			break;

		/* The device already has an event notification associated with it. This value is equal to the E_ACCESSDENIED standard COM return value */
		case DIERR_HANDLEEXISTS:
			pszError = "DIERR_HANDLEEXISTS";
			break;

		/* The effect could not be downloaded because essential information is missing. For example, no axes have been associated with the effect, or no type-specific information has been supplied */
		case DIERR_INCOMPLETEEFFECT:
			pszError = "DIERR_INCOMPLETEEFFECT";
			break;

		/* Access to the input device has been lost. It must be reacquired */
		case DIERR_INPUTLOST:
			pszError = "DIERR_INPUTLOST";
			break;

		/* An invalid parameter was passed to the returning function, or the object was not in a state that permitted the function to be called. This value is equal to the E_INVALIDARG standard COM return value */
		case DIERR_INVALIDPARAM:
			pszError = "DIERR_INVALIDPARAM";
			break;

		/* Not all the requested information fitted into the buffe */
		case DIERR_MOREDATA:
			pszError = "DIERR_MOREDATA";
			break;

		/* This object does not support aggregation */
		case DIERR_NOAGGREGATION:
			pszError = "DIERR_NOAGGREGATION";
			break;

		/* The specified interface is not supported by the object. This value is equal to the E_NOINTERFACE standard COM return value */
		case DIERR_NOINTERFACE:
			pszError = "DIERR_NOINTERFACE";
			break;

		/* The operation cannot be performed unless the device is acquired */
		case DIERR_NOTACQUIRED:
			pszError = "DIERR_NOTACQUIRED";
			break;

		/* The device is not buffered. Set the DIPROP_BUFFERSIZE property to enable bufferin */
		case DIERR_NOTBUFFERED:
			pszError = "DIERR_NOTBUFFERED";
			break;

		/* The effect is not downloade */
		case DIERR_NOTDOWNLOADED:
			pszError = "DIERR_NOTDOWNLOADED";
			break;

		/* The operation cannot be performed unless the device is acquired in DISCL_EXCLUSIVE mode */
		case DIERR_NOTEXCLUSIVEACQUIRED:
			pszError = "DIERR_NOTEXCLUSIVEACQUIRED";
			break;

		/* The requested object does not exist */
		case DIERR_NOTFOUND:
			pszError = "DIERR_NOTFOUND";
			break;

		/* This object has not been initialized */
		case DIERR_NOTINITIALIZED:
			pszError = "DIERR_NOTINITIALIZED";
			break;

		/* The requested object does not exist */
//		case DIERR_OBJECTNOTFOUND:
//			pszError = "DIERR_OBJECTNOTFOUND";
//			break;

		/* The application requires a newer version of DirectInput */
		case DIERR_OLDDIRECTINPUTVERSION:
			pszError = "DIERR_OLDDIRECTINPUTVERSION";
			break;

		/* Another application has a higher priority level, preventing this call from succeeding. This value is equal to the E_ACCESSDENIED standard COM return value.
		   This error can be returned when an application has only foreground access to a device but is attempting to acquire the device while in the background */
//		case DIERR_OTHERAPPHASPRIO:
//			pszError = "DIERR_OTHERAPPHASPRIO";
//			break;

		/* The DirectInput subsystem couldn't allocate sufficient memory to complete the call. This value is equal to the E_OUTOFMEMORY standard COM return value */
		case DIERR_OUTOFMEMORY:
			pszError = "DIERR_OUTOFMEMORY";
			break;

		/* The specified property cannot be changed. This value is equal to the E_ACCESSDENIED standard COM return value */
//		case DIERR_READONLY:
//			pszError = "DIERR_READONLY";
//			break;

		/* The function called is not supported at this time. This value is equal to the E_NOTIMPL standard COM return value */
		case DIERR_UNSUPPORTED:
			pszError = "DIERR_UNSUPPORTED";
			break;

		/* Data is not yet availabl */
		case E_PENDING:
			pszError = "E_PENDING";
			break;

		/* Unknown DI Error */
		default:
			sprintf( szMsg, "Error #%ld", (DWORD)(hResult & 0x0000FFFFL) );
			pszError = szMsg;
			break;
	}
    /* Copy DI Error string to buff */
    dwLen = strlen( pszError );
    if( dwLen >= dwError )
    {
        dwLen = dwError - 1;
    }
	if( dwLen )
    {
        _strncpy( pszErrorBuff, pszError, dwLen );
        pszErrorBuff[ dwLen ] = '\0';
    }
    return TRUE;

} /* #OF# DI_GetErrorString */
