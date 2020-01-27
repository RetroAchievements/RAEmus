/****************************************************************************
File    : input_win.h
/*
@(#) #SY# Atari800Win PLus
@(#) #IS# InputWin public methods and objects prototypes
@(#) #BY# Tomasz Szymankowski
@(#) #LM# 19.10.2003
*/

#ifndef __INPUT_WIN_H__
#define __INPUT_WIN_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Constants declarations */

#define MAX_INPUT_DEVICES			8
#define MAX_ATARI_JOYPORTS			4
#define NUM_KBJOY_DEVICES			4
#define NUM_KBJOY_KEYS				10
#define INPUT_DEV_NAMELEN			64

#define NUMPAD_JOYSTICK				-4
#define CURSOR_JOYSTICK				-3
#define KEYS_A_JOYSTICK				-2
#define KEYS_B_JOYSTICK				-1
/*
Found input devs here				(0, ..., MAX_INPUT_DEVICES - 1)
*/
#define NO_JOYSTICK					127

#define TIMER_READ_JOYSTICK			6969
										/* Warning: See special keys */
#define SPECIAL_HANDLER_MASK		0xf000	/* codes in atari.h file */
#define AKEY_OPTIONDWN				0xffe1
#define AKEY_SELECTDWN				0xffe2
#define AKEY_STARTDWN				0xffe3
#define AKEY_OPTIONUP				0xffe7
#define AKEY_SELECTUP				0xffe8
#define AKEY_STARTUP				0xffe9
#define AKEY_MONITOR				0xffea

#define KEYBOARD_TABLE_SIZE			256 /* Number of virtual key codes */

#define KEYSET_NW					0
#define KEYSET_NORTH				1
#define KEYSET_NE					2
#define KEYSET_EAST					3
#define KEYSET_SE					4
#define KEYSET_SOUTH				5
#define KEYSET_SW					6
#define KEYSET_WEST					7
#define KEYSET_CENTRE				8
#define KEYSET_FIRE					9
									/* Arrow keys modes     */
#define ARROWKEYS_CTRLARROWS		0	/* Control + arrows */
#define ARROWKEYS_ARROWS			1	/* Arrows only      */
#define ARROWKEYS_F1F4				2	/* Function keys    */

#define IS_JOY_FIRE_ONLY			0x00000001
#define IS_JOY_STICK_RELEASE		0x00000002
#define IS_JOY_EXIT_PAUSE			0x00000004
#define IS_JOY_DONT_EXCLUDE			0x00000008
#define IS_KEY_USE_TEMPLATE			0x00000010
#define IS_KEY_TYPEMATIC_RATE		0x00000020
#define IS_CAPTURE_MOUSE			0x00000040
#define IS_CAPTURE_CTRLESC			0x00000080

#define DEF_INPUT_STATE				IS_JOY_STICK_RELEASE
#define DEF_AUTOFIRE_MODE			INPUT_AUTOFIRE_OFF
#define DEF_MOUSE_MODE				INPUT_MOUSE_OFF
#define DEF_PEN_OFFSET_X			0
#define DEF_PEN_OFFSET_Y			0
#define DEF_MOUSE_PORT				0
#define DEF_MOUSE_SPEED				3
#define DEF_POT_MIN					0
#define DEF_POT_MAX					228
#define DEF_JOY_INERTIA				10
#define DEF_ARROWS_MODE				ARROWKEYS_ARROWS
#define DEF_JOY_SELECTS				0x7f7f7ffc
#define DEF_AUTOFIRE_STICKS			0x0f

/* Exported methods */

BOOL Input_Initialise         ( BOOL bInitDInput );
void Input_Reset              ( void );
void Input_Clear              ( void );
BOOL Input_ReadJoystick       ( int nStickNum, BOOL bButton );
void Input_UpdateJoystick     ( void );
void Input_ToggleMouseCapture ( void );
void Input_UpdateMouse        ( void );
void Input_ResetMouse         ( BOOL bCenter );
void Input_SetArrowKeys       ( int nMode );
BOOL Input_KeyDown            ( WPARAM wp, LPARAM lp );
BOOL Input_KeyUp              ( WPARAM wp, LPARAM lp );
BOOL Input_SysKeyUp           ( WPARAM wp, LPARAM lp );
void Input_ResetKeys          ( void );
void Input_EnableEscCapture   ( BOOL bEnable );
void Input_InstallKeyboardHook( BOOL bEnable );
void Input_RefreshBounds      ( HWND hViewWnd, BOOL bClipCursor );
/* This one is invoked directly by kernel */
int  Atari_Keyboard           ( void );
int  Atari_PORT				  ( int );
int  Atari_TRIG				  ( int );

#ifdef WIN_NETWORK_GAMES

int  Input_GetStick           ( int nPort );
int  Input_GetTrig            ( int nPort );
void Input_SetStick           ( int nPort, int nValue );
void Input_SetTrig            ( int nPort, int nValue );

#endif /*WIN_NETWORK_GAMES*/

/* Exported globals */

struct InputCtrl_t
{
	ULONG ulState;	/* Input state flags */
	int   anDevSelected[ MAX_ATARI_JOYPORTS ];
	char  acDevNames   [ MAX_INPUT_DEVICES ][ INPUT_DEV_NAMELEN + 1 ];
	int   nDevFoundNum;
	/* Joystick */
	struct JoyCtrl_t
	{
		ULONG ulSelected;
		ULONG ulAutoSticks;
		int   nAutoMode;
		BOOL  bKBJoystick;
		WORD (*anKeysets)[ NUM_KBJOY_KEYS ];
	} Joy;
	/* Keyboard */
	struct KeyCtrl_t
	{
		int   nArrowsMode;
		int   nCurrentKey;
		int   nCurrentVK;
		int   nNewKey;
		int   nNewVK;
		int   nControl;
		int   nConsol;
		int   nShift;
		BYTE *anExtKeys;
		int  *anKBTable;
	} Key;
};
extern struct InputCtrl_t g_Input;

#ifdef __cplusplus
}
#endif

#endif /*__INPUT_WIN_H__*/
