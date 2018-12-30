#ifndef PAUSE_H_INCLUDED
#define PAUSE_H_INCLUDED

/************************************************************************/
/* 一時停止モード							*/
/************************************************************************/

void	pause_init(void);
void	pause_main(void);



/*----------------------------------------------------------------------
 * イベント処理の対処
 *----------------------------------------------------------------------*/
void	pause_event_focus_out_when_exec(void);
void	pause_event_focus_in_when_pause(void);
void	pause_event_key_on_esc(void);
void	pause_event_key_on_menu(void);





#endif	/* PAUSE_H_INCLUDED */
