#ifndef G_DDRAW_H
#define G_DDRAW_H

#ifndef DIRECTDRAW_VERSION
#define DIRECTDRAW_VERSION         0x0500
#endif

#include <ddraw.h>
#include <time.h>

extern clock_t Last_Time;
extern clock_t New_Time;
extern clock_t Used_Time;

extern int Flag_Clr_Scr;
extern int Sleep_Time;
extern int FS_VSync;
extern int W_VSync;
extern int Stretch;
extern int Blit_Soft;
extern int Effect_Color;
extern int FPS_Style;
extern int Message_Style;
extern int Kaillera_Error;

extern void (*Blit_FS)(unsigned char *Dest, int pitch, int x, int y, int offset);
extern void (*Blit_W)(unsigned char *Dest, int pitch, int x, int y, int offset);
extern int (*Update_Frame)();
extern int (*Update_Frame_Fast)();

int Init_Fail(HWND hwnd);
int Init_DDraw(HWND hWnd);
int Clear_Primary_Screen(HWND hWnd);
int Clear_Back_Screen(HWND hWnd);
int Update_Gens_Logo(HWND hWnd);
int Update_Crazy_Effect(HWND hWnd);
int Update_Emulation(HWND hWnd);
int Update_Emulation_One(HWND hWnd);
int Update_Emulation_Netplay(HWND hWnd, int player, int num_player);
int Eff_Screen(void);
int Pause_Screen(void);
void Put_Info(char *Message, int Duree);
int Show_Genesis_Screen(HWND hWnd);
int Flip(HWND hWnd);
void Restore_Primary(void);
int Take_Shot();
void End_DDraw(void);


//void MP3_update_test();
//void MP3_init_test();


#endif