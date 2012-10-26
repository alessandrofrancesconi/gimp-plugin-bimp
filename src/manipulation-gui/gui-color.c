#include <gtk/gtk.h>
#include "gui-color.h"
#include "../bimp-manipulations.h"
#include "../bimp-manipulations-gui.h"
#include "../plugin-intl.h"

GtkWidget *scale_bright, *scale_contrast;
GtkWidget *check_autolevels, *check_grayscale;
GSList *button_group;
	
GtkWidget* bimp_color_gui_new(color_settings settings)
{
	GtkWidget *gui, *hbox_brightness, *hbox_contrast;
	GtkWidget *label_bright, *label_contrast;
	
	gui = gtk_vbox_new(FALSE, 5);
	
	hbox_brightness = gtk_hbox_new(FALSE, 5);
	label_bright = gtk_label_new(_("Brightness"));
	gtk_widget_set_size_request (label_bright, LABEL_BC_W, LABEL_BC_H);
	gtk_misc_set_alignment(GTK_MISC(label_bright), 0.5, 0.8);
	scale_bright = gtk_hscale_new_with_range(-127, 127, 1);
	gtk_range_set_value(GTK_RANGE(scale_bright), settings->brightness);
	gtk_widget_set_size_request (scale_bright, SCALE_BC_W, SCALE_BC_H);
	
	hbox_contrast = gtk_hbox_new(FALSE, 5);
	label_contrast = gtk_label_new(_("Contrast"));
	gtk_widget_set_size_request (label_contrast, LABEL_BC_W, LABEL_BC_H);
	gtk_misc_set_alignment(GTK_MISC(label_contrast), 0.5, 0.8);
	scale_contrast = gtk_hscale_new_with_range(-127, 127, 1);
	gtk_range_set_value(GTK_RANGE(scale_contrast), settings->contrast);
	gtk_widget_set_size_request (scale_contrast, SCALE_BC_W, SCALE_BC_H);
	
	check_grayscale = gtk_check_button_new_with_label(_("Convert to grayscale"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_grayscale), settings->grayscale);
	check_autolevels = gtk_check_button_new_with_label(_("Automatic color levels correction"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_autolevels), settings->levels_auto);
		
	gtk_box_pack_start(GTK_BOX(hbox_brightness), label_bright, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox_brightness), scale_bright, FALSE, FALSE, 0);
	
	gtk_box_pack_start(GTK_BOX(hbox_contrast), label_contrast, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox_contrast), scale_contrast, FALSE, FALSE, 0);
		
	gtk_box_pack_start(GTK_BOX(gui), hbox_brightness, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(gui), hbox_contrast, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(gui), check_grayscale, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(gui), check_autolevels, FALSE, FALSE, 0);
		
	return gui;
}

void bimp_color_save(color_settings orig_settings)
{
	orig_settings->brightness = gtk_range_get_value(GTK_RANGE(scale_bright));
	orig_settings->contrast = gtk_range_get_value(GTK_RANGE(scale_contrast));
	orig_settings->grayscale = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_grayscale));
	orig_settings->levels_auto = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_autolevels));
}








