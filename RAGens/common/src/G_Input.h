#ifndef G_INPUT_H
#define G_INPUT_H

#define DIRECTINPUT_VERSION 0x0800  // for joystick support

#include <dinput.h>
//#include <mmsystem.h>

struct K_Def {
	unsigned int Start, Mode;
	unsigned int A, B, C;
	unsigned int X, Y, Z;
	unsigned int Up, Down, Left, Right;
	unsigned int RA_Home;
	unsigned int Rewind;	// ##RW: Rewind key
	};

extern struct K_Def Keys_Def[8];
extern unsigned char Kaillera_Keys[16];

int RewindRequested(void);

int String_Size(char *Chaine);
unsigned int Get_Key(void);
int Check_Key_Pressed(int key);
int Setting_Keys(HWND hset, int Player, int Type);
int Init_Input(HINSTANCE hInst, HWND hWnd);
void End_Input(void);
void Update_Input(void);
void Update_Controllers(void);
void Scan_Player_Net(int Player);
void Update_Controllers_Net(int num_player);


#endif