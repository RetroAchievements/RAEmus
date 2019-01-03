#ifndef SCREEN_FUNC_H_INCLUDED
#define SCREEN_FUNC_H_INCLUDED


enum {	V_COLOR,  V_MONO,   V_UNDISP, V_HIRESO,	  V_VRAM_MODE };
enum {	V_80x25,  V_80x20,  V_40x25,  V_40x20,	  V_TEXT_MODE };
enum {	V_DIF,	  V_ALL,			  V_METHOD    };

/* void (*関数リスト[ V_VRAM_MODE ][ V_TEXT_MODE ][ V_METHOD ])(void); */

/* ------------------------------------------------------------------------- */
#ifdef	SUPPORT_8BPP
extern	int  (*vram2screen_list_F_N__8[4][4][2])(void);
extern	int  (*vram2screen_list_F_S__8[4][4][2])(void);
extern	int  (*vram2screen_list_F_I__8[4][4][2])(void);
extern	int  (*vram2screen_list_H_N__8[4][4][2])(void);
extern	int  (*vram2screen_list_H_P__8[4][4][2])(void);
extern	int  (*vram2screen_list_D_N__8[4][4][2])(void);
extern	int  (*vram2screen_list_D_S__8[4][4][2])(void);
extern	int  (*vram2screen_list_D_I__8[4][4][2])(void);

extern	int  (*vram2screen_list_F_N__8_d[4][4][2])(void);
extern	int  (*vram2screen_list_D_N__8_d[4][4][2])(void);
extern	int  (*vram2screen_list_D_S__8_d[4][4][2])(void);
extern	int  (*vram2screen_list_D_I__8_d[4][4][2])(void);

extern	void	screen_buf_init__8(void);

extern	int	menu2screen_F_N__8(void);
extern	int	menu2screen_H_N__8(void);
extern	int	menu2screen_H_P__8(void);
extern	int	menu2screen_D_N__8(void);

extern	void	status2screen__8(int kind, byte pixmap[], int w, int h);
extern	void	status_buf_init__8(void);
extern	void	status_buf_clear__8(void);
#endif

/* ------------------------------------------------------------------------- */
#ifdef	SUPPORT_16BPP
extern	int  (*vram2screen_list_F_N_16[4][4][2])(void);
extern	int  (*vram2screen_list_F_S_16[4][4][2])(void);
extern	int  (*vram2screen_list_F_I_16[4][4][2])(void);
extern	int  (*vram2screen_list_H_N_16[4][4][2])(void);
extern	int  (*vram2screen_list_H_P_16[4][4][2])(void);
extern	int  (*vram2screen_list_D_N_16[4][4][2])(void);
extern	int  (*vram2screen_list_D_S_16[4][4][2])(void);
extern	int  (*vram2screen_list_D_I_16[4][4][2])(void);

extern	int  (*vram2screen_list_F_N_16_d[4][4][2])(void);
extern	int  (*vram2screen_list_D_N_16_d[4][4][2])(void);
extern	int  (*vram2screen_list_D_S_16_d[4][4][2])(void);
extern	int  (*vram2screen_list_D_I_16_d[4][4][2])(void);

extern	void	screen_buf_init_16(void);

extern	int	menu2screen_F_N_16(void);
extern	int	menu2screen_H_N_16(void);
extern	int	menu2screen_H_P_16(void);
extern	int	menu2screen_D_N_16(void);

extern	void	status2screen_16(int kind, byte pixmap[], int w, int h);
extern	void	status_buf_init_16(void);
extern	void	status_buf_clear_16(void);
#endif

/* ------------------------------------------------------------------------- */
#ifdef	SUPPORT_32BPP
extern	int  (*vram2screen_list_F_N_32[4][4][2])(void);
extern	int  (*vram2screen_list_F_S_32[4][4][2])(void);
extern	int  (*vram2screen_list_F_I_32[4][4][2])(void);
extern	int  (*vram2screen_list_H_N_32[4][4][2])(void);
extern	int  (*vram2screen_list_H_P_32[4][4][2])(void);
extern	int  (*vram2screen_list_D_N_32[4][4][2])(void);
extern	int  (*vram2screen_list_D_S_32[4][4][2])(void);
extern	int  (*vram2screen_list_D_I_32[4][4][2])(void);

extern	int  (*vram2screen_list_F_N_32_d[4][4][2])(void);
extern	int  (*vram2screen_list_D_N_32_d[4][4][2])(void);
extern	int  (*vram2screen_list_D_S_32_d[4][4][2])(void);
extern	int  (*vram2screen_list_D_I_32_d[4][4][2])(void);

extern	void	screen_buf_init_32(void);

extern	int	menu2screen_F_N_32(void);
extern	int	menu2screen_H_N_32(void);
extern	int	menu2screen_H_P_32(void);
extern	int	menu2screen_D_N_32(void);

extern	void	status2screen_32(int kind, byte pixmap[], int w, int h);
extern	void	status_buf_init_32(void);
extern	void	status_buf_clear_32(void);
#endif

/* ------------------------------------------------------------------------- */
extern	int  (*snapshot_list_normal[4][4][2])(void);
extern	int  (*snapshot_list_skipln[4][4][2])(void);
extern	int  (*snapshot_list_itlace[4][4][2])(void);

extern	void	snapshot_clear(void);



#endif	/* SCREEN_FUNC_H_INCLUDED */
