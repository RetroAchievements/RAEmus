#ifndef Q8TK_GLIB_H_INCLUDED
#define Q8TK_GLIB_H_INCLUDED


/* 罫線 角 123456789_I = └┴┘├┼┤┌┬┐─│ */

#define	Q8GR_G_1	(0x9a)
#define	Q8GR_G_2	(0x90)
#define	Q8GR_G_3	(0x9b)
#define	Q8GR_G_4	(0x93)
#define	Q8GR_G_5	(0x8f)
#define	Q8GR_G_6	(0x92)
#define	Q8GR_G_7	(0x98)
#define	Q8GR_G_8	(0x91)
#define	Q8GR_G_9	(0x99)
#define	Q8GR_G__	(0x95)
#define	Q8GR_G_I	(0x96)

/* 罫線 丸 1357 = └┘┌┐ */

#define	Q8GR_C_1	(0x9e)
#define	Q8GR_C_3	(0x9f)
#define	Q8GR_C_7	(0x9c)
#define	Q8GR_C_9	(0x9d)

/* 矢印 ↑↓←→ */

#define	Q8GR_A_U	(0x1e)
#define	Q8GR_A_D	(0x1f)
#define	Q8GR_A_L	(0x1d)
#define	Q8GR_A_R	(0x1c)

/* ボタン等 ●○  ■×□＿ */

#define	Q8GR_B_ON	(0xec)
#define	Q8GR_B_OFF	(0xed)
#define	Q8GR_B_SPACE	(0x20)
#define	Q8GR_B_BOX	(0x87)
#define	Q8GR_B_X	(0xf0)
#define	Q8GR_B_B	(0xdb)
#define	Q8GR_B_UL	(0x80)

/* アッパーラインなど */

#define	Q8GR_L_UPPER	(0x94)
#define	Q8GR_L_UNDER	(0x80)
#define	Q8GR_L_LEFT	(0x88)
#define	Q8GR_L_RIGHT	(0x97)



void	q8gr_init(void);


void	q8gr_set_cursor_exist(int exist_flag);
int	q8gr_get_cursor_exist(void);
int	q8gr_get_cursor_blink(void);
void	q8gr_set_cursor_blink(void);


void	q8gr_clear_screen(void);

extern	Q8tkWidget	dummy_widget_window; /* ダミーの未使用ウィジット */
#define	Q8GR_WIDGET_NONE	(NULL)
#define	Q8GR_WIDGET_WINDOW	((void *)&dummy_widget_window)

void	q8gr_clear_focus_screen(void);
void	q8gr_set_focus_screen(int x, int y, int sx, int sy, void *p);
void	*q8gr_get_focus_screen(int x, int y);
int	q8gr_scan_focus_screen(void *p);
void	q8gr_set_screen_mask(int x, int y, int sx, int sy);
void	q8gr_reset_screen_mask(void);

void	q8gr_draw_mouse(int x, int y);


void	q8gr_draw_window(int x, int y, int sx, int sy, int shadow_type,
			 void *p);
void	q8gr_draw_button(int x, int y, int sx, int sy, int condition, void *p);
void	q8gr_draw_check_button(int x, int y, int condition, void *p);
void	q8gr_draw_radio_button(int x, int y, int condition, void *p);
void	q8gr_draw_notebook(int x, int y, int sx, int sy,
			   Q8tkWidget *notebook, void *p);
void	q8gr_draw_notepage(int code, const char *tag,
			   int select_flag, int active_flag,
			   Q8tkWidget *notebook, void *p);
void	q8gr_draw_vseparator(int x, int y, int height);
void	q8gr_draw_hseparator(int x, int y, int width);
void	q8gr_draw_frame(int x, int y, int sx, int sy, int shadow_type,
			int code, const char *str, void *p);
void	q8gr_draw_combo(int x, int y, int width, int active, void *p);
void	q8gr_draw_list_item(int x, int y, int width, int active,
			    int reverse, int underline,
			    int code, const char *text, void *p);

void	q8gr_draw_hscale(int x, int y, Q8Adjust *adj, int active,
			 int draw_value, int value_pos, void *p);
void	q8gr_draw_vscale(int x, int y, Q8Adjust *adj, int active,
			 int draw_value, int value_pos, void *p);

void	q8gr_draw_entry(int x, int y, int width, int code, const char *text,
			int disp_pos, int cursor_pos, void *p);

void	q8gr_draw_option_menu(int x, int y, int sx, int sy, int button,
			      void *p);
void	q8gr_draw_radio_menu_item(int x, int y, int width, int underline,
				  int code, const char *text, void *p);

void	q8gr_draw_scrolled_window(int x, int y, int sx, int sy,
				  int shadow_type, void *p);

void	q8gr_draw_logo(int x, int y);

int	q8gr_draw_label(int x, int y, int fg, int bg,
			int reverse, int underline, int code, const char *str,
			void *p);

int	q8gr_strlen(int code, const char *str);
int	q8gr_strchk(int code, const char *str, int pos);
int	q8gr_strdel(int code, char *str, int del_pos);
int	q8gr_stradd(int code, char *str, int add_pos, int add_chr);
void	q8gr_strncpy(int code, char *dst, const char *src, int size);
int	q8gr_strcode(const char *buffer, int size);


#endif	/* Q8TK_GLIB_H_INCLUDED */
