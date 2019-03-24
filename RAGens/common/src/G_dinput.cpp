#include <stdio.h>
#include "resource.h"
#include "G_Input.h"
#include "io.h"
#include "G_main.h"

// ##RA
#include "RA_Interface.h"

#define KEYDOWN(key) (Keys[key] & 0x80) 
#define MAX_JOYS 8

LPDIRECTINPUT lpDI;
LPDIRECTINPUTDEVICE lpDIDKeyboard;
LPDIRECTINPUTDEVICE lpDIDMouse;
char Phrase[1024];
int Nb_Joys = 0;
static IDirectInputDevice8 *Joy_ID[MAX_JOYS] = {NULL};
static DIJOYSTATE Joy_State[MAX_JOYS] = {{0}};
long MouseX, MouseY;
unsigned char Keys[256];
unsigned char Kaillera_Keys[16];


struct K_Def Keys_Def[8] = {
	{DIK_RETURN, DIK_RSHIFT,
	DIK_A, DIK_S, DIK_D,
	DIK_Z, DIK_X, DIK_C,
	DIK_UP, DIK_DOWN, DIK_LEFT, DIK_RIGHT, DIK_ESCAPE, DIK_V /* ##RW: standard rewind key */},
	{DIK_U, DIK_T,
	DIK_K, DIK_L, DIK_M,
	DIK_I, DIK_O, DIK_P,
	DIK_Y, DIK_H, DIK_G, DIK_J, DIK_ESCAPE, DIK_V /* ##RW: standard rewind key */},
	//	360 helper defaults:
	{4119, 4120,
	4114, 4115, 4113,
	4116, 4117, 4112,
	4225, 4227, 4228, 4226, 4118, 0000 /* ##RW: TODO: rewind key for 360? */},
};


int String_Size(char *Chaine)
{
	int i = 0;

	while (*(Chaine + i++));

	return(i - 1);	
}


void End_Input()
{
	int i;
	
	if (lpDI)
	{
		if(lpDIDMouse)
		{
			lpDIDMouse->Release();
			lpDIDMouse = NULL;
		}

		if(lpDIDKeyboard)
		{
			lpDIDKeyboard->Release();
			lpDIDKeyboard = NULL;
		}

		for(i = 0; i < MAX_JOYS; i++)
		{
			if (Joy_ID[i])
			{
				Joy_ID[i]->Unacquire();
				Joy_ID[i]->Release();
			}
		}

		Nb_Joys = 0;
		lpDI->Release();
		lpDI = NULL;
	}
}


BOOL CALLBACK InitJoystick(LPCDIDEVICEINSTANCE lpDIIJoy, LPVOID pvRef)
{
	HRESULT rval;
	LPDIRECTINPUTDEVICE	lpDIJoy;
	DIPROPRANGE diprg;
	int i;
 
	if (Nb_Joys >= MAX_JOYS) return(DIENUM_STOP);
		
	Joy_ID[Nb_Joys] = NULL;

	rval = lpDI->CreateDevice(lpDIIJoy->guidInstance, &lpDIJoy, NULL);
	if (rval != DI_OK)
	{
		MessageBox(HWnd, "IDirectInput::CreateDevice FAILED", "erreur joystick", MB_OK);
		return(DIENUM_CONTINUE);
	}

	rval = lpDIJoy->QueryInterface(IID_IDirectInputDevice8, (void **)&Joy_ID[Nb_Joys]);
	lpDIJoy->Release();
	if (rval != DI_OK)
	{
		MessageBox(HWnd, "IDirectInputDevice2::QueryInterface FAILED", "erreur joystick", MB_OK);
	    Joy_ID[Nb_Joys] = NULL;
	    return(DIENUM_CONTINUE);
	}

	rval = Joy_ID[Nb_Joys]->SetDataFormat(&c_dfDIJoystick);
	if (rval != DI_OK)
	{
		MessageBox(HWnd, "IDirectInputDevice::SetDataFormat FAILED", "erreur joystick", MB_OK);
		Joy_ID[Nb_Joys]->Release();
		Joy_ID[Nb_Joys] = NULL;
		return(DIENUM_CONTINUE);
	}

	rval = Joy_ID[Nb_Joys]->SetCooperativeLevel((HWND)pvRef, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);

	if (rval != DI_OK)
	{ 
		MessageBox(HWnd, "IDirectInputDevice::SetCooperativeLevel FAILED", "erreur joystick", MB_OK);
		Joy_ID[Nb_Joys]->Release();
		Joy_ID[Nb_Joys] = NULL;
		return(DIENUM_CONTINUE);
	}
 
	diprg.diph.dwSize = sizeof(diprg); 
	diprg.diph.dwHeaderSize = sizeof(diprg.diph); 
	diprg.diph.dwObj = DIJOFS_X;
	diprg.diph.dwHow = DIPH_BYOFFSET;
	diprg.lMin = -1000; 
	diprg.lMax = +1000;
 
	rval = Joy_ID[Nb_Joys]->SetProperty(DIPROP_RANGE, &diprg.diph);
	if ((rval != DI_OK) && (rval != DI_PROPNOEFFECT)) 
		MessageBox(HWnd, "IDirectInputDevice::SetProperty() (X-Axis) FAILED", "erreur joystick", MB_OK);

	diprg.diph.dwSize = sizeof(diprg); 
	diprg.diph.dwHeaderSize = sizeof(diprg.diph); 
	diprg.diph.dwObj = DIJOFS_Y;
	diprg.diph.dwHow = DIPH_BYOFFSET;
	diprg.lMin = -1000; 
	diprg.lMax = +1000;
 
	rval = Joy_ID[Nb_Joys]->SetProperty(DIPROP_RANGE, &diprg.diph);
	if ((rval != DI_OK) && (rval != DI_PROPNOEFFECT)) 
		MessageBox(HWnd, "IDirectInputDevice::SetProperty() (Y-Axis) FAILED", "erreur joystick", MB_OK);

	for(i = 0; i < 10; i++)
	{
		rval = Joy_ID[Nb_Joys]->Acquire();
		if (rval == DI_OK) break;
		Sleep(10);
	}

	Nb_Joys++;

	return(DIENUM_CONTINUE);
}


