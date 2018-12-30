/************************************************************************/
/* LINUX-USB用 ジョイスティック入力処理					*/
/*									*/
/*	このファイルは、 joystick.c からインクルードされます		*/
/*									*/
/************************************************************************/
#if	defined(JOY_LINUX_USB)

/*
  以下のコードは、引地さん [eiichi@licorp.co.jp] により提供されました。

	メイン側の処理に合わせて、変数名・関数名などを修正しています。
*/


#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "quasi88.h"
#include "keyboard.h"
#include "joystick.h"
#include "event.h"

int	enable_joystick = FALSE;	/* ジョイスティックの使用可否 */



#ifdef NAKAMIZO
unsigned char joy_code[][2][4] = {
    {{0x01, 0x80, 0x02, 0x01}, {0xef, 0x53, 0x02, 0x01}},    /* 上 */
    {{0xff, 0x7f, 0x02, 0x01}, {0xef, 0x53, 0x02, 0x01}},    /* 下 */
    {{0xff, 0x7f, 0x02, 0x00}, {0xef, 0x53, 0x02, 0x00}},    /* 右 */
    {{0x01, 0x80, 0x02, 0x00}, {0xef, 0x53, 0x02, 0x00}},    /* 左 */
    {{0x01, 0x00, 0x01, 0x0b}, {0x00, 0x00, 0x01, 0x0b}},    /* start */
    {{0x01, 0x00, 0x01, 0x0a}, {0x00, 0x00, 0x01, 0x0a}},    /* select */
    {{0x01, 0x00, 0x01, 0x00}, {0x00, 0x00, 0x01, 0x00}},    /* A */
    {{0x01, 0x00, 0x01, 0x01}, {0x00, 0x00, 0x01, 0x01}},    /* B */
    {{0x01, 0x00, 0x01, 0x02}, {0x00, 0x00, 0x01, 0x02}},    /* C */
    {{0x01, 0x00, 0x01, 0x03}, {0x00, 0x00, 0x01, 0x03}},    /* X */
    {{0x01, 0x00, 0x01, 0x04}, {0x00, 0x00, 0x01, 0x04}},    /* Y */
    {{0x01, 0x00, 0x01, 0x05}, {0x00, 0x00, 0x01, 0x05}},    /* Z */
    {{0x01, 0x00, 0x01, 0x06}, {0x00, 0x00, 0x01, 0x06}},    /* R1 */
    {{0x01, 0x00, 0x01, 0x08}, {0x00, 0x00, 0x01, 0x08}},    /* R2 */
    {{0x01, 0x00, 0x01, 0x07}, {0x00, 0x00, 0x01, 0x07}},    /* L1 */
    {{0x01, 0x00, 0x01, 0x09}, {0x00, 0x00, 0x01, 0x09}}     /* L2 */
};
#endif /* NAKAMIZO */

unsigned char joy_code[][2][4] = {
    {{0x01, 0x80, 0x02, 0x01}, {0x00, 0x00, 0x02, 0x01}},    /* 上 */
    {{0xff, 0x7f, 0x02, 0x01}, {0x00, 0x00, 0x02, 0x01}},    /* 下 */
    {{0xff, 0x7f, 0x02, 0x00}, {0x00, 0x00, 0x02, 0x00}},    /* 右 */
    {{0x01, 0x80, 0x02, 0x00}, {0x00, 0x00, 0x02, 0x00}},    /* 左 */
    {{0x01, 0x00, 0x01, 0x0b}, {0x00, 0x00, 0x01, 0x0b}},    /* start */
    {{0x01, 0x00, 0x01, 0x0a}, {0x00, 0x00, 0x01, 0x0a}},    /* select */
    {{0x01, 0x00, 0x01, 0x00}, {0x00, 0x00, 0x01, 0x00}},    /* A */
    {{0x01, 0x00, 0x01, 0x01}, {0x00, 0x00, 0x01, 0x01}},    /* B */
    {{0x01, 0x00, 0x01, 0x02}, {0x00, 0x00, 0x01, 0x02}},    /* C */
    {{0x01, 0x00, 0x01, 0x03}, {0x00, 0x00, 0x01, 0x03}},    /* X */
    {{0x01, 0x00, 0x01, 0x04}, {0x00, 0x00, 0x01, 0x04}},    /* Y */
    {{0x01, 0x00, 0x01, 0x05}, {0x00, 0x00, 0x01, 0x05}},    /* Z */
    {{0x01, 0x00, 0x01, 0x06}, {0x00, 0x00, 0x01, 0x06}},    /* R1 */
    {{0x01, 0x00, 0x01, 0x08}, {0x00, 0x00, 0x01, 0x08}},    /* R2 */
    {{0x01, 0x00, 0x01, 0x07}, {0x00, 0x00, 0x01, 0x07}},    /* L1 */
    {{0x01, 0x00, 0x01, 0x09}, {0x00, 0x00, 0x01, 0x09}}     /* L2 */
};

