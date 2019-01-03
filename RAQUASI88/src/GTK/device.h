#ifndef DEVICE_H_INCLUDED
#define DEVICE_H_INCLUDED


#include <gtk/gtk.h>


/*
 *	src/GTK/ 以下でのグローバル変数
 */
extern	int	gtksys_get_focus;	/* 現在、フォーカスありかどうか	*/



/*
 *	src/GTK/ 以下でのグローバル変数 (オプション設定可能な変数)
 */
extern	int	use_gdk_image;		/* 真で、GdkImageを使用 */



void	gtksys_set_signal_frame(GtkWidget *main_window);
void	gtksys_set_signal_view(GtkWidget *drawing_area);

void	gtksys_set_attribute_focus_in(void);
void	gktsys_set_attribute_focus_out(void);

void	create_menubar(GtkWidget *target_window,
		       GtkWidget **created_menubar);
void	menubar_setup(int active);



#endif	/* DEVICE_H_INCLUDED */
