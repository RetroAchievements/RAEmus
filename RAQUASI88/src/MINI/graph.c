/***********************************************************************
 * グラフィック処理 (システム依存)
 *
 *	詳細は、 graph.h 参照
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "quasi88.h"
#include "graph.h"
#include "device.h"


static	T_GRAPH_SPEC	graph_spec;		/* 基本情報		*/

static	int		graph_exist;		/* 真で、画面生成済み	*/
static	T_GRAPH_INFO	graph_info;		/* その時の、画面情報	*/


/************************************************************************
 *	グラフィック処理の初期化
 *	グラフィック処理の動作
 *	グラフィック処理の終了
 ************************************************************************/

const T_GRAPH_SPEC	*graph_init(void)
{
    if (verbose_proc) {
	printf("Initializing Graphic System ... ");
    }

    graph_spec.window_max_width      = 640;
    graph_spec.window_max_height     = 480;
    graph_spec.fullscreen_max_width  = 0;
    graph_spec.fullscreen_max_height = 0;
    graph_spec.forbid_status         = FALSE;
    graph_spec.forbid_half           = FALSE;

    if (verbose_proc)
	printf("OK\n");

    return &graph_spec;
}

/************************************************************************/

static unsigned char *buffer = NULL;

const T_GRAPH_INFO	*graph_setup(int width, int height,
				     int fullscreen, double aspect)
{
    if (graph_exist == FALSE) {

	/* まだウインドウが無いならば、ウインドウを生成する */

	if (buffer == NULL) {
	    buffer = malloc(width * height * sizeof(unsigned short));
	    if (buffer == FALSE) {
		return NULL;
	    }
	}
    } else {

	/* すでにウインドウが有るならば、リサイズする */
    }

    /* graph_info に諸言をセットする */

    graph_info.fullscreen	= FALSE;
    graph_info.width		= width;
    graph_info.height		= height;
    graph_info.byte_per_pixel	= 2;
    graph_info.byte_per_line	= width * 2;
    graph_info.buffer		= buffer;
    graph_info.nr_color		= 255;
    graph_info.write_only	= FALSE;
    graph_info.broken_mouse	= FALSE;
    graph_info.draw_start	= NULL;
    graph_info.draw_finish	= NULL;
    graph_info.dont_frameskip	= FALSE;

    graph_exist = TRUE;

    return &graph_info;
}

/************************************************************************/

void	graph_exit(void)
{
    if (buffer) {
	free(buffer);
    }
}

/************************************************************************
 *	色の確保
 *	色の解放
 ************************************************************************/

void	graph_add_color(const PC88_PALETTE_T color[],
			int nr_color, unsigned long pixel[])
{
    int i;
    for (i=0; i<nr_color; i++) {
	pixel[i] = ((((unsigned long) color[i].red   >> 3) << 10) |
	    	    (((unsigned long) color[i].green >> 3) <<  5) |
	    	    (((unsigned long) color[i].blue  >> 3)));
    }
}

/************************************************************************/

void	graph_remove_color(int nr_pixel, unsigned long pixel[])
{
}

/************************************************************************
 *	グラフィックの更新
 ************************************************************************/

void	graph_update(int nr_rect, T_GRAPH_RECT rect[])
{
}


/************************************************************************
 *	タイトルの設定
 *	属性の設定
 ************************************************************************/

void	graph_set_window_title(const char *title)
{
}

/************************************************************************/

void	graph_set_attribute(int mouse_show, int grab, int keyrepeat_on)
{
}
