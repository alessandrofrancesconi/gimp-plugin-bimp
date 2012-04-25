/* 
 * Functions to open a popup dialog and edit the selected manipulation
 */

#include <gtk/gtk.h>
#include <libgimpbase/gimpbase.h>
#include <string.h>
#include "bimp-manipulations-gui.h"
#include "bimp-manipulations.h"
#include "bimp-gui.h"

#include "manipulation-gui/gui-resize.h"
#include "manipulation-gui/gui-crop.h"
#include "manipulation-gui/gui-fliprotate.h"
#include "manipulation-gui/gui-color.h"
#include "manipulation-gui/gui-sharp.h"
#include "manipulation-gui/gui-changeformat.h"
#include "manipulation-gui/gui-watermark.h"
#include "manipulation-gui/gui-rename.h"
#include "manipulation-gui/gui-userdef.h"


void bimp_open_editwindow(manipulation man, gboolean first_time) 
{
	GtkWidget* window, *vbox, *label_title;
	GtkWidget* gui;
	gint result;
	
	void (*save)(manipulation_settings) = NULL; /* Pointer to the correct setting saving function */
	
	window = gtk_dialog_new_with_buttons(
		manipulation_type_string[man->type],
		GTK_WINDOW(bimp_window_main),
		GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_STOCK_OK,
		GTK_RESPONSE_ACCEPT,
		GTK_STOCK_CANCEL,
		GTK_RESPONSE_REJECT,
		NULL
	);
	gtk_window_set_resizable (GTK_WINDOW(window), FALSE);
	gtk_container_set_border_width(GTK_CONTAINER(window), 10);
	
	vbox = gtk_vbox_new(FALSE, 10);
	label_title = gtk_label_new(NULL);
	gchar* titletext = g_strconcat("<big><b>", manipulation_type_string[man->type], "</b></big>", NULL); /* dialog title */
	gtk_label_set_markup (GTK_LABEL (label_title), titletext);
	gtk_box_pack_start(GTK_BOX(vbox), label_title, FALSE, FALSE, 0);
	
	if (man->type == MANIP_RESIZE) {
		gtk_widget_set_size_request (window, RESIZE_WINDOW_W, RESIZE_WINDOW_H);
		gui = bimp_resize_gui_new((resize_settings)(man->settings));
		save = &bimp_resize_save;
	}
	else if (man->type == MANIP_CROP) {
		gtk_widget_set_size_request (window, CROP_WINDOW_W, CROP_WINDOW_H);
		gui = bimp_crop_gui_new((crop_settings)(man->settings));
		save = &bimp_crop_save;
	}
	else if (man->type == MANIP_FLIPROTATE) {
		gtk_widget_set_size_request (window, FLIPROTATE_WINDOW_W, FLIPROTATE_WINDOW_H);
		gui = bimp_fliprotate_gui_new((fliprotate_settings)(man->settings));
		save = &bimp_fliprotate_save;
	}
	else if (man->type == MANIP_COLOR) {
		gtk_widget_set_size_request (window, COLOR_WINDOW_W, COLOR_WINDOW_H);
		gui = bimp_color_gui_new((color_settings)(man->settings));
		save = &bimp_color_save;
	}
	else if (man->type == MANIP_SHARPEN) {
		gtk_widget_set_size_request (window, SHARP_WINDOW_W, SHARP_WINDOW_H);
		gui = bimp_sharp_gui_new((sharp_settings)(man->settings));
		save = &bimp_sharp_save;
	}
	else if (man->type == MANIP_CHANGEFORMAT) {
		gtk_widget_set_size_request (window, CHANGEFORMAT_WINDOW_W, CHANGEFORMAT_WINDOW_H);
		gui = bimp_changeformat_gui_new((changeformat_settings)(man->settings));
		save = &bimp_changeformat_save;
	}
	else if (man->type == MANIP_WATERMARK) {
		gtk_widget_set_size_request (window, WATERMARK_WINDOW_W, WATERMARK_WINDOW_H);
		gui = bimp_watermark_gui_new((watermark_settings)(man->settings));
		save = &bimp_watermark_save;
	}
	else if (man->type == MANIP_RENAME) {
		gtk_widget_set_size_request (window, RENAME_WINDOW_W, RENAME_WINDOW_H);
		gui = bimp_rename_gui_new((rename_settings)(man->settings), window);
		save = &bimp_rename_save;
	}
	else if (man->type == MANIP_USERDEF) {
		gtk_widget_set_size_request (window, USERDEF_WINDOW_W, USERDEF_WINDOW_H);
		gui = bimp_userdef_gui_new((userdef_settings)(man->settings), window);
		save = &bimp_userdef_save;
	}
	else {
		gtk_widget_destroy (window);
		return;
	}
	
	gtk_container_add (GTK_CONTAINER (vbox), gui);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(window)->vbox), vbox);
	
	gtk_widget_show_all(window);
	result = gtk_dialog_run (GTK_DIALOG (window));
	/* if the users clicks on 'OK', the proper saving function for this manipulation is called.
	 * but if the users clicks 'CANCEL' and it was the first time the users selected this manipulation, it's removed from the list */
	if (result == GTK_RESPONSE_ACCEPT) {
		save(man->settings);
	}
	else if (result == GTK_RESPONSE_REJECT && first_time == TRUE){ 
		bimp_remove_manipulation(man);
		bimp_refresh_sequence_panel();
	}
	
	gtk_widget_destroy (window);
}
