/************************************************************************/
/*									*/
/* ポーズ (一時停止) モード						*/
/*									*/
/************************************************************************/

/*	変数 pause_by_focus_out により処理が変わる			*/
/*	・pause_by_focus_out == 0 の時					*/
/*		ESCが押されると解除。	画面中央に PAUSEと表示		*/
/*	・pause_by_focus_out != 0 の時					*/
/*		X のマウスが画面内に入ると解除				*/

#include <stdio.h>

#include "quasi88.h"
#include "pause.h"

#include "emu.h"
#include "initval.h"
#include "status.h"
#include "screen.h"
#include "wait.h"
#include "event.h"


int	need_focus = FALSE;			/* フォーカスアウト停止あり */


static	int	pause_by_focus_out = FALSE;

/*
 * エミュ処理中に、フォーカスが無くなった (-focus指定時は、ポーズ開始)
 */
void	pause_event_focus_out_when_exec(void)
{
    if (need_focus) {				/* -focus 指定時は */
	pause_by_focus_out = TRUE;
	quasi88_pause();			/* ここで PAUSE する */
    }
}

/*
 * ポーズ中に、フォーカスを得た
 */
void	pause_event_focus_in_when_pause(void)
{
    if (pause_by_focus_out) {
	quasi88_exec();
    }
}

/*
 * ポーズ中に、ポーズ終了のキー(ESCキー)押下検知した
 */
void	pause_event_key_on_esc(void)
{
    quasi88_exec();
}

/*
 * ポーズ中に、メニュー開始のキー押下検知した
 */
void	pause_event_key_on_menu(void)
{
    quasi88_menu();
}









void	pause_init(void)
{
    status_message_default(0, " PAUSE ");
    status_message_default(1, "<ESC> key to return");
    status_message_default(2, NULL);
}


void	pause_main(void)
{
    /* 終了などを検知するために、イベント処理だけ実施 */
    event_update();


    /* 一時停止を抜けたら、ワーク再初期化 */
    if (quasi88_event_flags & EVENT_MODE_CHANGED) {

	pause_by_focus_out = FALSE;

    } else {

	quasi88_event_flags |= EVENT_FRAME_UPDATE;
    }
}