#define JOY_BUTTON_UP      0
#define JOY_BUTTON_DOWN    1
#define JOY_BUTTON_RIGHT   2
#define JOY_BUTTON_LEFT    3
#define JOY_BUTTON_START   4
#define JOY_BUTTON_SELECT  5
#define JOY_BUTTON_A       6
#define JOY_BUTTON_B       7
#define JOY_BUTTON_C       8
#define JOY_BUTTON_X       9
#define JOY_BUTTON_Y       10
#define JOY_BUTTON_Z       11
#define JOY_BUTTON_R1      12
#define JOY_BUTTON_R2      13
#define JOY_BUTTON_L1      14
#define JOY_BUTTON_L2      15

typedef struct {
    int button;
    int is_press;
} JOY_BUTTON;

static	JOY_BUTTON joy_button[10];      /* 変化のあった Joystick のボタン */
static	FILE*   joystick_device;        /* Joystick デバイス */

void joystick_init(void)
{
    int   fd;

    enable_joystick = FALSE;
    if( verbose_proc ) printf( "\nInitializing joystick ... " );

    /*
    if( mouse_mode != 3 ){
      joystick_device = NULL;
      return;
    }
    */

    fd = open("/dev/input/js0", O_RDONLY | O_NONBLOCK);
    if (fd != -1) {
	joystick_device = fdopen(fd, "r");
	enable_joystick = TRUE;
    } else {
	joystick_device = NULL;
    }

    if (verbose_proc) printf("%s\n", enable_joystick ? "OK" : "FAILED");
}




void joystick_exit(void)
{
    if (enable_joystick == FALSE ) { return; }

    if (joystick_device) {
	fclose(joystick_device);
    }
    enable_joystick = FALSE;
}





static int press_up_down = 0;           /* コントローラ上下ボタン押下状態 */
static int press_right_left = 0;        /* コントローラ左右ボタン押下状態 */

void joystick_update(void)
{
    int i;
    int j;
    int len;
    unsigned char joydata[8];

    if (enable_joystick == FALSE ) { return; }

    j = 0;
    for (i = 0; i < 10; i ++) {
	joy_button[i].button = -1;
	joy_button[i].is_press = -1;
    }
    if (joystick_device != NULL) {
	while ((len = fread(joydata, 1, sizeof(joydata), joystick_device)) == 8) {
	    if (len == 8) {
		for (i = 0; i <= 16; i ++) {
		    if (!memcmp(joydata + 4, joy_code[i][0], 4)) {
			joy_button[j].button = i;
			joy_button[j].is_press = 1;
			j ++;
			break;
		    }
		    if (!memcmp(joydata + 4, joy_code[i][1], 4)) {
			joy_button[j].button = i;
			joy_button[j].is_press = 0;
			j ++;
			break;
		    }
		}
		if (j == 10) {
		    break;
		}
	    }
	}

	for (i = 0; i < 10; i ++) {
	  if (joy_button[i].button == -1) {
	    break;
	  }
	  if (joy_button[i].button == JOY_BUTTON_A) {

	    quasi88_pad( KEY88_PAD1_A, (joy_button[i].is_press) );

	  } else if (joy_button[i].button == JOY_BUTTON_B) {

	    quasi88_pad( KEY88_PAD1_B, (joy_button[i].is_press) );

	  } else if (joy_button[i].button == JOY_BUTTON_UP) {
	    if (joy_button[i].is_press) {
	      press_up_down = 1;
	    } else {
	      press_up_down = 0;
	    }
	  } else if (joy_button[i].button == JOY_BUTTON_DOWN) {
	    if (joy_button[i].is_press) {
	      press_up_down = 2;
	    } else {
	      press_up_down = 0;
	    }
	  } else if (joy_button[i].button == JOY_BUTTON_RIGHT) {
	    if (joy_button[i].is_press) {
	      press_right_left = 1;
	    } else {
	      press_right_left = 0;
	    }
	  } else if (joy_button[i].button == JOY_BUTTON_LEFT) {
	    if (joy_button[i].is_press) {
	      press_right_left = 2;
	    } else {
	      press_right_left = 0;
	    }
	  }
	}

	/* 上下ボタン */
	quasi88_pad( KEY88_PAD1_UP,    FALSE );
	quasi88_pad( KEY88_PAD1_DOWN,  FALSE );
	if (press_up_down == 1) {
	  quasi88_pad( KEY88_PAD1_UP,    TRUE );
	} else if (press_up_down == 2) {
	  quasi88_pad( KEY88_PAD1_DOWN,  TRUE );
	}

	/* 左右ボタン */
	quasi88_pad( KEY88_PAD1_RIGHT, FALSE );
	quasi88_pad( KEY88_PAD1_LEFT,  FALSE );
	if (press_right_left == 1) {
	  quasi88_pad( KEY88_PAD1_RIGHT, TRUE );
	} else if (press_right_left == 2) {
	  quasi88_pad( KEY88_PAD1_LEFT,  TRUE );
	}
    }
}



int	event_get_joystick_num(void)
{
    return (enable_joystick) ? 1 :0;
}

#endif	/* JOY_LINUX_USB */
