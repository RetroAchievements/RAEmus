/************************************************************************/
/* ダミー用 ジョイスティック入力処理					*/
/*									*/
/*	このファイルは、 joystick.c からインクルードされます		*/
/*									*/
/************************************************************************/
#if	defined(JOY_NOTHING)


#include <stdio.h>

#include "quasi88.h"
#include "device.h"
#include "keyboard.h"
#include "joystick.h"
#include "event.h"



void	joystick_init(void)
{
    if (use_joydevice) {
	if (verbose_proc) printf("Joystick not supported\n");
    }
}

void	joystick_exit(void)
{
}

void	joystick_update(void)
{
}

int	event_get_joystick_num(void)
{
    return 0;
}

#endif	/* JOY_NOTHING */
