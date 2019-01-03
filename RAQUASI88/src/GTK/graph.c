/***********************************************************************
 * グラフィック処理 (システム依存)
 *
 *	詳細は、 graph.h 参照
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>

#include "quasi88.h"
#include "graph.h"
#include "device.h"


int	use_gdk_image = TRUE;			/* 真で、GdkImageを使用 */

static	T_GRAPH_SPEC	graph_spec;		/* 基本情報		*/

static	int		graph_exist;		/* 真で、画面生成済み	*/
static	T_GRAPH_INFO	graph_info;		/* その時の、画面情報	*/


/************************************************************************
 *	グラフィック処理の初期化
 *	グラフィック処理の動作
 *	グラフィック処理の終了
 ************************************************************************/

static	GdkVisual 	*visual;
static	GdkColormap	*colormap;

const T_GRAPH_SPEC	*graph_init(void)
{
    if (verbose_proc) {
	printf("Initializing Graphic System ... ");
    }

    /* ??? よくわからん */
    {
	GtkStyle *style = gtk_widget_get_default_style();
	if (style != NULL) {
	    GdkFont* font = NULL;
#if 0
#ifdef ウインドウズ？
	    font = gdk_font_load("-*-*-*-*-*--*-*-*-*-*-*-windows-shiftjis");
#else
#ifdef __CYGWIN__
	    font = gdk_fontset_load("-*-*-*-*-*--*-*-*-*-*-*-iso8859-1,"
				    "-*-*-*-*-*--*-*-*-*-*-*-jisx0208.*-*");
#endif
#endif
#endif
	    if (font != NULL) {
		if (style->font != NULL) {
		    gdk_font_unref(style->font);
		}
		style->font = font;
		gdk_font_ref(style->font);
		gtk_widget_set_default_style(style);
	    }
	}
    }

    /* やはりわからん */
    {
	int found = FALSE;
	GdkVisual *v = gdk_visual_get_system();

	if (use_gdk_image) {
	    if (v->type == GDK_VISUAL_TRUE_COLOR ||
		v->type == GDK_VISUAL_PSEUDO_COLOR) {
		if (v->depth == 32) {
#ifdef	SUPPORT_32BPP
		    found = TRUE;
#endif
		} else if (v->depth == 16 ||
			   v->depth == 15) {
#ifdef	SUPPORT_16BPP
		    found = TRUE;
#endif
		} else if (v->depth == 8) {
#ifdef	SUPPORT_8BPP
		    found = TRUE;
#endif
		}
	    }

	    if (found) {
#ifdef	LSB_FIRST
		if (v->byte_order != GDK_LSB_FIRST) found = FALSE;
#else
		if (v->byte_order != GDK_MSB_FIRST) found = FALSE;
#endif
	    }
	}

	if (found) {
	    use_gdk_image = TRUE;

	    visual   = v;
	    colormap = gdk_colormap_get_system();
	} else {
	    use_gdk_image = FALSE;

#ifdef	SUPPORT_8BPP

	    gdk_rgb_init();
	    gtk_widget_set_default_colormap(gdk_rgb_get_cmap());
	    gtk_widget_set_default_visual(gdk_rgb_get_visual());

	    visual   = gdk_rgb_get_visual();	/* 未使用だけど */
	    colormap = gdk_rgb_get_cmap();

#else
	    visual = NULL;
	    colormap = NULL;
#endif
	}
    }

    if (visual && colormap) {

	graph_spec.window_max_width      = 10000;
	graph_spec.window_max_height     = 10000;
	graph_spec.fullscreen_max_width  = 0;
	graph_spec.fullscreen_max_height = 0;
	graph_spec.forbid_status         = FALSE;
	graph_spec.forbid_half           = FALSE;

	if (verbose_proc)
	    printf("OK (%s)\n", (use_gdk_image) ? "GdkImage" : "GdkRGB");

	return &graph_spec;

    } else {
	if (verbose_proc) printf("FAILED\n");
	return NULL;
    }
}

/************************************************************************/
static	GtkWidget	*main_window;
static	GtkWidget	*menu_bar;
static	GtkWidget	*drawing_area;

static	GdkGC		*graphic_context;

static int create_image(int width, int height);

const T_GRAPH_INFO	*graph_setup(int width, int height,
				     int fullscreen, double aspect)
{
    GtkWidget *vbox;

    /* fullscreen, aspect は未使用 */

    if (graph_exist == FALSE) {

	/* ウインドウを生成する */
	main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	{
	    gtk_window_set_policy(GTK_WINDOW(main_window), FALSE, FALSE, TRUE);
	    gtksys_set_signal_frame(main_window);
	}

	/* メニューバーを生成する */
	{
	    create_menubar(main_window, &menu_bar);
	    gtk_widget_show(menu_bar);
	}


	/* 描画領域を生成する */
	drawing_area = gtk_drawing_area_new();
	{
	    gtk_drawing_area_size(GTK_DRAWING_AREA(drawing_area),
				  width, height);
	    gtksys_set_signal_view(drawing_area);
	}
	gtk_widget_show(drawing_area);


	/* アクセラレーター */


	/* メニューバーと描画領域をパックして表示 */
	vbox = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(vbox);
	gtk_box_pack_start(GTK_BOX(vbox), menu_bar, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), drawing_area, FALSE, FALSE, 0);

	gtk_container_add(GTK_CONTAINER(main_window), vbox);
	gtk_widget_show(main_window);


	/* グラフィックコンテキストの設定 (表示後でないとだめ) ? */
	graphic_context = gdk_gc_new(drawing_area->window);
    }

    if (create_image(width, height)) {

	gtk_drawing_area_size(GTK_DRAWING_AREA(drawing_area),
			      width, height);

	graph_exist = TRUE;

	return &graph_info;
    }

    return NULL;
}

