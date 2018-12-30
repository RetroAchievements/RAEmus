/************************************************************************/
/* BSD-USB用 ジョイスティック入力処理					*/
/*									*/
/*	このファイルは、 joystick.c からインクルードされます		*/
/*									*/
/************************************************************************/
#if	defined(JOY_BSD_USB)

#include <stdio.h>
#include <string.h>

#include "quasi88.h"
#include "keyboard.h"
#include "joystick.h"
#include "event.h"



#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>


#if defined(HAVE_USB_H)
#include <usb.h>
#endif
#include <dev/usb/usb.h>
#include <dev/usb/usbhid.h>

#if defined(HAVE_USBHID_H)
#include <usbhid.h>
#elif defined(HAVE_LIBUSB_H)
#include <libusb.h>
#elif defined(HAVE_LIBUSBHID_H)
#include <libusbhid.h>
#endif

#ifdef __FreeBSD__
#include <osreldate.h>				/* __FreeBSD_version */
#endif

#if defined(__NetBSD__) || defined(__OpenBSD__)
#include <machine/joystick.h>
#endif



#define	JOY_MAX   	KEY88_PAD_MAX		/* ジョイスティック上限(2個) */

#define	BUTTON_MAX	KEY88_PAD_BUTTON_MAX	/* ボタン上限(8個)	     */
#define	ITEM_X		(BUTTON_MAX +0)
#define	ITEM_Y		(BUTTON_MAX +1)
#define	ITEM_MAX	(BUTTON_MAX +2)

#define	AXIS_U		0x01
#define	AXIS_D		0x02
#define	AXIS_L		0x04
#define	AXIS_R		0x08

typedef struct {

    int			fd;			/* -1 で未使用 */
    char		*buf;
    int			size;

    int			num;	/* QUASI88 でのジョイスティック番号 0〜 */
    int			axis;			/* 方向ボタン押下状態	*/
    int			nr_button;		/* 有効なボタンの数	*/

    struct {
	struct hid_item	*hitem;
	int		val;
	int		adj;
	int		border;
    } item[ ITEM_MAX ];

} T_JOY_INFO;

static T_JOY_INFO joy_info[ JOY_MAX ];

static	int	joystick_num;		/* オープンしたジョイスティックの数 */




static int set_item(T_JOY_INFO *joy, int idx, struct hid_item *hitem)
{
    joy->item[idx].hitem = malloc(sizeof(*hitem));
    if (joy->item[idx].hitem == NULL) {
	return FALSE;
    }

    memcpy(joy->item[idx].hitem, hitem, sizeof(*hitem));

    joy->item[idx].val = 0;
    joy->item[idx].adj = (hitem->logical_maximum - hitem->logical_minimum) / 2;
    joy->item[idx].border = joy->item[idx].adj * 50 / 100;	/* 閾値 50% */

    /*printf("%d, %d %d\n", idx, joy->item[idx].adj, joy->item[idx].border);*/

    return TRUE;
}



void	joystick_init(void)
{
    int i, j;
    char s[16];
    struct report_desc	*rep_desc;
    int			rep_id;
    struct hid_data	*hdata;
    struct hid_item	hitem;
    int			is_joystick;
    int			error_occur;
    int			nr_button;

    T_JOY_INFO *joy = &joy_info[0];

    joystick_num = 0;

    memset(joy_info, 0, sizeof(joy_info));
    for (i=0; i<JOY_MAX; i++) {
	joy_info[i].fd = -1;
    }


    if (verbose_proc) printf("Initializing joystick ... ");


    for (i=0; i<JOY_MAX; i++, joy++) {

	sprintf(s, "/dev/uhid%d", i);

	joy->fd = open(s, O_RDONLY|O_NONBLOCK);
	if (joy->fd != -1) {

	    rep_desc = hid_get_report_desc(joy->fd);
	    if (rep_desc != NULL) {

		rep_id = 0;

#ifdef __FreeBSD__
# if (__FreeBSD_version >= 460000)
#  if (__FreeBSD_version <= 500111)
		joy->size = hid_report_size(rep_desc, rep_id, hid_input);
#  else
		joy->size = hid_report_size(rep_desc, hid_input, rep_id);
#  endif
# else
		joy->size = hid_report_size(rep_desc, hid_input, &rep_id);
# endif
#else
# ifdef USBHID_NEW
		joy->size = hid_report_size(rep_desc, hid_input, &rep_id);
# else
		joy->size = hid_report_size(rep_desc, hid_input, rep_id);
# endif
#endif
		if (joy->size > 0) {

		    joy->buf = malloc(joy->size);
		    if (joy->buf) {

#if defined(USBHID_NEW) || (defined(__FreeBSD__) && __FreeBSD_version >= 500111)
			hdata = hid_start_parse(rep_desc, 1 << hid_input,
						rep_id);
#else
			hdata = hid_start_parse(rep_desc, 1 << hid_input);
#endif
			if (hdata) {

			    is_joystick = FALSE;
			    error_occur = FALSE;
			    nr_button   = 0;
			    for (j=0; j<ITEM_MAX; j++)
				joy->item[j].hitem = NULL;

			    while (hid_get_item(hdata, &hitem) > 0) {

				switch (hitem.kind) {

				case hid_collection:
				    switch (HID_PAGE(hitem.usage)) {
				    case HUP_GENERIC_DESKTOP:
					switch (HID_USAGE(hitem.usage)) {
					case HUG_JOYSTICK:
					case HUG_GAME_PAD:
					    is_joystick = TRUE;
					    break;
					}
				    }
				    break;

				case hid_input:
				    if (is_joystick) {
					switch (HID_PAGE(hitem.usage)) {
					case HUP_GENERIC_DESKTOP:
					    switch (HID_USAGE(hitem.usage)) {
					    case HUG_X:
						if (set_item(joy,
							     ITEM_X,
							     &hitem) == FALSE)
						    error_occur = TRUE;
						break;
					    case HUG_Y:
						if (set_item(joy,
							     ITEM_Y,
							     &hitem) == FALSE)
						    error_occur = TRUE;
						break;
					    default:
						break;
					    }
					    break;

					case HUP_BUTTON:
					    if (nr_button < BUTTON_MAX) {
						if (set_item(joy,
							     nr_button,
							     &hitem) == FALSE)
						    error_occur = TRUE;
						nr_button ++;
					    }
					    break;

					default:
					    break;
					}
				    }
				    break;

				default:
				    break;
				}

			    }
			    hid_end_parse(hdata);

			    if (is_joystick               &&
				error_occur == FALSE      &&
				joy->item[ 0 ].hitem      &&
				joy->item[ 1 ].hitem      &&
				joy->item[ ITEM_X ].hitem &&
				joy->item[ ITEM_Y ].hitem) {

				/* SUCCESSED !!! */
				joy->num = joystick_num ++;
				joy->nr_button = nr_button;
				continue;
			    }

			    /* FAILED ... */
			    for (j=0; j<ITEM_MAX; j++) {
				if (joy->item[j].hitem)
				    free(joy->item[j].hitem);
			    }
			}
			free(joy->buf);
		    }
		}
		hid_dispose_report_desc(rep_desc);
	    }
	    close(joy->fd);
	    joy->fd = -1;
	}
    }

    if (verbose_proc) printf("%s\n", (joystick_num > 0) ? "OK" : "FAILED");

    return;
}







