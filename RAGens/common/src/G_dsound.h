/////////////////////////////////////////////////////////////////////////////////////////////
// SOUND.H
/////////////////////////////////////////////////////////////////////////////////////////////
#ifndef SOUND_H
#define SOUND_H

#ifdef __cplusplus
extern "C" {
#endif /* #ifdef __cplusplus */

#define DIRECTSOUND_VERSION         0x0300

#include <dsound.h>

extern int Sound_Rate;
extern int Sound_Segs;
extern int Sound_Enable;
extern int Sound_Stereo;
extern int Sound_Is_Playing;
extern int Sound_Initialised;
extern int WAV_Dumping;
extern int GYM_Playing;
extern int Seg_L[882], Seg_R[882];
extern int Seg_Lenght;
extern int WP, RP;

extern unsigned int Sound_Interpol[882];
extern unsigned int Sound_Extrapol[312][2];

extern char Dump_Dir[1024];
extern char Dump_GYM_Dir[1024];

int Init_Sound(HWND hWnd);
void End_Sound(void);
int Get_Current_Seg(void);
int Check_Sound_Timing(void);
int Write_Sound_Buffer(void *Dump_Buf);
int Clear_Sound_Buffer(void);
int Play_Sound(void);
int Stop_Sound(void);
int Start_WAV_Dump(void);
int Update_WAV_Dump(void);
int Stop_WAV_Dump(void);
int Start_GYM_Dump(void);
int Stop_GYM_Dump(void);
int Start_Play_GYM(void);
int Play_GYM(void);
int Stop_Play_GYM(void);

#ifdef __cplusplus
}
#endif /* #ifdef __cplusplus */

#endif