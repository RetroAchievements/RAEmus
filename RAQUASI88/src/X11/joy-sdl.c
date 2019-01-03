/************************************************************************/
/* SDL用 ジョイスティック入力処理					*/
/*									*/
/*	このファイルは、 joystick.c からインクルードされます		*/
/*									*/
/************************************************************************/
#if	defined(JOY_SDL)

#include <stdio.h>
#include <SDL.h>

#include "quasi88.h"
#include "keyboard.h"
#include "joystick.h"
#include "event.h"


#define	JOY_MAX   	KEY88_PAD_MAX		/* ジョイスティック上限(2個) */

#define	BUTTON_MAX	KEY88_PAD_BUTTON_MAX	/* ボタン上限(8個)	     */

#define	AXIS_U		0x01
#define	AXIS_D		0x02
#define	AXIS_L		0x04
#define	AXIS_R		0x08

typedef struct {

    SDL_Joystick *dev;		/* オープンしたジョイスティックの構造体 */
    int		  num;		/* QUASI88 でのジョイスティック番号 0〜 */

    int		  axis;			/* 方向ボタン押下状態		*/
    int		  nr_button;		/* 有効なボタンの数		*/
    Uint8	  button[ BUTTON_MAX ];	/* ボタンの押下状態		*/

} T_JOY_INFO;

static T_JOY_INFO joy_info[ JOY_MAX ];

static	int	joystick_num;		/* オープンしたジョイスティックの数 */




void	joystick_init(void)
{
    SDL_Joystick *dev;
    int i, max, nr_button;

    /* ワーク初期化 */
    joystick_num = 0;

    memset(joy_info, 0, sizeof(joy_info));
    for (i=0; i<JOY_MAX; i++) {
	joy_info[i].dev = NULL;
    }

    /* ジョイスティックサブシステム初期化 */
    if (verbose_proc) printf("Initializing joystick ... ");

    if (! SDL_WasInit(SDL_INIT_JOYSTICK)) {
	if (SDL_InitSubSystem(SDL_INIT_JOYSTICK)) {
	    if (verbose_proc) printf("FAILED\n");
	    return;
	}
    }

    /* ジョイスティックの数を調べて、デバイスオープン */
    max = SDL_NumJoysticks();
    max = MIN(max, JOY_MAX);		/* ワークの数だけ、有効 */

    for (i=0; i<max; i++) {
	dev = SDL_JoystickOpen(i);	/* i番目のジョイスティックをオープン */

	if (dev) {
	    /* ボタンの数を調べる */
	    nr_button = SDL_JoystickNumButtons(dev);
	    nr_button = MIN(nr_button, BUTTON_MAX);

	    joy_info[i].dev = dev;
	    joy_info[i].num = joystick_num ++;
	    joy_info[i].nr_button = nr_button;
	}
    }

    if (joystick_num > 0) {			/* 1個以上オープンできたら  */
	SDL_JoystickEventState(SDL_IGNORE);
    } else {
	SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
    }


    if (verbose_proc) {
	if (max == 0) {
	        printf("FAILED (not found joystick)\n");
	} else {
	    if (joystick_num == 0) {
		printf("FAILED (can't open joystick)\n");
	    } else {
		printf("OK (found %d joystick(s))\n", joystick_num);
	    }
	}
    }
}



void	joystick_exit(void)
{
    int i;

    if (joystick_num > 0) {

	for (i=0; i<JOY_MAX; i++) {
	    if (joy_info[i].dev) {
		SDL_JoystickClose(joy_info[i].dev);
		joy_info[i].dev = NULL;
	    }
	}

	joystick_num = 0;
    }
}



void	joystick_update(void)
{
    int i, j;
    int now, chg;
    T_JOY_INFO *joy = &joy_info[0];
    Sint16 x, y;
    Uint8 button;

    if (joystick_num > 0) {

	SDL_JoystickUpdate();

	for (i=0; i<JOY_MAX; i++, joy++) {

	    if (joy->dev) {
		int offset = (joy->num) * KEY88_PAD_OFFSET;
		/*Uint8 pad = SDL_JoystickGetHat(joy->dev, 
					       SDL_HAT_UP   | SDL_HAT_RIGHT | 
					       SDL_HAT_DOWN | SDL_HAT_LEFT);*/

		now = 0;
		x = SDL_JoystickGetAxis(joy->dev, 0);
		y = SDL_JoystickGetAxis(joy->dev, 1);

		if      (x < -0x4000) now |= AXIS_L;
		else if (x >  0x4000) now |= AXIS_R;

		if      (y < -0x4000) now |= AXIS_U;
		else if (y >  0x4000) now |= AXIS_D;

		chg = joy->axis ^ now;

		if (chg & AXIS_L) {
		    quasi88_pad(KEY88_PAD1_LEFT  + offset, (now & AXIS_L));
		}
		if (chg & AXIS_R) {
		    quasi88_pad(KEY88_PAD1_RIGHT + offset, (now & AXIS_R));
		}
		if (chg & AXIS_U) {
		    quasi88_pad(KEY88_PAD1_UP    + offset, (now & AXIS_U));
		}
		if (chg & AXIS_D) {
		    quasi88_pad(KEY88_PAD1_DOWN  + offset, (now & AXIS_D));
		}

		joy->axis = now;

		for (j=0; j<joy->nr_button; j++) {

		    button = SDL_JoystickGetButton(joy->dev, j);

		    if (joy->button[j] != button) {
			quasi88_pad(KEY88_PAD1_A + j + offset, (button));
			joy->button[j] = button;
		    }
		}
	    }
	}
    }
}



int	event_get_joystick_num(void)
{
    return joystick_num;
}

#endif	/* JOY_SDL */