void	joystick_exit(void)
{
    int i, j;
    T_JOY_INFO *joy = &joy_info[0];

    if (joystick_num > 0) {

	for (i=0; i<JOY_MAX; i++, joy++) {
	    if (joy->fd != -1) {
		for (j=0; j<ITEM_MAX; j++) {
		    if (joy->item[j].hitem) free(joy->item[j].hitem);
		}
		free(joy->buf);
		close(joy->fd);
	    }
	}

	joystick_num = 0;
    }
}




void	joystick_update(void)
{
    int i, j, val, now, chg, offset;
    T_JOY_INFO *joy = &joy_info[0];

    if (joystick_num > 0) {

	for (i=0; i<JOY_MAX; i++, joy++) {

	    if (joy->fd == -1) continue;
	    if (read(joy->fd, joy->buf, joy->size) != joy->size) continue;

	    offset = (joy->num) * KEY88_PAD_OFFSET;

	    for (j=0; j<ITEM_MAX; j++) {

		if (joy->item[j].hitem == NULL) continue;

		val = hid_get_data(joy->buf, joy->item[j].hitem);

		if        (j == ITEM_X) {

		    now = joy->axis & ~(AXIS_L|AXIS_R);

		    val -= joy->item[j].adj;
		    if      (val < -joy->item[j].border) now |= AXIS_L;
		    else if (val > +joy->item[j].border) now |= AXIS_R;

		    chg = joy->axis ^ now;
		    if (chg & AXIS_L) {
			quasi88_pad(KEY88_PAD1_LEFT  + offset, (now & AXIS_L));
		    }
		    if (chg & AXIS_R) {
			quasi88_pad(KEY88_PAD1_RIGHT + offset, (now & AXIS_R));
		    }

		    joy->axis = now;

		} else if (j == ITEM_Y) {

		    now = joy->axis & ~(AXIS_U|AXIS_D);
		
		    val -= joy->item[j].adj;
		    if      (val < -joy->item[j].border) now |= AXIS_U;
		    else if (val > +joy->item[j].border) now |= AXIS_D;

		    chg = joy->axis ^ now;
		    if (chg & AXIS_U) {
			quasi88_pad(KEY88_PAD1_UP   + offset, (now & AXIS_U));
		    }
		    if (chg & AXIS_D) {
			quasi88_pad(KEY88_PAD1_DOWN + offset, (now & AXIS_D));
		    }

		    joy->axis = now;

		} else { /* ボタン */

		    if (joy->item[j].val != val) {
			quasi88_pad(KEY88_PAD1_A + j + offset, (val));
			joy->item[j].val = val;
		    }

		}
		/*
		printf("%d[%d] :%d (%d,%d)\n", i, j,val,
		       joy->item[j].hitem->logical_maximum,
		       joy->item[j].hitem->logical_minimum);
		*/
	    }
	}
    }
}



int	event_get_joystick_num(void)
{
    return joystick_num;
}

#endif	/* JOY_BSD_USB */