/*----------------------------------------------------------------------*/

static	GdkImage	*image;
static	guchar		*indexbuf;
static	GdkRgbCmap	indexrgb;

static void color_trash(void);

static int create_image(int width, int height)
{
    if (use_gdk_image) {

	if (image) {
	    color_trash();

	    gdk_image_destroy(image);
	    image = NULL;
	}

	image = gdk_image_new(GDK_IMAGE_FASTEST,
			      visual,
			      width, height);

	if (image == NULL) return FALSE;

	graph_info.byte_per_pixel	= image->bpp;
	graph_info.byte_per_line	= image->bpl;
	graph_info.buffer		= image->mem;

    } else {

	if (indexbuf) {
	    color_trash();

	    free(indexbuf);
	    indexbuf = NULL;
	}

	indexbuf = malloc(width * height * sizeof(guchar));

	if (indexbuf == NULL) return FALSE;

	memset(indexbuf, 0, width * height * sizeof(guchar));

	graph_info.byte_per_pixel	= sizeof(guchar);
	graph_info.byte_per_line	= width;
	graph_info.buffer		= indexbuf;
    }

    graph_info.fullscreen	= FALSE;
    graph_info.width		= width;
    graph_info.height		= height;
    graph_info.nr_color		= 255;
    graph_info.write_only	= FALSE;
    graph_info.broken_mouse	= FALSE;
    graph_info.draw_start	= NULL;
    graph_info.draw_finish	= NULL;
    graph_info.dont_frameskip	= FALSE;

    return TRUE;
}



/************************************************************************/

void	graph_exit(void)
{
}

/************************************************************************
 *	色の確保
 *	色の解放
 ************************************************************************/
static	GdkColor	color_cell[256];	/* 確保した色の内容 */
static	int		nr_color_cell;		/* 確保した色の個数 */

void	graph_add_color(const PC88_PALETTE_T color[],
			int nr_color, unsigned long pixel[])
{
    int i;
    gboolean success[256];

    for (i = 0; i < nr_color; i++) {
	color_cell[nr_color_cell + i].red   = (gushort)color[i].red   << 8;
	color_cell[nr_color_cell + i].green = (gushort)color[i].green << 8;
	color_cell[nr_color_cell + i].blue  = (gushort)color[i].blue  << 8;
    }

    i = gdk_colormap_alloc_colors(colormap,
				  &color_cell[ nr_color_cell ],
				  nr_color,
				  FALSE,
				  FALSE,
				  success);

    /* debug */
    if (i != 0) printf("Color Alloc Failed %d/%d\n", i, nr_color);


    if (use_gdk_image) {

	for (i = 0; i < nr_color; i++) {
	    pixel[i] = color_cell[nr_color_cell + i].pixel;
	}

    } else {

	for (i = 0; i < nr_color; i++) {
	    indexrgb.colors[nr_color_cell + i] =
		(((guint32)color_cell[nr_color_cell + i].red   & 0xff00) << 8)|
		 ((guint32)color_cell[nr_color_cell + i].green & 0xff00)      |
		 ((guint32)color_cell[nr_color_cell + i].blue            >> 8);
	    indexrgb.lut[nr_color_cell + i] =
		(guchar)color_cell[nr_color_cell + i].pixel;

	    pixel[i] = nr_color_cell + i;
	}

    }

    nr_color_cell += nr_color;
}

/************************************************************************/

void	graph_remove_color(int nr_pixel, unsigned long pixel[])
{
    nr_color_cell -= nr_pixel;

    gdk_colormap_free_colors(colormap,
			     &color_cell[ nr_color_cell ],
			     nr_pixel);
}

/*----------------------------------------------------------------------*/

static void color_trash(void)
{
    if (nr_color_cell) {
	gdk_colormap_free_colors(colormap,
				 &color_cell[ 0 ],
				 nr_color_cell);
    }

    nr_color_cell = 0;
}


/************************************************************************
 *	グラフィックの更新
 ************************************************************************/

void	graph_update(int nr_rect, T_GRAPH_RECT rect[])
{
    int i;

    if (use_gdk_image) {
	for (i=0; i<nr_rect; i++) {
	    gdk_draw_image(drawing_area->window, graphic_context,
			   image,
			   rect[i].x, rect[i].y,
			   rect[i].x, rect[i].y,
			   rect[i].width, rect[i].height);
	}

    } else {

	gdk_draw_indexed_image(drawing_area->window, graphic_context,
			       0, 0,
			       graph_info.width, graph_info.height,
			       GDK_RGB_DITHER_NONE,
			       indexbuf,
			       graph_info.width,
			       &indexrgb);
    }
}



/************************************************************************
 *	タイトルの設定
 *	属性の設定
 ************************************************************************/

void	graph_set_window_title(const char *title)
{
    if (main_window) {
	gtk_window_set_title(GTK_WINDOW(main_window), title);
    }
}

/************************************************************************/

static	int	gtksys_keyrepeat_on = TRUE;

void	gtksys_set_attribute_focus_out(void)
{
    gdk_key_repeat_restore();
}

void	gtksys_set_attribute_focus_in(void)
{
    if (gtksys_keyrepeat_on == FALSE) {
	gdk_key_repeat_disable();
    } else {
	gdk_key_repeat_restore();
    }
}

void	graph_set_attribute(int mouse_show, int grab, int keyrepeat_on)
{
    /* マウスは未対応 */
    /* グラブは未対応 */
    gtksys_keyrepeat_on = keyrepeat_on;

    if (gtksys_get_focus) {
	gtksys_set_attribute_focus_in();
    }
}