int Init_Input(HINSTANCE hInst, HWND hWnd)
{
	int i;
	HRESULT rval;

	End_Input();
	
	rval = DirectInput8Create(
		hInst, 
		DIRECTINPUT_VERSION,
		IID_IDirectInput8A, 
		(void**)&lpDI,
		NULL);

	if (rval != DI_OK)
	{
		MessageBox(hWnd, "DirectInput failed ...You must have DirectX 5", "Error", MB_OK);
		return 0;
	}
	
	Nb_Joys = 0;

	for(i = 0; i < MAX_JOYS; i++) Joy_ID[i] = NULL;

	rval = lpDI->EnumDevices(DI8DEVCLASS_GAMECTRL, &InitJoystick, hWnd, DIEDFL_ATTACHEDONLY);
	if (rval != DI_OK) return 0;

//	rval = lpDI->CreateDevice(GUID_SysMouse, &lpDIDMouse, NULL);
	rval = lpDI->CreateDevice(GUID_SysKeyboard, &lpDIDKeyboard, NULL);
	if (rval != DI_OK) return 0;

//	rval = lpDIDMouse->SetCooperativeLevel(hWnd, DISCL_EXCLUSIVE | DISCL_FOREGROUND);
	rval = lpDIDKeyboard->SetCooperativeLevel(hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
	if (rval != DI_OK) return 0;

//	rval = lpDIDMouse->SetDataFormat(&c_dfDIMouse);
	rval = lpDIDKeyboard->SetDataFormat(&c_dfDIKeyboard);
	if (rval != DI_OK) return 0;

//	rval = lpDIDMouse->Acquire();
	for(i = 0; i < 10; i++)
	{
		rval = lpDIDKeyboard->Acquire();
		if (rval == DI_OK) break;
		Sleep(10);
	}

	return 1;
}


void Restore_Input()
{
//	lpDIDMouse->Acquire();
	lpDIDKeyboard->Acquire();
}


void Update_Input()
{
	if( !lpDIDKeyboard )
		return;

//	DIMOUSESTATE MouseState;
	HRESULT rval;
	int i;

	rval = lpDIDKeyboard->GetDeviceState(256, &Keys);

	if ((rval == DIERR_INPUTLOST) | (rval == DIERR_NOTACQUIRED))
		Restore_Input();

	for (i = 0; i < Nb_Joys; i++)
	{
		if (Joy_ID[i])
		{
			Joy_ID[i]->Poll();
			rval = Joy_ID[i]->GetDeviceState(sizeof(Joy_State[i]), &Joy_State[i]);
			if (rval != DI_OK) Joy_ID[i]->Acquire();
		}
	}

//	rval = lpDIDMouse->GetDeviceState(sizeof(MouseState), &MouseState);
	
//	if ((rval == DIERR_INPUTLOST) | (rval == DIERR_NOTACQUIRED))
//		Restore_Input();

//  MouseX = MouseState.lX;
//  MouseY = MouseState.lY;
}


int Check_Key_Pressed(unsigned int key)
{
	int Num_Joy;

	if (key < 0x100)
	{
		if KEYDOWN(key) return(1);
	}
	else
	{
		Num_Joy = ((key >> 8) & 0xF);

		if (Joy_ID[Num_Joy])
		{
			if (key & 0x80)			// Test POV Joys
			{
				if( Joy_State[Num_Joy].rgdwPOV[(key >> 4) & 3] == (unsigned int)-1 )
				{
					//	early out: this value is special: nothing pressed
					return 0;
				}

				switch(key & 0xF)
				{
					case 1:	//	up
						if( (Joy_State[Num_Joy].rgdwPOV[(key >> 4) & 3] <= 4500) ||		//up+right
							(Joy_State[Num_Joy].rgdwPOV[(key >> 4) & 3] >= 31500) )		//up+left
						{
							return(1);
						}
						break;

					case 2:	//	right
						if( (Joy_State[Num_Joy].rgdwPOV[(key >> 4) & 3] >= 4500) &&		//up+right
							(Joy_State[Num_Joy].rgdwPOV[(key >> 4) & 3] <= 13500) )		//down+right
						{
							return(1);
						}
						break;

					case 3:	//	down
						if( (Joy_State[Num_Joy].rgdwPOV[(key >> 4) & 3] >= 13500) &&	//down+right
							(Joy_State[Num_Joy].rgdwPOV[(key >> 4) & 3] <= 22500) )		//down+left
						{
							return(1);
						}
						break;

					case 4:	//	left
						if( (Joy_State[Num_Joy].rgdwPOV[(key >> 4) & 3] >= 22500) &&	//down+left
							(Joy_State[Num_Joy].rgdwPOV[(key >> 4) & 3] <= 31500) )		//up+left
						{
							return(1);
						}
						break;
// 
// 					case 2:
// 						if (Joy_State[Num_Joy].rgdwPOV[(key >> 4) & 3] == 9000) return(1); break;
// 
// 					case 3:
// 						if (Joy_State[Num_Joy].rgdwPOV[(key >> 4) & 3] == 18000) return(1); break;
// 
// 					case 4:
// 						if (Joy_State[Num_Joy].rgdwPOV[(key >> 4) & 3] == 27000) return(1); break;
				}

			}
			else if (key & 0x70)		// Test Button Joys
			{
				if (Joy_State[Num_Joy].rgbButtons[(key & 0xFF) - 0x10]) return(1);
			}
			else
			{
				switch(key & 0xF)
				{
					case 1:
						if (Joy_State[Num_Joy].lY < -500) return(1); break;

					case 2:
						if (Joy_State[Num_Joy].lY > +500) return(1); break;

					case 3:
						if (Joy_State[Num_Joy].lX < -500) return(1); break;

					case 4:
						if (Joy_State[Num_Joy].lX > +500) return(1); break;
				}
			}
		}
	}

	return 0;
}


unsigned int Get_Key(void)
{
	int i, j;

	while(1)
	{
		Update_Input();

		for(i = 1; i < 256; i++)
			if KEYDOWN(i) return i;

		for(i = 0; i < Nb_Joys; i++)
		{
			if (Joy_ID[i])
			{
				if (Joy_State[i].lY < -500)
					return(0x1000 + (0x100 * i) + 0x1);

				if (Joy_State[i].lY > +500)
					return(0x1000 + (0x100 * i) + 0x2);

				if (Joy_State[i].lX < -500)
					return(0x1000 + (0x100 * i) + 0x3);

				if (Joy_State[i].lX > +500)
					return(0x1000 + (0x100 * i) + 0x4);

				for (j = 0; j < 4; j++)
					if (Joy_State[i].rgdwPOV[j] == 0)
						return(0x1080 + (0x100 * i) + (0x10 * j) + 0x1);

				for (j = 0; j < 4; j++)
					if (Joy_State[i].rgdwPOV[j] == 9000)
						return(0x1080 + (0x100 * i) + (0x10 * j) + 0x2);

				for (j = 0; j < 4; j++)
					if (Joy_State[i].rgdwPOV[j] == 18000)
						return(0x1080 + (0x100 * i) + (0x10 * j) + 0x3);

				for (j = 0; j < 4; j++)
					if (Joy_State[i].rgdwPOV[j] == 27000)
						return(0x1080 + (0x100 * i) + (0x10 * j) + 0x4);

				for (j = 0; j < 32; j++)
					if (Joy_State[i].rgbButtons[j])
						return(0x1010 + (0x100 * i) + j);
			}
		}

		Sleep(10);
	}
}

// ##RW
int RewindRequested()
{
	if(Check_Key_Pressed(Keys_Def[0].Rewind) || Check_Key_Pressed(Keys_Def[1].Rewind))
	{
        if (RA_HardcoreModeIsActive())
            return 0;

		return 1;
	}
	else
	{
		return 0;
	}
}

void Update_Controllers()
{
	Update_Input();

	static bool bMergeAllControllers = true;
	if( bMergeAllControllers )
	{
		if (Check_Key_Pressed(Keys_Def[0].Up) || Check_Key_Pressed(Keys_Def[2].Up) || Check_Key_Pressed(Keys_Def[3].Up) || Check_Key_Pressed(Keys_Def[4].Up))
		{
			Controller_1_Up = 0;
			Controller_1_Down = 1;
		}
		else
		{
			Controller_1_Up = 1;
			if (Check_Key_Pressed(Keys_Def[0].Down) || Check_Key_Pressed(Keys_Def[2].Down) || Check_Key_Pressed(Keys_Def[3].Down) || Check_Key_Pressed(Keys_Def[4].Down))
				Controller_1_Down = 0;
			else 
				Controller_1_Down = 1;
		}

		if (Check_Key_Pressed(Keys_Def[0].Left) || Check_Key_Pressed(Keys_Def[2].Left) || Check_Key_Pressed(Keys_Def[3].Left) || Check_Key_Pressed(Keys_Def[4].Left))
		{
			Controller_1_Left = 0;
			Controller_1_Right = 1;
		}
		else
		{
			Controller_1_Left = 1;
			if (Check_Key_Pressed(Keys_Def[0].Right) || Check_Key_Pressed(Keys_Def[2].Right) || Check_Key_Pressed(Keys_Def[3].Right) || Check_Key_Pressed(Keys_Def[4].Right)) 
				Controller_1_Right = 0;
			else 
				Controller_1_Right = 1;
		}

		if (Check_Key_Pressed(Keys_Def[0].Start) || Check_Key_Pressed(Keys_Def[2].Start) || Check_Key_Pressed(Keys_Def[3].Start) || Check_Key_Pressed(Keys_Def[4].Start))
			Controller_1_Start = 0;
		else 
			Controller_1_Start = 1;

		if (Check_Key_Pressed(Keys_Def[0].A) || Check_Key_Pressed(Keys_Def[2].A) || Check_Key_Pressed(Keys_Def[3].A) || Check_Key_Pressed(Keys_Def[4].A)) 
			Controller_1_A = 0;
		else 
			Controller_1_A = 1;

		if (Check_Key_Pressed(Keys_Def[0].B) || Check_Key_Pressed(Keys_Def[2].B) || Check_Key_Pressed(Keys_Def[3].B) || Check_Key_Pressed(Keys_Def[4].B))
			Controller_1_B = 0;
		else 
			Controller_1_B = 1;

		if (Check_Key_Pressed(Keys_Def[0].C) || Check_Key_Pressed(Keys_Def[2].C) || Check_Key_Pressed(Keys_Def[3].C) || Check_Key_Pressed(Keys_Def[4].C))
			Controller_1_C = 0;
		else 
			Controller_1_C = 1;

		if (Controller_1_Type & 1)
		{
			if (Check_Key_Pressed(Keys_Def[0].Mode) || Check_Key_Pressed(Keys_Def[2].Mode) || Check_Key_Pressed(Keys_Def[3].Mode) || Check_Key_Pressed(Keys_Def[4].Mode))
				Controller_1_Mode = 0;
			else 
				Controller_1_Mode = 1;

			if (Check_Key_Pressed(Keys_Def[0].X) || Check_Key_Pressed(Keys_Def[2].X) || Check_Key_Pressed(Keys_Def[3].X) || Check_Key_Pressed(Keys_Def[4].X))
				Controller_1_X = 0;
			else 
				Controller_1_X = 1;

			if (Check_Key_Pressed(Keys_Def[0].Y) || Check_Key_Pressed(Keys_Def[2].Y) || Check_Key_Pressed(Keys_Def[3].Y) || Check_Key_Pressed(Keys_Def[4].Y))
				Controller_1_Y = 0;
			else 
				Controller_1_Y = 1;

			if (Check_Key_Pressed(Keys_Def[0].Z) || Check_Key_Pressed(Keys_Def[2].Z) || Check_Key_Pressed(Keys_Def[3].Z) || Check_Key_Pressed(Keys_Def[4].Z))
				Controller_1_Z = 0;
			else 
				Controller_1_Z = 1;
		}
	}
	else
	{
		if (Check_Key_Pressed(Keys_Def[0].Up))
		{
			Controller_1_Up = 0;
			Controller_1_Down = 1;
		}
		else
		{
			Controller_1_Up = 1;
			if (Check_Key_Pressed(Keys_Def[0].Down)) Controller_1_Down = 0;
			else Controller_1_Down = 1;
		}

		if (Check_Key_Pressed(Keys_Def[0].Left))
		{
			Controller_1_Left = 0;
			Controller_1_Right = 1;
		}
		else
		{
			Controller_1_Left = 1;
			if (Check_Key_Pressed(Keys_Def[0].Right)) Controller_1_Right = 0;
			else Controller_1_Right = 1;
		}

		if (Check_Key_Pressed(Keys_Def[0].Start)) Controller_1_Start = 0;
		else Controller_1_Start = 1;

		if (Check_Key_Pressed(Keys_Def[0].A)) Controller_1_A = 0;
		else Controller_1_A = 1;

		if (Check_Key_Pressed(Keys_Def[0].B)) Controller_1_B = 0;
		else Controller_1_B = 1;

		if (Check_Key_Pressed(Keys_Def[0].C)) Controller_1_C = 0;
		else Controller_1_C = 1;

		if (Controller_1_Type & 1)
		{
			if (Check_Key_Pressed(Keys_Def[0].Mode)) Controller_1_Mode = 0;
			else Controller_1_Mode = 1;

			if (Check_Key_Pressed(Keys_Def[0].X)) Controller_1_X = 0;
			else Controller_1_X = 1;

			if (Check_Key_Pressed(Keys_Def[0].Y)) Controller_1_Y = 0;
			else Controller_1_Y = 1;

			if (Check_Key_Pressed(Keys_Def[0].Z)) Controller_1_Z = 0;
			else Controller_1_Z = 1;
		}

	}
	
	if (Check_Key_Pressed(Keys_Def[1].Up))
	{
		Controller_2_Up = 0;
		Controller_2_Down = 1;
	}
	else
	{
		Controller_2_Up = 1;
		if (Check_Key_Pressed(Keys_Def[1].Down)) Controller_2_Down = 0;
		else Controller_2_Down = 1;
	}

	
	if (Check_Key_Pressed(Keys_Def[1].Left))
	{
		Controller_2_Left = 0;
		Controller_2_Right = 1;
	}
	else
	{
		Controller_2_Left = 1;
		if (Check_Key_Pressed(Keys_Def[1].Right)) Controller_2_Right = 0;
		else Controller_2_Right = 1;
	}

	if (Check_Key_Pressed(Keys_Def[1].Start)) Controller_2_Start = 0;
	else Controller_2_Start = 1;

	if (Check_Key_Pressed(Keys_Def[1].A)) Controller_2_A = 0;
	else Controller_2_A = 1;

	if (Check_Key_Pressed(Keys_Def[1].B)) Controller_2_B = 0;
	else Controller_2_B = 1;

	if (Check_Key_Pressed(Keys_Def[1].C)) Controller_2_C = 0;
	else Controller_2_C = 1;

	if (Controller_2_Type & 1)
	{
		if (Check_Key_Pressed(Keys_Def[1].Mode)) Controller_2_Mode = 0;
		else Controller_2_Mode = 1;

		if (Check_Key_Pressed(Keys_Def[1].X)) Controller_2_X = 0;
		else Controller_2_X = 1;

		if (Check_Key_Pressed(Keys_Def[1].Y)) Controller_2_Y = 0;
		else Controller_2_Y = 1;

		if (Check_Key_Pressed(Keys_Def[1].Z)) Controller_2_Z = 0;
		else Controller_2_Z = 1;
	}

	if (Controller_1_Type & 0x10)			// TEAMPLAYER PORT 1
	{
		if (Check_Key_Pressed(Keys_Def[2].Up))
		{
			Controller_1B_Up = 0;
			Controller_1B_Down = 1;
		}
		else
		{
			Controller_1B_Up = 1;
			if (Check_Key_Pressed(Keys_Def[2].Down)) Controller_1B_Down = 0;
			else Controller_1B_Down = 1;
		}
	
		if (Check_Key_Pressed(Keys_Def[2].Left))
		{
			Controller_1B_Left = 0;
			Controller_1B_Right = 1;
		}
		else
		{
			Controller_1B_Left = 1;
			if (Check_Key_Pressed(Keys_Def[2].Right)) Controller_1B_Right = 0;
			else Controller_1B_Right = 1;
		}

		if (Check_Key_Pressed(Keys_Def[2].Start)) Controller_1B_Start = 0;
		else Controller_1B_Start = 1;

		if (Check_Key_Pressed(Keys_Def[2].A)) Controller_1B_A = 0;
		else Controller_1B_A = 1;

		if (Check_Key_Pressed(Keys_Def[2].B)) Controller_1B_B = 0;
		else Controller_1B_B = 1;

		if (Check_Key_Pressed(Keys_Def[2].C)) Controller_1B_C = 0;
		else Controller_1B_C = 1;

		if (Controller_1B_Type & 1)
		{
			if (Check_Key_Pressed(Keys_Def[2].Mode)) Controller_1B_Mode = 0;
			else Controller_1B_Mode = 1;

			if (Check_Key_Pressed(Keys_Def[2].X)) Controller_1B_X = 0;
			else Controller_1B_X = 1;

			if (Check_Key_Pressed(Keys_Def[2].Y)) Controller_1B_Y = 0;
			else Controller_1B_Y = 1;

			if (Check_Key_Pressed(Keys_Def[2].Z)) Controller_1B_Z = 0;
			else Controller_1B_Z = 1;
		}

		if (Check_Key_Pressed(Keys_Def[3].Up))
		{
			Controller_1C_Up = 0;
			Controller_1C_Down = 1;
		}
		else
		{
			Controller_1C_Up = 1;
			if (Check_Key_Pressed(Keys_Def[3].Down)) Controller_1C_Down = 0;
			else Controller_1C_Down = 1;
		}
	
		if (Check_Key_Pressed(Keys_Def[3].Left))
		{
			Controller_1C_Left = 0;
			Controller_1C_Right = 1;
		}
		else
		{
			Controller_1C_Left = 1;
			if (Check_Key_Pressed(Keys_Def[3].Right)) Controller_1C_Right = 0;
			else Controller_1C_Right = 1;
		}

		if (Check_Key_Pressed(Keys_Def[3].Start)) Controller_1C_Start = 0;
		else Controller_1C_Start = 1;

		if (Check_Key_Pressed(Keys_Def[3].A)) Controller_1C_A = 0;
		else Controller_1C_A = 1;

		if (Check_Key_Pressed(Keys_Def[3].B)) Controller_1C_B = 0;
		else Controller_1C_B = 1;

		if (Check_Key_Pressed(Keys_Def[3].C)) Controller_1C_C = 0;
		else Controller_1C_C = 1;

		if (Controller_1C_Type & 1)
		{
			if (Check_Key_Pressed(Keys_Def[3].Mode)) Controller_1C_Mode = 0;
			else Controller_1C_Mode = 1;

			if (Check_Key_Pressed(Keys_Def[3].X)) Controller_1C_X = 0;
			else Controller_1C_X = 1;

			if (Check_Key_Pressed(Keys_Def[3].Y)) Controller_1C_Y = 0;
			else Controller_1C_Y = 1;

			if (Check_Key_Pressed(Keys_Def[3].Z)) Controller_1C_Z = 0;
			else Controller_1C_Z = 1;
		}

		if (Check_Key_Pressed(Keys_Def[4].Up))
		{
			Controller_1D_Up = 0;
			Controller_1D_Down = 1;
		}
		else
		{
			Controller_1D_Up = 1;
			if (Check_Key_Pressed(Keys_Def[4].Down)) Controller_1D_Down = 0;
			else Controller_1D_Down = 1;
		}
	
		if (Check_Key_Pressed(Keys_Def[4].Left))
		{
			Controller_1D_Left = 0;
			Controller_1D_Right = 1;
		}
		else
		{
			Controller_1D_Left = 1;
			if (Check_Key_Pressed(Keys_Def[4].Right)) Controller_1D_Right = 0;
			else Controller_1D_Right = 1;
		}

		if (Check_Key_Pressed(Keys_Def[4].Start)) Controller_1D_Start = 0;
		else Controller_1D_Start = 1;

		if (Check_Key_Pressed(Keys_Def[4].A)) Controller_1D_A = 0;
		else Controller_1D_A = 1;

		if (Check_Key_Pressed(Keys_Def[4].B)) Controller_1D_B = 0;
		else Controller_1D_B = 1;

		if (Check_Key_Pressed(Keys_Def[4].C)) Controller_1D_C = 0;
		else Controller_1D_C = 1;

		if (Controller_1D_Type & 1)
		{
			if (Check_Key_Pressed(Keys_Def[4].Mode)) Controller_1D_Mode = 0;
			else Controller_1D_Mode = 1;

			if (Check_Key_Pressed(Keys_Def[4].X)) Controller_1D_X = 0;
			else Controller_1D_X = 1;

			if (Check_Key_Pressed(Keys_Def[4].Y)) Controller_1D_Y = 0;
			else Controller_1D_Y = 1;

			if (Check_Key_Pressed(Keys_Def[4].Z)) Controller_1D_Z = 0;
			else Controller_1D_Z = 1;
		}
	}

	if (Controller_2_Type & 0x10)			// TEAMPLAYER PORT 2
	{
		if (Check_Key_Pressed(Keys_Def[5].Up))
		{
			Controller_2B_Up = 0;
			Controller_2B_Down = 1;
		}
		else
		{
			Controller_2B_Up = 1;
			if (Check_Key_Pressed(Keys_Def[5].Down)) Controller_2B_Down = 0;
			else Controller_2B_Down = 1;
		}
	
		if (Check_Key_Pressed(Keys_Def[5].Left))
		{
			Controller_2B_Left = 0;
			Controller_2B_Right = 1;
		}
		else
		{
			Controller_2B_Left = 1;
			if (Check_Key_Pressed(Keys_Def[5].Right)) Controller_2B_Right = 0;
			else Controller_2B_Right = 1;
		}

		if (Check_Key_Pressed(Keys_Def[5].Start)) Controller_2B_Start = 0;
		else Controller_2B_Start = 1;

		if (Check_Key_Pressed(Keys_Def[5].A)) Controller_2B_A = 0;
		else Controller_2B_A = 1;

		if (Check_Key_Pressed(Keys_Def[5].B)) Controller_2B_B = 0;
		else Controller_2B_B = 1;

		if (Check_Key_Pressed(Keys_Def[5].C)) Controller_2B_C = 0;
		else Controller_2B_C = 1;

		if (Controller_2B_Type & 1)
		{
			if (Check_Key_Pressed(Keys_Def[5].Mode)) Controller_2B_Mode = 0;
			else Controller_2B_Mode = 1;

			if (Check_Key_Pressed(Keys_Def[5].X)) Controller_2B_X = 0;
			else Controller_2B_X = 1;

			if (Check_Key_Pressed(Keys_Def[5].Y)) Controller_2B_Y = 0;
			else Controller_2B_Y = 1;

			if (Check_Key_Pressed(Keys_Def[5].Z)) Controller_2B_Z = 0;
			else Controller_2B_Z = 1;
		}

		if (Check_Key_Pressed(Keys_Def[6].Up))
		{
			Controller_2C_Up = 0;
			Controller_2C_Down = 1;
		}
		else
		{
			Controller_2C_Up = 1;
			if (Check_Key_Pressed(Keys_Def[6].Down)) Controller_2C_Down = 0;
			else Controller_2C_Down = 1;
		}
	
		if (Check_Key_Pressed(Keys_Def[6].Left))
		{
			Controller_2C_Left = 0;
			Controller_2C_Right = 1;
		}
		else
		{
			Controller_2C_Left = 1;
			if (Check_Key_Pressed(Keys_Def[6].Right)) Controller_2C_Right = 0;
			else Controller_2C_Right = 1;
		}

		if (Check_Key_Pressed(Keys_Def[6].Start)) Controller_2C_Start = 0;
		else Controller_2C_Start = 1;

		if (Check_Key_Pressed(Keys_Def[6].A)) Controller_2C_A = 0;
		else Controller_2C_A = 1;

		if (Check_Key_Pressed(Keys_Def[6].B)) Controller_2C_B = 0;
		else Controller_2C_B = 1;

		if (Check_Key_Pressed(Keys_Def[6].C)) Controller_2C_C = 0;
		else Controller_2C_C = 1;

		if (Controller_2C_Type & 1)
		{
			if (Check_Key_Pressed(Keys_Def[6].Mode)) Controller_2C_Mode = 0;
			else Controller_2C_Mode = 1;

			if (Check_Key_Pressed(Keys_Def[6].X)) Controller_2C_X = 0;
			else Controller_2C_X = 1;

			if (Check_Key_Pressed(Keys_Def[6].Y)) Controller_2C_Y = 0;
			else Controller_2C_Y = 1;

			if (Check_Key_Pressed(Keys_Def[6].Z)) Controller_2C_Z = 0;
			else Controller_2C_Z = 1;
		}

		if (Check_Key_Pressed(Keys_Def[7].Up))
		{
			Controller_2D_Up = 0;
			Controller_2D_Down = 1;
		}
		else
		{
			Controller_2D_Up = 1;
			if (Check_Key_Pressed(Keys_Def[7].Down)) Controller_2D_Down = 0;
			else Controller_2D_Down = 1;
		}
	
		if (Check_Key_Pressed(Keys_Def[7].Left))
		{
			Controller_2D_Left = 0;
			Controller_2D_Right = 1;
		}
		else
		{
			Controller_2D_Left = 1;
			if (Check_Key_Pressed(Keys_Def[7].Right)) Controller_2D_Right = 0;
			else Controller_2D_Right = 1;
		}

		if (Check_Key_Pressed(Keys_Def[7].Start)) Controller_2D_Start = 0;
		else Controller_2D_Start = 1;

		if (Check_Key_Pressed(Keys_Def[7].A)) Controller_2D_A = 0;
		else Controller_2D_A = 1;

		if (Check_Key_Pressed(Keys_Def[7].B)) Controller_2D_B = 0;
		else Controller_2D_B = 1;

		if (Check_Key_Pressed(Keys_Def[7].C)) Controller_2D_C = 0;
		else Controller_2D_C = 1;

		if (Controller_2D_Type & 1)
		{
			if (Check_Key_Pressed(Keys_Def[7].Mode)) Controller_2D_Mode = 0;
			else Controller_2D_Mode = 1;

			if (Check_Key_Pressed(Keys_Def[7].X)) Controller_2D_X = 0;
			else Controller_2D_X = 1;

			if (Check_Key_Pressed(Keys_Def[7].Y)) Controller_2D_Y = 0;
			else Controller_2D_Y = 1;

			if (Check_Key_Pressed(Keys_Def[7].Z)) Controller_2D_Z = 0;
			else Controller_2D_Z = 1;
		}
	}

	//	##RA Home button access:
	static int nBlockHomeButton = 0;
	bool bHomePressed = ( (Check_Key_Pressed(Keys_Def[0].RA_Home) ) || (Check_Key_Pressed(Keys_Def[2].RA_Home) ) || (Check_Key_Pressed(Keys_Def[3].RA_Home) ) || (Check_Key_Pressed(Keys_Def[4].RA_Home) ) );
	if( !nBlockHomeButton && bHomePressed )
	{
		//	Attempt toggle overlay
		nBlockHomeButton = 1;
		SendMessage( HWnd, WM_COMMAND, ID_EMULATION_PAUSED, 0 );
	}
	else if( !bHomePressed )
	{
		nBlockHomeButton = 0;
	}
}


int Setting_Keys(HWND hset, int Player, int TypeP)
{
	HWND Txt1, Txt2;
	MSG m;

	Sleep(250);
	Txt1 = GetDlgItem(hset, IDC_STATIC_TEXT1);
	Txt2 = GetDlgItem(hset, IDC_STATIC_TEXT2);
	if (Txt1 == NULL) return 0;
	if (Txt2 == NULL) return 0;

	SetWindowText(Txt1, "INPUT KEY FOR UP");
	Keys_Def[Player].Up = Get_Key();
	Sleep(250);

	SetWindowText(Txt1, "INPUT KEY FOR DOWN");
	Keys_Def[Player].Down = Get_Key();
	Sleep(250);
	
	SetWindowText(Txt1, "INPUT KEY FOR LEFT");
	Keys_Def[Player].Left = Get_Key();
	Sleep(250);
	
	SetWindowText(Txt1, "INPUT KEY FOR RIGHT");
	Keys_Def[Player].Right = Get_Key();
	Sleep(250);
	
	SetWindowText(Txt1, "INPUT KEY FOR START");
	Keys_Def[Player].Start = Get_Key();
	Sleep(250);
	
	SetWindowText(Txt1, "INPUT KEY FOR A");
	Keys_Def[Player].A = Get_Key();
	Sleep(250);
	
	SetWindowText(Txt1, "INPUT KEY FOR B");
	Keys_Def[Player].B = Get_Key();
	Sleep(250);
	
	SetWindowText(Txt1, "INPUT KEY FOR C");
	Keys_Def[Player].C = Get_Key();
	Sleep(250);

	if (TypeP)
	{
		SetWindowText(Txt1, "INPUT KEY FOR MODE");
		Keys_Def[Player].Mode = Get_Key();
		Sleep(250);

		SetWindowText(Txt1, "INPUT KEY FOR X");
		Keys_Def[Player].X = Get_Key();
		Sleep(250);

		SetWindowText(Txt1, "INPUT KEY FOR Y");
		Keys_Def[Player].Y = Get_Key();
		Sleep(250);

		SetWindowText(Txt1, "INPUT KEY FOR Z");
		Keys_Def[Player].Z = Get_Key();
		Sleep(250);
	}

	SetWindowText(Txt1, "INPUT KEY FOR HOME");
	Keys_Def[Player].RA_Home = Get_Key();
	Sleep(250);

	// ##RW: Added key profile
	SetWindowText(Txt1, "INPUT KEY FOR REWiND");
	Keys_Def[Player].Rewind = Get_Key();
	Sleep(250);

	SetWindowText(Txt1, "CONFIGURATION SUCCESSFULL");
	SetWindowText(Txt2, "PRESS A KEY TO CONTINUE ...");
	Get_Key();
	Sleep(500);

	while (PeekMessage(&m, hset, WM_KEYDOWN, WM_KEYDOWN, PM_REMOVE));

	SetWindowText(Txt1, "");
	SetWindowText(Txt2, "");

	return 1;
}


void Scan_Player_Net(int Player)
{
	if (!Player) return;

	Update_Input();

	if (Check_Key_Pressed(Keys_Def[0].Up))
	{
		Kaillera_Keys[0] &= ~0x08;
		Kaillera_Keys[0] |= 0x04;
	}
	else
	{
		Kaillera_Keys[0] |= 0x08;
		if (Check_Key_Pressed(Keys_Def[0].Down)) Kaillera_Keys[0] &= ~0x04;
		else Kaillera_Keys[0] |= 0x04;
	}
	
	if (Check_Key_Pressed(Keys_Def[0].Left))
	{
		Kaillera_Keys[0] &= ~0x02;
		Kaillera_Keys[0] |= 0x01;
	}
	else
	{
		Kaillera_Keys[0] |= 0x02;
		if (Check_Key_Pressed(Keys_Def[0].Right)) Kaillera_Keys[0] &= ~0x01;
		else Kaillera_Keys[0] |= 0x01;
	}

	if (Check_Key_Pressed(Keys_Def[0].Start)) Kaillera_Keys[0] &= ~0x80;
	else Kaillera_Keys[0] |= 0x80;

	if (Check_Key_Pressed(Keys_Def[0].A)) Kaillera_Keys[0] &= ~0x40;
	else Kaillera_Keys[0] |= 0x40;

	if (Check_Key_Pressed(Keys_Def[0].B)) Kaillera_Keys[0] &= ~0x20;
	else Kaillera_Keys[0] |= 0x20;

	if (Check_Key_Pressed(Keys_Def[0].C)) Kaillera_Keys[0] &= ~0x10;
	else Kaillera_Keys[0] |= 0x10;

	if (Controller_1_Type & 1)
	{
		if (Check_Key_Pressed(Keys_Def[0].Mode)) Kaillera_Keys[1] &= ~0x08;
		else Kaillera_Keys[1] |= 0x08;

		if (Check_Key_Pressed(Keys_Def[0].X)) Kaillera_Keys[1] &= ~0x04;
		else Kaillera_Keys[1] |= 0x04;

		if (Check_Key_Pressed(Keys_Def[0].Y)) Kaillera_Keys[1] &= ~0x02;
		else Kaillera_Keys[1] |= 0x02;

		if (Check_Key_Pressed(Keys_Def[0].Z)) Kaillera_Keys[1] &= ~0x01;
		else Kaillera_Keys[1] |= 0x01;
	}
}


void Update_Controllers_Net(int num_player)
{
	Controller_1_Up = (Kaillera_Keys[0] & 0x08) >> 3;
	Controller_1_Down = (Kaillera_Keys[0] & 0x04) >> 2;
	Controller_1_Left = (Kaillera_Keys[0] & 0x02) >> 1;
	Controller_1_Right = (Kaillera_Keys[0] & 0x01);
	Controller_1_Start = (Kaillera_Keys[0] & 0x80) >> 7;
	Controller_1_A = (Kaillera_Keys[0] & 0x40) >> 6;
	Controller_1_B = (Kaillera_Keys[0] & 0x20) >> 5;
	Controller_1_C = (Kaillera_Keys[0] & 0x10) >> 4;

	if (Controller_1_Type & 1)
	{
		Controller_1_Mode = (Kaillera_Keys[0 + 1] & 0x08) >> 3;
		Controller_1_X = (Kaillera_Keys[0 + 1] & 0x04) >> 2;
		Controller_1_Y = (Kaillera_Keys[0 + 1] & 0x02) >> 1;
		Controller_1_Z = (Kaillera_Keys[0 + 1] & 0x01);
	}

	if (num_player > 2)			// TEAMPLAYER
	{
		Controller_1B_Up = (Kaillera_Keys[2] & 0x08) >> 3;
		Controller_1B_Down = (Kaillera_Keys[2] & 0x04) >> 2;
		Controller_1B_Left = (Kaillera_Keys[2] & 0x02) >> 1;
		Controller_1B_Right = (Kaillera_Keys[2] & 0x01);
		Controller_1B_Start = (Kaillera_Keys[2] & 0x80) >> 7;
		Controller_1B_A = (Kaillera_Keys[2] & 0x40) >> 6;
		Controller_1B_B = (Kaillera_Keys[2] & 0x20) >> 5;
		Controller_1B_C = (Kaillera_Keys[2] & 0x10) >> 4;

		if (Controller_1B_Type & 1)
		{
			Controller_1B_Mode = (Kaillera_Keys[2 + 1] & 0x08) >> 3;
			Controller_1B_X = (Kaillera_Keys[2 + 1] & 0x04) >> 2;
			Controller_1B_Y = (Kaillera_Keys[2 + 1] & 0x02) >> 1;
			Controller_1B_Z = (Kaillera_Keys[2 + 1] & 0x01);
		}

		Controller_1C_Up = (Kaillera_Keys[4] & 0x08) >> 3;
		Controller_1C_Down = (Kaillera_Keys[4] & 0x04) >> 2;
		Controller_1C_Left = (Kaillera_Keys[4] & 0x02) >> 1;
		Controller_1C_Right = (Kaillera_Keys[4] & 0x01);
		Controller_1C_Start = (Kaillera_Keys[4] & 0x80) >> 7;
		Controller_1C_A = (Kaillera_Keys[4] & 0x40) >> 6;
		Controller_1C_B = (Kaillera_Keys[4] & 0x20) >> 5;
		Controller_1C_C = (Kaillera_Keys[4] & 0x10) >> 4;

		if (Controller_1C_Type & 1)
		{
			Controller_1C_Mode = (Kaillera_Keys[4 + 1] & 0x08) >> 3;
			Controller_1C_X = (Kaillera_Keys[4 + 1] & 0x04) >> 2;
			Controller_1C_Y = (Kaillera_Keys[4 + 1] & 0x02) >> 1;
			Controller_1C_Z = (Kaillera_Keys[4 + 1] & 0x01);
		}

		Controller_1D_Up = (Kaillera_Keys[6] & 0x08) >> 3;
		Controller_1D_Down = (Kaillera_Keys[6] & 0x04) >> 2;
		Controller_1D_Left = (Kaillera_Keys[6] & 0x02) >> 1;
		Controller_1D_Right = (Kaillera_Keys[6] & 0x01);
		Controller_1D_Start = (Kaillera_Keys[6] & 0x80) >> 7;
		Controller_1D_A = (Kaillera_Keys[6] & 0x40) >> 6;
		Controller_1D_B = (Kaillera_Keys[6] & 0x20) >> 5;
		Controller_1D_C = (Kaillera_Keys[6] & 0x10) >> 4;

		if (Controller_1D_Type & 1)
		{
			Controller_1D_Mode = (Kaillera_Keys[6 + 1] & 0x08) >> 3;
			Controller_1D_X = (Kaillera_Keys[6 + 1] & 0x04) >> 2;
			Controller_1D_Y = (Kaillera_Keys[6 + 1] & 0x02) >> 1;
			Controller_1D_Z = (Kaillera_Keys[6 + 1] & 0x01);
		}
	}
	else
	{
		Controller_2_Up = (Kaillera_Keys[2] & 0x08) >> 3;
		Controller_2_Down = (Kaillera_Keys[2] & 0x04) >> 2;
		Controller_2_Left = (Kaillera_Keys[2] & 0x02) >> 1;
		Controller_2_Right = (Kaillera_Keys[2] & 0x01);
		Controller_2_Start = (Kaillera_Keys[2] & 0x80) >> 7;
		Controller_2_A = (Kaillera_Keys[2] & 0x40) >> 6;
		Controller_2_B = (Kaillera_Keys[2] & 0x20) >> 5;
		Controller_2_C = (Kaillera_Keys[2] & 0x10) >> 4;

		if (Controller_2_Type & 1)
		{
			Controller_2_Mode = (Kaillera_Keys[2 + 1] & 0x08) >> 3;
			Controller_2_X = (Kaillera_Keys[2 + 1] & 0x04) >> 2;
			Controller_2_Y = (Kaillera_Keys[2 + 1] & 0x02) >> 1;
			Controller_2_Z = (Kaillera_Keys[2 + 1] & 0x01);
		}
	}
}


